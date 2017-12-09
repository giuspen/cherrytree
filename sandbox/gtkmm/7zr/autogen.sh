#! /bin/sh -e
test -n "$srcdir" || srcdir=`dirname "$0"`
test -n "$srcdir" || srcdir=.

if [ ! -d "$srcdir"/po ]; then
    mkdir "$srcdir"/po
fi

autoreconf --force --install --verbose --warnings=all "$srcdir"
echo "Running intltoolize --copy --force --automake"
intltoolize --copy --force --automake
test -n "$NOCONFIGURE" || "$srcdir/configure" "$@"
