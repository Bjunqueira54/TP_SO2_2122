#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>

#include "..\Global Data Structures\GameBoard.h"
#include "..\Global Data Structures\DrawingFunctions.h"

TCHAR memoryName[] = TEXT("GameMemory");
TCHAR mutexName[] = TEXT("MutexMemory");
TCHAR boardEventName[] = TEXT("BoardEvent");
TCHAR cmdEventName[] = TEXT("CommandEvent");

DWORD WINAPI drawingThread(LPVOID param)
{
	GameBoard* gb;
	Data* data = (Data*)param;
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

			if (WaitForSingleObject(data->hMutex, INFINITE) == WAIT_OBJECT_0)
			{
				UnmapViewOfFile(data->fc);
				data->fc = (FlowControl*)MapViewOfFile(data->hGameMemory, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(FlowControl));
				if (data->fc == NULL)
				{
					ReleaseMutex(data->hMutex);
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
			ReleaseMutex(data->hMutex);
			drawBoardToConsole(gb);
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

	data->fc = (FlowControl*)MapViewOfFile(data->hGameMemory, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(FlowControl));
	if (data->fc == NULL)
	{
		_tprintf(L"Error on readFlowControlFromMemory(): (%d)!\n", GetLastError());
		return -1;
	}

	data->hBoardEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, boardEventName);
	data->hCommandEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, cmdEventName);
	data->hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, mutexName);

	if (data->hBoardEvent == NULL || data->hCommandEvent == NULL || data->hMutex == NULL)
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
	drawBoardToConsole(&data->fc->gameboard);

	while (data->fc->gameboard.isGameRunning)
	{
		_tprintf(L"Command to send to the server: ");
		TCHAR cmd[CMD_MAX_LENGTH];
		_fgetts(cmd, CMD_MAX_LENGTH, stdin);

		int str_size = _tcsclen(cmd) - 1;
		if (cmd[str_size] == '\n')
			cmd[str_size] = '\0';

		if (WaitForSingleObject(data->hMutex, INFINITE) == WAIT_OBJECT_0)
		{
			UnmapViewOfFile(data->fc);
			data->fc = (FlowControl*)MapViewOfFile(data->hGameMemory, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(FlowControl));
			if (data->fc == NULL)
			{
				_tprintf(L"Error on readFlowControlFromMemory(): (%d)!\n", GetLastError());
				ReleaseMutex(data->hMutex);
				return -1;
			}

			int in = data->fc->buffer.in;
			memcpy(data->fc->buffer.cmdBuffer[in], cmd, CMD_MAX_LENGTH);
			in = (in + 1) % DIM;
			data->fc->buffer.in = in;

			SetEvent(data->hCommandEvent);
		}
		ReleaseMutex(data->hMutex);

		if (_tcscmp(cmd, L"exit") == 0) break;
	}

	WaitForSingleObject(drawingThreadHandle, INFINITE);

	return 0;
}