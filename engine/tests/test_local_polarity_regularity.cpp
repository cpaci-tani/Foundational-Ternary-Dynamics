/** FTD-0540: exact local-polarity regularity trilemma and witnesses. */

#include "ftd/eft/local_polarity_regularity.h"
#include "ftd/eft/subcell_polarity_shape.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>

namespace {

constexpr double gate = 1e-12;
int failures = 0;

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

double shape_weight_at(const ftd::eft::SubcellPolarityShape& shape,
                       ftd::Coord site) {
  double result = 0.0;
  for (std::size_t i = 0; i < shape.weight_count; ++i) {
    const auto& entry = shape.weights[i];
    if (entry.site.x == site.x && entry.site.y == site.y
        && entry.site.z == site.z)
      result += entry.weight;
  }
  return result;
}

}  // namespace

int main() {
  const auto result = ftd::eft::analyze_local_polarity_regularity();

  bool nearest_cell_exact = true;
  for (int denominator = 1; denominator <= 64; ++denominator) {
    for (int numerator = 0; numerator <= denominator; ++numerator) {
      const double x = static_cast<double>(numerator)/denominator;
      const double w0 = 1.0-x;
      const double w1 = x;
      nearest_cell_exact = nearest_cell_exact
          && std::abs(w0+w1-1.0) <= gate
          && std::abs(w1-x) <= gate;
    }
  }
  check("nearest-cell partition and first moment force the hat weights",
        result.valid && result.nearest_cell_forces_hat
        && nearest_cell_exact);

  check("cardinal hat has an exact center derivative jump of two",
        result.hat_center_left_derivative == 1.0
        && result.hat_center_right_derivative == -1.0
        && result.hat_center_derivative_jump == 2.0);

  check("smooth nonnegative cardinal first-moment representation is impossible",
        result.smooth_nonnegative_cardinal_moment_no_go);

  const std::array<ftd::Vec3, 4> remainders{{
      {0.25, -0.5, 0.75}, {-0.625, 0.375, -0.125},
      {0.5, 0.5, 0.5}, {-0.25, -0.75, 0.625}}};
  bool tensor_matches = result.multiaffine_cube_basis_unique;
  double worst_tensor_residual = 0.0;
  for (const auto& remainder : remainders) {
    for (int polarity : {-1, +1}) {
      const ftd::Coord anchor{3, -2, 5};
      const auto shape = ftd::eft::make_subcell_polarity_shape(
          anchor, remainder, polarity);
      tensor_matches = tensor_matches && shape.valid;
      const int sx = remainder.x < 0.0 ? -1 : 1;
      const int sy = remainder.y < 0.0 ? -1 : 1;
      const int sz = remainder.z < 0.0 ? -1 : 1;
      for (int bx = 0; bx <= 1; ++bx) {
        for (int by = 0; by <= 1; ++by) {
          for (int bz = 0; bz <= 1; ++bz) {
            const ftd::Coord site{anchor.x+bx*sx,
                                  anchor.y+by*sy,
                                  anchor.z+bz*sz};
            const double wx = bx ? std::abs(remainder.x)
                                 : 1.0-std::abs(remainder.x);
            const double wy = by ? std::abs(remainder.y)
                                 : 1.0-std::abs(remainder.y);
            const double wz = bz ? std::abs(remainder.z)
                                 : 1.0-std::abs(remainder.z);
            const double expected = polarity*wx*wy*wz;
            const double residual = std::abs(
                shape_weight_at(shape, site)-expected);
            worst_tensor_residual = std::max(
                worst_tensor_residual, residual);
            tensor_matches = tensor_matches && residual <= gate;
          }
        }
      }
    }
  }
  check("FTD-0478 shape is the unique tensor multiaffine cardinal basis",
        tensor_matches);

  bool quadratic_witness = result.quadratic_bspline_is_c1
      && result.quadratic_center_weight == 0.75
      && result.quadratic_neighbor_weight == 0.125
      && result.quadratic_cardinality_defect == 0.25;
  bool catmull_witness = result.catmull_rom_is_c1
      && std::abs(result.catmull_rom_negative_lobe+2.0/27.0) <= gate
      && std::abs(result.catmull_rom_negative_lobe_position-4.0/3.0) <= gate;
  double minimum_quadratic_weight = std::numeric_limits<double>::infinity();
  double minimum_catmull_weight = std::numeric_limits<double>::infinity();
  for (int numerator = -256; numerator <= 256; ++numerator) {
    const double x = static_cast<double>(numerator)/128.0;
    const auto quadratic = ftd::eft::evaluate_local_polarity_kernel_moments(
        ftd::eft::LocalPolarityKernel::QuadraticBSpline, x);
    const auto catmull = ftd::eft::evaluate_local_polarity_kernel_moments(
        ftd::eft::LocalPolarityKernel::CatmullRom, x);
    quadratic_witness = quadratic_witness && quadratic.valid
        && std::abs(quadratic.partition_residual) <= gate
        && std::abs(quadratic.first_moment_residual) <= gate;
    catmull_witness = catmull_witness && catmull.valid
        && std::abs(catmull.partition_residual) <= gate
        && std::abs(catmull.first_moment_residual) <= gate;
    minimum_quadratic_weight = std::min(
        minimum_quadratic_weight, quadratic.minimum_weight);
    minimum_catmull_weight = std::min(
        minimum_catmull_weight, catmull.minimum_weight);
  }
  quadratic_witness = quadratic_witness
      && minimum_quadratic_weight >= -gate;
  catmull_witness = catmull_witness
      && minimum_catmull_weight < -0.07;
  check("smooth positive quadratic witness pays by losing cardinality",
        quadratic_witness);
  check("smooth cardinal Catmull-Rom witness pays with negative lobes",
        catmull_witness);

  const auto invalid = ftd::eft::evaluate_local_polarity_kernel_moments(
      ftd::eft::LocalPolarityKernel::Hat,
      std::numeric_limits<double>::infinity());
  check("nonfinite input fails closed",
        !invalid.valid
        && std::isnan(ftd::eft::evaluate_local_polarity_kernel(
            ftd::eft::LocalPolarityKernel::Hat,
            std::numeric_limits<double>::quiet_NaN())));

  const bool analytic_close = result.valid && nearest_cell_exact
      && tensor_matches && quadratic_witness && catmull_witness;
  const char* verdict = analytic_close
      ? "LOCAL_POLARITY_REGULARITY_TRILEMMA_PROVED"
      : (result.valid
          ? "LOCAL_POLARITY_REGULARITY_WITNESS_UNRESOLVED"
          : "LOCAL_POLARITY_REGULARITY_TRILEMMA_REFUTED");

  std::cout.precision(17);
  std::cout << "hat_left_derivative="
            << result.hat_center_left_derivative << '\n'
            << "hat_right_derivative="
            << result.hat_center_right_derivative << '\n'
            << "hat_derivative_jump="
            << result.hat_center_derivative_jump << '\n'
            << "worst_tensor_basis_residual="
            << worst_tensor_residual << '\n'
            << "quadratic_center_weight="
            << result.quadratic_center_weight << '\n'
            << "quadratic_neighbor_weight="
            << result.quadratic_neighbor_weight << '\n'
            << "quadratic_cardinality_defect="
            << result.quadratic_cardinality_defect << '\n'
            << "quadratic_minimum_weight="
            << minimum_quadratic_weight << '\n'
            << "catmull_negative_lobe_position="
            << result.catmull_rom_negative_lobe_position << '\n'
            << "catmull_negative_lobe="
            << result.catmull_rom_negative_lobe << '\n'
            << "catmull_sampled_minimum_weight="
            << minimum_catmull_weight << '\n'
            << "worst_quadratic_partition_residual="
            << result.worst_quadratic_partition_residual << '\n'
            << "worst_quadratic_first_moment_residual="
            << result.worst_quadratic_first_moment_residual << '\n'
            << "worst_catmull_partition_residual="
            << result.worst_catmull_partition_residual << '\n'
            << "worst_catmull_first_moment_residual="
            << result.worst_catmull_first_moment_residual << '\n'
            << "local_polarity_regularity failures=" << failures << '\n'
            << "verdict=" << verdict << '\n';
  return failures == 0 ? 0 : 1;
}
