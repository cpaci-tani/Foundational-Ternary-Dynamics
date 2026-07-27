#pragma once
/**
 * @file subcell_polarity_shape.h
 * @brief Compact signed trilinear charge shape for a manifested polarity.
 *
 * A particle represented by an integer anchor site and a signed sub-cell
 * remainder has effective position
 *
 *   x_eff = anchor + remainder.
 *
 * Each remainder component is admitted on the closed movement interval
 * [-1,+1].  The endpoint values are the movement thresholds: a component of
 * +1 deposits entirely on the positive neighbour and -1 entirely on the
 * negative neighbour.  Interior values select at most two sites per axis,
 * hence at most eight sites in the signed octant containing x_eff.
 *
 * This is an isolated EFT representation.  It does not alter Voxel state or
 * select a production movement rule.
 */

#include "ftd/lattice.h"
#include "ftd/voxel.h"

#include <array>
#include <cstddef>

namespace ftd::eft {

struct SubcellSiteWeight {
  Coord site{};
  double weight = 0.0;
};

struct SubcellPolarityShape {
  Coord anchor{};
  Vec3 remainder{};
  Vec3 effective_position{};
  int polarity = 0;
  std::array<SubcellSiteWeight, 8> weights{};
  std::size_t weight_count = 0;
  double partition_residual = 0.0;
  Vec3 first_moment_residual{};
  bool valid = false;
};

/// Construct the compact signed-octant shape.  Only polarities +/-1 and
/// finite remainder components in the closed interval [-1,+1] are accepted.
SubcellPolarityShape make_subcell_polarity_shape(
    Coord anchor, const Vec3& remainder, int polarity);

/// Largest absolute component of the stored first-moment residual.
double max_first_moment_residual(const SubcellPolarityShape& shape);

}  // namespace ftd::eft
