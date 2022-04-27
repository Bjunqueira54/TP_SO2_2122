#pragma once

#include <Windows.h>

void initSharedMemory();
void UnmapSharedMemory();
//DWORD WINAPI readPagedMemory();
void triggerEvent();