/**
 * FTD-0395 lock instrument: exact full-state collision under evaporation.
 *
 * Test-only comparator. No production API or engine behavior is changed.
 */

#include "ftd/render_bridge.h"

#include <cstring>
#include <iostream>
#include <string>
#include <vector>

namespace {

int failures = 0;

void check(const std::string& name, bool ok) {
  std::cout << (ok ? "PASS " : "FAIL ") << name << '\n';
  if (!ok) ++failures;
}

bool bits(double a, double b) {
  return std::memcmp(&a, &b, sizeof(double)) == 0;
}

bool vec_equal(const ftd::Vec3& a, const ftd::Vec3& b) {
  return bits(a.x, b.x) && bits(a.y, b.y) && bits(a.z, b.z);
}

bool voxel_equal(const ftd::Voxel& a, const ftd::Voxel& b) {
  return a.state == b.state &&
         vec_equal(a.flux, b.flux) && vec_equal(a.wave_vel, b.wave_vel) &&
         vec_equal(a.flux_L, b.flux_L) && vec_equal(a.flux_R, b.flux_R) &&
         vec_equal(a.wave_vel_L, b.wave_vel_L) && vec_equal(a.wave_vel_R, b.wave_vel_R) &&
         vec_equal(a.velocity, b.velocity) && vec_equal(a.remainder, b.remainder) &&
         bits(a.latency, b.latency) && bits(a.tau, b.tau) && bits(a.phase, b.phase) &&
         a.locked == b.locked && a.particle_id == b.particle_id &&
         a.pair_id == b.pair_id && a.spin == b.spin && a.color == b.color &&
         a.flavor == b.flavor && bits(a.accel_mag, b.accel_mag) &&
         vec_equal(a.flux_strong, b.flux_strong) &&
         vec_equal(a.wave_vel_strong, b.wave_vel_strong) &&
         vec_equal(a.flux_weak, b.flux_weak) &&
         vec_equal(a.wave_vel_weak, b.wave_vel_weak);
}

bool audit_equal(const ftd::EnergyAudit& a, const ftd::EnergyAudit& b) {
  return bits(a.field_energy, b.field_energy) &&
         bits(a.wave_energy, b.wave_energy) &&
         bits(a.particle_ke, b.particle_ke) &&
         bits(a.total_energy, b.total_energy) &&
         bits(a.gauss_violation, b.gauss_violation) &&
         bits(a.max_gauss_error, b.max_gauss_error) &&
         bits(a.self_field_injection, b.self_field_injection) &&
         bits(a.coulomb_pe, b.coulomb_pe) &&
         bits(a.E_field_energy, b.E_field_energy) &&
         bits(a.B_field_energy, b.B_field_energy) &&
         a.charge_total == b.charge_total &&
         a.manifested_count == b.manifested_count &&
         vec_equal(a.total_poynting, b.total_poynting) &&
         bits(a.E_L_total, b.E_L_total) && bits(a.E_R_total, b.E_R_total) &&
         bits(a.wv_L_total, b.wv_L_total) && bits(a.wv_R_total, b.wv_R_total) &&
         bits(a.chirality_total, b.chirality_total) &&
         bits(a.strong_energy, b.strong_energy) && bits(a.weak_energy, b.weak_energy) &&
         bits(a.particle_rest_energy, b.particle_rest_energy) &&
         bits(a.particle_energy, b.particle_energy) &&
         vec_equal(a.particle_momentum, b.particle_momentum) &&
         bits(a.dynamic_energy, b.dynamic_energy);
}

bool ledger_equal(const ftd::EnergyLedger& a, const ftd::EnergyLedger& b) {
  return a.tick_prev == b.tick_prev && bits(a.E_prev, b.E_prev) &&
         bits(a.E_curr, b.E_curr) && bits(a.dE_dt, b.dE_dt) &&
         bits(a.drift_frac, b.drift_frac) && bits(a.expected_rate, b.expected_rate) &&
         bits(a.residual, b.residual) &&
         bits(a.cumulative_injection, b.cumulative_injection) &&
         bits(a.cumulative_dissipation, b.cumulative_dissipation) &&
         bits(a.max_residual_seen, b.max_residual_seen);
}

bool complete_equal(const ftd::RenderBridge& a, const ftd::RenderBridge& b) {
  if (a.lattice().size() != b.lattice().size() ||
      a.backend_kind() != b.backend_kind() ||
      a.current_tick() != b.current_tick() || !bits(a.physical_time(), b.physical_time()) ||
      !bits(a.dt(), b.dt()) || a.sor_iterations() != b.sor_iterations() ||
      a.injector().peek_next_particle_id() != b.injector().peek_next_particle_id() ||
      a.injector().peek_next_pair_id() != b.injector().peek_next_pair_id() ||
      a.charge_sum() != b.charge_sum() ||
      a.genesis_events_this_tick() != b.genesis_events_this_tick() ||
      a.evaporation_events_this_tick() != b.evaporation_events_this_tick() ||
      !audit_equal(a.energy_audit(), b.energy_audit()) ||
      !ledger_equal(a.energy_ledger(), b.energy_ledger())) {
    return false;
  }
  const auto& av = a.voxels();
  const auto& bv = b.voxels();
  if (av.size() != bv.size()) return false;
  for (std::size_t i = 0; i < av.size(); ++i) {
    if (!voxel_equal(av[i], bv[i])) return false;
  }
  return true;
}

void configure(ftd::RenderBridge& rb, bool evaporation) {
  rb.force_cpu();
  rb.toggles.disable_all();
  rb.toggles.evaporation = evaporation;
  rb.seed_rng(20260422);
}

void inject_pair(ftd::RenderBridge& a, ftd::RenderBridge& b) {
  a.inject_particle(3, 3, 3, +1, {1e-5, 0.0, 0.0}, +1, 1);
  b.inject_particle(3, 3, 3, +1, {1e-5, 0.0, 0.0}, -1, 3);
}

bool pre_diff_exactly_labels(const ftd::RenderBridge& a, const ftd::RenderBridge& b) {
  const int center = a.lattice().index(3, 3, 3);
  const auto& av = a.voxels();
  const auto& bv = b.voxels();
  if (av.size() != bv.size()) return false;
  for (std::size_t i = 0; i < av.size(); ++i) {
    if (static_cast<int>(i) == center) {
      if (av[i].spin == bv[i].spin || av[i].color == bv[i].color) return false;
      ftd::Voxel masked = av[i];
      masked.spin = bv[i].spin;
      masked.color = bv[i].color;
      if (!voxel_equal(masked, bv[i])) return false;
    } else if (!voxel_equal(av[i], bv[i])) {
      return false;
    }
  }
  return a.current_tick() == b.current_tick() &&
         bits(a.physical_time(), b.physical_time()) &&
         a.injector().peek_next_particle_id() == b.injector().peek_next_particle_id() &&
         a.injector().peek_next_pair_id() == b.injector().peek_next_pair_id();
}

}  // namespace

int main() {
  std::cout << "FTD-0395 FULL-STATE IRREVERSIBILITY LOCK INSTRUMENT\n";

  ftd::RenderBridge a(8), b(8);
  configure(a, true);
  configure(b, true);
  inject_pair(a, b);

  check("G2 cpu backend", a.backend_kind() == ftd::Backend::Kind::Cpu &&
                          b.backend_kind() == ftd::Backend::Kind::Cpu);
  check("G3 prestate differs exactly in spin/color", pre_diff_exactly_labels(a, b));
  check("prestate complete comparator detects difference", !complete_equal(a, b));

  a.tick();
  b.tick();
  const int center = a.lattice().index(3, 3, 3);
  const bool same_tick_evap = a.current_tick() == 1 && b.current_tick() == 1 &&
      a.evaporation_events_this_tick() == 1 && b.evaporation_events_this_tick() == 1 &&
      a.voxels()[center].state == 0 && b.voxels()[center].state == 0;
  check("G4 same full-tick evaporation", same_tick_evap);

  const bool immediate_equal = complete_equal(a, b);
  std::cout << "OBS complete state equal after collision tick "
            << (immediate_equal ? 1 : 0) << '\n';

  bool tail_equal = immediate_equal;
  for (int i = 0; i < 16; ++i) {
    a.tick();
    b.tick();
    tail_equal = tail_equal && complete_equal(a, b);
  }
  std::cout << "OBS sixteen-tick hidden-cache tail equal "
            << (tail_equal ? 1 : 0) << '\n';

  ftd::RenderBridge control_a(8), control_b(8);
  configure(control_a, false);
  configure(control_b, false);
  inject_pair(control_a, control_b);
  control_a.tick();
  control_b.tick();
  const auto& ca = control_a.voxels()[center];
  const auto& cb = control_b.voxels()[center];
  const bool control_ok = ca.state == +1 && cb.state == +1 &&
      ca.particle_id == cb.particle_id && ca.spin == +1 && cb.spin == -1 &&
      ca.color == 1 && cb.color == 3 && !complete_equal(control_a, control_b);
  check("G5 evaporation-off negative control preserves labels", control_ok);

  const bool valid = failures == 0;
  const bool phase_record_equal = same_tick_evap &&
      a.voxels()[center].state == b.voxels()[center].state &&
      a.voxels()[center].spin == b.voxels()[center].spin &&
      a.voxels()[center].color == b.voxels()[center].color;

  std::string outcome;
  if (!valid) outcome = "INVALID";
  else if (immediate_equal && tail_equal) outcome = "FULL-NONINJECTIVE";
  else if (phase_record_equal) outcome = "PHASE-ONLY";
  else outcome = "READOUT-ONLY";

  std::cout << "OUTCOME " << outcome << '\n';
  return valid ? 0 : 1;
}
