#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>

#include "..\Global Data Structures\GameBoard.h"
#include "Functions.h"
#include "SharedMemoryFunctions.h"

TCHAR memoryName[] = TEXT("GameMemory");
TCHAR mutexName[] = TEXT("MutexMemory");
TCHAR boardEventName[] = TEXT("BoardEvent");
TCHAR cmdEventName[] = TEXT("CommandEvent");

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

	//start game

	FlowControl* fc = malloc(sizeof(FlowControl));
	if (fc == NULL)
	{
		_tprintf(L"Error allocating memory for Flow Control structure: (%d)!\n", GetLastError());
		return -1;
	}

	//init all flow control fields
	CopyMemory(&fc->gameboard, initGameboard(), sizeof(GameBoard));
	fc->buffer.in = 0;
	fc->buffer.out = 0;
	for (int i = 0; i < DIM; i++)
	{
		CopyMemory(fc->buffer.cmdBuffer[i], L"\0", sizeof(TCHAR));
	}

	HANDLE gameMemoryHandle = NULL;
	Data* data = malloc(sizeof(Data));
	if (data == NULL) return -1;

	initSharedMemory(data, &gameMemoryHandle);
	data->fc = fc;

	//isto é só para fazer com que o InteliSense pare de
	//moer o meu juizo á custa dos handles talvez serem 0.
	if (data->hBoardEvent == NULL || data->hMutex == NULL || data->hCommandEvent == NULL)
	{
		_tprintf(L"What happened inside initSharedMemory? Handles are not innitialized: (%d)!\n", GetLastError());
		return -1;
	}

	_tprintf(L"MainThread: I'm waiting for the mutex\n");
	WaitForSingleObject(data->hMutex, INFINITE);
	_tprintf(L"MainThread: I'm controlling the mutex\n");
	fc->gameboard.isGameRunning = TRUE;

	copyFlowControltoMemory(fc, gameMemoryHandle);

	ReleaseMutex(data->hMutex);
	_tprintf(L"MainThread: I've released the mutex\n");

	HANDLE waterThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) waterControlThread, (LPVOID) data, NULL, NULL);
	if (waterThreadHandle == NULL)
	{
		_tprintf(L"Cannot create waterThreadControl: (%d)!\n", GetLastError());
		return -1;
	}

	HANDLE cmdThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)cmdControlThread, (LPVOID) data, NULL, NULL);
	if (cmdThreadHandle == NULL)
	{
		_tprintf(L"Cannot create cmdThreadControl: (%d)!\n", GetLastError());
		return -1;
	}

	while (fc->gameboard.isGameRunning)
	{
		//we take commands here?
	}

	WaitForSingleObject(waterThreadHandle, INFINITE);
	WaitForSingleObject(cmdThreadHandle, INFINITE);

	_tprintf(L"Game ended.\n");

	UnmapSharedMemory(gameMemoryHandle);
	free(fc);

	return 0;
}