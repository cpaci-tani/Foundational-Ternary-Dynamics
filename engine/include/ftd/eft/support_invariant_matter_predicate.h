#pragma once
/**
 * @file support_invariant_matter_predicate.h
 * @brief State-only relational-core predicate for FTD-0755.
 *
 * The predicate intentionally excludes every support-dependent field energy,
 * characteristic, visualization, history label, and future state.  It is an
 * observer for the selected DerivedCompactPair research sector only.
 */

#include "ftd/eft/connected_moore_block_action.h"

namespace ftd::eft {

struct SupportInvariantMatterPredicate {
  bool valid = false;
  bool state_only = true;
  bool support_independent = true;
  bool sector_valid = false;
  bool member = false;
  int constituent_count = 0;
  int net_charge = 0;
  Vec3 center{};
  Vec3 relative_position{};
  Vec3 total_momentum{};
  double separation_squared = 0.0;
  double graph_margin = 0.0;
  double constituent_kinetic_energy = 0.0;
  double binding_energy = 0.0;
  double pair_energy = 0.0;
  double energy_margin = 0.0;
};

SupportInvariantMatterPredicate observe_support_invariant_matter(
    const ConnectedMooreBlockState& state,
    const ConnectedMooreBlockOptions& options = {});

}  // namespace ftd::eft
