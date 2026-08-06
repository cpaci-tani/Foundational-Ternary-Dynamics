// FTD-0651: qualify repeated exact-root acceleration without changing physics.

#include "ftd/eft/connected_moore_block_action.h"
#include "ftd/eft/production_hop_kinematics.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <future>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace {

using ftd::Vec3;
using ftd::eft::ConnectedMooreBlockOptions;
using ftd::eft::ConnectedMooreBlockSolveCache;
using ftd::eft::ConnectedMooreBlockState;
using ftd::eft::ConnectedMooreBlockStepResult;

constexpr const char* protocol_sha =
    "06371B4E788FBB3E2840875340557F620617C593D686E8C410ECFE341266298A";

struct Spec {
  std::string label;
  std::string direction_name;
  int width = 0;
  Vec3 direction{};
  double speed = 0.0;
};

struct Totals {
  long long evaluations = 0;
  long long iterations = 0;
  long long matvecs = 0;
  long long refreshes = 0;
  long long reuses = 0;
  double seconds = 0.0;
};

struct Arm {
  Spec spec;
  int ticks = 0;
  bool initialized = false;
  bool matrix_free_valid = false;
  bool cached_valid = false;
  bool action = false;
  bool equivalent = false;
  bool inverse = false;
  bool reuse = false;
  double maximum_state_difference = INFINITY;
  double maximum_matrix_free_action = INFINITY;
  double maximum_cached_action = INFINITY;
  double matrix_free_recovery = INFINITY;
  double cached_recovery = INFINITY;
  Totals matrix_free;
  Totals cached;
};

struct Summary {
  std::vector<Arm> arms;
  bool coverage = false;
  bool convergence = false;
  bool action = false;
  bool equivalence = false;
  bool inverse = false;
  bool reuse = false;
  bool cheaper = false;
  long long repeated_matrix_free_evaluations = 0;
  long long repeated_cached_evaluations = 0;
  double worst_difference = 0.0;
  double worst_action = 0.0;
  double worst_recovery = 0.0;
  std::string verdict;
};

std::vector<Spec> specs() {
  const double inv_sqrt2 = 1.0/std::sqrt(2.0);
  const double inv_sqrt3 = 1.0/std::sqrt(3.0);
  std::vector<Spec> result;
  for (int width : {2,3,4}) {
    result.push_back({"w"+std::to_string(width)+"_v01_100",
        "100",width,{1,0,0},0.01});
    result.push_back({"w"+std::to_string(width)+"_v04_100",
        "100",width,{1,0,0},0.04});
    result.push_back({"w"+std::to_string(width)+"_v04_110",
        "110",width,{inv_sqrt2,inv_sqrt2,0},0.04});
    result.push_back({"w"+std::to_string(width)+"_v04_111",
        "111",width,{inv_sqrt3,inv_sqrt3,inv_sqrt3},0.04});
  }
  return result;
}

void scale_field(ConnectedMooreBlockState& state, double factor) {
  for (auto* values : {&state.electric.x,&state.electric.y,&state.electric.z,
                       &state.magnetic_half.x,&state.magnetic_half.y,
                       &state.magnetic_half.z})
    for (double& value : *values) value *= factor;
}

double action_max(const ConnectedMooreBlockStepResult& step) {
  return std::max({step.root_residual,step.force_residual,
      step.kinematic_residual,step.continuity_residual,
      step.gauss_before_residual,step.gauss_after_residual,
      step.kinetic_discrete_gradient_residual,
      step.electric_adjoint_residual,step.magnetic_work_residual,
      step.binding_work_residual,step.binding_impulse_sum_residual,
      step.matter_work_residual,step.field_work_residual,
      step.total_energy_residual,step.causal_speed_excess});
}

void accumulate(Totals& totals, const ConnectedMooreBlockStepResult& step,
                double seconds) {
  totals.evaluations += step.solve.residual_evaluations;
  totals.iterations += step.solve.iterations;
  totals.matvecs += step.solve.krylov_matvecs;
  totals.refreshes += step.solve.jacobian_refreshes;
  totals.reuses += step.solve.jacobian_reuses;
  totals.seconds += seconds;
}

template <typename Function>
ConnectedMooreBlockStepResult timed_solve(Function&& function,
                                          Totals& totals) {
  const auto start = std::chrono::steady_clock::now();
  auto step = function();
  const auto stop = std::chrono::steady_clock::now();
  accumulate(totals,step,
      std::chrono::duration<double>(stop-start).count());
  return step;
}

bool step_valid(const ConnectedMooreBlockStepResult& step) {
  return step.valid && step.solve.converged && step.common_action_gates_pass
      && action_max(step) <= 1e-9;
}

Arm run_arm(const Spec& spec) {
  Arm arm;
  arm.spec = spec;
  arm.ticks = spec.width == 2 ? 3 : 1;
  const double a = 2.0/spec.width;
  const double cell_measure = a*a*a;
  const int L = 8*spec.width+1;
  const auto initialized = ftd::eft::initialize_connected_moore_block(
      L,spec.width,0,0,0.0,1e-13,16384);
  if (!initialized.valid) return arm;

  auto initial = initialized.state;
  scale_field(initial,cell_measure);
  const Vec3 velocity = spec.direction*spec.speed;
  const Vec3 launch = ftd::eft::production_flat_momentum(velocity)
      *cell_measure;
  for (auto& point : initial.constituents) point.momentum = launch;
  arm.initialized = initial.constituents.size()
      == static_cast<std::size_t>(2*spec.width*spec.width*spec.width);
  if (!arm.initialized) return arm;

  ConnectedMooreBlockOptions matrix_free_options;
  matrix_free_options.allow_shared_anchor_chart = true;
  matrix_free_options.constituent_mass_scale = cell_measure;
  matrix_free_options.polarity_scale = cell_measure;
  matrix_free_options.binding_stiffness = cell_measure;
  matrix_free_options.field_energy_scale = 1.0/a;
  matrix_free_options.use_matrix_free_newton_krylov = true;
  ConnectedMooreBlockOptions cached_options = matrix_free_options;
  cached_options.use_matrix_free_newton_krylov = false;

  ConnectedMooreBlockSolveCache forward_cache,reverse_cache;
  auto matrix_state = initial;
  auto cached_state = initial;
  arm.maximum_state_difference = 0.0;
  arm.maximum_matrix_free_action = 0.0;
  arm.maximum_cached_action = 0.0;
  arm.matrix_free_valid = true;
  arm.cached_valid = true;

  for (int tick = 0; tick < arm.ticks; ++tick) {
    const auto matrix_step = timed_solve([&]() {
      return ftd::eft::solve_connected_moore_block_forward(
          matrix_state,matrix_free_options);
    },arm.matrix_free);
    const auto cached_step = timed_solve([&]() {
      return ftd::eft::solve_connected_moore_block_forward(
          cached_state,cached_options,&forward_cache);
    },arm.cached);
    arm.matrix_free_valid = arm.matrix_free_valid && step_valid(matrix_step);
    arm.cached_valid = arm.cached_valid && step_valid(cached_step);
    arm.maximum_matrix_free_action = std::max(
        arm.maximum_matrix_free_action,action_max(matrix_step));
    arm.maximum_cached_action = std::max(
        arm.maximum_cached_action,action_max(cached_step));
    if (!arm.matrix_free_valid || !arm.cached_valid) break;
    matrix_state = matrix_step.later;
    cached_state = cached_step.later;
    arm.maximum_state_difference = std::max(arm.maximum_state_difference,
        ftd::eft::connected_moore_block_state_max_difference(
            matrix_state,cached_state));
  }

  if (arm.matrix_free_valid && arm.cached_valid) {
    for (int tick = arm.ticks; tick > 0; --tick) {
      const auto matrix_step = timed_solve([&]() {
        return ftd::eft::solve_connected_moore_block_reverse(
            matrix_state,matrix_free_options);
      },arm.matrix_free);
      const auto cached_step = timed_solve([&]() {
        return ftd::eft::solve_connected_moore_block_reverse(
            cached_state,cached_options,&reverse_cache);
      },arm.cached);
      arm.matrix_free_valid = arm.matrix_free_valid && step_valid(matrix_step);
      arm.cached_valid = arm.cached_valid && step_valid(cached_step);
      arm.maximum_matrix_free_action = std::max(
          arm.maximum_matrix_free_action,action_max(matrix_step));
      arm.maximum_cached_action = std::max(
          arm.maximum_cached_action,action_max(cached_step));
      if (!arm.matrix_free_valid || !arm.cached_valid) break;
      matrix_state = matrix_step.earlier;
      cached_state = cached_step.earlier;
      arm.maximum_state_difference = std::max(arm.maximum_state_difference,
          ftd::eft::connected_moore_block_state_max_difference(
              matrix_state,cached_state));
    }
  }

  if (arm.matrix_free_valid && arm.cached_valid) {
    arm.matrix_free_recovery =
        ftd::eft::connected_moore_block_state_max_difference(
            initial,matrix_state);
    arm.cached_recovery =
        ftd::eft::connected_moore_block_state_max_difference(
            initial,cached_state);
  }
  arm.action = arm.matrix_free_valid && arm.cached_valid
      && arm.maximum_matrix_free_action <= 1e-9
      && arm.maximum_cached_action <= 1e-9;
  arm.equivalent = arm.action && arm.maximum_state_difference <= 1e-8;
  arm.inverse = arm.equivalent && arm.matrix_free_recovery <= 1e-8
      && arm.cached_recovery <= 1e-8;
  arm.reuse = arm.cached.refreshes >= 1 && arm.cached.reuses >= 1;
  return arm;
}

std::filesystem::path project_root() {
  return std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
}

void write_csv(const Summary& summary, const std::filesystem::path& path) {
  std::ofstream out(path);
  out << "ftd_id,label,width,direction,speed,ticks,initialized,matrix_free_valid,"
         "cached_valid,action,equivalent,inverse,reuse,max_state_difference,"
         "max_matrix_free_action,max_cached_action,matrix_free_recovery,"
         "cached_recovery,matrix_free_evaluations,cached_evaluations,"
         "matrix_free_iterations,cached_iterations,matrix_free_matvecs,"
         "cached_refreshes,cached_reuses,matrix_free_seconds,cached_seconds\n";
  out << std::setprecision(17) << std::boolalpha;
  for (const auto& arm : summary.arms)
    out << "FTD-0651," << arm.spec.label << ',' << arm.spec.width << ','
        << arm.spec.direction_name << ',' << arm.spec.speed << ',' << arm.ticks
        << ',' << arm.initialized << ',' << arm.matrix_free_valid << ','
        << arm.cached_valid << ',' << arm.action << ',' << arm.equivalent << ','
        << arm.inverse << ',' << arm.reuse << ','
        << arm.maximum_state_difference << ','
        << arm.maximum_matrix_free_action << ',' << arm.maximum_cached_action
        << ',' << arm.matrix_free_recovery << ',' << arm.cached_recovery << ','
        << arm.matrix_free.evaluations << ',' << arm.cached.evaluations << ','
        << arm.matrix_free.iterations << ',' << arm.cached.iterations << ','
        << arm.matrix_free.matvecs << ',' << arm.cached.refreshes << ','
        << arm.cached.reuses << ',' << arm.matrix_free.seconds << ','
        << arm.cached.seconds << '\n';
}

void write_json(const Summary& summary, const std::filesystem::path& path) {
  std::ofstream out(path);
  out << std::setprecision(17) << std::boolalpha
      << "{\n"
      << "  \"ftd_id\": \"FTD-0651\",\n"
      << "  \"protocol_sha256\": \"" << protocol_sha << "\",\n"
      << "  \"arm_count\": " << summary.arms.size() << ",\n"
      << "  \"coverage\": " << summary.coverage << ",\n"
      << "  \"convergence\": " << summary.convergence << ",\n"
      << "  \"action\": " << summary.action << ",\n"
      << "  \"equivalence\": " << summary.equivalence << ",\n"
      << "  \"inverse\": " << summary.inverse << ",\n"
      << "  \"cache_reuse\": " << summary.reuse << ",\n"
      << "  \"cheaper\": " << summary.cheaper << ",\n"
      << "  \"repeated_matrix_free_evaluations\": "
      << summary.repeated_matrix_free_evaluations << ",\n"
      << "  \"repeated_cached_evaluations\": "
      << summary.repeated_cached_evaluations << ",\n"
      << "  \"worst_state_difference\": " << summary.worst_difference << ",\n"
      << "  \"worst_action_residual\": " << summary.worst_action << ",\n"
      << "  \"worst_recovery\": " << summary.worst_recovery << ",\n"
      << "  \"verdict\": \"" << summary.verdict << "\"\n"
      << "}\n";
}

Summary summarize(std::vector<Arm> arms) {
  Summary summary;
  summary.arms = std::move(arms);
  summary.coverage = summary.arms.size() == 12;
  summary.convergence = summary.coverage;
  summary.action = summary.coverage;
  summary.equivalence = summary.coverage;
  summary.inverse = summary.coverage;
  summary.reuse = summary.coverage;
  for (const auto& arm : summary.arms) {
    summary.coverage = summary.coverage && arm.initialized;
    summary.convergence = summary.convergence && arm.matrix_free_valid
        && arm.cached_valid;
    summary.action = summary.action && arm.action;
    summary.equivalence = summary.equivalence && arm.equivalent;
    summary.inverse = summary.inverse && arm.inverse;
    summary.reuse = summary.reuse && arm.reuse;
    summary.worst_difference = std::max(
        summary.worst_difference,arm.maximum_state_difference);
    summary.worst_action = std::max({summary.worst_action,
        arm.maximum_matrix_free_action,arm.maximum_cached_action});
    summary.worst_recovery = std::max({summary.worst_recovery,
        arm.matrix_free_recovery,arm.cached_recovery});
    if (arm.spec.width == 2) {
      summary.repeated_matrix_free_evaluations += arm.matrix_free.evaluations;
      summary.repeated_cached_evaluations += arm.cached.evaluations;
    }
  }
  summary.cheaper = summary.repeated_cached_evaluations
      < summary.repeated_matrix_free_evaluations;
  if (!summary.coverage || !summary.convergence)
    summary.verdict = "REPEATED_EXACT_ROOT_ACCELERATION_EXECUTION_INVALID";
  else if (!summary.action || !summary.equivalence || !summary.inverse
           || !summary.reuse)
    summary.verdict = "REPEATED_EXACT_ROOT_ACCELERATION_CLOSED";
  else if (!summary.cheaper)
    summary.verdict =
        "REPEATED_EXACT_ROOT_ACCELERATION_EQUIVALENT_BUT_NOT_CHEAPER";
  else
    summary.verdict = "REPEATED_EXACT_ROOT_ACCELERATION_CONSTRUCTIVE";
  return summary;
}

}  // namespace

int main() {
  const auto all_specs = specs();
  std::vector<Arm> arms;
  constexpr std::size_t batch = 4;
  for (std::size_t start = 0; start < all_specs.size(); start += batch) {
    const std::size_t end = std::min(start+batch,all_specs.size());
    std::vector<std::future<Arm>> futures;
    for (std::size_t i = start; i < end; ++i)
      futures.push_back(std::async(std::launch::async,
          [spec=all_specs[i]]() { return run_arm(spec); }));
    for (std::size_t i = start; i < end; ++i) {
      arms.push_back(futures[i-start].get());
      std::cout << "completed " << all_specs[i].label << std::endl;
    }
  }

  const Summary summary = summarize(std::move(arms));
  const auto output = project_root()/"engine"/"results"/"ftd_0651";
  std::filesystem::create_directories(output);
  write_csv(summary,output/"ftd_0651_repeated_exact_root_acceleration_arms_v1.csv");
  write_json(summary,output/"ftd_0651_repeated_exact_root_acceleration_v1.json");
  std::cout << summary.verdict << '\n'
            << "difference=" << summary.worst_difference
            << " action=" << summary.worst_action
            << " recovery=" << summary.worst_recovery << '\n'
            << "repeated evaluations: matrix-free="
            << summary.repeated_matrix_free_evaluations
            << " cached=" << summary.repeated_cached_evaluations << '\n';
  return summary.verdict ==
      "REPEATED_EXACT_ROOT_ACCELERATION_EXECUTION_INVALID" ? 1 : 0;
}
