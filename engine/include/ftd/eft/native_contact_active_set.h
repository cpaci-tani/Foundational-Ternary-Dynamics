#pragma once
/**
 * @file native_contact_active_set.h
 * @brief Geometry observer for frozen production vs selected hard contact
 *        (FTD-0525).
 */

#include "ftd/eft/boundary_chart_capacity.h"

namespace ftd::eft {

struct NativeContactActiveSetGeometry {
  bool valid = false;
  int L = 0;
  int polarity = 0;
  Coord chart_direction{};
  Vec3 collision_position{};
  Vec3 normal{};
  double speed = 0.0;
  double offset = 0.0;
  Coord first_anchor{};
  Coord second_anchor{};
  Vec3 first_separated_remainder{};
  Vec3 second_separated_remainder{};
  Vec3 first_contact_remainder{};
  Vec3 second_contact_remainder{};
  Vec3 first_crossed_remainder{};
  Vec3 second_crossed_remainder{};
  double separated_gap = 0.0;
  double contact_gap = 0.0;
  double crossed_gap = 0.0;
  double expected_separated_gap = 0.0;
  double expected_crossed_gap = 0.0;
  double gap_residual = 0.0;
  double stable_chart_residual = 0.0;
  double contact_hop_margin = 0.0;
  double crossed_hop_margin = 0.0;
  double exact_hop_delay = 0.0;
  int predicted_hop_delay_ticks = 0;
  int minimum_missing_charge = 0;
  bool same_site_occupancy = false;
  bool full_phase_distinguishes_gap = false;
  BoundaryChartCollisionResult charts{};
};

/// Construct separated/contact/crossed configurations on the same two stable
/// ternary anchors and compute the later production hop threshold.
NativeContactActiveSetGeometry analyze_native_contact_active_set_geometry(
    int L,
    const Vec3& collision_position,
    Coord chart_direction,
    int polarity,
    double speed,
    double tolerance = 1e-12);

}  // namespace ftd::eft
