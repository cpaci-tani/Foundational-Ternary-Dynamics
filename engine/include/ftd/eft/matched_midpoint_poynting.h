#pragma once
/**
 * @file matched_midpoint_poynting.h
 * @brief Exact matched-Maxwell midpoint work identity (FTD-0544).
 */

#include "ftd/eft/quadratic_coat_face_current.h"
#include "ftd/eft/matched_gauss_transport.h"

#include <vector>

namespace ftd::eft {

struct MatchedMidpointPoyntingResult {
  int L = 0;
  double temporal_scale = 0.0;
  MatchedFaceFlux current{};
  MatchedFaceFlux electric_midpoint{};
  MatchedEdgeField magnetic_midpoint{};
  MatchedFaceFlux electric_before{};
  MatchedFaceFlux electric_after{};
  MatchedEdgeField magnetic_before{};
  MatchedEdgeField magnetic_after{};
  std::vector<double> rho_before;
  std::vector<double> rho_after;

  double electric_midpoint_residual = 0.0;
  double magnetic_midpoint_residual = 0.0;
  double ampere_residual = 0.0;
  double faraday_residual = 0.0;
  double adjoint_residual = 0.0;
  double field_energy_before = 0.0;
  double field_energy_after = 0.0;
  double current_work = 0.0;
  double poynting_residual = 0.0;
  double gauss_transport_residual = 0.0;
  bool valid = false;
};

MatchedMidpointPoyntingResult evaluate_matched_midpoint_poynting(
    const MatchedFaceFlux& electric_midpoint,
    const MatchedEdgeField& magnetic_midpoint,
    const QuadraticCoatFaceCurrent& current,
    double temporal_scale);

}  // namespace ftd::eft
