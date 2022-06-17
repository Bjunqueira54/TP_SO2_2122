#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>

#include "..\Global Data Structures\GameBoard.h"
#include "Functions.h"
#include "SharedMemoryFunctions.h"

TCHAR memoryName[] = TEXT("GameMemory");
TCHAR boardEventName[] = TEXT("BoardEvent");
TCHAR cmdEventName[] = TEXT("CommandEvent");
TCHAR sMutexName[] = L"Semaforo_Mutex";
TCHAR sItemsName[] = L"Semaforo_1";
TCHAR sEmptyName[] = L"Semaforo_2";

//SERVER
int _tmain(int argc, TCHAR** argv)
{
#ifdef UNICODE

	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);

#endif

	//https://en.wikipedia.org/wiki/Box_Drawing
	//https://en.wikipedia.org/wiki/Block_Elements
	//https://en.wikipedia.org/wiki/Arrows_(Unicode_block)
	//_tprintf(L"\u2554 \u2557 \u255a \u255d \u2550 \u2551 \n");
	//_tprintf(L"\u259b \u259c \u2599 \u259f \u2583 \u2590 \n");
	//_tprintf(L"\u2190 \u2191 \u2192 \u2193 \n");
	//_tprintf(L"\u2593");

	//start game
	Data* data = malloc(sizeof(Data));
	if (data == NULL) return -1;

	data->hGameMemory = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, memoryName);
	if (data->hGameMemory != NULL)
	{
		_tprintf(L"One instance of the server is already running!\n");
		return 0;
	}

	initSharedMemory(data);
	if (data->hBoardEvent == NULL || data->sMutex == NULL || data->hCommandEvent == NULL || data->sItems == NULL || data->sEmpty == NULL)
	{
		_tprintf(L"What happened inside initSharedMemory? Handles are not innitialized: (%d)!\n", GetLastError());
		return -1;
	}

	if (WaitForSingleObject(data->sMutex, INFINITE) == WAIT_OBJECT_0)
	{
		data->fc = (FlowControl*)MapViewOfFile(data->hGameMemory, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(FlowControl));
		if (data->fc == NULL)
		{
			_tprintf(L"Error allocating memory for Flow Control structure: (%d)!\n", GetLastError());
			return -1;
		}

		//init all flow control fields
		CopyMemory(&data->fc->gameboard, initGameboard(), sizeof(GameBoard));
		data->fc->buffer.in = 0;
		data->fc->buffer.out = 0;
		for (int i = 0; i < DIM; i++)
			CopyMemory(data->fc->buffer.cmdBuffer[i], L"\0", sizeof(TCHAR));

		data->fc->gameboard.isGameRunning = TRUE;

		copyFlowControltoMemory(data->fc, data->hGameMemory);
	}
	ReleaseSemaphore(data->sMutex, 1, NULL);

	HANDLE waterThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) waterControlThread, (LPVOID) data, 0, NULL);
	if (waterThreadHandle == NULL)
	{
		_tprintf(L"Cannot create waterThreadControl: (%d)!\n", GetLastError());
		return -1;
	}

	HANDLE cmdThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)cmdControlThread, (LPVOID) data, 0, NULL);
	if (cmdThreadHandle == NULL)
	{
		_tprintf(L"Cannot create cmdThreadControl: (%d)!\n", GetLastError());
		return -1;
	}

	while (TRUE)
	{
		if (WaitForSingleObject(data->sMutex, INFINITE) == WAIT_OBJECT_0)
		{
			UnmapViewOfFile(data->fc);
			data->fc = (FlowControl*)MapViewOfFile(data->hGameMemory, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(FlowControl));
			if (data->fc == NULL)
				return -1;

			if (!data->fc->gameboard.isGameRunning)
			{
				ReleaseSemaphore(data->sMutex, 1, NULL);
				break;
			}
		}
		ReleaseSemaphore(data->sMutex, 1, NULL);
	}

	SetEvent(data->hCommandEvent);	//Trigger this event to pop cmdControlThread
	WaitForSingleObject(waterThreadHandle, INFINITE);
	WaitForSingleObject(cmdThreadHandle, INFINITE);

	_tprintf(L"Game ended.\n");

	UnmapSharedMemory(data->hGameMemory);
	free(data->fc);
	CloseHandle(data->hBoardEvent);
	CloseHandle(data->hCommandEvent);
	CloseHandle(data->sMutex);
	CloseHandle(data->sItems);
	CloseHandle(data->sEmpty);
	free(data);

	return 0;
}