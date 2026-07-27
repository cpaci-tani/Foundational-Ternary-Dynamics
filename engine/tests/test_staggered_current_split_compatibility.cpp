/** FTD-0535: exact endpoint current split versus staggered phase order. */

#include "ftd/eft/staggered_current_split_compatibility.h"

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
  ftd::eft::StaggeredCurrentSplitCompatibilityResult result{};
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
  if (shell == 1) return {1, 0, 0};
  if (shell == 2) return {1, 1, 0};
  return {1, 1, 1};
}

}  // namespace

int main() {
  const std::array<ftd::Coord, 3> translations{{
      {-2, 1, 0}, {0, 0, 0}, {2, -1, 1}}};
  const std::array<double, 2> speeds{{0.125, 0.25}};
  std::vector<Row> rows;
  rows.reserve(312);
  for (int dx = -1; dx <= 1; ++dx) {
    for (int dy = -1; dy <= 1; ++dy) {
      for (int dz = -1; dz <= 1; ++dz) {
        const int shell = dx*dx+dy*dy+dz*dz;
        if (shell == 0) continue;
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
                  ftd::eft::analyze_staggered_current_split_compatibility(
                      L, contact, direction, polarity,
                      speeds[static_cast<std::size_t>(speed_index)], gate)});
            }
          }
        }
      }
    }
  }

  bool all_valid = true;
  bool split_ok = true;
  bool diagonal_obstructed = true;
  bool axial_null = true;
  int axial_arms = 0;
  int diagonal_arms = 0;
  double minimum_start_curl_norm_squared = INFINITY;
  double maximum_start_curl_norm_squared = 0.0;
  double minimum_mismatch_norm = INFINITY;
  double maximum_mismatch_norm = 0.0;
  double minimum_start_current_l1 = INFINITY;
  double maximum_start_current_l1 = 0.0;
  double worst_recombination = 0.0;
  double worst_split_continuity = 0.0;
  double worst_component_identity = 0.0;
  double worst_norm_identity = 0.0;
  double worst_inherited = 0.0;
  double worst_translation = 0.0;
  double worst_polarity = 0.0;
  double worst_cubic = 0.0;

  for (const auto& row : rows) {
    const auto& r = row.result;
    all_valid = all_valid && r.valid;
    split_ok = split_ok
        && r.split_recombination_residual <= gate
        && r.split_continuity_residual <= gate
        && r.component_identity_residual <= gate
        && r.norm_identity_residual <= gate;
    worst_recombination = std::max(
        worst_recombination, r.split_recombination_residual);
    worst_split_continuity = std::max(
        worst_split_continuity, r.split_continuity_residual);
    worst_component_identity = std::max(
        worst_component_identity, r.component_identity_residual);
    worst_norm_identity = std::max(
        worst_norm_identity, r.norm_identity_residual);
    worst_inherited = std::max(
        worst_inherited, r.inherited_endpoint_residual);
    if (row.shell == 1) {
      ++axial_arms;
      axial_null = axial_null && !r.used_coupled_endpoint
          && r.frozen_staggered_transverse_compatible
          && r.total_current_l1 <= gate
          && r.start_current_l1 <= gate
          && r.end_current_l1 <= gate
          && r.start_current_curl_norm_squared <= gate*gate
          && r.faraday_mismatch_norm_squared <= gate*gate;
    } else {
      ++diagonal_arms;
      const double mismatch_norm = std::sqrt(
          r.faraday_mismatch_norm_squared);
      diagonal_obstructed = diagonal_obstructed
          && r.used_coupled_endpoint && r.coupled.valid
          && !r.frozen_staggered_transverse_compatible
          && r.start_current_curl_norm_squared > gate*gate
          && mismatch_norm > 1e-8;
      minimum_start_curl_norm_squared = std::min(
          minimum_start_curl_norm_squared,
          r.start_current_curl_norm_squared);
      maximum_start_curl_norm_squared = std::max(
          maximum_start_curl_norm_squared,
          r.start_current_curl_norm_squared);
      minimum_mismatch_norm = std::min(minimum_mismatch_norm, mismatch_norm);
      maximum_mismatch_norm = std::max(maximum_mismatch_norm, mismatch_norm);
      minimum_start_current_l1 = std::min(
          minimum_start_current_l1, r.start_current_l1);
      maximum_start_current_l1 = std::max(
          maximum_start_current_l1, r.start_current_l1);
    }

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
          r.start_current_curl_norm_squared
          -translated->result.start_current_curl_norm_squared));
      worst_polarity = std::max(worst_polarity, std::abs(
          r.faraday_mismatch_norm_squared
          -mirrored->result.faraday_mismatch_norm_squared));
      worst_cubic = std::max(worst_cubic, std::abs(
          r.start_current_curl_norm_squared
          -rotated->result.start_current_curl_norm_squared));
    }
  }

  check("all exact endpoint-split observers are valid",
        all_valid && rows.size() == 312);
  check("start and end deposits recombine to exact total current",
        split_ok && worst_recombination <= gate
        && worst_split_continuity <= gate);
  check("every diagonal start split obstructs frozen Faraday ordering",
        diagonal_obstructed && diagonal_arms == 240);
  check("mismatch equals minus lambda times start-current curl",
        worst_component_identity <= gate && worst_norm_identity <= gate);
  check("all axial endpoint current splits remain exactly null",
        axial_null && axial_arms == 72);
  check("split obstruction is translation polarity and cubic covariant",
        worst_translation <= gate && worst_polarity <= gate
        && worst_cubic <= gate && worst_inherited <= 1e-10);
  check("invalid endpoint-split inputs fail closed",
        !ftd::eft::analyze_staggered_current_split_compatibility(
            2, {}, {1, 0, 0}, +1, 0.25).valid
        && !ftd::eft::analyze_staggered_current_split_compatibility(
            L, {}, {2, 0, 0}, +1, 0.25).valid
        && !ftd::eft::analyze_staggered_current_split_compatibility(
            L, {}, {1, 0, 0}, 0, 0.25).valid);

  std::cout.precision(17);
  std::cout << "arms=" << rows.size() << '\n'
            << "axial_arms=" << axial_arms << '\n'
            << "diagonal_arms=" << diagonal_arms << '\n'
            << "minimum_diagonal_start_current_l1="
            << minimum_start_current_l1 << '\n'
            << "maximum_diagonal_start_current_l1="
            << maximum_start_current_l1 << '\n'
            << "minimum_diagonal_start_curl_norm_squared="
            << minimum_start_curl_norm_squared << '\n'
            << "maximum_diagonal_start_curl_norm_squared="
            << maximum_start_curl_norm_squared << '\n'
            << "minimum_diagonal_faraday_mismatch_norm="
            << minimum_mismatch_norm << '\n'
            << "maximum_diagonal_faraday_mismatch_norm="
            << maximum_mismatch_norm << '\n'
            << "worst_split_recombination_residual="
            << worst_recombination << '\n'
            << "worst_split_continuity_residual="
            << worst_split_continuity << '\n'
            << "worst_component_identity_residual="
            << worst_component_identity << '\n'
            << "worst_norm_identity_residual=" << worst_norm_identity << '\n'
            << "worst_inherited_endpoint_residual=" << worst_inherited << '\n'
            << "worst_translation_residual=" << worst_translation << '\n'
            << "worst_polarity_mirror_residual=" << worst_polarity << '\n'
            << "worst_signed_cubic_residual=" << worst_cubic << '\n'
            << "staggered_current_split_compatibility failures="
            << failures << '\n'
            << "verdict=EXACT_WORLDLINE_SPLIT_REQUIRES_IMPLICIT_ATOMIC_FIELD_TRANSACTION\n";
  return failures == 0 ? 0 : 1;
}

