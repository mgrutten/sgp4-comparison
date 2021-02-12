#include <stdio.h>
#include <stdlib.h>
//#include <assert.h>
//#include <math.h>

// Library headers
#include "AstroFunc.h"
#include "DllMain.h"
#include "EnvConst.h"
#include "TimeFunc.h"

// Length of messages from AFSPC libraries
#define LOGMSGLEN 128

// Extract and show an error message to stdout
void ShowErrMsg() {
    char errMsg[LOGMSGLEN];

    GetLastErrMsg(errMsg);
    errMsg[LOGMSGLEN - 1] = 0;
    printf("%s\n", errMsg);
}

// Initialise the AFSPC libraries
void InitialiseLibs() {
    char str[LOGMSGLEN];
    uint64_t apPtr;
    int errCode;

    // Inialise DllMain library
    apPtr = DllMainInit();
    DllMainGetInfo(str);
    str[LOGMSGLEN - 1] = 0;
    printf("%s\n", str);

    // Initialise EnvConst library
    errCode = EnvInit(apPtr);
    if (errCode != 0) {
        printf("EnvInit broken %d.\n", errCode);
        ShowErrMsg();
    } else {
        EnvGetInfo(str);
        str[LOGMSGLEN - 1] = 0;
        printf("%s\n", str);
    }

    // Initialise TimeFunc library
    errCode = TimeFuncInit(apPtr);
    if (errCode != 0) {
        printf("TimeFuncInit broken %d.\n", errCode);
        ShowErrMsg();
    } else {
        TimeFuncGetInfo(str);
        str[LOGMSGLEN - 1] = 0;
        printf("%s\n", str);
    }

    // Initialise AstroFunc library
    errCode = AstroFuncInit(apPtr);
    if (errCode != 0) {
        printf("AstroFuncInit broken %d.\n", errCode);
        ShowErrMsg();
    } else {
        AstroFuncGetInfo(str);
        str[LOGMSGLEN - 1] = 0;
        printf("%s\n", str);
    }
}

int main() {
    
    // Initialise AFSPC libraries
    InitialiseLibs();

    exit(0);
}
