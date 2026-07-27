#pragma once
/**
 * @file centered_trace_work.h
 * @brief Exact work omitted by the centered knot trace (FTD-0493).
 */

#include "ftd/eft/centered_knot_trace.h"
#include "ftd/eft/face_current_segment.h"

namespace ftd::eft {

struct CenteredTraceWorkResult {
  bool valid = false;
  int L = 0;
  int charge = 0;
  Coord site{};
  Vec3 displacement{};
  Vec3 jump{};
  double coupling = 0.0;
  double field_work = 0.0;
  double centered_work = 0.0;
  double omitted_work = 0.0;
  double predicted_cusp_work = 0.0;
  double cusp_formula_residual = 0.0;
  double field_energy_change = 0.0;
  double field_energy_residual = 0.0;
  double relative_gauss_transport_residual = 0.0;
  double continuity_residual = 0.0;
  double reverse_field_work_residual = 0.0;
  double reverse_centered_work_residual = 0.0;
  double reverse_omitted_work_residual = 0.0;
};

CenteredTraceWorkResult evaluate_centered_trace_work(
    const MatchedFaceFlux& midpoint_electric,
    Coord site,
    const Vec3& displacement,
    int charge,
    double coupling = 1.0);

}  // namespace ftd::eft
