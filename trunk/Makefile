CXXFLAGS=-g -Wall

all: utests

utests.o: utests.cpp
	g++ -o $@ -c $(CXXFLAGS) $<

utests: utests.o
	g++ -o $@ $(CXXFLAGS) $^

clean:
	rm -f *.o *~ utests
