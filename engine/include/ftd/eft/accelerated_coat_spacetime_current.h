#pragma once
/**
 * @file accelerated_coat_spacetime_current.h
 * @brief Nonuniform-time quadratic-coat current deposits (FTD-0548).
 */

#include "ftd/eft/accelerated_worldline_energy.h"
#include "ftd/eft/quadratic_coat_spacetime_action.h"

#include <vector>

namespace ftd::eft {

struct AcceleratedCoatSpacetimeCurrent {
  bool valid = false;
  int L = 0;
  int charge = 0;
  double temporal_scale = 0.0;
  Vec3 start_position{};
  Vec3 end_position{};
  AcceleratedWorldlineEnergyResult trajectory{};
  QuadraticCoatFaceCurrent spatial{};
  MatchedFaceFlux spatial_quadrature{};
  MatchedFaceFlux spatial_start{};
  MatchedFaceFlux spatial_end{};
  std::vector<double> temporal_charge;

  int quadrature_pieces = 0;
  double total_current_residual = 0.0;
  double split_recombination_residual = 0.0;
  double temporal_partition_residual = 0.0;
  double split_continuity_start_residual = 0.0;
  double split_continuity_end_residual = 0.0;
  double linear_start_difference = 0.0;
  double linear_end_difference = 0.0;
  double linear_temporal_difference = 0.0;

  explicit AcceleratedCoatSpacetimeCurrent(int size = 0);
  int index(int x, int y, int z) const;
};

AcceleratedCoatSpacetimeCurrent make_accelerated_coat_spacetime_current(
    int L,
    const Vec3& start_position,
    double rest_energy,
    double c_speed,
    double temporal_scale,
    double midpoint_momentum,
    double half_impulse,
    const Vec3& direction,
    int charge);

double accelerated_coat_gauge_endpoint_residual(
    const AcceleratedCoatSpacetimeCurrent& current,
    const std::vector<double>& chi_start,
    const std::vector<double>& chi_end);

}  // namespace ftd::eft
