#pragma once
/**
 * @file hard_contact_corner_action.h
 * @brief Observer-only relativistic hard-contact corner action (FTD-0516).
 */

#include "ftd/eft/momentum_face_balance.h"

namespace ftd::eft {

struct HardContactCornerActionResult {
  bool valid = false;
  int L = 0;
  int polarity = 0;
  Coord chart_direction{};
  Vec3 collision_position{};
  Vec3 normal{};
  Vec3 momentum_first_before{};
  Vec3 momentum_second_before{};
  Vec3 momentum_first_after{};
  Vec3 momentum_second_after{};
  Vec3 velocity_first_before{};
  Vec3 velocity_second_before{};
  Vec3 velocity_first_after{};
  Vec3 velocity_second_after{};
  double rest_energy = 0.0;
  double c_speed = 0.0;
  double speed = 0.0;
  double contact_gap = 0.0;
  double relative_normal_momentum = 0.0;
  double impulse_multiplier = 0.0;
  double inactive_control_multiplier = 0.0;
  double incoming_gap_rate = 0.0;
  double outgoing_gap_rate = 0.0;
  double reference_collision_residual = 0.0;
  double multiplier_match_residual = 0.0;
  double normal_impulse_residual = 0.0;
  double tangential_corner_residual = 0.0;
  double common_corner_gradient_residual = 0.0;
  double collision_time_gradient_residual = 0.0;
  double total_momentum_residual = 0.0;
  double total_energy_residual = 0.0;
  double action_density_residual = 0.0;
  double legendre_residual = 0.0;
  double branch_polynomial_residual = 0.0;
  double nontrivial_branch_residual = 0.0;
  double kkt_dual_residual = 0.0;
  double complementarity_residual = 0.0;
  double incoming_gate_residual = 0.0;
  double outgoing_gate_residual = 0.0;
  double face_balance_residual = 0.0;
  double reversal_residual = 0.0;
  double reversal_multiplier_residual = 0.0;
  ConstituentRelativeCollisionResult reference_collision{};
  CollisionMomentumFaceBalance face_balance{};
};

/// Algebraically eliminate the selected unilateral contact multiplier.
/// Positive gap is inactive; contact with positive incoming q_n selects 2q_n.
double selected_hard_contact_multiplier(double gap,
                                        double incoming_q_n,
                                        double tolerance = 1e-12);

/// Test the selected hard-contact corner action on the restricted FTD-0512
/// equal-mass, axial-relative, zero-COM collision class.
HardContactCornerActionResult analyze_hard_contact_corner_action(
    int L,
    const Vec3& collision_position,
    Coord chart_direction,
    int polarity,
    double speed,
    double rest_energy,
    double c_speed,
    double segment_distance = 0.25,
    double tolerance = 1e-12);

}  // namespace ftd::eft
