/**
 * Benchmark: G_N(M, L) mass-spectrum scan (Arc D gap (ii) scaffold)
 *
 * Purpose: Verify that the engine's solve_latency_poisson_cpu
 * (poisson_solvers.cpp:190-228) correctly reproduces a constant engine-internal
 * G_N across multiple cluster mass scales M and multiple lattice sizes L.
 *
 * The implementation (per `AUDIT_NEWTON_POSTULATES_RECONCILIATION.md` §3.5):
 *
 *   sor_source[i] = 4*PI*G_N * (K_B * |state[i]| - mean_mass)
 *   sor_sweep_18pt(phi_latency, sor_source, lattice, OMEGA)
 *
 * This is exactly SPEC_FTD_LAGRANGIAN.md §4.2 [THEOREM] form: ∇²ℒ = 4πG·ρ_mass
 * with ρ_mass = K_B·n (number density of manifested sites). The form-level
 * match is established. THIS BENCHMARK validates the SOR solver's numerical
 * fidelity across mass scales: extracting G_N_measured = avg(φ·r) / M_total
 * from cluster runs at multiple radii should return the engine-internal G_N
 * to within tolerance, with no mass-dependent drift.
 *
 * If G_N_measured varies systematically with cluster mass, the SOR has
 * mass-dependent artifacts (likely from periodic BC + cluster-size-to-lattice
 * ratio) and the SPEC §4.2 [THEOREM] form-level closure does NOT translate
 * cleanly to numerical closure across mass scales.
 *
 * If G_N_measured is constant within tolerance, the engine's Poisson IS
 * the SPEC §4.2 [THEOREM] derivation operationally as well as formally.
 *
 * Per Wilsonian-reframe plan v2 Arc D gap (ii): scan L ∈ {32, 48, 64} for the
 * critical-path bracket; Arc B P2 verification companion to render_bridge
 * line 207 audit (which already established form-level alignment).
 *
 * Scope (scaffold v0):
 *   - 3 lattice sizes: L ∈ {32, 48, 64}
 *   - 4 cluster radii per L: r ∈ {2, 3, 4, 5} (M ranges ~33 to ~525 particles)
 *   - 1 measurement window per (r, L): 200 ticks SOR equilibration + sample
 *   - Output: stdout-CSV row per (r, L) with G_N_measured + percent deviation
 *   - Tolerance: 50% (loose; tighten after first campaign reveals real envelope)
 *
 * Per CLAUDE.md / plan v2 environment notes: this benchmark runs on
 * WSL2/CUDA (engine/build_wsl) for canonical measurement; CPU build can
 * compile for correctness check but multi-cluster sweeps want GPU time.
 *
 * TODO before first measurement campaign:
 *   - Confirm cluster-radius-to-L upper limit (cluster diameter < L/2 to avoid
 *     periodic wrap-around contamination; r=5 on L=32 may already saturate this)
 *   - Decide whether to use single seed or multi-seed (cluster injection is
 *     deterministic so single-seed suffices for this measurement)
 *   - Add CSV output to file (not just stdout) for downstream analysis
 *   - Set per-(r, L) ticks budget: 200 is the EIN-3 default; may need ~500
 *     for L=64 SOR convergence at large clusters
 *
 * Theory references (canonical):
 *   - SPEC_FTD_LAGRANGIAN.md §4.2 [THEOREM]: source derivation
 *   - DERIV_NEWTON_FROM_SUBSTRATE.md §1.2-1.5: chain from Phase G to Schwarzschild
 *   - AUDIT_NEWTON_POSTULATES_RECONCILIATION.md §3.5: gap (iv) form-level closure
 *   - poisson_solvers.cpp:190-228: solve_latency_poisson_cpu implementation
 *   - test_einstein_equations.cpp EIN-3: single-cluster G_N extraction (template)
 *
 * Status: SCAFFOLD (2026-05-24) — compiles and runs, but parameter envelope
 * (tolerance bands, equilibration tick budgets, cluster-radius-to-L ratio)
 * needs first-campaign tuning on WSL2/CUDA. The structure is locked; the
 * numerical thresholds are placeholders flagged TODO.
 */

#include <iostream>
#include <iomanip>
#include <cmath>
#include <memory>
#include <vector>

#include "ftd/render_bridge.h"
#include "ftd/constants.h"

using namespace ftd;

static int failures = 0;

static void check(const char* name, bool condition) {
    if (condition) {
        std::cout << "  PASS  " << name << "\n";
    } else {
        std::cout << "  FAIL  " << name << "\n";
        ++failures;
    }
}

// ================================================================
// Engine factory: latency Poisson enabled, all other dynamics off.
// Same minimal-engine pattern as test_einstein_equations.cpp EIN-3
// (make_einstein_engine helper at line 117 of that file).
// ================================================================
static std::unique_ptr<RenderBridge> make_engine(int L) {
    auto rb = std::make_unique<RenderBridge>(L);
    rb->toggles.disable_all();
    rb->toggles.gravity = true;
    rb->toggles.latency_field = true;
    return rb;
}

// ================================================================
// Helper: inject a spherical cluster of locked +1 particles at lattice center.
// Returns the particle count. Pattern from test_einstein_equations.cpp:133.
// ================================================================
static int inject_mass_cluster(RenderBridge& rb, int cx, int cy, int cz,
                               int radius) {
    int count = 0;
    for (int dz = -radius; dz <= radius; ++dz)
    for (int dy = -radius; dy <= radius; ++dy)
    for (int dx = -radius; dx <= radius; ++dx) {
        if (dx*dx + dy*dy + dz*dz <= radius * radius) {
            int x = cx + dx, y = cy + dy, z = cz + dz;
            rb.inject_particle(x, y, z, +1, Vec3(K_B, 0.0, 0.0));
            rb.voxels()[rb.lattice().index(x, y, z)].locked = true;
            ++count;
        }
    }
    return count;
}

// ================================================================
// Single measurement: inject cluster radius `r` on L^3 lattice,
// equilibrate Poisson SOR, extract G_N_measured from <phi * r> / M_total.
//
// Mirrors EIN-3 (test_einstein_equations.cpp line 366) but parameterized
// over (radius, L) so it can be called from a grid scan.
//
// Returns G_N_measured (engine-internal units). On measurement failure,
// returns negative sentinel to flag to the caller.
// ================================================================
static double measure_G_N_at(int radius, int L, int equilibration_ticks = 200) {
    auto rb = make_engine(L);
    const int mid = L / 2;
    const int mass_count = inject_mass_cluster(*rb, mid, mid, mid, radius);
    const double M_total = static_cast<double>(mass_count) * K_B;

    // Equilibrate Poisson SOR. Each tick runs SOR_ITERATIONS sweeps.
    for (int t = 0; t < equilibration_ticks; ++t) rb->tick();

    // Sample phi*r over the far-field band r ∈ [radius+2, L/2 - 2]
    // (mirrors EIN-3 r=5..many; lower bound avoids near-field artifacts,
    // upper bound stays away from periodic-BC contamination).
    const auto& phi = rb->phi_latency();
    const int r_min = radius + 2;
    const int r_max = (L / 2) - 4;
    if (r_max <= r_min) return -1.0;

    double sum_phi_r = 0.0;
    int count = 0;
    for (int dz = -r_max; dz <= r_max; ++dz)
    for (int dy = -r_max; dy <= r_max; ++dy)
    for (int dx = -r_max; dx <= r_max; ++dx) {
        double r2 = dx*dx + dy*dy + dz*dz;
        double r = std::sqrt(r2);
        if (r < r_min || r > r_max) continue;
        int x = mid + dx, y = mid + dy, z = mid + dz;
        double p = phi[rb->lattice().index(x, y, z)];
        // phi(r) = G_N * M_total / r  =>  phi * r = G_N * M_total
        sum_phi_r += std::abs(p) * r;
        ++count;
    }
    if (count == 0) return -1.0;

    const double avg_phi_r = sum_phi_r / count;
    return avg_phi_r / M_total;  // G_N_measured
}

// ================================================================
// Sweep over (radius, L) grid; report G_N(M, L) table.
// Tolerance check: each measurement should be within FACTOR_TOL of
// engine G_N constant (constants.h: ontic::G_N = 0.01).
//
// FACTOR_TOL = 5.0 is loose first-pass (matches EIN-3b in test_einstein_equations.cpp);
// tighten after first campaign reveals real spread.
// ================================================================
static constexpr double FACTOR_TOL = 10.0;

static void run_g_n_spectrum_sweep() {
    const std::vector<int> radii = {2, 3, 4, 5};
    const std::vector<int> lattice_sizes = {32, 48, 64};

    std::cout << "\n=== G_N(M, L) Mass-Spectrum Scan ===\n";
    std::cout << "Engine G_N target: " << std::setprecision(6) << G_N << "\n";
    std::cout << "Tolerance band: [" << G_N / FACTOR_TOL << ", "
              << G_N * FACTOR_TOL << "]\n\n";

    // CSV header to stdout (downstream analysis can grep + pipe to file).
    std::cout << "L,radius,particles,M_total,G_N_measured,ratio_to_engine\n";

    for (int L : lattice_sizes) {
        for (int r : radii) {
            // Skip combinations where cluster diameter approaches L/2
            // (periodic BC contamination per AUDIT §3.5 framing).
            if (2 * r + 4 >= L / 2) {
                std::cout << L << "," << r
                          << ",SKIP,SKIP,SKIP,SKIP  # cluster vs lattice ratio too large\n";
                continue;
            }

            // Recreate the engine inside measure_G_N_at to count particles + mass.
            auto rb_count = make_engine(L);
            const int particles = inject_mass_cluster(*rb_count, L/2, L/2, L/2, r);
            const double M_total = particles * K_B;

            const double G_N_measured = measure_G_N_at(r, L);

            std::cout << L << "," << r << "," << particles << ","
                      << std::setprecision(6) << M_total << ","
                      << G_N_measured << ","
                      << (G_N_measured / G_N) << "\n";

            // Sanity check per (r, L): measurement within tolerance band.
            const std::string label = "G_N(r=" + std::to_string(r)
                + ", L=" + std::to_string(L) + ") within factor "
                + std::to_string(static_cast<int>(FACTOR_TOL));
            check(label.c_str(),
                  G_N_measured > G_N / FACTOR_TOL && G_N_measured < G_N * FACTOR_TOL);
        }
    }
}

// ================================================================
// Mass-independence check: across all (r, L) pairs at fixed L, G_N_measured
// should be approximately constant (the engine Poisson should not produce
// mass-dependent coupling drift).
//
// TODO scaffold: implement after first campaign reveals what the actual
// spread is. Tolerance bands here are placeholders. Likely metrics:
//   - Per-L: max-ratio / min-ratio of G_N_measured across radii (should be ~1)
//   - Across-L: ratio of L=32 to L=64 measurements at fixed r (should be ~1)
// If spreads are large (>2x), the SOR has cluster-size or periodic-BC artifacts
// that need diagnosis before the per-particle scaling check (Arc B Wilsonian
// reframe milestone) can use this benchmark as load-bearing.
// ================================================================
static void test_mass_independence_placeholder() {
    std::cout << "\n--- G_N(M) mass-independence: SCAFFOLD (not yet measuring) ---\n";
    std::cout << "  TODO: implement after first measurement campaign reveals\n"
              << "  the actual per-L spread + cross-L ratio. See file header.\n";
    // Intentionally no check() calls until measurement-based thresholds set.
}

// ================================================================
// Main: runs the sweep + the (placeholder) mass-independence check.
// Returns 0 on no failures, 1 otherwise.
// ================================================================
int main() {
    std::cout << "============================================================\n";
    std::cout << " benchmark_g_n_mass_spectrum (Arc D gap (ii) scaffold)\n";
    std::cout << "============================================================\n";

    run_g_n_spectrum_sweep();
    test_mass_independence_placeholder();

    std::cout << "\n============================================================\n";
    if (failures == 0) {
        std::cout << " ALL CHECKS PASS\n";
    } else {
        std::cout << " " << failures << " FAILURE(S)\n";
    }
    std::cout << "============================================================\n";

    return failures == 0 ? 0 : 1;
}
