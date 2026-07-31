#pragma once
/**
 * @file spline_poynting_momentum.h
 * @brief Observer-only B-spline Poynting momentum candidate (FTD-0619).
 *
 * The candidate uses the already selected FTD-0550 face/edge
 * reconstructions. It does not alter the matched fields or production tick.
 */

#include "ftd/eft/quadratic_coat_orbit_gather.h"

namespace ftd::eft {

struct SplinePoyntingMomentumResult {
  bool valid = false;
  double beta = 0.0;
  double wave_speed = 0.0;
  double dt = 0.0;
  Vec3 integrated_cross{};
  Vec3 momentum{};
};

/// Reconstruct B at the integer time represented by (E_n,B_{n-1/2}).
MatchedEdgeField matched_integer_time_magnetic(
    const MatchedFaceFlux& electric,
    const MatchedEdgeField& magnetic_half,
    double wave_speed,
    double dt = 1.0);

/// Exact periodic-cell integral of the locked FTD-0550 spline fields.
Vec3 integrate_quadratic_spline_cross(
    const MatchedFaceFlux& electric,
    const MatchedEdgeField& magnetic_integer);

/// P=(beta/c) integral E_h cross B_h, with no fitted normalization.
SplinePoyntingMomentumResult measure_spline_poynting_momentum(
    const MatchedFaceFlux& electric,
    const MatchedEdgeField& magnetic_half,
    double wave_speed,
    double dt,
    double beta);

}  // namespace ftd::eft

