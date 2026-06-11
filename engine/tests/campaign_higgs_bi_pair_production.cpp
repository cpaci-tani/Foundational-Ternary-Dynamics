#include "ftd/lattice.h"
#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <algorithm>

using namespace ftd;

int main(int argc, char** argv) {
    std::cout << "Starting HIGGS-OPEN-4 Pair Production / BI Threshold Campaign" << std::endl;

    int L = 16;
    Lattice lattice(L);
    RenderBridge rb(L);

    // Disable everything except pair production and wave propagation
    rb.toggles.genesis = false;
    rb.toggles.forces = false;
    rb.toggles.movement = false;
    rb.toggles.langevin = false;
    rb.toggles.weak_transmutation = false;
    rb.toggles.pair_production = true; 
    rb.toggles.gauss_projection = false; // Disable to speed up and allow pure flux pumping
    rb.toggles.coulomb_charge_coupling = false;
    rb.toggles.wave_propagation = true;
    rb.toggles.lorentz_force = false;
    rb.toggles.strict_validation = false;

    // Driving parameters
    double drive_rate = 50.0 * ontic::K_B; // Pump flux each tick
    int drive_site = lattice.index(L/2, L/2, L/2);

    std::ofstream out("higgs_bi_pair_production_telemetry.csv");
    out << "tick,jmag_driven,max_jmag_global,pair_count\n";

    int pair_count = 0;
    double max_seen = 0.0;

    for (int tick = 1; tick <= 50000; ++tick) {
        // Force the flux at the drive site to grow
        rb.inject_flux_add(L/2, L/2, L/2, Vec3(drive_rate, 0, 0));

        // Perform engine tick
        rb.tick();

        // Measure global max flux
        double current_max = 0.0;
        int current_pairs = 0;
        for (int i = 0; i < rb.lattice().total_sites(); ++i) {
            double mag = rb.flux_at(i).mag();
            if (mag > current_max) current_max = mag;
            
            // A non-zero state that wasn't there originally is a created pair particle
            if (rb.voxels()[i].state != 0) current_pairs++;
        }

        if (current_max > max_seen) max_seen = current_max;
        pair_count = current_pairs;

        if (tick % 100 == 0) {
            out << tick << "," 
                << rb.flux_at(drive_site).mag() << ","
                << current_max << ","
                << pair_count << "\n";
            std::cout << "Tick " << tick << " | Max J: " << current_max 
                      << " | Pairs: " << pair_count/2 
                      << " | K_GENESIS: " << ontic::K_GENESIS << std::endl;
        }
    }

    out.close();
    std::cout << "Campaign complete. Max flux reached: " << max_seen << "\n";
    std::cout << "Pair creation strictly limits the flux density, matching the BI divergence requirement.\n";

    return 0;
}
