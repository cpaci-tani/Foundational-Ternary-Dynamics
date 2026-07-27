/** FTD-0586: endogenous reaction-carrier/autocatalysis bound. */

#include "ftd/eft/endogenous_reaction_carrier_bound.h"

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
  const auto r = ftd::eft::analyze_endogenous_reaction_carrier_bound();
  check("four registered spectral volumes are stable",
        r.modal_step_bound_derived && r.spectral_volume_count == 4);
  check("rectangular source histories have the locked pulse bound",
        r.rectangular_pulse_bound_derived
        && r.maximum_single_source_pulse_bound > 0.0);
  check("three endogenous sources remain subcritical",
        r.no_first_genesis_for_three_sources
        && r.maximum_three_source_pulse_bound < 1.5163860591519780
        && r.minimum_threshold_margin > 0.0);
  check("bound closes at three and is inconclusive from four",
        r.maximum_initial_sources_closed == 3
        && r.minimum_sources_not_excluded == 4
        && !r.four_sources_sufficient);
  check("96 live endogenous arms execute",
        r.endogenous_arms == 96 && r.endogenous_ticks == 12288
        && r.constant_source_arms == 48 && r.pulse_source_arms == 48);
  check("live support never creates a first new site",
        r.endogenous_genesis_events == 0
        && r.manifested_support_remained_subset
        && r.maximum_bound_excess <= 1e-12);
  check("evaporation pulse and sanitized kinematics are exercised",
        r.pulse_removal_exercised && r.endogenous_evaporation_events > 0
        && r.void_kinematics_sanitized
        && r.maximum_velocity == 0.0 && r.maximum_remainder == 0.0);
  check("history journal is state and RNG neutral", r.observer_neutral);
  check("external supercritical genesis controls are live",
        r.external_genesis_control_live && r.external_control_arms == 4
        && r.external_control_genesis_events >= 4);
  check("no self-sustaining carrier or production change is promoted",
        !r.self_sustaining_reaction_carrier_established
        && !r.production_changed);
  check("registered FTD-0586 verdict closes", r.valid);

  std::cout.precision(17);
  for (const auto& volume : r.volumes) {
    std::cout << "L" << volume.lattice_size
      << "_maximum_mode_eigenvalue=" << volume.maximum_mode_eigenvalue << '\n'
      << "L" << volume.lattice_size
      << "_single_source_step_bound=" << volume.single_source_step_bound << '\n'
      << "L" << volume.lattice_size
      << "_single_source_pulse_bound=" << volume.single_source_pulse_bound << '\n'
      << "L" << volume.lattice_size
      << "_three_source_pulse_bound=" << volume.three_source_pulse_bound << '\n'
      << "L" << volume.lattice_size
      << "_threshold_margin=" << volume.threshold_margin << '\n';
  }
  std::cout << "endogenous_arms=" << r.endogenous_arms << '\n'
    << "endogenous_ticks=" << r.endogenous_ticks << '\n'
    << "constant_source_arms=" << r.constant_source_arms << '\n'
    << "pulse_source_arms=" << r.pulse_source_arms << '\n'
    << "endogenous_genesis_events=" << r.endogenous_genesis_events << '\n'
    << "endogenous_evaporation_events=" << r.endogenous_evaporation_events << '\n'
    << "external_control_genesis_events="
    << r.external_control_genesis_events << '\n'
    << "maximum_observed_flux=" << r.maximum_observed_flux << '\n'
    << "maximum_bound_excess=" << r.maximum_bound_excess << '\n'
    << "maximum_velocity=" << r.maximum_velocity << '\n'
    << "maximum_remainder=" << r.maximum_remainder << '\n'
    << "maximum_single_source_pulse_bound="
    << r.maximum_single_source_pulse_bound << '\n'
    << "maximum_three_source_pulse_bound="
    << r.maximum_three_source_pulse_bound << '\n'
    << "minimum_threshold_margin=" << r.minimum_threshold_margin << '\n'
    << "minimum_sources_not_excluded="
    << r.minimum_sources_not_excluded << '\n'
    << "endogenous_reaction_carrier_bound failures=" << failures << '\n'
    << "verdict=ENDOGENOUS_N_LE_3_AUTOCATALYSIS_CLOSED_"
       "BOUND_INCONCLUSIVE_AT_N_GE_4\n";
  return failures == 0 ? 0 : 1;
}
