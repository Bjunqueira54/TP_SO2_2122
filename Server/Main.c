#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>

//SERVER
int _tmain(int argc, TCHAR** argv)
{
#ifdef UNICODE

	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);

#endif

	_tprintf(L"Teste unicode no servidor: á à\n");

	return 0;
}