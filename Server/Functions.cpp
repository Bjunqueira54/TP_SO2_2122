#include "Functions.h"

void initGameboard()
{
	//fecth registry values here


	//Allocate the board

	GameBoard gb;

	gb.y = 20;	//Registry Values
	gb.x = 20;	//Replace later with correct values

	gb.board = (GameCell**) malloc(gb.x * sizeof(GameCell*));

	for (int i = 0; i < gb.x; i++)
	{
		gb.board[i] = (GameCell*) malloc(gb.y * sizeof(GameCell));
	}


}