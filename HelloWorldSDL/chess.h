#ifndef CHESS_H
#define CHESS_H

#include "log.h"
#include "types.h"
#include "utils.h"

#define BOARD_SIDE_LENGTH 8

typedef struct {
	i32 x, y;
} pos;

typedef enum {
	WHITE,
	BLACK,
	COLOR_MAX
} piece_color;

static inline const char *piece_color_string(piece_color c)
{
	static const char *s[] = { "white", "black", "invalid" };
	ASSERT_WARNING (COLOR_MAX != c, "Invalid value PIECE_COLOR_MAX");
	return s[c];
}

typedef enum {
	PAWN,
	ROOK,
	KNIGHT,
	BISHOP,
	QUEEN,
	KING,
	PIECE_TYPE_MAX
} piece_type;

static inline const char *piece_type_string(piece_type t)
{
	static const char *s[] = { "pawn", "rook", "knight", "bishop", "queen", "king", "invalid" };
	ASSERT_WARNING (PIECE_TYPE_MAX != t, "Invalid value PIECE_TYPE_MAX");
	return s[t];
}

typedef struct {
	bool is_piece;
	piece_color c;
	piece_type t;
} piece;

typedef enum {
	LEFT,
	RIGHT,
	DIRECTION_MAX
} direction;

typedef struct {
	piece_color active_color;
	bool can_castle[DIRECTION_MAX][COLOR_MAX];
	piece board[BOARD_SIDE_LENGTH][BOARD_SIDE_LENGTH];
	dllist allowed_moves[BOARD_SIDE_LENGTH][BOARD_SIDE_LENGTH];
	bool can_en_pessant[BOARD_SIDE_LENGTH][COLOR_MAX];
} chess_state;

typedef enum {
	TARGET_ENEMY = 1 << 0,
	TARGET_ALLY = 1 << 1,
	TARGET_EN_PESSANT = 1 << 2,
	TARGET_INVALID = 1 << 3,
	TARGET_EMPTY = 1 << 4,
	CASTLE_L = 1 << 5,
	CASTLE_R = 1 << 6,
} move_target;

typedef struct {
	chess_state before;
	chess_state after;
	pos from;
	pos to;
	move_target move_type;
} move;

typedef struct {
	dllist history;
	chess_state current_state;
} chess;

chess * init_chess(chess *c);

const dllist * valid_moves_from(const chess *c, pos p);

bool try_move(chess *c, pos from, pos to);

void print_move(const move *m);

#endif