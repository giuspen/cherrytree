#! /bin/sh -e
test -n "$srcdir" || srcdir=`dirname "$0"`
test -n "$srcdir" || srcdir=.

autoreconf --force --install --verbose "$srcdir"
echo "Running intltoolize --copy --force --automake"
intltoolize --copy --force --automake
test -n "$NOCONFIGURE" || "$srcdir/configure" "$@"
