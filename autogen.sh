#!/bin/sh

libtoolize --copy

aclocal \
&& automake --add-missing \
&& autoconf
