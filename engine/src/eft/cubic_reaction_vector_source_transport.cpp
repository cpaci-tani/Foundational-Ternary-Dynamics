#include "ftd/eft/cubic_reaction_vector_source_transport.h"

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

Vec3 effective_position(const Coord& anchor, const Vec3& remainder) {
  return {static_cast<double>(anchor.x) + remainder.x,
          static_cast<double>(anchor.y) + remainder.y,
          static_cast<double>(anchor.z) + remainder.z};
}

Vec3 parallel_component(const Vec3& value, const Vec3& axis,
                        double axis_norm2) {
  if (!(axis_norm2 > 0.0)) return {};
  return axis * (value.dot(axis) / axis_norm2);
}

}  // namespace

CubicReactionSourceTransportResult
analyze_cubic_reaction_vector_source_transport(
    const CubicReactionSourceTransportInput& input) {
  CubicReactionSourceTransportResult result;
  result.reaction_coordinate = input.reaction_coordinate;
  result.required_matter_impulse = input.required_matter_impulse;

  if (!finite(input.reaction_coordinate)
      || !finite(input.required_matter_impulse)
      || !finite(input.source_remainder)
      || !finite(input.residual_amplitude)
      || !finite(input.rest_energy)
      || !finite(input.limiting_speed)
      || !finite(input.dt)
      || !finite(input.tolerance)) {
    return result;
  }
  if (!(input.rest_energy > 0.0)) {
    result.status = CubicReactionSourceTransportStatus::InvalidRestEnergy;
    return result;
  }
  if (!(input.limiting_speed > 0.0)) {
    result.status = CubicReactionSourceTransportStatus::InvalidSpeed;
    return result;
  }
  if (!(input.dt > 0.0) || input.dt > 1.0) {
    result.status = CubicReactionSourceTransportStatus::InvalidTimeStep;
    return result;
  }
  if (input.lattice_size < 4) {
    result.status = CubicReactionSourceTransportStatus::InvalidLatticeSize;
    return result;
  }
  if (input.charge != -1 && input.charge != 1) {
    result.status = CubicReactionSourceTransportStatus::InvalidCharge;
    return result;
  }

  const double c2 = input.limiting_speed * input.limiting_speed;
  const double p2 = input.required_matter_impulse.mag2();
  const double total_energy = std::sqrt(
      input.rest_energy * input.rest_energy + c2 * p2);
  result.required_kinetic_energy = total_energy - input.rest_energy;
  result.residual_energy = 0.5 * input.residual_amplitude
      * input.residual_amplitude;
  result.low_energy_inertial_mass = input.rest_energy / c2;

  const double energy_scale = std::max(
      {1.0, result.residual_energy, result.required_kinetic_energy});
  if (result.required_kinetic_energy
      > result.residual_energy + input.tolerance * energy_scale) {
    result.status =
        CubicReactionSourceTransportStatus::InsufficientResidualEnergy;
    return result;
  }

  result.reaction_energy = std::max(0.0,
      result.required_kinetic_energy);
  result.history_energy = std::max(0.0,
      result.residual_energy - result.reaction_energy);
  if (result.residual_energy > input.tolerance) {
    const double ratio = std::clamp(
        result.reaction_energy / result.residual_energy, 0.0, 1.0);
    result.split_angle = std::asin(std::sqrt(ratio));
  }
  result.equal_split = std::abs(
      result.reaction_energy - result.history_energy)
      <= input.tolerance * energy_scale;
  result.split_angle_fixed_by_local_conservation = true;

  result.reaction_radius = std::sqrt(2.0 * result.reaction_energy);
  const double p_norm = std::sqrt(p2);
  if (p_norm > input.tolerance) {
    result.reaction_momentum = input.required_matter_impulse
        * (result.reaction_radius / p_norm);
    result.orientation_defined_by_field_impulse = true;
  }

  const double rho2 = result.reaction_momentum.mag2();
  const double root = std::sqrt(input.rest_energy + 0.25 * rho2);
  result.tangential_jacobian_eigenvalue =
      root / input.limiting_speed;
  result.radial_jacobian_eigenvalue =
      (input.rest_energy + 0.5 * rho2)
      / (input.limiting_speed * root);
  result.jacobian_determinant =
      result.tangential_jacobian_eigenvalue
      * result.tangential_jacobian_eigenvalue
      * result.radial_jacobian_eigenvalue;

  result.physical_momentum = result.reaction_momentum
      * result.tangential_jacobian_eigenvalue;

  if (rho2 > input.tolerance * input.tolerance) {
    const Vec3 reaction_parallel = parallel_component(
        input.reaction_coordinate, result.reaction_momentum, rho2);
    const Vec3 reaction_transverse = input.reaction_coordinate
        - reaction_parallel;
    result.physical_coordinate = reaction_transverse
            * (1.0 / result.tangential_jacobian_eigenvalue)
        + reaction_parallel * (1.0 / result.radial_jacobian_eigenvalue);

    const Vec3 physical_parallel = parallel_component(
        result.physical_coordinate, result.reaction_momentum, rho2);
    const Vec3 physical_transverse = result.physical_coordinate
        - physical_parallel;
    result.recovered_reaction_coordinate = physical_transverse
            * result.tangential_jacobian_eigenvalue
        + physical_parallel * result.radial_jacobian_eigenvalue;
  } else {
    result.physical_coordinate = input.reaction_coordinate
        * (1.0 / result.tangential_jacobian_eigenvalue);
    result.recovered_reaction_coordinate = result.physical_coordinate
        * result.tangential_jacobian_eigenvalue;
  }

  const double physical_p_norm = result.physical_momentum.mag();
  const double chart_total_energy = std::sqrt(
      input.rest_energy * input.rest_energy
      + c2 * result.physical_momentum.mag2());
  const double chart_kinetic_energy =
      chart_total_energy - input.rest_energy;
  if (physical_p_norm > input.tolerance) {
    const double recovered_radius = std::sqrt(
        2.0 * std::max(0.0, chart_kinetic_energy));
    result.recovered_reaction_momentum = result.physical_momentum
        * (recovered_radius / physical_p_norm);
  }

  result.energy_chart_residual = std::abs(
      chart_kinetic_energy - 0.5 * rho2);
  result.reaction_inverse_residual = max_abs(
      result.recovered_reaction_momentum - result.reaction_momentum);
  result.coordinate_inverse_residual = max_abs(
      result.recovered_reaction_coordinate - input.reaction_coordinate);
  result.split_amplitude_residual = std::abs(
      result.reaction_radius
      - std::abs(input.residual_amplitude) * std::sin(result.split_angle));
  result.exact_relativistic_energy_chart =
      result.energy_chart_residual <= input.tolerance * energy_scale
      && result.reaction_inverse_residual <= input.tolerance
      && result.coordinate_inverse_residual <= input.tolerance;

  result.physical_velocity = result.physical_momentum
      * (c2 / chart_total_energy);
  const Vec3 base_position = effective_position(
      input.source_anchor, input.source_remainder);
  result.source_before = centered_canonical_subcell_chart(
      base_position + result.physical_coordinate);
  if (!result.source_before.valid) {
    result.status = CubicReactionSourceTransportStatus::InvalidSubcellChart;
    return result;
  }
  const Vec3 drift = result.physical_velocity * input.dt;
  result.source_after = translate_centered_canonical_chart(
      result.source_before, drift);
  if (!result.source_after.valid) {
    result.status = CubicReactionSourceTransportStatus::InvalidSubcellChart;
    return result;
  }
  const Vec3 before_position = subcell_chart_position(result.source_before);
  const Vec3 after_position = subcell_chart_position(result.source_after);
  result.drift_inverse_residual = max_abs(
      after_position - drift - before_position);
  result.exact_reversible_free_transport =
      result.drift_inverse_residual <= input.tolerance
      && result.physical_velocity.mag()
          < input.limiting_speed + input.tolerance;

  result.current_segment = make_face_current_segment(
      input.lattice_size,
      result.source_before.anchor, result.source_before.remainder,
      result.source_after.anchor, result.source_after.remainder,
      input.charge);
  result.current_continuity_residual =
      result.current_segment.continuity_residual;
  result.exact_face_current_continuity = result.current_segment.valid
      && result.current_continuity_residual <= input.tolerance;
  if (!result.exact_face_current_continuity) {
    result.status =
        CubicReactionSourceTransportStatus::CurrentContinuityFailure;
    return result;
  }

  const double impulse_residual = max_abs(
      result.physical_momentum - input.required_matter_impulse);
  const bool finite_output = finite(result.physical_coordinate)
      && finite(result.physical_momentum)
      && finite(result.physical_velocity)
      && finite(result.split_angle)
      && finite(result.jacobian_determinant)
      && finite(result.drift_inverse_residual);
  if (finite_output && result.exact_relativistic_energy_chart
      && result.exact_reversible_free_transport
      && result.exact_face_current_continuity
      && result.split_amplitude_residual <= input.tolerance * energy_scale
      && impulse_residual <= input.tolerance) {
    result.status = CubicReactionSourceTransportStatus::Valid;
  }
  return result;
}

}  // namespace ftd::eft
