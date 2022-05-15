#pragma once

#include <Windows.h>
#include <tchar.h>

#include "Functions.h"
#include "..\Global Data Structures\DrawingFunctions.h"
#include "parser.h"

typedef struct
{
	Data* data;

	PieceType pipe_type;
	int pipe_y;
	int pipe_x;
} PipeInfo;
typedef struct
{
	Data* data;

	DWORD stopWaterMili;
} WaterStop;
typedef struct
{
	Data* data;

	int y, x;
} WallInfo;


DWORD WINAPI addPipeThread(LPVOID param)
{
	PipeInfo* pi = (PipeInfo*)param;
	if (pi == NULL) return -1;

	Data* data = pi->data;

	if (WaitForSingleObject(data->hMutex, INFINITE) == WAIT_OBJECT_0)
	{
		UnmapViewOfFile(data->fc);
		data->fc = (FlowControl*)MapViewOfFile(data->hGameMemory, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(FlowControl));
		if (data->fc == NULL)
		{
			ReleaseMutex(data->hMutex);
			return -1;
		}
		if (data->fc->gameboard.board[pi->pipe_y][pi->pipe_x].isEnabled == TRUE && data->fc->gameboard.board[pi->pipe_y][pi->pipe_x].isWall == FALSE)
			data->fc->gameboard.board[pi->pipe_y][pi->pipe_x].piece = pi->pipe_type;

	}
	ReleaseMutex(data->hMutex);
	SetEvent(data->hBoardEvent);

	return 0;
}
DWORD WINAPI stopWaterThread(LPVOID param)
{
	WaterStop* ws = (WaterStop*)param;
	if (ws == NULL) return -1;

	Data* data = ws->data;
	if (WaitForSingleObject(data->hMutex, INFINITE) == WAIT_OBJECT_0)
	{
		UnmapViewOfFile(data->fc);
		data->fc = (FlowControl*)MapViewOfFile(data->hGameMemory, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(FlowControl));
		if (data->fc == NULL) return -1;


		data->fc->gameboard.isWaterRunning = FALSE;
	}
	ReleaseMutex(data->hMutex);

	Sleep(ws->stopWaterMili);

	if (WaitForSingleObject(data->hMutex, INFINITE) == WAIT_OBJECT_0)
		data->fc->gameboard.isWaterRunning = TRUE;
	ReleaseMutex(data->hMutex);

	return 0;
}
DWORD WINAPI placeWallThread(LPVOID param)
{
	WallInfo* wi = (WallInfo*)param;
	if (wi == NULL) return -1;

	Data* data = wi->data;
	if (data == NULL) return -1;

	if (WaitForSingleObject(data->hMutex, INFINITE) == WAIT_OBJECT_0)
	{
		UnmapViewOfFile(data->fc);
		data->fc = (FlowControl*)MapViewOfFile(data->hGameMemory, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(FlowControl));
		if (data->fc == NULL)
		{
			ReleaseMutex(data->hMutex);
			return -1;
		}

		data->fc->gameboard.board[wi->y][wi->x].isWall = TRUE;
	}
	ReleaseMutex(data->hMutex);
	SetEvent(data->hBoardEvent);

	return 0;
}

DWORD WINAPI waterControlThread(LPVOID param)
{
	Data* data = (Data*) param;

	if (WaitForSingleObject(data->hMutex, INFINITE) == WAIT_OBJECT_0)
	{
		UnmapViewOfFile(data->fc);
		data->fc = (FlowControl*)MapViewOfFile(data->hGameMemory, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(FlowControl));
		if (data->fc == NULL)
		{
			ReleaseMutex(data->hMutex);
			return -1;
		}
	}
	ReleaseMutex(data->hMutex);

	int v_current_water_posx = -1;
	int v_current_water_posy = -1;
	Side v_water_dir = NO;

	//Find water start location
	if(WaitForSingleObject(data->hMutex, INFINITE) == WAIT_OBJECT_0)
	{
		for (int y = 0; y < data->fc->gameboard.y; y++)
		{
			for (int x = 0; x < data->fc->gameboard.x; x++)
			{
				if (data->fc->gameboard.board[y][x].isStart)
				{
					v_current_water_posx = x;
					v_current_water_posy = y;
					switch (data->fc->gameboard.board[y][x].side)
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
				}

				if (v_water_dir != NO) break;
			}
			if (v_water_dir != NO) break;
		}
	}
	ReleaseMutex(data->hMutex);

	if (v_current_water_posx == -1 || v_current_water_posy == -1 || v_water_dir == NO)
	{
		_tprintf(L"Uninitialized water start position!\n");
		return -1;
	}

	while (TRUE)
	{
		if (WaitForSingleObject(data->hMutex, INFINITE) == WAIT_OBJECT_0)
		{
			UnmapViewOfFile(data->fc);
			data->fc = (FlowControl*)MapViewOfFile(data->hGameMemory, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(FlowControl));
			if (data->fc == NULL)
			{
				ReleaseMutex(data->hMutex);
				break;	//I mean, it's gonna crash anyway
			}

			if (!data->fc->gameboard.isGameRunning)
			{
				ReleaseMutex(data->hMutex);
				return 0;
			}
			if (!data->fc->gameboard.isWaterRunning)
			{
				ReleaseMutex(data->hMutex);
				continue;
			}

			data->fc->gameboard.board[v_current_water_posy][v_current_water_posx].isFlooded = TRUE;

			PieceType v_current_cell_piece = data->fc->gameboard.board[v_current_water_posy][v_current_water_posx].piece;

			//Yes, I'm nuts. How could you tell?
			if ((v_water_dir == L &&		(v_current_cell_piece == V		||
											v_current_cell_piece == UL		||
											v_current_cell_piece == DL))	||
					(v_water_dir == R &&	(v_current_cell_piece == V		||
											v_current_cell_piece == UR		||
											v_current_cell_piece == DR))	||
					(v_water_dir == S &&	(v_current_cell_piece == H		||
											v_current_cell_piece == DL		||
											v_current_cell_piece == DR))	||
					(v_water_dir == N &&	(v_current_cell_piece == H		||
											v_current_cell_piece == UL		||
											v_current_cell_piece == UR))	||
					(v_current_cell_piece == E))
			{
				data->fc->gameboard.isGameRunning = FALSE;
				_tprintf(L"You Lost!\n");
				ReleaseMutex(data->hMutex);
				SetEvent(data->hBoardEvent);
				return 0;
			}
			else if (	(v_water_dir == L && v_current_cell_piece == DR) ||
						(v_water_dir == R && v_current_cell_piece == DL))
			{
				v_water_dir = S;
			}
			else if (	(v_water_dir == L && v_current_cell_piece == UR) ||
						(v_water_dir == R && v_current_cell_piece == UL))
			{
				v_water_dir = N;
			}
			else if (	(v_water_dir == N && v_current_cell_piece == DL) ||
						(v_water_dir == S && v_current_cell_piece == UL))
			{
				v_water_dir = L;
			}
			else if (	(v_water_dir == N && v_current_cell_piece == DR) ||
						(v_water_dir == S && v_current_cell_piece == UR))
			{
				v_water_dir = R;
			}

			//check victory
			if (data->fc->gameboard.board[v_current_water_posy][v_current_water_posx].isEnd == TRUE	&&
				v_water_dir == data->fc->gameboard.board[v_current_water_posy][v_current_water_posx].side)
			{
				data->fc->gameboard.isGameRunning = FALSE;
				_tprintf(L"You Win!\n");
				SetEvent(data->hBoardEvent);
				ReleaseMutex(data->hMutex);
				return 0;
			}
		}
		SetEvent(data->hBoardEvent);
		ReleaseMutex(data->hMutex);
		if (data->fc == NULL) break;	//something went wrong

		//Update next water position
		switch (v_water_dir)
		{
		case N:
			v_current_water_posy = (v_current_water_posy - 1) < 0 ? 0 : (v_current_water_posy - 1);
			break;
		case S:
			v_current_water_posy = (v_current_water_posy + 1) >= data->fc->gameboard.y ? data->fc->gameboard.y : (v_current_water_posy + 1);
			break;
		case R:
			v_current_water_posx = (v_current_water_posx + 1) >= data->fc->gameboard.x ? data->fc->gameboard.x : (v_current_water_posx + 1);
			break;
		case L:
			v_current_water_posx = (v_current_water_posx - 1) < 0 ? 0 : (v_current_water_posx - 1);
			break;
		default:
			break;
		}

		Sleep(1000);
	}

	return 0;
}

PieceType getPieceTypeByString(TCHAR* str)
{
	if (str == NULL) return E;

	if (_tcscmp(str, L"H") == 0) return H;
	else if (_tcscmp(str, L"V") == 0) return V;
	else if (_tcscmp(str, L"UL") == 0) return UL;
	else if (_tcscmp(str, L"UR") == 0) return UR;
	else if (_tcscmp(str, L"DL") == 0) return DL;
	else if (_tcscmp(str, L"DR") == 0) return DR;
	else return E;
}

void cmdProcessing(Data* data, TCHAR* cmd)
{
	TCHAR** parsedCmd = stringParser(cmd, ' ');

	if (_tcscmp(parsedCmd[0], L"stopwater") == 0)
	{
		WaterStop* ws = malloc(sizeof(WaterStop));
		if (ws == NULL) return;
		ws->data = data;
		ws->stopWaterMili = _ttoi(parsedCmd[1]) * 1000;	//it's in miliseconds

		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)stopWaterThread, (LPVOID)ws, 0, NULL);
	}
	else if (_tcscmp(parsedCmd[0], L"exit") == 0)
	{
		if (WaitForSingleObject(data->hMutex, INFINITE) == WAIT_OBJECT_0)
		{
			UnmapViewOfFile(data->fc);
			data->fc = (FlowControl*) MapViewOfFile(data->hGameMemory, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(FlowControl));
			if (data->fc == NULL)
			{
				ReleaseMutex(data->hMutex);
				return;
			}
			data->fc->gameboard.isGameRunning = FALSE;
		}
		ReleaseMutex(data->hMutex);
	}
	else if (_tcscmp(parsedCmd[0], L"wall") == 0)
	{
		WallInfo* wi = malloc(sizeof(WallInfo));
		if (wi == NULL) return;
		wi->data = data;
		wi->y = _ttoi(parsedCmd[1]) - 1;
		wi->x = _ttoi(parsedCmd[2]) - 1;

		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)placeWallThread, (LPVOID)wi, 0, NULL);
	}
	else if (_tcscmp(parsedCmd[0], L"startwater") == 0)
	{
		if (WaitForSingleObject(data->hMutex, INFINITE) == WAIT_OBJECT_0)
		{
			UnmapViewOfFile(data->fc);
			data->fc = (FlowControl*)MapViewOfFile(data->hGameMemory, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(FlowControl));
			if (data->fc == NULL)
			{
				ReleaseMutex(data->hMutex);
				return;
			}
			data->fc->gameboard.isWaterRunning = TRUE;
		}
		ReleaseMutex(data->hMutex);
	}
	else if (_tcscmp(parsedCmd[0], L"pipe") == 0)
	{
		PieceType pipe_type = getPieceTypeByString(parsedCmd[1]);
		int pipe_y = _ttoi(parsedCmd[2]) - 1;
		int pipe_x = _ttoi(parsedCmd[3]) - 1;

		PipeInfo* pi = malloc(sizeof(PipeInfo));
		if (pi == NULL) return;

		pi->data = data;
		pi->pipe_type = pipe_type;
		pi->pipe_y = pipe_y;
		pi->pipe_x = pipe_x;

		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)addPipeThread, (LPVOID)pi, 0, NULL);
	}
}

DWORD WINAPI cmdControlThread(LPVOID param)
{
	Data* data = (Data*) param;
	if (data == NULL)
	{
		_tprintf(L"cmdControlThread: where's my parameter?\n");
		return -1;
	}

	if (WaitForSingleObject(data->hMutex, INFINITE) == WAIT_OBJECT_0)
	{
		UnmapViewOfFile(data->fc);
		data->fc = (FlowControl*)MapViewOfFile(data->hGameMemory, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(FlowControl));
		if (data->fc == NULL)
		{
			_tprintf(L"Where's my flow control?\n");
			ReleaseMutex(data->hMutex);
			return -1;
		}
	}

	while (TRUE)
	{
		if (WaitForSingleObject(data->hCommandEvent, INFINITE) == WAIT_OBJECT_0)	//New event OwO
		{
			if (WaitForSingleObject(data->hMutex, INFINITE) == WAIT_OBJECT_0)	//Take control of the mutex
			{
				if(data->fc != NULL) UnmapViewOfFile(data->fc);
				data->fc = (FlowControl*) MapViewOfFile(data->hGameMemory, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(FlowControl));	//Update shared memory mapping
				if (data->fc == NULL)
				{
					ReleaseMutex(data->hMutex);
					return -1;
				}

				if (!data->fc->gameboard.isGameRunning)
				{
					ReleaseMutex(data->hMutex);
					return 0;
				}

				int out = data->fc->buffer.out;
				_tprintf(L"Received from monitor: %s\n", data->fc->buffer.cmdBuffer[out]);
				TCHAR cmd[CMD_MAX_LENGTH] = L"";
				//_tcscpy_s(cmd, sizeof(TCHAR) * CMD_MAX_LENGTH, data->fc->buffer.cmdBuffer[out]);
				memcpy(cmd, data->fc->buffer.cmdBuffer[out], sizeof(TCHAR) * CMD_MAX_LENGTH);

				cmdProcessing(data, cmd);

				//update buffer
				out = (out + 1) % DIM;
				data->fc->buffer.out = out;

				ResetEvent(data->hCommandEvent);
			}
			ReleaseMutex(data->hMutex);
		}
	}

	return 0;
}

GameBoard* initGameboard()
{
	///////////////////////////
	///Fecth Registry Values///
	///////////////////////////



	////////////////////////
	///Allocate the board///
	////////////////////////

	GameBoard* gb = (GameBoard*)malloc(sizeof(GameBoard));

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
			gb->board[y][x].isWall = FALSE;
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
	gb->board[0][0].side = L;			//Water needs to leave from this side.

	return gb;
}
