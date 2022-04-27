#pragma once

#include "SharedMemoryFunctions.h"
#include "Functions.h"

TCHAR mapSharedMemoryBlockName[] = TEXT("MapMemory");
TCHAR mutexName[] = TEXT("MutexMemory");
TCHAR mainEventName[] = TEXT("EventMemory");

HANDLE mapMemoryHandle;
HANDLE hMutex;
HANDLE hEvent;

BOOL running = TRUE;

void initSharedMemory()
{
	mapMemoryHandle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(GameBoard), mapSharedMemoryBlockName);

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
}

void UnmapSharedMemory()
{
	CloseHandle(mapMemoryHandle);
}

//Code from last year's project. Adapt as necessary
/*
DWORD WINAPI readPagedMemory()
{
	MyData* data = (MyData*) MapViewOfFile(	mainMemoryHandle,		// handle to map object
											FILE_MAP_ALL_ACCESS,	// read/write permission
											0,
											0,
											sizeof(MyData));

	if (data == NULL)
	{
		_tprintf(TEXT("Error mapping memory"));
		return 0;
	}

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

void triggerEvent()
{
	SetEvent(hEvent);
}