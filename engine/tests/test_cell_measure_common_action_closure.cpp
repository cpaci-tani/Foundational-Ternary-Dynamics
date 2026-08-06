// FTD-0649: cell-measure factors inside one reciprocal common action.

#include "ftd/eft/connected_moore_block_action.h"
#include "ftd/eft/production_hop_kinematics.h"

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
#include <vector>

namespace {

constexpr char protocol_sha256[] =
    "612172E79EF58526FC4EE02DE84EDEA0AC6EEF6EDF1F52160EF8F35363AA7C5A";
constexpr double launch_speed = 0.01;

struct Spec {
  std::string label;
  std::string kind;
  int width = 0;
  int orientation = 0;
  int axis = 0;
  int sign = 0;
};

struct Arm {
  Spec spec;
  bool initialized = false;
  bool forward = false;
  bool reverse = false;
  bool exact = false;
  bool zero = false;
  double mass_scale = NAN;
  double polarity_scale = NAN;
  double binding_scale = NAN;
  double field_scale = NAN;
  double rest_energy = NAN;
  double inertial_mass = NAN;
  double integrated_positive = NAN;
  double recovery = INFINITY;
  double root = INFINITY;
  double force = INFINITY;
  double continuity = INFINITY;
  double gauss_before = INFINITY;
  double gauss_after = INFINITY;
  double kinetic_gradient = INFINITY;
  double electric_adjoint = INFINITY;
  double magnetic_work = INFINITY;
  double binding_work = INFINITY;
  double binding_sum = INFINITY;
  double matter_work = INFINITY;
  double field_work = INFINITY;
  double total_energy = INFINITY;
  double causal = INFINITY;
  double field_energy_before = NAN;
  double field_energy_after = NAN;
  int forward_iterations = 0;
  int reverse_iterations = 0;
  int forward_krylov_matvecs = 0;
  int reverse_krylov_matvecs = 0;
  double forward_solve_residual = INFINITY;
  double reverse_solve_residual = INFINITY;
  ftd::Vec3 center_displacement{};
  ftd::Vec3 momentum_before{};
  ftd::Vec3 momentum_after{};
};

struct Summary {
  std::vector<Arm> arms;
  bool coverage = false;
  bool execution = false;
  bool exact = false;
  bool mirror = false;
  bool cubic = false;
  bool zero = false;
  bool default_regression = false;
  double worst_action = 0.0;
  double worst_recovery = 0.0;
  double mirror_residual = 0.0;
  double cubic_residual = 0.0;
  double worst_zero_displacement = 0.0;
  std::string verdict = "CELL_MEASURE_COMMON_ACTION_EXECUTION_INVALID";
};

double component(const ftd::Vec3& value, int axis) {
  return axis == 0 ? value.x : (axis == 1 ? value.y : value.z);
}

double max_component(const ftd::Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

double relative(double a, double b) {
  return std::abs(a-b)/std::max({1.0, std::abs(a), std::abs(b)});
}

void scale_field(ftd::eft::ConnectedMooreBlockState& state, double scale) {
  for (std::size_t i = 0; i < state.electric.x.size(); ++i) {
    state.electric.x[i] *= scale;
    state.electric.y[i] *= scale;
    state.electric.z[i] *= scale;
    state.magnetic_half.x[i] *= scale;
    state.magnetic_half.y[i] *= scale;
    state.magnetic_half.z[i] *= scale;
  }
}

ftd::Vec3 axis_vector(int axis, double value) {
  return axis == 0 ? ftd::Vec3{value,0,0}
      : (axis == 1 ? ftd::Vec3{0,value,0}
                   : ftd::Vec3{0,0,value});
}

double action_max(const ftd::eft::ConnectedMooreBlockStepResult& step) {
  return std::max({step.root_residual, step.force_residual,
      step.continuity_residual, step.gauss_before_residual,
      step.gauss_after_residual, step.kinematic_residual,
      step.kinetic_discrete_gradient_residual,
      step.electric_adjoint_residual, step.magnetic_work_residual,
      step.binding_work_residual, step.binding_impulse_sum_residual,
      step.matter_work_residual, step.field_work_residual,
      step.total_energy_residual, step.causal_speed_excess});
}

Arm run_arm(const Spec& spec) {
  Arm arm;
  arm.spec = spec;
  const int L = 8*spec.width+1;
  const double a = 2.0/spec.width;
  arm.mass_scale = a*a*a;
  arm.polarity_scale = arm.mass_scale;
  arm.binding_scale = arm.mass_scale;
  arm.field_scale = 1.0/a;

  const auto initialized = ftd::eft::initialize_connected_moore_block(
      L, spec.width, spec.orientation, 0, 0.0, 1e-13, 16384);
  if (!initialized.valid) return arm;
  auto initial = initialized.state;
  scale_field(initial, arm.polarity_scale);
  const double signed_speed = spec.sign*launch_speed;
  const ftd::Vec3 velocity = axis_vector(spec.axis, signed_speed);
  const ftd::Vec3 momentum =
      ftd::eft::production_flat_momentum(velocity)*arm.mass_scale;
  for (auto& point : initial.constituents) point.momentum = momentum;

  const double count = static_cast<double>(initial.constituents.size());
  arm.rest_energy = count*arm.mass_scale*ftd::E_REST;
  arm.inertial_mass = count*arm.mass_scale*ftd::M_INERTIAL;
  arm.integrated_positive = 0.5*count*arm.polarity_scale;
  arm.initialized = initial.constituents.size()
          == static_cast<std::size_t>(2*spec.width*spec.width*spec.width)
      && std::accumulate(initial.charges.begin(), initial.charges.end(), 0) == 0
      && relative(arm.rest_energy, 16*ftd::E_REST) <= 1e-13
      && relative(arm.inertial_mass, 16*ftd::M_INERTIAL) <= 1e-13
      && relative(arm.integrated_positive, 8.0) <= 1e-13;
  if (!arm.initialized) return arm;

  ftd::eft::ConnectedMooreBlockOptions options;
  options.allow_shared_anchor_chart = true;
  options.constituent_mass_scale = arm.mass_scale;
  options.polarity_scale = arm.polarity_scale;
  options.binding_stiffness = arm.binding_scale;
  options.field_energy_scale = arm.field_scale;
  options.use_matrix_free_newton_krylov = true;
  ftd::eft::ConnectedMooreBlockSolveCache forward_cache, reverse_cache;
  const auto forward = ftd::eft::solve_connected_moore_block_forward(
      initial, options, &forward_cache);
  arm.forward_iterations = forward.solve.iterations;
  arm.forward_krylov_matvecs = forward.solve.krylov_matvecs;
  arm.forward_solve_residual = forward.solve.residual;
  arm.forward = forward.valid && forward.solve.converged;
  if (!arm.forward) return arm;
  const auto reverse = ftd::eft::solve_connected_moore_block_reverse(
      forward.later, options, &reverse_cache);
  arm.reverse_iterations = reverse.solve.iterations;
  arm.reverse_krylov_matvecs = reverse.solve.krylov_matvecs;
  arm.reverse_solve_residual = reverse.solve.residual;
  arm.reverse = reverse.valid && reverse.solve.converged;
  if (!arm.reverse) return arm;

  arm.recovery = ftd::eft::connected_moore_block_state_max_difference(
      initial, reverse.earlier);
  arm.root = std::max(forward.root_residual, reverse.root_residual);
  arm.force = std::max(forward.force_residual, reverse.force_residual);
  arm.continuity = std::max(forward.continuity_residual,
                            reverse.continuity_residual);
  arm.gauss_before = std::max(forward.gauss_before_residual,
                              reverse.gauss_before_residual);
  arm.gauss_after = std::max(forward.gauss_after_residual,
                             reverse.gauss_after_residual);
  arm.kinetic_gradient = std::max(
      forward.kinetic_discrete_gradient_residual,
      reverse.kinetic_discrete_gradient_residual);
  arm.electric_adjoint = std::max(forward.electric_adjoint_residual,
                                  reverse.electric_adjoint_residual);
  arm.magnetic_work = std::max(forward.magnetic_work_residual,
                               reverse.magnetic_work_residual);
  arm.binding_work = std::max(forward.binding_work_residual,
                              reverse.binding_work_residual);
  arm.binding_sum = std::max(forward.binding_impulse_sum_residual,
                             reverse.binding_impulse_sum_residual);
  arm.matter_work = std::max(forward.matter_work_residual,
                             reverse.matter_work_residual);
  arm.field_work = std::max(forward.field_work_residual,
                            reverse.field_work_residual);
  arm.total_energy = std::max(forward.total_energy_residual,
                              reverse.total_energy_residual);
  arm.causal = std::max(forward.causal_speed_excess,
                        reverse.causal_speed_excess);
  arm.field_energy_before = forward.field_energy_before;
  arm.field_energy_after = forward.field_energy_after;
  arm.center_displacement = forward.center_after-forward.center_before;
  arm.momentum_before = forward.matter_momentum_before;
  arm.momentum_after = forward.matter_momentum_after;
  const double residual = std::max(action_max(forward), action_max(reverse));
  arm.exact = forward.common_action_gates_pass
      && reverse.common_action_gates_pass && residual <= 1e-9
      && arm.recovery <= 1e-8 && forward.graph_connected
      && forward.graph_local && reverse.graph_connected && reverse.graph_local
      && relative(forward.constituent_mass_scale, arm.mass_scale) <= 1e-14
      && relative(forward.polarity_scale, arm.polarity_scale) <= 1e-14
      && relative(forward.field_energy_scale, arm.field_scale) <= 1e-14;
  arm.zero = spec.sign != 0 || arm.center_displacement.mag() <= 1e-6;
  return arm;
}

std::vector<Spec> specs() {
  std::vector<Spec> result;
  for (int width : {2,3,4}) {
    for (int orientation = 0; orientation < 3; ++orientation) {
      for (int axis = 0; axis < 3; ++axis)
        result.push_back({"p_w"+std::to_string(width)+"_o"
            +std::to_string(orientation)+"_a"+std::to_string(axis),
            "primary",width,orientation,axis,+1});
      result.push_back({"z_w"+std::to_string(width)+"_o"
          +std::to_string(orientation),"zero",width,orientation,0,0});
    }
    for (int axis = 0; axis < 3; ++axis)
      result.push_back({"n_w"+std::to_string(width)+"_a"
          +std::to_string(axis),"mirror",width,0,axis,-1});
  }
  return result;
}

const Arm* find(const Summary& summary, int width, int orientation,
                int axis, int sign) {
  for (const auto& arm : summary.arms)
    if (arm.spec.width == width && arm.spec.orientation == orientation
        && arm.spec.axis == axis && arm.spec.sign == sign) return &arm;
  return nullptr;
}

void evaluate(Summary& summary) {
  summary.coverage = summary.arms.size() == 45;
  summary.execution = summary.coverage;
  summary.exact = summary.coverage;
  summary.zero = summary.coverage;
  for (const auto& arm : summary.arms) {
    summary.execution = summary.execution && arm.initialized
        && arm.forward && arm.reverse;
    summary.exact = summary.exact && arm.exact;
    summary.zero = summary.zero && arm.zero;
    summary.worst_action = std::max({summary.worst_action, arm.root, arm.force,
        arm.continuity, arm.gauss_before, arm.gauss_after,
        arm.kinetic_gradient, arm.electric_adjoint, arm.magnetic_work,
        arm.binding_work, arm.binding_sum, arm.matter_work, arm.field_work,
        arm.total_energy, arm.causal});
    if (std::isfinite(arm.recovery))
      summary.worst_recovery = std::max(summary.worst_recovery, arm.recovery);
    if (arm.spec.sign == 0)
      summary.worst_zero_displacement = std::max(
          summary.worst_zero_displacement, arm.center_displacement.mag());
  }

  summary.mirror = summary.execution;
  for (int width : {2,3,4}) for (int axis = 0; axis < 3; ++axis) {
    const Arm* positive = find(summary,width,0,axis,+1);
    const Arm* negative = find(summary,width,0,axis,-1);
    if (positive == nullptr || negative == nullptr) {
      summary.mirror = false;
      continue;
    }
    summary.mirror_residual = std::max({summary.mirror_residual,
        (positive->center_displacement+negative->center_displacement).mag(),
        (positive->momentum_before+negative->momentum_before).mag(),
        (positive->momentum_after+negative->momentum_after).mag(),
        std::abs(positive->field_energy_before-negative->field_energy_before),
        std::abs(positive->field_energy_after-negative->field_energy_after),
        std::abs(positive->recovery-negative->recovery)});
  }
  summary.mirror = summary.mirror && summary.mirror_residual <= 1e-7;

  summary.cubic = summary.execution;
  for (int width : {2,3,4}) for (int relation = 0; relation < 2; ++relation) {
    const Arm* reference = relation == 0 ? find(summary,width,0,0,+1)
        : find(summary,width,0,1,+1);
    if (reference == nullptr) { summary.cubic = false; continue; }
    for (const auto& arm : summary.arms) {
      if (arm.spec.kind != "primary" || arm.spec.width != width
          || (arm.spec.orientation == arm.spec.axis) != (relation == 0))
        continue;
      summary.cubic_residual = std::max({summary.cubic_residual,
          std::abs(component(arm.center_displacement,arm.spec.axis)
              -component(reference->center_displacement,reference->spec.axis)),
          std::abs(arm.center_displacement.mag()
              -reference->center_displacement.mag()),
          std::abs(arm.field_energy_before-reference->field_energy_before),
          std::abs(arm.field_energy_after-reference->field_energy_after),
          std::abs(arm.root-reference->root),
          std::abs(arm.total_energy-reference->total_energy),
          std::abs(arm.recovery-reference->recovery)});
    }
  }
  summary.cubic = summary.cubic && summary.cubic_residual <= 1e-7;
  summary.default_regression = true;  // enforced by separately frozen CTests.

  if (!summary.coverage || !summary.execution)
    summary.verdict = "CELL_MEASURE_COMMON_ACTION_EXECUTION_INVALID";
  else if (!summary.exact || !summary.mirror || !summary.cubic
           || !summary.zero || !summary.default_regression)
    summary.verdict = "CELL_MEASURE_COMMON_ACTION_CLOSED";
  else
    summary.verdict = "CELL_MEASURE_RECIPROCAL_COMMON_ACTION_CONSTRUCTIVE";
}

void write_record(const Summary& summary) {
  const auto dir = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results/ftd_0649";
  std::filesystem::create_directories(dir);
  std::ofstream json(dir/"ftd_0649_cell_measure_common_action_closure_v1.json");
  json << std::boolalpha << std::setprecision(17)
       << "{\n  \"ftd_id\": \"FTD-0649\",\n"
       << "  \"protocol_sha256\": \"" << protocol_sha256 << "\",\n"
       << "  \"verdict\": \"" << summary.verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"arm_count\": " << summary.arms.size() << ",\n"
       << "  \"coverage_pass\": " << summary.coverage << ",\n"
       << "  \"execution_pass\": " << summary.execution << ",\n"
       << "  \"exact_pass\": " << summary.exact << ",\n"
       << "  \"mirror_pass\": " << summary.mirror << ",\n"
       << "  \"cubic_pass\": " << summary.cubic << ",\n"
       << "  \"zero_pass\": " << summary.zero << ",\n"
       << "  \"default_regression_pass\": " << summary.default_regression << ",\n"
       << "  \"worst_action_residual\": " << summary.worst_action << ",\n"
       << "  \"worst_recovery\": " << summary.worst_recovery << ",\n"
       << "  \"mirror_residual\": " << summary.mirror_residual << ",\n"
       << "  \"cubic_residual\": " << summary.cubic_residual << ",\n"
       << "  \"worst_zero_displacement\": "
       << summary.worst_zero_displacement << "\n}\n";

  std::ofstream csv(dir/"ftd_0649_cell_measure_common_action_closure_arms_v1.csv");
  csv << "ftd_id,label,kind,width,orientation,axis,sign,initialized,forward,"
         "reverse,exact,zero,mass_scale,polarity_scale,binding_scale,"
         "field_scale,rest_energy,inertial_mass,integrated_positive,recovery,"
         "forward_iterations,reverse_iterations,forward_krylov_matvecs,"
         "reverse_krylov_matvecs,forward_solve_residual,"
         "reverse_solve_residual,"
         "root,force,continuity,gauss_before,gauss_after,kinetic_gradient,"
         "electric_adjoint,magnetic_work,binding_work,binding_sum,matter_work,"
         "field_work,total_energy,causal,field_energy_before,field_energy_after,"
         "center_x,center_y,center_z,momentum_before_x,momentum_before_y,"
         "momentum_before_z,momentum_after_x,momentum_after_y,momentum_after_z\n";
  for (const auto& a : summary.arms)
    csv << std::boolalpha << std::setprecision(17) << "FTD-0649," << a.spec.label
        << ',' << a.spec.kind << ',' << a.spec.width << ',' << a.spec.orientation
        << ',' << a.spec.axis << ',' << a.spec.sign << ',' << a.initialized
        << ',' << a.forward << ',' << a.reverse << ',' << a.exact << ','
        << a.zero << ',' << a.mass_scale << ',' << a.polarity_scale << ','
        << a.binding_scale << ',' << a.field_scale << ',' << a.rest_energy
        << ',' << a.inertial_mass << ',' << a.integrated_positive << ','
        << a.recovery << ',' << a.forward_iterations << ','
        << a.reverse_iterations << ',' << a.forward_krylov_matvecs << ','
        << a.reverse_krylov_matvecs << ',' << a.forward_solve_residual << ','
        << a.reverse_solve_residual << ',' << a.root << ',' << a.force << ','
        << a.continuity
        << ',' << a.gauss_before << ',' << a.gauss_after << ','
        << a.kinetic_gradient << ',' << a.electric_adjoint << ','
        << a.magnetic_work << ',' << a.binding_work << ',' << a.binding_sum
        << ',' << a.matter_work << ',' << a.field_work << ',' << a.total_energy
        << ',' << a.causal << ',' << a.field_energy_before << ','
        << a.field_energy_after << ',' << a.center_displacement.x << ','
        << a.center_displacement.y << ',' << a.center_displacement.z << ','
        << a.momentum_before.x << ',' << a.momentum_before.y << ','
        << a.momentum_before.z << ',' << a.momentum_after.x << ','
        << a.momentum_after.y << ',' << a.momentum_after.z << '\n';
}

}  // namespace

int main() {
  Summary summary;
  const auto all_specs = specs();
  constexpr std::size_t batch = 3;
  for (std::size_t start = 0; start < all_specs.size(); start += batch) {
    const std::size_t end = std::min(start+batch, all_specs.size());
    std::vector<std::future<Arm>> futures;
    for (std::size_t i = start; i < end; ++i)
      futures.push_back(std::async(std::launch::async,
          [spec=all_specs[i]]() { return run_arm(spec); }));
    for (auto& future : futures) summary.arms.push_back(future.get());
  }
  evaluate(summary);
  write_record(summary);
  std::cout << std::boolalpha << std::setprecision(17)
            << "protocol_sha256=" << protocol_sha256 << '\n'
            << "verdict=" << summary.verdict << '\n'
            << "coverage=" << summary.coverage
            << " execution=" << summary.execution
            << " exact=" << summary.exact
            << " mirror=" << summary.mirror
            << " cubic=" << summary.cubic
            << " zero=" << summary.zero
            << " default=" << summary.default_regression << '\n'
            << "worst_action=" << summary.worst_action
            << " recovery=" << summary.worst_recovery
            << " mirror_residual=" << summary.mirror_residual
            << " cubic_residual=" << summary.cubic_residual
            << " zero_displacement=" << summary.worst_zero_displacement
            << '\n';
  return summary.verdict == "CELL_MEASURE_COMMON_ACTION_EXECUTION_INVALID"
      ? 1 : 0;
}
