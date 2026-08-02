// FTD-0622: runtime-size connected Moore-block common action.

#include "ftd/eft/connected_moore_block_action.h"
#include "ftd/eft/ternary_block_bipole_peierls.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace {

using ftd::Vec3;

constexpr char protocol_sha256_0622[] =
    "7E09ADBC2A16513DD3495BB117015F574E150F7B8BA5632C03BC96783AFE00AF";
constexpr char parent_sha256_0622[] =
    "D6ED6A0BF3C9B351ED59E4B16C0FD82430A4713B4ED06B0092F9BDCBB4026383";
constexpr int L = 17;
constexpr double aggregate_gate = 1e-10;
constexpr double inverse_gate = 1e-8;

struct ArmSpec {
  int width = 0;
  int orientation = 0;
  int phase_axis = 0;
  double phase = 0.0;
  std::string label;
};

struct ArmResult {
  ArmSpec spec{};
  bool initialization_pass = false;
  bool forward_pass = false;
  bool reverse_pass = false;
  bool count_pass = false;
  bool no_hop_pass = false;
  bool rest_pass = true;
  std::size_t constituent_count = 0;
  std::size_t edge_count = 0;
  int forward_iterations = 0;
  int reverse_iterations = 0;
  double recovery = INFINITY;
  double peierls_index = INFINITY;
  double normalized_spline_defect = INFINITY;
  double local_defect = INFINITY;
  double spline_defect = INFINITY;
  double field_energy = INFINITY;
  double center_displacement = INFINITY;
  double maximum_edge_strain = INFINITY;
  double maximum_common_residual = INFINITY;
  Vec3 matter_delta{};
  Vec3 local_total_defect{};
  Vec3 spline_total_defect{};
  Vec3 center_delta{};
};

Vec3 cycle(const Vec3& value) {
  return {value.z,value.x,value.y};
}

double maximum_component(const Vec3& value) {
  return std::max({std::abs(value.x),std::abs(value.y),std::abs(value.z)});
}

double relative_difference(double lhs, double rhs) {
  return std::abs(lhs-rhs)/std::max({1e-300,std::abs(lhs),std::abs(rhs)});
}

std::size_t expected_edges(int width) {
  if (width == 1) return 1;
  if (width == 2) return 72;
  if (width == 3) return 365;
  return 0;
}

double maximum_residual(
    const ftd::eft::ConnectedMooreBlockStepResult& step) {
  return std::max({step.root_residual,step.continuity_residual,
      step.gauss_before_residual,step.gauss_after_residual,
      step.force_residual,step.kinematic_residual,
      step.kinetic_discrete_gradient_residual,
      step.electric_adjoint_residual,step.magnetic_work_residual,
      step.binding_work_residual,step.binding_impulse_sum_residual,
      step.matter_work_residual,step.field_work_residual,
      step.total_energy_residual,step.causal_speed_excess});
}

ArmResult run_arm(const ArmSpec& spec,
                  const ftd::eft::ConnectedMooreBlockOptions& options,
                  double beta) {
  ArmResult result;
  result.spec = spec;
  const auto initialization = ftd::eft::initialize_connected_moore_block(
      L,spec.width,spec.orientation,spec.phase_axis,spec.phase);
  result.initialization_pass = initialization.valid
      && initialization.poisson_residual <= 1e-11
      && initialization.gauss_residual <= 1e-11
      && initialization.curl_adjoint_residual <= 1e-11;
  if (!result.initialization_pass) return result;
  result.constituent_count = initialization.state.constituents.size();
  result.edge_count = initialization.state.edges.size();
  result.count_pass = result.constituent_count
          == static_cast<std::size_t>(2*spec.width*spec.width*spec.width)
      && result.edge_count == expected_edges(spec.width);
  const auto spectrum = ftd::eft::evaluate_ternary_block_bipole_peierls(
      L,spec.width,spec.orientation,beta);
  if (spectrum.valid)
    result.peierls_index = spectrum.pinning_index[spec.phase_axis];

  const auto forward = ftd::eft::solve_connected_moore_block_forward(
      initialization.state,options);
  result.forward_iterations = forward.solve.iterations;
  result.forward_pass = forward.common_action_gates_pass;
  if (!result.forward_pass) {
    result.maximum_common_residual = maximum_residual(forward);
    return result;
  }
  result.no_hop_pass = forward.site_hops == 0;
  result.normalized_spline_defect = forward.normalized_spline_defect;
  result.local_defect = forward.local_defect_norm;
  result.spline_defect = forward.spline_defect_norm;
  result.field_energy = forward.field_energy_before;
  result.center_displacement = forward.center_displacement;
  result.maximum_edge_strain = forward.maximum_edge_strain;
  result.maximum_common_residual = maximum_residual(forward);
  result.matter_delta = forward.matter_momentum_after
      -forward.matter_momentum_before;
  result.local_total_defect = forward.local_total_defect;
  result.spline_total_defect = forward.spline_total_defect;
  result.center_delta = forward.center_after-forward.center_before;
  result.rest_pass = spec.phase != 0.0
      || (result.matter_delta.mag() <= 1e-8
          && result.center_displacement <= 1e-8);

  const auto reverse = ftd::eft::solve_connected_moore_block_reverse(
      forward.later,options);
  result.reverse_iterations = reverse.solve.iterations;
  if (reverse.common_action_gates_pass)
    result.recovery = ftd::eft::connected_moore_block_state_max_difference(
        initialization.state,reverse.earlier);
  result.reverse_pass = reverse.common_action_gates_pass
      && result.recovery <= inverse_gate;
  result.maximum_common_residual = std::max(
      result.maximum_common_residual,maximum_residual(reverse));
  return result;
}

struct Summary {
  bool parent_pass = false;
  bool coverage_pass = false;
  bool small_width_pass = false;
  bool all_width_pass = false;
  bool covariance_pass = false;
  bool rest_pass = false;
  bool peierls_trend_pass = false;
  bool defect_trend_pass = false;
  double beta = 0.0;
  double worst_covariance_residual = INFINITY;
  double worst_common_residual = INFINITY;
  double worst_recovery = INFINITY;
  std::vector<ArmResult> arms;
  std::string verdict;
};

const ArmResult* find_arm(const Summary& summary, int width,
                          int orientation, int phase_axis,
                          double phase) {
  const auto found = std::find_if(summary.arms.begin(),summary.arms.end(),
      [&](const ArmResult& arm) {
        return arm.spec.width == width
            && arm.spec.orientation == orientation
            && arm.spec.phase_axis == phase_axis
            && arm.spec.phase == phase;
      });
  return found == summary.arms.end() ? nullptr : &*found;
}

bool complete(const ArmResult& arm) {
  return arm.initialization_pass && arm.forward_pass && arm.reverse_pass
      && arm.count_pass && arm.no_hop_pass && arm.rest_pass
      && std::isfinite(arm.peierls_index)
      && arm.maximum_common_residual <= aggregate_gate;
}

void evaluate_summary(Summary& summary) {
  summary.coverage_pass = summary.arms.size() == 13;
  summary.small_width_pass = summary.coverage_pass;
  summary.all_width_pass = summary.coverage_pass;
  summary.rest_pass = summary.coverage_pass;
  summary.worst_common_residual = 0.0;
  summary.worst_recovery = 0.0;
  for (const auto& arm : summary.arms) {
    if (arm.spec.width <= 2)
      summary.small_width_pass = summary.small_width_pass && complete(arm);
    summary.all_width_pass = summary.all_width_pass && complete(arm);
    if (arm.spec.phase == 0.0)
      summary.rest_pass = summary.rest_pass && arm.rest_pass;
    if (std::isfinite(arm.maximum_common_residual))
      summary.worst_common_residual = std::max(
          summary.worst_common_residual,arm.maximum_common_residual);
    if (std::isfinite(arm.recovery))
      summary.worst_recovery = std::max(summary.worst_recovery,arm.recovery);
  }

  summary.worst_covariance_residual = 0.0;
  summary.covariance_pass = summary.all_width_pass;
  for (int phase_class = 0; phase_class < 2; ++phase_class) {
    const int base_phase_axis = phase_class == 0 ? 0 : 1;
    const auto* base = find_arm(summary,2,0,base_phase_axis,0.25);
    if (base == nullptr) {
      summary.covariance_pass = false;
      continue;
    }
    for (int turns = 1; turns <= 2; ++turns) {
      const int orientation = turns;
      const int phase_axis = phase_class == 0
          ? turns : (turns+1)%3;
      const auto* rotated = find_arm(
          summary,2,orientation,phase_axis,0.25);
      if (rotated == nullptr) {
        summary.covariance_pass = false;
        continue;
      }
      Vec3 matter = base->matter_delta;
      Vec3 local = base->local_total_defect;
      Vec3 spline = base->spline_total_defect;
      Vec3 center = base->center_delta;
      for (int turn = 0; turn < turns; ++turn) {
        matter = cycle(matter);
        local = cycle(local);
        spline = cycle(spline);
        center = cycle(center);
      }
      summary.worst_covariance_residual = std::max({
          summary.worst_covariance_residual,
          maximum_component(rotated->matter_delta-matter),
          maximum_component(rotated->local_total_defect-local),
          maximum_component(rotated->spline_total_defect-spline),
          maximum_component(rotated->center_delta-center),
          relative_difference(rotated->field_energy,base->field_energy),
          relative_difference(rotated->normalized_spline_defect,
                              base->normalized_spline_defect),
          relative_difference(rotated->maximum_edge_strain,
                              base->maximum_edge_strain)});
    }
  }
  summary.covariance_pass = summary.covariance_pass
      && summary.worst_covariance_residual <= 1e-8;

  summary.peierls_trend_pass = summary.all_width_pass;
  summary.defect_trend_pass = summary.all_width_pass;
  for (int phase_axis : {0,1}) {
    double previous_pi = INFINITY;
    double previous_defect = INFINITY;
    for (int width : {1,2,3}) {
      const auto* arm = find_arm(summary,width,0,phase_axis,0.25);
      if (arm == nullptr) {
        summary.peierls_trend_pass = false;
        summary.defect_trend_pass = false;
        continue;
      }
      summary.peierls_trend_pass = summary.peierls_trend_pass
          && arm->peierls_index < previous_pi;
      summary.defect_trend_pass = summary.defect_trend_pass
          && arm->normalized_spline_defect < previous_defect;
      previous_pi = arm->peierls_index;
      previous_defect = arm->normalized_spline_defect;
    }
  }

  if (!summary.parent_pass || !summary.small_width_pass)
    summary.verdict = "CONNECTED_MOORE_BLOCK_COMMON_ACTION_INVALID";
  else if (!summary.all_width_pass)
    summary.verdict = "CONNECTED_ACTION_SCALING_SOLVER_UNRESOLVED";
  else if (summary.covariance_pass && summary.rest_pass
           && summary.peierls_trend_pass && summary.defect_trend_pass)
    summary.verdict =
        "CONNECTED_MOORE_BLOCK_ACTION_CONSTRUCTIVE_IR_TREND_POSITIVE";
  else
    summary.verdict =
        "CONNECTED_MOORE_BLOCK_ACTION_CONSTRUCTIVE_IR_TREND_NEGATIVE";
}

bool parent_fingerprint() {
  const auto path = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results" / "ftd_0621"
      / "ftd_0621_ternary_block_bipole_peierls_v1.json";
  std::ifstream stream(path,std::ios::binary);
  const std::string bytes((std::istreambuf_iterator<char>(stream)),
                          std::istreambuf_iterator<char>());
  return bytes.find("\"ftd_id\": \"FTD-0621\"") != std::string::npos
      && bytes.find("INTEGER_TERNARY_EXTENSION_SUPPRESSES_PEIERLS")
          != std::string::npos;
}

void write_record(const Summary& summary) {
  const auto dir = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results" / "ftd_0622";
  std::filesystem::create_directories(dir);
  std::ofstream json(dir / "ftd_0622_connected_moore_block_action_v1.json");
  json << std::setprecision(17) << "{\n"
       << "  \"ftd_id\": \"FTD-0622\",\n"
       << "  \"protocol_sha256\": \"" << protocol_sha256_0622 << "\",\n"
       << "  \"parent_result_sha256\": \"" << parent_sha256_0622 << "\",\n"
       << "  \"verdict\": \"" << summary.verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"parent_pass\": " << summary.parent_pass << ",\n"
       << "  \"coverage_pass\": " << summary.coverage_pass << ",\n"
       << "  \"small_width_pass\": " << summary.small_width_pass << ",\n"
       << "  \"all_width_pass\": " << summary.all_width_pass << ",\n"
       << "  \"covariance_pass\": " << summary.covariance_pass << ",\n"
       << "  \"rest_pass\": " << summary.rest_pass << ",\n"
       << "  \"peierls_trend_pass\": " << summary.peierls_trend_pass << ",\n"
       << "  \"defect_trend_pass\": " << summary.defect_trend_pass << ",\n"
       << "  \"beta\": " << summary.beta << ",\n"
       << "  \"worst_covariance_residual\": "
       << summary.worst_covariance_residual << ",\n"
       << "  \"worst_common_residual\": "
       << summary.worst_common_residual << ",\n"
       << "  \"worst_recovery\": " << summary.worst_recovery << "\n"
       << "}\n";

  std::ofstream csv(dir / "ftd_0622_connected_moore_block_action_v1.csv");
  csv << "ftd_id,label,width,orientation,phase_axis,phase,init,forward,"
         "reverse,count,no_hop,rest,constituents,edges,forward_iterations,"
         "reverse_iterations,recovery,peierls_index,normalized_spline_defect,"
         "local_defect,spline_defect,field_energy,center_displacement,"
         "maximum_edge_strain,maximum_common_residual,matter_x,matter_y,"
         "matter_z,local_x,local_y,local_z,spline_x,spline_y,spline_z,"
         "center_x,center_y,center_z\n";
  for (const auto& arm : summary.arms)
    csv << std::setprecision(17) << "FTD-0622," << arm.spec.label << ','
        << arm.spec.width << ',' << arm.spec.orientation << ','
        << arm.spec.phase_axis << ',' << arm.spec.phase << ','
        << arm.initialization_pass << ',' << arm.forward_pass << ','
        << arm.reverse_pass << ',' << arm.count_pass << ','
        << arm.no_hop_pass << ',' << arm.rest_pass << ','
        << arm.constituent_count << ',' << arm.edge_count << ','
        << arm.forward_iterations << ',' << arm.reverse_iterations << ','
        << arm.recovery << ',' << arm.peierls_index << ','
        << arm.normalized_spline_defect << ',' << arm.local_defect << ','
        << arm.spline_defect << ',' << arm.field_energy << ','
        << arm.center_displacement << ',' << arm.maximum_edge_strain << ','
        << arm.maximum_common_residual << ',' << arm.matter_delta.x << ','
        << arm.matter_delta.y << ',' << arm.matter_delta.z << ','
        << arm.local_total_defect.x << ',' << arm.local_total_defect.y << ','
        << arm.local_total_defect.z << ',' << arm.spline_total_defect.x << ','
        << arm.spline_total_defect.y << ',' << arm.spline_total_defect.z << ','
        << arm.center_delta.x << ',' << arm.center_delta.y << ','
        << arm.center_delta.z << '\n';
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  Summary summary;
  summary.parent_pass = parent_fingerprint();
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  summary.beta = normalization.mapped_field_work_coefficient;
  ftd::eft::ConnectedMooreBlockOptions options;
  options.gate_tolerance = aggregate_gate;
  options.solve_tolerance = 2e-11;
  options.max_iterations = 48;

  std::vector<ArmSpec> specs;
  for (int width : {1,2,3}) {
    specs.push_back({width,0,0,0.0,"rest_w"+std::to_string(width)});
    specs.push_back({width,0,0,0.25,"parallel_w"+std::to_string(width)});
    specs.push_back({width,0,1,0.25,"transverse_w"+std::to_string(width)});
  }
  specs.push_back({2,1,1,0.25,"parallel_w2_rot1"});
  specs.push_back({2,1,2,0.25,"transverse_w2_rot1"});
  specs.push_back({2,2,2,0.25,"parallel_w2_rot2"});
  specs.push_back({2,2,0,0.25,"transverse_w2_rot2"});

  if (summary.parent_pass && normalization.valid)
    for (const auto& spec : specs) {
      std::cout << "running " << spec.label << std::endl;
      summary.arms.push_back(run_arm(spec,options,summary.beta));
    }
  evaluate_summary(summary);
  write_record(summary);

  std::cout << "protocol_sha256=" << protocol_sha256_0622 << '\n'
            << "verdict=" << summary.verdict << '\n'
            << "arms=" << summary.arms.size()
            << " small=" << summary.small_width_pass
            << " all=" << summary.all_width_pass
            << " covariance=" << summary.covariance_pass
            << " rest=" << summary.rest_pass
            << " pi_trend=" << summary.peierls_trend_pass
            << " defect_trend=" << summary.defect_trend_pass << '\n'
            << "worst_common=" << summary.worst_common_residual
            << " recovery=" << summary.worst_recovery
            << " covariance_residual="
            << summary.worst_covariance_residual << '\n';
  for (const auto& arm : summary.arms)
    std::cout << arm.spec.label << " n=" << arm.constituent_count
              << " e=" << arm.edge_count << " iter="
              << arm.forward_iterations << '/' << arm.reverse_iterations
              << " pass=" << complete(arm)
              << " pi=" << arm.peierls_index
              << " D=" << arm.normalized_spline_defect
              << " dx=" << arm.center_displacement
              << " strain=" << arm.maximum_edge_strain
              << " rec=" << arm.recovery << '\n';
  return summary.parent_pass && summary.coverage_pass
      && summary.small_width_pass ? 0 : 1;
}
