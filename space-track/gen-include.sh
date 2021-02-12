#!/usr/bin/bash

libs=(AstroFunc
	DllMain
	EnvConst
	Sgp4Prop
	TimeFunc
	Tle)

mkdir -p include
for l in ${libs[@]}; do
    echo "$l"
    python3 afspc-fix.py "Sgp4Prop_small/SampleCode/C/DriverExamples/wrappers/${l}Dll.h" > "include/${l}.h"
done