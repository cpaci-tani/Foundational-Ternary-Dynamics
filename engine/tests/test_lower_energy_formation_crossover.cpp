// FTD-0724: locked lower-energy formation-crossover campaign.

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
    "BCCCE4691FA5EBA22C61CB88554DC4972E07B0178C55396CA9A79ED98D3F4762";
constexpr int kL = 33;
constexpr int kTicks = 48;
constexpr double kGate = 1e-10;
constexpr double kBoundMomentum = 0.015;
constexpr std::array<double, 5> kUnboundMomenta{
    0.0060, 0.0075, 0.0085, 0.0095, 0.0120};

struct Direction {
  int x = 0;
  int y = 0;
  int z = 0;
  std::string label;
};

struct ArmResult {
  std::string family;
  std::string direction;
  std::string polarity;
  std::string translation;
  double momentum = 0.0;
  bool initialized = false;
  bool executed = false;
  bool identity_pass = false;
  bool inverse_pass = false;
  bool recoil_pass = false;
  bool bound_control_pass = false;
  bool negative_sector = false;
  bool outgoing_field_pass = false;
  bool captured = false;
  int graph_transitions = 0;
  int graph_active_ticks = 0;
  double initial_pair_internal = 0.0;
  double final_pair_internal = 0.0;
  double initial_field_energy = 0.0;
  double final_field_energy = 0.0;
  double pair_field_balance = INFINITY;
  double dynamic_field_norm = 0.0;
  double magnetic_energy = 0.0;
  int dynamic_median_radius2 = 0;
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
  int captured = 0;
  int negative_sector = 0;
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

MatchedMatterPoint point_at(const Vec3& position, const Vec3& momentum) {
  MatchedMatterPoint point;
  const long long ax = std::llround(position.x);
  const long long ay = std::llround(position.y);
  const long long az = std::llround(position.z);
  point.anchor = {wrap(static_cast<int>(ax), kL),
                  wrap(static_cast<int>(ay), kL),
                  wrap(static_cast<int>(az), kL)};
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

ConnectedMooreBlockState make_geometry(const Direction& direction,
                                       bool conjugate, bool translated,
                                       double separation, double momentum) {
  ConnectedMooreBlockState state(kL);
  const Vec3 shift = translated ? Vec3{4.0, -3.0, 2.0} : Vec3{};
  const Vec3 center{static_cast<double>(kL / 2) + shift.x,
                    static_cast<double>(kL / 2) + shift.y,
                    static_cast<double>(kL / 2) + shift.z};
  const Vec3 ray{static_cast<double>(direction.x),
                 static_cast<double>(direction.y),
                 static_cast<double>(direction.z)};
  const Vec3 unit = ray * (1.0 / ray.mag());
  state.constituents.push_back(point_at(
      center - unit * (0.5 * separation), unit * momentum));
  state.constituents.push_back(point_at(
      center + unit * (0.5 * separation), unit * (-momentum)));
  const int first = conjugate ? -1 : +1;
  state.charges = {first, -first};
  state.edges.clear();
  return state;
}

ArmResult run_arm(const std::string& family, double momentum,
                  const Direction& direction, bool conjugate,
                  bool translated, const ConnectedMooreBlockOptions& options,
                  double interaction_scale) {
  ArmResult result;
  result.family = family;
  result.direction = direction.label;
  result.polarity = conjugate ? "minus_plus" : "plus_minus";
  result.translation = translated ? "shifted" : "origin";
  result.momentum = momentum;
  const bool unbound = family == "unbound";
  const double separation = unbound ? 1.30 : 1.00;
  const auto initial = ftd::eft::redress_derived_compact_pair(
      make_geometry(direction, conjugate, translated, separation, momentum),
      options, 1e-13, 4096);
  result.initialized = initial.valid;
  if (!initial.valid) return result;

  ConnectedMooreBlockState state = initial.state;
  const ConnectedMooreBlockState original = state;
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
    if (step.relational_graph_changed) ++result.graph_transitions;
    const bool next_edge = step.relational_edge_after;
    if (next_edge) entered = true;
    if (entered && !next_edge) exited_after_entry = true;
    edge = next_edge;
    state = step.later;
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

  const auto final_dress = ftd::eft::redress_derived_compact_pair(
      state, options, 1e-13, 4096);
  if (!final_dress.valid) {
    result.executed = false;
    return result;
  }
  const Vec3 initial_center{
      static_cast<double>(kL / 2) + (translated ? 4.0 : 0.0),
      static_cast<double>(kL / 2) + (translated ? -3.0 : 0.0),
      static_cast<double>(kL / 2) + (translated ? 2.0 : 0.0)};
  const auto profile = ftd::eft::observe_component_aware_radial_field_profile(
      final_dress.state.electric, final_dress.state.magnetic_half,
      state.electric, state.magnetic_half, initial_center,
      interaction_scale, options.wave_speed, 1e-10);
  result.executed = result.executed && profile.valid;
  result.dynamic_field_norm = profile.total_norm;
  result.dynamic_median_radius2 = profile.doubled_radius_50;
  result.magnetic_energy = interaction_scale
      * ftd::eft::quadratic_energy(state.magnetic_half);
  result.outgoing_field_pass = profile.valid
      && result.dynamic_field_norm > 1e-8
      && result.magnetic_energy > 1e-10
      && result.dynamic_median_radius2 >= 4;

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
  if (unbound) {
    result.captured = result.initial_pair_internal > 1e-6
        && result.separation_history.front()
             * result.separation_history.front()
             >= options.compact_pair_cutoff_distance_squared
        && entered && !exited_after_entry && result.negative_sector
        && result.pair_field_balance <= 1e-8
        && result.outgoing_field_pass;
  } else {
    result.bound_control_pass = result.initial_pair_internal < -1e-6
        && !exited_after_entry && result.negative_sector;
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

double history_spread(const std::vector<ArmResult>& arms) {
  double result = 0.0;
  for (const auto& reference : arms) {
    if (reference.polarity != "plus_minus"
        || reference.translation != "origin") continue;
    for (const auto& arm : arms) {
      if (arm.family != reference.family
          || arm.direction != reference.direction
          || arm.momentum != reference.momentum) continue;
      if (arm.separation_history.size() != reference.separation_history.size()
          || arm.internal_history.size() != reference.internal_history.size()
          || arm.field_history.size() != reference.field_history.size())
        return INFINITY;
      for (std::size_t i = 0; i < arm.separation_history.size(); ++i) {
        result = std::max(result, std::abs(
            arm.separation_history[i] - reference.separation_history[i]));
        result = std::max(result, std::abs(
            arm.internal_history[i] - reference.internal_history[i]));
        result = std::max(result, std::abs(
            arm.field_history[i] - reference.field_history[i]));
      }
    }
  }
  return result;
}

std::array<MomentumSummary, kUnboundMomenta.size()> summarize_momenta(
    const std::vector<ArmResult>& arms) {
  std::array<MomentumSummary, kUnboundMomenta.size()> summaries{};
  for (std::size_t i = 0; i < kUnboundMomenta.size(); ++i)
    summaries[i].momentum = kUnboundMomenta[i];
  for (const auto& arm : arms) {
    if (arm.family != "unbound") continue;
    const auto found = std::find(kUnboundMomenta.begin(),
                                 kUnboundMomenta.end(), arm.momentum);
    if (found == kUnboundMomenta.end()) continue;
    auto& summary = summaries[static_cast<std::size_t>(
        std::distance(kUnboundMomenta.begin(), found))];
    ++summary.arms;
    summary.captured += arm.captured ? 1 : 0;
    summary.negative_sector += arm.negative_sector ? 1 : 0;
    summary.graph_transitions += arm.graph_transitions;
    const double export_energy =
        arm.final_field_energy - arm.initial_field_energy;
    summary.minimum_energy_export = std::min(
        summary.minimum_energy_export, export_energy);
    summary.maximum_energy_export = std::max(
        summary.maximum_energy_export, export_energy);
    summary.minimum_final_pair_internal = std::min(
        summary.minimum_final_pair_internal, arm.final_pair_internal);
    summary.maximum_final_pair_internal = std::max(
        summary.maximum_final_pair_internal, arm.final_pair_internal);
    summary.minimum_dynamic_field_norm = std::min(
        summary.minimum_dynamic_field_norm, arm.dynamic_field_norm);
    summary.maximum_dynamic_field_norm = std::max(
        summary.maximum_dynamic_field_norm, arm.dynamic_field_norm);
    summary.minimum_magnetic_energy = std::min(
        summary.minimum_magnetic_energy, arm.magnetic_energy);
    summary.maximum_magnetic_energy = std::max(
        summary.maximum_magnetic_energy, arm.magnetic_energy);
    summary.minimum_median_radius2 = std::min(
        summary.minimum_median_radius2, arm.dynamic_median_radius2);
    summary.maximum_median_radius2 = std::max(
        summary.maximum_median_radius2, arm.dynamic_median_radius2);
  }
  return summaries;
}

void write_records(
    const std::vector<ArmResult>& arms,
    const std::array<MomentumSummary, kUnboundMomenta.size()>& summaries,
    const std::string& verdict, double scalar_spread,
    bool monotone_negative_fraction, bool monotone_capture_fraction) {
  const auto directory = std::filesystem::path(__FILE__).parent_path()
      .parent_path() / "results" / "ftd_0724";
  std::filesystem::create_directories(directory);
  int captured = 0, negative = 0, bound = 0, identities = 0,
      inverses = 0, recoils = 0, executed = 0;
  double worst_common = 0.0, worst_recoil = 0.0,
      worst_inverse = 0.0, worst_balance = 0.0;
  for (const auto& arm : arms) {
    captured += arm.family == "unbound" && arm.captured ? 1 : 0;
    negative += arm.family == "unbound" && arm.negative_sector ? 1 : 0;
    bound += arm.bound_control_pass ? 1 : 0;
    identities += arm.identity_pass ? 1 : 0;
    inverses += arm.inverse_pass ? 1 : 0;
    recoils += arm.recoil_pass ? 1 : 0;
    executed += arm.executed ? 1 : 0;
    worst_common = std::max(worst_common, arm.maximum_common_residual);
    worst_recoil = std::max(worst_recoil, arm.maximum_recoil_defect);
    worst_inverse = std::max(worst_inverse, arm.inverse_recovery);
    worst_balance = std::max(worst_balance, arm.pair_field_balance);
  }

  std::ofstream csv(directory /
      "ftd_0724_lower_energy_formation_crossover_v1.csv");
  csv << "family,momentum,direction,polarity,translation,initialized,executed,"
         "identity_pass,inverse_pass,recoil_pass,bound_control_pass,"
         "negative_sector,outgoing_field_pass,captured,graph_transitions,"
         "active_ticks,initial_pair_internal,final_pair_internal,"
         "initial_field_energy,final_field_energy,energy_export,"
         "pair_field_balance,dynamic_field_norm,magnetic_energy,"
         "dynamic_median_radius2,inverse_recovery,max_common_residual,"
         "max_recoil_defect\n" << std::setprecision(17);
  for (const auto& arm : arms)
    csv << arm.family << ',' << arm.momentum << ',' << arm.direction << ','
        << arm.polarity << ',' << arm.translation << ',' << arm.initialized
        << ',' << arm.executed << ',' << arm.identity_pass << ','
        << arm.inverse_pass << ',' << arm.recoil_pass << ','
        << arm.bound_control_pass << ',' << arm.negative_sector << ','
        << arm.outgoing_field_pass << ',' << arm.captured << ','
        << arm.graph_transitions << ',' << arm.graph_active_ticks << ','
        << arm.initial_pair_internal << ',' << arm.final_pair_internal << ','
        << arm.initial_field_energy << ',' << arm.final_field_energy << ','
        << arm.final_field_energy - arm.initial_field_energy << ','
        << arm.pair_field_balance << ',' << arm.dynamic_field_norm << ','
        << arm.magnetic_energy << ',' << arm.dynamic_median_radius2 << ','
        << arm.inverse_recovery << ',' << arm.maximum_common_residual << ','
        << arm.maximum_recoil_defect << '\n';

  std::ofstream json(directory /
      "ftd_0724_lower_energy_formation_crossover_v1.json");
  json << std::setprecision(17)
       << "{\n  \"identifier\": \"FTD-0724\",\n"
       << "  \"protocol_sha256\": \"" << kProtocolSha256 << "\",\n"
       << "  \"verdict\": \"" << verdict << "\",\n"
       << "  \"arm_count\": " << arms.size() << ",\n"
       << "  \"executed_arms\": " << executed << ",\n"
       << "  \"identity_pass_arms\": " << identities << ",\n"
       << "  \"inverse_pass_arms\": " << inverses << ",\n"
       << "  \"recoil_pass_arms\": " << recoils << ",\n"
       << "  \"captured_unbound_arms\": " << captured << ",\n"
       << "  \"negative_sector_unbound_arms\": " << negative << ",\n"
       << "  \"bound_control_pass_arms\": " << bound << ",\n"
       << "  \"monotone_negative_fraction\": "
       << (monotone_negative_fraction ? "true" : "false") << ",\n"
       << "  \"monotone_capture_fraction\": "
       << (monotone_capture_fraction ? "true" : "false") << ",\n"
       << "  \"maximum_common_residual\": " << worst_common << ",\n"
       << "  \"maximum_recoil_defect\": " << worst_recoil << ",\n"
       << "  \"maximum_inverse_recovery\": " << worst_inverse << ",\n"
       << "  \"maximum_pair_field_balance\": " << worst_balance << ",\n"
       << "  \"maximum_scalar_history_spread\": " << scalar_spread << ",\n"
       << "  \"momentum_summaries\": [\n";
  for (std::size_t i = 0; i < summaries.size(); ++i) {
    const auto& summary = summaries[i];
    json << "    {\"momentum\": " << summary.momentum
         << ", \"arms\": " << summary.arms
         << ", \"captured\": " << summary.captured
         << ", \"negative_sector\": " << summary.negative_sector
         << ", \"graph_transitions\": " << summary.graph_transitions
         << ", \"minimum_energy_export\": "
         << summary.minimum_energy_export
         << ", \"maximum_energy_export\": "
         << summary.maximum_energy_export
         << ", \"minimum_final_pair_internal\": "
         << summary.minimum_final_pair_internal
         << ", \"maximum_final_pair_internal\": "
         << summary.maximum_final_pair_internal
         << ", \"minimum_dynamic_field_norm\": "
         << summary.minimum_dynamic_field_norm
         << ", \"maximum_dynamic_field_norm\": "
         << summary.maximum_dynamic_field_norm
         << ", \"minimum_magnetic_energy\": "
         << summary.minimum_magnetic_energy
         << ", \"maximum_magnetic_energy\": "
         << summary.maximum_magnetic_energy
         << ", \"minimum_median_radius2\": "
         << summary.minimum_median_radius2
         << ", \"maximum_median_radius2\": "
         << summary.maximum_median_radius2 << "}"
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
  options.solve_tolerance = 2e-11;
  options.max_iterations = 48;
  options.use_sparse_local_current = true;
  options.use_local_residual_evaluation = true;

  const auto normalization = ftd::eft::measure_face_flux_normalization();
  const double interaction_scale =
      normalization.mapped_field_work_coefficient;
  std::vector<ArmResult> arms;
  for (double momentum : kUnboundMomenta)
    for (const auto& direction : directions())
      for (bool conjugate : {false, true})
        for (bool translated : {false, true})
          arms.push_back(run_arm("unbound", momentum, direction, conjugate,
                                 translated, options, interaction_scale));
  for (const auto& direction : directions())
    for (bool conjugate : {false, true})
      for (bool translated : {false, true})
        arms.push_back(run_arm("bound", kBoundMomentum, direction, conjugate,
                               translated, options, interaction_scale));

  const double scalar_spread = history_spread(arms);
  const auto summaries = summarize_momenta(arms);
  const int captured = static_cast<int>(std::count_if(
      arms.begin(), arms.end(), [](const ArmResult& arm) {
        return arm.family == "unbound" && arm.captured;
      }));
  const int negative = static_cast<int>(std::count_if(
      arms.begin(), arms.end(), [](const ArmResult& arm) {
        return arm.family == "unbound" && arm.negative_sector;
      }));
  const int bound_pass = static_cast<int>(std::count_if(
      arms.begin(), arms.end(),
      [](const ArmResult& arm) { return arm.bound_control_pass; }));
  const bool executed = normalization.valid && arms.size() == 312
      && std::all_of(arms.begin(), arms.end(),
          [](const ArmResult& arm) { return arm.initialized && arm.executed; });
  const bool algebra = executed && scalar_spread <= 1e-9
      && std::all_of(arms.begin(), arms.end(), [](const ArmResult& arm) {
        return arm.identity_pass && arm.inverse_pass && arm.recoil_pass;
      });
  bool monotone_negative_fraction = true;
  bool monotone_capture_fraction = true;
  for (std::size_t i = 1; i < summaries.size(); ++i) {
    monotone_negative_fraction = monotone_negative_fraction
        && summaries[i].negative_sector
             <= summaries[i - 1].negative_sector;
    monotone_capture_fraction = monotone_capture_fraction
        && summaries[i].captured <= summaries[i - 1].captured;
  }
  const bool predicted_crossover = summaries[0].negative_sector == 52
      && summaries[1].negative_sector == 52
      && summaries[3].negative_sector == 0
      && summaries[4].negative_sector == 0;

  std::string verdict;
  if (!algebra)
    verdict = "LOWER_ENERGY_TRANSACTION_UNRESOLVED";
  else if (bound_pass != 52)
    verdict = "DERIVED_PAIR_BOUND_STATE_UNSTABLE_AT_48_TICKS";
  else if (!monotone_negative_fraction || !monotone_capture_fraction)
    verdict = "NONMONOTONE_LOWER_ENERGY_RESPONSE";
  else if (predicted_crossover)
    verdict = "PREDICTED_LOWER_ENERGY_CROSSOVER_CONFIRMED";
  else if (negative > 0)
    verdict = "LOWER_ENERGY_CROSSOVER_SHIFTED_OR_DIRECTIONAL";
  else
    verdict = "NO_LOWER_ENERGY_CROSSOVER_OBSERVED_LOCKED_V1";

  write_records(arms, summaries, verdict, scalar_spread,
                monotone_negative_fraction, monotone_capture_fraction);
  std::cout << "FTD-0724 " << verdict << " arms=" << arms.size()
            << " captured=" << captured << "/260"
            << " negative=" << negative << "/260"
            << " bound=" << bound_pass << "/52"
            << " negative_counts=";
  for (std::size_t i = 0; i < summaries.size(); ++i)
    std::cout << (i == 0 ? "" : "/") << summaries[i].negative_sector;
  std::cout << " capture_counts=";
  for (std::size_t i = 0; i < summaries.size(); ++i)
    std::cout << (i == 0 ? "" : "/") << summaries[i].captured;
  std::cout << " history_spread=" << std::setprecision(17)
            << scalar_spread << '\n';
  return executed ? 0 : 1;
}
