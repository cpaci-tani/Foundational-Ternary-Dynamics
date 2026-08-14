#include "ftd/eft/dressed_boost_momentum_map.h"

#include <algorithm>
#include <cmath>

namespace ftd::eft {
namespace {

bool finite(double value) {
  return std::isfinite(value);
}

bool finite(const Vec3& value) {
  return finite(value.x) && finite(value.y) && finite(value.z);
}

double max_abs(const Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

}  // namespace

DressedBoostMomentumMapResult analyze_dressed_boost_momentum_map(
    const DressedBoostMomentumMapInput& input) {
  DressedBoostMomentumMapResult result;
  if (!finite(input.matter_cost) || !finite(input.field_cost)
      || !finite(input.kinetic_coupling)
      || !finite(input.matter_momentum_weight)
      || !finite(input.field_momentum_weight)
      || !finite(input.total_momentum)
      || !finite(input.static_energy_offset)
      || !finite(input.momentum_map_scale)
      || !finite(input.tolerance)) {
    return result;
  }
  if (!(input.tolerance > 0.0)) {
    result.status = DressedBoostMomentumMapStatus::InvalidTolerance;
    return result;
  }

  const double a = input.matter_cost;
  const double k = input.field_cost;
  const double g = input.kinetic_coupling;
  const double b_m = input.matter_momentum_weight;
  const double b_f = input.field_momentum_weight;
  result.energy_hessian_determinant = a * k - g * g;
  result.energy_hessian_positive_definite =
      a > 0.0 && k > 0.0
      && result.energy_hessian_determinant > input.tolerance;
  if (!result.energy_hessian_positive_definite) {
    result.status = DressedBoostMomentumMapStatus::NonPositiveEnergyHessian;
    return result;
  }

  const double b_norm2 = b_m * b_m + b_f * b_f;
  result.momentum_map_rank_one_per_axis = b_norm2 > input.tolerance;
  if (!result.momentum_map_rank_one_per_axis) {
    result.status = DressedBoostMomentumMapStatus::ZeroMomentumMap;
    return result;
  }
  if (std::abs(input.momentum_map_scale) <= input.tolerance) {
    result.status = DressedBoostMomentumMapStatus::InvalidMomentumMapScale;
    return result;
  }

  const double inverse_matter_component =
      (k * b_m - g * b_f) / result.energy_hessian_determinant;
  const double inverse_field_component =
      (a * b_f - g * b_m) / result.energy_hessian_determinant;
  result.dressed_inertial_mass =
      b_m * inverse_matter_component + b_f * inverse_field_component;
  if (!(result.dressed_inertial_mass > input.tolerance)
      || !finite(result.dressed_inertial_mass)) {
    result.status = DressedBoostMomentumMapStatus::ZeroMomentumMap;
    return result;
  }

  result.inverse_mass_curvature = 1.0 / result.dressed_inertial_mass;
  result.matter_odd_amplitude = input.total_momentum
      * (inverse_matter_component / result.dressed_inertial_mass);
  result.field_odd_amplitude = input.total_momentum
      * (inverse_field_component / result.dressed_inertial_mass);
  result.reconstructed_momentum = result.matter_odd_amplitude * b_m
      + result.field_odd_amplitude * b_f;
  result.momentum_residual = max_abs(
      result.reconstructed_momentum - input.total_momentum);

  result.minimum_kinetic_energy = 0.5 * (
      a * result.matter_odd_amplitude.mag2()
      + 2.0 * g * result.matter_odd_amplitude.dot(
          result.field_odd_amplitude)
      + k * result.field_odd_amplitude.mag2());
  const double expected_kinetic = 0.5 * input.total_momentum.mag2()
      / result.dressed_inertial_mass;
  result.energy_residual = std::abs(
      result.minimum_kinetic_energy - expected_kinetic);
  result.minimum_total_energy = input.static_energy_offset
      + result.minimum_kinetic_energy;

  const double scale2 = input.momentum_map_scale
      * input.momentum_map_scale;
  result.scaled_momentum_map_mass = scale2
      * result.dressed_inertial_mass;
  result.field_odd_sector_participates =
      std::abs(inverse_field_component) > input.tolerance;
  result.momentum_scale_ambiguity_exposed =
      std::abs(result.scaled_momentum_map_mass
               - scale2 * result.dressed_inertial_mass)
      <= input.tolerance * std::max(1.0, result.scaled_momentum_map_mass);
  const double scale = std::max({
      1.0, input.total_momentum.mag(),
      std::abs(result.minimum_kinetic_energy),
      result.dressed_inertial_mass});
  result.unique_constrained_minimum = true;
  result.exact_conditional_dressed_mass =
      result.momentum_residual <= input.tolerance * scale
      && result.energy_residual <= input.tolerance * scale
      && result.momentum_scale_ambiguity_exposed;

  if (result.exact_conditional_dressed_mass) {
    result.status = DressedBoostMomentumMapStatus::Valid;
  }
  return result;
}

}  // namespace ftd::eft
