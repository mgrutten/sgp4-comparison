
INCLUDEDIR=include
LIBDIR=Sgp4Prop_small/Lib/Linux64

CPPFLAGS=-O -Wall -I${INCLUDEDIR} -std=c++11
LIBFLAGS=


test-sgp4: test-sgp4.cpp SGP4.o
	g++ $(CPPFLAGS) $^ -o $@ $(LIBFLAGS)

#MathTimeLib.o: MathTimeLib.cpp
#	g++ $(CPPFLAGS) $<

SGP4.o: SGP4.cpp
	g++ -c $(CPPFLAGS) $<

.PHONY: clean
clean:
	rm -f test-sgp4
