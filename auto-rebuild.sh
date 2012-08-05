#!/bin/sh
#
# This script runs all the GNU autotools over again to rebuild the
# configure script, makefile.in's, config.h.in, aclocal.m4, etc...

aclocal
autoheader
autoconf
automake
