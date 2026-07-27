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

namespace ftd::eft {

struct NativeWaveEnergy {
  long double amplitude = 0.0L;
  long double kinetic = 0.0L;
  long double gradient = 0.0L;
  long double cross = 0.0L;
  long double naive = 0.0L;
  long double tick_invariant = 0.0L;
  bool finite = true;
};

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

