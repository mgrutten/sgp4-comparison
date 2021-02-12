#include <stdio.h>
#include <stdlib.h>
//#include <assert.h>
//#include <math.h>

// Library headers
#include "AstroFunc.h"
#include "DllMain.h"
#include "EnvConst.h"
#include "Sgp4Prop.h"
#include "TimeFunc.h"
#include "Tle.h"

// Length of messages from AFSPC libraries
#define LOGMSGLEN 128

// Extract and show an error message to stdout
void ShowErrMsg() {
    char errMsg[LOGMSGLEN];

    GetLastErrMsg(errMsg);
    errMsg[LOGMSGLEN - 1] = 0;
    printf("%s\n", errMsg);

    exit(1);
}

// Initialise the AFSPC libraries
void InitialiseLibs() {
    uint64_t apPtr;

    // Inialise Main library
    apPtr = DllMainInit();

    // Initialise Env library
    if (EnvInit(apPtr) != 0) ShowErrMsg();

    // Initialise TimeFunc library
    if (TimeFuncInit(apPtr) != 0) ShowErrMsg();

    // Initialise AstroFunc library
    if (AstroFuncInit(apPtr) != 0) ShowErrMsg();

    // Initialise Tle library
    if (TleInit(apPtr) != 0) ShowErrMsg();

    // Initialise Sgp4Prop library
    if (Sgp4Init(apPtr) != 0) ShowErrMsg();
}

int main() {
    // Initialise AFSPC libraries
    InitialiseLibs();

    char line1[] =
        "1 25544U 98067A   21043.27289556  .00003336  00000-0  68749-4 0  9997";
    char line2[] =
        "2 25544  51.6440 240.5646 0002850   6.0842 149.2030 15.48970429269234";

    // Load the TLE
    int64_t satkey = TleAddSatFrLines(line1, line2);
    if (satkey <= 0) ShowErrMsg();

    // Initialize the loaded TLE before it can be propagated (see Sgp4Prop dll
    // document) This is important!!!
    if (Sgp4InitSat(satkey) != 0) ShowErrMsg();

    // SGP4 propagation using minutes since epoch
    double mse = 0.0;
    double ds50UTC;
    double llh[3];
    double pos[3];
    double vel[3];
    if (Sgp4PropMse(satkey, mse, &ds50UTC, pos, vel, llh) != 0) ShowErrMsg();

    // Print position and velocity (in m and m/s)
    for (int i = 0; i < 3; ++i) {
        printf("%23.15e", pos[i] * 1e3);
    }
    for (int i = 0; i < 3; ++i) {
        printf("%23.15e", vel[i] * 1e3);
    }
    printf("\n");

    // Remove initialized TLE from memory
    if (Sgp4RemoveSat(satkey) != 0) ShowErrMsg();

    // Remove loaded TLE from memory
    if (TleRemoveSat(satkey) != 0) ShowErrMsg();

    exit(0);
}
