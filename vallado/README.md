# David Vallado implmentation
Vallado's implementation, available from [celestrak](https://celestrak.com/software/vallado-sw.php), is widely regarded as the reference implementation.  It was effectively the primary source before the asto standards were made available on space-track.

Steps are:
* Download the code [celestrak](https://celestrak.com/software/vallado/cpp.zip) and extract the `SGP4.hpp` and `SGP4.cpp` files into this folder.
* Compile code using
```
$ make
```
* Run using
```
$ ./test-sgp4
```

## Dependencies
I have am relying on the [C++ JSON parser](https://github.com/nlohmann/json), which I have downloaded explicitly.  There is an Ubuntu package, but it isn't compatible with the compiler I'm using.

