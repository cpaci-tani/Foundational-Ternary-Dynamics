/**
 * campaign_alpha_readout_scattering.cpp
 * ARC-D1 Empirical Readout Campaign
 * 
 * Injects a stable cluster (A=14) and applies a minimal flux perturbation (delta=0.5).
 * Sweeps over thousands of random seeds to measure the integer branching ratio
 * between non-radiative elastic scattering (cluster survives intact) and
 * radiative fission (cluster breaks apart).
 *
 * If this ratio inherently scales with 137.036, it provides a geometric
 * discrete-native derivation of the fine structure constant.
 */
#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "ftd/cluster_genealogy.h"
#include <iostream>
#include <fstream>
#include <vector>

using namespace ftd;

static const int WARMUP = 40;
static const int RUN = 200;
static const int STRIDE = 4;

static void quiescent_config(RenderBridge& rb, double T, unsigned seed) {
    rb.force_cpu();
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.genesis          = true;
    rb.toggles.langevin         = true;
    rb.toggles.langevin_T       = T;
    rb.toggles.langevin_gamma   = 0.02;
    rb.toggles.langevin_seed    = seed;
    rb.seed_rng(seed);
}

int main(int argc, char** argv) {
    int L = 64;
    int num_seeds = 1000;
    
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--L" && i + 1 < argc) L = std::atoi(argv[++i]);
        if (a == "--seeds" && i + 1 < argc) num_seeds = std::atoi(argv[++i]);
    }

    std::cout << "================================================================\n";
    std::cout << "  ARC-D1 Alpha Readout Campaign (Empirical Measurement)\n";
    std::cout << "  L = " << L << " | seeds = " << num_seeds << "\n";
    std::cout << "================================================================\n";

    double A = 14.0; // Known stable soliton amplitude
    double delta = 0.5; // Minimal flux perturbation driver
    double T = 0.005;

    int n_fission = 0;
    int n_elastic = 0;
    int n_death = 0;

    std::cout << "Running massive Monte Carlo sweep...\n";

    for (int s = 0; s < num_seeds; ++s) {
        unsigned seed = 0xFEEDBEEF + static_cast<unsigned>(s);
        
        RenderBridge rb(L);
        quiescent_config(rb, T, seed);
        
        int cx = L / 2, cy = L / 2, cz = L / 2;
        rb.inject_flux(cx, cy, cz, {A * K_GENESIS, 0.0, 0.0});
        
        for (int t = 0; t < WARMUP; ++t) rb.tick();
        
        // Quadrupole dent to induce scattering/fission
        rb.inject_flux_add(cx + 2, cy, cz, {+delta * K_GENESIS, 0.0, 0.0});
        rb.inject_flux_add(cx - 2, cy, cz, {-delta * K_GENESIS, 0.0, 0.0});
        
        ClusterGenealogyTracker g;
        g.record(rb);
        
        for (int t = 1; t <= RUN; ++t) { 
            rb.tick(); 
            if (t % STRIDE == 0) g.record(rb); 
        }
        
        int fissions = g.count(EventType::Fission);
        int deaths = g.count(EventType::Death);
        
        if (deaths > 0) {
            n_death++;
        } else if (fissions > 0) {
            n_fission++;
        } else {
            n_elastic++;
        }
        
        if ((s + 1) % 100 == 0) {
            std::cout << "  Progress: " << (s + 1) << " / " << num_seeds << "\n";
            if (n_fission > 0) {
                double R_fission = static_cast<double>(n_elastic) / n_fission;
                std::cout << "  [Current R_fission = " << R_fission << "]\n";
            }
        }
    }

    std::cout << "\n================================================================\n";
    std::cout << "  RESULTS\n";
    std::cout << "  Elastic Bounces:  " << n_elastic << "\n";
    std::cout << "  Fissions (Split): " << n_fission << "\n";
    std::cout << "  Annihilations:    " << n_death << "\n";
    
    if (n_fission > 0) {
        double R_fission = static_cast<double>(n_elastic) / n_fission;
        std::cout << "  R_fission (Elastic/Fission) = " << R_fission << "\n";
        std::cout << "  Target value (1/alpha):     ~137.036\n";
    } else {
        std::cout << "  R_fission = UNDEFINED (no fissions occurred)\n";
    }
    std::cout << "================================================================\n";

    return 0;
}
