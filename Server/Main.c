#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>

#include "..\Global Data Structures\GameBoard.h"
#include "Functions.h"
#include "SharedMemoryFunctions.h"

//SERVER
int _tmain(int argc, TCHAR** argv)
{
#ifdef UNICODE

	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);

#endif

	//_tprintf(L"Teste unicode no servidor:\n");
	//https://en.wikipedia.org/wiki/Box_Drawing
	//https://en.wikipedia.org/wiki/Block_Elements
	//https://en.wikipedia.org/wiki/Arrows_(Unicode_block)
	//_tprintf(L"\u2554 \u2557 \u255a \u255d \u2550 \u2551 \n");
	//_tprintf(L"\u259b \u259c \u2599 \u259f \u2583 \u2590 \n");
	//_tprintf(L"\u2190 \u2191 \u2192 \u2193 \n");

	//start game

	GameBoard* gb = initGameboard();

	_getch();

	BOOL isGameRunning = TRUE;

	while (isGameRunning)
	{
		drawBoardToConsole(gb);
		isGameRunning = FALSE;
	}

	_tprintf(L"Game ended.\n");

	//initSharedMemory();
	//UnmapSharedMemory();
	free(gb);

	return 0;
}