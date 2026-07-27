#pragma once
/**
 * @file quadratic_coat_matter_work.h
 * @brief Endpoint Legendre and matter-work audit for the smooth coat (FTD-0545).
 */

#include "ftd/eft/quadratic_coat_spacetime_action.h"

namespace ftd::eft {

struct QuadraticCoatMatterWorkResult {
  bool valid = false;
  bool derivative_smooth = false;
  int charge = 0;
  double rest_energy = 0.0;
  double c_speed = 0.0;
  double temporal_scale = 0.0;
  double beta = 0.0;
  double coupling = 0.0;
  Vec3 start_position{};
  Vec3 end_position{};
  Vec3 displacement{};
  QuadraticCoatSpacetimeCurrent current{};

  double matter_action = 0.0;
  double direct_interaction_action = 0.0;
  double deposited_interaction_action = 0.0;
  double deposited_action_residual = 0.0;
  Vec3 d1_interaction{};
  Vec3 d2_interaction{};
  Vec3 free_momentum{};
  Vec3 canonical_start{};
  Vec3 canonical_end{};
  Vec3 connection_start{};
  Vec3 connection_end{};
  Vec3 kinetic_start{};
  Vec3 kinetic_end{};
  MatchedFaceFlux electric{};
  double matter_energy_before = 0.0;
  double matter_energy_after = 0.0;
  double matter_energy_change = 0.0;
  double field_work = 0.0;
  double matter_work_defect = 0.0;
};

QuadraticCoatMatterWorkResult evaluate_quadratic_coat_matter_work(
    const Vec3& start_position,
    const Vec3& end_position,
    int charge,
    double rest_energy,
    double c_speed,
    double beta,
    const DualGaugePotentialSlab& slab);

}  // namespace ftd::eft
