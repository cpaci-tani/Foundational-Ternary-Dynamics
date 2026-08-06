#include "ftd/eft/localized_basin_observer.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>

namespace {

int checks = 0;
int failures = 0;

void check(const std::string& label, bool pass) {
  ++checks;
  if (!pass) {
    ++failures;
    std::cerr << "FAIL: " << label << '\n';
  }
}

bool close(double left, double right, double tolerance = 1e-13) {
  return std::abs(left - right) <= tolerance;
}

ftd::eft::ConnectedMooreBlockState make_reference() {
  ftd::eft::ConnectedMooreBlockState state(7);
  state.width = 2;
  state.orientation_axis = -1;
  state.constituents = {
      {{2, 3, 3}, {}, {}},
      {{4, 3, 3}, {}, {}}};
  state.charges = {1, -1};
  state.edges.push_back({0, 1, {2, 0, 0}, 4.0});
  return state;
}

void shift_field(std::vector<double>& output,
                 const std::vector<double>& input,
                 int L,
                 int dx,
                 int dy,
                 int dz) {
  output.assign(input.size(), 0.0);
  auto mod = [L](int value) { return (value % L + L) % L; };
  for (int z = 0; z < L; ++z)
    for (int y = 0; y < L; ++y)
      for (int x = 0; x < L; ++x) {
        const int source = (x * L + y) * L + z;
        const int target = (mod(x + dx) * L + mod(y + dy)) * L
            + mod(z + dz);
        output[static_cast<std::size_t>(target)] =
            input[static_cast<std::size_t>(source)];
      }
}

ftd::eft::ConnectedMooreBlockState translated(
    const ftd::eft::ConnectedMooreBlockState& input,
    int dx,
    int dy,
    int dz) {
  auto result = input;
  const int L = input.electric.L;
  auto mod = [L](int value) { return (value % L + L) % L; };
  for (auto& point : result.constituents) {
    point.anchor.x = mod(point.anchor.x + dx);
    point.anchor.y = mod(point.anchor.y + dy);
    point.anchor.z = mod(point.anchor.z + dz);
  }
  shift_field(result.electric.x, input.electric.x, L, dx, dy, dz);
  shift_field(result.electric.y, input.electric.y, L, dx, dy, dz);
  shift_field(result.electric.z, input.electric.z, L, dx, dy, dz);
  shift_field(result.magnetic_half.x, input.magnetic_half.x, L, dx, dy, dz);
  shift_field(result.magnetic_half.y, input.magnetic_half.y, L, dx, dy, dz);
  shift_field(result.magnetic_half.z, input.magnetic_half.z, L, dx, dy, dz);
  return result;
}

ftd::Vec3 cycle(const ftd::Vec3& value) {
  return {value.y, value.z, value.x};
}

ftd::Coord cycle(const ftd::Coord& value) {
  return {value.y, value.z, value.x};
}

ftd::eft::ConnectedMooreBlockState cycled(
    const ftd::eft::ConnectedMooreBlockState& input) {
  auto result = input;
  const int L = input.electric.L;
  for (auto& point : result.constituents) {
    point.anchor = cycle(point.anchor);
    point.remainder = cycle(point.remainder);
    point.momentum = cycle(point.momentum);
  }
  for (auto& edge : result.edges) edge.reference_delta = cycle(edge.reference_delta);
  auto transform = [L](std::vector<double>& ox,
                       std::vector<double>& oy,
                       std::vector<double>& oz,
                       const std::vector<double>& ix,
                       const std::vector<double>& iy,
                       const std::vector<double>& iz) {
    ox.assign(ix.size(), 0.0);
    oy.assign(ix.size(), 0.0);
    oz.assign(ix.size(), 0.0);
    for (int z = 0; z < L; ++z)
      for (int y = 0; y < L; ++y)
        for (int x = 0; x < L; ++x) {
          const int source = (x * L + y) * L + z;
          const int target = (y * L + z) * L + x;
          ox[static_cast<std::size_t>(target)] = iy[static_cast<std::size_t>(source)];
          oy[static_cast<std::size_t>(target)] = iz[static_cast<std::size_t>(source)];
          oz[static_cast<std::size_t>(target)] = ix[static_cast<std::size_t>(source)];
        }
  };
  transform(result.electric.x, result.electric.y, result.electric.z,
            input.electric.x, input.electric.y, input.electric.z);
  transform(result.magnetic_half.x, result.magnetic_half.y,
            result.magnetic_half.z, input.magnetic_half.x,
            input.magnetic_half.y, input.magnetic_half.z);
  return result;
}

}  // namespace

int main() {
  constexpr double mass = 0.5;
  constexpr double omega = 2.0;
  constexpr double beta = 3.0;
  constexpr double speed = 0.5;
  // Deliberately asymmetric so an x/z storage-index reversal cannot pass.
  const ftd::Vec3 origin{3.0, 2.0, 1.0};
  const auto reference = make_reference();

  const auto zero = ftd::eft::observe_localized_basin(
      reference, reference, origin, 1, 2, omega, beta, speed, mass);
  check("identical state valid", zero.valid);
  check("identical core metric zero", close(zero.core_phase_metric, 0.0));
  check("identical field metric zero", close(zero.total_dynamic_field, 0.0));

  auto collective = reference;
  for (auto& point : collective.constituents) {
    point.remainder += ftd::Vec3{0.25, -0.125, 0.0625};
    point.momentum += ftd::Vec3{0.1, -0.2, 0.3};
  }
  const auto quotient = ftd::eft::observe_localized_basin(
      reference, collective, origin, 1, 2, omega, beta, speed, mass);
  check("collective state valid", quotient.valid);
  check("translation quotiented", close(quotient.internal_position_metric, 0.0));
  check("boost quotiented", close(quotient.internal_momentum_metric, 0.0));
  check("translation recorded", quotient.center_offset_norm > 0.0);
  check("boost recorded", quotient.mean_momentum_offset_norm > 0.0);

  auto candidate = reference;
  candidate.constituents[0].remainder.x = 0.25;
  candidate.constituents[1].remainder.x = -0.25;
  candidate.constituents[0].momentum.y = 0.2;
  candidate.constituents[1].momentum.y = -0.2;
  const int near_index = candidate.electric.index(3, 2, 1);
  const int middle_index = candidate.electric.index(5, 2, 1);
  const int far_index = candidate.electric.index(6, 2, 1);
  candidate.electric.x[static_cast<std::size_t>(near_index)] = 2.0;
  candidate.electric.y[static_cast<std::size_t>(middle_index)] = 3.0;
  candidate.magnetic_half.z[static_cast<std::size_t>(far_index)] = 4.0;

  const auto observed = ftd::eft::observe_localized_basin(
      reference, candidate, origin, 1, 2, omega, beta, speed, mass);
  const double expected_position = mass * 2.0 * 0.25 * 0.25;
  const double expected_momentum = 2.0 * 0.2 * 0.2 / mass;
  const double expected_near = 0.5 * beta * 4.0;
  const double expected_middle = 0.5 * beta * 9.0;
  const double expected_far = 0.5 * beta * speed * speed * 16.0;
  check("distorted state valid", observed.valid);
  check("position metric exact", close(observed.internal_position_metric,
                                        expected_position));
  check("momentum metric exact", close(observed.internal_momentum_metric,
                                        expected_momentum));
  check("phase metric exact", close(observed.core_phase_metric,
      omega * omega * expected_position + expected_momentum));
  check("near field exact", close(observed.near_dynamic_field, expected_near));
  check("middle field exact", close(observed.intermediate_dynamic_field,
                                     expected_middle));
  check("far field exact", close(observed.far_dynamic_field, expected_far));
  check("field partition exact", close(observed.total_dynamic_field,
      expected_near + expected_middle + expected_far));
  check("partition residual", observed.field_partition_residual <= 1e-14);
  check("edge difference detected", observed.maximum_edge_length_difference > 0.0);

  const auto reference_shifted = translated(reference, 1, 2, -1);
  const auto candidate_shifted = translated(candidate, 1, 2, -1);
  const auto shift_observed = ftd::eft::observe_localized_basin(
      reference_shifted, candidate_shifted, origin + ftd::Vec3{1, 2, -1},
      1, 2, omega, beta, speed, mass);
  check("integer translation valid", shift_observed.valid);
  check("integer translation core covariance",
        close(shift_observed.core_phase_metric, observed.core_phase_metric));
  check("integer translation field covariance",
        close(shift_observed.total_dynamic_field, observed.total_dynamic_field));
  check("integer translation shell covariance",
        close(shift_observed.near_dynamic_field, observed.near_dynamic_field)
        && close(shift_observed.far_dynamic_field, observed.far_dynamic_field));

  const auto reference_cycled = cycled(reference);
  const auto candidate_cycled = cycled(candidate);
  const auto cycle_observed = ftd::eft::observe_localized_basin(
      reference_cycled, candidate_cycled, cycle(origin),
      1, 2, omega, beta, speed, mass);
  check("cubic cycle valid", cycle_observed.valid);
  check("cubic core covariance",
        close(cycle_observed.core_phase_metric, observed.core_phase_metric));
  check("cubic field covariance",
        close(cycle_observed.total_dynamic_field, observed.total_dynamic_field));
  check("cubic shell covariance",
        close(cycle_observed.near_dynamic_field, observed.near_dynamic_field)
        && close(cycle_observed.intermediate_dynamic_field,
                 observed.intermediate_dynamic_field)
        && close(cycle_observed.far_dynamic_field, observed.far_dynamic_field));

  auto wrong_topology = candidate;
  wrong_topology.charges[0] = -1;
  const auto rejected = ftd::eft::observe_localized_basin(
      reference, wrong_topology, origin, 1, 2, omega, beta, speed, mass);
  check("topology mismatch rejected", !rejected.valid && !rejected.topology_match);

  std::cout << std::setprecision(17)
            << "FTD-0677 localized-basin observer\n"
            << "checks=" << checks << " failures=" << failures << '\n'
            << "position=" << observed.internal_position_metric
            << " momentum=" << observed.internal_momentum_metric
            << " phase=" << observed.core_phase_metric << '\n'
            << "field=(" << observed.near_dynamic_field << ','
            << observed.intermediate_dynamic_field << ','
            << observed.far_dynamic_field << ") residual="
            << observed.field_partition_residual << '\n';
  return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
