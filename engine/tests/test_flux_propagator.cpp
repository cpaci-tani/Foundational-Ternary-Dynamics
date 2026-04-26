/**
 * @file test_flux_propagator.cpp
 * @brief Phase-4g: measure the 2-point flux correlator on the Langevin
 *        ensemble and classify as bosonic-vector vs fermionic/anomalous.
 *
 * The mode-erasure theorem + FTD-0061..0074 show that no site-local single-
 * tick probe produces Clifford structure. The propagator test is the next-
 * most-rigorous fermion-emergence probe: measure the 2-point function
 *   G_ij(r) = ⟨J_i(x) J_j(x+r)⟩_Langevin
 * on a thermalized Langevin ensemble (FTD-0069) and classify its scaling.
 *
 * Expected scaling laws (Euclidean 3+0 dimensions):
 *
 *   Bosonic massless vector (photon-like):     G_∥(r) ~ 1/r   at large r
 *   Bosonic massive vector:                    G_∥(r) ~ e^{-mr}/r
 *   Dirac fermion:                             G(r) ~ 1/r²   at large r
 *                                              with off-diagonal γ structure
 *   Bosonic scalar (Klein-Gordon):             G(r) ~ 1/r · e^{-mr}
 *
 * The power of r in the large-r tail distinguishes the classes.
 *
 * Procedure:
 *   1. L=32 lattice, Langevin thermostat + gauss projection.
 *   2. Burn in, then sample N_SAMPLES flux snapshots at stride SAMPLE_STRIDE.
 *   3. For each sample, measure G_∥(r) and G_⊥(r) along the x-axis:
 *        G_∥(r) = ⟨J_x(x,y,z) J_x(x+r,y,z)⟩  averaged over x,y,z
 *        G_⊥(r) = ⟨J_y(x,y,z) J_y(x+r,y,z)⟩  (same, for y-component)
 *        G_off(r) = ⟨J_x(x,y,z) J_y(x+r,y,z)⟩  (off-diagonal)
 *   4. Fit log G(r) vs log r at large r; extract power-law exponent.
 *
 * Classification:
 *   exponent ≈ -1.0 → bosonic massless (expected for FTD flux, the canonical
 *                                        native result)
 *   exponent ≈ -2.0 → fermionic (would be a surprise)
 *   G_off nonzero and r-dependent → anomalous cross-component coupling
 *                                    (potential Dirac-like structure)
 */

#include "ftd/constants.h"
#include "ftd/render_bridge.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

namespace {

double mean(const std::vector<double>& xs) {
    if (xs.empty()) return 0.0;
    double s = 0.0;
    for (double x : xs) s += x;
    return s / xs.size();
}

double stderr_of_mean(const std::vector<double>& xs) {
    if (xs.size() < 2) return 0.0;
    const double m = mean(xs);
    double ss = 0.0;
    for (double x : xs) ss += (x - m) * (x - m);
    return std::sqrt(ss / (xs.size() - 1) / xs.size());
}

// Measure 2-point correlator G_ij(r) along x-axis.
// Returns vector of length R_MAX+1 with G(r=0..R_MAX).
std::vector<double> correlator_along_x(const ftd::RenderBridge& rb,
                                       int ci, int cj, int R_MAX) {
    const int L = rb.lattice().size();
    const auto& voxels = rb.voxels();
    std::vector<double> G(R_MAX + 1, 0.0);
    const int N_PAIRS = L * L * L;

    auto flux_of = [&](int idx, int comp) {
        if (comp == 0) return voxels[idx].flux.x;
        if (comp == 1) return voxels[idx].flux.y;
        return voxels[idx].flux.z;
    };

    for (int r = 0; r <= R_MAX; ++r) {
        double s = 0.0;
        for (int x = 0; x < L; ++x)
        for (int y = 0; y < L; ++y)
        for (int z = 0; z < L; ++z) {
            const int i1 = rb.lattice().index(x, y, z);
            const int x2 = (x + r) % L;
            const int i2 = rb.lattice().index(x2, y, z);
            s += flux_of(i1, ci) * flux_of(i2, cj);
        }
        G[r] = s / N_PAIRS;
    }
    return G;
}

}  // namespace

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    std::printf("================================================================\n");
    std::printf("  Phase-4g: Flux Propagator Fit on Langevin Ensemble\n");
    std::printf("================================================================\n");

    const int L = 32;
    const int R_MAX = L / 2;
    const int N_BURN = 300;
    const int N_SAMPLES = 60;
    const int SAMPLE_STRIDE = 5;

    ftd::RenderBridge rb(L);
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.langevin         = true;
    rb.toggles.langevin_T       = 0.005;
    rb.toggles.langevin_gamma   = 0.02;
    rb.seed_rng(0xDAD2011F);

    std::printf("  L=%d, Langevin (T=%.3f, γ=%.3f), burn=%d, samples=%d, stride=%d\n",
                L, rb.toggles.langevin_T, rb.toggles.langevin_gamma,
                N_BURN, N_SAMPLES, SAMPLE_STRIDE);

    std::printf("  Burn-in...\n");
    rb.run(N_BURN);

    // Per-r ensembles for 3 correlators
    std::vector<std::vector<double>> G_par(R_MAX + 1);   // J_x · J_x
    std::vector<std::vector<double>> G_perp(R_MAX + 1);  // J_y · J_y
    std::vector<std::vector<double>> G_off(R_MAX + 1);   // J_x · J_y

    std::printf("  Sampling...\n");
    for (int s = 0; s < N_SAMPLES; ++s) {
        rb.run(SAMPLE_STRIDE);
        const auto g_par  = correlator_along_x(rb, 0, 0, R_MAX);
        const auto g_perp = correlator_along_x(rb, 1, 1, R_MAX);
        const auto g_off  = correlator_along_x(rb, 0, 1, R_MAX);
        for (int r = 0; r <= R_MAX; ++r) {
            G_par[r].push_back(g_par[r]);
            G_perp[r].push_back(g_perp[r]);
            G_off[r].push_back(g_off[r]);
        }
        if ((s + 1) % 20 == 0) std::printf("    sample %d/%d\n", s + 1, N_SAMPLES);
    }

    // Report mean ± stderr vs r
    std::printf("\n  r  |  G_∥ (J_x·J_x)      |  G_⊥ (J_y·J_y)      |  G_off (J_x·J_y)\n");
    std::printf("  ---+---------------------+---------------------+---------------------\n");
    for (int r = 0; r <= R_MAX; ++r) {
        std::printf("  %2d | %+.4e ± %.2e | %+.4e ± %.2e | %+.4e ± %.2e\n",
                    r,
                    mean(G_par[r]),  stderr_of_mean(G_par[r]),
                    mean(G_perp[r]), stderr_of_mean(G_perp[r]),
                    mean(G_off[r]),  stderr_of_mean(G_off[r]));
    }

    // Fit power law at large r: log |G(r)| = a - alpha * log r
    // Use the range r ∈ [r_lo, r_hi] where noise is small and finite-L
    // effects haven't dominated.
    const int r_lo = std::max(2, L / 8);
    const int r_hi = std::min(R_MAX, L / 2 - 1);
    int n = 0;
    double sx = 0, sy = 0, sxx = 0, sxy = 0;
    for (int r = r_lo; r <= r_hi; ++r) {
        const double g = std::abs(mean(G_par[r]));
        if (g <= 1e-30) continue;
        const double lx = std::log(static_cast<double>(r));
        const double ly = std::log(g);
        sx += lx; sy += ly; sxx += lx * lx; sxy += lx * ly;
        ++n;
    }
    const double denom = n * sxx - sx * sx;
    const double alpha = (denom > 1e-30) ? -(n * sxy - sx * sy) / denom : 0.0;
    const double intercept_a = (denom > 1e-30) ? (sy + alpha * sx) / n : 0.0;

    std::printf("\n--- Power-law fit of G_∥(r) over r ∈ [%d, %d] ---\n", r_lo, r_hi);
    std::printf("  G_∥(r) ∝ r^{-α} with α = %.3f  (amplitude exp(%.3f))\n",
                alpha, intercept_a);

    std::printf("\n--- Classification ---\n");
    std::printf("  α ≈ 1.0 → bosonic massless vector (photon-like, canonical FTD)\n");
    std::printf("  α ≈ 2.0 → fermionic (Dirac propagator 1/r² at large r)\n");
    std::printf("  α ≈ 0   → short-ranged, massive exponential\n");

    const bool is_bosonic   = std::abs(alpha - 1.0) < 0.3;
    const bool is_fermionic = std::abs(alpha - 2.0) < 0.3;

    // Off-diagonal should vanish for a pure bosonic vector field
    double max_off = 0.0;
    double max_par = 0.0;
    for (int r = 1; r <= R_MAX; ++r) {
        max_off = std::max(max_off, std::abs(mean(G_off[r])));
        max_par = std::max(max_par, std::abs(mean(G_par[r])));
    }
    const double off_ratio = (max_par > 1e-30) ? max_off / max_par : 0.0;
    std::printf("\n  max |G_off| / max |G_∥|  =  %.4f\n", off_ratio);
    std::printf("  (bosonic vector expects 0; Dirac-like would show structure)\n");

    std::printf("\n================================================================\n");
    if (is_bosonic) {
        std::printf("  RESULT: Flux propagator is BOSONIC (α ≈ 1) — consistent with\n");
        std::printf("  canonical vector field. Closes fermion-propagator question:\n");
        std::printf("  the native FTD flux field is not fermionic at the 2-point level.\n");
    } else if (is_fermionic) {
        std::printf("  RESULT: Flux propagator has FERMIONIC scaling (α ≈ 2).\n");
        std::printf("  This would be a surprise — the native flux field is behaving\n");
        std::printf("  like a Dirac 2-point function. Further investigation required.\n");
    } else {
        std::printf("  RESULT: Flux propagator has anomalous scaling α = %.3f.\n", alpha);
        std::printf("  Neither clean bosonic nor clean fermionic. Check finite-L,\n");
        std::printf("  ensemble-size effects, or a truly anomalous dimension.\n");
    }
    std::printf("================================================================\n");
    return 0;
}
