#pragma once

#include "SharedMemoryFunctions.h"
#include "Functions.h"

void initSharedMemory(Data* data, HANDLE* gameMemoryHandle)
{
	*gameMemoryHandle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(FlowControl), memoryName);

	if (gameMemoryHandle == NULL)
	{
		_tprintf(TEXT("Could not create the game shared memory blocks: (%d).\n"), GetLastError());
		return;
	}

	data->hMutex = CreateMutex(NULL, FALSE, mutexName);

	if (data->hMutex == NULL)
		_tprintf(TEXT("Could not create the mutex: (%d).\n"), GetLastError());
	else if (GetLastError() == ERROR_ALREADY_EXISTS)
		_tprintf(TEXT("Mutex already exists\n"));

	data->hBoardEvent = CreateEvent(NULL, TRUE, FALSE, boardEventName);
	if (data->hBoardEvent == NULL)
	{
		_tprintf(L"Error creating Board event handle: (%d)!\n", GetLastError());
		return;
	}

	data->hCommandEvent = CreateEvent(NULL, TRUE, FALSE, cmdEventName);
	if (data->hCommandEvent == NULL)
	{
		_tprintf(L"Error creating Command event handle: (%d)!\n", GetLastError());
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

	UnmapViewOfFile(mem_fc);
}