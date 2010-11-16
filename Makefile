CXXFLAGS=-g -Wall

all: utests libcpptoken.a

LIB_OBJS=re_parse.o

$(LIB_OBJS): cpptoken_private.h

libcpptoken.a: $(LIB_OBJS)
	rm -f $@
	ar rc $@ $(LIB_OBJS)

utests.o: utests.cpp cpptoken_private.h
	g++ -o $@ -c $(CXXFLAGS) $<

utests: utests.o libcpptoken.a
	g++ -o $@ $(CXXFLAGS) $^ -L. -lcpptoken

clean:
	rm -f *.o *~ utests libcpptoken.a
