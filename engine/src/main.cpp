/**
 * FTD Render-Bridge Simulation Engine — CLI entry point.
 *
 * Scenarios:
 *   A: Particle interaction (electron-proton attraction via flux)
 *   B: Pair production from high-energy flux pulse
 *   D: Locked particle stability test (default)
 *   E: Helium atom (2 locked protons + 2 orbiting electrons)
 *   F: Gravitational cluster (20 particles, pairwise gravity)
 *   G: Scale stress test (benchmark at specified lattice size)
 *   H: Helium atom with CSV export (density slices + timeseries)
 *   I: Interference pattern with CSV export (4-source, density heatmaps)
 *   J: Pair production with CSV export (counter-propagating beams)
 *   K: Force-law profile (does 1/r^2 emerge from grad(div(J))?)
 *
 * Scenario implementations live in engine/src/cli_demos/.
 *
 * Usage: ftd_sim [scenario] [lattice_size] [num_ticks] [outdir]
 */

#include <iostream>
#include <string>
#include <cstdlib>

#include "ftd/cli_demos.h"

int main(int argc, char* argv[]) {
    char scenario = 'D';
    int lattice_size = 32;
    int num_ticks = 2000;

    if (argc > 1) {
        char c = argv[1][0];
        if (c == 'A' || c == 'a') scenario = 'A';
        else if (c == 'B' || c == 'b') scenario = 'B';
        else if (c == 'E' || c == 'e') scenario = 'E';
        else if (c == 'F' || c == 'f') scenario = 'F';
        else if (c == 'G' || c == 'g') scenario = 'G';
        else if (c == 'H' || c == 'h') scenario = 'H';
        else if (c == 'I' || c == 'i') scenario = 'I';
        else if (c == 'J' || c == 'j') scenario = 'J';
        else if (c == 'K' || c == 'k') scenario = 'K';
        else scenario = 'D';
    }
    if (argc > 2) lattice_size = std::atoi(argv[2]);
    if (argc > 3) num_ticks = std::atoi(argv[3]);

    if (lattice_size < 4 || lattice_size > 256) {
        std::cerr << "Lattice size must be between 4 and 256\n";
        return 1;
    }

    ftd::cli_demos::print_header();

    // Output directory for CSV-exporting scenarios (4th arg or default)
    std::string outdir = "output";
    if (argc > 4) outdir = argv[4];

    using namespace ftd::cli_demos;
    switch (scenario) {
        case 'A': scenario_A(lattice_size, num_ticks); break;
        case 'B': scenario_B(lattice_size, num_ticks); break;
        case 'E': scenario_E(lattice_size, num_ticks); break;
        case 'F': scenario_F(lattice_size, num_ticks); break;
        case 'G': scenario_G(lattice_size, num_ticks); break;
        case 'H': scenario_H(lattice_size, num_ticks, outdir); break;
        case 'I': scenario_I(lattice_size, num_ticks, outdir); break;
        case 'J': scenario_J(lattice_size, num_ticks, outdir); break;
        case 'K': scenario_K(lattice_size, num_ticks, outdir); break;
        default:  scenario_default(lattice_size, num_ticks); break;
    }

    return 0;
}
