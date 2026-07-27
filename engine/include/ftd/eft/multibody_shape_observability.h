#pragma once
/**
 * @file multibody_shape_observability.h
 * @brief Additive trilinear-shape and exact face-current observability for
 *        multiple worldline segments (FTD-0501).
 */

#include "ftd/eft/face_current_segment.h"

#include <vector>

namespace ftd::eft {

struct ShapeWorldline {
  Vec3 start_position{};
  Vec3 end_position{};
  int charge = 0;
};

struct OneDimensionalCICMoment {
  int signed_charge = 0;
  double signed_first_moment = 0.0;
  double lower_weight = 0.0;
  double upper_weight = 0.0;
  bool valid = false;
};

/// Exact 1D CIC factorization rho=(Q-M,M) for fractional positions in [0,1].
OneDimensionalCICMoment one_dimensional_cic_moment(
    const std::vector<double>& fractions,
    const std::vector<int>& charges);

struct AggregateShapeCurrent {
  int L = 0;
  int particle_count = 0;
  int total_charge = 0;
  Vec3 signed_first_moment_before{};
  Vec3 signed_first_moment_after{};
  Vec3 unsigned_center_before{};
  Vec3 unsigned_center_after{};
  std::vector<double> rho_before;
  std::vector<double> rho_after;
  std::vector<double> current_x;
  std::vector<double> current_y;
  std::vector<double> current_z;
  double aggregate_continuity_residual = 0.0;
  double aggregate_current_l1 = 0.0;
  double constituent_current_l1 = 0.0;
  bool valid = false;

  int index(int x, int y, int z) const;
};

/// Sum individually exact FTD-0478/0484 shape-current segments. Worldline
/// ordering is used only to supply each start/end pairing; it is not retained
/// by the aggregate record.
AggregateShapeCurrent make_aggregate_shape_current(
    int L,
    const std::vector<ShapeWorldline>& worldlines);

/// Exact squared separation for a two-body configuration; NaN otherwise.
double two_body_squared_separation(
    const std::vector<ShapeWorldline>& worldlines,
    bool use_end_positions = false);

}  // namespace ftd::eft
