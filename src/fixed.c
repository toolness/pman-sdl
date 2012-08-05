#include "globals.h"

#include <stdlib.h>

#include "SDL.h"

#include "fixed.h"

const FixedVector fixed_vector_zero = { 0,0 };
const FixedVector fixed_vector_up = { 0,FIXED_SET_INT(-1) };
const FixedVector fixed_vector_down = { 0,FIXED_SET_INT(1) };
const FixedVector fixed_vector_left = { FIXED_SET_INT(-1),0 };
const FixedVector fixed_vector_right = { FIXED_SET_INT(1),0 };

/* Returns a random number in the range 0 <= rand_int(int_max) < int_max. */
int rand_int(int max_int)
{
	int r;

	r = rand();
	if (r == RAND_MAX) r--;
	return (int) ( ((double) r / (RAND_MAX)) * (max_int));
}

/* Returns a pointer to a random vector from the given list with the given
   number of elements. */
FixedVector *fixed_vector_choose_random(FixedVector vlist[], int num_vectors)
{
	return &vlist[rand_int(num_vectors)];
}

/* convert a float to a fixed point number. */
fixed fixed_from_float(double flo)
{
  fixed int_part, frac_part;

  int_part = (fixed) flo;

  frac_part = (fixed) ((flo - int_part) * FIXED_PRECISION_AMOUNT);

  return FIXED_SET_INT(int_part) + frac_part;
}

/* convert a fixed point number to a float. */
double fixed_to_float(fixed f)
{
  fixed int_part;

  int_part = FIXED_GET_INT(f);

  return int_part + ( (double) (f - FIXED_SET_INT(int_part)) / FIXED_PRECISION_AMOUNT );
}

/* multiply two fixed point numbers (this is shown only for the proof; for speed, use
   the FIXED_MULT() macro) */
fixed fixed_mult(fixed f1, fixed f2)
{
  /* Let f1' = (the actual number f1 is supposed to be)
	 Same with f2'...

     Since f1 is f1' * FIXED_PRECISION_AMOUNT and
           f2 is f2' * FIXED_PRECISION_AMOUNT,

     f1*f2 = FIXED_PRECISION_AMOUNT^2 * f1' * f2'

	 So we need to divide by FIXED_PRECISION_AMOUNT to get
	 the right answer. */
	 
  return (f1 * f2) >> FIXED_PRECISION_WIDTH;
}

/* divide two fixed point numbers */
fixed fixed_div(fixed f1, fixed f2)
{
  fixed quotient;
  fixed remainder;
  fixed frac_part;

  quotient = f1 / f2;

  remainder = f1 - (quotient * f2);

  /* Now convert the remainder to a binary fraction:
  
     ( remainder / f2 ) = (frac_part / FIXED_PRECISION_AMOUNT)

	 cross-multiplying we have...

	 remainder * FIXED_PRECISION_AMOUNT = f2 * frac_part
  */

  frac_part = (remainder << FIXED_PRECISION_WIDTH) / f2;
  
  return FIXED_SET_INT(quotient) + frac_part;
}

/* Sets the given fixed vector to the given amounts. */
void fixed_vector_set(FixedVector *v, double x, double y)
{
	v->x = fixed_from_float(x);
	v->y = fixed_from_float(y);
}

/* Adds the two vectors. */
FixedVector fixed_vector_add(const FixedVector *v1, const FixedVector *v2)
{
	FixedVector v3;

	v3.x = v1->x + v2->x;
	v3.y = v1->y + v2->y;

	return v3;
}

/* Scales the vector by the given fixed amount.  e.g. (1,1) scaled by 3 is
   (3,3).  */
FixedVector fixed_vector_scale(const FixedVector *v, fixed scale)
{
	FixedVector v2;

	v2.x = FIXED_MULT(v->x, scale);
	v2.y = FIXED_MULT(v->y, scale);

	return v2;
}

/* Returns true if the given fixed vectors are equal. */
int fixed_vector_equals(const FixedVector *v1, const FixedVector *v2)
{
	return (v1->x == v2->x && v1->y == v2->y);
}

/* Returns true if the given vector is a zero vector (i.e., [0,0]). */
int fixed_vector_is_zero(const FixedVector *v1)
{
	return (v1->x == 0 && v1->y == 0);
}

/* Returns true if the given rectangles intersect/overlap. */
int rects_intersect(SDL_Rect *r1, SDL_Rect *r2)
{
    return ( (r1->x + r1->w >= r2->x) &&
             (r2->x + r2->w >= r1->x) &&
             (r1->y + r1->h >= r2->y) &&
             (r2->y + r2->h >= r1->y) );
}

/* Merge the two rects, storing the resulting rect in merged_rect.
   (By "merge", we mean find the smallest possible rect that encloses
   the two given ones.) */
void rects_merge(SDL_Rect *r1, SDL_Rect *r2, SDL_Rect *merged_rect)
{
	int r1_x2, r1_y2, r2_x2, r2_y2, merged_x2, merged_y2;

	/* Figure out the top-left of the merged rectangle. */
	merged_rect->x = r1->x < r2->x ? r1->x : r2->x;
	merged_rect->y = r1->y < r2->y ? r1->y : r2->y;

	/* Figure out the bottom-right of the source rectangles. */
	r1_x2 = r1->x + r1->w; r1_y2 = r1->y + r1->h;
	r2_x2 = r2->x + r2->w; r2_y2 = r2->y + r2->h;

	/* Figure out the bottom-right of the merged rectangle. */
	merged_x2 = r1_x2 > r2_x2 ? r1_x2 : r2_x2;
	merged_y2 = r1_y2 > r2_y2 ? r1_y2 : r2_y2;

	/* Convert that bottom-right corner to width and height values. */
	merged_rect->w = (Uint16) (merged_x2 - merged_rect->x);
	merged_rect->h = (Uint16) (merged_y2 - merged_rect->y);
}

/* Converts the given fixed vector into the x,y coordinates of the supplied
   rect. */
void fixed_vector_to_rect_coords(const FixedVector *v, SDL_Rect *r_dest)
{
	r_dest->x = (Sint16) FIXED_GET_INT(v->x);
	r_dest->y = (Sint16) FIXED_GET_INT(v->y);

}

/* Converts the given fixed vector into the height and width dimensions of the supplied
   rect. */
void fixed_vector_to_rect_dimensions(const FixedVector *v_dimensions, SDL_Rect *r_dest)
{
	r_dest->h = (Sint16) FIXED_GET_INT(v_dimensions->y);
	r_dest->w = (Sint16) FIXED_GET_INT(v_dimensions->x);
}

/* Turns the x,y coordinates of the given rect into a fixed vector. */
FixedVector fixed_vector_from_rect(SDL_Rect *r)
{
	FixedVector v;

	v.x = FIXED_SET_INT(r->x);
	v.y = FIXED_SET_INT(r->y);
	return v;
}

/* Returns the manhattan distance between the given vectors.  Manhattan distance,
   or "taxicab distance", is just the sum of the x-axis distance and the y-axis
   distance between two points.  It's a perfectly accurate metric in the pman world,
   since game agents can't move diagonally. */
fixed fixed_vector_manhattan_distance(const FixedVector *v1, const FixedVector *v2)
{
	return abs(v1->x - v2->x) + abs(v1->y - v2->y);
}

/* Returns vector rotated 90 degrees to the left (counterclockwise) */
FixedVector fixed_vector_rotate_left(const FixedVector *v)
{
	FixedVector v2;

	v2.x = v->y;
	v2.y = -v->x;

	return v2;
}

/* Returns vector rotated 90 degrees to the right (clockwise) */
FixedVector fixed_vector_rotate_right(const FixedVector *v)
{
	FixedVector v2;

	v2.x = -v->y;
	v2.y = v->x;

	return v2;
}

/* Reverses the given fixed vector.  i.e., if given (-1,1) it returns (1,-1). */
FixedVector fixed_vector_reverse(const FixedVector *v)
{
	return fixed_vector_scale(v, FIXED_SET_INT(-1));
}
