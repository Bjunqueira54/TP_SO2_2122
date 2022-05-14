#pragma once

#include <Windows.h>
#include <tchar.h>

#include "..\Global Data Structures\GameBoard.h"

HANDLE initSharedMemory(HANDLE, HANDLE);
void copyBoardtoMemory(GameBoard*, HANDLE);
void UnmapSharedMemory(HANDLE);
//DWORD WINAPI readPagedMemory();
void triggerEvent(HANDLE);