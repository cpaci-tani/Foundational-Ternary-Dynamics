#include "ftd/eft/reciprocal_carry_reservoir.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>

namespace {

int failures = 0;

void check(const char* name, bool condition) {
  std::cout << (condition ? "PASS  " : "FAIL  ") << name << '\n';
  if (!condition) ++failures;
}

double max_abs_difference(const ftd::eft::ReciprocalTriplet& left,
                          const ftd::eft::ReciprocalTriplet& right) {
  return std::max({std::abs(left[0] - right[0]),
                   std::abs(left[1] - right[1]),
                   std::abs(left[2] - right[2])});
}

ftd::eft::ReciprocalTriplet signed_cubic_transform(
    const ftd::eft::ReciprocalTriplet& value) {
  return {-value[1], value[0], -value[2]};
}

ftd::eft::ReciprocalCarryTriplet signed_cubic_transform(
    const ftd::eft::ReciprocalCarryTriplet& value) {
  return {-value[1], value[0], -value[2]};
}

}  // namespace

int main() {
  using namespace ftd::eft;
  const double pi = std::acos(-1.0);

  ReciprocalCarryInput input;
  input.principal_first = {3.0 * pi / 4.0, -4.0 * pi / 5.0, pi / 3.0};
  input.principal_second = {pi / 2.0, -pi / 2.0, 5.0 * pi / 6.0};
  input.opposite_increment = {pi / 2.0, -3.0 * pi / 4.0,
                              7.0 * pi / 3.0};
  input.reciprocal_reservoir = {2, -1, 4};
  input.momentum_scale = 1.75;

  const auto result = apply_reciprocal_carry_transaction(input);
  check("reciprocal-carry transaction is valid", result.valid());
  check("first carries retain each crossed reciprocal zone",
        result.carry_first == ReciprocalCarryTriplet{1, -1, 1});
  check("second carries retain each crossed reciprocal zone",
        result.carry_second == ReciprocalCarryTriplet{0, 0, -1});
  check("aggregate reservoir receives the unique carry sum",
        result.reciprocal_reservoir_after
            == ReciprocalCarryTriplet{3, -2, 4});
  check("both updated labels remain principal",
        result.principal_first_after[0] >= -pi
            && result.principal_first_after[0] < pi
            && result.principal_second_after[2] >= -pi
            && result.principal_second_after[2] < pi);
  check("lifted dimensionless aggregate is conserved",
        result.reciprocal_carry_update_exact
            && result.conservation_residual <= input.tolerance);
  check("reservoir update is unique after branch selection",
        result.reservoir_increment_unique_given_branch_and_conservation);
  check("opposite increment exactly reverses the full state",
        result.full_state_reversal_exact
            && result.reversal_residual <= input.tolerance);
  check("physical candidate conserves only after imposed conversion",
        max_abs_difference(result.physical_momentum_before,
                           result.physical_momentum_after)
            <= input.tolerance);
  check("opposite increment need not conserve periodic band energy",
        std::abs(result.band_energy_change) > 1e-6);
  check("periodic band energy is blind to the integer reservoir",
        result.periodic_band_energy_blind_to_reservoir);

  ReciprocalCarryInput multi_zone;
  multi_zone.principal_first = {pi / 4.0, -pi / 5.0, pi / 7.0};
  multi_zone.principal_second = {-pi / 4.0, pi / 5.0, -pi / 7.0};
  multi_zone.opposite_increment = {9.0 * pi / 2.0,
                                   -13.0 * pi / 3.0,
                                   17.0 * pi / 4.0};
  multi_zone.reciprocal_reservoir = {-3, 4, 1};
  const auto large_step = apply_reciprocal_carry_transaction(multi_zone);
  check("multiple-zone increments are supported", large_step.valid()
            && large_step.multi_zone_increment_supported);
  check("multiple-zone aggregate remains exact",
        large_step.conservation_residual <= multi_zone.tolerance
            && large_step.full_state_reversal_exact);

  ReciprocalCarryInput no_crossing;
  no_crossing.principal_first = {pi / 8.0, -pi / 7.0, pi / 9.0};
  no_crossing.principal_second = {-pi / 10.0, pi / 12.0, -pi / 11.0};
  no_crossing.opposite_increment = {pi / 20.0, -pi / 30.0, pi / 40.0};
  const auto quiet = apply_reciprocal_carry_transaction(no_crossing);
  check("no-crossing control is valid", quiet.valid());
  check("no-crossing control changes no reciprocal reservoir",
        quiet.carry_first == ReciprocalCarryTriplet{0, 0, 0}
            && quiet.carry_second == ReciprocalCarryTriplet{0, 0, 0}
            && quiet.reciprocal_reservoir_after
                == ReciprocalCarryTriplet{0, 0, 0});

  ReciprocalCarryInput transformed_input = input;
  transformed_input.principal_first = signed_cubic_transform(
      input.principal_first);
  transformed_input.principal_second = signed_cubic_transform(
      input.principal_second);
  transformed_input.opposite_increment = signed_cubic_transform(
      input.opposite_increment);
  transformed_input.reciprocal_reservoir = signed_cubic_transform(
      input.reciprocal_reservoir);
  const auto transformed = apply_reciprocal_carry_transaction(
      transformed_input);
  check("signed cubic transform remains valid", transformed.valid());
  check("updated lifted aggregate is cubic covariant",
        max_abs_difference(
            transformed.dimensionless_total_after,
            signed_cubic_transform(result.dimensionless_total_after))
            <= 20.0 * input.tolerance);
  check("reciprocal reservoir is cubic covariant away from branch edges",
        transformed.reciprocal_reservoir_after
            == signed_cubic_transform(result.reciprocal_reservoir_after));

  auto nonfinite = input;
  nonfinite.opposite_increment[0] =
      std::numeric_limits<double>::quiet_NaN();
  check("nonfinite increment fails closed",
        apply_reciprocal_carry_transaction(nonfinite).status
            == ReciprocalCarryStatus::NonFiniteInput);
  auto invalid_tolerance = input;
  invalid_tolerance.tolerance = 0.0;
  check("nonpositive tolerance fails closed",
        apply_reciprocal_carry_transaction(invalid_tolerance).status
            == ReciprocalCarryStatus::InvalidTolerance);
  auto invalid_scale = input;
  invalid_scale.momentum_scale = 0.0;
  check("nonpositive momentum scale fails closed",
        apply_reciprocal_carry_transaction(invalid_scale).status
            == ReciprocalCarryStatus::InvalidMomentumScale);
  auto nonprincipal = input;
  nonprincipal.principal_first[0] = pi;
  check("nonprincipal input label fails closed",
        apply_reciprocal_carry_transaction(nonprincipal).status
            == ReciprocalCarryStatus::NonPrincipalLabel);
  auto carry_out_of_range = input;
  carry_out_of_range.opposite_increment[0] =
      std::numeric_limits<double>::max();
  check("unrepresentable reciprocal carry fails closed",
        apply_reciprocal_carry_transaction(carry_out_of_range).status
            == ReciprocalCarryStatus::CarryOutOfRange);
  auto reservoir_overflow = input;
  reservoir_overflow.reciprocal_reservoir[0] =
      std::numeric_limits<std::int64_t>::max();
  check("integer reservoir overflow fails closed",
        apply_reciprocal_carry_transaction(reservoir_overflow).status
            == ReciprocalCarryStatus::ReservoirOverflow);

  check("interaction origin and substrate identification remain open",
        !result.interaction_increment_derived
            && !result.reservoir_substrate_identification_derived);
  check("reservoir energy and physical momentum scale remain open",
        !result.reservoir_energy_law_derived
            && !result.physical_momentum_scale_derived);
  check("total field-matter momentum and absolute mass remain open",
        !result.total_field_matter_momentum_map_derived
            && !result.absolute_mass_derived);
  check("production, Born, and native Gstar remain untouched",
        !result.production_coupling_supplied
            && !result.born_target_used
            && !result.native_gstar_synchronization_supplied);
  check("no new selected vector type is added",
        !result.new_selected_vector_type_added);

  std::cout << "FTD-0897 reciprocal carry reservoir: "
            << (failures == 0 ? "PASS" : "FAIL") << '\n';
  return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
