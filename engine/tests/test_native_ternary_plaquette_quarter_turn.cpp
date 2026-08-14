#include "ftd/eft/native_ternary_plaquette_quarter_turn.h"

#include <cmath>
#include <iostream>
#include <string>

namespace {

int failures = 0;
int checks = 0;

void check(bool condition, const std::string& name) {
  ++checks;
  if (condition) {
    std::cout << "[PASS] " << name << '\n';
  } else {
    ++failures;
    std::cout << "[FAIL] " << name << '\n';
  }
}

}  // namespace

int main() {
  const auto result =
      ftd::eft::analyze_native_ternary_plaquette_quarter_turn();
  check(result.ternary_neutral_orbit, "ternary neutral orbit");
  check(result.forward_shift_order_four, "forward shift order four");
  check(result.alternating_subspace_complex_structure,
        "alternating subspace complex structure");
  check(result.reverse_is_negative_complex_structure,
        "reverse is negative complex structure");
  check(result.dipole_quarter_turn_exact, "dipole quarter turn exact");
  check(result.forward_bivector_constant_nonzero,
        "forward bivector constant nonzero");
  check(result.reverse_bivector_is_negative,
        "reverse bivector is negative");
  check(result.transition_bivector_time_odd, "transition bivector time odd");
  check(result.coordinate_free_successor_exact,
        "coordinate-free successor exact");
  check(result.self_dual_energy_split_exact, "self-dual energy split exact");
  check(result.signed_cubic_arms == 48, "all signed cubic arms");
  check(result.signed_cubic_covariance_exact,
        "signed cubic covariance exact");
  check(result.symmetric_square_loses_orientation,
        "symmetric square loses orientation");
  check(result.ordered_bivector_retains_orientation,
        "ordered bivector retains orientation");
  check(result.instantaneous_word_direction_ambiguous,
        "instantaneous word direction ambiguous");
  check(result.minimum_cardinal_cycle_is_four,
        "minimum cardinal cycle is four");
  check(result.contraction_samples == 5, "contraction sample count");
  check(result.ordinary_real_lift_contracts_to_zero,
        "ordinary real lift contracts to zero");
  check(result.maximum_reconstruction_residual == 0.0,
        "zero reconstruction residual");
  check(result.maximum_covariance_residual == 0.0,
        "zero covariance residual");
  check(result.maximum_contraction_residual == 0.0,
        "zero contraction residual");
  check(!result.topological_protection_derived,
        "topological protection remains open");
  check(!result.production_orbit_invariant_derived,
        "production orbit invariant remains open");
  check(!result.gstar_used, "G* not used");
  check(!result.gamma_magnitude_derived, "gamma magnitude not derived");
  check(!result.born_or_bell_target_used, "Born/Bell target not used");
  check(!result.production_changed, "production unchanged");
  check(!result.new_selected_type_added, "no selected type added");
  check(result.valid, "combined FTD-0914 result");

  std::cout << "FTD-0914 native ternary plaquette quarter-turn: "
            << (checks - failures) << "/" << checks << " PASS\n";
  std::cout << "PLAQUETTE_QUARTER_TURN_RECURSION=EXACT\n";
  std::cout << "CLOCKWISE_COUNTERCLOCKWISE_BIVECTOR=RETAINED\n";
  std::cout << "INSTANTANEOUS_WORD_DIRECTION=AMBIGUOUS\n";
  std::cout << "TOPOLOGICAL_PROTECTION=NOT_DERIVED\n";
  std::cout << "PRODUCTION_ORBIT_INVARIANT=OPEN\n";
  return failures == 0 ? 0 : 1;
}
