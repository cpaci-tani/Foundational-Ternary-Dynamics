#include "ftd/lattice.h"
#include "ftd/render_bridge.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>

using namespace ftd;

int main() {
    int L = 16;
    Lattice lattice(L);
    RenderBridge rb(lattice.size());
    
    // Enable relevant physics
    rb.toggles.wave_propagation = true;
    rb.toggles.genesis = true;
    rb.toggles.movement = true;
    rb.toggles.forces = true;
    rb.toggles.strict_validation = false;

    std::ofstream out("ew_phase_transition_telemetry.csv");
    out << "drive_D,mean_J,mean_s,phase\n";

    double max_J_target = 5.0 * ontic::K_B * ontic::N_C; // We sweep up to J = 15 K_B
    double alpha = 1.0 / 137.036;
    double max_D = max_J_target * alpha; // Steady state D/alpha = max_J_target

    int num_steps = 100;
    
    // Function to run a step and measure
    auto measure_step = [&](double D, const std::string& phase) {
        // Equilibrate
        for (int t = 0; t < 100; ++t) {
            // Inject uniform background flux D
            for(int x=0; x<L; ++x) {
                for(int y=0; y<L; ++y) {
                    for(int z=0; z<L; ++z) {
                        rb.inject_flux_add(x, y, z, Vec3(D, 0, 0));
                    }
                }
            }
            rb.tick();
        }
        
        // Measure over 50 ticks
        double total_s = 0.0;
        double total_J = 0.0;
        int count = 0;
        
        for (int t = 0; t < 50; ++t) {
            for(int x=0; x<L; ++x) {
                for(int y=0; y<L; ++y) {
                    for(int z=0; z<L; ++z) {
                        rb.inject_flux_add(x, y, z, Vec3(D, 0, 0));
                    }
                }
            }
            rb.tick();
            
            for(int x=0; x<L; ++x) {
                for(int y=0; y<L; ++y) {
                    for(int z=0; z<L; ++z) {
                        total_s += std::abs(rb.state_at(lattice.index(x,y,z)));
                        total_J += rb.flux_at(lattice.index(x,y,z)).mag();
                        count++;
                    }
                }
            }
        }
        
        double mean_s = total_s / count;
        double mean_J = total_J / count;
        
        out << D << "," << mean_J << "," << mean_s << "," << phase << "\n";
        std::cout << phase << " | D: " << D << " | Mean J: " << mean_J 
                  << " | Mean |s|: " << mean_s << "\n";
    };

    std::cout << "Starting Sweep UP...\n";
    for (int step = 0; step <= num_steps; ++step) {
        double D = max_D * step / num_steps;
        measure_step(D, "UP");
    }

    std::cout << "Starting Sweep DOWN...\n";
    for (int step = num_steps; step >= 0; --step) {
        double D = max_D * step / num_steps;
        measure_step(D, "DOWN");
    }

    out.close();
    std::cout << "Campaign complete.\n";
    return 0;
}
