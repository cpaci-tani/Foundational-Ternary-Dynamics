// Applicability of the existing reference wave-energy observer.
// Small native trajectories only; no species or continuum recovery claim.
#include "ftd/eft/wave_morphology_observer.h"
#include "ftd/eft/dynamical_flux_dressing_observer.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <utility>

namespace {
int failures = 0;
int checks = 0;

void check(const std::string& name, bool pass) {
  ++checks;
  if (!pass) {
    ++failures;
    std::cerr << "FAIL: " << name << '\n';
  }
}

void configure(ftd::RenderBridge& bridge) {
  // disable_all() intentionally preserves opt-in, non-bulk terms such as
  // pumps and alternate integrators. Each independent arm clears all terms.
  for (const auto& spec : ftd::TOGGLE_SPECS)
    bridge.toggles.*(spec.field) = false;
  bridge.toggles.wave_propagation = true;
  bridge.toggles.strict_validation = true;
  bridge.toggles.bcc_stencil = ftd::BccStencilMode::FULL;
  bridge.toggles.flux_boundary = ftd::FluxBoundaryMode::Periodic;
  bridge.set_dt(1.0);
}

bool same_decomposition(const ftd::eft::NativeWaveEnergy& a,
                        const ftd::eft::NativeWaveEnergy& b) {
  return a.amplitude == b.amplitude && a.kinetic == b.kinetic
      && a.gradient == b.gradient && a.cross == b.cross
      && a.naive == b.naive && a.tick_invariant == b.tick_invariant;
}

void unavailable(ftd::RenderBridge& bridge,
                 const ftd::eft::NativeWaveEnergy& reference,
                 const std::string& name) {
  const auto energy = ftd::eft::measure_native_wave_energy(bridge);
  const auto morphology = ftd::eft::observe_wave_morphology(bridge, +1, 2.0);
  const auto dressing = ftd::eft::observe_dynamical_flux_dressing(
      bridge, bridge.lattice().index(4, 4, 4), +1);
  check(name + ": raw partial account retained",
        energy.finite && same_decomposition(energy, reference));
  check(name + ": invariant scope rejected with reason",
        !energy.tick_invariant_applicable && !energy.tick_invariant_reason.empty());
  check(name + ": finite morphology remains usable", morphology.valid);
  check(name + ": exact energy unavailable rather than zero",
        !morphology.exact_tick_energy_available
        && std::isnan(morphology.exact_tick_energy)
        && morphology.exact_tick_energy_reason == energy.tick_invariant_reason);
  check(name + ": dressing morphology retains explicit energy boundary",
        dressing.valid && !dressing.exact_tick_energy_available
        && std::isnan(dressing.exact_tick_energy)
        && dressing.exact_tick_energy_reason == energy.tick_invariant_reason);
}
}  // namespace

int main() {
  ftd::RenderBridge bridge(9);
  bridge.force_cpu();
  configure(bridge);
  check("the trajectory uses the forced native CPU owner",
        bridge.backend_kind() == ftd::Backend::Kind::Cpu);
  const auto empty = ftd::eft::observe_wave_morphology(bridge, +1, 2.0);
  check("null field has exact zero energy but no packet morphology",
        !empty.valid && empty.exact_tick_energy_available
        && empty.exact_tick_energy == 0.0L);
  bridge.voxel_at(4, 4, 4).flux = {0.03, 0.08, -0.02};
  bridge.voxel_at(4, 4, 4).wave_vel = {0.01, -0.02, 0.04};
  bridge.voxel_at(5, 4, 4).flux = {-0.01, 0.03, 0.02};

  const auto initial_voxels = std::as_const(bridge).voxels();
  const int initial_tick = bridge.current_tick();
  const auto reference = ftd::eft::measure_native_wave_energy(bridge);
  const auto morphology = ftd::eft::observe_wave_morphology(bridge, +1, 2.0);
  const auto dressing = ftd::eft::observe_dynamical_flux_dressing(
      bridge, bridge.lattice().index(4, 4, 4), +1);
  check("supported energy is finite and applicable",
        reference.finite && reference.tick_invariant_applicable
        && reference.tick_invariant_reason.empty());
  check("supported morphology publishes exact energy",
        morphology.valid && morphology.exact_tick_energy_available
        && morphology.exact_tick_energy == reference.tick_invariant
        && morphology.exact_tick_energy_reason.empty());
  check("supported dressing observation publishes exact energy",
        dressing.valid && dressing.exact_tick_energy_available
        && dressing.exact_tick_energy == reference.tick_invariant
        && dressing.exact_tick_energy_reason.empty());

  for (const auto mode : {ftd::BccStencilMode::SC, ftd::BccStencilMode::FCC,
                          ftd::BccStencilMode::BCC}) {
    configure(bridge);
    bridge.toggles.bcc_stencil = mode;
    unavailable(bridge, reference, "sublattice stencil "
                + std::to_string(static_cast<int>(mode)));
  }
  for (const auto boundary : {ftd::FluxBoundaryMode::Reflective,
                              ftd::FluxBoundaryMode::Dispersal}) {
    configure(bridge);
    bridge.toggles.flux_boundary = boundary;
    unavailable(bridge, reference, "boundary "
                + std::to_string(static_cast<int>(boundary)));
  }

  struct ExtraTerm { const char* name; bool ftd::TermToggles::*field; };
  const ExtraTerm terms[] = {
      {"state coupling", &ftd::TermToggles::coupling},
      {"Gauss projection", &ftd::TermToggles::gauss_projection},
      {"damping", &ftd::TermToggles::damping},
      {"genesis", &ftd::TermToggles::genesis},
      {"movement", &ftd::TermToggles::movement},
      {"Langevin forcing", &ftd::TermToggles::langevin},
      {"clock restoring term", &ftd::TermToggles::de_broglie_clock},
      {"dual state owner", &ftd::TermToggles::dual_substrate},
      {"matched face operator", &ftd::TermToggles::matched_gauss_dynamics},
      {"prescribed drive", &ftd::TermToggles::ew_background_sweep},
      {"pump", &ftd::TermToggles::flux_pump},
      {"port transactions", &ftd::TermToggles::flux_cell_port},
      {"Verlet integrator", &ftd::TermToggles::verlet_wave_integrator},
      {"period-two kicks", &ftd::TermToggles::lorentz_period2_floquet},
      {"BCC temporal kicks", &ftd::TermToggles::lorentz_bcc_time_floquet},
  };
  for (const auto& term : terms) {
    configure(bridge);
    bridge.toggles.*(term.field) = true;
    unavailable(bridge, reference, term.name);
  }
  configure(bridge);
  bridge.toggles.wave_propagation = false;
  unavailable(bridge, reference, "wave disabled");
  configure(bridge);
  bridge.toggles.symplectic_leapfrog = true;
  bridge.set_dt(0.5);
  unavailable(bridge, reference, "half-step kick-drift");

  configure(bridge);
  bridge.toggles.knot_tracking = true;
  check("observation-only telemetry preserves applicability",
        ftd::eft::measure_native_wave_energy(bridge).tick_invariant_applicable);
  configure(bridge);
  bool unchanged = bridge.current_tick() == initial_tick;
  const auto& observed_voxels = std::as_const(bridge).voxels();
  for (std::size_t i = 0; i < initial_voxels.size(); ++i) {
    const auto& a = initial_voxels[i];
    const auto& b = observed_voxels[i];
    unchanged = unchanged && a.state == b.state
        && a.flux.x == b.flux.x && a.flux.y == b.flux.y && a.flux.z == b.flux.z
        && a.wave_vel.x == b.wave_vel.x && a.wave_vel.y == b.wave_vel.y
        && a.wave_vel.z == b.wave_vel.z;
  }
  check("observer calls do not evolve the owner", unchanged);

  // The supported algebra agrees with actual production ticks; no fitting.
  for (int tick = 0; tick < 8; ++tick) {
    bridge.tick();
    const auto energy = ftd::eft::measure_native_wave_energy(bridge);
    check("native unit kick-drift conserves its applicable invariant",
          energy.tick_invariant_applicable
          && std::abs(energy.tick_invariant - reference.tick_invariant)
              < 1e-11L * reference.tick_invariant);
  }
  bridge.toggles.symplectic_leapfrog = true;
  bridge.tick();
  const auto equivalent = ftd::eft::measure_native_wave_energy(bridge);
  check("unit-step symplectic path is the same supported map",
        equivalent.tick_invariant_applicable
        && std::abs(equivalent.tick_invariant - reference.tick_invariant)
            < 1e-11L * reference.tick_invariant);

  // An actual different stencil exposes why a finite quadratic is insufficient.
  configure(bridge);
  bridge.toggles.bcc_stencil = ftd::BccStencilMode::SC;
  const auto sc_before = ftd::eft::measure_native_wave_energy(bridge);
  bridge.tick();
  const auto sc_after = ftd::eft::measure_native_wave_energy(bridge);
  check("SC trajectory changes the FULL reference quadratic",
        !sc_after.tick_invariant_applicable && sc_after.finite
        && std::abs(sc_after.tick_invariant - sc_before.tick_invariant) > 1e-8L);

  configure(bridge);
  bridge.voxel_at(4, 4, 4).flux.x = std::numeric_limits<double>::quiet_NaN();
  const auto corrupt = ftd::eft::measure_native_wave_energy(bridge);
  check("non-finite data never receive invariant applicability",
        !corrupt.finite && !corrupt.tick_invariant_applicable
        && !corrupt.tick_invariant_reason.empty());
  std::cout << "Native wave observer scope: " << checks << " checks, "
            << failures << " failures\n";
  return failures ? 1 : 0;
}
