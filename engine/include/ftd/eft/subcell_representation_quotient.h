#pragma once
/**
 * @file subcell_representation_quotient.h
 * @brief Exact overlapping-chart quotient of site + subcell remainder
 *        (FTD-0498).
 *
 * Observer only.  These helpers characterize the redundancy already present
 * in the frozen representation; they do not declare the chart index gauge or
 * alter any production state.
 */

#include "ftd/lattice.h"
#include "ftd/voxel.h"

#include <vector>

namespace ftd::eft {

struct SubcellChart {
  Coord anchor{};
  Vec3 remainder{};
  bool valid = false;
};

/// Enumerate all stable charts with remainder components strictly in (-1,1)
/// representing the supplied unwrapped effective position.
std::vector<SubcellChart> enumerate_subcell_charts(
    const Vec3& effective_position);

/// Exact projection pi(anchor,remainder)=anchor+remainder.
Vec3 subcell_chart_position(const SubcellChart& chart);

/// True when both charts are valid and have the same effective position to
/// the supplied componentwise tolerance.
bool equivalent_subcell_charts(const SubcellChart& lhs,
                               const SubcellChart& rhs,
                               double tolerance = 1e-12);

/// Apply the existing componentwise +/-1 threshold transition without a
/// periodic wrap.  Arbitrary finite displacements are supported by repeated
/// chart transitions; one production tick normally needs at most one.
SubcellChart translate_subcell_chart(
    const SubcellChart& start,
    const Vec3& displacement);

}  // namespace ftd::eft
