#pragma once
/**
 * @file localized_transverse_packet.h
 * @brief Observer-side finite divergence-free packet and reversible wave tick.
 */

#include "ftd/render_bridge.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace ftd::eft {

struct LocalizedPacketSpec {
  double x0 = 0.0;
  double y0 = 0.0;
  double z0 = 0.0;
  double sigma_x = 3.0;
  double sigma_t = 3.0;
  double amplitude = 1.0;
  int direction = 1;
  double carrier_k = PI / 4.0;
  double carrier_phase = 0.0;
};

inline Vec3 packet_laplacian(const RenderBridge& bridge,
                             const std::vector<Vec3>& field, int index) {
  Vec3 lap = field[static_cast<std::size_t>(index)] * -4.0;
  for (int neighbor : bridge.lattice().neighbors_6(index))
    lap += field[static_cast<std::size_t>(neighbor)]
        * LAPLACIAN_FACE_WEIGHT;
  for (int neighbor : bridge.lattice().neighbors_12(index))
    lap += field[static_cast<std::size_t>(neighbor)]
        * LAPLACIAN_EDGE_WEIGHT;
  return lap;
}

inline void seed_localized_transverse_packet(
    RenderBridge& bridge, const LocalizedPacketSpec& spec) {
  for (auto& voxel : bridge.voxels()) voxel = {};
  const int length = bridge.lattice().size();
  const double sigma_x = std::max(1.0, spec.sigma_x);
  const double sigma_t = std::max(1.0, spec.sigma_t);
  const double psi_amplitude = spec.amplitude * sigma_t;
  const auto periodic_delta = [length](double a, double b) {
    double delta = a - b;
    while (delta > 0.5 * length) delta -= length;
    while (delta < -0.5 * length) delta += length;
    return delta;
  };
  const auto psi = [&](double x, double y, double z) {
    const double dx = periodic_delta(x, spec.x0);
    const double dy = periodic_delta(y, spec.y0);
    const double dz = periodic_delta(z, spec.z0);
    const double radius2 = dx * dx / (sigma_x * sigma_x)
        + (dy * dy + dz * dz) / (sigma_t * sigma_t);
    if (radius2 > 18.0) return 0.0;
    return psi_amplitude * std::exp(-0.5 * radius2)
        * std::cos(spec.carrier_k * dx + spec.carrier_phase);
  };
  const auto field = [&](double x, double y, double z) {
    return Vec3{0.0,
                0.5 * (psi(x, y, z + 1.0) - psi(x, y, z - 1.0)),
                -0.5 * (psi(x, y + 1.0, z) - psi(x, y - 1.0, z))};
  };

  std::vector<Vec3> flux(bridge.voxels().size());
  for (int z = 0; z < length; ++z)
    for (int y = 0; y < length; ++y)
      for (int x = 0; x < length; ++x) {
        const int index = bridge.lattice().index(x, y, z);
        flux[static_cast<std::size_t>(index)] = field(x, y, z);
      }
  const double sign = spec.direction >= 0 ? 1.0 : -1.0;
  for (int z = 0; z < length; ++z)
    for (int y = 0; y < length; ++y)
      for (int x = 0; x < length; ++x) {
        const int index = bridge.lattice().index(x, y, z);
        const int plus = bridge.lattice().index(x + 1, y, z);
        const int minus = bridge.lattice().index(x - 1, y, z);
        const Vec3 derivative =
            (flux[static_cast<std::size_t>(plus)]
             - flux[static_cast<std::size_t>(minus)]) * 0.5;
        bridge.voxels()[static_cast<std::size_t>(index)].flux =
            flux[static_cast<std::size_t>(index)];
        bridge.voxels()[static_cast<std::size_t>(index)].wave_vel =
            derivative * (-sign * C_WAVE)
            - packet_laplacian(bridge, flux, index)
                * (0.5 * C_WAVE * C_WAVE);
      }
}

inline void advance_source_free_wave(RenderBridge& bridge) {
  const std::size_t count = bridge.voxels().size();
  std::vector<Vec3> next_flux(count);
  std::vector<Vec3> next_wave_vel(count);
  for (int index = 0; index < static_cast<int>(count); ++index) {
    next_wave_vel[static_cast<std::size_t>(index)] =
        bridge.voxels()[static_cast<std::size_t>(index)].wave_vel
        + bridge.laplacian_flux(index) * (C_WAVE * C_WAVE);
    next_flux[static_cast<std::size_t>(index)] =
        bridge.voxels()[static_cast<std::size_t>(index)].flux
        + next_wave_vel[static_cast<std::size_t>(index)];
  }
  for (std::size_t index = 0; index < count; ++index) {
    bridge.voxels()[index].flux = next_flux[index];
    bridge.voxels()[index].wave_vel = next_wave_vel[index];
  }
}

inline void reverse_source_free_wave(RenderBridge& bridge) {
  const std::size_t count = bridge.voxels().size();
  std::vector<Vec3> previous_flux(count);
  std::vector<Vec3> current_wave_vel(count);
  for (std::size_t index = 0; index < count; ++index) {
    current_wave_vel[index] = bridge.voxels()[index].wave_vel;
    previous_flux[index] = bridge.voxels()[index].flux
        - current_wave_vel[index];
  }
  for (int index = 0; index < static_cast<int>(count); ++index) {
    bridge.voxels()[static_cast<std::size_t>(index)].flux =
        previous_flux[static_cast<std::size_t>(index)];
    bridge.voxels()[static_cast<std::size_t>(index)].wave_vel =
        current_wave_vel[static_cast<std::size_t>(index)]
        - packet_laplacian(bridge, previous_flux, index)
            * (C_WAVE * C_WAVE);
  }
}

inline double wave_state_max_residual(
    const RenderBridge& lhs, const RenderBridge& rhs) {
  if (lhs.voxels().size() != rhs.voxels().size()) return INFINITY;
  double residual = 0.0;
  for (std::size_t index = 0; index < lhs.voxels().size(); ++index) {
    residual = std::max(residual,
        (lhs.voxels()[index].flux - rhs.voxels()[index].flux).mag());
    residual = std::max(residual,
        (lhs.voxels()[index].wave_vel
         - rhs.voxels()[index].wave_vel).mag());
  }
  return residual;
}

}  // namespace ftd::eft

