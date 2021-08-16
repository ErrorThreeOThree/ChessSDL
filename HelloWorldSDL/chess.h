#ifndef CHESS_H
#define CHESS_H

#include "log.h"
#include "types.h"
#include "utils.h"

#define BOARD_SIDE_LENGTH 8

typedef struct {
	i32 x; /* horizontal position */
	i32 y; /* vertical position */
} pos;

typedef enum {
	WHITE,
	BLACK,
	COLOR_MAX
} piece_color;

/// <summary>
/// piece_color enum item to string. Don't the returned memory!
/// </summary>
/// <param name="c">color</param>
/// <returns>Pointer to enum item string</returns>
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

/// <summary>
/// piece_type enum item to string. Don't the returned memory!
/// </summary>
/// <param name="t">type</param>
/// <returns>Pointer to enum item string</returns>
static inline const char *piece_type_string(piece_type t)
{
	static const char *s[] = { "pawn", "rook", "knight", "bishop", "queen", "king", "invalid" };
	ASSERT_WARNING (PIECE_TYPE_MAX != t, "Invalid value PIECE_TYPE_MAX");
	return s[t];
}

/// <summary>
/// chess piece struct
/// </summary>
typedef struct {
	bool is_piece; /* true if this is an actual piece, false if the field is empty */
	piece_color c; /* color of the piece if an actual piece, else undefined */
	piece_type t; /* type of the piece if an actual piece, else undefined */
} piece;

typedef enum {
	LEFT,
	RIGHT,
	DIRECTION_MAX
} direction;

/// <summary>
/// game state struct
/// </summary>
typedef struct {
	piece_color active_color; /* player who is next */
	bool can_castle[DIRECTION_MAX][COLOR_MAX]; /* keeps track who can castle on which side */
	piece board[BOARD_SIDE_LENGTH][BOARD_SIDE_LENGTH] /* current board state */;
	dllist allowed_moves[BOARD_SIDE_LENGTH][BOARD_SIDE_LENGTH]; /* list of allowed moves from each board position */
	bool can_en_pessant[BOARD_SIDE_LENGTH][COLOR_MAX]; /* keeps track which pawn can be en pessanted at the moment */
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

/// <summary>
/// move structure
/// </summary>
typedef struct {
	chess_state before; /* game state before move */
	chess_state after; /* game state after move */
	pos from; /* to be moved piece starting position */
	pos to; /* moved piece position after move */
	move_target move_type; /* move_target bitmap with containing the type of move*/
} move;

/// <summary>
/// chess game structe 
/// </summary>
typedef struct {
	dllist history; /* list of played moves */
	chess_state current_state; /* current game state */
	bool is_game_over; /* true if game is over */
	bool is_draw; /* true if game ended in draw */
	piece_color winner; /* contains the winning color if is_draw is false */
} chess;

/// <summary>
/// Initializes chess struct with chess opening state
/// </summary>
/// <param name="c">chess structure to be inintialized</param>
/// <returns></returns>
chess * init_chess(chess *c);

/// <summary>
/// Get all valid moves from the active color from the specified position.
/// </summary>
/// <param name="c">chess struct with current game state</param>
/// <param name="p">starting pos of moves</param>
/// <returns>list of moves from pos p</returns>
const dllist * valid_moves_from(const chess *c, pos p);

/// <summary>
/// Check if move if valid and if so, do the move
/// </summary>
/// <param name="c">chess struct with current game state</param>
/// <param name="from">starting pos of moves</param>
/// <param name="to">starting pos of moves</param>
/// <returns>true if move is valid and applied, else false</returns>
bool try_move(chess *c, pos from, pos to);


/// <summary>
/// Print move to log file and console
/// </summary>
/// <param name="m">move to be printed</param>
void print_move(const move *m);

#endif