#include "ftd/eft/collective_reaction_triplet_inertia.h"

#include <algorithm>
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

ftd::Vec3 spatial_transform(const ftd::Vec3& value) {
  return {-value.y, value.x, value.z};
}

ftd::eft::CollectiveReactionTripletInput make_input() {
  using ftd::Vec3;
  ftd::eft::CollectiveReactionTripletInput input;
  input.positions = {
      Vec3{1.0, 2.0, -1.0}, Vec3{-2.0, 1.0, 3.0},
      Vec3{0.5, -1.5, 2.0}, Vec3{2.5, 0.0, -2.0}};
  input.momenta = {
      Vec3{0.2, -0.1, 0.3}, Vec3{-0.4, 0.2, 0.1},
      Vec3{0.1, 0.3, -0.2}, Vec3{0.5, -0.2, 0.4}};
  input.position_tangents = {
      Vec3{0.3, -0.2, 0.1}, Vec3{-0.1, 0.4, 0.2},
      Vec3{0.2, 0.1, -0.3}, Vec3{-0.4, 0.2, 0.5}};
  input.constituent_rest_energies = {0.5, 0.75, 1.0, 1.25};
  input.constituent_impulses = {
      Vec3{0.1, 0.2, -0.1}, Vec3{-0.3, 0.0, 0.4},
      Vec3{0.2, -0.5, -0.2}, Vec3{0.0, 0.3, -0.1}};
  input.static_binding_offset = -0.2;
  return input;
}

}  // namespace

int main() {
  using namespace ftd;
  using namespace ftd::eft;

  const auto input = make_input();
  const auto result = analyze_collective_reaction_triplet_inertia(input);
  check("collective analysis is valid", result.valid());
  check("center is arithmetic constituent mean",
        max_abs(result.center - Vec3{0.5, 0.375, 0.5}) <= input.tolerance);
  check("total momentum is exact constituent sum",
        max_abs(result.total_momentum - Vec3{0.4, 0.2, 0.6})
            <= input.tolerance);
  check("Helmert sector preserves canonical one-form",
        result.exact_collective_symplectic_sector
            && result.one_form_residual <= input.tolerance);
  check("the collective sector is exactly three canonical pairs",
        result.three_collective_canonical_pairs);
  check("position reconstruction closes",
        result.position_reconstruction_residual <= input.tolerance);
  check("momentum reconstruction closes",
        result.momentum_reconstruction_residual <= input.tolerance);
  check("internal pair impulses cancel exactly",
        result.internal_zero_sum_impulses_cancel
            && result.summed_constituent_impulse.mag() <= input.tolerance);
  check("summed impulses update collective momentum exactly",
        result.external_impulses_sum_to_collective_kick
            && max_abs(result.momentum_after_impulse
                       - result.total_momentum) <= input.tolerance);
  check("selected constituent dispersion is strictly convex",
        result.constituent_dispersion_strictly_convex);
  check("minimum-energy constituents have one common velocity",
        result.common_velocity_residual <= input.tolerance);
  check("minimum constituent energy equals composite dispersion",
        result.exact_conditional_composite_dispersion
            && result.composite_energy_residual <= input.tolerance);

  const double expected_rest = 3.5;
  const double c2 = input.limiting_speed * input.limiting_speed;
  check("summed rest energy is exact",
        std::abs(result.summed_rest_energy - expected_rest) <= input.tolerance);
  check("conditional composite inertia is summed rest energy over c squared",
        std::abs(result.collective_inertial_mass - expected_rest / c2)
            <= input.tolerance);
  check("zero-momentum curvature is inverse collective mass",
        std::abs(result.zero_momentum_energy_curvature
                 * result.collective_inertial_mass - 1.0)
            <= input.tolerance);
  check("static binding offset exposes rest-inertia mismatch",
        std::abs(result.static_offset_mass_mismatch
                 - input.static_binding_offset / c2) <= input.tolerance
            && !result.static_binding_offset_participates_in_boost);

  auto external_input = input;
  external_input.constituent_impulses[0] += Vec3{0.25, -0.5, 0.75};
  const auto external = analyze_collective_reaction_triplet_inertia(
      external_input);
  check("nonzero external impulse analysis is valid", external.valid());
  check("nonzero external impulse equals collective kick",
        max_abs(external.momentum_after_impulse
                - external.total_momentum - Vec3{0.25, -0.5, 0.75})
            <= external_input.tolerance);

  auto transformed_input = input;
  for (auto* values : {&transformed_input.positions,
                       &transformed_input.momenta,
                       &transformed_input.position_tangents,
                       &transformed_input.constituent_impulses}) {
    for (auto& value : *values) value = spatial_transform(value);
  }
  const auto transformed = analyze_collective_reaction_triplet_inertia(
      transformed_input);
  check("signed spatial transform remains valid", transformed.valid());
  check("center is cubic covariant",
        max_abs(transformed.center - spatial_transform(result.center))
            <= input.tolerance);
  check("collective momentum is cubic covariant",
        max_abs(transformed.total_momentum
                - spatial_transform(result.total_momentum))
            <= input.tolerance);
  check("composite energy is a cubic scalar",
        std::abs(transformed.collective_dispersion_energy
                 - result.collective_dispersion_energy) <= input.tolerance);

  auto identical_input = input;
  identical_input.constituent_rest_energies.assign(4, 0.8);
  const auto identical = analyze_collective_reaction_triplet_inertia(
      identical_input);
  check("identical-constituent witness is valid", identical.valid());
  check("identical constituent inertias add conditionally",
        std::abs(identical.collective_inertial_mass - 4.0 * 0.8 / c2)
            <= input.tolerance && identical.conditional_inertial_additivity);

  CollectiveReactionTripletInput empty;
  check("empty constituent set is rejected",
        analyze_collective_reaction_triplet_inertia(empty).status
            == CollectiveReactionTripletStatus::EmptyConstituentSet);
  auto mismatch = input;
  mismatch.position_tangents.pop_back();
  check("mismatched constituent arrays are rejected",
        analyze_collective_reaction_triplet_inertia(mismatch).status
            == CollectiveReactionTripletStatus::SizeMismatch);
  auto invalid_rest = input;
  invalid_rest.constituent_rest_energies[1] = 0.0;
  check("nonpositive constituent rest energy is rejected",
        analyze_collective_reaction_triplet_inertia(invalid_rest).status
            == CollectiveReactionTripletStatus::InvalidRestEnergy);
  auto invalid_speed = input;
  invalid_speed.limiting_speed = 0.0;
  check("nonpositive limiting speed is rejected",
        analyze_collective_reaction_triplet_inertia(invalid_speed).status
            == CollectiveReactionTripletStatus::InvalidSpeed);
  auto invalid_finite = input;
  invalid_finite.momenta[0].x = std::numeric_limits<double>::quiet_NaN();
  check("nonfinite constituent data are rejected",
        analyze_collective_reaction_triplet_inertia(invalid_finite).status
            == CollectiveReactionTripletStatus::NonFiniteInput);

  check("static Hessian is not claimed to determine inertia",
        !result.static_hessian_determines_inertia);
  check("rest energy alone is not claimed to fix dispersion curvature",
        !result.rest_energy_alone_determines_dispersion_curvature);
  check("absolute mass scale is not claimed derived",
        !result.absolute_mass_scale_derived);
  check("exact total field-matter Noether momentum remains open",
        !result.exact_total_field_matter_noether_momentum_supplied);
  check("constituent phase space and stable pole remain unclaimed",
        !result.constituent_phase_space_derived
            && !result.stable_matter_pole_derived);
  check("production remains uncoupled", !result.production_coupling_supplied);
  check("Born and Gstar are not read",
        !result.born_target_used
            && !result.native_gstar_synchronization_supplied);
  check("no new selected vector type is added",
        !result.new_selected_vector_type_added);

  std::cout << "FTD-0892 collective reaction triplet/inertia: "
            << (failures == 0 ? "PASS" : "FAIL") << '\n';
  return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
