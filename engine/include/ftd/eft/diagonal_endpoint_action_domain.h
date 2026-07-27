#pragma once
/**
 * @file diagonal_endpoint_action_domain.h
 * @brief Composition audit for the coupled diagonal endpoint and compact
 *        one-cell worldline action (FTD-0532).
 */

#include "ftd/eft/symmetric_diagonal_coupled_endpoint.h"
#include "ftd/eft/two_slab_variational_force.h"

namespace ftd::eft {

struct DiagonalEndpointActionDomainResult {
  bool valid = false;
  bool coupled_endpoint_valid = false;
  bool reference_crossings_preserved = false;
  bool previous_segments_are_nonzero_and_interior = false;
  bool zero_connection_rejected = false;
  int shell = 0;
  int minimum_crossed_planes = 0;
  int maximum_crossed_planes = 0;
  int accepted_previous_segment_controls = 0;
  int rejected_carriers = 0;
  double minimum_crossing_parameter = 0.0;
  double maximum_crossing_parameter = 0.0;
  double simultaneous_crossing_residual = 0.0;
  double minimum_endpoint_overshoot = 0.0;
  double minimum_reference_endpoint_overshoot = 0.0;
  double endpoint_shift = 0.0;
  double minimum_previous_segment_displacement = 0.0;
  double maximum_previous_segment_displacement = 0.0;
  double previous_segment_causal_excess = 0.0;
  double coupled_identity_residual = 0.0;
  SymmetricDiagonalCoupledEndpointResult coupled{};
};

/**
 * Reconstruct the two FTD-0531 worldlines and classify their simultaneous
 * coordinate-plane crossings.  A zero auxiliary connection is then supplied
 * to FTD-0485 so that any invalid result is a pure domain rejection.
 */
DiagonalEndpointActionDomainResult
analyze_diagonal_endpoint_action_domain(
    int L,
    const Vec3& contact_position,
    Coord diagonal_direction,
    int polarity,
    double speed,
    double tolerance = 1e-12);

}  // namespace ftd::eft
