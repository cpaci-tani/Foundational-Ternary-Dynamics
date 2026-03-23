/**
 * Compare inner radial profiles across damping modes.
 * Question: are r_50=5 and r_90=17 intrinsic, or damping artifacts?
 */
#include <cmath>
#include <cstdio>
#include <vector>
#include "ftd/gpu_engine.h"
#include "ftd/constants.h"

using namespace ftd;
using namespace ftd::gpu;

struct Profile {
    double avg_flux[61] = {};
    double cum_energy[61] = {};
    int count[61] = {};
    double total_E = 0;
};

Profile measure(const std::vector<Voxel>& v, int L, int cx, int cy, int cz) {
    Profile p;
    for (int x = 0; x < L; ++x)
      for (int y = 0; y < L; ++y)
        for (int z = 0; z < L; ++z) {
            double dx=x-cx, dy=y-cy, dz=z-cz;
            int ri = (int)(std::sqrt(dx*dx+dy*dy+dz*dz) + 0.5);
            if (ri > 60) ri = 60;
            int idx = x*L*L + y*L + z;
            double j2 = v[idx].flux.mag2();
            p.avg_flux[ri] += std::sqrt(j2);
            p.cum_energy[ri] += 0.5*j2;
            p.count[ri]++;
            p.total_E += 0.5*j2;
        }
    for (int r = 0; r <= 60; ++r)
        if (p.count[r] > 0) p.avg_flux[r] /= p.count[r];
    // Make cumulative
    for (int r = 1; r <= 60; ++r)
        p.cum_energy[r] += p.cum_energy[r-1];
    return p;
}

int main() {
    constexpr int L = 128, C = L/2;
    std::printf("RADIAL PROFILE COMPARISON — 128^3, t=2000\n");
    std::printf("=========================================================\n\n");

    const char* labels[] = {"Uniform", "Selective", "No-damp"};
    Profile profiles[3];

    for (int mode = 0; mode < 3; ++mode) {
        GpuEngine gpu(L);
        gpu.toggles.enable_all();
        gpu.toggles.genesis = false;
        gpu.toggles.movement = false;
        if (mode == 0) gpu.toggles.selective_damping = false;  // uniform
        if (mode == 1) gpu.toggles.selective_damping = true;   // selective
        if (mode == 2) { gpu.toggles.damping = false; gpu.toggles.selective_damping = false; } // none

        gpu.inject_particle(C, C, C, +1, {0, 0, K_B}, 0, 0);
        { std::vector<Voxel> v(L*L*L); gpu.sync_to_host(v); v[C*L*L+C*L+C].locked=true; gpu.upload_from_host(v); }
        gpu.run(2000);
        std::vector<Voxel> v(L*L*L); gpu.sync_to_host(v);
        profiles[mode] = measure(v, L, C, C, C);
    }

    // Print comparison
    std::printf("%4s  %12s  %12s  %12s  |  %8s  %8s  %8s\n",
                "r", "Uniform", "Selective", "No-damp", "Sel/Uni", "ND/Uni", "Sel/ND");
    std::printf("----  ------------  ------------  ------------  |  --------  --------  --------\n");
    for (int r = 0; r <= 40; ++r) {
        double u = profiles[0].avg_flux[r];
        double s = profiles[1].avg_flux[r];
        double n = profiles[2].avg_flux[r];
        double su = (u > 1e-15) ? s/u : 0;
        double nu = (u > 1e-15) ? n/u : 0;
        double sn = (n > 1e-15) ? s/n : 0;
        std::printf("%4d  %12.6e  %12.6e  %12.6e  |  %8.4f  %8.4f  %8.4f\n",
                    r, u, s, n, su, nu, sn);
    }

    // Cumulative energy fractions
    std::printf("\nCUMULATIVE ENERGY FRACTION (E(<r) / E_total)\n");
    std::printf("%4s  %10s  %10s  %10s\n", "r", "Uniform", "Selective", "No-damp");
    std::printf("----  ----------  ----------  ----------\n");
    for (int r : {1, 2, 3, 5, 7, 10, 15, 17, 20, 25, 28, 30, 40, 50, 60}) {
        if (r > 60) continue;
        double u = profiles[0].cum_energy[r] / profiles[0].total_E;
        double s = profiles[1].cum_energy[r] / profiles[1].total_E;
        double n = profiles[2].cum_energy[r] / profiles[2].total_E;
        std::printf("%4d  %10.6f  %10.6f  %10.6f%s\n", r, u, s, n,
                    (r == 5) ? "  <-- r_50?" : (r == 17) ? "  <-- r_90?" : (r == 28) ? "  <-- r_shell?" : "");
    }

    std::printf("\nTotal energy: Uniform=%.6e  Selective=%.6e  No-damp=%.6e\n",
                profiles[0].total_E, profiles[1].total_E, profiles[2].total_E);

    return 0;
}
