#include "Functions.h"
#include "pipeFunctions.h"

Data* initData()
{
	Data* data = calloc(1, sizeof(Data));
	if (data == NULL) return -1;
	data->fc = calloc(1, sizeof(FlowControl));
	if (data->fc == NULL) return -1;
	data->hMoveEvent = CreateEvent(NULL, TRUE, FALSE, internalClientEvent);
	data->hPipeEvent = CreateEvent(NULL, TRUE, FALSE, pipeEventName);

	GameBoard* board = calloc(1, sizeof(GameBoard));
	if (board == NULL) return -1;
	memset(board, 0, sizeof(GameBoard));
	CopyMemory(&data->fc->gameboard, board, sizeof(GameBoard));

	data->fc->gameboard.x = -1;

	return data;
}