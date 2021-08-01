#include <stdlib.h>
#include <string.h>

#include "chess.h"
#include "log.h"

bool insert_valid_move(const chess *c, dllist **valid_moves, pos from, pos to);
bool check_for_check(const chess *c, bool is_check_white);
dllist *unchecked_moves_starting_from(const chess *c, pos p);
bool filter_check_own_checkmate_after_move(void *m);

typedef enum {
	TARGET_EMPTY = 1 << 0,
	TARGET_ENEMY = 1 << 1,
	TARGET_ALLY = 1 << 2
} move_target;
bool add_move_if_target_valid(const chess *c, pos from, pos to, dllist **moves, move_target target_types);
bool check_target_valid(const chess *c, pos to, move_target target_types);

chess * init_chess(chess *c) {
	u8 board_initial_state[BOARD_SIDE_LENGTH][BOARD_SIDE_LENGTH] = 
	{
		{ROOK | WHITE, KNIGHT | WHITE, BISHOP | WHITE, QUEEN | WHITE, KING | WHITE, BISHOP | WHITE, KNIGHT | WHITE, ROOK | WHITE},
		{PAWN | WHITE, PAWN | WHITE, PAWN | WHITE, PAWN | WHITE, PAWN | WHITE, PAWN | WHITE, PAWN | WHITE, PAWN | WHITE},
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0},
		{PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN},
		{ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK}
	};

	ASSERT_ERROR (c, "Argument c was NULL");

	bool white_castle_l_possible = true;
	bool white_castle_r_possible = true;
	bool black_castle_l_possible = true;
	bool black_castle_r_possible = true;

	ASSERT_ERROR (memcpy(c->board, &board_initial_state, sizeof (board_initial_state)), "Memcpy returned NULL");

	return c;
}

bool in_check_next_move(chess *c, pos from, pos to);

dllist *valid_moves_starting_from(const chess *c, pos p)
{
	dllist *moves = unchecked_moves_starting_from(c, p);

	dllist_filter(moves, &filter_check_own_checkmate_after_move);

	return moves;
}

dllist *unchecked_moves_starting_from(const chess *c, pos p)
{
	dllist *moves = NULL;
	u8 x_target, y_target;

	ASSERT_ERROR (c && (p.x < BOARD_SIDE_LENGTH) && (p.y < BOARD_SIDE_LENGTH), "Error invalid arguments");

	if (0 == c->board[p.y][p.x] || c->is_whites_turn != (c->board[p.y][p.x] & WHITE)) {
		//LOG_DEBUG ("No %s piece found at (%hhu,%hhu)", c->is_whites_turn ? "white" : "black", p.x, p.y);
		return NULL;
	}

	// TODO add functionality
	switch (c->board[p.y][p.x] & ~WHITE) {
	case PAWN: // TODO test
		ASSERT_ERROR (p.y > 0 && p.y < BOARD_SIDE_LENGTH - 1, "%s pawn on invalid rank %hhu", c->is_whites_turn ? "White" : "Black", p.y + 1);

		// check move one straight
		if (add_move_if_target_valid(c, p, (pos) { p.x, p.y + (c->is_whites_turn ? 1 : (-1)) }, &moves, TARGET_EMPTY)) {
			// check double move from beginning rank
			add_move_if_target_valid(c, p, (pos) { p.x, (c->is_whites_turn ? 3 : 4) }, &moves, TARGET_EMPTY);
		}

		// check attacks left and right
		add_move_if_target_valid(c, p, (pos) { p.x + 1, p.y + (c->is_whites_turn ? 1 : -1) }, &moves, TARGET_EMPTY);
		add_move_if_target_valid(c, p, (pos) { p.x - 1, p.y + (c->is_whites_turn ? 1 : -1) }, &moves, TARGET_EMPTY);

		break;

	case ROOK: // TODO testing
		for (y_target = p.y + 1; add_move_if_target_valid(c, p, (pos) { p.x, y_target }, &moves, TARGET_EMPTY); y_target++);
		add_move_if_target_valid(c, p, (pos) { p.x, y_target }, &moves, TARGET_ENEMY);

		for (y_target = p.y - 1; add_move_if_target_valid(c, p, (pos) { p.x, y_target }, &moves, TARGET_EMPTY); y_target--);
		add_move_if_target_valid(c, p, (pos) { p.x, y_target }, &moves, TARGET_ENEMY);

		for (x_target = p.x + 1; add_move_if_target_valid(c, p, (pos) { x_target, p.y }, &moves, TARGET_EMPTY); x_target++);
		add_move_if_target_valid(c, p, (pos) { x_target, p.y }, &moves, TARGET_ENEMY);

		for (x_target = p.x - 1; add_move_if_target_valid(c, p, (pos) { x_target, p.y }, &moves, TARGET_EMPTY); x_target--);
		add_move_if_target_valid(c, p, (pos) { x_target, p.y }, &moves, TARGET_ENEMY);

		break;

	case KNIGHT:
		add_move_if_target_valid(c, p, (pos) { p.x + 1, p.y + 2 }, &moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_target_valid(c, p, (pos) { p.x - 1, p.y + 2 }, &moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_target_valid(c, p, (pos) { p.x + 1, p.y - 2 }, &moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_target_valid(c, p, (pos) { p.x - 1, p.y - 2 }, &moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_target_valid(c, p, (pos) { p.x + 2, p.y + 1 }, &moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_target_valid(c, p, (pos) { p.x - 2, p.y + 1 }, &moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_target_valid(c, p, (pos) { p.x + 2, p.y - 1 }, &moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_target_valid(c, p, (pos) { p.x - 2, p.y - 1 }, &moves, TARGET_ENEMY | TARGET_EMPTY);
		

		break;

	case BISHOP: // TODO testing
		for (x_target = p.x + 1, y_target = p.y + 1; add_move_if_target_valid(c, p, (pos) { x_target, y_target }, &moves, TARGET_EMPTY); x_target++, y_target++);
		add_move_if_target_valid(c, p, (pos) { x_target, y_target }, &moves, TARGET_ENEMY);

		for (x_target = p.x + 1, y_target = p.y - 1; add_move_if_target_valid(c, p, (pos) { x_target, y_target }, &moves, TARGET_EMPTY); x_target++, y_target--);
		add_move_if_target_valid(c, p, (pos) { x_target, y_target }, &moves, TARGET_ENEMY);

		for (x_target = p.x - 1, y_target = p.y + 1; add_move_if_target_valid(c, p, (pos) { x_target, y_target }, &moves, TARGET_EMPTY); x_target--, y_target++);
		add_move_if_target_valid(c, p, (pos) { x_target, y_target }, &moves, TARGET_ENEMY);

		for (x_target = p.x - 1, y_target = p.y - 1; add_move_if_target_valid(c, p, (pos) { x_target, y_target }, &moves, TARGET_EMPTY); x_target--, y_target--);
		add_move_if_target_valid(c, p, (pos) { x_target, y_target }, &moves, TARGET_ENEMY);

		break;

	case QUEEN:
		for (x_target = p.x + 1, y_target = p.y + 1; add_move_if_target_valid(c, p, (pos) { x_target, y_target }, &moves, TARGET_EMPTY); x_target++, y_target++);
		add_move_if_target_valid(c, p, (pos) { x_target, y_target }, &moves, TARGET_ENEMY);

		for (x_target = p.x + 1, y_target = p.y - 1; add_move_if_target_valid(c, p, (pos) { x_target, y_target }, &moves, TARGET_EMPTY); x_target++, y_target--);
		add_move_if_target_valid(c, p, (pos) { x_target, y_target }, &moves, TARGET_ENEMY);

		for (x_target = p.x - 1, y_target = p.y + 1; add_move_if_target_valid(c, p, (pos) { x_target, y_target }, &moves, TARGET_EMPTY); x_target--, y_target++);
		add_move_if_target_valid(c, p, (pos) { x_target, y_target }, &moves, TARGET_ENEMY);

		for (x_target = p.x - 1, y_target = p.y - 1; add_move_if_target_valid(c, p, (pos) { x_target, y_target }, &moves, TARGET_EMPTY); x_target--, y_target--);
		add_move_if_target_valid(c, p, (pos) { x_target, y_target }, &moves, TARGET_ENEMY);

		for (y_target = p.y + 1; add_move_if_target_valid(c, p, (pos) { p.x, y_target }, &moves, TARGET_EMPTY); y_target++);
		add_move_if_target_valid(c, p, (pos) { p.x, y_target }, &moves, TARGET_ENEMY);

		for (y_target = p.y - 1; add_move_if_target_valid(c, p, (pos) { p.x, y_target }, &moves, TARGET_EMPTY); y_target--);
		add_move_if_target_valid(c, p, (pos) { p.x, y_target }, &moves, TARGET_ENEMY);

		for (x_target = p.x + 1; add_move_if_target_valid(c, p, (pos) { x_target, p.y }, &moves, TARGET_EMPTY); x_target++);
		add_move_if_target_valid(c, p, (pos) { x_target, p.y }, &moves, TARGET_ENEMY);

		for (x_target = p.x - 1; add_move_if_target_valid(c, p, (pos) { x_target, p.y }, &moves, TARGET_EMPTY); x_target--);
		add_move_if_target_valid(c, p, (pos) { x_target, p.y }, &moves, TARGET_ENEMY);
		break;
	case KING:
		add_move_if_target_valid(c, p, (pos) { p.x + 1, p.y }, &moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_target_valid(c, p, (pos) { p.x - 1, p.y }, &moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_target_valid(c, p, (pos) { p.x, p.y + 1 }, &moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_target_valid(c, p, (pos) { p.x, p.y - 1 }, &moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_target_valid(c, p, (pos) { p.x + 1, p.y + 1 }, &moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_target_valid(c, p, (pos) { p.x - 1, p.y + 1 }, &moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_target_valid(c, p, (pos) { p.x + 1, p.y - 1 }, &moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_target_valid(c, p, (pos) { p.x - 1, p.y - 1 }, &moves, TARGET_ENEMY | TARGET_EMPTY);
		break;
	}

	return moves;
}

#define TARGET_ENEMY (1 << 0)
#define TARGET_EMPTY (1 << 1)
bool check_target_valid(const chess *c, pos to, move_target target_types)
{
	bool target_empty_statisfied = (target_types & TARGET_EMPTY)
		&& 0 == c->board[to.y][to.x];
	bool target_enemy_statisfied = (target_types & TARGET_ENEMY)
		&& 0 != c->board[to.y][to.x] && c->is_whites_turn != (c->board[to.y][to.x] * WHITE);

	return (0 <= to.x && to.x < BOARD_SIDE_LENGTH && 0 <= to.y && to.y < BOARD_SIDE_LENGTH) && (target_enemy_statisfied || target_empty_statisfied);
}

bool add_move_if_target_valid(const chess *c, pos from, pos to, dllist **moves, move_target target_types)
{
	move *m;
	if (check_target_valid(c, to, target_types)) {
		m = calloc(1, sizeof (move));
		ASSERT_ERROR (m, "calloc returned NULL!");
		memcpy(&m->before, c, sizeof (chess));
		memcpy(&m->after, c, sizeof (chess));
		m->after.board[to.y][to.x] = m->before.board[from.y][from.x];
		m->before.board[from.y][from.x] = 0;

		if (m->before.is_whites_turn) {
			m->after.white_castle_l_possible |= (from.x == 0 && from.y == 0 || from.x == 4 && from.y == 0);
			m->after.white_castle_r_possible |= (from.x == 7 && from.y == 0 || from.x == 4 && from.y == 0);
		} else {
			m->after.black_castle_l_possible |= (from.x == 0 && from.y == 7 || from.x == 4 && from.y == 7);
			m->after.black_castle_r_possible |= (from.x == 7 && from.y == 7 || from.x == 4 && from.y == 7);
		}

		dllist_insert_last(*moves, m);
		return true;
	}
	return false;
}

bool filter_check_own_checkmate_after_move(move *m)
{
	ASSERT_ERROR (m, "Argument is NULL");
	return false;
}