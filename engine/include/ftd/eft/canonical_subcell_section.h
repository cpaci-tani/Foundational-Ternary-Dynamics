#pragma once
/**
 * @file canonical_subcell_section.h
 * @brief Observer-only centered canonical section and exact half-cell
 *        symmetry obstruction (FTD-0500).
 */

#include "ftd/eft/subcell_representation_quotient.h"

namespace ftd::eft {

/// Componentwise nearest-site section with the frozen half-open convention
/// remainder in [-1/2,1/2).
SubcellChart centered_canonical_subcell_chart(
    const Vec3& effective_position);

/// Translate physical position and reselect the centered canonical chart.
SubcellChart translate_centered_canonical_chart(
    const SubcellChart& start,
    const Vec3& displacement);

struct HalfCellSectionObstruction {
  bool valid = false;
  bool integer_solution_exists = false;
  int positive_anchor = 0;
  int negative_anchor = 0;
  int translation_predicted_negative_anchor = 0;
  int inversion_predicted_negative_anchor = 0;
  int diophantine_residual = 0;
  double raw_anchor_mismatch = 0.0;
  double raw_remainder_mismatch = 0.0;
  double physical_inversion_residual = 0.0;
};

/// Analyze x=1/2, where translation covariance and inversion covariance would
/// demand the impossible integer equation 2*a(1/2)=1.
HalfCellSectionObstruction analyze_half_cell_section_obstruction();

}  // namespace ftd::eft
