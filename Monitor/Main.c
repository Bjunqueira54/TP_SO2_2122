#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>

#include "..\Global Data Structures\GameBoard.h"
#include "..\Global Data Structures\DrawingFunctions.h"

TCHAR memoryName[] = TEXT("GameMemory");
TCHAR mutexName[] = TEXT("MutexMemory");
TCHAR mainEventName[] = TEXT("EventMemory");

HANDLE gameMemoryHandle;

FlowControl* fc;

void initHandles()
{
	gameMemoryHandle = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, memoryName);

	if (gameMemoryHandle == NULL)
	{
		_tprintf(L"Could not open the game shared memory block: (%d).\n", GetLastError());
		return;
	}
}

FlowControl* readFlowControlFromMemory()
{
	FlowControl* mem_fc = (FlowControl*)MapViewOfFile(gameMemoryHandle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(FlowControl));
	if (mem_fc == NULL)
	{
		_tprintf(L"Error mapping game memory: (%d)!\n", GetLastError());
		return -1;
	}

	FlowControl* fc = (FlowControl*) malloc(sizeof(FlowControl));

	if (fc == NULL)
	{
		_tprintf(L"Error allocating new memory!\n");
		return -1;
	}

	CopyMemory(fc, mem_fc, sizeof(GameBoard));

	return fc;
}

//MONITOR
int _tmain(int argc, TCHAR** argv)
{
#ifdef UNICODE

	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);

#endif

	_tprintf(L"Teste unicode no monitor: á à\n");

	initHandles();
	fc = readFlowControlFromMemory();

	if (fc == NULL)
	{
		_tprintf(L"Error on readFlowControlFromMemory(): (%d)!\n", GetLastError());
		return -1;
	}

	HANDLE hStdout;
	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	

	while (fc->gb.isGameRunning)
	{
		WaitForSingleObject(fc->hMutex, INFINITE);

		cls(hStdout);
		drawBoardToConsole(&fc->gb);

		ReleaseMutex(fc->hMutex);

		Sleep(1000);
	}

	return 0;
}