/** FTD-0531: symmetry-reduced diagonal field/matter endpoint solve. */

#include "ftd/eft/symmetric_diagonal_coupled_endpoint.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {

constexpr int L = 17;
constexpr double gate = 1e-12;
int failures = 0;

struct Row {
  ftd::Coord direction{};
  int shell = 0;
  int speed_index = 0;
  int polarity = 0;
  int translation_index = 0;
  ftd::eft::SymmetricDiagonalCoupledEndpointResult result{};
};

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

bool same_direction(ftd::Coord lhs, ftd::Coord rhs) {
  return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

const Row* find_row(const std::vector<Row>& rows,
                    ftd::Coord direction, int speed_index,
                    int polarity, int translation_index) {
  for (const auto& row : rows) {
    if (same_direction(row.direction, direction)
        && row.speed_index == speed_index
        && row.polarity == polarity
        && row.translation_index == translation_index) return &row;
  }
  return nullptr;
}

ftd::Coord canonical_direction(int shell) {
  return shell == 2 ? ftd::Coord{1, 1, 0} : ftd::Coord{1, 1, 1};
}

}  // namespace

int main() {
  const std::array<ftd::Coord, 3> translations{{
      {-2, 1, 0}, {0, 0, 0}, {2, -1, 1}}};
  const std::array<double, 2> speeds{{0.125, 0.25}};
  std::vector<Row> rows;
  rows.reserve(240);
  for (int dx = -1; dx <= 1; ++dx) {
    for (int dy = -1; dy <= 1; ++dy) {
      for (int dz = -1; dz <= 1; ++dz) {
        const int shell = dx*dx+dy*dy+dz*dz;
        if (shell != 2 && shell != 3) continue;
        const ftd::Coord direction{dx, dy, dz};
        for (int speed_index = 0; speed_index < 2; ++speed_index) {
          for (int polarity : {-1, +1}) {
            for (int translation_index = 0; translation_index < 3;
                 ++translation_index) {
              const auto translation = translations[
                  static_cast<std::size_t>(translation_index)];
              const ftd::Coord source{
                  8+translation.x, 8+translation.y, 8+translation.z};
              const ftd::Vec3 contact{
                  static_cast<double>(source.x)+0.5*dx,
                  static_cast<double>(source.y)+0.5*dy,
                  static_cast<double>(source.z)+0.5*dz};
              rows.push_back({direction, shell, speed_index, polarity,
                  translation_index,
                  ftd::eft::solve_symmetric_diagonal_coupled_endpoint(
                      L, contact, direction, polarity,
                      speeds[static_cast<std::size_t>(speed_index)], gate)});
            }
          }
        }
      }
    }
  }

  bool root_ok = true;
  bool algebra_ok = true;
  bool endpoint_ok = true;
  bool inverse_ok = true;
  int maximum_iterations = 0;
  double minimum_monotonic_increment = INFINITY;
  double worst_root = 0.0;
  double worst_continuity = 0.0;
  double worst_gauss = 0.0;
  double worst_embedding = 0.0;
  double worst_field_work = 0.0;
  double worst_matter_work = 0.0;
  double worst_total_energy = 0.0;
  double worst_displacement = 0.0;
  double worst_causal = 0.0;
  double worst_inverse = 0.0;
  double minimum_momentum_change = INFINITY;
  double maximum_momentum_change = 0.0;
  double minimum_endpoint_change = INFINITY;
  double maximum_endpoint_change = 0.0;
  double minimum_transverse_norm = INFINITY;
  double maximum_transverse_norm = 0.0;
  double worst_translation = 0.0;
  double worst_polarity = 0.0;
  double worst_cubic = 0.0;

  for (const auto& row : rows) {
    const auto& r = row.result;
    root_ok = root_ok && r.valid && r.root_bracketed && r.converged
        && r.monotonic_on_locked_grid
        && r.minimum_monotonic_increment > 0.0
        && r.root_residual <= gate;
    algebra_ok = algebra_ok
        && r.continuity_residual <= gate
        && r.gauss_before_residual <= gate
        && r.gauss_after_residual <= gate
        && r.staggered_embedding_residual <= gate
        && r.field_work_residual <= gate
        && r.matter_work_residual <= gate
        && r.total_energy_residual <= gate
        && r.displacement_residual <= gate
        && r.causal_excess <= gate;
    endpoint_ok = endpoint_ok && r.momentum_change > 1e-8
        && r.endpoint_change > 0.0 && r.speed < ftd::C_SPEED;
    inverse_ok = inverse_ok && r.inverse_residual <= 1e-10;
    maximum_iterations = std::max(maximum_iterations, r.iterations);
    minimum_monotonic_increment = std::min(
        minimum_monotonic_increment, r.minimum_monotonic_increment);
    worst_root = std::max(worst_root, r.root_residual);
    worst_continuity = std::max(worst_continuity, r.continuity_residual);
    worst_gauss = std::max({worst_gauss,
        r.gauss_before_residual, r.gauss_after_residual});
    worst_embedding = std::max(
        worst_embedding, r.staggered_embedding_residual);
    worst_field_work = std::max(worst_field_work, r.field_work_residual);
    worst_matter_work = std::max(worst_matter_work, r.matter_work_residual);
    worst_total_energy = std::max(
        worst_total_energy, r.total_energy_residual);
    worst_displacement = std::max(
        worst_displacement, r.displacement_residual);
    worst_causal = std::max(worst_causal, r.causal_excess);
    worst_inverse = std::max(worst_inverse, r.inverse_residual);
    minimum_momentum_change = std::min(
        minimum_momentum_change, r.momentum_change);
    maximum_momentum_change = std::max(
        maximum_momentum_change, r.momentum_change);
    minimum_endpoint_change = std::min(
        minimum_endpoint_change, r.endpoint_change);
    maximum_endpoint_change = std::max(
        maximum_endpoint_change, r.endpoint_change);
    minimum_transverse_norm = std::min(
        minimum_transverse_norm, r.reference_transverse_norm_squared);
    maximum_transverse_norm = std::max(
        maximum_transverse_norm, r.reference_transverse_norm_squared);

    const Row* translated = find_row(rows, row.direction,
        row.speed_index, row.polarity, 1);
    const Row* mirrored = find_row(rows, row.direction,
        row.speed_index, -row.polarity, row.translation_index);
    const Row* rotated = find_row(rows, canonical_direction(row.shell),
        row.speed_index, row.polarity, row.translation_index);
    if (!translated || !mirrored || !rotated) {
      worst_translation = INFINITY;
      worst_polarity = INFINITY;
      worst_cubic = INFINITY;
    } else {
      worst_translation = std::max(worst_translation, std::abs(
          r.momentum_after-translated->result.momentum_after));
      worst_polarity = std::max(worst_polarity, std::abs(
          r.momentum_after-mirrored->result.momentum_after));
      worst_cubic = std::max(worst_cubic, std::abs(
          r.momentum_after-rotated->result.momentum_after));
    }
  }

  check("all diagonal scalar energy roots are bracketed converged and monotone",
        root_ok && rows.size() == 240);
  check("current Gauss staggered work energy displacement and causality close",
        algebra_ok);
  check("field work changes existing momentum and the exact endpoint",
        endpoint_ok && minimum_momentum_change > 1e-8
        && minimum_endpoint_change > 0.0);
  check("explicit reversed histories restore field density and energy",
        inverse_ok && worst_inverse <= 1e-10);
  check("coupled endpoint is translation polarity and cubic covariant",
        worst_translation <= gate && worst_polarity <= gate
        && worst_cubic <= gate);
  check("invalid diagonal endpoint inputs fail closed",
        !ftd::eft::solve_symmetric_diagonal_coupled_endpoint(
            2, {}, {1, 1, 0}, +1, 0.25).valid
        && !ftd::eft::solve_symmetric_diagonal_coupled_endpoint(
            L, {8.5, 8.0, 8.0}, {1, 0, 0}, +1, 0.25).valid);

  std::cout.precision(17);
  std::cout << "arms=" << rows.size() << '\n'
            << "maximum_iterations=" << maximum_iterations << '\n'
            << "minimum_monotonic_increment="
            << minimum_monotonic_increment << '\n'
            << "worst_root_residual=" << worst_root << '\n'
            << "worst_continuity_residual=" << worst_continuity << '\n'
            << "worst_absolute_gauss_residual=" << worst_gauss << '\n'
            << "worst_staggered_embedding_residual=" << worst_embedding << '\n'
            << "worst_field_work_residual=" << worst_field_work << '\n'
            << "worst_matter_work_residual=" << worst_matter_work << '\n'
            << "worst_total_energy_residual=" << worst_total_energy << '\n'
            << "worst_displacement_residual=" << worst_displacement << '\n'
            << "worst_causal_excess=" << worst_causal << '\n'
            << "worst_inverse_residual=" << worst_inverse << '\n'
            << "minimum_momentum_change=" << minimum_momentum_change << '\n'
            << "maximum_momentum_change=" << maximum_momentum_change << '\n'
            << "minimum_endpoint_change=" << minimum_endpoint_change << '\n'
            << "maximum_endpoint_change=" << maximum_endpoint_change << '\n'
            << "minimum_reference_transverse_norm_squared="
            << minimum_transverse_norm << '\n'
            << "maximum_reference_transverse_norm_squared="
            << maximum_transverse_norm << '\n'
            << "worst_translation_magnitude_residual=" << worst_translation << '\n'
            << "worst_polarity_mirror_magnitude_residual=" << worst_polarity << '\n'
            << "worst_cubic_orbit_magnitude_residual=" << worst_cubic << '\n'
            << "symmetric_diagonal_coupled_endpoint failures=" << failures << '\n'
            << "verdict=SYMMETRIC_DIAGONAL_ENERGY_COUPLED_ENDPOINT_CONSTRUCTIVE\n";
  return failures == 0 ? 0 : 1;
}

