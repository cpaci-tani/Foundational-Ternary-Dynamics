/**
 * Energy ledger computation — implementation.
 * Extracted from render_bridge.cpp, 2026-04-18 refactor ticket R3.
 */

#include "ftd/energy_ledger_compute.h"
#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "ftd/strong_stress_energy.h"
#include <cmath>

namespace ftd {

void commit_energy_ledger(RenderBridge& rb, double E_total) {
  // P3 (2026-07-26): damping decays a QUADRATIC measure at (1-g)^2 - 1.
  //
  // phase_write multiplies AMPLITUDES by (1-DAMPING) once per tick, so an
  // energy-like (quadratic) functional decays by (1-g)^2 - 1 = -2g + g^2, not
  // by -g. The old `-DAMPING` made the declared contract at
  // render_bridge_diagnostics.h ( |dE/E + g| < eps ) false by construction.
  //
  // NOTE (still open, see audit W2): `selective_damping` -- ON by default --
  // damps only manifested sites plus their 6 face neighbours, so no single
  // global scalar can express the expected rate for that regime. This
  // correction fixes the uniform-damping case only; when selective_damping is
  // on, `expected_rate` remains an approximation and `residual` should not be
  // read as a conservation violation.
  const double g = DAMPING;
  const double quadratic_damping_rate = -2.0 * g + g * g;
  auto& L = rb.energy_ledger_;
  ++L.updates;

  if (L.tick_prev < 0) {
    L.tick_prev  = rb.tick_;
    L.E_prev     = E_total;
    L.E_curr     = E_total;
    L.dE_dt      = 0.0;
    L.drift_frac = 0.0;
    L.residual   = 0.0;
    L.expected_rate = rb.toggles.damping ? quadratic_damping_rate : 0.0;
    return;
  }

  const double E_prev = L.E_curr;
  L.tick_prev  = rb.tick_ - 1;
  L.E_prev     = E_prev;
  L.E_curr     = E_total;
  L.dE_dt      = (E_total - E_prev) / std::max(rb.dt_, 1e-12);

  const double denom = std::max(std::abs(E_prev), 1e-12);
  L.drift_frac = (E_total - E_prev) / denom;
  L.expected_rate = rb.toggles.damping ? quadratic_damping_rate : 0.0;
  L.residual   = L.drift_frac - L.expected_rate;

  if (L.residual > 0.0) {
    L.cumulative_injection += L.residual * denom;
  } else {
    L.cumulative_dissipation += (-L.residual) * denom;
  }
  const double abs_res = std::abs(L.residual);
  if (abs_res > L.max_residual_seen) {
    L.max_residual_seen = abs_res;
  }
}

void update_energy_ledger_cpu(RenderBridge& rb) {
  const auto& voxels = rb.voxels_;
  const int N = static_cast<int>(rb.lattice_.total_sites());
  double E_field = 0.0, E_wave = 0.0, E_kin = 0.0;
  for (int i = 0; i < N; ++i) {
    const auto& v = voxels[i];
    E_field += v.flux.mag2();
    E_wave  += v.wave_vel.mag2();
  }
  for (int i : rb.ordered_active_indices()) {
    const auto& v = voxels[i];
    if (v.state != 0) E_kin += flat_particle_kinetic_energy(v.velocity.mag2());
  }
  // The drift ledger deliberately tracks the rest-offset-free accounted
  // channels.  Rest energy is displayed separately and interaction energies
  // remain incomplete until NCEMC.
  const double E_strong = rb.toggles.strong_stress_energy
      ? compute_strong_potential_energy(rb) : 0.0;
  const double E_total = 0.5 * (E_field + E_wave) + E_kin + E_strong;
  commit_energy_ledger(rb, E_total);
}

void update_energy_ledger_from_audit(RenderBridge& rb) {
  // Interactive GPU path only. The host AoS shadow (rb.voxels_) is deliberately
  // stale here (the 469 B/site device->host mirror is deferred every tick), so
  // update_energy_ledger_cpu would sum zeros and report E_curr = 0. Source the
  // identical channels from the compact device-side reduction instead.
  //
  // Convention parity (empirically verified — see the CPU cross-check in
  // test_gpu_energy_ledger_parity.cpp and the pre-existing field/wave/kinetic
  // CPU/GPU parity in test_gpu_compact_diagnostics.cpp): at the production unit
  // edge V_cell = 1 (volumetric_measure.h static_assert), the audit reports
  //   field_energy = Σ ½|J|²,  wave_energy = Σ ½|wave_vel|²,
  //   particle_ke  = Σ flat_particle_kinetic_energy(|v|²),
  // which are byte-for-byte the same channels update_energy_ledger_cpu sums.
  // Hence  field_energy + wave_energy + particle_ke  ==  the host formula
  //   0.5*(Σ|J|² + Σ|wave_vel|²) + E_kin,  and strong_potential_energy is the
  // same compute_strong_potential_energy(rb) the host path adds as E_strong.
  //
  // energy_audit() returns the compact device reduction when strong is OFF
  // (a few scalars D2H, no full mirror); when strong_stress_energy is ON it
  // falls through to compute_energy_audit(), whose voxels() access syncs the
  // host fresh and populates strong_potential_energy — correct at the cost of a
  // full mirror on that off-by-default toggle only.
  const EnergyAudit a = rb.energy_audit();
  double E_total = a.field_energy + a.wave_energy + a.particle_ke;
  if (rb.toggles.strong_stress_energy) E_total += a.strong_potential_energy;
  commit_energy_ledger(rb, E_total);
}

}  // namespace ftd
