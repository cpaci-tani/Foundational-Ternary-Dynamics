/**
 * @file test_langevin_sublattice_equipartition.cpp
 * @brief Verify the Langevin thermostat with site-class filter only thermalizes
 *        the selected parity class.
 *
 * Setup mirrors test_langevin_equipartition.cpp but with
 * langevin_site_filter = SiteClass::BCC_SITES. Damping is OFF so non-selected
 * voxels neither thermalize NOR damp — they should retain |wave_vel|^2 ≈ 0
 * (ignoring small leakage from coupling/wave terms; we leave coupling=false).
 *
 * Targets at equilibrium:
 *     <|wave_vel|^2>_{BCC voxels}  ≈ 3T  (within ~10%)
 *     <|wave_vel|^2>_{SC, FCC}     ≈ 0    (much smaller than 3T)
 *
 * Falsification: if BCC equipartition fails OR if non-BCC voxels equilibrate,
 * the filter is not isolating sublattices.
 */

#include <cmath>
#include <cstdio>
#include <vector>

#include "ftd/render_bridge.h"
#include "ftd/sublattice.h"

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    const int L = 16;
    const double T = 0.01;
    const double gamma = 0.05;
    const int N_BURN = 1000;
    const int N_MEASURE = 1000;

    std::printf("================================================================\n");
    std::printf("  Sublattice Langevin equipartition (BCC_SITES filter)\n");
    std::printf("================================================================\n");
    std::printf("  L=%d, T=%.4f, gamma=%.4f\n", L, T, gamma);
    std::printf("  Target: <|v|^2>_BCC = 3T = %.4f, <|v|^2>_other ~ 0\n\n", 3.0*T);

    ftd::RenderBridge rb(L);
    // GPU sublattice Langevin filter wired 2026-04-26 in
    // kernels_stencil.cu::phase_write_kernel via langevin_site_match().
    // No force_cpu — test runs on GPU when CUDA is enabled.

    // Bare lattice + langevin only on BCC sites.
    // wave_propagation is OFF so the Laplacian doesn't redistribute the BCC
    // thermal injection across neighbors — the OU update on wave_vel is the
    // only velocity dynamics. This isolates the sublattice-filter behaviour.
    // (For the actual spectrum measurement, wave_propagation is ON because
    // the Laplacian IS the dynamical operator we measure spectra of.)
    rb.toggles.disable_all();
    rb.toggles.langevin = true;
    rb.toggles.langevin_T = T;
    rb.toggles.langevin_gamma = gamma;
    rb.toggles.langevin_site_filter = ftd::SiteClass::BCC_SITES;

    // Validate the toggle combination.
    std::string err;
    if (!rb.toggles.validate(&err)) {
        std::printf("[FAIL] toggle validation: %s\n", err.c_str());
        return 1;
    }

    std::printf("  Burn-in (%d ticks)...\n", N_BURN);
    rb.run(N_BURN);

    // Categorize voxels by parity class.
    const int N = L * L * L;
    std::vector<int> bcc_voxels, sc_voxels, fcc_voxels;
    bcc_voxels.reserve(N/8); sc_voxels.reserve(N/8); fcc_voxels.reserve(6*N/8);
    for (int x = 0; x < L; ++x)
        for (int y = 0; y < L; ++y)
            for (int z = 0; z < L; ++z) {
                int i = rb.lattice().index(x, y, z);
                ftd::SiteClass c = ftd::classify_voxel(x, y, z);
                if (c == ftd::SiteClass::BCC_SITES) bcc_voxels.push_back(i);
                else if (c == ftd::SiteClass::SC_SITES) sc_voxels.push_back(i);
                else fcc_voxels.push_back(i);
            }
    std::printf("  Parity classes: SC=%zu, BCC=%zu, FCC=%zu\n",
                sc_voxels.size(), bcc_voxels.size(), fcc_voxels.size());

    // Measure means over N_MEASURE ticks.
    double sum_bcc = 0.0, sum_sc = 0.0, sum_fcc = 0.0;
    long long count_bcc = 0, count_sc = 0, count_fcc = 0;
    const ftd::RenderBridge& crb = rb;

    for (int step = 0; step < N_MEASURE; ++step) {
        rb.run(1);
        const auto& vox = crb.voxels();
        for (int i : bcc_voxels) { sum_bcc += vox[i].wave_vel.mag2(); ++count_bcc; }
        for (int i : sc_voxels)  { sum_sc  += vox[i].wave_vel.mag2(); ++count_sc;  }
        for (int i : fcc_voxels) { sum_fcc += vox[i].wave_vel.mag2(); ++count_fcc; }
    }

    const double v2_bcc = sum_bcc / static_cast<double>(count_bcc);
    const double v2_sc  = sum_sc  / static_cast<double>(count_sc);
    const double v2_fcc = sum_fcc / static_cast<double>(count_fcc);
    const double target = 3.0 * T;
    const double dev_bcc = (v2_bcc - target) / target;

    std::printf("\n  Results:\n");
    std::printf("    <|v|^2>_BCC = %.4e  (target %.4e, dev %+.2f%%)\n", v2_bcc, target, 100.0*dev_bcc);
    std::printf("    <|v|^2>_SC  = %.4e  (target ~ 0; ratio to BCC: %.4f)\n", v2_sc,  v2_sc / std::max(v2_bcc, 1e-18));
    std::printf("    <|v|^2>_FCC = %.4e  (target ~ 0; ratio to BCC: %.4f)\n", v2_fcc, v2_fcc / std::max(v2_bcc, 1e-18));

    // Acceptance:
    //   (a) BCC equipartition within 15% (loose; thermal noise + finite ensemble)
    //   (b) SC and FCC remain at <|v|^2> < 0.2 * BCC (significant separation)
    int failures = 0;
    if (std::abs(dev_bcc) > 0.15) {
        std::printf("[FAIL] BCC equipartition deviation %+.2f%% > 15%%\n", 100.0*dev_bcc);
        ++failures;
    } else {
        std::printf("[ ok ] BCC equipartition within 15%%\n");
    }
    if (v2_sc > 0.2 * v2_bcc) {
        std::printf("[FAIL] SC voxels not isolated: <|v|^2>_SC = %.4e > 0.2*<|v|^2>_BCC\n", v2_sc);
        ++failures;
    } else {
        std::printf("[ ok ] SC voxels isolated (<|v|^2>_SC < 0.2 * BCC)\n");
    }
    if (v2_fcc > 0.2 * v2_bcc) {
        std::printf("[FAIL] FCC voxels not isolated: <|v|^2>_FCC = %.4e > 0.2*<|v|^2>_BCC\n", v2_fcc);
        ++failures;
    } else {
        std::printf("[ ok ] FCC voxels isolated (<|v|^2>_FCC < 0.2 * BCC)\n");
    }

    std::printf("================================================================\n");
    std::printf("  Result: %s (%d failure(s))\n",
                failures == 0 ? "PASS" : "FAIL", failures);
    std::printf("================================================================\n");
    return failures == 0 ? 0 : 1;
}
