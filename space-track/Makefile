
INCLUDEDIR=include
LIBDIR=Sgp4Prop_small/Lib/Linux64

CFLAGS=-O -Wall -I${INCLUDEDIR}
LIBFLAGS=-Wl,-rpath=${LIBDIR} -L${LIBDIR} -ldllmain -ltimefunc -lenvconst -lastrofunc -lm


test-sgp4: test-sgp4.c
	gcc $(CFLAGS) $< -o $@ $(LIBFLAGS)

.PHONY: clean
clean:
	rm -f test-sgp4
