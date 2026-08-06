// FTD-0624: dynamical classification of connected-block translation extrema.

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
#include <tuple>
#include <vector>

namespace {

using ftd::Vec3;
constexpr char protocol_sha256[] =
    "CB8AA8843B92F2D8ACB791C5DB01081C6BB2F6AD70E86EC074BBE0EA3E5720A2";
constexpr char parent_sha256[] =
    "4E86C850BB1354EC1A9C738FF1C50B94D558528966FED2F0EE40B26B67D69926";
constexpr int L = 17, width = 2, tick_count = 8;
constexpr double epsilon = 1.0/64.0;
constexpr double action_gate = 1e-10, static_gate = 1e-10;

enum class ArmKind {
  exact_maximum,
  maximum_perturbation,
  exact_minimum,
  minimum_perturbation
};

struct ArmSpec {
  std::string label;
  int orientation = 0;
  int phase_axis = 0;
  double phase = 0.0;
  ArmKind kind = ArmKind::exact_maximum;
  bool cyclic = false;
};

struct TickRecord {
  int tick = 0, hops = 0;
  Vec3 displacement{}, momentum{};
  double phase = 0.0, target_distance = 0.0;
  double kinetic = 0.0, binding = 0.0, field = 0.0, total = 0.0;
  double energy_drift = 0.0, shape = 0.0, strain = 0.0;
  double common_residual = 0.0;
  bool topology_pass = false;
};

struct ArmResult {
  ArmSpec spec{};
  bool initialization_pass = false, static_law_pass = false;
  bool forward_pass = false, reverse_pass = false, coherence_pass = false;
  bool extremum_pass = true, runaway_pass = true, restoring_pass = true;
  double coefficient = INFINITY, initial_field = INFINITY;
  double expected_field = INFINITY, static_residual = INFINITY;
  double maximum_common = INFINITY, maximum_energy_drift = INFINITY;
  double maximum_shape = INFINITY, maximum_strain = INFINITY;
  double recovery = INFINITY, maximum_target_distance = 0.0;
  int total_hops = 0, failure_tick = 0;
  bool failed_solve_converged = false;
  bool failed_site_projection = false;
  Vec3 final_displacement{}, final_momentum{}, first_momentum{};
  std::vector<TickRecord> history;
};

struct StaticAxisCheck {
  int axis = 0;
  bool pass = false;
  double coefficient = INFINITY, barrier = INFINITY;
  double energy_zero = INFINITY, energy_positive_half = INFINITY;
  double energy_negative_half = INFINITY, maximum_residual = INFINITY;
};

struct Summary {
  bool parent_pass = false, coverage_pass = false, action_pass = false;
  bool static_law_pass = false, exact_extrema_pass = false;
  bool runaway_pass = false, restoring_pass = false;
  bool mirror_pass = false, covariance_pass = false;
  double beta = 0.0, worst_common = INFINITY;
  double worst_energy_drift = INFINITY, worst_recovery = INFINITY;
  double mirror_residual = INFINITY, covariance_residual = INFINITY;
  std::string verdict;
  std::vector<StaticAxisCheck> static_axes;
  std::vector<ArmResult> arms;
};

double component(const Vec3& v, int axis) {
  return axis == 0 ? v.x : (axis == 1 ? v.y : v.z);
}

double max_component(const Vec3& v) {
  return std::max({std::abs(v.x),std::abs(v.y),std::abs(v.z)});
}

Vec3 cycle(const Vec3& v) { return {v.z,v.x,v.y}; }

double relative(double a, double b) {
  return std::abs(a-b)/std::max({1e-300,std::abs(a),std::abs(b)});
}

double q(double f) { return f*f*f*f-0.5*f*f; }

const char* kind_name(ArmKind kind) {
  switch (kind) {
    case ArmKind::exact_maximum: return "exact_maximum";
    case ArmKind::maximum_perturbation: return "maximum_perturbation";
    case ArmKind::exact_minimum: return "exact_minimum";
    case ArmKind::minimum_perturbation: return "minimum_perturbation";
  }
  return "unknown";
}

Vec3 position(const ftd::eft::MatchedMatterPoint& p) {
  return {static_cast<double>(p.anchor.x)+p.remainder.x,
          static_cast<double>(p.anchor.y)+p.remainder.y,
          static_cast<double>(p.anchor.z)+p.remainder.z};
}

Vec3 center(const ftd::eft::ConnectedMooreBlockState& state) {
  Vec3 result{};
  for (const auto& p : state.constituents) result += position(p);
  return result*(1.0/static_cast<double>(state.constituents.size()));
}

Vec3 momentum(const ftd::eft::ConnectedMooreBlockState& state) {
  Vec3 result{};
  for (const auto& p : state.constituents) result += p.momentum;
  return result;
}

double kinetic(const ftd::eft::ConnectedMooreBlockState& state) {
  long double result = 0.0L;
  for (const auto& p : state.constituents)
    result += ftd::eft::production_flat_energy_from_momentum(p.momentum);
  return static_cast<double>(result);
}

double field(const ftd::eft::ConnectedMooreBlockState& state, double beta) {
  return beta*ftd::eft::matched_modified_energy(
      state.electric,state.magnetic_half,ftd::C_SPEED);
}

double total(const ftd::eft::ConnectedMooreBlockState& state,
             const ftd::eft::ConnectedMooreBlockOptions& options,
             double beta) {
  return kinetic(state)+ftd::eft::connected_moore_block_binding_energy(
      state,options)+field(state,beta);
}

double shape_error(const ftd::eft::ConnectedMooreBlockState& initial,
                   const ftd::eft::ConnectedMooreBlockState& state) {
  const Vec3 c0 = center(initial), c1 = center(state);
  long double sum = 0.0L;
  for (std::size_t i = 0; i < state.constituents.size(); ++i) {
    const Vec3 delta = (position(state.constituents[i])-c1)
        -(position(initial.constituents[i])-c0);
    sum += delta.dot(delta);
  }
  return std::sqrt(static_cast<double>(
      sum/static_cast<long double>(state.constituents.size())));
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

bool parent_fingerprint() {
  const auto path = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results/ftd_0623/ftd_0623_connected_moore_block_repeated_v1.json";
  std::ifstream input(path,std::ios::binary);
  const std::string bytes((std::istreambuf_iterator<char>(input)),{});
  return bytes.find("\"ftd_id\": \"FTD-0623\"") != std::string::npos
      && bytes.find("CONNECTED_INTEGER_OBJECT_REPEATED_MOBILITY_CONSTRUCTIVE")
          != std::string::npos;
}

StaticAxisCheck check_static_axis(int axis, double beta) {
  StaticAxisCheck result;
  result.axis = axis;
  const auto spectrum = ftd::eft::evaluate_ternary_block_bipole_peierls(
      L,width,0,beta);
  const auto zero = ftd::eft::initialize_connected_moore_block(
      L,width,0,axis,0.0);
  const auto positive = ftd::eft::initialize_connected_moore_block(
      L,width,0,axis,0.5);
  const auto negative = ftd::eft::initialize_connected_moore_block(
      L,width,0,axis,-0.5);
  if (!spectrum.valid || !zero.valid || !positive.valid || !negative.valid)
    return result;
  result.coefficient = spectrum.peierls_coefficient[axis];
  result.barrier = result.coefficient/16.0;
  result.energy_zero = field(zero.state,beta);
  result.energy_positive_half = field(positive.state,beta);
  result.energy_negative_half = field(negative.state,beta);
  result.maximum_residual = std::max({
      std::abs(result.energy_zero-spectrum.energy),
      std::abs(result.energy_positive_half
          -(spectrum.energy+result.coefficient*q(0.5))),
      std::abs(result.energy_negative_half
          -(spectrum.energy+result.coefficient*q(-0.5))),
      std::abs((result.energy_zero-result.energy_positive_half)
          -result.barrier),
      std::abs((result.energy_zero-result.energy_negative_half)
          -result.barrier)});
  result.pass = result.coefficient > 0.0
      && result.energy_zero > result.energy_positive_half
      && result.energy_zero > result.energy_negative_half
      && result.maximum_residual <= static_gate;
  return result;
}

ArmResult run_arm(const ArmSpec& spec,
                  const ftd::eft::ConnectedMooreBlockOptions& options,
                  double beta) {
  ArmResult result;
  result.spec = spec;
  const auto initialization = ftd::eft::initialize_connected_moore_block(
      L,width,spec.orientation,spec.phase_axis,spec.phase);
  result.initialization_pass = initialization.valid
      && initialization.state.constituents.size() == 16
      && initialization.state.edges.size() == 72
      && initialization.poisson_residual <= 1e-11
      && initialization.gauss_residual <= 1e-11
      && initialization.curl_adjoint_residual <= 1e-11;
  if (!result.initialization_pass) return result;

  const auto spectrum = ftd::eft::evaluate_ternary_block_bipole_peierls(
      L,width,spec.orientation,beta);
  result.coefficient = spectrum.peierls_coefficient[spec.phase_axis];
  result.initial_field = field(initialization.state,beta);
  result.expected_field = spectrum.energy+result.coefficient*q(spec.phase);
  result.static_residual = std::abs(
      result.initial_field-result.expected_field);
  result.static_law_pass = spectrum.valid && result.coefficient > 0.0
      && result.static_residual <= static_gate;
  if (!result.static_law_pass) return result;

  const auto initial = initialization.state;
  auto state = initial;
  const Vec3 initial_center = center(initial);
  const double initial_total = total(initial,options,beta);
  const double target = spec.phase < 0.0 ? -0.5 : 0.5;
  result.maximum_common = result.maximum_energy_drift = 0.0;
  result.maximum_shape = result.maximum_strain = 0.0;
  result.forward_pass = true;
  for (int tick = 1; tick <= tick_count; ++tick) {
    const auto step = ftd::eft::solve_connected_moore_block_forward(
        state,options);
    const double residual = maximum_residual(step);
    result.maximum_common = std::max(result.maximum_common,residual);
    const bool topology = step.graph_connected && step.graph_local
        && step.site_projection_valid;
    if (!step.common_action_gates_pass || !topology
        || residual > action_gate) {
      result.failure_tick = tick;
      result.failed_solve_converged = step.solve.converged;
      result.failed_site_projection = !step.site_projection_valid;
      result.forward_pass = false;
      break;
    }
    state = step.later;
    result.total_hops += step.site_hops;
    const Vec3 displacement = center(state)-initial_center;
    const Vec3 matter_momentum = momentum(state);
    const double current_phase = spec.phase
        +component(displacement,spec.phase_axis);
    const double target_distance = std::abs(current_phase-target);
    const double kinetic_energy = kinetic(state);
    const double binding_energy =
        ftd::eft::connected_moore_block_binding_energy(state,options);
    const double field_energy = field(state,beta);
    const double total_energy = kinetic_energy+binding_energy+field_energy;
    const double drift = std::abs(total_energy-initial_total);
    const double shape = shape_error(initial,state);
    result.maximum_energy_drift = std::max(
        result.maximum_energy_drift,drift);
    result.maximum_shape = std::max(result.maximum_shape,shape);
    result.maximum_strain = std::max(
        result.maximum_strain,step.maximum_edge_strain);
    if (spec.kind == ArmKind::minimum_perturbation)
      result.maximum_target_distance = std::max(
          result.maximum_target_distance,target_distance);
    if (tick == 1) result.first_momentum = matter_momentum;
    result.history.push_back({tick,step.site_hops,displacement,matter_momentum,
        current_phase,target_distance,kinetic_energy,binding_energy,
        field_energy,total_energy,drift,shape,step.maximum_edge_strain,
        residual,topology});
  }
  result.forward_pass = result.forward_pass
      && result.history.size() == tick_count
      && result.maximum_energy_drift <= 1e-9;
  result.coherence_pass = result.forward_pass
      && result.maximum_shape <= 0.05 && result.maximum_strain <= 0.10;
  result.final_displacement = center(state)-initial_center;
  result.final_momentum = momentum(state);

  if (spec.kind == ArmKind::exact_maximum
      || spec.kind == ArmKind::exact_minimum) {
    result.extremum_pass = result.coherence_pass;
    for (const auto& tick : result.history)
      result.extremum_pass = result.extremum_pass
          && tick.displacement.mag() <= 1e-8
          && tick.momentum.mag() <= 1e-8;
  } else if (spec.kind == ArmKind::maximum_perturbation) {
    const double p1 = component(result.first_momentum,spec.phase_axis);
    const double displacement = component(
        result.final_displacement,spec.phase_axis);
    result.runaway_pass = result.coherence_pass
        && p1*spec.phase > 0.0
        && spec.phase*displacement > 0.0
        && std::abs(displacement) >= 1e-6
        && std::abs(spec.phase+displacement) > std::abs(spec.phase);
  } else {
    const double direction = target-spec.phase;
    const double p1 = component(result.first_momentum,spec.phase_axis);
    const double first_distance = result.history.empty() ? INFINITY
        : result.history.front().target_distance;
    result.restoring_pass = result.coherence_pass
        && p1*direction > 0.0
        && first_distance < epsilon
        && result.maximum_target_distance <= 2.0*epsilon;
  }

  result.reverse_pass = result.forward_pass;
  for (int tick = tick_count; result.reverse_pass && tick >= 1; --tick) {
    const auto step = ftd::eft::solve_connected_moore_block_reverse(
        state,options);
    const double residual = maximum_residual(step);
    result.maximum_common = std::max(result.maximum_common,residual);
    if (!step.common_action_gates_pass || !step.graph_connected
        || !step.graph_local || !step.site_projection_valid
        || residual > action_gate)
      result.reverse_pass = false;
    else state = step.earlier;
  }
  if (result.reverse_pass)
    result.recovery = ftd::eft::connected_moore_block_state_max_difference(
        initial,state);
  result.reverse_pass = result.reverse_pass && result.recovery <= 1e-8;
  return result;
}

const ArmResult* find(const Summary& summary, const std::string& label) {
  const auto found = std::find_if(summary.arms.begin(),summary.arms.end(),
      [&](const ArmResult& arm) { return arm.spec.label == label; });
  return found == summary.arms.end() ? nullptr : &*found;
}

bool basic(const ArmResult& arm) {
  return arm.initialization_pass && arm.static_law_pass && arm.forward_pass
      && arm.reverse_pass && arm.coherence_pass
      && arm.maximum_common <= action_gate;
}

double mirror_pair(const ArmResult& positive, const ArmResult& negative) {
  if (positive.history.size() != negative.history.size()) return INFINITY;
  double result = 0.0;
  for (std::size_t tick = 0; tick < positive.history.size(); ++tick) {
    const auto& p = positive.history[tick];
    const auto& n = negative.history[tick];
    result = std::max({result,
        max_component(p.displacement+n.displacement),
        max_component(p.momentum+n.momentum),
        std::abs(p.field-n.field),std::abs(p.shape-n.shape),
        std::abs(p.strain-n.strain),
        std::abs(static_cast<double>(p.hops-n.hops))});
  }
  return result;
}

double covariance_pair(const ArmResult& base, const ArmResult& rotated) {
  if (base.history.size() != rotated.history.size()) return INFINITY;
  double result = 0.0;
  for (std::size_t tick = 0; tick < base.history.size(); ++tick) {
    const auto& a = base.history[tick];
    const auto& b = rotated.history[tick];
    result = std::max({result,
        max_component(b.displacement-cycle(a.displacement)),
        max_component(b.momentum-cycle(a.momentum)),
        relative(b.field,a.field),relative(b.shape,a.shape),
        relative(b.strain,a.strain),
        std::abs(static_cast<double>(b.hops-a.hops))});
  }
  return result;
}

void evaluate(Summary& summary) {
  summary.coverage_pass = summary.arms.size() == 14
      && summary.static_axes.size() == 2;
  summary.static_law_pass = summary.coverage_pass;
  for (const auto& check : summary.static_axes)
    summary.static_law_pass = summary.static_law_pass && check.pass;
  summary.action_pass = summary.parent_pass && summary.coverage_pass;
  summary.exact_extrema_pass = summary.runaway_pass
      = summary.restoring_pass = true;
  summary.worst_common = summary.worst_energy_drift
      = summary.worst_recovery = 0.0;
  for (const auto& arm : summary.arms) {
    summary.static_law_pass = summary.static_law_pass && arm.static_law_pass;
    summary.action_pass = summary.action_pass && basic(arm);
    if (arm.spec.kind == ArmKind::exact_maximum
        || arm.spec.kind == ArmKind::exact_minimum)
      summary.exact_extrema_pass = summary.exact_extrema_pass
          && arm.extremum_pass;
    if (arm.spec.kind == ArmKind::maximum_perturbation)
      summary.runaway_pass = summary.runaway_pass && arm.runaway_pass;
    if (arm.spec.kind == ArmKind::minimum_perturbation)
      summary.restoring_pass = summary.restoring_pass && arm.restoring_pass;
    if (std::isfinite(arm.maximum_common))
      summary.worst_common = std::max(
          summary.worst_common,arm.maximum_common);
    if (std::isfinite(arm.maximum_energy_drift))
      summary.worst_energy_drift = std::max(
          summary.worst_energy_drift,arm.maximum_energy_drift);
    if (std::isfinite(arm.recovery))
      summary.worst_recovery = std::max(
          summary.worst_recovery,arm.recovery);
  }

  summary.mirror_residual = 0.0;
  summary.mirror_pass = summary.action_pass;
  for (const auto& labels : std::array<std::array<const char*,2>,4>{{
      {{"x_max_positive","x_max_negative"}},
      {{"x_min_positive","x_min_negative"}},
      {{"y_max_positive","y_max_negative"}},
      {{"y_min_positive","y_min_negative"}}}}) {
    const auto* positive = find(summary,labels[0]);
    const auto* negative = find(summary,labels[1]);
    if (positive == nullptr || negative == nullptr) {
      summary.mirror_pass = false;
      summary.mirror_residual = INFINITY;
    } else {
      summary.mirror_residual = std::max(
          summary.mirror_residual,mirror_pair(*positive,*negative));
    }
  }
  summary.mirror_pass = summary.mirror_pass
      && summary.mirror_residual <= 1e-8;

  const auto* x_max = find(summary,"x_max_positive");
  const auto* cyclic_max = find(summary,"cyclic_max_positive");
  const auto* x_min = find(summary,"x_min_positive");
  const auto* cyclic_min = find(summary,"cyclic_min_positive");
  summary.covariance_pass = summary.action_pass && x_max && cyclic_max
      && x_min && cyclic_min;
  summary.covariance_residual = summary.covariance_pass
      ? std::max(covariance_pair(*x_max,*cyclic_max),
                 covariance_pair(*x_min,*cyclic_min))
      : INFINITY;
  summary.covariance_pass = summary.covariance_pass
      && summary.covariance_residual <= 1e-8;

  if (!summary.action_pass || !summary.static_law_pass)
    summary.verdict = "CONNECTED_TRANSLATION_STABILITY_EXECUTION_INVALID";
  else if (summary.exact_extrema_pass && summary.runaway_pass
      && summary.restoring_pass && summary.mirror_pass
      && summary.covariance_pass)
    summary.verdict =
        "INTEGER_MAXIMUM_UNSTABLE_HALF_CELL_MINIMUM_RESTORING";
  else
    summary.verdict = "CONNECTED_TRANSLATION_STABILITY_DYNAMICS_INCONCLUSIVE";
}

void write_records(const Summary& summary) {
  const auto directory = std::filesystem::path(__FILE__).parent_path()
      .parent_path()/"results/ftd_0624";
  std::filesystem::create_directories(directory);
  std::ofstream json(directory/
      "ftd_0624_connected_block_translation_stability_v1.json");
  json << std::setprecision(17) << "{\n  \"ftd_id\": \"FTD-0624\",\n"
       << "  \"protocol_sha256\": \"" << protocol_sha256 << "\",\n"
       << "  \"parent_result_sha256\": \"" << parent_sha256 << "\",\n"
       << "  \"verdict\": \"" << summary.verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"epsilon\": " << epsilon << ",\n"
       << "  \"beta\": " << summary.beta << ",\n"
       << "  \"parent_pass\": " << summary.parent_pass << ",\n"
       << "  \"coverage_pass\": " << summary.coverage_pass << ",\n"
       << "  \"action_pass\": " << summary.action_pass << ",\n"
       << "  \"static_law_pass\": " << summary.static_law_pass << ",\n"
       << "  \"exact_extrema_pass\": " << summary.exact_extrema_pass << ",\n"
       << "  \"runaway_pass\": " << summary.runaway_pass << ",\n"
       << "  \"restoring_pass\": " << summary.restoring_pass << ",\n"
       << "  \"mirror_pass\": " << summary.mirror_pass << ",\n"
       << "  \"covariance_pass\": " << summary.covariance_pass << ",\n"
       << "  \"worst_common_residual\": " << summary.worst_common << ",\n"
       << "  \"worst_energy_drift\": "
       << summary.worst_energy_drift << ",\n"
       << "  \"worst_recovery\": " << summary.worst_recovery << ",\n"
       << "  \"mirror_residual\": " << summary.mirror_residual << ",\n"
       << "  \"covariance_residual\": ";
  if (std::isfinite(summary.covariance_residual))
    json << summary.covariance_residual;
  else
    json << "null";
  json << ",\n  \"static_axes\": [\n";
  for (std::size_t i = 0; i < summary.static_axes.size(); ++i) {
    const auto& check = summary.static_axes[i];
    json << "    {\"axis\": " << check.axis << ", \"pass\": "
         << check.pass << ", \"coefficient\": " << check.coefficient
         << ", \"barrier\": " << check.barrier
         << ", \"energy_zero\": " << check.energy_zero
         << ", \"energy_positive_half\": " << check.energy_positive_half
         << ", \"energy_negative_half\": " << check.energy_negative_half
         << ", \"maximum_residual\": " << check.maximum_residual << "}"
         << (i+1 == summary.static_axes.size() ? "\n" : ",\n");
  }
  json << "  ]\n}\n";

  std::ofstream arms(directory/
      "ftd_0624_connected_block_translation_stability_arms_v1.csv");
  arms << "ftd_id,label,kind,orientation,phase_axis,phase,cyclic,init,static_law,forward,reverse,coherence,extremum,runaway,restoring,failure_tick,failed_solve_converged,failed_site_projection,coefficient,initial_field,expected_field,static_residual,total_hops,final_dx,final_dy,final_dz,first_px,first_py,first_pz,final_px,final_py,final_pz,maximum_target_distance,maximum_shape,maximum_strain,maximum_common,maximum_energy_drift,recovery\n";
  for (const auto& arm : summary.arms)
    arms << std::setprecision(17) << "FTD-0624," << arm.spec.label << ','
         << kind_name(arm.spec.kind) << ',' << arm.spec.orientation << ','
         << arm.spec.phase_axis << ',' << arm.spec.phase << ','
         << arm.spec.cyclic << ',' << arm.initialization_pass << ','
         << arm.static_law_pass << ',' << arm.forward_pass << ','
         << arm.reverse_pass << ',' << arm.coherence_pass << ','
         << arm.extremum_pass << ',' << arm.runaway_pass << ','
         << arm.restoring_pass << ',' << arm.failure_tick << ','
         << arm.failed_solve_converged << ','
         << arm.failed_site_projection << ',' << arm.coefficient << ','
         << arm.initial_field << ',' << arm.expected_field << ','
         << arm.static_residual << ',' << arm.total_hops << ','
         << arm.final_displacement.x << ',' << arm.final_displacement.y << ','
         << arm.final_displacement.z << ',' << arm.first_momentum.x << ','
         << arm.first_momentum.y << ',' << arm.first_momentum.z << ','
         << arm.final_momentum.x << ',' << arm.final_momentum.y << ','
         << arm.final_momentum.z << ',' << arm.maximum_target_distance << ','
         << arm.maximum_shape << ',' << arm.maximum_strain << ','
         << arm.maximum_common << ',' << arm.maximum_energy_drift << ','
         << arm.recovery << '\n';

  std::ofstream ticks(directory/
      "ftd_0624_connected_block_translation_stability_ticks_v1.csv");
  ticks << "ftd_id,label,tick,dx,dy,dz,px,py,pz,phase,target_distance,kinetic,binding,field,total,energy_drift,shape,strain,hops,common_residual,topology_pass\n";
  for (const auto& arm : summary.arms) for (const auto& tick : arm.history)
    ticks << std::setprecision(17) << "FTD-0624," << arm.spec.label << ','
          << tick.tick << ',' << tick.displacement.x << ','
          << tick.displacement.y << ',' << tick.displacement.z << ','
          << tick.momentum.x << ',' << tick.momentum.y << ','
          << tick.momentum.z << ',' << tick.phase << ','
          << tick.target_distance << ',' << tick.kinetic << ','
          << tick.binding << ',' << tick.field << ',' << tick.total << ','
          << tick.energy_drift << ',' << tick.shape << ',' << tick.strain
          << ',' << tick.hops << ',' << tick.common_residual << ','
          << tick.topology_pass << '\n';
}

}  // namespace

int main(int argc, char** argv) {
  std::cout << std::setprecision(17);
  if (argc == 2 && std::string(argv[1]) == "--diagnose-exact-half") {
    const auto initialization = ftd::eft::initialize_connected_moore_block(
        L,width,0,0,0.5);
    ftd::eft::ConnectedMooreBlockOptions diagnostic_options;
    diagnostic_options.gate_tolerance = action_gate;
    diagnostic_options.solve_tolerance = 2e-11;
    diagnostic_options.max_iterations = 48;
    const auto step = ftd::eft::solve_connected_moore_block_forward(
        initialization.state,diagnostic_options);
    std::cout << "init=" << initialization.valid
              << " attempted=" << step.solve.attempted
              << " converged=" << step.solve.converged
              << " iterations=" << step.solve.iterations
              << " rejected=" << step.solve.rejected_steps
              << " root=" << step.root_residual
              << " solve_residual=" << step.solve.residual
              << " min_pivot=" << step.solve.minimum_abs_jacobian_pivot
              << " valid=" << step.valid
              << " common=" << step.common_action_gates_pass
              << " site_projection=" << step.site_projection_valid
              << " force=" << step.force_residual << '\n';
    std::map<std::tuple<int,int,int>,std::vector<std::size_t>> anchors;
    for (std::size_t index = 0; index < step.later.constituents.size(); ++index) {
      const auto& point = step.later.constituents[index];
      anchors[{point.anchor.x,point.anchor.y,point.anchor.z}].push_back(index);
    }
    for (const auto& [anchor,indices] : anchors) {
      if (indices.size() < 2) continue;
      std::cout << "collision_anchor=" << std::get<0>(anchor) << ','
                << std::get<1>(anchor) << ',' << std::get<2>(anchor)
                << " multiplicity=" << indices.size() << '\n';
      for (std::size_t index : indices) {
        const auto& point = step.later.constituents[index];
        const Vec3 x = position(point);
        std::cout << "  constituent=" << index
                  << " charge=" << step.later.charges[index]
                  << " effective=" << x.x << ',' << x.y << ',' << x.z
                  << " remainder=" << point.remainder.x << ','
                  << point.remainder.y << ',' << point.remainder.z << '\n';
      }
    }
    return 0;
  }
  Summary summary;
  summary.parent_pass = parent_fingerprint();
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  summary.beta = normalization.mapped_field_work_coefficient;
  ftd::eft::ConnectedMooreBlockOptions options;
  options.gate_tolerance = action_gate;
  options.solve_tolerance = 2e-11;
  options.max_iterations = 48;
  const std::vector<ArmSpec> specs{
      {"x_exact_max",0,0,0.0,ArmKind::exact_maximum,false},
      {"x_max_positive",0,0,+epsilon,ArmKind::maximum_perturbation,false},
      {"x_max_negative",0,0,-epsilon,ArmKind::maximum_perturbation,false},
      {"x_exact_min",0,0,+0.5,ArmKind::exact_minimum,false},
      {"x_min_positive",0,0,+0.5-epsilon,
          ArmKind::minimum_perturbation,false},
      {"x_min_negative",0,0,-0.5+epsilon,
          ArmKind::minimum_perturbation,false},
      {"y_exact_max",0,1,0.0,ArmKind::exact_maximum,false},
      {"y_max_positive",0,1,+epsilon,ArmKind::maximum_perturbation,false},
      {"y_max_negative",0,1,-epsilon,ArmKind::maximum_perturbation,false},
      {"y_exact_min",0,1,+0.5,ArmKind::exact_minimum,false},
      {"y_min_positive",0,1,+0.5-epsilon,
          ArmKind::minimum_perturbation,false},
      {"y_min_negative",0,1,-0.5+epsilon,
          ArmKind::minimum_perturbation,false},
      {"cyclic_max_positive",1,1,+epsilon,
          ArmKind::maximum_perturbation,true},
      {"cyclic_min_positive",1,1,+0.5-epsilon,
          ArmKind::minimum_perturbation,true}};
  if (summary.parent_pass && normalization.valid) {
    summary.static_axes.push_back(check_static_axis(0,summary.beta));
    summary.static_axes.push_back(check_static_axis(1,summary.beta));
    for (const auto& spec : specs) {
      std::cout << "running " << spec.label << std::endl;
      summary.arms.push_back(run_arm(spec,options,summary.beta));
    }
  }
  evaluate(summary);
  write_records(summary);
  std::cout << "protocol_sha256=" << protocol_sha256 << '\n'
            << "verdict=" << summary.verdict << '\n'
            << "action=" << summary.action_pass
            << " static=" << summary.static_law_pass
            << " extrema=" << summary.exact_extrema_pass
            << " runaway=" << summary.runaway_pass
            << " restoring=" << summary.restoring_pass
            << " mirror=" << summary.mirror_pass
            << " covariance=" << summary.covariance_pass << '\n'
            << "worst_common=" << summary.worst_common
            << " energy_drift=" << summary.worst_energy_drift
            << " recovery=" << summary.worst_recovery << '\n';
  for (const auto& arm : summary.arms)
    std::cout << arm.spec.label << " basic=" << basic(arm)
              << " extrema=" << arm.extremum_pass
              << " runaway=" << arm.runaway_pass
              << " restoring=" << arm.restoring_pass << " d="
              << arm.final_displacement.x << ',' << arm.final_displacement.y
              << ',' << arm.final_displacement.z << " p1="
              << arm.first_momentum.x << ',' << arm.first_momentum.y << ','
              << arm.first_momentum.z << " shape=" << arm.maximum_shape
              << " strain=" << arm.maximum_strain
              << " recovery=" << arm.recovery << '\n';
  return summary.verdict
      == "CONNECTED_TRANSLATION_STABILITY_EXECUTION_INVALID" ? 1 : 0;
}
