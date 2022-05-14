#pragma once

#include <Windows.h>
#include <tchar.h>

#include "..\Global Data Structures\GameBoard.h"

void initSharedMemory(FlowControl*, HANDLE*);
void copyFlowControltoMemory(FlowControl*, HANDLE);
void UnmapSharedMemory(HANDLE);
//DWORD WINAPI readPagedMemory();
void triggerEvent(HANDLE);