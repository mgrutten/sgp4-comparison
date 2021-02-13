
INCLUDEDIR=include
LIBDIR=Sgp4Prop_small/Lib/Linux64

CPPFLAGS=-O -Wall -I${INCLUDEDIR} -std=c++11
LIBFLAGS=-Wl,-rpath=${LIBDIR} -L${LIBDIR} -ldllmain -ltimefunc -lenvconst -lastrofunc -ltle -lsgp4prop


test-sgp4: test-sgp4.cpp
	g++ $(CPPFLAGS) $< -o $@ $(LIBFLAGS)

.PHONY: clean
clean:
	rm -f test-sgp4
