AC_INIT([Pman], [0.1.5], [Atul Varma], [pman])
AM_CONFIG_HEADER(config.h)

AC_CANONICAL_HOST
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE
AC_PROG_CC
AC_PROG_INSTALL

AM_SDL_ENABLER

if test "x$enable_sdl" = "xyes" ; then
  dnl Check for SDL
  SDL_VERSION=1.2.0
  AM_PATH_SDL($SDL_VERSION, :, AC_MSG_ERROR([*** SDL version $SDL_VERSION not found!]))
  CFLAGS="$CFLAGS $SDL_CFLAGS"
  LIBS="$LIBS $SDL_LIBS"
fi

AC_CONFIG_FILES([Makefile docs/Makefile resources/Makefile resources/bmp/Makefile src/Makefile VisualC/Makefile])
AC_OUTPUT
