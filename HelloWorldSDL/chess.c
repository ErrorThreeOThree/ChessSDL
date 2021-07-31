#include <stdlib.h>
#include <string.h>

#include "chess.h"
#include "log.h"

bool insert_valid_move(const chess *c, llist **valid_moves, pos from, pos to);
bool check_target_empty(const chess *c, pos to);
bool check_target_enemy(const chess *c, pos to);
bool check_for_check(const chess *c, bool is_check_white);

typedef enum {
	TARGET_EMPTY = 1 << 0,
	TARGET_ENEMY = 1 << 1,
	TARGET_ALLY = 1 << 2
} move_target;
bool add_move_if_valid(const chess *c, pos from, pos to, llist **valid_moves, move_target allowed_targets);

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

	c->is_whites_turn = true;
	c->white_king_moved = false;
	c->black_king_moved = false;

	ASSERT_ERROR (memcpy(c->board, &board_initial_state, sizeof (board_initial_state)), "Memcpy returned NULL");

	return c;
}

bool in_check_next_move(chess *c, pos from, pos to);

llist *get_moves_by_pos(const chess *c, pos p)
{
	llist *valid_moves = NULL;
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
		if (add_move_if_valid(c, p, (pos) { p.x, p.y + (c->is_whites_turn ? 1 : (-1)) }, &valid_moves, TARGET_EMPTY)) {
			// check double move from beginning rank
			add_move_if_valid(c, p, (pos) { p.x, (c->is_whites_turn ? 3 : 4) }, &valid_moves, TARGET_EMPTY);
		}

		// check attacks left and right
		add_move_if_valid(c, p, (pos) { p.x + 1, p.y + (c->is_whites_turn ? 1 : -1) }, &valid_moves, TARGET_EMPTY);
		add_move_if_valid(c, p, (pos) { p.x - 1, p.y + (c->is_whites_turn ? 1 : -1) }, &valid_moves, TARGET_EMPTY);

		break;

	case ROOK: // TODO testing
		for (y_target = p.y + 1; add_move_if_valid(c, p, (pos) { p.x, y_target }, &valid_moves, TARGET_EMPTY); y_target++);
		add_move_if_valid(c, p, (pos) { p.x, y_target }, &valid_moves, TARGET_ENEMY);

		for (y_target = p.y - 1; add_move_if_valid(c, p, (pos) { p.x, y_target }, &valid_moves, TARGET_EMPTY); y_target--);
		add_move_if_valid(c, p, (pos) { p.x, y_target }, &valid_moves, TARGET_ENEMY);

		for (x_target = p.x + 1; add_move_if_valid(c, p, (pos) { x_target, p.y }, &valid_moves, TARGET_EMPTY); x_target++);
		add_move_if_valid(c, p, (pos) { x_target, p.y }, &valid_moves, TARGET_ENEMY);

		for (x_target = p.x - 1; add_move_if_valid(c, p, (pos) { x_target, p.y }, &valid_moves, TARGET_EMPTY); x_target--);
		add_move_if_valid(c, p, (pos) { x_target, p.y }, &valid_moves, TARGET_ENEMY);

		break;

	case KNIGHT:
		add_move_if_valid(c, p, (pos) { p.x + 1, p.y + 2 }, &valid_moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_valid(c, p, (pos) { p.x - 1, p.y + 2 }, &valid_moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_valid(c, p, (pos) { p.x + 1, p.y - 2 }, &valid_moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_valid(c, p, (pos) { p.x - 1, p.y - 2 }, &valid_moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_valid(c, p, (pos) { p.x + 2, p.y + 1 }, &valid_moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_valid(c, p, (pos) { p.x - 2, p.y + 1 }, &valid_moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_valid(c, p, (pos) { p.x + 2, p.y - 1 }, &valid_moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_valid(c, p, (pos) { p.x - 2, p.y - 1 }, &valid_moves, TARGET_ENEMY | TARGET_EMPTY);
		

		break;

	case BISHOP: // TODO testing
		for (x_target = p.x + 1, y_target = p.y + 1; add_move_if_valid(c, p, (pos) { x_target, y_target }, &valid_moves, TARGET_EMPTY); x_target++, y_target++);
		add_move_if_valid(c, p, (pos) { x_target, y_target }, &valid_moves, TARGET_ENEMY);

		for (x_target = p.x + 1, y_target = p.y - 1; add_move_if_valid(c, p, (pos) { x_target, y_target }, &valid_moves, TARGET_EMPTY); x_target++, y_target--);
		add_move_if_valid(c, p, (pos) { x_target, y_target }, &valid_moves, TARGET_ENEMY);

		for (x_target = p.x - 1, y_target = p.y + 1; add_move_if_valid(c, p, (pos) { x_target, y_target }, &valid_moves, TARGET_EMPTY); x_target--, y_target++);
		add_move_if_valid(c, p, (pos) { x_target, y_target }, &valid_moves, TARGET_ENEMY);

		for (x_target = p.x - 1, y_target = p.y - 1; add_move_if_valid(c, p, (pos) { x_target, y_target }, &valid_moves, TARGET_EMPTY); x_target--, y_target--);
		add_move_if_valid(c, p, (pos) { x_target, y_target }, &valid_moves, TARGET_ENEMY);

		break;

	case QUEEN:
		for (x_target = p.x + 1, y_target = p.y + 1; add_move_if_valid(c, p, (pos) { x_target, y_target }, &valid_moves, TARGET_EMPTY); x_target++, y_target++);
		add_move_if_valid(c, p, (pos) { x_target, y_target }, &valid_moves, TARGET_ENEMY);

		for (x_target = p.x + 1, y_target = p.y - 1; add_move_if_valid(c, p, (pos) { x_target, y_target }, &valid_moves, TARGET_EMPTY); x_target++, y_target--);
		add_move_if_valid(c, p, (pos) { x_target, y_target }, &valid_moves, TARGET_ENEMY);

		for (x_target = p.x - 1, y_target = p.y + 1; add_move_if_valid(c, p, (pos) { x_target, y_target }, &valid_moves, TARGET_EMPTY); x_target--, y_target++);
		add_move_if_valid(c, p, (pos) { x_target, y_target }, &valid_moves, TARGET_ENEMY);

		for (x_target = p.x - 1, y_target = p.y - 1; add_move_if_valid(c, p, (pos) { x_target, y_target }, &valid_moves, TARGET_EMPTY); x_target--, y_target--);
		add_move_if_valid(c, p, (pos) { x_target, y_target }, &valid_moves, TARGET_ENEMY);

		for (y_target = p.y + 1; add_move_if_valid(c, p, (pos) { p.x, y_target }, &valid_moves, TARGET_EMPTY); y_target++);
		add_move_if_valid(c, p, (pos) { p.x, y_target }, &valid_moves, TARGET_ENEMY);

		for (y_target = p.y - 1; add_move_if_valid(c, p, (pos) { p.x, y_target }, &valid_moves, TARGET_EMPTY); y_target--);
		add_move_if_valid(c, p, (pos) { p.x, y_target }, &valid_moves, TARGET_ENEMY);

		for (x_target = p.x + 1; add_move_if_valid(c, p, (pos) { x_target, p.y }, &valid_moves, TARGET_EMPTY); x_target++);
		add_move_if_valid(c, p, (pos) { x_target, p.y }, &valid_moves, TARGET_ENEMY);

		for (x_target = p.x - 1; add_move_if_valid(c, p, (pos) { x_target, p.y }, &valid_moves, TARGET_EMPTY); x_target--);
		add_move_if_valid(c, p, (pos) { x_target, p.y }, &valid_moves, TARGET_ENEMY);
		break;
	case KING:
		add_move_if_valid(c, p, (pos) { p.x + 1, p.y }, &valid_moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_valid(c, p, (pos) { p.x - 1, p.y }, &valid_moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_valid(c, p, (pos) { p.x, p.y + 1 }, &valid_moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_valid(c, p, (pos) { p.x, p.y - 1 }, &valid_moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_valid(c, p, (pos) { p.x + 1, p.y + 1 }, &valid_moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_valid(c, p, (pos) { p.x - 1, p.y + 1 }, &valid_moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_valid(c, p, (pos) { p.x + 1, p.y - 1 }, &valid_moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_valid(c, p, (pos) { p.x - 1, p.y - 1 }, &valid_moves, TARGET_ENEMY | TARGET_EMPTY);
		break;
	}

	return valid_moves;
}


bool check_target_empty(const chess *c, pos to)
{
	ASSERT_ERROR (c, "Variable chess c is NULL!");
	return (0 <= to.x && to.x < BOARD_SIDE_LENGTH && 0 <= to.y && to.y < BOARD_SIDE_LENGTH)
		&& (0 == c->board[to.y][to.x]);
}

bool check_target_enemy(const chess *c, pos to)
{
	ASSERT_ERROR (c, "Variable chess c is NULL!");
	return (0 <= to.x && to.x < BOARD_SIDE_LENGTH && 0 <= to.y && to.y < BOARD_SIDE_LENGTH)
		&& c->is_whites_turn != (c->board[to.y][to.x] & WHITE);
}

bool add_move_if_valid(const chess *now, pos from, pos to, llist **valid_moves, move_target allowed_targets)
{
	chess *after;
	u8 moving_piece;
	if (((allowed_targets & TARGET_EMPTY) && check_target_empty(now, to))
		|| ((allowed_targets & TARGET_ENEMY) && check_target_enemy(now, to)))
	{
		moving_piece = now->board[from.y][from.x];
		after = calloc(1, sizeof (chess));
		ASSERT_ERROR (after, "calloc returned NULL");
		memcpy(after, now, sizeof (chess));
		after->board[from.y][from.x] = 0;
		after->board[to.y][to.x] = now->board[from.y][from.x];
		after->is_whites_turn = !now->is_whites_turn;

		if (check_for_check(after, now->is_whites_turn)) {
			goto EXIT_ERROR_CLEANUP;
		}

		after->is_check = check_for_check(after, after->is_whites_turn);

		if (now->is_whites_turn) {
			after->white_checked_before = now->white_checked_before;
			after->black_checked_before = now->black_checked_before || after->is_check;
		} else {
			after->black_checked_before = now->black_checked_before;
			after->white_checked_before = now->white_checked_before || after->is_check;
		}

		if (moving_piece & KING) {
			if (now->is_whites_turn) {
				after->white_king_moved = true;;
			} else {
				after->black_king_moved = true;
			}
		}

		if (moving_piece & ROOK) {
			if (from.x == 0 && from.y == 0) {
				after->white_rook_0_moved = true;
			} else if (from.x = 7 && from.y == 0) {
				after->white_rook_7_moved = true;
			} else if (from.x == 0 && from.y == 7) {
				after->black_rook_0_moved = true;
			} else if (from.x == 7 && from.y == 7) {
				after->black_rook_7_moved = true;
			}
		}

		llist_insert_last(*valid_moves, after);
		return true;
	}

	goto EXIT_ERROR;

	EXIT_ERROR_CLEANUP:
		free(after);
	EXIT_ERROR:
		return false;
}

bool check_for_check(const chess *c, bool is_check_white)
{
	llist *valid_moves, *tmp;
	u8 x, y;
	move *m;

	for (y = 0; y < BOARD_SIDE_LENGTH; ++y) {
		for (x = 0; x < BOARD_SIDE_LENGTH; ++x) {
			if (is_check_white == (WHITE & c->board[y][x]))
				continue;

			valid_moves = get_moves_by_pos(c, (pos) { x, y });

			if (!valid_moves)
				continue;

			tmp = valid_moves;

			while (tmp) {
				m = (move *) tmp->data;

				if (m->c.board[m->to.x][m->to.y] & KING && is_check_white == (m->c.board[m->to.x][m->to.y] & WHITE)) {
					llist_destroy(valid_moves, true);
					return true;
				}

				tmp = tmp->next;
			}
			llist_destroy(valid_moves, true);
		}
	}
	return false;
}