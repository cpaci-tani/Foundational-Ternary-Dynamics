/**
 * @file campaign_native_energy_contract_reconciliation.cpp
 * @brief FTD-0452 native energy-contract and diagnostic reconciliation.
 */

#include "ftd/eft/half_tick_link_exchange.h"
#include "ftd/eft/native_energy_contract.h"
#include "ftd/lagrangian.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>

namespace {

constexpr int kDiagnosticL = 9;
constexpr int kWaveL = 17;
constexpr int kWaveTicks = 64;
constexpr int kMode = 2;
constexpr double kAmplitude = 0.125;
constexpr double kWaveAmplitude = 0.05;
constexpr double kWork = 1e-4;
constexpr double kGate = 1e-12;
constexpr double kTickGate = 1e-10;
constexpr double kGradientSeparationGate = 1e-6;
constexpr double kPi = 3.141592653589793238462643383279502884;

void disable_dynamics(ftd::RenderBridge& bridge) {
  bridge.force_cpu();
  bridge.toggles.disable_all();
  bridge.toggles.strict_validation = true;
}

void configure_source_free_wave(ftd::RenderBridge& bridge) {
  disable_dynamics(bridge);
  bridge.toggles.wave_propagation = true;
  bridge.toggles.coupling = false;
  bridge.toggles.damping = false;
  bridge.toggles.gauss_projection = false;
  bridge.toggles.genesis = false;
  bridge.toggles.forces = false;
  bridge.toggles.movement = false;
  bridge.set_dt(1.0);
}

void inject_transverse_mode(ftd::RenderBridge& bridge, int mode,
                            double amplitude, bool travelling) {
  const int length = bridge.lattice().size();
  const double k = 2.0 * kPi * static_cast<double>(mode)
      / static_cast<double>(length);
  const double omega = 2.0 * ftd::C_WAVE * std::abs(std::sin(0.5 * k));
  for (int x = 0; x < length; ++x) {
    const double phase = k * static_cast<double>(x);
    const double jy = amplitude * std::cos(phase);
    const double wy = travelling ? omega * amplitude * std::sin(phase) : 0.0;
    for (int y = 0; y < length; ++y)
      for (int z = 0; z < length; ++z) {
        auto& voxel = bridge.voxel_at(x, y, z);
        voxel.flux.y = jy;
        voxel.wave_vel.y = wy;
      }
  }
}

long double relative_difference(long double a, long double b) {
  return std::abs(a - b) / std::max(std::abs(a), 1e-30L);
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  std::cout << "FTD-0452 native energy-contract reconciliation v1\n";
  std::cout << "protocol,diagnostic_L," << kDiagnosticL
            << ",wave_L," << kWaveL << ",wave_ticks," << kWaveTicks
            << ",mode," << kMode << ",amplitude," << kAmplitude
            << ",work," << kWork << ",gate," << kGate
            << ",tick_gate," << kTickGate << '\n';

  ftd::RenderBridge vacuum(kDiagnosticL);
  disable_dynamics(vacuum);
  const auto vacuum_lag = ftd::compute_lagrangian_diagnostics(vacuum);

  ftd::RenderBridge kinetic(kDiagnosticL);
  disable_dynamics(kinetic);
  for (auto& voxel : kinetic.voxels())
    voxel.wave_vel = {kAmplitude, -0.5 * kAmplitude, 0.25 * kAmplitude};
  const auto kinetic_native = ftd::eft::measure_native_wave_energy(kinetic);
  const auto kinetic_lag = ftd::compute_lagrangian_diagnostics(kinetic);
  const double kinetic_h_sensitivity =
      kinetic_lag.total_hamiltonian - vacuum_lag.total_hamiltonian;

  ftd::RenderBridge patterned(kDiagnosticL);
  disable_dynamics(patterned);
  inject_transverse_mode(patterned, kMode, kAmplitude, false);
  const auto patterned_native = ftd::eft::measure_native_wave_energy(patterned);
  const auto patterned_lag = ftd::compute_lagrangian_diagnostics(patterned);
  const auto patterned_audit = patterned.energy_audit();
  const double patterned_h_sensitivity =
      patterned_lag.total_hamiltonian - vacuum_lag.total_hamiltonian;

  ftd::RenderBridge uniform(kDiagnosticL);
  disable_dynamics(uniform);
  const long double site_count = static_cast<long double>(uniform.voxels().size());
  const double uniform_amplitude = std::sqrt(static_cast<double>(
      2.0L * patterned_native.amplitude / site_count));
  for (auto& voxel : uniform.voxels()) voxel.flux.y = uniform_amplitude;
  const auto uniform_native = ftd::eft::measure_native_wave_energy(uniform);
  const auto uniform_audit = uniform.energy_audit();
  const double amplitude_audit_residual =
      uniform_audit.field_energy - patterned_audit.field_energy;
  const long double gradient_separation =
      patterned_native.gradient - uniform_native.gradient;

  const ftd::Vec3 velocity{0.15, 0.03, 0.0};
  const auto momentum = ftd::eft::production_flat_momentum(velocity);
  const ftd::eft::CubicVector displacement{{1, 0, 0}};
  const auto link = ftd::eft::make_half_tick_link_exchange(
      17, momentum, displacement, kWork);
  const double particle_delta =
      link.particle_energy_after - link.particle_energy_before;
  const double interaction_h_delta = -kWork;
  const double particle_work_residual = particle_delta - kWork;
  const double interaction_work_residual = interaction_h_delta + kWork;
  const double particle_interaction_residual =
      particle_delta + interaction_h_delta;
  const double double_counted_residual = particle_delta
      + interaction_h_delta + link.field_energy_exchange;

  ftd::RenderBridge wave(kWaveL);
  configure_source_free_wave(wave);
  inject_transverse_mode(wave, kMode, kWaveAmplitude, true);
  const auto tick_initial = ftd::eft::measure_native_wave_energy(wave);
  long double max_tick_abs_drift = 0.0L;
  long double max_tick_rel_drift = 0.0L;
  bool finite = tick_initial.finite;
  for (int tick = 0; tick < kWaveTicks; ++tick) {
    wave.tick();
    const auto energy = ftd::eft::measure_native_wave_energy(wave);
    const long double absolute = std::abs(
        energy.tick_invariant - tick_initial.tick_invariant);
    max_tick_abs_drift = std::max(max_tick_abs_drift, absolute);
    max_tick_rel_drift = std::max(max_tick_rel_drift,
        relative_difference(tick_initial.tick_invariant,
                            energy.tick_invariant));
    finite = finite && energy.finite;
  }

  finite = finite && kinetic_native.finite && patterned_native.finite
      && uniform_native.finite && link.valid
      && std::isfinite(vacuum_lag.total_hamiltonian)
      && std::isfinite(kinetic_h_sensitivity)
      && std::isfinite(patterned_h_sensitivity)
      && std::isfinite(double_counted_residual);
  const bool hop_contract_pass =
      std::abs(particle_work_residual) <= kGate
      && std::abs(interaction_work_residual) <= kGate
      && std::abs(particle_interaction_residual) <= kGate
      && std::abs(double_counted_residual) >= 0.5 * std::abs(kWork);
  const bool hamiltonian_diagnostic_incomplete =
      kinetic_native.kinetic > kGradientSeparationGate
      && patterned_native.gradient > kGradientSeparationGate
      && std::abs(kinetic_h_sensitivity) <= kGate
      && std::abs(patterned_h_sensitivity) <= kGate;
  const bool amplitude_diagnostic_degenerate =
      std::abs(amplitude_audit_residual) <= kGate
      && gradient_separation > kGradientSeparationGate;
  const bool tick_invariant_pass =
      max_tick_abs_drift <= kTickGate
      && max_tick_rel_drift <= kTickGate;

  const char* verdict = "PROTOCOL_INVALID";
  if (finite && hop_contract_pass && hamiltonian_diagnostic_incomplete
      && amplitude_diagnostic_degenerate && tick_invariant_pass)
    verdict = "ENERGY_CONTRACT_RECONCILED_DIAGNOSTICS_INCOMPLETE";
  else if (finite && !hop_contract_pass
           && std::abs(link.energy_residual) <= kGate)
    verdict = "HOP_REQUIRES_SEPARATE_WAVE_ENERGY_EXCHANGE";
  else if (finite && (!hamiltonian_diagnostic_incomplete
                      || !amplitude_diagnostic_degenerate))
    verdict = "DIAGNOSTICS_CAPTURE_CANONICAL_FIELD_ENERGY";

  std::cout << "hamiltonian_diagnostic,vacuum," << vacuum_lag.total_hamiltonian
            << ",kinetic_native," << static_cast<double>(kinetic_native.kinetic)
            << ",kinetic_sensitivity," << kinetic_h_sensitivity
            << ",gradient_native," << static_cast<double>(patterned_native.gradient)
            << ",gradient_sensitivity," << patterned_h_sensitivity << '\n';
  std::cout << "amplitude_diagnostic,uniform," << uniform_audit.field_energy
            << ",patterned," << patterned_audit.field_energy
            << ",residual," << amplitude_audit_residual
            << ",uniform_gradient," << static_cast<double>(uniform_native.gradient)
            << ",patterned_gradient," << static_cast<double>(patterned_native.gradient)
            << ",separation," << static_cast<double>(gradient_separation) << '\n';
  std::cout << "hop_contract,particle_delta," << particle_delta
            << ",interaction_h_delta," << interaction_h_delta
            << ",old_named_field_delta," << link.field_energy_exchange
            << ",particle_work_residual," << particle_work_residual
            << ",interaction_work_residual," << interaction_work_residual
            << ",particle_interaction_residual," << particle_interaction_residual
            << ",double_counted_residual," << double_counted_residual << '\n';
  std::cout << "tick_invariant,initial,"
            << static_cast<double>(tick_initial.tick_invariant)
            << ",max_abs_drift," << static_cast<double>(max_tick_abs_drift)
            << ",max_rel_drift," << static_cast<double>(max_tick_rel_drift)
            << '\n';
  std::cout << "gates,finite," << (finite ? "true" : "false")
            << ",hop_contract," << (hop_contract_pass ? "true" : "false")
            << ",hamiltonian_diagnostic_incomplete,"
            << (hamiltonian_diagnostic_incomplete ? "true" : "false")
            << ",amplitude_diagnostic_degenerate,"
            << (amplitude_diagnostic_degenerate ? "true" : "false")
            << ",tick_invariant," << (tick_invariant_pass ? "true" : "false")
            << '\n';
  std::cout << "verdict," << verdict << '\n';
  return std::string(verdict) == "PROTOCOL_INVALID" ? 1 : 0;
}
