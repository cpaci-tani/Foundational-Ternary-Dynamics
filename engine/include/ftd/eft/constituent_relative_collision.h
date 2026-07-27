#pragma once
/**
 * @file constituent_relative_collision.h
 * @brief Observer-only constituent-relative boundary collision audit
 *        (FTD-0512).
 *
 * The record constructs a selected central elastic reflection from existing
 * boundary charts and carrier momenta.  It also tests whether aggregate
 * trilinear density/current can observe the reflected relative mode.  It does
 * not modify RenderBridge or claim that the face-field action derives the
 * contact premise.
 */

#include "ftd/eft/boundary_chart_capacity.h"

namespace ftd::eft {

struct ConstituentRelativeCollisionResult {
  bool valid = false;
  bool selected_central_contact = false;
  bool face_direction = false;
  bool aggregate_face_kernel = false;
  int L = 0;
  int polarity = 0;
  Coord chart_direction{};
  Vec3 collision_position{};
  Vec3 chart_normal{};
  Vec3 momentum_first_before{};
  Vec3 momentum_second_before{};
  Vec3 momentum_first_after{};
  Vec3 momentum_second_after{};
  Vec3 impulse_first{};
  Vec3 impulse_second{};
  double speed = 0.0;
  double momentum_magnitude = 0.0;
  double impulse_multiplier = 0.0;
  double chart_position_residual = 0.0;
  double incoming_normal_momentum = 0.0;
  double outgoing_normal_momentum = 0.0;
  double normal_com_momentum_residual = 0.0;
  double impulse_sum_residual = 0.0;
  double total_momentum_residual = 0.0;
  double matter_energy_residual = 0.0;
  double central_impulse_residual = 0.0;
  double impulse_solution_residual = 0.0;
  double tangential_relative_residual = 0.0;
  double outgoing_condition_residual = 0.0;
  double involution_residual = 0.0;
  double time_reversal_residual = 0.0;
  double causal_residual = 0.0;
  double continuity_residual = 0.0;
  double aggregate_static_separating_residual = 0.0;
  double aggregate_current_l1 = 0.0;
  double constituent_current_l1 = 0.0;
  double relative_mode_projection_gap = 0.0;
  double matter_kinetic_energy_gap = 0.0;
  BoundaryChartCollisionResult charts{};
  PiecewiseCurrentSignature aggregate_static{};
  PiecewiseCurrentSignature aggregate_separating{};
  PiecewiseCurrentSignature first_separating{};
  PiecewiseCurrentSignature second_separating{};
};

/** Analyze the preregistered equal-mass, zero-COM-normal collision arm.
 *
 * The nontrivial selected solution is the Householder reflection of relative
 * momentum across the chart-normal contact plane.  `observation_distance`
 * controls only the outgoing current observer, not the impulse.
 */
ConstituentRelativeCollisionResult analyze_constituent_relative_collision(
    int L,
    const Vec3& collision_position,
    Coord chart_direction,
    int polarity,
    double speed,
    double rest_energy,
    double c_speed,
    double observation_distance = 0.25,
    double tolerance = 1e-12);

}  // namespace ftd::eft
