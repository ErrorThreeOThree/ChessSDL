#include <stdlib.h>
#include <string.h>

#include "chess.h"
#include "log.h"
#include <stdio.h>

typedef enum {
	TARGET_EMPTY = 1 << 0,
	TARGET_ENEMY = 1 << 1,
	TARGET_ALLY = 1 << 2
} move_target;

static dllist *unchecked_moves_starting_from(const chess_state *c, pos p, dllist *move);
static bool filter_check_own_check_after_move(move *m);
static bool check_target_valid(const chess_state *c, pos to, move_target target_types);
static bool add_move_if_target_valid(const chess_state *c, pos from, pos to, dllist *moves, move_target target_types);
static bool check_target_valid(const chess_state *c, pos to, move_target target_types);
static move *clone_move(const move *m);
static dllist *create_movelist(void);
static void populate_moves_after_move(move *m);
static dllist *unchecked_to_actual_moves(dllist *moves);

void print_move(const move *m)
{
	LOG_INFO("Move from (%hhu,%hhu) to (%hhu,%hhu)", m->from.x, m->from.y, m->to.x, m->to.y);
}

chess *init_chess(chess *c) {
	pos p;
	chess initial_state = {
		.current_state = (chess_state)
		{
			.active_color = WHITE,
			.white_castle_l_possible = true,
			.white_castle_r_possible = true,
			.black_castle_l_possible = true,
			.black_castle_r_possible = true,
			.allowed_moves = {0},
			.board = {
				{(piece) { WHITE, ROOK }, (piece) { WHITE, KNIGHT }, (piece) { WHITE, BISHOP }, (piece) { WHITE, QUEEN }, (piece) { WHITE, KING }, (piece) { WHITE, BISHOP }, (piece) { WHITE, KNIGHT }, (piece) { WHITE, ROOK }},
				{(piece) { WHITE, PAWN }, (piece) { WHITE, PAWN }, (piece) { WHITE, PAWN }, (piece) { WHITE, PAWN }, (piece) { WHITE, PAWN }, (piece) { WHITE, PAWN }, (piece) { WHITE, PAWN }, (piece) { WHITE, PAWN }},
				{0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0},
				{(piece) { BLACK, PAWN }, (piece) { BLACK, PAWN }, (piece) { BLACK, PAWN }, (piece) { BLACK, PAWN }, (piece) { BLACK, PAWN }, (piece) { BLACK, PAWN }, (piece) { BLACK, PAWN }, (piece) { BLACK, PAWN }},
				{(piece) { BLACK, ROOK }, (piece) { BLACK, KNIGHT }, (piece) { BLACK, BISHOP }, (piece) { BLACK, QUEEN }, (piece) { BLACK, KING }, (piece) { BLACK, BISHOP }, (piece) { BLACK, KNIGHT }, (piece) { BLACK, ROOK }},
			}
		},
	};

	dllist_init(&initial_state.history, clone_move, free);

	ASSERT_ERROR (c, "Argument c was NULL");
	ASSERT_ERROR (memcpy(c, &initial_state, sizeof (chess_state)), "memcpy returned NULL");
	for (p.y = 0; p.y < BOARD_SIDE_LENGTH; ++p.y) {
		for (p.x = 0; p.x < BOARD_SIDE_LENGTH; ++p.x) {
			dllist_init(&c->current_state.allowed_moves[p.y][p.x], clone_move, free);
		}
	}

	for (p.y = 0; p.y < BOARD_SIDE_LENGTH; ++p.y) {
		for (p.x = 0; p.x < BOARD_SIDE_LENGTH; ++p.x) {
			unchecked_moves_starting_from(&c->current_state, p, &c->current_state.allowed_moves[p.y][p.x]);

			unchecked_to_actual_moves(&c->current_state.allowed_moves[p.y][p.x]);
		}
	}

	return c;
}

bool try_move(chess *c, pos from, pos to)
{
	// find if move exists
	dllist_elem *iter = c->current_state.allowed_moves[from.y][from.x].head;
	move *m, *m_copy;
	pos p;
	while (iter)
	{
		m = (move *) iter->data;
		if (m->to.x == to.x && m->to.y == to.y) {
			// Add move to history
			m_copy = malloc(sizeof (move));
			ASSERT_ERROR (m_copy, "calloc returned NULL!");
			ASSERT_ERROR (memcpy(m_copy, m, sizeof (move)), "memcpy returned NULL!");
			dllist_insert_head(&c->history, m_copy);

			// set after state as current state
			ASSERT_ERROR (memcpy(&c->current_state, &m->after, sizeof (chess_state)), "memcpy returned NULL!");
			for (p.y = 0; p.y < BOARD_SIDE_LENGTH; ++p.y) {
				for (p.x = 0; p.x < BOARD_SIDE_LENGTH; ++p.x) {
					if (dllist_size(&c->current_state.allowed_moves[p.y][p.x]) > 0) {
						unchecked_to_actual_moves(&c->current_state.allowed_moves[p.y][p.x]);
					}
				}
			}
			return true;
		}
		iter = iter->next;
	}
	return false;
}

dllist *valid_moves_from(const chess *c, pos p)
{
	return dllist_duplicate(&c->current_state.allowed_moves[p.y][p.x]);
}

static dllist *unchecked_to_actual_moves(dllist *moves)
{
	dllist_apply(moves, &populate_moves_after_move);
	LOG_DEBUG ("list size before filter_check_own_check_after_move %hhu", dllist_size(moves));
	dllist_filter(moves, &filter_check_own_check_after_move);
	LOG_DEBUG ("list size after filter_check_own_check_after_move %hhu", dllist_size(moves));
	return moves;

}

static void populate_moves_after_move(move *m)
{
	pos p;
	for (p.y = 0; p.y < BOARD_SIDE_LENGTH; ++p.y) {
		for (p.x = 0; p.x < BOARD_SIDE_LENGTH; ++p.x) {
			dllist_init(&m->after.allowed_moves[p.y][p.x], clone_move, free);
			unchecked_moves_starting_from(&m->after, p, &m->after.allowed_moves[p.y][p.x]);
		}
	}

}

static dllist *unchecked_moves_starting_from(const chess_state *c, pos p, dllist *moves)
{
	u8 x_target, y_target;

	ASSERT_ERROR (c && (p.x < BOARD_SIDE_LENGTH) && (p.y < BOARD_SIDE_LENGTH), "Error invalid arguments");

	if (c->active_color != (c->board[p.y][p.x].c)) {
		return moves;
	}

	switch (c->board[p.y][p.x].t) {
	case PAWN: // TODO test
		ASSERT_ERROR (p.y > 0 && p.y < BOARD_SIDE_LENGTH - 1, "%s pawn on invalid rank %hhu", piece_color_string(c->active_color), p.y + 1);

		// check move one straight
		if (add_move_if_target_valid(c, p, (pos) { p.x, p.y + (c->active_color == WHITE ? 1 : (-1)) }, moves, TARGET_EMPTY)) {
			// check double moves from beginning rank
			add_move_if_target_valid(c, p, (pos) { p.x, (c->active_color == WHITE ? 3 : 4) }, moves, TARGET_EMPTY);
		}

		// check attacks left and right
		add_move_if_target_valid(c, p, (pos) { p.x + 1, p.y + (c->active_color == WHITE ? 1 : -1) }, moves, TARGET_ENEMY);
		add_move_if_target_valid(c, p, (pos) { p.x - 1, p.y + (c->active_color == WHITE ? 1 : -1) }, moves, TARGET_ENEMY);

		break;

	case ROOK:
		for (y_target = p.y + 1; add_move_if_target_valid(c, p, (pos) { p.x, y_target }, moves, TARGET_EMPTY); y_target++);
		add_move_if_target_valid(c, p, (pos) { p.x, y_target }, moves, TARGET_ENEMY);

		for (y_target = p.y - 1; add_move_if_target_valid(c, p, (pos) { p.x, y_target }, moves, TARGET_EMPTY); y_target--);
		add_move_if_target_valid(c, p, (pos) { p.x, y_target }, moves, TARGET_ENEMY);

		for (x_target = p.x + 1; add_move_if_target_valid(c, p, (pos) { x_target, p.y }, moves, TARGET_EMPTY); x_target++);
		add_move_if_target_valid(c, p, (pos) { x_target, p.y }, moves, TARGET_ENEMY);

		for (x_target = p.x - 1; add_move_if_target_valid(c, p, (pos) { x_target, p.y }, moves, TARGET_EMPTY); x_target--);
		add_move_if_target_valid(c, p, (pos) { x_target, p.y }, moves, TARGET_ENEMY);

		break;

	case KNIGHT:
		add_move_if_target_valid(c, p, (pos) { p.x + 1, p.y + 2 }, moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_target_valid(c, p, (pos) { p.x - 1, p.y + 2 }, moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_target_valid(c, p, (pos) { p.x + 1, p.y - 2 }, moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_target_valid(c, p, (pos) { p.x - 1, p.y - 2 }, moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_target_valid(c, p, (pos) { p.x + 2, p.y + 1 }, moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_target_valid(c, p, (pos) { p.x - 2, p.y + 1 }, moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_target_valid(c, p, (pos) { p.x + 2, p.y - 1 }, moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_target_valid(c, p, (pos) { p.x - 2, p.y - 1 }, moves, TARGET_ENEMY | TARGET_EMPTY);
		

		break;

	case BISHOP:
		for (x_target = p.x + 1, y_target = p.y + 1; add_move_if_target_valid(c, p, (pos) { x_target, y_target }, moves, TARGET_EMPTY); x_target++, y_target++);
		add_move_if_target_valid(c, p, (pos) { x_target, y_target }, moves, TARGET_ENEMY);

		for (x_target = p.x + 1, y_target = p.y - 1; add_move_if_target_valid(c, p, (pos) { x_target, y_target }, moves, TARGET_EMPTY); x_target++, y_target--);
		add_move_if_target_valid(c, p, (pos) { x_target, y_target }, moves, TARGET_ENEMY);

		for (x_target = p.x - 1, y_target = p.y + 1; add_move_if_target_valid(c, p, (pos) { x_target, y_target }, moves, TARGET_EMPTY); x_target--, y_target++);
		add_move_if_target_valid(c, p, (pos) { x_target, y_target }, moves, TARGET_ENEMY);

		for (x_target = p.x - 1, y_target = p.y - 1; add_move_if_target_valid(c, p, (pos) { x_target, y_target }, moves, TARGET_EMPTY); x_target--, y_target--);
		add_move_if_target_valid(c, p, (pos) { x_target, y_target }, moves, TARGET_ENEMY);

		break;

	case QUEEN:
		for (x_target = p.x + 1, y_target = p.y + 1; add_move_if_target_valid(c, p, (pos) { x_target, y_target }, moves, TARGET_EMPTY); x_target++, y_target++);
		add_move_if_target_valid(c, p, (pos) { x_target, y_target }, moves, TARGET_ENEMY);

		for (x_target = p.x + 1, y_target = p.y - 1; add_move_if_target_valid(c, p, (pos) { x_target, y_target }, moves, TARGET_EMPTY); x_target++, y_target--);
		add_move_if_target_valid(c, p, (pos) { x_target, y_target }, moves, TARGET_ENEMY);

		for (x_target = p.x - 1, y_target = p.y + 1; add_move_if_target_valid(c, p, (pos) { x_target, y_target }, moves, TARGET_EMPTY); x_target--, y_target++);
		add_move_if_target_valid(c, p, (pos) { x_target, y_target }, moves, TARGET_ENEMY);

		for (x_target = p.x - 1, y_target = p.y - 1; add_move_if_target_valid(c, p, (pos) { x_target, y_target }, moves, TARGET_EMPTY); x_target--, y_target--);
		add_move_if_target_valid(c, p, (pos) { x_target, y_target }, moves, TARGET_ENEMY);

		for (y_target = p.y + 1; add_move_if_target_valid(c, p, (pos) { p.x, y_target }, moves, TARGET_EMPTY); y_target++);
		add_move_if_target_valid(c, p, (pos) { p.x, y_target }, moves, TARGET_ENEMY);

		for (y_target = p.y - 1; add_move_if_target_valid(c, p, (pos) { p.x, y_target }, moves, TARGET_EMPTY); y_target--);
		add_move_if_target_valid(c, p, (pos) { p.x, y_target }, moves, TARGET_ENEMY);

		for (x_target = p.x + 1; add_move_if_target_valid(c, p, (pos) { x_target, p.y }, moves, TARGET_EMPTY); x_target++);
		add_move_if_target_valid(c, p, (pos) { x_target, p.y }, moves, TARGET_ENEMY);

		for (x_target = p.x - 1; add_move_if_target_valid(c, p, (pos) { x_target, p.y }, moves, TARGET_EMPTY); x_target--);
		add_move_if_target_valid(c, p, (pos) { x_target, p.y }, moves, TARGET_ENEMY);

		break;

	case KING:
		add_move_if_target_valid(c, p, (pos) { p.x + 1, p.y }, moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_target_valid(c, p, (pos) { p.x - 1, p.y }, moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_target_valid(c, p, (pos) { p.x, p.y + 1 }, moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_target_valid(c, p, (pos) { p.x, p.y - 1 }, moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_target_valid(c, p, (pos) { p.x + 1, p.y + 1 }, moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_target_valid(c, p, (pos) { p.x - 1, p.y + 1 }, moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_target_valid(c, p, (pos) { p.x + 1, p.y - 1 }, moves, TARGET_ENEMY | TARGET_EMPTY);
		add_move_if_target_valid(c, p, (pos) { p.x - 1, p.y - 1 }, moves, TARGET_ENEMY | TARGET_EMPTY);
		break;
	}

	return moves;
}

static bool check_target_valid(const chess_state *c, pos to, move_target target_types)
{
	return (0 <= to.x && to.x < BOARD_SIDE_LENGTH && 0 <= to.y && to.y < BOARD_SIDE_LENGTH)
		&& ((target_types & TARGET_EMPTY)	&& NONE == c->board[to.y][to.x].c
			|| ((target_types & TARGET_ENEMY)	&& NONE != c->board[to.y][to.x].c && c->active_color != c->board[to.y][to.x].c));
}

static bool add_move_if_target_valid(const chess_state *c, pos from, pos to, dllist *moves, move_target target_types)
{
	move *m;
	dllist *tmp = create_movelist();

	if (check_target_valid(c, to, target_types)) {
		m = calloc(1, sizeof (move));
		ASSERT_ERROR (m, "calloc returned NULL!");
		memcpy(&m->before, c, sizeof (chess_state));
		memcpy(&m->after, c, sizeof (chess_state));
		m->after.board[to.y][to.x] = m->before.board[from.y][from.x];
		m->after.board[from.y][from.x].c = NONE;
		m->after.board[from.y][from.x].t = EMPTY;
		m->after.active_color = (m->before.active_color == WHITE) ? BLACK : WHITE;
		memcpy(&m->from, &from, sizeof (pos));
		memcpy(&m->to, &to, sizeof (pos));

		if (WHITE == m->before.active_color) {
			m->after.white_castle_l_possible |= (from.x == 0 && from.y == 0 || from.x == 4 && from.y == 0);
			m->after.white_castle_r_possible |= (from.x == 7 && from.y == 0 || from.x == 4 && from.y == 0);
		} else if (BLACK == m->before.active_color) {
			m->after.black_castle_l_possible |= (from.x == 0 && from.y == 7 || from.x == 4 && from.y == 7);
			m->after.black_castle_r_possible |= (from.x == 7 && from.y == 7 || from.x == 4 && from.y == 7);
		}

		dllist_insert_head(moves, m);
		free(tmp);
		return true;
	}
	return false;
}

static bool check_winning_move(const move *m)
{
	return m->after.board[m->to.y][m->to.x].c == m->after.active_color && m->after.board[m->to.y][m->to.x].t == KING;
}

static bool filter_check_own_check_after_move(move *m)
{
	pos attacker_pos;
	ASSERT_ERROR (m, "Argument is NULL");
	for (attacker_pos.y = 0; attacker_pos.y < BOARD_SIDE_LENGTH; ++attacker_pos.y) {
		for (attacker_pos.x = 0; attacker_pos.x < BOARD_SIDE_LENGTH; ++attacker_pos.x) {
			if (dllist_exists(&m->after.allowed_moves[attacker_pos.y][attacker_pos.x], &check_winning_move)) {
				return false;
			}
		}
	}
	return true;
}

static move *clone_move(const move *m)
{
	move *m_clone;
	ASSERT_ERROR (m, "Argument m is NULL");
	m_clone = malloc(sizeof (move));
	ASSERT_ERROR (m_clone, "malloc returned NULL!");
	ASSERT_ERROR (memcpy(m_clone, m, sizeof (move)), "memcpy returned NULL");
	return m_clone;
}

static dllist *create_movelist(void)
{
	return dllist_init(malloc(sizeof (dllist)), clone_move, free);
}