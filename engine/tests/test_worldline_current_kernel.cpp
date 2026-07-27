/** FTD-0502: endpoint multiset versus divergence-free worldline current. */

#include "ftd/eft/multibody_shape_observability.h"
#include "ftd/eft/worldline_current_kernel.h"
#include "ftd/ontic.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {

constexpr int L = 17;
constexpr double gate = 1e-12;
constexpr double kappa = 0.73;
int failures = 0;
int transformed_history_triples = 0;
double worst_tree_routing_residual = 0.0;
double endpoint_density_difference = 0.0;
double static_current_l1 = 0.0;
double clockwise_current_l1 = 0.0;
double current_opposition_residual = 0.0;
double loop_divergence_residual = 0.0;
double maximum_segment_length = 0.0;
double static_field_energy = 0.0;
double clockwise_field_energy = 0.0;
double counterclockwise_field_energy = 0.0;
double field_opposition_residual = 0.0;
double field_gauss_residual = 0.0;
double worst_transformed_residual = 0.0;

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

double max_difference(const std::vector<double>& lhs,
                      const std::vector<double>& rhs) {
  if (lhs.size() != rhs.size()) return INFINITY;
  double result = 0.0;
  for (std::size_t i = 0; i < lhs.size(); ++i) {
    result = std::max(result, std::abs(lhs[i] - rhs[i]));
  }
  return result;
}

double field_l1(const ftd::eft::AggregateShapeCurrent& history) {
  double result = 0.0;
  for (double value : history.current_x) result += std::abs(value);
  for (double value : history.current_y) result += std::abs(value);
  for (double value : history.current_z) result += std::abs(value);
  return result;
}

double current_opposition(
    const ftd::eft::AggregateShapeCurrent& lhs,
    const ftd::eft::AggregateShapeCurrent& rhs) {
  if (!lhs.valid || !rhs.valid || lhs.current_x.size() != rhs.current_x.size()) {
    return INFINITY;
  }
  double result = 0.0;
  for (std::size_t i = 0; i < lhs.current_x.size(); ++i) {
    result = std::max({result,
        std::abs(lhs.current_x[i] + rhs.current_x[i]),
        std::abs(lhs.current_y[i] + rhs.current_y[i]),
        std::abs(lhs.current_z[i] + rhs.current_z[i])});
  }
  return result;
}

double max_current_divergence(
    const ftd::eft::AggregateShapeCurrent& history) {
  double result = 0.0;
  const auto at = [&history](const std::vector<double>& field,
                             int x, int y, int z) {
    return field[static_cast<std::size_t>(history.index(x, y, z))];
  };
  for (int x = 0; x < history.L; ++x) {
    for (int y = 0; y < history.L; ++y) {
      for (int z = 0; z < history.L; ++z) {
        const double divergence =
            at(history.current_x, x, y, z)
            - at(history.current_x, x - 1, y, z)
            + at(history.current_y, x, y, z)
            - at(history.current_y, x, y - 1, z)
            + at(history.current_z, x, y, z)
            - at(history.current_z, x, y, z - 1);
        result = std::max(result, std::abs(divergence));
      }
    }
  }
  return result;
}

double sourced_field_energy(
    const ftd::eft::AggregateShapeCurrent& history) {
  double sum = 0.0;
  for (double value : history.current_x) sum += kappa * kappa * value * value;
  for (double value : history.current_y) sum += kappa * kappa * value * value;
  for (double value : history.current_z) sum += kappa * kappa * value * value;
  return 0.5 * sum;
}

double sourced_field_opposition(
    const ftd::eft::AggregateShapeCurrent& lhs,
    const ftd::eft::AggregateShapeCurrent& rhs) {
  // E=-kappa J, so this is the same componentwise opposition with scale.
  return kappa * current_opposition(lhs, rhs);
}

std::array<ftd::Vec3, 4> square_points() {
  return {{{8.25, 8.25, 8.0}, {8.75, 8.25, 8.0},
           {8.75, 8.75, 8.0}, {8.25, 8.75, 8.0}}};
}

std::vector<ftd::eft::ShapeWorldline> make_history(
    const std::array<ftd::Vec3, 4>& points,
    const std::array<int, 4>& endpoint_index) {
  std::vector<ftd::eft::ShapeWorldline> result;
  result.reserve(4);
  for (int i = 0; i < 4; ++i) {
    result.push_back({points[static_cast<std::size_t>(i)],
                      points[static_cast<std::size_t>(endpoint_index[
                          static_cast<std::size_t>(i)])], +1});
  }
  return result;
}

double maximum_length(
    const std::vector<ftd::eft::ShapeWorldline>& worldlines) {
  double result = 0.0;
  for (const auto& line : worldlines) {
    result = std::max(
        result, (line.end_position - line.start_position).mag());
  }
  return result;
}

ftd::Vec3 permute_signed(const ftd::Vec3& value,
                         const std::array<int, 3>& permutation,
                         const std::array<int, 3>& sign) {
  const std::array<double, 3> source{value.x, value.y, value.z};
  return {sign[0] * source[static_cast<std::size_t>(permutation[0])],
          sign[1] * source[static_cast<std::size_t>(permutation[1])],
          sign[2] * source[static_cast<std::size_t>(permutation[2])]};
}

std::vector<ftd::eft::ShapeWorldline> transform_worldlines(
    const std::vector<ftd::eft::ShapeWorldline>& input,
    const std::array<int, 3>& permutation,
    const std::array<int, 3>& sign,
    const ftd::Vec3& translation) {
  const ftd::Vec3 origin{8.0, 8.0, 8.0};
  std::vector<ftd::eft::ShapeWorldline> result;
  result.reserve(input.size());
  for (const auto& line : input) {
    const auto transform = [&](const ftd::Vec3& point) {
      return origin + permute_signed(
          point - origin, permutation, sign) + translation;
    };
    result.push_back({transform(line.start_position),
                      transform(line.end_position), line.charge});
  }
  return result;
}

double history_triple_residual(
    const ftd::eft::AggregateShapeCurrent& stationary,
    const ftd::eft::AggregateShapeCurrent& clockwise,
    const ftd::eft::AggregateShapeCurrent& counterclockwise) {
  if (!stationary.valid || !clockwise.valid || !counterclockwise.valid) {
    return INFINITY;
  }
  return std::max({
      max_difference(stationary.rho_before, clockwise.rho_before),
      max_difference(stationary.rho_before, counterclockwise.rho_before),
      max_difference(stationary.rho_after, clockwise.rho_after),
      max_difference(stationary.rho_after, counterclockwise.rho_after),
      max_difference(stationary.rho_before, stationary.rho_after),
      max_difference(clockwise.rho_before, clockwise.rho_after),
      max_difference(counterclockwise.rho_before,
                     counterclockwise.rho_after),
      current_opposition(clockwise, counterclockwise),
      stationary.aggregate_current_l1,
      stationary.aggregate_continuity_residual,
      clockwise.aggregate_continuity_residual,
      counterclockwise.aggregate_continuity_residual,
      max_current_divergence(clockwise),
      max_current_divergence(counterclockwise),
      std::abs(sourced_field_energy(clockwise)
               - sourced_field_energy(counterclockwise))});
}

}  // namespace

int main() {
  bool dimensions_ok = true;
  for (int side : {3, 5, 17}) {
    const auto dimension = ftd::eft::face_complex_kernel_dimension(side);
    const std::uint64_t volume = static_cast<std::uint64_t>(side * side * side);
    dimensions_ok = dimensions_ok && dimension.valid
        && dimension.site_dimension == volume
        && dimension.face_current_dimension == 3 * volume
        && dimension.divergence_rank == volume - 1
        && dimension.divergence_kernel_dimension == 2 * volume + 1;

    std::vector<double> source(static_cast<std::size_t>(volume), 0.0);
    double nonroot_sum = 0.0;
    for (std::size_t i = 1; i < source.size(); ++i) {
      source[i] = static_cast<double>(static_cast<int>((7 * i) % 11) - 5);
      nonroot_sum += source[i];
    }
    source[0] = -nonroot_sum;
    const auto routed = ftd::eft::route_zero_sum_source_on_tree(side, source);
    worst_tree_routing_residual = std::max(
        worst_tree_routing_residual, routed.routing_residual);
    dimensions_ok = dimensions_ok && routed.valid
        && routed.routing_residual <= gate;
  }
  check("periodic face divergence has exact rank V-1 and kernel 2V+1",
        dimensions_ok);
  check("spanning-tree construction reaches arbitrary zero-sum sources",
        worst_tree_routing_residual <= gate);

  const auto points = square_points();
  const auto stationary_lines = make_history(points, {{0, 1, 2, 3}});
  const auto clockwise_lines = make_history(points, {{1, 2, 3, 0}});
  const auto counterclockwise_lines = make_history(points, {{3, 0, 1, 2}});
  const auto stationary = ftd::eft::make_aggregate_shape_current(
      L, stationary_lines);
  const auto clockwise = ftd::eft::make_aggregate_shape_current(
      L, clockwise_lines);
  const auto counterclockwise = ftd::eft::make_aggregate_shape_current(
      L, counterclockwise_lines);

  endpoint_density_difference = std::max({
      max_difference(stationary.rho_before, stationary.rho_after),
      max_difference(stationary.rho_before, clockwise.rho_before),
      max_difference(stationary.rho_before, clockwise.rho_after),
      max_difference(stationary.rho_before, counterclockwise.rho_before),
      max_difference(stationary.rho_before, counterclockwise.rho_after)});
  check("static, CW, and CCW histories have identical endpoint matter",
        stationary.valid && clockwise.valid && counterclockwise.valid
        && endpoint_density_difference <= gate);

  static_current_l1 = field_l1(stationary);
  clockwise_current_l1 = field_l1(clockwise);
  current_opposition_residual = current_opposition(
      clockwise, counterclockwise);
  loop_divergence_residual = std::max(
      max_current_divergence(clockwise),
      max_current_divergence(counterclockwise));
  check("same endpoints support zero and nonzero divergence-free currents",
        static_current_l1 <= gate && clockwise_current_l1 > 0.1
        && loop_divergence_residual <= gate);
  check("reversing the square worldlines reverses exact face current",
        current_opposition_residual <= gate);

  maximum_segment_length = std::max(
      maximum_length(clockwise_lines),
      maximum_length(counterclockwise_lines));
  check("all loop segments obey the registered causal speed",
        std::abs(maximum_segment_length - 0.5) <= gate
        && maximum_segment_length < ftd::C_SPEED);

  static_field_energy = sourced_field_energy(stationary);
  clockwise_field_energy = sourced_field_energy(clockwise);
  counterclockwise_field_energy = sourced_field_energy(counterclockwise);
  field_opposition_residual = sourced_field_opposition(
      clockwise, counterclockwise);
  field_gauss_residual = kappa * loop_divergence_residual;
  check("current-kernel branches produce distinct Gauss-preserving fields",
        static_field_energy <= gate
        && clockwise_field_energy > 1e-6
        && std::abs(clockwise_field_energy
                    - counterclockwise_field_energy) <= gate
        && field_opposition_residual <= gate
        && field_gauss_residual <= gate);

  const std::array<std::array<int, 3>, 6> permutations{{
      {{0, 1, 2}}, {{0, 2, 1}}, {{1, 0, 2}},
      {{1, 2, 0}}, {{2, 0, 1}}, {{2, 1, 0}}}};
  const std::array<ftd::Vec3, 3> translations{{
      {-2.0, 1.0, 0.0}, {0.0, 0.0, 0.0}, {2.0, -1.0, 1.0}}};
  bool transformed_ok = true;
  for (const auto& permutation : permutations) {
    for (int mask = 0; mask < 8; ++mask) {
      const std::array<int, 3> sign{{
          (mask & 1) ? -1 : +1,
          (mask & 2) ? -1 : +1,
          (mask & 4) ? -1 : +1}};
      for (const auto& translation : translations) {
        const auto transformed_stationary_lines = transform_worldlines(
            stationary_lines, permutation, sign, translation);
        const auto transformed_clockwise_lines = transform_worldlines(
            clockwise_lines, permutation, sign, translation);
        const auto transformed_counterclockwise_lines = transform_worldlines(
            counterclockwise_lines, permutation, sign, translation);
        const auto transformed_stationary =
            ftd::eft::make_aggregate_shape_current(
                L, transformed_stationary_lines);
        const auto transformed_clockwise =
            ftd::eft::make_aggregate_shape_current(
                L, transformed_clockwise_lines);
        const auto transformed_counterclockwise =
            ftd::eft::make_aggregate_shape_current(
                L, transformed_counterclockwise_lines);
        const double residual = history_triple_residual(
            transformed_stationary,
            transformed_clockwise,
            transformed_counterclockwise);
        worst_transformed_residual = std::max(
            worst_transformed_residual, residual);
        const double transformed_length = std::max(
            maximum_length(transformed_clockwise_lines),
            maximum_length(transformed_counterclockwise_lines));
        transformed_ok = transformed_ok && residual <= gate
            && transformed_length < ftd::C_SPEED
            && field_l1(transformed_clockwise) > 0.1;
        ++transformed_history_triples;
      }
    }
  }
  check("loop ambiguity survives every cubic map and translation",
        transformed_ok && transformed_history_triples == 144
        && worst_transformed_residual <= gate);

  std::vector<double> nonneutral(27, 0.0);
  nonneutral[0] = 1.0;
  check("invalid dimensions and nonzero-sum routing fail closed",
        !ftd::eft::face_complex_kernel_dimension(1).valid
        && !ftd::eft::route_zero_sum_source_on_tree(
            3, nonneutral).valid);

  const auto l17_dimension = ftd::eft::face_complex_kernel_dimension(L);
  std::cout.precision(17);
  std::cout << "L17_site_dimension="
            << l17_dimension.site_dimension << '\n'
            << "L17_face_current_dimension="
            << l17_dimension.face_current_dimension << '\n'
            << "L17_divergence_rank="
            << l17_dimension.divergence_rank << '\n'
            << "L17_current_kernel_dimension="
            << l17_dimension.divergence_kernel_dimension << '\n'
            << "worst_tree_routing_residual="
            << worst_tree_routing_residual << '\n'
            << "endpoint_density_difference="
            << endpoint_density_difference << '\n'
            << "static_current_l1=" << static_current_l1 << '\n'
            << "clockwise_current_l1=" << clockwise_current_l1 << '\n'
            << "current_opposition_residual="
            << current_opposition_residual << '\n'
            << "loop_divergence_residual="
            << loop_divergence_residual << '\n'
            << "maximum_segment_length="
            << maximum_segment_length << '\n'
            << "static_field_energy=" << static_field_energy << '\n'
            << "clockwise_field_energy="
            << clockwise_field_energy << '\n'
            << "counterclockwise_field_energy="
            << counterclockwise_field_energy << '\n'
            << "field_opposition_residual="
            << field_opposition_residual << '\n'
            << "field_gauss_residual="
            << field_gauss_residual << '\n'
            << "transformed_history_triples="
            << transformed_history_triples << '\n'
            << "worst_transformed_residual="
            << worst_transformed_residual << '\n'
            << "worldline_current_kernel failures="
            << failures << '\n'
            << "verdict=WORLDLINE_PATH_IS_REQUIRED_STATE\n";
  return failures == 0 ? 0 : 1;
}
