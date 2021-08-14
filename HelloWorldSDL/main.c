#include <stdio.h>
#include <stdlib.h>

#include "SDL.h"
#include "SDL_image.h"
#include "chess.h"
#include "log.h"
#include "utils.h"

#define BOARD_VERTICAL_OFFSET ((700 - 512) / 2)

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
	r.x = m->to.x * 64;
	r.y = m->to.y * 64 + BOARD_VERTICAL_OFFSET;
	r.w = 64;
	r.h = 64;
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
			r.x = x * 64 + 32 - piece_textures[p->c][p->t].x_center_offset / 4;
			r.y = y * 64 + BOARD_VERTICAL_OFFSET + 32 - piece_textures[p->c][p->t].y_center_offset / 4;
			ASSERT_ERROR (!SDL_RenderCopy(renderer, piece_textures[p->c][p->t].t, NULL, &r), "SDL_RendererCopy failed: %s", SDL_GetError());
		}
	}


	if (is_active_field && c->current_state.board[active_field.y][active_field.x].is_piece) {
		r.x = active_field.x * 64;
		r.y = active_field.y * 64 + BOARD_VERTICAL_OFFSET;
		r.w = 64;
		r.h = 64;
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
	SDL_PumpEvents();

	if (SDL_GetMouseState(&x, &y) & SDL_BUTTON_LMASK) {\
		if (!is_active_field) {
			LOG_INFO ("Got mouse click at %d %d", x, y);
			is_active_field = x >= 0 && x < 64 * 8 && y >= BOARD_VERTICAL_OFFSET && y < 64 * 8 + BOARD_VERTICAL_OFFSET;
			if (is_active_field) {
				active_field.x = x / 64;
				active_field.y = (y - BOARD_VERTICAL_OFFSET) / 64;
			}

		} else {
			is_move_input = x >= 0 && x < 64 * 8 && y >= BOARD_VERTICAL_OFFSET && y < 64 * 8 + BOARD_VERTICAL_OFFSET;
			if (is_move_input) {
				move_input.x = x / 64;
				move_input.y = (y - BOARD_VERTICAL_OFFSET) / 64;
				if (!try_move(c, active_field, move_input)) {

					is_active_field = x >= 0 && x < 64 * 8 && y >= BOARD_VERTICAL_OFFSET && y < 64 * 8 + BOARD_VERTICAL_OFFSET;
					if (is_active_field) {
						active_field.x = x / 64;
						active_field.y = (y - BOARD_VERTICAL_OFFSET) / 64;
					}
				}
			} else {
				is_active_field = x >= 0 && x < 64 * 8 && y >= BOARD_VERTICAL_OFFSET && y < 64 * 8 + BOARD_VERTICAL_OFFSET;
				if (is_active_field) {
					active_field.x = x / 64;
					active_field.y = (y - BOARD_VERTICAL_OFFSET) / 64;
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