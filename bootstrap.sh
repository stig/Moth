#!/bin/sh

set -x
aclocal || exit 1
autoheader || exit 1
automake --foreign --add-missing || exit 1
autoconf || exit 1
