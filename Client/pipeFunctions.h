#pragma once

#include <Windows.h>
#include "..\Global Data Structures\GameBoard.h"

extern TCHAR internalClientEvent[];

typedef struct
{
	HANDLE hWnd;
	Data* data;
} PipeThreadInfo;

BoardCoord bc;

DWORD WINAPI pipeThread(LPVOID);