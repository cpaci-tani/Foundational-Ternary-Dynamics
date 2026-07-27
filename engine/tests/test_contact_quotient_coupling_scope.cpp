/** FTD-0528: native snapshot source versus matched history quotient. */

#include "ftd/eft/contact_quotient_coupling_scope.h"

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
  ftd::eft::ContactQuotientCouplingScopeResult result{};
};

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

bool same_direction(ftd::Coord lhs, ftd::Coord rhs) {
  return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

const Row* find_row(const std::vector<Row>& rows,
                    ftd::Coord direction,
                    int speed_index,
                    int polarity,
                    int translation_index) {
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
        if (dx == 0 && dy == 0 && dz == 0) continue;
        const ftd::Coord direction{dx, dy, dz};
        const int shell = dx*dx+dy*dy+dz*dz;
        for (int speed_index = 0; speed_index < 2; ++speed_index) {
          for (int polarity : {-1, +1}) {
            for (int translation_index = 0;
                 translation_index < 3; ++translation_index) {
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
                  ftd::eft::analyze_contact_quotient_coupling_scope(
                      L, contact, direction, polarity,
                      speeds[static_cast<std::size_t>(speed_index)], gate)});
            }
          }
        }
      }
    }
  }

  bool formula_ok = true;
  bool gradient_ok = true;
  bool curl_explains = true;
  bool axial_factors = true;
  bool diagonal_breaks = true;
  bool matched_factors = true;
  bool common_output_factors = true;
  int axial_arms = 0;
  int diagonal_arms = 0;
  double worst_formula = 0.0;
  double worst_gradient = 0.0;
  double worst_curl_explanation = 0.0;
  double minimum_axial_response = INFINITY;
  double worst_axial_response = 0.0;
  double minimum_diagonal_response = INFINITY;
  double maximum_diagonal_response = 0.0;
  double worst_matched_current = 0.0;
  double worst_matched_field = 0.0;
  double worst_continuity = 0.0;
  double worst_common_output = 0.0;
  double worst_translation = 0.0;
  double worst_polarity_mirror = 0.0;
  double worst_cubic_orbit = 0.0;

  for (const auto& row : rows) {
    const auto& result = row.result;
    formula_ok = formula_ok && result.valid
        && result.coupling_formula_residual <= gate;
    gradient_ok = gradient_ok
        && result.gradient_source_difference <= gate;
    curl_explains = curl_explains
        && result.curl_explanation_residual <= gate
        && std::abs(result.native_response_difference
                    - result.curl_source_difference) <= gate;
    matched_factors = matched_factors && result.matched_history_factors
        && result.matched_density_residual <= gate
        && result.matched_current_residual <= gate
        && result.matched_field_response_residual <= gate
        && result.continuity_residual <= gate;
    common_output_factors = common_output_factors
        && result.common_output_native_residual <= gate;
    worst_formula = std::max(worst_formula,
        result.coupling_formula_residual);
    worst_gradient = std::max(worst_gradient,
        result.gradient_source_difference);
    worst_curl_explanation = std::max(worst_curl_explanation,
        result.curl_explanation_residual);
    worst_matched_current = std::max(worst_matched_current,
        result.matched_current_residual);
    worst_matched_field = std::max(worst_matched_field,
        result.matched_field_response_residual);
    worst_continuity = std::max(worst_continuity,
        result.continuity_residual);
    worst_common_output = std::max(worst_common_output,
        result.common_output_native_residual);
    if (row.shell == 1) {
      ++axial_arms;
      axial_factors = axial_factors && result.native_snapshot_factors;
      minimum_axial_response = std::min(
          minimum_axial_response, result.native_response_difference);
      worst_axial_response = std::max(
          worst_axial_response, result.native_response_difference);
    } else {
      ++diagonal_arms;
      diagonal_breaks = diagonal_breaks
          && !result.native_snapshot_factors
          && result.native_response_difference > 1e-6;
      minimum_diagonal_response = std::min(
          minimum_diagonal_response, result.native_response_difference);
      maximum_diagonal_response = std::max(
          maximum_diagonal_response, result.native_response_difference);
    }

    const Row* translated = find_row(rows, row.direction,
        row.speed_index, row.polarity, 1);
    const Row* mirrored = find_row(rows, row.direction,
        row.speed_index, -row.polarity, row.translation_index);
    const Row* rotated = find_row(rows, canonical_direction(row.shell),
        row.speed_index, row.polarity, row.translation_index);
    if (!translated || !mirrored || !rotated) {
      worst_translation = INFINITY;
      worst_polarity_mirror = INFINITY;
      worst_cubic_orbit = INFINITY;
    } else {
      worst_translation = std::max(worst_translation, std::abs(
          result.native_response_difference
          - translated->result.native_response_difference));
      worst_polarity_mirror = std::max(worst_polarity_mirror, std::abs(
          result.native_response_difference
          - mirrored->result.native_response_difference));
      worst_cubic_orbit = std::max(worst_cubic_orbit, std::abs(
          result.native_response_difference
          - rotated->result.native_response_difference));
    }
  }

  check("actual CPU coupling matches -G_C grad(s)+G_C curl(sv)",
        formula_ok && rows.size() == 312 && worst_formula <= gate);
  check("the primitive gradient source factors through this pair quotient",
        gradient_ok && worst_gradient <= gate);
  check("the complete native difference is exactly the velocity-curl term",
        curl_explains && worst_curl_explanation <= gate);
  check("the preregistered axial-factorization hypothesis is rejected",
        !axial_factors && axial_arms == 72
        && minimum_axial_response > 1e-6);
  check("every edge and corner representative is distinguished natively",
        diagonal_breaks && diagonal_arms == 240
        && minimum_diagonal_response > 1e-6);
  check("exact density current and matched field response factor",
        matched_factors && worst_matched_current <= gate
        && worst_matched_field <= gate && worst_continuity <= gate);
  check("native coupling factors after the common FTD-0527 output",
        common_output_factors && worst_common_output <= gate);
  check("translation polarity mirror and cubic-orbit magnitudes agree",
        worst_translation <= gate && worst_polarity_mirror <= gate
        && worst_cubic_orbit <= gate);
  check("invalid coupling-scope inputs fail closed",
        !ftd::eft::analyze_contact_quotient_coupling_scope(
            2, {}, {1, 0, 0}, +1, 0.25).valid
        && !ftd::eft::analyze_contact_quotient_coupling_scope(
            L, {8.5, 8.0, 8.0}, {1, 0, 0}, 0, 0.25).valid);

  std::cout.precision(17);
  std::cout << "arms=" << rows.size() << '\n'
            << "axial_arms=" << axial_arms << '\n'
            << "diagonal_arms=" << diagonal_arms << '\n'
            << "worst_formula_residual=" << worst_formula << '\n'
            << "worst_gradient_difference=" << worst_gradient << '\n'
            << "worst_curl_explanation_residual="
            << worst_curl_explanation << '\n'
            << "worst_axial_response_difference="
            << worst_axial_response << '\n'
            << "minimum_axial_response_difference="
            << minimum_axial_response << '\n'
            << "minimum_diagonal_response_difference="
            << minimum_diagonal_response << '\n'
            << "maximum_diagonal_response_difference="
            << maximum_diagonal_response << '\n'
            << "worst_matched_current_residual="
            << worst_matched_current << '\n'
            << "worst_matched_field_response_residual="
            << worst_matched_field << '\n'
            << "worst_continuity_residual=" << worst_continuity << '\n'
            << "worst_common_output_native_residual="
            << worst_common_output << '\n'
            << "worst_translation_magnitude_residual="
            << worst_translation << '\n'
            << "worst_polarity_mirror_magnitude_residual="
            << worst_polarity_mirror << '\n'
            << "worst_cubic_orbit_magnitude_residual="
            << worst_cubic_orbit << '\n'
            << "contact_quotient_coupling_scope failures="
            << failures << '\n'
            << "preregistered_verdict=CONTACT_QUOTIENT_COUPLING_SCOPE_UNRESOLVED\n"
            << "mechanistic_result=NATIVE_COUPLING_BREAKS_CONTACT_QUOTIENT_ALL_DIRECTIONS_MATCHED_HISTORY_FACTORS\n";
  return failures == 0 ? 0 : 1;
}
