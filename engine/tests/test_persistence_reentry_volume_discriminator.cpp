// FTD-0730: locked persistence/re-entry volume discriminator.

#include "ftd/eft/component_aware_radial_field_profile.h"
#include "ftd/eft/connected_moore_block_action.h"
#include "ftd/eft/face_flux_normalization.h"

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

using ftd::Coord;
using ftd::Vec3;
using ftd::eft::ConnectedBindingLaw;
using ftd::eft::ConnectedMooreBlockOptions;
using ftd::eft::ConnectedMooreBlockSolveCache;
using ftd::eft::ConnectedMooreBlockState;
using ftd::eft::MatchedMatterPoint;

constexpr char kProtocolSha256[] =
    "50582DF6FAE3DBBC27AF4E9B271F4E141597BE04E1EF55FE0DF6C137C9ABEB83";
constexpr int kTicks = 96;
constexpr double kGate = 1e-10;
constexpr double kBoundMomentum = 0.015;
constexpr std::array<double, 3> kUnboundMomenta{
    0.0060, 0.0095, 0.0120};
constexpr std::array<int, 2> kVolumes{33, 65};

struct Direction {
  int x = 0;
  int y = 0;
  int z = 0;
  std::string label;
};

struct ArmResult {
  int volume = 0;
  std::string family;
  std::string direction;
  std::string polarity;
  double momentum = 0.0;
  bool initialized = false;
  bool executed = false;
  bool identity_pass = false;
  bool inverse_pass = false;
  bool recoil_pass = false;
  bool bound_control_pass = false;
  bool negative_sector = false;
  bool tail_persistent = false;
  bool localized_dressing_96 = false;
  bool escape_control_pass = false;
  int graph_transitions = 0;
  std::array<int, 3> graph_transition_ticks{{-1, -1, -1}};
  int graph_active_ticks = 0;
  double initial_pair_internal = 0.0;
  double final_pair_internal = 0.0;
  double initial_field_energy = 0.0;
  double final_field_energy = 0.0;
  double pair_field_balance = INFINITY;
  double dynamic_field_norm = 0.0;
  double magnetic_energy = 0.0;
  int dynamic_median_radius2 = 0;
  double dynamic_field_norm_48 = 0.0;
  double magnetic_energy_48 = 0.0;
  int dynamic_median_radius2_48 = 0;
  double inverse_recovery = INFINITY;
  double maximum_common_residual = 0.0;
  double maximum_recoil_defect = 0.0;
  std::vector<double> separation_history;
  std::vector<double> internal_history;
  std::vector<double> field_history;
};

struct MomentumSummary {
  double momentum = 0.0;
  int arms = 0;
  int negative_sector = 0;
  int tail_persistent = 0;
  int localized_dressing_96 = 0;
  int escape_control_pass = 0;
  int graph_transitions = 0;
  double minimum_energy_export = INFINITY;
  double maximum_energy_export = -INFINITY;
  double minimum_final_pair_internal = INFINITY;
  double maximum_final_pair_internal = -INFINITY;
  double minimum_dynamic_field_norm = INFINITY;
  double maximum_dynamic_field_norm = -INFINITY;
  double minimum_magnetic_energy = INFINITY;
  double maximum_magnetic_energy = -INFINITY;
  int minimum_median_radius2 = 1000000;
  int maximum_median_radius2 = -1000000;
  int minimum_median_radius2_48 = 1000000;
  int maximum_median_radius2_48 = -1000000;
};

struct FieldMorphology {
  bool valid = false;
  double dynamic_norm = 0.0;
  double magnetic_energy = 0.0;
  int doubled_median_radius = 0;
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

std::vector<Direction> directions() {
  std::vector<Direction> result;
  for (int x = -1; x <= 1; ++x)
    for (int y = -1; y <= 1; ++y)
      for (int z = -1; z <= 1; ++z) {
        if (x == 0 && y == 0 && z == 0) continue;
        const int first = x != 0 ? x : (y != 0 ? y : z);
        if (first < 0) continue;
        result.push_back({x, y, z, std::to_string(x) + "_"
              + std::to_string(y) + "_" + std::to_string(z)});
      }
  return result;
}

double pair_separation(const ConnectedMooreBlockState& state) {
  return (effective_position(state.constituents[1])
          - effective_position(state.constituents[0])).mag();
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

FieldMorphology observe_dynamic_field(
    const ConnectedMooreBlockState& state,
    const ConnectedMooreBlockOptions& options, const Vec3& center,
    double interaction_scale) {
  FieldMorphology result;
  const auto static_dress = ftd::eft::redress_derived_compact_pair(
      state, options, 1e-13, 4096);
  if (!static_dress.valid) return result;
  const auto profile = ftd::eft::observe_component_aware_radial_field_profile(
      static_dress.state.electric, static_dress.state.magnetic_half,
      state.electric, state.magnetic_half, center, interaction_scale,
      options.wave_speed, 1e-10);
  result.valid = profile.valid;
  result.dynamic_norm = profile.total_norm;
  result.magnetic_energy = interaction_scale
      * ftd::eft::quadratic_energy(state.magnetic_half);
  result.doubled_median_radius = profile.doubled_radius_50;
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
                                       bool conjugate,
                                       double separation, double momentum) {
  ConnectedMooreBlockState state(L);
  const Vec3 center{static_cast<double>(L / 2),
                    static_cast<double>(L / 2),
                    static_cast<double>(L / 2)};
  const Vec3 ray{static_cast<double>(direction.x),
                 static_cast<double>(direction.y),
                 static_cast<double>(direction.z)};
  const Vec3 unit = ray * (1.0 / ray.mag());
  state.constituents.push_back(point_at(
      center - unit * (0.5 * separation), unit * momentum, L));
  state.constituents.push_back(point_at(
      center + unit * (0.5 * separation), unit * (-momentum), L));
  const int first = conjugate ? -1 : +1;
  state.charges = {first, -first};
  state.edges.clear();
  return state;
}

ArmResult run_arm(int L, const std::string& family, double momentum,
                  const Direction& direction, bool conjugate,
                  const ConnectedMooreBlockOptions& options,
                  double interaction_scale) {
  ArmResult result;
  result.volume = L;
  result.family = family;
  result.direction = direction.label;
  result.polarity = conjugate ? "minus_plus" : "plus_minus";
  result.momentum = momentum;
  const bool unbound = family == "unbound";
  const double separation = unbound ? 1.30 : 1.00;
  const Vec3 initial_center{
      static_cast<double>(L / 2), static_cast<double>(L / 2),
      static_cast<double>(L / 2)};
  const auto initial = ftd::eft::redress_derived_compact_pair(
      make_geometry(L, direction, conjugate, separation, momentum),
      options, 1e-13, 4096);
  result.initialized = initial.valid;
  if (!initial.valid) return result;

  ConnectedMooreBlockState state = initial.state;
  const ConnectedMooreBlockState original = state;
  ConnectedMooreBlockState state_at_48 = state;
  result.initial_pair_internal = pair_internal_energy(state, options);
  result.initial_field_energy = field_energy(state, options,
                                              interaction_scale);
  result.separation_history.push_back(pair_separation(state));
  result.internal_history.push_back(result.initial_pair_internal);
  result.field_history.push_back(result.initial_field_energy);
  bool edge = pair_separation(state) * pair_separation(state)
      < options.compact_pair_cutoff_distance_squared;
  bool valid_roots = true;
  bool common = true;
  bool recoil = true;
  bool entered = edge;
  bool exited_after_entry = false;
  ConnectedMooreBlockSolveCache forward_cache;

  for (int tick = 0; tick < kTicks; ++tick) {
    if (edge) ++result.graph_active_ticks;
    const auto step = ftd::eft::solve_connected_moore_block_forward(
        state, options, &forward_cache);
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
    if (step.relational_graph_changed) {
      if (result.graph_transitions < 3)
        result.graph_transition_ticks[
            static_cast<std::size_t>(result.graph_transitions)] = tick + 1;
      ++result.graph_transitions;
    }
    const bool next_edge = step.relational_edge_after;
    if (next_edge) entered = true;
    if (entered && !next_edge) exited_after_entry = true;
    edge = next_edge;
    state = step.later;
    if (tick + 1 == 48) state_at_48 = state;
    result.separation_history.push_back(pair_separation(state));
    result.internal_history.push_back(pair_internal_energy(state, options));
    result.field_history.push_back(field_energy(state, options,
                                                interaction_scale));
  }

  result.executed = valid_roots
      && result.internal_history.size() == static_cast<std::size_t>(kTicks + 1);
  if (!result.executed) return result;
  result.identity_pass = common;
  result.recoil_pass = recoil;
  result.final_pair_internal = result.internal_history.back();
  result.final_field_energy = result.field_history.back();
  result.pair_field_balance = std::abs(
      result.final_field_energy - result.initial_field_energy
      + result.final_pair_internal - result.initial_pair_internal);

  const auto morphology_48 = observe_dynamic_field(
      state_at_48, options, initial_center, interaction_scale);
  const auto morphology_96 = observe_dynamic_field(
      state, options, initial_center, interaction_scale);
  if (!morphology_48.valid || !morphology_96.valid) {
    result.executed = false;
    return result;
  }
  result.dynamic_field_norm_48 = morphology_48.dynamic_norm;
  result.magnetic_energy_48 = morphology_48.magnetic_energy;
  result.dynamic_median_radius2_48 = morphology_48.doubled_median_radius;
  result.dynamic_field_norm = morphology_96.dynamic_norm;
  result.magnetic_energy = morphology_96.magnetic_energy;
  result.dynamic_median_radius2 = morphology_96.doubled_median_radius;

  const auto final_eight_begin = result.internal_history.end() - 8;
  const bool final_eight_negative = std::all_of(
      final_eight_begin, result.internal_history.end(),
      [](double value) { return value < -1e-6; });
  const bool final_eight_inside = std::all_of(
      result.separation_history.end() - 8,
      result.separation_history.end(), [&](double value) {
        return value * value
            < options.compact_pair_cutoff_distance_squared;
      });
  result.negative_sector = final_eight_negative && final_eight_inside;
  bool tail_negative = true;
  bool tail_inside = true;
  bool tail_positive = true;
  bool tail_outside = true;
  for (int tick = 49; tick <= kTicks; ++tick) {
    tail_negative = tail_negative
        && result.internal_history[static_cast<std::size_t>(tick)] < -1e-6;
    tail_positive = tail_positive
        && result.internal_history[static_cast<std::size_t>(tick)] > 1e-6;
    const double distance =
        result.separation_history[static_cast<std::size_t>(tick)];
    const bool inside = distance * distance
        < options.compact_pair_cutoff_distance_squared;
    tail_inside = tail_inside && inside;
    tail_outside = tail_outside && !inside;
  }
  result.tail_persistent = entered && !exited_after_entry
      && tail_negative && tail_inside;
  result.localized_dressing_96 = result.tail_persistent
      && result.dynamic_field_norm > 1e-8
      && result.magnetic_energy > 1e-10
      && result.dynamic_median_radius2 <= 4;
  if (unbound) {
    if (std::abs(momentum - 0.0120) < 1e-12)
      result.escape_control_pass = result.initial_pair_internal > 1e-6
          && entered && exited_after_entry && tail_positive && tail_outside;
  } else {
    result.bound_control_pass = result.initial_pair_internal < -1e-6
        && result.tail_persistent;
  }

  ConnectedMooreBlockSolveCache reverse_cache;
  ConnectedMooreBlockState recovered = state;
  bool reverse_valid = true;
  for (int tick = 0; tick < kTicks; ++tick) {
    const auto reverse = ftd::eft::solve_connected_moore_block_reverse(
        recovered, options, &reverse_cache);
    reverse_valid = reverse_valid && reverse.valid
        && reverse.common_action_gates_pass;
    if (!reverse.valid) break;
    recovered = reverse.earlier;
  }
  result.inverse_recovery = reverse_valid
      ? ftd::eft::connected_moore_block_state_max_difference(
          original, recovered)
      : INFINITY;
  result.inverse_pass = reverse_valid && result.inverse_recovery <= 1e-8;
  return result;
}

struct VolumeSummary {
  int volume = 0;
  int arms = 0;
  int executed = 0;
  int identities = 0;
  int inverses = 0;
  int recoils = 0;
  int bound_controls = 0;
  int parent_persistent = 0;
  int p012_arms = 0;
  int p012_reentered = 0;
  int p012_final_negative = 0;
  double maximum_common = 0.0;
  double maximum_recoil = 0.0;
  double maximum_inverse = 0.0;
  double maximum_balance = 0.0;
};

struct VolumeComparison {
  int matched_p012 = 0;
  int both_reentered = 0;
  int l65_reentered = 0;
  int matched_parents = 0;
  int persistent_both = 0;
  int maximum_third_transition_tick_difference = 0;
  int maximum_parent_radius_difference = 0;
};

bool principal_direction(const Direction& direction) {
  return direction.label == "0_0_1" || direction.label == "0_1_-1"
      || direction.label == "1_1_1";
}

const ArmResult* matched_arm(const std::vector<ArmResult>& arms,
                            const ArmResult& reference, int volume) {
  const auto found = std::find_if(arms.begin(), arms.end(),
      [&](const ArmResult& arm) {
        return arm.volume == volume && arm.family == reference.family
            && arm.momentum == reference.momentum
            && arm.direction == reference.direction
            && arm.polarity == reference.polarity;
      });
  return found == arms.end() ? nullptr : &*found;
}

VolumeSummary summarize_volume(const std::vector<ArmResult>& arms, int volume) {
  VolumeSummary summary;
  summary.volume = volume;
  for (const auto& arm : arms) {
    if (arm.volume != volume) continue;
    ++summary.arms;
    summary.executed += arm.executed ? 1 : 0;
    summary.identities += arm.identity_pass ? 1 : 0;
    summary.inverses += arm.inverse_pass ? 1 : 0;
    summary.recoils += arm.recoil_pass ? 1 : 0;
    summary.bound_controls += arm.bound_control_pass ? 1 : 0;
    const bool parent = arm.family == "unbound" && arm.momentum < 0.0120;
    summary.parent_persistent += parent && arm.tail_persistent ? 1 : 0;
    if (arm.family == "unbound"
        && std::abs(arm.momentum - 0.0120) < 1e-12) {
      ++summary.p012_arms;
      summary.p012_reentered += arm.graph_transitions >= 3 ? 1 : 0;
      summary.p012_final_negative += arm.negative_sector ? 1 : 0;
    }
    summary.maximum_common = std::max(
        summary.maximum_common, arm.maximum_common_residual);
    summary.maximum_recoil = std::max(
        summary.maximum_recoil, arm.maximum_recoil_defect);
    summary.maximum_inverse = std::max(
        summary.maximum_inverse, arm.inverse_recovery);
    summary.maximum_balance = std::max(
        summary.maximum_balance, arm.pair_field_balance);
  }
  return summary;
}

VolumeComparison compare_volumes(const std::vector<ArmResult>& arms) {
  VolumeComparison comparison;
  for (const auto& arm : arms) {
    if (arm.volume != 33) continue;
    const ArmResult* larger = matched_arm(arms, arm, 65);
    if (larger == nullptr) continue;
    if (arm.family == "unbound"
        && std::abs(arm.momentum - 0.0120) < 1e-12) {
      ++comparison.matched_p012;
      if (larger->graph_transitions >= 3) ++comparison.l65_reentered;
      if (arm.graph_transitions >= 3 && larger->graph_transitions >= 3) {
        ++comparison.both_reentered;
        comparison.maximum_third_transition_tick_difference = std::max(
            comparison.maximum_third_transition_tick_difference,
            std::abs(arm.graph_transition_ticks[2]
                     - larger->graph_transition_ticks[2]));
      }
    }
    if (arm.family == "unbound" && arm.momentum < 0.0120) {
      ++comparison.matched_parents;
      comparison.persistent_both +=
          arm.tail_persistent && larger->tail_persistent ? 1 : 0;
      comparison.maximum_parent_radius_difference = std::max(
          comparison.maximum_parent_radius_difference,
          std::abs(arm.dynamic_median_radius2
                   - larger->dynamic_median_radius2));
    }
  }
  return comparison;
}

void write_records(const std::vector<ArmResult>& arms,
                   const std::array<VolumeSummary, 2>& summaries,
                   const VolumeComparison& comparison,
                   const std::string& verdict) {
  const auto directory = std::filesystem::path(__FILE__).parent_path()
      .parent_path() / "results" / "ftd_0730";
  std::filesystem::create_directories(directory);
  std::ofstream csv(directory /
      "ftd_0730_persistence_reentry_volume_discriminator_v1.csv");
  csv << "volume,family,momentum,direction,polarity,initialized,executed,"
         "identity_pass,inverse_pass,recoil_pass,bound_control_pass,"
         "negative_sector,tail_persistent,localized_dressing_96,"
         "graph_transitions,transition_tick_1,transition_tick_2,"
         "transition_tick_3,active_ticks,initial_pair_internal,"
         "final_pair_internal,energy_export,pair_field_balance,"
         "dynamic_field_norm_48,magnetic_energy_48,"
         "dynamic_median_radius2_48,dynamic_field_norm_96,"
         "magnetic_energy_96,dynamic_median_radius2_96,inverse_recovery,"
         "max_common_residual,max_recoil_defect\n" << std::setprecision(17);
  for (const auto& arm : arms)
    csv << arm.volume << ',' << arm.family << ',' << arm.momentum << ','
        << arm.direction << ',' << arm.polarity << ',' << arm.initialized
        << ',' << arm.executed << ',' << arm.identity_pass << ','
        << arm.inverse_pass << ',' << arm.recoil_pass << ','
        << arm.bound_control_pass << ',' << arm.negative_sector << ','
        << arm.tail_persistent << ',' << arm.localized_dressing_96 << ','
        << arm.graph_transitions << ',' << arm.graph_transition_ticks[0]
        << ',' << arm.graph_transition_ticks[1] << ','
        << arm.graph_transition_ticks[2] << ',' << arm.graph_active_ticks
        << ',' << arm.initial_pair_internal << ',' << arm.final_pair_internal
        << ',' << arm.final_field_energy - arm.initial_field_energy << ','
        << arm.pair_field_balance << ',' << arm.dynamic_field_norm_48 << ','
        << arm.magnetic_energy_48 << ',' << arm.dynamic_median_radius2_48
        << ',' << arm.dynamic_field_norm << ',' << arm.magnetic_energy << ','
        << arm.dynamic_median_radius2 << ',' << arm.inverse_recovery << ','
        << arm.maximum_common_residual << ',' << arm.maximum_recoil_defect
        << '\n';

  std::ofstream json(directory /
      "ftd_0730_persistence_reentry_volume_discriminator_v1.json");
  json << std::setprecision(17)
       << "{\n  \"identifier\": \"FTD-0730\",\n"
       << "  \"protocol_sha256\": \"" << kProtocolSha256 << "\",\n"
       << "  \"verdict\": \"" << verdict << "\",\n"
       << "  \"arm_count\": " << arms.size() << ",\n"
       << "  \"matched_p012\": " << comparison.matched_p012 << ",\n"
       << "  \"both_reentered\": " << comparison.both_reentered << ",\n"
       << "  \"l65_reentered\": " << comparison.l65_reentered << ",\n"
       << "  \"matched_parents\": " << comparison.matched_parents << ",\n"
       << "  \"persistent_both\": " << comparison.persistent_both << ",\n"
       << "  \"maximum_third_transition_tick_difference\": "
       << comparison.maximum_third_transition_tick_difference << ",\n"
       << "  \"maximum_parent_radius_difference\": "
       << comparison.maximum_parent_radius_difference << ",\n"
       << "  \"volumes\": [\n";
  for (std::size_t i = 0; i < summaries.size(); ++i) {
    const auto& s = summaries[i];
    json << "    {\"volume\": " << s.volume
         << ", \"arms\": " << s.arms
         << ", \"executed\": " << s.executed
         << ", \"identities\": " << s.identities
         << ", \"inverses\": " << s.inverses
         << ", \"recoils\": " << s.recoils
         << ", \"bound_controls\": " << s.bound_controls
         << ", \"parent_persistent\": " << s.parent_persistent
         << ", \"p012_arms\": " << s.p012_arms
         << ", \"p012_reentered\": " << s.p012_reentered
         << ", \"p012_final_negative\": " << s.p012_final_negative
         << ", \"maximum_common\": " << s.maximum_common
         << ", \"maximum_recoil\": " << s.maximum_recoil
         << ", \"maximum_inverse\": " << s.maximum_inverse
         << ", \"maximum_balance\": " << s.maximum_balance << "}"
         << (i + 1 == summaries.size() ? "\n" : ",\n");
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
  const auto all_directions = directions();
  std::vector<ArmResult> arms;
  arms.reserve(88);
  for (int volume : kVolumes) {
    for (const auto& direction : all_directions)
      for (bool conjugate : {false, true})
        arms.push_back(run_arm(volume, "unbound", 0.0120, direction,
                               conjugate, options, interaction_scale));
    for (double momentum : {0.0060, 0.0095})
      for (const auto& direction : all_directions)
        if (principal_direction(direction))
          for (bool conjugate : {false, true})
            arms.push_back(run_arm(volume, "unbound", momentum, direction,
                                   conjugate, options, interaction_scale));
    for (const auto& direction : all_directions)
      if (principal_direction(direction))
        for (bool conjugate : {false, true})
          arms.push_back(run_arm(volume, "bound", kBoundMomentum, direction,
                                 conjugate, options, interaction_scale));
  }

  const std::array<VolumeSummary, 2> summaries{{
      summarize_volume(arms, 33), summarize_volume(arms, 65)}};
  const auto comparison = compare_volumes(arms);
  const bool matrix = normalization.valid && arms.size() == 88
      && summaries[0].arms == 44 && summaries[1].arms == 44;
  const bool executed = matrix && std::all_of(
      arms.begin(), arms.end(), [](const ArmResult& arm) {
        return arm.initialized && arm.executed;
      });
  const bool algebra = executed && std::all_of(
      arms.begin(), arms.end(), [](const ArmResult& arm) {
        return arm.identity_pass && arm.inverse_pass && arm.recoil_pass
            && arm.pair_field_balance <= 1e-8;
      });
  const bool l33_reproduction = summaries[0].parent_persistent == 12
      && summaries[0].p012_reentered == 26
      && summaries[0].bound_controls == 6
      && std::all_of(arms.begin(), arms.end(), [](const ArmResult& arm) {
        if (arm.volume != 33) return true;
        if (arm.family == "bound") return arm.graph_transitions == 0;
        if (std::abs(arm.momentum - 0.0120) < 1e-12)
          return arm.graph_transitions == 3;
        return arm.graph_transitions == 1 && arm.tail_persistent
            && arm.dynamic_median_radius2_48 == 3
            && arm.dynamic_median_radius2 >= 5
            && arm.dynamic_median_radius2 <= 6;
      });
  std::string verdict;
  if (!algebra || !l33_reproduction || summaries[1].bound_controls != 6)
    verdict = "VOLUME_DISCRIMINATOR_UNRESOLVED";
  else if (summaries[1].parent_persistent != 12)
    verdict = "LOWER_ENERGY_PERSISTENCE_VOLUME_SENSITIVE";
  else if (comparison.maximum_parent_radius_difference > 1)
    verdict = "PERSISTENT_CORE_FIELD_MORPHOLOGY_VOLUME_SENSITIVE";
  else if (comparison.l65_reentered == 0)
    verdict = "P012_REENTRY_FINITE_VOLUME_RECURRENCE";
  else if (comparison.l65_reentered == 26
           && comparison.maximum_third_transition_tick_difference <= 2)
    verdict = "P012_REENTRY_LOCAL_DYNAMICS_VOLUME_STABLE";
  else if (comparison.l65_reentered == 26)
    verdict = "P012_REENTRY_VOLUME_DEPENDENT_TIMING";
  else
    verdict = "P012_REENTRY_DIRECTIONAL_VOLUME_SPLIT";

  write_records(arms, summaries, comparison, verdict);
  std::cout << "FTD-0730 " << verdict
            << " parent_persistent=" << summaries[0].parent_persistent
            << '/' << summaries[1].parent_persistent
            << " p012_reentry=" << summaries[0].p012_reentered
            << '/' << summaries[1].p012_reentered
            << " p012_negative=" << summaries[0].p012_final_negative
            << '/' << summaries[1].p012_final_negative
            << " max_transition_shift="
            << comparison.maximum_third_transition_tick_difference
            << " max_radius_shift="
            << comparison.maximum_parent_radius_difference << '\n';
  return executed ? 0 : 1;
}

