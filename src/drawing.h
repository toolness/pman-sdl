#ifndef INCLUDE_DRAWING
#define INCLUDE_DRAWING

/* drawing.h

   Primitive shape and pixel-drawing routines.
   Atul Varma - 7/6/2003

   Contains functions for drawing pixels, lines, circles, triangles, etc.,
   All of these drawing
   routines will clip along the x-axis of the surface they draw to, but not
   the y-axis (this is because pman needs x-axis clipping when game agents
   go through the wraparound tunnel).
*/

#include "SDL.h"
#include "SDL_endian.h"

#include "fixed.h"

#define DIRECTION_UP    0
#define DIRECTION_DOWN  1
#define DIRECTION_LEFT  2
#define DIRECTION_RIGHT 3

#define FILL_NONE 0
#define FILL_FULL 1
#define FILL_TOP_HALF_ONLY 2

int map_fixed_vector_to_direction(FixedVector *v);

void drawCircle(SDL_Surface *screen, int x1, int y1, int r, int filled, Uint32 color);

void drawPixel(SDL_Surface *screen, int x, int y, Uint32 color);
void drawHLine(SDL_Surface *screen, int x1, int y1, int x2, int color);

void drawLine(SDL_Surface *screen, int x1, int y1, int x2, int y2, Uint32 color);
void drawFlatBottomTriangle(SDL_Surface *screen, int a_x, int a_y, int b_x, int b_y, int c_x, int c_y, Uint32 color);
void drawFlatTopTriangle(SDL_Surface *screen, int a_x, int a_y, int b_x, int b_y, int c_x, int c_y, Uint32 color);

#endif
