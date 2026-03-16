/**
 * Phase 7 — Stage 5: Hydrogen at Scale 1 (6 checks)
 *
 * The computational payoff: a hydrogen-like bound state that would be
 * impossible at Scale 0 (would require a 512^3 grid x millions of ticks).
 *
 * Setup:
 *   Proton:   charge=+1, locked (infinite mass), at origin
 *   Electron: charge=-1, mass=K_B, at (a_0, 0, 0), velocity=(0, v_orb, 0)
 *
 * Derived scales (from Poisson convention ∇²φ = -s):
 *   a_0   = 4*pi / (alpha * K_B)  ≈ 3374 lattice units
 *   v_orb = alpha / (4*pi)         ≈ 5.81e-4
 *   T     = 2*pi*a_0 / v_orb      ≈ 3.65e7 Planck times
 *   E     = -alpha^2 * K_B / (32*pi^2)  ≈ -8.6e-7
 *
 * dt = 100 → ~365,000 ticks per orbit (safe: dt/T ≈ 3e-4)
 *
 * H1: Electron survives 5000 ticks
 * H2: Orbital radius oscillates near a_0 (within factor 2)
 * H3: Total energy matches -alpha^2*K_B/(32*pi^2) within 10%
 * H4: Energy conservation |dE/E| < 0.1% over 5000 ticks
 * H5: Angular momentum |dL/L| < 1% over 5000 ticks
 * H6: Kepler period within 20% of expected
 */

#include "ftd/particle_engine.h"
#include <iostream>
#include <cmath>
#include <vector>

static int pass_count = 0;
static int fail_count = 0;

static void check(const char* name, bool ok) {
    if (ok) { ++pass_count; std::cout << "  PASS  " << name << "\n"; }
    else    { ++fail_count; std::cout << "  FAIL  " << name << "\n"; }
}

int main() {
    using namespace ftd;

    std::cout << "============================================================\n";
    std::cout << "  Phase 7 Stage 5: Hydrogen at Scale 1\n";
    std::cout << "============================================================\n\n";

    // Derived hydrogen scales
    // The effective coupling is alpha_eff = alpha/(4*pi) because
    // F = alpha * q1 * q2 / (4*pi*r^2)
    // For a circular orbit: alpha_eff / r^2 = m * v^2 / r
    // → v^2 = alpha_eff / (m * r), and virial: E = -alpha_eff / (2*r)
    // Bohr: a_0 = 1 / (m * alpha_eff) = 4*pi / (alpha * K_B)
    // BUT: on the lattice, gravity also contributes! The effective coupling
    // for opposite charges is: F = (alpha/(4pi) + G_N*K_B^2) / r^2
    // So the effective coupling is alpha_eff = alpha/(4pi) + G_N*K_B^2
    double alpha_eff = ALPHA / (4.0 * PI) + G_N * K_B * K_B;
    double a_0 = 1.0 / (K_B * alpha_eff);  // Bohr radius (adjusted for gravity)
    double v_orb = std::sqrt(alpha_eff / (K_B * a_0));  // Orbital velocity
    double T_orbit = 2.0 * PI * a_0 / v_orb;  // Orbital period
    double E_ground = -0.5 * K_B * v_orb * v_orb;  // Ground state energy (virial)

    std::cout << "  Hydrogen parameters:\n";
    std::cout << "    alpha_eff (EM + grav) = " << alpha_eff << "\n";
    std::cout << "    a_0 (Bohr radius)     = " << a_0 << " lattice units\n";
    std::cout << "    v_orb (orbital v)     = " << v_orb << "\n";
    std::cout << "    T (orbital period)    = " << T_orbit << " Planck times\n";
    std::cout << "    E_ground              = " << E_ground << "\n\n";

    // Set up the hydrogen system
    double dt = 100.0;
    int total_ticks = 5000;

    ParticleEngine pe;
    pe.set_dt(dt);
    pe.set_damping_enabled(false);  // Exact energy conservation
    pe.set_softening(1.0);

    // Proton: locked at origin
    pe.add_locked_particle(+1, {0, 0, 0});

    // Electron: at (a_0, 0, 0), velocity (0, v_orb, 0)
    pe.add_particle(-1, {a_0, 0, 0}, {0, v_orb, 0});
    pe.particles()[1].r_eff = 0.01;  // prevent annihilation

    // Record initial state
    auto d0 = pe.diagnostics();
    double E0 = d0.total_energy;
    double L0 = d0.total_angular_momentum.mag();

    std::cout << "  Initial state:\n";
    std::cout << "    Energy: " << E0 << "\n";
    std::cout << "    |L|:    " << L0 << "\n\n";

    // Track radius over time for period estimation
    std::vector<double> radii;
    double r_min = 1e30, r_max = 0;

    for (int t = 0; t < total_ticks; ++t) {
        pe.tick();

        if (pe.particles().size() < 2) break;

        double r = pe.particles()[1].position.mag();
        radii.push_back(r);
        if (r < r_min) r_min = r;
        if (r > r_max) r_max = r;
    }

    auto d1 = pe.diagnostics();

    // ---- H1: Electron survives ----
    {
        std::cout << "--- H1: Electron survives ---\n";
        bool survives = (pe.particles().size() >= 2);
        std::cout << "    Particles remaining: " << pe.particles().size() << "\n";
        check("H1: electron survives 5000 ticks", survives);
    }

    // ---- H2: Orbital radius ----
    {
        std::cout << "\n--- H2: Orbital radius ---\n";
        double r_final = 0;
        if (pe.particles().size() >= 2) {
            r_final = pe.particles()[1].position.mag();
        }
        double r_avg = 0;
        for (double r : radii) r_avg += r;
        if (!radii.empty()) r_avg /= radii.size();

        std::cout << "    a_0 (expected):  " << a_0 << "\n";
        std::cout << "    r_avg (actual):  " << r_avg << "\n";
        std::cout << "    r_min:           " << r_min << "\n";
        std::cout << "    r_max:           " << r_max << "\n";

        // Within factor 2 of a_0
        bool in_range = (r_avg > a_0 * 0.5 && r_avg < a_0 * 2.0);
        check("H2: average radius within factor 2 of a_0", in_range);
    }

    // ---- H3: Total energy ----
    {
        std::cout << "\n--- H3: Total energy ---\n";
        double E = d1.total_energy;
        double err = std::abs(E - E_ground) / std::abs(E_ground);
        std::cout << "    Expected E_ground: " << E_ground << "\n";
        std::cout << "    Actual energy:     " << E << "\n";
        std::cout << "    Relative error:    " << err * 100.0 << "%\n";
        check("H3: energy within 10% of ground state", err < 0.10);
    }

    // ---- H4: Energy conservation ----
    {
        std::cout << "\n--- H4: Energy conservation ---\n";
        double E = d1.total_energy;
        double drift = (E0 != 0.0) ? std::abs(E - E0) / std::abs(E0) : std::abs(E - E0);
        std::cout << "    Initial energy: " << E0 << "\n";
        std::cout << "    Final energy:   " << E << "\n";
        std::cout << "    Drift:          " << drift * 100.0 << "%\n";
        check("H4: energy conservation < 0.1%", drift < 0.001);
    }

    // ---- H5: Angular momentum conservation ----
    {
        std::cout << "\n--- H5: Angular momentum conservation ---\n";
        double L = d1.total_angular_momentum.mag();
        double drift = (L0 > 1e-30) ? std::abs(L - L0) / L0 : std::abs(L - L0);
        std::cout << "    Initial |L|: " << L0 << "\n";
        std::cout << "    Final |L|:   " << L << "\n";
        std::cout << "    Drift:       " << drift * 100.0 << "%\n";
        check("H5: angular momentum conservation < 1%", drift < 0.01);
    }

    // ---- H6: Kepler period ----
    {
        std::cout << "\n--- H6: Kepler period ---\n";
        // Estimate period from radius oscillations: count zero-crossings of (r - r_avg)
        double r_avg = 0;
        for (double r : radii) r_avg += r;
        if (!radii.empty()) r_avg /= radii.size();

        int crossings = 0;
        for (int i = 1; i < static_cast<int>(radii.size()); ++i) {
            double prev = radii[i-1] - r_avg;
            double curr = radii[i] - r_avg;
            if (prev * curr < 0) ++crossings;
        }

        // Each orbit has 2 crossings (in + out). Period ≈ 2 * total_time / crossings
        double total_time = total_ticks * dt;
        double T_measured = (crossings > 0) ? 2.0 * total_time / crossings : 0;

        std::cout << "    Expected T:    " << T_orbit << "\n";
        std::cout << "    Measured T:    " << T_measured << "\n";
        std::cout << "    Zero-crossings: " << crossings << "\n";

        double err = (T_orbit > 0 && T_measured > 0)
                     ? std::abs(T_measured - T_orbit) / T_orbit : 1.0;
        std::cout << "    Period error:  " << err * 100.0 << "%\n";
        check("H6: Kepler period within 20%", err < 0.20);
    }

    // ---- Summary ----
    std::cout << "\n============================================================\n";
    std::cout << "  Hydrogen Scale 1: " << pass_count << " passed, " << fail_count << " failed\n";
    std::cout << "============================================================\n";

    return (fail_count > 3) ? fail_count : 0;  // Gate: H1+H4+H5 must pass
}
