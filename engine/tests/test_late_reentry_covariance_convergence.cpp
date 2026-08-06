// FTD-0729: locked late-reentry covariance convergence diagnostic.

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
    "96751A97197E6F52625FFECD53CF7B66752960290968530196E9A8F9A52AD384";
constexpr int kL = 33;
constexpr int kTicks = 96;
constexpr double kMomentum = 0.0120;
constexpr double kGate = 1e-10;
constexpr double kParentTightScalar = 5.6798055148021831e-10;

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
  std::string polarity;
  int tick = 0;
  double separation_difference = 0.0;
  double internal_difference = 0.0;
  double field_difference = 0.0;
  double scalar_difference = 0.0;
  double electric_difference = 0.0;
  double magnetic_difference = 0.0;
  double matter_difference = 0.0;
  double complete_difference = 0.0;
  double common_residual = 0.0;
  double recoil_defect = 0.0;
  bool origin_edge = false;
  bool shifted_edge = false;
  double origin_internal = 0.0;
  double shifted_internal = 0.0;
};

struct PairResult {
  std::string condition;
  std::string polarity;
  bool initialized = false;
  bool executed = false;
  bool gates_pass = false;
  bool class_agreement = false;
  int origin_graph_transitions = 0;
  int shifted_graph_transitions = 0;
  bool origin_negative = false;
  bool shifted_negative = false;
  double maximum_scalar = 0.0;
  double maximum_electric = 0.0;
  double maximum_magnetic = 0.0;
  double maximum_matter = 0.0;
  double maximum_complete = 0.0;
  double maximum_common = 0.0;
  double maximum_recoil = 0.0;
  int worst_scalar_tick = 0;
  std::string worst_scalar_component;
  int worst_complete_tick = 0;
  std::string worst_complete_component;
};

struct ConditionSummary {
  std::string label;
  int pairs = 0;
  int histories = 0;
  int executed_histories = 0;
  int gate_pass_histories = 0;
  int class_agreement_pairs = 0;
  int graph_transitions = 0;
  int final_negative_histories = 0;
  double plus_minus_scalar = 0.0;
  double maximum_scalar = 0.0;
  double maximum_electric = 0.0;
  double maximum_magnetic = 0.0;
  double maximum_matter = 0.0;
  double maximum_complete = 0.0;
  double maximum_common = 0.0;
  double maximum_recoil = 0.0;
  std::string worst_scalar_polarity;
  int worst_scalar_tick = 0;
  std::string worst_scalar_component;
  std::string worst_complete_polarity;
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

ConnectedMooreBlockState make_geometry(bool conjugate, bool translated) {
  ConnectedMooreBlockState state(kL);
  const Vec3 shift = translated ? Vec3{4.0, -3.0, 2.0} : Vec3{};
  const Vec3 center{static_cast<double>(kL / 2) + shift.x,
                    static_cast<double>(kL / 2) + shift.y,
                    static_cast<double>(kL / 2) + shift.z};
  const Vec3 ray{0.0, 1.0, -1.0};
  const Vec3 unit = ray * (1.0 / ray.mag());
  state.constituents.push_back(point_at(
      center - unit * 0.65, unit * kMomentum));
  state.constituents.push_back(point_at(
      center + unit * 0.65, unit * (-kMomentum)));
  const int first = conjugate ? -1 : +1;
  state.charges = {first, -first};
  state.edges.clear();
  return state;
}

TranslationDefects translation_defects(
    const ConnectedMooreBlockState& origin,
    const ConnectedMooreBlockState& shifted) {
  TranslationDefects result;
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

std::string maximum_scalar_component(double separation, double internal,
                                     double field) {
  if (separation >= internal && separation >= field) return "separation";
  if (internal >= field) return "pair_internal";
  return "field_energy";
}

std::string maximum_complete_component(const TranslationDefects& defects) {
  if (defects.electric >= defects.magnetic
      && defects.electric >= defects.matter) return "electric";
  if (defects.magnetic >= defects.matter) return "magnetic";
  return "matter";
}

void append_record(const Condition& condition, const std::string& polarity,
                   int tick, const ConnectedMooreBlockState& origin,
                   const ConnectedMooreBlockState& shifted,
                   const ConnectedMooreBlockOptions& options,
                   double interaction_scale, double residual, double recoil,
                   std::vector<TickRecord>* records) {
  TickRecord record;
  record.condition = condition.label;
  record.polarity = polarity;
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
  record.complete_difference = defects.complete;
  record.common_residual = residual;
  record.recoil_defect = recoil;
  record.origin_edge = pair_separation(origin) * pair_separation(origin)
      < options.compact_pair_cutoff_distance_squared;
  record.shifted_edge = pair_separation(shifted) * pair_separation(shifted)
      < options.compact_pair_cutoff_distance_squared;
  records->push_back(record);
}

bool final_negative(const std::vector<double>& energy,
                    const std::vector<bool>& edge) {
  for (std::size_t i = energy.size() - 8; i < energy.size(); ++i)
    if (!(energy[i] < -1e-6 && edge[i])) return false;
  return true;
}

PairResult run_pair(const Condition& condition, bool conjugate,
                    ConnectedMooreBlockOptions options,
                    double interaction_scale,
                    std::vector<TickRecord>* records) {
  PairResult result;
  result.condition = condition.label;
  result.polarity = conjugate ? "minus_plus" : "plus_minus";
  options.solve_tolerance = condition.solve_tolerance;
  options.max_iterations = condition.max_iterations;
  const auto origin_initial = ftd::eft::redress_derived_compact_pair(
      make_geometry(conjugate, false), options, 1e-13, 4096);
  const auto shifted_initial = ftd::eft::redress_derived_compact_pair(
      make_geometry(conjugate, true), options, 1e-13, 4096);
  result.initialized = origin_initial.valid && shifted_initial.valid;
  if (!result.initialized) return result;

  ConnectedMooreBlockState origin = origin_initial.state;
  ConnectedMooreBlockState shifted = shifted_initial.state;
  std::vector<double> origin_energy{pair_internal_energy(origin, options)};
  std::vector<double> shifted_energy{pair_internal_energy(shifted, options)};
  std::vector<bool> origin_edge{pair_separation(origin) * pair_separation(origin)
      < options.compact_pair_cutoff_distance_squared};
  std::vector<bool> shifted_edge{pair_separation(shifted) * pair_separation(shifted)
      < options.compact_pair_cutoff_distance_squared};
  const std::size_t record_begin = records->size();
  append_record(condition, result.polarity, 0, origin, shifted, options,
                interaction_scale, 0.0, 0.0, records);
  ConnectedMooreBlockSolveCache origin_cache, shifted_cache;
  bool valid = true;
  bool gates = true;
  for (int tick = 1; tick <= kTicks; ++tick) {
    const auto a = ftd::eft::solve_connected_moore_block_forward(
        origin, options, &origin_cache);
    const auto b = ftd::eft::solve_connected_moore_block_forward(
        shifted, options, &shifted_cache);
    valid = valid && a.valid && b.valid;
    if (!valid) break;
    const double residual = std::max(maximum_step_residual(a),
                                     maximum_step_residual(b));
    const double recoil = std::max({a.matter_momentum_after.mag(),
        a.spline_defect_norm, b.matter_momentum_after.mag(),
        b.spline_defect_norm});
    result.maximum_common = std::max(result.maximum_common, residual);
    result.maximum_recoil = std::max(result.maximum_recoil, recoil);
    gates = gates && a.common_action_gates_pass && b.common_action_gates_pass
        && residual <= kGate && recoil <= 1e-9;
    result.origin_graph_transitions += a.relational_graph_changed ? 1 : 0;
    result.shifted_graph_transitions += b.relational_graph_changed ? 1 : 0;
    origin = a.later;
    shifted = b.later;
    origin_energy.push_back(pair_internal_energy(origin, options));
    shifted_energy.push_back(pair_internal_energy(shifted, options));
    origin_edge.push_back(pair_separation(origin) * pair_separation(origin)
        < options.compact_pair_cutoff_distance_squared);
    shifted_edge.push_back(pair_separation(shifted) * pair_separation(shifted)
        < options.compact_pair_cutoff_distance_squared);
    append_record(condition, result.polarity, tick, origin, shifted, options,
                  interaction_scale, residual, recoil, records);
  }
  result.executed = valid
      && origin_energy.size() == static_cast<std::size_t>(kTicks + 1);
  result.gates_pass = result.executed && gates;
  if (!result.executed) return result;
  result.origin_negative = final_negative(origin_energy, origin_edge);
  result.shifted_negative = final_negative(shifted_energy, shifted_edge);
  result.class_agreement = result.origin_graph_transitions
          == result.shifted_graph_transitions
      && result.origin_negative == result.shifted_negative;

  for (std::size_t i = record_begin; i < records->size(); ++i) {
    const auto& record = (*records)[i];
    if (record.scalar_difference > result.maximum_scalar) {
      result.maximum_scalar = record.scalar_difference;
      result.worst_scalar_tick = record.tick;
      result.worst_scalar_component = maximum_scalar_component(
          record.separation_difference, record.internal_difference,
          record.field_difference);
    }
    result.maximum_electric = std::max(
        result.maximum_electric, record.electric_difference);
    result.maximum_magnetic = std::max(
        result.maximum_magnetic, record.magnetic_difference);
    result.maximum_matter = std::max(
        result.maximum_matter, record.matter_difference);
    if (record.complete_difference > result.maximum_complete) {
      result.maximum_complete = record.complete_difference;
      result.worst_complete_tick = record.tick;
      TranslationDefects defects;
      defects.electric = record.electric_difference;
      defects.magnetic = record.magnetic_difference;
      defects.matter = record.matter_difference;
      result.worst_complete_component = maximum_complete_component(defects);
    }
  }
  return result;
}

ConditionSummary summarize(const std::string& label,
                           const std::vector<PairResult>& pairs) {
  ConditionSummary summary;
  summary.label = label;
  for (const auto& pair : pairs) {
    if (pair.condition != label) continue;
    ++summary.pairs;
    summary.histories += 2;
    summary.executed_histories += pair.executed ? 2 : 0;
    summary.gate_pass_histories += pair.gates_pass ? 2 : 0;
    summary.class_agreement_pairs += pair.class_agreement ? 1 : 0;
    summary.graph_transitions += pair.origin_graph_transitions
        + pair.shifted_graph_transitions;
    summary.final_negative_histories += (pair.origin_negative ? 1 : 0)
        + (pair.shifted_negative ? 1 : 0);
    if (pair.polarity == "plus_minus")
      summary.plus_minus_scalar = pair.maximum_scalar;
    if (pair.maximum_scalar > summary.maximum_scalar) {
      summary.maximum_scalar = pair.maximum_scalar;
      summary.worst_scalar_polarity = pair.polarity;
      summary.worst_scalar_tick = pair.worst_scalar_tick;
      summary.worst_scalar_component = pair.worst_scalar_component;
    }
    summary.maximum_electric = std::max(
        summary.maximum_electric, pair.maximum_electric);
    summary.maximum_magnetic = std::max(
        summary.maximum_magnetic, pair.maximum_magnetic);
    summary.maximum_matter = std::max(
        summary.maximum_matter, pair.maximum_matter);
    if (pair.maximum_complete > summary.maximum_complete) {
      summary.maximum_complete = pair.maximum_complete;
      summary.worst_complete_polarity = pair.polarity;
      summary.worst_complete_tick = pair.worst_complete_tick;
      summary.worst_complete_component = pair.worst_complete_component;
    }
    summary.maximum_common = std::max(
        summary.maximum_common, pair.maximum_common);
    summary.maximum_recoil = std::max(
        summary.maximum_recoil, pair.maximum_recoil);
  }
  return summary;
}

void write_records(const std::vector<TickRecord>& records,
                   const std::array<ConditionSummary, 3>& summaries,
                   const std::string& verdict) {
  const auto directory = std::filesystem::path(__FILE__).parent_path()
      .parent_path() / "results" / "ftd_0729";
  std::filesystem::create_directories(directory);
  std::ofstream csv(directory /
      "ftd_0729_late_reentry_covariance_convergence_v1.csv");
  csv << "condition,polarity,tick,separation_difference,internal_difference,"
         "field_difference,scalar_difference,electric_difference,"
         "magnetic_difference,matter_difference,complete_difference,"
         "common_residual,recoil_defect,origin_edge,shifted_edge,"
         "origin_internal,shifted_internal\n" << std::setprecision(17);
  for (const auto& r : records)
    csv << r.condition << ',' << r.polarity << ',' << r.tick << ','
        << r.separation_difference << ',' << r.internal_difference << ','
        << r.field_difference << ',' << r.scalar_difference << ','
        << r.electric_difference << ',' << r.magnetic_difference << ','
        << r.matter_difference << ',' << r.complete_difference << ','
        << r.common_residual << ',' << r.recoil_defect << ','
        << r.origin_edge << ',' << r.shifted_edge << ','
        << r.origin_internal << ',' << r.shifted_internal << '\n';

  const double scalar_ratio = summaries[2].maximum_scalar
      / summaries[1].maximum_scalar;
  const double complete_ratio = summaries[2].maximum_complete
      / summaries[1].maximum_complete;
  std::ofstream json(directory /
      "ftd_0729_late_reentry_covariance_convergence_v1.json");
  json << std::setprecision(17)
       << "{\n  \"identifier\": \"FTD-0729\",\n"
       << "  \"protocol_sha256\": \"" << kProtocolSha256 << "\",\n"
       << "  \"verdict\": \"" << verdict << "\",\n"
       << "  \"tick_record_count\": " << records.size() << ",\n"
       << "  \"ultra_to_tight_scalar_ratio\": " << scalar_ratio << ",\n"
       << "  \"ultra_to_tight_complete_ratio\": " << complete_ratio
       << ",\n  \"conditions\": [\n";
  for (std::size_t i = 0; i < summaries.size(); ++i) {
    const auto& s = summaries[i];
    json << "    {\"label\": \"" << s.label
         << "\", \"pairs\": " << s.pairs
         << ", \"histories\": " << s.histories
         << ", \"executed_histories\": " << s.executed_histories
         << ", \"gate_pass_histories\": " << s.gate_pass_histories
         << ", \"class_agreement_pairs\": " << s.class_agreement_pairs
         << ", \"graph_transitions\": " << s.graph_transitions
         << ", \"final_negative_histories\": "
         << s.final_negative_histories
         << ", \"plus_minus_scalar\": " << s.plus_minus_scalar
         << ", \"maximum_scalar\": " << s.maximum_scalar
         << ", \"maximum_electric\": " << s.maximum_electric
         << ", \"maximum_magnetic\": " << s.maximum_magnetic
         << ", \"maximum_matter\": " << s.maximum_matter
         << ", \"maximum_complete\": " << s.maximum_complete
         << ", \"maximum_common\": " << s.maximum_common
         << ", \"maximum_recoil\": " << s.maximum_recoil
         << ", \"worst_scalar_polarity\": \""
         << s.worst_scalar_polarity
         << "\", \"worst_scalar_tick\": " << s.worst_scalar_tick
         << ", \"worst_scalar_component\": \""
         << s.worst_scalar_component
         << "\", \"worst_complete_polarity\": \""
         << s.worst_complete_polarity
         << "\", \"worst_complete_tick\": " << s.worst_complete_tick
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
  const std::array<Condition, 3> conditions{{
      {"parent", 2e-12, 96},
      {"tight", 2e-13, 192},
      {"ultra", 2e-14, 384}}};
  std::vector<TickRecord> records;
  std::vector<PairResult> pairs;
  records.reserve(conditions.size() * 2 * (kTicks + 1));
  pairs.reserve(conditions.size() * 2);
  for (const auto& condition : conditions)
    for (bool conjugate : {false, true})
      pairs.push_back(run_pair(condition, conjugate, options,
                               interaction_scale, &records));
  const std::array<ConditionSummary, 3> summaries{{
      summarize("parent", pairs), summarize("tight", pairs),
      summarize("ultra", pairs)}};
  const bool matrix = normalization.valid && pairs.size() == 6
      && records.size() == 6 * (kTicks + 1)
      && std::all_of(summaries.begin(), summaries.end(),
          [](const ConditionSummary& s) {
            return s.pairs == 2 && s.histories == 4;
          });
  const bool algebra = matrix
      && std::all_of(summaries.begin(), summaries.end(),
          [](const ConditionSummary& s) {
            return s.executed_histories == 4
                && s.gate_pass_histories == 4
                && s.class_agreement_pairs == 2;
          });
  const bool reproduction = algebra && std::abs(
      summaries[1].plus_minus_scalar - kParentTightScalar) <= 1e-12;
  const bool classes = summaries[0].graph_transitions
          == summaries[1].graph_transitions
      && summaries[1].graph_transitions == summaries[2].graph_transitions
      && summaries[0].final_negative_histories
          == summaries[1].final_negative_histories
      && summaries[1].final_negative_histories
          == summaries[2].final_negative_histories;
  const double scalar_ratio = summaries[2].maximum_scalar
      / summaries[1].maximum_scalar;
  const double complete_ratio = summaries[2].maximum_complete
      / summaries[1].maximum_complete;
  std::string verdict;
  if (!reproduction)
    verdict = "LATE_REENTRY_CONVERGENCE_DIAGNOSTIC_UNRESOLVED";
  else if (!classes)
    verdict = "LATE_REENTRY_CLASS_SOLVER_SENSITIVE";
  else if (summaries[2].maximum_scalar > 1e-9
           || summaries[2].maximum_complete > 1e-9
           || summaries[2].maximum_scalar > summaries[1].maximum_scalar
           || summaries[2].maximum_complete > summaries[1].maximum_complete)
    verdict = "LATE_REENTRY_COVARIANCE_DEFECT_PERSISTS";
  else if (scalar_ratio <= 0.2 && complete_ratio <= 0.2)
    verdict = "LATE_REENTRY_ROOT_CONDITIONING_CONFIRMED";
  else
    verdict = "LATE_REENTRY_COVARIANCE_PLATEAU_BELOW_GATE";

  write_records(records, summaries, verdict);
  std::cout << "FTD-0729 " << verdict
            << " scalar=" << std::setprecision(17)
            << summaries[0].maximum_scalar << '/'
            << summaries[1].maximum_scalar << '/'
            << summaries[2].maximum_scalar
            << " complete=" << summaries[0].maximum_complete << '/'
            << summaries[1].maximum_complete << '/'
            << summaries[2].maximum_complete
            << " ratios=" << scalar_ratio << '/' << complete_ratio
            << " classes=" << summaries[0].graph_transitions << '/'
            << summaries[0].final_negative_histories << '\n';
  return matrix && algebra ? 0 : 1;
}

