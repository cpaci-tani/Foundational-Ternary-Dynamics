#pragma once

/**
 * Target-blind action-angle chart for one nonzero source-free field mode.
 *
 * FTD-0574 derives the production free-field kick-drift map and proves that
 * (J,W) is its native discrete canonical pair.  For a K-eigenmode with
 * 0<a<4 this header supplies the exact canonical chart in which one primitive
 * engine tick is a rigid rotation.  No G*, target period, measured frequency,
 * or fitted coupling enters the construction.
 *
 * Scope: isolated source-free modal carrier only.  This is not a localized
 * maintained clock, matter clock, actualization gate, or SI-time calibration.
 */

#include "ftd/ontic/gauge_couplings.h"

#include <cmath>
#include <stdexcept>

namespace ftd::eft {

struct NativeModalState {
  double coordinate = 0.0;  // J-mode amplitude
  double momentum = 0.0;    // W-mode amplitude (native Legendre momentum)
};

struct NativeModalPhaseAction {
  double eigenvalue = 0.0;       // a in U_a
  double cos_theta = 1.0;
  double sin_theta = 0.0;
  double radians_per_tick = 0.0;
  double canonical_q = 0.0;
  double canonical_p = 0.0;
  double action = 0.0;
  double phase = 0.0;
};

inline bool is_elliptic_mode_eigenvalue(double eigenvalue) {
  return std::isfinite(eigenvalue) && eigenvalue > 0.0 && eigenvalue < 4.0;
}

inline NativeModalState advance_native_modal_tick(
    const NativeModalState& state, double eigenvalue) {
  if (!is_elliptic_mode_eigenvalue(eigenvalue)) {
    throw std::invalid_argument(
        "native modal phase/action requires a finite eigenvalue in (0,4)");
  }
  NativeModalState next;
  next.momentum = state.momentum - eigenvalue * state.coordinate;
  next.coordinate = state.coordinate + next.momentum;
  return next;
}

inline NativeModalPhaseAction native_modal_phase_action(
    const NativeModalState& state, double eigenvalue) {
  if (!is_elliptic_mode_eigenvalue(eigenvalue)) {
    throw std::invalid_argument(
        "native modal phase/action requires a finite eigenvalue in (0,4)");
  }

  NativeModalPhaseAction result;
  result.eigenvalue = eigenvalue;
  result.cos_theta = 1.0 - 0.5 * eigenvalue;
  result.sin_theta = std::sqrt(eigenvalue * (1.0 - 0.25 * eigenvalue));
  result.radians_per_tick = std::atan2(result.sin_theta, result.cos_theta);

  const double root_sine = std::sqrt(result.sin_theta);
  result.canonical_q = root_sine * state.coordinate;
  result.canonical_p =
      (state.momentum - 0.5 * eigenvalue * state.coordinate) / root_sine;
  result.action = 0.5 * (
      result.canonical_q * result.canonical_q
      + result.canonical_p * result.canonical_p);
  result.phase = std::atan2(result.canonical_p, result.canonical_q);
  return result;
}

inline double wrap_native_modal_phase(double phase) {
  const double pi = std::acos(-1.0);
  const double two_pi = 2.0 * pi;
  double wrapped = std::fmod(phase + pi, two_pi);
  if (wrapped < 0.0) wrapped += two_pi;
  return wrapped - pi;
}

/** Positive symbol -L_18(k) of the production face+edge stencil. */
inline double production18_spatial_symbol(
    double kx, double ky, double kz) {
  const double cx = std::cos(kx);
  const double cy = std::cos(ky);
  const double cz = std::cos(kz);
  return 4.0 - (2.0 / 3.0) * (
      cx + cy + cz + cx * cy + cy * cz + cz * cx);
}

/**
 * Source-fixed production eigenvalue a(k)=C_WAVE^2[-L_18(k)].
 *
 * C_WAVE=1/sqrt(3) is a declared [SELECTION].  Once that engine selection and
 * the production stencil are fixed, this modal rate has no free parameter.
 */
inline double production18_mode_eigenvalue(
    double kx, double ky, double kz) {
  const double c2 = ftd::ontic::C_WAVE * ftd::ontic::C_WAVE;
  return c2 * production18_spatial_symbol(kx, ky, kz);
}

}  // namespace ftd::eft
