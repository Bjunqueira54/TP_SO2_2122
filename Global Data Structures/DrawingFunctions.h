#pragma once
#include <Windows.h>
#include <tchar.h>

#include "GameBoard.h"

void drawBoardToConsole(GameBoard);
void ClearConsoleScreen(HANDLE);
void OverwriteConsoleScreen(HANDLE);