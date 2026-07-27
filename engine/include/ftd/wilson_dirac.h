#pragma once
/**
 * Wilson-Dirac Matter Sector for FTD (Phase II.2 of the campaign)
 *
 * Pre-registration: docs/theory/10_eft_program/PREREG_PHASE_II_WILSON_DIRAC_G2.md
 *                   (tag: preregister-phase-ii-wilson-dirac-g2-v1)
 * Specification:    docs/theory/10_eft_program/SPEC_WILSON_DIRAC_FTD.md
 *
 * SCOPE OF THIS HEADER:
 *   - 4-component complex spinor field on the L^3 lattice
 *   - U(1) gauge link variables on lattice edges (3 spatial directions)
 *   - Spatial Wilson-Dirac operator D_W: applies to a spinor field
 *   - Hermitian Wilson Hamiltonian H_W for real-time evolution
 *   - 4th-order Runge-Kutta time evolution: i d/dt psi = H_W psi
 *   - Plane-wave initial-state helpers (smoke-test fixtures)
 *
 * OUTSIDE THIS MODULE:
 *   - CUDA mirrors the retained spatial D_W diagnostic, not H_W evolution
 *   - No toggle integration with RenderBridge::tick()
 *   - No dynamical gauge-field or photon-loop sector
 *   - The historical cyclotron/g-2 campaign is invalidated by FTD-0412
 *
 * CONVENTIONS (committed pre-measurement per SPEC_WILSON_DIRAC_FTD.md):
 *   - Lattice spacing a = 1 (engine-internal units; physical = ell_P per FTD calibration)
 *   - 3 spatial dimensions; time evolved continuously via RK4
 *   - Wilson parameter r = 1 (canonical)
 *   - Chiral (Weyl) basis for gamma matrices
 *   - Periodic boundary conditions (matching engine convention)
 */

#include <array>
#include <complex>
#include <cstddef>
#include <vector>

#include "ftd/lattice.h"

namespace ftd {
namespace wilson_dirac {

using cdouble = std::complex<double>;

// =============================================================================
// 4-component complex spinor.
// Index 0,1 = upper (left-chiral) components.
// Index 2,3 = lower (right-chiral) components.
// =============================================================================
using Spinor = std::array<cdouble, 4>;

inline Spinor zero_spinor() {
    return Spinor{cdouble{0, 0}, cdouble{0, 0}, cdouble{0, 0}, cdouble{0, 0}};
}

// Hermitian inner product <a|b> = sum_alpha conj(a[alpha]) * b[alpha]
inline cdouble spinor_dot(const Spinor& a, const Spinor& b) {
    cdouble result{0, 0};
    for (std::size_t k = 0; k < 4; ++k) {
        result += std::conj(a[k]) * b[k];
    }
    return result;
}

inline double spinor_norm_squared(const Spinor& a) {
    double n = 0.0;
    for (std::size_t k = 0; k < 4; ++k) {
        n += std::norm(a[k]);
    }
    return n;
}

inline Spinor add(const Spinor& a, const Spinor& b) {
    return {a[0] + b[0], a[1] + b[1], a[2] + b[2], a[3] + b[3]};
}
inline Spinor scale(cdouble c, const Spinor& a) {
    return {c * a[0], c * a[1], c * a[2], c * a[3]};
}

// =============================================================================
// Gamma matrices in chiral (Weyl) basis.
//
// gamma^0 = ((0, I), (I, 0))    -- temporal (used in inner product structure)
// gamma^1 = ((0, -sigma^1), (sigma^1, 0))
// gamma^2 = ((0, -sigma^2), (sigma^2, 0))
// gamma^3 = ((0, -sigma^3), (sigma^3, 0))
//
// Each is a 4x4 complex matrix. We store them compactly as four-of-four
// 2x2 sigma blocks; multiplication kernel below.
//
// Spatial-direction index: 0 = x, 1 = y, 2 = z. (Maps to gamma^1, gamma^2, gamma^3.)
// =============================================================================

// Apply (r * I - gamma^mu) to a spinor, where mu in {0, 1, 2} maps to
// gamma^1, gamma^2, gamma^3 (spatial gamma matrices).
// In chiral basis, gamma^i = ((0, -sigma^i), (sigma^i, 0)).
Spinor apply_r_minus_gamma_spatial(double r, int mu, const Spinor& psi);

// Apply (r * I + gamma^mu) similarly.
Spinor apply_r_plus_gamma_spatial(double r, int mu, const Spinor& psi);

// Apply spin operator Sigma^i = (i/2) [gamma^j, gamma^k] in chiral basis,
// = diag(sigma^i, sigma^i). For spin-precession measurement.
Spinor apply_sigma_spatial(int i, const Spinor& psi);

// =============================================================================
// SpinorField -- one Spinor per lattice site, indexed via Lattice::index(x,y,z).
// =============================================================================
struct SpinorField {
    int L;
    std::vector<Spinor> data;

    explicit SpinorField(int lattice_size)
        : L(lattice_size), data(static_cast<std::size_t>(lattice_size) * lattice_size * lattice_size,
                                 zero_spinor()) {}

    Spinor& at(int idx) { return data[static_cast<std::size_t>(idx)]; }
    const Spinor& at(int idx) const { return data[static_cast<std::size_t>(idx)]; }

    // Total norm-squared (Hermitian inner product, no spacing factor).
    double total_norm_squared() const;
};

// =============================================================================
// GaugeLinks -- one U(1) phase per lattice edge in each of 3 spatial directions.
// U_mu(n) = exp(i a g_FTD A_mu(n)) for FTD-coupled simulation.
// For free-fermion smoke test: all U_mu(n) = 1 (identity).
// =============================================================================
struct GaugeLinks {
    int L;
    // Layout: [direction (0=x, 1=y, 2=z)][site_index] -> phase
    std::array<std::vector<cdouble>, 3> U;

    explicit GaugeLinks(int lattice_size) : L(lattice_size) {
        const std::size_t N = static_cast<std::size_t>(lattice_size) * lattice_size * lattice_size;
        for (int mu = 0; mu < 3; ++mu) {
            U[mu].assign(N, cdouble{1, 0});
        }
    }

    // Set all links to identity (free-fermion configuration).
    void set_identity();

    // Set links to uniform B = B0 z-hat in Landau gauge:
    //   U_x(n) = exp(-i g a B0 n_y a)   (negative-y phase along x)
    //   U_y(n) = U_z(n) = 1
    void set_uniform_B_z(double g_a_B0_a);
};

// =============================================================================
// Spatial Wilson-Dirac operator D_W applied to a spinor field.
//
//   (D_W psi)(n) = (m + 3 c_s r/a) psi(n)
//                  - (c_s/2a) sum_{mu=0,1,2} [ (r - gamma^mu) U_mu(n) psi(n + mu)
//                                                 + (r + gamma^mu) U_mu^dag(n - mu) psi(n - mu) ]
//
// where mu in {0, 1, 2} indexes the 3 spatial directions.
//
// IMPORTANT (FTD-0412): this spatial operator is retained for the Euclidean
// and gauge-covariance campaign tests.  It is not itself the real-time
// relativistic Hamiltonian, and ||D_W psi|| is not an energy eigenvalue for a
// generic spinor.  Real-time evolution uses apply_wilson_hamiltonian below.
// =============================================================================
struct WilsonDiracParams {
    double m = 0.51099895069e-3;  // electron mass in lattice units (placeholder; calibration TBD)
    double r = 1.0;              // Wilson parameter
    double a = 1.0;              // lattice spacing (engine-internal units)
    double spatial_speed = 1.0;  // [SELECTION] spatial normalization c_s; legacy default preserved
    // H_W-only transverse averaging weight b.  The free kinetic symbol is
    //   K_i = sin(q_i)/a * [(1-2b) + b(cos(q_j)+cos(q_k))].
    // b=0 preserves the axial Wilson Hamiltonian.  FTD-0413 selects b=1/3
    // together with r^2=4/3 for a nearest-Moore q^4-free matter pole.
    // The retained spatial D_W diagnostic and its CUDA mirror ignore b.
    double kinetic_transverse_weight = 0.0;
};

// Apply D_W to `psi` and write result to `out`.
// `lattice` provides index(...) and neighbor lookups; `links` provides the U_mu.
// Both fields must have the same L as `lattice`.
void apply_wilson_dirac(SpinorField& out,
                        const SpinorField& psi,
                        const GaugeLinks& links,
                        const Lattice& lattice,
                        const WilsonDiracParams& params);

// Hermitian conjugate of D_W (for diagnostic + adjoint methods if needed later).
// (D_W^dag psi)(n) = same form with U_mu <-> U_mu^dag and sign of gamma^mu flipped.
void apply_wilson_dirac_dagger(SpinorField& out,
                               const SpinorField& psi,
                               const GaugeLinks& links,
                               const Lattice& lattice,
                               const WilsonDiracParams& params);

// Apply the Hermitian spatial Wilson Hamiltonian
//
//   H_W(q) = sum_i c_s alpha_i K_i(q)
//          + beta [m + c_s r sum_i(1-cos(q_i))/a].
//
//   K_i(q) = sin(q_i)/a * [(1-2b) + b(cos(q_j)+cos(q_k))],
//
// where b=params.kinetic_transverse_weight and {i,j,k}={x,y,z}.  In position
// space the b terms use the 12 face-diagonal Moore neighbours.  With gauge
// links, each diagonal transporter is the equal average of its two shortest
// oriented paths, which preserves U(1) covariance and Hermiticity.
//
// For identity links its exact free spectrum is
//
//   E^2 = c_s^2 sum_i K_i(q)^2
//       + [m + c_s r sum_i(1-cos(q_i))/a]^2.
//
// This is the operator used by evolve_rk4_step.  The optional c_s parameter
// permits an explicitly selected leading-cone diagnostic; c_s=1 preserves the
// pre-FTD-0412 normalization.
void apply_wilson_hamiltonian(SpinorField& out,
                              const SpinorField& psi,
                              const GaugeLinks& links,
                              const Lattice& lattice,
                              const WilsonDiracParams& params);

// =============================================================================
// Time evolution: i d/dt psi = H_W psi.
// One RK4 step of length dt. Updates `psi` in-place.
// Two scratch buffers required (k1, k_temp).
// =============================================================================
void evolve_rk4_step(SpinorField& psi,
                     SpinorField& k1,
                     SpinorField& k_temp,
                     const GaugeLinks& links,
                     const Lattice& lattice,
                     const WilsonDiracParams& params,
                     double dt);

// =============================================================================
// Plane-wave initial state for smoke testing.
//
// psi(n) = u(p, s) * exp(i p . n)
//
// where u(p, s) is the standard positive-energy spinor for momentum p and spin
// axis s. Useful for verifying the free-fermion dispersion (Phase II.2-A).
// =============================================================================
void initialize_plane_wave(SpinorField& psi,
                           const Lattice& lattice,
                           const std::array<double, 3>& momentum,
                           int spin_index);  // 0 = up along z, 1 = down along z

}  // namespace wilson_dirac
}  // namespace ftd
