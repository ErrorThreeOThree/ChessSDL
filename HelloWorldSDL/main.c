#include <stdio.h>
#include <stdlib.h>

#include "SDL.h"
#include "chess.h"
#include "log.h"
#include "utils.h"

int main(int argc, char **argv)
{
	LOG_INFO ("Starting program");
	LOG_DEBUG ("Got arguments:");
	while (argc--)
	{
		LOG_DEBUG ("Argument %d \"%s\"", argc, argv);
	}
	chess_state c;
	pos p;

	init_chess(&c);
	dllist *moves = valid_moves(&c);

	printf("Number of valid moves: %llu\n", dllist_size(moves));
	for (p.y = 0; p.y < BOARD_SIDE_LENGTH; ++p.y) {
		for (p.x = 0; p.x < BOARD_SIDE_LENGTH; ++p.x) {
			moves = valid_moves_starting_from(&c, p);
			if (dllist_size(moves) > 0)
				printf("Number of valid moves starting from (%hhu,%hhu): %llu\n", p.x, p.y, dllist_size(moves));
		}
	}
	LOG_INFO ("Exiting program successfully");
	return EXIT_SUCCESS;
}