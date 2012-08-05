#include "globals.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "SDL.h"

#include "game.h"
#include "font.h"

/* Constructs a new font object.  Loads a tiled font file from the given bitmap image
   file, using the given character width and height (the font is monospaced).  The
   characters-per-line are used when extracting a character from the tiled bitmap file
   and corresponds to the number of characters per "line of characters" there are
   in the bitmap file. */
void font_init(Font *f, const char *bmp_filename, int char_width, int char_height, int bitmap_chars_per_line)
{
	SDL_Surface *s;

	f->bitmap_chars_per_line = bitmap_chars_per_line;
	f->char_height = char_height;
	f->char_width = char_width;
	
	s = game_load_bmp(bmp_filename);

	SDL_SetColorKey(s, SDL_SRCCOLORKEY | SDL_RLEACCEL, 1);
	f->bitmap = SDL_DisplayFormatAlpha(s);
	assert(f->bitmap != NULL);

	SDL_FreeSurface(s);
}

/* Draws the given string in the given font, centered at the specified (x,y) coordinates. */
void font_draw_string_centered(Font *f, SDL_Surface *surface, int x, int y, const char *string)
{
	int x_ofs;
	int y_ofs;

	x_ofs = x - ( ((int) strlen(string) * f->char_width) / 2);
	y_ofs = y - (f->char_height / 2);

	font_draw_string(f, surface, x_ofs, y_ofs, string);
}

/* Draws the given string with a black rectangle behind it and then updates the
   game's rectangle list for redrawing. */
void font_draw_string_opaque(Font *f, SDL_Surface *surface, int x, int y, const char *string)
{
  SDL_Rect r;

  /* Draw a black rectangle behind where the string is going to be drawn */
  r.x = x;
  r.y = y;
  r.w = (Uint16)(f->char_width*strlen(string));
  r.h = (Uint16) f->char_height;

  SDL_FillRect(surface, &r, 0);

  /* Draw the string */
  font_draw_string(f, surface, r.x, r.y, string);

  /* Add the rectangle of the area we just drew onto to the list of rects to redraw. */
  game_update_rect_add(&r);
}

/* Draws the given string in the given font, so that the upper-left corner of the string
   is at the given (x,y) pixel coordinates. */
void font_draw_string(Font *f, SDL_Surface *surface, int x, int y, const char *string)
{
	int i;
	int string_length;

	SDL_Rect r_src;
	SDL_Rect r_dst;

	r_dst.x = (Sint16) x;
	r_dst.y = (Sint16) y;
	r_dst.w = (Uint16) f->char_width;
	r_dst.h = (Uint16) f->char_height;

	r_src.w = (Uint16) f->char_width;
	r_src.h = (Uint16) f->char_height;

	string_length = (int) strlen(string);

	for (i = 0; i < string_length; i++) {
		int char_code, char_x, char_y;

		char_code = string[i] - CHAR_CODE_OFFSET;
		char_x = char_code % f->bitmap_chars_per_line;
		char_y = char_code / f->bitmap_chars_per_line;

		r_src.x = (Sint16) (char_x * f->char_width);
		r_src.y = (Sint16) (char_y * f->char_height);

		SDL_BlitSurface(f->bitmap, &r_src, surface, &r_dst);
		r_dst.x = r_dst.x + (Sint16) f->char_width;
	}
}

/* Frees memory allocated by the font constructor. */
void font_destroy(Font *f)
{
	/* Free the tiled character bitmap image. */
	SDL_FreeSurface(f->bitmap);
}
