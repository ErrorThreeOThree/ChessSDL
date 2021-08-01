#ifndef CHESS_H
#define CHESS_H

#include "log.h"
#include "types.h"
#include "utils.h"

#define BOARD_SIDE_LENGTH 8

typedef struct {
	u8 x, y;
} pos;

typedef enum {
	WHITE = 1 << 0,
	PAWN = 1 << 1,
	ROOK = 1 << 2,
	KNIGHT = 1 << 3,
	BISHOP = 1 << 4,
	QUEEN = 1 << 5,
	KING = 1 << 6
} piece_type_bm;

typedef struct {
	bool is_whites_turn;
	bool white_castle_l_possible;
	bool white_castle_r_possible;
	bool black_castle_l_possible;
	bool black_castle_r_possible;
	u8 board[BOARD_SIDE_LENGTH][BOARD_SIDE_LENGTH];
} chess;

typedef struct {
	chess before;
	chess after;
	pos from;
	pos to;
} move;

chess * init_chess(chess *c);

dllist *valid_moves_starting_from(const chess *c, pos p);

move * is_legal_move(move *m);

bool * try_move(chess *c, move *m, dllist *history);

#endif