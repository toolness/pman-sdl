#ifndef INCLUDE_FONT
#define INCLUDE_FONT

/* font.h

   Simple bitmap font module.
   Atul Varma - 7/22/2003

   Image data for bitmap fonts is stored in a tiled image, and when a string needs to be
   printed, a character is retrieved from a sub-rectangle of the font's tiled image.
*/

#include "SDL.h"

/* ASCII value to subtract from every ASCII character we're asked to print. */
#define CHAR_CODE_OFFSET 32

/* The font structure. */
typedef struct Font {
	/* Tiled bitmap used to get the character bitmap data from. */
	SDL_Surface *bitmap;

	/* Width of each character on the tiled bitmap. */
	int char_width;

	/* Height of each character on the tiled bitmap. */
	int char_height;

	/* Characters per line on the tiled bitmap. */
	int bitmap_chars_per_line;
} Font;

void font_init(Font *f, const char *bmp_filename, int char_width, int char_height, int bitmap_chars_per_line);
void font_draw_string(Font *f, SDL_Surface *surface, int x, int y, const char *string);
void font_draw_string_opaque(Font *f, SDL_Surface *surface, int x, int y, const char *string);
void font_draw_string_centered(Font *f, SDL_Surface *surface, int x, int y, const char *string);
void font_destroy(Font *f);

#endif
