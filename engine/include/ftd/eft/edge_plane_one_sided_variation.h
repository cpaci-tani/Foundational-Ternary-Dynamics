#pragma once
/**
 * @file edge_plane_one_sided_variation.h
 * @brief In-plane solve and one-sided normal audit for shell-2 action (FTD-0539).
 */

#include "ftd/eft/implicit_atomic_endpoint_solve.h"

namespace ftd::eft {

struct EdgePlaneOneSidedVariationResult {
  bool valid = false;
  bool converged = false;
  bool normal_differentiable = false;
  bool normal_interval_contains_zero = false;
  int normal_axis = -1;
  int iterations = 0;
  double initial_active_residual = 0.0;
  double final_active_residual = 0.0;
  double active_derivative_convergence = 0.0;
  double minimum_jacobian_pivot = 0.0;
  double minimum_accepted_step_factor = 0.0;
  double maximum_endpoint_change_from_free = 0.0;
  double maximum_normal_residual_jump = 0.0;
  std::array<Vec3, 2> endpoint{};
  std::array<Vec3, 2> displacement{};
  ImplicitAtomicInitialFixture fixture{};
  AtomicFaceEndpointTrialResult trial{};
  AtomicFaceOneSidedNormalResult normal{};
};

EdgePlaneOneSidedVariationResult solve_edge_plane_one_sided_variation(
    int L,
    const Vec3& contact_position,
    Coord edge_direction,
    int polarity,
    double speed,
    double derivative_step = 0.000244140625,
    double root_tolerance = 1e-8,
    double derivative_tolerance = 1e-7,
    double algebra_tolerance = 1e-12);

}  // namespace ftd::eft

