#ifndef CHESS_H
#define CHESS_H

#include "log.h"
#include "types.h"
#include "utils.h"

#define BOARD_SIDE_LENGTH 8

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
	bool is_white_turn;
	bool white_king_moved;
	bool black_king_moved;
	u32 board[BOARD_SIDE_LENGTH][BOARD_SIDE_LENGTH];
} chess;

typedef struct {
	chess before, after;
	u8 x, y;
} move;

chess * init_chess(chess *c);

llist *piece_moves(const chess *c, u8 x, u8 y);

move * is_legal_move(move *m);

bool * try_move(chess *c, move *m, llist *history);

#endif