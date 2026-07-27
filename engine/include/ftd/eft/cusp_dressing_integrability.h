#pragma once
/**
 * @file cusp_dressing_integrability.h
 * @brief Cellwise cusp energy and global gluing obstruction (FTD-0494).
 *
 * Observer only.  This interface does not modify the production field,
 * movement rule, remainder, or energy audit.
 */

#include "ftd/eft/centered_trace_work.h"

namespace ftd::eft {

struct CuspDressingIntegrabilityResult {
  bool valid = false;
  int L = 0;
  int charge = 0;
  Coord site{};
  Vec3 remainder{};
  Vec3 jump{};
  double coupling = 0.0;

  double local_energy = 0.0;
  double exact_cusp_work = 0.0;
  double local_primitive_residual = 0.0;
  double reverse_residual = 0.0;
  Vec3 cusp_position_gradient{};
  double branch_trace_gradient_residual = 0.0;

  Vec3 threshold_site_offset_increment{};
  double threshold_representation_mismatch = 0.0;

  double path_xy = 0.0;
  double path_yx = 0.0;
  double plaquette_holonomy_xy = 0.0;
  double predicted_holonomy_xy = 0.0;
  double holonomy_residual = 0.0;
  double local_divergence = 0.0;

  double field_euler_derivative_l2 = 0.0;
  double predicted_field_euler_derivative_l2 = 0.0;
  double field_euler_derivative_residual = 0.0;
};

/// Unique cellwise state function fixed by exact cusp work from the knot.
double local_cusp_dressing_energy(
    const Vec3& jump,
    const Vec3& remainder,
    int charge,
    double coupling = 1.0);

/// Evaluate the local primitive, hop-representation gluing equation, one xy
/// plaquette holonomy, and the extra field Euler derivative.
CuspDressingIntegrabilityResult evaluate_cusp_dressing_integrability(
    const MatchedFaceFlux& electric,
    Coord site,
    const Vec3& remainder,
    int charge,
    double coupling = 1.0);

}  // namespace ftd::eft
