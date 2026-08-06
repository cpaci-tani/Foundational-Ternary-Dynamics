/**
 * test_wave_spin_invariant.cpp — field circulation ledger gate.
 *
 * THEOREM (stencil-independent): the free componentwise wave update
 *
 *     wave_vel += c² ∇²J    (kick)
 *     flux     += wave_vel   (drift)
 *
 * exactly conserves the total field spin  S = Σ_i J_i × W_i.  Proof: per
 * Fourier mode with symbol m, each component pair maps as
 * J' = (1−m)J + W,  W' = W − mJ, so the antisymmetric bilinear
 * L_ab = J_a W_b − J_b W_a transforms as (1−m)L + mL = L for ANY m — the
 * proof never uses the stencil, so it holds for the production 18-point
 * isotropic Laplacian exactly as for any other componentwise stencil.
 * The same algebra applies to mixed products of two solutions, so the
 * dual-substrate (L/R) recombined fields conserve S as well.
 *
 * The static twist H = Σ_i J_i · (∇×J)_i is NOT conserved (per-mode
 * symmetric bilinears mix under evolution) — asserted here as a negative
 * control so the ledger's asymmetry is visible.
 *
 * Spin-breaking channels intentionally OUTSIDE this test (audited
 * 2026-07-28): damping (flux *= 1−γ), Gauss projection (projects flux
 * without co-projecting wave_vel — a spin torque), and the state-coupling
 * sources (−G_C ∇s + G_C curl(s·v) — the physical matter↔field spin
 * exchange). All are toggled off; this gate covers the free sector only.
 */

#include <cmath>
#include <iostream>
#include <iomanip>
#include <algorithm>

#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "ftd/diagnostics_compute.h"
#include "ftd/render_bridge_diagnostics.h"
#include "ftd/test_telemetry.h"

using namespace ftd;

namespace {

constexpr double kTwoPi = 6.283185307179586476925286766559;

// Strip every optional physics path — free wave advance only.
void strip_toggles(RenderBridge& rb, bool dual) {
    rb.toggles.damping            = false;
    rb.toggles.gauss_projection   = false;
    rb.toggles.genesis            = false;
    rb.toggles.forces             = false;
    rb.toggles.movement           = false;
    rb.toggles.lorentz_force      = false;
    rb.toggles.gravity            = false;
    rb.toggles.poisson_coulomb    = false;
    rb.toggles.emergent_forces    = false;
    rb.toggles.coupling           = false;   // no state → flux source
    rb.toggles.selective_damping  = false;
    rb.toggles.weak_transmutation = false;
    rb.toggles.dual_substrate     = dual;
}

// ABC-flow (Beltrami) seed: strongly helical, multi-mode, deterministic.
// wave_vel is seeded with a phase-shifted copy so S(0) is solidly nonzero.
void seed_abc(RenderBridge& rb, int L, double amp) {
    const double k = kTwoPi / L;
    auto& voxels = rb.voxels();
    const auto& lattice = rb.lattice();
    const int N = static_cast<int>(lattice.total_sites());
    for (int i = 0; i < N; ++i) {
        Coord c = lattice.coord(i);
        const double x = k * c.x, y = k * c.y, z = k * c.z;
        Vec3 J{amp * (std::sin(z) + 0.5 * std::cos(y)),
               amp * (std::sin(x) + 0.5 * std::cos(z)),
               amp * (std::sin(y) + 0.5 * std::cos(x))};
        Vec3 W{0.3 * amp * (std::cos(z) - 0.5 * std::sin(y)),
               0.3 * amp * (std::cos(x) - 0.5 * std::sin(z)),
               0.3 * amp * (std::cos(y) - 0.5 * std::sin(x))};
        voxels[i].flux = J;
        voxels[i].wave_vel = W;
        if (rb.toggles.dual_substrate) {
            voxels[i].flux_L = J * 0.5;
            voxels[i].flux_R = J * 0.5;
            voxels[i].wave_vel_L = W * 0.5;
            voxels[i].wave_vel_R = W * 0.5;
        }
    }
}

// Cancellation scale for the S sum: Σ |J_i × W_i| (all-positive version of
// the invariant), so the float-noise gate is amplitude-aware.
double spin_scale(const RenderBridge& rb) {
    double s = 0.0;
    for (const auto& v : rb.voxels()) {
        s += Vec3::cross(v.flux, v.wave_vel).mag();
    }
    return s;
}

struct RunResult {
    Vec3 S0;
    double H0 = 0.0;
    double max_spin_drift = 0.0;
    double max_helicity_drift = 0.0;
    double scale = 0.0;
    Vec3 S_final_diag;      // from compute_diagnostics
    Vec3 S_final_manual;    // recomputed directly from voxels
};

RunResult run_free_wave(int L, int ticks, bool dual) {
    RenderBridge rb(L);
    strip_toggles(rb, dual);
    seed_abc(rb, L, 0.05);   // well below K_GENESIS

    RunResult r;
    r.scale = spin_scale(rb);
    Diagnostics d0 = compute_diagnostics(rb);
    r.S0 = d0.field_spin;
    r.H0 = d0.field_helicity;

    Diagnostics d;
    for (int n = 0; n < ticks; ++n) {
        rb.run(1);
        d = compute_diagnostics(rb);
        r.max_spin_drift = std::max(
            r.max_spin_drift,
            std::max({std::abs(d.field_spin.x - r.S0.x),
                      std::abs(d.field_spin.y - r.S0.y),
                      std::abs(d.field_spin.z - r.S0.z)}));
        r.max_helicity_drift =
            std::max(r.max_helicity_drift, std::abs(d.field_helicity - r.H0));
    }
    r.S_final_diag = d.field_spin;
    Vec3 manual;
    for (const auto& v : rb.voxels()) {
        manual += Vec3::cross(v.flux, v.wave_vel);
    }
    r.S_final_manual = manual;
    return r;
}

}  // namespace

int main() {
    std::cout << "=== Wave-sector spin invariant (field circulation ledger) ===\n";
    std::cout << std::scientific << std::setprecision(3);

    // --- Arm A: single substrate, 512 ticks, L=16 ---
    {
        auto r = run_free_wave(16, 512, /*dual=*/false);
        std::cout << "\n--- single substrate, L=16, 512 ticks ---\n";
        std::cout << "  S0 = (" << r.S0.x << ", " << r.S0.y << ", " << r.S0.z
                  << ")  scale = " << r.scale << "\n";
        std::cout << "  max spin drift     = " << r.max_spin_drift << "\n";
        std::cout << "  H0 = " << r.H0
                  << "  max helicity drift = " << r.max_helicity_drift << "\n";
        ftd::test::check("A: nonzero invariant (test is non-vacuous)",
                         r.S0.mag() > 1e-6 * std::max(r.scale, 1e-30));
        ftd::test::check("A: spin conserved (free sector, 18-pt stencil)",
                         r.max_spin_drift <= 1e-10 * std::max(r.scale, 1e-6));
        ftd::test::check("A: helicity NOT conserved (negative control)",
                         r.max_helicity_drift >
                             1e-3 * std::max(std::abs(r.H0), 1e-12));
        ftd::test::check("A: diagnostics plumbing exact",
                         (r.S_final_diag.x == r.S_final_manual.x) &&
                         (r.S_final_diag.y == r.S_final_manual.y) &&
                         (r.S_final_diag.z == r.S_final_manual.z));
    }

    // --- Arm B: dual substrate (default-on production path), 512 ticks ---
    {
        auto r = run_free_wave(16, 512, /*dual=*/true);
        std::cout << "\n--- dual substrate, L=16, 512 ticks ---\n";
        std::cout << "  S0 = (" << r.S0.x << ", " << r.S0.y << ", " << r.S0.z
                  << ")  scale = " << r.scale << "\n";
        std::cout << "  max spin drift     = " << r.max_spin_drift << "\n";
        ftd::test::check("B: nonzero invariant (dual path)",
                         r.S0.mag() > 1e-6 * std::max(r.scale, 1e-30));
        ftd::test::check("B: spin conserved (dual substrate recombination)",
                         r.max_spin_drift <= 1e-10 * std::max(r.scale, 1e-6));
    }

    return ftd::test::finalize();
}
