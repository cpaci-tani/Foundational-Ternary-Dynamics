#pragma once
/**
 * @file boundary_chart_capacity.h
 * @brief Stable-chart storage audit for coincident manifested carriers
 *        (FTD-0507).
 *
 * Observer only.  The analysis uses the already selected physical projection
 * x_eff=anchor+remainder and the existing open stable chart interval
 * remainder in (-1,1)^3.  It does not declare the anchor a production gauge
 * label and does not modify RenderBridge.
 */

#include "ftd/eft/subcell_representation_quotient.h"
#include "ftd/eft/ternary_collision_vertex.h"

#include <vector>

namespace ftd::eft {

struct BoundaryChartCapacityResult {
  bool valid = false;
  Vec3 effective_position{};
  int polarity = 0;
  int multiplicity = 0;
  int integer_coordinate_count = 0;
  int expected_chart_count = 0;
  int chart_count = 0;
  int distinct_anchor_count = 0;
  int stored_carriers = 0;
  int minimum_missing_charge = 0;
  int canonical_single_anchor_defect = 0;
  int minimum_per_anchor_occupancy = 0;
  int minimum_chart_aware_alphabet_symbols = 0;
  int canonical_single_anchor_alphabet_symbols = 0;
  double chart_position_residual = 0.0;
  double chart_shape_residual = 0.0;
  double aggregate_shape_residual = 0.0;
  double aggregate_charge_residual = 0.0;
  Vec3 aggregate_first_moment_residual{};
  std::vector<SubcellChart> charts;
};

/// Count exact stable chart capacity and compare all stored chart shapes.
BoundaryChartCapacityResult analyze_boundary_chart_capacity(
    const Vec3& effective_position,
    int multiplicity,
    int polarity,
    double tolerance = 1e-12);

struct BoundaryChartCollisionResult {
  bool valid = false;
  int L = 0;
  int polarity = 0;
  Coord chart_direction{};
  Vec3 unit_direction{};
  Vec3 collision_position{};
  SubcellChart first_chart{};
  SubcellChart second_chart{};
  BoundaryChartCapacityResult capacity{};
  PiecewiseCurrentSignature bounce{};
  PiecewiseCurrentSignature pass_through{};
  double anchor_direction_residual = 0.0;
  double collision_position_residual = 0.0;
  double endpoint_density_residual = 0.0;
  double current_quotient_residual = 0.0;
  double continuity_residual = 0.0;
};

/// Select the two stable charts whose anchors differ by chart_direction and
/// compare identical-particle bounce/pass-through current descriptions.
BoundaryChartCollisionResult analyze_boundary_chart_collision(
    int L,
    const Vec3& collision_position,
    Coord chart_direction,
    int polarity,
    double half_step_distance = 0.25,
    double tolerance = 1e-12);

}  // namespace ftd::eft

