/**
 * Emergent Measurements — High-Fidelity Lattice Physics Verification
 *
 * Five quantitative tests using high SOR resolution to measure emergent
 * quantities that arise from the six FTD update rules:
 *
 *   EM1: Coulomb exponent (theory: -2.0)
 *   EM2: Coulomb-solver consistency — force amplitude vs. its input coupling
 *   EM3: Larmor power ∝ a² (accelerated charge loses more energy)
 *   EM4: Wave group velocity (theory: C_WAVE = 1/√3)
 *   EM5: Gauss violation at high SOR (theory: RMS → 0)
 *
 * EPISTEMIC NOTE (2026-04-23): EM2 is a CONSISTENCY CHECK of the Poisson +
 * Gauss + force-readout pipeline against the coupling constant it consumes
 * (ALPHA_EFT = G_C² = ALPHA by construction, see constants.h). It is NOT a
 * derivation of the physical fine-structure constant. The EFT Recovery
 * Program (docs/theory/10_eft_program/scopes_and_specs/SPEC_EFT_RECOVERY_PROGRAM.md) found
 * α_∞ ≈ 3.6 × α_ref on the emergent_forces path in the continuum limit —
 * that is where the physical-α comparison lives, not here.
 *
 * Uses set_sor_iterations(30) for scientific accuracy.
 */

#include <cmath>
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include "ftd/render_bridge.h"
#include "ftd/constants.h"

// Minimal test framework (matches engine convention)
static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg) do { \
    if (cond) { std::cerr << "  PASS  " << msg << "\n"; g_pass++; } \
    else      { std::cerr << "  FAIL  " << msg << "\n"; g_fail++; } \
} while(0)

// ================================================================
// EM1: Coulomb Force Law Exponent
// ================================================================
void test_coulomb_exponent() {
    std::cerr << "\n--- EM1: Coulomb Force Law Exponent ---\n";
    const int L = 48;
    const int mid = L / 2;
    const int setup_ticks = 100;

    std::vector<double> r_vals, f_vals;
    for (int r = 4; r <= 18; r += 2) {
        ftd::RenderBridge rb(L);
        rb.set_sor_iterations(30);  // High accuracy
        rb.toggles.genesis = false;
        rb.toggles.gravity = false;

        rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

        int px = mid + r;
        rb.inject_particle(px, mid, mid, -1, {0, 0, -ftd::K_B});
        rb.voxels()[rb.lattice().index(px, mid, mid)].locked = true;

        rb.run(setup_ticks);
        double f = rb.force_diag_at(px, mid, mid).f_coulomb.mag();

        if (f > 1e-30) {
            r_vals.push_back(static_cast<double>(r));
            f_vals.push_back(f);
        }
    }

    // Log-log linear regression
    double sx = 0, sy = 0, sxx = 0, sxy = 0;
    int n = static_cast<int>(r_vals.size());
    for (int i = 0; i < n; ++i) {
        double lx = std::log(r_vals[i]), ly = std::log(f_vals[i]);
        sx += lx; sy += ly; sxx += lx * lx; sxy += lx * ly;
    }
    double denom = n * sxx - sx * sx;
    double exponent = (denom > 1e-30) ? (n * sxy - sx * sy) / denom : 0.0;

    double err_pct = 100.0 * std::abs(exponent - (-2.0)) / 2.0;
    std::cerr << "    Exponent: " << exponent << " (theory: -2.0, err: " << err_pct << "%)\n";
    CHECK(err_pct < 15.0, "Coulomb exponent within 15% of -2.0");
}

// ================================================================
// EM2: Coulomb-Solver Consistency (α_in vs. α_out)
// ----------------------------------------------------------------
// This measures F = α / (4 π r²) and compares the extracted α to the
// coupling constant ALPHA fed into the Poisson solver. Passing means the
// Poisson→Gauss→force-readout path is self-consistent to within the SOR
// tolerance. It does NOT derive α from first principles — the physical-α
// question lives in the EFT Recovery Program on the emergent_forces path.
// ================================================================
void test_alpha_extraction() {
    std::cerr << "\n--- EM2: Coulomb-Solver Consistency (α_in vs. α_out) ---\n";
    const int L = 48;
    const int mid = L / 2;
    const int r = 12;  // Far enough to avoid self-field, close enough for signal

    ftd::RenderBridge rb(L);
    rb.set_sor_iterations(30);
    rb.toggles.genesis = false;
    rb.toggles.gravity = false;

    rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
    rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

    int px = mid + r;
    rb.inject_particle(px, mid, mid, -1, {0, 0, -ftd::K_B});
    rb.voxels()[rb.lattice().index(px, mid, mid)].locked = true;

    rb.run(150);

    double F = rb.force_diag_at(px, mid, mid).f_coulomb.mag();
    // F = alpha / (4*pi*r^2) for unit charges in Poisson convention
    double alpha_meas = F * 4.0 * ftd::PI * r * r;
    // Target is the solver's input coupling, not the physical CODATA value.
    const double alpha_in = ftd::ALPHA_EFT;
    double err_pct = 100.0 * std::abs(alpha_meas - alpha_in) / alpha_in;

    std::cerr << "    alpha_measured: " << alpha_meas
              << " (alpha_in = ALPHA_EFT = " << alpha_in
              << ", err: " << err_pct << "%)\n";
    CHECK(err_pct < 10.0, "Coulomb solver reproduces alpha_in within 10%");
}

// ================================================================
// EM3: Larmor Radiation — Accelerated vs Static Energy
// ================================================================
void test_larmor_radiation() {
    std::cerr << "\n--- EM3: Larmor Radiation ---\n";
    const int L = 32;
    const int mid = L / 2;
    const int ticks = 100;

    double E_accel = 0, E_static = 0;

    for (int test = 0; test < 2; ++test) {
        ftd::RenderBridge rb(L);
        rb.toggles.genesis = false;
        rb.toggles.larmor_radiation = true;
        rb.toggles.selective_damping = true;

        rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});

        if (test == 0) {
            // Give velocity for acceleration
            rb.voxels()[rb.lattice().index(mid, mid, mid)].velocity = {0.3, 0, 0};
        }

        rb.run(ticks);
        auto audit = rb.energy_audit();
        double E = audit.field_energy + audit.wave_energy;
        if (test == 0) E_accel = E;
        else E_static = E;
    }

    // Accelerated charge should lose MORE energy (lower final E)
    std::cerr << "    E_accel=" << E_accel << " E_static=" << E_static << "\n";
    CHECK(E_accel < E_static, "Accelerated charge loses more energy than static (Larmor)");
}

// ================================================================
// EM4: Wave Group Velocity
// ================================================================
void test_wave_speed() {
    std::cerr << "\n--- EM4: Wave Group Velocity ---\n";
    const int L = 48;
    const int mid = L / 2;
    const int total_ticks = 40;

    ftd::RenderBridge rb(L);
    rb.toggles.genesis = false;
    rb.toggles.gravity = false;

    rb.inject_flux(mid, mid, mid, {0, 0, 5.0});
    rb.run(total_ticks);

    // Find furthest site above threshold along +x
    double threshold = 0.001;
    int furthest = 0;
    for (int dx = 1; dx < L / 3; ++dx) {
        int idx = rb.lattice().index(mid + dx, mid, mid);
        double rho = rb.voxels()[idx].density();
        if (rho > threshold) furthest = dx;
    }

    double measured = (furthest > 0) ? static_cast<double>(furthest) / total_ticks : 0.0;
    double theory = ftd::C_WAVE;
    double err_pct = (theory > 0) ? 100.0 * std::abs(measured - theory) / theory : 100.0;

    std::cerr << "    Speed: " << measured << " (theory: " << theory
              << ", front at " << furthest << ")\n";
    CHECK(err_pct < 40.0, "Wave speed within 40% of C_WAVE");
}

// ================================================================
// EM5: Gauss Violation at High SOR Resolution
// ================================================================
void test_gauss_high_sor() {
    std::cerr << "\n--- EM5: Gauss Constraint at High SOR ---\n";
    const int L = 32;
    const int mid = L / 2;

    ftd::RenderBridge rb(L);
    rb.set_sor_iterations(30);
    rb.toggles.genesis = false;

    rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
    rb.inject_particle(mid + 5, mid, mid, -1, {0, 0, -ftd::K_B});
    rb.run(100);

    auto audit = rb.energy_audit();
    double gauss_rms = std::sqrt(audit.gauss_violation / (L * L * L));

    std::cerr << "    Gauss RMS: " << std::scientific << gauss_rms << "\n";
    // Note: The Gauss constraint is continuously sourced by the self-field
    // coupling (g_c * grad(s)), so RMS approaches but cannot reach zero even
    // at high SOR. Expect O(0.01) for a system with manifested particles.
    CHECK(gauss_rms < 0.01, "Gauss violation RMS < 0.01 at SOR=30");
}

// ================================================================
// Main
// ================================================================
int main() {
    std::cerr << "============================================\n";
    std::cerr << "  EMERGENT MEASUREMENTS — High-Fidelity Test\n";
    std::cerr << "============================================\n";

    test_coulomb_exponent();
    test_alpha_extraction();
    test_larmor_radiation();
    test_wave_speed();
    test_gauss_high_sor();

    std::cerr << "\n============================================\n";
    std::cerr << "  " << g_pass << " passed, " << g_fail << " failed\n";
    std::cerr << "============================================\n";

    return g_fail > 0 ? 1 : 0;
}
