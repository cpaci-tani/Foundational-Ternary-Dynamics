#pragma once
/**
 * @file genesis_natural_extension.h
 * @brief Observer-only exact-real natural extension and branchwise symplectic
 *        lift of the canonical single-genesis trial (FTD-0570).
 */

#include <array>

#include "ftd/voxel.h"

namespace ftd::eft {

struct NaturalExtensionPhase {
  double u = 0.0;
  double v = 0.0;
};

struct NaturalExtensionPhaseStep {
  bool valid = false;
  bool accepted = false;
  double probability = 0.0;
  NaturalExtensionPhase before{};
  NaturalExtensionPhase after{};
  NaturalExtensionPhase recovered{};
  double jacobian_determinant = 0.0;
  double inverse_residual = 0.0;
};

/// Generalized-baker natural extension of a Bernoulli draw.
NaturalExtensionPhaseStep advance_natural_extension_phase(
    const NaturalExtensionPhase& before, double probability);

/// Inverse branch is encoded by the future v interval.
NaturalExtensionPhaseStep reverse_natural_extension_phase(
    const NaturalExtensionPhase& after, double probability);

struct GenesisNaturalLiftState {
  Vec3 flux{};
  Vec3 wave{};
  std::array<double, 6> conjugate{};
  NaturalExtensionPhase phase{};
  double tau = 0.0;
  double reservoir_energy = 0.0;
};

struct GenesisNaturalLiftOptions {
  double genesis_threshold = 1.0;
  double manifestation_scale = 1.0;
  double kinetic_drain = 0.5;
};

struct GenesisNaturalLiftStep {
  bool valid = false;
  bool accepted = false;
  GenesisNaturalLiftState before{};
  GenesisNaturalLiftState after{};
  GenesisNaturalLiftState recovered{};

  double probability = 0.0;
  double field_withdrawal = 0.0;
  double raw_radial_symplectic_defect = 0.0;
  double raw_tangential_symplectic_defect = 0.0;
  double raw_volume_jacobian = 1.0;

  double inverse_residual = 0.0;
  double energy_residual = 0.0;
  double phase_generator_residual = 0.0;
  double conjugate_generator_residual = 0.0;
  double reservoir_generator_residual = 0.0;
  double raw_symplectic_formula_residual = 0.0;
};

/// Branch is selected by before.phase.u relative to p(q). The projected q map
/// is the frozen accepted genesis drain or the rejected identity. All added
/// variables are observer-only mathematical coordinates.
GenesisNaturalLiftStep advance_genesis_natural_lift(
    const GenesisNaturalLiftState& before,
    const GenesisNaturalLiftOptions& options = {});

struct GenesisNaturalExtensionResult {
  bool valid = false;
  int baker_arms = 0;
  int lift_arms = 0;
  int accepted_lift_arms = 0;

  double maximum_baker_inverse_residual = 0.0;
  double maximum_baker_jacobian_residual = 0.0;
  double maximum_lift_inverse_residual = 0.0;
  double maximum_energy_residual = 0.0;
  double maximum_phase_generator_residual = 0.0;
  double maximum_conjugate_generator_residual = 0.0;
  double maximum_reservoir_generator_residual = 0.0;
  double maximum_raw_symplectic_formula_residual = 0.0;
  double minimum_raw_tangential_defect_magnitude = 0.0;
  double maximum_raw_volume_jacobian = 0.0;
  double projected_log_forward_reverse_ratio = 0.0;

  bool exact_real_natural_extension = false;
  bool raw_genesis_is_not_canonical = false;
  bool branchwise_symplectic_energy_lift = false;
  bool binary64_history_collision = false;
  bool exact_real_is_infinite_information = false;
  bool projected_kernel_absolutely_irreversible = false;
  bool additional_primitives_required = false;
  bool production_common_action_recovered = false;
};

GenesisNaturalExtensionResult analyze_genesis_natural_extension();

}  // namespace ftd::eft
