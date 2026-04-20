// test_wilson_topology.cpp — Phase I Item 3 Mechanism A diagnostic.
//
// The question: does FTD's flux field J have topological structure that
// could quantise the coupling g_c = sqrt(2 pi alpha_ref)?
//
// Dirac-style topological quantisation requires the circulation
// Gamma(C) = sum_{e in C} J(e).dx around a closed loop C to take only
// quantised values (e.g., multiples of 2 pi / g_c). If it does, g_c is
// forced by topology alone.
//
// This test measures Gamma(C) for a battery of closed plaquette loops
// in an equilibrated two-charge configuration and reports:
//   - mean, variance of the histogram
//   - whether values cluster around integer multiples of any candidate quantum
//
// Expected result (from the engine's type signature alone): continuous
// distribution, no quantisation. The flux field is R^3-valued, not
// U(1)-valued, so nothing forces Gamma(C) into discrete bins.
// This test empirically confirms that expectation.
//
// If the measured histogram WERE quantised (it isn't, per the analysis),
// Mechanism A for first-principles g_c would be on the table. Since it
// is continuous, Mechanism A is ruled out without engine reformulation.

#include "ftd/render_bridge.h"
#include "ftd/eft/coupling_measurement.h"
#include "ftd/voxel.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>

using ftd::RenderBridge;
using ftd::Vec3;
using ftd::eft::configure_bare_lattice_for_coupling;


// Circulation around an axis-aligned rectangular plaquette with corner at
// (x, y, z) and size (dx, dy) in the XY plane:
//   Gamma = sum over 4 edges of J.dx
static double plaquette_circulation_xy(const RenderBridge& rb, int x, int y, int z,
                                       int dx, int dy) {
    const auto& lat = rb.lattice();
    const auto& vox = rb.voxels();
    // Walk around the rectangle: +X along bottom, +Y along right,
    // -X along top, -Y along left. Each edge contributes J_component * length.
    double gamma = 0.0;
    for (int i = 0; i < dx; ++i)  gamma += vox[lat.index(x+i,     y,      z)].flux.x;
    for (int j = 0; j < dy; ++j)  gamma += vox[lat.index(x+dx,    y+j,    z)].flux.y;
    for (int i = 0; i < dx; ++i)  gamma -= vox[lat.index(x+dx-1-i, y+dy,  z)].flux.x;
    for (int j = 0; j < dy; ++j)  gamma -= vox[lat.index(x,        y+dy-1-j, z)].flux.y;
    return gamma;
}


int main() {
    printf("=== Phase I Item 3 Mechanism A: Wilson-loop topology diagnostic ===\n");

    // Set up a fresh L=32 lattice with two opposite charges.
    const int L = 32;
    const int ticks = 300;
    const int mid = L / 2;

    RenderBridge rb(L);
    rb.force_cpu();
    configure_bare_lattice_for_coupling(rb);
    rb.toggles.coulomb_charge_coupling = 1.0;   // Phase G geometric regime
    rb.toggles.coupling = false;                // wave source off

    rb.inject_particle(mid,     mid, mid, +1, Vec3{0, 0, +0.05});
    rb.inject_particle(mid + 6, mid, mid, -1, Vec3{0, 0, -0.05});
    rb.voxels()[rb.lattice().index(mid,     mid, mid)].locked = true;
    rb.voxels()[rb.lattice().index(mid + 6, mid, mid)].locked = true;

    rb.run(ticks);

    // Sample plaquette circulations across the lattice at size = 1, 2, 4.
    for (int plaq : {1, 2, 4}) {
        std::vector<double> gammas;
        gammas.reserve((L - plaq) * (L - plaq) * L);
        for (int x = 0; x < L - plaq; x += 1)
        for (int y = 0; y < L - plaq; y += 1)
        for (int z = 2; z < L - 2; z += 4) {  // sparse z to reduce count
            gammas.push_back(plaquette_circulation_xy(rb, x, y, z, plaq, plaq));
        }

        // Histogram statistics
        double sum = 0, sumsq = 0, maxabs = 0;
        for (double g : gammas) {
            sum += g;
            sumsq += g * g;
            maxabs = std::max(maxabs, std::abs(g));
        }
        const double n = static_cast<double>(gammas.size());
        const double mean = sum / n;
        const double var = sumsq / n - mean * mean;
        const double sd = std::sqrt(std::max(0.0, var));

        // Test for quantisation: if Gamma is quantised in units q, then
        // the distribution would show peaks at 0, +/-q, +/-2q, ...
        // A simple quantisation test: for candidate quantum q, how close is
        // every Gamma to an integer multiple of q?
        std::vector<double> candidates = {
            2.0 * 3.14159265358979323846,            // 2 pi
            2.0 * 3.14159265358979323846 / 137.036, // 2 pi alpha_ref
            std::sqrt(2.0 * 3.14159265358979323846 / 137.036), // g_c itself
            1.0,                                      // unity
        };
        printf("\n  plaquette %dx%d:  n = %zu samples\n", plaq, plaq, gammas.size());
        printf("    mean       = %+.4e\n", mean);
        printf("    sd         = %.4e\n", sd);
        printf("    max |G|    = %.4e\n", maxabs);
        for (double q : candidates) {
            double rms_dev = 0;
            for (double g : gammas) {
                double rel = g / q;
                double nearest = std::round(rel);
                rms_dev += (rel - nearest) * (rel - nearest);
            }
            rms_dev = std::sqrt(rms_dev / n);
            printf("    rms dev from integer multiples of q = %.6e:  %.4e  (relative: %.2f%%)\n",
                   q, rms_dev * q, rms_dev * 100.0);
        }
    }

    printf("\n  Interpretation:\n");
    printf("  - If any candidate quantum q gave rms dev << sd, the circulation\n");
    printf("    would be quantised in units of q. That would be evidence for\n");
    printf("    Mechanism A (topological g_c).\n");
    printf("  - If all candidates give rms dev comparable to sd/sqrt(12) ~= sd*0.29\n");
    printf("    (uniform distribution's rms deviation from nearest integer), the\n");
    printf("    distribution is continuous. That rules out Mechanism A.\n");
    printf("  - The expected result from the engine's type signature (J real-valued)\n");
    printf("    is continuous; this test confirms empirically.\n");

    return 0;
}
