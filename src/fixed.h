#ifndef INCLUDE_FIXED
#define INCLUDE_FIXED

/* fixed.h

   Generic fixed point module.
   Atul Varma - 7/19/2003

   The algorithms in the functions of this module were originally derived using
   base-10 notation and were then converted to base-2.  A fixed point number is
   stored as a 32-bit integer with (by default, at least) a 16-bit integer part
   and an 8-bit binary fraction.  The other 8 bits are used up when multiplying
   two fixed point numbers, which is a little weird (see the multiply function's
   proof for details).
*/

#include "SDL.h"

typedef long int fixed;

/* bit width of fractional part of fixed point numbers: */
#define FIXED_PRECISION_WIDTH 8

/* denominator of fractional part of fixed point numbers: */
#define FIXED_PRECISION_AMOUNT (1 << FIXED_PRECISION_WIDTH)

/* bit width of integer part of fixed point numbers: */
#define FIXED_INT_WIDTH ( ( sizeof(fixed) * 8 ) - (FIXED_PRECISION_WIDTH*2) )

/* maximum and minimum possible int values for fixed point numbers:
   (1 is subtracted b/c it is used as sign bit) */
#define FIXED_INT_MAX (1 << (FIXED_INT_WIDTH - 1) )
#define FIXED_INT_MIN (-1 << (FIXED_INT_WIDTH - 1) )

/* get the integer part of a fixed point number: */
#define FIXED_GET_INT(x) ((x) >> FIXED_PRECISION_WIDTH)

/* set the integer part of a fixed point number: */
#define FIXED_SET_INT(x) ((x) << FIXED_PRECISION_WIDTH)

/* quick macro for fixed point multiplication:
   (see fixed_mult() below for proof) */
#define FIXED_MULT(f1,f2) (((f1) * (f2)) >> FIXED_PRECISION_WIDTH)

fixed fixed_from_float(double flo);
double fixed_to_float(fixed f);
fixed fixed_mult(fixed f1, fixed f2);
fixed fixed_div(fixed f1, fixed f2);

/* A FixedVector is just a "coordinate" type for fixed numbers and represents a
   point in R^2. */
typedef struct FixedVector {
	fixed x, y;
} FixedVector;

/* FixedVector constants for directions.  All of these are normalized (they have a
   magnitude of 1). */
extern const FixedVector fixed_vector_zero;
extern const FixedVector fixed_vector_up;
extern const FixedVector fixed_vector_down;
extern const FixedVector fixed_vector_left;
extern const FixedVector fixed_vector_right;

void fixed_vector_set(FixedVector *v, double x, double y);
FixedVector fixed_vector_add(const FixedVector *v1, const FixedVector *v2);
FixedVector fixed_vector_scale(const FixedVector *v, fixed scale);
int fixed_vector_equals(const FixedVector *v1, const FixedVector *v2);
int fixed_vector_is_zero(const FixedVector *v1);

int rects_intersect(SDL_Rect *r1, SDL_Rect *r2);

void fixed_vector_to_rect_coords(const FixedVector *v, SDL_Rect *r_dest);
void fixed_vector_to_rect_dimensions(const FixedVector *v_dimensions, SDL_Rect *r_dest);
FixedVector fixed_vector_from_rect(SDL_Rect *r);
fixed fixed_vector_manhattan_distance(const FixedVector *v1, const FixedVector *v2);

FixedVector fixed_vector_rotate_left(const FixedVector *v);
FixedVector fixed_vector_rotate_right(const FixedVector *v);

FixedVector fixed_vector_reverse(const FixedVector *v);
FixedVector *fixed_vector_choose_random(FixedVector vlist[], int num_vectors);
int rand_int(int max_int);
void rects_merge(SDL_Rect *r1, SDL_Rect *r2, SDL_Rect *merged_rect);

#endif
