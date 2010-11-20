check_PROGRAMS = utests

lib_LIBRARIES = libcpptoken.a
libcpptoken_a_SOURCES = cpptoken.cpp re_parse.cpp cpptoken_private.h ../include/cpptoken.h
libcpptoken_a_CXXFLAGS = -I$(top_srcdir)/include

lib_LTLIBRARIES = libcpptoken.la
libcpptoken_la_SOURCES = cpptoken.cpp re_parse.cpp cpptoken_private.h ../include/cpptoken.h
libcpptoken_la_CXXFLAGS = -I$(top_srcdir)/include

utests_SOURCES = utests.cpp
utests_CXXFLAGS = -I$(top_srcdir)/include
utests_LDADD = -lcpptoken
