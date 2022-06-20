#include <string.h>
#include <tchar.h>
#include "pipeFunctions.h"
#include "..\Global Data Structures\GameBoard.h"

//numero maximo de jogadores
#define N_MAX_LEITORES 10

//Tam default
#define TAM 256

//Named pipes
#define PIPE_NAME1 TEXT("\\\\.\\pipe\\teste1")//Escreve
#define PIPE_NAME2 TEXT("\\\\.\\pipe\\teste2")//Le

TCHAR CLIENT_PIPE_NAME[TAM] = L"\\\\.\\pipe\\cliente"; //escrita para servidor
TCHAR SERVER_PIPE_NAME[TAM] = L"\\\\.\\pipe\\servidor"; //leitura do servidor

typedef struct
{
	HANDLE hPipe;
	Data* data;
	HANDLE hWnd;
} NamedPipeInfoThread;

DWORD WINAPI pipeWriteThread(LPVOID);
DWORD WINAPI pipeReadThread(LPVOID);

DWORD WINAPI pipeThread(LPVOID param)
{
	PipeThreadInfo* pti = (PipeThreadInfo*)param;

	Data* data = pti->data;
	HANDLE hWnd = pti->hWnd;

	int option;
	HANDLE PipeEscrita = NULL;
	HANDLE PipeLeitura = NULL;
	HANDLE hPipe = NULL;
	data->hBoardEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, boardEventName);
	DWORD extra = NULL;
	DWORD pid = GetCurrentProcessId();
	int ret;

	if (!WaitNamedPipe(PIPE_NAME2, 10000))
		return -1;

	if (data->hBoardEvent == NULL)
		return -1;

	hPipe = CreateFile(PIPE_NAME2, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hPipe == NULL)
		return -1;

	ConnectNamedPipe(hPipe, NULL);

	ret = WriteFile(hPipe, (LPVOID)&pid, sizeof(pid), &extra, NULL);
	if (ret != TRUE) return -1;

	DisconnectNamedPipe(hPipe);

	TCHAR buffer[32] = L"";
	_stprintf_s(buffer, (unsigned)_countof(buffer), L"\\%d", pid);

	_tcscat_s(CLIENT_PIPE_NAME, sizeof(buffer), buffer);
	_tcscat_s(SERVER_PIPE_NAME, sizeof(buffer), buffer);

	WaitNamedPipe(SERVER_PIPE_NAME, NMPWAIT_WAIT_FOREVER);
	Sleep(1000);

	PipeEscrita = CreateFile(CLIENT_PIPE_NAME, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hPipe == NULL)
		return -1;
	
	PipeLeitura = CreateFile(SERVER_PIPE_NAME, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hPipe == NULL)
		return -1;

	NamedPipeInfoThread* ReadPipe = malloc(sizeof(NamedPipeInfoThread));
	if (ReadPipe == NULL) return -1;

	ReadPipe->hPipe = PipeLeitura;
	ReadPipe->data = data;
	ReadPipe->hWnd = hWnd;

	NamedPipeInfoThread* WritePipe = malloc(sizeof(NamedPipeInfoThread));
	if (WritePipe == NULL) return -1;

	WritePipe->hPipe = PipeEscrita;
	WritePipe->data = data;
	WritePipe->hWnd = NULL;

	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)pipeWriteThread, (LPVOID)WritePipe, NULL, NULL);
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)pipeReadThread, (LPVOID)ReadPipe, NULL, NULL);

	return 0;
}

DWORD WINAPI pipeReadThread(LPVOID param)
{
	NamedPipeInfoThread* pi = (NamedPipeInfoThread*)param;

	HANDLE PipeLeitura = pi->hPipe;
	Data* data = pi->data;
	HANDLE hWnd = pi->hWnd;
	DWORD extra;
	
	while (TRUE)
	{
		if (WaitForSingleObject(data->hPipeEvent, INFINITE) == WAIT_OBJECT_0)
		{
			if (ReadFile(PipeLeitura, (LPVOID)&data->fc->gameboard, sizeof(GameBoard), &extra, NULL) == FALSE)
				break;
			
			InvalidateRect(hWnd, NULL, FALSE);
		}

		ResetEvent(data->hPipeEvent);

		if (!data->fc->gameboard.isGameRunning)
			break;
	}
	
	return 0;
}

DWORD WINAPI pipeWriteThread(LPVOID param)
{
	NamedPipeInfoThread* pi = (NamedPipeInfoThread*)param;

	HANDLE PipeEscrita = pi->hPipe;
	Data* data = pi->data;
	DWORD extra;

	//Send moves here
	while (TRUE)
	{
		if (WaitForSingleObject(data->hMoveEvent, INFINITE) == WAIT_OBJECT_0)
		{
			if (!WriteFile(PipeEscrita, (LPCVOID)&bc, sizeof(BoardCoord), &extra, NULL))
				break;
		}
		ResetEvent(data->hMoveEvent);
	}
	return 0;
}