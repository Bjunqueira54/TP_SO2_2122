#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>

#include "..\Global Data Structures\GameBoard.h"
#include "..\Global Data Structures\DrawingFunctions.h"

TCHAR memoryName[]		= L"GameMemory";
TCHAR boardEventName[]	= L"BoardEvent";
TCHAR cmdEventName[]	= L"CommandEvent";
TCHAR sMutexName[]		= L"Semaforo_Mutex";
TCHAR sItemsName[]		= L"Semaforo_1";
TCHAR sEmptyName[]		= L"Semaforo_2";

DWORD WINAPI drawingThread(LPVOID param)
{
	GameBoard* gb;
	Data* data = (Data*)param;
	if (WaitForSingleObject(data->sMutex, INFINITE) == WAIT_OBJECT_0)
	{
		UnmapViewOfFile(data->fc);
		data->fc = (FlowControl*)MapViewOfFile(data->hGameMemory, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(FlowControl));
		if (data->fc == NULL)
		{
			ReleaseSemaphore(data->sMutex, 1, NULL);
			return -1;
		}
	}
	ReleaseSemaphore(data->sMutex, 1, NULL);

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	while (data->fc->gameboard.isGameRunning)
	{
		DWORD boardEventResult = WaitForSingleObject(data->hBoardEvent, INFINITE);
		if (boardEventResult == WAIT_OBJECT_0)	//we got confirmation that the board was modified
		{
			if (!data->fc->gameboard.isGameRunning) break;

			CONSOLE_SCREEN_BUFFER_INFO csbi;
			if (!GetConsoleScreenBufferInfo(hConsole, &csbi))
			{
				ResetEvent(data->hBoardEvent);
				continue;
			}

			COORD currentScreenCoord = csbi.dwCursorPosition;
			OverwriteConsoleScreen(hConsole);

			if (WaitForSingleObject(data->sMutex, INFINITE) == WAIT_OBJECT_0)
			{
				UnmapViewOfFile(data->fc);
				data->fc = (FlowControl*)MapViewOfFile(data->hGameMemory, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(FlowControl));
				if (data->fc == NULL)
				{
					ReleaseSemaphore(data->sMutex, 1, NULL);
					return -1;
				}
				gb = malloc(sizeof(GameBoard));
				if (gb == NULL) return -1;

				//I was having problems with memory read exceptions
				//and I figured one of the processes was overwritting the
				//board.
				//Since I don't want to stall everything with
				//a slow drawing operation, I just snapshot the board and draw it.
				memcpy(gb, &data->fc->gameboard, sizeof(GameBoard));
			}
			else continue;
			ResetEvent(data->hBoardEvent);
			ReleaseSemaphore(data->sMutex, 1, NULL);
			drawBoardToConsole(*gb);
			free(gb);
			SetConsoleCursorPosition(hConsole, currentScreenCoord);
		}
	}

	return 0;
}

//MONITOR
int _tmain(int argc, TCHAR** argv)
{
#ifdef UNICODE

	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);

#endif

	Data* data = malloc(sizeof(Data));
	if (data == NULL) return -1;

	data->hGameMemory = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, memoryName);
	if (data->hGameMemory == NULL)
	{
		_tprintf(L"Could not open the game shared memory block: (%d).\n", GetLastError());
		return -1;
	}

	//First read requires no mutex control
	data->fc = (FlowControl*)MapViewOfFile(data->hGameMemory, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(FlowControl));
	if (data->fc == NULL)
	{
		_tprintf(L"Error on readFlowControlFromMemory(): (%d)!\n", GetLastError());
		return -1;
	}

	data->hBoardEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, boardEventName);
	data->hCommandEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, cmdEventName);
	data->sMutex = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, sMutexName);
	data->sItems = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, sItemsName);
	data->sEmpty = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, sEmptyName);

	if (data->hBoardEvent == NULL || data->hCommandEvent == NULL || data->sMutex == NULL || data->sItems == NULL || data->sEmpty == NULL)
	{
		_tprintf(L"Error creating handles: (%d)!\n", GetLastError());
		return -1;
	}

	HANDLE hStdout;
	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	
	HANDLE drawingThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) drawingThread, (LPVOID) data, 0, NULL);
	if (drawingThreadHandle == NULL)
	{
		_tprintf(L"Error creating drawingThread: (%d)!\n", GetLastError());
		return -1;
	}

	//draw the board once before game start
	if (&data->fc->gameboard == NULL)
	{
		_tprintf(L"No gameboard present?\n");
		return -1;
	}
	drawBoardToConsole(data->fc->gameboard);

	while (data->fc->gameboard.isGameRunning)
	{
		_tprintf(L"Command to send to the server: ");
		TCHAR cmd[CMD_MAX_LENGTH];
		_fgetts(cmd, sizeof(TCHAR) * CMD_MAX_LENGTH, stdin);

		int str_size = _tcsclen(cmd) - 1;
		if (cmd[str_size] == '\n')
			cmd[str_size] = '\0';

		if(WaitForSingleObject(data->sEmpty, INFINITE) == WAIT_OBJECT_0)
		{
			if (WaitForSingleObject(data->sMutex, INFINITE) == WAIT_OBJECT_0)
			{
				UnmapViewOfFile(data->fc);
				data->fc = (FlowControl*)MapViewOfFile(data->hGameMemory, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(FlowControl));
				if (data->fc == NULL)
				{
					_tprintf(L"Error on readFlowControlFromMemory(): (%d)!\n", GetLastError());
					ReleaseSemaphore(data->sMutex, 1, NULL);
					return -1;
				}
			}
			ReleaseSemaphore(data->sMutex, 1, NULL);

			int in = data->fc->buffer.in;
			memcpy(data->fc->buffer.cmdBuffer[in], cmd, sizeof(TCHAR) * CMD_MAX_LENGTH);
			in = (in + 1) % DIM;
			data->fc->buffer.in = in;

			SetEvent(data->hCommandEvent);
		}
		ReleaseSemaphore(data->sItems, 1, NULL);
		ClearConsoleScreen(hStdout);

		if (_tcscmp(cmd, L"exit") == 0) break;
	}

	WaitForSingleObject(drawingThreadHandle, INFINITE);

	return 0;
}