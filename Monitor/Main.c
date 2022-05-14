#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>

#include "..\Global Data Structures\GameBoard.h"

TCHAR mapSharedMemoryName[] = TEXT("MapMemory");
TCHAR mutexName[] = TEXT("MutexMemory");
TCHAR mainEventName[] = TEXT("EventMemory");

HANDLE mapMemoryHandle;
HANDLE hMutex;

void initSharedMemory()
{
	//mapMemoryHandle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(GameBoard), mapSharedMemoryName);
	mapMemoryHandle = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, mapSharedMemoryName);

	if (mapMemoryHandle == NULL)
	{
		_tprintf(TEXT("Could not create the shared memory blocks: (%d).\n"), GetLastError());
		return;
	}

	hMutex = OpenMutex(NULL, FALSE, mutexName);
}

GameBoard* readGameBoardFromMemory()
{
	GameBoard* mem_gb = (GameBoard*)MapViewOfFile(mapMemoryHandle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(GameBoard));

	if (mem_gb == NULL)
	{
		_tprintf(L"Error mapping memory!\n");
		return -1;
	}

	GameBoard* gb = (GameBoard*) malloc(sizeof(GameBoard));

	if (gb == NULL)
	{
		_tprintf(L"Error allocating new memory!\n");
		return -1;
	}

	gb->board = (GameCell**) malloc(sizeof(GameCell*) * mem_gb->x);

	if (gb->board == NULL)
	{
		_tprintf(L"Error allocation GameCell Array!\n");
		return NULL;
	}

	for (int i = 0; i < mem_gb->x; i++)
	{
		gb->board[i] = (GameCell*) malloc(sizeof(GameCell) * mem_gb->y);

		if (gb->board[i] == NULL) return NULL;
	}

	CopyMemory(gb, mem_gb, sizeof(GameBoard));
	CopyMemory(gb->board, mem_gb->board, sizeof(GameBoard) * mem_gb->y * mem_gb->x);

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

//MONITOR
int _tmain(int argc, TCHAR** argv)
{
#ifdef UNICODE

	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);

#endif

	_tprintf(L"Teste unicode no monitor: á à\n");

	initSharedMemory();

	WaitForSingleObject(hMutex, INFINITE);

	GameBoard* gb = readGameBoardFromMemory();

	drawBoardToConsole(gb);

	ReleaseMutex(hMutex);

	return 0;
}