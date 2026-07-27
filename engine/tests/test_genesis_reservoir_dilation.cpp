/** FTD-0569: exact one-event genesis reservoir dilation and cycle obstruction. */

#include "ftd/eft/genesis_reservoir_dilation.h"

#include <cmath>
#include <iostream>
#include <string>

namespace {

int failures = 0;

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

}  // namespace

int main() {
  const auto result = ftd::eft::analyze_genesis_reservoir_dilation();

  check("540 accepted single-path arms executed",
        result.accepted_single_arms == 540);
  check("accepted canonical genesis is conditionally invertible",
        result.accepted_genesis_conditionally_invertible
        && result.maximum_genesis_inverse_residual <= 1e-12);
  check("unit kinetic drain has an exact wave-state collision",
        result.unit_drain_has_wave_collision);
  check("one-step Bernoulli phase dilation is exact",
        result.one_step_bernoulli_dilation_exact
        && result.bernoulli_arms == 16
        && result.maximum_bernoulli_inverse_residual <= 1e-15);
  check("twenty erased trials require twenty retained branch bits",
        result.erased_trials_require_unbounded_history
        && result.history_depth == 20
        && result.erased_preimages_at_depth == 1048576
        && result.minimum_history_bits_at_depth == 20
        && result.maximum_history_inverse_residual <= 1e-15);
  check("production evaporation is not the inverse genesis event",
        result.evaporation_is_not_genesis_inverse
        && std::abs(result.minimum_evaporation_composition_flux_distance - 1.0)
            <= 1e-12
        && result.maximum_evaporation_flux_distance_residual <= 1e-12
        && result.maximum_evaporation_wave_distance_residual <= 1e-12);
  check("the paired production event kernel violates detailed balance",
        result.production_pair_violates_detailed_balance);
  check("exact energy closure needs a continuous branch-dependent payload",
        result.continuous_energy_payload_required
        && result.maximum_withdrawal_residual <= 1e-12
        && result.maximum_withdrawal_slope_residual <= 1e-12
        && result.withdrawal_span > 0.0);
  check("single and dual genesis have different energy exchange",
        result.dual_and_single_energy_exchange_differ);
  check("no finite local reversible dilation of the frozen cycle exists",
        !result.finite_local_reversible_production_dilation);
  check("only the one-event/open-system dilation survives",
        result.valid && result.one_event_dilation_open_system_only);

  check("invalid Bernoulli phases and probabilities fail closed",
        !ftd::eft::dilate_bernoulli_phase(-0.1, 0.5).valid
        && !ftd::eft::dilate_bernoulli_phase(0.1, 0.0).valid
        && !ftd::eft::dilate_bernoulli_phase(0.1, 1.0).valid
        && std::isnan(ftd::eft::recover_bernoulli_phase(2, 0.5, 0.5)));

  std::cout.precision(17);
  std::cout << "accepted_single_arms=" << result.accepted_single_arms << '\n'
            << "bernoulli_arms=" << result.bernoulli_arms << '\n'
            << "maximum_genesis_inverse_residual="
            << result.maximum_genesis_inverse_residual << '\n'
            << "maximum_bernoulli_inverse_residual="
            << result.maximum_bernoulli_inverse_residual << '\n'
            << "maximum_history_inverse_residual="
            << result.maximum_history_inverse_residual << '\n'
            << "maximum_withdrawal_residual="
            << result.maximum_withdrawal_residual << '\n'
            << "withdrawal_span=" << result.withdrawal_span << '\n'
            << "erased_preimages_at_depth="
            << result.erased_preimages_at_depth << '\n'
            << "minimum_history_bits_at_depth="
            << result.minimum_history_bits_at_depth << '\n'
            << "genesis_reservoir_dilation failures=" << failures << '\n'
            << "verdict=ONE_EVENT_DILATION_OPEN_SYSTEM_ONLY\n";
  return failures == 0 ? 0 : 1;
}
