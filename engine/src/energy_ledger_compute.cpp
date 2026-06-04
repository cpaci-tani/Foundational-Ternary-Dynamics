/**
 * Energy ledger computation — implementation.
 * Extracted from render_bridge.cpp, 2026-04-18 refactor ticket R3.
 */

#include "ftd/energy_ledger_compute.h"
#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include <cmath>

namespace ftd {

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
    if (v.state != 0) E_kin += 0.5 * v.velocity.mag2();
  }
  const double E_total = 0.5 * (E_field + E_wave) + E_kin;
  auto& L = rb.energy_ledger_;

  if (L.tick_prev < 0) {
    L.tick_prev  = rb.tick_;
    L.E_prev     = E_total;
    L.E_curr     = E_total;
    L.dE_dt      = 0.0;
    L.drift_frac = 0.0;
    L.residual   = 0.0;
    L.expected_rate = rb.toggles.damping ? -DAMPING : 0.0;
    return;
  }

  const double E_prev = L.E_curr;
  L.tick_prev  = rb.tick_ - 1;
  L.E_prev     = E_prev;
  L.E_curr     = E_total;
  L.dE_dt      = (E_total - E_prev) / std::max(rb.dt_, 1e-12);

  const double denom = std::max(std::abs(E_prev), 1e-12);
  L.drift_frac = (E_total - E_prev) / denom;
  L.expected_rate = rb.toggles.damping ? -DAMPING : 0.0;
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

}  // namespace ftd
