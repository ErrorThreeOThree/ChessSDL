#include <stdio.h>
#include <stdlib.h>
#include "chess.h"
#include "log.h"
#include "utils.h"

int tests()
{
	chess c;
	pos p;
	u64 cnt = 0;
	dllist *moves;

	init_chess(&c);

	for (p.y = 0; p.y < BOARD_SIDE_LENGTH; ++p.y) {
		for (p.x = 0; p.x < BOARD_SIDE_LENGTH; ++p.x) {
			moves = valid_moves_from(&c, p);
			cnt += dllist_size(moves);
			if (dllist_size(moves) > 0) {
				printf("Number of valid moves starting from (%hhu,%hhu): %llu\n", p.x, p.y, dllist_size(moves));
				dllist_apply(moves, print_move);
			}
		}
	}
	LOG_INFO ("Total number of allowed moves: %llu", cnt);
	ASSERT_ERROR (20 == cnt, "Error: expected 20 moves, got %d", cnt);
	LOG_INFO ("");
	LOG_INFO ("");
	LOG_INFO ("");
	LOG_INFO ("");
	LOG_INFO ("");
	cnt = 0;

	ASSERT_ERROR (try_move(&c, (pos) { 1, 1 }, (pos) { 1, 3 }), "try_move returned false!");

	for (p.y = 0; p.y < BOARD_SIDE_LENGTH; ++p.y) {
		for (p.x = 0; p.x < BOARD_SIDE_LENGTH; ++p.x) {
			moves = valid_moves_from(&c, p);
			cnt += dllist_size(moves);
			if (dllist_size(moves) > 0) {
				printf("Number of valid moves starting from (%hhu,%hhu): %llu\n", p.x, p.y, dllist_size(moves));
				dllist_apply(moves, print_move);
			}
		}
	}
	LOG_INFO ("Total number of allowed moves: %llu", cnt);
	ASSERT_ERROR (20 == cnt, "Error: expected 20 moves, got %d", cnt);
	LOG_INFO ("");
	LOG_INFO ("");
	LOG_INFO ("");
	LOG_INFO ("");
	LOG_INFO ("");
	cnt = 0;

	ASSERT_ERROR(try_move(&c, (pos) { 1, 6 }, (pos) { 1, 4 }), "try_move returned false!");

	for (p.y = 0; p.y < BOARD_SIDE_LENGTH; ++p.y) {
		for (p.x = 0; p.x < BOARD_SIDE_LENGTH; ++p.x) {
			moves = valid_moves_from(&c, p);
			cnt += dllist_size(moves);
			if (dllist_size(moves) > 0) {
				printf("Number of valid moves starting from (%hhu,%hhu): %llu\n", p.x, p.y, dllist_size(moves));
				dllist_apply(moves, print_move);
			}
		}
	}
	LOG_INFO ("Total number of allowed moves: %llu", cnt);
	ASSERT_ERROR (18 == cnt, "Error: expected 18 moves, got %d", cnt);
	return EXIT_SUCCESS;
}