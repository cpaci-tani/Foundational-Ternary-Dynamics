/**
 * @file test_dynamic_flux_dressing_scenario.cpp
 * @brief Behavioral admission gate for s0-seed-dynamical-flux-dressing.
 */

#include "ftd/eft/dynamical_flux_dressing_observer.h"
#include "ftd/scenarios.h"

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
  int result = 0;
  for (const auto& voxel : bridge.voxels())
    if (voxel.state != 0) ++result;
  return result;
}

int field_support(const ftd::RenderBridge& bridge, double threshold = 1e-14) {
  int result = 0;
  for (const auto& voxel : bridge.voxels())
    if (voxel.flux.mag2() + voxel.wave_vel.mag2()
        > threshold * threshold) ++result;
  return result;
}

}  // namespace

int main() {
  constexpr int L = 33;
  const int centre = L / 2;
  ftd::RenderBridge bridge(L);
  bridge.force_cpu();

  check("scenario dispatches", ftd::dispatch_scenario(
      bridge, "s0-seed-dynamical-flux-dressing"));
  const int source = bridge.lattice().index(centre, centre, centre);
  const auto& initial = bridge.voxels()[static_cast<std::size_t>(source)];
  check("exactly one manifested site initially", manifested_count(bridge) == 1);
  check("source is central positive and locked",
        initial.state == +1 && initial.locked);
  check("initial field and wave momentum are exactly zero",
        field_support(bridge, 0.0) == 0);
  check("only native wave and coupling terms are active",
        bridge.toggles.wave_propagation && bridge.toggles.coupling
        && !bridge.toggles.gauss_projection
        && !bridge.toggles.matched_gauss_dynamics
        && !bridge.toggles.damping && !bridge.toggles.forces
        && !bridge.toggles.movement && !bridge.toggles.genesis
        && !bridge.toggles.evaporation && !bridge.toggles.pair_production
        && !bridge.toggles.weak_transmutation
        && !bridge.toggles.dual_substrate);
  check("scenario pins periodic computational boundary",
        bridge.toggles.flux_boundary == ftd::FluxBoundaryMode::Periodic);

  bridge.tick();
  check("first tick has exact six-face source support",
        field_support(bridge) == 6);
  const int face[6][3] = {
      {+1,0,0},{-1,0,0},{0,+1,0},{0,-1,0},{0,0,+1},{0,0,-1}};
  bool exact_faces = true;
  for (const auto& offset : face) {
    const auto& voxel = bridge.voxel_at(
        centre + offset[0], centre + offset[1], centre + offset[2]);
    const ftd::Vec3 expected{
        0.5 * ftd::G_C * offset[0],
        0.5 * ftd::G_C * offset[1],
        0.5 * ftd::G_C * offset[2]};
    exact_faces = exact_faces
        && (voxel.flux - expected).mag() <= 1e-15
        && (voxel.wave_vel - expected).mag() <= 1e-15;
  }
  check("first-tick face vectors are exactly outward -G_C grad(s)",
        exact_faces);
  auto observation = ftd::eft::observe_dynamical_flux_dressing(
      bridge, source, +1);
  check("first response is finite and perfectly radial",
        observation.valid
        && std::abs(observation.radial_alignment - 1.0) <= 1e-15
        && observation.signed_source_divergence > 0.0
        && observation.max_support_radius == 1);

  for (int tick = 2; tick <= 8; ++tick) bridge.tick();
  observation = ftd::eft::observe_dynamical_flux_dressing(
      bridge, source, +1);
  check("eight-tick response remains inside native dependency cone",
        observation.valid && observation.max_support_radius <= 8);
  check("field is generated without matter creation or motion",
        observation.activity > 1e-8 && manifested_count(bridge) == 1
        && bridge.voxels()[static_cast<std::size_t>(source)].state == +1
        && bridge.voxels()[static_cast<std::size_t>(source)].locked);

  std::cout << "dynamic_flux_dressing_scenario failures=" << failures << '\n';
  return failures == 0 ? 0 : 1;
}
