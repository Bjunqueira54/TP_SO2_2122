#pragma once
#include <Windows.h>
#include <tchar.h>

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

typedef struct _gamepiece
{
	PieceType piece;

} GamePiece;

typedef struct _gamecell
{
	GamePiece current;

	int isStart;
	int end_side;
} GameCell;

typedef struct _gameboard
{
	int y;
	int x;

	GameCell** board;	//Array of game cells
} GameBoard, *pGameBoard;