#pragma once
/**
 * @file implicit_atomic_face_action.h
 * @brief Observer-only minimal implicit face-action endpoint audit (FTD-0536).
 */

#include "ftd/eft/matched_face_energy_transaction.h"
#include "ftd/eft/staggered_current_split_compatibility.h"

#include <array>

namespace ftd::eft {

struct ImplicitAtomicFaceActionResult {
  bool valid = false;
  bool scalar_root_stationary = false;
  int shell = 0;
  int carrier_count = 0;
  double beta = 0.0;
  double temporal_scale = 0.0;
  double coupling = 0.0;
  double momentum_before = 0.0;
  double momentum_after = 0.0;
  double matter_legendre_momentum = 0.0;
  double matter_action = 0.0;
  double field_action = 0.0;
  double interaction_action = 0.0;
  double total_action = 0.0;
  double current_split_residual = 0.0;
  double continuity_residual = 0.0;
  double field_start_equation_residual = 0.0;
  double field_end_equation_residual = 0.0;
  double field_update_residual = 0.0;
  double gauss_evolution_residual = 0.0;
  double endpoint_derivative_convergence = 0.0;
  double kinetic_start_residual = 0.0;
  double kinetic_end_residual = 0.0;
  double longitudinal_start_residual = 0.0;
  double longitudinal_end_residual = 0.0;
  double transverse_start_residual = 0.0;
  double transverse_end_residual = 0.0;
  double matter_energy_change = 0.0;
  double field_energy_change = 0.0;
  double total_energy_defect = 0.0;
  double inherited_endpoint_residual = 0.0;
  SymmetricDiagonalCoupledEndpointResult coupled{};
};

struct AtomicFaceEndpointTrialResult {
  bool valid = false;
  int L = 0;
  int carrier_count = 0;
  double beta = 0.0;
  double temporal_scale = 0.0;
  double coupling = 0.0;
  std::array<Vec3, 2> start_position{};
  std::array<Vec3, 2> end_position{};
  std::array<int, 2> charge{};
  std::array<Vec3, 2> prescribed_kinetic_start{};
  std::array<Vec3, 2> kinetic_start{};
  std::array<Vec3, 2> kinetic_end{};
  std::array<Vec3, 2> start_residual{};
  double residual_infinity_norm = 0.0;
  double endpoint_derivative_convergence = 0.0;
  double minimum_endpoint_chart_clearance = 0.0;
  double minimum_endpoint_derivative_step = 0.0;
  double current_split_residual = 0.0;
  double continuity_residual = 0.0;
  double field_start_equation_residual = 0.0;
  double field_end_equation_residual = 0.0;
  double field_update_residual = 0.0;
  double gauss_evolution_residual = 0.0;
  double causal_excess = 0.0;
  double matter_action = 0.0;
  double field_action = 0.0;
  double interaction_action = 0.0;
  double total_action = 0.0;
  double matter_energy_change = 0.0;
  double ordinary_field_energy_change = 0.0;
  double modified_field_energy_change = 0.0;
  double ordinary_total_energy_defect = 0.0;
  double modified_total_energy_defect = 0.0;
  MatchedFaceFlux potential_after;
  MatchedFaceFlux electric_after;
  MatchedEdgeField magnetic_after;

  explicit AtomicFaceEndpointTrialResult(int size = 0)
      : L(size), potential_after(size), electric_after(size),
        magnetic_after(size) {}
};

/** Evaluate the FTD-0536 action on arbitrary two-carrier endpoints after
 * eliminating A_1 with the initial connection equation.
 */
AtomicFaceEndpointTrialResult evaluate_atomic_face_endpoint_trial(
    const std::array<Vec3, 2>& start_position,
    const std::array<Vec3, 2>& end_position,
    const std::array<int, 2>& charge,
    const std::array<Vec3, 2>& prescribed_kinetic_start,
    const MatchedFaceFlux& potential_before,
    const MatchedFaceFlux& electric_before,
    double beta,
    double temporal_scale,
    double rest_energy,
    double c_speed,
    double derivative_step = 0.000244140625,
    double tolerance = 1e-12,
    bool chart_contained_derivative = false);

struct AtomicFaceOneSidedNormalResult {
  bool valid = false;
  int normal_axis = -1;
  double derivative_convergence = 0.0;
  double maximum_derivative_jump = 0.0;
  std::array<double, 2> start_derivative_left{};
  std::array<double, 2> start_derivative_right{};
  std::array<double, 2> end_derivative_left{};
  std::array<double, 2> end_derivative_right{};
  std::array<double, 2> incoming_residual_left{};
  std::array<double, 2> incoming_residual_right{};
};

/** Evaluate left/right normal derivatives of the unchanged interaction action
 * at an endpoint chart plane, holding the solved connection slab fixed.
 */
AtomicFaceOneSidedNormalResult evaluate_atomic_face_one_sided_normal(
    const std::array<Vec3, 2>& start_position,
    const std::array<Vec3, 2>& end_position,
    const std::array<int, 2>& charge,
    const std::array<Vec3, 2>& prescribed_kinetic_start,
    const MatchedFaceFlux& potential_before,
    const MatchedFaceFlux& electric_before,
    double beta,
    double temporal_scale,
    double rest_energy,
    double c_speed,
    int normal_axis,
    double derivative_step = 0.000244140625,
    double tolerance = 1e-12);

/** Reconstruct the minimal FTD-0484/FTD-0490 one-slab action on one locked
 * FTD-0531 diagonal endpoint and test its field and particle equations.
 */
ImplicitAtomicFaceActionResult analyze_implicit_atomic_face_action(
    int L,
    const Vec3& contact_position,
    Coord diagonal_direction,
    int polarity,
    double speed,
    double derivative_step = 0.000244140625,
    double tolerance = 1e-12);

}  // namespace ftd::eft
