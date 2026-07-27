#pragma once
/**
 * @file discrete_legendre_worldline.h
 * @brief Interior discrete Legendre transform of the FTD-0484 action.
 *
 * FTD-0490 uses the existing relativistic production dispersion and exact
 * cubical worldline coupling. It is an observer and does not update engine
 * state.
 */

#include "ftd/eft/spacetime_worldline_coupling.h"

namespace ftd::eft {

struct DiscreteLegendreWorldlineResult {
  bool valid = false;
  int L = 0;
  int charge = 0;
  double rest_energy = 0.0;
  double c_speed = 0.0;
  double temporal_scale = 0.0;
  double coupling = 0.0;
  Vec3 start_position{};
  Vec3 end_position{};
  Vec3 displacement{};
  double matter_action = 0.0;
  double interaction_action = 0.0;
  double total_action = 0.0;
  Vec3 d1_matter{};
  Vec3 d2_matter{};
  Vec3 d1_interaction{};
  Vec3 d2_interaction{};
  Vec3 canonical_start{};
  Vec3 canonical_end{};
  Vec3 connection_start{};
  Vec3 connection_end{};
  Vec3 kinetic_start{};
  Vec3 kinetic_end{};
  double deposited_action_residual = 0.0;
  double dispersion_residual = 0.0;
};

/** Evaluate one straight within-cell discrete action and both endpoint maps.
 *
 * temporal_scale is lambda=c*dt. Segments touching or crossing a cell face
 * are deliberately rejected by this interior-only observer.
 */
DiscreteLegendreWorldlineResult evaluate_discrete_legendre_worldline(
    const Vec3& start_position,
    const Vec3& end_position,
    int charge,
    double rest_energy,
    double c_speed,
    const DualGaugePotentialSlab& slab,
    double coupling = 1.0);

/// Analytic free displacement corresponding to the production dispersion.
Vec3 free_displacement_from_momentum(const Vec3& momentum,
                                     double rest_energy,
                                     double c_speed,
                                     double temporal_scale);

}  // namespace ftd::eft
