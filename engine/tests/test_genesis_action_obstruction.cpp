/** FTD-0567: production genesis does not lock amplitude or share the written action. */

#include "ftd/eft/genesis_action_obstruction.h"

#include <iomanip>
#include <iostream>
#include <string>

int main() {
  int failures=0;
  const auto check=[&](const std::string& label,bool condition) {
    if (!condition) {
      std::cerr << "FAIL: " << label << '\n';
      ++failures;
    }
  };
  const auto result=ftd::eft::analyze_genesis_action_obstruction();
  check("observer verdict",result.valid);
  check("locked arms",result.arms.size()==48);
  check("single map preserves overshoot",result.single_map_preserves_overshoot);
  check("no post-genesis amplitude lock",result.no_post_genesis_amplitude_lock);
  check("four distinct post amplitudes",result.distinct_single_post_amplitudes==4);
  check("no fixed ternary energy quantum",result.no_fixed_ternary_energy_quantum);
  check("acceptance conditioning does not lock",result.acceptance_conditioning_does_not_lock);
  check("dual has no latent heat payment",result.dual_branch_has_no_latent_heat_payment);
  check("evaporation collapses signed preimages",result.evaporation_signed_preimages_collapse);
  check("written action misses magnitude gate",result.written_action_cannot_generate_magnitude_gate);
  check("written action polarity degeneracy",result.written_action_zero_divergence_polarity_degenerate);
  check("frozen common-action route closed",result.frozen_common_action_route_closed);
  check("extended route remains open",result.extended_reservoir_or_open_system_remains_open);
  check("amplitude residual",result.maximum_amplitude_residual<=1e-12);
  check("flux energy residual",result.maximum_flux_energy_residual<=1e-12);
  check("wave energy residual",result.maximum_wave_energy_residual<=1e-12);
  check("polarity scalar residual",result.maximum_polarity_scalar_residual<=1e-12);
  check("action threshold residual",result.maximum_action_threshold_residual<=1e-12);
  check("fixed quantum spread nonzero",result.fixed_quantum_energy_spread>1e-12);

  std::cout << std::setprecision(17)
            << "arms=" << result.arms.size() << '\n'
            << "distinct_single_post_amplitudes="
            << result.distinct_single_post_amplitudes << '\n'
            << "maximum_amplitude_residual="
            << result.maximum_amplitude_residual << '\n'
            << "maximum_flux_energy_residual="
            << result.maximum_flux_energy_residual << '\n'
            << "maximum_wave_energy_residual="
            << result.maximum_wave_energy_residual << '\n'
            << "maximum_polarity_scalar_residual="
            << result.maximum_polarity_scalar_residual << '\n'
            << "maximum_action_threshold_residual="
            << result.maximum_action_threshold_residual << '\n'
            << "fixed_quantum_energy_spread="
            << result.fixed_quantum_energy_spread << '\n'
            << "verdict="
            << (result.valid ? "GENESIS_ACTION_OBSTRUCTION"
                             : "GENESIS_ACTION_TEST_FAILED") << '\n'
            << "failures=" << failures << '\n';
  return failures==0 ? 0 : 1;
}
