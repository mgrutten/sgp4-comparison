package com.github.sgp4_comparison;

import com.squareup.moshi.JsonAdapter;
import com.squareup.moshi.Moshi;
import com.squareup.moshi.Types;
import okio.BufferedSource;
import okio.Okio;
import okio.Source;
import org.orekit.data.DataContext;
import org.orekit.data.DataProvidersManager;
import org.orekit.data.DirectoryCrawler;
import org.orekit.propagation.SpacecraftState;
import org.orekit.propagation.analytical.tle.TLE;
import org.orekit.propagation.analytical.tle.TLEPropagator;
import org.orekit.time.AbsoluteDate;
import org.orekit.time.TimeScalesFactory;

import java.io.File;
import java.io.IOException;
import java.util.List;


public class SGP4Test {

    /**
     * initialiseOrekit
     * <p>
     * Initialise orekit time transformations, using data in the "orekit-data-master" directory
     * in the user's home directory.
     *
     * @throws IOException if the orekit data directory cannot be found
     */
    public static void initialiseOrekit() throws IOException {
        // Set up orekit
        final File home = new File(System.getProperty("user.home"));
        final File orekitData = new File(home, "orekit-data-master");
        if (!orekitData.exists()) {
            throw new IOException("Could not find orekit data");
        }
        final DataProvidersManager manager = DataContext.getDefault().getDataProvidersManager();
        manager.addProvider(new DirectoryCrawler(orekitData));

    }

    /**
     * main
     * <p>
     * The entry point for the SGP implementation test.
     *
     * @param args the command-line arguments supplied to the program.
     * @throws IOException if the orekit data directory cannot be found
     */
    public static void main(String[] args) throws IOException {

        // Parse the input file
        String catalogueFileName = args[0];
        File catalogueFile = new File(catalogueFileName);

        Moshi moshi = new Moshi.Builder().build();
        JsonAdapter<List<GPRecord>> jsonAdapter = moshi.adapter(Types.newParameterizedType(List.class, GPRecord.class));

        List<GPRecord> gpRecords;
        try (Source fileSource = Okio.source(catalogueFile);
             BufferedSource bufferedSource = Okio.buffer(fileSource)) {

            gpRecords = jsonAdapter.fromJson(bufferedSource);

        }
        if (gpRecords == null) {
            throw new RuntimeException("No valid records found in " + catalogueFileName);
        }
        System.out.println(gpRecords.size());


        // Initialise orekit
        initialiseOrekit();

        // Loop over all GP records
        for (final GPRecord gpRecord : gpRecords) {

            // Values same for each object
            char classification = 'U';
            int launchYear = 0;
            int launchNumber = 1;
            String launchPiece = "A";
            int ephemerisType = 0;
            int elementNumber = 1;
            int revNumber = 1;

            // Transform elements to units used by orekit
            AbsoluteDate epoch = new AbsoluteDate(gpRecord.getEpoch(), TimeScalesFactory.getUTC());
            // GP data is in revs/day -> rad/s
            double meanMotion = gpRecord.getMeanMotion() * 2.0 * Math.PI / (24.0 * 60.0 * 60.0);
            // GP is revs/day**2 / 2 -> rad/s**2
            double meanMotionDot = gpRecord.getMeanMotionDot() * 2.0 * Math.PI / Math.pow(24.0 * 60.0 * 60.0, 2.0) * 2.0;
            // GP is revs/day**3 / 6 -> rad/s**3
            double meanMotionDDot = gpRecord.getMeanMotionDDot() * 2.0 * Math.PI / Math.pow(24.0 * 60.0 * 60.0, 3.0) * 6.0;
            double eccentricity = gpRecord.getEccentricity();
            double inclination = Math.toRadians(gpRecord.getInclination());
            double argOfPericenter = Math.toRadians(gpRecord.getArgOfPericenter());
            double raOfAscNode = Math.toRadians(gpRecord.getRaOfAscNode());
            double meanAnomaly = Math.toRadians(gpRecord.getMeanAnomaly());
            double bstar = gpRecord.getBstar();

            final TLE satTLE = new TLE(gpRecord.getNoradID(), classification, launchYear, launchNumber, launchPiece,
                    ephemerisType, elementNumber, epoch, meanMotion, meanMotionDot, meanMotionDDot, eccentricity,
                    inclination, argOfPericenter, raOfAscNode, meanAnomaly, revNumber, bstar);
            final TLEPropagator tlePropagator = TLEPropagator.selectExtrapolator(satTLE);

            SpacecraftState spacecraftState = tlePropagator.propagate(epoch);
            System.out.println(spacecraftState.getPVCoordinates());

        }

    }
}
