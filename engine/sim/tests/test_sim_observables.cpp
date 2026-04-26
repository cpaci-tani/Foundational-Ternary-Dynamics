/**
 * @file test_sim_observables.cpp
 * @brief Phase D — observable-library unit tests (analytical correctness).
 *
 * For each new observable, we construct a lattice whose configuration
 * gives a known analytical answer, then assert the observable
 * reproduces it to machine precision.
 *
 * FluxCorrelator:
 *   - Uniform flux J₀ everywhere → C(r) = |J₀|² at every r (constant).
 *   - Single-site flux → C(0) = |J|²/N, C(r>0) = 0.
 *
 * EwsbCondensateCount:
 *   - Mixed injected states {+1×3, -1×2, rest 0} + known flux → all
 *     four snapshot fields recover the exact input values.
 *
 * FieldEnergyAudit:
 *   - Correctly forwards to engine audit; field_energy matches direct
 *     voxel reduction.
 *
 * These are backend-agnostic tests (CPU only for Phase D unit tests;
 * parity against GPU covered separately in Phase C / test_sim_parity).
 */

#include <cmath>
#include <cstdio>
#include <memory>

#include "ftd/sim/pipeline.h"
#include "ftd/sim/backend_cpu.h"
#include "ftd/sim/fit_scaling_dimension.h"
#include "ftd/sim/measure_v_of_r.h"
#include "ftd/sim/observables/flux_correlator.h"
#include "ftd/sim/observables/ewsb_condensate_count.h"
#include "ftd/sim/observables/field_energy_audit.h"
#include "ftd/test_telemetry.h"

#include <vector>
#include <utility>

using namespace ftd::sim;

static int g_failures = 0;
static void check(const char* name, bool ok, const char* detail = nullptr) {
    if (ok) std::printf("  PASS  %s\n", name);
    else {
        std::printf("  FAIL  %s%s%s\n", name,
                    detail ? "  " : "", detail ? detail : "");
        ++g_failures;
    }
}

// ========== FluxCorrelator ==========

// FC1 — uniform flux: C(r) = |J|² at every r, constant across r.
static void fc1_uniform() {
    std::puts("\n--- FC1: FluxCorrelator on uniform flux — constant C(r) ---");
    const int L = 12;
    Pipeline<BackendCpu> p(L);
    const ftd::Vec3 J{0.3, -0.1, 0.4};
    const double J2 = J.dot(J);  // 0.09 + 0.01 + 0.16 = 0.26
    for (int x = 0; x < L; ++x)
        for (int y = 0; y < L; ++y)
            for (int z = 0; z < L; ++z)
                p.inject_flux(x, y, z, J);

    FluxCorrelator<BackendCpu> obs(L / 2);
    obs.measure(p.state());
    const auto C = obs.result_host();

    char buf[192];
    double max_dev = 0.0;
    for (int r = 0; r < static_cast<int>(C.size()); ++r) {
        const double dev = std::abs(C[r] - J2);
        if (dev > max_dev) max_dev = dev;
    }
    std::snprintf(buf, sizeof buf, "(expected %.6f at every r; max dev %.2e; C[0]=%.6f C[L/4]=%.6f)",
                  J2, max_dev, C[0], C[L / 4]);
    check("FC1 C(r) = |J|² everywhere to 1e-12", max_dev < 1e-12, buf);
}

// FC2 — plane wave: C(r) = 1/6 · cos(k·r) (derivation in test_eft_anisotropy.cpp)
static void fc2_plane_wave() {
    std::puts("\n--- FC2: FluxCorrelator on plane wave J_x = sin(k·z) ---");
    constexpr double PI = 3.14159265358979323846;
    const int L = 32;
    const double k = 2.0 * PI / L;
    Pipeline<BackendCpu> p(L);
    for (int x = 0; x < L; ++x)
        for (int y = 0; y < L; ++y)
            for (int z = 0; z < L; ++z) {
                const double jx = std::sin(k * z);
                p.inject_flux(x, y, z, {jx, 0.0, 0.0});
            }

    FluxCorrelator<BackendCpu> obs(L / 2);
    obs.measure(p.state());
    const auto C = obs.result_host();

    // For J_x(z) = sin(k·z):
    //   along x axis: ⟨sin(kz)·sin(kz)⟩ = 1/2 (constant in r)
    //   along y axis: same constant 1/2 (J depends only on z)
    //   along z axis: ⟨sin(kz)·sin(k(z+r))⟩ = cos(kr)/2
    // Direction-averaged over 3 axes: (1 + 1 + cos(kr))/6 = 1/3 + cos(kr)/6

    double max_dev = 0.0;
    double dev_at_Lq = 0.0;
    for (int r = 0; r < static_cast<int>(C.size()); ++r) {
        const double expected = (1.0 / 3.0) + std::cos(k * r) / 6.0;
        const double dev = std::abs(C[r] - expected);
        if (dev > max_dev) max_dev = dev;
        if (r == L / 4) dev_at_Lq = dev;
    }
    char buf[160];
    std::snprintf(buf, sizeof buf, "(L/4 dev = %.2e; max dev %.2e)", dev_at_Lq, max_dev);
    check("FC2 C(r) matches (1/3 + cos(kr)/6) to 1e-10", max_dev < 1e-10, buf);
}

// ========== EwsbCondensateCount ==========

// EW1 — known injection reproduces all four fields
static void ew1_known_injection() {
    std::puts("\n--- EW1: EwsbCondensateCount on known state + flux ---");
    const int L = 10;
    Pipeline<BackendCpu> p(L);
    // 3 +1s, 2 -1s; rest state=0
    p.inject_particle(1, 1, 1, +1, {0.1, 0.0, 0.0});
    p.inject_particle(2, 2, 2, +1, {0.0, 0.1, 0.0});
    p.inject_particle(3, 3, 3, +1, {0.0, 0.0, 0.1});
    p.inject_particle(4, 4, 4, -1, {-0.1, 0.0, 0.0});
    p.inject_particle(5, 5, 5, -1, {0.0, -0.1, 0.0});

    EwsbCondensateCount<BackendCpu> obs;
    obs.measure(p.state());
    const auto s = obs.result_host();

    char buf[192];
    std::snprintf(buf, sizeof buf,
                  "(n+=%lld n-=%lld n0=%lld; <|J|>=%.6e; Efield=%.6e)",
                  s.n_plus, s.n_minus, s.n_zero,
                  s.mean_abs_J, s.field_energy);
    check("EW1 n_plus  = 3",  s.n_plus == 3, buf);
    check("EW1 n_minus = 2",  s.n_minus == 2, buf);
    check("EW1 n_zero  = L³-5", s.n_zero == static_cast<long long>(L)*L*L - 5, buf);
    check("EW1 imbalance = +1", s.imbalance() == 1, buf);
    check("EW1 manifested = 5", s.manifested() == 5, buf);
    // Total |J|² = 5 particles × 0.01 = 0.05
    check("EW1 field_energy ≈ 0.05 (5 particles × 0.01)",
          std::abs(s.field_energy - 0.05) < 1e-12, buf);
    // mean_abs_J = 5 × 0.1 / 1000 = 5e-4
    check("EW1 mean_abs_J ≈ 5e-4",
          std::abs(s.mean_abs_J - 5e-4) < 1e-10, buf);
}

// ========== FieldEnergyAudit ==========

// FEA1 — audit.field_energy matches a direct reduction
static void fea1_matches_direct() {
    std::puts("\n--- FEA1: FieldEnergyAudit vs direct reduction ---");
    const int L = 10;
    Pipeline<BackendCpu> p(L);
    // Random-ish flux pattern
    for (int x = 0; x < L; ++x)
        for (int y = 0; y < L; ++y)
            for (int z = 0; z < L; ++z) {
                const double phase = 0.1 * (x + 2*y + 3*z);
                p.inject_flux(x, y, z,
                              {0.2 * std::cos(phase), 0.3 * std::sin(phase),
                               0.1 * std::cos(2 * phase)});
            }

    FieldEnergyAudit<BackendCpu> obs;
    obs.measure(p.state());
    const auto a = obs.result_host();

    // Direct reduction
    double sum = 0.0;
    for (const auto& v : p.state().voxels()) sum += v.flux.dot(v.flux);
    char buf[160];
    std::snprintf(buf, sizeof buf, "(audit=%.6e direct=%.6e rel=%.3e)",
                  a.field_energy, sum,
                  std::abs(a.field_energy - sum) / std::max(std::abs(sum), 1e-30));
    check("FEA1 audit.field_energy matches direct reduction to 1e-6",
          std::abs(a.field_energy - sum) / std::max(std::abs(sum), 1e-30) < 1e-6, buf);
}

// ========== fit_scaling_dimension ==========

// SD1 — synthetic C(r) = 1 · r^(-2·Δ) with Δ=1.5 must recover Δ=1.5
static void sd1_recovery() {
    std::puts("\n--- SD1: fit_scaling_dimension recovers synthetic Δ=1.5 ---");
    const double delta_true = 1.5;
    std::vector<double> C;
    for (int r = 0; r < 20; ++r) {
        C.push_back(r == 0 ? 1e30 : std::pow(static_cast<double>(r), -2.0 * delta_true));
    }
    auto f = fit_scaling_dimension(C, 2, 20);
    char buf[160];
    std::snprintf(buf, sizeof buf, "(Δ=%.4f R²=%.4f n=%d valid=%s)",
                  f.delta, f.r2, f.n_points, f.valid ? "y" : "n");
    check("SD1 fit_scaling_dimension valid", f.valid, buf);
    check("SD1 Δ within 5% of 1.5", std::abs(f.delta - 1.5) / 1.5 < 0.05, buf);
    check("SD1 R² > 0.999", f.r2 > 0.999, buf);
}

// SD2 — synthetic C(r) = exp(-r/λ) with λ=5 must recover ξ=5, mass=0.2
static void sd2_exp_fit() {
    std::puts("\n--- SD2: fit_exponential_decay recovers ξ=5, m=0.2 ---");
    std::vector<double> C;
    for (int r = 0; r < 25; ++r) C.push_back(std::exp(-r / 5.0));
    auto f = fit_exponential_decay(C, 0, 25);
    char buf[160];
    std::snprintf(buf, sizeof buf, "(ξ=%.4f m=%.4f A=%.4f R²=%.5f valid=%s)",
                  f.xi, f.mass, f.amplitude, f.r2, f.valid ? "y" : "n");
    check("SD2 fit_exponential_decay valid", f.valid, buf);
    check("SD2 ξ within 1% of 5.0", std::abs(f.xi - 5.0) / 5.0 < 0.01, buf);
    check("SD2 mass within 1% of 0.2", std::abs(f.mass - 0.2) / 0.2 < 0.01, buf);
    check("SD2 R² > 0.9999", f.r2 > 0.9999, buf);
}

// ========== measure_v_of_r ==========

// VR1 — the V(r) pipeline just runs without crashing on small L
//       (we don't validate the numerical α here because extracting
//       an accurate α on L=8 with short ticks is not meaningful;
//       for that we'd use Phase E benchmarks. Here we just verify
//       the plumbing: N+2 pipelines, N V-samples, a valid fit
//       struct.)
static void vr1_plumbing() {
    std::puts("\n--- VR1: measure_v_of_r plumbing on L=16 ---");
    const std::vector<int> r_values{4, 6};
    auto res = measure_v_of_r<BackendCpu>(16, r_values, /*n_ticks=*/20);
    char buf[192];
    std::snprintf(buf, sizeof buf,
                  "(L=%d n_ticks=%d |data|=%zu e_self+=%.4f e_self-=%.4f α_fit=%.4f)",
                  res.L, res.n_ticks, res.data.size(),
                  res.e_self_pos, res.e_self_neg, res.alpha_fit);
    check("VR1 returns data for each r", res.data.size() == r_values.size(), buf);
    check("VR1 r-values preserved",
          res.data.size() == 2 && res.data[0].r == 4 && res.data[1].r == 6);
    // α_fit is only computed when |data| >= 3; with 2 r-points it stays default.
    check("VR1 α_fit default (too few points for fit)", res.alpha_fit == 0.0);
}

int main() {
    ftd::test::contract({
        "observable/measurement-map",
        "[MEASUREMENT]",
        "pipeline device state, observable descriptor, analytical fixture",
        "CPU pipeline backend for analytical observable tests",
        "field_energy, state_histogram, flux_correlator",
        "finite pipeline lattice fixture",
        "CPU unit test; GPU parity covered separately",
        "observables reproduce known analytical configurations",
        "failure means observable map implementation is wrong"});

    std::puts("================================================================");
    std::puts("  Sim Pipeline — Phase D Observable Unit Tests");
    std::puts("================================================================");

    fc1_uniform();
    fc2_plane_wave();
    ew1_known_injection();
    fea1_matches_direct();
    sd1_recovery();
    sd2_exp_fit();
    vr1_plumbing();

    std::puts("\n----------------------------------------------------------------");
    if (g_failures == 0) {
        std::puts("  All Phase-D observable checks PASS");
        return 0;
    }
    std::printf("  %d Phase-D check(s) FAILED\n", g_failures);
    return 1;
}
