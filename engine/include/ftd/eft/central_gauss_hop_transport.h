#pragma once
/**
 * @file central_gauss_hop_transport.h
 * @brief Observer-only realizability of one-site source transport under the
 *        production cell-centered central divergence.
 *
 * A cell-centered component J_i(x) contributes to central divergence at
 * x-e_i and x+e_i.  It is therefore an oriented edge on the step-two graph.
 * On even periodic L that graph splits by checkerboard parity, so an adjacent
 * source/sink pair is outside the divergence image.  On odd L the graph is
 * connected, but the shortest axial path has (L-1)/2 edges.
 */

#include "ftd/field_operators.h"
#include "ftd/lattice.h"
#include "ftd/voxel.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace ftd::eft {

struct CentralGaussHopTransport {
  int L = 0;
  int source = -1;
  int target = -1;
  int axis = -1;
  int direction = 0;
  int charge = 0;
  int graph_steps = 0;
  int support_sites = 0;
  double desired_parity_pairing = 0.0;
  double gauss_residual = 0.0;
  bool realizable = false;
  bool valid = false;
  std::vector<Vec3> flux_delta;
};

inline double parity_character(const Coord& coordinate, int axis) {
  int value = coordinate.x;
  if (axis == 1) value = coordinate.y;
  if (axis == 2) value = coordinate.z;
  return (value & 1) == 0 ? 1.0 : -1.0;
}

inline CentralGaussHopTransport construct_central_gauss_face_hop(
    int L, int source, int axis, int direction, int charge) {
  CentralGaussHopTransport result;
  result.L = L;
  result.source = source;
  result.axis = axis;
  result.direction = direction;
  result.charge = charge;
  if (L <= 2 || source < 0 || source >= L * L * L
      || axis < 0 || axis >= 3
      || (direction != -1 && direction != 1)
      || (charge != -1 && charge != 1)) {
    return result;
  }

  Lattice geometry(L);
  const Coord source_coordinate = geometry.coord(source);
  int tx = source_coordinate.x;
  int ty = source_coordinate.y;
  int tz = source_coordinate.z;
  if (axis == 0) tx += direction;
  if (axis == 1) ty += direction;
  if (axis == 2) tz += direction;
  result.target = geometry.index(tx, ty, tz);
  const Coord target_coordinate = geometry.coord(result.target);
  result.desired_parity_pairing =
      -static_cast<double>(charge)
          * parity_character(source_coordinate, axis)
      + static_cast<double>(charge)
          * parity_character(target_coordinate, axis);

  result.flux_delta.assign(static_cast<std::size_t>(L * L * L), {});
  if ((L & 1) == 0) {
    result.realizable = false;
    result.valid = std::abs(result.desired_parity_pairing) == 2.0;
    return result;
  }

  result.graph_steps = (L - 1) / 2;
  Coord current = source_coordinate;
  for (int step = 0; step < result.graph_steps; ++step) {
    int cx = current.x;
    int cy = current.y;
    int cz = current.z;
    if (axis == 0) cx -= direction;
    if (axis == 1) cy -= direction;
    if (axis == 2) cz -= direction;
    const int center = geometry.index(cx, cy, cz);
    const double value = 2.0 * static_cast<double>(charge * direction);
    if (axis == 0)
      result.flux_delta[static_cast<std::size_t>(center)].x += value;
    if (axis == 1)
      result.flux_delta[static_cast<std::size_t>(center)].y += value;
    if (axis == 2)
      result.flux_delta[static_cast<std::size_t>(center)].z += value;
    if (axis == 0) current.x -= 2 * direction;
    if (axis == 1) current.y -= 2 * direction;
    if (axis == 2) current.z -= 2 * direction;
    current.x = (current.x % L + L) % L;
    current.y = (current.y % L + L) % L;
    current.z = (current.z % L + L) % L;
  }

  const int endpoint = geometry.index(current.x, current.y, current.z);
  result.support_sites = static_cast<int>(std::count_if(
      result.flux_delta.begin(), result.flux_delta.end(),
      [](const Vec3& value) { return value.mag2() > 0.0; }));

  std::vector<Voxel> observer(result.flux_delta.size());
  for (std::size_t index = 0; index < result.flux_delta.size(); ++index)
    observer[index].flux = result.flux_delta[index];
  for (int index = 0; index < L * L * L; ++index) {
    double desired = 0.0;
    if (index == result.source) desired -= static_cast<double>(charge);
    if (index == result.target) desired += static_cast<double>(charge);
    result.gauss_residual = std::max(result.gauss_residual,
        std::abs(divergence_flux_op(observer, geometry, index) - desired));
  }
  result.realizable = endpoint == result.target;
  result.valid = result.realizable
      && result.support_sites == result.graph_steps
      && std::isfinite(result.gauss_residual);
  return result;
}

}  // namespace ftd::eft
