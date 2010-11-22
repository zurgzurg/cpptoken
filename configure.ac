AC_INIT([cpptoken], [1.0])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/re_parse.cpp])
AC_CONFIG_FILES([Makefile src/Makefile include/Makefile])

AC_PROG_INSTALL
AC_PROG_RANLIB

AC_PROG_CXXCPP
AC_PROG_CXX

AC_LANG([C++])

CXXFLAGS="-g -O0"


AC_PROG_LIBTOOL

AC_ARG_ENABLE(
	[mt-exec],
	[AS_HELP_STRING(
                        [--disable-mt-exec],
                        [disable multi-threaded execution])],
	[mt_exec=${enableval}],
	[mt_exec=yes])

have_pthreads=no
AC_SEARCH_LIBS([pthread_create], [pthread], [have_pthreads=yes])
if test "x${have_pthreads}" = "xyes"; then
  AC_CHECK_HEADERS([pthread.h], [], [have_pthreads=no])
fi
  
AC_CONFIG_HEADERS([src/config.h])
AC_CHECK_HEADERS([unistd.h cstddef cstring exception list sstream iostream],,
                 [AC_MSG_ERROR([Required header files missing.])])

AC_OUTPUT

echo ""
echo "Summary of options"
echo ""
echo "Use multi-threading          ${mt_exec}"
echo ""
