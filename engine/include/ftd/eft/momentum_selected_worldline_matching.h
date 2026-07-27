#pragma once
/**
 * @file momentum_selected_worldline_matching.h
 * @brief Exact free discrete-Legendre endpoint permutation matcher
 *        (FTD-0503).
 */

#include "ftd/eft/discrete_legendre_worldline.h"
#include "ftd/eft/multibody_shape_observability.h"

#include <vector>

namespace ftd::eft {

struct PhaseSpaceCarrier {
  Vec3 start_position{};
  Vec3 kinetic_momentum{};
  int charge = 0;
};

struct MomentumSelectedMatching {
  bool valid = false;
  bool collision_rule_required = false;
  int carrier_count = 0;
  int permutations_evaluated = 0;
  int valid_permutations = 0;
  int exact_match_count = 0;
  std::vector<int> assignment;
  double best_residual = 0.0;
  double second_best_residual = 0.0;
  double residual_gap = 0.0;
};

/// Enumerate every endpoint permutation (up to 8 carriers) and apply the
/// exact zero-field FTD-0490 kinetic Legendre equation. Duplicate endpoints
/// fail explicitly into collision_rule_required before enumeration.
MomentumSelectedMatching match_free_worldline_endpoints(
    int L,
    const std::vector<PhaseSpaceCarrier>& carriers,
    const std::vector<Vec3>& unordered_endpoints,
    double rest_energy,
    double c_speed,
    double dt = 1.0,
    double tolerance = 1e-12);

/// Reconstruct the selected signed worldline list. Returns empty unless the
/// matching is valid and dimensionally consistent.
std::vector<ShapeWorldline> worldlines_from_matching(
    const std::vector<PhaseSpaceCarrier>& carriers,
    const std::vector<Vec3>& unordered_endpoints,
    const MomentumSelectedMatching& matching);

}  // namespace ftd::eft
