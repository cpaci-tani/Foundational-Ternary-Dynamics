#pragma once
/**
 * @file closed_neutral_trimer_pair.h
 * @brief Observer-only closed neutral pair of charged constituent trimers
 *        (FTD-0601).
 *
 * Six explicit ternary constituent records couple to one matched face/edge
 * field.  There is no stationary compensating density and no RenderBridge
 * or production-tick integration.
 */

#include "ftd/eft/constituent_complete_charged_trimer.h"

#include <array>

namespace ftd::eft {

constexpr std::size_t CLOSED_NEUTRAL_PAIR_SIZE = 6;
constexpr std::size_t CLOSED_NEUTRAL_TRIMER_SIZE = 3;

struct ClosedNeutralTrimerPairState {
  MatchedFaceFlux electric;
  MatchedEdgeField magnetic_half;
  std::array<MatchedMatterPoint, CLOSED_NEUTRAL_PAIR_SIZE> constituents{};
  std::array<int, CLOSED_NEUTRAL_PAIR_SIZE> charges{
      {-1, -1, +1, +1, +1, -1}};

  explicit ClosedNeutralTrimerPairState(int size = 0)
      : electric(size), magnetic_half(size) {}
};

using ClosedNeutralPairOptions = ChargedTrimerOptions;
using ClosedNeutralPairSolveDiagnostics = ChargedTrimerSolveDiagnostics;

struct ClosedNeutralTrimerPairStepResult {
  bool valid = false;
  bool common_action_gates_pass = false;
  bool isolated_momentum_gate_pass = false;
  bool forward = true;
  bool site_projection_valid = false;
  int net_charge = 0;
  FaceFluxNormalization normalization{};
  double interaction_scale = 0.0;
  ClosedNeutralTrimerPairState earlier;
  ClosedNeutralTrimerPairState later;
  ClosedNeutralPairSolveDiagnostics solve{};
  std::array<QuadraticCoatFaceCurrent, CLOSED_NEUTRAL_PAIR_SIZE> segments{};
  std::array<QuadraticCoatOrbitGatherResult,
             CLOSED_NEUTRAL_PAIR_SIZE> gathers{};
  std::array<Vec3, CLOSED_NEUTRAL_PAIR_SIZE> velocities{};
  std::array<Vec3, CLOSED_NEUTRAL_PAIR_SIZE> electric_impulses{};
  std::array<Vec3, CLOSED_NEUTRAL_PAIR_SIZE> magnetic_impulses{};
  std::array<Vec3, CLOSED_NEUTRAL_PAIR_SIZE> binding_impulses{};
  std::array<Vec3, CLOSED_NEUTRAL_PAIR_SIZE> total_impulses{};

  double kinetic_energy_before = 0.0;
  double kinetic_energy_after = 0.0;
  double binding_energy_before = 0.0;
  double binding_energy_after = 0.0;
  double field_energy_before = 0.0;
  double field_energy_after = 0.0;
  double current_work = 0.0;
  std::array<Vec3, 2> composite_momentum_before{};
  std::array<Vec3, 2> composite_momentum_after{};
  Vec3 matter_momentum_before{};
  Vec3 matter_momentum_after{};
  Vec3 field_pseudomomentum_before{};
  Vec3 field_pseudomomentum_after{};
  Vec3 total_pseudomomentum_before{};
  Vec3 total_pseudomomentum_after{};
  Vec3 pseudomomentum_defect{};

  double root_residual = 0.0;
  double continuity_residual = 0.0;
  double gauss_before_residual = 0.0;
  double gauss_after_residual = 0.0;
  double force_residual = 0.0;
  double kinematic_residual = 0.0;
  double kinetic_discrete_gradient_residual = 0.0;
  double electric_adjoint_residual = 0.0;
  double magnetic_work_residual = 0.0;
  double binding_work_residual = 0.0;
  double binding_impulse_sum_residual = 0.0;
  double matter_work_residual = 0.0;
  double field_work_residual = 0.0;
  double total_energy_residual = 0.0;
  double causal_speed_excess = 0.0;
  double pseudomomentum_defect_norm = 0.0;
  double inward_impulse = 0.0;
  double center_separation_before = 0.0;
  double center_separation_after = 0.0;
  double minimum_internal_pair_distance = 0.0;
  double maximum_internal_pair_distance = 0.0;

  explicit ClosedNeutralTrimerPairStepResult(int size = 0)
      : earlier(size), later(size) {}
};

double closed_neutral_pair_binding_energy(
    const ClosedNeutralTrimerPairState& state,
    const ClosedNeutralPairOptions& options = {});

ClosedNeutralTrimerPairStepResult solve_closed_neutral_pair_forward(
    const ClosedNeutralTrimerPairState& earlier,
    const ClosedNeutralPairOptions& options = {});

ClosedNeutralTrimerPairStepResult solve_closed_neutral_pair_reverse(
    const ClosedNeutralTrimerPairState& later,
    const ClosedNeutralPairOptions& options = {});

double closed_neutral_pair_state_max_difference(
    const ClosedNeutralTrimerPairState& lhs,
    const ClosedNeutralTrimerPairState& rhs);

}  // namespace ftd::eft

