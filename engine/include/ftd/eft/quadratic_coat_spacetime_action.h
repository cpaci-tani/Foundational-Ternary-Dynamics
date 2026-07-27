#pragma once
/**
 * @file quadratic_coat_spacetime_action.h
 * @brief Exact spacetime completion of the quadratic coupling coat (FTD-0542).
 *
 * This observer constructs endpoint-weighted face currents and the temporal
 * coat required by a gauge-covariant open-worldline interaction.  It does not
 * update RenderBridge or define a production force.
 */

#include "ftd/eft/quadratic_coat_face_current.h"
#include "ftd/eft/spacetime_worldline_coupling.h"

#include <vector>

namespace ftd::eft {

struct QuadraticCoatSpacetimeCurrent {
  int L = 0;
  int charge = 0;
  double temporal_scale = 0.0;
  QuadraticCoatFaceCurrent spatial{};
  MatchedFaceFlux spatial_start{};
  MatchedFaceFlux spatial_end{};
  std::vector<double> temporal_charge;

  int temporal_support = 0;
  double spatial_split_residual = 0.0;
  double temporal_partition_residual = 0.0;
  double split_continuity_start_residual = 0.0;
  double split_continuity_end_residual = 0.0;
  double locality_residual = 0.0;
  double causal_excess = 0.0;
  bool valid = false;

  int index(int x, int y, int z) const;
};

struct QuadraticCoatGaugeActionResult {
  bool valid = false;
  double coupling = 0.0;
  double interaction_action = 0.0;
  double transformed_action = 0.0;
  double action_shift = 0.0;
  double endpoint_shift = 0.0;
  double gauge_endpoint_residual = 0.0;
  double electric_invariance_residual = 0.0;
  double magnetic_invariance_residual = 0.0;
  double curl_gradient_residual = 0.0;
  MatchedFaceFlux electric{};
  MatchedEdgeField magnetic_start{};
  MatchedEdgeField magnetic_end{};
};

QuadraticCoatSpacetimeCurrent make_quadratic_coat_spacetime_current(
    int L,
    const Vec3& start_effective_position,
    const Vec3& end_effective_position,
    int charge,
    double temporal_scale);

double quadratic_coat_interaction_action(
    const QuadraticCoatSpacetimeCurrent& current,
    const DualGaugePotentialSlab& slab,
    double coupling = 1.0);

QuadraticCoatGaugeActionResult evaluate_quadratic_coat_gauge_action(
    const QuadraticCoatSpacetimeCurrent& current,
    const DualGaugePotentialSlab& slab,
    const std::vector<double>& chi_start,
    const std::vector<double>& chi_end,
    double coupling = 1.0);

}  // namespace ftd::eft
