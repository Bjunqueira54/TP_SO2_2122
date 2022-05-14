#pragma once

#include "SharedMemoryFunctions.h"
#include "Functions.h"

TCHAR memoryName[] = TEXT("GameMemory");
TCHAR mutexName[] = TEXT("MutexMemory");
TCHAR mainEventName[] = TEXT("EventMemory");

BOOL running = TRUE;

void initSharedMemory(FlowControl* fc, HANDLE* gameMemoryHandle)
{
	unsigned int boardSize = sizeof(GameCell) * (fc->gb.y) * (fc->gb.x);
	*gameMemoryHandle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(FlowControl), memoryName);

	if (gameMemoryHandle == NULL)
	{
		_tprintf(TEXT("Could not create the game shared memory blocks: (%d).\n"), GetLastError());
		return;
	}

	fc->hMutex = CreateMutex(NULL, FALSE, mutexName);

	if (fc->hMutex == NULL)
		_tprintf(TEXT("Could not create the shared memory blocks: (%d).\n"), GetLastError());
	else if (GetLastError() == ERROR_ALREADY_EXISTS)
		_tprintf(TEXT("Mutex already exists\n"));

	fc->hEvent = CreateEvent(NULL, TRUE, FALSE, mainEventName);
	if (fc->hEvent == NULL)
	{
		_tprintf(L"Error creating event handle: (%d)!\n", GetLastError());
		return;
	}
}

void UnmapSharedMemory(HANDLE gameMemoryHandle)
{
	CloseHandle(gameMemoryHandle);
}

void copyFlowControltoMemory(FlowControl* fc, HANDLE gameMemoryHandle)
{
	FlowControl* mem_fc = (FlowControl*) MapViewOfFile(gameMemoryHandle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(FlowControl));

	if (mem_fc == NULL)
	{
		_tprintf(L"Error mapping memory: (%d)!\n", GetLastError());
		return -1;
	}

	memcpy((void*) mem_fc, (void*) fc, sizeof(FlowControl));
}

//Code from last year's project. Adapt as necessary
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