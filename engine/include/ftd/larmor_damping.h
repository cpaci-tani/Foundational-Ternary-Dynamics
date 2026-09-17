#pragma once
/**
 * Larmor radiation-reaction damping law — single CPU/CUDA source of truth.
 *
 * Used by `phase_write` (engine/src/render_bridge_phases/phase_write.cpp, both
 * the dual- and single-substrate damping blocks) and by the device mirror
 * `ftd::gpu::kernels::effective_damping` (engine/cuda/kernels_stencil_common.cuh).
 * Both backends call THIS header so the two implementations cannot drift; the
 * file follows the `causal_kinematics.h` host/device pattern (FTD-0402).
 *
 * ── The law ────────────────────────────────────────────────────────────────
 *
 *   gain(a) = min(1 + K_LARMOR·a², LARMOR_MAX_GAIN)        (≥ 1, monotone ↑)
 *   eff(a)  = damping_factor ^ gain(a)                     (≤ baseline, > 0)
 *
 * where `damping_factor` is the baseline per-step survival factor phase_write
 * already computes (`1 − DAMPING`, or `pow(1 − DAMPING, dt)` for a super-unit
 * step) and `a` is the local acceleration magnitude carried in `near_accel`.
 *
 * Equivalently the dissipation RATE carries the a² term,
 *   Γ(a) = Γ₀·gain(a),  Γ₀ = −ln(damping_factor),  eff = exp(−Γ(a)),
 * which is the textbook radiation-reaction statement: an accelerating charge
 * loses energy at a rate that grows with a², on TOP of the baseline loss.
 *
 * Properties the regression test (tests/test_larmor_damping_law.cpp) pins:
 *   1. eff(0) == damping_factor exactly — switching the toggle on changes
 *      nothing at zero acceleration.
 *   2. eff is monotonically non-increasing in |a| ⇒ the damping applied,
 *      1 − eff, is monotonically non-decreasing in |a|.
 *   3. eff(a) < damping_factor strictly for every a > 0 ⇒ Larmor always
 *      dissipates MORE than the baseline, never less.
 *   4. 0 < eff(a) ≤ damping_factor for every finite a ⇒ the field register is
 *      never sign-flipped or amplified, at any acceleration. The gain clamp
 *      bounds the per-step loss at 1 − damping_factor^LARMOR_MAX_GAIN.
 *
 * ── Why this replaced the previous form (2026-09-16) ───────────────────────
 *
 * phase_write and the device mirror both used
 *
 *     larmor_mod = min(1, LARMOR_FLOOR + K_LARMOR·a²)
 *     eff        = 1 − DAMPING · larmor_mod
 *
 * i.e. the baseline loss MULTIPLIED by a factor capped at 1. That made the
 * toggle a dissipation *reducer*: at a = 0 it applied 1% of the baseline loss
 * (LARMOR_FLOOR), it reached parity with the baseline only at
 * a = sqrt((1 − LARMOR_FLOOR)/K_LARMOR) ≈ 0.171, and it could never exceed the
 * baseline for any acceleration. An accelerating charge therefore radiated
 * LESS than a static one — the opposite of Larmor — and the note in
 * constants.h that K_LARMOR is scaled by N_EFF so that "Larmor dominates" the
 * coupling injection rate was unreachable for ANY value of the constant, since
 * the cap bounded the loss by DAMPING itself.
 *
 * K_LARMOR and LARMOR_FLOOR (constants.h) are unchanged. LARMOR_FLOOR's stated
 * purpose — "ensures thermodynamic dissipation is never completely off" — now
 * holds by construction, because gain ≥ 1 means the Larmor site always takes at
 * least the full baseline damping; the constant is retained for the pinned
 * constant-value checks and for provenance.
 *
 * Bit-parity note: `pow` is the only non-elementary operation here and it is
 * evaluated with the same argument on both backends. Host libm and CUDA libdevice
 * `pow` are each ≤ 2 ulp, so CPU/GPU agree to ~1e-16 relative — far inside the
 * 10% audit tolerance the larmor parity gate uses (test_gpu_parity_complete.cpp,
 * GPC-18). Unlike the previous add-multiply-min form this is NOT bit-exact
 * across backends; no test asserted bit-exactness under this toggle.
 */

#include "constants.h"

#ifndef __CUDACC__
#include <cmath>
#endif

#ifdef __CUDACC__
#define FTD_LARMOR_HD __host__ __device__ __forceinline__
#else
#define FTD_LARMOR_HD inline
#endif

namespace ftd {

// Upper bound on the radiation-reaction gain. With DAMPING = ALPHA the worst
// case survival factor is (1 − ALPHA)^256 ≈ 0.153, i.e. at most ~85% of the
// register is dissipated in one step — strong enough that Larmor dominates any
// per-site coupling injection (which needs gain ≈ 12), and bounded enough that
// no acceleration, however large or non-finite the input, can drive the factor
// out of (0, damping_factor].
inline constexpr double LARMOR_MAX_GAIN = 256.0;

FTD_LARMOR_HD double larmor_pow(double base, double exponent) {
#ifdef __CUDACC__
  return pow(base, exponent);
#else
  return std::pow(base, exponent);
#endif
}

// gain(a) = min(1 + K_LARMOR·a², LARMOR_MAX_GAIN), clamped to [1, MAX].
// The comparison ordering also maps a NaN acceleration onto the clamp rather
// than propagating NaN into the field registers.
FTD_LARMOR_HD double larmor_damping_gain(double accel_mag) {
  const double gain = 1.0 + K_LARMOR * accel_mag * accel_mag;
  if (!(gain > 1.0)) return 1.0;                     // a == 0, or NaN
  return gain < LARMOR_MAX_GAIN ? gain : LARMOR_MAX_GAIN;
}

// Effective per-step survival factor at a site whose local acceleration
// magnitude is `accel_mag`, given the baseline factor phase_write computed.
FTD_LARMOR_HD double larmor_effective_damping(double damping_factor,
                                              double accel_mag) {
  const double gain = larmor_damping_gain(accel_mag);
  if (gain <= 1.0) return damping_factor;            // exact baseline at a = 0
  return larmor_pow(damping_factor, gain);
}

}  // namespace ftd
