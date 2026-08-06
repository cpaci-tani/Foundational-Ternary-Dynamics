// FTD-0731: locked multi-pass formation persistence campaign.

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
#include <sstream>
#include <string>
#include <vector>

namespace {

using ftd::Vec3;
using ftd::eft::ConnectedBindingLaw;
using ftd::eft::ConnectedMooreBlockOptions;
using ftd::eft::ConnectedMooreBlockSolveCache;
using ftd::eft::ConnectedMooreBlockState;
using ftd::eft::MatchedMatterPoint;

constexpr char kProtocolSha256[] =
    "F319B4CA5C0A8F9A777578507828FC0881E996023FD09AA83033D797B47C01EE";
constexpr int kTicks = 192;
constexpr double kGate = 1e-10;
constexpr double kBoundMomentum = 0.015;
constexpr std::array<int, 2> kVolumes{{33, 65}};
constexpr std::array<int, 5> kMorphologyTicks{{48, 96, 128, 160, 192}};

struct Direction {
  int x;
  int y;
  int z;
  const char* label;
  int parent_reentry_tick;
};

constexpr std::array<Direction, 3> kDirections{{
    {0, 0, 1, "0_0_1", 63},
    {0, 1, -1, "0_1_-1", 79},
    {1, 1, 1, "1_1_1", 96},
}};

struct FieldMorphology {
  bool valid = false;
  double dynamic_norm = 0.0;
  double magnetic_energy = 0.0;
  int doubled_median_radius = 0;
};

struct ArmResult {
  int volume = 0;
  std::string family;
  std::string direction;
  std::string polarity;
  double momentum = 0.0;
  int expected_parent_reentry_tick = -1;
  bool initialized = false;
  bool executed = false;
  bool identity_pass = false;
  bool inverse_pass = false;
  bool recoil_pass = false;
  bool bound_control_pass = false;
  bool extended_persistent = false;
  bool durable_multipass_capture = false;
  bool recurrent_scattering = false;
  bool later_release = false;
  bool initial_positive_outside = false;
  bool final_negative_inside_tail = false;
  bool morphology_receiver_evidence = false;
  bool positive_field_gain = false;
  int graph_active_ticks = 0;
  std::vector<int> graph_transition_ticks;
  std::string final_class = "unclassified";
  double initial_pair_internal = 0.0;
  double final_pair_internal = 0.0;
  double initial_field_energy = 0.0;
  double final_field_energy = 0.0;
  double pair_field_balance = INFINITY;
  double inverse_recovery = INFINITY;
  double maximum_common_residual = 0.0;
  double maximum_recoil_defect = 0.0;
  std::array<bool, 5> morphology_valid{{false, false, false, false, false}};
  std::array<double, 5> dynamic_field_norm{{0, 0, 0, 0, 0}};
  std::array<double, 5> magnetic_energy{{0, 0, 0, 0, 0}};
  std::array<int, 5> dynamic_median_radius2{{0, 0, 0, 0, 0}};
  std::vector<double> separation_history;
  std::vector<double> internal_history;
  std::vector<double> field_history;
};

struct VolumeSummary {
  int volume = 0;
  int arms = 0;
  int executed = 0;
  int identities = 0;
  int inverses = 0;
  int recoils = 0;
  int parent_persistent = 0;
  int bound_controls = 0;
  int p012_arms = 0;
  int durable_capture = 0;
  int recurrent_scattering = 0;
  int later_release = 0;
  int p012_other = 0;
  double maximum_common = 0.0;
  double maximum_recoil = 0.0;
  double maximum_inverse = 0.0;
  double maximum_balance = 0.0;
};

struct VolumeComparison {
  int matched_arms = 0;
  int matched_p012 = 0;
  int transition_count_mismatches = 0;
  int transition_timing_mismatches = 0;
  int final_class_mismatches = 0;
  int maximum_transition_tick_difference = 0;
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

MatchedMatterPoint point_at(const Vec3& position, const Vec3& momentum, int L) {
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

bool graph_inside(double separation, const ConnectedMooreBlockOptions& options) {
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
                                       bool conjugate, double separation,
                                       double momentum) {
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

bool negative_inside_range(const ArmResult& result, int first, int last,
                           const ConnectedMooreBlockOptions& options) {
  for (int tick = first; tick <= last; ++tick) {
    const auto i = static_cast<std::size_t>(tick);
    if (!(result.internal_history[i] < -1e-6)
        || !graph_inside(result.separation_history[i], options)) return false;
  }
  return true;
}

bool positive_outside_range(const ArmResult& result, int first, int last,
                            const ConnectedMooreBlockOptions& options) {
  for (int tick = first; tick <= last; ++tick) {
    const auto i = static_cast<std::size_t>(tick);
    if (!(result.internal_history[i] > 1e-6)
        || graph_inside(result.separation_history[i], options)) return false;
  }
  return true;
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
  result.expected_parent_reentry_tick = direction.parent_reentry_tick;
  const bool unbound = family == "unbound";
  const double separation = unbound ? 1.30 : 1.00;
  const Vec3 initial_center{static_cast<double>(L / 2),
                            static_cast<double>(L / 2),
                            static_cast<double>(L / 2)};
  const auto initial = ftd::eft::redress_derived_compact_pair(
      make_geometry(L, direction, conjugate, separation, momentum),
      options, 1e-13, 4096);
  result.initialized = initial.valid;
  if (!initial.valid) return result;

  ConnectedMooreBlockState state = initial.state;
  const ConnectedMooreBlockState original = state;
  result.initial_pair_internal = pair_internal_energy(state, options);
  result.initial_field_energy = field_energy(state, options, interaction_scale);
  result.separation_history.push_back(pair_separation(state));
  result.internal_history.push_back(result.initial_pair_internal);
  result.field_history.push_back(result.initial_field_energy);
  bool edge = graph_inside(result.separation_history.front(), options);
  result.initial_positive_outside = result.initial_pair_internal > 1e-6
      && !edge;
  bool valid_roots = true;
  bool common = true;
  bool recoil = true;
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
    if (step.relational_graph_changed)
      result.graph_transition_ticks.push_back(tick + 1);
    edge = step.relational_edge_after;
    state = step.later;
    result.separation_history.push_back(pair_separation(state));
    result.internal_history.push_back(pair_internal_energy(state, options));
    result.field_history.push_back(field_energy(
        state, options, interaction_scale));
    const auto found = std::find(
        kMorphologyTicks.begin(), kMorphologyTicks.end(), tick + 1);
    if (found != kMorphologyTicks.end()) {
      const auto index = static_cast<std::size_t>(
          std::distance(kMorphologyTicks.begin(), found));
      const auto morphology = observe_dynamic_field(
          state, options, initial_center, interaction_scale);
      result.morphology_valid[index] = morphology.valid;
      result.dynamic_field_norm[index] = morphology.dynamic_norm;
      result.magnetic_energy[index] = morphology.magnetic_energy;
      result.dynamic_median_radius2[index] =
          morphology.doubled_median_radius;
    }
  }

  result.executed = valid_roots
      && result.internal_history.size() == static_cast<std::size_t>(kTicks + 1)
      && std::all_of(result.morphology_valid.begin(),
                     result.morphology_valid.end(),
                     [](bool value) { return value; });
  if (!result.executed) return result;
  result.identity_pass = common;
  result.recoil_pass = recoil;
  result.final_pair_internal = result.internal_history.back();
  result.final_field_energy = result.field_history.back();
  result.pair_field_balance = std::abs(
      result.final_field_energy - result.initial_field_energy
      + result.final_pair_internal - result.initial_pair_internal);
  result.positive_field_gain =
      result.final_field_energy > result.initial_field_energy;
  result.morphology_receiver_evidence = false;
  for (std::size_t i = 0; i < kMorphologyTicks.size(); ++i)
    result.morphology_receiver_evidence = result.morphology_receiver_evidence
        || (result.dynamic_field_norm[i] > 1e-8
            && result.magnetic_energy[i] > 1e-10
            && result.dynamic_median_radius2[i] >= 5);

  result.final_negative_inside_tail = negative_inside_range(
      result, 129, 192, options);
  const bool final_transition_is_entry =
      !result.graph_transition_ticks.empty()
      && result.graph_transition_ticks.size() % 2 == 1
      && graph_inside(result.separation_history.back(), options);
  const bool parent_sequence = result.graph_transition_ticks.size() >= 3;
  if (unbound && std::abs(momentum - 0.0120) < 1e-12) {
    result.durable_multipass_capture = result.initial_positive_outside
        && parent_sequence && final_transition_is_entry
        && result.final_negative_inside_tail && result.positive_field_gain
        && result.morphology_receiver_evidence;
    result.recurrent_scattering = !result.durable_multipass_capture
        && result.graph_transition_ticks.size() >= 4
        && !result.final_negative_inside_tail;
    bool negative_inside_after_reentry = false;
    if (parent_sequence) {
      const int first = result.graph_transition_ticks[2];
      for (int tick = first; tick <= kTicks; ++tick) {
        const auto i = static_cast<std::size_t>(tick);
        negative_inside_after_reentry = negative_inside_after_reentry
            || (result.internal_history[i] < -1e-6
                && graph_inside(result.separation_history[i], options));
      }
    }
    const bool final_transition_is_exit =
        !result.graph_transition_ticks.empty()
        && result.graph_transition_ticks.size() % 2 == 0
        && !graph_inside(result.separation_history.back(), options);
    result.later_release = !result.durable_multipass_capture
        && negative_inside_after_reentry && final_transition_is_exit
        && positive_outside_range(result, 185, 192, options);
    if (result.durable_multipass_capture)
      result.final_class = "durable_multipass_capture";
    else if (result.later_release)
      result.final_class = "later_release";
    else if (result.recurrent_scattering)
      result.final_class = "recurrent_scattering";
    else
      result.final_class = "p012_other";
  } else {
    result.extended_persistent = negative_inside_range(result, 97, 192,
                                                       options);
    if (family == "bound") {
      result.bound_control_pass = result.initial_pair_internal < -1e-6
          && result.extended_persistent;
      result.final_class = result.bound_control_pass
          ? "bound_persistent" : "bound_unstable";
    } else {
      result.final_class = result.extended_persistent
          ? "parent_persistent" : "parent_unstable";
    }
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
          original, recovered) : INFINITY;
  result.inverse_pass = reverse_valid && result.inverse_recovery <= 1e-8;
  return result;
}

bool is_p012(const ArmResult& arm) {
  return arm.family == "unbound"
      && std::abs(arm.momentum - 0.0120) < 1e-12;
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
    summary.parent_persistent += arm.family == "unbound"
        && arm.momentum < 0.0120 && arm.extended_persistent ? 1 : 0;
    if (is_p012(arm)) {
      ++summary.p012_arms;
      summary.durable_capture += arm.durable_multipass_capture ? 1 : 0;
      summary.recurrent_scattering += arm.recurrent_scattering ? 1 : 0;
      summary.later_release += arm.later_release ? 1 : 0;
      summary.p012_other += arm.final_class == "p012_other" ? 1 : 0;
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
    ++comparison.matched_arms;
    if (is_p012(arm)) ++comparison.matched_p012;
    if (arm.graph_transition_ticks.size()
        != larger->graph_transition_ticks.size()) {
      ++comparison.transition_count_mismatches;
    } else {
      bool timing_mismatch = false;
      for (std::size_t i = 0; i < arm.graph_transition_ticks.size(); ++i) {
        const int shift = std::abs(arm.graph_transition_ticks[i]
                                   - larger->graph_transition_ticks[i]);
        comparison.maximum_transition_tick_difference = std::max(
            comparison.maximum_transition_tick_difference, shift);
        timing_mismatch = timing_mismatch || shift > 2;
      }
      comparison.transition_timing_mismatches += timing_mismatch ? 1 : 0;
    }
    comparison.final_class_mismatches +=
        arm.final_class != larger->final_class ? 1 : 0;
  }
  return comparison;
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
                   const std::array<VolumeSummary, 2>& summaries,
                   const VolumeComparison& comparison,
                   const std::string& verdict) {
  const auto directory = std::filesystem::path(__FILE__).parent_path()
      .parent_path() / "results" / "ftd_0731";
  std::filesystem::create_directories(directory);
  std::ofstream csv(directory /
      "ftd_0731_multipass_formation_persistence_v1.csv");
  csv << "volume,family,momentum,direction,polarity,initialized,executed,"
         "identity_pass,inverse_pass,recoil_pass,bound_control_pass,"
         "extended_persistent,durable_multipass_capture,recurrent_scattering,"
         "later_release,initial_positive_outside,final_negative_inside_tail,"
         "positive_field_gain,morphology_receiver_evidence,final_class,"
         "graph_transitions,transition_ticks,active_ticks,initial_pair_internal,"
         "final_pair_internal,energy_export,pair_field_balance,inverse_recovery,"
         "max_common_residual,max_recoil_defect,separation_history,"
         "internal_history,field_history";
  for (int tick : kMorphologyTicks)
    csv << ",dynamic_norm_" << tick << ",magnetic_energy_" << tick
        << ",median_radius2_" << tick;
  csv << '\n' << std::setprecision(17);
  for (const auto& arm : arms) {
    csv << arm.volume << ',' << arm.family << ',' << arm.momentum << ','
        << arm.direction << ',' << arm.polarity << ',' << arm.initialized
        << ',' << arm.executed << ',' << arm.identity_pass << ','
        << arm.inverse_pass << ',' << arm.recoil_pass << ','
        << arm.bound_control_pass << ',' << arm.extended_persistent << ','
        << arm.durable_multipass_capture << ',' << arm.recurrent_scattering
        << ',' << arm.later_release << ',' << arm.initial_positive_outside
        << ',' << arm.final_negative_inside_tail << ','
        << arm.positive_field_gain << ',' << arm.morphology_receiver_evidence
        << ',' << arm.final_class << ',' << arm.graph_transition_ticks.size()
        << ',' << join_ticks(arm.graph_transition_ticks) << ','
        << arm.graph_active_ticks << ',' << arm.initial_pair_internal << ','
        << arm.final_pair_internal << ','
        << arm.final_field_energy - arm.initial_field_energy << ','
        << arm.pair_field_balance << ',' << arm.inverse_recovery << ','
        << arm.maximum_common_residual << ',' << arm.maximum_recoil_defect
        << ',' << join_values(arm.separation_history)
        << ',' << join_values(arm.internal_history)
        << ',' << join_values(arm.field_history);
    for (std::size_t i = 0; i < kMorphologyTicks.size(); ++i)
      csv << ',' << arm.dynamic_field_norm[i] << ',' << arm.magnetic_energy[i]
          << ',' << arm.dynamic_median_radius2[i];
    csv << '\n';
  }

  std::ofstream json(directory /
      "ftd_0731_multipass_formation_persistence_v1.json");
  json << std::setprecision(17)
       << "{\n  \"identifier\": \"FTD-0731\",\n"
       << "  \"protocol_sha256\": \"" << kProtocolSha256 << "\",\n"
       << "  \"verdict\": \"" << verdict << "\",\n"
       << "  \"arm_count\": " << arms.size() << ",\n"
       << "  \"matched_arms\": " << comparison.matched_arms << ",\n"
       << "  \"matched_p012\": " << comparison.matched_p012 << ",\n"
       << "  \"transition_count_mismatches\": "
       << comparison.transition_count_mismatches << ",\n"
       << "  \"transition_timing_mismatches\": "
       << comparison.transition_timing_mismatches << ",\n"
       << "  \"final_class_mismatches\": "
       << comparison.final_class_mismatches << ",\n"
       << "  \"maximum_transition_tick_difference\": "
       << comparison.maximum_transition_tick_difference << ",\n"
       << "  \"volumes\": [\n";
  for (std::size_t i = 0; i < summaries.size(); ++i) {
    const auto& s = summaries[i];
    json << "    {\"volume\": " << s.volume
         << ", \"arms\": " << s.arms
         << ", \"executed\": " << s.executed
         << ", \"identities\": " << s.identities
         << ", \"inverses\": " << s.inverses
         << ", \"recoils\": " << s.recoils
         << ", \"parent_persistent\": " << s.parent_persistent
         << ", \"bound_controls\": " << s.bound_controls
         << ", \"p012_arms\": " << s.p012_arms
         << ", \"durable_capture\": " << s.durable_capture
         << ", \"recurrent_scattering\": " << s.recurrent_scattering
         << ", \"later_release\": " << s.later_release
         << ", \"p012_other\": " << s.p012_other
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
  std::vector<ArmResult> arms;
  arms.reserve(48);
  for (int volume : kVolumes) {
    for (double momentum : {0.0060, 0.0095, 0.0120})
      for (const auto& direction : kDirections)
        for (bool conjugate : {false, true})
          arms.push_back(run_arm(volume, "unbound", momentum, direction,
                                 conjugate, options, interaction_scale));
    for (const auto& direction : kDirections)
      for (bool conjugate : {false, true})
        arms.push_back(run_arm(volume, "bound", kBoundMomentum, direction,
                               conjugate, options, interaction_scale));
  }

  const std::array<VolumeSummary, 2> summaries{{
      summarize_volume(arms, 33), summarize_volume(arms, 65)}};
  const auto comparison = compare_volumes(arms);
  const bool matrix = normalization.valid && arms.size() == 48
      && summaries[0].arms == 24 && summaries[1].arms == 24
      && comparison.matched_arms == 24;
  const bool executed = matrix && std::all_of(
      arms.begin(), arms.end(), [](const ArmResult& arm) {
        return arm.initialized && arm.executed;
      });
  const bool algebra = executed && std::all_of(
      arms.begin(), arms.end(), [](const ArmResult& arm) {
        return arm.identity_pass && arm.inverse_pass && arm.recoil_pass
            && arm.pair_field_balance <= 1e-8;
      });
  const bool controls = summaries[0].parent_persistent == 12
      && summaries[1].parent_persistent == 12
      && summaries[0].bound_controls == 6
      && summaries[1].bound_controls == 6;
  const bool parent_reproduction = std::all_of(
      arms.begin(), arms.end(), [](const ArmResult& arm) {
        if (!is_p012(arm)) return true;
        return arm.graph_transition_ticks.size() >= 3
            && std::abs(arm.graph_transition_ticks[0] - 7) <= 2
            && std::abs(arm.graph_transition_ticks[1] - 26) <= 2
            && std::abs(arm.graph_transition_ticks[2]
                        - arm.expected_parent_reentry_tick) <= 2;
      });
  const bool volume_match = comparison.transition_count_mismatches == 0
      && comparison.transition_timing_mismatches == 0
      && comparison.final_class_mismatches == 0;
  const bool all_durable = summaries[0].durable_capture == 6
      && summaries[1].durable_capture == 6;
  int matched_durable_directions = 0;
  for (const auto& direction : kDirections) {
    bool direction_durable = true;
    for (const auto& arm : arms)
      if (is_p012(arm) && arm.direction == direction.label)
        direction_durable = direction_durable
            && arm.durable_multipass_capture;
    matched_durable_directions += direction_durable ? 1 : 0;
  }
  const bool all_recurrent = summaries[0].recurrent_scattering == 6
      && summaries[1].recurrent_scattering == 6;
  const bool any_release = summaries[0].later_release > 0
      || summaries[1].later_release > 0;

  std::string verdict;
  if (!algebra || !parent_reproduction)
    verdict = "MULTIPASS_FORMATION_TRANSACTION_UNRESOLVED";
  else if (!controls)
    verdict = "LONG_HORIZON_BOUND_CORE_UNSTABLE";
  else if (!volume_match)
    verdict = "MULTIPASS_DYNAMICS_VOLUME_SENSITIVE";
  else if (all_durable)
    verdict = "MULTIPASS_RADIATIVE_CAPTURE_VOLUME_STABLE";
  else if (matched_durable_directions > 0)
    verdict = "DIRECTIONAL_MULTIPASS_CAPTURE_VOLUME_STABLE";
  else if (all_recurrent)
    verdict = "VOLUME_STABLE_RECURRENT_SCATTERING";
  else if (any_release)
    verdict = "DIRECTIONAL_LATER_RELEASE";
  else
    verdict = "MULTIPASS_FORMATION_TRANSACTION_UNRESOLVED";

  write_records(arms, summaries, comparison, verdict);
  std::cout << "FTD-0731 " << verdict
            << " durable=" << summaries[0].durable_capture << '/'
            << summaries[1].durable_capture
            << " recurrent=" << summaries[0].recurrent_scattering << '/'
            << summaries[1].recurrent_scattering
            << " release=" << summaries[0].later_release << '/'
            << summaries[1].later_release
            << " parent=" << summaries[0].parent_persistent << '/'
            << summaries[1].parent_persistent
            << " bound=" << summaries[0].bound_controls << '/'
            << summaries[1].bound_controls
            << " max_transition_shift="
            << comparison.maximum_transition_tick_difference << '\n';
  return executed ? 0 : 1;
}
