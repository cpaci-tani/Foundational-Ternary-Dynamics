#pragma once
/**
 * @file ftd/eft/lattice_coulomb_gate.h
 * @brief Phase-G lattice Coulomb gate paired with energy_audit conventions.
 *
 * DERIV_EMERGENT_COULOMB_GEOMETRIC.md derives alpha_r = 2 r G_L(r) under
 * field_energy = SUM |J|^2. Since 2026-04-27 diagnostics_compute.cpp reports
 * field_energy = 1/2 SUM |J|^2, the paired gate for energy_audit().field_energy
 * is alpha_r_expected = r G_L(r).
 *
 * Epistemic status: [TOOLING] — documents observable/gate pairing only.
 */

#include <cmath>

namespace ftd {
namespace eft {

/// Which field-energy accumulator convention the gate must pair with.
enum class FieldEnergyConvention {
    /// energy_audit().field_energy = 1/2 SUM |J|^2 (current engine default).
    HalfSumJ2,
    /// Legacy Phase-G derivation: SUM |J|^2 with no 1/2 prefactor.
    SumJ2NoHalf,
};

/// Periodic cubic-torus Poisson Green's function G_L(r) along +x (k-space sum).
inline double lattice_greens_function(int lattice_size, int r) {
    constexpr double kPi = 3.141592653589793238462643383279502884;
    double G = 0.0;
    const double twopi_L = 2.0 * kPi / static_cast<double>(lattice_size);
    for (int kx = 0; kx < lattice_size; ++kx) {
        for (int ky = 0; ky < lattice_size; ++ky) {
            for (int kz = 0; kz < lattice_size; ++kz) {
                if (kx == 0 && ky == 0 && kz == 0) {
                    continue;
                }
                const double sx = std::sin(twopi_L * kx * 0.5);
                const double sy = std::sin(twopi_L * ky * 0.5);
                const double sz = std::sin(twopi_L * kz * 0.5);
                const double lambda = 4.0 * (sx * sx + sy * sy + sz * sz);
                const double phase = twopi_L * kx * r;
                G += std::cos(phase) / lambda;
            }
        }
    }
    const double volume = static_cast<double>(lattice_size) *
                          static_cast<double>(lattice_size) *
                          static_cast<double>(lattice_size);
    return G / volume;
}

/// Expected alpha_r(r,L) for the chosen field-energy convention.
inline double phase_g_alpha_r(int lattice_size, int r,
                              FieldEnergyConvention convention) {
    const double rG = static_cast<double>(r) * lattice_greens_function(lattice_size, r);
    switch (convention) {
        case FieldEnergyConvention::HalfSumJ2:
            return rG;
        case FieldEnergyConvention::SumJ2NoHalf:
            return 2.0 * rG;
    }
    return rG;
}

}  // namespace eft
}  // namespace ftd
