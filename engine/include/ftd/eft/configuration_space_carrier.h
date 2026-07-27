#pragma once
/**
 * @file configuration_space_carrier.h
 * @brief Observer-only fixed-source configuration-space carrier gate
 *        (FTD-0584).
 */

namespace ftd::eft {

struct ConfigurationSpaceCarrierResult {
  bool valid = false;
  int volumes = 0;
  int fibre_fixtures = 0;
  int homotopy_samples = 0;
  int uncontained_support_samples = 0;
  int transition_rows = 0;
  int registered_feature_rank = 0;
  int registered_feature_nullity = 0;
  int vacuum_pi0_nontrivial = 0;
  int vacuum_pi1_rank = 0;
  int vacuum_pi2_rank = 0;
  int vacuum_pi3_rank = 0;
  double maximum_gauss_residual = 0.0;
  double maximum_harmonic_coordinate_residual = 0.0;
  double maximum_affine_residual = 0.0;
  double maximum_energy_polynomial_residual = 0.0;
  double maximum_divergence_free_deformation = 0.0;
  int maximum_support_excess = 0;
  bool fixed_source_fibres_affine_contractible = false;
  bool uncontained_finite_energy_space_contractible = false;
  bool snapshot_is_disjoint_union_of_contractible_fibres = false;
  bool ternary_snapshot_disconnectedness_is_conservation = false;
  bool registered_additive_transition_invariant_exists = false;
  bool universal_transition_graph_invariant_excluded = false;
  bool frozen_vacuum_is_single_point = false;
  bool normalized_direction_protected_while_zero_allowed = false;
  bool two_derivative_static_core_size_stable = false;
  bool four_derivative_term_can_balance_scaling = false;
  bool compact_u1_automatically_supplies_electric_charge = false;
  bool compact_flux_integer_requires_admissibility = false;
  bool same_variable_active_localized_mode_excluded = false;
  bool production_changed = false;
};

ConfigurationSpaceCarrierResult analyze_configuration_space_carrier();

}  // namespace ftd::eft
