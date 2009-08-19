#! /bin/sh
libtoolize --force --copy \
&& aclocal \
&& automake --add-missing \
&& autoconf \
&& autoheader
