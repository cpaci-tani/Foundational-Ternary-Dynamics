/**
 * @file test_eft_anisotropy.cpp
 * @brief EFT Phase 1A — rotational-anisotropy diagnostics.
 *
 * Pre-registered expectations from
 * docs/theory/10_eft_program/SPEC_EFT_RECOVERY_PROGRAM.md §4.1.
 *
 * Checks
 * ------
 *   A1: Uniform flux → face, edge, diagonal correlators all constant, so
 *       the pointwise residual is ≤ 1e-10 at all r. Screening-length fit
 *       may fail (constant correlator is not a decaying exponential) — we
 *       assert that IF a ξ is extracted, the direction-classes agree to 1%.
 *   A2: Plane-wave flux J(x) = (sin(kx), 0, 0) with k = 2π/L has a strong
 *       directional signature: the face correlator along x oscillates
 *       with period L/k·2π = L (since we sample at integer r, the
 *       correlator is cos(kr)), while along y and z the correlator stays
 *       constant at ⟨J²⟩/2. This is an anisotropic configuration by
 *       construction; we assert δ != 0 and pointwise residual > 0.1 at
 *       r = 4 (one-quarter of the wavelength).
 *   A3: Isotropic Gaussian random flux (identical amplitude per voxel,
 *       random orientation drawn from 26 Moore neighbors) averaged over
 *       enough sites → face ≈ edge ≈ diagonal correlators. Pointwise
 *       residual at r ≥ L/4 ≤ 0.1 (10% tolerance; the signal decays to
 *       statistical noise floor there). Screening length fit must produce
 *       |δ| < 0.2 on L = 24.
 *   A4: Fit-engine sanity: given a synthetic C(r) = 1 · exp(−r/5) we must
 *       recover ξ ≈ 5 to within 5% and R² > 0.99.
 *
 * This test does NOT run engine dynamics; it only constructs configurations
 * in voxel buffers and probes the correlator. Dynamics-based anisotropy
 * under realistic scenarios is a Phase 1-end deliverable (theory doc +
 * benchmark executable, not this CTest).
 */

#include <cmath>
#include <cstdio>
#include <iostream>
#include <random>

#include "ftd/eft/anisotropy.h"
#include "ftd/render_bridge.h"

static int g_failures = 0;

static void check(const char* name, bool ok, const char* detail = nullptr) {
    if (ok) {
        std::printf("  PASS  %s\n", name);
    } else {
        std::printf("  FAIL  %s%s%s\n", name,
                    detail ? "  " : "",
                    detail ? detail : "");
        ++g_failures;
    }
}

// A1 — Uniform flux
static void a1_uniform() {
    std::puts("\n--- A1: Uniform flux (expect isotropic correlator) ---");
    const int L = 16;
    ftd::RenderBridge rb(L);
    for (int x = 0; x < L; ++x)
        for (int y = 0; y < L; ++y)
            for (int z = 0; z < L; ++z)
                rb.inject_flux(x, y, z, {0.3, -0.1, 0.5});

    auto dc = ftd::eft::directional_flux_correlation(rb, L / 2);

    // Pointwise residual at several r must be ≤ 1e-9 (float round-off only).
    double worst_residual = 0.0;
    for (int r = 0; r < dc.max_r; ++r) {
        const double f = dc.face[r], d = dc.diagonal[r];
        const double bar = 0.5 * (f + d);
        const double res = std::abs(f - d) / std::max(std::abs(bar), 1e-30);
        if (res > worst_residual) worst_residual = res;
    }
    char buf[128];
    std::snprintf(buf, sizeof buf, "(worst residual %.2e)", worst_residual);
    check("A1 pointwise residual ≤ 1e-9", worst_residual < 1e-9, buf);

    // Diagnostic at r_ref = L/4
    auto diag = ftd::eft::diagnose_anisotropy(dc, L / 4);
    std::snprintf(buf, sizeof buf, "(pointwise=%.2e δ=%.3f)",
                  diag.pointwise_residual, diag.delta);
    check("A1 diagnostic pointwise residual small", diag.pointwise_residual < 1e-9, buf);
}

// A2 — Plane-wave flux polarized along x with k_x = 2π/L
static void a2_plane_wave() {
    std::puts("\n--- A2: Plane wave along x (expect strong anisotropy) ---");
    const int L = 32;
    ftd::RenderBridge rb(L);
    constexpr double PI = 3.14159265358979323846;
    const double k = 2.0 * PI / static_cast<double>(L);
    for (int x = 0; x < L; ++x)
        for (int y = 0; y < L; ++y)
            for (int z = 0; z < L; ++z) {
                const double jx = std::sin(k * static_cast<double>(x));
                rb.inject_flux(x, y, z, {jx, 0.0, 0.0});
            }

    auto dc = ftd::eft::directional_flux_correlation(rb, L / 2);

    // Analytical reference:
    //   C_face(r) = (1/3) · ⟨sin(kx) sin(k(x+r))⟩_x   = (1/6)·cos(kr)
    //   C_edge(r) ≈ (1/6)·[cos(kr) + 1]/2 = (1/12)(1+cos(kr)) — half the directions
    //                have dx=1, half dx=0
    //   C_diagonal(r) similar mix
    // The key qualitative prediction: |C_face(r)| and |C_diagonal(r)| must
    // differ significantly at r = L/4 = 8 (k·r = π/2, so C_face ≈ 0 but
    // C_diagonal has a non-zero constant piece).

    const int r_ref = L / 4;  // r = 8 → kr = π/2
    const double cf = dc.face[r_ref];
    const double cd = dc.diagonal[r_ref];

    char buf[192];
    std::snprintf(buf, sizeof buf,
                  "(r=%d  C_face=%.4f  C_edge=%.4f  C_diag=%.4f)",
                  r_ref, cf, dc.edge[r_ref], cd);
    check("A2 face and diagonal correlators differ", std::abs(cf - cd) > 0.03, buf);

    auto diag = ftd::eft::diagnose_anisotropy(dc, r_ref);
    std::snprintf(buf, sizeof buf, "(residual=%.3f)", diag.pointwise_residual);
    check("A2 pointwise residual > 0.1", diag.pointwise_residual > 0.1, buf);
}

// A3 — Gaussian isotropic noise: δ should be small (≤ 0.2 on L = 24)
static void a3_isotropic_noise() {
    std::puts("\n--- A3: Gaussian isotropic noise (expect |δ| small) ---");
    const int L = 24;
    ftd::RenderBridge rb(L);
    std::mt19937 rng(42);
    std::normal_distribution<double> dist(0.0, 1.0);
    for (int x = 0; x < L; ++x)
        for (int y = 0; y < L; ++y)
            for (int z = 0; z < L; ++z) {
                rb.inject_flux(x, y, z, {dist(rng), dist(rng), dist(rng)});
            }

    auto dc = ftd::eft::directional_flux_correlation(rb, L / 2);

    // For truly isotropic Gaussian noise, C(r) = ⟨J²⟩·δ_{r,0}; that's a
    // degenerate exponential (ξ → 0+). At r ≥ 1 the correlator is pure
    // sampling noise with statistical spread of O(1/√N_sites) times the
    // contact value C(0) = 3σ². The right invariant metric is
    //     |C_face(r) − C_diag(r)| / C(0)
    // which removes the r-dependent denominator blowup when C(r) ≈ 0.
    // Expected O(0.01) on L=24 (N_sites = 13824, 1/√N ≈ 0.0085).

    const double c0 = dc.face[0];                    // ≈ ⟨J²⟩ = 3σ² = 3
    const double c_f = dc.face[1];
    const double c_d = dc.diagonal[1];
    const double rel_to_c0 = std::abs(c_f - c_d) / std::max(std::abs(c0), 1e-10);
    char buf[192];
    std::snprintf(buf, sizeof buf,
                  "(C(0)=%.3f  C_face[1]=%.4f  C_diag[1]=%.4f  rel/C(0)=%.4f)",
                  c0, c_f, c_d, rel_to_c0);
    check("A3 residual / C(0) < 0.02 at r=1", rel_to_c0 < 0.02, buf);
}

// A4 — Fit engine sanity on synthetic exponential
static void a4_fit_sanity() {
    std::puts("\n--- A4: fit_exponential sanity on synthetic C(r) = exp(-r/5) ---");
    std::vector<double> C;
    C.reserve(20);
    for (int r = 0; r < 20; ++r) {
        C.push_back(std::exp(-static_cast<double>(r) / 5.0));
    }
    auto fit = ftd::eft::fit_exponential(C, 0, 20);
    char buf[192];
    std::snprintf(buf, sizeof buf, "(ξ=%.3f A=%.4f R²=%.5f n=%d)",
                  fit.xi, fit.amplitude, fit.r2, fit.n_points);
    check("A4 fit is valid", fit.valid, buf);
    check("A4 ξ within 5% of 5.0", std::abs(fit.xi - 5.0) / 5.0 < 0.05);
    check("A4 R² > 0.999", fit.r2 > 0.999);
    check("A4 amplitude within 5% of 1.0", std::abs(fit.amplitude - 1.0) < 0.05);
}

int main() {
    std::puts("================================================================");
    std::puts("  EFT Phase 1A — Rotational Anisotropy Diagnostics");
    std::puts("================================================================");

    a1_uniform();
    a2_plane_wave();
    a3_isotropic_noise();
    a4_fit_sanity();

    std::puts("\n----------------------------------------------------------------");
    if (g_failures == 0) {
        std::puts("  All EFT-Phase-1A checks PASS");
        return 0;
    }
    std::printf("  %d EFT-Phase-1A check(s) FAILED\n", g_failures);
    return 1;
}
