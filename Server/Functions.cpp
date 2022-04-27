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
			gb.board[y][x].current.piece = E;	//All cells to empty
			gb.board[y][x].end_side = NO;		//No cell is the end yet
			gb.board[y][x].isStart = 0;			//No cell is the start yet
		}
	}

	///////////////////////
	///Choose start cell///
	///////////////////////



	//////////////////////////////////
	///Choose end cell and the side///
	//////////////////////////////////

	gb.board[0][0].end_side = R;
}