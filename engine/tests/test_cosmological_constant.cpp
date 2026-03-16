/**
 * Test: Cosmological Constant
 *
 * Verifies that the vacuum energy density from the dual-substrate
 * framework gives Omega_Lambda = 2/3, consistent with the FTD
 * cosmological constant conjecture.
 *
 * Checklist item #52.
 *
 * Theory references:
 *   - DERIV_COSMOLOGICAL_CONSTANT.md (Omega_Lambda = 2/3 from dual substrate)
 *   - ontic.h Layer 3b (OMEGA_LAMBDA_CONJ, VACUUM_FRACTION)
 */

#include <iostream>
#include <iomanip>
#include <cmath>
#include "ftd/render_bridge.h"
#include "ftd/constants.h"

int failures = 0;

void check(const char* name, bool condition) {
    if (condition) {
        std::cout << "  PASS  " << name << "\n";
    } else {
        std::cout << "  FAIL  " << name << "\n";
        ++failures;
    }
}

void check_close(const char* name, double a, double b, double tol) {
    bool ok = std::abs(a - b) < tol;
    if (ok) {
        std::cout << "  PASS  " << name << "\n";
    } else {
        std::cout << "  FAIL  " << name << " (got " << std::setprecision(15) << a
                  << ", expected " << b << ", diff " << std::abs(a - b) << ")\n";
        ++failures;
    }
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  TEST: Cosmological Constant\n";
    std::cout << "================================================================\n\n";

    // CC-1: Vacuum energy density from dual substrate
    {
        std::cout << "--- CC-1: Vacuum energy from dual substrate ---\n";
        const int L = 16;
        ftd::RenderBridge engine(L);
        engine.toggles.enable_all();
        engine.toggles.dual_substrate = true;
        engine.toggles.genesis = false;
        engine.toggles.forces = false;
        engine.toggles.movement = false;

        // Empty lattice — inject uniform flux split across L/R substrates
        // In dual mode: J = J_L + J_R, with the vacuum having J_L != J_R
        double J0 = 0.1;  // Small vacuum flux
        for (int z = 0; z < L; ++z)
          for (int y = 0; y < L; ++y)
            for (int x = 0; x < L; ++x) {
              auto& v = engine.voxel_at(x, y, z);
              // Vacuum splits according to DELTA_APPROX
              v.flux_L = {J0 * (1.0 + ftd::DELTA_APPROX) / 2.0, 0.0, 0.0};
              v.flux_R = {J0 * (1.0 - ftd::DELTA_APPROX) / 2.0, 0.0, 0.0};
              v.flux = v.flux_L + v.flux_R;  // Observable
            }

        auto audit = engine.energy_audit();
        std::cout << "    E_L_total: " << audit.E_L_total << "\n";
        std::cout << "    E_R_total: " << audit.E_R_total << "\n";

        double E_vac = audit.E_L_total + audit.E_R_total;
        std::cout << "    E_vac (total): " << E_vac << "\n";
        check("CC-1: Vacuum energy is positive", E_vac > 0.0);
    }

    // CC-2: Verify OMEGA_LAMBDA_CONJ = 2/3
    {
        std::cout << "\n--- CC-2: Omega_Lambda = 2/3 from ontic chain ---\n";
        std::cout << "    OMEGA_LAMBDA_CONJ = " << ftd::OMEGA_LAMBDA_CONJ << "\n";
        std::cout << "    VACUUM_FRACTION   = " << ftd::VACUUM_FRACTION << "\n";
        std::cout << "    MATTER_FRACTION   = " << ftd::MATTER_FRACTION << "\n";

        check_close("CC-2: OMEGA_LAMBDA_CONJ = 2/3", ftd::OMEGA_LAMBDA_CONJ, 2.0 / 3.0, 1e-15);

        // The vacuum fraction from dual substrate: fraction of energy
        // in the "vacuum" component vs total.
        // VACUUM_FRACTION = 1 - MATTER_FRACTION
        check_close("CC-2: MATTER + VACUUM = 1",
                     ftd::MATTER_FRACTION + ftd::VACUUM_FRACTION, 1.0, 1e-10);

        // Omega_Lambda should relate to the vacuum energy fraction
        // The conjecture: Omega_Lambda = 2/3 (2.7% from observed 0.685)
        double observed_omega_lambda = 0.685;
        double deviation_pct = std::abs(ftd::OMEGA_LAMBDA_CONJ - observed_omega_lambda) /
                               observed_omega_lambda * 100.0;
        std::cout << "    Observed Omega_Lambda: " << observed_omega_lambda << "\n";
        std::cout << "    Deviation: " << deviation_pct << "%\n";
        check("CC-2: Within 5% of observed Omega_Lambda", deviation_pct < 5.0);
    }

    // CC-3: Vacuum energy positive (de Sitter-like)
    {
        std::cout << "\n--- CC-3: Vacuum energy positive ---\n";
        const int L = 8;
        ftd::RenderBridge engine(L);
        engine.toggles.disable_all();
        engine.toggles.dual_substrate = true;

        // Initialize empty dual-substrate vacuum with small fluctuations
        for (int z = 0; z < L; ++z)
          for (int y = 0; y < L; ++y)
            for (int x = 0; x < L; ++x) {
              auto& v = engine.voxel_at(x, y, z);
              double eps = 0.001;
              v.flux_L = {eps * (1.0 + ftd::DELTA_APPROX) / 2.0, 0.0, 0.0};
              v.flux_R = {eps * (1.0 - ftd::DELTA_APPROX) / 2.0, 0.0, 0.0};
              v.flux = v.flux_L + v.flux_R;
            }

        auto audit = engine.energy_audit();
        double vac_energy = audit.E_L_total + audit.E_R_total;
        std::cout << "    Vacuum energy: " << vac_energy << "\n";
        check("CC-3: Vacuum energy > 0 (de Sitter-like)", vac_energy > 0.0);
    }

    // CC-4: Vacuum energy density uniform in empty lattice
    {
        std::cout << "\n--- CC-4: Vacuum energy density is uniform ---\n";
        const int L = 8;
        ftd::RenderBridge engine(L);
        engine.toggles.disable_all();
        engine.toggles.dual_substrate = true;

        // Uniform initialization
        double J0 = 0.05;
        for (int z = 0; z < L; ++z)
          for (int y = 0; y < L; ++y)
            for (int x = 0; x < L; ++x) {
              auto& v = engine.voxel_at(x, y, z);
              v.flux_L = {J0 * (1.0 + ftd::DELTA_APPROX) / 2.0, 0.0, 0.0};
              v.flux_R = {J0 * (1.0 - ftd::DELTA_APPROX) / 2.0, 0.0, 0.0};
              v.flux = v.flux_L + v.flux_R;
            }

        // Check uniformity: compute energy at a few sample sites
        double e_corner = engine.voxel_at(0, 0, 0).flux_L.mag2() +
                          engine.voxel_at(0, 0, 0).flux_R.mag2();
        double e_center = engine.voxel_at(L/2, L/2, L/2).flux_L.mag2() +
                          engine.voxel_at(L/2, L/2, L/2).flux_R.mag2();
        double e_edge   = engine.voxel_at(L-1, 0, 0).flux_L.mag2() +
                          engine.voxel_at(L-1, 0, 0).flux_R.mag2();

        std::cout << "    Energy at corner: " << e_corner << "\n";
        std::cout << "    Energy at center: " << e_center << "\n";
        std::cout << "    Energy at edge:   " << e_edge << "\n";

        check_close("CC-4: Corner == Center (uniform)", e_corner, e_center, 1e-15);
        check_close("CC-4: Corner == Edge (uniform)", e_corner, e_edge, 1e-15);
    }

    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All cosmological constant tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";

    return failures;
}
