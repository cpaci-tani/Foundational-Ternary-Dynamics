#include "ftd/eft/site_ontic_atomic_reciprocal_hop.h"

#include <cmath>
#include <iostream>
#include <string>

int main() {
  const auto result =
      ftd::eft::analyze_site_ontic_atomic_reciprocal_hop();
  const auto& arm = result.decisive_result;
  long double q_norm2 = 0.0L;
  long double source_norm2 = 0.0L;
  for (std::size_t i = 0; i < arm.source.size(); ++i) {
    const ftd::Vec3 q = arm.deposited.current_start[i]
        + arm.deposited.current_end[i];
    q_norm2 += q.dot(q);
    source_norm2 += arm.source[i].dot(arm.source[i]);
  }

  std::cout.precision(17);
  std::cout
      << "protocol_sha256="
      << "DDD146E19C06E488C584AFBAB4092FB802E72F4DFC13F12407A5A914704E8886\n"
      << "verdict=" << result.verdict << '\n'
      << "decisive_arm=" << result.decisive_arm << '\n'
      << "failure_gate=" << arm.failure_gate << '\n'
      << "arms_attempted=" << result.arms_attempted << '\n'
      << "arms_passed=" << result.arms_passed << '\n'
      << "roots=" << arm.forward_root.admitted_roots << '\n'
      << "converged_starts=" << arm.forward_root.converged_starts << '\n'
      << "root_residual=" << arm.forward_root.residual << '\n'
      << "jacobian_condition=" << arm.forward_root.jacobian_condition << '\n'
      << "continuity_residual=" << arm.continuity_residual << '\n'
      << "current_reconstruction_residual="
      << arm.deposited.current_reconstruction_residual << '\n'
      << "q_norm=" << std::sqrt(static_cast<double>(q_norm2)) << '\n'
      << "source_norm=" << std::sqrt(static_cast<double>(source_norm2)) << '\n'
      << "recoil_residual=" << arm.recoil_residual << '\n'
      << "kinematic_residual=" << arm.kinematic_residual << '\n'
      << "energy_relative_residual=" << arm.energy_relative_residual << '\n'
      << "work_relative_residual=" << arm.work_relative_residual << '\n'
      << "total_energy_before=" << arm.total_energy_before << '\n'
      << "total_energy_after=" << arm.total_energy_after << '\n'
      << "particle_energy_before=" << arm.particle_energy_before << '\n'
      << "particle_energy_after=" << arm.particle_energy_after << '\n'
      << "field_energy_before=" << arm.field_energy_before << '\n'
      << "field_energy_after=" << arm.field_energy_after << '\n'
      << "interaction_energy_before=" << arm.interaction_energy_before << '\n'
      << "interaction_energy_after=" << arm.interaction_energy_after << '\n'
      << "matter_work=" << arm.matter_work << '\n'
      << "p0=" << arm.momentum_before.x << ',' << arm.momentum_before.y
      << ',' << arm.momentum_before.z << '\n'
      << "p1=" << arm.momentum_after.x << ',' << arm.momentum_after.y
      << ',' << arm.momentum_after.z << '\n'
      << "impulse=" << arm.matter_impulse.x << ',' << arm.matter_impulse.y
      << ',' << arm.matter_impulse.z << '\n'
      << "site_shift=" << arm.site_shift.x << ',' << arm.site_shift.y
      << ',' << arm.site_shift.z << '\n';

  int failures = 0;
  const auto check = [&](bool condition, const char* message) {
    if (!condition) {
      std::cerr << "FAIL: " << message << '\n';
      ++failures;
    }
  };
  check(result.valid, "campaign produced a valid conjunctive verdict");
  check(result.verdict
            == "SITE_ONTIC_NATIVE_RECOIL_MAP_FAILS_ATOMIC_COMPATIBILITY",
        "locked R0 candidate closes at the first preregistered counterexample");
  check(result.arms_attempted >= 1, "at least one locked arm evaluated");
  check(arm.evaluated, "decisive arm evaluated");
  check(arm.failure_gate != "independent_root_certificate_pending",
        "a physical/algebraic gate fails before interval certification");
  check(!arm.one_event_gates_pass, "one-event conjunctive gate remains closed");
  check(std::isfinite(arm.energy_relative_residual),
        "energy residual is finite");
  check(std::isfinite(arm.work_relative_residual),
        "work residual is finite");

  std::cout << "site_ontic_atomic_reciprocal_hop failures="
            << failures << '\n';
  return failures == 0 ? 0 : 1;
}
