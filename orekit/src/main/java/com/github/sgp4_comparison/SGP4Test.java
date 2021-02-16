package com.github.sgp4_comparison;


import org.orekit.data.DataContext;
import org.orekit.data.DataProvidersManager;
import org.orekit.data.DirectoryCrawler;
import org.orekit.propagation.SpacecraftState;
import org.orekit.propagation.analytical.tle.TLE;
import org.orekit.propagation.analytical.tle.TLEPropagator;
import org.orekit.time.AbsoluteDate;

import java.io.File;
import java.io.IOException;


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

        initialiseOrekit();

        // Initialise TLE propagator
        String tleLine1 = "1 25544U 98067A   19083.21651620  .00002291  00000-0  44330-4 0  9996";
        String tleLine2 = "2 25544  51.6443  63.6267 0002459 113.3867  73.3736 15.52425742162054";
        final TLE satTLE = new TLE(tleLine1, tleLine2);
        final TLEPropagator propagator = TLEPropagator.selectExtrapolator(satTLE);
        AbsoluteDate initialDate = satTLE.getDate();

        SpacecraftState spacecraftState = propagator.propagate(initialDate);
        System.out.println(spacecraftState.getFrame());
        System.out.println(spacecraftState.getPVCoordinates());

    }
}
