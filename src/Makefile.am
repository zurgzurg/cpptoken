noinst_PROGRAMS = utests
TESTS = utests_script

lib_LTLIBRARIES = libcpptoken.la
libcpptoken_la_SOURCES = cpptoken.cpp re_parse.cpp cpptoken_private.h
libcpptoken_la_CXXFLAGS = -I$(top_srcdir)/include

utests_SOURCES = utests.cpp
utests_LDADD = libcpptoken.la
utests_CXXFLAGS = -I$(top_srcdir)/include
