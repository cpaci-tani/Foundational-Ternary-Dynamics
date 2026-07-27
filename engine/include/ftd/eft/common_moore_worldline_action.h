#pragma once
/**
 * @file common_moore_worldline_action.h
 * @brief Observer-only spacetime/action completion of the Moore coat (FTD-0578).
 *
 * The records in this file are derived coupling observers.  They neither add
 * a persistent field variable nor change the production tick.
 */

#include "ftd/eft/minimal_moore_compatibility_coat.h"

#include <vector>

namespace ftd::eft {

struct MooreSpacetimeCurrent {
  int L = 0;
  int charge = 0;
  Vec3 start_effective_position{};
  Vec3 end_effective_position{};

  std::vector<double> rho_start;
  std::vector<double> rho_end;
  std::vector<double> temporal_density_start;
  std::vector<double> temporal_density_end;
  std::vector<Vec3> current_start;
  std::vector<Vec3> current_end;

  double temporal_partition_residual = 0.0;
  double current_reconstruction_residual = 0.0;
  double split_continuity_start_residual = 0.0;
  double split_continuity_end_residual = 0.0;
  double aggregate_continuity_residual = 0.0;
  bool finite_range = false;
  bool valid = false;

  int index(int x, int y, int z) const;
};

MooreSpacetimeCurrent make_common_moore_spacetime_current(
    int L, const Vec3& start_effective_position,
    const Vec3& end_effective_position, int charge);

struct CommonMooreWorldlineActionResult {
  bool valid = false;

  int aggregate_split_arms = 0;
  int polarity_arms = 0;
  int volume_arms = 0;
  int translation_arms = 0;
  int proper_cubic_rotation_arms = 0;
  int action_fixture_arms = 0;
  int centering_arms = 0;
  int peierls_arms = 0;

  double maximum_temporal_partition_residual = 0.0;
  double maximum_current_reconstruction_residual = 0.0;
  double maximum_split_continuity_residual = 0.0;
  double maximum_aggregate_continuity_residual = 0.0;
  double maximum_deposit_orbit_action_residual = 0.0;
  double maximum_endpoint_field_adjoint_residual = 0.0;
  double maximum_magnetic_scalar_work_residual = 0.0;
  double maximum_translation_covariance_residual = 0.0;
  double maximum_cubic_covariance_residual = 0.0;
  double axial_centering_norm2 = 0.0;
  double edge_centering_norm2 = 0.0;
  double body_centering_norm2 = 0.0;
  double maximum_centering_rational_residual = 0.0;
  double minimum_diagonal_centering_norm2 = 0.0;
  double minimum_peierls_coefficient = 0.0;
  double minimum_peierls_barrier = 0.0;
  double maximum_peierls_law_residual = 0.0;
  double maximum_peierls_polarity_residual = 0.0;
  double maximum_peierls_cubic_residual = 0.0;

  bool coated_spacetime_continuity_exact = false;
  bool common_action_deposition_and_gather_adjoint = false;
  bool reciprocal_path_gather_derived = false;
  bool magnetic_scalar_work_zero = false;
  bool axial_energy_centering_exact = false;
  bool diagonal_energy_centering_fails = false;
  bool point_carrier_peierls_pinned = false;
  bool unmodified_action_is_free_mobile_law = false;
  bool production_changed = false;
};

CommonMooreWorldlineActionResult analyze_common_moore_worldline_action();

}  // namespace ftd::eft
