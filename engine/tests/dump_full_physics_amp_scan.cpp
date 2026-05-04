/**
 * Multi-amplitude scan under FULL PHYSICS at L=64.
 *
 * Map the amplitude landscape under proper full-physics methodology.
 * Single +x flux injection, 1 seed per amplitude (deterministic anyway).
 * Sweep A from genesis threshold (~1) to 16 in 0.5 steps.
 */
#include <iostream>
#include <iomanip>
#include <vector>
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"
#include "ftd/constants.h"

int main() {
    const int L = 64;
    std::cerr << "[full-amp-scan] L=" << L << ", full physics, +x flux, 200 ticks ..." << std::endl;

    std::vector<double> A_vals;
    for (double a = 1.0; a <= 16.01; a += 0.5) A_vals.push_back(a);

    std::cout << "{\n  \"runs\": [\n";
    bool first = true;
    for (double A : A_vals) {
        std::cerr << "  A=" << A << " ..." << std::flush;

        ftd::RenderBridge rb(L);
        rb.toggles.color_forces      = true;
        rb.toggles.strong_force      = true;
        rb.toggles.triad_binding     = true;
        rb.toggles.pair_production   = true;
        rb.toggles.exchange_force    = true;
        rb.toggles.latency_field     = true;
        rb.toggles.langevin          = true;
        rb.toggles.langevin_T        = 0.005;
        rb.toggles.langevin_gamma    = 0.02;
        rb.toggles.langevin_seed     = 1;

        const int c = L / 2;
        rb.inject_flux(c, c, c, {A * ftd::K_GENESIS, 0.0, 0.0});
        for (int t = 0; t < 200; ++t) rb.tick();

        const auto& vox = rb.voxels();
        const auto& lat = rb.lattice();
        int n=0, nR=0, nG=0, nB=0, nNone=0, nM=0, nA=0;
        for (int64_t i = 0; i < lat.total_sites(); ++i) {
            if (vox[i].state == 0) continue;
            ++n;
            if (vox[i].color == 1) ++nR;
            else if (vox[i].color == 2) ++nG;
            else if (vox[i].color == 3) ++nB;
            else ++nNone;
            if (vox[i].state > 0) ++nM; else ++nA;
        }
        std::cerr << " n=" << n << " (R=" << nR << " G=" << nG << " B=" << nB
                  << " none=" << nNone << " matter=" << nM << " anti=" << nA << ")" << std::endl;

        if (!first) std::cout << ",\n";
        std::cout << "    {\"A\":" << A << ",\"n_total\":" << n
                  << ",\"n_R\":" << nR << ",\"n_G\":" << nG << ",\"n_B\":" << nB
                  << ",\"n_none\":" << nNone
                  << ",\"n_matter\":" << nM << ",\"n_antimatter\":" << nA << "}";
        first = false;
    }
    std::cout << "\n  ]\n}\n";
    std::cerr << "[full-amp-scan] DONE" << std::endl;
    return 0;
}
