#pragma once
#include <Windows.h>

#define CMD_MAX_LENGHT 25
#define DIM 5
#define DIM_S 25

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
	TCHAR cmdBuffer[DIM][DIM_S];
} Buffer;

typedef struct
{
	HANDLE hMutex;
	HANDLE hEvent;

	GameBoard gb;

	Buffer buffer;
} FlowControl;