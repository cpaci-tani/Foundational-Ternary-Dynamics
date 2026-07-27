/** FTD-0604: symmetric breathing matter-core discriminator. */

#include "ftd/eft/closed_neutral_trimer_pair.h"

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

constexpr int L = 17;
constexpr double gate = 1e-12;
constexpr double lambda_min = 0.8;
constexpr double lambda_max = 1.2;
constexpr double derivative_step = 1e-5;
constexpr double static_solver_tolerance = 1e-15;
constexpr const char* protocol_sha256 =
    "CD8DB5F38A6E9F01BB8EDFAF63664EF940BF0D1F87C1CE8BF5B17789616FDACE";

using ftd::Vec3;
using ftd::eft::ClosedNeutralPairOptions;
using ftd::eft::ClosedNeutralTrimerPairState;
using ftd::eft::ClosedNeutralTrimerPairStepResult;

int wrap(int value) {
  const int remainder = value % L;
  return remainder < 0 ? remainder + L : remainder;
}

int index(int x, int y, int z) {
  return (wrap(x) * L + wrap(y)) * L + wrap(z);
}

Vec3 effective_position(const ftd::eft::MatchedMatterPoint& point) {
  return {point.anchor.x + point.remainder.x,
          point.anchor.y + point.remainder.y,
          point.anchor.z + point.remainder.z};
}

ftd::eft::MatchedMatterPoint point_at(const Vec3& position) {
  ftd::eft::MatchedMatterPoint point;
  const long long ax = std::llround(position.x);
  const long long ay = std::llround(position.y);
  const long long az = std::llround(position.z);
  point.anchor = {wrap(static_cast<int>(ax)), wrap(static_cast<int>(ay)),
                  wrap(static_cast<int>(az))};
  point.remainder = {position.x - ax, position.y - ay, position.z - az};
  point.momentum = {};
  return point;
}

std::vector<double> density_of(const ClosedNeutralTrimerPairState& state) {
  std::vector<double> density(static_cast<std::size_t>(L) * L * L, 0.0);
  for (std::size_t a = 0; a < state.constituents.size(); ++a) {
    const auto coat = ftd::eft::make_quadratic_polarity_coat(
        effective_position(state.constituents[a]), state.charges[a]);
    if (!coat.valid) return {};
    for (std::size_t item = 0; item < coat.weight_count; ++item) {
      const auto& weight = coat.weights[item];
      density[static_cast<std::size_t>(index(
          weight.site.x, weight.site.y, weight.site.z))] += weight.weight;
    }
  }
  return density;
}

long double dot(const std::vector<double>& lhs,
                const std::vector<double>& rhs) {
  long double result = 0.0L;
  for (std::size_t i = 0; i < lhs.size(); ++i)
    result += static_cast<long double>(lhs[i]) * rhs[i];
  return result;
}

void apply_ddt(const std::vector<double>& scalar,
               std::vector<double>& result) {
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z) {
        const int i = index(x, y, z);
        result[static_cast<std::size_t>(i)] =
            6.0 * scalar[static_cast<std::size_t>(i)]
            - scalar[static_cast<std::size_t>(index(x + 1, y, z))]
            - scalar[static_cast<std::size_t>(index(x - 1, y, z))]
            - scalar[static_cast<std::size_t>(index(x, y + 1, z))]
            - scalar[static_cast<std::size_t>(index(x, y - 1, z))]
            - scalar[static_cast<std::size_t>(index(x, y, z + 1))]
            - scalar[static_cast<std::size_t>(index(x, y, z - 1))];
      }
}

struct MinimumField {
  bool valid = false;
  double solver_residual = INFINITY;
  double gauss_residual = INFINITY;
  double curl_residual = INFINITY;
  ftd::eft::MatchedFaceFlux electric{L};
};

MinimumField initialize_minimum_energy(const std::vector<double>& density) {
  MinimumField result;
  const std::size_t count = static_cast<std::size_t>(L) * L * L;
  if (density.size() != count) return result;
  long double total = 0.0L;
  for (double value : density) total += value;
  if (std::abs(static_cast<double>(total)) > gate) return result;
  std::vector<double> phi(count, 0.0);
  std::vector<double> residual = density;
  std::vector<double> direction = density;
  std::vector<double> image(count, 0.0);
  long double rr = dot(residual, residual);
  for (int iteration = 1; iteration <= 20 * L; ++iteration) {
    apply_ddt(direction, image);
    const long double p_ap = dot(direction, image);
    if (!(p_ap > 0.0L)) break;
    const long double alpha = rr / p_ap;
    for (std::size_t i = 0; i < count; ++i) {
      phi[i] += static_cast<double>(alpha * direction[i]);
      residual[i] -= static_cast<double>(alpha * image[i]);
    }
    result.solver_residual = 0.0;
    for (double value : residual)
      result.solver_residual = std::max(result.solver_residual,
                                        std::abs(value));
    if (result.solver_residual <= static_solver_tolerance) break;
    const long double next = dot(residual, residual);
    const long double beta = next / rr;
    for (std::size_t i = 0; i < count; ++i)
      direction[i] = residual[i] + static_cast<double>(beta * direction[i]);
    rr = next;
  }
  if (result.solver_residual > static_solver_tolerance) return result;
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z) {
        const int i = index(x, y, z);
        result.electric.x[static_cast<std::size_t>(i)] =
            phi[static_cast<std::size_t>(i)]
            - phi[static_cast<std::size_t>(index(x + 1, y, z))];
        result.electric.y[static_cast<std::size_t>(i)] =
            phi[static_cast<std::size_t>(i)]
            - phi[static_cast<std::size_t>(index(x, y + 1, z))];
        result.electric.z[static_cast<std::size_t>(i)] =
            phi[static_cast<std::size_t>(i)]
            - phi[static_cast<std::size_t>(index(x, y, z + 1))];
      }
  result.gauss_residual = ftd::eft::max_fractional_gauss_residual(
      result.electric, density);
  result.curl_residual = ftd::eft::max_curl_adjoint(result.electric);
  result.valid = result.gauss_residual <= gate && result.curl_residual <= gate;
  return result;
}

const std::array<Vec3, 3> reference_offsets{{
    {-2.0 / 3.0, -1.0 / 3.0, -1.0 / 3.0},
    { 1.0 / 3.0,  2.0 / 3.0, -1.0 / 3.0},
    { 1.0 / 3.0, -1.0 / 3.0,  2.0 / 3.0}}};
const Vec3 center_a{4.839666666666667, 7.114333333333334,
                    7.620333333333333};
const Vec3 center_b{11.196333333333333, 8.857666666666667,
                    8.433666666666667};

ClosedNeutralTrimerPairState make_state(double phase, double scale) {
  ClosedNeutralTrimerPairState state(L);
  const Vec3 shift{phase, 0.0, 0.0};
  for (std::size_t a = 0; a < 3; ++a) {
    state.constituents[a] = point_at(
        center_a + shift + reference_offsets[a] * scale);
    state.constituents[a + 3] = point_at(
        center_b + shift - reference_offsets[a] * scale);
  }
  return state;
}

struct StaticEvaluation {
  bool valid = false;
  double scale = NAN;
  double binding_energy = INFINITY;
  double field_energy = INFINITY;
  double total_energy = INFINITY;
  MinimumField field{};
  ClosedNeutralTrimerPairState state{L};
};

StaticEvaluation evaluate_static(double phase, double scale,
                                 const ClosedNeutralPairOptions& options,
                                 double beta) {
  StaticEvaluation result;
  result.scale = scale;
  result.state = make_state(phase, scale);
  result.field = initialize_minimum_energy(density_of(result.state));
  if (!result.field.valid) return result;
  result.state.electric = result.field.electric;
  result.binding_energy = ftd::eft::closed_neutral_pair_binding_energy(
      result.state, options);
  result.field_energy = beta * ftd::eft::quadratic_energy(result.state.electric);
  result.total_energy = result.binding_energy + result.field_energy;
  result.valid = std::isfinite(result.total_energy);
  return result;
}

struct RelaxationResult {
  bool valid = false;
  int iterations = 0;
  int evaluations = 0;
  StaticEvaluation minimum{};
};

RelaxationResult relax_breathing(double phase,
                                 const ClosedNeutralPairOptions& options,
                                 double beta) {
  RelaxationResult result;
  constexpr double ratio = 0.6180339887498948482;
  double left = lambda_min;
  double right = lambda_max;
  double x1 = right - ratio * (right - left);
  double x2 = left + ratio * (right - left);
  auto f1 = evaluate_static(phase, x1, options, beta);
  auto f2 = evaluate_static(phase, x2, options, beta);
  result.evaluations = 2;
  if (!f1.valid || !f2.valid) return result;
  for (int iteration = 0; iteration < 96 && right - left > 1e-10;
       ++iteration) {
    result.iterations = iteration + 1;
    if (f1.total_energy <= f2.total_energy) {
      right = x2;
      x2 = x1;
      f2 = f1;
      x1 = right - ratio * (right - left);
      f1 = evaluate_static(phase, x1, options, beta);
      ++result.evaluations;
      if (!f1.valid) return result;
    } else {
      left = x1;
      x1 = x2;
      f1 = f2;
      x2 = left + ratio * (right - left);
      f2 = evaluate_static(phase, x2, options, beta);
      ++result.evaluations;
      if (!f2.valid) return result;
    }
  }
  const auto midpoint = evaluate_static(
      phase, 0.5 * (left + right), options, beta);
  ++result.evaluations;
  result.minimum = midpoint.total_energy < std::min(
      f1.total_energy, f2.total_energy) ? midpoint
      : (f1.total_energy <= f2.total_energy ? f1 : f2);
  result.valid = result.minimum.valid && right - left <= 1e-10;
  return result;
}

double maximum_common_gate(const ClosedNeutralTrimerPairStepResult& result) {
  return std::max({result.root_residual, result.continuity_residual,
      result.gauss_before_residual, result.gauss_after_residual,
      result.force_residual, result.kinematic_residual,
      result.kinetic_discrete_gradient_residual,
      result.electric_adjoint_residual, result.magnetic_work_residual,
      result.binding_work_residual, result.binding_impulse_sum_residual,
      result.matter_work_residual, result.field_work_residual,
      result.total_energy_residual, result.causal_speed_excess});
}

ClosedNeutralTrimerPairState translate_x(
    const ClosedNeutralTrimerPairState& source, int amount) {
  ClosedNeutralTrimerPairState target(L);
  target.charges = source.charges;
  for (std::size_t a = 0; a < source.constituents.size(); ++a) {
    target.constituents[a] = source.constituents[a];
    target.constituents[a].anchor.x = wrap(
        target.constituents[a].anchor.x + amount);
  }
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z) {
        const int from = source.electric.index(x, y, z);
        const int to = target.electric.index(x + amount, y, z);
        target.electric.x[to] = source.electric.x[from];
        target.electric.y[to] = source.electric.y[from];
        target.electric.z[to] = source.electric.z[from];
        target.magnetic_half.x[to] = source.magnetic_half.x[from];
        target.magnetic_half.y[to] = source.magnetic_half.y[from];
        target.magnetic_half.z[to] = source.magnetic_half.z[from];
      }
  return target;
}

struct PhaseRecord {
  int phase_index = 0;
  double phase = 0.0;
  double rigid_energy = INFINITY;
  double relaxed_energy = INFINITY;
  double scale = NAN;
  double stationarity = INFINITY;
  double curvature = -INFINITY;
  double gauss_gate = INFINITY;
  double common_gate = INFINITY;
  double inverse = INFINITY;
  double inward_impulse = NAN;
  double separation_decrease = NAN;
  double pseudomomentum_defect = INFINITY;
  bool attractive = false;
};

struct Summary {
  int phase_arms = 0;
  int forward_arms = 0;
  int reverse_arms = 0;
  int attractive_phases = 0;
  bool optimizer_pass = true;
  bool interior_pass = true;
  bool stability_pass = true;
  bool energy_pass = true;
  bool common_pass = true;
  bool inverse_pass = true;
  bool periodicity_pass = false;
  bool attraction_robust = true;
  double minimum_scale = INFINITY;
  double maximum_scale = -INFINITY;
  double worst_stationarity = 0.0;
  double minimum_curvature = INFINITY;
  double worst_gauss_gate = 0.0;
  double worst_common_gate = 0.0;
  double worst_inverse = 0.0;
  double minimum_inward_impulse = INFINITY;
  double minimum_separation_decrease = INFINITY;
  double maximum_pseudomomentum_defect = 0.0;
  double rigid_barrier = NAN;
  double relaxed_barrier = NAN;
  double barrier_ratio = NAN;
  double periodicity_residual = INFINITY;
  double periodicity_scale_residual = INFINITY;
  std::vector<PhaseRecord> phases{};
  std::string verdict;
};

void write_record(const Summary& s) {
  const auto dir = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results" / "ftd_0604";
  std::filesystem::create_directories(dir);
  std::ofstream json(dir / "ftd_0604_symmetric_breathing_core_v1.json");
  json << std::setprecision(17) << "{\n"
       << "  \"ftd_id\": \"FTD-0604\",\n"
       << "  \"protocol_sha256\": \"" << protocol_sha256 << "\",\n"
       << "  \"verdict\": \"" << s.verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"phase_arms\": " << s.phase_arms << ",\n"
       << "  \"forward_arms\": " << s.forward_arms << ",\n"
       << "  \"reverse_arms\": " << s.reverse_arms << ",\n"
       << "  \"attractive_phases\": " << s.attractive_phases << ",\n"
       << "  \"optimizer_pass\": " << (s.optimizer_pass ? "true" : "false") << ",\n"
       << "  \"interior_pass\": " << (s.interior_pass ? "true" : "false") << ",\n"
       << "  \"stability_pass\": " << (s.stability_pass ? "true" : "false") << ",\n"
       << "  \"energy_pass\": " << (s.energy_pass ? "true" : "false") << ",\n"
       << "  \"common_pass\": " << (s.common_pass ? "true" : "false") << ",\n"
       << "  \"inverse_pass\": " << (s.inverse_pass ? "true" : "false") << ",\n"
       << "  \"periodicity_pass\": " << (s.periodicity_pass ? "true" : "false") << ",\n"
       << "  \"attraction_robust\": " << (s.attraction_robust ? "true" : "false") << ",\n"
       << "  \"minimum_scale\": " << s.minimum_scale << ",\n"
       << "  \"maximum_scale\": " << s.maximum_scale << ",\n"
       << "  \"worst_stationarity\": " << s.worst_stationarity << ",\n"
       << "  \"minimum_curvature\": " << s.minimum_curvature << ",\n"
       << "  \"worst_gauss_gate\": " << s.worst_gauss_gate << ",\n"
       << "  \"worst_common_gate\": " << s.worst_common_gate << ",\n"
       << "  \"worst_inverse\": " << s.worst_inverse << ",\n"
       << "  \"minimum_inward_impulse\": " << s.minimum_inward_impulse << ",\n"
       << "  \"minimum_separation_decrease\": " << s.minimum_separation_decrease << ",\n"
       << "  \"maximum_pseudomomentum_defect\": "
       << s.maximum_pseudomomentum_defect << ",\n"
       << "  \"rigid_barrier\": " << s.rigid_barrier << ",\n"
       << "  \"relaxed_barrier\": " << s.relaxed_barrier << ",\n"
       << "  \"barrier_ratio\": " << s.barrier_ratio << ",\n"
       << "  \"periodicity_residual\": " << s.periodicity_residual << ",\n"
       << "  \"periodicity_scale_residual\": "
       << s.periodicity_scale_residual << "\n}\n";
  std::ofstream csv(dir / "ftd_0604_symmetric_breathing_core_samples_v1.csv");
  csv << "ftd_id,phase_index,phase,rigid_energy,relaxed_energy,scale,"
         "stationarity,curvature,gauss_gate,common_gate,inverse,"
         "inward_impulse,separation_decrease,pseudomomentum_defect,attractive\n";
  for (const auto& p : s.phases)
    csv << std::setprecision(17) << "FTD-0604," << p.phase_index << ','
        << p.phase << ',' << p.rigid_energy << ',' << p.relaxed_energy << ','
        << p.scale << ',' << p.stationarity << ',' << p.curvature << ','
        << p.gauss_gate << ',' << p.common_gate << ',' << p.inverse << ','
        << p.inward_impulse << ',' << p.separation_decrease << ','
        << p.pseudomomentum_defect << ',' << p.attractive << '\n';
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  ClosedNeutralPairOptions options;
  options.gate_tolerance = gate;
  options.solve_tolerance = 2e-13;
  options.max_iterations = 64;
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  Summary summary;
  if (!normalization.valid) {
    summary.optimizer_pass = false;
    summary.verdict = "SYMMETRIC_BREATHING_CORE_STATIC_BRANCH_CLOSED_NEGATIVE";
    write_record(summary);
    return 1;
  }
  const double beta = normalization.mapped_field_work_coefficient;
  double rigid_minimum = INFINITY, rigid_maximum = -INFINITY;
  double relaxed_minimum = INFINITY, relaxed_maximum = -INFINITY;

  ClosedNeutralTrimerPairStepResult phase_zero_step(L);
  RelaxationResult phase_zero_relaxation;
  for (int j = 0; j < 32; ++j) {
    PhaseRecord record;
    record.phase_index = j;
    record.phase = static_cast<double>(j) / 32.0;
    const auto rigid = evaluate_static(record.phase, 1.0, options, beta);
    const auto relaxed = relax_breathing(record.phase, options, beta);
    ++summary.phase_arms;
    summary.optimizer_pass = summary.optimizer_pass
        && rigid.valid && relaxed.valid;
    if (!rigid.valid || !relaxed.valid) {
      summary.common_pass = false;
      summary.inverse_pass = false;
      summary.attraction_robust = false;
      summary.phases.push_back(record);
      continue;
    }
    record.rigid_energy = rigid.total_energy;
    record.relaxed_energy = relaxed.minimum.total_energy;
    record.scale = relaxed.minimum.scale;
    const auto lower = evaluate_static(record.phase,
        record.scale - derivative_step, options, beta);
    const auto upper = evaluate_static(record.phase,
        record.scale + derivative_step, options, beta);
    summary.optimizer_pass = summary.optimizer_pass
        && lower.valid && upper.valid;
    if (!lower.valid || !upper.valid) {
      summary.phases.push_back(record);
      continue;
    }
    record.stationarity = std::abs(upper.total_energy - lower.total_energy)
        / (2.0 * derivative_step);
    record.curvature = (upper.total_energy - 2.0 * record.relaxed_energy
        + lower.total_energy) / (derivative_step * derivative_step);
    record.gauss_gate = std::max({
        relaxed.minimum.field.solver_residual,
        relaxed.minimum.field.gauss_residual,
        relaxed.minimum.field.curl_residual});
    summary.minimum_scale = std::min(summary.minimum_scale, record.scale);
    summary.maximum_scale = std::max(summary.maximum_scale, record.scale);
    summary.worst_stationarity = std::max(
        summary.worst_stationarity, record.stationarity);
    summary.minimum_curvature = std::min(
        summary.minimum_curvature, record.curvature);
    summary.worst_gauss_gate = std::max(
        summary.worst_gauss_gate, record.gauss_gate);
    summary.interior_pass = summary.interior_pass
        && record.scale >= lambda_min + 1e-4
        && record.scale <= lambda_max - 1e-4;
    summary.stability_pass = summary.stability_pass
        && record.stationarity <= 1e-8 && record.curvature > 1e-6;
    summary.energy_pass = summary.energy_pass
        && record.relaxed_energy <= record.rigid_energy + 1e-12;
    rigid_minimum = std::min(rigid_minimum, record.rigid_energy);
    rigid_maximum = std::max(rigid_maximum, record.rigid_energy);
    relaxed_minimum = std::min(relaxed_minimum, record.relaxed_energy);
    relaxed_maximum = std::max(relaxed_maximum, record.relaxed_energy);

    const auto forward = ftd::eft::solve_closed_neutral_pair_forward(
        relaxed.minimum.state, options);
    ++summary.forward_arms;
    record.common_gate = maximum_common_gate(forward);
    summary.worst_common_gate = std::max(
        summary.worst_common_gate, record.common_gate);
    summary.common_pass = summary.common_pass
        && forward.common_action_gates_pass && record.common_gate <= gate;
    if (forward.valid) {
      const auto reverse = ftd::eft::solve_closed_neutral_pair_reverse(
          forward.later, options);
      ++summary.reverse_arms;
      record.inverse = reverse.valid
          ? ftd::eft::closed_neutral_pair_state_max_difference(
              relaxed.minimum.state, reverse.earlier)
          : INFINITY;
      record.inward_impulse = forward.inward_impulse;
      record.separation_decrease = forward.center_separation_before
          - forward.center_separation_after;
      record.pseudomomentum_defect = forward.pseudomomentum_defect_norm;
      record.attractive = record.inward_impulse > 1e-10
          && record.separation_decrease > 0.0;
      if (record.attractive) ++summary.attractive_phases;
      summary.worst_inverse = std::max(summary.worst_inverse, record.inverse);
      summary.inverse_pass = summary.inverse_pass
          && reverse.common_action_gates_pass && record.inverse <= 1e-10;
      summary.minimum_inward_impulse = std::min(
          summary.minimum_inward_impulse, record.inward_impulse);
      summary.minimum_separation_decrease = std::min(
          summary.minimum_separation_decrease, record.separation_decrease);
      summary.maximum_pseudomomentum_defect = std::max(
          summary.maximum_pseudomomentum_defect,
          record.pseudomomentum_defect);
      summary.attraction_robust = summary.attraction_robust
          && record.attractive;
    } else {
      summary.common_pass = false;
      summary.inverse_pass = false;
      summary.attraction_robust = false;
    }
    if (j == 0) {
      phase_zero_step = forward;
      phase_zero_relaxation = relaxed;
    }
    summary.phases.push_back(record);
  }
  summary.rigid_barrier = rigid_maximum - rigid_minimum;
  summary.relaxed_barrier = relaxed_maximum - relaxed_minimum;
  summary.barrier_ratio = summary.rigid_barrier > 0.0
      ? summary.relaxed_barrier / summary.rigid_barrier : NAN;

  if (phase_zero_relaxation.valid && phase_zero_step.valid) {
    const auto phase_one_state = translate_x(
        phase_zero_relaxation.minimum.state, 1);
    const auto phase_one_step = ftd::eft::solve_closed_neutral_pair_forward(
        phase_one_state, options);
    summary.periodicity_scale_residual = 0.0;
    summary.periodicity_residual = phase_one_step.valid
        ? ftd::eft::closed_neutral_pair_state_max_difference(
            translate_x(phase_zero_step.later, 1), phase_one_step.later)
        : INFINITY;
    summary.periodicity_pass = summary.periodicity_scale_residual <= gate
        && summary.periodicity_residual <= gate;
  }

  const bool static_gates = summary.optimizer_pass && summary.interior_pass
      && summary.stability_pass && summary.energy_pass
      && summary.worst_gauss_gate <= gate;
  if (!static_gates || !summary.common_pass || !summary.inverse_pass
      || !summary.periodicity_pass) {
    summary.verdict = "SYMMETRIC_BREATHING_CORE_STATIC_BRANCH_CLOSED_NEGATIVE";
  } else if (summary.attraction_robust) {
    summary.verdict = "SYMMETRIC_BREATHING_CORE_PHASE_ROBUST_CONSTRUCTIVE";
  } else if (!summary.attraction_robust) {
    summary.verdict = "SYMMETRIC_BREATHING_RELAXES_BUT_FORCE_SIGN_FAILS";
  } else {
    summary.verdict = "SYMMETRIC_BREATHING_CORE_UNRESOLVED";
  }

  int failures = 0;
  const auto check = [&](bool condition, const std::string& label) {
    std::cout << (condition ? "  PASS  " : "  FAIL  ") << label << '\n';
    if (!condition) ++failures;
  };
  check(summary.phase_arms == 32, "all 32 locked phase arms attempted");
  check(summary.phases.size() == 32, "all 32 locked phase arms recorded");
  check(!summary.verdict.empty(), "campaign produced a locked verdict");
  write_record(summary);
  std::cout << "protocol_sha256=" << protocol_sha256 << '\n'
            << "verdict=" << summary.verdict << '\n'
            << "attractive_phases=" << summary.attractive_phases << "/32\n"
            << "scale_range=" << summary.minimum_scale << ".."
            << summary.maximum_scale << '\n'
            << "worst_stationarity=" << summary.worst_stationarity << '\n'
            << "minimum_curvature=" << summary.minimum_curvature << '\n'
            << "rigid_barrier=" << summary.rigid_barrier << '\n'
            << "relaxed_barrier=" << summary.relaxed_barrier << '\n'
            << "barrier_ratio=" << summary.barrier_ratio << '\n'
            << "minimum_inward_impulse=" << summary.minimum_inward_impulse << '\n'
            << "minimum_separation_decrease="
            << summary.minimum_separation_decrease << '\n'
            << "periodicity_residual=" << summary.periodicity_residual << '\n'
            << "failures=" << failures << '\n';
  return failures == 0 ? 0 : 1;
}
