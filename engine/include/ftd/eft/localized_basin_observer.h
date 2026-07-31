#pragma once
/**
 * @file localized_basin_observer.h
 * @brief Observer-only localized rest-basin metric (FTD-0677).
 *
 * The observer separates three logically different records of a perturbation:
 *
 *   1. internal constituent phase-space distance from a reference rest family;
 *   2. near/intermediate/far positive difference-field norm;
 *   3. collective translation and boost, reported but quotiented from (1).
 *
 * It changes no connected-action state or update rule.  Constituent labels and
 * the binding graph must match.  Cubic covariance is a property under applying
 * the same signed coordinate permutation to both states and the shell origin;
 * the observer does not search over relabellings or orientations.
 */

#include "ftd/eft/connected_moore_block_action.h"

namespace ftd::eft {

struct LocalizedBasinObservation {
  bool valid = false;
  bool topology_match = false;
  bool finite = false;
  int inner_radius = 0;
  int outer_radius = 0;
  int constituent_count = 0;
  double mass = 0.0;
  double reference_frequency = 0.0;
  double field_energy_scale = 0.0;
  double wave_speed = 0.0;

  Vec3 center_offset{};
  Vec3 mean_momentum_offset{};
  double center_offset_norm = 0.0;
  double mean_momentum_offset_norm = 0.0;

  // Translation/boost-quotiented positive internal metrics.
  double internal_position_metric = 0.0;  // m sum_i |dx_i-dx_bar|^2
  double internal_momentum_metric = 0.0;  // (1/m) sum_i |dp_i-dp_bar|^2
  double core_phase_metric = 0.0;         // omega^2 D_x + D_p
  double maximum_internal_position_offset = 0.0;
  double maximum_internal_momentum_offset = 0.0;
  double maximum_edge_length_difference = 0.0;

  // Positive control-relative field self energy, partitioned by storage-cell
  // Chebyshev shells about `origin`.
  double near_dynamic_field = 0.0;
  double intermediate_dynamic_field = 0.0;
  double far_dynamic_field = 0.0;
  double total_dynamic_field = 0.0;
  double field_partition_residual = 0.0;
  double near_fraction = 0.0;
  double far_fraction = 0.0;
};

LocalizedBasinObservation observe_localized_basin(
    const ConnectedMooreBlockState& reference,
    const ConnectedMooreBlockState& candidate,
    const Vec3& origin,
    int inner_radius,
    int outer_radius,
    double reference_frequency,
    double field_energy_scale,
    double wave_speed = C_SPEED,
    double constituent_mass = M_INERTIAL,
    double tolerance = 1e-12);

}  // namespace ftd::eft

