#include "globals.h"

#include <stdlib.h>
#include <stdio.h>

/* Print out the given error message to stderr.  If fatal is nonzero,
   the application exits with the given error code. */
void err(const char *err_msg, int fatal)
{
	fprintf(stderr, err_msg);
	if (fatal) {
		exit(fatal);
	}
}
