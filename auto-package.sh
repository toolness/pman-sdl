#!/bin/sh
#
# This script just builds the source distribution archive (e.g.,
# "pman-0.1.1.tar.gz") from the sources.

./configure
make dist
