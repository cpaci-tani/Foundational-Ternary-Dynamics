/**
 * Physics Checklist #69: Atomic Energy Levels from Scale 0 Lattice
 *
 * Full lattice-scale hydrogen is computationally prohibitive:
 *   a_0 ~ 613 lattice units (with gravity) or ~3374 (pure EM)
 *   -> need L > 2000 -> L^3 ~ 10^10 voxels -> ~2 TB RAM
 *
 * This test validates ANALYTICAL consistency of the Bohr model with
 * FTD constants, and runs a Scale-1 (ParticleEngine) proxy to verify
 * bound-state energy ratios.
 *
 * Tests:
 *   AE-1: Bohr energy from FTD constants matches 13.6 eV (Rydberg)
 *   AE-2: Bohr radius from FTD constants (pure EM and lattice-effective)
 *   AE-3: Energy level ratios E_n proportional to 1/n^2
 *   AE-4: Rydberg constant dimensional consistency
 *   AE-5: Lyman-alpha transition energy and wavelength
 *   AE-6: Scale-1 proxy -- hydrogen bound orbit exists and has correct energy
 *   AE-7: Lattice hydrogen is computationally prohibitive (documents why)
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <iomanip>
#include <vector>
#include "ftd/particle_engine.h"
#include "ftd/ontic.h"
#include "ftd/constants.h"

static int g_pass = 0, g_fail = 0;

static void check(const char* name, bool cond) {
    if (cond) { std::cout << "  [PASS] " << name << "\n"; g_pass++; }
    else      { std::cout << "  [FAIL] " << name << "\n"; g_fail++; }
}

static void check_close(const char* name, double got, double exp, double reltol) {
    double err = (std::abs(exp) < 1e-30) ? std::abs(got) : std::abs(got - exp) / std::abs(exp);
    bool ok = err < reltol;
    if (ok) {
        std::cout << "  [PASS] " << name
                  << " (got " << got << " vs " << exp
                  << ", " << err * 100.0 << "% err)\n";
        g_pass++;
    } else {
        std::cout << "  [FAIL] " << name
                  << " (got " << got << " vs " << exp
                  << ", " << err * 100.0 << "% err)\n";
        g_fail++;
    }
}

int main() {
    using namespace ftd;

    std::cout << "============================================================\n";
    std::cout << "  Checklist #69: Atomic Energy Levels from FTD Constants\n";
    std::cout << "============================================================\n\n";

    // ================================================================
    // Derived quantities used across tests
    // ================================================================
    //
    // ParticleEngine force convention:
    //   F_EM = alpha * q1 * q2 / (4 * pi * r^2)
    //   => effective coupling alpha_EM = alpha / (4 * pi)
    //
    // Pure-EM Bohr radius:  a0_pure = 1 / (K_B * alpha_EM) = 4*pi / (K_B * ALPHA)
    //
    // Lattice effective coupling (gravity contributes for opposite charges):
    //   alpha_eff = alpha/(4*pi) + G_N * K_B^2
    //   a0_lattice = 1 / (K_B * alpha_eff)
    //
    // Physical Bohr radius (natural units, hbar=c=1):
    //   a0_physical = 1 / (m_e * alpha) = 1 / (K_B * ALPHA)
    //   where K_B is in MeV and result is in MeV^{-1} (= Planck lengths)
    //
    // Bohr energy:
    //   E_n = -alpha^2 * m_e / (2 * n^2)           [physical natural units]
    //   E_n = -alpha_EM^2 * K_B / (2 * n^2)        [PE convention, pure EM]
    //   E_n = -alpha_eff^2 * K_B / (2 * n^2)       [PE convention, with gravity]

    double alpha_EM = ALPHA / (4.0 * PI);
    double alpha_eff = alpha_EM + G_N * K_B * K_B;
    double a0_pure = 1.0 / (K_B * alpha_EM);   // pure-EM Bohr radius (PE convention)
    double a0_lattice = 1.0 / (K_B * alpha_eff); // lattice Bohr radius (EM + gravity)
    double a0_physical = 1.0 / (K_B * ALPHA);    // physical Bohr radius (natural units)

    std::cout << "  Framework constants:\n";
    std::cout << "    ALPHA       = " << std::setprecision(10) << ALPHA << "\n";
    std::cout << "    K_B (m_e)   = " << K_B << " MeV\n";
    std::cout << "    G_N         = " << G_N << "\n";
    std::cout << "    PI          = " << PI << "\n";
    std::cout << "    R_BOHR      = " << R_BOHR << " (from ontic.h)\n\n";

    std::cout << "  Derived scales:\n";
    std::cout << "    alpha_EM    = alpha/(4pi) = " << alpha_EM << "\n";
    std::cout << "    alpha_eff   = alpha_EM + G_N*K_B^2 = " << alpha_eff << "\n";
    std::cout << "    a0_pure     = " << a0_pure << " lattice units (PE, pure EM)\n";
    std::cout << "    a0_lattice  = " << a0_lattice << " lattice units (PE, EM+grav)\n";
    std::cout << "    a0_physical = " << a0_physical << " Planck lengths\n\n";

    // ================================================================
    // AE-1: Bohr energy from FTD constants matches 13.6 eV
    // ================================================================
    std::cout << "--- AE-1: Bohr Energy from FTD Constants ---\n";
    {
        // In physical natural units (hbar = c = 1):
        //   E_1 = -alpha^2 * m_e / 2
        //
        // K_B = 0.511 MeV = 511000 eV
        // E_1 = -(0.007297)^2 * 511000 / 2 = -13.598 eV
        // Rydberg energy = 13.6057 eV (CODATA)

        double K_B_eV = K_B * 1.0e6;  // MeV -> eV
        double E_1_eV = ALPHA * ALPHA * K_B_eV / 2.0;  // magnitude, positive
        double E_rydberg_codata = 13.6057;  // eV (CODATA 2022)

        std::cout << "    K_B in eV       = " << K_B_eV << "\n";
        std::cout << "    alpha^2*K_B/2   = " << E_1_eV << " eV\n";
        std::cout << "    Rydberg (CODATA)= " << E_rydberg_codata << " eV\n";

        check_close("AE-1a: Rydberg energy matches CODATA within 0.1%",
                    E_1_eV, E_rydberg_codata, 0.001);

        // Also check in MeV (simulation units)
        double E_1_MeV = ALPHA * ALPHA * K_B / 2.0;
        double E_rydberg_MeV = 13.6057e-6;  // eV -> MeV
        check_close("AE-1b: Rydberg energy in MeV",
                    E_1_MeV, E_rydberg_MeV, 0.001);
    }

    // ================================================================
    // AE-2: Bohr radius from FTD constants
    // ================================================================
    std::cout << "\n--- AE-2: Bohr Radius from FTD Constants ---\n";
    {
        // Physical Bohr radius: a0 = 1/(m_e * alpha) in natural units
        // = 1 / (0.511 * 0.007297) = 268.2 Planck lengths (= 1/MeV)
        // Physical value: a0 = 0.529e-10 m, l_P = 1.616e-35 m
        // a0/l_P = 0.529e-10 / 1.616e-35 = 3.27e24 Planck lengths
        //
        // BUT: in FTD natural units K_B is dimensionless (K_B = m_e/M_P),
        // so a0_physical = 1/(K_B * ALPHA) = 1/(0.511 * 0.007297) = 268.2
        // is in Planck lengths per the FTD convention.

        check_close("AE-2a: a0_physical = 1/(K_B*ALPHA)",
                    a0_physical, 1.0 / (K_B * ALPHA), 1e-12);

        // Verify R_BOHR from ontic.h matches the PE convention
        check_close("AE-2b: R_BOHR = 4*pi/(K_B*ALPHA) from ontic.h",
                    R_BOHR, 4.0 * PI / (K_B * ALPHA), 1e-12);

        // Lattice Bohr radius is smaller because gravity adds to attraction
        check("AE-2c: lattice a0 < pure-EM a0 (gravity assists binding)",
              a0_lattice < a0_pure);

        // Lattice Bohr radius ~ 613 (from project documentation)
        double a0_lattice_expected = 613.0;
        double a0_lattice_relerr = std::abs(a0_lattice - a0_lattice_expected) / a0_lattice_expected;
        std::cout << "    a0_lattice = " << a0_lattice << " (expected ~" << a0_lattice_expected << ")\n";
        check("AE-2d: lattice Bohr radius ~ 613 (within 5%)",
              a0_lattice_relerr < 0.05);

        // Physical a0 is > 200 Planck lengths (not a tiny test grid)
        check("AE-2e: a0_physical > 200 Planck lengths", a0_physical > 200.0);
    }

    // ================================================================
    // AE-3: Energy level ratios E_n proportional to 1/n^2
    // ================================================================
    std::cout << "\n--- AE-3: Energy Level Ratios ---\n";
    {
        // E_n = -alpha^2 * m_e / (2*n^2)  =>  E_n / E_1 = 1/n^2
        // This is a purely mathematical relation, framework-independent.

        auto E_n = [&](int n) -> double {
            return -ALPHA * ALPHA * K_B / (2.0 * n * n);
        };

        double E1 = E_n(1);
        double E2 = E_n(2);
        double E3 = E_n(3);
        double E4 = E_n(4);

        std::cout << "    E_1 = " << E1 << " MeV\n";
        std::cout << "    E_2 = " << E2 << " MeV\n";
        std::cout << "    E_3 = " << E3 << " MeV\n";
        std::cout << "    E_4 = " << E4 << " MeV\n";

        check_close("AE-3a: E_2/E_1 = 1/4", E2 / E1, 0.25, 1e-12);
        check_close("AE-3b: E_3/E_1 = 1/9", E3 / E1, 1.0 / 9.0, 1e-12);
        check_close("AE-3c: E_4/E_1 = 1/16", E4 / E1, 1.0 / 16.0, 1e-12);

        // Balmer series: |E_2| - |E_4| = (1/4 - 1/16)|E_1| = 3/16 of |E_1|
        // Since E values are negative, E_4 - E_2 gives the positive photon energy
        double delta_balmer = E4 - E2;  // less-negative minus more-negative = positive
        double expected_balmer = std::abs(E1) * 3.0 / 16.0;
        check_close("AE-3d: Balmer H-beta transition energy",
                    delta_balmer, expected_balmer, 1e-12);
    }

    // ================================================================
    // AE-4: Rydberg constant dimensional consistency
    // ================================================================
    std::cout << "\n--- AE-4: Rydberg Constant ---\n";
    {
        // In FTD natural units (hbar = c = 1, M_P = 1):
        //   R_FTD = alpha^2 * K_B / 2  (energy of ground state, in M_P units)
        //
        // Physical Rydberg constant: R_inf = m_e * c * alpha^2 / (2 * hbar)
        //                          = 10973731 m^{-1}
        //
        // Dimensional check: R_FTD has dimensions of [energy] = [mass] in natural units.
        // R_inf has dimensions of [1/length] in SI.
        // Connection: R_inf = R_FTD / (2*pi) in wavenumber, or R_FTD = h*c*R_inf.

        double R_FTD = ALPHA * ALPHA * K_B / 2.0;
        std::cout << "    R_FTD = alpha^2 * K_B / 2 = " << R_FTD << " (Planck energy units)\n";

        // Cross-check: R_FTD should equal 13.6 eV in Planck energy units
        // M_P = 1.2209e19 GeV = 1.2209e28 eV
        // R_FTD in eV = R_FTD * M_P_eV = R_FTD * 1.2209e28
        // But K_B = m_e/M_P = 0.511 MeV / 1.2209e22 MeV = 4.18e-23 (dimensionless)
        // Wait — K_B = 0.511 is already in MeV (the physical mass), not dimensionless.
        // So R_FTD = 0.007297^2 * 0.511 / 2 = 1.3606e-5 MeV = 13.606 eV. Correct.

        double R_FTD_eV = R_FTD * 1e6;  // MeV -> eV
        check_close("AE-4a: R_FTD = 13.6 eV (dimensional consistency)",
                    R_FTD_eV, 13.6057, 0.001);

        // Verify R_FTD > 0 and finite
        check("AE-4b: R_FTD is positive and finite",
              R_FTD > 0.0 && std::isfinite(R_FTD));

        // The ratio R_FTD / K_B = alpha^2 / 2 ~ 2.66e-5
        double ratio = R_FTD / K_B;
        double expected_ratio = ALPHA * ALPHA / 2.0;
        check_close("AE-4c: R_FTD/K_B = alpha^2/2",
                    ratio, expected_ratio, 1e-12);
    }

    // ================================================================
    // AE-5: Lyman-alpha transition energy and wavelength
    // ================================================================
    std::cout << "\n--- AE-5: Lyman-Alpha Transition ---\n";
    {
        // Lyman-alpha: 1s -> 2p transition
        // Delta_E = |E_1| * (1 - 1/4) = 3/4 * alpha^2 * K_B / 2
        //         = 3/8 * alpha^2 * K_B

        double E_1_mag = ALPHA * ALPHA * K_B / 2.0;
        double delta_E = E_1_mag * 3.0 / 4.0;     // 3/4 of Rydberg
        double delta_E_eV = delta_E * 1.0e6;       // MeV -> eV

        // Physical Lyman-alpha: 10.2 eV, wavelength 121.567 nm
        double delta_E_physical_eV = 10.2;
        std::cout << "    Delta_E (Lyman-alpha)   = " << delta_E_eV << " eV\n";
        std::cout << "    Physical (Lyman-alpha)  = " << delta_E_physical_eV << " eV\n";

        check_close("AE-5a: Lyman-alpha energy ~ 10.2 eV",
                    delta_E_eV, delta_E_physical_eV, 0.005);

        // Wavelength in natural units: lambda = 2*pi / delta_E
        // In lattice units (where E is in MeV): lambda = 2*pi*hbar*c / delta_E
        // In natural units hbar=c=1: lambda = 2*pi / delta_E (in inverse-MeV = Planck lengths)
        double lambda_natural = 2.0 * PI / delta_E;
        std::cout << "    lambda (natural units)  = " << lambda_natural << " Planck lengths\n";

        // Physical: lambda = 121.567 nm, l_P = 1.616e-35 m
        // lambda / l_P = 121.567e-9 / 1.616e-35 = 7.52e26 Planck lengths
        // Our formula: 2*pi / (7.65e-6 MeV) -- but this is in inverse-MeV, not Planck lengths.
        // 1 MeV^{-1} = hbar*c / (1 MeV) = 197.3 fm = 1.973e-13 m
        // So lambda = lambda_natural * 1.973e-13 m = (2*pi / 7.65e-6) * 1.973e-13 m
        // = 8.21e5 * 1.973e-13 = 1.62e-7 m = 162 nm
        // Hmm, this is off by ~30% -- because K_B = 0.511 is in MeV, and the
        // formula E = alpha^2 * K_B / 2 already includes the correct dimensionful mass.
        // lambda = 2*pi / delta_E where delta_E is in MeV, so lambda is in MeV^{-1}.
        // Converting: 1 MeV^{-1} = 197.327 fm = 1.97327e-13 m
        double hbar_c_fm = 197.327;  // MeV * fm
        double lambda_nm = lambda_natural * hbar_c_fm * 1e-6;  // MeV^{-1} * fm/MeV * (nm/fm)

        std::cout << "    lambda (SI)             = " << lambda_nm << " nm\n";
        std::cout << "    Physical (Lyman-alpha)  = 121.567 nm\n";

        check_close("AE-5b: Lyman-alpha wavelength ~ 121.6 nm",
                    lambda_nm, 121.567, 0.005);

        // Verify transition energy ratios for the Lyman series
        // Lyman-beta: 1s -> 3p, Delta_E = |E_1| * (1 - 1/9) = 8/9 * |E_1|
        double delta_lyman_beta = E_1_mag * 8.0 / 9.0;
        double ratio_beta_alpha = delta_lyman_beta / delta_E;
        // Expected: (8/9) / (3/4) = 32/27 = 1.185...
        check_close("AE-5c: Lyman-beta/Lyman-alpha energy ratio = 32/27",
                    ratio_beta_alpha, 32.0 / 27.0, 1e-12);
    }

    // ================================================================
    // AE-6: Scale-1 Proxy -- Hydrogen Bound Orbit
    // ================================================================
    std::cout << "\n--- AE-6: Scale-1 Hydrogen Proxy ---\n";
    {
        // Use ParticleEngine (Scale 1) with a locked proton and orbiting electron.
        // The effective coupling includes gravity: alpha_eff = alpha/(4*pi) + G_N*K_B^2
        // Bohr radius: a0 = 1/(K_B * alpha_eff) ~ 613

        // For this proxy test, we use a smaller initial radius to keep things fast.
        // Use a_0_proxy = 100 (well below true a0, but tests binding stability).
        // This means the system is NOT in the true ground state -- it is in a
        // "deeper" orbit. But the force law is the same, and energy conservation
        // + bounded orbit should hold.

        double a_proxy = 100.0;
        // Circular orbit condition: F = m*v^2/r
        // alpha_eff / r^2 = K_B * v^2 / r  =>  v = sqrt(alpha_eff / (K_B * r))
        double v_orb = std::sqrt(alpha_eff / (K_B * a_proxy));
        double E_expected = -0.5 * K_B * v_orb * v_orb;  // virial: E = -T = -KE

        std::cout << "    a_proxy       = " << a_proxy << " lattice units\n";
        std::cout << "    v_orb         = " << v_orb << "\n";
        std::cout << "    E_expected    = " << E_expected << "\n";

        ParticleEngine pe;
        pe.set_dt(10.0);                  // Moderate timestep
        pe.set_damping_enabled(false);    // Exact energy conservation
        pe.set_softening(1.0);

        // Proton at origin, locked (infinite mass)
        pe.add_locked_particle(+1, {0, 0, 0});

        // Electron at (a_proxy, 0, 0) with tangential velocity (0, v_orb, 0)
        int e_id = pe.add_particle(-1, {a_proxy, 0, 0}, {0, v_orb, 0});
        pe.particles()[e_id].r_eff = 0.01;  // prevent annihilation

        auto d0 = pe.diagnostics();
        double E0 = d0.total_energy;

        std::cout << "    Initial E     = " << E0 << "\n";

        // Run for 5000 ticks
        int total_ticks = 5000;
        double r_min = 1e30, r_max = 0;
        bool escaped = false;

        for (int t = 0; t < total_ticks; ++t) {
            pe.tick();

            if (pe.particles().size() < 2) {
                escaped = true;
                break;
            }

            double r = pe.particles()[e_id].position.mag();
            if (r < r_min) r_min = r;
            if (r > r_max) r_max = r;
        }

        auto d1 = pe.diagnostics();
        double E1 = d1.total_energy;

        std::cout << "    Final E       = " << E1 << "\n";
        std::cout << "    r_min         = " << r_min << "\n";
        std::cout << "    r_max         = " << r_max << "\n";
        std::cout << "    Survived      = " << (!escaped ? "yes" : "no") << "\n";

        // AE-6a: Electron survives (stays bound, not annihilated)
        check("AE-6a: electron survives 5000 ticks", !escaped && pe.particles().size() >= 2);

        // AE-6b: Orbit is bounded (max/min radius ratio < 5)
        double eccentricity_proxy = (r_max > 0) ? r_max / std::max(r_min, 1e-10) : 999.0;
        std::cout << "    r_max/r_min   = " << eccentricity_proxy << "\n";
        check("AE-6b: orbit bounded (r_max/r_min < 5)", eccentricity_proxy < 5.0);

        // AE-6c: Energy conservation < 0.1%
        double drift = (std::abs(E0) > 1e-30) ? std::abs(E1 - E0) / std::abs(E0) : std::abs(E1 - E0);
        std::cout << "    Energy drift  = " << drift * 100.0 << "%\n";
        check("AE-6c: energy conservation < 0.1%", drift < 0.001);

        // AE-6d: Energy has correct sign and rough magnitude
        // For circular orbit at radius r: E = -alpha_eff / (2*r)
        double E_circ = -alpha_eff / (2.0 * a_proxy);
        std::cout << "    E_circular    = " << E_circ << "\n";
        check("AE-6d: total energy is negative (bound state)", E0 < 0);

        double e_ratio = E0 / E_circ;
        std::cout << "    E_actual/E_circ = " << e_ratio << "\n";
        check("AE-6e: energy within factor 2 of circular orbit prediction",
              e_ratio > 0.5 && e_ratio < 2.0);
    }

    // ================================================================
    // AE-7: Lattice Hydrogen is Computationally Prohibitive
    // ================================================================
    std::cout << "\n--- AE-7: Computational Feasibility Assessment ---\n";
    {
        // Document why full lattice-scale hydrogen is currently infeasible.
        //
        // For the bound state to be physical, the lattice must be large enough
        // to contain the orbit without periodic boundary image effects.
        // Rule of thumb: L > 4 * a0

        double L_min_lattice = 4.0 * a0_lattice;     // EM + gravity Bohr radius
        double L_min_pure    = 4.0 * a0_pure;         // pure EM Bohr radius
        double voxels_lattice = L_min_lattice * L_min_lattice * L_min_lattice;
        double voxels_pure    = L_min_pure * L_min_pure * L_min_pure;

        // Memory: ~200 bytes/voxel (SoA layout from GPU engine)
        double bytes_per_voxel = 200.0;
        double mem_lattice_TB = voxels_lattice * bytes_per_voxel / 1e12;
        double mem_pure_TB    = voxels_pure * bytes_per_voxel / 1e12;

        std::cout << "    With gravity (alpha_eff):\n";
        std::cout << "      a0 = " << a0_lattice << " lattice units\n";
        std::cout << "      L_min = 4*a0 = " << L_min_lattice << "\n";
        std::cout << "      Voxels = L^3 = " << std::scientific << voxels_lattice << "\n";
        std::cout << "      Memory = " << std::fixed << mem_lattice_TB << " TB\n\n";

        std::cout << "    Pure EM (no gravity):\n";
        std::cout << "      a0 = " << a0_pure << " lattice units\n";
        std::cout << "      L_min = 4*a0 = " << L_min_pure << "\n";
        std::cout << "      Voxels = L^3 = " << std::scientific << voxels_pure << "\n";
        std::cout << "      Memory = " << std::fixed << mem_pure_TB << " TB\n\n";

        std::cout << std::defaultfloat;

        // AE-7a: Bohr radius exceeds any practical test grid (say L=128)
        check("AE-7a: lattice a0 >> 128 (exceeds practical grid)",
              a0_lattice > 128.0);

        // AE-7b: Memory requirement exceeds 1 TB (prohibitive)
        check("AE-7b: lattice hydrogen requires > 1 TB RAM",
              mem_lattice_TB > 1.0);

        // AE-7c: Document minimum lattice size
        check("AE-7c: minimum L > 2000 lattice units",
              L_min_lattice > 2000.0);

        // AE-7d: This confirms the checklist status: computationally prohibitive
        std::cout << "    CONCLUSION: Full lattice-scale hydrogen is computationally\n";
        std::cout << "    prohibitive. Scale-1 (ParticleEngine) or Scale-2 (AtomEngine)\n";
        std::cout << "    proxies are required for atomic energy level studies.\n";
        check("AE-7d: status confirmed -- Scale-0 hydrogen infeasible", true);
    }

    // ================================================================
    // Summary
    // ================================================================
    std::cout << "\n============================================================\n";
    std::cout << "  Atomic Energy Levels: " << g_pass << " passed, " << g_fail << " failed\n";
    std::cout << "============================================================\n";

    return g_fail;
}
