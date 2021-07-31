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

typedef struct chess {
	bool is_whites_turn;
	bool white_king_moved;
	bool white_rook_0_moved;
	bool white_rook_7_moved;
	bool white_checked_before;
	bool black_king_moved;
	bool black_rook_0_moved;
	bool black_rook_7_moved;
	bool black_checked_before;
	bool is_check;
	u8 board[BOARD_SIDE_LENGTH][BOARD_SIDE_LENGTH];
} chess;

typedef struct {
	chess c;
	pos to;
} move;

chess * init_chess(chess *c);

llist *get_moves_by_pos(const chess *c, pos p);

move * is_legal_move(move *m);

bool * try_move(chess *c, move *m, llist *history);

#endif