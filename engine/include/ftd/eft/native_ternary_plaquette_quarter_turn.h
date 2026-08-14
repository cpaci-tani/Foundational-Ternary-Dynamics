#pragma once
/**
 * @file native_ternary_plaquette_quarter_turn.h
 * @brief FTD-0914 isolated ternary-plaquette recursion analyzer.
 *
 * The analyzer realizes the order-four complex structure on the minimum
 * cardinal square using the neutral ternary word (+,0,-,0).  It proves
 * representability and the exact recurrence/protection boundary only; it
 * does not read or modify production state.
 */

#include <array>

namespace ftd::eft {

using PlaquetteVector = std::array<double, 3>;
using PlaquetteWord = std::array<int, 4>;

struct NativeTernaryPlaquetteQuarterTurnResult {
  std::array<PlaquetteWord, 4> forward_words{};
  std::array<PlaquetteWord, 4> reverse_words{};
  std::array<PlaquetteVector, 4> forward_dipoles{};
  std::array<PlaquetteVector, 4> forward_bivectors{};
  std::array<PlaquetteVector, 4> reverse_bivectors{};

  int signed_cubic_arms = 0;
  int contraction_samples = 0;
  double dipole_norm_squared = 0.0;
  double bivector_norm = 0.0;
  double radial_energy = 0.0;
  double tangential_energy = 0.0;
  double maximum_reconstruction_residual = 0.0;
  double maximum_covariance_residual = 0.0;
  double maximum_contraction_residual = 0.0;

  bool valid = false;
  bool ternary_neutral_orbit = false;
  bool forward_shift_order_four = false;
  bool alternating_subspace_complex_structure = false;
  bool reverse_is_negative_complex_structure = false;
  bool dipole_quarter_turn_exact = false;
  bool forward_bivector_constant_nonzero = false;
  bool reverse_bivector_is_negative = false;
  bool transition_bivector_time_odd = false;
  bool coordinate_free_successor_exact = false;
  bool self_dual_energy_split_exact = false;
  bool signed_cubic_covariance_exact = false;
  bool symmetric_square_loses_orientation = false;
  bool ordered_bivector_retains_orientation = false;
  bool instantaneous_word_direction_ambiguous = false;
  bool minimum_cardinal_cycle_is_four = false;
  bool ordinary_real_lift_contracts_to_zero = false;
  bool topological_protection_derived = false;
  bool production_orbit_invariant_derived = false;
  bool gstar_used = false;
  bool gamma_magnitude_derived = false;
  bool born_or_bell_target_used = false;
  bool production_changed = false;
  bool new_selected_type_added = false;
};

NativeTernaryPlaquetteQuarterTurnResult
analyze_native_ternary_plaquette_quarter_turn();

}  // namespace ftd::eft
