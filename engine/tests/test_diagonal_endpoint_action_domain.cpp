/** FTD-0532: composition audit at simultaneous diagonal hop planes. */

#include "ftd/eft/diagonal_endpoint_action_domain.h"

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
  ftd::eft::DiagonalEndpointActionDomainResult result{};
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
                  ftd::eft::analyze_diagonal_endpoint_action_domain(
                      L, contact, direction, polarity,
                      speeds[static_cast<std::size_t>(speed_index)], gate)});
            }
          }
        }
      }
    }
  }

  bool coupled_ok = true;
  bool crossing_ok = true;
  bool domain_ok = true;
  bool identities_ok = true;
  double minimum_tau = INFINITY;
  double maximum_tau = 0.0;
  double worst_simultaneous = 0.0;
  double minimum_overshoot = INFINITY;
  double minimum_reference_overshoot = INFINITY;
  double minimum_previous_displacement = INFINITY;
  double maximum_previous_displacement = 0.0;
  double worst_previous_causal_excess = 0.0;
  double worst_identity = 0.0;
  double worst_translation = 0.0;
  double worst_polarity = 0.0;
  double worst_cubic = 0.0;
  double minimum_endpoint_shift = INFINITY;
  double maximum_endpoint_shift = 0.0;
  int edge_arms = 0;
  int corner_arms = 0;
  int rejected_carrier_calls = 0;
  int accepted_previous_controls = 0;

  for (const auto& row : rows) {
    const auto& r = row.result;
    coupled_ok = coupled_ok && r.coupled_endpoint_valid && r.coupled.valid;
    crossing_ok = crossing_ok && r.valid
        && r.minimum_crossed_planes == row.shell
        && r.maximum_crossed_planes == row.shell
        && r.minimum_crossing_parameter > 0.0
        && r.maximum_crossing_parameter < 1.0
        && r.simultaneous_crossing_residual <= gate
        && r.minimum_endpoint_overshoot > 1e-8
        && r.minimum_reference_endpoint_overshoot > 0.0
        && r.reference_crossings_preserved;
    domain_ok = domain_ok
        && r.previous_segments_are_nonzero_and_interior
        && r.minimum_previous_segment_displacement > 1e-12
        && r.maximum_previous_segment_displacement <= ftd::C_SPEED+gate
        && r.previous_segment_causal_excess <= gate
        && r.accepted_previous_segment_controls == 2
        && r.zero_connection_rejected && r.rejected_carriers == 2;
    identities_ok = identities_ok && r.coupled_identity_residual <= 1e-10;
    if (row.shell == 2) ++edge_arms;
    else ++corner_arms;
    rejected_carrier_calls += r.rejected_carriers;
    accepted_previous_controls += r.accepted_previous_segment_controls;
    minimum_tau = std::min(minimum_tau, r.minimum_crossing_parameter);
    maximum_tau = std::max(maximum_tau, r.maximum_crossing_parameter);
    worst_simultaneous = std::max(
        worst_simultaneous, r.simultaneous_crossing_residual);
    minimum_overshoot = std::min(
        minimum_overshoot, r.minimum_endpoint_overshoot);
    minimum_reference_overshoot = std::min(
        minimum_reference_overshoot,
        r.minimum_reference_endpoint_overshoot);
    minimum_previous_displacement = std::min(
        minimum_previous_displacement,
        r.minimum_previous_segment_displacement);
    maximum_previous_displacement = std::max(
        maximum_previous_displacement,
        r.maximum_previous_segment_displacement);
    worst_previous_causal_excess = std::max(
        worst_previous_causal_excess,
        r.previous_segment_causal_excess);
    worst_identity = std::max(worst_identity, r.coupled_identity_residual);
    minimum_endpoint_shift = std::min(
        minimum_endpoint_shift, r.endpoint_shift);
    maximum_endpoint_shift = std::max(
        maximum_endpoint_shift, r.endpoint_shift);

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
          r.minimum_crossing_parameter
          -translated->result.minimum_crossing_parameter));
      worst_polarity = std::max(worst_polarity, std::abs(
          r.minimum_endpoint_overshoot
          -mirrored->result.minimum_endpoint_overshoot));
      worst_cubic = std::max(worst_cubic, std::abs(
          r.maximum_crossing_parameter
          -rotated->result.maximum_crossing_parameter));
    }
  }

  check("all FTD-0531 diagonal coupled roots remain valid",
        coupled_ok && rows.size() == 240);
  check("edge paths cross two and corner paths cross three planes",
        crossing_ok && edge_arms == 144 && corner_arms == 96);
  check("all active planes are crossed simultaneously in the open segment",
        minimum_tau > 0.0 && maximum_tau < 1.0
        && worst_simultaneous <= gate);
  check("positive reference and coupled overshoot survives field endpoint shift",
        minimum_overshoot > 1e-8 && minimum_reference_overshoot > 0.0
        && minimum_endpoint_shift > 0.0);
  check("nonzero interior previous slabs isolate next-slab domain rejection",
        domain_ok && accepted_previous_controls == 480
        && rejected_carrier_calls == 480);
  check("FTD-0531 continuity Gauss energy causality and inverse stay closed",
        identities_ok && worst_identity <= 1e-10);
  check("crossing data is translation polarity and signed-cubic covariant",
        worst_translation <= gate && worst_polarity <= gate
        && worst_cubic <= gate);
  check("invalid axial endpoint inputs fail closed",
        !ftd::eft::analyze_diagonal_endpoint_action_domain(
            L, {8.5, 8.0, 8.0}, {1, 0, 0}, +1, 0.25).valid);

  std::cout.precision(17);
  std::cout << "arms=" << rows.size() << '\n'
            << "edge_arms=" << edge_arms << '\n'
            << "corner_arms=" << corner_arms << '\n'
            << "rejected_carrier_calls=" << rejected_carrier_calls << '\n'
            << "accepted_previous_segment_controls="
            << accepted_previous_controls << '\n'
            << "minimum_crossing_parameter=" << minimum_tau << '\n'
            << "maximum_crossing_parameter=" << maximum_tau << '\n'
            << "worst_simultaneous_crossing_residual="
            << worst_simultaneous << '\n'
            << "minimum_endpoint_overshoot=" << minimum_overshoot << '\n'
            << "minimum_reference_endpoint_overshoot="
            << minimum_reference_overshoot << '\n'
            << "minimum_previous_segment_displacement="
            << minimum_previous_displacement << '\n'
            << "maximum_previous_segment_displacement="
            << maximum_previous_displacement << '\n'
            << "worst_previous_segment_causal_excess="
            << worst_previous_causal_excess << '\n'
            << "minimum_endpoint_shift=" << minimum_endpoint_shift << '\n'
            << "maximum_endpoint_shift=" << maximum_endpoint_shift << '\n'
            << "worst_coupled_identity_residual=" << worst_identity << '\n'
            << "worst_translation_residual=" << worst_translation << '\n'
            << "worst_polarity_mirror_residual=" << worst_polarity << '\n'
            << "worst_signed_cubic_residual=" << worst_cubic << '\n'
            << "diagonal_endpoint_action_domain failures=" << failures << '\n'
            << "verdict=ENERGY_ENDPOINT_CONSTRUCTIVE_COMPACT_COMMON_ACTION_OUT_OF_DOMAIN_AT_HOP\n";
  return failures == 0 ? 0 : 1;
}
