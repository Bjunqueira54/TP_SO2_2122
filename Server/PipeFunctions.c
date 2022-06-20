#include <string.h>
#include <tchar.h>

#include "PipeFunctions.h"
#include "..\Global Data Structures\GameBoard.h"
#include "Functions.h"

Cliente* c1 = NULL;
Cliente* c2 = NULL;

typedef struct
{
	Data* data;
	Cliente* client;
} ClientInfo;

typedef struct
{
	Data* data;

	PieceType pipe_type;
	int pipe_y;
	int pipe_x;
} PipeInfo;

typedef struct
{
	Data* data;
	HANDLE hPipe;
} NamedPipeInfo;

DWORD WINAPI pipeReadThread(LPVOID param)
{
	NamedPipeInfo* pi = (NamedPipeInfo*)param;
	Data* data = pi->data;
	HANDLE PipeLeitura = pi->hPipe;
	DWORD extra;

	ConnectNamedPipe(PipeLeitura, NULL);

	while (TRUE)
	{
		if (WaitForSingleObject(data->hPipeEvent, INFINITE) == WAIT_OBJECT_0)
		{
			if (WaitForSingleObject(data->sMutex, INFINITE) == WAIT_OBJECT_0)
			{
				UnmapViewOfFile(data->fc);
				data->fc = (FlowControl*)MapViewOfFile(data->hGameMemory, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(FlowControl));
				if (data->fc == NULL)
				{
					ReleaseSemaphore(data->sMutex, 1, NULL);
					return -1;
				}
			}

			if (!ReadFile(PipeLeitura, (LPCVOID)&bc, sizeof(GameBoard), &extra, NULL))
			{
				ReleaseSemaphore(data->sMutex, 1, NULL);
				break;
			}
		

			if (!data->fc->gameboard.isGameRunning)
			{
				ReleaseSemaphore(data->sMutex, 1, NULL);
				break;
			}

			PipeInfo* pi = malloc(sizeof(PipeInfo));
			if (pi == NULL) return;

			pi->data = data;
			pi->pipe_type = bc.piece;
			pi->pipe_y = bc.y;
			pi->pipe_x = bc.x;
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)addPipeThread, (LPVOID)pi, 0, NULL);
		}
		ReleaseSemaphore(data->sMutex, 1, NULL);
		ResetEvent(data->hPipeEvent);
	}

	return 0;
}

DWORD WINAPI pipeWriteThread(LPVOID param)
{
	NamedPipeInfo* pi = (NamedPipeInfo*) param;
	Data* data = pi->data;
	HANDLE PipeEscrita = pi->hPipe;
	DWORD extra;

	ConnectNamedPipe(PipeEscrita, NULL);

	while (TRUE)
	{
		if (WaitForSingleObject(data->sMutex, INFINITE) == WAIT_OBJECT_0)
		{
			UnmapViewOfFile(data->fc);
			data->fc = (FlowControl*)MapViewOfFile(data->hGameMemory, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(FlowControl));
			if (data->fc == NULL)
			{
				ReleaseSemaphore(data->sMutex, 1, NULL);
				return -1;
			}
			
			if (!WriteFile(PipeEscrita, (LPCVOID)&data->fc->gameboard, sizeof(GameBoard), &extra, NULL)) break;
		}
		if (!data->fc->gameboard.isGameRunning)
		{
			ReleaseSemaphore(data->sMutex, 1, NULL);
			break;
		}

		ReleaseSemaphore(data->sMutex, 1, NULL);
		SetEvent(data->hPipeEvent);
		Sleep(1000);
	}
	DisconnectNamedPipe(PipeEscrita);
	return 0;
}

DWORD WINAPI ThreadComunicacao(LPVOID param)
{
	ClientInfo* ci = (ClientInfo*) param;
	Data* data = ci->data;

	HANDLE PipeEscrita;
	HANDLE PipeLeitura;
	DWORD extra;

	PipeEscrita = CreateNamedPipe(ci->client->pipe_servidor, PIPE_ACCESS_OUTBOUND, PIPE_WAIT | PIPE_TYPE_BYTE
		| PIPE_READMODE_BYTE, N_MAX_LEITORES, sizeof(GameBoard), sizeof(GameBoard), 1000, NULL);
	if (PipeEscrita == NULL)
	{
		_tprintf(TEXT("[ERRO Thread Comunicacao] Ligar ao pipe escrita\n"));
		return -1;
	}

	PipeLeitura = CreateNamedPipe(ci->client->pipe_cliente, PIPE_ACCESS_INBOUND, PIPE_WAIT | PIPE_TYPE_BYTE
		| PIPE_READMODE_BYTE, N_MAX_LEITORES, sizeof(BoardCoord), sizeof(BoardCoord), 1000, NULL);
	if (PipeLeitura == NULL)
	{
		_tprintf(TEXT("[ERRO Thread Comunicacao] Ligar ao pipe leitura\n"));
		return -1;
	}

	NamedPipeInfo* ReadPipe = malloc(sizeof(NamedPipeInfo));
	if (ReadPipe == NULL) return -1;

	ReadPipe->data = data;
	ReadPipe->hPipe = PipeLeitura;

	NamedPipeInfo* WritePipe = malloc(sizeof(NamedPipeInfo));
	if (WritePipe == NULL) return -1;

	WritePipe->data = data;
	WritePipe->hPipe = PipeEscrita;

	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)pipeReadThread, (LPVOID)ReadPipe, NULL, NULL);
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)pipeWriteThread, (LPVOID)WritePipe, NULL, NULL);

	//free(ci);
	return 0;
}

DWORD WINAPI ThreadRecebeClientes(LPVOID param)
{
	Data* data = (Data*) param;
	DWORD pid;
	int ret;
	DWORD lixo;
	while (TRUE)
	{
		if (c1 == NULL || c2 == NULL)
		{
			_tprintf(TEXT("[SERVIDOR Thread] Esperando Clientes \n"));
			ConnectNamedPipe(data->hPipe, NULL);

			if (!ReadFile(data->hPipe, (LPVOID)&pid, sizeof(DWORD), &lixo, NULL)) break;

			DisconnectNamedPipe(data->hPipe);

			_tprintf(TEXT("[SERVIDOR Thread] PID do Cliente : %d \n"), pid);

			if (c1 == NULL)
			{
				c1 = calloc(1, sizeof(Cliente));
				if (c1 == NULL) return -1;
				c1->pid = pid;
				c1->modo_jogo = -1;

				_stprintf_s(c1->pipe_servidor, (unsigned)sizeof(TCHAR) * 32, L"%s\0", PIPE_NAME4);
				_stprintf_s(c1->pipe_cliente, (unsigned)sizeof(TCHAR) * 32, L"%s\0", PIPE_NAME3);


				TCHAR buffer[32] = L"";
				_stprintf_s(buffer, (unsigned)_countof(buffer), L"\\%d", c1->pid);

				_tcscat_s(c1->pipe_servidor, sizeof(buffer), buffer);
				_tcscat_s(c1->pipe_cliente, sizeof(buffer), buffer);

				ClientInfo* ci = malloc(sizeof(ClientInfo));
				if (ci == NULL) return -1;
				ci->data = data;
				ci->client = c1;

				HANDLE ThreadComunicacaoHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadComunicacao, (LPVOID) ci, 0, NULL);
			}
			else if (c2 == NULL)
			{
				c2 = malloc(sizeof(Cliente));
				if (c2 == NULL) return -1;
				memset(c2, 0, sizeof(Cliente));
				c2->pid = pid;
				c2->modo_jogo = -1;

				_stprintf_s(c2->pipe_servidor, (unsigned)sizeof(TCHAR) * 32, L"%s\0", PIPE_NAME4);
				_stprintf_s(c2->pipe_cliente, (unsigned)sizeof(TCHAR) * 32, L"%s\0", PIPE_NAME3);


				TCHAR buffer[32] = L"";
				_stprintf_s(buffer, (unsigned)_countof(buffer), L"%d", c2->pid);

				_tcscat_s(c2->pipe_servidor, sizeof(buffer), buffer);
				_tcscat_s(c2->pipe_cliente, sizeof(buffer), buffer);

				ClientInfo* ci = malloc(sizeof(ClientInfo));
				if (ci == NULL) return -1;
				ci->data = data;
				ci->client = c2;

				HANDLE ThreadComunicacaoHandle2 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadComunicacao, (LPVOID) ci, 0, NULL);
			}
			else
				_tprintf(TEXT("[SERVIDOR Thread] Erro\n"));
		}
		else
		{
			_tprintf(TEXT("[SERVIDOR Thread] Clientes full, waiting 10 secs\n"));
			Sleep(10000);
		}
	}
}