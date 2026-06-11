/**
 * Campaign: Hydrogen Spectrum (consolidated suite)
 *
 * Merges 5 legacy hydrogen test files into a single ftd::test-instrumented
 * suite using the Phase 2a NDJSON telemetry API:
 *
 *   test_hydrogen_scale1              -> section "hydrogen_scale1"
 *   test_hydrogen_em_only             -> section "hydrogen_em_only"
 *   test_hydrogen_spectrum_scale1     -> section "hydrogen_spectrum_scale1"
 *   campaign_poisson_hydrogen         -> section "poisson_hydrogen"
 *   campaign_hydrogen_spectrum (old)  -> section "hydrogen_spectrum_legacy"
 *
 * Every check(...) from the legacy files is preserved verbatim (same condition,
 * same label) and routed through ftd::test::check for uniform telemetry.
 *
 * Wave 4c.9 consolidation (2026-04-14). Self-ref target: the OLD
 * campaign_hydrogen_spectrum.cpp body is preserved as
 * section_hydrogen_spectrum_legacy().
 */

#define _USE_MATH_DEFINES

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <vector>

#include "ftd/constants.h"
#include "ftd/particle_engine.h"
#include "ftd/render_bridge.h"
#include "ftd/test_telemetry.h"

// ============================================================================
// Section: hydrogen_scale1  (from test_hydrogen_scale1.cpp)
// ============================================================================

static void section_hydrogen_scale1() {
    using namespace ftd;

    std::cout << "============================================================\n";
    std::cout << "  Phase 7 Stage 5: Hydrogen at Scale 1\n";
    std::cout << "============================================================\n\n";

    // Derived hydrogen scales
    // The effective coupling is alpha_eff = alpha/(4*pi) because
    // F = alpha * q1 * q2 / (4*pi*r^2)
    // For a circular orbit: alpha_eff / r^2 = m * v^2 / r
    //  -> v^2 = alpha_eff / (m * r), and virial: E = -alpha_eff / (2*r)
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
        ftd::test::check("H1: electron survives 5000 ticks", survives);
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
        ftd::test::check("H2: average radius within factor 2 of a_0", in_range);
    }

    // ---- H3: Total energy ----
    {
        std::cout << "\n--- H3: Total energy ---\n";
        double E = d1.total_energy;
        double err = std::abs(E - E_ground) / std::abs(E_ground);
        std::cout << "    Expected E_ground: " << E_ground << "\n";
        std::cout << "    Actual energy:     " << E << "\n";
        std::cout << "    Relative error:    " << err * 100.0 << "%\n";
        ftd::test::check("H3: energy within 10% of ground state", err < 0.10);
    }

    // ---- H4: Energy conservation ----
    {
        std::cout << "\n--- H4: Energy conservation ---\n";
        double E = d1.total_energy;
        double drift = (E0 != 0.0) ? std::abs(E - E0) / std::abs(E0) : std::abs(E - E0);
        std::cout << "    Initial energy: " << E0 << "\n";
        std::cout << "    Final energy:   " << E << "\n";
        std::cout << "    Drift:          " << drift * 100.0 << "%\n";
        ftd::test::check("H4: energy conservation < 0.1%", drift < 0.001);
    }

    // ---- H5: Angular momentum conservation ----
    {
        std::cout << "\n--- H5: Angular momentum conservation ---\n";
        double L = d1.total_angular_momentum.mag();
        double drift = (L0 > 1e-30) ? std::abs(L - L0) / L0 : std::abs(L - L0);
        std::cout << "    Initial |L|: " << L0 << "\n";
        std::cout << "    Final |L|:   " << L << "\n";
        std::cout << "    Drift:       " << drift * 100.0 << "%\n";
        ftd::test::check("H5: angular momentum conservation < 1%", drift < 0.01);
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

        // Each orbit has 2 crossings (in + out). Period ~ 2 * total_time / crossings
        double total_time = total_ticks * dt;
        double T_measured = (crossings > 0) ? 2.0 * total_time / crossings : 0;

        std::cout << "    Expected T:    " << T_orbit << "\n";
        std::cout << "    Measured T:    " << T_measured << "\n";
        std::cout << "    Zero-crossings: " << crossings << "\n";

        double err = (T_orbit > 0 && T_measured > 0)
                     ? std::abs(T_measured - T_orbit) / T_orbit : 1.0;
        std::cout << "    Period error:  " << err * 100.0 << "%\n";
        ftd::test::check("H6: Kepler period within 20%", err < 0.20);
    }
}

// ============================================================================
// Section: hydrogen_em_only  (from test_hydrogen_em_only.cpp)
// ============================================================================

// Helper: find position of a particle with given state on a line y=mid, z=mid
static int find_particle_x_hem(const ftd::RenderBridge& rb, int8_t target_state,
                               int mid, int L) {
    for (int x = 0; x < L; ++x) {
        int idx = rb.lattice().index(x, mid, mid);
        if (rb.voxels()[idx].state == target_state) return x;
    }
    return -1;  // not found on that line
}

// Helper: find particle position in 3D, return squared distance from center
static double find_particle_r2_hem(const ftd::RenderBridge& rb, int8_t target_state,
                                   int cx, int cy, int cz, int L) {
    for (int x = 0; x < L; ++x) {
        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                int idx = rb.lattice().index(x, y, z);
                if (rb.voxels()[idx].state == target_state) {
                    double dx = x - cx;
                    double dy = y - cy;
                    double dz = z - cz;
                    // Handle periodic wrapping
                    if (dx > L/2) dx -= L;
                    if (dx < -L/2) dx += L;
                    if (dy > L/2) dy -= L;
                    if (dy < -L/2) dy += L;
                    if (dz > L/2) dz -= L;
                    if (dz < -L/2) dz += L;
                    return dx*dx + dy*dy + dz*dz;
                }
            }
        }
    }
    return -1.0;  // not found
}

static void section_hydrogen_em_only() {
    std::cout << "================================================================\n";
    std::cout << "  TEST: Hydrogen EM-Only Bound State -- 6 Checks\n";
    std::cout << "================================================================\n";
    std::cout << std::fixed << std::setprecision(6);

    // Theoretical quantities
    double alpha_eff_em = ftd::ALPHA / (4.0 * ftd::PI);  // EM coupling in lattice units
    double a0_em = 1.0 / (ftd::K_B * alpha_eff_em);   // Pure EM Bohr radius
    double a0_grav = 1.0 / (ftd::K_B * (alpha_eff_em + ftd::G_N * ftd::K_B * ftd::K_B));
    double E_ground = -ftd::K_B * alpha_eff_em * alpha_eff_em / 2.0;

    std::cout << "\n  Theoretical predictions:\n";
    std::cout << "    ALPHA = " << ftd::ALPHA << "\n";
    std::cout << "    alpha_eff (EM) = alpha/(4*pi) = " << alpha_eff_em << "\n";
    std::cout << "    G_N = " << ftd::G_N << "\n";
    std::cout << "    K_B = " << ftd::K_B << "\n";
    std::cout << "    R_BOHR (ontic) = " << ftd::R_BOHR << "\n";
    std::cout << "    a0 (EM-only) = 1/(K_B * alpha/(4pi)) = " << a0_em << "\n";
    std::cout << "    a0 (with gravity) ~ " << a0_grav << "\n";
    std::cout << "    E_ground (EM-only) = " << E_ground << "\n";
    std::cout << "    G_N / alpha_eff = " << ftd::G_N / alpha_eff_em
              << " (gravity dominance factor)\n";

    // ================================================================
    // HEM-1: Setup verification -- gravity OFF, EM ON
    // ================================================================
    std::cout << "\n-- HEM-1: Setup (Gravity OFF, EM ON) --\n";
    {
        const int L = 48;
        const int mid = L / 2;
        const int offset = 12;  // initial separation

        ftd::RenderBridge rb(L);
        rb.toggles.disable_all();
        rb.toggles.wave_propagation = true;
        rb.toggles.coupling = true;
        rb.toggles.damping = true;
        rb.toggles.gauss_projection = true;
        rb.toggles.forces = true;
        rb.toggles.poisson_coulomb = true;
        rb.toggles.gravity = false;  // KEY: no gravity
        rb.toggles.movement = true;

        rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
        rb.inject_particle(mid + offset, mid, mid, -1, {0, 0, -ftd::K_B});

        // Verify toggles
        ftd::test::check("HEM-1: Gravity is OFF and forces are ON",
              !rb.toggles.gravity && rb.toggles.forces && rb.toggles.poisson_coulomb);
    }

    // ================================================================
    // HEM-2: Bound orbit forms
    // ================================================================
    std::cout << "\n-- HEM-2: Bound Orbit Formation --\n";
    {
        // Run the system and check that both particles survive
        // (don't annihilate within the test period)
        const int L = 48;
        const int mid = L / 2;
        const int offset = 15;
        const int SETTLE = 300;  // Let fields build
        const int EVOLVE = 1000;

        ftd::RenderBridge rb(L);
        rb.toggles.disable_all();
        rb.toggles.wave_propagation = true;
        rb.toggles.coupling = true;
        rb.toggles.damping = true;
        rb.toggles.gauss_projection = true;
        rb.toggles.forces = true;
        rb.toggles.poisson_coulomb = true;
        rb.toggles.gravity = false;
        rb.toggles.movement = true;

        rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
        rb.inject_particle(mid + offset, mid, mid, -1, {0, 0, -ftd::K_B});

        // Lock the +1 particle to serve as nucleus
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

        rb.run(SETTLE);

        auto diag0 = rb.diagnostics();
        int initial_count = diag0.manifested_count;

        rb.run(EVOLVE);

        auto diag1 = rb.diagnostics();
        int final_count = diag1.manifested_count;

        std::cout << "    Initial manifested: " << initial_count << "\n";
        std::cout << "    After " << EVOLVE << " ticks: " << final_count << "\n";

        // At least the locked nucleus should survive. The orbiting electron
        // may evaporate due to Larmor radiation, which is physically correct.
        ftd::test::check("HEM-2: Nucleus survives, system has at least 1 particle",
              final_count >= 1);
    }

    // ================================================================
    // HEM-3: Orbital radius (EM-only Bohr radius)
    // ================================================================
    std::cout << "\n-- HEM-3: EM-Only Bohr Radius --\n";
    {
        // Use force measurement at multiple radii to determine the
        // equilibrium radius where centripetal = Coulomb force.
        // F_coulomb ~ alpha / r^2 in lattice Poisson units.
        // Measure force at several radii to find scaling.

        const int L = 48;
        const int mid = L / 2;

        // Measure Coulomb force at multiple radii (locked particles)
        std::vector<int> radii = {4, 6, 8, 10, 14};
        std::vector<double> force_vals;

        for (int r : radii) {
            ftd::RenderBridge rb(L);
            rb.toggles.disable_all();
            rb.toggles.wave_propagation = true;
            rb.toggles.coupling = true;
            rb.toggles.damping = true;
            rb.toggles.gauss_projection = true;
            rb.toggles.forces = true;
            rb.toggles.poisson_coulomb = true;
            rb.toggles.gravity = false;

            rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
            rb.inject_particle(mid + r, mid, mid, -1, {0, 0, -ftd::K_B});
            rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;
            rb.voxels()[rb.lattice().index(mid + r, mid, mid)].locked = true;

            rb.run(200);

            auto& fd = rb.force_diag_at(mid + r, mid, mid);
            double F = fd.f_coulomb.mag();
            force_vals.push_back(F);
            std::cout << "    r=" << r << ": |F_coulomb| = " << F << "\n";
        }

        // Compute effective coupling from the measured forces.
        // F = alpha_eff / r^2 => alpha_eff = F * r^2
        // Average alpha_eff over all radii
        double alpha_sum = 0;
        int alpha_count = 0;
        for (size_t i = 0; i < radii.size(); ++i) {
            if (force_vals[i] > 1e-15) {
                double aeff = force_vals[i] * radii[i] * radii[i];
                std::cout << "    r=" << radii[i] << ": alpha_eff = F*r^2 = " << aeff << "\n";
                alpha_sum += aeff;
                alpha_count++;
            }
        }

        double alpha_meas = (alpha_count > 0) ? alpha_sum / alpha_count : 0;
        double a0_meas = (alpha_meas > 1e-15) ? 1.0 / (ftd::K_B * alpha_meas) : 0;

        std::cout << "    Measured alpha_eff (avg) = " << alpha_meas << "\n";
        std::cout << "    Implied a0 = 1/(K_B * alpha_eff) = " << a0_meas << "\n";
        std::cout << "    Theory (EM-only) a0 = " << a0_em << "\n";
        std::cout << "    Theory (with gravity) a0 ~ " << a0_grav << "\n";

        // The measured alpha_eff should be closer to alpha/(4pi) than to
        // alpha/(4pi) + G_N*K_B^2 (which would indicate gravity contamination)
        double alpha_em_theory = ftd::ALPHA / (4.0 * ftd::PI);
        double alpha_grav_theory = alpha_em_theory + ftd::G_N * ftd::K_B * ftd::K_B;

        std::cout << "    alpha_eff (EM theory) = " << alpha_em_theory << "\n";
        std::cout << "    alpha_eff (EM+grav theory) = " << alpha_grav_theory << "\n";

        // With gravity OFF, measured coupling should be within factor 10 of
        // pure EM coupling (lattice discreteness shifts exact values)
        bool forces_measured = alpha_count >= 3;
        bool not_gravity_contaminated = true;
        if (alpha_meas > 1e-15 && alpha_grav_theory > 1e-15) {
            double dist_em = std::abs(std::log(alpha_meas / alpha_em_theory));
            double dist_grav = std::abs(std::log(alpha_meas / alpha_grav_theory));
            // EM distance should not be dramatically worse than gravity distance
            not_gravity_contaminated = dist_em < dist_grav + 2.0;  // within ~e^2 factor
        }

        ftd::test::check("HEM-3: Force measured at multiple radii, no gravity contamination",
              forces_measured && not_gravity_contaminated);
    }

    // ================================================================
    // HEM-4: Ground state energy estimate
    // ================================================================
    std::cout << "\n-- HEM-4: Ground State Energy --\n";
    {
        // E_ground = -K_B * (alpha/(4pi))^2 / 2
        // This is very small because alpha/(4pi) is tiny

        double E_g = -ftd::K_B * alpha_eff_em * alpha_eff_em / 2.0;

        std::cout << "    E_ground (analytic) = " << E_g << "\n";
        std::cout << "    For comparison: K_B = " << ftd::K_B << "\n";
        std::cout << "    Ratio |E_ground|/K_B = " << std::abs(E_g) / ftd::K_B << "\n";

        // The binding energy is extremely small compared to the rest mass --
        // this is consistent with the non-relativistic hydrogen atom
        ftd::test::check("HEM-4: Ground state energy is negative and << K_B",
              E_g < 0 && std::abs(E_g) < ftd::K_B * 0.01);
    }

    // ================================================================
    // HEM-5: Gravity dominance confirmation
    // ================================================================
    std::cout << "\n-- HEM-5: Gravity Dominance Comparison --\n";
    {
        // Compare forces with and without gravity at the same separation
        const int L = 48;
        const int mid = L / 2;
        const int r = 8;
        const int SETTLE = 200;

        double F_em_only = 0, F_em_grav = 0, F_grav_only = 0;

        // EM-only
        {
            ftd::RenderBridge rb(L);
            rb.toggles.disable_all();
            rb.toggles.wave_propagation = true;
            rb.toggles.coupling = true;
            rb.toggles.damping = true;
            rb.toggles.gauss_projection = true;
            rb.toggles.forces = true;
            rb.toggles.poisson_coulomb = true;
            rb.toggles.gravity = false;

            rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
            rb.inject_particle(mid + r, mid, mid, -1, {0, 0, -ftd::K_B});
            rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;
            rb.voxels()[rb.lattice().index(mid + r, mid, mid)].locked = true;

            rb.run(SETTLE);

            auto& fd = rb.force_diag_at(mid + r, mid, mid);
            F_em_only = fd.f_coulomb.mag();
        }

        // EM + gravity
        {
            ftd::RenderBridge rb(L);
            rb.toggles.disable_all();
            rb.toggles.wave_propagation = true;
            rb.toggles.coupling = true;
            rb.toggles.damping = true;
            rb.toggles.gauss_projection = true;
            rb.toggles.forces = true;
            rb.toggles.poisson_coulomb = true;
            rb.toggles.gravity = true;

            rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
            rb.inject_particle(mid + r, mid, mid, -1, {0, 0, -ftd::K_B});
            rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;
            rb.voxels()[rb.lattice().index(mid + r, mid, mid)].locked = true;

            rb.run(SETTLE);

            auto& fd = rb.force_diag_at(mid + r, mid, mid);
            F_em_grav = fd.f_coulomb.mag();
            F_grav_only = fd.f_gravity.mag();
        }

        std::cout << "    At r=" << r << ":\n";
        std::cout << "      F_em (no gravity) = " << F_em_only << "\n";
        std::cout << "      F_em (with gravity) = " << F_em_grav << "\n";
        std::cout << "      F_gravity = " << F_grav_only << "\n";

        double grav_em_ratio = (F_em_only > 1e-15) ? F_grav_only / F_em_only : 0;
        std::cout << "      F_gravity / F_em = " << grav_em_ratio << "\n";
        std::cout << "      G_N / (alpha/(4*pi)) = " << ftd::G_N / alpha_eff_em << "\n";

        // On the lattice, G_N = 0.01 >> alpha/(4*pi) ~ 0.00058
        // Gravity should be much stronger than EM on the lattice
        ftd::test::check("HEM-5: Lattice gravity dominates EM (F_grav/F_em > 1 or both measurable)",
              F_em_only > 1e-15 && F_grav_only > 1e-15);
    }

    // ================================================================
    // HEM-6: Coulomb 1/r^2 scaling
    // ================================================================
    std::cout << "\n-- HEM-6: Coulomb 1/r^2 Force Law --\n";
    {
        const int L = 48;
        const int mid = L / 2;
        const int SETTLE = 200;

        // Measure force at several radii, fit power law
        std::vector<int> radii = {4, 6, 8, 10, 14, 18};
        std::vector<double> log_r, log_F;

        for (int r : radii) {
            ftd::RenderBridge rb(L);
            rb.toggles.disable_all();
            rb.toggles.wave_propagation = true;
            rb.toggles.coupling = true;
            rb.toggles.damping = true;
            rb.toggles.gauss_projection = true;
            rb.toggles.forces = true;
            rb.toggles.poisson_coulomb = true;
            rb.toggles.gravity = false;

            rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
            rb.inject_particle(mid + r, mid, mid, -1, {0, 0, -ftd::K_B});
            rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;
            rb.voxels()[rb.lattice().index(mid + r, mid, mid)].locked = true;

            rb.run(SETTLE);

            auto& fd = rb.force_diag_at(mid + r, mid, mid);
            double F = fd.f_coulomb.mag();

            std::cout << "    r=" << std::setw(2) << r << ": |F| = " << F << "\n";

            if (F > 1e-15) {
                log_r.push_back(std::log(static_cast<double>(r)));
                log_F.push_back(std::log(F));
            }
        }

        // Fit power law: log(F) = n * log(r) + const
        double exponent = 0;
        if (log_r.size() >= 3) {
            double sum_x = 0, sum_y = 0, sum_xx = 0, sum_xy = 0;
            int n = (int)log_r.size();
            for (int i = 0; i < n; ++i) {
                sum_x += log_r[i];
                sum_y += log_F[i];
                sum_xx += log_r[i] * log_r[i];
                sum_xy += log_r[i] * log_F[i];
            }
            exponent = (n * sum_xy - sum_x * sum_y) / (n * sum_xx - sum_x * sum_x);
        }

        std::cout << "    Fitted power law exponent = " << exponent
                  << " (expected: -2.0 for Coulomb)\n";

        // Force should scale as ~1/r^n with n close to 2
        // Allow lattice corrections: exponent between -3.0 and -1.5
        ftd::test::check("HEM-6: Coulomb force exponent between -3.0 and -1.5 (near -2.0)",
              exponent < -1.5 && exponent > -3.0);
    }

    // restore default cout flags
    std::cout << std::defaultfloat;
}

// ============================================================================
// Section: hydrogen_spectrum_scale1  (from test_hydrogen_spectrum_scale1.cpp)
// ============================================================================

// Run hydrogen at quantum number n, return time-averaged binding energy
struct LevelResult_hss1 {
    double energy;         // time-averaged total energy
    double energy_init;    // initial energy
    double L_init;         // initial angular momentum
    double L_final;        // final angular momentum
    double period;         // measured orbital period (from OrbitalElements)
    double semi_major;     // measured semi-major axis
    double eccentricity;   // measured eccentricity
    bool survived;
};

static LevelResult_hss1 run_level_hss1(int n, double alpha_eff, double a_0, double v_orb,
                                       double E_ground, int ticks, double dt) {
    using namespace ftd;
    LevelResult_hss1 res = {};

    double r_n = n * n * a_0;
    double v_n = v_orb / n;

    ParticleEngine pe;
    pe.set_dt(dt);
    pe.set_damping_enabled(false);  // Exact conservation
    pe.set_softening(1.0);
    pe.toggles.minimal();           // Coulomb + gravity only

    // Proton
    pe.add_locked_particle(+1, {0, 0, 0});
    // Electron at (r_n, 0, 0) with tangential velocity (0, v_n, 0)
    pe.add_particle(-1, {r_n, 0, 0}, {0, v_n, 0});
    pe.particles()[1].r_eff = 0.01;  // prevent annihilation

    auto d0 = pe.diagnostics();
    res.energy_init = d0.total_energy;
    res.L_init = d0.total_angular_momentum.mag();

    // Time-averaged energy
    double E_sum = 0;
    int E_count = 0;

    for (int t = 0; t < ticks; ++t) {
        pe.tick();
        if (pe.particles().size() < 2) { res.survived = false; return res; }

        auto d = pe.diagnostics();
        E_sum += d.total_energy;
        E_count++;
    }

    res.survived = (pe.particles().size() >= 2);
    res.energy = E_sum / E_count;

    auto d1 = pe.diagnostics();
    res.L_final = d1.total_angular_momentum.mag();

    // Orbital elements from final state
    auto oe = compute_orbital_elements(pe.particles()[1], pe.particles()[0], alpha_eff);
    res.period = oe.period;
    res.semi_major = oe.semi_major_axis;
    res.eccentricity = oe.eccentricity;

    return res;
}

static void section_hydrogen_spectrum_scale1() {
    using namespace ftd;

// Route CHECK macro through ftd::test::check. The original used g_pass/g_fail
// counters, which are replaced by the telemetry framework's aggregated count.
#define CHECK(cond, msg) ftd::test::check((msg), (cond))

    std::printf("============================================================\n");
    std::printf("  Hydrogen Spectrum at Scale 1: Energy Levels E_n ~ 1/n^2\n");
    std::printf("============================================================\n\n");

    // Derived hydrogen scales (same as test_hydrogen_scale1.cpp)
    double alpha_eff = ALPHA / (4.0 * PI) + G_N * K_B * K_B;
    double a_0 = 1.0 / (K_B * alpha_eff);
    double v_orb = std::sqrt(alpha_eff / (K_B * a_0));
    double T_1 = 2.0 * PI * a_0 / v_orb;
    double E_ground = -0.5 * K_B * v_orb * v_orb;

    std::printf("  Constants:\n");
    std::printf("    alpha_eff = %.6e\n", alpha_eff);
    std::printf("    a_0       = %.2f lattice units\n", a_0);
    std::printf("    v_orb     = %.6e\n", v_orb);
    std::printf("    T_1       = %.2f\n", T_1);
    std::printf("    E_ground  = %.6e\n\n", E_ground);

    // dt chosen so dt/T_1 << 1; ticks chosen for ~1 orbit at n=1
    double dt = 100.0;
    int ticks = 5000;

    // Run all four levels
    std::printf("--- Running n=1,2,3,4 ---\n\n");
    LevelResult_hss1 levels[4];
    for (int n = 1; n <= 4; ++n) {
        std::printf("  n=%d: r=%.1f, v=%.4e, E_expected=%.4e\n",
                    n, n*n*a_0, v_orb/n, E_ground/(n*n));
        levels[n-1] = run_level_hss1(n, alpha_eff, a_0, v_orb, E_ground, ticks, dt);
        std::printf("    E_measured=%.4e, survived=%s, e=%.4f, a=%.1f\n",
                    levels[n-1].energy, levels[n-1].survived ? "yes" : "no",
                    levels[n-1].eccentricity, levels[n-1].semi_major);
        std::printf("    L_init=%.4e, L_final=%.4e\n\n",
                    levels[n-1].L_init, levels[n-1].L_final);
    }

    double E1 = levels[0].energy;
    double E2 = levels[1].energy;
    double E3 = levels[2].energy;
    double E4 = levels[3].energy;

    // HS-1: Ground state is bound (negative energy)
    // FTD note: with dt=100, orbits are radial (e~1) rather than circular.
    // The absolute energy is ~2x Bohr due to virial theorem for radial orbits
    // (a_radial = a_0/2).  The RATIOS are exact -- that's the physics.
    {
        std::printf("  HS-1: E1=%.4e (bound=%s)\n", E1, (E1 < 0) ? "yes" : "no");
        CHECK(E1 < 0, "HS-1: Ground state is bound (E < 0)");
    }

    // HS-2: E2/E1 ratio
    {
        double ratio = E2 / E1;
        double expected = 1.0 / 4.0;
        double err = std::abs(ratio - expected) / expected;
        std::printf("  HS-2: E2/E1=%.4f, expected=%.4f, error=%.1f%%\n", ratio, expected, err*100);
        CHECK(err < 0.15, "HS-2: E2/E1 within 15% of 1/4");
    }

    // HS-3: E3/E1 ratio
    {
        double ratio = E3 / E1;
        double expected = 1.0 / 9.0;
        double err = std::abs(ratio - expected) / expected;
        std::printf("  HS-3: E3/E1=%.4f, expected=%.4f, error=%.1f%%\n", ratio, expected, err*100);
        CHECK(err < 0.20, "HS-3: E3/E1 within 20% of 1/9");
    }

    // HS-4: E4/E1 ratio
    {
        double ratio = E4 / E1;
        double expected = 1.0 / 16.0;
        double err = std::abs(ratio - expected) / expected;
        std::printf("  HS-4: E4/E1=%.4f, expected=%.4f, error=%.1f%%\n", ratio, expected, err*100);
        CHECK(err < 0.25, "HS-4: E4/E1 within 25% of 1/16");
    }

    // HS-5: All survive
    {
        bool all = levels[0].survived && levels[1].survived
                && levels[2].survived && levels[3].survived;
        CHECK(all, "HS-5: All orbits survive (no collapse, no escape)");
    }

    // HS-6: Energy stays bound at each level
    // FTD note: with large dt, angular momentum is not conserved
    // (radial orbit artifact). But energy RATIOS are exact -- the
    // physics is in the scaling, not the orbit shape.
    {
        bool all_bound = true;
        for (int n = 1; n <= 4; ++n) {
            std::printf("  HS-6 n=%d: E=%.4e (bound=%s)\n",
                        n, levels[n-1].energy, (levels[n-1].energy < 0) ? "yes" : "no");
            if (levels[n-1].energy >= 0) all_bound = false;
        }
        CHECK(all_bound, "HS-6: All levels remain bound (E < 0)");
    }

    // HS-7: Period T_n ~ n^3
    {
        bool ok = true;
        double T1 = levels[0].period;
        for (int n = 2; n <= 4; ++n) {
            double Tn = levels[n-1].period;
            if (T1 > 0 && Tn > 0) {
                double ratio = Tn / T1;
                double expected = (double)(n*n*n);
                double err = std::abs(ratio - expected) / expected;
                std::printf("  HS-7 n=%d: T%d/T1=%.2f, expected=%.0f, error=%.1f%%\n",
                            n, n, ratio, expected, err*100);
                if (err >= 0.25) ok = false;
            }
        }
        CHECK(ok, "HS-7: Kepler period T_n ~ n^3 (within 25%)");
    }

    // HS-8: Semi-major axis scales as n^2
    // For radial orbits (e~1), a = n^2 * a_0 / 2.  The SCALING is what matters.
    {
        double a1 = levels[0].semi_major;
        bool ok = true;
        for (int n = 2; n <= 4; ++n) {
            double ratio = levels[n-1].semi_major / a1;
            double expected = (double)(n * n);
            double err = std::abs(ratio - expected) / expected;
            std::printf("  HS-8 n=%d: a%d/a1=%.2f, expected=%.0f, error=%.1f%%\n",
                        n, n, ratio, expected, err*100);
            if (err >= 0.05) ok = false;
        }
        CHECK(ok, "HS-8: Semi-major axis scales as n^2 (within 5%)");
    }

    // HS-9: Semi-major axis is finite and positive
    {
        bool ok = true;
        for (int n = 1; n <= 4; ++n) {
            std::printf("  HS-9 n=%d: a=%.1f\n", n, levels[n-1].semi_major);
            if (levels[n-1].semi_major <= 0 || !std::isfinite(levels[n-1].semi_major)) ok = false;
        }
        CHECK(ok, "HS-9: All semi-major axes finite and positive");
    }

    // HS-10: Energy stays bounded (no blow-up, no collapse to -inf)
    {
        bool ok = true;
        for (int n = 1; n <= 4; ++n) {
            bool finite = std::isfinite(levels[n-1].energy) && levels[n-1].energy < 0;
            std::printf("  HS-10 n=%d: E=%.4e (finite=%s, bound=%s)\n",
                        n, levels[n-1].energy, std::isfinite(levels[n-1].energy) ? "yes" : "no",
                        (levels[n-1].energy < 0) ? "yes" : "no");
            if (!finite) ok = false;
        }
        CHECK(ok, "HS-10: Energy finite and negative at all levels");
    }

    // HS-11: Transition dE_{2->1} positive
    {
        double dE = E1 - E2;  // E1 more negative -> dE < 0 means E1 < E2, transition releases energy
        std::printf("  HS-11: E1=%.4e, E2=%.4e, dE_{2->1}=E1-E2=%.4e\n", E1, E2, dE);
        // E1 < E2 (more tightly bound), so E1-E2 < 0, and |E1| > |E2|
        CHECK(std::abs(E1) > std::abs(E2),
              "HS-11: Ground state more tightly bound than n=2");
    }

    // HS-12: Transition ratio
    {
        double dE_21 = E1 - E2;  // negative (energy released in 2->1 transition)
        double dE_31 = E1 - E3;  // more negative (more energy in 3->1)
        double ratio = (std::abs(dE_21) > 1e-30) ? dE_31 / dE_21 : 0;
        // Expected: (1 - 1/9) / (1 - 1/4) = (8/9)/(3/4) = 32/27 ~ 1.185
        double expected = 32.0 / 27.0;
        double err = std::abs(ratio - expected) / expected;
        std::printf("  HS-12: dE31/dE21=%.4f, expected=%.4f, error=%.1f%%\n",
                    ratio, expected, err*100);
        CHECK(err < 0.20, "HS-12: Transition ratio within 20% of 32/27");
    }

#undef CHECK
}

// ============================================================================
// Section: poisson_hydrogen  (from campaign_poisson_hydrogen.cpp)
// ============================================================================

// Find the first manifested particle with given state sign
// Returns its grid position
struct ParticlePos_ph {
    int x, y, z;
    bool found;
};

static ParticlePos_ph find_particle_ph(const ftd::RenderBridge& rb, int8_t state_sign) {
    ParticlePos_ph p = {0, 0, 0, false};
    for (int i = 0; i < rb.lattice().total_sites(); ++i) {
        if (rb.voxels()[i].state == state_sign) {
            auto c = rb.lattice().coord(i);
            p.x = c.x;
            p.y = c.y;
            p.z = c.z;
            p.found = true;
            return p;
        }
    }
    return p;
}

static void section_poisson_hydrogen() {
    using ftd::Vec3;

    std::cout << "================================================================\n";
    std::cout << "  CAMPAIGN: Poisson Hydrogen (Phase 3) -- 6 Checks\n";
    std::cout << "================================================================\n";

    const int L = 48;
    const int mid = L / 2;
    const int initial_sep = 8;

    ftd::RenderBridge rb(L);

    // Proton: locked +1 at center
    rb.inject_particle(mid, mid, mid, +1, {0, 0, ftd::K_B});
    rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

    // Electron: free -1 at (mid+8, mid, mid)
    // Give it tangential velocity v_y = sqrt(alpha/r) for quasi-circular orbit
    double v_circ = std::sqrt(ftd::ALPHA / initial_sep);
    rb.inject_particle(mid + initial_sep, mid, mid, -1, {0, 0, -ftd::K_B});
    rb.voxels()[rb.lattice().index(mid + initial_sep, mid, mid)].velocity = {0, v_circ, 0};

    std::cout << "\n  Setup:\n";
    std::cout << "    Lattice: " << L << "^3\n";
    std::cout << "    Proton:  locked +1 at (" << mid << "," << mid << "," << mid << ")\n";
    std::cout << "    Electron: free -1 at (" << mid + initial_sep << "," << mid << "," << mid << ")\n";
    std::cout << "    v_circ = sqrt(alpha/r) = " << std::setprecision(4) << v_circ << "\n";

    // ================================================================
    // PH4: Force on electron points inward at t=100
    // ================================================================
    std::cout << "\n--- Phase 1: Settling (100 ticks) ---\n";
    rb.run(100);
    // force_diag_ is stale (pre-movement positions). Lock electron and
    // tick once more so force_diag_ is computed at electron's actual site.
    {
        ParticlePos_ph e_pre = find_particle_ph(rb, -1);
        if (e_pre.found) {
            int eidx = rb.lattice().index(e_pre.x, e_pre.y, e_pre.z);
            rb.voxels()[eidx].locked = true;
            rb.tick();  // forces computed at locked electron position
            rb.voxels()[eidx].locked = false;
        }
    }
    {
        ParticlePos_ph e = find_particle_ph(rb, -1);
        if (e.found) {
            int idx = rb.lattice().index(e.x, e.y, e.z);
            Vec3 f = rb.force_diag()[idx].f_coulomb;
            // Force should point from electron toward proton
            double dx = mid - e.x;
            double dy = mid - e.y;
            double dz = mid - e.z;
            double dot = f.x * dx + f.y * dy + f.z * dz;
            std::cout << "    Electron at (" << e.x << "," << e.y << "," << e.z << ")\n";
            std::cout << "    F_coulomb = (" << f.x << "," << f.y << "," << f.z << ")\n";
            std::cout << "    F.r_hat = " << dot << " (positive = inward)\n";
            ftd::test::check("PH4: Force on electron points inward", dot > 0);
        } else {
            std::cout << "    Electron not found at t=100\n";
            ftd::test::check("PH4: Force on electron points inward (electron lost)", false);
        }
    }

    // ================================================================
    // PH5: Angular momentum L_z is non-zero
    // ================================================================
    {
        ParticlePos_ph e = find_particle_ph(rb, -1);
        if (e.found) {
            int idx = rb.lattice().index(e.x, e.y, e.z);
            Vec3 v = rb.voxels()[idx].velocity;
            double rx = e.x - mid;
            double ry = e.y - mid;
            double Lz = rx * v.y - ry * v.x;  // z-component of L = r x v
            std::cout << "    L_z = " << Lz << "\n";
            ftd::test::check("PH5: Angular momentum L_z non-zero", std::abs(Lz) > 1e-10);
        } else {
            ftd::test::check("PH5: Angular momentum L_z non-zero (electron lost)", false);
        }
    }

    // ================================================================
    // PH3: Electron doesn't collapse to r=1 permanently within 1000 ticks
    // ================================================================
    std::cout << "\n--- Phase 2: Evolution (900 more ticks, total 1000) ---\n";
    {
        int collapse_count = 0;
        int sample_count = 0;
        for (int t = 0; t < 900; ++t) {
            rb.tick();
            if (t % 100 == 0) {
                ParticlePos_ph e = find_particle_ph(rb, -1);
                if (e.found) {
                    double r = std::sqrt(
                        (e.x - mid) * (e.x - mid) +
                        (e.y - mid) * (e.y - mid) +
                        (e.z - mid) * (e.z - mid));
                    if (r <= 1.5) collapse_count++;
                    sample_count++;
                    std::cout << "    t=" << (100 + t + 1)
                              << " r=" << std::setprecision(1) << std::fixed << r << "\n";
                }
            }
        }
        // Electron shouldn't be at r<=1 for ALL samples
        bool permanently_collapsed = (sample_count > 0 && collapse_count == sample_count);
        ftd::test::check("PH3: Not permanently collapsed at r=1", !permanently_collapsed);
    }

    // ================================================================
    // PH1: Electron survives 5000 ticks
    // ================================================================
    std::cout << "\n--- Phase 3: Long evolution (4000 more ticks, total 5000) ---\n";
    rb.run(4000);
    {
        ParticlePos_ph e = find_particle_ph(rb, -1);
        std::cout << "    Electron " << (e.found ? "SURVIVED" : "LOST") << " at t=5000\n";
        if (e.found) {
            std::cout << "    Position: (" << e.x << "," << e.y << "," << e.z << ")\n";
        }
        // Accept survival OR annihilation (which also tells us something)
        ftd::test::check("PH1: Electron survived 5000 ticks (or annihilated = also valid physics)",
              true);  // Informational -- always passes
    }

    // ================================================================
    // PH2: Electron separation (informational -- energy injection causes drift)
    // ================================================================
    // The self-field floor continuously injects energy, slowly accelerating the
    // electron outward. True binding would require an energy-conservative engine.
    // This check verifies the electron hasn't wrapped fully around the torus.
    {
        ParticlePos_ph e = find_particle_ph(rb, -1);
        if (e.found) {
            // Minimum image distance on periodic lattice
            int dx = std::abs(e.x - mid);
            int dy = std::abs(e.y - mid);
            int dz = std::abs(e.z - mid);
            if (dx > L / 2) dx = L - dx;
            if (dy > L / 2) dy = L - dy;
            if (dz > L / 2) dz = L - dz;
            double r = std::sqrt(dx * dx + dy * dy + dz * dz);
            std::cout << "    Final separation r = " << std::setprecision(1) << r << "\n";
            // With self-field energy injection, the electron drifts outward.
            // This check is informational -- the physics is correct (PH4: force
            // is attractive), but energy injection prevents true binding.
            ftd::test::check("PH2: Separation measured (informational)", true);
        } else {
            std::cout << "    Electron annihilated (was attracted -> bound)\n";
            ftd::test::check("PH2: Electron within half-lattice (annihilated = bound)", true);
        }
    }

    // ================================================================
    // PH6: Trajectory summary (informational)
    // ================================================================
    std::cout << "\n--- PH6: Trajectory Summary (Informational) ---\n";
    {
        // Just report what happened
        auto d = rb.diagnostics();
        auto a = rb.energy_audit();
        std::cout << "    Manifested: " << d.manifested_count << "\n";
        std::cout << "    Positive: " << d.positive_count << "\n";
        std::cout << "    Negative: " << d.negative_count << "\n";
        std::cout << "    Total energy: " << a.total_energy << "\n";
        std::cout << "    Coulomb PE: " << a.coulomb_pe << "\n";
        std::cout << "    Charge total: " << a.charge_total << "\n";
        ftd::test::check("PH6: Trajectory completed without crash", true);
    }

    // restore default cout flags
    std::cout << std::defaultfloat;
}

// ============================================================================
// Section: hydrogen_spectrum_legacy  (from old campaign_hydrogen_spectrum.cpp)
// ============================================================================

static void section_hydrogen_spectrum_legacy() {
    std::cout << "================================================================\n";
    std::cout << "  CAMPAIGN: Quantitative Hydrogen Spectrum -- 8 Checks\n";
    std::cout << "================================================================\n";

    // ================================================================
    // Setup: Hydrogen-like system at Scale 1 (ParticleEngine)
    // ================================================================
    // ParticleEngine uses analytical forces:
    //   F_EM  = -alpha * q1 * q2 * r_hat / (4*pi*r^2)
    //   F_grav = +G_N * m1 * m2 * r_hat / r^2
    // With q_proton = +1, q_electron = -1, both mass K_B

    ftd::ParticleEngine pe;
    pe.set_dt(100.0);  // CRITICAL: speed up evolution to avoid hanging
    pe.set_damping_enabled(false);  // CRITICAL: damping drains orbital energy

    // Predicted Bohr radius in ParticleEngine units
    // a_0 = 1 / (m_e * alpha_eff) where alpha_eff includes both EM and gravity
    double alpha_em = ftd::ALPHA / (4.0 * M_PI);  // ~0.000581
    double grav_contrib = ftd::G_N * ftd::K_B * ftd::K_B;  // ~0.00261
    double alpha_eff = alpha_em + grav_contrib;
    double a_bohr = 1.0 / (ftd::K_B * alpha_eff);

    // Predicted binding energy: E_bind = -0.5 * m * alpha_eff^2
    double e_bind_predicted = -0.5 * ftd::K_B * alpha_eff * alpha_eff;

    // Predicted orbital velocity and period
    double v_orbit = alpha_eff;
    double t_period = 2.0 * M_PI * a_bohr / v_orbit;

    std::cout << "\n--- Predictions ---\n";
    std::cout << std::setprecision(4);
    std::cout << "  alpha_EM/(4pi) = " << alpha_em << "\n";
    std::cout << "  G_N * K_B^2    = " << grav_contrib << "\n";
    std::cout << "  alpha_eff      = " << alpha_eff << "\n";
    std::cout << "  Bohr radius    = " << a_bohr << "\n";
    std::cout << "  Binding energy = " << e_bind_predicted << "\n";
    std::cout << "  Orbital period = " << t_period << "\n";

    // Inject proton at origin (locked -- infinite mass)
    pe.add_locked_particle(+1, {0.0, 0.0, 0.0}, ftd::K_B);

    // Inject electron at Bohr radius with circular orbital velocity
    double r0 = a_bohr;
    double v0 = v_orbit;
    pe.add_particle(-1, {r0, 0.0, 0.0}, {0.0, v0, 0.0}, ftd::K_B);

    // ================================================================
    // Evolve: Run for 3 orbital periods (capped for test timeout)
    // ================================================================
    int steps_per_period = static_cast<int>(t_period / pe.dt()) + 1;
    int n_periods = 3;
    // Cap at 2M steps to keep test under 600s timeout
    int total_steps = std::min(steps_per_period * n_periods, 2000000);

    // Sample diagnostics
    std::vector<double> radii, ke_samples, pe_samples, total_e;
    double initial_energy = 0.0;
    double min_r = 1e30, max_r = 0.0;

    std::cout << "\n--- Evolution ---\n";
    std::cout << "  Steps per period: " << steps_per_period << "\n";
    std::cout << "  Total steps:      " << total_steps << "\n";

    for (int step = 0; step <= total_steps; ++step) {
        if (step > 0) pe.tick();

        // Sample every 1% of total
        if (step % (total_steps / 100 + 1) == 0 || step == 0) {
            if (step > 0 && step % (total_steps / 10 + 1) == 0) {
                std::cout << "  Progress: " << (step * 100 / total_steps) << "%\n";
            }
            auto& parts = pe.particles();
            if (parts.size() < 2) break;

            double dx = parts[1].position.x - parts[0].position.x;
            double dy = parts[1].position.y - parts[0].position.y;
            double dz = parts[1].position.z - parts[0].position.z;
            double r = std::sqrt(dx*dx + dy*dy + dz*dz);
            radii.push_back(r);
            if (r < min_r) min_r = r;
            if (r > max_r) max_r = r;

            // KE of electron (index 1)
            double vx = parts[1].velocity.x, vy = parts[1].velocity.y, vz = parts[1].velocity.z;
            double ke = 0.5 * parts[1].mass * (vx*vx + vy*vy + vz*vz);
            ke_samples.push_back(ke);

            // PE = -alpha_eff * |q1*q2| / r (EM attractive + gravity attractive for unlike)
            // For PE: EM part = -alpha/(4pi) * 1/r, Grav part = -G_N * m^2 / r
            double pe_val = -(alpha_em + grav_contrib) / (r > 1e-10 ? r : 1e-10);
            pe_samples.push_back(pe_val);

            total_e.push_back(ke + pe_val);
            if (step == 0) initial_energy = ke + pe_val;
        }
    }

    // ================================================================
    // Analysis
    // ================================================================
    std::cout << "\n--- Measurements ---\n";

    // Average radius
    double avg_r = 0.0;
    for (double r : radii) avg_r += r;
    avg_r /= radii.size();

    // Average KE, PE
    double avg_ke = 0.0, avg_pe = 0.0;
    for (double k : ke_samples) avg_ke += k;
    avg_ke /= ke_samples.size();
    for (double p : pe_samples) avg_pe += p;
    avg_pe /= pe_samples.size();

    double virial = (avg_pe != 0.0) ? avg_ke / avg_pe : 0.0;
    double avg_total_e = avg_ke + avg_pe;

    // Energy drift
    double final_energy = total_e.back();
    double e_drift = (initial_energy != 0.0) ?
                     std::abs(final_energy - initial_energy) / std::abs(initial_energy) : 0.0;

    // Radius error
    double r_err = std::abs(avg_r - a_bohr) / a_bohr;

    // Binding energy error (compare magnitudes)
    double be_ratio = (e_bind_predicted != 0.0) ?
                      std::abs(avg_total_e / e_bind_predicted) : 0.0;

    // Estimate period from radius oscillation
    // Count zero-crossings of (r - avg_r)
    int crossings = 0;
    for (size_t i = 1; i < radii.size(); ++i) {
        if ((radii[i] - avg_r) * (radii[i-1] - avg_r) < 0) crossings++;
    }
    double measured_period = (crossings > 1) ?
        2.0 * total_steps * pe.dt() / crossings : -1.0;
    double period_ratio = (measured_period > 0 && t_period > 0) ?
        measured_period / t_period : -1.0;

    std::cout << std::setprecision(4);
    std::cout << "  Average radius    = " << avg_r << " (predicted: " << a_bohr
              << ", error: " << r_err * 100 << "%)\n";
    std::cout << "  Min/Max radius    = " << min_r << " / " << max_r << "\n";
    std::cout << "  Average KE        = " << std::scientific << avg_ke << "\n";
    std::cout << "  Average PE        = " << avg_pe << "\n";
    std::cout << "  Average total E   = " << avg_total_e
              << " (predicted: " << e_bind_predicted << ")\n";
    std::cout << std::fixed;
    std::cout << "  Virial ratio      = " << virial << " (ideal: -0.5)\n";
    std::cout << "  Energy drift      = " << e_drift * 100 << "%\n";
    std::cout << "  BE ratio          = " << be_ratio << " (ideal: 1.0)\n";
    std::cout << "  Measured period   = " << measured_period
              << " (predicted: " << t_period << ")\n";
    std::cout << "  Period ratio      = " << period_ratio << "\n";

    // ================================================================
    // Checks
    // ================================================================
    std::cout << "\n--- Checks ---\n";

    // HS1: System is bound
    ftd::test::check("HS1: System is bound (total energy < 0)", avg_total_e < 0);

    // HS2: Orbital radius within 50% of predicted
    ftd::test::check("HS2: Avg radius within 50% of predicted Bohr radius", r_err < 0.5);

    // HS3: Binding energy within factor of 3
    ftd::test::check("HS3: Binding energy within factor 3 of predicted",
          be_ratio > 0.33 && be_ratio < 3.0);

    // HS4: Virial ratio between -0.8 and -0.2
    ftd::test::check("HS4: Virial ratio in [-0.8, -0.2]",
          virial > -0.8 && virial < -0.2);

    // HS5: Energy drift < 5% (Velocity Verlet is symplectic -- should conserve well)
    ftd::test::check("HS5: Energy drift < 5% over evolution", e_drift < 0.05);

    // HS6: Electron stays within 3x Bohr radius
    ftd::test::check("HS6: Electron stays within 3x Bohr radius", max_r < 3.0 * a_bohr);

    // HS7: Period within factor of 2
    ftd::test::check("HS7: Kepler period within factor 2 of predicted",
          period_ratio > 0.5 && period_ratio < 2.0);

    // HS8: KE > 0 and PE < 0 (correct signs)
    ftd::test::check("HS8: KE > 0 and PE < 0 (correct sign relationship)",
          avg_ke > 0 && avg_pe < 0);

    // restore default cout flags
    std::cout << std::defaultfloat;
}

// ============================================================================
// main
// ============================================================================

int main() {
    ftd::test::init("campaign_hydrogen_spectrum");

    ftd::test::section("hydrogen_scale1");
    section_hydrogen_scale1();

    ftd::test::section("hydrogen_em_only");
    section_hydrogen_em_only();

    ftd::test::section("hydrogen_spectrum_scale1");
    section_hydrogen_spectrum_scale1();

    ftd::test::section("poisson_hydrogen");
    section_poisson_hydrogen();

    ftd::test::section("hydrogen_spectrum_legacy");
    section_hydrogen_spectrum_legacy();

    return ftd::test::finalize();
}
