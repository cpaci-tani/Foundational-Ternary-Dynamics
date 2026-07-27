#pragma once
/**
 * @file coupled_wave_tick.h
 * @brief Reversible observer form of the production wave/coupling kick-drift.
 */

#include "ftd/render_bridge.h"

#include <vector>

namespace ftd::eft {

inline void advance_coupled_wave_tick(RenderBridge& bridge) {
  const std::size_t count = bridge.voxels().size();
  std::vector<Vec3> next_flux(count);
  std::vector<Vec3> next_wave_vel(count);
  for (int index = 0; index < static_cast<int>(count); ++index) {
    const Vec3 acceleration =
        bridge.laplacian_flux(index) * (C_WAVE * C_WAVE)
        - bridge.gradient_state(index) * G_C
        + bridge.curl_state_velocity(index) * G_C;
    next_wave_vel[static_cast<std::size_t>(index)] =
        bridge.voxels()[static_cast<std::size_t>(index)].wave_vel
        + acceleration;
    next_flux[static_cast<std::size_t>(index)] =
        bridge.voxels()[static_cast<std::size_t>(index)].flux
        + next_wave_vel[static_cast<std::size_t>(index)];
  }
  for (std::size_t index = 0; index < count; ++index) {
    bridge.voxels()[index].flux = next_flux[index];
    bridge.voxels()[index].wave_vel = next_wave_vel[index];
  }
}

inline void reverse_coupled_wave_tick(RenderBridge& bridge) {
  const std::size_t count = bridge.voxels().size();
  std::vector<Vec3> previous_flux(count);
  std::vector<Vec3> current_wave_vel(count);
  for (std::size_t index = 0; index < count; ++index) {
    current_wave_vel[index] = bridge.voxels()[index].wave_vel;
    previous_flux[index] = bridge.voxels()[index].flux
        - current_wave_vel[index];
    bridge.voxels()[index].flux = previous_flux[index];
  }
  for (int index = 0; index < static_cast<int>(count); ++index) {
    const Vec3 acceleration =
        bridge.laplacian_flux(index) * (C_WAVE * C_WAVE)
        - bridge.gradient_state(index) * G_C
        + bridge.curl_state_velocity(index) * G_C;
    bridge.voxels()[static_cast<std::size_t>(index)].wave_vel =
        current_wave_vel[static_cast<std::size_t>(index)] - acceleration;
  }
}

}  // namespace ftd::eft

