#pragma once
/**
 * @file discrete_interaction_work.h
 * @brief Exact finite-site virtual work for L_int = G_C sum_x s_x div(J)_x.
 *
 * Observer/analysis helper only.  It does not participate in the production
 * tick and does not define a continuous interpolation of ternary state.
 */

#include "ftd/constants.h"
#include "ftd/voxel.h"

#include <cmath>
#include <cstdint>
#include <vector>

namespace ftd::eft {

inline double discrete_interaction_action(
    const std::vector<std::int8_t>& state,
    const std::vector<double>& divergence) {
  if (state.size() != divergence.size()) return 0.0;
  double action = 0.0;
  for (std::size_t i = 0; i < state.size(); ++i)
    action += G_C * static_cast<double>(state[i]) * divergence[i];
  return action;
}

inline double discrete_hop_work(std::int8_t charge,
                                double divergence_from,
                                double divergence_to) {
  return G_C * static_cast<double>(charge)
      * (divergence_to - divergence_from);
}

inline Vec3 symmetric_interaction_force(std::int8_t charge,
                                        double phi_x_plus,
                                        double phi_x_minus,
                                        double phi_y_plus,
                                        double phi_y_minus,
                                        double phi_z_plus,
                                        double phi_z_minus) {
  const double vertex = G_C * static_cast<double>(charge) * 0.5;
  return {vertex * (phi_x_plus - phi_x_minus),
          vertex * (phi_y_plus - phi_y_minus),
          vertex * (phi_z_plus - phi_z_minus)};
}

struct DiscreteHopWorkResult {
  double action_before = 0.0;
  double action_after = 0.0;
  double action_change = 0.0;
  double endpoint_work = 0.0;
  double residual = 0.0;
  bool valid = false;
};

inline DiscreteHopWorkResult evaluate_discrete_hop_work(
    const std::vector<std::int8_t>& state,
    const std::vector<double>& divergence,
    std::size_t from,
    std::size_t to,
    std::int8_t charge) {
  DiscreteHopWorkResult result;
  if (state.size() != divergence.size() || from >= state.size()
      || to >= state.size() || from == to || charge == 0
      || state[from] != charge)
    return result;

  auto moved = state;
  const int from_after = static_cast<int>(moved[from])
      - static_cast<int>(charge);
  const int to_after = static_cast<int>(moved[to])
      + static_cast<int>(charge);
  if (from_after < -1 || from_after > 1 || to_after < -1 || to_after > 1)
    return result;
  moved[from] = static_cast<std::int8_t>(from_after);
  moved[to] = static_cast<std::int8_t>(to_after);

  result.action_before = discrete_interaction_action(state, divergence);
  result.action_after = discrete_interaction_action(moved, divergence);
  result.action_change = result.action_after - result.action_before;
  result.endpoint_work = discrete_hop_work(
      charge, divergence[from], divergence[to]);
  result.residual = result.action_change - result.endpoint_work;
  result.valid = std::isfinite(result.action_before)
      && std::isfinite(result.action_after)
      && std::isfinite(result.endpoint_work)
      && std::isfinite(result.residual);
  return result;
}

}  // namespace ftd::eft
