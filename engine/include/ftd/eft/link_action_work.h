#pragma once
/**
 * @file link_action_work.h
 * @brief Observer-only comparison between site-gradient impulse and exact
 *        finite-link interaction work.
 *
 * For the written interaction H_int = -G_C sum_x s div(J), moving charge q
 * from site a to a face neighbour b changes the interaction exactly by
 *
 *   W_hop = G_C q [div(J)(b) - div(J)(a)].
 *
 * A trapezoidal use of the site force G_C q grad(div J) supplies instead
 *
 *   W_site = (G_C q / 2) [grad divJ(a) + grad divJ(b)] . (b-a).
 *
 * These agree for affine/quadratic potentials but not for a general lattice
 * field.  The exact event impulse along the oriented face link is
 *
 *   I_link = W_hop (b-a),
 *
 * because |b-a|^2=1.  This helper observes those quantities and does not
 * modify RenderBridge or select whether a hop occurs.
 */

#include "ftd/constants.h"
#include "ftd/render_bridge.h"

#include <cmath>

namespace ftd::eft {

struct LinkActionWork {
  int source = -1;
  int target = -1;
  int charge = 0;
  Vec3 displacement{};
  double divergence_source = 0.0;
  double divergence_target = 0.0;
  double exact_work = 0.0;
  Vec3 centered_site_impulse{};
  double centered_site_work = 0.0;
  Vec3 exact_link_impulse{};
  double exact_link_work = 0.0;
  double centered_defect = 0.0;
  double link_residual = 0.0;
  bool valid = false;
};

inline LinkActionWork measure_face_link_action_work(
    const RenderBridge& bridge, int source, int axis, int direction,
    int charge) {
  LinkActionWork result;
  result.source = source;
  result.charge = charge;
  if (source < 0
      || source >= static_cast<int>(bridge.voxels().size())
      || axis < 0 || axis >= 3
      || (direction != -1 && direction != 1)
      || (charge != -1 && charge != 1)) {
    return result;
  }

  const auto coordinate = bridge.lattice().coord(source);
  int x = coordinate.x;
  int y = coordinate.y;
  int z = coordinate.z;
  if (axis == 0) x += direction;
  if (axis == 1) y += direction;
  if (axis == 2) z += direction;
  result.target = bridge.lattice().index(x, y, z);
  if (axis == 0) result.displacement.x = static_cast<double>(direction);
  if (axis == 1) result.displacement.y = static_cast<double>(direction);
  if (axis == 2) result.displacement.z = static_cast<double>(direction);

  result.divergence_source = bridge.divergence_flux(source);
  result.divergence_target = bridge.divergence_flux(result.target);
  const double vertex = G_C * static_cast<double>(charge);
  result.exact_work = vertex
      * (result.divergence_target - result.divergence_source);
  result.centered_site_impulse =
      (bridge.gradient_divergence(source)
       + bridge.gradient_divergence(result.target)) * (0.5 * vertex);
  result.centered_site_work =
      result.centered_site_impulse.dot(result.displacement);
  result.exact_link_impulse = result.displacement * result.exact_work;
  result.exact_link_work = result.exact_link_impulse.dot(
      result.displacement);
  result.centered_defect =
      result.centered_site_work - result.exact_work;
  result.link_residual = result.exact_link_work - result.exact_work;
  result.valid = std::isfinite(result.divergence_source)
      && std::isfinite(result.divergence_target)
      && std::isfinite(result.exact_work)
      && std::isfinite(result.centered_site_work)
      && std::isfinite(result.exact_link_work)
      && std::isfinite(result.centered_defect)
      && std::isfinite(result.link_residual);
  return result;
}

}  // namespace ftd::eft
