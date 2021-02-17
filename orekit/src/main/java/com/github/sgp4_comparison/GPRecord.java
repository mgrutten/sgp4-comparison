package com.github.sgp4_comparison;

import com.squareup.moshi.Json;

/**
 * GPRecord
 * <p>
 * Class for JSON import from a space-track GP ephemeris record.
 * Just keep the parts that we need for comparison.
 */
public class GPRecord {
    // Individual orbital elements
    @Json(name = "OBJECT_NAME")
    private final String name;

    @Json(name = "NORAD_CAT_ID")
    private final int noradID;

    @Json(name = "BSTAR")
    private final double bstar;

    @Json(name = "MEAN_MOTION_DOT")
    private final double meanMotionDot;

    @Json(name = "MEAN_MOTION_DDOT")
    private final double meanMotionDDot;

    @Json(name = "MEAN_MOTION")
    private final double meanMotion;

    @Json(name = "ECCENTRICITY")
    private final double eccentricity;

    @Json(name = "INCLINATION")
    private final double inclination;

    @Json(name = "RA_OF_ASC_NODE")
    private final double raOfAscNode;

    @Json(name = "ARG_OF_PERICENTER")
    private final double argOfPericenter;

    @Json(name = "MEAN_ANOMALY")
    private final double meanAnomaly;

    @Json(name = "EPOCH")
    private final String epoch;

    // Orbital period (derived value)
    @Json(name = "PERIOD")
    private final double period;

    // Line 1 and 2 from TLE
    @Json(name = "TLE_LINE1")
    private final String tleLine1;

    @Json(name = "TLE_LINE2")
    private final String tleLine2;


    public GPRecord(String name, int noradID, double bstar, double meanMotionDot, double meanMotionDDot,
                    double meanMotion, double eccentricity, double inclination, double raOfAscNode,
                    double argOfPericenter, double meanAnomaly, String epoch, double period,
                    String tleLine1, String tleLine2) {
        this.name = name;
        this.noradID = noradID;
        this.bstar = bstar;
        this.meanMotionDot = meanMotionDot;
        this.meanMotionDDot = meanMotionDDot;
        this.meanMotion = meanMotion;
        this.eccentricity = eccentricity;
        this.inclination = inclination;
        this.raOfAscNode = raOfAscNode;
        this.argOfPericenter = argOfPericenter;
        this.meanAnomaly = meanAnomaly;
        this.epoch = epoch;
        this.period = period;
        this.tleLine1 = tleLine1;
        this.tleLine2 = tleLine2;
    }

    public String getName() {
        return name;
    }

    public int getNoradID() {
        return noradID;
    }

    public double getBstar() {
        return bstar;
    }

    public double getMeanMotionDot() {
        return meanMotionDot;
    }

    public double getMeanMotionDDot() {
        return meanMotionDDot;
    }

    public double getMeanMotion() {
        return meanMotion;
    }

    public double getEccentricity() {
        return eccentricity;
    }

    public double getInclination() {
        return inclination;
    }

    public double getRaOfAscNode() {
        return raOfAscNode;
    }

    public double getArgOfPericenter() {
        return argOfPericenter;
    }

    public double getMeanAnomaly() {
        return meanAnomaly;
    }

    public String getEpoch() {
        return epoch;
    }

    public double getPeriod() {
        return period;
    }

    public String getTleLine1() {
        return tleLine1;
    }

    public String getTleLine2() {
        return tleLine2;
    }

}
