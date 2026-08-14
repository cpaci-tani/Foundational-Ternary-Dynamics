#include "ftd/eft/dressed_boost_momentum_map.h"

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

double max_abs(const ftd::Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

ftd::Vec3 cubic_transform(const ftd::Vec3& value) {
  return {-value.y, value.x, -value.z};
}

}  // namespace

int main() {
  using namespace ftd;
  using namespace ftd::eft;

  DressedBoostMomentumMapInput input;
  input.matter_cost = 2.0;
  input.field_cost = 3.0;
  input.kinetic_coupling = 1.0;
  input.matter_momentum_weight = 1.0;
  input.field_momentum_weight = 1.0;
  input.total_momentum = {0.4, -0.3, 0.2};
  input.static_energy_offset = -1.5;
  input.momentum_map_scale = 2.0;

  const auto result = analyze_dressed_boost_momentum_map(input);
  check("conditional dressed-boost analysis is valid", result.valid());
  check("energy Hessian is positive definite",
        result.energy_hessian_positive_definite
            && std::abs(result.energy_hessian_determinant - 5.0)
                <= input.tolerance);
  check("momentum row has one rank per cubic axis",
        result.momentum_map_rank_one_per_axis);
  check("conditional mass equals three fifths",
        std::abs(result.dressed_inertial_mass - 0.6) <= input.tolerance);
  check("matter allocation is two thirds of total momentum",
        max_abs(result.matter_odd_amplitude
                - input.total_momentum * (2.0 / 3.0)) <= input.tolerance);
  check("field allocation is one third of total momentum",
        max_abs(result.field_odd_amplitude
                - input.total_momentum * (1.0 / 3.0)) <= input.tolerance);
  check("momentum constraint closes",
        result.momentum_residual <= input.tolerance
            && max_abs(result.reconstructed_momentum
                       - input.total_momentum) <= input.tolerance);
  check("minimum energy is P squared over two M",
        result.energy_residual <= input.tolerance
            && std::abs(result.minimum_kinetic_energy
                        - input.total_momentum.mag2()
                            / (2.0 * result.dressed_inertial_mass))
                <= input.tolerance);
  check("field-like odd sector participates",
        result.field_odd_sector_participates);
  check("constrained minimum and mass formula are exact conditionally",
        result.unique_constrained_minimum
            && result.exact_conditional_dressed_mass);
  check("momentum-map scaling exposes quadratic mass ambiguity",
        result.momentum_scale_ambiguity_exposed
            && std::abs(result.scaled_momentum_map_mass - 2.4)
                <= input.tolerance);

  auto offset_input = input;
  offset_input.static_energy_offset = 73.0;
  const auto offset = analyze_dressed_boost_momentum_map(offset_input);
  check("static energy offset leaves inertia unchanged",
        offset.valid()
            && std::abs(offset.dressed_inertial_mass
                        - result.dressed_inertial_mass) <= input.tolerance
            && !offset.static_offset_contributes_to_inertia);
  check("static energy offset shifts only total energy",
        std::abs((offset.minimum_total_energy - result.minimum_total_energy)
                 - (offset_input.static_energy_offset
                    - input.static_energy_offset)) <= input.tolerance);

  auto transformed_input = input;
  transformed_input.total_momentum = cubic_transform(input.total_momentum);
  const auto transformed = analyze_dressed_boost_momentum_map(
      transformed_input);
  check("signed cubic transform remains valid", transformed.valid());
  check("conditional mass is a cubic scalar",
        std::abs(transformed.dressed_inertial_mass
                 - result.dressed_inertial_mass) <= input.tolerance);
  check("matter allocation is cubic covariant",
        max_abs(transformed.matter_odd_amplitude
                - cubic_transform(result.matter_odd_amplitude))
            <= input.tolerance);
  check("field allocation is cubic covariant",
        max_abs(transformed.field_odd_amplitude
                - cubic_transform(result.field_odd_amplitude))
            <= input.tolerance);

  DressedBoostMomentumMapInput matter_only;
  matter_only.matter_cost = 0.5;
  matter_only.field_cost = 3.0;
  matter_only.matter_momentum_weight = 1.0;
  matter_only.field_momentum_weight = 0.0;
  matter_only.total_momentum = {0.2, 0.1, -0.4};
  const auto matter = analyze_dressed_boost_momentum_map(matter_only);
  check("matter-only control returns mass two",
        matter.valid()
            && std::abs(matter.dressed_inertial_mass - 2.0)
                <= matter_only.tolerance
            && !matter.field_odd_sector_participates);

  auto independent = matter_only;
  independent.field_cost = 0.25;
  independent.field_momentum_weight = 1.0;
  const auto additive = analyze_dressed_boost_momentum_map(independent);
  check("independent matter and field channels add conditionally",
        additive.valid()
            && std::abs(additive.dressed_inertial_mass - 6.0)
                <= independent.tolerance
            && additive.field_odd_sector_participates);

  auto non_spd = input;
  non_spd.kinetic_coupling = std::sqrt(6.0);
  check("singular energy Hessian fails closed",
        analyze_dressed_boost_momentum_map(non_spd).status
            == DressedBoostMomentumMapStatus::NonPositiveEnergyHessian);
  auto zero_map = input;
  zero_map.matter_momentum_weight = 0.0;
  zero_map.field_momentum_weight = 0.0;
  check("zero momentum map fails closed",
        analyze_dressed_boost_momentum_map(zero_map).status
            == DressedBoostMomentumMapStatus::ZeroMomentumMap);
  auto zero_scale = input;
  zero_scale.momentum_map_scale = 0.0;
  check("zero scale control fails closed",
        analyze_dressed_boost_momentum_map(zero_scale).status
            == DressedBoostMomentumMapStatus::InvalidMomentumMapScale);
  auto invalid_tolerance = input;
  invalid_tolerance.tolerance = 0.0;
  check("nonpositive tolerance fails closed",
        analyze_dressed_boost_momentum_map(invalid_tolerance).status
            == DressedBoostMomentumMapStatus::InvalidTolerance);
  auto nonfinite = input;
  nonfinite.matter_cost = std::numeric_limits<double>::quiet_NaN();
  check("nonfinite input fails closed",
        analyze_dressed_boost_momentum_map(nonfinite).status
            == DressedBoostMomentumMapStatus::NonFiniteInput);

  check("total physical momentum map is not claimed derived",
        !result.total_momentum_map_derived);
  check("absolute mass and common-action Noether closure remain open",
        !result.absolute_mass_derived
            && !result.common_action_noether_closure);
  check("stable pole and production remain open",
        !result.stable_matter_pole_derived
            && !result.production_coupling_supplied);
  check("Born and native Gstar are not read",
        !result.born_target_used
            && !result.native_gstar_synchronization_supplied);
  check("no new selected vector type is added",
        !result.new_selected_vector_type_added);

  std::cout << "FTD-0893 dressed boost momentum map: "
            << (failures == 0 ? "PASS" : "FAIL") << '\n';
  return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
