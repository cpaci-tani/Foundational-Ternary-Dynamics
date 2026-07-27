#include "ftd/eft/open_worldline_hop_selector.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace ftd::eft {
namespace {

OpenWorldlineHopCandidate make_candidate(
    int L,
    Coord start_anchor,
    const Vec3& start_remainder,
    Coord end_anchor,
    const Vec3& end_remainder,
    double matter_cost,
    int charge,
    const DualGaugePotentialSlab& slab,
    const std::vector<double>& chi_start,
    const std::vector<double>& chi_end,
    double coupling) {
  OpenWorldlineHopCandidate candidate;
  candidate.end_anchor = end_anchor;
  candidate.end_remainder = end_remainder;
  candidate.matter_cost = matter_cost;
  if (!std::isfinite(matter_cost)) return candidate;

  const auto current = make_spacetime_worldline_current(
      L, start_anchor, start_remainder,
      end_anchor, end_remainder, charge, slab.temporal_scale);
  const auto coupling_result = evaluate_spacetime_gauge_coupling(
      current, slab, chi_start, chi_end, coupling);
  if (!current.valid || !coupling_result.valid) return candidate;

  candidate.interaction_action = coupling_result.interaction_action;
  candidate.transformed_interaction_action =
      coupling_result.transformed_action;
  candidate.total_action = matter_cost + candidate.interaction_action;
  candidate.transformed_total_action =
      matter_cost + candidate.transformed_interaction_action;
  candidate.endpoint_shift = coupling_result.endpoint_shift;
  candidate.gauge_endpoint_residual =
      coupling_result.gauge_endpoint_residual;
  candidate.electric_invariance_residual =
      coupling_result.electric_invariance_residual;
  candidate.magnetic_invariance_residual =
      coupling_result.magnetic_invariance_residual;
  candidate.valid = std::isfinite(candidate.total_action)
      && std::isfinite(candidate.transformed_total_action)
      && std::isfinite(candidate.endpoint_shift);
  return candidate;
}

int threshold_component(double remainder, double velocity, double dt) {
  const double updated = remainder + velocity * dt;
  if (updated >= 1.0) return +1;
  if (updated <= -1.0) return -1;
  return 0;
}

}  // namespace

OpenWorldlineHopComparison compare_open_worldline_hops(
    int L,
    Coord start_anchor,
    const Vec3& start_remainder,
    Coord first_end_anchor,
    const Vec3& first_end_remainder,
    double first_matter_cost,
    Coord second_end_anchor,
    const Vec3& second_end_remainder,
    double second_matter_cost,
    int charge,
    const DualGaugePotentialSlab& slab,
    const std::vector<double>& chi_start,
    const std::vector<double>& chi_end,
    double coupling) {
  OpenWorldlineHopComparison result;
  result.first = make_candidate(
      L, start_anchor, start_remainder,
      first_end_anchor, first_end_remainder, first_matter_cost,
      charge, slab, chi_start, chi_end, coupling);
  result.second = make_candidate(
      L, start_anchor, start_remainder,
      second_end_anchor, second_end_remainder, second_matter_cost,
      charge, slab, chi_start, chi_end, coupling);
  if (!result.first.valid || !result.second.valid) return result;

  result.action_difference =
      result.first.total_action - result.second.total_action;
  result.transformed_action_difference =
      result.first.transformed_total_action
      - result.second.transformed_total_action;
  result.predicted_transformed_difference = result.action_difference
      + result.first.endpoint_shift - result.second.endpoint_shift;
  result.difference_shift_residual = std::abs(
      result.transformed_action_difference
      - result.predicted_transformed_difference);
  result.valid = std::isfinite(result.action_difference)
      && std::isfinite(result.transformed_action_difference)
      && std::isfinite(result.difference_shift_residual);
  return result;
}

CubicHopOrbitSummary summarize_cubic_moore_hops() {
  CubicHopOrbitSummary result;
  for (int x = -1; x <= 1; ++x) {
    for (int y = -1; y <= 1; ++y) {
      for (int z = -1; z <= 1; ++z) {
        const int squared_length = x * x + y * y + z * z;
        if (squared_length == 0) continue;
        if (squared_length == 1) ++result.face_count;
        if (squared_length == 2) ++result.edge_count;
        if (squared_length == 3) ++result.corner_count;
        result.orbit_sum.x += x;
        result.orbit_sum.y += y;
        result.orbit_sum.z += z;

        // Fixed by all three coordinate reflections iff x=y=z=0.
        const bool fixed_x = x == -x;
        const bool fixed_y = y == -y;
        const bool fixed_z = z == -z;
        if (fixed_x && fixed_y && fixed_z) {
          ++result.nonzero_reflection_fixed_count;
        }
      }
    }
  }
  return result;
}

Coord threshold_moore_displacement(const Vec3& remainder_before,
                                   const Vec3& velocity,
                                   double dt) {
  if (!std::isfinite(remainder_before.x)
      || !std::isfinite(remainder_before.y)
      || !std::isfinite(remainder_before.z)
      || !std::isfinite(velocity.x)
      || !std::isfinite(velocity.y)
      || !std::isfinite(velocity.z)
      || !std::isfinite(dt) || dt < 0.0) {
    return {};
  }
  return {threshold_component(remainder_before.x, velocity.x, dt),
          threshold_component(remainder_before.y, velocity.y, dt),
          threshold_component(remainder_before.z, velocity.z, dt)};
}

}  // namespace ftd::eft
