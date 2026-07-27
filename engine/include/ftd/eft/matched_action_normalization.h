#pragma once
/**
 * @file matched_action_normalization.h
 * @brief Coefficient consequences of the selected matched gauge action.
 *
 * With E=-(1/c) A_dot-G Phi and
 * L_int=g q(A.v-c Phi), Gauss and Ampere carry the common source coefficient
 * g c/kappa.  Unit matched-source normalization therefore fixes g=kappa/c,
 * and the same path variation fixes F=kappa q(E+v cross B/c).
 */

#include <cmath>

namespace ftd::eft {

struct MatchedActionNormalization {
  double wave_speed = 0.0;
  double field_work_scale = 0.0;
  double connection_coupling = 0.0;
  double gauss_source_coefficient = 0.0;
  double ampere_source_coefficient = 0.0;
  double electric_force_coefficient = 0.0;
  double magnetic_force_coefficient = 0.0;
  double magnetic_to_electric_ratio = 0.0;
  double field_power_coefficient = 0.0;
  double matter_power_coefficient = 0.0;
  double power_residual = 0.0;
  double equal_force_coefficient_residual = 0.0;
  bool equal_force_coefficients_compatible = false;
  bool valid = false;
};

inline MatchedActionNormalization derive_matched_action_normalization(
    double wave_speed, double field_work_scale) {
  MatchedActionNormalization result;
  result.wave_speed = wave_speed;
  result.field_work_scale = field_work_scale;
  if (!(wave_speed > 0.0) || !(field_work_scale > 0.0)
      || !std::isfinite(wave_speed)
      || !std::isfinite(field_work_scale)) {
    return result;
  }
  result.connection_coupling = field_work_scale / wave_speed;
  result.gauss_source_coefficient =
      result.connection_coupling * wave_speed / field_work_scale;
  result.ampere_source_coefficient = result.gauss_source_coefficient;
  result.electric_force_coefficient =
      result.connection_coupling * wave_speed;
  result.magnetic_force_coefficient = result.connection_coupling;
  result.magnetic_to_electric_ratio =
      result.magnetic_force_coefficient
      / result.electric_force_coefficient;
  result.field_power_coefficient = -field_work_scale;
  result.matter_power_coefficient =
      result.electric_force_coefficient;
  result.power_residual = result.field_power_coefficient
      + result.matter_power_coefficient;
  result.equal_force_coefficient_residual =
      result.magnetic_force_coefficient
      - result.electric_force_coefficient;
  result.equal_force_coefficients_compatible =
      result.equal_force_coefficient_residual == 0.0;
  result.valid = std::isfinite(result.connection_coupling)
      && std::isfinite(result.gauss_source_coefficient)
      && std::isfinite(result.electric_force_coefficient)
      && std::isfinite(result.magnetic_force_coefficient)
      && std::isfinite(result.power_residual);
  return result;
}

}  // namespace ftd::eft
