/** FTD-0582: frozen native active-mode backreaction discriminator. */

#include "ftd/eft/native_active_mode_backreaction.h"

#include <iostream>
#include <string>

namespace {
int failures = 0;
void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}
}

int main() {
  const auto r = ftd::eft::analyze_native_active_mode_backreaction();
  check("production source graph is one-way with selected forces disabled",
        r.source_graph_one_way);
  check("all 144 energetic phase arms evolve fields but leave matter exact",
        r.active_native_backreaction_absent
        && r.active_mode_arms == 144
        && r.active_mode_ticks == 18432
        && r.active_field_changed_arms == 144
        && r.maximum_initial_energy_relative_residual <= 1e-12
        && r.minimum_initial_energy_to_barrier_ratio >= 2.0 * (1.0 - 1e-12)
        && r.maximum_native_velocity_response == 0.0
        && r.maximum_native_remainder_response == 0.0
        && r.maximum_native_anchor_displacement == 0.0
        && r.maximum_native_movement_events == 0);
  check("ballistic, selected-force, and source-coupling controls respond",
        r.sensitivity_controls_pass
        && r.ballistic_arms == 12
        && r.minimum_ballistic_movement_events >= 3
        && r.maximum_ballistic_speed_residual <= 1e-12
        && r.maximum_ballistic_reaction_events == 0
        && r.selected_force_control_arms == 4
        && r.minimum_selected_force_response > 1e-12
        && r.maximum_selected_force_mirror_residual <= 1e-12
        && r.coupling_control_pairs == 6
        && r.coupling_control_differences == 6);
  check("no common-action branch or production behavior is promoted",
        !r.native_common_action_implemented && !r.production_changed);
  check("registered FTD-0582 verdict closes", r.valid);

  std::cout.precision(17);
  std::cout << "active_mode_arms=" << r.active_mode_arms << '\n'
    << "active_mode_ticks=" << r.active_mode_ticks << '\n'
    << "active_field_changed_arms=" << r.active_field_changed_arms << '\n'
    << "ballistic_arms=" << r.ballistic_arms << '\n'
    << "selected_force_control_arms=" << r.selected_force_control_arms << '\n'
    << "coupling_control_pairs=" << r.coupling_control_pairs << '\n'
    << "coupling_control_differences="
    << r.coupling_control_differences << '\n'
    << "maximum_initial_energy_relative_residual="
    << r.maximum_initial_energy_relative_residual << '\n'
    << "minimum_initial_energy_to_barrier_ratio="
    << r.minimum_initial_energy_to_barrier_ratio << '\n'
    << "maximum_native_velocity_response="
    << r.maximum_native_velocity_response << '\n'
    << "maximum_native_remainder_response="
    << r.maximum_native_remainder_response << '\n'
    << "maximum_native_anchor_displacement="
    << r.maximum_native_anchor_displacement << '\n'
    << "maximum_native_movement_events="
    << r.maximum_native_movement_events << '\n'
    << "minimum_ballistic_movement_events="
    << r.minimum_ballistic_movement_events << '\n'
    << "maximum_ballistic_speed_residual="
    << r.maximum_ballistic_speed_residual << '\n'
    << "maximum_ballistic_reaction_events="
    << r.maximum_ballistic_reaction_events << '\n'
    << "minimum_selected_force_response="
    << r.minimum_selected_force_response << '\n'
    << "maximum_selected_force_mirror_residual="
    << r.maximum_selected_force_mirror_residual << '\n'
    << "native_active_mode_backreaction failures=" << failures << '\n'
    << "verdict=FROZEN_NATIVE_FIELD_IS_ONE_WAY_TO_MATTER_ACTIVE_TRAVERSAL_CLOSED\n";
  return failures == 0 ? 0 : 1;
}

