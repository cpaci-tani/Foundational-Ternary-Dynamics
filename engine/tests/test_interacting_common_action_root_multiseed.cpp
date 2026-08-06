// FTD-0720: deterministic multiseed probe of the interacting common-action root.

#include "ftd/eft/connected_moore_block_action.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace {

using ftd::Vec3;
using ftd::eft::ConnectedMooreBlockState;
using ftd::eft::ConnectedMooreBlockStepResult;

constexpr char protocol_sha256[] =
    "DB516877F7762BECF9E61AB54861A616F84DCA7E6701907A8CE1F3D4BA21668C";
constexpr int L = 17;
constexpr double seed_magnitude = 0.2;
constexpr double root_comparison_gate = 1e-9;
constexpr double current_comparison_gate = 1e-9;
constexpr double inverse_gate = 1e-8;

struct ArmSpec {
  std::string label;
  int phase_axis = 0;
  double phase = 0.0;
};

struct SeedSpec {
  std::string label;
  int axis = -1;
  int sign = 0;
  bool charge_odd = false;
};

struct SeedResult {
  std::string arm;
  std::string seed;
  bool accepted = false;
  bool inverse = false;
  int iterations = 0;
  double root_residual = INFINITY;
  double state_difference = INFINITY;
  double current_difference = INFINITY;
  double recovery = INFINITY;
};

struct ArmResult {
  ArmSpec spec;
  bool initialized = false;
  bool canonical = false;
  bool relabeling = false;
  bool all_seeds = false;
  bool multiple_roots = false;
  double maximum_state_difference = 0.0;
  double maximum_current_difference = 0.0;
  double maximum_recovery = 0.0;
  std::vector<SeedResult> seeds;
};

Vec3 axis_vector(int axis, double value) {
  return axis == 0 ? Vec3{value,0.0,0.0}
      : (axis == 1 ? Vec3{0.0,value,0.0}
                   : Vec3{0.0,0.0,value});
}

std::vector<SeedSpec> seed_specs() {
  std::vector<SeedSpec> result{{"incoming",-1,0,false}};
  for (bool odd : {false,true})
    for (int axis = 0; axis < 3; ++axis)
      for (int sign : {-1,+1})
        result.push_back({std::string(odd ? "odd_" : "common_")
              +static_cast<char>('x'+axis)+(sign < 0 ? "_minus" : "_plus"),
            axis,sign,odd});
  return result;
}

std::vector<Vec3> make_seed(const ConnectedMooreBlockState& state,
                            const SeedSpec& spec) {
  std::vector<Vec3> result;
  result.reserve(state.constituents.size());
  if (spec.axis < 0) {
    for (const auto& point : state.constituents)
      result.push_back(point.momentum);
    return result;
  }
  const Vec3 common = axis_vector(
      spec.axis, spec.sign*seed_magnitude);
  for (std::size_t a = 0; a < state.constituents.size(); ++a)
    result.push_back(spec.charge_odd ? common*state.charges[a] : common);
  return result;
}

ftd::eft::MatchedFaceFlux aggregate_current(
    const ConnectedMooreBlockStepResult& step) {
  ftd::eft::MatchedFaceFlux result(step.earlier.electric.L);
  for (const auto& segment : step.segments) {
    if (!segment.dense_materialized) continue;
    for (std::size_t i = 0; i < result.x.size(); ++i) {
      result.x[i] += segment.current_x[i];
      result.y[i] += segment.current_y[i];
      result.z[i] += segment.current_z[i];
    }
  }
  return result;
}

double current_difference(const ConnectedMooreBlockStepResult& lhs,
                          const ConnectedMooreBlockStepResult& rhs) {
  return ftd::eft::matched_face_max_difference(
      aggregate_current(lhs),aggregate_current(rhs));
}

ConnectedMooreBlockState swap_two(const ConnectedMooreBlockState& source) {
  ConnectedMooreBlockState result = source;
  if (result.constituents.size() != 2 || result.charges.size() != 2)
    return ConnectedMooreBlockState{};
  std::swap(result.constituents[0],result.constituents[1]);
  std::swap(result.charges[0],result.charges[1]);
  for (auto& edge : result.edges) {
    const std::size_t first = edge.first == 0 ? 1 : 0;
    const std::size_t second = edge.second == 0 ? 1 : 0;
    edge.first = std::min(first,second);
    edge.second = std::max(first,second);
    edge.reference_delta = {-edge.reference_delta.x,
                            -edge.reference_delta.y,
                            -edge.reference_delta.z};
  }
  return result;
}

SeedResult evaluate_seed(
    const ArmSpec& arm, const SeedSpec& seed,
    const ConnectedMooreBlockState& initial,
    const ConnectedMooreBlockStepResult& canonical,
    const ftd::eft::ConnectedMooreBlockOptions& base_options) {
  SeedResult result;
  result.arm = arm.label;
  result.seed = seed.label;
  auto options = base_options;
  options.root_momentum_seed = make_seed(initial,seed);
  const auto step = ftd::eft::solve_connected_moore_block_forward(
      initial,options);
  result.accepted = step.common_action_gates_pass;
  result.iterations = step.solve.iterations;
  result.root_residual = step.root_residual;
  if (!result.accepted) return result;
  result.state_difference =
      ftd::eft::connected_moore_block_state_max_difference(
          canonical.later,step.later);
  result.current_difference = current_difference(canonical,step);
  const auto reverse = ftd::eft::solve_connected_moore_block_reverse(
      step.later,base_options);
  if (reverse.common_action_gates_pass)
    result.recovery = ftd::eft::connected_moore_block_state_max_difference(
        initial,reverse.earlier);
  result.inverse = reverse.common_action_gates_pass
      && result.recovery <= inverse_gate;
  return result;
}

ArmResult run_arm(const ArmSpec& spec,
                  const ftd::eft::ConnectedMooreBlockOptions& options) {
  ArmResult result;
  result.spec = spec;
  const auto initialization = ftd::eft::initialize_connected_moore_block(
      L,1,0,spec.phase_axis,spec.phase);
  result.initialized = initialization.valid;
  if (!result.initialized) return result;

  const auto canonical = ftd::eft::solve_connected_moore_block_forward(
      initialization.state,options);
  result.canonical = canonical.common_action_gates_pass;
  if (!result.canonical) return result;

  result.all_seeds = true;
  for (const auto& seed : seed_specs()) {
    auto observation = evaluate_seed(
        spec,seed,initialization.state,canonical,options);
    result.all_seeds = result.all_seeds && observation.accepted
        && observation.inverse;
    if (observation.accepted) {
      result.maximum_state_difference = std::max(
          result.maximum_state_difference,observation.state_difference);
      result.maximum_current_difference = std::max(
          result.maximum_current_difference,observation.current_difference);
      if (std::isfinite(observation.recovery))
        result.maximum_recovery = std::max(
            result.maximum_recovery,observation.recovery);
      result.multiple_roots = result.multiple_roots
          || observation.state_difference > root_comparison_gate
          || observation.current_difference > current_comparison_gate;
    }
    result.seeds.push_back(observation);
  }

  const auto swapped_initial = swap_two(initialization.state);
  const auto swapped = ftd::eft::solve_connected_moore_block_forward(
      swapped_initial,options);
  if (swapped.common_action_gates_pass) {
    const auto unpermuted_later = swap_two(swapped.later);
    const double state_difference =
        ftd::eft::connected_moore_block_state_max_difference(
            canonical.later,unpermuted_later);
    const double current_residual = current_difference(canonical,swapped);
    const auto reversed = ftd::eft::solve_connected_moore_block_reverse(
        swapped.later,options);
    const double recovery = reversed.common_action_gates_pass
        ? ftd::eft::connected_moore_block_state_max_difference(
              swapped_initial,reversed.earlier)
        : INFINITY;
    result.relabeling = state_difference <= root_comparison_gate
        && current_residual <= current_comparison_gate
        && recovery <= inverse_gate;
    result.maximum_state_difference = std::max(
        result.maximum_state_difference,state_difference);
    result.maximum_current_difference = std::max(
        result.maximum_current_difference,current_residual);
    if (std::isfinite(recovery))
      result.maximum_recovery = std::max(result.maximum_recovery,recovery);
    result.multiple_roots = result.multiple_roots
        || state_difference > root_comparison_gate
        || current_residual > current_comparison_gate;
  }
  return result;
}

std::string classify(const std::vector<ArmResult>& arms) {
  if (arms.size() != 3
      || std::any_of(arms.begin(),arms.end(),
          [](const ArmResult& arm) { return !arm.canonical; }))
    return "INTERACTING_COMMON_ACTION_INVALID";
  if (std::any_of(arms.begin(),arms.end(),
          [](const ArmResult& arm) { return arm.multiple_roots; }))
    return "MULTIPLE_INTERACTING_ROOTS_WITNESSED";
  if (std::all_of(arms.begin(),arms.end(),[](const ArmResult& arm) {
        return arm.initialized && arm.all_seeds && arm.relabeling
            && arm.maximum_state_difference <= root_comparison_gate
            && arm.maximum_current_difference <= current_comparison_gate;
      }))
    return "INTERACTING_COMMON_ACTION_ONE_BASIN_WITNESSED";
  return "INTERACTING_ROOT_GLOBAL_UNIQUENESS_UNRESOLVED";
}

void write_record(const std::vector<ArmResult>& arms,
                  const std::string& verdict) {
  const auto directory = std::filesystem::path(__FILE__).parent_path()
      .parent_path()/"results"/"ftd_0720";
  std::filesystem::create_directories(directory);
  double maximum_state = 0.0, maximum_current = 0.0,
         maximum_recovery = 0.0;
  int accepted = 0, inverse = 0, relabeling = 0,
      all_seed_arms = 0, multiple_root_arms = 0;
  for (const auto& arm : arms) {
    maximum_state = std::max(maximum_state,arm.maximum_state_difference);
    maximum_current = std::max(
        maximum_current,arm.maximum_current_difference);
    maximum_recovery = std::max(maximum_recovery,arm.maximum_recovery);
    relabeling += arm.relabeling ? 1 : 0;
    all_seed_arms += arm.all_seeds ? 1 : 0;
    multiple_root_arms += arm.multiple_roots ? 1 : 0;
    for (const auto& seed : arm.seeds) {
      accepted += seed.accepted ? 1 : 0;
      inverse += seed.inverse ? 1 : 0;
    }
  }
  std::ofstream json(directory/
      "ftd_0720_interacting_common_action_root_multiseed_v1.json");
  json << std::setprecision(17) << "{\n"
       << "  \"ftd_id\": \"FTD-0720\",\n"
       << "  \"protocol_sha256\": \"" << protocol_sha256 << "\",\n"
       << "  \"verdict\": \"" << verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"arms\": " << arms.size() << ",\n"
       << "  \"registered_seeds\": " << 3*13 << ",\n"
       << "  \"accepted_seeds\": " << accepted << ",\n"
       << "  \"inverse_pass_seeds\": " << inverse << ",\n"
       << "  \"all_seed_arms\": " << all_seed_arms << ",\n"
       << "  \"relabeling_pass_arms\": " << relabeling << ",\n"
       << "  \"multiple_root_arms\": " << multiple_root_arms << ",\n"
       << "  \"maximum_state_difference\": " << maximum_state << ",\n"
       << "  \"maximum_current_difference\": " << maximum_current << ",\n"
       << "  \"maximum_inverse_recovery\": " << maximum_recovery << "\n"
       << "}\n";

  std::ofstream csv(directory/
      "ftd_0720_interacting_common_action_root_multiseed_v1.csv");
  csv << "ftd_id,protocol_sha256,arm,seed,accepted,inverse,iterations,"
         "root_residual,state_difference,current_difference,recovery\n";
  for (const auto& arm : arms)
    for (const auto& seed : arm.seeds)
      csv << std::setprecision(17) << "FTD-0720," << protocol_sha256 << ','
          << seed.arm << ',' << seed.seed << ',' << seed.accepted << ','
          << seed.inverse << ',' << seed.iterations << ','
          << seed.root_residual << ',' << seed.state_difference << ','
          << seed.current_difference << ',' << seed.recovery << '\n';
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  ftd::eft::ConnectedMooreBlockOptions options;
  options.gate_tolerance = 1e-10;
  options.solve_tolerance = 2e-11;
  options.max_iterations = 64;
  const std::array<ArmSpec,3> specs{{
      {"rest_axial",0,0.0},
      {"fractional_parallel",0,0.25},
      {"fractional_transverse",1,0.25}}};
  std::vector<ArmResult> arms;
  for (const auto& spec : specs) {
    std::cout << "running " << spec.label << std::endl;
    arms.push_back(run_arm(spec,options));
  }
  const std::string verdict = classify(arms);
  write_record(arms,verdict);
  int accepted = 0;
  for (const auto& arm : arms) {
    for (const auto& seed : arm.seeds) accepted += seed.accepted ? 1 : 0;
    std::cout << arm.spec.label << " canonical=" << arm.canonical
              << " seeds=" << arm.all_seeds
              << " relabel=" << arm.relabeling
              << " multiple=" << arm.multiple_roots
              << " state=" << arm.maximum_state_difference
              << " current=" << arm.maximum_current_difference
              << " recovery=" << arm.maximum_recovery << '\n';
  }
  std::cout << "protocol_sha256=" << protocol_sha256 << '\n'
            << "accepted_seeds=" << accepted << "/39\n"
            << "verdict=" << verdict << '\n';
  return verdict == "INTERACTING_COMMON_ACTION_INVALID" ? 1 : 0;
}
