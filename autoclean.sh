#!/bin/sh

##
## remove ALL the generated files.
##

rm -f aclocal.m4 configure depcomp
rm -f install-sh ltmain.sh missing Makefile.in *~
rm -fr autom4te.cache
rm -f m4/ltsugar.m4 m4/libtool.m4 m4/ltversion.m4 m4/lt~obsolete.m4
rm -f m4/ltoptions.m4
rm -f include/Makefile.in
