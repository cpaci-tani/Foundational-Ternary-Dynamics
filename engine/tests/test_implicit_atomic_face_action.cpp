/** FTD-0536: minimal implicit face action versus FTD-0531 scalar roots. */

#include "ftd/eft/implicit_atomic_face_action.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {

constexpr int L = 17;
constexpr double algebra_gate = 1e-12;
constexpr double derivative_gate = 1e-7;
int failures = 0;

struct Row {
  ftd::Coord direction{};
  int shell = 0;
  int speed_index = 0;
  int polarity = 0;
  int translation_index = 0;
  ftd::eft::ImplicitAtomicFaceActionResult result{};
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

double scalar_metric_difference(
    const ftd::eft::ImplicitAtomicFaceActionResult& lhs,
    const ftd::eft::ImplicitAtomicFaceActionResult& rhs) {
  return std::max({
      std::abs(lhs.kinetic_start_residual-rhs.kinetic_start_residual),
      std::abs(lhs.kinetic_end_residual-rhs.kinetic_end_residual),
      std::abs(lhs.longitudinal_start_residual
               -rhs.longitudinal_start_residual),
      std::abs(lhs.longitudinal_end_residual
               -rhs.longitudinal_end_residual),
      std::abs(lhs.transverse_start_residual
               -rhs.transverse_start_residual),
      std::abs(lhs.transverse_end_residual
               -rhs.transverse_end_residual),
      std::abs(std::abs(lhs.total_energy_defect)
               -std::abs(rhs.total_energy_defect))});
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
                  source.x+0.5*dx, source.y+0.5*dy,
                  source.z+0.5*dz};
              rows.push_back({direction, shell, speed_index, polarity,
                  translation_index,
                  ftd::eft::analyze_implicit_atomic_face_action(
                      L, contact, direction, polarity,
                      speeds[static_cast<std::size_t>(speed_index)],
                      0.000244140625, algebra_gate)});
            }
          }
        }
      }
    }
  }

  bool all_valid = true;
  bool algebra_ok = true;
  bool derivative_ok = true;
  bool classification_ok = true;
  int stationary_arms = 0;
  double worst_split = 0.0;
  double worst_continuity = 0.0;
  double worst_field_start = 0.0;
  double worst_field_end = 0.0;
  double worst_field_update = 0.0;
  double worst_gauss = 0.0;
  double worst_derivative_convergence = 0.0;
  double minimum_start_kinetic_residual = INFINITY;
  double maximum_start_kinetic_residual = 0.0;
  double minimum_end_kinetic_residual = INFINITY;
  double maximum_end_kinetic_residual = 0.0;
  double minimum_energy_defect = INFINITY;
  double maximum_energy_defect = 0.0;
  double minimum_transverse_residual = INFINITY;
  double maximum_transverse_residual = 0.0;
  double minimum_longitudinal_residual = INFINITY;
  double maximum_longitudinal_residual = 0.0;
  double worst_inherited = 0.0;
  double worst_translation = 0.0;
  double worst_polarity = 0.0;
  double worst_cubic = 0.0;

  for (const auto& row : rows) {
    const auto& r = row.result;
    all_valid = all_valid && r.valid;
    algebra_ok = algebra_ok
        && r.current_split_residual <= algebra_gate
        && r.continuity_residual <= algebra_gate
        && r.field_start_equation_residual <= algebra_gate
        && r.field_end_equation_residual <= algebra_gate
        && r.field_update_residual <= algebra_gate
        && r.gauss_evolution_residual <= algebra_gate;
    derivative_ok = derivative_ok
        && r.endpoint_derivative_convergence <= derivative_gate;
    if (r.scalar_root_stationary) ++stationary_arms;
    classification_ok = classification_ok && (
        r.scalar_root_stationary
        || r.kinetic_start_residual > derivative_gate
        || r.kinetic_end_residual > derivative_gate
        || std::abs(r.total_energy_defect) > algebra_gate);

    worst_split = std::max(worst_split, r.current_split_residual);
    worst_continuity = std::max(worst_continuity, r.continuity_residual);
    worst_field_start = std::max(
        worst_field_start, r.field_start_equation_residual);
    worst_field_end = std::max(
        worst_field_end, r.field_end_equation_residual);
    worst_field_update = std::max(
        worst_field_update, r.field_update_residual);
    worst_gauss = std::max(worst_gauss, r.gauss_evolution_residual);
    worst_derivative_convergence = std::max(
        worst_derivative_convergence, r.endpoint_derivative_convergence);
    minimum_start_kinetic_residual = std::min(
        minimum_start_kinetic_residual, r.kinetic_start_residual);
    maximum_start_kinetic_residual = std::max(
        maximum_start_kinetic_residual, r.kinetic_start_residual);
    minimum_end_kinetic_residual = std::min(
        minimum_end_kinetic_residual, r.kinetic_end_residual);
    maximum_end_kinetic_residual = std::max(
        maximum_end_kinetic_residual, r.kinetic_end_residual);
    const double energy_defect = std::abs(r.total_energy_defect);
    minimum_energy_defect = std::min(minimum_energy_defect, energy_defect);
    maximum_energy_defect = std::max(maximum_energy_defect, energy_defect);
    const double transverse = std::max(
        r.transverse_start_residual, r.transverse_end_residual);
    const double longitudinal = std::max(
        r.longitudinal_start_residual, r.longitudinal_end_residual);
    minimum_transverse_residual = std::min(
        minimum_transverse_residual, transverse);
    maximum_transverse_residual = std::max(
        maximum_transverse_residual, transverse);
    minimum_longitudinal_residual = std::min(
        minimum_longitudinal_residual, longitudinal);
    maximum_longitudinal_residual = std::max(
        maximum_longitudinal_residual, longitudinal);
    worst_inherited = std::max(
        worst_inherited, r.inherited_endpoint_residual);

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
      worst_translation = std::max(worst_translation,
          scalar_metric_difference(r, translated->result));
      worst_polarity = std::max(worst_polarity,
          scalar_metric_difference(r, mirrored->result));
      worst_cubic = std::max(worst_cubic,
          scalar_metric_difference(r, rotated->result));
    }
  }

  check("all implicit atomic action observers are coherent",
        all_valid && rows.size() == 240);
  check("endpoint current and both field Euler equations close",
        algebra_ok && worst_split <= algebra_gate
        && worst_continuity <= algebra_gate
        && worst_gauss <= algebra_gate);
  check("complete deposited endpoint derivatives converge",
        derivative_ok && worst_derivative_convergence <= derivative_gate);
  check("every scalar root receives a locked stationarity verdict",
        classification_ok);
  check("stationarity metrics are translation polarity and cubic covariant",
        worst_translation <= derivative_gate
        && worst_polarity <= derivative_gate
        && worst_cubic <= derivative_gate
        && worst_inherited <= 1e-10);
  check("invalid implicit-action inputs fail closed",
        !ftd::eft::analyze_implicit_atomic_face_action(
            2, {}, {1, 1, 0}, +1, 0.25).valid
        && !ftd::eft::analyze_implicit_atomic_face_action(
            L, {}, {1, 0, 0}, +1, 0.25).valid
        && !ftd::eft::analyze_implicit_atomic_face_action(
            L, {}, {1, 1, 0}, 0, 0.25).valid
        && !ftd::eft::analyze_implicit_atomic_face_action(
            L, {}, {1, 1, 0}, +1, 0.25, -1.0).valid);

  const char* verdict = stationary_arms == 240
      ? "IMPLICIT_ATOMIC_ACTION_CLOSES_DIAGONAL_ENDPOINT"
      : (all_valid && classification_ok
          ? "ATOMIC_FACE_ACTION_CONSTRUCTIVE_SCALAR_ROOT_NOT_STATIONARY"
          : "IMPLICIT_ATOMIC_FACE_ACTION_UNRESOLVED");
  std::cout.precision(17);
  std::cout << "arms=" << rows.size() << '\n'
            << "stationary_arms=" << stationary_arms << '\n'
            << "worst_current_split_residual=" << worst_split << '\n'
            << "worst_continuity_residual=" << worst_continuity << '\n'
            << "worst_field_start_equation_residual="
            << worst_field_start << '\n'
            << "worst_field_end_equation_residual="
            << worst_field_end << '\n'
            << "worst_field_update_residual=" << worst_field_update << '\n'
            << "worst_gauss_evolution_residual=" << worst_gauss << '\n'
            << "worst_endpoint_derivative_convergence="
            << worst_derivative_convergence << '\n'
            << "minimum_start_kinetic_residual="
            << minimum_start_kinetic_residual << '\n'
            << "maximum_start_kinetic_residual="
            << maximum_start_kinetic_residual << '\n'
            << "minimum_end_kinetic_residual="
            << minimum_end_kinetic_residual << '\n'
            << "maximum_end_kinetic_residual="
            << maximum_end_kinetic_residual << '\n'
            << "minimum_total_energy_defect=" << minimum_energy_defect << '\n'
            << "maximum_total_energy_defect=" << maximum_energy_defect << '\n'
            << "minimum_transverse_stationarity_residual="
            << minimum_transverse_residual << '\n'
            << "maximum_transverse_stationarity_residual="
            << maximum_transverse_residual << '\n'
            << "minimum_longitudinal_stationarity_residual="
            << minimum_longitudinal_residual << '\n'
            << "maximum_longitudinal_stationarity_residual="
            << maximum_longitudinal_residual << '\n'
            << "worst_inherited_endpoint_residual=" << worst_inherited << '\n'
            << "worst_translation_residual=" << worst_translation << '\n'
            << "worst_polarity_mirror_residual=" << worst_polarity << '\n'
            << "worst_signed_cubic_residual=" << worst_cubic << '\n'
            << "implicit_atomic_face_action failures=" << failures << '\n'
            << "verdict=" << verdict << '\n';
  return failures == 0 ? 0 : 1;
}

