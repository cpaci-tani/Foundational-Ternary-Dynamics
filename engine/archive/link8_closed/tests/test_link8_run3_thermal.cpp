/**
 * @file test_link8_run3_thermal.cpp
 * @brief Link 8 Candidate 1 — Run 3 redo on Langevin-thermalized ensemble.
 *
 * ============================================================================
 *  [CLOSED NEGATIVE 2026-04-20] — DO NOT LAND IN CI
 * ----------------------------------------------------------------------------
 *  Closure memo:  docs/theory/10_eft_program/AUDIT_LINK8_CLOSURE.md
 *  Ledger entry:  FTD-0050
 *
 *  The master-quadratic-as-RG-step hypothesis was closed NEGATIVE on
 *  2026-04-20 — the engine's (SC+FCC)/2 stencil is structurally orthogonal
 *  to the BCC sub-stencil where 16 G*^2 lives. This file is archived for
 *  its reusable Langevin-ensemble + connected-correlator scaffolding; the
 *  master-quadratic framing is retired.
 * ============================================================================
 *
 * Original Run 3 (from link8_candidate1_instructions.md):
 *   FIELD: |J|^2(x), the real scalar density
 *   BLOCK_RULE: local 2x2x2 average of the scalar field
 *   COUPLING_EXTRACTION: fit two-point correlation function amplitude at
 *                        largest separation
 *   BLOCKING_SEQUENCE: L -> L/2 -> L/4 -> ...
 *
 * First-attempt verdict (Session A, pre-thermostat): underspecified because
 * the bare-lattice engine with a charge-pair seed produces a deterministic
 * localized |J|^2 distribution, not a statistical ensemble with a meaningful
 * connected correlator.
 *
 * Session B built a Langevin thermostat (OU noise on wave_vel; verified
 * equipartition within 4% on wave_vel). This test uses that thermostat
 * to generate an ensemble of J configurations and extracts the Run-3
 * observable properly.
 *
 * Extraction: at each blocking level n, compute
 *     rho_n(x) = |J_n(x)|^2
 *     rho_bar_n = <rho_n>_voxel
 *     C_n(r) = <rho_n(x) rho_n(x+r)> - rho_bar_n^2        (connected)
 *     y_n = C_n(r_max)  where r_max = L_n / 4
 *
 * y_n is then checked against the target recurrence
 *     y_{n+1} = A y_n + B y_{n-1},
 *     A = 16 G*^2 = 140.0601,  B = -16 G*^3 = -414.3924.
 *
 * Ensemble: N_SEEDS independent thermal configs, averaged.
 */

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>

#include "ftd/render_bridge.h"

namespace {

// --- targets ---------------------------------------------------------------
constexpr double G_STAR = 2.958675119188638889;
constexpr double A_TARGET =  16.0 * G_STAR * G_STAR;
constexpr double B_TARGET = -16.0 * G_STAR * G_STAR * G_STAR;

// --- knobs -----------------------------------------------------------------
constexpr int    L_FINE   = 16;
constexpr int    N_LEVELS = 4;       // 16 -> 8 -> 4 -> 2
constexpr int    N_BURN   = 5000;    // burn-in for thermalization
constexpr int    N_SEEDS  = 4;       // independent thermal configurations
constexpr double T_THERM  = 0.01;
constexpr double GAMMA    = 0.01;

// --- helpers ---------------------------------------------------------------
std::vector<double> jsq_field(const ftd::RenderBridge& rb) {
    const auto& vox = rb.voxels();
    std::vector<double> rho(vox.size());
    for (size_t i = 0; i < vox.size(); ++i) rho[i] = vox[i].flux.mag2();
    return rho;
}

// Block a scalar field by 2x2x2 arithmetic mean. Returns (L/2)^3 vector.
std::vector<double> block_scalar(const std::vector<double>& fine, int L_fine) {
    const int Lc = L_fine / 2;
    std::vector<double> out(Lc * Lc * Lc, 0.0);
    for (int x = 0; x < L_fine; ++x) {
        for (int y = 0; y < L_fine; ++y) {
            for (int z = 0; z < L_fine; ++z) {
                const int xc = x / 2, yc = y / 2, zc = z / 2;
                const int idx_f = (x * L_fine + y) * L_fine + z;
                const int idx_c = (xc * Lc + yc) * Lc + zc;
                out[idx_c] += fine[idx_f];
            }
        }
    }
    // 8 fine voxels -> 1 coarse voxel; divide by 8 for arithmetic mean
    for (double& v : out) v *= 0.125;
    return out;
}

// Connected two-point correlator along the 3 principal axes, averaged.
// C(r) = <rho(x) rho(x+r)> - <rho>^2,   r = 0, 1, ..., L/2 - 1
std::vector<double> connected_correlator(const std::vector<double>& rho, int L) {
    const int max_r = L / 2;
    std::vector<double> C(max_r, 0.0);
    std::vector<long long> counts(max_r, 0);

    double rho_bar = 0.0;
    for (double v : rho) rho_bar += v;
    rho_bar /= rho.size();

    auto idx = [L](int x, int y, int z) {
        auto w = [L](int i) { return ((i % L) + L) % L; };
        return (w(x) * L + w(y)) * L + w(z);
    };

    for (int x = 0; x < L; ++x) {
        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                const double r0 = rho[idx(x, y, z)];
                for (int r = 0; r < max_r; ++r) {
                    C[r] += r0 * rho[idx(x + r, y, z)];
                    C[r] += r0 * rho[idx(x, y + r, z)];
                    C[r] += r0 * rho[idx(x, y, z + r)];
                    counts[r] += 3;
                }
            }
        }
    }
    for (int r = 0; r < max_r; ++r) {
        if (counts[r] > 0) C[r] = C[r] / counts[r] - rho_bar * rho_bar;
    }
    return C;
}

struct LevelStat {
    int L = 0;
    int r_max = 0;
    double rho_bar = 0;
    double C_at_rmax = 0;  // connected correlator amplitude at r_max
    std::vector<double> C_full;  // C(0..L/2-1)
};

}  // namespace

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    std::printf("================================================================\n");
    std::printf("  Link 8 Run 3 redo -- thermalized |J|^2 correlator extraction\n");
    std::printf("  L_fine = %d, N_levels = %d, T = %.4f, gamma = %.4f\n",
                L_FINE, N_LEVELS, T_THERM, GAMMA);
    std::printf("  Burn-in = %d ticks, N_seeds = %d (ensemble)\n", N_BURN, N_SEEDS);
    std::printf("  Target A = %.6f, B = %.6f\n", A_TARGET, B_TARGET);
    std::printf("================================================================\n\n");

    // For each blocking level, accumulate over the seed ensemble.
    std::vector<double> y_sum(N_LEVELS, 0.0);
    std::vector<double> rho_bar_sum(N_LEVELS, 0.0);

    for (int seed_idx = 0; seed_idx < N_SEEDS; ++seed_idx) {
        const unsigned seed = 1u + static_cast<unsigned>(seed_idx);
        std::printf("  --- Seed %d/%d ---\n", seed_idx + 1, N_SEEDS);

        ftd::RenderBridge rb(L_FINE);
        rb.toggles.wave_propagation = true;
        rb.toggles.coupling         = false;
        rb.toggles.damping          = false;
        rb.toggles.genesis          = false;
        rb.toggles.gauss_projection = true;
        rb.toggles.forces           = false;
        rb.toggles.gravity          = false;
        rb.toggles.poisson_coulomb  = false;
        rb.toggles.movement         = false;
        rb.toggles.lorentz_force    = false;
        rb.toggles.selective_damping= false;
        rb.toggles.larmor_radiation = false;
        rb.toggles.dual_substrate   = false;
        rb.toggles.weak_transmutation = false;
        rb.toggles.latency_field    = false;
        rb.toggles.langevin         = true;
        rb.toggles.langevin_T       = T_THERM;
        rb.toggles.langevin_gamma   = GAMMA;
        rb.seed_rng(seed);

        rb.run(N_BURN);

        // Compute |J|^2 at each blocking level and its connected correlator.
        std::vector<double> rho_fine = jsq_field(rb);
        int L_cur = L_FINE;
        std::vector<double> rho_cur = rho_fine;
        for (int n = 0; n < N_LEVELS; ++n) {
            // Compute correlator at this level.
            auto C = connected_correlator(rho_cur, L_cur);
            const int r_max = L_cur / 2 - 1;
            double rho_bar = 0.0;
            for (double v : rho_cur) rho_bar += v;
            rho_bar /= rho_cur.size();

            if (r_max >= 0 && r_max < (int)C.size()) {
                double y = C[r_max];
                y_sum[n] += y;
                rho_bar_sum[n] += rho_bar;
                std::printf("    n=%d L=%2d r_max=%d rho_bar=%.4e C(r_max)=%+.4e\n",
                            n, L_cur, r_max, rho_bar, y);
            } else {
                std::printf("    n=%d L=%2d r_max=%d (insufficient lattice)\n",
                            n, L_cur, r_max);
            }

            // Block to next level (unless last).
            if (n + 1 < N_LEVELS) {
                auto next = block_scalar(rho_cur, L_cur);
                rho_cur = next;
                L_cur /= 2;
            }
        }
        std::printf("\n");
    }

    // Ensemble averages.
    std::printf("  === Ensemble averages over %d seeds ===\n", N_SEEDS);
    std::vector<double> y_n(N_LEVELS, 0.0);
    for (int n = 0; n < N_LEVELS; ++n) {
        y_n[n] = y_sum[n] / N_SEEDS;
        const double rb_avg = rho_bar_sum[n] / N_SEEDS;
        std::printf("    n=%d L=%2d  y_n = <C(r_max)>_ensemble = %+.6e  (rho_bar = %.4e)\n",
                    n, L_FINE >> n, y_n[n], rb_avg);
    }
    std::printf("\n");

    // Recurrence residual check.
    std::printf("  === Recurrence check (target A=%.4f, B=%.4f) ===\n",
                A_TARGET, B_TARGET);
    for (int k = 2; k < N_LEVELS; ++k) {
        const double lhs = y_n[k];
        const double rhs = A_TARGET * y_n[k-1] + B_TARGET * y_n[k-2];
        const double abs_err = lhs - rhs;
        const double rel = (std::abs(rhs) > 1e-30) ? abs_err / rhs : 0.0;
        std::printf("    triple (y_%d,y_%d,y_%d): y_n=%+.4e  rhs=%+.4e  residual=%+.4e  rel=%.2e\n",
                    k-2, k-1, k, lhs, rhs, abs_err, rel);
    }

    // 2-eq fit if N_LEVELS >= 4.
    if (N_LEVELS >= 4) {
        // y[2] = A y[1] + B y[0];  y[3] = A y[2] + B y[1]
        // M = [[y[1], y[0]], [y[2], y[1]]]; rhs = (y[2], y[3])
        const double m11 = y_n[1], m12 = y_n[0];
        const double m21 = y_n[2], m22 = y_n[1];
        const double det = m11 * m22 - m12 * m21;
        std::printf("\n  === 2-eq fit (det M = %.4e) ===\n", det);
        if (std::abs(det) > 1e-30) {
            const double r1 = y_n[2], r2 = y_n[3];
            const double A_fit = ( m22 * r1 - m12 * r2) / det;
            const double B_fit = (-m21 * r1 + m11 * r2) / det;
            const double dA = 100.0 * (A_fit - A_TARGET) / A_TARGET;
            const double dB = 100.0 * (B_fit - B_TARGET) / B_TARGET;
            std::printf("    A_fit = %+.6f   target %.6f   dev = %+7.2f%%\n",
                        A_fit, A_TARGET, dA);
            std::printf("    B_fit = %+.6f   target %.6f   dev = %+7.2f%%\n",
                        B_fit, B_TARGET, dB);
        } else {
            std::printf("    (singular; cannot fit)\n");
        }
    }

    std::printf("\n================================================================\n");
    std::printf("  Run 3 (thermalized) complete. Raw data above.\n");
    std::printf("================================================================\n");
    return 0;
}
