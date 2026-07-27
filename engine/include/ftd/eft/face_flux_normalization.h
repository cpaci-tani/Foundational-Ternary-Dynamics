#pragma once
/**
 * @file face_flux_normalization.h
 * @brief Selected normalization map from the matched face complex to native J.
 *
 * The native reaction-free linear source has infrared susceptibility
 *
 *   div(J) / s -> G_C / C_WAVE^2.
 *
 * The matched face field E is normalized by div(E)=rho.  Therefore the only
 * multiplicative map that reproduces the native susceptibility is
 *
 *   J_face = z E,  K_J = z K,  z = G_C / C_WAVE^2.
 *
 * With native longitudinal field-energy coefficient C_WAVE^2, the matched
 * current work coefficient is C_WAVE^2 z^2.  This equals the coefficient
 * G_C z obtained from the written interaction G_C s div(J).  The equality
 * is an algebraic compatibility check; it does not make the face complex a
 * native consequence of the five postulates.
 */

#include "ftd/constants.h"

#include <algorithm>
#include <cmath>

namespace ftd::eft {

struct FaceFluxNormalization {
  double field_scale = 0.0;
  double current_scale = 0.0;
  double energy_scale = 0.0;
  double native_susceptibility = 0.0;
  double mapped_susceptibility = 0.0;
  double native_action_work_coefficient = 0.0;
  double mapped_field_work_coefficient = 0.0;
  double susceptibility_residual = 0.0;
  double work_residual = 0.0;
  bool valid = false;
};

inline FaceFluxNormalization measure_face_flux_normalization() {
  FaceFluxNormalization result;
  const double c2 = C_WAVE * C_WAVE;
  if (!(c2 > 0.0) || !std::isfinite(c2) || !std::isfinite(G_C))
    return result;

  result.native_susceptibility = G_C / c2;
  result.field_scale = result.native_susceptibility;
  result.current_scale = result.field_scale;
  result.energy_scale = c2;
  result.mapped_susceptibility = result.field_scale;
  result.native_action_work_coefficient = G_C * result.field_scale;
  result.mapped_field_work_coefficient =
      result.energy_scale * result.field_scale * result.current_scale;
  result.susceptibility_residual =
      result.mapped_susceptibility - result.native_susceptibility;
  result.work_residual = result.mapped_field_work_coefficient
      - result.native_action_work_coefficient;
  result.valid = std::isfinite(result.field_scale)
      && std::isfinite(result.native_action_work_coefficient)
      && std::abs(result.susceptibility_residual) <= 1e-15
      && std::abs(result.work_residual) <= 1e-15;
  return result;
}

}  // namespace ftd::eft
