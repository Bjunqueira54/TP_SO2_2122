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

	_tprintf(L"Teste unicode no servidor: á à\n");

	initSharedMemory();

	_getch();

	UnmapSharedMemory();

	return 0;
}