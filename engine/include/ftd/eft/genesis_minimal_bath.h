#pragma once
/**
 * @file genesis_minimal_bath.h
 * @brief Observer-only minimum symplectic-bath construction for the accepted
 *        production genesis derivative (FTD-0572).
 */

namespace ftd::eft {

struct GenesisMinimalBathResult {
  bool valid = false;
  int matrix_arms = 0;
  int pair_arms = 0;
  int defective_pair_arms = 0;
  int rank_four_arms = 0;
  int rank_six_arms = 0;
  int minimum_bath_pairs_zero_drain = 0;
  int minimum_bath_pairs_positive_drain = 0;

  double maximum_pair_symplectic_residual = 0.0;
  double maximum_prepared_projection_residual = 0.0;
  double maximum_two_step_formula_residual = 0.0;
  double minimum_nonzero_two_step_deviation = 0.0;
  double minimum_passive_commutator = 0.0;

  bool rank_lower_bound_proved = false;
  bool feedback_and_record_ranks_saturate = false;
  bool minimum_dilation_constructed = false;
  bool fixed_zero_bath_section_cannot_repeat = false;
  bool passive_equal_weight_energy_obstructed = false;
  bool reset_or_active_energy_reservoir_required = false;
};

GenesisMinimalBathResult analyze_genesis_minimal_bath();

}  // namespace ftd::eft
