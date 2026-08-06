// FTD-0621: exact ternary block-bipole Peierls scaling.

#include "ftd/eft/face_flux_normalization.h"
#include "ftd/eft/ternary_block_bipole_peierls.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <vector>

namespace {

constexpr char protocol_sha256_0621[] =
    "905819BD83E8C4AC6698A75D7C87640B807BC2C621350DCAAE8563945148CB31";
constexpr double structure_gate = 1e-11;
constexpr double identity_gate = 1e-12;
constexpr double covariance_gate = 1e-12;
constexpr double volume_gate = 0.08;
constexpr double endpoint_gate = 5e-5;

using Result = ftd::eft::TernaryBlockBipolePeierlsResult;
using Key = std::tuple<int,int,int>;  // L, width, orientation

double relative_difference(double lhs, double rhs) {
  return std::abs(lhs-rhs)/std::max({1e-300, std::abs(lhs), std::abs(rhs)});
}

double log_slope(const std::vector<int>& widths,
                 const std::vector<double>& values) {
  long double sx = 0.0L, sy = 0.0L, sxx = 0.0L, sxy = 0.0L;
  const long double count = static_cast<long double>(widths.size());
  for (std::size_t i = 0; i < widths.size(); ++i) {
    const long double x = std::log(static_cast<long double>(widths[i]));
    const long double y = std::log(static_cast<long double>(values[i]));
    sx += x;
    sy += y;
    sxx += x*x;
    sxy += x*y;
  }
  return static_cast<double>((count*sxy-sx*sy)/(count*sxx-sx*sx));
}

struct Summary0621 {
  bool coverage = false;
  bool algebraic_pass = false;
  bool covariance_pass = false;
  bool volume_pass = false;
  bool monotonic_pass = false;
  bool scaling_pass = false;
  bool endpoint_pass = false;
  double beta = 0.0;
  double worst_structure_residual = 0.0;
  double worst_spectral_identity = 0.0;
  double worst_covariance_residual = 0.0;
  double worst_volume_relative_difference = 0.0;
  double worst_energy_slope_residual = 0.0;
  double worst_barrier_slope_residual = 0.0;
  double worst_pinning_slope_residual = 0.0;
  double largest_endpoint_pinning_index = 0.0;
  double smallest_endpoint_improvement = INFINITY;
  std::string verdict;
  std::map<Key, Result> results;
};

void evaluate_summary(Summary0621& summary,
                      const std::vector<int>& widths) {
  summary.coverage = summary.results.size() == 30;
  summary.algebraic_pass = summary.coverage;
  for (const auto& [key, result] : summary.results) {
    const auto [L, width, orientation] = key;
    (void)L;
    summary.worst_structure_residual = std::max(
        summary.worst_structure_residual,
        result.structure_factor_relative_residual);
    for (double residual : result.spectral_identity_residual)
      summary.worst_spectral_identity = std::max(
          summary.worst_spectral_identity, residual);
    summary.algebraic_pass = summary.algebraic_pass && result.valid
        && result.exactly_neutral && result.support_does_not_wrap
        && result.positive_sites == static_cast<std::int64_t>(width)*width*width
        && result.negative_sites == result.positive_sites
        && result.occupied_sites == 2*result.positive_sites
        && result.structure_factor_relative_residual <= structure_gate;
    for (int axis = 0; axis < 3; ++axis)
      summary.algebraic_pass = summary.algebraic_pass
          && result.spectral_identity_residual[axis] <= identity_gate;
    (void)orientation;
  }

  summary.covariance_pass = summary.coverage;
  for (int L : {193,257}) {
    for (int width : widths) {
      const auto& reference = summary.results.at({L,width,0});
      const double parallel = reference.pinning_index[0];
      const double transverse = reference.pinning_index[1];
      for (int orientation = 0; orientation < 3; ++orientation) {
        const auto& result = summary.results.at({L,width,orientation});
        summary.worst_covariance_residual = std::max(
            summary.worst_covariance_residual,
            relative_difference(result.energy, reference.energy));
        for (int axis = 0; axis < 3; ++axis) {
          const double expected = axis == orientation
              ? parallel : transverse;
          summary.worst_covariance_residual = std::max(
              summary.worst_covariance_residual,
              relative_difference(result.pinning_index[axis], expected));
        }
      }
    }
  }
  summary.covariance_pass = summary.covariance_pass
      && summary.worst_covariance_residual <= covariance_gate;

  summary.volume_pass = summary.coverage;
  for (int width : widths) {
    if (width > 23) continue;
    for (int orientation = 0; orientation < 3; ++orientation) {
      const auto& main = summary.results.at({257,width,orientation});
      const auto& replica = summary.results.at({193,width,orientation});
      for (int axis = 0; axis < 3; ++axis)
        summary.worst_volume_relative_difference = std::max(
            summary.worst_volume_relative_difference,
            relative_difference(main.pinning_index[axis],
                                replica.pinning_index[axis]));
    }
  }
  summary.volume_pass = summary.volume_pass
      && summary.worst_volume_relative_difference <= volume_gate;

  summary.monotonic_pass = summary.coverage;
  for (int orientation = 0; orientation < 3; ++orientation) {
    for (int axis = 0; axis < 3; ++axis) {
      double previous = INFINITY;
      for (int width : widths) {
        const double value =
            summary.results.at({257,width,orientation}).pinning_index[axis];
        summary.monotonic_pass = summary.monotonic_pass && value < previous;
        previous = value;
      }
      const auto& first = summary.results.at({257,widths.front(),orientation});
      const auto& last = summary.results.at({257,widths.back(),orientation});
      summary.smallest_endpoint_improvement = std::min(
          summary.smallest_endpoint_improvement,
          first.pinning_index[axis]/last.pinning_index[axis]);
      summary.largest_endpoint_pinning_index = std::max(
          summary.largest_endpoint_pinning_index,
          last.pinning_index[axis]);
    }
  }
  summary.endpoint_pass = summary.largest_endpoint_pinning_index
      < endpoint_gate;

  const std::vector<int> fit_widths(widths.begin()+1, widths.end());
  summary.scaling_pass = summary.coverage;
  for (int orientation = 0; orientation < 3; ++orientation) {
    std::vector<double> energies;
    for (int width : fit_widths)
      energies.push_back(summary.results.at(
          {257,width,orientation}).energy);
    const double energy_slope = log_slope(fit_widths, energies);
    summary.worst_energy_slope_residual = std::max(
        summary.worst_energy_slope_residual,
        std::abs(energy_slope-5.0));
    summary.scaling_pass = summary.scaling_pass
        && energy_slope >= 4.65 && energy_slope <= 5.35;
    for (int axis = 0; axis < 3; ++axis) {
      std::vector<double> barriers;
      std::vector<double> pinning;
      for (int width : fit_widths) {
        const auto& result = summary.results.at({257,width,orientation});
        barriers.push_back(result.half_cell_barrier[axis]);
        pinning.push_back(result.pinning_index[axis]);
      }
      const double barrier_slope = log_slope(fit_widths, barriers);
      const double pinning_slope = log_slope(fit_widths, pinning);
      summary.worst_barrier_slope_residual = std::max(
          summary.worst_barrier_slope_residual,
          std::abs(barrier_slope-2.0));
      summary.worst_pinning_slope_residual = std::max(
          summary.worst_pinning_slope_residual,
          std::abs(pinning_slope+3.0));
      summary.scaling_pass = summary.scaling_pass
          && barrier_slope >= 1.65 && barrier_slope <= 2.35
          && pinning_slope >= -3.35 && pinning_slope <= -2.65;
    }
  }

  if (!summary.algebraic_pass || !summary.covariance_pass)
    summary.verdict = "TERNARY_BLOCK_BIPOLE_OBSERVER_INVALID";
  else if (summary.volume_pass && summary.monotonic_pass
           && summary.scaling_pass && summary.endpoint_pass)
    summary.verdict = "INTEGER_TERNARY_EXTENSION_SUPPRESSES_PEIERLS";
  else if (summary.monotonic_pass)
    summary.verdict =
        "INTEGER_TERNARY_SUPPRESSION_NOT_ASYMPTOTICALLY_QUALIFIED";
  else
    summary.verdict = "INTEGER_TERNARY_EXTENSION_DOES_NOT_SUPPRESS_PINNING";
}

void write_record(const Summary0621& summary) {
  const auto dir = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results" / "ftd_0621";
  std::filesystem::create_directories(dir);
  std::ofstream json(dir / "ftd_0621_ternary_block_bipole_peierls_v1.json");
  json << std::setprecision(17) << "{\n"
       << "  \"ftd_id\": \"FTD-0621\",\n"
       << "  \"protocol_sha256\": \"" << protocol_sha256_0621 << "\",\n"
       << "  \"verdict\": \"" << summary.verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"coverage_pass\": " << summary.coverage << ",\n"
       << "  \"algebraic_pass\": " << summary.algebraic_pass << ",\n"
       << "  \"covariance_pass\": " << summary.covariance_pass << ",\n"
       << "  \"volume_pass\": " << summary.volume_pass << ",\n"
       << "  \"monotonic_pass\": " << summary.monotonic_pass << ",\n"
       << "  \"scaling_pass\": " << summary.scaling_pass << ",\n"
       << "  \"endpoint_pass\": " << summary.endpoint_pass << ",\n"
       << "  \"beta\": " << summary.beta << ",\n"
       << "  \"worst_structure_residual\": "
       << summary.worst_structure_residual << ",\n"
       << "  \"worst_spectral_identity\": "
       << summary.worst_spectral_identity << ",\n"
       << "  \"worst_covariance_residual\": "
       << summary.worst_covariance_residual << ",\n"
       << "  \"worst_volume_relative_difference\": "
       << summary.worst_volume_relative_difference << ",\n"
       << "  \"worst_energy_slope_residual\": "
       << summary.worst_energy_slope_residual << ",\n"
       << "  \"worst_barrier_slope_residual\": "
       << summary.worst_barrier_slope_residual << ",\n"
       << "  \"worst_pinning_slope_residual\": "
       << summary.worst_pinning_slope_residual << ",\n"
       << "  \"largest_endpoint_pinning_index\": "
       << summary.largest_endpoint_pinning_index << ",\n"
       << "  \"smallest_endpoint_improvement\": "
       << summary.smallest_endpoint_improvement << "\n"
       << "}\n";

  std::ofstream csv(dir / "ftd_0621_ternary_block_bipole_peierls_v1.csv");
  csv << "ftd_id,L,width,orientation,translation_axis,positive_sites,"
         "negative_sites,energy,coefficient,barrier,pinning_index,"
         "spectral_average,spectral_identity,structure_residual\n";
  for (const auto& [key, result] : summary.results) {
    const auto [L,width,orientation] = key;
    for (int axis = 0; axis < 3; ++axis)
      csv << std::setprecision(17) << "FTD-0621," << L << ',' << width
          << ',' << orientation << ',' << axis << ','
          << result.positive_sites << ',' << result.negative_sites << ','
          << result.energy << ',' << result.peierls_coefficient[axis] << ','
          << result.half_cell_barrier[axis] << ','
          << result.pinning_index[axis] << ','
          << result.spectral_average[axis] << ','
          << result.spectral_identity_residual[axis] << ','
          << result.structure_factor_relative_residual << '\n';
  }
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  Summary0621 summary;
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  summary.beta = normalization.mapped_field_work_coefficient;
  const std::vector<int> widths{5,9,15,23,35};
  if (normalization.valid) {
    for (int L : {193,257})
      for (int width : widths)
        for (int orientation = 0; orientation < 3; ++orientation)
          summary.results.emplace(Key{L,width,orientation},
              ftd::eft::evaluate_ternary_block_bipole_peierls(
                  L, width, orientation, summary.beta));
  }
  evaluate_summary(summary, widths);
  write_record(summary);

  std::cout << "protocol_sha256=" << protocol_sha256_0621 << '\n'
            << "verdict=" << summary.verdict << '\n'
            << "arms=" << summary.results.size()*3
            << " algebra=" << summary.algebraic_pass
            << " covariance=" << summary.covariance_pass
            << " volume=" << summary.volume_pass
            << " monotonic=" << summary.monotonic_pass
            << " scaling=" << summary.scaling_pass
            << " endpoint=" << summary.endpoint_pass << '\n'
            << "structure=" << summary.worst_structure_residual
            << " spectral_identity=" << summary.worst_spectral_identity
            << " covariance_residual="
            << summary.worst_covariance_residual
            << " volume_difference="
            << summary.worst_volume_relative_difference << '\n'
            << "slope_residuals=" << summary.worst_energy_slope_residual
            << ',' << summary.worst_barrier_slope_residual << ','
            << summary.worst_pinning_slope_residual
            << " endpoint=" << summary.largest_endpoint_pinning_index
            << " improvement=" << summary.smallest_endpoint_improvement
            << '\n';
  return summary.coverage && summary.algebraic_pass
      && summary.covariance_pass ? 0 : 1;
}

