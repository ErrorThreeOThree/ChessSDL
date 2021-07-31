#include <stdlib.h>
#include <string.h>

#include "chess.h"
#include "log.h"

bool insert_valid_move(const chess *c, llist **valid_moves, u8 x, u8 y, u8 x_target, u8 y_target);
bool check_target_empty(const chess *c, u8 x, u8 y);
bool check_target_enemy(const chess *c, bool mover_is_white, u8 x, u8 y);

typedef enum {
	TARGET_EMPTY = 1 << 0,
	TARGET_ENEMY = 1 << 1,
	TARGET_ALLY = 1 << 2
} move_target;
bool add_move_if_valid(const chess *c, bool is_white, u8 x, u8 y, llist **valid_moves, move_target allowed_targets);

chess * init_chess(chess *c) {
	u8 board_initial_state[BOARD_SIDE_LENGTH][BOARD_SIDE_LENGTH] = 
	{
		{ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK},
		{PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN},
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0},
		{PAWN | WHITE, PAWN | WHITE, PAWN | WHITE, PAWN | WHITE, PAWN | WHITE, PAWN | WHITE, PAWN | WHITE, PAWN | WHITE},
		{ROOK | WHITE, KNIGHT | WHITE, BISHOP | WHITE, QUEEN | WHITE, KING | WHITE, BISHOP | WHITE, KNIGHT | WHITE, ROOK | WHITE}
	};

	ASSERT_ERROR (c, "Argument c was NULL");

	c->is_white_turn = true;
	c->white_king_moved = false;
	c->black_king_moved = false;

	ASSERT_ERROR (memcpy(&c->board, &board_initial_state, sizeof (board_initial_state)), "Memcpy returned NULL");

	return c;
}

llist *piece_moves(const chess *c, u8 x, u8 y)
{
	llist *valid_moves = NULL;
	u8 x_target, y_target;
	bool is_white;

	ASSERT_ERROR (c && (x < BOARD_SIDE_LENGTH) && (y < BOARD_SIDE_LENGTH), "Error invalid arguments");

	if (0 == c->board[y][x]) {
		LOG_INFO ("No piece found at (%hhu,%hhu)", x, y);
		return NULL;
	}

	is_white = c->board[y][x] & WHITE;

	// TODO add functionality
	switch (c->board[y][x] & ~WHITE) {
	case PAWN: // TODO test
		ASSERT_ERROR (y > 0 && y < BOARD_SIDE_LENGTH - 1, "pawn on invalid rank %hhu", is_white ? "White" : "Black", y + 1);

		// check move one straight
		if (add_move_if_valid(c, is_white, x, y + (is_white ? 1 : (-1)), &valid_moves, TARGET_EMPTY)) {
			// check double move from beginning rank
			add_move_if_valid(c, is_white, x, (is_white ? 3 : 4), &valid_moves, TARGET_EMPTY);
		}

		// check attacks left and right
		add_move_if_valid(c, is_white, x + 1, y + (is_white ? 1 : -1), &valid_moves, TARGET_EMPTY);
		add_move_if_valid(c, is_white, x - 1, y + (is_white ? 1 : -1), &valid_moves, TARGET_EMPTY);

		break;

	case ROOK: // TODO testing
		for (y_target = y + 1; add_move_if_valid(c, is_white, x, y_target, &valid_moves, TARGET_EMPTY); y_target++);
		add_move_if_valid(c, is_white, x, y_target, &valid_moves, TARGET_ENEMY);

		for (y_target = y - 1; add_move_if_valid(c, is_white, x, y_target, &valid_moves, TARGET_EMPTY); y_target--);
		add_move_if_valid(c, is_white, x, y_target, &valid_moves, TARGET_ENEMY);

		for (x_target = x  +1; add_move_if_valid(c, is_white, x_target, y, &valid_moves, TARGET_EMPTY); x_target++);
		add_move_if_valid(c, is_white, x_target, y, &valid_moves, TARGET_ENEMY);

		for (x_target = x - 1; add_move_if_valid(c, is_white, x_target, y, &valid_moves, TARGET_EMPTY); x_target--);
		add_move_if_valid(c, is_white, x_target, y, &valid_moves, TARGET_ENEMY);

		break;

	case KNIGHT:

		break;

	case BISHOP: // TODO testing
		for (x_target = x + 1, y_target = y + 1; add_move_if_valid(c, is_white, x_target, y_target, &valid_moves, TARGET_EMPTY); x_target++, y_target++);
		add_move_if_valid(c, is_white, x_target, y_target, &valid_moves, TARGET_ENEMY);

		for (x_target = x + 1, y_target = y - 1; add_move_if_valid(c, is_white, x_target, y_target, &valid_moves, TARGET_EMPTY); x_target++, y_target--);
		add_move_if_valid(c, is_white, x_target, y_target, &valid_moves, TARGET_ENEMY);

		for (x_target = x - 1, y_target = y + 1; add_move_if_valid(c, is_white, x_target, y_target, &valid_moves, TARGET_EMPTY); x_target--, y_target++);
		add_move_if_valid(c, is_white, x_target, y_target, &valid_moves, TARGET_ENEMY);

		for (x_target = x - 1, y_target = y - 1; add_move_if_valid(c, is_white, x_target, y_target, &valid_moves, TARGET_EMPTY); x_target--, y_target--);
		add_move_if_valid(c, is_white, x_target, y_target, &valid_moves, TARGET_ENEMY);

		break;

	case QUEEN:
		break;
	case KING:
		break;
	}

	return valid_moves;
}

bool insert_valid_move(const chess *c, llist **valid_moves, u8 x, u8 y, u8 x_target, u8 y_target)
{
	move *m = calloc(1, sizeof (move));
	ASSERT_ERROR (m, "calloc returned NULL");
	ASSERT_WARNING (memcpy(&(m->before), c, sizeof (chess)), "memcpy returned NULL");
	ASSERT_WARNING (memcpy(&(m->after), c, sizeof (chess)), "memcpy returned NULL");
	m->after.board[y][x] = 0;
	m->after.board[y_target][x_target] = c->board[y][x];
	llist_insert_last(*valid_moves, m);
	return true;
}

bool check_target_empty(const chess *c, u8 x, u8 y)
{
	ASSERT_ERROR (c, "Variable chess c is NULL!");
	return (0 <= x && x < BOARD_SIDE_LENGTH && 0 <= y && y < BOARD_SIDE_LENGTH)
		&& (0 == c->board[y][x]);
}

bool check_target_enemy(const chess *c, bool is_white, u8 x, u8 y)
{
	ASSERT_ERROR (c, "Variable chess c is NULL!");
	return (0 <= x && x < BOARD_SIDE_LENGTH && 0 <= y && y < BOARD_SIDE_LENGTH)
		&& is_white != (c->board[y][x] & WHITE);
}

bool add_move_if_valid(const chess *c, bool is_white, u8 x, u8 y, llist **valid_moves, move_target allowed_targets)
{
	if ((allowed_targets & TARGET_EMPTY) && check_target_empty(c, x, y)
		|| (allowed_targets & TARGET_ENEMY) && check_target_enemy(c, is_white, x, y))
	{
		ASSERT_ERROR (insert_valid_move(c, valid_moves, x, y, x, y), "insert_valid_move_failed()!");
		return true;
	}

	return false;
}