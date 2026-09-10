#pragma once
/**
 * @file native_energy_contract.h
 * @brief Observer-only energy decomposition for the exact production wave tick.
 *
 * This helper does not modify RenderBridge.  It distinguishes the amplitude
 * norm used by EnergyAudit from the gradient-plus-cross invariant preserved by
 * the source-free symplectic-Euler update.
 */

#include "ftd/constants.h"
#include "ftd/lagrangian.h"
#include "ftd/render_bridge.h"

#include <cmath>
#include <string>

namespace ftd::eft {

struct NativeWaveEnergy {
  long double amplitude = 0.0L;
  long double kinetic = 0.0L;
  long double gradient = 0.0L;
  long double cross = 0.0L;
  long double naive = 0.0L;
  // Raw reference quadratic, retained even outside its invariant domain for
  // callers using it as one component of a separate work/exchange ledger.
  long double tick_invariant = 0.0L;
  bool finite = true;
  bool tick_invariant_applicable = false;
  std::string tick_invariant_reason = "not evaluated";
};

// Scope of the quadratic computed below, not a test of whether a particular
// short trajectory happens to conserve it. This deliberately admits only the
// source-free periodic FULL-stencil unit kick-drift profile. Other profiles
// may have their own invariants; finite values here do not certify them.
// Unknown future dynamics are rejected by the table-driven allowlist.
inline std::string native_wave_invariant_unavailability_reason(
    const RenderBridge& bridge) {
  const auto& toggles = bridge.toggles;
  if (!toggles.wave_propagation)
    return "wave propagation is disabled";
  if (toggles.flux_boundary != FluxBoundaryMode::Periodic)
    return "requires the periodic boundary operator";
  if (toggles.bcc_stencil != BccStencilMode::FULL)
    return "requires the FULL 18-point stencil";
  if (bridge.dt() != 1.0)
    return "requires a unit tick duration";
  if (toggles.verlet_wave_integrator || toggles.lorentz_period2_floquet
      || toggles.lorentz_bcc_time_floquet)
    return "requires the unit kick-drift wave integrator";
  for (const auto& spec : TOGGLE_SPECS) {
    // At dt=1 symplectic_leapfrog executes exactly the same kick and drift.
    // Validation and knot telemetry do not change the field transaction.
    if (spec.field == &TermToggles::wave_propagation
        || spec.field == &TermToggles::symplectic_leapfrog
        || spec.field == &TermToggles::strict_validation
        || spec.field == &TermToggles::knot_tracking)
      continue;
    if (toggles.*(spec.field))
      return std::string("outside the source-free wave contract: ") + spec.name;
  }
  return {};
}

inline long double dot_long_double(const Vec3& a, const Vec3& b) {
  return static_cast<long double>(a.x) * static_cast<long double>(b.x)
      + static_cast<long double>(a.y) * static_cast<long double>(b.y)
      + static_cast<long double>(a.z) * static_cast<long double>(b.z);
}

inline NativeWaveEnergy measure_native_wave_energy(
    const RenderBridge& bridge) {
  NativeWaveEnergy result;
  const auto& voxels = bridge.voxels();
  const long double c2 = static_cast<long double>(C_WAVE)
      * static_cast<long double>(C_WAVE);
  for (int i = 0; i < static_cast<int>(voxels.size()); ++i) {
    const auto& voxel = voxels[static_cast<std::size_t>(i)];
    result.amplitude += 0.5L * dot_long_double(voxel.flux, voxel.flux);
    result.kinetic += 0.5L * dot_long_double(
        voxel.wave_vel, voxel.wave_vel);
    result.gradient -= static_cast<long double>(field_gradient_term(
        voxel.flux, bridge.lattice().neighbors_6(i),
        bridge.lattice().neighbors_12(i), voxels));
    const Vec3 delta = bridge.laplacian_flux(i) * static_cast<double>(c2);
    result.cross += 0.5L * dot_long_double(voxel.wave_vel, delta);
  }
  result.naive = result.kinetic + result.gradient;
  result.tick_invariant = result.naive + result.cross;
  result.finite = std::isfinite(result.amplitude)
      && std::isfinite(result.kinetic) && std::isfinite(result.gradient)
      && std::isfinite(result.cross) && std::isfinite(result.naive)
      && std::isfinite(result.tick_invariant);
  result.tick_invariant_reason =
      native_wave_invariant_unavailability_reason(bridge);
  if (!result.finite)
    result.tick_invariant_reason = "non-finite reference energy decomposition";
  result.tick_invariant_applicable = result.tick_invariant_reason.empty();
  return result;
}

inline long double coupling_hamiltonian(const RenderBridge& bridge) {
  long double result = 0.0L;
  for (int i = 0; i < static_cast<int>(bridge.voxels().size()); ++i) {
    const auto& voxel = bridge.voxels()[static_cast<std::size_t>(i)];
    result -= static_cast<long double>(G_C)
        * static_cast<long double>(voxel.state)
        * static_cast<long double>(bridge.divergence_flux(i));
  }
  return result;
}

}  // namespace ftd::eft
