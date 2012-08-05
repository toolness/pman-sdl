#include "globals.h"

#include "drawing.h"

/* Given a fixed vector in a cardinal direction (e.g. fixed_vector_left),
   returns the DIRECTION_* constant for that given direction. */
int map_fixed_vector_to_direction(FixedVector *v)
{
	if (v->x < 0)
		return DIRECTION_LEFT;
	else if (v->x > 0)
		return DIRECTION_RIGHT;
	else if (v->y < 0)
		return DIRECTION_UP;
	else if (v->y > 0)
		return DIRECTION_DOWN;
	else return -1;
}

/* Taken from SDL tutorial code at \lib_sdl\docs\html\guidevideo.html */
void drawPixel(SDL_Surface *screen, int x, int y, Uint32 color)
{
	Uint8 *bufp8;
	Uint16 *bufp16;
	Uint32 *bufp32;

	if (x < screen->clip_rect.x || x >= screen->clip_rect.x + screen->clip_rect.w) return;

	switch (screen->format->BytesPerPixel) {
		case 1:
			bufp8 = (Uint8 *) screen->pixels + y*screen->pitch + x;
			*bufp8 = (Uint8) color;
			break;
		case 2: {
			bufp16 = (Uint16 *) screen->pixels + y*screen->pitch/2 + x;
			*bufp16 = (Uint16) color; }
			break;
		case 3: {
			bufp8 = (Uint8 *) screen->pixels + y*screen->pitch + x * 3;
			if (SDL_BYTEORDER == SDL_LIL_ENDIAN) {
				bufp8[0] = (Uint8) color;
				bufp8[1] = (Uint8) (color >> 8);
				bufp8[2] = (Uint8) (color >> 16);
			} else {
				bufp8[2] = (Uint8) color;
				bufp8[1] = (Uint8) (color >> 8);
				bufp8[0] = (Uint8) (color >> 16);
			} }
			break;
		case 4: {
			bufp32 = (Uint32 *)screen->pixels + y*screen->pitch/4 + x;
			*bufp32 = color; }
			break;
	}
}

/* Draws a horizontal line. */
void drawHLine(SDL_Surface *screen, int x1, int y1, int x2, int color)
{
	int i;

	int clip_x1, clip_x2;

	if (x1 > x2) {
		int temp_x;

		temp_x = x1;
		x1 = x2;
		x2 = temp_x;
	}

	clip_x1 = screen->clip_rect.x;
	clip_x2 = screen->clip_rect.x + screen->clip_rect.w;

	if ( x1 < clip_x1 ) {
		if ( x2 < clip_x1 ) {
			return;
		} else x1 = clip_x1;
	} else if ( x2 > clip_x2 ) {
		if ( x1 > clip_x2 ) {
			return;
		} else x2 = clip_x2;
	}

	for (i = x1; i <= x2; i++)
		drawPixel(screen, i, y1, color);
}

/* Circle-filling algorithm, to be used instead of fillArcPoint to draw filled circles
   instead of outlines.  If given a center and a point on the 1/8 circle arc, fills 4
   parts of the circle with it. */
void fillArcPoint(SDL_Surface *screen, int x1, int y1, int x, int y, int amount_filled, Uint32 color)
{
	drawHLine(screen, x1-x, y1-y, x1+x, color);
	drawHLine(screen, x1-y, y1-x, x1+y, color);
	if (amount_filled != FILL_TOP_HALF_ONLY) {
		drawHLine(screen, x1-y, y1+x, x1+y, color);
		drawHLine(screen, x1-x, y1+y, x1+x, color);
	}
}

/* Circle-outline drawing.  Given a center and a point on the 1/8 circle arc, draws
   the 8 corresponding points on the circle by mirroring along the x, y, and x=y
   axes. */
void drawArcPoint(SDL_Surface *screen, int x1, int y1, int x, int y, Uint32 color)
{
	drawPixel(screen, x1+x, y1-y, color);
	drawPixel(screen, x1+y, y1-x, color);
	drawPixel(screen, x1-x, y1+y, color);
	drawPixel(screen, x1-y, y1+x, color);

	drawPixel(screen, x1+x, y1+y, color);
	drawPixel(screen, x1+y, y1+x, color);
	drawPixel(screen, x1-x, y1-y, color);
	drawPixel(screen, x1-y, y1-x, color);
}

/* Circle-drawing routine.  Derived using concepts covered in
   http://www.cs.unc.edu/~davemc/Class/136/Lecture10/circle.html. 
   Set "filled" to the FILL_* constants. */
void drawCircle(SDL_Surface *screen, int x1, int y1, int r, int filled, Uint32 color)
{
	int curr_val = (5 - r*4)/4;

	int x = 0;
	int y = r;

	if (filled != FILL_TOP_HALF_ONLY) {
		drawPixel(screen, x1, y1+r, color);
	}
	drawPixel(screen, x1, y1-r, color);
	if (filled)
		drawHLine(screen, x1-r, y1, x1+r, color);
	else {
		drawPixel(screen, x1+r, y1, color);
		drawPixel(screen, x1-r, y1, color);
	}

	while (x < y) {
		x++;
		if (curr_val >= 0) {
			curr_val += 2 + (x << 1) - (y << 1);
			y--;
		} else {
			curr_val += 1 + (x << 1);
		}
		if (filled)
			fillArcPoint(screen, x1, y1, x, y, filled, color);
		else
			drawArcPoint(screen, x1, y1, x, y, color);
	}
}

/* My implementation of the Bresenham Line Drawing Algorithm.  Derived using
   concepts covered in http://www.cs.helsinki.fi/group/goa/mallinnus/lines/bresenh.html
   and http://www.gamedev.net/reference/articles/article1275.asp. */
void drawLine(SDL_Surface *screen, int x1, int y1, int x2, int y2, Uint32 color)
{
	int deltax, deltay;
	int x, y;

	if (x1 > x2) {
		int temp_x, temp_y;

		temp_x = x1;
		temp_y = y1;
		x1 = x2;
		y1 = y2;
		x2 = temp_x;
		y2 = temp_y;
	}

	deltax = x2 - x1;
	deltay = y2 - y1;

	x = x1;
	y = y1;

	drawPixel(screen, x, y, color);

	/* Remember that we're using the computer's coordinate system, so
	   the y-axis is reversed. */

	/* If the line is in the 1st quadrant... */
	if (deltay < 0) {
		deltay = -deltay;
		if (deltax > deltay) {
			/* If its slope is less than 1 */
			int y_counter = deltax >> 1;
			while (x < x2) {
				x++;
				y_counter += deltay;
				if (y_counter > deltax) {
					y--;
					y_counter -= deltax;
				}
				drawPixel(screen, x, y, color);	
			}
		} else {
			/* If its slope is greater than 1 */
			int x_counter = deltay >> 1;
			while (y > y2) {
				y--;
				x_counter += deltax;
				if (x_counter > deltay) {
					x++;
					x_counter -= deltay;
				}
				drawPixel(screen, x, y, color);
			}
		}
		return;
	}

	/* If the line is in the 4th quadrant... */
	if (deltay > deltax) {
		/* If its slope is greater than 1 */
		int x_counter = deltay >> 1;
		while (y < y2) {
			y++;
			x_counter += deltax;
			if (x_counter > deltay) {
				x++;
				x_counter -= deltay;
			}
			drawPixel(screen, x, y, color);
		}
	} else {
		/* If its slop is less than 1 */
		int y_counter = deltax >> 1;
		while (x < x2) {
			x++;
			y_counter += deltay;
			if (y_counter > deltax) {
				y++;
				y_counter -= deltax;
			}
			drawPixel(screen, x, y, color);
		}
	}
}

/* Draws a flat-bottom triangle. */
void drawFlatBottomTriangle(SDL_Surface *screen, int a_x, int a_y, int b_x, int b_y, int c_x, int c_y, Uint32 color)
{
	int a_int;

	int a_sx = b_x - a_x;
	int a_sy = a_y - b_y;
	int a_dir;

	int start_x, start_x_num;
	int c_sx, c_sy, c_dir;

	int c_int;
	int end_x, end_x_num;

	int i;

	if (a_sx < 0) {
		a_sx = -a_sx;
		a_dir = -1;
	} else {
		a_dir = 1;
	}

	/* Integral part of discriminate */
	if (a_sy == 0) return;
	a_int = a_sx / a_sy;

	/* Fractional part of discriminate (numerator only, denominator is a_sy) */
	a_sx -= a_int * a_sy;

	a_int *= a_dir;

	start_x = a_x;
	start_x_num = a_sy >> 1;

	c_sx = b_x - c_x;
	c_sy = c_y - b_y;
	c_dir;

	if (c_sx < 0) {
		c_sx = -c_sx;
		c_dir = -1;
	} else {
		c_dir = 1;
	}

	/* Integral part of discriminate */
	c_int = c_sx / c_sy;

	/* Fractional part of discriminate (numerator only, denominator is a_sy) */
	c_sx -= c_int * c_sy;

	c_int *= c_dir;

	end_x = c_x;
	end_x_num = c_sy >> 1;

	for (i = a_y; i >= b_y; i--) {
		drawHLine(screen, start_x, i, end_x, color);
		start_x += a_int;
		start_x_num += a_sx;
		if (start_x_num >= a_sy) {
			start_x = start_x + a_dir;
			start_x_num -= a_sy;
		}
		end_x += c_int;
		end_x_num += c_sx;
		if (end_x_num >= c_sy) {
			end_x = end_x + c_dir;
			end_x_num -= c_sy;
		}
	}
}

/* Draws a flat-top triangle. */
void drawFlatTopTriangle(SDL_Surface *screen, int a_x, int a_y, int b_x, int b_y, int c_x, int c_y, Uint32 color)
{
	int a_int;

	int a_sx = b_x - a_x;
	int a_sy = a_y - b_y;
	int a_dir;

	int start_x, start_x_num;
	int c_sx, c_sy, c_dir;

	int c_int;
	int end_x, end_x_num;

	int i;

	a_sx = b_x - a_x;
	a_sy = b_y - a_y;
	a_dir;

	if (a_sx < 0) {
		a_sx = -a_sx;
		a_dir = -1;
	} else {
		a_dir = 1;
	}

	/* Integral part of discriminate */
	if (a_sy == 0) return;
	a_int = a_sx / a_sy;

	/* Fractional part of discriminate (numerator only, denominator is a_sy) */
	a_sx -= a_int * a_sy;

	a_int *= a_dir;

	start_x = a_x;
	start_x_num = a_sy >> 1;

	c_sx = b_x - c_x;
	c_sy = b_y - c_y;

	if (c_sx < 0) {
		c_sx = -c_sx;
		c_dir = -1;
	} else {
		c_dir = 1;
	}

	/* Integral part of discriminate */
	c_int = c_sx / c_sy;

	/* Fractional part of discriminate (numerator only, denominator is a_sy) */
	c_sx -= c_int * c_sy;

	c_int *= c_dir;

	end_x = c_x;
	end_x_num = c_sy >> 1;

	for (i = a_y; i <= b_y; i++) {
		drawHLine(screen, start_x, i, end_x, color);
		start_x += a_int;
		start_x_num += a_sx;
		if (start_x_num >= a_sy) {
			start_x = start_x + a_dir;
			start_x_num -= a_sy;
		}
		end_x += c_int;
		end_x_num += c_sx;
		if (end_x_num >= c_sy) {
			end_x = end_x + c_dir;
			end_x_num -= c_sy;
		}
	}
}
