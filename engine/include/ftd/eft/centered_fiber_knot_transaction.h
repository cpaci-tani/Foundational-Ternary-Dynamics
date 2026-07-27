#pragma once
/**
 * @file centered_fiber_knot_transaction.h
 * @brief Unique centered knot-to-subcell transaction with an explicit
 *        dressing-history fiber (FTD-0496).
 *
 * Observer only.  This is a selected nonholonomic extension and does not
 * modify RenderBridge or Voxel.
 */

#include "ftd/eft/dressing_fiber_ledger.h"

namespace ftd::eft {

struct CenteredFiberKnotInput {
  MatchedFaceFlux electric_before;
  Coord site{};
  Vec3 momentum_before{};
  double dressing_before = 0.0;
  int charge = 0;
  double coupling = 1.0;
  double dt = 1.0;
  double rest_energy = 0.511;
  double causal_speed = 0.57735026918962576451;
  Vec3 initial_guess{};
  bool use_initial_guess = false;
  int max_iterations = 512;
  double solver_tolerance = 1e-15;
};

struct CenteredFiberKnotStep {
  bool valid = false;
  bool converged = false;
  bool uniqueness_certified = false;
  int iterations = 0;
  Coord site{};
  int charge = 0;
  double coupling = 0.0;
  double dt = 0.0;
  double rest_energy = 0.0;
  double causal_speed = 0.0;
  double contraction_bound = 0.0;

  Vec3 momentum_before{};
  Vec3 momentum_after{};
  Vec3 displacement{};
  Vec3 centered_field_before{};
  Vec3 centered_field_midpoint{};
  Vec3 centered_current_trace{};
  Vec3 predicted_centered_current_trace{};

  FaceCurrentSegment current;
  MatchedFaceFlux electric_before;
  MatchedFaceFlux electric_midpoint;
  MatchedFaceFlux electric_after;
  double dressing_before = 0.0;
  double dressing_after = 0.0;

  double fixed_point_residual = 0.0;
  double centered_current_trace_residual = 0.0;
  double midpoint_field_residual = 0.0;
  double impulse_residual = 0.0;
  double displacement_residual = 0.0;
  double matter_work_residual = 0.0;
  double field_work = 0.0;
  double centered_work = 0.0;
  double dressing_change = 0.0;
  double total_energy_residual = 0.0;
  double continuity_residual = 0.0;
  double relative_gauss_residual = 0.0;
  double locality_residual = 0.0;
  double speed = 0.0;
  double causal_excess = 0.0;
  double inverse_residual = 0.0;
};

Vec3 predict_centered_knot_current_trace(
    const Vec3& displacement,
    int charge);

double centered_knot_contraction_bound(
    double coupling,
    double dt,
    double rest_energy,
    double causal_speed);

CenteredFiberKnotStep solve_centered_fiber_knot_step(
    const CenteredFiberKnotInput& input);

}  // namespace ftd::eft
