#pragma once
/**
 * @file moore_link_routes.h
 * @brief Analysis-only routing of one Moore hop through oriented SC faces.
 *
 * A face/edge/corner Moore hop is primitive in the production movement rule.
 * Representing an edge or corner hop on the oriented-face continuity complex
 * requires an ordering of its nonzero Cartesian components.  This helper
 * exposes that ordering; it does not select one as physical.
 */

#include "ftd/eft/dual_cell_continuity.h"

#include <array>
#include <cmath>

namespace ftd::eft {

inline bool valid_axis_order(const std::array<int, 3>& order) {
  std::array<bool, 3> seen{{false, false, false}};
  for (const int axis : order) {
    if (axis < 0 || axis > 2 || seen[static_cast<std::size_t>(axis)])
      return false;
    seen[static_cast<std::size_t>(axis)] = true;
  }
  return true;
}

inline void add_route_face(DualCellContinuity& history,
                           int x, int y, int z,
                           int axis, int direction, int charge) {
  if (direction == 0) return;
  if (axis == 0) {
    if (direction > 0)
      history.current_x[static_cast<std::size_t>(history.index(x, y, z))]
          += charge;
    else
      history.current_x[static_cast<std::size_t>(history.index(x - 1, y, z))]
          -= charge;
  } else if (axis == 1) {
    if (direction > 0)
      history.current_y[static_cast<std::size_t>(history.index(x, y, z))]
          += charge;
    else
      history.current_y[static_cast<std::size_t>(history.index(x, y - 1, z))]
          -= charge;
  } else {
    if (direction > 0)
      history.current_z[static_cast<std::size_t>(history.index(x, y, z))]
          += charge;
    else
      history.current_z[static_cast<std::size_t>(history.index(x, y, z - 1))]
          -= charge;
  }
}

inline DualCellContinuity route_single_moore_hop(
    int L, int source_index, const std::array<int, 3>& delta,
    int charge, const std::array<int, 3>& order) {
  DualCellContinuity history(L);
  if (L <= 2 || source_index < 0 || source_index >= history.total_sites()
      || charge == 0 || !valid_axis_order(order))
    return history;
  int nonzero = 0;
  for (const int component : delta) {
    if (component < -1 || component > 1) return DualCellContinuity(L);
    nonzero += component != 0 ? 1 : 0;
  }
  if (nonzero == 0) return DualCellContinuity(L);

  int z = source_index % L;
  const int xy = source_index / L;
  int y = xy % L;
  int x = xy / L;
  history.rho_before[static_cast<std::size_t>(source_index)] = charge;
  for (const int axis : order) {
    const int direction = delta[static_cast<std::size_t>(axis)];
    add_route_face(history, x, y, z, axis, direction, charge);
    if (axis == 0) x = (x + direction + L) % L;
    if (axis == 1) y = (y + direction + L) % L;
    if (axis == 2) z = (z + direction + L) % L;
  }
  history.rho_after[static_cast<std::size_t>(history.index(x, y, z))] = charge;
  return history;
}

inline double current_l2_distance(const DualCellContinuity& a,
                                  const DualCellContinuity& b) {
  if (a.L != b.L || a.current_x.size() != b.current_x.size()) return -1.0;
  double norm2 = 0.0;
  for (std::size_t i = 0; i < a.current_x.size(); ++i) {
    const double dx = a.current_x[i] - b.current_x[i];
    const double dy = a.current_y[i] - b.current_y[i];
    const double dz = a.current_z[i] - b.current_z[i];
    norm2 += dx * dx + dy * dy + dz * dz;
  }
  return std::sqrt(norm2);
}

}  // namespace ftd::eft
