#pragma once

#include "SharedMemoryFunctions.h"
#include "Functions.h"

TCHAR mapSharedMemoryBlockName[] = TEXT("MapMemory");
TCHAR mutexName[] = TEXT("MutexMemory");
TCHAR mainEventName[] = TEXT("EventMemory");

BOOL running = TRUE;

HANDLE initSharedMemory(HANDLE hMutex, HANDLE hEvent)
{
	HANDLE mapMemoryHandle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(GameBoard), mapSharedMemoryBlockName);

	if (mapMemoryHandle == NULL)
	{
		_tprintf(TEXT("Could not create the shared memory blocks: (%d).\n"), GetLastError());
		return;
	}

	hMutex = CreateMutex(NULL, FALSE, mutexName);

	if (hMutex == NULL)
		_tprintf(TEXT("Could not create the shared memory blocks: (%d).\n"), GetLastError());
	else if (GetLastError() == ERROR_ALREADY_EXISTS)
		_tprintf(TEXT("Mutex already exists\n"));

	hEvent = CreateEvent(NULL, TRUE, FALSE, mainEventName);

	return mapMemoryHandle;
}

void UnmapSharedMemory(HANDLE mapMemoryHandle)
{
	CloseHandle(mapMemoryHandle);
}

//Code from last year's project. Adapt as necessary

void copyBoardtoMemory(GameBoard* gb, HANDLE mapMemoryHandle)
{
	GameBoard* mem_gb = (GameBoard*) MapViewOfFile(mapMemoryHandle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(GameBoard));

	if (mem_gb == NULL)
	{
		_tprintf(L"Error mapping memory!\n");
		return -1;
	}

	memcpy((void*)mem_gb, (void*)gb, sizeof(GameBoard));
	memcpy((void*)mem_gb->board, (void*)gb->board, sizeof(GameCell) * gb->y * gb->x);
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