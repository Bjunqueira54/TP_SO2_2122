#include "Functions.h"

void initGameboard()
{
	///////////////////////////
	///Fecth Registry Values///
	///////////////////////////



	////////////////////////
	///Allocate the board///
	////////////////////////

	GameBoard gb;

	gb.y = 20;	//Registry Values
	gb.x = 20;	//Replace later with correct values

	gb.board = (GameCell**) malloc(gb.x * sizeof(GameCell*));

	for (int i = 0; i < gb.x; i++)
	{
		gb.board[i] = (GameCell*) malloc(gb.y * sizeof(GameCell));
	}

	///////////////
	///Set cells///
	///////////////

	for (int y = 0; y < gb.y; y++)
	{
		for (int x = 0; x < gb.x; x++)
		{
			gb.board[y][x].piece = E;		//All cells are empty
			gb.board[y][x].isStart = FALSE;	//No cell is the start yet
			gb.board[y][x].isEnd = FALSE;	//No cell is the end yet
			gb.board[y][x].side = NO;		//Since no start or end, set to NO
		}
	}

	///////////////////////
	///Choose start cell///
	///////////////////////

	gb.board[gb.y - 1][gb.x - 1].isStart = TRUE;
	gb.board[gb.y - 1][gb.x - 1].isEnd = FALSE;	//Redundant, but better safe than sorry.
	gb.board[gb.y - 1][gb.x - 1].side = S;

	//////////////////////////////////
	///Choose end cell and the side///
	//////////////////////////////////

	gb.board[0][0].isEnd = TRUE;
	gb.board[0][0].isStart = FALSE;	//Redundant, but better safe than sorry.
	gb.board[0][0].side = L;
}