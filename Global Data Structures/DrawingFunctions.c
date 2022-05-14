#pragma once

#include "DrawingFunctions.h"

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