/** FTD-0555: extended local-source Peierls scaling. */

#include "ftd/eft/extended_source_peierls_scaling.h"
#include "ftd/eft/face_flux_normalization.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace {

constexpr double identity_gate = 1e-12;
constexpr double slope_gate = 0.15;
constexpr double constant_gate = 0.10;
constexpr double volume_gate = 0.10;
int failures = 0;

void check(const std::string& label, bool pass) {
  if (pass) return;
  ++failures;
  std::cerr << "FAIL: " << label << '\n';
}

double relative_error(double measured, double expected) {
  return std::abs(measured-expected)/std::abs(expected);
}

double log_slope(const std::vector<int>& orders,
                 const std::vector<double>& values) {
  double sx = 0.0;
  double sy = 0.0;
  double sxx = 0.0;
  double sxy = 0.0;
  const double count = static_cast<double>(orders.size());
  for (std::size_t i = 0; i < orders.size(); ++i) {
    const double x = std::log(static_cast<double>(orders[i]));
    const double y = std::log(values[i]);
    sx += x;
    sy += y;
    sxx += x*x;
    sxy += x*y;
  }
  return (count*sxy-sx*sy)/(count*sxx-sx*sx);
}

ftd::Coord unit_axis(int axis) {
  return axis == 0 ? ftd::Coord{1,0,0}
      : (axis == 1 ? ftd::Coord{0,1,0} : ftd::Coord{0,0,1});
}

std::vector<ftd::eft::ExtendedPeierlsProfile> profiles_for_axis(int axis) {
  return {
      {ftd::eft::ExtendedPeierlsProfileKind::MonopoleBackground, {0,0,0}},
      {ftd::eft::ExtendedPeierlsProfileKind::Dipole, unit_axis(axis)},
      {ftd::eft::ExtendedPeierlsProfileKind::Dipole,
          unit_axis((axis+1)%3)},
      {ftd::eft::ExtendedPeierlsProfileKind::Dipole, {1,1,1}},
  };
}

struct Series {
  std::vector<double> energy;
  std::vector<double> barrier;
  std::vector<double> relative;
};

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  check("native normalization", normalization.valid);
  const double beta = normalization.mapped_field_work_coefficient;
  const std::vector<int> main_orders{8,16,32,64,128};
  const std::vector<int> replication_orders{8,16,32,64};
  const std::vector<int> fit_orders{16,32,64,128};
  const auto profiles = profiles_for_axis(0);

  std::map<int, ftd::eft::ExtendedSourcePeierlsResult> main_results;
  std::map<int, ftd::eft::ExtendedSourcePeierlsResult> replication_results;
  double worst_envelope_identity = 0.0;
  double worst_spectral_identity = 0.0;
  for (int order : main_orders) {
    auto result = ftd::eft::evaluate_extended_source_peierls(
        257, order, 0, profiles, beta);
    check("main observer valid m="+std::to_string(order), result.valid);
    check("main algebra m="+std::to_string(order),
        result.envelope.maximum_identity_residual <= identity_gate
        && result.maximum_identity_residual <= identity_gate);
    worst_envelope_identity = std::max(worst_envelope_identity,
        result.envelope.maximum_identity_residual);
    worst_spectral_identity = std::max(worst_spectral_identity,
        result.maximum_identity_residual);
    main_results.emplace(order, std::move(result));
  }
  for (int order : replication_orders) {
    auto result = ftd::eft::evaluate_extended_source_peierls(
        129, order, 0, profiles, beta);
    check("replication observer valid m="+std::to_string(order), result.valid);
    check("replication algebra m="+std::to_string(order),
        result.envelope.maximum_identity_residual <= identity_gate
        && result.maximum_identity_residual <= identity_gate);
    worst_envelope_identity = std::max(worst_envelope_identity,
        result.envelope.maximum_identity_residual);
    worst_spectral_identity = std::max(worst_spectral_identity,
        result.maximum_identity_residual);
    replication_results.emplace(order, std::move(result));
  }

  std::array<Series,4> series;
  double worst_slope_residual = 0.0;
  double worst_constant_relative_error = 0.0;
  double largest_m128_relative_barrier = 0.0;
  double minimum_improvement_factor = INFINITY;
  for (std::size_t profile = 0; profile < profiles.size(); ++profile) {
    double previous = INFINITY;
    for (int order : main_orders) {
      const auto& sample = main_results.at(order).samples[profile];
      check("strict pinning decrease profile="+std::to_string(profile)
                +" m="+std::to_string(order),
          sample.relative_barrier < previous);
      previous = sample.relative_barrier;
    }
    minimum_improvement_factor = std::min(minimum_improvement_factor,
        main_results.at(8).samples[profile].relative_barrier
        /main_results.at(128).samples[profile].relative_barrier);
    largest_m128_relative_barrier = std::max(
        largest_m128_relative_barrier,
        main_results.at(128).samples[profile].relative_barrier);
    check("m128 relative barrier profile="+std::to_string(profile),
        main_results.at(128).samples[profile].relative_barrier < 5e-5);

    for (int order : fit_orders) {
      const auto& sample = main_results.at(order).samples[profile];
      series[profile].energy.push_back(sample.energy_zero);
      series[profile].barrier.push_back(sample.half_cell_barrier);
      series[profile].relative.push_back(sample.relative_barrier);
    }
    const double energy_target = profile == 0 ? -0.5 : -1.5;
    const double barrier_target = profile == 0 ? -2.5 : -3.5;
    const double energy_slope = log_slope(fit_orders, series[profile].energy);
    const double barrier_slope = log_slope(fit_orders, series[profile].barrier);
    const double relative_slope = log_slope(fit_orders, series[profile].relative);
    worst_slope_residual = std::max({worst_slope_residual,
        std::abs(energy_slope-energy_target),
        std::abs(barrier_slope-barrier_target),
        std::abs(relative_slope+2.0)});
    check("energy slope profile="+std::to_string(profile),
        std::abs(energy_slope-energy_target) <= slope_gate);
    check("barrier slope profile="+std::to_string(profile),
        std::abs(barrier_slope-barrier_target) <= slope_gate);
    check("relative slope profile="+std::to_string(profile),
        std::abs(relative_slope+2.0) <= slope_gate);

    const auto& endpoint = main_results.at(128).samples[profile];
    const std::array<double,3> measured{{
        endpoint.scaled_energy_constant,
        endpoint.scaled_barrier_constant,
        endpoint.scaled_relative_constant}};
    const std::array<double,3> expected{{
        endpoint.expected_energy_constant,
        endpoint.expected_barrier_constant,
        endpoint.expected_relative_constant}};
    for (int item = 0; item < 3; ++item) {
      const double error = relative_error(measured[item], expected[item]);
      worst_constant_relative_error = std::max(
          worst_constant_relative_error, error);
      check("asymptotic constant profile="+std::to_string(profile)
                +" item="+std::to_string(item), error <= constant_gate);
    }
  }

  double worst_volume_relative_difference = 0.0;
  double worst_convergence_direction_excess = 0.0;
  for (int order : replication_orders) {
    for (std::size_t profile = 0; profile < profiles.size(); ++profile) {
      const auto& main = main_results.at(order).samples[profile];
      const auto& replica = replication_results.at(order).samples[profile];
      const double difference = relative_error(
          replica.relative_barrier, main.relative_barrier);
      worst_volume_relative_difference = std::max(
          worst_volume_relative_difference, difference);
      check("volume relative barrier profile="+std::to_string(profile)
                +" m="+std::to_string(order), difference < volume_gate);
    }
  }
  for (std::size_t profile = 0; profile < profiles.size(); ++profile) {
    const auto& main = main_results.at(64).samples[profile];
    const auto& replica = replication_results.at(64).samples[profile];
    const std::array<double,3> main_values{{main.scaled_energy_constant,
        main.scaled_barrier_constant, main.scaled_relative_constant}};
    const std::array<double,3> replica_values{{replica.scaled_energy_constant,
        replica.scaled_barrier_constant, replica.scaled_relative_constant}};
    const std::array<double,3> expected{{main.expected_energy_constant,
        main.expected_barrier_constant, main.expected_relative_constant}};
    for (int item = 0; item < 3; ++item) {
      const double excess = relative_error(main_values[item], expected[item])
          -relative_error(replica_values[item], expected[item]);
      worst_convergence_direction_excess = std::max(
          worst_convergence_direction_excess, excess);
      check("volume convergence direction profile="+std::to_string(profile)
                +" item="+std::to_string(item), excess <= 1e-12);
    }
  }

  double worst_rotation_relative_residual = 0.0;
  const auto rotation_reference = ftd::eft::evaluate_extended_source_peierls(
      65, 16, 0, profiles_for_axis(0), beta);
  check("rotation reference valid", rotation_reference.valid);
  for (int axis : {1,2}) {
    const auto rotated = ftd::eft::evaluate_extended_source_peierls(
        65, 16, axis, profiles_for_axis(axis), beta);
    check("rotation observer valid axis="+std::to_string(axis), rotated.valid);
    for (std::size_t profile = 0; profile < profiles.size(); ++profile) {
      const auto& lhs = rotation_reference.samples[profile];
      const auto& rhs = rotated.samples[profile];
      const double residual = std::max({
          relative_error(rhs.energy_zero, lhs.energy_zero),
          relative_error(rhs.half_cell_barrier, lhs.half_cell_barrier),
          relative_error(rhs.relative_barrier, lhs.relative_barrier)});
      worst_rotation_relative_residual = std::max(
          worst_rotation_relative_residual, residual);
      check("cubic covariance axis="+std::to_string(axis)
                +" profile="+std::to_string(profile), residual <= 1e-13);
    }
  }

  check("locked cardinalities", main_results.size() == 5
      && replication_results.size() == 4 && profiles.size() == 4);
  const bool passed = failures == 0;
  std::cout << "beta=" << beta << '\n'
            << "main_volume=257\n"
            << "replication_volume=129\n"
            << "registered_main_samples=" << 5*4 << '\n'
            << "registered_replication_samples=" << 4*4 << '\n'
            << "registered_rotation_samples=" << 3*4 << '\n'
            << "worst_envelope_identity_residual="
            << worst_envelope_identity << '\n'
            << "worst_spectral_identity_residual="
            << worst_spectral_identity << '\n'
            << "worst_slope_residual=" << worst_slope_residual << '\n'
            << "worst_constant_relative_error="
            << worst_constant_relative_error << '\n'
            << "worst_volume_relative_difference="
            << worst_volume_relative_difference << '\n'
            << "worst_convergence_direction_excess="
            << worst_convergence_direction_excess << '\n'
            << "worst_rotation_relative_residual="
            << worst_rotation_relative_residual << '\n'
            << "largest_m128_relative_barrier="
            << largest_m128_relative_barrier << '\n'
            << "minimum_improvement_factor="
            << minimum_improvement_factor << '\n';
  for (std::size_t profile = 0; profile < profiles.size(); ++profile) {
    std::cout << "profile_" << profile << "_energy_slope="
              << log_slope(fit_orders, series[profile].energy) << '\n'
              << "profile_" << profile << "_barrier_slope="
              << log_slope(fit_orders, series[profile].barrier) << '\n'
              << "profile_" << profile << "_relative_slope="
              << log_slope(fit_orders, series[profile].relative) << '\n'
              << "profile_" << profile << "_m128_scaled_relative="
              << main_results.at(128).samples[profile].scaled_relative_constant
              << '\n';
  }
  std::cout << "verdict="
            << (passed
                ? "LOCAL_EXTENSION_SUPPRESSES_COMPACT_COAT_PINNING"
                : "SCALING_THEOREM_NUMERICAL_CONTROL_FAILED") << '\n'
            << "extended_source_peierls_scaling failures="
            << failures << '\n';
  return passed ? 0 : 1;
}
