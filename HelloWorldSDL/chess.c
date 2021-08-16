#include <stdlib.h>
#include <string.h>

#include "chess.h"
#include "log.h"

static dllist *unchecked_moves_starting_from(const chess_state *c, pos p, dllist *move);
static bool filter_check_own_check_after_move(move *m);
static move_target check_target_valid(const chess_state *c, pos to, move_target target_types);
static bool add_move_if_target_valid(const chess_state *c, pos from, pos to, dllist *moves, move_target target_types);
static move *clone_move(const move *m);
static dllist *create_movelist(void);
static void populate_moves_after_move(move *m);
static dllist *unchecked_to_actual_moves(dllist *moves);
static bool check_losing_move(const move *m);

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
			.can_castle = {{true, true}, {true, true}},
			.allowed_moves = {0},
			.can_en_pessant = {0},
			.board = {
				{(piece) { true, WHITE, ROOK }, (piece) { true, WHITE, KNIGHT }, (piece) { true, WHITE, BISHOP }, (piece) { true, WHITE, QUEEN }, (piece) { true, WHITE, KING }, (piece) { true, WHITE, BISHOP }, (piece) { true, WHITE, KNIGHT }, (piece) { true, WHITE, ROOK }},
				{(piece) { true, WHITE, PAWN }, (piece) { true, WHITE, PAWN }, (piece) { true, WHITE, PAWN }, (piece) { true, WHITE, PAWN }, (piece) { true, WHITE, PAWN }, (piece) { true, WHITE, PAWN }, (piece) { true, WHITE, PAWN }, (piece) { true, WHITE, PAWN }},
				{0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0},
				{(piece) { true, BLACK, PAWN }, (piece) { true, BLACK, PAWN }, (piece) { true, BLACK, PAWN }, (piece) { true, BLACK, PAWN }, (piece) { true, BLACK, PAWN }, (piece) { true, BLACK, PAWN }, (piece) { true, BLACK, PAWN }, (piece) { true, BLACK, PAWN }},
				{(piece) { true, BLACK, ROOK }, (piece) { true, BLACK, KNIGHT }, (piece) { true, BLACK, BISHOP }, (piece) { true, BLACK, QUEEN }, (piece) { true, BLACK, KING }, (piece) { true, BLACK, BISHOP }, (piece) { true, BLACK, KNIGHT }, (piece) { true, BLACK, ROOK }},
			}
		},
		.is_game_over = false
	};

	dllist_init(&initial_state.history, clone_move, free);

	ASSERT_ERROR (c, "Argument c was NULL");
	ASSERT_ERROR (memcpy(c, &initial_state, sizeof (chess)), "memcpy returned NULL");
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
	u64 move_num = 0;
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
						move_num += dllist_size(&c->current_state.allowed_moves[p.y][p.x]);
					}
				}
			}
			c->is_game_over = (move_num == 0);

			if (c->is_game_over) {
				// check if the other player has a check
				c->current_state.active_color = c->current_state.active_color;
				c->winner = c->current_state.active_color;
				c->is_draw = true;
				for (p.y = 0; p.y < BOARD_SIDE_LENGTH && c->is_draw; ++p.y) {
					for (p.x = 0; p.x < BOARD_SIDE_LENGTH && c->is_draw; ++p.x) {
						unchecked_moves_starting_from(&c->current_state, p, &c->current_state.allowed_moves[p.y][p.x]);
						if (dllist_exists(&c->current_state.allowed_moves[p.y][p.x], check_losing_move)) {
							c->is_draw = false;
						}
					}
				}
			}
			return true;
		}
		iter = iter->next;
	}
	return false;
}

const dllist *valid_moves_from(const chess *c, pos p)
{
	return &c->current_state.allowed_moves[p.y][p.x];
}

static dllist *unchecked_to_actual_moves(dllist *moves)
{
	dllist_apply(moves, &populate_moves_after_move);
	LOG_DEBUG ("list size before filter_check_own_check_after_move %hhu", dllist_size(moves));
	dllist_filter(moves, &filter_check_own_check_after_move);
	LOG_DEBUG ("list size after filter_check_own_check_after_move %hhu", dllist_size(moves));
	dllist_apply(moves, print_move);
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

static bool filter_check_own_check_after_move(move *m)
{
	pos attacker_pos;
	ASSERT_ERROR (m, "Argument is NULL");
	for (attacker_pos.y = 0; attacker_pos.y < BOARD_SIDE_LENGTH; ++attacker_pos.y) {
		for (attacker_pos.x = 0; attacker_pos.x < BOARD_SIDE_LENGTH; ++attacker_pos.x) {
			dllist_apply(&m->after.allowed_moves[attacker_pos.y][attacker_pos.x], print_move);
			if (dllist_exists(&m->after.allowed_moves[attacker_pos.y][attacker_pos.x], &check_losing_move)) {
				return false;
			}
		}
	}
	return true;
}

static dllist *unchecked_moves_starting_from(const chess_state *c, pos p, dllist *moves)
{
	i32 x_target, y_target;

	ASSERT_ERROR (c && (p.x < BOARD_SIDE_LENGTH) && (p.y < BOARD_SIDE_LENGTH), "Error invalid arguments");

	if (!c->board[p.y][p.x].is_piece || c->active_color != (c->board[p.y][p.x].c) || !c->board[p.y][p.x].is_piece) {
		return moves;
	}

	switch (c->board[p.y][p.x].t) {
	case PAWN: // TODO test
		ASSERT_ERROR (p.y > 0 && p.y < BOARD_SIDE_LENGTH - 1, "%s pawn on invalid rank %hhu", piece_color_string(c->active_color), p.y + 1);

		// check move one straight
		if (add_move_if_target_valid(c, p, (pos) { p.x, p.y + (c->active_color == WHITE ? 1 : (-1)) }, moves, TARGET_EMPTY)) {
			// check double moves from beginning rank
			if (p.y == 1 && c->active_color == WHITE || p.y == 6)
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

		add_move_if_target_valid(c, p, (pos) { p.x - 2, p.y }, moves, CASTLE_L);
		add_move_if_target_valid(c, p, (pos) { p.x + 2, p.y }, moves, CASTLE_R);
		break;
	}

	return moves;
}

static move_target check_target_valid(const chess_state *c, pos to, move_target target_types)
{
	piece_color color = c->active_color;

	if (!(0 <= to.x && to.x < BOARD_SIDE_LENGTH && 0 <= to.y && to.y < BOARD_SIDE_LENGTH))
		return TARGET_INVALID;

	if (target_types & TARGET_ENEMY
		&& c->board[to.y][to.x].is_piece
		&& c->board[to.y][to.x].c != color)
	{
		return TARGET_ENEMY;

	} else if (target_types & TARGET_ENEMY
		&& !c->board[to.y][to.x].is_piece
		&& to.y == ((color == WHITE) ? 5 : 2)
		&& c->can_en_pessant[to.x][color == WHITE ? BLACK : WHITE])
	{
		return TARGET_EN_PESSANT | TARGET_ENEMY;

	} else if (target_types & CASTLE_L
		&& c->can_castle[LEFT][color]
		&& c->board[(color == WHITE ? 0 : 7)][0].is_piece
		&& !c->board[(color == WHITE ? 0 : 7)][1].is_piece
		&& !c->board[(color == WHITE ? 0 : 7)][3].is_piece)
	{
		return CASTLE_L;

	} else if (target_types & CASTLE_R
		&& c->can_castle[RIGHT][color]
		&& c->board[(color == WHITE ? 0 : 7)][7].is_piece
		&& !c->board[(color == WHITE ? 0 : 7)][6].is_piece
		&& !c->board[(color == WHITE ? 0 : 7)][5].is_piece
		&& c->board[(color == WHITE ? 0 : 7)][4].is_piece)
	{
		return CASTLE_R;

	} else if (target_types & TARGET_EMPTY
			&& !c->board[to.y][to.x].is_piece)
	{
		return TARGET_EMPTY;
	} else {
		return TARGET_INVALID;
	}
}

static bool add_move_if_target_valid(const chess_state *c, pos from, pos to, dllist *moves, move_target target_types)
{
	move *m;
	u8 x;
	move_target move_type = check_target_valid(c, to, target_types);
	piece_color color = c->active_color;
	int tmp;

	if (move_type & target_types) {
		m = calloc(1, sizeof (move));
		ASSERT_ERROR (m, "calloc returned NULL!");
		memcpy(&m->before, c, sizeof (chess_state));
		memcpy(&m->after, c, sizeof (chess_state));

		if (target_types & CASTLE_L) {
			tmp = (WHITE == color) ? 0 : 7;
			m->after.board[tmp][0] = (piece) {.is_piece = false, .c = 0, .t = 0};
			m->after.board[tmp][1] = (piece) {.is_piece = false, .c = 0, .t = 0};
			m->after.board[tmp][2] = (piece) {.is_piece = true, .c = color, .t = KING};
			m->after.board[tmp][3] = (piece) {.is_piece = true, .c = color, .t = ROOK };
			m->after.board[tmp][4] = (piece) {.is_piece = false, .c = 0, .t = 0};
			m->after.can_castle[LEFT][color] = false;
			m->after.can_castle[RIGHT][color] = false;
		} else if (target_types & CASTLE_R) {
			tmp = (WHITE == color) ? 0 : 7;
			m->after.board[tmp][7] = (piece) {.is_piece = false, .c = 0, .t = 0};
			m->after.board[tmp][6] = (piece) {.is_piece = true, .c = color, .t = KING};
			m->after.board[tmp][5] = (piece) {.is_piece = true, .c = color, .t = ROOK};
			m->after.board[tmp][4] = (piece) {.is_piece = false, .c = 0, .t = 0};
			m->after.can_castle[LEFT][color] = false;
			m->after.can_castle[RIGHT][color] = false;
		} else {
			m->after.board[to.y][to.x] = m->before.board[from.y][from.x];
			m->after.board[from.y][from.x] = (piece) {.is_piece = false, .c = 0, .t = 0};
		}

		m->after.active_color = (color == WHITE) ? BLACK : WHITE;
		memcpy(&m->from, &from, sizeof (pos));
		memcpy(&m->to, &to, sizeof (pos));
		m->move_type = move_type;

		for (x = 0; x < BOARD_SIDE_LENGTH; ++x) {
			m->after.can_en_pessant[x][WHITE] = false;
			m->after.can_en_pessant[x][BLACK] = false;
		}

		if (2 == abs(from.y - to.y) && m->before.board[from.y][from.x].t == PAWN && m->before.board[from.y][from.x].is_piece) {
			m->after.can_en_pessant[to.x][color] = true;
		}

		if (move_type & TARGET_EN_PESSANT) {
			m->after.board[to.y + ((color == WHITE) ? -1 : 1)][to.x] = (piece) {.is_piece = false, .c = 0, .t = 0};
		}

		m->after.can_castle[LEFT][WHITE] &= !(from.x == 0 && from.y == 0 || from.x == 4 && from.y == 0 || to.x == 0 && to.y == 0 || to.x == 4 && to.y == 0);
		m->after.can_castle[LEFT][BLACK] &= !(from.x == 0 && from.y == 7 || from.x == 4 && from.y == 7 || to.x == 0 && to.y == 7 || to.x == 4 && to.y == 7);
		m->after.can_castle[RIGHT][WHITE] &= !(from.x == 7 && from.y == 7 || from.x == 4 && from.y == 7 || to.x == 7 && to.y == 7 || to.x == 4 && to.y == 7);
		m->after.can_castle[RIGHT][BLACK] &= !(from.x == 7 && from.y == 7 || from.x == 4 && from.y == 7 || to.x == 7 && to.y == 7 || to.x == 4 && to.y == 7);

		dllist_insert_head(moves, m);
		return true;
	}
	return false;
}

static bool check_losing_move(const move *m) {
	return m->before.board[m->to.y][m->to.x].c == m->after.active_color
		&& m->before.board[m->to.y][m->to.x].t == KING
		&& m->before.board[m->from.y][m->from.x].c != m->after.active_color;

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