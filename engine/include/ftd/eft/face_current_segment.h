#pragma once
/**
 * @file face_current_segment.h
 * @brief Exact straight-segment current for the sub-cell polarity shape.
 *
 * The endpoint charge uses SubcellPolarityShape.  Current is deposited on
 * positively oriented periodic faces and obeys the same backward-divergence
 * convention as the matched face complex:
 *
 *   rho_after - rho_before + div(current) = 0.
 *
 * A straight segment is split at every crossed integer coordinate plane.  On
 * each open piece the two transverse hat functions are linear, so their
 * product is integrated analytically.  No quadrature or Gauss projection is
 * used.
 */

#include "ftd/eft/subcell_polarity_shape.h"

#include <vector>

namespace ftd::eft {

struct FaceCurrentSegment {
  int L = 0;
  int charge = 0;
  Coord start_anchor{};
  Coord end_anchor{};
  Vec3 start_remainder{};
  Vec3 end_remainder{};

  /// Start position and the nearest periodic image of the end position.  The
  /// latter makes the deposited path explicit across a periodic boundary.
  Vec3 start_effective_position{};
  Vec3 end_effective_position{};

  SubcellPolarityShape start_shape{};
  SubcellPolarityShape end_shape{};
  std::vector<double> rho_before;
  std::vector<double> rho_after;
  std::vector<double> current_x;
  std::vector<double> current_y;
  std::vector<double> current_z;

  int rho_support = 0;
  int current_support = 0;
  double partition_residual = 0.0;
  double first_moment_residual = 0.0;
  double continuity_residual = 0.0;
  double locality_residual = 0.0;
  bool valid = false;

  int total_sites() const { return L * L * L; }
  int index(int x, int y, int z) const;
};

/// Deposit one signed straight segment.  Anchors are periodic lattice sites;
/// the end anchor is unwrapped to its nearest image relative to the start.
/// This is unambiguous for the sub-lattice one-tick paths for which this
/// observer is intended.
FaceCurrentSegment make_face_current_segment(
    int L,
    Coord start_anchor,
    const Vec3& start_remainder,
    Coord end_anchor,
    const Vec3& end_remainder,
    int charge);

double face_current_divergence_at(
    const FaceCurrentSegment& segment, int x, int y, int z);

double face_current_continuity_at(
    const FaceCurrentSegment& segment, int x, int y, int z);

double max_face_current_continuity_residual(
    const FaceCurrentSegment& segment);

}  // namespace ftd::eft
