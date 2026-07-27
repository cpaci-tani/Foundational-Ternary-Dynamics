/**
 * @file campaign_moore_hop_route_ambiguity.cpp
 * @brief FTD-0445 Moore-hop to oriented-face routing audit.
 */

#include "ftd/eft/dual_cell_continuity.h"
#include "ftd/eft/matched_gauss_transport.h"
#include "ftd/eft/moore_link_routes.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {

constexpr int kL = 9;
constexpr double kAlgebraGate = 1e-14;
constexpr double kDistinctGate = 1.0;
constexpr double kEnergySplitGate = 0.5;
constexpr double kBackgroundScale = 0.25;

const std::array<std::array<int, 3>, 6> kOrders{{
    {{0, 1, 2}}, {{0, 2, 1}}, {{1, 0, 2}},
    {{1, 2, 0}}, {{2, 0, 1}}, {{2, 1, 0}}}};

int factorial(int value) {
  return value <= 1 ? 1 : value * factorial(value - 1);
}

ftd::eft::DualCellContinuity swap_xy(
    const ftd::eft::DualCellContinuity& input) {
  ftd::eft::DualCellContinuity output(input.L);
  for (int x = 0; x < input.L; ++x) {
    for (int y = 0; y < input.L; ++y) {
      for (int z = 0; z < input.L; ++z) {
        const int source = input.index(x, y, z);
        const int target = output.index(y, x, z);
        output.rho_before[static_cast<std::size_t>(target)] =
            input.rho_before[static_cast<std::size_t>(source)];
        output.rho_after[static_cast<std::size_t>(target)] =
            input.rho_after[static_cast<std::size_t>(source)];
        output.reaction[static_cast<std::size_t>(target)] =
            input.reaction[static_cast<std::size_t>(source)];
        output.current_x[static_cast<std::size_t>(target)] =
            input.current_y[static_cast<std::size_t>(source)];
        output.current_y[static_cast<std::size_t>(target)] =
            input.current_x[static_cast<std::size_t>(source)];
        output.current_z[static_cast<std::size_t>(target)] =
            input.current_z[static_cast<std::size_t>(source)];
      }
    }
  }
  return output;
}

ftd::eft::DualCellContinuity average_routes(
    const std::vector<ftd::eft::DualCellContinuity>& routes) {
  if (routes.empty()) return ftd::eft::DualCellContinuity{};
  ftd::eft::DualCellContinuity output(routes.front().L);
  output.rho_before = routes.front().rho_before;
  output.rho_after = routes.front().rho_after;
  const double weight = 1.0 / static_cast<double>(routes.size());
  for (const auto& route : routes) {
    for (std::size_t i = 0; i < output.current_x.size(); ++i) {
      output.current_x[i] += weight * route.current_x[i];
      output.current_y[i] += weight * route.current_y[i];
      output.current_z[i] += weight * route.current_z[i];
    }
  }
  return output;
}

bool has_fractional_current(const ftd::eft::DualCellContinuity& history) {
  for (std::size_t i = 0; i < history.current_x.size(); ++i) {
    for (const double value : {history.current_x[i], history.current_y[i],
                               history.current_z[i]}) {
      if (std::abs(value - std::round(value)) > kAlgebraGate) return true;
    }
  }
  return false;
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  std::cout << "FTD-0445 Moore-hop route ambiguity v1\n";
  std::cout << "protocol,L," << kL
            << ",algebra_gate," << kAlgebraGate
            << ",distinct_gate," << kDistinctGate
            << ",energy_split_gate," << kEnergySplitGate
            << ",background_scale," << kBackgroundScale << '\n';

  const int c = kL / 2;
  const ftd::eft::DualCellContinuity indexer(kL);
  const int source = indexer.index(c, c, c);
  double worst_continuity_residual = 0.0;
  double minimum_distinct_route_separation =
      std::numeric_limits<double>::infinity();
  int face_cases = 0;
  int edge_cases = 0;
  int corner_cases = 0;
  int route_count_mismatches = 0;
  bool finite = true;

  for (int dx = -1; dx <= 1; ++dx) {
    for (int dy = -1; dy <= 1; ++dy) {
      for (int dz = -1; dz <= 1; ++dz) {
        if (dx == 0 && dy == 0 && dz == 0) continue;
        const std::array<int, 3> delta{{dx, dy, dz}};
        const int nonzero = (dx != 0) + (dy != 0) + (dz != 0);
        if (nonzero == 1) ++face_cases;
        if (nonzero == 2) ++edge_cases;
        if (nonzero == 3) ++corner_cases;

        std::vector<ftd::eft::DualCellContinuity> unique_routes;
        for (const auto& order : kOrders) {
          auto route = ftd::eft::route_single_moore_hop(
              kL, source, delta, 1, order);
          worst_continuity_residual = std::max(
              worst_continuity_residual,
              ftd::eft::max_continuity_residual(route));
          bool duplicate = false;
          for (const auto& existing : unique_routes) {
            if (ftd::eft::current_l2_distance(route, existing)
                <= kAlgebraGate) {
              duplicate = true;
              break;
            }
          }
          if (!duplicate) unique_routes.push_back(std::move(route));
        }
        if (static_cast<int>(unique_routes.size()) != factorial(nonzero))
          ++route_count_mismatches;
        for (std::size_t i = 0; i < unique_routes.size(); ++i) {
          for (std::size_t j = i + 1; j < unique_routes.size(); ++j) {
            minimum_distinct_route_separation = std::min(
                minimum_distinct_route_separation,
                ftd::eft::current_l2_distance(
                    unique_routes[i], unique_routes[j]));
          }
        }
      }
    }
  }

  const std::array<int, 3> corner_delta{{1, 1, 1}};
  std::vector<ftd::eft::DualCellContinuity> corner_routes;
  for (const auto& order : kOrders)
    corner_routes.push_back(ftd::eft::route_single_moore_hop(
        kL, source, corner_delta, 1, order));
  const auto& canonical = corner_routes.front();
  const auto swapped_canonical = swap_xy(canonical);
  const double canonical_swap_distance =
      ftd::eft::current_l2_distance(canonical, swapped_canonical);
  const auto symmetric_average = average_routes(corner_routes);
  const double average_continuity_residual =
      ftd::eft::max_continuity_residual(symmetric_average);
  const double average_swap_residual = ftd::eft::current_l2_distance(
      symmetric_average, swap_xy(symmetric_average));
  const bool average_fractional = has_fractional_current(symmetric_average);

  std::vector<int> rho_before(static_cast<std::size_t>(kL * kL * kL), 0);
  std::vector<int> rho_after(rho_before.size(), 0);
  rho_before[static_cast<std::size_t>(source)] = 1;
  rho_after[static_cast<std::size_t>(indexer.index(c + 1, c + 1, c + 1))] = 1;
  ftd::eft::DualCellContinuity extracted(kL);
  const auto extraction = ftd::eft::extract_moore_history_from_snapshots(
      kL, rho_before, rho_after, extracted);
  const double extractor_canonical_residual =
      ftd::eft::current_l2_distance(extracted, canonical);

  ftd::eft::MatchedFaceFlux background(kL);
  for (std::size_t i = 0; i < background.x.size(); ++i) {
    background.x[i] = kBackgroundScale
        * (corner_routes[0].current_x[i] - corner_routes[2].current_x[i]);
    background.y[i] = kBackgroundScale
        * (corner_routes[0].current_y[i] - corner_routes[2].current_y[i]);
    background.z[i] = kBackgroundScale
        * (corner_routes[0].current_z[i] - corner_routes[2].current_z[i]);
  }
  const double background_divergence = ftd::eft::max_divergence(background);
  auto field_xyz = background;
  auto field_yxz = background;
  const auto update_xyz =
      ftd::eft::apply_conservative_current(field_xyz, corner_routes[0]);
  const auto update_yxz =
      ftd::eft::apply_conservative_current(field_yxz, corner_routes[2]);
  const double energy_xyz = ftd::eft::quadratic_energy(field_xyz);
  const double energy_yxz = ftd::eft::quadratic_energy(field_yxz);
  const double route_energy_split = std::abs(energy_xyz - energy_yxz);

  finite = finite && std::isfinite(worst_continuity_residual)
      && std::isfinite(minimum_distinct_route_separation)
      && std::isfinite(canonical_swap_distance)
      && std::isfinite(average_continuity_residual)
      && std::isfinite(average_swap_residual)
      && std::isfinite(extractor_canonical_residual)
      && std::isfinite(background_divergence)
      && std::isfinite(route_energy_split);
  const bool combinatorics_pass = face_cases == 6 && edge_cases == 12
      && corner_cases == 8 && route_count_mismatches == 0
      && minimum_distinct_route_separation >= kDistinctGate;
  const bool continuity_pass = worst_continuity_residual <= kAlgebraGate;
  const bool engine_route_selected = extraction.valid
      && extraction.transported_events == 1
      && extractor_canonical_residual <= kAlgebraGate;
  const bool stabilizer_breaks = canonical_swap_distance >= kDistinctGate;
  const bool symmetric_average_pass =
      average_continuity_residual <= kAlgebraGate
      && average_swap_residual <= kAlgebraGate && average_fractional;
  const bool energy_ambiguous = update_xyz.valid && update_yxz.valid
      && background_divergence <= kAlgebraGate
      && route_energy_split >= kEnergySplitGate;

  const char* verdict = "PROTOCOL_INVALID";
  if (finite && combinatorics_pass && continuity_pass
      && engine_route_selected && stabilizer_breaks
      && symmetric_average_pass && energy_ambiguous)
    verdict = "FACE_ROUTING_UNDERDETERMINED";
  else if (finite && combinatorics_pass && continuity_pass
           && engine_route_selected && !energy_ambiguous)
    verdict = "ROUTES_ENERGETICALLY_EQUIVALENT_IN_REGISTERED_BACKGROUND";

  std::cout << "route_counts,face_cases," << face_cases
            << ",edge_cases," << edge_cases
            << ",corner_cases," << corner_cases
            << ",count_mismatches," << route_count_mismatches
            << ",minimum_distinct_separation,"
            << minimum_distinct_route_separation << '\n';
  std::cout << "continuity,worst_residual," << worst_continuity_residual
            << ",pass," << (continuity_pass ? "true" : "false") << '\n';
  std::cout << "engine_extractor,valid,"
            << (extraction.valid ? "true" : "false")
            << ",transported_events," << extraction.transported_events
            << ",canonical_xyz_residual," << extractor_canonical_residual
            << ",selected," << (engine_route_selected ? "true" : "false")
            << '\n';
  std::cout << "corner_stabilizer,canonical_swap_distance,"
            << canonical_swap_distance
            << ",symmetric_average_swap_residual," << average_swap_residual
            << ",average_continuity_residual," << average_continuity_residual
            << ",average_fractional,"
            << (average_fractional ? "true" : "false") << '\n';
  std::cout << "route_energy,background_divergence,"
            << background_divergence
            << ",xyz_energy," << energy_xyz
            << ",yxz_energy," << energy_yxz
            << ",split," << route_energy_split
            << ",ambiguous," << (energy_ambiguous ? "true" : "false")
            << '\n';
  std::cout << "gates,finite," << (finite ? "true" : "false")
            << ",combinatorics_pass,"
            << (combinatorics_pass ? "true" : "false")
            << ",continuity_pass," << (continuity_pass ? "true" : "false")
            << ",engine_route_selected,"
            << (engine_route_selected ? "true" : "false")
            << ",stabilizer_breaks,"
            << (stabilizer_breaks ? "true" : "false")
            << ",symmetric_average_pass,"
            << (symmetric_average_pass ? "true" : "false")
            << ",energy_ambiguous,"
            << (energy_ambiguous ? "true" : "false") << '\n';
  std::cout << "verdict," << verdict << '\n';
  return std::string(verdict) == "PROTOCOL_INVALID" ? 1 : 0;
}
