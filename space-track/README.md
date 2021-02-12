# Official US Astrodynamics Standards Implementation
The way that include files and libraries are used in the example code seem very strange, so I've done things a bit differently.  It probably ends up messier, but at least I know what's going on.

Note that I'm using linux (actually Ubuntu under Windows Subsystem for Linux).

Steps are:
* Download the propagator library (in the file `Sgp4Prop_small_v8.0.zip`) and extract into the folder `Sgp4Prop_small`.
* Run `bash gen-include.sh` (which in turn calls `afspc-fix.py` a few times).  This creates an `include` folder and populates it with sane include files for the AFSPC libraries.
* Compile code using
```
$ make
```
* Run using
```
$ LD_LIBRARY_PATH=Sgp4Prop_small/Lib/Linux64 ./test-sgp4
```