#!/bin/sh
aclocal
libtoolize --force --copy --automake
automake -afc
autoconf
rm -rf autom4te.cache/
