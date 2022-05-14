#pragma once

#include <Windows.h>
#include <tchar.h>

#include "..\Global Data Structures\GameBoard.h"

void initSharedMemory(GameBoard*, HANDLE*, HANDLE*, HANDLE*, HANDLE*);
void copyBoardtoMemory(GameBoard*, HANDLE, HANDLE);
void UnmapSharedMemory(HANDLE);
//DWORD WINAPI readPagedMemory();
void triggerEvent(HANDLE);