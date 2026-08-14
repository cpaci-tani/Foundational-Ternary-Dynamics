#include "ftd/eft/quartic_relative_carry_gearbox.h"

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

bool close(double left, double right, double tolerance = 1e-10) {
  return std::abs(left - right)
      <= tolerance * std::max({1.0, std::abs(left), std::abs(right)});
}

}  // namespace

int main() {
  using namespace ftd::eft;
  const double pi = std::acos(-1.0);
  const double root_two = std::sqrt(2.0);

  QuarticRelativeCarryInput input;
  input.relative_state = {0.7, 0.0};
  input.relative_parameters.mass = 1.7;
  input.relative_parameters.coupling = 0.8;
  input.relative_parameters.step = 0.025;
  input.relative_parameters.residual_tolerance = 2e-14;
  input.relative_parameters.max_iterations = 96;
  input.momentum_scale = 0.01;

  const auto preview = advance_native_pair_energy(
      input.relative_state, input.relative_parameters);
  const double preview_increment = preview.after.momentum
      / (root_two * input.momentum_scale);
  input.common_momentum = root_two * input.momentum_scale
      * (pi - std::abs(preview_increment) / 2.0);

  const auto result = analyze_quartic_relative_carry_gearbox(input);
  check("quartic-relative carry gearbox is valid", result.valid());
  check("relative recursion generates the channel increment",
        result.relative_increment_derived_inside_selected_recursion
            && close(result.generated_dimensionless_increment,
                     preview_increment));
  check("channel impulses are exactly equal and opposite",
        result.channel_impulses_equal_and_opposite
            && close(result.common_momentum_before,
                     result.common_momentum_after));
  check("selected relative energy closes exactly",
        result.relative_energy_exact
            && std::abs(result.relative_energy_residual) <= 1e-11);
  check("zone crossing is retained by reciprocal carry",
        result.reciprocal_carry_composition_exact
            && (std::any_of(result.carry_step.carry_first.begin(),
                            result.carry_step.carry_first.end(),
                            [](const std::int64_t carry) {
                              return carry != 0;
                            })
                || std::any_of(result.carry_step.carry_second.begin(),
                               result.carry_step.carry_second.end(),
                               [](const std::int64_t carry) {
                                 return carry != 0;
                               })));
  check("independent endpoint chart matches composed carry",
        result.chart_endpoint_residual <= input.tolerance);
  check("signed-step inverse recovers the complete state",
        result.full_state_reversal_exact
            && result.reverse_residual <= input.tolerance);
  const double expected_period_product = std::sqrt(pi)
      * std::tgamma(0.25) / std::tgamma(0.75)
      * std::sqrt(input.relative_parameters.mass
                  / (2.0 * input.relative_parameters.coupling));
  check("continuum Gstar period factor is the same quartic invariant",
        result.continuum_gstar_period_factor_exact
            && close(result.continuum_period_amplitude_product,
                     expected_period_product, 1e-14));

  QuarticRelativeCarryInput no_crossing = input;
  no_crossing.common_momentum = 0.0;
  no_crossing.momentum_scale = 10.0;
  const auto quiet = analyze_quartic_relative_carry_gearbox(no_crossing);
  check("no-crossing control remains valid", quiet.valid());
  check("no-crossing control has zero carry",
        quiet.carry_step.carry_first
                == ReciprocalCarryTriplet{0, 0, 0}
            && quiet.carry_step.carry_second
                == ReciprocalCarryTriplet{0, 0, 0});

  QuarticRelativeCarryInput multi_zone = input;
  multi_zone.common_momentum = 0.0;
  multi_zone.momentum_scale = std::abs(preview.after.momentum)
      / (root_two * 4.5 * pi);
  const auto large = analyze_quartic_relative_carry_gearbox(multi_zone);
  check("multiple-zone generated impulse remains valid", large.valid());
  check("multiple-zone generated impulse is recorded exactly",
        large.carry_step.multi_zone_increment_supported
            && large.reciprocal_carry_composition_exact
            && large.full_state_reversal_exact);

  auto nonfinite = input;
  nonfinite.common_momentum =
      std::numeric_limits<double>::quiet_NaN();
  check("nonfinite common momentum fails closed",
        analyze_quartic_relative_carry_gearbox(nonfinite).status
            == QuarticRelativeCarryStatus::NonFiniteInput);
  auto invalid_tolerance = input;
  invalid_tolerance.tolerance = 0.0;
  check("nonpositive tolerance fails closed",
        analyze_quartic_relative_carry_gearbox(invalid_tolerance).status
            == QuarticRelativeCarryStatus::InvalidTolerance);
  auto invalid_scale = input;
  invalid_scale.momentum_scale = 0.0;
  check("nonpositive momentum scale fails closed",
        analyze_quartic_relative_carry_gearbox(invalid_scale).status
            == QuarticRelativeCarryStatus::InvalidMomentumScale);
  auto invalid_relative = input;
  invalid_relative.relative_parameters.coupling = 0.0;
  check("invalid relative recursion fails closed",
        analyze_quartic_relative_carry_gearbox(invalid_relative).status
            == QuarticRelativeCarryStatus::RelativeStepFailure);
  auto chart_overflow = input;
  chart_overflow.common_momentum =
      std::numeric_limits<double>::max() / 4.0;
  chart_overflow.momentum_scale =
      std::numeric_limits<double>::min();
  check("unrepresentable chart winding fails closed",
        analyze_quartic_relative_carry_gearbox(chart_overflow).status
            == QuarticRelativeCarryStatus::ChartCarryOutOfRange);

  check("common coupling and matter-field identification remain open",
        !result.common_mode_coupling_derived
            && !result.matter_field_identification_derived);
  check("momentum scale and integer-tick Gstar cadence remain open",
        !result.physical_momentum_scale_derived
            && !result.integer_tick_gstar_cadence_derived);
  check("carry energy and absolute mass remain open",
        !result.carry_energy_law_derived
            && !result.absolute_mass_derived);
  check("production and Born remain untouched",
        !result.production_coupling_supplied && !result.born_target_used);
  check("no new selected type is added", !result.new_selected_type_added);

  std::cout << "FTD-0898 quartic relative carry gearbox: "
            << (failures == 0 ? "PASS" : "FAIL") << '\n';
  return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
