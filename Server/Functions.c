#pragma once

#include <Windows.h>
#include <tchar.h>

#include "Functions.h"

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

	gb->board = (GameCell**) malloc(gb->x * sizeof(GameCell*));

	if (gb->board == NULL) return NULL;

	for (int i = 0; i < gb->x; i++)
	{
		gb->board[i] = (GameCell*) malloc(gb->y * sizeof(GameCell));

		if (gb->board[i] == NULL) return NULL;
	}

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
		}
	}

	///////////////////////
	///Choose start cell///
	///////////////////////

	gb->board[gb->y - 1][gb->x - 1].isStart = TRUE;
	gb->board[gb->y - 1][gb->x - 1].isEnd = FALSE;	//Redundant, but better safe than sorry.
	gb->board[gb->y - 1][gb->x - 1].side = S;

	//////////////////////////////////
	///Choose end cell and the side///
	//////////////////////////////////

	gb->board[0][0].isEnd = TRUE;
	gb->board[0][0].isStart = FALSE;	//Redundant, but better safe than sorry.
	gb->board[0][0].side = L;

	return gb;
}

void drawBoardToConsole(GameBoard* gb)
{
	_tprintf(L" ");
	_tprintf(L" ");
	//Draw water start if it's on the top border
	for (int i = 0; i < gb->x; i++)
	{
		if (gb->board[0][i].isStart == TRUE)
		{
			if (gb->board[0][i].side == N)
				_tprintf(L"\u2193");
		}

		else if (gb->board[0][i].isEnd == TRUE)
		{
			if (gb->board[0][i].side == N)
				_tprintf(L"\u2191");
		}

		else
			_tprintf(L" ");

		_tprintf(L" ");
	}
	_tprintf(L"\n");

	_tprintf(L" ");
	_tprintf(L" ");
	//Draw top border
	for (int i = 0; i < gb->x; i++)
		_tprintf(L"_ ");
	_tprintf(L"\n");

	//Draw board
	for (int y = 0; y < gb->y; y++)
	{
		if (gb->board[y][0].isStart == TRUE)
		{
			if (gb->board[y][0].side == R)
				_tprintf(L"\u2192");
		}
		else if (gb->board[y][0].isEnd == TRUE)
		{
			if (gb->board[y][0].side == L)
				_tprintf(L"\u2190");
		}
		else
			_tprintf(L" ");

		_tprintf(L"|");

		for (int x = 0; x < gb->x; x++)
		{
			if (gb->board[y][x].isFlooded == FALSE)
			{
				switch (gb->board[y][x].piece)
				{
				case E:
					_tprintf(L"_");
					break;
				case V:
					_tprintf(L"\u2551");
					break;
				case H:
					_tprintf(L"\u2550");
					break;
				case UL:
					_tprintf(L"\u255d");
					break;
				case UR:
					_tprintf(L"\u255a");
					break;
				case DL:
					_tprintf(L"\u2557");
					break;
				case DR:
					_tprintf(L"\u2554");
					break;
				default: break;
				}
			}
			if (gb->board[y][x].isFlooded == TRUE)
			{
				switch (gb->board[y][x].piece)
				{
				case E:
					_tprintf(L"x");
					break;
				case V:
					_tprintf(L"\u2590");
					break;
				case H:
					_tprintf(L"\u2583");
					break;
				case UL:
					_tprintf(L"\u259f");
					break;
				case UR:
					_tprintf(L"\u2599");
					break;
				case DL:
					_tprintf(L"\u259c");
					break;
				case DR:
					_tprintf(L"\u259b");
					break;
				default: break;
				}
			}

			_tprintf(L"|");
		}

		if (gb->board[y][gb->x - 1].isStart == TRUE)
		{
			if (gb->board[y][gb->x - 1].side == R)
				_tprintf(L"\u2190");
		}
		else if (gb->board[y][gb->x - 1].isEnd == TRUE)
		{
			if (gb->board[y][gb->x - 1].side == R)
				_tprintf(L"\u2192");
		}
		else
			_tprintf(L" ");
		_tprintf(L"\n");
	}

	_tprintf(L" ");
	_tprintf(L" ");
	for (int i = 0; i < gb->x; i++)
	{
		if (gb->board[gb->y - 1][i].isStart == TRUE)
		{
			if (gb->board[gb->y - 1][i].side == S)
				_tprintf(L"\u2191");
		}
		else if (gb->board[gb->y - 1][i].isEnd == TRUE)
		{
			if (gb->board[gb->y - 1][i].side == S)
				_tprintf(L"\u2193");
		}
		else
			_tprintf(L" ");
		_tprintf(L" ");
	}
	_tprintf(L"\n");
}