#pragma once

#include "..\Global Data Structures\GameBoard.h"

GameBoard* initGameboard();
DWORD WINAPI waterControlThread(LPVOID);
DWORD WINAPI cmdControlThread(LPVOID);
DWORD WINAPI addPipeThread(LPVOID);