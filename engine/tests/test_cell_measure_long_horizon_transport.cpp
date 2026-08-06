// FTD-0650: long-horizon transport of one fixed-mass cell-measure object.

#include "ftd/eft/connected_moore_block_action.h"
#include "ftd/eft/production_hop_kinematics.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <future>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace {

constexpr char protocol_sha256[] =
    "2670F4B0E1C67911D85FDC80DE64F5DFB15EC54F7B76E3C20882AD66F93CD131";
constexpr double physical_horizon = 32.0;
constexpr int fibre_limit = 8;

using ftd::Coord;
using ftd::Vec3;
using ftd::eft::ConnectedMooreBlockState;
using ftd::eft::ConnectedMooreBlockStepResult;

struct Spec {
  std::string label;
  std::string kind;
  std::string family;
  int width = 0;
  int orientation = 0;
  int rotation_maps = 0;
  Vec3 direction{};
  double speed = 0.0;
};

struct TickRecord {
  std::string phase;
  int tick = 0;
  std::uint64_t state_hash = 0;
  bool valid = false;
  bool graph_connected = false;
  bool graph_local = false;
  int constituent_count = 0;
  int anchor_multiplicity = 0;
  int site_hops = 0;
  int solve_iterations = 0;
  int krylov_matvecs = 0;
  Vec3 center{};
  Vec3 matter_momentum{};
  Vec3 local_field_momentum{};
  Vec3 spline_field_momentum{};
  double kinetic = INFINITY;
  double binding = INFINITY;
  double field = INFINITY;
  double total = INFINITY;
  double energy_drift = INFINITY;
  double relative_edge_strain = INFINITY;
  double engine_edge_strain = INFINITY;
  double action = INFINITY;
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
};

struct Arm {
  Spec spec;
  bool initialized = false;
  bool forward = false;
  bool reverse = false;
  bool exact = false;
  bool coherent = false;
  bool persistent = false;
  int ticks = 0;
  int constituent_count = 0;
  int total_hops = 0;
  int maximum_anchor_multiplicity = 0;
  int total_solve_iterations = 0;
  int total_krylov_matvecs = 0;
  double a = NAN;
  double mass_scale = NAN;
  double polarity_scale = NAN;
  double binding_scale = NAN;
  double field_scale = NAN;
  double rest_energy = NAN;
  double inertial_mass = NAN;
  double integrated_positive = NAN;
  double maximum_action = 0.0;
  double maximum_causal = 0.0;
  double maximum_relative_edge_strain = 0.0;
  double maximum_energy_drift = 0.0;
  double recovery = INFINITY;
  double final_energy = INFINITY;
  double normalized_spline_defect = INFINITY;
  double parallel_displacement = NAN;
  double transverse_displacement = NAN;
  double mobility = NAN;
  std::array<double,4> window_advance{{NAN,NAN,NAN,NAN}};
  std::uint64_t initial_hash = 0;
  std::uint64_t final_hash = 0;
  std::uint64_t recovered_hash = 0;
  Vec3 initial_center{};
  Vec3 final_center{};
  Vec3 initial_matter_momentum{};
  Vec3 final_matter_momentum{};
  Vec3 initial_spline_momentum{};
  Vec3 final_spline_momentum{};
  ConnectedMooreBlockState final_state;
  std::vector<TickRecord> records;

  Arm() : final_state(0) {}
};

struct Summary {
  std::vector<Arm> arms;
  bool coverage = false;
  bool execution = false;
  bool exact = false;
  bool coherence = false;
  bool zero = false;
  bool mirror = false;
  bool cubic = false;
  bool transport = false;
  bool mobility_trend = false;
  bool anisotropy_trend = false;
  bool defect_trend = false;
  bool resolution = false;
  int high_persistent = 0;
  int low_persistent = 0;
  double worst_action = 0.0;
  double worst_causal = 0.0;
  double worst_strain = 0.0;
  double worst_recovery = 0.0;
  double worst_zero = 0.0;
  double mirror_residual = 0.0;
  double cubic_residual = 0.0;
  std::map<int,double> minimum_high_mobility;
  std::map<int,double> high_mobility_span;
  std::map<int,double> maximum_high_defect;
  std::string verdict = "CELL_MEASURE_LONG_HORIZON_EXECUTION_INVALID";
};

int wrap(int value, int L) {
  value %= L;
  return value < 0 ? value+L : value;
}

std::size_t index(int L, int x, int y, int z) {
  return static_cast<std::size_t>((wrap(x,L)*L+wrap(y,L))*L+wrap(z,L));
}

Vec3 position(const ftd::eft::MatchedMatterPoint& point) {
  return {point.anchor.x+point.remainder.x,
          point.anchor.y+point.remainder.y,
          point.anchor.z+point.remainder.z};
}

Vec3 center(const ConnectedMooreBlockState& state) {
  Vec3 result{};
  if (state.constituents.empty()) return result;
  for (const auto& point : state.constituents) result += position(point);
  return result*(1.0/static_cast<double>(state.constituents.size()));
}

Vec3 momentum(const ConnectedMooreBlockState& state) {
  Vec3 result{};
  for (const auto& point : state.constituents) result += point.momentum;
  return result;
}

double max_component(const Vec3& value) {
  return std::max({std::abs(value.x),std::abs(value.y),std::abs(value.z)});
}

Vec3 cycle(Vec3 value, int maps = 1) {
  for (int map = 0; map < maps; ++map)
    value = {value.z,value.x,value.y};
  return value;
}

Coord cycle(Coord value, int maps = 1) {
  for (int map = 0; map < maps; ++map)
    value = {value.z,value.x,value.y};
  return value;
}

double action_max(const ConnectedMooreBlockStepResult& step) {
  return std::max({step.root_residual,step.force_residual,
      step.continuity_residual,step.gauss_before_residual,
      step.gauss_after_residual,step.kinematic_residual,
      step.kinetic_discrete_gradient_residual,
      step.electric_adjoint_residual,step.magnetic_work_residual,
      step.binding_work_residual,step.binding_impulse_sum_residual,
      step.matter_work_residual,step.field_work_residual,
      step.total_energy_residual});
}

double relative_edge_strain(const ConnectedMooreBlockState& state) {
  double result = 0.0;
  for (const auto& edge : state.edges) {
    if (!(edge.rest_length_squared > 0.0)) return INFINITY;
    const Vec3 delta = position(state.constituents[edge.first])
        -position(state.constituents[edge.second]);
    result = std::max(result,std::abs(
        std::sqrt(delta.mag2()/edge.rest_length_squared)-1.0));
  }
  return result;
}

int anchor_multiplicity(const ConnectedMooreBlockState& state) {
  int result = 0;
  for (std::size_t a = 0; a < state.constituents.size(); ++a) {
    int count = 0;
    for (const auto& point : state.constituents)
      if (point.anchor.x == state.constituents[a].anchor.x
          && point.anchor.y == state.constituents[a].anchor.y
          && point.anchor.z == state.constituents[a].anchor.z) ++count;
    result = std::max(result,count);
  }
  return result;
}

bool finite_state(const ConnectedMooreBlockState& state) {
  auto finite_field = [](const std::vector<double>& values) {
    return std::all_of(values.begin(),values.end(),
        [](double value) { return std::isfinite(value); });
  };
  if (!finite_field(state.electric.x) || !finite_field(state.electric.y)
      || !finite_field(state.electric.z)
      || !finite_field(state.magnetic_half.x)
      || !finite_field(state.magnetic_half.y)
      || !finite_field(state.magnetic_half.z)) return false;
  for (const auto& point : state.constituents)
    if (!std::isfinite(point.remainder.x)
        || !std::isfinite(point.remainder.y)
        || !std::isfinite(point.remainder.z)
        || !std::isfinite(point.momentum.x)
        || !std::isfinite(point.momentum.y)
        || !std::isfinite(point.momentum.z)) return false;
  return true;
}

void hash_bytes(std::uint64_t& hash, const void* data, std::size_t size) {
  const auto* bytes = static_cast<const unsigned char*>(data);
  for (std::size_t i = 0; i < size; ++i) {
    hash ^= bytes[i];
    hash *= UINT64_C(1099511628211);
  }
}

template <class T>
void hash_value(std::uint64_t& hash, const T& value) {
  hash_bytes(hash,&value,sizeof(value));
}

std::uint64_t state_hash(const ConnectedMooreBlockState& state) {
  std::uint64_t hash = UINT64_C(1469598103934665603);
  hash_value(hash,state.electric.L);
  hash_value(hash,state.width);
  hash_value(hash,state.orientation_axis);
  for (const auto* values : {&state.electric.x,&state.electric.y,
       &state.electric.z,&state.magnetic_half.x,&state.magnetic_half.y,
       &state.magnetic_half.z})
    for (double value : *values) hash_value(hash,value);
  hash_value(hash,state.constituents.size());
  for (const auto& point : state.constituents) {
    hash_value(hash,point.anchor.x); hash_value(hash,point.anchor.y);
    hash_value(hash,point.anchor.z); hash_value(hash,point.remainder.x);
    hash_value(hash,point.remainder.y); hash_value(hash,point.remainder.z);
    hash_value(hash,point.momentum.x); hash_value(hash,point.momentum.y);
    hash_value(hash,point.momentum.z);
  }
  for (int charge : state.charges) hash_value(hash,charge);
  hash_value(hash,state.edges.size());
  for (const auto& edge : state.edges) {
    hash_value(hash,edge.first); hash_value(hash,edge.second);
    hash_value(hash,edge.reference_delta.x);
    hash_value(hash,edge.reference_delta.y);
    hash_value(hash,edge.reference_delta.z);
    hash_value(hash,edge.rest_length_squared);
  }
  return hash;
}

void scale_field(ConnectedMooreBlockState& state, double scale) {
  for (std::size_t i = 0; i < state.electric.x.size(); ++i) {
    state.electric.x[i] *= scale;
    state.electric.y[i] *= scale;
    state.electric.z[i] *= scale;
    state.magnetic_half.x[i] *= scale;
    state.magnetic_half.y[i] *= scale;
    state.magnetic_half.z[i] *= scale;
  }
}

double total_energy(const ConnectedMooreBlockStepResult& step, bool later) {
  return later ? step.kinetic_energy_after+step.binding_energy_after
          +step.field_energy_after
      : step.kinetic_energy_before+step.binding_energy_before
          +step.field_energy_before;
}

TickRecord tick_record(const std::string& phase, int tick,
                       const ConnectedMooreBlockStepResult& step,
                       const ConnectedMooreBlockState& state, bool later,
                       double initial_energy) {
  TickRecord row;
  row.phase = phase;
  row.tick = tick;
  row.state_hash = state_hash(state);
  row.valid = step.valid && step.solve.converged && finite_state(state);
  row.graph_connected = step.graph_connected;
  row.graph_local = step.graph_local;
  row.constituent_count = static_cast<int>(state.constituents.size());
  row.anchor_multiplicity = anchor_multiplicity(state);
  row.site_hops = step.site_hops;
  row.solve_iterations = step.solve.iterations;
  row.krylov_matvecs = step.solve.krylov_matvecs;
  row.center = later ? step.center_after : step.center_before;
  row.matter_momentum = later ? step.matter_momentum_after
                              : step.matter_momentum_before;
  row.local_field_momentum = later ? step.local_field_momentum_after
                                   : step.local_field_momentum_before;
  row.spline_field_momentum = later ? step.spline_field_momentum_after
                                    : step.spline_field_momentum_before;
  row.kinetic = later ? step.kinetic_energy_after : step.kinetic_energy_before;
  row.binding = later ? step.binding_energy_after : step.binding_energy_before;
  row.field = later ? step.field_energy_after : step.field_energy_before;
  row.total = row.kinetic+row.binding+row.field;
  row.energy_drift = std::abs(row.total-initial_energy);
  row.relative_edge_strain = relative_edge_strain(state);
  row.engine_edge_strain = step.maximum_edge_strain;
  row.action = action_max(step);
  row.root = step.root_residual;
  row.force = step.force_residual;
  row.continuity = step.continuity_residual;
  row.gauss_before = step.gauss_before_residual;
  row.gauss_after = step.gauss_after_residual;
  row.kinetic_gradient = step.kinetic_discrete_gradient_residual;
  row.electric_adjoint = step.electric_adjoint_residual;
  row.magnetic_work = step.magnetic_work_residual;
  row.binding_work = step.binding_work_residual;
  row.binding_sum = step.binding_impulse_sum_residual;
  row.matter_work = step.matter_work_residual;
  row.field_work = step.field_work_residual;
  row.total_energy = step.total_energy_residual;
  row.causal = step.causal_speed_excess;
  return row;
}

std::vector<Spec> specs() {
  const double inv_sqrt2 = 1.0/std::sqrt(2.0);
  const double inv_sqrt3 = 1.0/std::sqrt(3.0);
  const std::array<std::pair<std::string,Vec3>,3> directions{{
      {"100",{1,0,0}},
      {"110",{inv_sqrt2,inv_sqrt2,0}},
      {"111",{inv_sqrt3,inv_sqrt3,inv_sqrt3}}}};
  std::vector<Spec> result;
  for (int width : {2,3,4}) {
    for (double speed : {0.01,0.04})
      for (const auto& family : directions)
        result.push_back({"p_w"+std::to_string(width)+"_v"
            +(speed < 0.02 ? "01_" : "04_")+family.first,
            "primary",family.first,width,0,0,family.second,speed});
    result.push_back({"z_w"+std::to_string(width),"zero","zero",
        width,0,0,{1,0,0},0.0});
    result.push_back({"m_w"+std::to_string(width)+"_v04_100",
        "mirror","100",width,0,0,{1,0,0},-0.04});
    result.push_back({"c_w"+std::to_string(width)+"_o1",
        "cubic","100",width,1,1,{0,1,0},0.04});
    result.push_back({"c_w"+std::to_string(width)+"_o2",
        "cubic","100",width,2,2,{0,0,1},0.04});
  }
  return result;
}

Arm run_arm(const Spec& spec) {
  Arm arm;
  arm.spec = spec;
  arm.a = 2.0/spec.width;
  arm.ticks = 16*spec.width;
  arm.mass_scale = arm.a*arm.a*arm.a;
  arm.polarity_scale = arm.mass_scale;
  arm.binding_scale = arm.mass_scale;
  arm.field_scale = 1.0/arm.a;
  const int L = 8*spec.width+1;
  const auto initialized = ftd::eft::initialize_connected_moore_block(
      L,spec.width,spec.orientation,0,0.0,1e-13,16384);
  if (!initialized.valid) return arm;
  auto initial = initialized.state;
  scale_field(initial,arm.polarity_scale);
  const Vec3 launch_velocity = spec.direction*spec.speed;
  const Vec3 launch_momentum =
      ftd::eft::production_flat_momentum(launch_velocity)*arm.mass_scale;
  for (auto& point : initial.constituents) point.momentum = launch_momentum;
  arm.constituent_count = static_cast<int>(initial.constituents.size());
  arm.rest_energy = arm.constituent_count*arm.mass_scale*ftd::E_REST;
  arm.inertial_mass = arm.constituent_count*arm.mass_scale*ftd::M_INERTIAL;
  arm.integrated_positive = 0.5*arm.constituent_count*arm.polarity_scale;
  arm.initialized = arm.constituent_count
          == 2*spec.width*spec.width*spec.width
      && std::abs(arm.rest_energy-16*ftd::E_REST) <= 1e-13
      && std::abs(arm.inertial_mass-16*ftd::M_INERTIAL) <= 1e-13
      && std::abs(arm.integrated_positive-8.0) <= 1e-13;
  if (!arm.initialized) return arm;

  ftd::eft::ConnectedMooreBlockOptions options;
  options.allow_shared_anchor_chart = true;
  options.constituent_mass_scale = arm.mass_scale;
  options.polarity_scale = arm.polarity_scale;
  options.binding_stiffness = arm.binding_scale;
  options.field_energy_scale = arm.field_scale;
  options.use_matrix_free_newton_krylov = true;

  auto state = initial;
  arm.initial_hash = state_hash(initial);
  arm.initial_center = center(initial);
  arm.initial_matter_momentum = momentum(initial);
  double initial_energy = NAN;
  std::vector<Vec3> forward_centers;
  forward_centers.reserve(static_cast<std::size_t>(arm.ticks));
  arm.forward = true;
  for (int tick = 1; tick <= arm.ticks; ++tick) {
    const auto step = ftd::eft::solve_connected_moore_block_forward(
        state,options);
    if (tick == 1) {
      initial_energy = total_energy(step,false);
      arm.initial_matter_momentum = step.matter_momentum_before;
      arm.initial_spline_momentum = step.spline_field_momentum_before;
    }
    const TickRecord row = tick_record(
        "forward",tick,step,step.later,true,initial_energy);
    arm.records.push_back(row);
    arm.maximum_action = std::max(arm.maximum_action,row.action);
    arm.maximum_causal = std::max(arm.maximum_causal,row.causal);
    arm.maximum_relative_edge_strain = std::max(
        arm.maximum_relative_edge_strain,row.relative_edge_strain);
    arm.maximum_energy_drift = std::max(
        arm.maximum_energy_drift,row.energy_drift);
    arm.maximum_anchor_multiplicity = std::max(
        arm.maximum_anchor_multiplicity,row.anchor_multiplicity);
    arm.total_hops += row.site_hops;
    arm.total_solve_iterations += row.solve_iterations;
    arm.total_krylov_matvecs += row.krylov_matvecs;
    const bool pass = row.valid && step.common_action_gates_pass
        && row.graph_connected && row.graph_local
        && row.constituent_count == arm.constituent_count
        && row.anchor_multiplicity <= fibre_limit
        && row.action <= 1e-9 && row.causal <= 1e-12;
    if (!pass) {
      arm.forward = false;
      break;
    }
    state = step.later;
    forward_centers.push_back(row.center);
  }
  arm.forward = arm.forward
      && forward_centers.size() == static_cast<std::size_t>(arm.ticks);
  if (!arm.forward) return arm;

  arm.final_state = state;
  arm.final_hash = state_hash(state);
  arm.final_center = center(state);
  const auto& last_forward = arm.records.back();
  arm.final_matter_momentum = last_forward.matter_momentum;
  arm.final_spline_momentum = last_forward.spline_field_momentum;
  arm.final_energy = last_forward.total;
  const Vec3 displacement = arm.final_center-arm.initial_center;
  const double projected_lattice = displacement.dot(spec.direction);
  const Vec3 transverse = displacement-spec.direction*projected_lattice;
  arm.parallel_displacement = arm.a*projected_lattice;
  arm.transverse_displacement = arm.a*transverse.mag();
  if (spec.speed != 0.0)
    arm.mobility = arm.parallel_displacement
        /(spec.speed*physical_horizon);
  Vec3 previous = arm.initial_center;
  for (int window = 0; window < 4; ++window) {
    const int endpoint = (window+1)*arm.ticks/4;
    const Vec3 current = forward_centers[static_cast<std::size_t>(endpoint-1)];
    arm.window_advance[static_cast<std::size_t>(window)] =
        arm.a*(current-previous).dot(spec.direction);
    previous = current;
  }
  const Vec3 total_defect = arm.final_matter_momentum
      +arm.final_spline_momentum-arm.initial_matter_momentum
      -arm.initial_spline_momentum;
  arm.normalized_spline_defect = total_defect.mag()
      /std::max(arm.initial_matter_momentum.mag(),1e-15);
  arm.persistent = spec.speed > 0.0
      && std::all_of(arm.window_advance.begin(),arm.window_advance.end(),
          [](double value) { return value > 0.0; })
      && arm.mobility >= 0.50
      && arm.transverse_displacement
          /(std::abs(spec.speed)*physical_horizon) <= 0.10;

  arm.reverse = true;
  for (int tick = arm.ticks; tick >= 1; --tick) {
    const auto step = ftd::eft::solve_connected_moore_block_reverse(
        state,options);
    const TickRecord row = tick_record(
        "reverse",tick,step,step.earlier,false,initial_energy);
    arm.records.push_back(row);
    arm.maximum_action = std::max(arm.maximum_action,row.action);
    arm.maximum_causal = std::max(arm.maximum_causal,row.causal);
    arm.maximum_relative_edge_strain = std::max(
        arm.maximum_relative_edge_strain,row.relative_edge_strain);
    arm.maximum_energy_drift = std::max(
        arm.maximum_energy_drift,row.energy_drift);
    arm.maximum_anchor_multiplicity = std::max(
        arm.maximum_anchor_multiplicity,row.anchor_multiplicity);
    arm.total_hops += row.site_hops;
    arm.total_solve_iterations += row.solve_iterations;
    arm.total_krylov_matvecs += row.krylov_matvecs;
    const bool pass = row.valid && step.common_action_gates_pass
        && row.graph_connected && row.graph_local
        && row.constituent_count == arm.constituent_count
        && row.anchor_multiplicity <= fibre_limit
        && row.action <= 1e-9 && row.causal <= 1e-12;
    if (!pass) {
      arm.reverse = false;
      break;
    }
    state = step.earlier;
  }
  arm.reverse = arm.reverse
      && arm.records.size() == static_cast<std::size_t>(2*arm.ticks);
  if (arm.reverse) {
    arm.recovery = ftd::eft::connected_moore_block_state_max_difference(
        initial,state);
    arm.recovered_hash = state_hash(state);
  }
  arm.exact = arm.forward && arm.reverse
      && arm.maximum_action <= 1e-9 && arm.maximum_causal <= 1e-12
      && arm.recovery <= 1e-7;
  arm.coherent = arm.exact
      && arm.maximum_relative_edge_strain <= 0.10
      && arm.maximum_anchor_multiplicity <= fibre_limit;
  return arm;
}

const Arm* find(const Summary& summary, int width, const std::string& kind,
                const std::string& family, double speed,
                int orientation = 0) {
  for (const auto& arm : summary.arms)
    if (arm.spec.width == width && arm.spec.kind == kind
        && arm.spec.family == family && arm.spec.orientation == orientation
        && std::abs(arm.spec.speed-speed) <= 1e-14) return &arm;
  return nullptr;
}

double rotated_state_residual(const ConnectedMooreBlockState& base,
                              const ConnectedMooreBlockState& candidate,
                              int maps) {
  if (base.electric.L != candidate.electric.L
      || base.constituents.size() != candidate.constituents.size()
      || base.edges.size() != candidate.edges.size()) return INFINITY;
  const int L = base.electric.L;
  double residual = 0.0;
  std::vector<std::size_t> match(base.constituents.size(),candidate.constituents.size());
  std::vector<bool> used(candidate.constituents.size(),false);
  for (std::size_t a = 0; a < base.constituents.size(); ++a) {
    const Vec3 target = cycle(position(base.constituents[a]),maps);
    double best = INFINITY;
    for (std::size_t b = 0; b < candidate.constituents.size(); ++b) {
      if (used[b] || base.charges[a] != candidate.charges[b]) continue;
      const double distance = (target-position(candidate.constituents[b])).mag();
      if (distance < best) { best = distance; match[a] = b; }
    }
    if (match[a] == candidate.constituents.size()) return INFINITY;
    used[match[a]] = true;
    residual = std::max({residual,best,
        max_component(cycle(base.constituents[a].momentum,maps)
          -candidate.constituents[match[a]].momentum)});
  }
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z) {
        const auto source = index(L,x,y,z);
        const Coord target_coord = cycle(Coord{x,y,z},maps);
        const auto target = index(L,target_coord.x,target_coord.y,target_coord.z);
        const Vec3 electric = cycle(Vec3{base.electric.x[source],
                                         base.electric.y[source],
                                         base.electric.z[source]},maps);
        const Vec3 magnetic = cycle(Vec3{base.magnetic_half.x[source],
                                         base.magnetic_half.y[source],
                                         base.magnetic_half.z[source]},maps);
        residual = std::max({residual,
            std::abs(electric.x-candidate.electric.x[target]),
            std::abs(electric.y-candidate.electric.y[target]),
            std::abs(electric.z-candidate.electric.z[target]),
            std::abs(magnetic.x-candidate.magnetic_half.x[target]),
            std::abs(magnetic.y-candidate.magnetic_half.y[target]),
            std::abs(magnetic.z-candidate.magnetic_half.z[target])});
      }
  for (const auto& edge : base.edges) {
    const auto first = match[edge.first], second = match[edge.second];
    bool found = false;
    for (const auto& other : candidate.edges) {
      const bool same = other.first == first && other.second == second;
      const bool reverse = other.first == second && other.second == first;
      if (!same && !reverse) continue;
      const Coord expected = cycle(edge.reference_delta,maps);
      const Coord actual = reverse
          ? Coord{-other.reference_delta.x,-other.reference_delta.y,
                  -other.reference_delta.z}
          : other.reference_delta;
      residual = std::max({residual,
          static_cast<double>(std::abs(expected.x-actual.x)),
          static_cast<double>(std::abs(expected.y-actual.y)),
          static_cast<double>(std::abs(expected.z-actual.z)),
          std::abs(edge.rest_length_squared-other.rest_length_squared)});
      found = true;
      break;
    }
    if (!found) return INFINITY;
  }
  return residual;
}

std::vector<const TickRecord*> forward_records(const Arm& arm) {
  std::vector<const TickRecord*> result;
  for (const auto& row : arm.records)
    if (row.phase == "forward") result.push_back(&row);
  return result;
}

void evaluate(Summary& summary) {
  summary.coverage = summary.arms.size() == 30;
  summary.execution = summary.coverage;
  summary.exact = summary.coverage;
  summary.coherence = summary.coverage;
  for (const auto& arm : summary.arms) {
    summary.execution = summary.execution && arm.initialized
        && arm.forward && arm.reverse
        && arm.records.size() == static_cast<std::size_t>(2*arm.ticks);
    summary.exact = summary.exact && arm.exact;
    summary.coherence = summary.coherence && arm.coherent;
    summary.worst_action = std::max(summary.worst_action,arm.maximum_action);
    summary.worst_causal = std::max(summary.worst_causal,arm.maximum_causal);
    summary.worst_strain = std::max(
        summary.worst_strain,arm.maximum_relative_edge_strain);
    if (std::isfinite(arm.recovery))
      summary.worst_recovery = std::max(summary.worst_recovery,arm.recovery);
    if (arm.spec.kind == "primary" && arm.spec.speed == 0.04
        && arm.persistent) ++summary.high_persistent;
    if (arm.spec.kind == "primary" && arm.spec.speed == 0.01
        && arm.persistent) ++summary.low_persistent;
  }

  summary.zero = summary.execution;
  for (int width : {2,3,4}) {
    const Arm* arm = find(summary,width,"zero","zero",0.0);
    if (!arm) { summary.zero = false; continue; }
    const double displacement = arm->a
        *(arm->final_center-arm->initial_center).mag();
    summary.worst_zero = std::max(summary.worst_zero,displacement);
    summary.zero = summary.zero && displacement <= 1e-6;
  }

  summary.mirror = summary.execution;
  for (int width : {2,3,4}) {
    const Arm* positive = find(summary,width,"primary","100",0.04);
    const Arm* negative = find(summary,width,"mirror","100",-0.04);
    if (!positive || !negative) { summary.mirror = false; continue; }
    const Vec3 positive_displacement =
        (positive->final_center-positive->initial_center)*positive->a;
    const Vec3 negative_displacement =
        (negative->final_center-negative->initial_center)*negative->a;
    summary.mirror_residual = std::max({summary.mirror_residual,
        (positive_displacement+negative_displacement).mag(),
        (positive->final_matter_momentum
          +negative->final_matter_momentum).mag(),
        std::abs(positive->final_energy-negative->final_energy),
        std::abs(positive->recovery-negative->recovery)});
  }
  summary.mirror = summary.mirror && summary.mirror_residual <= 1e-6;

  summary.cubic = summary.execution;
  for (int width : {2,3,4}) {
    const Arm* base = find(summary,width,"primary","100",0.04,0);
    if (!base) { summary.cubic = false; continue; }
    const auto base_rows = forward_records(*base);
    for (int maps : {1,2}) {
      const Arm* rotated = find(summary,width,"cubic","100",0.04,maps);
      if (!rotated) { summary.cubic = false; continue; }
      const auto rotated_rows = forward_records(*rotated);
      if (rotated_rows.size() != base_rows.size()) {
        summary.cubic = false;
        continue;
      }
      for (std::size_t tick = 0; tick < base_rows.size(); ++tick) {
        const auto& lhs = *base_rows[tick];
        const auto& rhs = *rotated_rows[tick];
        summary.cubic_residual = std::max({summary.cubic_residual,
            max_component(cycle(lhs.center-base->initial_center,maps)
              -(rhs.center-rotated->initial_center)),
            max_component(cycle(lhs.matter_momentum,maps)
              -rhs.matter_momentum),
            max_component(cycle(lhs.local_field_momentum,maps)
              -rhs.local_field_momentum),
            max_component(cycle(lhs.spline_field_momentum,maps)
              -rhs.spline_field_momentum),
            std::abs(lhs.kinetic-rhs.kinetic),
            std::abs(lhs.binding-rhs.binding),
            std::abs(lhs.field-rhs.field),
            std::abs(lhs.total-rhs.total),
            std::abs(lhs.relative_edge_strain-rhs.relative_edge_strain),
            std::abs(lhs.action-rhs.action)});
        if (lhs.site_hops != rhs.site_hops) summary.cubic = false;
      }
      summary.cubic_residual = std::max({summary.cubic_residual,
          rotated_state_residual(base->final_state,rotated->final_state,maps),
          std::abs(base->recovery-rotated->recovery)});
    }
  }
  summary.cubic = summary.cubic && summary.cubic_residual <= 1e-6;

  summary.transport = summary.execution;
  for (int width : {2,3,4}) {
    double minimum = INFINITY, maximum = -INFINITY, defect = 0.0;
    for (const std::string family : {"100","110","111"}) {
      const Arm* arm = find(summary,width,"primary",family,0.04);
      if (!arm) { summary.transport = false; continue; }
      summary.transport = summary.transport && arm->persistent;
      minimum = std::min(minimum,arm->mobility);
      maximum = std::max(maximum,arm->mobility);
      defect = std::max(defect,arm->normalized_spline_defect);
    }
    summary.minimum_high_mobility[width] = minimum;
    summary.high_mobility_span[width] = maximum-minimum;
    summary.maximum_high_defect[width] = defect;
  }
  summary.mobility_trend = summary.execution
      && summary.minimum_high_mobility[3]+1e-4
          >= summary.minimum_high_mobility[2]
      && summary.minimum_high_mobility[4]+1e-4
          >= summary.minimum_high_mobility[3];
  summary.anisotropy_trend = summary.execution
      && summary.high_mobility_span[4] < summary.high_mobility_span[2];
  summary.defect_trend = summary.execution
      && summary.maximum_high_defect[4] < summary.maximum_high_defect[2];
  summary.resolution = summary.mobility_trend
      && summary.anisotropy_trend && summary.defect_trend;

  if (!summary.coverage || !summary.execution)
    summary.verdict = "CELL_MEASURE_LONG_HORIZON_EXECUTION_INVALID";
  else if (!summary.exact || !summary.coherence || !summary.zero
           || !summary.mirror || !summary.cubic)
    summary.verdict = "CELL_MEASURE_LONG_HORIZON_CLOSED";
  else if (!summary.transport || !summary.resolution)
    summary.verdict = "CELL_MEASURE_LONG_HORIZON_MIXED";
  else
    summary.verdict = "CELL_MEASURE_LONG_HORIZON_IR_TREND_CONSTRUCTIVE";
}

void write_records(const Summary& summary) {
  const auto dir = std::filesystem::path(__FILE__).parent_path().parent_path()
      /"results/ftd_0650";
  std::filesystem::create_directories(dir);
  std::ofstream json(dir/"ftd_0650_cell_measure_long_horizon_v1.json");
  json << std::boolalpha << std::setprecision(17)
       << "{\n  \"ftd_id\": \"FTD-0650\",\n"
       << "  \"protocol_sha256\": \"" << protocol_sha256 << "\",\n"
       << "  \"verdict\": \"" << summary.verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"arm_count\": " << summary.arms.size() << ",\n"
       << "  \"coverage_pass\": " << summary.coverage << ",\n"
       << "  \"execution_pass\": " << summary.execution << ",\n"
       << "  \"exact_pass\": " << summary.exact << ",\n"
       << "  \"coherence_pass\": " << summary.coherence << ",\n"
       << "  \"zero_pass\": " << summary.zero << ",\n"
       << "  \"mirror_pass\": " << summary.mirror << ",\n"
       << "  \"cubic_pass\": " << summary.cubic << ",\n"
       << "  \"transport_pass\": " << summary.transport << ",\n"
       << "  \"mobility_trend_pass\": " << summary.mobility_trend << ",\n"
       << "  \"anisotropy_trend_pass\": " << summary.anisotropy_trend << ",\n"
       << "  \"defect_trend_pass\": " << summary.defect_trend << ",\n"
       << "  \"resolution_pass\": " << summary.resolution << ",\n"
       << "  \"high_persistent_count\": " << summary.high_persistent << ",\n"
       << "  \"low_persistent_count\": " << summary.low_persistent << ",\n"
       << "  \"worst_action_residual\": " << summary.worst_action << ",\n"
       << "  \"worst_causal_excess\": " << summary.worst_causal << ",\n"
       << "  \"worst_relative_edge_strain\": " << summary.worst_strain << ",\n"
       << "  \"worst_recovery\": " << summary.worst_recovery << ",\n"
       << "  \"worst_zero_displacement\": " << summary.worst_zero << ",\n"
       << "  \"mirror_residual\": " << summary.mirror_residual << ",\n"
       << "  \"cubic_residual\": " << summary.cubic_residual << ",\n"
       << "  \"minimum_high_mobility\": {\"2\": "
       << summary.minimum_high_mobility.at(2) << ", \"3\": "
       << summary.minimum_high_mobility.at(3) << ", \"4\": "
       << summary.minimum_high_mobility.at(4) << "},\n"
       << "  \"high_mobility_span\": {\"2\": "
       << summary.high_mobility_span.at(2) << ", \"3\": "
       << summary.high_mobility_span.at(3) << ", \"4\": "
       << summary.high_mobility_span.at(4) << "},\n"
       << "  \"maximum_high_defect\": {\"2\": "
       << summary.maximum_high_defect.at(2) << ", \"3\": "
       << summary.maximum_high_defect.at(3) << ", \"4\": "
       << summary.maximum_high_defect.at(4) << "}\n}\n";

  std::ofstream arms(dir/"ftd_0650_cell_measure_long_horizon_arms_v1.csv");
  arms << "ftd_id,label,kind,family,width,orientation,rotation_maps,speed,"
          "initialized,forward,reverse,exact,coherent,persistent,ticks,"
          "constituent_count,total_hops,max_anchor_multiplicity,"
          "total_solve_iterations,total_krylov_matvecs,a,mass_scale,"
          "polarity_scale,binding_scale,field_scale,rest_energy,inertial_mass,"
          "integrated_positive,max_action,max_causal,max_relative_edge_strain,"
          "max_energy_drift,recovery,final_energy,normalized_spline_defect,"
          "parallel_displacement,transverse_displacement,mobility,window1,"
          "window2,window3,window4,initial_hash,final_hash,recovered_hash,"
          "initial_center_x,initial_center_y,initial_center_z,final_center_x,"
          "final_center_y,final_center_z,initial_matter_px,initial_matter_py,"
          "initial_matter_pz,final_matter_px,final_matter_py,final_matter_pz,"
          "initial_spline_px,initial_spline_py,initial_spline_pz,"
          "final_spline_px,final_spline_py,final_spline_pz\n";
  for (const auto& arm : summary.arms)
    arms << std::boolalpha << std::setprecision(17) << "FTD-0650,"
         << arm.spec.label << ',' << arm.spec.kind << ',' << arm.spec.family
         << ',' << arm.spec.width << ',' << arm.spec.orientation << ','
         << arm.spec.rotation_maps << ',' << arm.spec.speed << ','
         << arm.initialized << ',' << arm.forward << ',' << arm.reverse << ','
         << arm.exact << ',' << arm.coherent << ',' << arm.persistent << ','
         << arm.ticks << ',' << arm.constituent_count << ',' << arm.total_hops
         << ',' << arm.maximum_anchor_multiplicity << ','
         << arm.total_solve_iterations << ',' << arm.total_krylov_matvecs << ','
         << arm.a << ',' << arm.mass_scale << ',' << arm.polarity_scale << ','
         << arm.binding_scale << ',' << arm.field_scale << ',' << arm.rest_energy
         << ',' << arm.inertial_mass << ',' << arm.integrated_positive << ','
         << arm.maximum_action << ',' << arm.maximum_causal << ','
         << arm.maximum_relative_edge_strain << ',' << arm.maximum_energy_drift
         << ',' << arm.recovery << ',' << arm.final_energy << ','
         << arm.normalized_spline_defect << ',' << arm.parallel_displacement
         << ',' << arm.transverse_displacement << ',' << arm.mobility << ','
         << arm.window_advance[0] << ',' << arm.window_advance[1] << ','
         << arm.window_advance[2] << ',' << arm.window_advance[3] << ','
         << arm.initial_hash << ',' << arm.final_hash << ',' << arm.recovered_hash
         << ',' << arm.initial_center.x << ',' << arm.initial_center.y << ','
         << arm.initial_center.z << ',' << arm.final_center.x << ','
         << arm.final_center.y << ',' << arm.final_center.z << ','
         << arm.initial_matter_momentum.x << ',' << arm.initial_matter_momentum.y
         << ',' << arm.initial_matter_momentum.z << ','
         << arm.final_matter_momentum.x << ',' << arm.final_matter_momentum.y
         << ',' << arm.final_matter_momentum.z << ','
         << arm.initial_spline_momentum.x << ',' << arm.initial_spline_momentum.y
         << ',' << arm.initial_spline_momentum.z << ','
         << arm.final_spline_momentum.x << ',' << arm.final_spline_momentum.y
         << ',' << arm.final_spline_momentum.z << '\n';

  std::ofstream ticks(dir/"ftd_0650_cell_measure_long_horizon_ticks_v1.csv");
  ticks << "ftd_id,label,phase,tick,state_hash,valid,graph_connected,graph_local,"
          "constituent_count,anchor_multiplicity,site_hops,solve_iterations,"
          "krylov_matvecs,center_x,center_y,center_z,matter_px,matter_py,"
          "matter_pz,local_field_px,local_field_py,local_field_pz,spline_px,"
          "spline_py,spline_pz,kinetic,binding,field,total,energy_drift,"
          "relative_edge_strain,engine_edge_strain,action,root,force,continuity,"
          "gauss_before,gauss_after,kinetic_gradient,electric_adjoint,"
          "magnetic_work,binding_work,binding_sum,matter_work,field_work,"
          "total_energy,causal\n";
  for (const auto& arm : summary.arms)
    for (const auto& row : arm.records)
      ticks << std::boolalpha << std::setprecision(17) << "FTD-0650,"
            << arm.spec.label << ',' << row.phase << ',' << row.tick << ','
            << row.state_hash << ',' << row.valid << ',' << row.graph_connected
            << ',' << row.graph_local << ',' << row.constituent_count << ','
            << row.anchor_multiplicity << ',' << row.site_hops << ','
            << row.solve_iterations << ',' << row.krylov_matvecs << ','
            << row.center.x << ',' << row.center.y << ',' << row.center.z << ','
            << row.matter_momentum.x << ',' << row.matter_momentum.y << ','
            << row.matter_momentum.z << ',' << row.local_field_momentum.x << ','
            << row.local_field_momentum.y << ',' << row.local_field_momentum.z
            << ',' << row.spline_field_momentum.x << ','
            << row.spline_field_momentum.y << ',' << row.spline_field_momentum.z
            << ',' << row.kinetic << ',' << row.binding << ',' << row.field << ','
            << row.total << ',' << row.energy_drift << ','
            << row.relative_edge_strain << ',' << row.engine_edge_strain << ','
            << row.action << ',' << row.root << ',' << row.force << ','
            << row.continuity << ',' << row.gauss_before << ',' << row.gauss_after
            << ',' << row.kinetic_gradient << ',' << row.electric_adjoint << ','
            << row.magnetic_work << ',' << row.binding_work << ','
            << row.binding_sum << ',' << row.matter_work << ',' << row.field_work
            << ',' << row.total_energy << ',' << row.causal << '\n';
}

}  // namespace

int main() {
  Summary summary;
  const auto all_specs = specs();
  constexpr std::size_t batch = 6;
  for (std::size_t start = 0; start < all_specs.size(); start += batch) {
    const std::size_t end = std::min(start+batch,all_specs.size());
    std::vector<std::future<Arm>> futures;
    for (std::size_t i = start; i < end; ++i)
      futures.push_back(std::async(std::launch::async,
          [spec=all_specs[i]]() { return run_arm(spec); }));
    for (std::size_t i = start; i < end; ++i) {
      summary.arms.push_back(futures[i-start].get());
      std::cout << "completed " << all_specs[i].label << std::endl;
    }
  }
  evaluate(summary);
  write_records(summary);
  std::cout << std::boolalpha << std::setprecision(17)
            << "protocol_sha256=" << protocol_sha256 << '\n'
            << "verdict=" << summary.verdict << '\n'
            << "coverage=" << summary.coverage
            << " execution=" << summary.execution
            << " exact=" << summary.exact
            << " coherence=" << summary.coherence
            << " zero=" << summary.zero
            << " mirror=" << summary.mirror
            << " cubic=" << summary.cubic
            << " transport=" << summary.transport
            << " resolution=" << summary.resolution << '\n'
            << "high_persistent=" << summary.high_persistent
            << " low_persistent=" << summary.low_persistent
            << " action=" << summary.worst_action
            << " recovery=" << summary.worst_recovery
            << " mirror_residual=" << summary.mirror_residual
            << " cubic_residual=" << summary.cubic_residual << '\n';
  for (int width : {2,3,4})
    std::cout << "w=" << width
              << " min_mu=" << summary.minimum_high_mobility[width]
              << " span=" << summary.high_mobility_span[width]
              << " max_D=" << summary.maximum_high_defect[width] << '\n';
  return summary.verdict == "CELL_MEASURE_LONG_HORIZON_EXECUTION_INVALID"
      ? 1 : 0;
}
