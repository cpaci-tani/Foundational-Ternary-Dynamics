#pragma once
/**
 * Field operators — discrete differential operators on the lattice.
 *
 * Extracted from render_bridge.cpp in the 2026-04-18 R6 refactor.
 * THESE MUST STAY INLINE: they are called per-voxel per-tick from phase
 * kernels and diagnostic reductions. Moving them to a .cpp TU would
 * depend on LTO to recover performance; we preserve header-inline as
 * the defensive choice.
 *
 * The free functions here take (const std::vector<Voxel>&, const Lattice&, int idx)
 * and return pure values — no state mutation. RenderBridge methods
 * (render_bridge.h) are thin inline forwarders that delegate here.
 */

#include <cassert>
#include <cmath>
#include <vector>
#include "lattice.h"
#include "voxel.h"

namespace ftd {

// 18-point isotropic Laplacian templated on Voxel field pointer.
// (1/3)·face_sum + (1/6)·edge_sum − 4·center cancels O(k^4) anisotropy.
template <Vec3 Voxel::*F>
inline Vec3 laplacian_field(const std::vector<Voxel>& voxels, const Lattice& lattice, int idx) {
  const auto& face = lattice.neighbors_6(idx);
  const auto& edge = lattice.neighbors_12(idx);
  Vec3 lap;
  for (int n : face) lap += voxels[n].*F * (1.0/3.0);
  for (int n : edge) lap += voxels[n].*F * (1.0/6.0);
  lap -= voxels[idx].*F * 4.0;
  return lap;
}

inline Vec3 laplacian_flux_op(const std::vector<Voxel>& voxels, const Lattice& lattice, int idx) {
  Vec3 lap = laplacian_field<&Voxel::flux>(voxels, lattice, idx);
  assert(!std::isnan(lap.x) && !std::isnan(lap.y) && !std::isnan(lap.z));
  return lap;
}

inline double divergence_flux_op(const std::vector<Voxel>& voxels, const Lattice& lattice, int idx) {
  const auto& nbrs = lattice.neighbors_6(idx);
  double div = 0.0;
  div += (voxels[nbrs[0]].flux.x - voxels[nbrs[1]].flux.x) * 0.5;
  div += (voxels[nbrs[2]].flux.y - voxels[nbrs[3]].flux.y) * 0.5;
  div += (voxels[nbrs[4]].flux.z - voxels[nbrs[5]].flux.z) * 0.5;
  return div;
}

// ARCH-7b: divergence variant that reads from an explicit Vec3 array
// rather than voxels[].flux. Race-free pair to curl_from_flux_array.
inline double divergence_from_flux_array(const std::vector<Vec3>& flux,
                                          const Lattice& lattice, int idx) {
  const auto& nbrs = lattice.neighbors_6(idx);
  double div = 0.0;
  div += (flux[nbrs[0]].x - flux[nbrs[1]].x) * 0.5;
  div += (flux[nbrs[2]].y - flux[nbrs[3]].y) * 0.5;
  div += (flux[nbrs[4]].z - flux[nbrs[5]].z) * 0.5;
  return div;
}

inline Vec3 curl_flux_op(const std::vector<Voxel>& voxels, const Lattice& lattice, int idx) {
  const auto& n = lattice.neighbors_6(idx);
  Vec3 curl;
  curl.x = (voxels[n[2]].flux.z - voxels[n[3]].flux.z) * 0.5 -
           (voxels[n[4]].flux.y - voxels[n[5]].flux.y) * 0.5;
  curl.y = (voxels[n[4]].flux.x - voxels[n[5]].flux.x) * 0.5 -
           (voxels[n[0]].flux.z - voxels[n[1]].flux.z) * 0.5;
  curl.z = (voxels[n[0]].flux.y - voxels[n[1]].flux.y) * 0.5 -
           (voxels[n[2]].flux.x - voxels[n[3]].flux.x) * 0.5;
  assert(!std::isnan(curl.x) && !std::isnan(curl.y) && !std::isnan(curl.z));
  return curl;
}

// ARCH-7b (2026-04-25): variant reading flux from an explicit Vec3 array.
// Used by phase_write genesis on a pre-write snapshot, so the curl read
// doesn't race against concurrent threads updating voxel flux in the same
// parallel pass. See engine/src/render_bridge.cpp::phase_write.
inline Vec3 curl_from_flux_array(const std::vector<Vec3>& flux,
                                  const Lattice& lattice, int idx) {
  const auto& n = lattice.neighbors_6(idx);
  Vec3 curl;
  curl.x = (flux[n[2]].z - flux[n[3]].z) * 0.5 -
           (flux[n[4]].y - flux[n[5]].y) * 0.5;
  curl.y = (flux[n[4]].x - flux[n[5]].x) * 0.5 -
           (flux[n[0]].z - flux[n[1]].z) * 0.5;
  curl.z = (flux[n[0]].y - flux[n[1]].y) * 0.5 -
           (flux[n[2]].x - flux[n[3]].x) * 0.5;
  return curl;
}

inline Vec3 gradient_scalar_op(const Lattice& lattice, int idx, const std::vector<double>& field) {
  const auto& nbrs = lattice.neighbors_6(idx);
  Vec3 grad;
  grad.x = (field[nbrs[0]] - field[nbrs[1]]) * 0.5;
  grad.y = (field[nbrs[2]] - field[nbrs[3]]) * 0.5;
  grad.z = (field[nbrs[4]] - field[nbrs[5]]) * 0.5;
  assert(!std::isnan(grad.x) && !std::isnan(grad.y) && !std::isnan(grad.z));
  return grad;
}

inline Vec3 gradient_state_op(const std::vector<Voxel>& voxels, const Lattice& lattice, int idx) {
  const auto& n = lattice.neighbors_6(idx);
  Vec3 grad;
  grad.x = (voxels[n[0]].state - voxels[n[1]].state) * 0.5;
  grad.y = (voxels[n[2]].state - voxels[n[3]].state) * 0.5;
  grad.z = (voxels[n[4]].state - voxels[n[5]].state) * 0.5;
  return grad;
}

inline Vec3 gradient_density_op(const std::vector<Voxel>& voxels, const Lattice& lattice, int idx) {
  const auto& n = lattice.neighbors_6(idx);
  Vec3 grad;
  grad.x = (voxels[n[0]].density() - voxels[n[1]].density()) * 0.5;
  grad.y = (voxels[n[2]].density() - voxels[n[3]].density()) * 0.5;
  grad.z = (voxels[n[4]].density() - voxels[n[5]].density()) * 0.5;
  return grad;
}

inline Vec3 gradient_divergence_op(const std::vector<Voxel>& voxels, const Lattice& lattice, int idx) {
  const auto& n = lattice.neighbors_6(idx);
  Vec3 grad;
  grad.x = (divergence_flux_op(voxels, lattice, n[0]) - divergence_flux_op(voxels, lattice, n[1])) * 0.5;
  grad.y = (divergence_flux_op(voxels, lattice, n[2]) - divergence_flux_op(voxels, lattice, n[3])) * 0.5;
  grad.z = (divergence_flux_op(voxels, lattice, n[4]) - divergence_flux_op(voxels, lattice, n[5])) * 0.5;
  return grad;
}

inline Vec3 curl_state_velocity_op(const std::vector<Voxel>& voxels, const Lattice& lattice, int idx) {
  auto c = lattice.coord(idx);
  auto jcur = [&](int x, int y, int z) -> Vec3 {
    int ni = lattice.index(x, y, z);
    return voxels[ni].velocity * static_cast<double>(voxels[ni].state);
  };
  Vec3 curl;
  curl.x = (jcur(c.x, c.y + 1, c.z).z - jcur(c.x, c.y - 1, c.z).z) * 0.5 -
           (jcur(c.x, c.y, c.z + 1).y - jcur(c.x, c.y, c.z - 1).y) * 0.5;
  curl.y = (jcur(c.x, c.y, c.z + 1).x - jcur(c.x, c.y, c.z - 1).x) * 0.5 -
           (jcur(c.x + 1, c.y, c.z).z - jcur(c.x - 1, c.y, c.z).z) * 0.5;
  curl.z = (jcur(c.x + 1, c.y, c.z).y - jcur(c.x - 1, c.y, c.z).y) * 0.5 -
           (jcur(c.x, c.y + 1, c.z).x - jcur(c.x, c.y - 1, c.z).x) * 0.5;
  return curl;
}

// stress = |div(F)| + |curl(F)| + |grad(|F|)| templated on Voxel field pointer.
template <Vec3 Voxel::*F>
inline double stress_field(const std::vector<Voxel>& voxels, const Lattice& lattice, int idx) {
  const auto& nbrs = lattice.neighbors_6(idx);

  double div = 0.0;
  div += ((voxels[nbrs[0]].*F).x - (voxels[nbrs[1]].*F).x) * 0.5;
  div += ((voxels[nbrs[2]].*F).y - (voxels[nbrs[3]].*F).y) * 0.5;
  div += ((voxels[nbrs[4]].*F).z - (voxels[nbrs[5]].*F).z) * 0.5;
  double div_mag = std::abs(div);

  Vec3 curl;
  curl.x = ((voxels[nbrs[2]].*F).z - (voxels[nbrs[3]].*F).z) * 0.5 -
           ((voxels[nbrs[4]].*F).y - (voxels[nbrs[5]].*F).y) * 0.5;
  curl.y = ((voxels[nbrs[4]].*F).x - (voxels[nbrs[5]].*F).x) * 0.5 -
           ((voxels[nbrs[0]].*F).z - (voxels[nbrs[1]].*F).z) * 0.5;
  curl.z = ((voxels[nbrs[0]].*F).y - (voxels[nbrs[1]].*F).y) * 0.5 -
           ((voxels[nbrs[2]].*F).x - (voxels[nbrs[3]].*F).x) * 0.5;
  double curl_mag = curl.mag();

  double gx = ((voxels[nbrs[0]].*F).mag() - (voxels[nbrs[1]].*F).mag()) * 0.5;
  double gy = ((voxels[nbrs[2]].*F).mag() - (voxels[nbrs[3]].*F).mag()) * 0.5;
  double gz = ((voxels[nbrs[4]].*F).mag() - (voxels[nbrs[5]].*F).mag()) * 0.5;
  double grad_mag = std::sqrt(gx*gx + gy*gy + gz*gz);

  return div_mag + curl_mag + grad_mag;
}

}  // namespace ftd
