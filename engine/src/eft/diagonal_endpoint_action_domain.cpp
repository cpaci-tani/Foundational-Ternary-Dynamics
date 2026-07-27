#include "ftd/eft/diagonal_endpoint_action_domain.h"

#include "ftd/constants.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace ftd::eft {
namespace {

double component(const Vec3& value, int axis) {
  if (axis == 0) return value.x;
  if (axis == 1) return value.y;
  return value.z;
}

Vec3 position(const ContactCarrierRecord& carrier) {
  return {static_cast<double>(carrier.anchor.x)+carrier.remainder.x,
          static_cast<double>(carrier.anchor.y)+carrier.remainder.y,
          static_cast<double>(carrier.anchor.z)+carrier.remainder.z};
}

struct CrossingData {
  int count = 0;
  double minimum_tau = INFINITY;
  double maximum_tau = -INFINITY;
  double simultaneous_residual = 0.0;
  double minimum_overshoot = INFINITY;
};

CrossingData crossings(const Vec3& start, const Vec3& end) {
  CrossingData result;
  std::array<double, 3> tau{};
  int tau_count = 0;
  constexpr double open_tolerance = 64.0
      * std::numeric_limits<double>::epsilon();
  for (int axis = 0; axis < 3; ++axis) {
    const double a = component(start, axis);
    const double b = component(end, axis);
    if (std::abs(b-a) <= open_tolerance) continue;
    double plane = 0.0;
    bool crossed = false;
    double overshoot = 0.0;
    if (b > a) {
      plane = std::floor(a)+1.0;
      if (std::abs(plane-a) <= open_tolerance) plane += 1.0;
      crossed = plane < b-open_tolerance;
      overshoot = b-plane;
    } else {
      plane = std::ceil(a)-1.0;
      if (std::abs(plane-a) <= open_tolerance) plane -= 1.0;
      crossed = plane > b+open_tolerance;
      overshoot = plane-b;
    }
    if (!crossed) continue;
    const double value = (plane-a)/(b-a);
    tau[static_cast<std::size_t>(tau_count++)] = value;
    ++result.count;
    result.minimum_tau = std::min(result.minimum_tau, value);
    result.maximum_tau = std::max(result.maximum_tau, value);
    result.minimum_overshoot = std::min(
        result.minimum_overshoot, overshoot);
  }
  for (int i = 0; i < tau_count; ++i) {
    for (int j = i+1; j < tau_count; ++j) {
      result.simultaneous_residual = std::max(
          result.simultaneous_residual,
          std::abs(tau[static_cast<std::size_t>(i)]
                   - tau[static_cast<std::size_t>(j)]));
    }
  }
  if (result.count == 0) {
    result.minimum_tau = 0.0;
    result.maximum_tau = 0.0;
    result.minimum_overshoot = 0.0;
  }
  return result;
}

/** A nonzero point immediately before start, guaranteed to stay in the
 * closed unit cell containing start for the registered non-boundary starts. */
Vec3 interior_previous_point(const Vec3& start, const Vec3& unit) {
  double maximum_step = INFINITY;
  for (int axis = 0; axis < 3; ++axis) {
    const double x = component(start, axis);
    const double direction = -component(unit, axis);
    if (std::abs(direction) <= 1e-15) continue;
    const double lower = std::floor(x);
    const double distance = direction > 0.0
        ? (lower+1.0-x)/direction
        : (x-lower)/(-direction);
    if (distance > 1e-14)
      maximum_step = std::min(maximum_step, distance);
  }
  if (!std::isfinite(maximum_step) || !(maximum_step > 0.0)) return start;
  const double step = 0.25*maximum_step;
  return start-unit*step;
}

double coupled_identity_residual(
    const SymmetricDiagonalCoupledEndpointResult& value) {
  return std::max({value.root_residual,
      value.continuity_residual,
      value.gauss_before_residual,
      value.gauss_after_residual,
      value.staggered_embedding_residual,
      value.field_work_residual,
      value.matter_work_residual,
      value.total_energy_residual,
      value.displacement_residual,
      value.causal_excess,
      value.inverse_residual});
}

}  // namespace

DiagonalEndpointActionDomainResult
analyze_diagonal_endpoint_action_domain(
    int L, const Vec3& contact_position, Coord diagonal_direction,
    int polarity, double speed, double tolerance) {
  DiagonalEndpointActionDomainResult result;
  result.shell = diagonal_direction.x*diagonal_direction.x
      + diagonal_direction.y*diagonal_direction.y
      + diagonal_direction.z*diagonal_direction.z;
  result.coupled = solve_symmetric_diagonal_coupled_endpoint(
      L, contact_position, diagonal_direction, polarity, speed, tolerance);
  result.coupled_endpoint_valid = result.coupled.valid;
  if ((result.shell != 2 && result.shell != 3)
      || !result.coupled.valid || !std::isfinite(tolerance)
      || tolerance < 0.0) return result;

  result.minimum_crossed_planes = 4;
  result.maximum_crossed_planes = 0;
  result.minimum_crossing_parameter = INFINITY;
  result.maximum_crossing_parameter = -INFINITY;
  result.minimum_endpoint_overshoot = INFINITY;
  result.minimum_reference_endpoint_overshoot = INFINITY;
  result.minimum_previous_segment_displacement = INFINITY;
  result.endpoint_shift = result.coupled.endpoint_change;
  result.coupled_identity_residual = coupled_identity_residual(
      result.coupled);

  DualGaugePotentialSlab previous_zero(L, C_SPEED);
  DualGaugePotentialSlab next_zero(L, C_SPEED);
  bool reference_preserved = true;
  bool previous_interior = true;
  for (int carrier_index = 0; carrier_index < 2; ++carrier_index) {
    const auto& carrier = result.coupled.rebase.bounce_preimage.carrier[
        static_cast<std::size_t>(carrier_index)];
    const Vec3 start = position(carrier);
    const Vec3 unit = carrier.velocity*(1.0/speed);
    const Vec3 end = start+unit*result.coupled.displacement_magnitude;
    const Vec3 reference_end = start
        +unit*result.coupled.reference_displacement_magnitude;
    const CrossingData coupled_crossings = crossings(start, end);
    const CrossingData reference_crossings = crossings(start, reference_end);
    result.minimum_crossed_planes = std::min(
        result.minimum_crossed_planes, coupled_crossings.count);
    result.maximum_crossed_planes = std::max(
        result.maximum_crossed_planes, coupled_crossings.count);
    result.minimum_crossing_parameter = std::min(
        result.minimum_crossing_parameter, coupled_crossings.minimum_tau);
    result.maximum_crossing_parameter = std::max(
        result.maximum_crossing_parameter, coupled_crossings.maximum_tau);
    result.simultaneous_crossing_residual = std::max(
        result.simultaneous_crossing_residual,
        coupled_crossings.simultaneous_residual);
    result.minimum_endpoint_overshoot = std::min(
        result.minimum_endpoint_overshoot,
        coupled_crossings.minimum_overshoot);
    result.minimum_reference_endpoint_overshoot = std::min(
        result.minimum_reference_endpoint_overshoot,
        reference_crossings.minimum_overshoot);
    reference_preserved = reference_preserved
        && reference_crossings.count == result.shell
        && coupled_crossings.count == reference_crossings.count;

    const Vec3 previous = interior_previous_point(start, unit);
    const double previous_displacement = (start-previous).mag();
    result.minimum_previous_segment_displacement = std::min(
        result.minimum_previous_segment_displacement,
        previous_displacement);
    result.maximum_previous_segment_displacement = std::max(
        result.maximum_previous_segment_displacement,
        previous_displacement);
    result.previous_segment_causal_excess = std::max(
        result.previous_segment_causal_excess,
        std::max(0.0, previous_displacement-C_SPEED));
    previous_interior = previous_interior
        && previous_displacement > 1e-12
        && previous_displacement <= C_SPEED+tolerance;
    const auto previous_control = evaluate_two_slab_variational_force(
        previous, start, start, polarity,
        previous_zero, next_zero, 1.0);
    if (previous_control.valid) ++result.accepted_previous_segment_controls;
    const auto action = evaluate_two_slab_variational_force(
        previous, start, end, polarity,
        previous_zero, next_zero, 1.0);
    if (!action.valid) ++result.rejected_carriers;
  }

  result.reference_crossings_preserved = reference_preserved;
  result.previous_segments_are_nonzero_and_interior = previous_interior
      && result.accepted_previous_segment_controls == 2;
  result.zero_connection_rejected = result.rejected_carriers == 2;
  const bool expected_crossings =
      result.minimum_crossed_planes == result.shell
      && result.maximum_crossed_planes == result.shell;
  result.valid = result.coupled_endpoint_valid
      && expected_crossings
      && result.minimum_crossing_parameter > 0.0
      && result.maximum_crossing_parameter < 1.0
      && result.simultaneous_crossing_residual <= tolerance
      && result.minimum_endpoint_overshoot > 1e-8
      && result.reference_crossings_preserved
      && result.previous_segments_are_nonzero_and_interior
      && result.zero_connection_rejected
      && result.coupled_identity_residual <= 1e-10;
  return result;
}

}  // namespace ftd::eft
