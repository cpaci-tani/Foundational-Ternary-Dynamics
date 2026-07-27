#pragma once
/**
 * @file ftd/lorentz_bcc_time.h
 * @brief Stable local IR surrogate for the selected BCC-time cone hypothesis.
 *
 * FTD-0411 separates the Moore layers by role: the production SC+FCC symbol
 * M18 supplies physical-space propagation, while the selected normalized BCC
 * time kernel is
 *
 *     T_B(theta) = (2/3) [1 - cos^3(theta)].
 *
 * Matching T_B(theta)=c^2 M18 and cancelling the complete q^4 pole term fixes
 * c^2=1/7.  A literal scalar cos^3 clock has two non-real cube-root branches,
 * so this header does NOT implement it.  Instead it exposes the determinant-
 * one period-two kick cell
 *
 *     kappa_even = (1+sqrt(2))/7,
 *     kappa_odd  = (1-sqrt(2))/7,
 *
 * whose exact Floquet pole is
 *
 *     sin^2(theta) = M18/7 + M18^2/196.
 *
 * Exact finite-state positive-norm linear/unitary localization rational in
 * M18 is excluded by the literal branch's irreducible cubic minimal polynomial.
 * This surrogate instead matches the selected BCC-time cone and its q^4
 * cancellation while keeping every microscopic tick nearest-Moore local and
 * the complete band stable.  It differs at isotropic q^6 and is therefore an
 * explicitly selected IR prototype, not an exact BCC temporal dynamics.
 */

namespace ftd {

inline constexpr double LORENTZ_BCC_TIME_SQRT2 =
    1.41421356237309504880168872420969807857;
inline constexpr double LORENTZ_BCC_TIME_KAPPA_EVEN =
    (1.0 + LORENTZ_BCC_TIME_SQRT2) / 7.0;
inline constexpr double LORENTZ_BCC_TIME_KAPPA_ODD =
    (1.0 - LORENTZ_BCC_TIME_SQRT2) / 7.0;
inline constexpr double LORENTZ_BCC_TIME_EFFECTIVE_C2 = 1.0 / 7.0;

inline constexpr double lorentz_bcc_time_kappa(int tick) {
    return (tick & 1) == 0
        ? LORENTZ_BCC_TIME_KAPPA_EVEN
        : LORENTZ_BCC_TIME_KAPPA_ODD;
}

inline constexpr double lorentz_bcc_time_floquet_x(double moore_symbol) {
    return moore_symbol / 7.0
         + moore_symbol * moore_symbol / 196.0;
}

}  // namespace ftd
