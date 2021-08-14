#include <stdio.h>
#include <stdlib.h>

#include "SDL.h"
#include "SDL_image.h"
#include "chess.h"
#include "log.h"
#include "utils.h"

#define TEXTURE_SIZE 64
#define BOARD_SIZE ((BOARD_SIDE_LENGTH) * TEXTURE_SIZE)
#define BOARD_VERTICAL_OFFSET ((700 - BOARD_SIZE) / 2)

typedef struct {
	SDL_Texture *t;
	int x_center_offset;
	int y_center_offset;
	int w;
	int h;
} piece_texture;

SDL_Window *window;
SDL_Renderer *renderer;
piece_texture piece_textures[COLOR_MAX][PIECE_TYPE_MAX];
SDL_Texture *board_texture;
SDL_Texture *highlight_texture;
pos active_field, move_input;
bool is_active_field, is_move_input;

#define STRING_MAX 256

void screen_pos_to_board_index(i32 x, i32 y, i32 *x_out, i32 *y_out)
{
	*x_out = x / TEXTURE_SIZE;
	*y_out = (- y + BOARD_VERTICAL_OFFSET + BOARD_SIZE) / TEXTURE_SIZE;
}

void board_index_to_screen_pos(i32 x, i32 y, i32* x_out, i32* y_out)
{
	*x_out = x * TEXTURE_SIZE;
	*y_out = BOARD_SIZE - (y + 1) * TEXTURE_SIZE + BOARD_VERTICAL_OFFSET;
}

void init_game(chess *c)
{
	piece_color player;
	piece_type piece_num;
	SDL_Surface *surface;
	char path[256];
	ASSERT_ERROR (!SDL_Init(SDL_INIT_EVERYTHING), "SDL_Init failed: %s", SDL_GetError());
	ASSERT_ERROR (!SDL_CreateWindowAndRenderer(512, 700, 0, &window, &renderer), "SDL_Init failed: %s", SDL_GetError());
	init_chess(c);

	for (player = 0; player < COLOR_MAX; ++player) {
		for (piece_num = 0; piece_num < PIECE_TYPE_MAX; ++piece_num) {
			snprintf(path, STRING_MAX, "Sprites/%c_%s_png_shadow_256px.png", player == WHITE ? 'w' : 'b', piece_type_string(piece_num));
			surface = IMG_Load(path);
			ASSERT_ERROR(surface, "IMG_Load failed: %s", IMG_GetError());

			piece_textures[player][piece_num].t = SDL_CreateTextureFromSurface(renderer, surface);

			ASSERT_ERROR(piece_textures[player][piece_num].t, "SDL_CreateTextureFromSurface failed: %s", SDL_GetError());
			piece_textures[player][piece_num].x_center_offset = surface->w / 2;
			piece_textures[player][piece_num].y_center_offset = surface->h / 2;
			piece_textures[player][piece_num].w = surface->w;
			piece_textures[player][piece_num].h = surface->h;

			SDL_FreeSurface(surface);
		}
	}

	surface = IMG_Load("Sprites/board.png");
	ASSERT_ERROR(surface, "IMG_Load failed: %s", SDL_GetError());

	board_texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
	surface = IMG_Load("Sprites/highlight_square.png");
	highlight_texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);

	SDL_SetRenderDrawColor(renderer, 81, 42, 42, 255);
}

void show_move_option(const move *m)
{
	SDL_Rect r;
	board_index_to_screen_pos(m->to.x, m->to.y, &r.x, &r.y);
	r.w = TEXTURE_SIZE;
	r.h = TEXTURE_SIZE;
	ASSERT_ERROR (!SDL_RenderCopy(renderer, highlight_texture, NULL, &r), "SDL_RendererCopy failed: %s", SDL_GetError());
}

void show_game(const chess *c)
{
	u8 x, y;
	SDL_Rect r;
	const piece *p;

	SDL_RenderClear(renderer);

	r.x = 0;
	r.y = 0;
	ASSERT_ERROR (!SDL_RenderCopy(renderer, board_texture, NULL, NULL), "SDL_RendererCopy failed: %s", SDL_GetError());


	for (x = 0; x < BOARD_SIDE_LENGTH; ++x) {
		for (y = 0; y < BOARD_SIDE_LENGTH; ++y) {
			p = &c->current_state.board[y][x];
			if (!p->is_piece)
				continue;

			r.w = piece_textures[p->c][p->t].w / 4;
			r.h = piece_textures[p->c][p->t].h / 4;
			board_index_to_screen_pos(x, y, &r.x, &r.y);
			r.x += (TEXTURE_SIZE / 2) - piece_textures[p->c][p->t].x_center_offset / 4;
			r.y += (TEXTURE_SIZE / 2) - piece_textures[p->c][p->t].y_center_offset / 4;
			ASSERT_ERROR (!SDL_RenderCopy(renderer, piece_textures[p->c][p->t].t, NULL, &r), "SDL_RendererCopy failed: %s", SDL_GetError());
		}
	}


	if (is_active_field && c->current_state.board[active_field.y][active_field.x].is_piece) {
		board_index_to_screen_pos(active_field.x, active_field.y, &r.x, &r.y);
		r.w = TEXTURE_SIZE;
		r.h = TEXTURE_SIZE;
		ASSERT_ERROR (!SDL_RenderCopy(renderer, highlight_texture, NULL, &r), "SDL_RendererCopy failed: %s", SDL_GetError());

		const dllist *moves = valid_moves_from(c, active_field);
		dllist_apply((dllist *) moves, show_move_option);
	} else {
		is_active_field = false;
	}
	

	SDL_RenderPresent(renderer);
}

void process_input(chess *c) {
	int x, y;
	int x_board, y_board;
	SDL_PumpEvents();

	if (SDL_GetMouseState(&x, &y) & SDL_BUTTON_LMASK) {
		screen_pos_to_board_index(x, y, &x_board, &y_board);
		if (!(0 <= x_board && x_board < BOARD_SIDE_LENGTH && 0 <= y_board && y_board < BOARD_SIDE_LENGTH))
			is_active_field = false;
		else {
			if (!is_active_field || (x_board == active_field.x && y_board == active_field.y)) {
				LOG_INFO("Got mouse click at %d %d", x, y);
				is_active_field = true;
				active_field.x = x_board;
				active_field.y = y_board;
			} else {
				is_move_input = true;
				move_input.x = x_board;
				move_input.y = y_board;
				if (!try_move(c, active_field, move_input)) {

					is_active_field = x_board <= 0 && x_board < BOARD_SIDE_LENGTH&& y_board <= 0 && y_board < BOARD_SIDE_LENGTH;
					if (is_active_field) {
						active_field.x = x_board;
						active_field.y = y_board;
					}
				}
			}

		}

	}
}

int main(int argc, char **argv)
{
	chess c;
	LOG_INFO ("Starting program");
	LOG_DEBUG ("Got arguments:");

	
	ASSERT_ERROR (!SDL_Init(SDL_INIT_EVERYTHING), "SDL_Init failed: %s", SDL_GetError());
	init_game(&c);

	while (true) {
		show_game(&c);

		process_input(&c);
		SDL_Delay(10);

	}


	return EXIT_SUCCESS;
}