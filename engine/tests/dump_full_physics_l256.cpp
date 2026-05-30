/**
 * L=256 full-physics spot check — does the FTD-framework-integer pattern continue?
 *
 * Cost-reference-frame: 3 axes × 1 seed × 100 ticks. L=256 has 16M voxels;
 * each tick at L=256 is ~16x slower than L=64.
 */
#include <iostream>
#include <iomanip>
#include <vector>
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"
#include "ftd/constants.h"

int main() {
    const int L = 256;
    std::cerr << "[full-L256] Running 3 axes at L=256, full physics, 100 ticks ..." << std::endl;

    std::cout << "{\n  \"runs\": [\n";
    bool first = true;
    for (char axis : std::vector<char>{'x', 'y', 'z'}) {
        std::cerr << "  axis=" << axis << " ..." << std::flush;

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
        const double A = 5.0 * ftd::K_GENESIS;
        double fx = 0.0, fy = 0.0, fz = 0.0;
        if (axis == 'x') fx = A;
        else if (axis == 'y') fy = A;
        else fz = A;
        rb.inject_flux(c, c, c, {fx, fy, fz});
        for (int t = 0; t < 100; ++t) rb.tick();

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
        std::cout << "    {\"L\":256,\"axis\":\"" << axis << "\",\"seed\":1,\"n_total\":" << n
                  << ",\"n_R\":" << nR << ",\"n_G\":" << nG << ",\"n_B\":" << nB
                  << ",\"n_none\":" << nNone
                  << ",\"n_matter\":" << nM << ",\"n_antimatter\":" << nA << "}";
        first = false;
    }
    std::cout << "\n  ]\n}\n";
    std::cerr << "[full-L256] DONE" << std::endl;
    return 0;
}
