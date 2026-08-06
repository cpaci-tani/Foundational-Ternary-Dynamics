// FTD-0648: observer-only cell-measure fixed-mass refinement scaling.

#include "ftd/eft/connected_moore_block_action.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <future>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <string>
#include <tuple>
#include <vector>

namespace {

constexpr char protocol_sha256[] =
    "9CB970060317A99B6D544C4DA05D81A6EC3F82CDD5399A149D5BD55B89A7F5BF";
constexpr std::array<int,6> widths{{2,3,4,5,6,8}};

struct Arm {
  int width = 0;
  int L = 0;
  int orientation = 0;
  int phase_axis = 0;
  std::size_t count = 0;
  int net_polarity = 0;
  bool integer_valid = false;
  bool half_valid = false;
  double worst_gauss = INFINITY;
  double mass_scale = NAN;
  double polarity_scale = NAN;
  double binding_scale = NAN;
  double beta_scale = NAN;
  double integrated_positive = NAN;
  double integrated_negative = NAN;
  double rest_energy = NAN;
  double inertial_mass = NAN;
  double unit_integer_field_energy = NAN;
  double unit_half_field_energy = NAN;
  double scaled_integer_field_energy = NAN;
  double scaled_half_field_energy = NAN;
  double scaled_barrier = NAN;
  double pinning_index = NAN;
  bool exact_pass = false;
};

using Orbit = std::pair<int,int>;

struct Summary {
  std::vector<Arm> arms;
  bool normalization = false;
  bool coverage = false;
  bool exact = false;
  bool positivity = false;
  bool monotonic = false;
  bool energy_scaling = false;
  bool barrier_scaling = false;
  bool endpoint = false;
  bool cubic = false;
  double beta0 = NAN;
  double worst_exact_relative = 0.0;
  double smallest_barrier = INFINITY;
  double worst_energy_slope = 0.0;
  double least_negative_barrier_slope = -INFINITY;
  double most_negative_barrier_slope = INFINITY;
  double worst_endpoint_ratio_deviation = 0.0;
  double worst_cubic_relative = 0.0;
  std::map<Orbit,double> energy_slopes;
  std::map<Orbit,double> barrier_slopes;
  std::map<Orbit,double> endpoint_ratios;
  std::string verdict = "CELL_MEASURE_REFINEMENT_EXECUTION_INVALID";
};

double relative(double a, double b) {
  return std::abs(a-b)/std::max({1.0, std::abs(a), std::abs(b)});
}

double log_slope(const std::vector<double>& x,
                 const std::vector<double>& y) {
  long double mx = 0.0L, my = 0.0L;
  for (std::size_t i = 0; i < x.size(); ++i) {
    mx += std::log(x[i]);
    my += std::log(y[i]);
  }
  mx /= static_cast<long double>(x.size());
  my /= static_cast<long double>(x.size());
  long double numerator = 0.0L, denominator = 0.0L;
  for (std::size_t i = 0; i < x.size(); ++i) {
    const long double dx = std::log(x[i])-mx;
    numerator += dx*(std::log(y[i])-my);
    denominator += dx*dx;
  }
  return static_cast<double>(numerator/denominator);
}

Arm run_arm(int width, int orientation, int phase_axis, double beta0) {
  Arm arm;
  arm.width = width;
  arm.L = 8*width+1;
  arm.orientation = orientation;
  arm.phase_axis = phase_axis;
  arm.mass_scale = std::pow(2.0/static_cast<double>(width), 3);
  arm.polarity_scale = arm.mass_scale;
  arm.binding_scale = arm.mass_scale;
  arm.beta_scale = 0.5*width;

  const auto integer = ftd::eft::initialize_connected_moore_block(
      arm.L, width, orientation, phase_axis, 0.0, 1e-13, 16384);
  const auto half = ftd::eft::initialize_connected_moore_block(
      arm.L, width, orientation, phase_axis, 0.5, 1e-13, 16384);
  arm.integer_valid = integer.valid;
  arm.half_valid = half.valid;
  arm.worst_gauss = std::max(integer.gauss_residual, half.gauss_residual);
  arm.count = integer.state.constituents.size();
  arm.net_polarity = std::accumulate(integer.state.charges.begin(),
      integer.state.charges.end(), 0);

  const double positive_count = 0.5*static_cast<double>(arm.count);
  arm.integrated_positive = positive_count*arm.polarity_scale;
  arm.integrated_negative = positive_count*arm.polarity_scale;
  arm.rest_energy = static_cast<double>(arm.count)*arm.mass_scale*ftd::E_REST;
  arm.inertial_mass = static_cast<double>(arm.count)*arm.mass_scale
      *ftd::M_INERTIAL;
  arm.unit_integer_field_energy = ftd::eft::matched_modified_energy(
      integer.state.electric, integer.state.magnetic_half, ftd::C_SPEED);
  arm.unit_half_field_energy = ftd::eft::matched_modified_energy(
      half.state.electric, half.state.magnetic_half, ftd::C_SPEED);
  const double field_factor = beta0*arm.beta_scale
      *arm.polarity_scale*arm.polarity_scale;
  arm.scaled_integer_field_energy =
      field_factor*arm.unit_integer_field_energy;
  arm.scaled_half_field_energy = field_factor*arm.unit_half_field_energy;
  arm.scaled_barrier = arm.scaled_integer_field_energy
      -arm.scaled_half_field_energy;
  arm.pinning_index = arm.scaled_barrier/arm.scaled_integer_field_energy;

  const std::size_t expected = static_cast<std::size_t>(2*width*width*width);
  arm.exact_pass = arm.integer_valid && arm.half_valid
      && integer.graph_connected && half.graph_connected
      && integer.graph_local && half.graph_local
      && integer.site_projection_valid && half.site_projection_valid
      && arm.worst_gauss <= 1e-11 && arm.count == expected
      && arm.net_polarity == 0
      && relative(arm.integrated_positive, 8.0) <= 1e-13
      && relative(arm.integrated_negative, 8.0) <= 1e-13
      && relative(arm.rest_energy, 16.0*ftd::E_REST) <= 1e-13
      && relative(arm.inertial_mass, 16.0*ftd::M_INERTIAL) <= 1e-13
      && arm.unit_integer_field_energy >= -1e-12
      && arm.unit_half_field_energy >= -1e-12;
  return arm;
}

const Arm* find(const Summary& summary, int width,
                int orientation, int phase_axis) {
  for (const auto& arm : summary.arms)
    if (arm.width == width && arm.orientation == orientation
        && arm.phase_axis == phase_axis) return &arm;
  return nullptr;
}

void evaluate(Summary& summary) {
  summary.coverage = summary.arms.size() == 54;
  summary.exact = summary.coverage;
  summary.positivity = summary.coverage;
  summary.monotonic = summary.coverage;
  summary.energy_scaling = summary.coverage;
  summary.barrier_scaling = summary.coverage;
  summary.endpoint = summary.coverage;
  summary.cubic = summary.coverage;

  for (const auto& arm : summary.arms) {
    summary.exact = summary.exact && arm.exact_pass;
    summary.positivity = summary.positivity
        && arm.scaled_integer_field_energy > 0.0
        && arm.scaled_half_field_energy > 0.0
        && arm.scaled_barrier > 0.0;
    summary.smallest_barrier = std::min(
        summary.smallest_barrier, arm.scaled_barrier);
    summary.worst_exact_relative = std::max({
        summary.worst_exact_relative,
        relative(arm.integrated_positive, 8.0),
        relative(arm.integrated_negative, 8.0),
        relative(arm.rest_energy, 16.0*ftd::E_REST),
        relative(arm.inertial_mass, 16.0*ftd::M_INERTIAL)});
  }

  const std::vector<double> fit_widths{4.0,5.0,6.0,8.0};
  for (int orientation = 0; orientation < 3; ++orientation)
    for (int phase_axis = 0; phase_axis < 3; ++phase_axis) {
      std::vector<double> energies, barriers;
      double previous = INFINITY;
      for (int width : widths) {
        const Arm* arm = find(summary, width, orientation, phase_axis);
        if (arm == nullptr) {
          summary.monotonic = false;
          continue;
        }
        summary.monotonic = summary.monotonic
            && arm->scaled_barrier < previous;
        previous = arm->scaled_barrier;
        if (width >= 4) {
          energies.push_back(arm->scaled_integer_field_energy);
          barriers.push_back(arm->scaled_barrier);
        }
      }
      if (energies.size() != fit_widths.size()
          || std::any_of(energies.begin(), energies.end(),
              [](double value) { return !(value > 0.0); })
          || std::any_of(barriers.begin(), barriers.end(),
              [](double value) { return !(value > 0.0); })) {
        summary.energy_scaling = false;
        summary.barrier_scaling = false;
        continue;
      }
      const Orbit orbit{orientation, phase_axis};
      const double energy_slope = log_slope(fit_widths, energies);
      const double barrier_slope = log_slope(fit_widths, barriers);
      const double endpoint_ratio = energies.back()/energies.front();
      summary.energy_slopes[orbit] = energy_slope;
      summary.barrier_slopes[orbit] = barrier_slope;
      summary.endpoint_ratios[orbit] = endpoint_ratio;
      summary.worst_energy_slope = std::max(
          summary.worst_energy_slope, std::abs(energy_slope));
      summary.least_negative_barrier_slope = std::max(
          summary.least_negative_barrier_slope, barrier_slope);
      summary.most_negative_barrier_slope = std::min(
          summary.most_negative_barrier_slope, barrier_slope);
      summary.worst_endpoint_ratio_deviation = std::max(
          summary.worst_endpoint_ratio_deviation,
          std::abs(endpoint_ratio-1.0));
      summary.energy_scaling = summary.energy_scaling
          && std::abs(energy_slope) <= 0.25;
      summary.barrier_scaling = summary.barrier_scaling
          && barrier_slope >= -3.5 && barrier_slope <= -2.5;
      summary.endpoint = summary.endpoint
          && endpoint_ratio >= 0.8 && endpoint_ratio <= 1.2;
    }

  for (int width : widths) {
    std::vector<const Arm*> parallel, perpendicular;
    for (const auto& arm : summary.arms) if (arm.width == width) {
      (arm.orientation == arm.phase_axis ? parallel : perpendicular)
          .push_back(&arm);
    }
    for (const auto& orbit : {parallel, perpendicular}) {
      if (orbit.empty()) {
        summary.cubic = false;
        continue;
      }
      const Arm* reference = orbit.front();
      for (const Arm* arm : orbit) {
        summary.worst_cubic_relative = std::max({
            summary.worst_cubic_relative,
            relative(reference->scaled_integer_field_energy,
                     arm->scaled_integer_field_energy),
            relative(reference->scaled_half_field_energy,
                     arm->scaled_half_field_energy),
            relative(reference->scaled_barrier, arm->scaled_barrier),
            relative(reference->pinning_index, arm->pinning_index)});
      }
    }
  }
  summary.cubic = summary.cubic
      && summary.worst_cubic_relative <= 1e-10;

  if (!summary.normalization || !summary.coverage || !summary.exact
      || !summary.cubic) {
    summary.verdict = "CELL_MEASURE_REFINEMENT_EXECUTION_INVALID";
  } else if (!summary.positivity || !summary.monotonic
      || !summary.energy_scaling || !summary.barrier_scaling
      || !summary.endpoint) {
    summary.verdict = "CELL_MEASURE_FIXED_MASS_SCALING_CLOSED";
  } else {
    summary.verdict = "CELL_MEASURE_FIXED_MASS_STATIC_DEPINNING_CONSTRUCTIVE";
  }
}

void write_record(const Summary& summary) {
  const auto dir = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results/ftd_0648";
  std::filesystem::create_directories(dir);
  std::ofstream json(dir / "ftd_0648_cell_measure_fixed_mass_refinement_v1.json");
  json << std::boolalpha << std::setprecision(17)
       << "{\n"
       << "  \"ftd_id\": \"FTD-0648\",\n"
       << "  \"protocol_sha256\": \"" << protocol_sha256 << "\",\n"
       << "  \"verdict\": \"" << summary.verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"arm_count\": " << summary.arms.size() << ",\n"
       << "  \"normalization_pass\": " << summary.normalization << ",\n"
       << "  \"coverage_pass\": " << summary.coverage << ",\n"
       << "  \"exact_pass\": " << summary.exact << ",\n"
       << "  \"positivity_pass\": " << summary.positivity << ",\n"
       << "  \"monotonic_pass\": " << summary.monotonic << ",\n"
       << "  \"energy_scaling_pass\": " << summary.energy_scaling << ",\n"
       << "  \"barrier_scaling_pass\": " << summary.barrier_scaling << ",\n"
       << "  \"endpoint_pass\": " << summary.endpoint << ",\n"
       << "  \"cubic_pass\": " << summary.cubic << ",\n"
       << "  \"beta0\": " << summary.beta0 << ",\n"
       << "  \"worst_exact_relative\": "
       << summary.worst_exact_relative << ",\n"
       << "  \"smallest_barrier\": " << summary.smallest_barrier << ",\n"
       << "  \"worst_energy_slope\": "
       << summary.worst_energy_slope << ",\n"
       << "  \"least_negative_barrier_slope\": "
       << summary.least_negative_barrier_slope << ",\n"
       << "  \"most_negative_barrier_slope\": "
       << summary.most_negative_barrier_slope << ",\n"
       << "  \"worst_endpoint_ratio_deviation\": "
       << summary.worst_endpoint_ratio_deviation << ",\n"
       << "  \"worst_cubic_relative\": "
       << summary.worst_cubic_relative << "\n"
       << "}\n";

  std::ofstream csv(dir / "ftd_0648_cell_measure_fixed_mass_refinement_arms_v1.csv");
  csv << "ftd_id,L,width,orientation,phase_axis,integer_valid,half_valid,"
         "worst_gauss,count,net_polarity,mass_scale,polarity_scale,"
         "binding_scale,beta_scale,integrated_positive,integrated_negative,"
         "rest_energy,inertial_mass,unit_integer_field_energy,"
         "unit_half_field_energy,scaled_integer_field_energy,"
         "scaled_half_field_energy,scaled_barrier,pinning_index,exact_pass\n";
  for (const auto& arm : summary.arms)
    csv << std::boolalpha << std::setprecision(17)
        << "FTD-0648," << arm.L << ',' << arm.width << ','
        << arm.orientation << ',' << arm.phase_axis << ','
        << arm.integer_valid << ',' << arm.half_valid << ','
        << arm.worst_gauss << ',' << arm.count << ',' << arm.net_polarity
        << ',' << arm.mass_scale << ',' << arm.polarity_scale << ','
        << arm.binding_scale << ',' << arm.beta_scale << ','
        << arm.integrated_positive << ',' << arm.integrated_negative << ','
        << arm.rest_energy << ',' << arm.inertial_mass << ','
        << arm.unit_integer_field_energy << ','
        << arm.unit_half_field_energy << ','
        << arm.scaled_integer_field_energy << ','
        << arm.scaled_half_field_energy << ',' << arm.scaled_barrier << ','
        << arm.pinning_index << ',' << arm.exact_pass << '\n';

  std::ofstream slopes(dir / "ftd_0648_cell_measure_fixed_mass_refinement_slopes_v1.csv");
  slopes << "ftd_id,orientation,phase_axis,energy_slope,barrier_slope,"
           "width8_width4_energy_ratio\n";
  for (const auto& [orbit, energy_slope] : summary.energy_slopes)
    slopes << std::setprecision(17) << "FTD-0648," << orbit.first << ','
           << orbit.second << ',' << energy_slope << ','
           << summary.barrier_slopes.at(orbit) << ','
           << summary.endpoint_ratios.at(orbit) << '\n';
}

}  // namespace

int main() {
  Summary summary;
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  summary.normalization = normalization.valid
      && normalization.mapped_field_work_coefficient > 0.0;
  summary.beta0 = normalization.mapped_field_work_coefficient;
  if (summary.normalization) {
    struct Spec { int width; int orientation; int phase_axis; };
    std::vector<Spec> specs;
    for (int width : widths)
      for (int orientation = 0; orientation < 3; ++orientation)
        for (int phase_axis = 0; phase_axis < 3; ++phase_axis)
          specs.push_back({width,orientation,phase_axis});
    constexpr std::size_t batch = 6;
    for (std::size_t start = 0; start < specs.size(); start += batch) {
      const std::size_t end = std::min(start+batch, specs.size());
      std::vector<std::future<Arm>> futures;
      for (std::size_t i = start; i < end; ++i) {
        const Spec spec = specs[i];
        futures.push_back(std::async(std::launch::async, [=]() {
          return run_arm(spec.width, spec.orientation, spec.phase_axis,
                         summary.beta0);
        }));
      }
      for (auto& future : futures) summary.arms.push_back(future.get());
    }
  }
  evaluate(summary);
  write_record(summary);
  std::cout << std::boolalpha << std::setprecision(17)
            << "protocol_sha256=" << protocol_sha256 << '\n'
            << "verdict=" << summary.verdict << '\n'
            << "coverage=" << summary.coverage
            << " exact=" << summary.exact
            << " positivity=" << summary.positivity
            << " monotonic=" << summary.monotonic
            << " energy_scaling=" << summary.energy_scaling
            << " barrier_scaling=" << summary.barrier_scaling
            << " endpoint=" << summary.endpoint
            << " cubic=" << summary.cubic << '\n'
            << "energy_slope_abs=" << summary.worst_energy_slope
            << " barrier_slopes=" << summary.most_negative_barrier_slope
            << ".." << summary.least_negative_barrier_slope
            << " endpoint_deviation="
            << summary.worst_endpoint_ratio_deviation
            << " cubic_residual=" << summary.worst_cubic_relative << '\n';
  return summary.verdict ==
      "CELL_MEASURE_REFINEMENT_EXECUTION_INVALID" ? 1 : 0;
}
