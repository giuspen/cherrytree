#! /bin/bash -e

SCRIPT_DIR=`dirname "$0"`

test -n "$srcdir" || srcdir=${SCRIPT_DIR}
test -n "$srcdir" || srcdir=.

if [ ! -d "$srcdir"/po ]; then
    mkdir "$srcdir"/po
fi

case ${1} in
    bin) AUTODIR="${SCRIPT_DIR}"/config/bin;;
    lib) AUTODIR="${SCRIPT_DIR}"/config/lib;;
    *) echo "argument 'bin' or 'lib' is required"; exit;;
esac

cp -v ${AUTODIR}/* .

autoreconf --force --install --verbose --warnings=all "$srcdir"
echo "Running intltoolize --copy --force --automake"
intltoolize --copy --force --automake
test -n "$NOCONFIGURE" || "$srcdir/configure"
