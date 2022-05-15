#pragma once

#include <Windows.h>
#include <tchar.h>

#include "Functions.h"
#include "..\Global Data Structures\DrawingFunctions.h"

GameBoard* initGameboard()
{
	///////////////////////////
	///Fecth Registry Values///
	///////////////////////////



	////////////////////////
	///Allocate the board///
	////////////////////////

	GameBoard* gb = (GameBoard*) malloc(sizeof(GameBoard));

	if (gb == NULL) return NULL;

	gb->y = 20;	//Registry Values
	gb->x = 20;	//Replace later with correct values

	gb->isWaterRunning = FALSE;
	gb->isGameRunning = FALSE;

	///////////////
	///Set cells///
	///////////////

	for (int y = 0; y < gb->y; y++)
	{
		for (int x = 0; x < gb->x; x++)
		{
			gb->board[y][x].piece = E;			//All cells are empty
			gb->board[y][x].isStart = FALSE;	//No cell is the start yet
			gb->board[y][x].isEnd = FALSE;		//No cell is the end yet
			gb->board[y][x].isFlooded = FALSE;	//No water at the start of the game
			gb->board[y][x].side = NO;			//Since no start or end, set to NO
			gb->board[y][x].isEnabled = TRUE;
		}
	}

	///////////////////////
	///Choose start cell///
	///////////////////////

	gb->board[gb->y - 1][gb->x - 1].isStart = TRUE;
	gb->board[gb->y - 1][gb->x - 1].isEnd = FALSE;	//Redundant, but better safe than sorry.
	gb->board[gb->y - 1][gb->x - 1].side = S;		//Water begins flowing from this side. THIS IS NOT WATER DIRECTION.

	//////////////////////////////////
	///Choose end cell and the side///
	//////////////////////////////////

	gb->board[0][0].isEnd = TRUE;
	gb->board[0][0].isStart = FALSE;	//Redundant, but better safe than sorry.
	gb->board[0][0].side = L;

	return gb;
}

DWORD WINAPI waterControlThread(LPVOID param)
{
	_tprintf(L"waterControlThread: started\n");
	Data* data = (Data*) param;
	FlowControl* fc = data->fc;

	int v_current_water_posx = -1;
	int v_current_water_posy = -1;
	Side v_water_dir;

	//Find water start location
	for (int y = 0; y < fc->gameboard.y; y++)
	{
		for (int x = 0; x < fc->gameboard.x; x++)
		{
			if (fc->gameboard.board[y][x].isStart)
			{
				v_current_water_posx = x;
				v_current_water_posy = y;
				switch (fc->gameboard.board[y][x].side)
				{
					case S:
						v_water_dir = N;
						break;
					case N:
						v_water_dir = S;
						break;
					case R:
						v_water_dir = L;
						break;
					case L:
						v_water_dir = R;
						break;
					default:
						v_water_dir = NO;
						break;
				}
				goto StartFound;	//Break out of for()
			}
		}
	}

//Don't kill me :)
StartFound:

	if (v_current_water_posx == -1 || v_current_water_posy == -1 || v_water_dir == NO)
	{
		_tprintf(L"Uninitialized water start position!\n");
		return -1;
	}

	_tprintf(L"waterControlThread: Found start position\n");

	while (fc->gameboard.isGameRunning)
	{
		Sleep(1000);

		if (!fc->gameboard.isWaterRunning) continue;

		_tprintf(L"waterControlThread: water is flowing\n");

		if (WaitForSingleObject(data->hMutex, INFINITE) == WAIT_OBJECT_0)
		{
			_tprintf(L"waterControlThread: I'm controlling the mutex\n");
			fc->gameboard.board[v_current_water_posx][v_current_water_posy].isFlooded = TRUE;

			PieceType v_current_cell_piece = fc->gameboard.board[v_current_water_posx][v_current_water_posy].piece;

			if(v_current_cell_piece == E)
			{
				fc->gameboard.isGameRunning = FALSE;
				_tprintf(L"You Lost!\n");
			}
			//Yes, I'm nuts. How could you tell?
			else if ((v_water_dir == L &&	(v_current_cell_piece == V		||
											v_current_cell_piece == UR		||
											v_current_cell_piece == DR))	||
					(v_water_dir == R &&	(v_current_cell_piece == V		||
											v_current_cell_piece == UL		||
											v_current_cell_piece == DL))	||
					(v_water_dir == S &&	(v_current_cell_piece == H		||
											v_current_cell_piece == DL		||
											v_current_cell_piece == DR))	||
					(v_water_dir == N &&	(v_current_cell_piece == H		||
											v_current_cell_piece == UL		||
											v_current_cell_piece == UR)))
			{
				fc->gameboard.isGameRunning = FALSE;
				_tprintf(L"You Lost!\n");
			}
			else if (	(v_water_dir == L && v_current_cell_piece == DL) ||
						(v_water_dir == R && v_current_cell_piece == DR))
			{
				v_water_dir = S;
			}
			else if (	(v_water_dir == L && v_current_cell_piece == UL) ||
						(v_water_dir == R && v_current_cell_piece == UR))
			{
				v_water_dir = N;
			}
			else if (	(v_water_dir == N && v_current_cell_piece == UL) ||
						(v_water_dir == S && v_current_cell_piece == DL))
			{
				v_water_dir = L;
			}
			else if (	(v_water_dir == N && v_current_cell_piece == UR) ||
						(v_water_dir == S && v_current_cell_piece == DR))
			{
				v_water_dir = R;
			}
		}
		SetEvent(data->hBoardEvent);
		_tprintf(L"waterControlThread: I've SET the BoardEvent\n");
		ReleaseMutex(data->hMutex);
		_tprintf(L"waterControlThread: I've released the mutex\n");
	}

	return 0;
}

DWORD WINAPI cmdControlThread(LPVOID param)
{
	_tprintf(L"cmdControlThread: I'm starting\n");
	Data* data = (Data*) param;
	FlowControl* fc = data->fc;

	while (fc->gameboard.isGameRunning)
	{
		_tprintf(L"cmdControlThread: I'm waiting for CommandEvent\n");
		if (WaitForSingleObject(data->hCommandEvent, INFINITE) == WAIT_OBJECT_0)	//new event, what could this mean? :o
		{
			_tprintf(L"cmdControlThread: I've caught CommandEvent\n");
			if (fc->gameboard.isGameRunning == FALSE) break;

			_tprintf(L"cmdControlThread: I'm waiting to control the mutex\n");
			if (WaitForSingleObject(data->hMutex, INFINITE) == WAIT_OBJECT_0)	//Take control of the mutex
			{
				_tprintf(L"cmdControlThread: I'm controlling the mutex\n");
				int out = fc->buffer.out;
				_tprintf(L"Received from server: %s\n", fc->buffer.cmdBuffer[out]);

				//update buffer
				out = (out + 1) % DIM;
				fc->buffer.out = out;

				ResetEvent(data->hCommandEvent);
				_tprintf(L"cmdControlThread: I've RESET the CommandEvent\n");
			}
			ReleaseMutex(data->hMutex);
			_tprintf(L"cmdControlThread: I've RELEASED the mutex\n");
		}
	}

	return 0;
}