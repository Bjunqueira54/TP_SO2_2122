#pragma once
#include <Windows.h>
#include <tchar.h>

#include "..\Global Data Structures\GameBoard.h"

//numero maximo de jogadores
#define N_MAX_LEITORES 10

//Tam default
#define TAM 256

//Named pipes
#define PIPE_NAME1 TEXT("\\\\.\\pipe\\teste1")//Escreve
#define PIPE_NAME2 TEXT("\\\\.\\pipe\\teste2")//Le

extern TCHAR PIPE_NAME3[TAM];
extern TCHAR PIPE_NAME4[TAM];

BoardCoord bc;

DWORD WINAPI ThreadComunicacao(LPVOID);
DWORD WINAPI ThreadRecebeClientes(LPVOID);