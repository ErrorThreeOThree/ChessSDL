#include <stdio.h>
#include <stdlib.h>

#include "SDL.h"
#include "chess.h"
#include "log.h"

int main(int argc, char **argv)
{
	LOG_INFO ("Starting program");
	chess c;

	init_chess(&c);
	dllist *moves = valid_moves_starting_from(&c, (pos) { 1, 1 });

	LOG_INFO ("Exiting program successfully");
	return EXIT_SUCCESS;
}