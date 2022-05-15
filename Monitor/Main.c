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

FlowControl* getFlowControlFromMemory(HANDLE gameMemoryHandle)
{
	FlowControl* mem_fc = (FlowControl*) MapViewOfFile(gameMemoryHandle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(FlowControl));
	if (mem_fc == NULL)
	{
		_tprintf(L"Error mapping game memory: (%d)!\n", GetLastError());
		return -1;
	}

	return mem_fc;
}

DWORD WINAPI drawingThread(LPVOID param)
{
	_tprintf(L"drawingThread: I'm starting\n");

	Data* data = (Data*)param;
	FlowControl* fc = data->fc;

	HANDLE hBoardEvent = data->hBoardEvent;
	HANDLE hMutex = data->hMutex;

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	while (fc->gameboard.isGameRunning)
	{
		_tprintf(L"drawingThread: I'm waiting for BoardEvent\n");
		DWORD boardEventResult = WaitForSingleObject(hBoardEvent, INFINITE);
		if (boardEventResult == WAIT_OBJECT_0)	//we got confirmation that the board was modified
		{
			_tprintf(L"drawingThread: I've found BoardEvent\n");
			if (!fc->gameboard.isGameRunning) break;

			CONSOLE_SCREEN_BUFFER_INFO csbi;
			if (!GetConsoleScreenBufferInfo(hConsole, &csbi))
			{
				ResetEvent(hBoardEvent);
				continue;
			}

			COORD currentScreenCoord = csbi.dwCursorPosition;
			OverwriteConsoleScreen(hConsole);

			_tprintf(L"drawingThread: I'm waiting to control the mutex\n");
			if (WaitForSingleObject(hMutex, INFINITE) == WAIT_OBJECT_0)
			{
				_tprintf(L"drawingThread: I'm controlling the mutex\n");
				drawBoardToConsole(&fc->gameboard.board);
				ResetEvent(hBoardEvent);
				_tprintf(L"drawingThread: I've RESET the BoardEvent\n");
				
			}
			ReleaseMutex(hMutex);
			_tprintf(L"drawingThread: I've released the mutex\n");
			SetConsoleCursorPosition(hConsole, currentScreenCoord);
		}
	}
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

	data->fc = getFlowControlFromMemory(data->hGameMemory);
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
	
	HANDLE drawingThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) drawingThread, (LPVOID) data, NULL, NULL);
	if (drawingThreadHandle == NULL)
	{
		_tprintf(L"Error creating drawingThread: (%d)!\n", GetLastError());
		return -1;
	}

	while (data->fc->gameboard.isGameRunning)
	{
		_tprintf(L"Command to send to the server: ");
		TCHAR cmd[CMD_MAX_LENGTH];
		_fgetts(cmd, CMD_MAX_LENGTH, stdin);

		_tprintf(L"MainThread: I'm waiting to control the mutex\n");
		if (WaitForSingleObject(data->hMutex, INFINITE) == WAIT_OBJECT_0)
		{
			_tprintf(L"MainThread: I'm controlling the mutex\n");
			int in = data->fc->buffer.in;
			memcpy(data->fc->buffer.cmdBuffer[in], cmd, CMD_MAX_LENGTH);
			in = (in + 1) % DIM;
			data->fc->buffer.in = in;

			SetEvent(data->hCommandEvent);
			_tprintf(L"MainThread: I've SET the CommandEvent\n");
		}
		ReleaseMutex(data->hMutex);
		_tprintf(L"MainThread: I've released the mutex\n");
	}

	WaitForSingleObject(drawingThreadHandle, INFINITE);

	return 0;
}