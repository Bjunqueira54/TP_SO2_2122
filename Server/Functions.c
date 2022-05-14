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

//Got this from:
//https://docs.microsoft.com/en-us/windows/console/clearing-the-screen
void cls(HANDLE hConsole)
{
	COORD coordScreen = { 0, 0 };    // home for the cursor
	DWORD cCharsWritten;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	DWORD dwConSize;

	// Get the number of character cells in the current buffer.
	if (!GetConsoleScreenBufferInfo(hConsole, &csbi))
	{
		return;
	}

	dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

	// Fill the entire screen with blanks.
	if (!FillConsoleOutputCharacter(hConsole,        // Handle to console screen buffer
		(TCHAR)' ',      // Character to write to the buffer
		dwConSize,       // Number of cells to write
		coordScreen,     // Coordinates of first cell
		&cCharsWritten)) // Receive number of characters written
	{
		return;
	}

	// Get the current text attribute.
	if (!GetConsoleScreenBufferInfo(hConsole, &csbi))
	{
		return;
	}

	// Set the buffer's attributes accordingly.
	if (!FillConsoleOutputAttribute(hConsole,         // Handle to console screen buffer
		csbi.wAttributes, // Character attributes to use
		dwConSize,        // Number of cells to set attribute
		coordScreen,      // Coordinates of first cell
		&cCharsWritten))  // Receive number of characters written
	{
		return;
	}

	// Put the cursor at its home coordinates.
	SetConsoleCursorPosition(hConsole, coordScreen);
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

DWORD WINAPI waterControlThread(LPVOID param)
{
	GameBoard* gb = (GameBoard*) param;

	int v_current_water_posx = -1;
	int v_current_water_posy = -1;
	Side v_water_dir;

	_tprintf(L"10 Seconds until water flows...\n");
	drawBoardToConsole(gb);
	Sleep(10000);

	//Get the console handle and clear the screen
	HANDLE hStdout;
	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	cls(hStdout);

	//Find water start location
	for (int y = 0; y < gb->y; y++)
	{
		for (int x = 0; x < gb->x; x++)
		{
			if (gb->board[y][x].isStart)
			{
				v_current_water_posx = x;
				v_current_water_posy = y;
				v_water_dir = gb->board[y][x].side;
			}
		}
	}

	if (v_current_water_posx == -1 || v_current_water_posy == -1)
	{
		_tprintf(L"Uninitialized water start position!\n");
		return -1;
	}

	while (gb->isGameRunning)
	{
		gb->board[v_current_water_posx][v_current_water_posy].isFlooded = TRUE;

		PieceType v_current_cell_piece = gb->board[v_current_water_posx][v_current_water_posy].piece;

		if(v_current_cell_piece == E)
		{
			gb->isGameRunning = FALSE;
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
			gb->isGameRunning = FALSE;
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
		drawBoardToConsole(gb);

		Sleep(1000);
	}

	return 0;
}