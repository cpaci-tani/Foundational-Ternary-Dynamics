/**
 * test_leapfrog_integrator_audit.cpp — closes TRACKER_OPEN_ITEMS §1.4.
 *
 * The advance pair in phase_write is:
 *
 *     wave_vel += delta_j;   // delta_j = c² ∇²J(t) computed in phase_read
 *     flux     += wave_vel;
 *
 * A previous audit mis-read this as forward Euler. Under the convention
 * where wave_vel = v(t + h/2) and flux = J(t), this is exactly Störmer–
 * Verlet leapfrog:
 *
 *     v(t + h/2) = v(t − h/2) + a(J(t)) · h         (kick)
 *     J(t + h)   = J(t)       + v(t + h/2) · h       (drift)
 *
 * Leapfrog is symplectic: the pseudo-Hamiltonian H̃ = ½|v|² + ½c²|∇J|²
 * is conserved to O(h²) per step and bounded forever — no secular drift
 * in a pure wave simulation without dissipation.
 *
 * This test seeds a pure wave packet, turns off damping and Gauss
 * projection (both drain energy on purpose), runs the sim for N ticks,
 * and asserts the EnergyLedger's cumulative-residual magnitude stays
 * within O(h²)-bounded noise (no secular growth).
 *
 * If this test passes, §1.4 is empirically closed — the integrator is
 * already correct and no code change is needed. If it fails, the pair
 * must be swapped for explicit half-kick leapfrog.
 */

#include <cmath>
#include <iostream>
#include <iomanip>

#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "ftd/test_telemetry.h"

using namespace ftd;

// Run N ticks of a pure wave simulation and report the energy-drift stats.
// Damping OFF, Gauss OFF, no particles — only the Laplacian advance runs.
static EnergyLedger run_pure_wave(int lattice_size, int ticks, double seed_amplitude) {
    RenderBridge rb(lattice_size);

    // Strip every optional physics path — test ONLY the leapfrog advance.
    rb.toggles.damping           = false;
    rb.toggles.gauss_projection  = false;
    rb.toggles.genesis           = false;
    rb.toggles.forces            = false;
    rb.toggles.movement          = false;
    rb.toggles.lorentz_force     = false;
    rb.toggles.gravity           = false;
    rb.toggles.poisson_coulomb   = false;
    rb.toggles.emergent_forces   = false;
    rb.toggles.coupling          = false;  // no state → flux source
    rb.toggles.selective_damping = false;
    rb.toggles.dual_substrate    = false;
    rb.toggles.weak_transmutation = false;
    // wave_propagation stays true — that's the thing we're testing.

    // Seed a sub-manifestation flux pulse at the centre.  Amplitude is kept
    // well below K_GENESIS so nothing manifests even if genesis were on.
    int c = lattice_size / 2;
    rb.inject_flux(c, c, c, Vec3{seed_amplitude, 0.0, 0.0});

    rb.run(ticks);
    return rb.energy_ledger();
}

int main() {
    std::cout << "=== Leapfrog integrator audit (closes TRACKER_OPEN_ITEMS §1.4) ===\n\n";

    // ── Why we measure cumulative balance, not per-tick drift ──────────
    // EnergyLedger.E_total = ½(Σ|J|² + Σ|v|²) is an L² indicator, NOT the
    // true conserved Hamiltonian of the wave equation. The real H
    // involves |∇J|², and energy sloshes between the two forms every
    // half-period — so per-tick drift_frac looks large even in a
    // perfectly symplectic simulation.
    //
    // The symplectic signature is CUMULATIVE BALANCE: positive drifts
    // (energy "gained" in one form at the L² level) are cancelled by
    // negative drifts (energy "given back" when it sloshes). Over many
    // periods, injection ≈ dissipation with no secular drift.
    //
    // Euler, by contrast, would show imbalance growing with tick count.
    // ──────────────────────────────────────────────────────────────────

    auto assert_balanced = [](const char* label, const EnergyLedger& led,
                              double tol_frac) {
        double inj = led.cumulative_injection;
        double dis = led.cumulative_dissipation;
        double avg = 0.5 * (inj + dis);
        double imbalance = std::abs(inj - dis) / std::max(avg, 1e-12);
        std::cout << "  cumulative_injection   = " << inj << "\n";
        std::cout << "  cumulative_dissipation = " << dis << "\n";
        std::cout << "  |injection-dissipation|/avg = " << imbalance << "\n";
        ftd::test::check(label, imbalance < tol_frac);
    };

    // --- Baseline: short run at modest amplitude ---
    std::cout << "--- 1000 ticks, L=16, amplitude 0.1 ---\n";
    {
        auto led = run_pure_wave(16, 1000, 0.1);
        std::cout << std::scientific << std::setprecision(3);
        std::cout << "  E_curr                 = " << led.E_curr << "\n";
        std::cout << "  max_residual_seen      = " << led.max_residual_seen << "\n";
        assert_balanced("L=16: cumulative injection ≈ dissipation (< 5%)",
                        led, 5.0e-2);
    }

    // --- Stability at larger lattice ---
    std::cout << "\n--- 500 ticks, L=32, amplitude 0.5 ---\n";
    {
        auto led = run_pure_wave(32, 500, 0.5);
        std::cout << "  E_curr                 = " << led.E_curr << "\n";
        std::cout << "  max_residual_seen      = " << led.max_residual_seen << "\n";
        assert_balanced("L=32: cumulative injection ≈ dissipation (< 5%)",
                        led, 5.0e-2);
    }

    // --- Long-run stability test: 5000 ticks, check no secular drift ---
    std::cout << "\n--- 5000 ticks, L=16, amplitude 0.1 (long-run) ---\n";
    {
        auto led = run_pure_wave(16, 5000, 0.1);
        std::cout << "  E_curr                 = " << led.E_curr << "\n";
        // Over 5000 ticks the symplectic invariant would fail on a
        // non-symplectic integrator (drift grows ∝ N), but leapfrog
        // keeps injection ≈ dissipation to the same tolerance.
        assert_balanced("5000 ticks: no secular drift (< 5%)",
                        led, 5.0e-2);
    }

    // --- No-physics null check: empty lattice ---
    std::cout << "\n--- 500 ticks, empty lattice (no injection) ---\n";
    {
        RenderBridge rb(16);
        rb.toggles.damping           = false;
        rb.toggles.gauss_projection  = false;
        rb.toggles.genesis           = false;
        rb.toggles.forces            = false;
        rb.toggles.movement          = false;
        rb.toggles.lorentz_force     = false;
        rb.toggles.gravity           = false;
        rb.toggles.poisson_coulomb   = false;
        rb.toggles.coupling          = false;
        rb.toggles.weak_transmutation = false;
        rb.run(500);
        const auto& led = rb.energy_ledger();
        ftd::test::check("Empty lattice: zero E", std::abs(led.E_curr) < 1e-20);
        ftd::test::check("Empty lattice: zero residual", led.max_residual_seen < 1e-10);
    }

    return ftd::test::finalize();
}
