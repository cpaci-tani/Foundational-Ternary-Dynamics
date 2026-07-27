#pragma once
/**
 * @file centered_knot_trace.h
 * @brief Unique local linear cubic-average trace at a lattice knot (FTD-0492).
 */

#include "ftd/eft/matched_gauss_transport.h"
#include "ftd/lattice.h"

#include <array>

namespace ftd::eft {

struct CenteredKnotTrace {
  bool valid = false;
  int L = 0;
  Coord site{};
  Vec3 incoming{};
  Vec3 outgoing{};
  Vec3 centered{};
  double divergence = 0.0;
  Vec3 incident_cell_average{};
  double incident_average_residual = 0.0;
  std::array<double, 8> invariant_weights{};
  double weight_sum_residual = 0.0;
};

CenteredKnotTrace evaluate_centered_knot_trace(
    const MatchedFaceFlux& electric,
    Coord site);

}  // namespace ftd::eft
