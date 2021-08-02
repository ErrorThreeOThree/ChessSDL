#include <stdlib.h>
#include <string.h>

#include "chess.h"
#include "log.h"

typedef enum {
	TARGET_EMPTY = 1 << 0,
	TARGET_ENEMY = 1 << 1,
	TARGET_ALLY = 1 << 2
} move_target;

dllist *unchecked_moves_starting_from(const chess *c, pos p);
bool filter_check_own_check_after_move(void *m);
bool check_target_valid(const chess *c, pos to, move_target target_types);
bool add_move_if_target_valid(const chess *c, pos from, pos to, dllist **moves, move_target target_types);
bool check_target_valid(const chess *c, pos to, move_target target_types);

chess * init_chess(chess *c) {
	piece board_initial_state[BOARD_SIDE_LENGTH][BOARD_SIDE_LENGTH] =
	{
		{(piece) { WHITE, ROOK }, (piece) { WHITE, KNIGHT }, (piece) { WHITE, BISHOP }, (piece) { WHITE, QUEEN }, (piece) { WHITE, KING }, (piece) { WHITE, BISHOP }, (piece) { WHITE, KNIGHT }, (piece) { WHITE, ROOK }},
		{(piece) { WHITE, PAWN }, (piece) { WHITE, PAWN }, (piece) { WHITE, PAWN }, (piece) { WHITE, PAWN }, (piece) { WHITE, PAWN }, (piece) { WHITE, PAWN }, (piece) { WHITE, PAWN }, (piece) { WHITE, PAWN }},
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0},
		{(piece) { BLACK, PAWN }, (piece) { BLACK, PAWN }, (piece) { BLACK, PAWN }, (piece) { BLACK, PAWN }, (piece) { BLACK, PAWN }, (piece) { BLACK, PAWN }, (piece) { BLACK, PAWN }, (piece) { BLACK, PAWN }},
		{(piece) { BLACK, ROOK }, (piece) { BLACK, KNIGHT }, (piece) { BLACK, BISHOP }, (piece) { BLACK, QUEEN }, (piece) { BLACK, KING }, (piece) { BLACK, BISHOP }, (piece) { BLACK, KNIGHT }, (piece) { BLACK, ROOK }},
	};

	ASSERT_ERROR (c, "Argument c was NULL");

	bool white_castle_l_possible = true;
	bool white_castle_r_possible = true;
	bool black_castle_l_possible = true;
	bool black_castle_r_possible = true;

	ASSERT_ERROR (memcpy(c->board, &board_initial_state, sizeof (board_initial_state)), "Memcpy returned NULL");

	return c;
}

dllist *valid_moves_starting_from(const chess *c, pos p)
{
	dllist *moves = unchecked_moves_starting_from(c, p);

	dllist_filter(moves, &filter_check_own_check_after_move);

	return moves;
}


dllist *unchecked_moves_starting_from(const chess *c, pos p)
{
	dllist *moves = NULL;
	u8 x_target, y_target;

	ASSERT_ERROR (c && (p.x < BOARD_SIDE_LENGTH) && (p.y < BOARD_SIDE_LENGTH), "Error invalid arguments");

	if (c->active_color != (c->board[p.y][p.x].c)) {
		return NULL;
	}

	switch (c->board[p.y][p.x].t) {
	case PAWN: // TODO test
		ASSERT_ERROR (p.y > 0 && p.y < BOARD_SIDE_LENGTH - 1, "%s pawn on invalid rank %hhu", piece_color_string(c->active_color), p.y + 1);

		// check move one straight
		if (add_move_if_target_valid(c, p, (pos) { p.x, p.y + (c->active_color == WHITE ? 1 : (-1)) }, &moves, TARGET_EMPTY)) {
			// check double move from beginning rank
			add_move_if_target_valid(c, p, (pos) { p.x, (c->active_color == WHITE ? 3 : 4) }, &moves, TARGET_EMPTY);
		}

		// check attacks left and right
		add_move_if_target_valid(c, p, (pos) { p.x + 1, p.y + (c->active_color == WHITE ? 1 : -1) }, &moves, TARGET_EMPTY);
		add_move_if_target_valid(c, p, (pos) { p.x - 1, p.y + (c->active_color == WHITE ? 1 : -1) }, &moves, TARGET_EMPTY);

		break;

	case ROOK:
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

	case BISHOP:
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

bool check_target_valid(const chess *c, pos to, move_target target_types)
{
	bool target_empty_statisfied = (target_types & TARGET_EMPTY)
		&& NONE == c->board[to.y][to.x].c;
	bool target_enemy_statisfied = (target_types & TARGET_ENEMY)
		&& NONE != c->board[to.y][to.x].c && c->active_color != c->board[to.y][to.x].c;

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
		m->before.board[from.y][from.x].c = NONE;
		m->before.board[from.y][from.x].t = EMPTY;

		if (WHITE == m->before.active_color) {
			m->after.white_castle_l_possible |= (from.x == 0 && from.y == 0 || from.x == 4 && from.y == 0);
			m->after.white_castle_r_possible |= (from.x == 7 && from.y == 0 || from.x == 4 && from.y == 0);
		} else if (BLACK == m->before.active_color) {
			m->after.black_castle_l_possible |= (from.x == 0 && from.y == 7 || from.x == 4 && from.y == 7);
			m->after.black_castle_r_possible |= (from.x == 7 && from.y == 7 || from.x == 4 && from.y == 7);
		}

		dllist_insert_last(*moves, m);
		return true;
	}
	return false;
}

bool filter_check_own_check_after_move(move *m)
{
	ASSERT_ERROR (m, "Argument is NULL");
	// TODO
	return false;
}