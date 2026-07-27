/** FTD-0530: Gauss-fixed axial contact longitudinal work. */

#include "ftd/eft/axial_contact_longitudinal_work.h"

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
  int speed_index = 0;
  int polarity = 0;
  int translation_index = 0;
  ftd::eft::AxialContactLongitudinalWorkResult result{};
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

}  // namespace

int main() {
  const std::array<ftd::Coord, 6> directions{{
      {+1, 0, 0}, {-1, 0, 0},
      {0, +1, 0}, {0, -1, 0},
      {0, 0, +1}, {0, 0, -1}}};
  const std::array<ftd::Coord, 3> translations{{
      {-2, 1, 0}, {0, 0, 0}, {2, -1, 1}}};
  const std::array<double, 2> speeds{{0.125, 0.25}};
  std::vector<Row> rows;
  rows.reserve(72);
  for (const auto direction : directions) {
    for (int speed_index = 0; speed_index < 2; ++speed_index) {
      for (int polarity : {-1, +1}) {
        for (int translation_index = 0; translation_index < 3;
             ++translation_index) {
          const auto translation = translations[
              static_cast<std::size_t>(translation_index)];
          const ftd::Coord source{
              8+translation.x, 8+translation.y, 8+translation.z};
          const ftd::Vec3 contact{
              static_cast<double>(source.x)+0.5*direction.x,
              static_cast<double>(source.y)+0.5*direction.y,
              static_cast<double>(source.z)+0.5*direction.z};
          rows.push_back({direction, speed_index, polarity,
              translation_index,
              ftd::eft::analyze_axial_contact_longitudinal_work(
                  L, contact, direction, polarity,
                  speeds[static_cast<std::size_t>(speed_index)], gate)});
        }
      }
    }
  }

  bool algebra_ok = true;
  bool same_gauss_work_ok = true;
  bool energy_ok = true;
  bool obstruction_ok = true;
  double worst_history = 0.0;
  double worst_continuity = 0.0;
  double worst_gauss = 0.0;
  double worst_curl = 0.0;
  double worst_harmonic = 0.0;
  double worst_current_norm = 0.0;
  double worst_density_change = 0.0;
  double worst_transverse_work = 0.0;
  double worst_harmonic_work = 0.0;
  double worst_embedding = 0.0;
  double worst_energy_identity = 0.0;
  double minimum_elastic_defect = INFINITY;
  double maximum_elastic_defect = 0.0;
  double minimum_required_impulse = INFINITY;
  double maximum_required_impulse = 0.0;
  double minimum_required_speed = INFINITY;
  double maximum_required_speed = 0.0;
  double worst_correction = 0.0;
  double worst_translation = 0.0;
  double worst_polarity = 0.0;
  double worst_axis_orbit = 0.0;

  for (const auto& row : rows) {
    const auto& r = row.result;
    algebra_ok = algebra_ok && r.valid
        && r.history_residual <= gate
        && r.continuity_residual <= gate
        && r.gauss_residual <= gate
        && r.curl_adjoint_norm_squared <= gate
        && r.harmonic_current_residual <= gate
        && r.current_norm_squared <= gate
        && r.endpoint_density_change_residual <= gate;
    same_gauss_work_ok = same_gauss_work_ok
        && r.transverse_work_difference <= gate
        && r.harmonic_work_difference <= gate;
    energy_ok = energy_ok
        && r.staggered_embedding_residual <= gate
        && r.field_energy_identity_residual <= gate
        && r.frozen_path_correction_residual <= gate;
    obstruction_ok = obstruction_ok && r.fixed_path_obstruction;
    worst_history = std::max(worst_history, r.history_residual);
    worst_continuity = std::max(worst_continuity, r.continuity_residual);
    worst_gauss = std::max(worst_gauss, r.gauss_residual);
    worst_curl = std::max(worst_curl, r.curl_adjoint_norm_squared);
    worst_harmonic = std::max(
        worst_harmonic, r.harmonic_current_residual);
    worst_current_norm = std::max(
        worst_current_norm, r.current_norm_squared);
    worst_density_change = std::max(
        worst_density_change, r.endpoint_density_change_residual);
    worst_transverse_work = std::max(
        worst_transverse_work, r.transverse_work_difference);
    worst_harmonic_work = std::max(
        worst_harmonic_work, r.harmonic_work_difference);
    worst_embedding = std::max(
        worst_embedding, r.staggered_embedding_residual);
    worst_energy_identity = std::max(
        worst_energy_identity, r.field_energy_identity_residual);
    minimum_elastic_defect = std::min(
        minimum_elastic_defect, r.unchanged_total_energy_residual);
    maximum_elastic_defect = std::max(
        maximum_elastic_defect, r.unchanged_total_energy_residual);
    minimum_required_impulse = std::min(
        minimum_required_impulse, r.required_impulse_magnitude);
    maximum_required_impulse = std::max(
        maximum_required_impulse, r.required_impulse_magnitude);
    minimum_required_speed = std::min(
        minimum_required_speed, r.required_speed);
    maximum_required_speed = std::max(
        maximum_required_speed, r.required_speed);
    worst_correction = std::max(
        worst_correction, r.frozen_path_correction_residual);

    const Row* translated = find_row(rows, row.direction,
        row.speed_index, row.polarity, 1);
    const Row* mirrored = find_row(rows, row.direction,
        row.speed_index, -row.polarity, row.translation_index);
    const Row* rotated = find_row(rows, {1, 0, 0},
        row.speed_index, row.polarity, row.translation_index);
    if (!translated || !mirrored || !rotated) {
      worst_translation = INFINITY;
      worst_polarity = INFINITY;
      worst_axis_orbit = INFINITY;
    } else {
      worst_translation = std::max(worst_translation, std::abs(
          r.unchanged_total_energy_residual
          - translated->result.unchanged_total_energy_residual));
      worst_polarity = std::max(worst_polarity, std::abs(
          r.unchanged_total_energy_residual
          - mirrored->result.unchanged_total_energy_residual));
      worst_axis_orbit = std::max(worst_axis_orbit, std::abs(
          r.unchanged_total_energy_residual
          - rotated->result.unchanged_total_energy_residual));
    }
  }

  check("axial histories close quotient continuity absolute Gauss and Hodge gates",
        algebra_ok && rows.size() == 72);
  check("same-Gauss transverse and harmonic fields cannot tune axial work",
        same_gauss_work_ok && worst_transverse_work <= gate
        && worst_harmonic_work <= gate);
  check("all staggered midpoint-energy and fixed-path correction identities close",
        energy_ok && worst_embedding <= gate
        && worst_energy_identity <= gate && worst_correction <= gate);
  check("the preregistered nonzero axial-work hypothesis is rejected",
        !obstruction_ok && maximum_elastic_defect <= gate);
  check("axial pair current and endpoint density change cancel pointwise",
        worst_current_norm <= gate && worst_density_change <= gate
        && maximum_required_impulse <= gate);
  check("axial defect is translation polarity and signed-cubic covariant",
        worst_translation <= gate && worst_polarity <= gate
        && worst_axis_orbit <= gate);
  check("invalid axial-work inputs fail closed",
        !ftd::eft::analyze_axial_contact_longitudinal_work(
            2, {}, {1, 0, 0}, +1, 0.25).valid
        && !ftd::eft::analyze_axial_contact_longitudinal_work(
            L, {8.5, 8.5, 8.0}, {1, 1, 0}, +1, 0.25).valid);

  std::cout.precision(17);
  std::cout << "arms=" << rows.size() << '\n'
            << "worst_history_residual=" << worst_history << '\n'
            << "worst_continuity_residual=" << worst_continuity << '\n'
            << "worst_absolute_gauss_residual=" << worst_gauss << '\n'
            << "worst_curl_adjoint_norm_squared=" << worst_curl << '\n'
            << "worst_harmonic_current_residual=" << worst_harmonic << '\n'
            << "worst_current_norm_squared=" << worst_current_norm << '\n'
            << "worst_endpoint_density_change_residual="
            << worst_density_change << '\n'
            << "worst_transverse_work_difference=" << worst_transverse_work << '\n'
            << "worst_harmonic_work_difference=" << worst_harmonic_work << '\n'
            << "worst_staggered_embedding_residual=" << worst_embedding << '\n'
            << "worst_field_energy_identity_residual="
            << worst_energy_identity << '\n'
            << "minimum_unchanged_elastic_energy_defect="
            << minimum_elastic_defect << '\n'
            << "maximum_unchanged_elastic_energy_defect="
            << maximum_elastic_defect << '\n'
            << "minimum_required_impulse_magnitude="
            << minimum_required_impulse << '\n'
            << "maximum_required_impulse_magnitude="
            << maximum_required_impulse << '\n'
            << "minimum_required_speed=" << minimum_required_speed << '\n'
            << "maximum_required_speed=" << maximum_required_speed << '\n'
            << "worst_frozen_path_correction_residual=" << worst_correction << '\n'
            << "worst_translation_magnitude_residual=" << worst_translation << '\n'
            << "worst_polarity_mirror_magnitude_residual=" << worst_polarity << '\n'
            << "worst_axis_orbit_magnitude_residual=" << worst_axis_orbit << '\n'
            << "axial_contact_longitudinal_work failures=" << failures << '\n'
            << "verdict=AXIAL_ELASTIC_CONTACT_IS_RECIPROCAL_ON_FIXED_PATH\n";
  return failures == 0 ? 0 : 1;
}
