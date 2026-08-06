// FTD-0626: connected Moore block under the already priced chart fibre.

#include "ftd/eft/connected_moore_block_action.h"
#include "ftd/eft/ternary_block_bipole_peierls.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <future>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <vector>

namespace {

using ftd::Vec3;
constexpr char protocol_sha256[] =
    "67806EA9B3D8ED02B2BF04A839B21E1053FDE1199DE46FB2E064D6E061544C52";
constexpr int L = 17, width = 2, tick_count = 16;
constexpr double epsilon = 1.0/64.0, action_gate = 1e-10;

struct ArmSpec {
  std::string label;
  int orientation = 0, phase_axis = 0, rotation_axis = 2;
  double phase = 0.5;
  int energy_multiple = 0, circulation_sign = 0;
  bool exact_rest = false;
};

struct Tick {
  Vec3 displacement{}, momentum{};
  int multiplicity = 1, pairs = 0, opposite_pairs = 0;
  double separation = INFINITY, angular = 0.0, shape = 0.0;
  double strain = 0.0, total = 0.0, drift = 0.0, common = 0.0;
};

struct Arm {
  ArmSpec spec{};
  bool initialization = false, amplitude_ok = false, forward = false;
  bool reverse = false, exact = false, fibre = false, stationary = false;
  bool metadata = false, qualified = false;
  int failure_tick = 0, hops = 0, shared_states = 0, max_multiplicity = 1;
  double barrier = INFINITY, amplitude = INFINITY;
  double amplitude_residual = INFINITY, initial_momentum = INFINITY;
  double initial_angular = 0.0, final_angular = 0.0;
  double min_separation = INFINITY, max_displacement = 0.0;
  double max_momentum = 0.0, max_shape = 0.0, max_strain = 0.0;
  double max_common = 0.0, max_drift = 0.0, recovery = INFINITY;
  Vec3 final_displacement{}, final_momentum{};
  std::vector<Tick> ticks;
};

struct Regression {
  std::string label;
  bool pass = false, converged = false, site_rejected = false;
  bool graph = false;
  int expected_tick = 0, failure_tick = 0, same_pairs = 0, opposite_pairs = 0;
  double residual = INFINITY;
};

struct Summary {
  bool parents = false, normalization = false, coverage = false;
  bool regression = false, execution = false, rest = false, motion = false;
  bool fibre_exercised = false, symmetry = false, covariance = false;
  double beta = 0.0, symmetry_residual = INFINITY;
  double covariance_residual = INFINITY, worst_common = 0.0;
  double worst_drift = 0.0, worst_recovery = 0.0;
  std::string verdict;
  std::vector<Arm> arms;
  std::vector<Regression> regressions;
};

double component(const Vec3& v, int axis) {
  return axis == 0 ? v.x : (axis == 1 ? v.y : v.z);
}

Vec3 axis_vector(int axis) {
  return axis == 0 ? Vec3{1,0,0}
      : (axis == 1 ? Vec3{0,1,0} : Vec3{0,0,1});
}

Vec3 cycle(const Vec3& v) { return {v.z,v.x,v.y}; }

double max_component(const Vec3& v) {
  return std::max({std::abs(v.x),std::abs(v.y),std::abs(v.z)});
}

double relative(double a, double b) {
  return std::abs(a-b)/std::max({1e-300,std::abs(a),std::abs(b)});
}

Vec3 position(const ftd::eft::MatchedMatterPoint& p) {
  return {p.anchor.x+p.remainder.x,p.anchor.y+p.remainder.y,
          p.anchor.z+p.remainder.z};
}

Vec3 center(const ftd::eft::ConnectedMooreBlockState& state) {
  Vec3 value{};
  for (const auto& p : state.constituents) value += position(p);
  return value*(1.0/static_cast<double>(state.constituents.size()));
}

Vec3 momentum(const ftd::eft::ConnectedMooreBlockState& state) {
  Vec3 value{};
  for (const auto& p : state.constituents) value += p.momentum;
  return value;
}

double kinetic(const ftd::eft::ConnectedMooreBlockState& state) {
  long double value = 0.0L;
  for (const auto& p : state.constituents)
    value += ftd::eft::production_flat_energy_from_momentum(p.momentum);
  return static_cast<double>(value);
}

double total(const ftd::eft::ConnectedMooreBlockState& state,
             const ftd::eft::ConnectedMooreBlockOptions& options,
             double beta) {
  return kinetic(state)+ftd::eft::connected_moore_block_binding_energy(
      state,options)+beta*ftd::eft::matched_modified_energy(
          state.electric,state.magnetic_half,options.wave_speed*options.dt);
}

double angular(const ftd::eft::ConnectedMooreBlockState& state, int axis) {
  const Vec3 c = center(state);
  Vec3 value{};
  for (const auto& p : state.constituents)
    value += Vec3::cross(position(p)-c,p.momentum);
  return component(value,axis);
}

double shape(const ftd::eft::ConnectedMooreBlockState& initial,
             const ftd::eft::ConnectedMooreBlockState& state) {
  const Vec3 a = center(initial), b = center(state);
  long double sum = 0.0L;
  for (std::size_t i = 0; i < state.constituents.size(); ++i) {
    const Vec3 d = (position(state.constituents[i])-b)
        -(position(initial.constituents[i])-a);
    sum += d.dot(d);
  }
  return std::sqrt(static_cast<double>(
      sum/static_cast<long double>(state.constituents.size())));
}

std::tuple<int,int,int,double> fibre_metrics(
    const ftd::eft::ConnectedMooreBlockState& state) {
  std::map<std::tuple<int,int,int>,std::vector<std::size_t>> grouped;
  for (std::size_t i = 0; i < state.constituents.size(); ++i) {
    const auto& a = state.constituents[i].anchor;
    grouped[{a.x,a.y,a.z}].push_back(i);
  }
  int maximum = 1, pairs = 0, opposite = 0;
  double minimum = INFINITY;
  for (const auto& group : grouped) {
    maximum = std::max(maximum,static_cast<int>(group.second.size()));
    for (std::size_t i = 0; i < group.second.size(); ++i)
      for (std::size_t j = i+1; j < group.second.size(); ++j) {
        ++pairs;
        const auto a = group.second[i], b = group.second[j];
        if (state.charges[a] != state.charges[b]) ++opposite;
        minimum = std::min(minimum,
            (position(state.constituents[a])-position(state.constituents[b]))
                .mag());
      }
  }
  return {maximum,pairs,opposite,minimum};
}

bool metadata_equal(const ftd::eft::ConnectedMooreBlockState& a,
                    const ftd::eft::ConnectedMooreBlockState& b) {
  if (a.width != b.width || a.orientation_axis != b.orientation_axis
      || a.charges != b.charges || a.constituents.size() != b.constituents.size()
      || a.edges.size() != b.edges.size()) return false;
  for (std::size_t i = 0; i < a.edges.size(); ++i) {
    const auto& x = a.edges[i];
    const auto& y = b.edges[i];
    if (x.first != y.first || x.second != y.second
        || x.reference_delta.x != y.reference_delta.x
        || x.reference_delta.y != y.reference_delta.y
        || x.reference_delta.z != y.reference_delta.z
        || x.rest_length_squared != y.rest_length_squared) return false;
  }
  return true;
}

double common_residual(const ftd::eft::ConnectedMooreBlockStepResult& s) {
  return std::max({s.root_residual,s.continuity_residual,
      s.gauss_before_residual,s.gauss_after_residual,s.force_residual,
      s.kinematic_residual,s.kinetic_discrete_gradient_residual,
      s.electric_adjoint_residual,s.magnetic_work_residual,
      s.binding_work_residual,s.binding_impulse_sum_residual,
      s.matter_work_residual,s.field_work_residual,s.total_energy_residual,
      s.causal_speed_excess});
}

bool parent_fingerprints() {
  const auto root = std::filesystem::path(__FILE__).parent_path().parent_path();
  const std::vector<std::pair<std::string,std::string>> items{
      {"results/ftd_0609/ftd_0609_shared_anchor_fibre_v1.json",
       "SHARED_ANCHOR_FIBRE_TRANSPORT_CLOSED_NEGATIVE"},
      {"results/ftd_0624/ftd_0624_connected_block_translation_stability_v1.json",
       "CONNECTED_TRANSLATION_STABILITY_EXECUTION_INVALID"},
      {"results/ftd_0625/ftd_0625_connected_block_dynamic_stabilization_v1.json",
       "RIGID_CIRCULATION_DYNAMIC_STABILIZATION_CLOSED_NEGATIVE"}};
  for (const auto& item : items) {
    std::ifstream in(root/item.first,std::ios::binary);
    const std::string bytes((std::istreambuf_iterator<char>(in)),{});
    if (bytes.find(item.second) == std::string::npos) return false;
  }
  return true;
}

std::vector<Vec3> circulation_basis(
    const ftd::eft::ConnectedMooreBlockState& state, int axis) {
  const Vec3 c = center(state), normal = axis_vector(axis);
  std::vector<Vec3> result;
  for (const auto& p : state.constituents)
    result.push_back(Vec3::cross(normal,position(p)-c));
  return result;
}

double excess_kinetic(const std::vector<Vec3>& basis, double amplitude) {
  const double rest = ftd::eft::production_flat_energy_from_momentum({});
  long double result = 0.0L;
  for (const Vec3& b : basis)
    result += ftd::eft::production_flat_energy_from_momentum(b*amplitude)-rest;
  return static_cast<double>(result);
}

std::pair<double,double> solve_amplitude(
    const std::vector<Vec3>& basis, double target) {
  if (target == 0.0) return {0.0,0.0};
  double low = 0.0, high = 1.0;
  while (excess_kinetic(basis,high) < target && high < 1e6) high *= 2.0;
  if (!(excess_kinetic(basis,high) >= target)) return {INFINITY,INFINITY};
  for (int i = 0; i < 160; ++i) {
    const double middle = 0.5*(low+high);
    if (excess_kinetic(basis,middle) < target) low = middle;
    else high = middle;
  }
  const double value = 0.5*(low+high);
  return {value,std::abs(excess_kinetic(basis,value)-target)};
}

bool prepare(const ArmSpec& spec,
             const ftd::eft::ConnectedMooreBlockOptions& options,
             double beta, ftd::eft::ConnectedMooreBlockState& state,
             double& barrier, double& amplitude, double& amplitude_residual) {
  const auto initialization = ftd::eft::initialize_connected_moore_block(
      L,width,spec.orientation,spec.phase_axis,spec.phase);
  if (!initialization.valid || initialization.state.constituents.size() != 16
      || initialization.state.edges.size() != 72
      || initialization.poisson_residual > 1e-11
      || initialization.gauss_residual > 1e-11) return false;
  state = initialization.state;
  const auto spectrum = ftd::eft::evaluate_ternary_block_bipole_peierls(
      L,width,spec.orientation,beta);
  barrier = spectrum.peierls_coefficient[spec.phase_axis]/16.0;
  const auto basis = circulation_basis(state,spec.rotation_axis);
  const auto root = solve_amplitude(basis,spec.energy_multiple*barrier);
  amplitude = root.first;
  amplitude_residual = root.second;
  if (!spectrum.valid || !(barrier > 0.0) || !std::isfinite(amplitude)
      || amplitude_residual > 1e-13) return false;
  for (std::size_t i = 0; i < state.constituents.size(); ++i)
    state.constituents[i].momentum = basis[i]
        *(spec.circulation_sign*amplitude);
  return momentum(state).mag() <= 1e-14 && total(state,options,beta) > 0.0;
}

Arm run_arm(const ArmSpec& spec,
            const ftd::eft::ConnectedMooreBlockOptions& options,
            double beta) {
  Arm result;
  result.spec = spec;
  ftd::eft::ConnectedMooreBlockState initial(L);
  result.initialization = prepare(spec,options,beta,initial,result.barrier,
      result.amplitude,result.amplitude_residual);
  result.amplitude_ok = result.initialization
      && result.amplitude_residual <= 1e-13;
  if (!result.amplitude_ok) return result;
  result.initial_momentum = momentum(initial).mag();
  result.initial_angular = angular(initial,spec.rotation_axis);
  result.metadata = result.exact = true;
  auto state = initial;
  const Vec3 c0 = center(initial);
  const double energy0 = total(initial,options,beta);
  for (int tick = 1; tick <= tick_count; ++tick) {
    const auto step = ftd::eft::solve_connected_moore_block_forward(
        state,options);
    const double common = common_residual(step);
    result.max_common = std::max(result.max_common,common);
    if (!step.common_action_gates_pass || common > action_gate
        || !step.graph_connected || !step.graph_local) {
      result.failure_tick = tick;
      result.exact = false;
      break;
    }
    result.metadata = result.metadata && metadata_equal(initial,step.later);
    state = step.later;
    int multiplicity = 1, pairs = 0, opposite = 0;
    double separation = INFINITY;
    std::tie(multiplicity,pairs,opposite,separation) = fibre_metrics(state);
    if (pairs > 0) ++result.shared_states;
    result.max_multiplicity = std::max(result.max_multiplicity,multiplicity);
    if (std::isfinite(separation))
      result.min_separation = std::min(result.min_separation,separation);
    const Vec3 displacement = center(state)-c0, p = momentum(state);
    const double shape_value = shape(initial,state);
    const double energy = total(state,options,beta);
    const double drift = std::abs(energy-energy0);
    result.hops += step.site_hops;
    result.max_displacement = std::max(result.max_displacement,
                                        displacement.mag());
    result.max_momentum = std::max(result.max_momentum,p.mag());
    result.max_shape = std::max(result.max_shape,shape_value);
    result.max_strain = std::max(result.max_strain,step.maximum_edge_strain);
    result.max_drift = std::max(result.max_drift,drift);
    result.ticks.push_back({displacement,p,multiplicity,pairs,opposite,
        separation,angular(state,spec.rotation_axis),shape_value,
        step.maximum_edge_strain,energy,drift,common});
  }
  result.forward = result.exact && result.ticks.size() == tick_count;
  result.fibre = result.forward && result.metadata
      && result.max_multiplicity <= 2
      && (result.shared_states == 0
          || (std::isfinite(result.min_separation)
              && result.min_separation >= 1e-3));
  result.stationary = !spec.exact_rest || (result.forward
      && result.max_displacement <= 1e-8 && result.max_momentum <= 1e-8
      && result.max_shape <= 1e-8 && result.max_strain <= 1e-8);
  result.final_displacement = center(state)-c0;
  result.final_momentum = momentum(state);
  result.final_angular = angular(state,spec.rotation_axis);

  result.reverse = result.forward;
  for (int tick = tick_count; result.reverse && tick >= 1; --tick) {
    const auto step = ftd::eft::solve_connected_moore_block_reverse(
        state,options);
    const double common = common_residual(step);
    result.max_common = std::max(result.max_common,common);
    int multiplicity = 1, pairs = 0, opposite = 0;
    double separation = INFINITY;
    if (step.common_action_gates_pass)
      std::tie(multiplicity,pairs,opposite,separation) =
          fibre_metrics(step.earlier);
    if (!step.common_action_gates_pass || common > action_gate
        || !step.graph_connected || !step.graph_local || multiplicity > 2
        || (pairs > 0 && separation < 1e-3)
        || !metadata_equal(initial,step.earlier)) result.reverse = false;
    else state = step.earlier;
  }
  if (result.reverse)
    result.recovery = ftd::eft::connected_moore_block_state_max_difference(
        initial,state);
  result.reverse = result.reverse && result.recovery <= 1e-8;
  result.qualified = result.forward && result.reverse && result.fibre
      && result.stationary && result.metadata && result.max_common <= action_gate
      && result.max_drift <= 1e-9;
  return result;
}

Regression run_regression(
    const ArmSpec& spec, int expected_tick,
    const ftd::eft::ConnectedMooreBlockOptions& options, double beta) {
  Regression result;
  result.label = spec.label;
  result.expected_tick = expected_tick;
  ftd::eft::ConnectedMooreBlockState state(L);
  double barrier = 0.0, amplitude = 0.0, amplitude_residual = 0.0;
  if (!prepare(spec,options,beta,state,barrier,amplitude,
               amplitude_residual)) return result;
  for (int tick = 1; tick <= tick_count; ++tick) {
    const auto step = ftd::eft::solve_connected_moore_block_forward(
        state,options);
    if (!step.common_action_gates_pass) {
      result.failure_tick = tick;
      result.converged = step.solve.converged;
      result.site_rejected = !step.site_projection_valid;
      result.graph = step.graph_connected && step.graph_local;
      result.residual = step.root_residual;
      int multiplicity = 1, pairs = 0;
      double separation = INFINITY;
      std::tie(multiplicity,pairs,result.opposite_pairs,separation) =
          fibre_metrics(step.later);
      result.same_pairs = pairs-result.opposite_pairs;
      break;
    }
    state = step.later;
  }
  result.pass = result.failure_tick == expected_tick && result.converged
      && result.site_rejected && result.graph && result.residual <= action_gate
      && result.same_pairs == 0 && result.opposite_pairs > 0;
  return result;
}

const Arm* find(const Summary& summary, const std::string& label) {
  const auto item = std::find_if(summary.arms.begin(),summary.arms.end(),
      [&](const Arm& arm) { return arm.spec.label == label; });
  return item == summary.arms.end() ? nullptr : &*item;
}

double signed_symmetry(const Arm& a, const Arm& b) {
  if (a.ticks.size() != b.ticks.size()) return INFINITY;
  double result = 0.0;
  for (std::size_t i = 0; i < a.ticks.size(); ++i) {
    const auto& x = a.ticks[i];
    const auto& y = b.ticks[i];
    result = std::max({result,std::abs(x.total-y.total),
        std::abs(x.shape-y.shape),std::abs(x.strain-y.strain),
        std::abs(x.separation-y.separation),std::abs(x.angular+y.angular),
        static_cast<double>(std::abs(x.multiplicity-y.multiplicity)),
        static_cast<double>(std::abs(x.pairs-y.pairs))});
  }
  return result;
}

double cyclic_covariance(const Arm& a, const Arm& b) {
  if (a.ticks.size() != b.ticks.size()) return INFINITY;
  double result = 0.0;
  for (std::size_t i = 0; i < a.ticks.size(); ++i) {
    const auto& x = a.ticks[i];
    const auto& y = b.ticks[i];
    result = std::max({result,max_component(y.displacement-cycle(x.displacement)),
        max_component(y.momentum-cycle(x.momentum)),relative(y.total,x.total),
        relative(y.shape,x.shape),relative(y.strain,x.strain),
        std::abs(y.separation-x.separation),std::abs(y.angular-x.angular),
        static_cast<double>(std::abs(y.multiplicity-x.multiplicity)),
        static_cast<double>(std::abs(y.pairs-x.pairs))});
  }
  return result;
}

void evaluate(Summary& s) {
  s.coverage = s.arms.size() == 9 && s.regressions.size() == 2;
  s.regression = s.regressions.size() == 2
      && std::all_of(s.regressions.begin(),s.regressions.end(),
          [](const Regression& r) { return r.pass; });
  s.execution = s.parents && s.normalization && s.coverage && s.regression;
  for (const auto& arm : s.arms) {
    s.execution = s.execution && arm.initialization && arm.amplitude_ok;
    s.worst_common = std::max(s.worst_common,arm.max_common);
    s.worst_drift = std::max(s.worst_drift,arm.max_drift);
    if (std::isfinite(arm.recovery))
      s.worst_recovery = std::max(s.worst_recovery,arm.recovery);
  }
  s.fibre_exercised = std::any_of(s.arms.begin(),s.arms.end(),
      [](const Arm& a) { return a.shared_states > 0; });
  const auto *rx=find(s,"exact_half_rest_x"), *ry=find(s,"exact_half_rest_y");
  s.rest = s.fibre_exercised && rx && ry && rx->qualified && ry->qualified;
  const auto *p1=find(s,"circulation_positive_1B");
  const auto *n1=find(s,"circulation_negative_1B");
  const auto *p4=find(s,"circulation_positive_4B");
  const auto *n4=find(s,"circulation_negative_4B");
  const auto *c1=find(s,"cyclic_positive_1B");
  const auto *c4=find(s,"cyclic_positive_4B");
  s.symmetry_residual = p1 && n1 && p4 && n4
      ? std::max(signed_symmetry(*p1,*n1),signed_symmetry(*p4,*n4))
      : INFINITY;
  s.covariance_residual = p1 && p4 && c1 && c4
      ? std::max(cyclic_covariance(*p1,*c1),cyclic_covariance(*p4,*c4))
      : INFINITY;
  s.symmetry = s.symmetry_residual <= 1e-8;
  s.covariance = s.covariance_residual <= 1e-8;
  s.motion = s.symmetry && s.covariance
      && std::all_of(s.arms.begin(),s.arms.end(),[](const Arm& a) {
           return a.spec.exact_rest || a.qualified;
         });
  if (!s.execution) s.verdict = "CONNECTED_BLOCK_FIBRE_EXECUTION_INVALID";
  else if (s.rest && s.motion)
    s.verdict = "CONNECTED_BLOCK_FIBRE_REST_AND_MOTION_CONSTRUCTIVE";
  else if (s.rest)
    s.verdict = "CONNECTED_BLOCK_FIBRE_REST_CONSTRUCTIVE_MOTION_OPEN";
  else s.verdict = "CONNECTED_BLOCK_FIBRE_CLOSED_NEGATIVE";
}

void number(std::ostream& out, double value) {
  if (std::isfinite(value)) out << value; else out << "null";
}

void write_records(const Summary& s) {
  const auto dir = std::filesystem::path(__FILE__).parent_path().parent_path()
      /"results/ftd_0626";
  std::filesystem::create_directories(dir);
  std::ofstream json(dir/"ftd_0626_connected_block_shared_anchor_fibre_v1.json");
  json << std::setprecision(17) << "{\n  \"ftd_id\": \"FTD-0626\",\n"
       << "  \"protocol_sha256\": \"" << protocol_sha256 << "\",\n"
       << "  \"verdict\": \"" << s.verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"shared_anchor_option_default\": false,\n"
       << "  \"coverage_pass\": " << s.coverage << ",\n"
       << "  \"execution_pass\": " << s.execution << ",\n"
       << "  \"default_false_regression_pass\": " << s.regression << ",\n"
       << "  \"fibre_exercised\": " << s.fibre_exercised << ",\n"
       << "  \"rest_pass\": " << s.rest << ",\n"
       << "  \"motion_pass\": " << s.motion << ",\n"
       << "  \"symmetry_residual\": "; number(json,s.symmetry_residual);
  json << ",\n  \"covariance_residual\": "; number(json,s.covariance_residual);
  json << ",\n  \"worst_common_residual\": " << s.worst_common
       << ",\n  \"worst_energy_drift\": " << s.worst_drift
       << ",\n  \"worst_recovery\": " << s.worst_recovery << "\n}\n";

  std::ofstream arms(dir/"ftd_0626_connected_block_shared_anchor_fibre_arms_v1.csv");
  arms << "ftd_id,label,exact_rest,init,amplitude_ok,forward,reverse,exact,fibre,stationary,metadata,qualified,failure_tick,hops,shared_states,max_multiplicity,min_separation,barrier,amplitude,amplitude_residual,initial_momentum,initial_angular,final_angular,max_displacement,max_momentum,max_shape,max_strain,max_common,max_drift,recovery,dx,dy,dz,px,py,pz\n";
  for (const auto& a : s.arms)
    arms << std::setprecision(17) << "FTD-0626," << a.spec.label << ','
         << a.spec.exact_rest << ',' << a.initialization << ','
         << a.amplitude_ok << ',' << a.forward << ',' << a.reverse << ','
         << a.exact << ',' << a.fibre << ',' << a.stationary << ','
         << a.metadata << ',' << a.qualified << ',' << a.failure_tick << ','
         << a.hops << ',' << a.shared_states << ',' << a.max_multiplicity << ','
         << a.min_separation << ',' << a.barrier << ',' << a.amplitude << ','
         << a.amplitude_residual << ',' << a.initial_momentum << ','
         << a.initial_angular << ',' << a.final_angular << ','
         << a.max_displacement << ',' << a.max_momentum << ',' << a.max_shape
         << ',' << a.max_strain << ',' << a.max_common << ',' << a.max_drift
         << ',' << a.recovery << ',' << a.final_displacement.x << ','
         << a.final_displacement.y << ',' << a.final_displacement.z << ','
         << a.final_momentum.x << ',' << a.final_momentum.y << ','
         << a.final_momentum.z << '\n';

  std::ofstream ticks(dir/"ftd_0626_connected_block_shared_anchor_fibre_ticks_v1.csv");
  ticks << "ftd_id,label,tick,multiplicity,pairs,opposite_pairs,separation,dx,dy,dz,px,py,pz,angular,shape,strain,total,drift,common\n";
  for (const auto& a : s.arms) for (std::size_t i=0;i<a.ticks.size();++i) {
    const auto& t=a.ticks[i];
    ticks << std::setprecision(17) << "FTD-0626," << a.spec.label << ','
          << i+1 << ',' << t.multiplicity << ',' << t.pairs << ','
          << t.opposite_pairs << ',' << t.separation << ','
          << t.displacement.x << ',' << t.displacement.y << ','
          << t.displacement.z << ',' << t.momentum.x << ',' << t.momentum.y
          << ',' << t.momentum.z << ',' << t.angular << ',' << t.shape << ','
          << t.strain << ',' << t.total << ',' << t.drift << ',' << t.common
          << '\n';
  }
  std::ofstream regressions(dir/"ftd_0626_connected_block_shared_anchor_fibre_regressions_v1.csv");
  regressions << "ftd_id,label,pass,expected_tick,failure_tick,converged,site_rejected,graph,residual,same_pairs,opposite_pairs\n";
  for (const auto& r : s.regressions)
    regressions << std::setprecision(17) << "FTD-0626," << r.label << ','
        << r.pass << ',' << r.expected_tick << ',' << r.failure_tick << ','
        << r.converged << ',' << r.site_rejected << ',' << r.graph << ','
        << r.residual << ',' << r.same_pairs << ',' << r.opposite_pairs << '\n';
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  Summary s;
  s.parents = parent_fingerprints();
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  s.normalization = normalization.valid;
  s.beta = normalization.mapped_field_work_coefficient;
  ftd::eft::ConnectedMooreBlockOptions fibre;
  fibre.gate_tolerance=action_gate; fibre.solve_tolerance=2e-11;
  fibre.max_iterations=48; fibre.allow_shared_anchor_chart=true;
  auto strict=fibre; strict.allow_shared_anchor_chart=false;
  const std::vector<ArmSpec> specs{
      {"exact_half_rest_x",0,0,2,0.5,0,0,true},
      {"exact_half_rest_y",1,1,0,0.5,0,0,true},
      {"near_half_zero",0,0,2,0.5-epsilon,0,0,false},
      {"circulation_positive_1B",0,0,2,0.5-epsilon,1,+1,false},
      {"circulation_negative_1B",0,0,2,0.5-epsilon,1,-1,false},
      {"circulation_positive_4B",0,0,2,0.5-epsilon,4,+1,false},
      {"circulation_negative_4B",0,0,2,0.5-epsilon,4,-1,false},
      {"cyclic_positive_1B",1,1,0,0.5-epsilon,1,+1,false},
      {"cyclic_positive_4B",1,1,0,0.5-epsilon,4,+1,false}};
  if (s.parents && s.normalization) {
    auto rest_regression = std::async(std::launch::async,[&]() {
      return run_regression(specs[0],1,strict,s.beta);
    });
    auto circulation_regression = std::async(std::launch::async,[&]() {
      return run_regression(specs[3],2,strict,s.beta);
    });
    std::vector<std::future<Arm>> futures;
    for (const auto& spec : specs)
      futures.push_back(std::async(std::launch::async,[&,spec]() {
        return run_arm(spec,fibre,s.beta);
      }));
    s.regressions.push_back(rest_regression.get());
    s.regressions.push_back(circulation_regression.get());
    for (std::size_t i = 0; i < specs.size(); ++i) {
      std::cout << "completed " << specs[i].label << std::endl;
      s.arms.push_back(futures[i].get());
    }
  }
  evaluate(s);
  write_records(s);
  std::cout << "protocol_sha256=" << protocol_sha256 << '\n'
            << "verdict=" << s.verdict << '\n'
            << "execution=" << s.execution << " regression=" << s.regression
            << " rest=" << s.rest << " motion=" << s.motion
            << " symmetry=" << s.symmetry_residual
            << " covariance=" << s.covariance_residual << '\n';
  for (const auto& a : s.arms)
    std::cout << a.spec.label << " forward=" << a.forward
              << " reverse=" << a.reverse << " fibre=" << a.fibre
              << " stationary=" << a.stationary
              << " qualified=" << a.qualified
              << " fail_tick=" << a.failure_tick
              << " shared=" << a.shared_states
              << " mult=" << a.max_multiplicity
              << " min_sep=" << a.min_separation
              << " disp=" << a.max_displacement
              << " shape=" << a.max_shape << " strain=" << a.max_strain
              << " common=" << a.max_common << " drift=" << a.max_drift
              << " recovery=" << a.recovery << '\n';
  return s.verdict == "CONNECTED_BLOCK_FIBRE_EXECUTION_INVALID" ? 1 : 0;
}
