
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>

#include "json.hpp"

// Library headers
namespace afspc {
extern "C" {
#include "AstroFunc.h"
#include "DllMain.h"
#include "EnvConst.h"
#include "Sgp4Prop.h"
#include "TimeFunc.h"
#include "Tle.h"
}
}  // namespace afspc

namespace json = nlohmann;

// Length of messages from AFSPC libraries
static const u_int16_t LOGMSGLEN = 128;

// Number of points evaluated per orbital period
static const u_int32_t PTS_PER_PERIOD = 11;

// Use TLE (true) or individual GP elements (false)
static const bool USE_TLE = false;

// Extract and show an error message to stdout
void ShowErrMsg() {
    char errMsg[LOGMSGLEN];

    afspc::GetLastErrMsg(errMsg);
    errMsg[LOGMSGLEN - 1] = 0;
    // printf("%s\n", errMsg);

    std::cerr << errMsg << std::endl;
}

// Extract and show an error message to stdout
void ThrowErrMsg() {
    char errMsg[LOGMSGLEN];

    afspc::GetLastErrMsg(errMsg);
    errMsg[LOGMSGLEN - 1] = 0;
    // printf("%s\n", errMsg);

    throw std::runtime_error(errMsg);
}

// Initialise the AFSPC libraries
void InitialiseLibs() {
    uint64_t apPtr;

    // Inialise Main library
    apPtr = afspc::DllMainInit();

    // Initialise Env library
    if (afspc::EnvInit(apPtr) != 0) ThrowErrMsg();

    // Initialise TimeFunc library
    if (afspc::TimeFuncInit(apPtr) != 0) ThrowErrMsg();

    // Initialise AstroFunc library
    if (afspc::AstroFuncInit(apPtr) != 0) ThrowErrMsg();

    // Initialise Tle library
    if (afspc::TleInit(apPtr) != 0) ThrowErrMsg();

    // Initialise Sgp4Prop library
    if (afspc::Sgp4Init(apPtr) != 0) ThrowErrMsg();
}

// a simple struct to model a person
struct catalogueEntry {
    std::string object_name;
    uint32_t norad_cat_id;
    double bstar;
    double mean_motion_dot;
    double mean_motion_ddot;
    double mean_motion;
    double eccentricity;
    double inclination;
    double ra_of_asc_node;
    double arg_of_pericenter;
    double mean_anomaly;
    std::string epoch;

    double period;

    std::string line1;
    std::string line2;
};

template <typename T>
T GetFromString(const json::json& j, const std::string label) {
    std::string tmpString = j.at(label).get<std::string>();
    // j.at(label).get_to(tmpString);
    std::istringstream is(tmpString);
    T val;
    is >> val;
    return val;
}

void from_json(const json::json& j, catalogueEntry& e) {
    j.at("OBJECT_NAME").get_to(e.object_name);
    // j.at("NORAD_CAT_ID").get_to(tmpString);
    e.norad_cat_id = GetFromString<uint32_t>(j, "NORAD_CAT_ID");
    e.bstar = GetFromString<double>(j, "BSTAR");
    e.mean_motion_dot = GetFromString<double>(j, "MEAN_MOTION_DOT");
    e.mean_motion_ddot = GetFromString<double>(j, "MEAN_MOTION_DDOT");
    e.mean_motion = GetFromString<double>(j, "MEAN_MOTION");
    e.eccentricity = GetFromString<double>(j, "ECCENTRICITY");
    e.inclination = GetFromString<double>(j, "INCLINATION");
    e.ra_of_asc_node = GetFromString<double>(j, "RA_OF_ASC_NODE");
    e.arg_of_pericenter = GetFromString<double>(j, "ARG_OF_PERICENTER");
    e.mean_anomaly = GetFromString<double>(j, "MEAN_ANOMALY");
    j.at("EPOCH").get_to(e.epoch);

    e.period = GetFromString<double>(j, "PERIOD") * 60.0;

    j.at("TLE_LINE1").get_to(e.line1);
    j.at("TLE_LINE2").get_to(e.line2);
}

//     file: output file
// numSteps: number of propagation steps to make
void PropagateAndWrite(std::ostream& file, const int numSteps,
                       const catalogueEntry& entry, const bool fromTLE) {
    // Load the TLE
    int64_t satkey;
    if (fromTLE) {
        // Initialise TLE
        satkey =
            afspc::TleAddSatFrLines(const_cast<char*>(entry.line1.c_str()),
                                    const_cast<char*>(entry.line2.c_str()));

    } else {
        // Same for every sat
        char classification = 'x';
        char sat_name[] = "xxxxxxxx";
        int ephemeris_type = 0;
        int elset_num = 1;
        int rev_num = 1;

        // Extract date components of epoch
        int year = std::stoi(entry.epoch.substr(0, 4));
        int month = std::stoi(entry.epoch.substr(5, 2));
        int day = std::stoi(entry.epoch.substr(8, 2));
        int hour = std::stoi(entry.epoch.substr(11, 2));
        int minute = std::stoi(entry.epoch.substr(14, 2));
        double secs = std::stod(entry.epoch.substr(17, 9));

        // Epoch in AFSPC format
        double ds50UTC =
            afspc::TimeComps2ToUTC(year, month, day, hour, minute, secs);

        // Year/date components
        int epoch_yr;
        double epoch_days;
        afspc::UTCToYrDays(ds50UTC, &epoch_yr, &epoch_days);

        // Initialse TLE entry
        satkey = afspc::TleAddSatFrFieldsGP2(
            entry.norad_cat_id, classification, sat_name, epoch_yr, epoch_days,
            entry.bstar, ephemeris_type, elset_num, entry.inclination,
            entry.ra_of_asc_node, entry.eccentricity, entry.arg_of_pericenter,
            entry.mean_anomaly, entry.mean_motion, rev_num,
            entry.mean_motion_dot, entry.mean_motion_ddot);
    }
    if (satkey <= 0) ThrowErrMsg();

    // Initialize the loaded TLE before it can be propagated (see Sgp4Prop dll
    // document) This is important!!!
    if (afspc::Sgp4InitSat(satkey) != 0) ThrowErrMsg();

    // Write the NORAD ID
    // std::cout << entry.norad_cat_id << std::endl;
    file.write(reinterpret_cast<const char*>(&entry.norad_cat_id),
               sizeof(uint32_t));

    // Propagate to individual time-steps from epoch
    for (int k = 0; k <= numSteps; ++k) {
        // SGP4 propagation using minutes since epoch
        double mse = double(k) * entry.period / double(numSteps);
        double ds50UTC;
        double llh[3];
        double pos[3];
        double vel[3];
        if (afspc::Sgp4PropMse(satkey, mse, &ds50UTC, pos, vel, llh) != 0) {
            ShowErrMsg();
            for (int i = 0; i < 3; ++i) {
                pos[i] = NAN;
                vel[i] = NAN;
            }
        }

        // Write time since epoch
        // std::cout << "  " << mse;
        file.write(reinterpret_cast<const char*>(&mse), sizeof(double));

        // Write position and velocity (in m and m/s)
        // for (int i = 0; i < 3; ++i) {
        //    std::cout << "  " << pos[i] * 1e3;
        /// }
        file.write(reinterpret_cast<const char*>(pos), 3 * sizeof(double));
        // for (int i = 0; i < 3; ++i) {
        //     std::cout << "  " << vel[i] * 1e3;
        // }
        file.write(reinterpret_cast<const char*>(vel), 3 * sizeof(double));
        // std::cout << std::endl;
    }

    // Remove initialized TLE from memory
    if (afspc::Sgp4RemoveSat(satkey) != 0) ThrowErrMsg();

    // Remove loaded TLE from memory
    if (afspc::TleRemoveSat(satkey) != 0) ThrowErrMsg();
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

    // Open file for writing
    std::ofstream outputFile("test.dat", std::ios::binary);
    if (outputFile.good()) {
        // Loop over each entry, propagating the TLE
        for (const catalogueEntry entry : testData) {
            PropagateAndWrite(outputFile, PTS_PER_PERIOD, entry, USE_TLE);
        }
    } else {
        throw std::runtime_error("Cannot open file for writing.");
    }
}