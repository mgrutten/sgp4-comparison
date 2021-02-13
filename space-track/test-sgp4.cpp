
#include <fstream>
#include <iostream>
#include <limits>
//#include <nlohmann/json.hpp>
#include <stdexcept>

#include "json.hpp"

// Library headers
extern "C" {
#include "AstroFunc.h"
#include "DllMain.h"
#include "EnvConst.h"
#include "Sgp4Prop.h"
#include "TimeFunc.h"
#include "Tle.h"
}

namespace json = nlohmann;

// Length of messages from AFSPC libraries
static const u_int16_t LOGMSGLEN = 128;

// Extract and show an error message to stdout
void ShowErrMsg() {
    char errMsg[LOGMSGLEN];

    GetLastErrMsg(errMsg);
    errMsg[LOGMSGLEN - 1] = 0;
    // printf("%s\n", errMsg);

    throw std::runtime_error(errMsg);
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

//     file: output file
// numSteps: number of propagation steps to make
//  noradId: NORAD ID of object
//   period: orbital period (s)
//    line1, line2: TLE lines
void PropagateAndWrite(std::ostream& file, const int numSteps,
                       const uint32_t noradId, const double period,
                       const std::string& line1, const std::string& line2) {
    // Load the TLE
    int64_t satkey = TleAddSatFrLines(const_cast<char*>(line1.c_str()),
                                      const_cast<char*>(line2.c_str()));
    if (satkey <= 0) ShowErrMsg();

    // Initialize the loaded TLE before it can be propagated (see Sgp4Prop dll
    // document) This is important!!!
    if (Sgp4InitSat(satkey) != 0) ShowErrMsg();

    // Write the NORAD ID
    // std::cout << noradId << std::endl;
    file.write(reinterpret_cast<const char*>(&noradId), sizeof(uint32_t));

    // Propagate to individual time-steps from epoch
    for (int k = 0; k <= numSteps; ++k) {
        // SGP4 propagation using minutes since epoch
        double mse = double(k) * period / 60.0 / double(numSteps);
        double ds50UTC;
        double llh[3];
        double pos[3];
        double vel[3];
        if (Sgp4PropMse(satkey, mse, &ds50UTC, pos, vel, llh) != 0)
            ShowErrMsg();

        // Write time since epoch
        // std::cout << "  " << mse;
        file.write(reinterpret_cast<const char*>(&mse), sizeof(double));

        // Write position and velocity (in m and m/s)
        // for (int i = 0; i < 3; ++i) {
        //    std::cout << "  " << pos[i] * 1e3;
        // }
        file.write(reinterpret_cast<const char*>(pos), 3 * sizeof(double));
        // for (int i = 0; i < 3; ++i) {
        //    std::cout << "  " << vel[i] * 1e3;
        // }
        file.write(reinterpret_cast<const char*>(vel), 3 * sizeof(double));
        // std::cout << std::endl;
    }

    // Remove initialized TLE from memory
    if (Sgp4RemoveSat(satkey) != 0) ShowErrMsg();

    // Remove loaded TLE from memory
    if (TleRemoveSat(satkey) != 0) ShowErrMsg();
}

// a simple struct to model a person
struct catalogueEntry {
    std::string object_name;
    uint32_t norad_cat_id;
};

void from_json(const json::json& j, catalogueEntry& e) {
    std::string tmpString;

    j.at("OBJECT_NAME").get_to(e.object_name);
    j.at("NORAD_CAT_ID").get_to(tmpString);
    e.norad_cat_id = std::stoi(tmpString);
}

int main() {
    // Initialise AFSPC libraries
    InitialiseLibs();

    // Set numerical precision of display
    std::cout.precision(std::numeric_limits<double>::max_digits10 - 2);

    // Import data from json file
    std::ifstream catalogueFile("catalogue.json");
    json::json catalogue;
    catalogueFile >> catalogue;

    std::vector<catalogueEntry> testData =
        catalogue.get<std::vector<catalogueEntry>>();

    for (const catalogueEntry entry : testData) {
        std::cout << entry.object_name << " " << entry.norad_cat_id
                  << std::endl;
    }

    // Open file for writing
    std::ofstream outputFile("test.dat", std::ios::binary);
    if (outputFile.good()) {
        // TLE lines
        std::string line1 =
            "1 25544U 98067A   21043.27289556  .00003336  00000-0  68749-4 0  "
            "9997";
        std::string line2 =
            "2 25544  51.6440 240.5646 0002850   6.0842 149.2030 "
            "15.48970429269234";

        PropagateAndWrite(outputFile, 101, 25544, 90.0 * 60.0, line1, line2);
    } else {
        throw std::runtime_error("Cannot open file for writing.");
    }
}