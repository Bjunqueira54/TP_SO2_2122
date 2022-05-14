#pragma once
#include <Windows.h>

typedef enum _piecetype
{
	H,	//Horizontal
	V,	//Vertical
	UL,	//Up-to-Left
	UR,	//Up-to-Right
	DL,	//Down-to-Left
	DR,	//Down-to-Right
	E	//Empty
} PieceType;

typedef enum _side
{
	N,	//North
	S,	//South
	R,	//East
	L,	//West
	NO	//None
} Side;

typedef struct _gamecell
{
	BOOL isStart;	//Make sure that: 1. Both are FALSE
	BOOL isEnd;		//2. Only one is TRUE

	BOOL isFlooded;	//Has water reached this pos?

	PieceType piece;
	Side side;	//Works for both start and end.
} GameCell;

typedef struct _gameboard
{
	int y;
	int x;

	BOOL isGameRunning;

	GameCell** board;	//Array of game cells
} GameBoard, *pGameBoard;