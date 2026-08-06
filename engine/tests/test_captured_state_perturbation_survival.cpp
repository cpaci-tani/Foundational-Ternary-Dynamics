// FTD-0732: locked captured-state perturbation survival campaign.

#include "ftd/eft/connected_moore_block_action.h"
#include "ftd/eft/coupled_matched_face_transaction.h"
#include "ftd/eft/face_flux_normalization.h"
#include "ftd/eft/quadratic_coat_face_current.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace {

using ftd::Vec3;
using ftd::eft::ConnectedBindingLaw;
using ftd::eft::ConnectedMooreBlockOptions;
using ftd::eft::ConnectedMooreBlockSolveCache;
using ftd::eft::ConnectedMooreBlockState;
using ftd::eft::MatchedEdgeField;
using ftd::eft::MatchedFaceFlux;
using ftd::eft::MatchedMatterPoint;

constexpr char kProtocolSha256[] =
    "1A93899A9960D099AC0F64E039E06A527260211393FF80F6CF833333801B0903";
constexpr int kParentTicks = 128;
constexpr int kContinuationTicks = 256;
constexpr double kGate = 1e-10;
constexpr double kMomentum = 0.0120;
constexpr double kEpsilon = 1.0 / 20.0;
constexpr double kImpulse = kEpsilon * kMomentum;

struct Direction {
  int x;
  int y;
  int z;
  const char* label;
  Vec3 tangent1;
  Vec3 tangent2;
  double expected_separation_128_l33;
  double expected_energy_128_l33;
  double expected_separation_128_l65;
  double expected_energy_128_l65;
};

const std::array<Direction, 3> kDirections{{
    {0, 0, 1, "0_0_1", {1, 0, 0}, {0, 1, 0},
     0.9508349579947755, -0.0009153064482694557,
     0.9509643670937464, -0.0009153425564001130},
    {0, 1, -1, "0_1_-1", {1, 0, 0},
     {0, 1.0 / std::sqrt(2.0), 1.0 / std::sqrt(2.0)},
     0.9640142739636979, -0.0006005619104165333,
     0.9645147711245921, -0.0006009481291792181},
    {1, 1, 1, "1_1_1",
     {1.0 / std::sqrt(2.0), -1.0 / std::sqrt(2.0), 0},
     {1.0 / std::sqrt(6.0), 1.0 / std::sqrt(6.0),
      -2.0 / std::sqrt(6.0)},
     0.9431987959627205, -0.00019282204292216457,
     0.9441694690031436, -0.00019358922796269828},
}};

const std::array<std::string, 11> kVariantNames{{
    "center", "separation_minus", "separation_plus",
    "radial_impulse_minus", "radial_impulse_plus",
    "tangent1_impulse_minus", "tangent1_impulse_plus",
    "tangent2_impulse_minus", "tangent2_impulse_plus",
    "dynamic_field_minus", "dynamic_field_plus"}};

struct ParentState {
  bool valid = false;
  bool reproduction_pass = false;
  double separation = 0.0;
  double pair_energy = 0.0;
  ConnectedMooreBlockState state;

  explicit ParentState(int L = 0) : state(L) {}
};

struct VariantState {
  bool valid = false;
  double gauss_residual = INFINITY;
  double momentum_preservation = INFINITY;
  double maximum_speed = INFINITY;
  double separation = INFINITY;
  double pair_energy = INFINITY;
  ConnectedMooreBlockState state;

  explicit VariantState(int L = 0) : state(L) {}
};

struct ArmResult {
  int volume = 0;
  std::string stage;
  std::string direction;
  std::string polarity;
  std::string variant;
  std::string selector;
  bool parent_valid = false;
  bool parent_reproduction_pass = false;
  bool initialized = false;
  bool executed = false;
  bool identity_pass = false;
  bool recoil_pass = false;
  bool inverse_pass = false;
  bool positive_field_energy = false;
  bool survives = false;
  int graph_transitions = 0;
  std::vector<int> graph_transition_ticks;
  std::string final_class = "unclassified";
  double initial_gauss_residual = INFINITY;
  double initial_momentum_preservation = INFINITY;
  double initial_maximum_speed = INFINITY;
  double initial_pair_energy = 0.0;
  double final_pair_energy = 0.0;
  double initial_field_energy = 0.0;
  double final_field_energy = 0.0;
  double pair_field_balance = INFINITY;
  double minimum_energy_margin = INFINITY;
  double minimum_graph_margin = INFINITY;
  double maximum_common_residual = 0.0;
  double maximum_recoil_defect = 0.0;
  double inverse_recovery = INFINITY;
  std::vector<double> separation_history;
  std::vector<double> internal_history;
  std::vector<double> field_history;
};

struct Selector {
  std::string direction;
  std::string polarity;
  std::string energy_variant;
  std::string graph_variant;
};

int wrap(int value, int L) {
  const int remainder = value % L;
  return remainder < 0 ? remainder + L : remainder;
}

Vec3 effective_position(const MatchedMatterPoint& point) {
  return {point.anchor.x + point.remainder.x,
          point.anchor.y + point.remainder.y,
          point.anchor.z + point.remainder.z};
}

MatchedMatterPoint point_at(const Vec3& position, const Vec3& momentum,
                            int L) {
  MatchedMatterPoint point;
  const long long ax = std::llround(position.x);
  const long long ay = std::llround(position.y);
  const long long az = std::llround(position.z);
  point.anchor = {wrap(static_cast<int>(ax), L),
                  wrap(static_cast<int>(ay), L),
                  wrap(static_cast<int>(az), L)};
  point.remainder = {position.x - ax, position.y - ay, position.z - az};
  point.momentum = momentum;
  return point;
}

double pair_separation(const ConnectedMooreBlockState& state) {
  return (effective_position(state.constituents[1])
          - effective_position(state.constituents[0])).mag();
}

bool graph_inside(double separation,
                  const ConnectedMooreBlockOptions& options) {
  return separation * separation
      < options.compact_pair_cutoff_distance_squared;
}

double pair_internal_energy(const ConnectedMooreBlockState& state,
                            const ConnectedMooreBlockOptions& options) {
  long double kinetic = 0.0L;
  for (const auto& point : state.constituents) {
    const double energy = std::sqrt(
        ftd::E_REST * ftd::E_REST
        + ftd::C_SPEED * ftd::C_SPEED * point.momentum.mag2());
    kinetic += energy - ftd::E_REST;
  }
  return static_cast<double>(kinetic)
      + ftd::eft::connected_moore_block_binding_energy(state, options);
}

double field_energy(const ConnectedMooreBlockState& state,
                    const ConnectedMooreBlockOptions& options,
                    double interaction_scale) {
  return interaction_scale * ftd::eft::matched_modified_energy(
      state.electric, state.magnetic_half,
      options.wave_speed * options.dt);
}

Vec3 total_momentum(const ConnectedMooreBlockState& state) {
  Vec3 result{};
  for (const auto& point : state.constituents) result += point.momentum;
  return result;
}

double maximum_speed(const ConnectedMooreBlockState& state) {
  double result = 0.0;
  for (const auto& point : state.constituents) {
    const double energy = std::sqrt(
        ftd::E_REST * ftd::E_REST
        + ftd::C_SPEED * ftd::C_SPEED * point.momentum.mag2());
    result = std::max(result,
        (point.momentum * (ftd::C_SPEED * ftd::C_SPEED / energy)).mag());
  }
  return result;
}

double maximum_step_residual(
    const ftd::eft::ConnectedMooreBlockStepResult& step) {
  return std::max({step.root_residual, step.continuity_residual,
      step.gauss_before_residual, step.gauss_after_residual,
      step.force_residual, step.kinematic_residual,
      step.kinetic_discrete_gradient_residual,
      step.electric_adjoint_residual, step.magnetic_work_residual,
      step.binding_work_residual, step.binding_impulse_sum_residual,
      step.matter_work_residual, step.field_work_residual,
      step.total_energy_residual, step.causal_speed_excess});
}

ConnectedMooreBlockState make_geometry(int L, const Direction& direction,
                                       bool conjugate) {
  ConnectedMooreBlockState state(L);
  const Vec3 center{static_cast<double>(L / 2),
                    static_cast<double>(L / 2),
                    static_cast<double>(L / 2)};
  const Vec3 ray{static_cast<double>(direction.x),
                 static_cast<double>(direction.y),
                 static_cast<double>(direction.z)};
  const Vec3 unit = ray * (1.0 / ray.mag());
  state.constituents.push_back(point_at(
      center - unit * 0.65, unit * kMomentum, L));
  state.constituents.push_back(point_at(
      center + unit * 0.65, unit * (-kMomentum), L));
  const int first = conjugate ? -1 : +1;
  state.charges = {first, -first};
  state.edges.clear();
  return state;
}

std::vector<double> fractional_density(
    const ConnectedMooreBlockState& state) {
  const int L = state.electric.L;
  const auto side = static_cast<std::size_t>(L);
  std::vector<double> density(side * side * side, 0.0);
  for (std::size_t a = 0; a < state.constituents.size(); ++a) {
    const auto coat = ftd::eft::make_quadratic_polarity_coat(
        effective_position(state.constituents[a]), state.charges[a]);
    if (!coat.valid) return {};
    for (std::size_t i = 0; i < coat.weight_count; ++i) {
      const auto& entry = coat.weights[i];
      const auto index = static_cast<std::size_t>(state.electric.index(
          entry.site.x, entry.site.y, entry.site.z));
      density[index] += entry.weight;
    }
  }
  return density;
}

void set_face_combination(MatchedFaceFlux& output,
                          const MatchedFaceFlux& base,
                          const MatchedFaceFlux& actual, double scale) {
  for (std::size_t i = 0; i < output.x.size(); ++i) {
    output.x[i] = base.x[i] + scale * (actual.x[i] - base.x[i]);
    output.y[i] = base.y[i] + scale * (actual.y[i] - base.y[i]);
    output.z[i] = base.z[i] + scale * (actual.z[i] - base.z[i]);
  }
}

void set_edge_combination(MatchedEdgeField& output,
                          const MatchedEdgeField& base,
                          const MatchedEdgeField& actual, double scale) {
  for (std::size_t i = 0; i < output.x.size(); ++i) {
    output.x[i] = base.x[i] + scale * (actual.x[i] - base.x[i]);
    output.y[i] = base.y[i] + scale * (actual.y[i] - base.y[i]);
    output.z[i] = base.z[i] + scale * (actual.z[i] - base.z[i]);
  }
}

void add_face_residual(MatchedFaceFlux& output,
                       const MatchedFaceFlux& actual,
                       const MatchedFaceFlux& base) {
  for (std::size_t i = 0; i < output.x.size(); ++i) {
    output.x[i] += actual.x[i] - base.x[i];
    output.y[i] += actual.y[i] - base.y[i];
    output.z[i] += actual.z[i] - base.z[i];
  }
}

void add_edge_residual(MatchedEdgeField& output,
                       const MatchedEdgeField& actual,
                       const MatchedEdgeField& base) {
  for (std::size_t i = 0; i < output.x.size(); ++i) {
    output.x[i] += actual.x[i] - base.x[i];
    output.y[i] += actual.y[i] - base.y[i];
    output.z[i] += actual.z[i] - base.z[i];
  }
}

ParentState build_parent(int L, const Direction& direction, bool conjugate,
                         const ConnectedMooreBlockOptions& options) {
  ParentState result(L);
  const auto initial = ftd::eft::redress_derived_compact_pair(
      make_geometry(L, direction, conjugate), options, 1e-13, 4096);
  if (!initial.valid) return result;
  ConnectedMooreBlockState state = initial.state;
  ConnectedMooreBlockSolveCache cache;
  bool valid = true;
  for (int tick = 0; tick < kParentTicks; ++tick) {
    const auto step = ftd::eft::solve_connected_moore_block_forward(
        state, options, &cache);
    valid = valid && step.valid && step.common_action_gates_pass
        && maximum_step_residual(step) <= kGate;
    if (!step.valid) break;
    state = step.later;
  }
  if (!valid) return result;
  result.separation = pair_separation(state);
  result.pair_energy = pair_internal_energy(state, options);
  const double expected_separation = L == 33
      ? direction.expected_separation_128_l33
      : direction.expected_separation_128_l65;
  const double expected_energy = L == 33
      ? direction.expected_energy_128_l33
      : direction.expected_energy_128_l65;
  result.reproduction_pass =
      std::abs(result.separation - expected_separation) <= 1e-12
      && std::abs(result.pair_energy - expected_energy) <= 1e-12;
  result.valid = result.reproduction_pass
      && graph_inside(result.separation, options)
      && result.pair_energy < -1e-6;
  result.state = state;
  return result;
}

VariantState make_variant(const ParentState& parent,
                          const Direction& direction,
                          const std::string& name,
                          const ConnectedMooreBlockOptions& options) {
  const int L = parent.state.electric.L;
  VariantState result(L);
  if (!parent.valid) return result;
  const auto static_parent = ftd::eft::redress_derived_compact_pair(
      parent.state, options, 1e-13, 4096);
  if (!static_parent.valid) return result;
  result.state = parent.state;
  const Vec3 p0 = total_momentum(parent.state);

  const Vec3 x0 = effective_position(parent.state.constituents[0]);
  const Vec3 x1 = effective_position(parent.state.constituents[1]);
  const Vec3 center = (x0 + x1) * 0.5;
  const Vec3 relative = x1 - x0;
  const Vec3 radial = relative * (1.0 / relative.mag());

  if (name == "separation_minus" || name == "separation_plus") {
    const double scale = name == "separation_minus"
        ? 1.0 - kEpsilon : 1.0 + kEpsilon;
    ConnectedMooreBlockState geometry = parent.state;
    geometry.constituents[0] = point_at(
        center - relative * (0.5 * scale),
        parent.state.constituents[0].momentum, L);
    geometry.constituents[1] = point_at(
        center + relative * (0.5 * scale),
        parent.state.constituents[1].momentum, L);
    const auto static_perturbed = ftd::eft::redress_derived_compact_pair(
        geometry, options, 1e-13, 4096);
    if (!static_perturbed.valid) return result;
    result.state = static_perturbed.state;
    add_face_residual(result.state.electric, parent.state.electric,
                      static_parent.state.electric);
    add_edge_residual(result.state.magnetic_half,
                      parent.state.magnetic_half,
                      static_parent.state.magnetic_half);
  } else if (name == "radial_impulse_minus"
             || name == "radial_impulse_plus") {
    const double sign = name == "radial_impulse_minus" ? -1.0 : 1.0;
    result.state.constituents[0].momentum -= radial * (sign * kImpulse);
    result.state.constituents[1].momentum += radial * (sign * kImpulse);
  } else if (name == "tangent1_impulse_minus"
             || name == "tangent1_impulse_plus") {
    const double sign = name == "tangent1_impulse_minus" ? -1.0 : 1.0;
    result.state.constituents[0].momentum -=
        direction.tangent1 * (sign * kImpulse);
    result.state.constituents[1].momentum +=
        direction.tangent1 * (sign * kImpulse);
  } else if (name == "tangent2_impulse_minus"
             || name == "tangent2_impulse_plus") {
    const double sign = name == "tangent2_impulse_minus" ? -1.0 : 1.0;
    result.state.constituents[0].momentum -=
        direction.tangent2 * (sign * kImpulse);
    result.state.constituents[1].momentum +=
        direction.tangent2 * (sign * kImpulse);
  } else if (name == "dynamic_field_minus"
             || name == "dynamic_field_plus") {
    const double scale = name == "dynamic_field_minus"
        ? 1.0 - kEpsilon : 1.0 + kEpsilon;
    result.state = static_parent.state;
    set_face_combination(result.state.electric,
        static_parent.state.electric, parent.state.electric, scale);
    set_edge_combination(result.state.magnetic_half,
        static_parent.state.magnetic_half, parent.state.magnetic_half, scale);
  } else if (name != "center") {
    return result;
  }

  const auto density = fractional_density(result.state);
  if (density.empty()) return result;
  result.gauss_residual = ftd::eft::max_fractional_gauss_residual(
      result.state.electric, density);
  result.momentum_preservation = (total_momentum(result.state) - p0).mag();
  result.maximum_speed = maximum_speed(result.state);
  result.separation = pair_separation(result.state);
  result.pair_energy = pair_internal_energy(result.state, options);
  result.valid = std::isfinite(result.gauss_residual)
      && result.gauss_residual <= 1e-12
      && result.momentum_preservation <= 1e-15
      && result.maximum_speed <= ftd::C_SPEED + 1e-12
      && graph_inside(result.separation, options)
      && result.pair_energy < -1e-6;
  return result;
}

ArmResult run_continuation(int L, const std::string& stage,
                           const Direction& direction, bool conjugate,
                           const std::string& variant,
                           const std::string& selector,
                           const ParentState& parent,
                           const ConnectedMooreBlockOptions& options,
                           double interaction_scale) {
  ArmResult result;
  result.volume = L;
  result.stage = stage;
  result.direction = direction.label;
  result.polarity = conjugate ? "minus_plus" : "plus_minus";
  result.variant = variant;
  result.selector = selector;
  result.parent_valid = parent.valid;
  result.parent_reproduction_pass = parent.reproduction_pass;
  const auto initial = make_variant(parent, direction, variant, options);
  result.initialized = initial.valid;
  result.initial_gauss_residual = initial.gauss_residual;
  result.initial_momentum_preservation = initial.momentum_preservation;
  result.initial_maximum_speed = initial.maximum_speed;
  result.initial_pair_energy = initial.pair_energy;
  result.separation_history.push_back(initial.separation);
  result.internal_history.push_back(initial.pair_energy);
  if (!initial.valid) return result;

  ConnectedMooreBlockState state = initial.state;
  const ConnectedMooreBlockState original = state;
  result.initial_pair_energy = pair_internal_energy(state, options);
  result.initial_field_energy = field_energy(state, options, interaction_scale);
  result.separation_history.clear();
  result.internal_history.clear();
  result.separation_history.push_back(pair_separation(state));
  result.internal_history.push_back(result.initial_pair_energy);
  result.field_history.push_back(result.initial_field_energy);
  bool edge = graph_inside(result.separation_history.front(), options);
  bool valid_roots = true;
  bool common = true;
  bool recoil = true;
  bool field_nonnegative = result.initial_field_energy >= -1e-12;
  ConnectedMooreBlockSolveCache cache;

  for (int tick = 0; tick < kContinuationTicks; ++tick) {
    const auto step = ftd::eft::solve_connected_moore_block_forward(
        state, options, &cache);
    valid_roots = valid_roots && step.valid;
    if (!step.valid) break;
    const double residual = maximum_step_residual(step);
    result.maximum_common_residual = std::max(
        result.maximum_common_residual, residual);
    common = common && step.common_action_gates_pass && residual <= kGate;
    const double recoil_defect = std::max(
        step.matter_momentum_after.mag(), step.spline_defect_norm);
    result.maximum_recoil_defect = std::max(
        result.maximum_recoil_defect, recoil_defect);
    recoil = recoil && recoil_defect <= 1e-9;
    if (step.relational_graph_changed)
      result.graph_transition_ticks.push_back(tick + 1);
    edge = step.relational_edge_after;
    state = step.later;
    result.separation_history.push_back(pair_separation(state));
    result.internal_history.push_back(pair_internal_energy(state, options));
    result.field_history.push_back(field_energy(
        state, options, interaction_scale));
    field_nonnegative = field_nonnegative
        && result.field_history.back() >= -1e-12;
  }

  result.executed = valid_roots
      && result.internal_history.size()
          == static_cast<std::size_t>(kContinuationTicks + 1);
  if (!result.executed) return result;
  result.identity_pass = common;
  result.recoil_pass = recoil;
  result.positive_field_energy = field_nonnegative;
  result.graph_transitions =
      static_cast<int>(result.graph_transition_ticks.size());
  result.final_pair_energy = result.internal_history.back();
  result.final_field_energy = result.field_history.back();
  result.pair_field_balance = std::abs(
      result.final_field_energy - result.initial_field_energy
      + result.final_pair_energy - result.initial_pair_energy);
  result.minimum_energy_margin = std::numeric_limits<double>::infinity();
  result.minimum_graph_margin = std::numeric_limits<double>::infinity();
  bool negative_inside = true;
  bool any_outside = false;
  bool any_nonnegative = false;
  for (std::size_t i = 0; i < result.internal_history.size(); ++i) {
    result.minimum_energy_margin = std::min(
        result.minimum_energy_margin, -result.internal_history[i] / 0.01);
    result.minimum_graph_margin = std::min(
        result.minimum_graph_margin,
        std::sqrt(options.compact_pair_cutoff_distance_squared)
            - result.separation_history[i]);
    const bool inside = graph_inside(result.separation_history[i], options);
    const bool negative = result.internal_history[i] < -1e-6;
    negative_inside = negative_inside && inside && negative;
    any_outside = any_outside || !inside;
    any_nonnegative = any_nonnegative || !negative;
  }

  ConnectedMooreBlockState recovered = state;
  ConnectedMooreBlockSolveCache reverse_cache;
  bool reverse_valid = true;
  for (int tick = 0; tick < kContinuationTicks; ++tick) {
    const auto reverse = ftd::eft::solve_connected_moore_block_reverse(
        recovered, options, &reverse_cache);
    reverse_valid = reverse_valid && reverse.valid
        && reverse.common_action_gates_pass;
    if (!reverse.valid) break;
    recovered = reverse.earlier;
  }
  result.inverse_recovery = reverse_valid
      ? ftd::eft::connected_moore_block_state_max_difference(
          original, recovered) : INFINITY;
  result.inverse_pass = reverse_valid && result.inverse_recovery <= 1e-8;
  result.survives = negative_inside && result.graph_transitions == 0
      && result.positive_field_energy && result.identity_pass
      && result.recoil_pass && result.pair_field_balance <= 1e-8
      && result.inverse_pass;
  if (result.survives)
    result.final_class = "survives";
  else if (any_outside && any_nonnegative)
    result.final_class = "graph_and_energy_failure";
  else if (any_outside)
    result.final_class = "graph_failure";
  else if (any_nonnegative)
    result.final_class = "energy_failure";
  else
    result.final_class = "transaction_failure";
  return result;
}

const ArmResult* find_arm(const std::vector<ArmResult>& arms, int volume,
                          const std::string& direction,
                          const std::string& polarity,
                          const std::string& variant) {
  const auto found = std::find_if(arms.begin(), arms.end(),
      [&](const ArmResult& arm) {
        return arm.volume == volume && arm.direction == direction
            && arm.polarity == polarity && arm.variant == variant;
      });
  return found == arms.end() ? nullptr : &*found;
}

std::vector<Selector> select_stress_arms(
    const std::vector<ArmResult>& arms) {
  std::vector<Selector> selectors;
  for (const auto& direction : kDirections) {
    for (const std::string polarity : {"plus_minus", "minus_plus"}) {
      std::vector<const ArmResult*> candidates;
      for (const auto& arm : arms)
        if (arm.volume == 33 && arm.direction == direction.label
            && arm.polarity == polarity && arm.variant != "center")
          candidates.push_back(&arm);
      std::sort(candidates.begin(), candidates.end(),
          [](const ArmResult* lhs, const ArmResult* rhs) {
            if (lhs->minimum_energy_margin != rhs->minimum_energy_margin)
              return lhs->minimum_energy_margin < rhs->minimum_energy_margin;
            return lhs->variant < rhs->variant;
          });
      const std::string energy = candidates.front()->variant;
      std::sort(candidates.begin(), candidates.end(),
          [](const ArmResult* lhs, const ArmResult* rhs) {
            if (lhs->minimum_graph_margin != rhs->minimum_graph_margin)
              return lhs->minimum_graph_margin < rhs->minimum_graph_margin;
            return lhs->variant < rhs->variant;
          });
      const auto graph = std::find_if(candidates.begin(), candidates.end(),
          [&](const ArmResult* arm) { return arm->variant != energy; });
      selectors.push_back({direction.label, polarity, energy,
                           (*graph)->variant});
    }
  }
  return selectors;
}

std::string join_ticks(const std::vector<int>& ticks) {
  std::ostringstream stream;
  for (std::size_t i = 0; i < ticks.size(); ++i) {
    if (i != 0) stream << ';';
    stream << ticks[i];
  }
  return stream.str();
}

std::string join_values(const std::vector<double>& values) {
  std::ostringstream stream;
  stream << std::setprecision(17);
  for (std::size_t i = 0; i < values.size(); ++i) {
    if (i != 0) stream << ';';
    stream << values[i];
  }
  return stream.str();
}

void write_records(const std::vector<ArmResult>& arms,
                   const std::vector<Selector>& selectors,
                   const std::string& verdict, int polarity_mismatches,
                   int volume_mismatches, int stage_a_survives,
                   int stage_b_survives, int center_survives,
                   double maximum_common, double maximum_recoil,
                   double maximum_inverse, double maximum_balance) {
  const auto directory = std::filesystem::path(__FILE__).parent_path()
      .parent_path() / "results" / "ftd_0732";
  std::filesystem::create_directories(directory);
  std::ofstream csv(directory /
      "ftd_0732_captured_state_perturbation_survival_v1.csv");
  csv << "volume,stage,direction,polarity,variant,selector,parent_valid,"
         "parent_reproduction_pass,initialized,executed,identity_pass,"
         "recoil_pass,inverse_pass,positive_field_energy,survives,"
         "final_class,graph_transitions,transition_ticks,initial_gauss_residual,"
         "initial_momentum_preservation,initial_maximum_speed,"
         "initial_pair_energy,final_pair_energy,energy_export,"
         "pair_field_balance,minimum_energy_margin,minimum_graph_margin,"
         "max_common_residual,max_recoil_defect,inverse_recovery,"
         "separation_history,internal_history,field_history\n"
      << std::setprecision(17);
  for (const auto& arm : arms)
    csv << arm.volume << ',' << arm.stage << ',' << arm.direction << ','
        << arm.polarity << ',' << arm.variant << ',' << arm.selector << ','
        << arm.parent_valid << ',' << arm.parent_reproduction_pass << ','
        << arm.initialized << ',' << arm.executed << ',' << arm.identity_pass
        << ',' << arm.recoil_pass << ',' << arm.inverse_pass << ','
        << arm.positive_field_energy << ',' << arm.survives << ','
        << arm.final_class << ',' << arm.graph_transitions << ','
        << join_ticks(arm.graph_transition_ticks) << ','
        << arm.initial_gauss_residual << ','
        << arm.initial_momentum_preservation << ','
        << arm.initial_maximum_speed << ',' << arm.initial_pair_energy << ','
        << arm.final_pair_energy << ','
        << arm.final_field_energy - arm.initial_field_energy << ','
        << arm.pair_field_balance << ',' << arm.minimum_energy_margin << ','
        << arm.minimum_graph_margin << ',' << arm.maximum_common_residual
        << ',' << arm.maximum_recoil_defect << ',' << arm.inverse_recovery
        << ',' << join_values(arm.separation_history) << ','
        << join_values(arm.internal_history) << ','
        << join_values(arm.field_history) << '\n';

  std::ofstream json(directory /
      "ftd_0732_captured_state_perturbation_survival_v1.json");
  json << std::setprecision(17)
       << "{\n  \"identifier\": \"FTD-0732\",\n"
       << "  \"protocol_sha256\": \"" << kProtocolSha256 << "\",\n"
       << "  \"verdict\": \"" << verdict << "\",\n"
       << "  \"arm_count\": " << arms.size() << ",\n"
       << "  \"stage_a_survives\": " << stage_a_survives << ",\n"
       << "  \"stage_b_survives\": " << stage_b_survives << ",\n"
       << "  \"center_survives\": " << center_survives << ",\n"
       << "  \"polarity_mismatches\": " << polarity_mismatches << ",\n"
       << "  \"volume_mismatches\": " << volume_mismatches << ",\n"
       << "  \"maximum_common\": " << maximum_common << ",\n"
       << "  \"maximum_recoil\": " << maximum_recoil << ",\n"
       << "  \"maximum_inverse\": " << maximum_inverse << ",\n"
       << "  \"maximum_balance\": " << maximum_balance << ",\n"
       << "  \"selectors\": [\n";
  for (std::size_t i = 0; i < selectors.size(); ++i) {
    const auto& selector = selectors[i];
    json << "    {\"direction\": \"" << selector.direction
         << "\", \"polarity\": \"" << selector.polarity
         << "\", \"energy_variant\": \"" << selector.energy_variant
         << "\", \"graph_variant\": \"" << selector.graph_variant
         << "\"}" << (i + 1 == selectors.size() ? "\n" : ",\n");
  }
  json << "  ]\n}\n";
}

}  // namespace

int main() {
  ConnectedMooreBlockOptions options;
  options.dt = 0.25;
  options.binding_law = ConnectedBindingLaw::DerivedCompactPair;
  options.compact_pair_well_depth = 0.01;
  options.compact_pair_cutoff_distance_squared = 1.5;
  options.allow_shared_anchor_chart = true;
  options.gate_tolerance = kGate;
  options.solve_tolerance = 2e-14;
  options.max_iterations = 384;
  options.use_sparse_local_current = true;
  options.use_local_residual_evaluation = true;
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  const double interaction_scale =
      normalization.mapped_field_work_coefficient;

  std::vector<ArmResult> arms;
  arms.reserve(84);
  for (const auto& direction : kDirections)
    for (bool conjugate : {false, true}) {
      const auto parent = build_parent(33, direction, conjugate, options);
      for (const auto& variant : kVariantNames)
        arms.push_back(run_continuation(
            33, "A", direction, conjugate, variant, "full_cross",
            parent, options, interaction_scale));
    }

  const auto selectors = select_stress_arms(arms);
  for (const auto& selector : selectors) {
    const auto direction = std::find_if(
        kDirections.begin(), kDirections.end(), [&](const Direction& item) {
          return item.label == selector.direction;
        });
    const bool conjugate = selector.polarity == "minus_plus";
    const auto parent = build_parent(65, *direction, conjugate, options);
    arms.push_back(run_continuation(
        65, "B", *direction, conjugate, "center", "center",
        parent, options, interaction_scale));
    arms.push_back(run_continuation(
        65, "B", *direction, conjugate, selector.energy_variant,
        "energy_stress", parent, options, interaction_scale));
    arms.push_back(run_continuation(
        65, "B", *direction, conjugate, selector.graph_variant,
        "graph_stress", parent, options, interaction_scale));
  }

  const bool matrix = normalization.valid && arms.size() == 84
      && std::count_if(arms.begin(), arms.end(), [](const ArmResult& arm) {
           return arm.volume == 33;
         }) == 66
      && std::count_if(arms.begin(), arms.end(), [](const ArmResult& arm) {
           return arm.volume == 65;
         }) == 18;
  const bool executed = matrix && std::all_of(
      arms.begin(), arms.end(), [](const ArmResult& arm) {
        return arm.parent_valid && arm.parent_reproduction_pass
            && arm.initialized && arm.executed;
      });
  const bool algebra = executed && std::all_of(
      arms.begin(), arms.end(), [](const ArmResult& arm) {
        return arm.identity_pass && arm.recoil_pass && arm.inverse_pass
            && arm.positive_field_energy
            && arm.pair_field_balance <= 1e-8;
      });

  int polarity_mismatches = 0;
  for (const auto& direction : kDirections)
    for (const auto& variant : kVariantNames) {
      const auto plus = find_arm(arms, 33, direction.label,
                                 "plus_minus", variant);
      const auto minus = find_arm(arms, 33, direction.label,
                                  "minus_plus", variant);
      polarity_mismatches += plus != nullptr && minus != nullptr
          && plus->survives != minus->survives ? 1 : 0;
    }

  int volume_mismatches = 0;
  for (const auto& arm : arms) {
    if (arm.volume != 65) continue;
    const auto smaller = find_arm(arms, 33, arm.direction,
                                  arm.polarity, arm.variant);
    bool match = smaller != nullptr && smaller->survives == arm.survives
        && smaller->graph_transition_ticks.size()
            == arm.graph_transition_ticks.size();
    if (match)
      for (std::size_t i = 0; i < arm.graph_transition_ticks.size(); ++i)
        match = match && std::abs(smaller->graph_transition_ticks[i]
                                 - arm.graph_transition_ticks[i]) <= 2;
    volume_mismatches += match ? 0 : 1;
  }

  const int stage_a_survives = static_cast<int>(std::count_if(
      arms.begin(), arms.end(), [](const ArmResult& arm) {
        return arm.volume == 33 && arm.survives;
      }));
  const int stage_b_survives = static_cast<int>(std::count_if(
      arms.begin(), arms.end(), [](const ArmResult& arm) {
        return arm.volume == 65 && arm.survives;
      }));
  const int center_survives = static_cast<int>(std::count_if(
      arms.begin(), arms.end(), [](const ArmResult& arm) {
        return arm.variant == "center" && arm.survives;
      }));
  double maximum_common = 0.0;
  double maximum_recoil = 0.0;
  double maximum_inverse = 0.0;
  double maximum_balance = 0.0;
  for (const auto& arm : arms) {
    if (!arm.executed) continue;
    maximum_common = std::max(maximum_common, arm.maximum_common_residual);
    maximum_recoil = std::max(maximum_recoil, arm.maximum_recoil_defect);
    maximum_inverse = std::max(maximum_inverse, arm.inverse_recovery);
    maximum_balance = std::max(maximum_balance, arm.pair_field_balance);
  }

  std::string verdict;
  if (!algebra)
    verdict = "CAPTURE_PERTURBATION_TRANSACTION_UNRESOLVED";
  else if (center_survives != 12)
    verdict = "CAPTURE_LONG_HORIZON_UNSTABLE";
  else if (polarity_mismatches != 0)
    verdict = "CAPTURE_PERTURBATION_POLARITY_SENSITIVE";
  else if (volume_mismatches != 0)
    verdict = "CAPTURE_PERTURBATION_VOLUME_SENSITIVE";
  else if (stage_a_survives == 66 && stage_b_survives == 18)
    verdict = "CAPTURE_FINITE_PERTURBATION_CROSS_SURVIVES";
  else
    verdict = "CAPTURE_FINITE_PERTURBATION_BOUNDARY_WITNESSED";

  write_records(arms, selectors, verdict, polarity_mismatches,
      volume_mismatches, stage_a_survives, stage_b_survives,
      center_survives, maximum_common, maximum_recoil,
      maximum_inverse, maximum_balance);
  std::cout << "FTD-0732 " << verdict
            << " stageA=" << stage_a_survives << "/66"
            << " stageB=" << stage_b_survives << "/18"
            << " centers=" << center_survives << "/12"
            << " polarity_mismatch=" << polarity_mismatches
            << " volume_mismatch=" << volume_mismatches << '\n';
  return executed ? 0 : 1;
}
