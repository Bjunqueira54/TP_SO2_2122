#pragma once
#include <Windows.h>
#define DIM 5
#define CMD_MAX_LENGTH 25

extern TCHAR memoryName[];
extern TCHAR boardEventName[];
extern TCHAR cmdEventName[];
extern TCHAR sMutexName[];
extern TCHAR sItemsName[];
extern TCHAR sEmptyName[];

typedef enum
{
	H,	//Horizontal
	V,	//Vertical
	UL,	//Up-to-Left
	UR,	//Up-to-Right
	DL,	//Down-to-Left
	DR,	//Down-to-Right
	E	//Empty
} PieceType;

typedef enum
{
	N,	//North
	S,	//South
	R,	//East
	L,	//West
	NO	//None
} Side;

typedef struct
{
	BOOL isStart;	//Make sure that: 1. Both are FALSE
	BOOL isEnd;		//2. Only one is TRUE
	BOOL isEnabled;
	BOOL isWall;

	BOOL isFlooded;	//Has water reached this pos?

	PieceType piece;
	Side side;	//Works for both start and end.
} GameCell;

typedef struct
{
	int y;
	int x;

	BOOL isGameRunning;
	BOOL isWaterRunning;

	GameCell board[20][20];	//Array of game cells
} GameBoard;

typedef struct
{
	int in;
	int out;
	TCHAR cmdBuffer[DIM][CMD_MAX_LENGTH];
} Buffer;

typedef struct
{
	GameBoard gameboard;

	Buffer buffer;
} FlowControl;

typedef struct
{
	FlowControl* fc;

	HANDLE hGameMemory;

	//HANDLE hMutex; 
	HANDLE sMutex; // semaforo "mutex" previne o acesso simultaneo a in(exclusao mutua do buffer circular)
	HANDLE sItems; // semaforo 1 Para contabilizar o numero de itens disponiveis  
	HANDLE sEmpty; // semaforo 2 Para contabilizar o numero de elementos livres no buffer

	HANDLE hBoardEvent;
	HANDLE hCommandEvent;
} Data;