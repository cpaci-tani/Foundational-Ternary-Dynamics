/**
 * @file test_reciprocal_moving_source_scenario.cpp
 * @brief Mechanical admission gate for the FTD-0477 dashboard scenario.
 */

#include "ftd/render_bridge.h"
#include "ftd/scenarios.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>

namespace {

int failures = 0;

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

int manifested_count(const ftd::RenderBridge& bridge) {
  int count = 0;
  for (const auto& voxel : bridge.voxels())
    if (voxel.state != 0) ++count;
  return count;
}

int find_particle(const ftd::RenderBridge& bridge, int particle_id) {
  for (int index = 0;
       index < static_cast<int>(bridge.voxels().size()); ++index) {
    const auto& voxel = bridge.voxels()[static_cast<std::size_t>(index)];
    if (voxel.state != 0 && voxel.particle_id == particle_id) return index;
  }
  return -1;
}

}  // namespace

int main() {
  constexpr int kL = 33;
  constexpr int kCentre = kL / 2;
  constexpr int kSourceY = kCentre + 2;
  ftd::RenderBridge bridge(kL);
  bridge.force_cpu();

  check("scenario dispatches", ftd::dispatch_scenario(
      bridge, "s0-seed-moving-source-reciprocity"));
  const int source_index = bridge.lattice().index(
      kCentre, kSourceY, kCentre);
  const auto& source = bridge.voxels()[static_cast<std::size_t>(source_index)];
  const int particle_id = source.particle_id;

  check("one unlocked positive source is admitted",
        manifested_count(bridge) == 1 && source.state == +1
        && !source.locked && particle_id >= 0);
  check("source begins at exact mechanical rest",
        source.velocity.mag2() == 0.0 && source.remainder.mag2() == 0.0);
  check("source site begins outside the driver support",
        source.flux.mag2() == 0.0 && source.wave_vel.mag2() == 0.0);

  double driver_norm2 = 0.0;
  double max_divergence = 0.0;
  for (int index = 0;
       index < static_cast<int>(bridge.voxels().size()); ++index) {
    const auto& voxel = bridge.voxels()[static_cast<std::size_t>(index)];
    driver_norm2 += voxel.flux.mag2() + voxel.wave_vel.mag2();
    max_divergence = std::max(
        max_divergence, std::abs(bridge.divergence_flux(index)));
  }
  check("separate finite driver is nonzero and transverse",
        driver_norm2 > 1e-6 && max_divergence <= 1e-12);

  const auto& toggles = bridge.toggles;
  check("only the registered selected-force profile is active",
        toggles.wave_propagation && toggles.coupling && toggles.forces
        && toggles.movement && toggles.emergent_forces
        && toggles.strict_validation && !toggles.damping
        && !toggles.genesis && !toggles.evaporation
        && !toggles.gauss_projection && !toggles.matched_gauss_dynamics
        && !toggles.gravity && !toggles.poisson_coulomb
        && !toggles.lorentz_force && !toggles.color_forces
        && !toggles.weak_transmutation && !toggles.dual_substrate
        && !toggles.pair_production && !toggles.langevin);
  check("scenario pins periodic computational boundary",
        toggles.flux_boundary == ftd::FluxBoundaryMode::Periodic);

  double max_force = 0.0;
  double max_speed = 0.0;
  for (int tick = 0; tick < 48; ++tick) {
    const int before_index = find_particle(bridge, particle_id);
    check("source survives before tick " + std::to_string(tick),
          before_index >= 0);
    if (before_index < 0) break;
    bridge.tick();
    const int after_index = find_particle(bridge, particle_id);
    if (after_index < 0) break;
    const auto& after = bridge.voxels()[static_cast<std::size_t>(after_index)];
    max_force = std::max(
        max_force, bridge.force_diag_at(before_index).f_coulomb.mag());
    max_speed = std::max(max_speed, after.speed());
  }
  check("source survives the visual response window",
        find_particle(bridge, particle_id) >= 0
        && manifested_count(bridge) == 1);
  check("separate packet produces a resolved selected-force response",
        max_force > 1e-8 && max_speed > 1e-8);

  std::cout << "reciprocal_moving_source_scenario failures="
            << failures << '\n';
  return failures == 0 ? 0 : 1;
}
