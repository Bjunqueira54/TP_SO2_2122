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
	FlowControl* fc = (FlowControl*) param;

	int v_current_water_posx = -1;
	int v_current_water_posy = -1;
	Side v_water_dir;

	_tprintf(L"10 Seconds until water flows...\n");
	drawBoardToConsole(&fc->gb);
	//Sleep(10000);

	//Get the console handle and clear the screen
	HANDLE hStdout;
	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	cls(hStdout);

	//Find water start location
	for (int y = 0; y < fc->gb.y; y++)
	{
		for (int x = 0; x < fc->gb.x; x++)
		{
			if (fc->gb.board[y][x].isStart)
			{
				v_current_water_posx = x;
				v_current_water_posy = y;
				switch (fc->gb.board[y][x].side)
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

	while (fc->gb.isGameRunning)
	{
		Sleep(1000);

		if (!fc->gb.isWaterRunning) continue;

		fc->gb.board[v_current_water_posx][v_current_water_posy].isFlooded = TRUE;

		PieceType v_current_cell_piece = fc->gb.board[v_current_water_posx][v_current_water_posy].piece;

		if(v_current_cell_piece == E)
		{
			fc->gb.isGameRunning = FALSE;
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
			fc->gb.isGameRunning = FALSE;
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

		cls(hStdout);
		drawBoardToConsole(&fc->gb);
	}

	return 0;
}

DWORD WINAPI cmdControlThread(LPVOID param)
{
	FlowControl* fc = (FlowControl*) param;
	TCHAR cmd[CMD_MAX_LENGHT] = L"";

	while (fc->gb.isGameRunning)
	{
		DWORD eventResult = WaitForSingleObject(fc->hEvent, INFINITE);

		if (fc->gb.isGameRunning == FALSE) break;

		if (eventResult == WAIT_OBJECT_0)	//new event, what could this mean? :o
		{
			DWORD mutexResult = WaitForSingleObject(fc->hMutex, INFINITE);

			if (mutexResult == WAIT_OBJECT_0)
			{
				//CopyMemory(cmd, getLastCmdFromMemory(), sizeof(TCHAR) * CMD_MAX_LENGHT);	//I wonder if this works :o
			}
			
			ReleaseMutex(fc->hMutex);
		}
	}

	return 0;
}