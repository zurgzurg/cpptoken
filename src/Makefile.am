noinst_PROGRAMS = utests
TESTS = utests

TESTS_ENVIRONMENT = $(VALGRIND)

lib_LTLIBRARIES = libcpptoken.la
libcpptoken_la_SOURCES = cpptoken.cpp re_parse.cpp nfa.cpp errors.cpp mem_util.cpp cpptoken_private.h
libcpptoken_la_CXXFLAGS = -I$(top_srcdir)/include

utests_SOURCES = utests.cpp
utests_LDADD = libcpptoken.la
utests_CXXFLAGS = -I$(top_srcdir)/include
