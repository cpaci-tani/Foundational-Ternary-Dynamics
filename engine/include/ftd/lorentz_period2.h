#pragma once
/**
 * @file ftd/lorentz_period2.h
 * @brief Exact coefficients for the P4-preserving period-two wave prototype.
 *
 * FTD-0408 replaces the constant free-wave kick coefficient by the selected
 * two-tick sequence
 *
 *     kappa_even = +3/13,    kappa_odd = -1/13.
 *
 * Each tick still reads only the current voxel and its nearest Moore shell.
 * For a production-stencil eigenvalue M, the two-tick Floquet pole is
 *
 *     sin^2(theta) = M/13 + 3 M^2/676,
 *
 * where theta is phase per microscopic tick.  This cancels the complete q^4
 * term in theta^2 while remaining stable for 0 <= M <= 16/3.  The negative
 * odd-tick coefficient and the period-two clock are selected architecture,
 * not consequences of P1-P5.  See AUDIT_LORENTZ_P4_PERIOD2.md.
 */

namespace ftd {

inline constexpr double LORENTZ_PERIOD2_KAPPA_EVEN = 3.0 / 13.0;
inline constexpr double LORENTZ_PERIOD2_KAPPA_ODD = -1.0 / 13.0;
inline constexpr double LORENTZ_PERIOD2_EFFECTIVE_C2 = 1.0 / 13.0;

inline constexpr double lorentz_period2_kappa(int tick) {
    return (tick & 1) == 0
        ? LORENTZ_PERIOD2_KAPPA_EVEN
        : LORENTZ_PERIOD2_KAPPA_ODD;
}

inline constexpr double lorentz_period2_floquet_x(double moore_symbol) {
    return moore_symbol / 13.0
         + 3.0 * moore_symbol * moore_symbol / 676.0;
}

}  // namespace ftd
