#include "ftd/eft/local_polarity_regularity.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace ftd::eft {
namespace {

double hat(double radius) {
  return radius < 1.0 ? 1.0-radius : 0.0;
}

double quadratic_bspline(double radius) {
  if (radius <= 0.5) return 0.75-radius*radius;
  if (radius < 1.5) {
    const double tail = 1.5-radius;
    return 0.5*tail*tail;
  }
  return 0.0;
}

double catmull_rom(double radius) {
  constexpr double a = -0.5;
  if (radius <= 1.0) {
    return (a+2.0)*radius*radius*radius
        -(a+3.0)*radius*radius+1.0;
  }
  if (radius < 2.0) {
    return a*radius*radius*radius-5.0*a*radius*radius
        +8.0*a*radius-4.0*a;
  }
  return 0.0;
}

double quadratic_radial_derivative(double radius, bool outer_side) {
  if (radius < 0.5 || (radius == 0.5 && !outer_side))
    return -2.0*radius;
  if (radius < 1.5 || (radius == 1.5 && !outer_side))
    return -(1.5-radius);
  return 0.0;
}

double catmull_radial_derivative(double radius, bool outer_side) {
  constexpr double a = -0.5;
  if (radius < 1.0 || (radius == 1.0 && !outer_side))
    return 3.0*(a+2.0)*radius*radius-2.0*(a+3.0)*radius;
  if (radius < 2.0 || (radius == 2.0 && !outer_side))
    return 3.0*a*radius*radius-10.0*a*radius+8.0*a;
  return 0.0;
}

}  // namespace

double evaluate_local_polarity_kernel(LocalPolarityKernel kernel, double u) {
  if (!std::isfinite(u)) return std::numeric_limits<double>::quiet_NaN();
  const double radius = std::abs(u);
  switch (kernel) {
    case LocalPolarityKernel::Hat:
      return hat(radius);
    case LocalPolarityKernel::QuadraticBSpline:
      return quadratic_bspline(radius);
    case LocalPolarityKernel::CatmullRom:
      return catmull_rom(radius);
  }
  return std::numeric_limits<double>::quiet_NaN();
}

KernelMomentDiagnostics evaluate_local_polarity_kernel_moments(
    LocalPolarityKernel kernel, double x) {
  KernelMomentDiagnostics result;
  if (!std::isfinite(x)) return result;

  const int center = static_cast<int>(std::floor(x));
  long double partition = 0.0L;
  long double first_moment = 0.0L;
  double minimum = std::numeric_limits<double>::infinity();
  for (int site = center-4; site <= center+4; ++site) {
    const double weight = evaluate_local_polarity_kernel(
        kernel, x-static_cast<double>(site));
    if (!std::isfinite(weight)) return result;
    if (weight != 0.0) {
      ++result.nonzero_weight_count;
      minimum = std::min(minimum, weight);
    }
    partition += static_cast<long double>(weight);
    first_moment += static_cast<long double>(site)
        *static_cast<long double>(weight);
  }
  result.partition_residual = static_cast<double>(partition-1.0L);
  result.first_moment_residual = static_cast<double>(
      first_moment-static_cast<long double>(x));
  result.minimum_weight = std::isfinite(minimum) ? minimum : 0.0;
  result.valid = true;
  return result;
}

LocalPolarityRegularityResult analyze_local_polarity_regularity() {
  LocalPolarityRegularityResult result;

  // On [0,1], [1 1; 0 1](w0,w1)^T=(1,x)^T has determinant 1.
  // Hence w1=x and w0=1-x.  Translation gives the cardinal hat.
  result.nearest_cell_forces_hat = true;

  // The eight multiaffine monomials form an 8-dimensional space and their
  // values on the eight cube vertices give an invertible Boolean evaluation
  // matrix.  The tensor Lagrange basis is therefore unique.
  result.multiaffine_cube_basis_unique = true;

  // A differentiable nonnegative off-center cardinal weight has a local
  // minimum at each integer, hence zero derivative.  A locally finite sum
  // then has d/dx sum_n n*w_n(0)=0, contradicting first-moment derivative 1.
  result.smooth_nonnegative_cardinal_moment_no_go = true;

  result.hat_center_left_derivative = 1.0;
  result.hat_center_right_derivative = -1.0;
  result.hat_center_derivative_jump = 2.0;

  result.quadratic_center_weight = quadratic_bspline(0.0);
  result.quadratic_neighbor_weight = quadratic_bspline(1.0);
  result.quadratic_cardinality_defect = std::max(
      std::abs(1.0-result.quadratic_center_weight),
      std::abs(result.quadratic_neighbor_weight));
  result.quadratic_bspline_is_c1 =
      quadratic_radial_derivative(0.5, false)
          == quadratic_radial_derivative(0.5, true)
      && quadratic_radial_derivative(1.5, false)
          == quadratic_radial_derivative(1.5, true);

  result.catmull_rom_negative_lobe_position = 4.0/3.0;
  result.catmull_rom_negative_lobe = catmull_rom(4.0/3.0);
  result.catmull_rom_is_c1 =
      catmull_radial_derivative(1.0, false)
          == catmull_radial_derivative(1.0, true)
      && catmull_radial_derivative(2.0, false)
          == catmull_radial_derivative(2.0, true);

  for (int numerator = -64; numerator <= 64; ++numerator) {
    const double x = static_cast<double>(numerator)/32.0;
    const auto quadratic = evaluate_local_polarity_kernel_moments(
        LocalPolarityKernel::QuadraticBSpline, x);
    const auto catmull = evaluate_local_polarity_kernel_moments(
        LocalPolarityKernel::CatmullRom, x);
    if (!quadratic.valid || !catmull.valid) return result;
    result.worst_quadratic_partition_residual = std::max(
        result.worst_quadratic_partition_residual,
        std::abs(quadratic.partition_residual));
    result.worst_quadratic_first_moment_residual = std::max(
        result.worst_quadratic_first_moment_residual,
        std::abs(quadratic.first_moment_residual));
    result.worst_catmull_partition_residual = std::max(
        result.worst_catmull_partition_residual,
        std::abs(catmull.partition_residual));
    result.worst_catmull_first_moment_residual = std::max(
        result.worst_catmull_first_moment_residual,
        std::abs(catmull.first_moment_residual));
  }

  result.valid = result.nearest_cell_forces_hat
      && result.multiaffine_cube_basis_unique
      && result.smooth_nonnegative_cardinal_moment_no_go
      && result.quadratic_bspline_is_c1
      && result.catmull_rom_is_c1;
  return result;
}

}  // namespace ftd::eft
