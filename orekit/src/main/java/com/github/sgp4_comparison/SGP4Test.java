package com.github.sgp4_comparison;

import com.squareup.moshi.JsonAdapter;
import com.squareup.moshi.Moshi;
import com.squareup.moshi.Types;
import okio.BufferedSource;
import okio.Okio;
import okio.Source;
import org.hipparchus.geometry.euclidean.threed.Vector3D;
import org.orekit.data.DataContext;
import org.orekit.data.DataProvidersManager;
import org.orekit.data.DirectoryCrawler;
import org.orekit.propagation.analytical.tle.TLE;
import org.orekit.propagation.analytical.tle.TLEPropagator;
import org.orekit.time.AbsoluteDate;
import org.orekit.time.TimeScalesFactory;
import org.orekit.utils.PVCoordinates;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.channels.FileChannel;
import java.util.List;


public class SGP4Test {

    /**
     * Number of points evaluated per orbital period
     */
    private static final int PTS_PER_PERIOD = 11;

    /**
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
     * Parse the GP catalogue in JSON format.  Leverages the Moshi JSON library.
     *
     * @param catalogueFile A java File object containing the input file
     * @return the catalogue as a list of GPRecords
     * @throws IOException when the input file is unavailable
     */
    public static List<GPRecord> parseCatalogue(final File catalogueFile) throws IOException {
        Moshi moshi = new Moshi.Builder().build();
        JsonAdapter<List<GPRecord>> jsonAdapter = moshi.adapter(Types.newParameterizedType(List.class, GPRecord.class));

        List<GPRecord> gpRecords;
        try (Source fileSource = Okio.source(catalogueFile);
             BufferedSource bufferedSource = Okio.buffer(fileSource)) {

            gpRecords = jsonAdapter.fromJson(bufferedSource);

        }
        if (gpRecords == null) {
            throw new RuntimeException("No valid records found in " + catalogueFile.getCanonicalPath());
        }

        return gpRecords;
    }

    /**
     * Propagate an orbit using SGP4 and write results to a file.
     *
     * @param gpRecord      The catalogue record containing orbital elements
     * @param outputChannel The output file to write (binary, not text) results
     */
    public static void propagateAndWrite(final GPRecord gpRecord, final FileChannel outputChannel) {

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

        // Initialse propagator
        final TLE satTLE = new TLE(gpRecord.getNoradID(), classification, launchYear, launchNumber, launchPiece,
                ephemerisType, elementNumber, epoch, meanMotion, meanMotionDot, meanMotionDDot, eccentricity,
                inclination, argOfPericenter, raOfAscNode, meanAnomaly, revNumber, bstar);
        final TLEPropagator tlePropagator = TLEPropagator.selectExtrapolator(satTLE);

        // Write sat ID
        try {
            ByteBuffer buffer = ByteBuffer.allocate(Integer.BYTES);
            buffer.order(ByteOrder.nativeOrder());

            buffer.putInt(gpRecord.getNoradID());

            buffer.flip();
            outputChannel.write(buffer);
        } catch (IOException e) {
            throw new RuntimeException("Cannot write to file");
        }

        // Loop over orbital period
        double period = gpRecord.getPeriod() * 60.0;
        for (int k = 0; k <= PTS_PER_PERIOD; ++k) {

            // Propagate
            double secondsSinceEpoch = (double) k * period / (double) PTS_PER_PERIOD;
            PVCoordinates state = tlePropagator.getPVCoordinates(epoch.shiftedBy(secondsSinceEpoch));

            // Allocate a buffer
            ByteBuffer buffer = ByteBuffer.allocate(7 * Double.BYTES);
            buffer.order(ByteOrder.nativeOrder());

            // Write time since epoch
            buffer.putDouble(secondsSinceEpoch);

            // Write position
            Vector3D position = state.getPosition();
            buffer.putDouble(position.getX());
            buffer.putDouble(position.getY());
            buffer.putDouble(position.getZ());

            // Write velocity
            Vector3D velocity = state.getVelocity();
            buffer.putDouble(velocity.getX());
            buffer.putDouble(velocity.getY());
            buffer.putDouble(velocity.getZ());

            // Transfer buffer to output stream
            try {
                buffer.flip();
                outputChannel.write(buffer);
            } catch (IOException e) {
                throw new RuntimeException("Cannot write to file");
            }

        }

    }

    /**
     * The entry point for the SGP4 implementation test.
     *
     * @param args the command-line arguments supplied to the program.
     * @throws IOException if the orekit data directory cannot be found
     */
    public static void main(String[] args) throws IOException {

        // Extract input arguments
        String catalogueFileName = args[0];
        String outputFileName = args[1];

        // Initialise orekit
        initialiseOrekit();

        // Parse input file
        File catalogueFile = new File(catalogueFileName);
        List<GPRecord> gpRecords = parseCatalogue(catalogueFile);

        // Initialise output stream
        try (FileOutputStream fileStream = new FileOutputStream(outputFileName);
             FileChannel outputChannel = fileStream.getChannel()) {

            // Write number of objects and number of points per period
            ByteBuffer buffer = ByteBuffer.allocate(2 * Integer.BYTES);
            buffer.order(ByteOrder.nativeOrder());

            buffer.putInt(gpRecords.size());
            buffer.putInt(PTS_PER_PERIOD);

            buffer.flip();
            outputChannel.write(buffer);

            // Loop over all GP records
            gpRecords.forEach(record -> propagateAndWrite(record, outputChannel));

        }


    }
}
