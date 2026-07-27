#pragma once
/**
 * @file open_worldline_hop_selector.h
 * @brief Observer for the gauge status of finite open-hop action comparisons.
 *
 * FTD-0489 distinguishes a valid fixed-history gauge coupling from an invalid
 * attempt to use open charged-worldline action values as endpoint costs.
 */

#include "ftd/eft/spacetime_worldline_coupling.h"

#include <array>

namespace ftd::eft {

struct OpenWorldlineHopCandidate {
  bool valid = false;
  Coord end_anchor{};
  Vec3 end_remainder{};
  double matter_cost = 0.0;
  double interaction_action = 0.0;
  double transformed_interaction_action = 0.0;
  double total_action = 0.0;
  double transformed_total_action = 0.0;
  double endpoint_shift = 0.0;
  double gauge_endpoint_residual = 0.0;
  double electric_invariance_residual = 0.0;
  double magnetic_invariance_residual = 0.0;
};

struct OpenWorldlineHopComparison {
  bool valid = false;
  OpenWorldlineHopCandidate first{};
  OpenWorldlineHopCandidate second{};
  double action_difference = 0.0;
  double transformed_action_difference = 0.0;
  double predicted_transformed_difference = 0.0;
  double difference_shift_residual = 0.0;
};

struct CubicHopOrbitSummary {
  int face_count = 0;
  int edge_count = 0;
  int corner_count = 0;
  int nonzero_reflection_fixed_count = 0;
  Vec3 orbit_sum{};
};

/** Compare two fixed open histories under one gauge transformation.
 *
 * Matter costs are required to be finite and gauge invariant. The observer
 * intentionally does not choose a candidate.
 */
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
    double coupling = 1.0);

/// Exact 26-neighbour orbit and reflection-fixed-point census.
CubicHopOrbitSummary summarize_cubic_moore_hops();

/// Production-style endpoint thresholding, isolated from mutation/collisions.
Coord threshold_moore_displacement(const Vec3& remainder_before,
                                   const Vec3& velocity,
                                   double dt);

}  // namespace ftd::eft
