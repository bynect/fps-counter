#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define eprintf(...) fprintf(stderr, __VA_ARGS__)
#define check_null(k, ...) do { if ((__VA_ARGS__) == NULL) { eprintf("Error in %s: %s\n", # __VA_ARGS__, k ## _GetError()); assert(false); } } while (false)
#define check_zero(k, ...) do { if ((__VA_ARGS__) < 0) { eprintf("Error in %s: %s\n", # __VA_ARGS__, k ## _GetError()); assert(false); } } while (false)

#define SCREEN_WIDTH 800.0
#define SCREEN_HEIGHT 600.0

#define FONT_WIDTH 256
#define FONT_HEIGHT 128
#define FONT_COLS 18
#define FONT_ROWS 7
#define FONT_CHAR_WIDTH  (FONT_WIDTH / FONT_COLS)
#define FONT_CHAR_HEIGHT (FONT_HEIGHT / FONT_ROWS)
#define FONT_SCALE 1.8

// Define FPS_CAP for capping fps to FPS
//#define FPS_CAP
#ifndef FPS
#define FPS 60
#endif

#define COLOR_BACK 24, 24, 24, 255
#define COLOR_TEXT 255, 255, 255, 255
#define COLOR_THING 255, 209, 220, 255

typedef struct {
	float x, y;
} Vec2;

#define ASCII_LOW 32
#define ASCII_HIGH 126

typedef struct {
    SDL_Texture *texture;
    SDL_Rect glyphs[ASCII_HIGH - ASCII_LOW + 1];
} Font;

Font load_font(SDL_Renderer *renderer, const char *path)
{
    Font font = {0};

    SDL_Surface *font_surface = IMG_Load(path);
	check_null(IMG, font_surface);

    font.texture = SDL_CreateTextureFromSurface(renderer, font_surface);
	check_null(SDL, font.texture);

    SDL_FreeSurface(font_surface);

    for (size_t ascii = ASCII_LOW; ascii <= ASCII_HIGH; ++ascii) {
        const size_t index = ascii - ASCII_LOW;
        const size_t col = index % FONT_COLS;
        const size_t row = index / FONT_COLS;
        font.glyphs[index] = (SDL_Rect) {
            .x = col * FONT_CHAR_WIDTH,
            .y = row * FONT_CHAR_HEIGHT,
            .w = FONT_CHAR_WIDTH,
            .h = FONT_CHAR_HEIGHT,
        };
    }

    return font;
}

void render_char(SDL_Renderer *renderer, Font *font, char c, float x, float y, float scale)
{
    const SDL_Rect dst = {
        .x = (int) floorf(x),
        .y = (int) floorf(y),
        .w = (int) floorf(FONT_CHAR_WIDTH * scale),
        .h = (int) floorf(FONT_CHAR_HEIGHT * scale),
    };

    assert(c >= ASCII_LOW && c <= ASCII_HIGH);
    const size_t index = c - ASCII_LOW;
    SDL_RenderCopy(renderer, font->texture, &font->glyphs[index], &dst);
}

void render_text(SDL_Renderer *renderer, Font *font, const char *text, SDL_Color color, float x, float y, float scale)
{
    check_zero(SDL, SDL_SetTextureColorMod(font->texture, color.r, color.g, color.b));
    check_zero(SDL, SDL_SetTextureAlphaMod(font->texture, color.a));

	size_t size = strlen(text);
    for (size_t i = 0; i < size; ++i) {
        render_char(renderer, font, text[i], x, y, scale);
        x += FONT_CHAR_WIDTH * scale;
    }
}

float thing_x = SCREEN_WIDTH / 2;
float thing_y = SCREEN_HEIGHT / 2;
float thing_dx = 0.4;
float thing_dy = 0.4;
int thing_size = 40;

bool running = true;
char fps_text[12] = {'0', 0};

void update(double delta_time)
{
	float x = thing_x + (thing_dx * delta_time);
	if (x < 0 || (x + thing_size) > SCREEN_WIDTH)
	{
		thing_dx *= -1;
	}
	else
	{
		thing_x = x;
	}

	float y = thing_y + (thing_dy * delta_time);
	if (y < 0 || (y + thing_size) > SCREEN_HEIGHT)
	{
		thing_dy *= -1;
	}
	else
	{
		thing_y = y;
	}
}

void render(SDL_Renderer *renderer, Font *font)
{
	SDL_SetRenderDrawColor(renderer, COLOR_BACK);
	SDL_RenderClear(renderer);

	SDL_Rect rect = {
		.x = thing_x,
		.y = thing_y,
		.w = thing_size,
		.h = thing_size,
	};

	SDL_SetRenderDrawColor(renderer, COLOR_THING);
	SDL_RenderFillRect(renderer, &rect);

	SDL_Color text_color = {COLOR_TEXT};
	float counter_x = SCREEN_WIDTH - 16 - (FONT_CHAR_WIDTH * FONT_SCALE * strlen(fps_text));
	float counter_y = 16;
	render_text(renderer, font, fps_text, text_color, counter_x, counter_y, FONT_SCALE);

	SDL_RenderPresent(renderer);
}

int main()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0)
	{
		eprintf("Unable to initialize SDL2: %s\n", SDL_GetError());
		return 1;
	}

	SDL_Window *window = SDL_CreateWindow("fps", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
										  SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	check_null(SDL, window);

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	check_null(SDL, render);

	if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG)
	{
		eprintf("Unable to initialize SDL2_image: %s\n", IMG_GetError());
		return 1;
	}

	Font font = load_font(renderer, "charmap.png");

	const double freq_ms = SDL_GetPerformanceFrequency();
	Uint64 last_time = SDL_GetPerformanceCounter();

	unsigned int frame_counter = 0;
	double frame_timer = last_time;

	while (running)
	{
		Uint64 current_time = SDL_GetPerformanceCounter();
		double delta = (current_time - last_time) / freq_ms * 1000.0;

		if (current_time > frame_timer + freq_ms)
		{
			sprintf(fps_text, "%d", frame_counter);
			frame_counter = 0;
			frame_timer = current_time;
		}

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_QUIT:
					running = false;
					break;

				default:
					break;
			}
		}

#ifdef FPS_CAP
		const double frame_delta = 1000.0 / FPS;

		if (delta > frame_delta)
#endif
		{
			update(delta);
			render(renderer, &font);

			last_time = current_time;
			++frame_counter;
		}
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}

