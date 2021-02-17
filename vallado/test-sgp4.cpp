
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>

// #include "MathTimeLib.h"
#include "SGP4.h"
#include "json.hpp"

// Rename the nlohmann namespace
namespace json = nlohmann;

// Number of points evaluated per orbital period
static const u_int32_t PTS_PER_PERIOD = 10;

// Julian Epoch for 1950 (used by AFSPC)
static const double JD50_EPOCH = 2433281.5;

// Convert date/time components into a Julian day.
// This is extracted from Vallado's MathTimeLib.
void jday(int year, int mon, int day, int hr, int minute, double sec,
          double& jd, double& jdFrac) {
    jd = 367.0 * year -
         std::floor((7 * (year + std::floor((mon + 9) / 12.0))) * 0.25) +
         std::floor(275 * mon / 9.0) + day + 1721013.5;
    jdFrac = (sec + minute * 60.0 + hr * 3600.0) / 86400.0;

    // check that the day and fractional day are correct
    if (std::abs(jdFrac) >= 1.0) {
        double dtt = std::floor(jdFrac);
        jd = jd + dtt;
        jdFrac = jdFrac - dtt;
    }
}

// A container for a catalogue entry.  Just keep the parts of an entry that we
// need for calculations.
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
    std::istringstream is(tmpString);
    T val;
    is >> val;
    return val;
}

void from_json(const json::json& j, catalogueEntry& e) {
    j.at("OBJECT_NAME").get_to(e.object_name);
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
                       const catalogueEntry& entry) {
    // Same for every sat
    char opsmode = 'i';
    const gravconsttype whichconst = wgs72;
    char sat_num[] = "xxxxx";

    // Extract date components of epoch
    int year = std::stoi(entry.epoch.substr(0, 4));
    int month = std::stoi(entry.epoch.substr(5, 2));
    int day = std::stoi(entry.epoch.substr(8, 2));
    int hour = std::stoi(entry.epoch.substr(11, 2));
    int minute = std::stoi(entry.epoch.substr(14, 2));
    double secs = std::stod(entry.epoch.substr(17, 9));

    // Year/date components
    double epoch_jd1, epoch_jd2;
    jday(year, month, day, hour, minute, secs, epoch_jd1, epoch_jd2);
    double jd50 = (epoch_jd1 - JD50_EPOCH) + epoch_jd2;

    // Initialise propagator
    elsetrec satrec;
    double mean_motion_dot =
        entry.mean_motion_dot * 2.0 * M_PI / (24.0 * 60.0) / (24.0 * 60.0);
    double mean_motion_ddot = entry.mean_motion_ddot * 2.0 * M_PI /
                              (24.0 * 60.0) / (24.0 * 60.0) / (24.0 * 60.0);
    double arg_perigee = entry.arg_of_pericenter * M_PI / 180.0;
    double inclination = entry.inclination * M_PI / 180.0;
    double mean_anomaly = entry.mean_anomaly * M_PI / 180.0;
    double mean_motion = entry.mean_motion * 2.0 * M_PI / (24.0 * 60.0);
    double raan = entry.ra_of_asc_node * M_PI / 180.0;

    SGP4Funcs::sgp4init(whichconst, opsmode, sat_num, jd50, entry.bstar,
                        mean_motion_dot, mean_motion_ddot, entry.eccentricity,
                        arg_perigee, inclination, mean_anomaly, mean_motion,
                        raan, satrec);
    if (satrec.error)
        throw std::runtime_error("sgp4init: SGP4 error number " +
                                 std::to_string(satrec.error));

    // Write the NORAD ID
    // std::cout << entry.norad_cat_id << std::endl;
    file.write(reinterpret_cast<const char*>(&entry.norad_cat_id),
               sizeof(uint32_t));

    // Propagate to individual time-steps from epoch
    for (int k = 0; k <= numSteps; ++k) {
        // SGP4 propagation using minutes since epoch
        double sse = double(k) * entry.period / double(numSteps);
        double pos[3];
        double vel[3];
        SGP4Funcs::sgp4(satrec, sse / 60.0, pos, vel);

        // Write time since epoch
        // std::cout << "  " << mse;
        file.write(reinterpret_cast<const char*>(&sse), sizeof(double));

        // Write position and velocity (in m and m/s)
        // for (int i = 0; i < 3; ++i) {
        //    std::cout << "  " << pos[i] * 1e3;
        /// }
        for (int i = 0; i < 3; ++i) {
            pos[i] *= 1e3;
        }
        file.write(reinterpret_cast<const char*>(pos), 3 * sizeof(double));
        // for (int i = 0; i < 3; ++i) {
        //     std::cout << "  " << vel[i] * 1e3;
        // }
        for (int i = 0; i < 3; ++i) {
            vel[i] *= 1e3;
        }
        file.write(reinterpret_cast<const char*>(vel), 3 * sizeof(double));
        // std::cout << std::endl;
    }
}

int main() {
    // Set numerical precision of display
    std::cout.precision(std::numeric_limits<double>::max_digits10 - 2);

    // Import data from json file
    std::ifstream catalogueFile("../catalogue.json");
    json::json catalogue;
    catalogueFile >> catalogue;

    std::vector<catalogueEntry> testData =
        catalogue.get<std::vector<catalogueEntry>>();
    uint32_t entryCount = testData.size();

    // Open file for writing
    std::ofstream outputFile("test.dat", std::ios::binary);
    if (outputFile.good()) {
        // Write the number of entries and the number of points per entry
        outputFile.write(reinterpret_cast<const char*>(&entryCount),
                         sizeof(uint32_t));
        outputFile.write(reinterpret_cast<const char*>(&PTS_PER_PERIOD),
                         sizeof(uint32_t));

        // Loop over each entry, propagating the TLE
        for (const catalogueEntry entry : testData) {
            PropagateAndWrite(outputFile, PTS_PER_PERIOD, entry);
        }
    } else {
        throw std::runtime_error("Cannot open file for writing.");
    }
}