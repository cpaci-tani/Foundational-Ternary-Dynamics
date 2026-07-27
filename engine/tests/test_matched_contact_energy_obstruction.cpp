/** FTD-0529: reciprocal matched-field energy obstruction at contact. */

#include "ftd/eft/matched_contact_energy_obstruction.h"

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
  ftd::eft::MatchedContactEnergyObstructionResult result{};
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
                  ftd::eft::analyze_matched_contact_energy_obstruction(
                      L, contact, direction, polarity,
                      speeds[static_cast<std::size_t>(speed_index)], gate)});
            }
          }
        }
      }
    }
  }

  bool histories_ok = true;
  bool gauss_ok = true;
  bool transverse_ok = true;
  bool embedding_ok = true;
  bool energy_identities_ok = true;
  bool split_ok = true;
  bool lower_bound_ok = true;
  int obstructed_arms = 0;
  double worst_history = 0.0;
  double worst_continuity = 0.0;
  double worst_gauss = 0.0;
  double worst_divergence = 0.0;
  double worst_adjoint = 0.0;
  double worst_embedding = 0.0;
  double worst_field_identity = 0.0;
  double worst_split_formula = 0.0;
  double worst_matter_change = 0.0;
  double minimum_transverse_norm = INFINITY;
  double maximum_transverse_norm = 0.0;
  double minimum_predicted_split = INFINITY;
  double maximum_predicted_split = 0.0;
  double minimum_incompatibility_margin = INFINITY;
  double worst_translation = 0.0;
  double worst_polarity_mirror = 0.0;
  double worst_cubic_orbit = 0.0;

  for (const auto& row : rows) {
    const auto& r = row.result;
    histories_ok = histories_ok && r.valid
        && r.history_residual <= gate && r.continuity_residual <= gate;
    gauss_ok = gauss_ok && r.gauss_before_residual <= gate
        && r.gauss_after_residual <= gate;
    transverse_ok = transverse_ok
        && r.challenge_divergence_residual <= gate
        && r.adjoint_identity_residual <= gate;
    embedding_ok = embedding_ok && r.staggered_embedding_residual <= gate;
    energy_identities_ok = energy_identities_ok
        && r.baseline_field_identity_residual <= gate
        && r.challenge_field_identity_residual <= gate
        && std::abs(r.matter_energy_change) <= gate;
    split_ok = split_ok && r.energy_split_formula_residual <= gate;
    if (r.transverse_norm_squared > gate) {
      ++obstructed_arms;
      lower_bound_ok = lower_bound_ok && r.obstruction_present
          && r.elastic_incompatibility_margin >= -gate;
      minimum_transverse_norm = std::min(
          minimum_transverse_norm, r.transverse_norm_squared);
      minimum_predicted_split = std::min(
          minimum_predicted_split, r.predicted_energy_split);
    }
    maximum_transverse_norm = std::max(
        maximum_transverse_norm, r.transverse_norm_squared);
    maximum_predicted_split = std::max(
        maximum_predicted_split, r.predicted_energy_split);
    minimum_incompatibility_margin = std::min(
        minimum_incompatibility_margin, r.elastic_incompatibility_margin);
    worst_history = std::max(worst_history, r.history_residual);
    worst_continuity = std::max(worst_continuity, r.continuity_residual);
    worst_gauss = std::max({worst_gauss,
        r.gauss_before_residual, r.gauss_after_residual});
    worst_divergence = std::max(
        worst_divergence, r.challenge_divergence_residual);
    worst_adjoint = std::max(
        worst_adjoint, r.adjoint_identity_residual);
    worst_embedding = std::max(
        worst_embedding, r.staggered_embedding_residual);
    worst_field_identity = std::max({worst_field_identity,
        r.baseline_field_identity_residual,
        r.challenge_field_identity_residual});
    worst_split_formula = std::max(
        worst_split_formula, r.energy_split_formula_residual);
    worst_matter_change = std::max(
        worst_matter_change, std::abs(r.matter_energy_change));

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
          r.transverse_norm_squared
          - translated->result.transverse_norm_squared));
      worst_polarity_mirror = std::max(worst_polarity_mirror, std::abs(
          r.transverse_norm_squared
          - mirrored->result.transverse_norm_squared));
      worst_cubic_orbit = std::max(worst_cubic_orbit, std::abs(
          r.transverse_norm_squared
          - rotated->result.transverse_norm_squared));
    }
  }

  check("crossing and bounce exact histories remain quotient-identical",
        histories_ok && rows.size() == 312);
  check("neutralized compatible fields close absolute Gauss before and after",
        gauss_ok && worst_gauss <= gate);
  check("C C^T K is divergence-free and satisfies exact adjoint pairing",
        transverse_ok && worst_divergence <= gate && worst_adjoint <= gate);
  check("the arbitrary deposition fields embed in the full staggered step",
        embedding_ok && worst_embedding <= gate);
  check("both matched field updates satisfy exact midpoint energy work",
        energy_identities_ok && worst_field_identity <= gate
        && worst_matter_change <= gate);
  check("the compatible-field energy split matches beta a ||C^T K||^2",
        split_ok && worst_split_formula <= gate);
  check("field-blind elastic contact fails at least one compatible field",
        lower_bound_ok && obstructed_arms > 0
        && minimum_predicted_split > 1e-10);
  check("baseline-independent obstruction is translation polarity and cubic covariant",
        worst_translation <= gate && worst_polarity_mirror <= gate
        && worst_cubic_orbit <= gate);
  check("invalid energy-obstruction inputs fail closed",
        !ftd::eft::analyze_matched_contact_energy_obstruction(
            2, {}, {1, 0, 0}, +1, 0.25).valid
        && !ftd::eft::analyze_matched_contact_energy_obstruction(
            L, {8.5, 8.0, 8.0}, {1, 0, 0}, 0, 0.25).valid);

  std::cout.precision(17);
  std::cout << "arms=" << rows.size() << '\n'
            << "obstructed_arms=" << obstructed_arms << '\n'
            << "worst_history_residual=" << worst_history << '\n'
            << "worst_continuity_residual=" << worst_continuity << '\n'
            << "worst_absolute_gauss_residual=" << worst_gauss << '\n'
            << "worst_challenge_divergence_residual=" << worst_divergence << '\n'
            << "worst_adjoint_identity_residual=" << worst_adjoint << '\n'
            << "worst_staggered_embedding_residual=" << worst_embedding << '\n'
            << "worst_field_energy_identity_residual=" << worst_field_identity << '\n'
            << "worst_energy_split_formula_residual=" << worst_split_formula << '\n'
            << "worst_matter_energy_change=" << worst_matter_change << '\n'
            << "minimum_transverse_norm_squared=" << minimum_transverse_norm << '\n'
            << "maximum_transverse_norm_squared=" << maximum_transverse_norm << '\n'
            << "minimum_predicted_energy_split=" << minimum_predicted_split << '\n'
            << "maximum_predicted_energy_split=" << maximum_predicted_split << '\n'
            << "minimum_elastic_incompatibility_margin="
            << minimum_incompatibility_margin << '\n'
            << "worst_translation_magnitude_residual=" << worst_translation << '\n'
            << "worst_polarity_mirror_magnitude_residual="
            << worst_polarity_mirror << '\n'
            << "worst_cubic_orbit_magnitude_residual=" << worst_cubic_orbit << '\n'
            << "matched_contact_energy_obstruction failures=" << failures << '\n'
            << "verdict=ELASTIC_CONTACT_CANNOT_COUPLE_RECIPROCALLY_WITHOUT_FIELD_DEPENDENT_MATTER_OR_DRESSING\n";
  return failures == 0 ? 0 : 1;
}

