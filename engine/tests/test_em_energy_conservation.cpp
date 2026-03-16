/**
 * Test: EM Energy Conservation in Undamped Vacuum
 *
 * Verifies that total electromagnetic energy is conserved when there are
 * no particles, no coupling, and no damping — pure wave equation dynamics.
 *
 * The leapfrog (kick-drift) scheme for d²J/dt² = c²∇²J conserves a
 * discrete Hamiltonian exactly (up to floating-point rounding):
 *
 *   H = Σ |wave_vel|² + c² × GradientEnergy(J)
 *
 * where GradientEnergy = -Σ J(v) · L₁₈(J(v))  (the negative inner product
 * of J with its own 18-point isotropic Laplacian, which equals |∇J|² on
 * periodic lattices).
 *
 * IMPORTANT: The naive measure |J|² + |wv|² is NOT conserved — it oscillates
 * as energy sloshes between field amplitude and wave velocity. The gradient
 * energy |∇J|² is the correct potential energy for the wave equation.
 *
 * Tests:
 *   M7a: Localized Gaussian pulse — H conserved over 2000 ticks
 *   M7b: Traveling plane wave — H conserved over 500 ticks
 *   M7c: Standing wave — H conserved over 1000 ticks
 *   M7d: Two counter-propagating pulses (interference) — H conserved
 *   M7e: Energy partition — field_energy and wave_energy oscillate (qualitative)
 *
 * Theory references:
 *   - SPEC_ENGINE.md §3 (Leapfrog integration preserves symplectic structure)
 *   - CLAUDE.md §3.2 (Flux wave equation)
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <iomanip>
#include "ftd/render_bridge.h"
#include "ftd/constants.h"

static int g_failures = 0;

static void check(const char* name, bool condition) {
    if (condition) {
        std::cout << "  PASS  " << name << "\n";
    } else {
        std::cout << "  FAIL  " << name << "\n";
        ++g_failures;
    }
}

/**
 * Compute the EXACT conserved Hamiltonian for the kick-drift leapfrog.
 *
 * The engine uses kick-drift (symplectic Euler):
 *   wv  ← wv + c² · L₁₈(J)     [kick]
 *   J   ← J + wv                 [drift]
 *
 * This preserves the MODIFIED Hamiltonian (derived from symplectic structure):
 *
 *   H_mod = ½Σ|wv|² + ½c²·Σ wv·L₁₈(J) + ½c²·(-Σ J·L₁₈(J))
 *
 * The three terms:
 *   T  = ½Σ|wv|²           — kinetic energy
 *   X  = ½c²·Σ wv·L₁₈(J)  — cross term (kick-drift correction)
 *   V  = ½c²·(-Σ J·L₁₈(J))— gradient energy (potential)
 *
 * The cross term arises because kick-drift stores velocity at time n+1
 * but position at time n+1 (half-step offset). For the standard
 * kick-drift-kick (Velocity Verlet), the cross term vanishes.
 *
 * L₁₈ is the 18-point isotropic Laplacian:
 *   L₁₈(J)(v) = (1/3)Σ_face J(u) + (1/6)Σ_edge J(u) - 4·J(v)
 */
static double conserved_hamiltonian(const ftd::RenderBridge& rb) {
    double H_kinetic = 0.0;
    double H_cross = 0.0;
    double H_gradient = 0.0;

    const int L = rb.lattice().size();
    const auto& vox = rb.voxels();

    for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
    for (int z = 0; z < L; ++z) {
        int idx = rb.lattice().index(x, y, z);
        const auto& v = vox[idx];

        // Kinetic energy: |wave_vel|²
        H_kinetic += v.wave_vel.x * v.wave_vel.x
                   + v.wave_vel.y * v.wave_vel.y
                   + v.wave_vel.z * v.wave_vel.z;

        // Compute L₁₈(J)(v) — 18-point isotropic Laplacian
        // Uses precomputed neighbor tables for performance
        double lap_x = 0.0, lap_y = 0.0, lap_z = 0.0;

        // 6 face neighbors (weight 1/3)
        const auto& n6 = rb.lattice().neighbors_6(idx);
        for (int f = 0; f < 6; ++f) {
            lap_x += vox[n6[f]].flux.x / 3.0;
            lap_y += vox[n6[f]].flux.y / 3.0;
            lap_z += vox[n6[f]].flux.z / 3.0;
        }

        // 12 edge neighbors (weight 1/6)
        const auto& n12 = rb.lattice().neighbors_12(idx);
        for (int e = 0; e < 12; ++e) {
            lap_x += vox[n12[e]].flux.x / 6.0;
            lap_y += vox[n12[e]].flux.y / 6.0;
            lap_z += vox[n12[e]].flux.z / 6.0;
        }

        // Center: -4
        lap_x -= 4.0 * v.flux.x;
        lap_y -= 4.0 * v.flux.y;
        lap_z -= 4.0 * v.flux.z;

        // Cross term: wv · L(J) (kick-drift correction)
        double wv_dot_L = v.wave_vel.x * lap_x
                        + v.wave_vel.y * lap_y
                        + v.wave_vel.z * lap_z;
        H_cross += wv_dot_L;

        // Gradient energy = -J · L(J) ≥ 0
        double neg_J_dot_L = -(v.flux.x * lap_x + v.flux.y * lap_y + v.flux.z * lap_z);
        H_gradient += neg_J_dot_L;
    }

    double c2 = ftd::C_WAVE * ftd::C_WAVE;
    return 0.5 * H_kinetic + 0.5 * c2 * H_cross + 0.5 * c2 * H_gradient;
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  TEST: EM Energy Conservation (Undamped Vacuum) — 5 Checks\n";
    std::cout << "================================================================\n";

    // ================================================================
    // M7a: Localized Gaussian pulse — H conserved over 2000 ticks
    // ================================================================
    std::cout << "\n-- M7a: Gaussian Pulse Hamiltonian Conservation --\n";
    {
        int L = 32;
        ftd::RenderBridge rb(L);
        rb.toggles.disable_all();
        rb.toggles.wave_propagation = true;

        // Inject a Gaussian flux pulse centered at (16,16,16), sigma=3
        double sigma = 3.0;
        int cx = L / 2, cy = L / 2, cz = L / 2;
        for (int x = 0; x < L; ++x)
            for (int y = 0; y < L; ++y)
                for (int z = 0; z < L; ++z) {
                    double r2 = (x - cx) * (x - cx) + (y - cy) * (y - cy) + (z - cz) * (z - cz);
                    double amp = 0.1 * std::exp(-r2 / (2.0 * sigma * sigma));
                    rb.inject_flux(x, y, z, {0, amp, 0});
                }

        double H0 = conserved_hamiltonian(rb);
        rb.run(500);
        double H500 = conserved_hamiltonian(rb);
        rb.run(500);
        double H1000 = conserved_hamiltonian(rb);
        rb.run(1000);
        double H2000 = conserved_hamiltonian(rb);

        double drift_500 = std::abs(H500 - H0) / H0 * 100.0;
        double drift_1000 = std::abs(H1000 - H0) / H0 * 100.0;
        double drift_2000 = std::abs(H2000 - H0) / H0 * 100.0;

        std::cout << std::setprecision(10);
        std::cout << "    H(0) = " << H0 << "\n";
        std::cout << "    H(500) = " << H500 << " (drift " << drift_500 << "%)\n";
        std::cout << "    H(1000) = " << H1000 << " (drift " << drift_1000 << "%)\n";
        std::cout << "    H(2000) = " << H2000 << " (drift " << drift_2000 << "%)\n";

        check("M7a: Gaussian pulse Hamiltonian drift < 0.001% over 2000 ticks",
              drift_2000 < 0.001);
    }

    // ================================================================
    // M7b: Traveling plane wave — H conserved over 500 ticks
    // ================================================================
    std::cout << "\n-- M7b: Plane Wave Hamiltonian Conservation --\n";
    {
        int L = 32;
        ftd::RenderBridge rb(L);
        rb.toggles.disable_all();
        rb.toggles.wave_propagation = true;

        // y-polarized traveling wave in +x
        int n = 4;
        double k = 2.0 * M_PI * n / L;
        double omega = 2.0 * ftd::C_WAVE * std::abs(std::sin(k / 2.0));
        double AMP = 0.05;

        for (int x = 0; x < L; ++x) {
            double jy = AMP * std::sin(k * x);
            double wv_y = -omega * AMP * std::cos(k * x);
            for (int y = 0; y < L; ++y)
                for (int z = 0; z < L; ++z) {
                    rb.inject_flux(x, y, z, {0, jy, 0});
                    rb.voxels()[rb.lattice().index(x, y, z)].wave_vel = {0, wv_y, 0};
                }
        }

        double H0 = conserved_hamiltonian(rb);
        rb.run(100);
        double H100 = conserved_hamiltonian(rb);
        rb.run(400);
        double H500 = conserved_hamiltonian(rb);

        double drift_100 = std::abs(H100 - H0) / H0 * 100.0;
        double drift_500 = std::abs(H500 - H0) / H0 * 100.0;

        std::cout << std::setprecision(10);
        std::cout << "    H(0) = " << H0 << "\n";
        std::cout << "    H(100) = " << H100 << " (drift " << drift_100 << "%)\n";
        std::cout << "    H(500) = " << H500 << " (drift " << drift_500 << "%)\n";

        check("M7b: Plane wave Hamiltonian drift < 0.001% over 500 ticks",
              drift_500 < 0.001);
    }

    // ================================================================
    // M7c: Standing wave — H conserved over 1000 ticks
    // ================================================================
    std::cout << "\n-- M7c: Standing Wave Hamiltonian Conservation --\n";
    {
        int L = 32;
        ftd::RenderBridge rb(L);
        rb.toggles.disable_all();
        rb.toggles.wave_propagation = true;

        int n = 4;
        double k = 2.0 * M_PI * n / L;
        double AMP = 0.05;
        for (int x = 0; x < L; ++x) {
            double jy = AMP * std::sin(k * x);
            for (int y = 0; y < L; ++y)
                for (int z = 0; z < L; ++z)
                    rb.inject_flux(x, y, z, {0, jy, 0});
        }

        double H0 = conserved_hamiltonian(rb);
        rb.run(1000);
        double H1000 = conserved_hamiltonian(rb);

        double drift = std::abs(H1000 - H0) / H0 * 100.0;
        std::cout << std::setprecision(10);
        std::cout << "    H(0) = " << H0 << ", H(1000) = " << H1000
                  << " (drift " << drift << "%)\n";

        check("M7c: Standing wave Hamiltonian drift < 0.001% over 1000 ticks",
              drift < 0.001);
    }

    // ================================================================
    // M7d: Two counter-propagating pulses (interference)
    // ================================================================
    std::cout << "\n-- M7d: Counter-Propagating Pulses --\n";
    {
        int L = 64;
        ftd::RenderBridge rb(L);
        rb.toggles.disable_all();
        rb.toggles.wave_propagation = true;

        double sigma = 3.0;
        for (int x = 0; x < L; ++x)
            for (int y = 0; y < L; ++y)
                for (int z = 0; z < L; ++z) {
                    double r1_2 = (x-16)*(x-16) + (y-32)*(y-32) + (z-32)*(z-32);
                    double r2_2 = (x-48)*(x-48) + (y-32)*(y-32) + (z-32)*(z-32);
                    double a1 = 0.05 * std::exp(-r1_2 / (2.0 * sigma * sigma));
                    double a2 = 0.05 * std::exp(-r2_2 / (2.0 * sigma * sigma));
                    rb.inject_flux(x, y, z, {0, a1 + a2, 0});
                }

        double H0 = conserved_hamiltonian(rb);
        rb.run(500);
        double H500 = conserved_hamiltonian(rb);

        double drift = std::abs(H500 - H0) / H0 * 100.0;
        std::cout << std::setprecision(10);
        std::cout << "    H(0) = " << H0 << ", H(500) = " << H500
                  << " (drift " << drift << "%)\n";

        check("M7d: Counter-propagating Hamiltonian drift < 0.001% over 500 ticks",
              drift < 0.001);
    }

    // ================================================================
    // M7e: Energy partition — oscillates between field and wave
    // ================================================================
    std::cout << "\n-- M7e: Energy Partition --\n";
    {
        int L = 32;
        ftd::RenderBridge rb(L);
        rb.toggles.disable_all();
        rb.toggles.wave_propagation = true;

        // Standing wave (zero initial wave_vel): all energy starts in field
        int n = 4;
        double k = 2.0 * M_PI * n / L;
        double AMP = 0.05;
        for (int x = 0; x < L; ++x) {
            double jy = AMP * std::sin(k * x);
            for (int y = 0; y < L; ++y)
                for (int z = 0; z < L; ++z)
                    rb.inject_flux(x, y, z, {0, jy, 0});
        }

        auto a0 = rb.energy_audit();
        double fe0 = a0.field_energy;
        double we0 = a0.wave_energy;

        // After some ticks, energy should partially transfer to wave_vel
        rb.run(20);
        auto a20 = rb.energy_audit();
        double fe20 = a20.field_energy;
        double we20 = a20.wave_energy;

        std::cout << "    t=0:  field=" << fe0 << ", wave=" << we0 << "\n";
        std::cout << "    t=20: field=" << fe20 << ", wave=" << we20 << "\n";

        check("M7e: Energy partition — wave_energy > 0 after 20 ticks (field→wave transfer)",
              we20 > 1e-10 && fe20 > 1e-10);
    }

    // ================================================================
    // Summary
    // ================================================================
    std::cout << "\n================================================================\n";
    if (g_failures == 0)
        std::cout << "  All 5 EM energy conservation tests PASSED.\n";
    else
        std::cout << "  " << g_failures << " test(s) FAILED.\n";
    std::cout << "================================================================\n";

    return g_failures;
}
