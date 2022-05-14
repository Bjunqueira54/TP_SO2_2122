#pragma once

#include "..\Global Data Structures\GameBoard.h"

GameBoard* initGameboard();
void drawBoardToConsole(GameBoard*);
DWORD WINAPI waterControlThread(LPVOID);