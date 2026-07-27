#pragma once
/**
 * @file local_polarity_regularity.h
 * @brief Exact regularity audit for local subcell polarity kernels (FTD-0540).
 *
 * This observer prices the representation assumptions behind the FTD-0478
 * trilinear shape.  It does not alter the production state or select a new
 * matter-field action.
 */

#include <cstddef>

namespace ftd::eft {

enum class LocalPolarityKernel {
  Hat,
  QuadraticBSpline,
  CatmullRom
};

struct KernelMomentDiagnostics {
  bool valid = false;
  double partition_residual = 0.0;
  double first_moment_residual = 0.0;
  double minimum_weight = 0.0;
  std::size_t nonzero_weight_count = 0;
};

struct LocalPolarityRegularityResult {
  bool valid = false;
  bool nearest_cell_forces_hat = false;
  bool multiaffine_cube_basis_unique = false;
  bool smooth_nonnegative_cardinal_moment_no_go = false;
  bool quadratic_bspline_is_c1 = false;
  bool catmull_rom_is_c1 = false;
  double hat_center_left_derivative = 0.0;
  double hat_center_right_derivative = 0.0;
  double hat_center_derivative_jump = 0.0;
  double quadratic_center_weight = 0.0;
  double quadratic_neighbor_weight = 0.0;
  double quadratic_cardinality_defect = 0.0;
  double catmull_rom_negative_lobe = 0.0;
  double catmull_rom_negative_lobe_position = 0.0;
  double worst_quadratic_partition_residual = 0.0;
  double worst_quadratic_first_moment_residual = 0.0;
  double worst_catmull_partition_residual = 0.0;
  double worst_catmull_first_moment_residual = 0.0;
};

/// Evaluate one of the three locked one-dimensional kernels.  Nonfinite input
/// returns NaN.
double evaluate_local_polarity_kernel(LocalPolarityKernel kernel, double u);

/// Sum a locally finite translated kernel family around x and report its
/// partition, first moment, support count, and minimum weight.
KernelMomentDiagnostics evaluate_local_polarity_kernel_moments(
    LocalPolarityKernel kernel, double x);

/// Assemble the locked analytic consequences and polynomial witnesses.
LocalPolarityRegularityResult analyze_local_polarity_regularity();

}  // namespace ftd::eft
