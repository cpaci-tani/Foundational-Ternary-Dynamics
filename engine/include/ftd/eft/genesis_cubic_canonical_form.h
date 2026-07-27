#pragma once
/**
 * @file genesis_cubic_canonical_form.h
 * @brief Observer-only O_h canonical-form classification and genesis
 *        bath-rank comparison (FTD-0573).
 */

namespace ftd::eft {

struct GenesisCubicCanonicalFormResult {
  bool valid = false;
  int full_cubic_group_elements = 0;
  int proper_cubic_group_elements = 0;
  int invariant_constraint_rank = 0;
  int invariant_nullity = 0;
  int production_arms = 0;
  int zero_drain_alternative_arms = 0;
  int positive_drain_alternative_arms = 0;
  int degenerate_a_equals_t_arms = 0;
  int symmetry_price_arms = 0;
  int symmetry_price_bath_pairs = 0;

  double maximum_cubic_invariance_residual = 0.0;
  double maximum_generic_determinant_formula_residual = 0.0;
  double minimum_generic_alternative_determinant = 0.0;

  bool standard_pairing_unique_up_to_scale = false;
  bool zero_drain_unconstrained_minimum_rank_two = false;
  bool generic_unconstrained_minimum_rank_four = false;
  bool degenerate_minimum_rank_six = false;
  bool cubic_covariance_prices_one_bath_pair = false;
  bool branchwise_alternatives_are_not_one_global_form = false;
  bool native_canonical_action_derived = false;
};

GenesisCubicCanonicalFormResult analyze_genesis_cubic_canonical_form();

}  // namespace ftd::eft
