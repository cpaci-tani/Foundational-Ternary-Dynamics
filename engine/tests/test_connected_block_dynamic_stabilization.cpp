// FTD-0625: existing-variable circulation at the connected-block collision surface.

#include "ftd/eft/connected_moore_block_action.h"
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

using ftd::Vec3;
constexpr char protocol_sha256[] =
    "E95F2EB5A91C599AEFF790F55A34E548628D95FE247E95C843446A6940E751CA";
constexpr char parent_sha256[] =
    "55D34381B4968653740DF57A0F2330A3D175CC2CFD52012A2C4657D601825653";
constexpr int L = 17, width = 2, tick_count = 16;
constexpr double epsilon = 1.0/64.0, phase = 0.5-epsilon;
constexpr double action_gate = 1e-10;

struct ArmSpec {
  std::string label;
  int orientation = 0, phase_axis = 0, rotation_axis = 2;
  int energy_multiple = 0, circulation_sign = 0;
  bool cyclic = false;
};

struct TickRecord {
  int tick = 0, hops = 0, conflicts = 0;
  Vec3 displacement{}, momentum{};
  double chart_margin = -INFINITY, half_distance = INFINITY;
  double angular_momentum = 0.0, shape = INFINITY, strain = INFINITY;
  double kinetic = INFINITY, binding = INFINITY, field = INFINITY;
  double total = INFINITY, energy_drift = INFINITY;
  double common_residual = INFINITY, local_defect = INFINITY;
  double spline_defect = INFINITY;
};

struct ArmResult {
  ArmSpec spec{};
  bool initialization_pass = false, amplitude_pass = false;
  bool forward_complete = false, reverse_pass = false;
  bool coherence_pass = false, exact_steps_pass = false;
  bool candidate_pass = false;
  bool failure_solve_converged = false, failure_site_projection = false;
  bool failure_graph = false;
  int failure_tick = 0, failure_conflicts = 0;
  int failure_same_polarity_pairs = 0, failure_opposite_polarity_pairs = 0;
  int total_hops = 0, maximum_conflicts = 0;
  double barrier = INFINITY, amplitude = INFINITY;
  double amplitude_residual = INFINITY, initial_momentum = INFINITY;
  double initial_angular_momentum = 0.0, final_angular_momentum = 0.0;
  double minimum_chart_margin = INFINITY, maximum_half_distance = 0.0;
  double maximum_shape = INFINITY, maximum_strain = INFINITY;
  double maximum_common = INFINITY, maximum_energy_drift = INFINITY;
  double recovery = INFINITY, failure_root_residual = INFINITY;
  Vec3 final_displacement{}, final_momentum{};
  std::vector<TickRecord> history;
};

struct Summary {
  bool parent_pass = false, normalization_pass = false;
  bool coverage_pass = false, execution_pass = false;
  bool symmetry_pass = false, covariance_pass = false;
  bool family_one_pass = false, family_four_pass = false;
  double beta = 0.0, baseline_margin = -INFINITY;
  double symmetry_residual = INFINITY, covariance_residual = INFINITY;
  double worst_common = INFINITY, worst_energy_drift = INFINITY;
  double worst_recovery = INFINITY;
  std::string verdict;
  std::vector<ArmResult> arms;
};

double component(const Vec3& value, int axis) {
  return axis == 0 ? value.x : (axis == 1 ? value.y : value.z);
}

Vec3 axis_vector(int axis) {
  return axis == 0 ? Vec3{1,0,0} : (axis == 1 ? Vec3{0,1,0}
                                                   : Vec3{0,0,1});
}

Vec3 cycle(const Vec3& value) { return {value.z,value.x,value.y}; }

double maximum_component(const Vec3& value) {
  return std::max({std::abs(value.x),std::abs(value.y),std::abs(value.z)});
}

double relative(double lhs, double rhs) {
  return std::abs(lhs-rhs)/std::max({1e-300,std::abs(lhs),std::abs(rhs)});
}

Vec3 position(const ftd::eft::MatchedMatterPoint& point) {
  return {point.anchor.x+point.remainder.x,
          point.anchor.y+point.remainder.y,
          point.anchor.z+point.remainder.z};
}

Vec3 center(const ftd::eft::ConnectedMooreBlockState& state) {
  Vec3 result{};
  for (const auto& point : state.constituents) result += position(point);
  return result*(1.0/static_cast<double>(state.constituents.size()));
}

Vec3 momentum(const ftd::eft::ConnectedMooreBlockState& state) {
  Vec3 result{};
  for (const auto& point : state.constituents) result += point.momentum;
  return result;
}

double kinetic(const ftd::eft::ConnectedMooreBlockState& state) {
  long double result = 0.0L;
  for (const auto& point : state.constituents)
    result += ftd::eft::production_flat_energy_from_momentum(point.momentum);
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

double angular_momentum(const ftd::eft::ConnectedMooreBlockState& state,
                        int axis) {
  const Vec3 origin = center(state);
  Vec3 result{};
  for (const auto& point : state.constituents)
    result += Vec3::cross(position(point)-origin,point.momentum);
  return component(result,axis);
}

double chart_margin(const ftd::eft::ConnectedMooreBlockState& state) {
  double result = INFINITY;
  for (const auto& point : state.constituents)
    result = std::min({result,0.5-std::abs(point.remainder.x),
        0.5-std::abs(point.remainder.y),
        0.5-std::abs(point.remainder.z)});
  return result;
}

int anchor_conflicts(const ftd::eft::ConnectedMooreBlockState& state) {
  std::map<std::tuple<int,int,int>,int> counts;
  for (const auto& point : state.constituents)
    ++counts[{point.anchor.x,point.anchor.y,point.anchor.z}];
  int result = 0;
  for (const auto& item : counts)
    if (item.second > 1) result += item.second-1;
  return result;
}

std::tuple<int,int,int> conflict_classes(
    const ftd::eft::ConnectedMooreBlockState& state) {
  std::map<std::tuple<int,int,int>,std::vector<std::size_t>> anchors;
  for (std::size_t index = 0; index < state.constituents.size(); ++index) {
    const auto& point = state.constituents[index];
    anchors[{point.anchor.x,point.anchor.y,point.anchor.z}].push_back(index);
  }
  int excess = 0, same = 0, opposite = 0;
  for (const auto& item : anchors) {
    const auto& indices = item.second;
    if (indices.size() > 1) excess += static_cast<int>(indices.size())-1;
    for (std::size_t i = 0; i < indices.size(); ++i)
      for (std::size_t j = i+1; j < indices.size(); ++j)
        if (state.charges[indices[i]] == state.charges[indices[j]]) ++same;
        else ++opposite;
  }
  return {excess,same,opposite};
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
      /"results/ftd_0624/ftd_0624_connected_block_translation_stability_v1.json";
  std::ifstream input(path,std::ios::binary);
  const std::string bytes((std::istreambuf_iterator<char>(input)),{});
  return bytes.find("\"ftd_id\": \"FTD-0624\"") != std::string::npos
      && bytes.find("CONNECTED_TRANSLATION_STABILITY_EXECUTION_INVALID")
          != std::string::npos;
}

std::vector<Vec3> circulation_basis(
    const ftd::eft::ConnectedMooreBlockState& state, int axis) {
  const Vec3 origin = center(state), normal = axis_vector(axis);
  std::vector<Vec3> result;
  result.reserve(state.constituents.size());
  for (const auto& point : state.constituents)
    result.push_back(Vec3::cross(normal,position(point)-origin));
  return result;
}

double excess_kinetic(const std::vector<Vec3>& basis, double amplitude) {
  const double rest = ftd::eft::production_flat_energy_from_momentum({});
  long double result = 0.0L;
  for (const Vec3& vector : basis)
    result += ftd::eft::production_flat_energy_from_momentum(vector*amplitude)
        -rest;
  return static_cast<double>(result);
}

std::pair<double,double> solve_amplitude(
    const std::vector<Vec3>& basis, double target) {
  if (target == 0.0) return {0.0,0.0};
  double low = 0.0, high = 1.0;
  while (excess_kinetic(basis,high) < target && high < 1e6) high *= 2.0;
  if (!(excess_kinetic(basis,high) >= target)) return {INFINITY,INFINITY};
  for (int iteration = 0; iteration < 160; ++iteration) {
    const double middle = 0.5*(low+high);
    if (excess_kinetic(basis,middle) < target) low = middle;
    else high = middle;
  }
  const double amplitude = 0.5*(low+high);
  return {amplitude,std::abs(excess_kinetic(basis,amplitude)-target)};
}

ArmResult run_arm(const ArmSpec& spec,
                  const ftd::eft::ConnectedMooreBlockOptions& options,
                  double beta) {
  ArmResult result;
  result.spec = spec;
  const auto initialization = ftd::eft::initialize_connected_moore_block(
      L,width,spec.orientation,spec.phase_axis,phase);
  result.initialization_pass = initialization.valid
      && initialization.state.constituents.size() == 16
      && initialization.state.edges.size() == 72
      && initialization.poisson_residual <= 1e-11
      && initialization.gauss_residual <= 1e-11;
  if (!result.initialization_pass) return result;
  const auto spectrum = ftd::eft::evaluate_ternary_block_bipole_peierls(
      L,width,spec.orientation,beta);
  result.barrier = spectrum.peierls_coefficient[spec.phase_axis]/16.0;
  const auto basis = circulation_basis(initialization.state,spec.rotation_axis);
  const double target = spec.energy_multiple*result.barrier;
  const auto amplitude = solve_amplitude(basis,target);
  result.amplitude = amplitude.first;
  result.amplitude_residual = amplitude.second;
  result.amplitude_pass = spectrum.valid && result.barrier > 0.0
      && std::isfinite(result.amplitude)
      && result.amplitude_residual <= 1e-13;
  if (!result.amplitude_pass) return result;
  auto initial = initialization.state;
  for (std::size_t i = 0; i < initial.constituents.size(); ++i)
    initial.constituents[i].momentum = basis[i]
        *(spec.circulation_sign*result.amplitude);
  result.initial_momentum = momentum(initial).mag();
  result.initial_angular_momentum = angular_momentum(
      initial,spec.rotation_axis);
  result.amplitude_pass = result.amplitude_pass
      && result.initial_momentum <= 1e-14;
  if (!result.amplitude_pass) return result;

  auto state = initial;
  const Vec3 initial_center = center(initial);
  const double initial_total = total(initial,options,beta);
  result.minimum_chart_margin = INFINITY;
  result.maximum_shape = result.maximum_strain = result.maximum_common = 0.0;
  result.maximum_energy_drift = 0.0;
  result.exact_steps_pass = true;
  for (int tick = 1; tick <= tick_count; ++tick) {
    const auto step = ftd::eft::solve_connected_moore_block_forward(
        state,options);
    const double residual = maximum_residual(step);
    result.maximum_common = std::max(result.maximum_common,residual);
    if (!step.common_action_gates_pass || residual > action_gate
        || !step.graph_connected || !step.graph_local
        || !step.site_projection_valid) {
      result.failure_tick = tick;
      result.failure_solve_converged = step.solve.converged;
      result.failure_site_projection = !step.site_projection_valid;
      result.failure_graph = !step.graph_connected || !step.graph_local;
      result.failure_root_residual = step.root_residual;
      std::tie(result.failure_conflicts,result.failure_same_polarity_pairs,
               result.failure_opposite_polarity_pairs) =
          conflict_classes(step.later);
      result.exact_steps_pass = false;
      break;
    }
    state = step.later;
    result.total_hops += step.site_hops;
    const Vec3 displacement = center(state)-initial_center;
    const int conflicts = anchor_conflicts(state);
    const double margin = chart_margin(state);
    const double half_distance = std::abs(
        0.5-(phase+component(displacement,spec.phase_axis)));
    const double shape = shape_error(initial,state);
    const double kinetic_energy = kinetic(state);
    const double binding_energy =
        ftd::eft::connected_moore_block_binding_energy(state,options);
    const double field_energy = field(state,beta);
    const double total_energy = kinetic_energy+binding_energy+field_energy;
    const double drift = std::abs(total_energy-initial_total);
    result.maximum_conflicts = std::max(result.maximum_conflicts,conflicts);
    result.minimum_chart_margin = std::min(result.minimum_chart_margin,margin);
    result.maximum_half_distance = std::max(
        result.maximum_half_distance,half_distance);
    result.maximum_shape = std::max(result.maximum_shape,shape);
    result.maximum_strain = std::max(
        result.maximum_strain,step.maximum_edge_strain);
    result.maximum_energy_drift = std::max(
        result.maximum_energy_drift,drift);
    result.history.push_back({tick,step.site_hops,conflicts,displacement,
        momentum(state),margin,half_distance,
        angular_momentum(state,spec.rotation_axis),shape,
        step.maximum_edge_strain,kinetic_energy,binding_energy,field_energy,
        total_energy,drift,residual,step.local_defect_norm,
        step.spline_defect_norm});
  }
  result.forward_complete = result.history.size() == tick_count
      && result.exact_steps_pass;
  result.coherence_pass = result.forward_complete
      && result.maximum_energy_drift <= 1e-9
      && result.maximum_shape <= 0.05 && result.maximum_strain <= 0.10
      && result.maximum_conflicts == 0;
  result.final_displacement = center(state)-initial_center;
  result.final_momentum = momentum(state);
  result.final_angular_momentum = angular_momentum(state,spec.rotation_axis);

  result.reverse_pass = result.forward_complete;
  for (int tick = tick_count; result.reverse_pass && tick >= 1; --tick) {
    const auto step = ftd::eft::solve_connected_moore_block_reverse(
        state,options);
    result.maximum_common = std::max(
        result.maximum_common,maximum_residual(step));
    if (!step.common_action_gates_pass || !step.graph_connected
        || !step.graph_local || !step.site_projection_valid
        || maximum_residual(step) > action_gate)
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

double signed_symmetry(const ArmResult& positive, const ArmResult& negative) {
  if (positive.history.size() != negative.history.size()) return INFINITY;
  double result = 0.0;
  for (std::size_t tick = 0; tick < positive.history.size(); ++tick) {
    const auto& p = positive.history[tick];
    const auto& n = negative.history[tick];
    result = std::max({result,std::abs(p.total-n.total),
        std::abs(p.shape-n.shape),std::abs(p.strain-n.strain),
        std::abs(p.chart_margin-n.chart_margin),
        std::abs(p.half_distance-n.half_distance),
        std::abs(p.angular_momentum+n.angular_momentum),
        std::abs(static_cast<double>(p.conflicts-n.conflicts))});
  }
  return result;
}

double cyclic_covariance(const ArmResult& base, const ArmResult& rotated) {
  if (base.history.size() != rotated.history.size()) return INFINITY;
  double result = 0.0;
  for (std::size_t tick = 0; tick < base.history.size(); ++tick) {
    const auto& a = base.history[tick];
    const auto& b = rotated.history[tick];
    result = std::max({result,
        maximum_component(b.displacement-cycle(a.displacement)),
        maximum_component(b.momentum-cycle(a.momentum)),
        relative(b.total,a.total),relative(b.shape,a.shape),
        relative(b.strain,a.strain),
        std::abs(b.chart_margin-a.chart_margin),
        std::abs(b.half_distance-a.half_distance),
        std::abs(b.angular_momentum-a.angular_momentum),
        std::abs(static_cast<double>(b.conflicts-a.conflicts))});
  }
  return result;
}

bool arm_feasible(const ArmResult& arm, double baseline_margin) {
  if (arm.spec.energy_multiple == 0) return false;
  return arm.forward_complete && arm.reverse_pass && arm.coherence_pass
      && arm.maximum_half_distance <= 2.0*epsilon
      && arm.minimum_chart_margin >= baseline_margin+epsilon/10.0
      && arm.initial_angular_momentum*arm.final_angular_momentum > 0.0;
}

void evaluate(Summary& summary) {
  summary.coverage_pass = summary.arms.size() == 7;
  summary.execution_pass = summary.parent_pass && summary.normalization_pass
      && summary.coverage_pass;
  summary.worst_common = summary.worst_energy_drift
      = summary.worst_recovery = 0.0;
  for (const auto& arm : summary.arms) {
    summary.execution_pass = summary.execution_pass
        && arm.initialization_pass && arm.amplitude_pass;
    if (std::isfinite(arm.maximum_common))
      summary.worst_common = std::max(summary.worst_common,arm.maximum_common);
    if (std::isfinite(arm.maximum_energy_drift))
      summary.worst_energy_drift = std::max(
          summary.worst_energy_drift,arm.maximum_energy_drift);
    if (std::isfinite(arm.recovery))
      summary.worst_recovery = std::max(summary.worst_recovery,arm.recovery);
  }
  const auto* control = find(summary,"near_half_zero");
  if (control != nullptr) summary.baseline_margin =
      control->minimum_chart_margin;
  summary.execution_pass = summary.execution_pass && control != nullptr
      && std::isfinite(summary.baseline_margin);

  const auto* p1 = find(summary,"circulation_positive_1B");
  const auto* n1 = find(summary,"circulation_negative_1B");
  const auto* p4 = find(summary,"circulation_positive_4B");
  const auto* n4 = find(summary,"circulation_negative_4B");
  const auto* c1 = find(summary,"cyclic_positive_1B");
  const auto* c4 = find(summary,"cyclic_positive_4B");
  summary.symmetry_residual = p1 && n1 && p4 && n4
      ? std::max(signed_symmetry(*p1,*n1),signed_symmetry(*p4,*n4))
      : INFINITY;
  summary.covariance_residual = p1 && p4 && c1 && c4
      ? std::max(cyclic_covariance(*p1,*c1),cyclic_covariance(*p4,*c4))
      : INFINITY;
  summary.symmetry_pass = summary.symmetry_residual <= 1e-8;
  summary.covariance_pass = summary.covariance_residual <= 1e-8;
  summary.family_one_pass = p1 && n1 && c1
      && arm_feasible(*p1,summary.baseline_margin)
      && arm_feasible(*n1,summary.baseline_margin)
      && arm_feasible(*c1,summary.baseline_margin)
      && summary.symmetry_pass && summary.covariance_pass;
  summary.family_four_pass = p4 && n4 && c4
      && arm_feasible(*p4,summary.baseline_margin)
      && arm_feasible(*n4,summary.baseline_margin)
      && arm_feasible(*c4,summary.baseline_margin)
      && summary.symmetry_pass && summary.covariance_pass;
  for (auto& arm : summary.arms)
    arm.candidate_pass = arm_feasible(arm,summary.baseline_margin);

  if (!summary.execution_pass)
    summary.verdict = "CONNECTED_DYNAMIC_STABILIZATION_EXECUTION_INVALID";
  else if (summary.family_one_pass || summary.family_four_pass)
    summary.verdict = "RIGID_CIRCULATION_DYNAMIC_STABILIZATION_FEASIBLE";
  else
    summary.verdict = "RIGID_CIRCULATION_DYNAMIC_STABILIZATION_CLOSED_NEGATIVE";
}

void write_number(std::ostream& output, double value) {
  if (std::isfinite(value)) output << value;
  else output << "null";
}

void write_records(const Summary& summary) {
  const auto directory = std::filesystem::path(__FILE__).parent_path()
      .parent_path()/"results/ftd_0625";
  std::filesystem::create_directories(directory);
  std::ofstream json(directory/
      "ftd_0625_connected_block_dynamic_stabilization_v1.json");
  json << std::setprecision(17) << "{\n  \"ftd_id\": \"FTD-0625\",\n"
       << "  \"protocol_sha256\": \"" << protocol_sha256 << "\",\n"
       << "  \"parent_result_sha256\": \"" << parent_sha256 << "\",\n"
       << "  \"verdict\": \"" << summary.verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"coverage_pass\": " << summary.coverage_pass << ",\n"
       << "  \"execution_pass\": " << summary.execution_pass << ",\n"
       << "  \"symmetry_pass\": " << summary.symmetry_pass << ",\n"
       << "  \"covariance_pass\": " << summary.covariance_pass << ",\n"
       << "  \"family_one_pass\": " << summary.family_one_pass << ",\n"
       << "  \"family_four_pass\": " << summary.family_four_pass << ",\n"
       << "  \"beta\": " << summary.beta << ",\n"
       << "  \"baseline_margin\": ";
  write_number(json,summary.baseline_margin);
  json << ",\n  \"symmetry_residual\": ";
  write_number(json,summary.symmetry_residual);
  json << ",\n  \"covariance_residual\": ";
  write_number(json,summary.covariance_residual);
  json << ",\n  \"worst_common_residual\": ";
  write_number(json,summary.worst_common);
  json << ",\n  \"worst_energy_drift\": ";
  write_number(json,summary.worst_energy_drift);
  json << ",\n  \"worst_recovery\": ";
  write_number(json,summary.worst_recovery);
  json << "\n}\n";

  std::ofstream arms(directory/
      "ftd_0625_connected_block_dynamic_stabilization_arms_v1.csv");
  arms << "ftd_id,label,orientation,phase_axis,rotation_axis,energy_multiple,sign,cyclic,init,amplitude_pass,forward_complete,reverse,coherence,exact_steps,candidate,failure_tick,failure_solve_converged,failure_site_projection,failure_graph,failure_root_residual,failure_conflicts,failure_same_polarity_pairs,failure_opposite_polarity_pairs,total_hops,max_conflicts,barrier,amplitude,amplitude_residual,initial_momentum,initial_angular_momentum,final_angular_momentum,min_chart_margin,max_half_distance,max_shape,max_strain,max_common,max_energy_drift,recovery,final_dx,final_dy,final_dz,final_px,final_py,final_pz\n";
  for (const auto& arm : summary.arms)
    arms << std::setprecision(17) << "FTD-0625," << arm.spec.label << ','
         << arm.spec.orientation << ',' << arm.spec.phase_axis << ','
         << arm.spec.rotation_axis << ',' << arm.spec.energy_multiple << ','
         << arm.spec.circulation_sign << ',' << arm.spec.cyclic << ','
         << arm.initialization_pass << ',' << arm.amplitude_pass << ','
         << arm.forward_complete << ',' << arm.reverse_pass << ','
         << arm.coherence_pass << ',' << arm.exact_steps_pass << ','
         << arm.candidate_pass << ',' << arm.failure_tick << ','
         << arm.failure_solve_converged << ','
         << arm.failure_site_projection << ',' << arm.failure_graph << ','
         << arm.failure_root_residual << ',' << arm.failure_conflicts << ','
         << arm.failure_same_polarity_pairs << ','
         << arm.failure_opposite_polarity_pairs << ','
         << arm.total_hops << ',' << arm.maximum_conflicts << ','
         << arm.barrier << ',' << arm.amplitude << ','
         << arm.amplitude_residual << ',' << arm.initial_momentum << ','
         << arm.initial_angular_momentum << ','
         << arm.final_angular_momentum << ',' << arm.minimum_chart_margin
         << ',' << arm.maximum_half_distance << ',' << arm.maximum_shape
         << ',' << arm.maximum_strain << ',' << arm.maximum_common << ','
         << arm.maximum_energy_drift << ',' << arm.recovery << ','
         << arm.final_displacement.x << ',' << arm.final_displacement.y << ','
         << arm.final_displacement.z << ',' << arm.final_momentum.x << ','
         << arm.final_momentum.y << ',' << arm.final_momentum.z << '\n';

  std::ofstream ticks(directory/
      "ftd_0625_connected_block_dynamic_stabilization_ticks_v1.csv");
  ticks << "ftd_id,label,tick,hops,conflicts,dx,dy,dz,px,py,pz,chart_margin,half_distance,angular_momentum,shape,strain,kinetic,binding,field,total,energy_drift,common_residual,local_defect,spline_defect\n";
  for (const auto& arm : summary.arms) for (const auto& tick : arm.history)
    ticks << std::setprecision(17) << "FTD-0625," << arm.spec.label << ','
          << tick.tick << ',' << tick.hops << ',' << tick.conflicts << ','
          << tick.displacement.x << ',' << tick.displacement.y << ','
          << tick.displacement.z << ',' << tick.momentum.x << ','
          << tick.momentum.y << ',' << tick.momentum.z << ','
          << tick.chart_margin << ',' << tick.half_distance << ','
          << tick.angular_momentum << ',' << tick.shape << ',' << tick.strain
          << ',' << tick.kinetic << ',' << tick.binding << ',' << tick.field
          << ',' << tick.total << ',' << tick.energy_drift << ','
          << tick.common_residual << ',' << tick.local_defect << ','
          << tick.spline_defect << '\n';
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  Summary summary;
  summary.parent_pass = parent_fingerprint();
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  summary.normalization_pass = normalization.valid;
  summary.beta = normalization.mapped_field_work_coefficient;
  ftd::eft::ConnectedMooreBlockOptions options;
  options.gate_tolerance = action_gate;
  options.solve_tolerance = 2e-11;
  options.max_iterations = 48;
  const std::vector<ArmSpec> specs{
      {"near_half_zero",0,0,2,0,0,false},
      {"circulation_positive_1B",0,0,2,1,+1,false},
      {"circulation_negative_1B",0,0,2,1,-1,false},
      {"circulation_positive_4B",0,0,2,4,+1,false},
      {"circulation_negative_4B",0,0,2,4,-1,false},
      {"cyclic_positive_1B",1,1,0,1,+1,true},
      {"cyclic_positive_4B",1,1,0,4,+1,true}};
  if (summary.parent_pass && summary.normalization_pass)
    for (const auto& spec : specs) {
      std::cout << "running " << spec.label << std::endl;
      summary.arms.push_back(run_arm(spec,options,summary.beta));
    }
  evaluate(summary);
  write_records(summary);
  std::cout << "protocol_sha256=" << protocol_sha256 << '\n'
            << "verdict=" << summary.verdict << '\n'
            << "execution=" << summary.execution_pass
            << " family1=" << summary.family_one_pass
            << " family4=" << summary.family_four_pass
            << " symmetry=" << summary.symmetry_pass
            << " covariance=" << summary.covariance_pass
            << " baseline_margin=" << summary.baseline_margin << '\n';
  for (const auto& arm : summary.arms)
    std::cout << arm.spec.label << " forward=" << arm.forward_complete
              << " reverse=" << arm.reverse_pass
              << " candidate=" << arm.candidate_pass
              << " failure_tick=" << arm.failure_tick
              << " solve=" << arm.failure_solve_converged
              << " site=" << arm.failure_site_projection
              << " graph=" << arm.failure_graph
              << " root=" << arm.failure_root_residual
              << " conflicts=" << arm.failure_conflicts
              << " same=" << arm.failure_same_polarity_pairs
              << " opposite=" << arm.failure_opposite_polarity_pairs
              << " amplitude=" << arm.amplitude
              << " margin=" << arm.minimum_chart_margin
              << " half_distance=" << arm.maximum_half_distance
              << " L0=" << arm.initial_angular_momentum
              << " L1=" << arm.final_angular_momentum
              << " shape=" << arm.maximum_shape
              << " recovery=" << arm.recovery << '\n';
  return summary.verdict
      == "CONNECTED_DYNAMIC_STABILIZATION_EXECUTION_INVALID" ? 1 : 0;
}
