#pragma once

#include <Windows.h>
#include <tchar.h>

void initSharedMemory();
void UnmapSharedMemory();
//DWORD WINAPI readPagedMemory();
void triggerEvent();