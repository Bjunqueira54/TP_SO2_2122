#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>

#include "..\Global Data Structures\GameBoard.h"
#include "Functions.h"
#include "SharedMemoryFunctions.h"

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
	CopyMemory(&fc->gb, initGameboard(), sizeof(GameBoard));
	fc->hEvent = NULL;
	fc->hMutex = NULL;
	fc->buffer.in = 0;
	fc->buffer.out = 0;
	for (int i = 0; i < DIM; i++)
		*fc->buffer.cmdBuffer[i] = NULL;

	HANDLE gameMemoryHandle = NULL;

	initSharedMemory(fc, &gameMemoryHandle);

	//isto é só para fazer com que o InteliSense pare de
	//moer o meu juizo á custa dos handles talvez serem 0.
	if (fc->hEvent == NULL || fc->hMutex == NULL)
	{
		_tprintf(L"What happened inside initSharedMemory? Handles are not innitialized: (%d)!\n", GetLastError());
		return -1;
	}

	WaitForSingleObject(fc->hMutex, INFINITE);

	fc->gb.isGameRunning = TRUE;

	copyFlowControltoMemory(fc, gameMemoryHandle);

	ReleaseMutex(fc->hMutex);

	HANDLE waterThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) waterControlThread, (LPVOID) fc, NULL, NULL);
	if (waterThreadHandle == NULL)
	{
		_tprintf(L"Cannot create waterThreadControl: (%d)!\n", GetLastError());
		return -1;
	}

	HANDLE cmdThreadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)cmdControlThread, (LPVOID) fc, NULL, NULL);
	if (cmdThreadHandle == NULL)
	{
		_tprintf(L"Cannot create cmdThreadControl: (%d)!\n", GetLastError());
		return -1;
	}

	while (fc->gb.isGameRunning)
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