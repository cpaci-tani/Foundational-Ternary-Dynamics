// FTD-0647: frozen-coefficient fixed-mass refinement obstruction.

#include "ftd/eft/connected_moore_block_action.h"
#include "ftd/eft/production_hop_kinematics.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

namespace {

constexpr char protocol_sha256[] =
    "5D3A8E64750936A1A437C4F743777297977AA0E6BEBAC241F8FF46BD647706D9";
constexpr int lattice_size = 17;

struct Arm {
  int width = 0;
  int orientation = 0;
  std::size_t expected_count = 0;
  std::size_t constituent_count = 0;
  int net_polarity = 0;
  bool initialized = false;
  bool graph_connected = false;
  bool graph_local = false;
  bool site_projection = false;
  double gauss_residual = INFINITY;
  double constituent_rest_sum = NAN;
  double rest_floor = NAN;
  double binding_energy = NAN;
  double modified_field_energy = NAN;
  double beta = NAN;
  double total_energy = NAN;
  double inertial_mass_floor = NAN;
  double rest_density = NAN;
  double inertial_mass_density = NAN;
  bool gates_pass = false;
};

struct Summary {
  std::vector<Arm> arms;
  bool normalization = false;
  bool coverage = false;
  bool initialization = false;
  bool count = false;
  bool neutrality = false;
  bool rest_sum = false;
  bool binding = false;
  bool field_positivity = false;
  bool lower_bound = false;
  bool cubic = false;
  bool scaling = false;
  double beta = NAN;
  double worst_rest_relative = 0.0;
  double worst_negative_field = 0.0;
  double worst_lower_bound_defect = 0.0;
  double worst_cubic_relative = 0.0;
  double worst_rest_density_relative = 0.0;
  double worst_mass_density_relative = 0.0;
  std::string verdict = "FIXED_MASS_OBSTRUCTION_EXECUTION_INVALID";
};

double relative(double a, double b) {
  return std::abs(a-b)/std::max({1.0, std::abs(a), std::abs(b)});
}

Arm run_arm(int width, int orientation, double beta) {
  Arm arm;
  arm.width = width;
  arm.orientation = orientation;
  arm.expected_count = static_cast<std::size_t>(2*width*width*width);
  arm.beta = beta;

  const auto initialized = ftd::eft::initialize_connected_moore_block(
      lattice_size, width, orientation, 0, 0.0, 1e-13, 16384);
  arm.initialized = initialized.valid;
  arm.graph_connected = initialized.graph_connected;
  arm.graph_local = initialized.graph_local;
  arm.site_projection = initialized.site_projection_valid;
  arm.gauss_residual = initialized.gauss_residual;
  arm.constituent_count = initialized.state.constituents.size();
  arm.net_polarity = std::accumulate(initialized.state.charges.begin(),
      initialized.state.charges.end(), 0);

  long double rest_sum = 0.0L;
  for (const auto& point : initialized.state.constituents)
    rest_sum += ftd::eft::production_flat_energy_from_momentum(point.momentum);
  arm.constituent_rest_sum = static_cast<double>(rest_sum);
  arm.rest_floor = static_cast<double>(arm.constituent_count)*ftd::E_REST;

  ftd::eft::ConnectedMooreBlockOptions options;
  arm.binding_energy = ftd::eft::connected_moore_block_binding_energy(
      initialized.state, options);
  arm.modified_field_energy = ftd::eft::matched_modified_energy(
      initialized.state.electric, initialized.state.magnetic_half,
      ftd::C_SPEED);
  arm.total_energy = arm.constituent_rest_sum + arm.binding_energy
      + beta*arm.modified_field_energy;
  arm.inertial_mass_floor = arm.rest_floor/(ftd::C_SPEED*ftd::C_SPEED);
  const double w3 = static_cast<double>(width*width*width);
  arm.rest_density = arm.rest_floor/w3;
  arm.inertial_mass_density = arm.inertial_mass_floor/w3;

  arm.gates_pass = arm.initialized && arm.graph_connected && arm.graph_local
      && arm.site_projection && arm.gauss_residual <= 1e-11
      && arm.constituent_count == arm.expected_count
      && arm.net_polarity == 0
      && relative(arm.constituent_rest_sum, arm.rest_floor) <= 1e-14
      && arm.binding_energy >= -1e-14
      && arm.binding_energy <= 1e-14
      && arm.modified_field_energy >= -1e-12
      && arm.total_energy+1e-12 >= arm.rest_floor
      && relative(arm.rest_density, 2.0*ftd::E_REST) <= 1e-14
      && relative(arm.inertial_mass_density,
                  2.0*ftd::M_INERTIAL) <= 1e-14;
  return arm;
}

void evaluate(Summary& summary) {
  summary.coverage = summary.arms.size() == 12;
  summary.initialization = summary.coverage;
  summary.count = summary.coverage;
  summary.neutrality = summary.coverage;
  summary.rest_sum = summary.coverage;
  summary.binding = summary.coverage;
  summary.field_positivity = summary.coverage;
  summary.lower_bound = summary.coverage;
  summary.scaling = summary.coverage;

  for (const auto& arm : summary.arms) {
    summary.initialization = summary.initialization && arm.initialized
        && arm.graph_connected && arm.graph_local && arm.site_projection
        && arm.gauss_residual <= 1e-11;
    summary.count = summary.count
        && arm.constituent_count == arm.expected_count;
    summary.neutrality = summary.neutrality && arm.net_polarity == 0;
    summary.worst_rest_relative = std::max(summary.worst_rest_relative,
        relative(arm.constituent_rest_sum, arm.rest_floor));
    summary.rest_sum = summary.rest_sum
        && relative(arm.constituent_rest_sum, arm.rest_floor) <= 1e-14;
    summary.binding = summary.binding && arm.binding_energy >= -1e-14
        && arm.binding_energy <= 1e-14;
    summary.worst_negative_field = std::max(summary.worst_negative_field,
        std::max(0.0, -arm.modified_field_energy));
    summary.field_positivity = summary.field_positivity
        && arm.modified_field_energy >= -1e-12;
    summary.worst_lower_bound_defect = std::max(
        summary.worst_lower_bound_defect,
        std::max(0.0, arm.rest_floor-arm.total_energy));
    summary.lower_bound = summary.lower_bound
        && arm.total_energy+1e-12 >= arm.rest_floor;
    summary.worst_rest_density_relative = std::max(
        summary.worst_rest_density_relative,
        relative(arm.rest_density, 2.0*ftd::E_REST));
    summary.worst_mass_density_relative = std::max(
        summary.worst_mass_density_relative,
        relative(arm.inertial_mass_density, 2.0*ftd::M_INERTIAL));
    summary.scaling = summary.scaling
        && relative(arm.rest_density, 2.0*ftd::E_REST) <= 1e-14
        && relative(arm.inertial_mass_density,
                    2.0*ftd::M_INERTIAL) <= 1e-14;
  }

  summary.cubic = summary.coverage;
  for (int width = 1; width <= 4; ++width) {
    std::array<const Arm*,3> copies{{nullptr,nullptr,nullptr}};
    for (const auto& arm : summary.arms)
      if (arm.width == width) copies[arm.orientation] = &arm;
    if (std::any_of(copies.begin(), copies.end(),
                    [](const Arm* value) { return value == nullptr; })) {
      summary.cubic = false;
      continue;
    }
    for (int orientation = 1; orientation < 3; ++orientation) {
      summary.worst_cubic_relative = std::max({
          summary.worst_cubic_relative,
          relative(copies[0]->constituent_rest_sum,
                   copies[orientation]->constituent_rest_sum),
          relative(copies[0]->binding_energy,
                   copies[orientation]->binding_energy),
          relative(copies[0]->modified_field_energy,
                   copies[orientation]->modified_field_energy),
          relative(copies[0]->total_energy,
                   copies[orientation]->total_energy)});
    }
  }
  summary.cubic = summary.cubic
      && summary.worst_cubic_relative <= 1e-10;

  if (!summary.normalization || !summary.coverage
      || !summary.initialization || !summary.count || !summary.neutrality
      || !summary.rest_sum || !summary.binding || !summary.lower_bound
      || !summary.cubic || !summary.scaling) {
    summary.verdict = "FIXED_MASS_OBSTRUCTION_EXECUTION_INVALID";
  } else if (!summary.field_positivity) {
    summary.verdict = "MODIFIED_ENERGY_POSITIVITY_ASSUMPTION_FALSIFIED";
  } else {
    summary.verdict =
        "FROZEN_ADDITIVE_CONSTITUENT_FIXED_MASS_REFINEMENT_CLOSED";
  }
}

void write_record(const Summary& summary) {
  const auto dir = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results/ftd_0647";
  std::filesystem::create_directories(dir);

  std::ofstream json(dir /
      "ftd_0647_connected_block_fixed_mass_refinement_obstruction_v1.json");
  json << std::boolalpha << std::setprecision(17)
       << "{\n"
       << "  \"ftd_id\": \"FTD-0647\",\n"
       << "  \"protocol_sha256\": \"" << protocol_sha256 << "\",\n"
       << "  \"verdict\": \"" << summary.verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"arm_count\": " << summary.arms.size() << ",\n"
       << "  \"normalization_pass\": " << summary.normalization << ",\n"
       << "  \"coverage_pass\": " << summary.coverage << ",\n"
       << "  \"initialization_pass\": " << summary.initialization << ",\n"
       << "  \"count_pass\": " << summary.count << ",\n"
       << "  \"neutrality_pass\": " << summary.neutrality << ",\n"
       << "  \"rest_sum_pass\": " << summary.rest_sum << ",\n"
       << "  \"binding_pass\": " << summary.binding << ",\n"
       << "  \"field_positivity_pass\": "
       << summary.field_positivity << ",\n"
       << "  \"lower_bound_pass\": " << summary.lower_bound << ",\n"
       << "  \"cubic_pass\": " << summary.cubic << ",\n"
       << "  \"scaling_pass\": " << summary.scaling << ",\n"
       << "  \"beta\": " << summary.beta << ",\n"
       << "  \"rest_energy_per_width_cubed\": "
       << 2.0*ftd::E_REST << ",\n"
       << "  \"inertial_mass_per_width_cubed\": "
       << 2.0*ftd::M_INERTIAL << ",\n"
       << "  \"worst_rest_relative\": "
       << summary.worst_rest_relative << ",\n"
       << "  \"worst_negative_field\": "
       << summary.worst_negative_field << ",\n"
       << "  \"worst_lower_bound_defect\": "
       << summary.worst_lower_bound_defect << ",\n"
       << "  \"worst_cubic_relative\": "
       << summary.worst_cubic_relative << ",\n"
       << "  \"worst_rest_density_relative\": "
       << summary.worst_rest_density_relative << ",\n"
       << "  \"worst_mass_density_relative\": "
       << summary.worst_mass_density_relative << "\n"
       << "}\n";

  std::ofstream csv(dir /
      "ftd_0647_connected_block_fixed_mass_refinement_obstruction_arms_v1.csv");
  csv << "ftd_id,L,width,orientation,initialized,graph_connected,graph_local,"
         "site_projection,gauss_residual,expected_count,constituent_count,"
         "net_polarity,constituent_rest_sum,rest_floor,binding_energy,"
         "modified_field_energy,beta,total_energy,inertial_mass_floor,"
         "rest_energy_per_width_cubed,inertial_mass_per_width_cubed,"
         "gates_pass\n";
  for (const auto& arm : summary.arms) {
    csv << std::boolalpha << std::setprecision(17)
        << "FTD-0647," << lattice_size << ',' << arm.width << ','
        << arm.orientation << ',' << arm.initialized << ','
        << arm.graph_connected << ',' << arm.graph_local << ','
        << arm.site_projection << ',' << arm.gauss_residual << ','
        << arm.expected_count << ',' << arm.constituent_count << ','
        << arm.net_polarity << ',' << arm.constituent_rest_sum << ','
        << arm.rest_floor << ',' << arm.binding_energy << ','
        << arm.modified_field_energy << ',' << arm.beta << ','
        << arm.total_energy << ',' << arm.inertial_mass_floor << ','
        << arm.rest_density << ',' << arm.inertial_mass_density << ','
        << arm.gates_pass << '\n';
  }
}

}  // namespace

int main() {
  Summary summary;
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  summary.normalization = normalization.valid
      && normalization.mapped_field_work_coefficient > 0.0;
  summary.beta = normalization.mapped_field_work_coefficient;
  if (summary.normalization) {
    for (int width = 1; width <= 4; ++width)
      for (int orientation = 0; orientation < 3; ++orientation)
        summary.arms.push_back(run_arm(width, orientation, summary.beta));
  }
  evaluate(summary);
  write_record(summary);

  std::cout << std::boolalpha << std::setprecision(17)
            << "protocol_sha256=" << protocol_sha256 << '\n'
            << "verdict=" << summary.verdict << '\n'
            << "coverage=" << summary.coverage
            << " initialization=" << summary.initialization
            << " count=" << summary.count
            << " neutrality=" << summary.neutrality
            << " rest_sum=" << summary.rest_sum
            << " binding=" << summary.binding
            << " field_positivity=" << summary.field_positivity
            << " lower_bound=" << summary.lower_bound
            << " cubic=" << summary.cubic
            << " scaling=" << summary.scaling << '\n'
            << "rest_per_w3=" << 2.0*ftd::E_REST
            << " inertial_mass_per_w3=" << 2.0*ftd::M_INERTIAL
            << " cubic_residual=" << summary.worst_cubic_relative << '\n';

  return summary.verdict ==
      "FROZEN_ADDITIVE_CONSTITUENT_FIXED_MASS_REFINEMENT_CLOSED" ? 0 : 1;
}
