#pragma once
/**
 * @file coupled_wave_tick_snapshot.h
 * @brief Linear-time snapshot observer for the coupled wave kick-drift.
 *
 * This is algebraically identical to coupled_wave_tick.h. It holds one const
 * voxel snapshot through the stencil loop and takes one mutable handout only
 * after all reads, avoiding a ternary-field rebuild at every lattice site.
 */

#include "ftd/field_operators.h"
#include "ftd/render_bridge.h"

#include <vector>

namespace ftd::eft {

inline Vec3 laplacian_from_flux_snapshot(
    const std::vector<Vec3>& flux, const Lattice& lattice, int index) {
  Vec3 laplacian = flux[static_cast<std::size_t>(index)] * -4.0;
  for (int neighbor : lattice.neighbors_6(index))
    laplacian += flux[static_cast<std::size_t>(neighbor)]
        * LAPLACIAN_FACE_WEIGHT;
  for (int neighbor : lattice.neighbors_12(index))
    laplacian += flux[static_cast<std::size_t>(neighbor)]
        * LAPLACIAN_EDGE_WEIGHT;
  return laplacian;
}

inline void advance_coupled_wave_tick_snapshot(RenderBridge& bridge) {
  const RenderBridge& read_bridge = bridge;
  const auto& voxels = read_bridge.voxels();
  const auto& lattice = read_bridge.lattice();
  const std::size_t count = voxels.size();
  std::vector<Vec3> next_flux(count);
  std::vector<Vec3> next_wave_vel(count);
  for (int index = 0; index < static_cast<int>(count); ++index) {
    const Vec3 acceleration = laplacian_flux_op(voxels, lattice, index)
            * (C_WAVE * C_WAVE)
        - gradient_state_op(voxels, lattice, index) * G_C
        + curl_state_velocity_op(voxels, lattice, index) * G_C;
    next_wave_vel[static_cast<std::size_t>(index)] =
        voxels[static_cast<std::size_t>(index)].wave_vel + acceleration;
    next_flux[static_cast<std::size_t>(index)] =
        voxels[static_cast<std::size_t>(index)].flux
        + next_wave_vel[static_cast<std::size_t>(index)];
  }
  auto& writable = bridge.voxels();
  for (std::size_t index = 0; index < count; ++index) {
    writable[index].flux = next_flux[index];
    writable[index].wave_vel = next_wave_vel[index];
  }
}

inline void reverse_coupled_wave_tick_snapshot(RenderBridge& bridge) {
  const RenderBridge& read_bridge = bridge;
  const auto& voxels = read_bridge.voxels();
  const auto& lattice = read_bridge.lattice();
  const std::size_t count = voxels.size();
  std::vector<Vec3> previous_flux(count);
  std::vector<Vec3> previous_wave_vel(count);
  std::vector<Vec3> current_wave_vel(count);
  for (std::size_t index = 0; index < count; ++index) {
    current_wave_vel[index] = voxels[index].wave_vel;
    previous_flux[index] = voxels[index].flux - current_wave_vel[index];
  }
  for (int index = 0; index < static_cast<int>(count); ++index) {
    const Vec3 acceleration = laplacian_from_flux_snapshot(
            previous_flux, lattice, index) * (C_WAVE * C_WAVE)
        - gradient_state_op(voxels, lattice, index) * G_C
        + curl_state_velocity_op(voxels, lattice, index) * G_C;
    previous_wave_vel[static_cast<std::size_t>(index)] =
        current_wave_vel[static_cast<std::size_t>(index)] - acceleration;
  }
  auto& writable = bridge.voxels();
  for (std::size_t index = 0; index < count; ++index) {
    writable[index].flux = previous_flux[index];
    writable[index].wave_vel = previous_wave_vel[index];
  }
}

}  // namespace ftd::eft
