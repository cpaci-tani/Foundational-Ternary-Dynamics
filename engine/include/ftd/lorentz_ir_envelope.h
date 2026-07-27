#pragma once
/**
 * @file ftd/lorentz_ir_envelope.h
 * @brief Leading infrared Lorentz-violation envelope for the FTD-0413 cone.
 *
 * FTD-0413 aligns the selected free BCC-time flux and improved Wilson-matter
 * poles through q^4.  Exact all-orders Lorentz symmetry is not assumed here.
 * Instead this header exposes the first surviving, q^6 pole coefficients and
 * the resulting O(q^4) phase-speed spread for q = |k| a << 1.
 *
 * The live flux clock is the FTD-0411 period-two surrogate.  The matter clock
 * is the selected b=1/3, r^2=4/3 Hamiltonian evolved by unit-step RK4.  After
 * factoring out c_s^2=1/7, their squared phases are
 *
 *   omega_m^2/c_s^2 = q^2 + B_m(n) q^6 + O(q^8),
 *   omega_f^2/c_s^2 = q^2 + B_f(n) q^6 + O(q^8),
 *
 * where A4=sum n_i^4, A6=sum n_i^6, |n|=1, and
 *
 *   B_m = 121/4410 + A4/36 - A6/15,
 *   B_f = -121/17640 + A4/72 - A6/90.
 *
 * Consequently v_phase/c_s = 1 + B(n) q^4/2 + O(q^6).  The leading
 * coefficient is an asymptotic diagnostic, not a rigorous finite-q error
 * bound and not evidence that any particular physical lattice spacing meets
 * experimental limits.
 */

#include <cmath>

namespace ftd {

struct LorentzDirectionMoments {
    double a4 = 0.0;
    double a6 = 0.0;
};

inline LorentzDirectionMoments lorentz_direction_moments(double nx,
                                                          double ny,
                                                          double nz) {
    const double x = nx * nx;
    const double y = ny * ny;
    const double z = nz * nz;
    const double norm2 = x + y + z;
    if (!(norm2 > 0.0)) return {};
    const double inv = 1.0 / norm2;
    const double ux = x * inv;
    const double uy = y * inv;
    const double uz = z * inv;
    return {
        ux * ux + uy * uy + uz * uz,
        ux * ux * ux + uy * uy * uy + uz * uz * uz,
    };
}

inline double lorentz_matter_q6_coefficient(double nx,
                                             double ny,
                                             double nz) {
    const LorentzDirectionMoments m = lorentz_direction_moments(nx, ny, nz);
    return 121.0 / 4410.0 + m.a4 / 36.0 - m.a6 / 15.0;
}

inline double lorentz_flux_q6_coefficient(double nx,
                                           double ny,
                                           double nz) {
    const LorentzDirectionMoments m = lorentz_direction_moments(nx, ny, nz);
    return -121.0 / 17640.0 + m.a4 / 72.0 - m.a6 / 90.0;
}

inline double lorentz_common_cone_q6_gap(double nx,
                                         double ny,
                                         double nz) {
    return lorentz_matter_q6_coefficient(nx, ny, nz)
         - lorentz_flux_q6_coefficient(nx, ny, nz);
}

// Exact leading coefficients after extremizing over the direction sphere.
inline constexpr double LORENTZ_IR_SELECTED_C2 = 1.0 / 7.0;
inline constexpr double LORENTZ_IR_MATTER_AXIS_B = -101.0 / 8820.0;
inline constexpr double LORENTZ_IR_MATTER_BODY_B = 155.0 / 5292.0;
inline constexpr double LORENTZ_IR_FLUX_AXIS_B = -1.0 / 245.0;
inline constexpr double LORENTZ_IR_FLUX_BODY_B = -55.0 / 15876.0;

// Largest same-direction matter/flux phase-speed gap:
//   max_n |v_m(n)-v_f(n)|/c_s = (65/3969) q^4 + O(q^6).
inline constexpr double LORENTZ_IR_COMMON_SPEED_GAP_COEFF = 65.0 / 3969.0;

// Largest speed spread across either selected sector and any direction:
//   [max_{sector,n} v - min_{sector,n} v]/c_s
//       = (11/540) q^4 + O(q^6).
inline constexpr double LORENTZ_IR_ALL_SPEED_SPREAD_COEFF = 11.0 / 540.0;

inline double lorentz_ir_leading_common_speed_gap(double q) {
    const double q2 = q * q;
    return LORENTZ_IR_COMMON_SPEED_GAP_COEFF * q2 * q2;
}

inline double lorentz_ir_leading_speed_spread(double q) {
    const double q2 = q * q;
    return LORENTZ_IR_ALL_SPEED_SPREAD_COEFF * q2 * q2;
}

// Leading-order IR adequacy threshold.  This inverts only the displayed q^4
// envelope; omitted O(q^6) terms still require a finite-q numerical audit.
inline double lorentz_ir_q_limit(double fractional_speed_tolerance) {
    if (!(fractional_speed_tolerance > 0.0)) return 0.0;
    return std::pow(fractional_speed_tolerance
                    / LORENTZ_IR_ALL_SPEED_SPREAD_COEFF, 0.25);
}

}  // namespace ftd
