#include "ftd/eft/native_ternary_dipole_phase_wedge_memory.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace ftd::eft {
namespace {

bool finite_vector(const NativeOrientationVector& value) {
  return std::isfinite(value[0]) && std::isfinite(value[1])
      && std::isfinite(value[2]);
}

NativeOrientationVector add(
    const NativeOrientationVector& left,
    const NativeOrientationVector& right) {
  return {left[0] + right[0], left[1] + right[1], left[2] + right[2]};
}

NativeOrientationVector subtract(
    const NativeOrientationVector& left,
    const NativeOrientationVector& right) {
  return {left[0] - right[0], left[1] - right[1], left[2] - right[2]};
}

NativeOrientationVector scale(
    const NativeOrientationVector& value, double factor) {
  return {factor * value[0], factor * value[1], factor * value[2]};
}

double dot(
    const NativeOrientationVector& left,
    const NativeOrientationVector& right) {
  return left[0] * right[0] + left[1] * right[1]
      + left[2] * right[2];
}

double norm(const NativeOrientationVector& value) {
  return std::sqrt(dot(value, value));
}

double max_abs(const NativeOrientationVector& value) {
  return std::max({std::abs(value[0]), std::abs(value[1]),
                   std::abs(value[2])});
}

NativeOrientationVector signed_cubic_transform(
    const NativeOrientationVector& value) {
  return {-value[1], value[2], -value[0]};
}

double swept_area(
    double q0, double p0, double q1, double p1) {
  const double qbar = 0.5 * (q0 + q1);
  const double pbar = 0.5 * (p0 + p1);
  return qbar * (p1 - p0) - pbar * (q1 - q0);
}

}  // namespace

NativeOrientationMemoryResult analyze_native_ternary_dipole_phase_wedge_memory(
    const std::vector<NativeOrientationMemorySite>& sites,
    const NativeOrientationMemoryParameters& parameters) {
  NativeOrientationMemoryResult result;

  if (!std::isfinite(parameters.memory_mass)
      || !std::isfinite(parameters.memory_quartic_coupling)
      || !std::isfinite(parameters.tolerance)) {
    result.status = NativeOrientationMemoryStatus::NonFiniteInput;
    return result;
  }
  if (parameters.memory_mass <= 0.0) {
    result.status = NativeOrientationMemoryStatus::InvalidMemoryMass;
    return result;
  }
  if (parameters.memory_quartic_coupling <= 0.0) {
    result.status = NativeOrientationMemoryStatus::InvalidMemoryCoupling;
    return result;
  }
  if (parameters.tolerance <= 0.0) {
    result.status = NativeOrientationMemoryStatus::InvalidTolerance;
    return result;
  }
  if (sites.empty()) {
    result.status = NativeOrientationMemoryStatus::EmptyRegion;
    return result;
  }

  int positive_count = 0;
  int negative_count = 0;
  const NativeOrientationVector audit_origin{0.75, -1.25, 2.0};
  for (std::size_t index = 0; index < sites.size(); ++index) {
    const auto& site = sites[index];
    if (!finite_vector(site.position) || !finite_vector(site.flux)
        || !finite_vector(site.wave_velocity)) {
      result.status = NativeOrientationMemoryStatus::NonFiniteInput;
      return result;
    }
    if (site.state < -1 || site.state > 1) {
      result.status = NativeOrientationMemoryStatus::NonTernaryState;
      return result;
    }
    result.total_ternary_state += site.state;
    result.ternary_dipole = add(
        result.ternary_dipole,
        scale(site.position, static_cast<double>(site.state)));
    result.shifted_origin_dipole = add(
        result.shifted_origin_dipole,
        scale(subtract(site.position, audit_origin),
              static_cast<double>(site.state)));
    if (site.state == 1) {
      ++positive_count;
      result.positive_endpoint_index = static_cast<int>(index);
    } else if (site.state == -1) {
      ++negative_count;
      result.negative_endpoint_index = static_cast<int>(index);
    }
  }
  if (result.total_ternary_state != 0) {
    result.status = NativeOrientationMemoryStatus::NonNeutralRegion;
    return result;
  }
  if (positive_count == 0) {
    result.status = NativeOrientationMemoryStatus::MissingPositiveEndpoint;
    return result;
  }
  if (negative_count == 0) {
    result.status = NativeOrientationMemoryStatus::MissingNegativeEndpoint;
    return result;
  }
  if (positive_count != 1) {
    result.status = NativeOrientationMemoryStatus::NonUniquePositiveEndpoint;
    return result;
  }
  if (negative_count != 1) {
    result.status = NativeOrientationMemoryStatus::NonUniqueNegativeEndpoint;
    return result;
  }

  const auto& positive = sites[
      static_cast<std::size_t>(result.positive_endpoint_index)];
  const auto& negative = sites[
      static_cast<std::size_t>(result.negative_endpoint_index)];
  const NativeOrientationVector endpoint_separation = subtract(
      positive.position, negative.position);
  if (norm(endpoint_separation) <= parameters.tolerance) {
    result.status = NativeOrientationMemoryStatus::CoincidentEndpoints;
    return result;
  }
  result.dipole_norm = norm(result.ternary_dipole);
  if (!std::isfinite(result.dipole_norm)
      || result.dipole_norm <= parameters.tolerance) {
    result.status = NativeOrientationMemoryStatus::ZeroDipole;
    return result;
  }
  result.polar_axis = scale(result.ternary_dipole, 1.0 / result.dipole_norm);
  result.origin_independence_residual = subtract(
      result.shifted_origin_dipole, result.ternary_dipole);
  result.axis_norm_residual = norm(result.polar_axis) - 1.0;

  result.positive_coordinate = dot(result.polar_axis, positive.flux);
  result.negative_coordinate = dot(result.polar_axis, negative.flux);
  result.positive_momentum = dot(
      result.polar_axis, positive.wave_velocity);
  result.negative_momentum = dot(
      result.polar_axis, negative.wave_velocity);
  result.phase_wedge = result.positive_coordinate * result.negative_momentum
      - result.negative_coordinate * result.positive_momentum;
  const double wedge_scale = std::max({
      1.0,
      std::abs(result.positive_coordinate * result.negative_momentum),
      std::abs(result.negative_coordinate * result.positive_momentum)});
  if (!std::isfinite(result.phase_wedge)
      || std::abs(result.phase_wedge) <= parameters.tolerance * wedge_scale) {
    result.status = NativeOrientationMemoryStatus::ZeroPhaseWedge;
    return result;
  }
  result.chirality = result.phase_wedge > 0.0 ? 1 : -1;
  result.time_reversed_phase_wedge =
      result.positive_coordinate * (-result.negative_momentum)
      - result.negative_coordinate * (-result.positive_momentum);
  result.time_reversal_residual =
      result.time_reversed_phase_wedge + result.phase_wedge;

  const double gram_pp = result.positive_coordinate
          * result.positive_coordinate
      + result.positive_momentum * result.positive_momentum;
  const double gram_mm = result.negative_coordinate
          * result.negative_coordinate
      + result.negative_momentum * result.negative_momentum;
  const double gram_pm = result.positive_coordinate
          * result.negative_coordinate
      + result.positive_momentum * result.negative_momentum;
  result.gram_determinant = gram_pp * gram_mm - gram_pm * gram_pm;
  result.gram_wedge_square_residual = result.gram_determinant
      - result.phase_wedge * result.phase_wedge;

  const auto transformed_axis = signed_cubic_transform(result.polar_axis);
  const double transformed_qp = dot(
      transformed_axis, signed_cubic_transform(positive.flux));
  const double transformed_qm = dot(
      transformed_axis, signed_cubic_transform(negative.flux));
  const double transformed_pp = dot(
      transformed_axis, signed_cubic_transform(positive.wave_velocity));
  const double transformed_pm = dot(
      transformed_axis, signed_cubic_transform(negative.wave_velocity));
  const double transformed_wedge = transformed_qp * transformed_pm
      - transformed_qm * transformed_pp;
  const double cubic_residual = transformed_wedge - result.phase_wedge;

  result.memory_radius_squared =
      result.positive_coordinate * result.positive_coordinate
      + result.negative_coordinate * result.negative_coordinate;
  if (!std::isfinite(result.memory_radius_squared)
      || result.memory_radius_squared <= parameters.tolerance) {
    result.status = NativeOrientationMemoryStatus::ZeroPhaseWedge;
    return result;
  }
  result.memory_energy = (
      result.positive_momentum * result.positive_momentum
      + result.negative_momentum * result.negative_momentum)
      / (2.0 * parameters.memory_mass)
      + parameters.memory_quartic_coupling
          * result.memory_radius_squared * result.memory_radius_squared;
  const double positive_qdot =
      result.positive_momentum / parameters.memory_mass;
  const double negative_qdot =
      result.negative_momentum / parameters.memory_mass;
  const double positive_pdot = -4.0 * parameters.memory_quartic_coupling
      * result.memory_radius_squared * result.positive_coordinate;
  const double negative_pdot = -4.0 * parameters.memory_quartic_coupling
      * result.memory_radius_squared * result.negative_coordinate;
  result.phase_wedge_derivative_residual =
      positive_qdot * result.negative_momentum
      + result.positive_coordinate * negative_pdot
      - negative_qdot * result.positive_momentum
      - result.negative_coordinate * positive_pdot;

  const double wedge_squared = result.phase_wedge * result.phase_wedge;
  result.radial_minimum = std::pow(
      wedge_squared / (4.0 * parameters.memory_mass
          * parameters.memory_quartic_coupling),
      1.0 / 6.0);
  result.radial_minimum_equation_residual =
      4.0 * parameters.memory_mass
          * parameters.memory_quartic_coupling
          * std::pow(result.radial_minimum, 6.0)
      - wedge_squared;
  result.radial_minimum_curvature = 24.0
      * parameters.memory_quartic_coupling
      * result.radial_minimum * result.radial_minimum;
  result.centrifugal_term_at_current_radius = wedge_squared
      / (2.0 * parameters.memory_mass * result.memory_radius_squared);

  result.swept_area_probe = advance_native_pair_energy(
      parameters.swept_area_probe_state,
      parameters.swept_area_probe_parameters);
  if (!result.swept_area_probe.valid) {
    result.status = NativeOrientationMemoryStatus::PairProbeFailure;
    return result;
  }
  const auto& probe_before = result.swept_area_probe.before;
  const auto& probe_after = result.swept_area_probe.after;
  const double reversed_area = swept_area(
      probe_after.coordinate, -probe_after.momentum,
      probe_before.coordinate, -probe_before.momentum);
  result.swept_area_full_time_reversal_residual =
      reversed_area - result.swept_area_probe.swept_area;

  const double accepted = 256.0 * parameters.tolerance * std::max({
      1.0, result.dipole_norm, std::abs(result.phase_wedge),
      std::abs(result.gram_determinant), std::abs(result.memory_energy)});
  result.neutral_dipole_axis_conditional_exact = true;
  result.origin_independence_exact =
      max_abs(result.origin_independence_residual) <= accepted;
  result.minimum_nonzero_neutral_body_is_plus_minus_pair = true;
  result.signed_cubic_covariance_exact = std::abs(cubic_residual) <= accepted;
  result.inversion_reverses_axis_exact = true;
  result.dipole_symmetric_square_loses_sign = true;
  result.projected_modes_spatial_scalars = true;
  result.phase_wedge_spatial_scalar = result.signed_cubic_covariance_exact;
  result.phase_wedge_time_odd =
      std::abs(result.time_reversal_residual) <= accepted;
  result.symmetric_gram_loses_wedge_sign =
      std::abs(result.gram_wedge_square_residual) <= accepted;
  result.one_step_swept_area_time_odd_memory = false;
  result.central_quartic_memory_imposed = true;
  result.central_memory_conserves_phase_wedge =
      std::abs(result.phase_wedge_derivative_residual) <= accepted;
  result.nonzero_wedge_bounded_recursive_memory =
      result.radial_minimum > 0.0
      && result.radial_minimum_curvature > 0.0
      && std::abs(result.radial_minimum_equation_residual) <= accepted;
  result.same_mode_nonzero_wedge_retains_pure_gstar_clock = false;
  result.separate_clock_and_chirality_memory_minimum = true;

  if (!std::isfinite(result.memory_energy)
      || !std::isfinite(result.radial_minimum)
      || !std::isfinite(result.radial_minimum_curvature)
      || !std::isfinite(result.centrifugal_term_at_current_radius)
      || !result.origin_independence_exact
      || std::abs(result.axis_norm_residual) > accepted
      || !result.signed_cubic_covariance_exact
      || !result.phase_wedge_time_odd
      || !result.symmetric_gram_loses_wedge_sign
      || !result.central_memory_conserves_phase_wedge
      || !result.nonzero_wedge_bounded_recursive_memory
      || std::abs(result.swept_area_full_time_reversal_residual) > accepted) {
    result.status = NativeOrientationMemoryStatus::InvariantFailure;
    return result;
  }

  result.status = NativeOrientationMemoryStatus::Valid;
  return result;
}

}  // namespace ftd::eft
