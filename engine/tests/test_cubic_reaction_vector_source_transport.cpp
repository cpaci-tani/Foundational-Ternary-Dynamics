#include "ftd/eft/cubic_reaction_vector_source_transport.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>

namespace {

int failures = 0;

void check(const char* name, bool condition) {
  if (condition) {
    std::cout << "PASS  " << name << '\n';
  } else {
    std::cout << "FAIL  " << name << '\n';
    ++failures;
  }
}

double max_abs(const ftd::Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

ftd::Vec3 transform(const ftd::Vec3& value, int arm) {
  switch (arm) {
    case 0: return {-value.x, value.y, value.z};
    case 1: return {value.x, -value.y, value.z};
    case 2: return {value.x, value.y, -value.z};
    case 3: return {value.y, value.x, value.z};
    case 4: return {value.z, value.y, value.x};
    default: return value;
  }
}

}  // namespace

int main() {
  using namespace ftd;
  using namespace ftd::eft;

  CubicReactionSourceTransportInput input;
  input.reaction_coordinate = {0.21, -0.13, 0.08};
  input.required_matter_impulse = {0.02, -0.03, 0.01};
  input.source_anchor = {4, 4, 4};
  input.source_remainder = {0.17, -0.11, 0.09};
  input.residual_amplitude = 1.0;
  input.lattice_size = 11;
  input.charge = 1;

  const auto base = analyze_cubic_reaction_vector_source_transport(input);
  check("base bridge is valid", base.valid());
  check("scalar reaction cannot choose a cubic vector",
        base.scalar_reaction_direction_forbidden_by_cubic_symmetry);
  check("orientation-free carrier requires three canonical pairs",
        base.orientation_free_vector_requires_three_canonical_pairs);
  check("field impulse supplies the nonzero orientation",
        base.orientation_defined_by_field_impulse);
  check("physical momentum equals required field recoil",
        max_abs(base.physical_momentum - input.required_matter_impulse)
            <= input.tolerance);
  check("relativistic energy chart closes",
        base.exact_relativistic_energy_chart
            && base.energy_chart_residual <= input.tolerance);
  check("reaction momentum inverse closes",
        base.reaction_inverse_residual <= input.tolerance);
  check("reaction coordinate inverse closes",
        base.coordinate_inverse_residual <= input.tolerance);
  check("cotangent chart is symplectic", base.cotangent_chart_symplectic);
  check("split angle is fixed by local conservation",
        base.split_angle_fixed_by_local_conservation
            && base.split_angle > 0.0 && base.split_angle < 0.5 * PI);
  check("history plus reaction equals residual energy",
        std::abs(base.history_energy + base.reaction_energy
                 - base.residual_energy) <= input.tolerance);
  check("reaction energy equals required kinetic energy",
        std::abs(base.reaction_energy - base.required_kinetic_energy)
            <= input.tolerance);
  check("split amplitude matches reaction radius",
        base.split_amplitude_residual <= input.tolerance);
  check("free source drift reverses exactly",
        base.exact_reversible_free_transport
            && base.drift_inverse_residual <= input.tolerance);
  check("source speed is causal",
        base.physical_velocity.mag() < input.limiting_speed);
  check("ternary face current obeys exact continuity",
        base.exact_face_current_continuity
            && base.current_segment.charge == 1
            && base.current_continuity_residual <= input.tolerance);
  check("low-energy mass equals E0 over c squared",
        std::abs(base.low_energy_inertial_mass
                 - input.rest_energy
                     / (input.limiting_speed * input.limiting_speed))
            <= input.tolerance);

  double worst_covariance = 0.0;
  double worst_scalar_invariant = 0.0;
  for (int arm = 0; arm < 5; ++arm) {
    auto transformed = input;
    transformed.reaction_coordinate = transform(input.reaction_coordinate, arm);
    transformed.required_matter_impulse = transform(
        input.required_matter_impulse, arm);
    transformed.source_anchor = {4, 4, 4};
    transformed.source_remainder = {};
    const auto result = analyze_cubic_reaction_vector_source_transport(
        transformed);
    check("cubic covariance arm is valid", result.valid());
    worst_covariance = std::max(worst_covariance, max_abs(
        result.physical_momentum
        - transform(base.physical_momentum, arm)));
    worst_scalar_invariant = std::max(worst_scalar_invariant,
        std::abs(result.required_kinetic_energy
                 - base.required_kinetic_energy));
  }
  check("physical momentum is cubic covariant",
        worst_covariance <= input.tolerance);
  check("kinetic energy and split angle are cubic scalars",
        worst_scalar_invariant <= input.tolerance);

  auto zero_input = input;
  zero_input.reaction_coordinate = {};
  zero_input.required_matter_impulse = {};
  const auto zero = analyze_cubic_reaction_vector_source_transport(zero_input);
  check("zero impulse bridge is valid", zero.valid());
  check("zero impulse creates no arbitrary orientation",
        !zero.orientation_defined_by_field_impulse
            && zero.physical_momentum.mag2() == 0.0
            && zero.physical_velocity.mag2() == 0.0);
  check("zero impulse selects eta zero",
        zero.split_angle == 0.0 && zero.reaction_energy == 0.0
            && zero.history_energy == zero.residual_energy);

  auto equal_input = input;
  equal_input.residual_amplitude = 0.4;
  const double equal_reaction_energy =
      0.25 * equal_input.residual_amplitude
      * equal_input.residual_amplitude;
  const double target_total = equal_input.rest_energy
      + equal_reaction_energy;
  const double equal_p = std::sqrt(
      (target_total * target_total
       - equal_input.rest_energy * equal_input.rest_energy)
      / (equal_input.limiting_speed * equal_input.limiting_speed));
  equal_input.required_matter_impulse = {equal_p, 0.0, 0.0};
  const auto equal = analyze_cubic_reaction_vector_source_transport(equal_input);
  check("equal-energy witness is valid", equal.valid());
  check("equal split is eta pi over four only at half residual energy",
        equal.equal_split
            && std::abs(equal.split_angle - 0.25 * PI) <= 2e-12);

  auto insufficient_input = input;
  insufficient_input.residual_amplitude = 1e-6;
  insufficient_input.required_matter_impulse = {1.0, 0.0, 0.0};
  const auto insufficient = analyze_cubic_reaction_vector_source_transport(
      insufficient_input);
  check("insufficient residual energy fails closed",
        insufficient.status
            == CubicReactionSourceTransportStatus::InsufficientResidualEnergy
        && !insufficient.valid());

  auto invalid_mass = input;
  invalid_mass.rest_energy = 0.0;
  check("nonpositive rest energy is rejected",
        analyze_cubic_reaction_vector_source_transport(invalid_mass).status
            == CubicReactionSourceTransportStatus::InvalidRestEnergy);
  auto invalid_charge = input;
  invalid_charge.charge = 0;
  check("nonternary source charge is rejected",
        analyze_cubic_reaction_vector_source_transport(invalid_charge).status
            == CubicReactionSourceTransportStatus::InvalidCharge);
  auto invalid_nan = input;
  invalid_nan.required_matter_impulse.x =
      std::numeric_limits<double>::quiet_NaN();
  check("nonfinite input is rejected",
        analyze_cubic_reaction_vector_source_transport(invalid_nan).status
            == CubicReactionSourceTransportStatus::NonFiniteInput);

  check("mass scale is not claimed derived",
        !base.inertial_mass_scale_derived);
  check("native vector common action remains open",
        !base.native_vector_common_action_supplied);
  check("production remains uncoupled",
        !base.production_coupling_supplied);
  check("Born and Gstar are not read",
        !base.born_target_used
            && !base.native_gstar_synchronization_supplied);
  check("no new selected type is added", !base.new_selected_type_added);

  std::cout << "FTD-0890 cubic reaction-vector/source transport: "
            << (failures == 0 ? "PASS" : "FAIL") << '\n';
  return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
