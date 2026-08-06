// FTD-0725: locked translation-covariance conditioning diagnostic.

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

using ftd::Vec3;
using ftd::eft::ConnectedBindingLaw;
using ftd::eft::ConnectedMooreBlockOptions;
using ftd::eft::ConnectedMooreBlockSolveCache;
using ftd::eft::ConnectedMooreBlockState;
using ftd::eft::MatchedMatterPoint;

constexpr char kProtocolSha256[] =
    "712F491F72E9F30239060406FAA85EBB0F3635DFD3A8BD2143CBF68249A7DCB9";
constexpr int kL = 33;
constexpr int kTicks = 48;
constexpr double kGate = 1e-10;
constexpr double kParentScalarSpread = 1.0680766715509549e-8;
constexpr std::array<double, 5> kUnboundMomenta{
    0.0060, 0.0075, 0.0085, 0.0095, 0.0120};

struct Direction {
  int x = 0;
  int y = 0;
  int z = 0;
  std::string label;
};

struct Condition {
  std::string label;
  double solve_tolerance = 0.0;
  int max_iterations = 0;
};

struct TranslationDefects {
  double electric = 0.0;
  double magnetic = 0.0;
  double matter = 0.0;
  double complete = 0.0;
};

struct TickRecord {
  std::string condition;
  std::string family;
  double momentum = 0.0;
  std::string direction;
  int tick = 0;
  double separation_difference = 0.0;
  double internal_difference = 0.0;
  double field_difference = 0.0;
  double scalar_difference = 0.0;
  double electric_difference = 0.0;
  double magnetic_difference = 0.0;
  double matter_difference = 0.0;
  double complete_state_difference = 0.0;
  double root_common_residual = 0.0;
  double recoil_defect = 0.0;
  bool origin_edge = false;
  bool shifted_edge = false;
  double origin_internal = 0.0;
  double shifted_internal = 0.0;
};

struct PairResult {
  std::string condition;
  std::string family;
  double momentum = 0.0;
  std::string direction;
  bool initialized = false;
  bool executed = false;
  bool gates_pass = false;
  bool class_agreement = false;
  bool origin_negative = false;
  bool shifted_negative = false;
  bool origin_bound_control = false;
  bool shifted_bound_control = false;
  double maximum_scalar_difference = 0.0;
  double maximum_complete_difference = 0.0;
  double maximum_common_residual = 0.0;
  double maximum_recoil_defect = 0.0;
};

struct ConditionSummary {
  std::string label;
  int pairs = 0;
  int histories = 0;
  int executed_histories = 0;
  int gate_pass_histories = 0;
  int class_agreement_pairs = 0;
  int negative_unbound_histories = 0;
  int bound_control_pass_histories = 0;
  double maximum_scalar_difference = 0.0;
  double maximum_electric_difference = 0.0;
  double maximum_magnetic_difference = 0.0;
  double maximum_matter_difference = 0.0;
  double maximum_complete_difference = 0.0;
  double maximum_common_residual = 0.0;
  double maximum_recoil_defect = 0.0;
  std::string worst_scalar_family;
  double worst_scalar_momentum = 0.0;
  std::string worst_scalar_direction;
  int worst_scalar_tick = 0;
  std::string worst_scalar_component;
  std::string worst_complete_family;
  double worst_complete_momentum = 0.0;
  std::string worst_complete_direction;
  int worst_complete_tick = 0;
  std::string worst_complete_component;
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

double maximum_component(const Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
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
                                       bool translated, double separation,
                                       double momentum) {
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
  state.charges = {+1, -1};
  state.edges.clear();
  return state;
}

TranslationDefects translation_defects(
    const ConnectedMooreBlockState& origin,
    const ConnectedMooreBlockState& shifted) {
  TranslationDefects result;
  if (origin.electric.L != kL || shifted.electric.L != kL
      || origin.magnetic_half.L != kL || shifted.magnetic_half.L != kL
      || origin.constituents.size() != shifted.constituents.size()
      || origin.charges != shifted.charges
      || origin.edges.size() != shifted.edges.size()) {
    result.electric = result.magnetic = result.matter =
        result.complete = INFINITY;
    return result;
  }
  constexpr int dx = 4, dy = -3, dz = 2;
  for (int x = 0; x < kL; ++x)
    for (int y = 0; y < kL; ++y)
      for (int z = 0; z < kL; ++z) {
        const int a = origin.electric.index(x, y, z);
        const int b = shifted.electric.index(x + dx, y + dy, z + dz);
        result.electric = std::max({result.electric,
            std::abs(origin.electric.x[a] - shifted.electric.x[b]),
            std::abs(origin.electric.y[a] - shifted.electric.y[b]),
            std::abs(origin.electric.z[a] - shifted.electric.z[b])});
        result.magnetic = std::max({result.magnetic,
            std::abs(origin.magnetic_half.x[a] - shifted.magnetic_half.x[b]),
            std::abs(origin.magnetic_half.y[a] - shifted.magnetic_half.y[b]),
            std::abs(origin.magnetic_half.z[a] - shifted.magnetic_half.z[b])});
      }
  for (std::size_t i = 0; i < origin.constituents.size(); ++i) {
    const auto& a = origin.constituents[i];
    const auto& b = shifted.constituents[i];
    const Vec3 anchor_defect{
        static_cast<double>(wrap(a.anchor.x + dx, kL) - b.anchor.x),
        static_cast<double>(wrap(a.anchor.y + dy, kL) - b.anchor.y),
        static_cast<double>(wrap(a.anchor.z + dz, kL) - b.anchor.z)};
    result.matter = std::max({result.matter,
        maximum_component(anchor_defect),
        maximum_component(a.remainder - b.remainder),
        maximum_component(a.momentum - b.momentum)});
  }
  result.complete = std::max({result.electric, result.magnetic,
                              result.matter});
  return result;
}

std::string scalar_component(double separation, double internal,
                             double field) {
  if (separation >= internal && separation >= field) return "separation";
  if (internal >= field) return "pair_internal";
  return "field_energy";
}

std::string complete_component(const TranslationDefects& defects) {
  if (defects.electric >= defects.magnetic
      && defects.electric >= defects.matter) return "electric";
  if (defects.magnetic >= defects.matter) return "magnetic";
  return "matter";
}

void append_tick(const Condition& condition, const std::string& family,
                 double momentum, const Direction& direction, int tick,
                 const ConnectedMooreBlockState& origin,
                 const ConnectedMooreBlockState& shifted,
                 const ConnectedMooreBlockOptions& options,
                 double interaction_scale, double residual,
                 double recoil, std::vector<TickRecord>* records) {
  TickRecord record;
  record.condition = condition.label;
  record.family = family;
  record.momentum = momentum;
  record.direction = direction.label;
  record.tick = tick;
  record.separation_difference = std::abs(
      pair_separation(origin) - pair_separation(shifted));
  record.origin_internal = pair_internal_energy(origin, options);
  record.shifted_internal = pair_internal_energy(shifted, options);
  record.internal_difference = std::abs(
      record.origin_internal - record.shifted_internal);
  record.field_difference = std::abs(
      field_energy(origin, options, interaction_scale)
      - field_energy(shifted, options, interaction_scale));
  record.scalar_difference = std::max({record.separation_difference,
      record.internal_difference, record.field_difference});
  const auto defects = translation_defects(origin, shifted);
  record.electric_difference = defects.electric;
  record.magnetic_difference = defects.magnetic;
  record.matter_difference = defects.matter;
  record.complete_state_difference = defects.complete;
  record.root_common_residual = residual;
  record.recoil_defect = recoil;
  record.origin_edge = pair_separation(origin) * pair_separation(origin)
      < options.compact_pair_cutoff_distance_squared;
  record.shifted_edge = pair_separation(shifted) * pair_separation(shifted)
      < options.compact_pair_cutoff_distance_squared;
  records->push_back(record);
}

bool final_negative(const std::vector<double>& energy,
                    const std::vector<bool>& edge) {
  if (energy.size() < 8 || edge.size() != energy.size()) return false;
  for (std::size_t i = energy.size() - 8; i < energy.size(); ++i)
    if (!(energy[i] < -1e-6 && edge[i])) return false;
  return true;
}

PairResult run_pair(const Condition& condition, const std::string& family,
                    double momentum, const Direction& direction,
                    ConnectedMooreBlockOptions options,
                    double interaction_scale,
                    std::vector<TickRecord>* records) {
  PairResult result;
  result.condition = condition.label;
  result.family = family;
  result.momentum = momentum;
  result.direction = direction.label;
  options.solve_tolerance = condition.solve_tolerance;
  options.max_iterations = condition.max_iterations;
  const bool unbound = family == "unbound";
  const double separation = unbound ? 1.30 : 1.00;
  const auto origin_initial = ftd::eft::redress_derived_compact_pair(
      make_geometry(direction, false, separation, momentum),
      options, 1e-13, 4096);
  const auto shifted_initial = ftd::eft::redress_derived_compact_pair(
      make_geometry(direction, true, separation, momentum),
      options, 1e-13, 4096);
  result.initialized = origin_initial.valid && shifted_initial.valid;
  if (!result.initialized) return result;

  ConnectedMooreBlockState origin = origin_initial.state;
  ConnectedMooreBlockState shifted = shifted_initial.state;
  std::vector<double> origin_energy{pair_internal_energy(origin, options)};
  std::vector<double> shifted_energy{pair_internal_energy(shifted, options)};
  std::vector<bool> origin_edge{
      pair_separation(origin) * pair_separation(origin)
          < options.compact_pair_cutoff_distance_squared};
  std::vector<bool> shifted_edge{
      pair_separation(shifted) * pair_separation(shifted)
          < options.compact_pair_cutoff_distance_squared};
  append_tick(condition, family, momentum, direction, 0, origin, shifted,
              options, interaction_scale, 0.0, 0.0, records);
  ConnectedMooreBlockSolveCache origin_cache, shifted_cache;
  bool valid = true;
  bool gates = true;

  for (int tick = 1; tick <= kTicks; ++tick) {
    const auto origin_step = ftd::eft::solve_connected_moore_block_forward(
        origin, options, &origin_cache);
    const auto shifted_step = ftd::eft::solve_connected_moore_block_forward(
        shifted, options, &shifted_cache);
    valid = valid && origin_step.valid && shifted_step.valid;
    if (!valid) break;
    const double residual = std::max(maximum_step_residual(origin_step),
                                     maximum_step_residual(shifted_step));
    const double recoil = std::max({origin_step.matter_momentum_after.mag(),
        origin_step.spline_defect_norm,
        shifted_step.matter_momentum_after.mag(),
        shifted_step.spline_defect_norm});
    result.maximum_common_residual = std::max(
        result.maximum_common_residual, residual);
    result.maximum_recoil_defect = std::max(
        result.maximum_recoil_defect, recoil);
    gates = gates && origin_step.common_action_gates_pass
        && shifted_step.common_action_gates_pass
        && residual <= kGate && recoil <= 1e-9;
    origin = origin_step.later;
    shifted = shifted_step.later;
    origin_energy.push_back(pair_internal_energy(origin, options));
    shifted_energy.push_back(pair_internal_energy(shifted, options));
    origin_edge.push_back(pair_separation(origin) * pair_separation(origin)
        < options.compact_pair_cutoff_distance_squared);
    shifted_edge.push_back(pair_separation(shifted) * pair_separation(shifted)
        < options.compact_pair_cutoff_distance_squared);
    append_tick(condition, family, momentum, direction, tick, origin, shifted,
                options, interaction_scale, residual, recoil, records);
  }

  result.executed = valid
      && origin_energy.size() == static_cast<std::size_t>(kTicks + 1);
  result.gates_pass = result.executed && gates;
  if (!result.executed) return result;
  const auto begin = records->end() - (kTicks + 1);
  for (auto it = begin; it != records->end(); ++it) {
    result.maximum_scalar_difference = std::max(
        result.maximum_scalar_difference, it->scalar_difference);
    result.maximum_complete_difference = std::max(
        result.maximum_complete_difference, it->complete_state_difference);
  }
  result.origin_negative = final_negative(origin_energy, origin_edge);
  result.shifted_negative = final_negative(shifted_energy, shifted_edge);
  result.class_agreement = result.origin_negative == result.shifted_negative;
  if (!unbound) {
    result.origin_bound_control = origin_energy.front() < -1e-6
        && result.origin_negative;
    result.shifted_bound_control = shifted_energy.front() < -1e-6
        && result.shifted_negative;
    result.class_agreement = result.class_agreement
        && result.origin_bound_control == result.shifted_bound_control;
  }
  return result;
}

ConditionSummary summarize(const std::string& label,
                           const std::vector<PairResult>& pairs,
                           const std::vector<TickRecord>& records) {
  ConditionSummary summary;
  summary.label = label;
  for (const auto& pair : pairs) {
    if (pair.condition != label) continue;
    ++summary.pairs;
    summary.histories += 2;
    summary.executed_histories += pair.executed ? 2 : 0;
    summary.gate_pass_histories += pair.gates_pass ? 2 : 0;
    summary.class_agreement_pairs += pair.class_agreement ? 1 : 0;
    if (pair.family == "unbound")
      summary.negative_unbound_histories +=
          (pair.origin_negative ? 1 : 0) + (pair.shifted_negative ? 1 : 0);
    else
      summary.bound_control_pass_histories +=
          (pair.origin_bound_control ? 1 : 0)
          + (pair.shifted_bound_control ? 1 : 0);
    summary.maximum_common_residual = std::max(
        summary.maximum_common_residual, pair.maximum_common_residual);
    summary.maximum_recoil_defect = std::max(
        summary.maximum_recoil_defect, pair.maximum_recoil_defect);
  }
  for (const auto& record : records) {
    if (record.condition != label) continue;
    summary.maximum_electric_difference = std::max(
        summary.maximum_electric_difference, record.electric_difference);
    summary.maximum_magnetic_difference = std::max(
        summary.maximum_magnetic_difference, record.magnetic_difference);
    summary.maximum_matter_difference = std::max(
        summary.maximum_matter_difference, record.matter_difference);
    if (record.scalar_difference > summary.maximum_scalar_difference) {
      summary.maximum_scalar_difference = record.scalar_difference;
      summary.worst_scalar_family = record.family;
      summary.worst_scalar_momentum = record.momentum;
      summary.worst_scalar_direction = record.direction;
      summary.worst_scalar_tick = record.tick;
      summary.worst_scalar_component = scalar_component(
          record.separation_difference, record.internal_difference,
          record.field_difference);
    }
    if (record.complete_state_difference
        > summary.maximum_complete_difference) {
      summary.maximum_complete_difference = record.complete_state_difference;
      summary.worst_complete_family = record.family;
      summary.worst_complete_momentum = record.momentum;
      summary.worst_complete_direction = record.direction;
      summary.worst_complete_tick = record.tick;
      TranslationDefects defects;
      defects.electric = record.electric_difference;
      defects.magnetic = record.magnetic_difference;
      defects.matter = record.matter_difference;
      summary.worst_complete_component = complete_component(defects);
    }
  }
  return summary;
}

void write_records(const std::vector<TickRecord>& records,
                   const std::array<ConditionSummary, 2>& summaries,
                   const std::string& verdict) {
  const auto directory = std::filesystem::path(__FILE__).parent_path()
      .parent_path() / "results" / "ftd_0725";
  std::filesystem::create_directories(directory);
  std::ofstream csv(directory /
      "ftd_0725_lower_energy_covariance_conditioning_v1.csv");
  csv << "condition,family,momentum,direction,tick,separation_difference,"
         "internal_difference,field_difference,scalar_difference,"
         "electric_difference,magnetic_difference,matter_difference,"
         "complete_state_difference,root_common_residual,recoil_defect,"
         "origin_edge,shifted_edge,origin_internal,shifted_internal\n"
      << std::setprecision(17);
  for (const auto& record : records)
    csv << record.condition << ',' << record.family << ',' << record.momentum
        << ',' << record.direction << ',' << record.tick << ','
        << record.separation_difference << ',' << record.internal_difference
        << ',' << record.field_difference << ',' << record.scalar_difference
        << ',' << record.electric_difference << ','
        << record.magnetic_difference << ',' << record.matter_difference
        << ',' << record.complete_state_difference << ','
        << record.root_common_residual << ',' << record.recoil_defect << ','
        << record.origin_edge << ',' << record.shifted_edge << ','
        << record.origin_internal << ',' << record.shifted_internal << '\n';

  const double scalar_ratio = summaries[1].maximum_scalar_difference
      / summaries[0].maximum_scalar_difference;
  const double complete_ratio = summaries[1].maximum_complete_difference
      / summaries[0].maximum_complete_difference;
  std::ofstream json(directory /
      "ftd_0725_lower_energy_covariance_conditioning_v1.json");
  json << std::setprecision(17)
       << "{\n  \"identifier\": \"FTD-0725\",\n"
       << "  \"protocol_sha256\": \"" << kProtocolSha256 << "\",\n"
       << "  \"verdict\": \"" << verdict << "\",\n"
       << "  \"tick_record_count\": " << records.size() << ",\n"
       << "  \"tight_to_baseline_scalar_ratio\": " << scalar_ratio << ",\n"
       << "  \"tight_to_baseline_complete_ratio\": " << complete_ratio
       << ",\n  \"conditions\": [\n";
  for (std::size_t i = 0; i < summaries.size(); ++i) {
    const auto& s = summaries[i];
    json << "    {\"label\": \"" << s.label
         << "\", \"pairs\": " << s.pairs
         << ", \"histories\": " << s.histories
         << ", \"executed_histories\": " << s.executed_histories
         << ", \"gate_pass_histories\": " << s.gate_pass_histories
         << ", \"class_agreement_pairs\": " << s.class_agreement_pairs
         << ", \"negative_unbound_histories\": "
         << s.negative_unbound_histories
         << ", \"bound_control_pass_histories\": "
         << s.bound_control_pass_histories
         << ", \"maximum_scalar_difference\": "
         << s.maximum_scalar_difference
         << ", \"maximum_electric_difference\": "
         << s.maximum_electric_difference
         << ", \"maximum_magnetic_difference\": "
         << s.maximum_magnetic_difference
         << ", \"maximum_matter_difference\": "
         << s.maximum_matter_difference
         << ", \"maximum_complete_difference\": "
         << s.maximum_complete_difference
         << ", \"maximum_common_residual\": "
         << s.maximum_common_residual
         << ", \"maximum_recoil_defect\": "
         << s.maximum_recoil_defect
         << ", \"worst_scalar_family\": \""
         << s.worst_scalar_family
         << "\", \"worst_scalar_momentum\": "
         << s.worst_scalar_momentum
         << ", \"worst_scalar_direction\": \""
         << s.worst_scalar_direction
         << "\", \"worst_scalar_tick\": " << s.worst_scalar_tick
         << ", \"worst_scalar_component\": \""
         << s.worst_scalar_component
         << "\", \"worst_complete_family\": \""
         << s.worst_complete_family
         << "\", \"worst_complete_momentum\": "
         << s.worst_complete_momentum
         << ", \"worst_complete_direction\": \""
         << s.worst_complete_direction
         << "\", \"worst_complete_tick\": "
         << s.worst_complete_tick
         << ", \"worst_complete_component\": \""
         << s.worst_complete_component << "\"}"
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
  options.use_sparse_local_current = true;
  options.use_local_residual_evaluation = true;
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  const double interaction_scale =
      normalization.mapped_field_work_coefficient;
  const std::array<Condition, 2> conditions{{
      {"baseline", 2e-11, 48}, {"tight", 2e-12, 96}}};
  std::vector<TickRecord> records;
  std::vector<PairResult> pairs;
  records.reserve(2 * 78 * (kTicks + 1));
  pairs.reserve(2 * 78);
  for (const auto& condition : conditions) {
    for (double momentum : kUnboundMomenta)
      for (const auto& direction : directions())
        pairs.push_back(run_pair(condition, "unbound", momentum, direction,
                                 options, interaction_scale, &records));
    for (const auto& direction : directions())
      pairs.push_back(run_pair(condition, "bound", 0.015, direction,
                               options, interaction_scale, &records));
  }
  const std::array<ConditionSummary, 2> summaries{{
      summarize("baseline", pairs, records),
      summarize("tight", pairs, records)}};
  const auto& baseline = summaries[0];
  const auto& tight = summaries[1];
  const bool matrix = normalization.valid && pairs.size() == 156
      && records.size() == 2 * 78 * (kTicks + 1)
      && baseline.pairs == 78 && tight.pairs == 78;
  const bool algebra = matrix
      && baseline.executed_histories == 156
      && tight.executed_histories == 156
      && baseline.gate_pass_histories == 156
      && tight.gate_pass_histories == 156;
  const bool reproduction = algebra
      && std::abs(baseline.maximum_scalar_difference - kParentScalarSpread)
          <= 1e-12;
  const bool classes = baseline.class_agreement_pairs == 78
      && tight.class_agreement_pairs == 78
      && baseline.negative_unbound_histories == 104
      && tight.negative_unbound_histories == 104
      && baseline.bound_control_pass_histories == 26
      && tight.bound_control_pass_histories == 26;
  const double scalar_ratio = tight.maximum_scalar_difference
      / baseline.maximum_scalar_difference;
  const double complete_ratio = tight.maximum_complete_difference
      / baseline.maximum_complete_difference;
  const bool fivefold = scalar_ratio <= 0.2 && complete_ratio <= 0.2;

  std::string verdict;
  if (!reproduction)
    verdict = "CONDITIONING_DIAGNOSTIC_UNRESOLVED";
  else if (!classes || tight.maximum_complete_difference > 1e-8
           || !fivefold)
    verdict = "LONG_INTERACTION_COVARIANCE_DEFECT_PERSISTS";
  else if (tight.maximum_scalar_difference <= 1e-9
           && tight.maximum_complete_difference <= 1e-9)
    verdict = "COVARIANCE_DEFECT_NUMERICAL_CONDITIONING_CONFIRMED";
  else
    verdict = "COVARIANCE_CONVERGENCE_INCOMPLETE";

  write_records(records, summaries, verdict);
  std::cout << "FTD-0725 " << verdict
            << " baseline_scalar=" << std::setprecision(17)
            << baseline.maximum_scalar_difference
            << " tight_scalar=" << tight.maximum_scalar_difference
            << " baseline_complete=" << baseline.maximum_complete_difference
            << " tight_complete=" << tight.maximum_complete_difference
            << " ratios=" << scalar_ratio << '/' << complete_ratio
            << " raw_negative=" << baseline.negative_unbound_histories
            << '/' << tight.negative_unbound_histories << '\n';
  return matrix && algebra ? 0 : 1;
}
