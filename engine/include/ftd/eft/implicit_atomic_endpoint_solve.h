#pragma once
/**
 * @file implicit_atomic_endpoint_solve.h
 * @brief Six-coordinate initial-value solve of the FTD-0536 action (FTD-0537).
 */

#include "ftd/eft/implicit_atomic_face_action.h"

namespace ftd::eft {

struct ImplicitAtomicInitialFixture {
  bool valid = false;
  int L = 0;
  int shell = 0;
  double speed = 0.0;
  double beta = 0.0;
  double temporal_scale = 0.0;
  std::array<Vec3, 2> start_position{};
  std::array<Vec3, 2> free_end_position{};
  std::array<int, 2> charge{};
  std::array<Vec3, 2> prescribed_kinetic_start{};
  MatchedFaceFlux potential_before;
  MatchedFaceFlux electric_before;
  OvershootPreservingContactRebaseResult rebase{};

  explicit ImplicitAtomicInitialFixture(int size = 0)
      : L(size), potential_before(size), electric_before(size) {}
};

ImplicitAtomicInitialFixture make_implicit_atomic_initial_fixture(
    int L,
    const Vec3& contact_position,
    Coord diagonal_direction,
    int polarity,
    double speed,
    double tolerance = 1e-12);

struct ImplicitAtomicEndpointSolveResult {
  bool valid = false;
  bool converged = false;
  int iterations = 0;
  double initial_residual = 0.0;
  double final_residual = 0.0;
  double minimum_pivot = 0.0;
  double minimum_accepted_step_factor = 0.0;
  double maximum_endpoint_change = 0.0;
  std::array<Vec3, 2> displacement{};
  ImplicitAtomicInitialFixture fixture{};
  AtomicFaceEndpointTrialResult trial{};
};

ImplicitAtomicEndpointSolveResult solve_implicit_atomic_endpoint(
    int L,
    const Vec3& contact_position,
    Coord diagonal_direction,
    int polarity,
    double speed,
    double derivative_step = 0.000244140625,
    double root_tolerance = 1e-8,
    double algebra_tolerance = 1e-12,
    bool chart_contained_derivative = false);

}  // namespace ftd::eft
