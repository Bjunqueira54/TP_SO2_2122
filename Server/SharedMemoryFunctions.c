#pragma once

#include "SharedMemoryFunctions.h"
#include "Functions.h"

TCHAR mapSharedMemoryName[] = TEXT("GameMemory");
TCHAR boardSharedMemoryName[] = TEXT("BoardMemory");
TCHAR mutexName[] = TEXT("MutexMemory");
TCHAR mainEventName[] = TEXT("EventMemory");

BOOL running = TRUE;

void initSharedMemory(GameBoard* gb, HANDLE* gameMemoryHandle, HANDLE* boardMemoryHandle, HANDLE* hMutex, HANDLE* hEvent)
{
	unsigned int boardSize = sizeof(GameCell) * (gb->y) * (gb->x);
	*gameMemoryHandle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(GameBoard), mapSharedMemoryName);
	*boardMemoryHandle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, boardSize, boardSharedMemoryName);

	if (gameMemoryHandle == NULL)
	{
		_tprintf(TEXT("Could not create the game shared memory blocks: (%d).\n"), GetLastError());
		return;
	}
	if (boardMemoryHandle == NULL)
	{
		_tprintf(L"Could not create the board shared memory block: (%d).\n", GetLastError());
		return;
	}

	*hMutex = CreateMutex(NULL, FALSE, mutexName);

	if (hMutex == NULL)
		_tprintf(TEXT("Could not create the shared memory blocks: (%d).\n"), GetLastError());
	else if (GetLastError() == ERROR_ALREADY_EXISTS)
		_tprintf(TEXT("Mutex already exists\n"));

	*hEvent = CreateEvent(NULL, TRUE, FALSE, mainEventName);
	if (hEvent == NULL)
	{
		_tprintf(L"Error creating event handle: (%d)!\n", GetLastError());
		return;
	}
}

void UnmapSharedMemory(HANDLE gameMemoryHandle, HANDLE boardMemoryHandle)
{
	CloseHandle(gameMemoryHandle);
	CloseHandle(boardMemoryHandle);
}

//Code from last year's project. Adapt as necessary

void copyBoardtoMemory(GameBoard* gb, HANDLE gameMemoryHandle, HANDLE boardMemoryHandle)
{
	unsigned int boardSize = sizeof(GameCell) * gb->y * gb->x;
	GameBoard* mem_gb = (GameBoard*) MapViewOfFile(gameMemoryHandle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(GameBoard));
	//GameCell** mem_board = (GameCell**) MapViewOfFile(boardMemoryHandle, FILE_MAP_ALL_ACCESS, 0, 0, boardSize);

	if (mem_gb == NULL) //|| mem_board == NULL)
	{
		_tprintf(L"Error mapping memory: (%d)!\n", GetLastError());
		return -1;
	}

	memcpy((void*)mem_gb, (void*)gb, sizeof(GameBoard));
	//memcpy((void*)mem_board, (void*) gb->board, boardSize);
}

/*
DWORD WINAPI readPagedMemory(GameBoard* gb, HANDLE hEvent)
{
	while (running)
	{
		DWORD eventResult = WaitForSingleObject(hEvent, INFINITE);

		if (running == FALSE)
			break;

		if (eventResult == WAIT_OBJECT_0)
		{
			_tprintf(TEXT("Found new plane!"));
			data->Accepted = FALSE;

			ResetEvent(hEvent);

			TCHAR eventName[64];
			_stprintf_s(eventName, 64, TEXT("Airplane_%i\0"), data->planeID);

			HANDLE planeEventHandle = OpenEvent(EVENT_ALL_ACCESS, FALSE, eventName);
			if (planeEventHandle != NULL)
				if (SetEvent(planeEventHandle) == TRUE)
					CloseHandle(planeEventHandle);
		}
	}

	return 0;
}
*/

void triggerEvent(HANDLE hEvent)
{
	SetEvent(hEvent);
}