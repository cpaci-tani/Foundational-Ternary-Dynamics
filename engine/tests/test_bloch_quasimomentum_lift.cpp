#include "ftd/eft/bloch_quasimomentum_lift.h"

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

double max_abs_difference(const ftd::eft::BlochTriplet& left,
                          const ftd::eft::BlochTriplet& right) {
  return std::max({std::abs(left[0] - right[0]),
                   std::abs(left[1] - right[1]),
                   std::abs(left[2] - right[2])});
}

ftd::eft::BlochTriplet signed_cubic_transform(
    const ftd::eft::BlochTriplet& value) {
  return {-value[1], value[0], -value[2]};
}

ftd::eft::BlochWinding signed_cubic_transform(
    const ftd::eft::BlochWinding& value) {
  return {-value[1], value[0], -value[2]};
}

}  // namespace

int main() {
  using namespace ftd::eft;
  const double pi = std::acos(-1.0);
  const double tau = 2.0 * pi;

  BlochQuasimomentumLiftInput input;
  input.principal_first = {3.0 * pi / 4.0, -4.0 * pi / 5.0, pi / 3.0};
  input.winding_first = {2, -1, 0};
  input.principal_second = {pi / 2.0, -pi / 2.0, 5.0 * pi / 6.0};
  input.winding_second = {-1, 3, -2};
  input.momentum_scale = 1.75;
  input.finite_range_order = 8;

  const auto result = analyze_bloch_quasimomentum_lift(input);
  check("Bloch lift reference analysis is valid", result.valid());
  check("principal sum wraps into the selected branch",
        std::abs(result.principal_sum[0] + 3.0 * pi / 4.0)
                <= input.tolerance
            && std::abs(result.principal_sum[1] - 7.0 * pi / 10.0)
                <= input.tolerance
            && std::abs(result.principal_sum[2] + 5.0 * pi / 6.0)
                <= input.tolerance);
  check("zone crossing produces exact reciprocal carries",
        result.principal_carry == BlochWinding{1, -1, 1});
  check("existing winding and carries combine exactly",
        result.combined_winding == BlochWinding{2, 1, -1});
  check("lifted real addition reconstructs exactly",
        result.winding_reconstructs_real_addition
            && result.real_addition_residual <= input.tolerance);
  check("torus quasimomentum addition is exact",
        result.torus_quasimomentum_addition_exact);
  check("principal-only labels lose reciprocal information",
        result.zone_crossing_observed
            && result.principal_only_loses_reciprocal_information
            && std::abs(result.reciprocal_information[0] - 2.0 * tau)
                <= input.tolerance);
  check("finite-range sawtooth witness remains periodic",
        result.finite_range_weight_is_periodic
            && result.periodicity_residual <= 20.0 * input.tolerance);
  check("imposed momentum candidate is scale-linear",
        max_abs_difference(
            result.doubled_scale_momentum_candidate,
            {2.0 * result.physical_momentum_candidate[0],
             2.0 * result.physical_momentum_candidate[1],
             2.0 * result.physical_momentum_candidate[2]})
            <= input.tolerance);

  BlochQuasimomentumLiftInput no_crossing;
  no_crossing.principal_first = {pi / 8.0, -pi / 7.0, pi / 9.0};
  no_crossing.principal_second = {pi / 6.0, pi / 10.0, -pi / 12.0};
  const auto quiet = analyze_bloch_quasimomentum_lift(no_crossing);
  check("no-crossing control is valid", quiet.valid());
  check("no-crossing control has zero carry and winding",
        quiet.principal_carry == BlochWinding{0, 0, 0}
            && quiet.combined_winding == BlochWinding{0, 0, 0}
            && !quiet.zone_crossing_observed
            && !quiet.principal_only_loses_reciprocal_information);

  BlochQuasimomentumLiftInput edge;
  edge.principal_first = {3.0 * pi / 4.0, 0.0, 0.0};
  edge.principal_second = {pi / 4.0, 0.0, 0.0};
  const auto branch_edge = analyze_bloch_quasimomentum_lift(edge);
  check("positive zone edge wraps to negative pi with carry one",
        branch_edge.valid()
            && std::abs(branch_edge.principal_sum[0] + pi) <= edge.tolerance
            && branch_edge.principal_carry[0] == 1);
  check("every finite sawtooth truncation misses the branch edge",
        std::abs(branch_edge.finite_range_sawtooth_weight[0])
                <= 10.0 * edge.tolerance
            && std::abs(branch_edge.finite_range_branch_residual - pi)
                <= 20.0 * edge.tolerance);

  BlochQuasimomentumLiftInput transformed_input = input;
  transformed_input.principal_first = signed_cubic_transform(
      input.principal_first);
  transformed_input.winding_first = signed_cubic_transform(
      input.winding_first);
  transformed_input.principal_second = signed_cubic_transform(
      input.principal_second);
  transformed_input.winding_second = signed_cubic_transform(
      input.winding_second);
  const auto transformed = analyze_bloch_quasimomentum_lift(
      transformed_input);
  check("signed cubic transform remains valid", transformed.valid());
  check("lifted sum is covariant under a signed cubic permutation",
        max_abs_difference(transformed.lifted_sum,
                           signed_cubic_transform(result.lifted_sum))
            <= 20.0 * input.tolerance);
  check("physical candidate is cubic covariant",
        max_abs_difference(
            transformed.physical_momentum_candidate,
            signed_cubic_transform(result.physical_momentum_candidate))
            <= 40.0 * input.tolerance);

  auto nonfinite = input;
  nonfinite.principal_first[0] =
      std::numeric_limits<double>::quiet_NaN();
  check("nonfinite label fails closed",
        analyze_bloch_quasimomentum_lift(nonfinite).status
            == BlochQuasimomentumLiftStatus::NonFiniteInput);
  auto invalid_tolerance = input;
  invalid_tolerance.tolerance = 0.0;
  check("nonpositive tolerance fails closed",
        analyze_bloch_quasimomentum_lift(invalid_tolerance).status
            == BlochQuasimomentumLiftStatus::InvalidTolerance);
  auto invalid_scale = input;
  invalid_scale.momentum_scale = 0.0;
  check("nonpositive momentum scale fails closed",
        analyze_bloch_quasimomentum_lift(invalid_scale).status
            == BlochQuasimomentumLiftStatus::InvalidMomentumScale);
  auto invalid_order = input;
  invalid_order.finite_range_order = 0;
  check("invalid finite-range order fails closed",
        analyze_bloch_quasimomentum_lift(invalid_order).status
            == BlochQuasimomentumLiftStatus::InvalidFiniteRangeOrder);
  auto nonprincipal = input;
  nonprincipal.principal_second[2] = pi;
  check("nonprincipal label fails closed",
        analyze_bloch_quasimomentum_lift(nonprincipal).status
            == BlochQuasimomentumLiftStatus::NonPrincipalLabel);
  auto overflow = input;
  overflow.winding_first[0] = std::numeric_limits<std::int64_t>::max();
  overflow.winding_second[0] = 1;
  check("winding overflow fails closed",
        analyze_bloch_quasimomentum_lift(overflow).status
            == BlochQuasimomentumLiftStatus::WindingOverflow);

  check("no continuous homomorphic torus-to-real section is claimed",
        !result.global_continuous_homomorphic_section_exists);
  check("no finite-range global unwrapped generator is claimed",
        !result.finite_range_global_unwrapped_generator_exists
            && !result.exact_principal_generator_is_finite_range);
  check("winding dynamics and physical momentum scale remain open",
        !result.winding_dynamics_derived
            && !result.physical_momentum_scale_derived);
  check("total field-matter momentum and absolute mass remain open",
        !result.total_field_matter_momentum_map_derived
            && !result.absolute_mass_derived);
  check("local stress route is not ruled out",
        !result.local_stress_route_ruled_out);
  check("production, Born, and native Gstar remain untouched",
        !result.production_coupling_supplied
            && !result.born_target_used
            && !result.native_gstar_synchronization_supplied);
  check("no new selected vector type is added",
        !result.new_selected_vector_type_added);

  std::cout << "FTD-0894/0896 Bloch quasimomentum lift: "
            << (failures == 0 ? "PASS" : "FAIL") << '\n';
  return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
