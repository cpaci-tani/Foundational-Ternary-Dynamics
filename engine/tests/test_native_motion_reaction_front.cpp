/** FTD-0585: native transport/reaction/source-memory discriminator. */

#include "ftd/eft/native_motion_reaction_front.h"

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
  const auto r = ftd::eft::analyze_native_motion_reaction_front();
  check("reaction-free rest matter has invariant zero kinematics",
        r.reaction_free_zero_kinematics_invariant
        && r.reaction_free_rest_arms == 12
        && r.reaction_free_rest_ticks == 384
        && r.maximum_rest_velocity == 0.0
        && r.maximum_rest_remainder == 0.0
        && r.maximum_rest_displacement == 0.0
        && r.ballistic_control_arms == 12
        && r.minimum_ballistic_hops >= 3);
  check("identical endpoint snapshots admit current or reaction ledgers",
        r.same_snapshot_admits_transport_or_reaction_decomposition
        && r.transport_fixtures == 36
        && r.reaction_front_fixtures == 36
        && r.moment_identity_samples == 72
        && r.maximum_continuity_residual <= 1e-12
        && r.maximum_charge_balance_residual <= 1e-12
        && r.maximum_first_moment_residual <= 1e-12
        && r.maximum_snapshot_difference == 0.0);
  check("balanced reaction source is not local transport or a worldline",
        !r.globally_balanced_reaction_source_is_local_current
        && !r.support_translation_implies_particle_worldline);
  check("evaporation/genesis round trip reuses hidden voxel kinematics",
        r.evaporation_preserves_hidden_kinematics
        && r.genesis_reuses_hidden_kinematics
        && r.stale_kinematics_arms == 12
        && r.maximum_evaporation_ticks > 0
        && r.maximum_evaporation_ticks <= 256
        && r.maximum_stale_velocity_residual == 0.0
        && r.maximum_stale_remainder_residual == 0.0);
  check("no selected force is promoted to a common action",
        !r.selected_force_is_common_action && !r.production_changed);
  check("registered FTD-0585 verdict closes", r.valid);

  std::cout.precision(17);
  std::cout << "reaction_free_rest_arms=" << r.reaction_free_rest_arms << '\n'
    << "reaction_free_rest_ticks=" << r.reaction_free_rest_ticks << '\n'
    << "ballistic_control_arms=" << r.ballistic_control_arms << '\n'
    << "minimum_ballistic_hops=" << r.minimum_ballistic_hops << '\n'
    << "transport_fixtures=" << r.transport_fixtures << '\n'
    << "reaction_front_fixtures=" << r.reaction_front_fixtures << '\n'
    << "moment_identity_samples=" << r.moment_identity_samples << '\n'
    << "stale_kinematics_arms=" << r.stale_kinematics_arms << '\n'
    << "maximum_evaporation_ticks=" << r.maximum_evaporation_ticks << '\n'
    << "maximum_rest_velocity=" << r.maximum_rest_velocity << '\n'
    << "maximum_rest_remainder=" << r.maximum_rest_remainder << '\n'
    << "maximum_rest_displacement=" << r.maximum_rest_displacement << '\n'
    << "maximum_continuity_residual=" << r.maximum_continuity_residual << '\n'
    << "maximum_charge_balance_residual="
    << r.maximum_charge_balance_residual << '\n'
    << "maximum_first_moment_residual="
    << r.maximum_first_moment_residual << '\n'
    << "maximum_snapshot_difference=" << r.maximum_snapshot_difference << '\n'
    << "maximum_stale_velocity_residual="
    << r.maximum_stale_velocity_residual << '\n'
    << "maximum_stale_remainder_residual="
    << r.maximum_stale_remainder_residual << '\n'
    << "native_motion_reaction_front failures=" << failures << '\n'
    << "verdict=TRANSPORT_REACTION_FRONT_AND_STALE_MEMORY_DISTINGUISHED_"
       "RECIPROCAL_NATIVE_PARTICLE_MOTION_STILL_CLOSED\n";
  return failures == 0 ? 0 : 1;
}
