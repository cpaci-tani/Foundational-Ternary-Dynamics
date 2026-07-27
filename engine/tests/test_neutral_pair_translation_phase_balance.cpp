/** FTD-0603: neutral-pair translation-phase/Umklapp discriminator. */

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
constexpr const char* protocol_sha256 =
    "9C88B2B593C2E31EA08999010E71EF85204ECB3F8C63AA248B7A86A937E16595";

using ftd::Coord;
using ftd::Vec3;
using ftd::eft::ClosedNeutralTrimerPairState;
using ftd::eft::ClosedNeutralTrimerPairStepResult;

int wrap(int value) {
  const int remainder = value % L;
  return remainder < 0 ? remainder + L : remainder;
}

int index(int x, int y, int z) {
  return (wrap(x) * L + wrap(y)) * L + wrap(z);
}

double component(const Vec3& value, int axis) {
  return axis == 0 ? value.x : (axis == 1 ? value.y : value.z);
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
    if (result.solver_residual <= 1e-13) break;
    const long double next = dot(residual, residual);
    const long double beta = next / rr;
    for (std::size_t i = 0; i < count; ++i)
      direction[i] = residual[i] + static_cast<double>(beta * direction[i]);
    rr = next;
  }
  if (result.solver_residual > 1e-13) return result;
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

struct Fixture {
  ClosedNeutralTrimerPairState state{L};
  MinimumField initialization{};
  bool valid = false;
};

Fixture make_phase_fixture(int axis, double phase) {
  Fixture fixture;
  const std::array<Coord, 6> anchors{{
      {4, 7, 7}, {5, 8, 7}, {5, 7, 8},
      {12, 9, 9}, {11, 8, 9}, {11, 9, 8}}};
  const Vec3 remainder_a{0.173, -0.219, 0.287};
  const Vec3 remainder_b{-0.137, 0.191, -0.233};
  for (std::size_t a = 0; a < anchors.size(); ++a) {
    Vec3 position{anchors[a].x + (a < 3 ? remainder_a.x : remainder_b.x),
                  anchors[a].y + (a < 3 ? remainder_a.y : remainder_b.y),
                  anchors[a].z + (a < 3 ? remainder_a.z : remainder_b.z)};
    if (axis == 0) position.x += phase;
    if (axis == 1) position.y += phase;
    if (axis == 2) position.z += phase;
    fixture.state.constituents[a] = point_at(position);
  }
  fixture.initialization = initialize_minimum_energy(density_of(fixture.state));
  fixture.state.electric = fixture.initialization.electric;
  fixture.valid = fixture.initialization.valid;
  return fixture;
}

ClosedNeutralTrimerPairState translate_state(
    const ClosedNeutralTrimerPairState& source, int axis, int amount) {
  ClosedNeutralTrimerPairState target(L);
  target.charges = source.charges;
  for (std::size_t a = 0; a < source.constituents.size(); ++a) {
    target.constituents[a] = source.constituents[a];
    if (axis == 0) target.constituents[a].anchor.x = wrap(
        target.constituents[a].anchor.x + amount);
    if (axis == 1) target.constituents[a].anchor.y = wrap(
        target.constituents[a].anchor.y + amount);
    if (axis == 2) target.constituents[a].anchor.z = wrap(
        target.constituents[a].anchor.z + amount);
  }
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z) {
        const int from = source.electric.index(x, y, z);
        int tx = x, ty = y, tz = z;
        if (axis == 0) tx += amount;
        if (axis == 1) ty += amount;
        if (axis == 2) tz += amount;
        const int to = target.electric.index(tx, ty, tz);
        target.electric.x[to] = source.electric.x[from];
        target.electric.y[to] = source.electric.y[from];
        target.electric.z[to] = source.electric.z[from];
        target.magnetic_half.x[to] = source.magnetic_half.x[from];
        target.magnetic_half.y[to] = source.magnetic_half.y[from];
        target.magnetic_half.z[to] = source.magnetic_half.z[from];
      }
  return target;
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

struct ResolutionSummary {
  int N = 0;
  std::array<double, 3> mean_matter{};
  std::array<double, 3> mean_pseudomomentum{};
  double maximum_mean_matter = 0.0;
  double maximum_mean_pseudomomentum = 0.0;
};

struct PhaseArmRecord {
  int axis = 0;
  int resolution = 0;
  int phase_index = 0;
  double phase = 0.0;
  double initializer_gate = INFINITY;
  double common_gate = INFINITY;
  double inward_impulse = NAN;
  double separation_decrease = NAN;
  double matter_parallel = NAN;
  double pseudomomentum_parallel = NAN;
  double matter_norm = NAN;
  double pseudomomentum_norm = NAN;
};

struct Summary {
  int phase_arms = 0;
  int periodic_arms = 0;
  bool initializer_pass = true;
  bool common_pass = true;
  bool attraction_robust = true;
  bool integer_periodicity_pass = true;
  double worst_initializer_gate = 0.0;
  double worst_common_gate = 0.0;
  double worst_integer_periodicity = 0.0;
  double minimum_inward_impulse = INFINITY;
  double minimum_separation_decrease = INFINITY;
  double maximum_instantaneous_matter_impulse = 0.0;
  double maximum_instantaneous_pseudomomentum_defect = 0.0;
  std::array<ResolutionSummary, 3> resolutions{{
      {8}, {16}, {32}}};
  std::vector<PhaseArmRecord> arms{};
  std::string matter_classification;
  std::string pseudomomentum_classification;
  std::string verdict;
};

std::string classify(double m8, double m16, double m32) {
  if (m32 <= 1e-8 && (m32 <= 0.5 * m16 || m32 <= 1e-12))
    return "PHASE_BALANCED";
  if (m32 > 1e-8
      && std::abs(m32 - m16) <= 0.1 * m32
      && std::abs(m16 - m8) <= 0.2 * m16)
    return "SECULAR";
  return "UNRESOLVED";
}

void write_record(const Summary& s) {
  const auto dir = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results" / "ftd_0603";
  std::filesystem::create_directories(dir);
  std::ofstream json(dir / "ftd_0603_translation_phase_balance_v1.json");
  json << std::setprecision(17) << "{\n"
       << "  \"ftd_id\": \"FTD-0603\",\n"
       << "  \"protocol_sha256\": \"" << protocol_sha256 << "\",\n"
       << "  \"verdict\": \"" << s.verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"phase_arms\": " << s.phase_arms << ",\n"
       << "  \"periodic_arms\": " << s.periodic_arms << ",\n"
       << "  \"initializer_pass\": " << (s.initializer_pass ? "true" : "false") << ",\n"
       << "  \"common_pass\": " << (s.common_pass ? "true" : "false") << ",\n"
       << "  \"attraction_robust\": " << (s.attraction_robust ? "true" : "false") << ",\n"
       << "  \"integer_periodicity_pass\": "
       << (s.integer_periodicity_pass ? "true" : "false") << ",\n"
       << "  \"matter_classification\": \"" << s.matter_classification << "\",\n"
       << "  \"pseudomomentum_classification\": \""
       << s.pseudomomentum_classification << "\",\n"
       << "  \"worst_initializer_gate\": " << s.worst_initializer_gate << ",\n"
       << "  \"worst_common_gate\": " << s.worst_common_gate << ",\n"
       << "  \"worst_integer_periodicity\": " << s.worst_integer_periodicity << ",\n"
       << "  \"minimum_inward_impulse\": " << s.minimum_inward_impulse << ",\n"
       << "  \"minimum_separation_decrease\": " << s.minimum_separation_decrease << ",\n"
       << "  \"maximum_instantaneous_matter_impulse\": "
       << s.maximum_instantaneous_matter_impulse << ",\n"
       << "  \"maximum_instantaneous_pseudomomentum_defect\": "
       << s.maximum_instantaneous_pseudomomentum_defect << ",\n"
       << "  \"resolutions\": [\n";
  for (std::size_t i = 0; i < s.resolutions.size(); ++i) {
    const auto& r = s.resolutions[i];
    json << "    {\"N\": " << r.N
         << ", \"mean_matter\": [" << r.mean_matter[0] << ", "
         << r.mean_matter[1] << ", " << r.mean_matter[2] << "]"
         << ", \"mean_pseudomomentum\": [" << r.mean_pseudomomentum[0]
         << ", " << r.mean_pseudomomentum[1] << ", "
         << r.mean_pseudomomentum[2] << "]"
         << ", \"maximum_mean_matter\": " << r.maximum_mean_matter
         << ", \"maximum_mean_pseudomomentum\": "
         << r.maximum_mean_pseudomomentum << "}"
         << (i + 1 == s.resolutions.size() ? "\n" : ",\n");
  }
  json << "  ]\n}\n";
  std::ofstream csv(dir / "ftd_0603_translation_phase_balance_v1.csv");
  csv << "ftd_id,verdict,N,max_mean_matter,max_mean_pseudomomentum," 
         "minimum_inward_impulse,worst_common_gate\n";
  for (const auto& r : s.resolutions)
    csv << std::setprecision(17) << "FTD-0603," << s.verdict << ',' << r.N
        << ',' << r.maximum_mean_matter << ','
        << r.maximum_mean_pseudomomentum << ',' << s.minimum_inward_impulse
        << ',' << s.worst_common_gate << '\n';
  std::ofstream samples(
      dir / "ftd_0603_translation_phase_samples_v1.csv");
  samples << "ftd_id,axis,resolution,phase_index,phase,initializer_gate,"
             "common_gate,inward_impulse,separation_decrease,matter_parallel,"
             "pseudomomentum_parallel,matter_norm,pseudomomentum_norm\n";
  for (const auto& arm : s.arms)
    samples << std::setprecision(17) << "FTD-0603," << arm.axis << ','
            << arm.resolution << ',' << arm.phase_index << ',' << arm.phase
            << ',' << arm.initializer_gate << ',' << arm.common_gate << ','
            << arm.inward_impulse << ',' << arm.separation_decrease << ','
            << arm.matter_parallel << ',' << arm.pseudomomentum_parallel << ','
            << arm.matter_norm << ',' << arm.pseudomomentum_norm << '\n';
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  ftd::eft::ClosedNeutralPairOptions options;
  options.gate_tolerance = gate;
  options.solve_tolerance = 2e-13;
  options.max_iterations = 64;
  Summary summary;

  for (auto& resolution : summary.resolutions) {
    for (int axis = 0; axis < 3; ++axis) {
      long double matter_sum = 0.0L;
      long double pseudo_sum = 0.0L;
      for (int j = 0; j < resolution.N; ++j) {
        PhaseArmRecord arm;
        arm.axis = axis;
        arm.resolution = resolution.N;
        arm.phase_index = j;
        arm.phase = static_cast<double>(j) / resolution.N;
        const Fixture fixture = make_phase_fixture(
            axis, arm.phase);
        ++summary.phase_arms;
        const double initializer_gate = std::max({
            fixture.initialization.solver_residual,
            fixture.initialization.gauss_residual,
            fixture.initialization.curl_residual});
        arm.initializer_gate = initializer_gate;
        summary.worst_initializer_gate = std::max(
            summary.worst_initializer_gate, initializer_gate);
        summary.initializer_pass = summary.initializer_pass && fixture.valid
            && initializer_gate <= gate;
        if (!fixture.valid) {
          summary.common_pass = false;
          summary.attraction_robust = false;
          summary.arms.push_back(arm);
          continue;
        }
        const auto step = ftd::eft::solve_closed_neutral_pair_forward(
            fixture.state, options);
        const double common = maximum_common_gate(step);
        arm.common_gate = common;
        summary.worst_common_gate = std::max(summary.worst_common_gate, common);
        summary.common_pass = summary.common_pass
            && step.common_action_gates_pass && common <= gate;
        const Vec3 matter_delta = step.matter_momentum_after
            - step.matter_momentum_before;
        arm.inward_impulse = step.inward_impulse;
        arm.separation_decrease = step.center_separation_before
            - step.center_separation_after;
        arm.matter_parallel = component(matter_delta, axis);
        arm.pseudomomentum_parallel = component(
            step.pseudomomentum_defect, axis);
        arm.matter_norm = matter_delta.mag();
        arm.pseudomomentum_norm = step.pseudomomentum_defect_norm;
        matter_sum += component(matter_delta, axis);
        pseudo_sum += component(step.pseudomomentum_defect, axis);
        summary.maximum_instantaneous_matter_impulse = std::max(
            summary.maximum_instantaneous_matter_impulse, matter_delta.mag());
        summary.maximum_instantaneous_pseudomomentum_defect = std::max(
            summary.maximum_instantaneous_pseudomomentum_defect,
            step.pseudomomentum_defect_norm);
        const double separation_decrease = step.center_separation_before
            - step.center_separation_after;
        summary.minimum_inward_impulse = std::min(
            summary.minimum_inward_impulse, step.inward_impulse);
        summary.minimum_separation_decrease = std::min(
            summary.minimum_separation_decrease, separation_decrease);
        summary.attraction_robust = summary.attraction_robust
            && step.inward_impulse > 1e-10 && separation_decrease > 0.0;
        summary.arms.push_back(arm);
      }
      resolution.mean_matter[axis] = static_cast<double>(
          matter_sum / resolution.N);
      resolution.mean_pseudomomentum[axis] = static_cast<double>(
          pseudo_sum / resolution.N);
      resolution.maximum_mean_matter = std::max(
          resolution.maximum_mean_matter,
          std::abs(resolution.mean_matter[axis]));
      resolution.maximum_mean_pseudomomentum = std::max(
          resolution.maximum_mean_pseudomomentum,
          std::abs(resolution.mean_pseudomomentum[axis]));
    }
  }

  for (int axis = 0; axis < 3; ++axis) {
    const Fixture phase0 = make_phase_fixture(axis, 0.0);
    const Fixture phase1 = make_phase_fixture(axis, 1.0);
    const auto step0 = ftd::eft::solve_closed_neutral_pair_forward(
        phase0.state, options);
    const auto step1 = ftd::eft::solve_closed_neutral_pair_forward(
        phase1.state, options);
    ++summary.periodic_arms;
    const double residual = step0.valid && step1.valid
        ? ftd::eft::closed_neutral_pair_state_max_difference(
            translate_state(step0.later, axis, 1), step1.later)
        : INFINITY;
    summary.worst_integer_periodicity = std::max(
        summary.worst_integer_periodicity, residual);
    summary.integer_periodicity_pass = summary.integer_periodicity_pass
        && residual <= gate;
  }

  summary.matter_classification = classify(
      summary.resolutions[0].maximum_mean_matter,
      summary.resolutions[1].maximum_mean_matter,
      summary.resolutions[2].maximum_mean_matter);
  summary.pseudomomentum_classification = classify(
      summary.resolutions[0].maximum_mean_pseudomomentum,
      summary.resolutions[1].maximum_mean_pseudomomentum,
      summary.resolutions[2].maximum_mean_pseudomomentum);

  if (!summary.initializer_pass || !summary.common_pass
      || !summary.integer_periodicity_pass) {
    summary.verdict = "TRANSLATION_PHASE_COMMON_ACTION_CLOSED_NEGATIVE";
  } else if (!summary.attraction_robust) {
    summary.verdict = "TRANSLATION_PHASE_ATTRACTION_NOT_ROBUST";
  } else if (summary.matter_classification == "PHASE_BALANCED"
      && summary.pseudomomentum_classification == "PHASE_BALANCED") {
    summary.verdict =
        "RELATIVE_ATTRACTION_WITH_PHASE_BALANCED_LATTICE_EXCHANGE";
  } else if (summary.matter_classification == "SECULAR"
      || summary.pseudomomentum_classification == "SECULAR") {
    summary.verdict = "RELATIVE_ATTRACTION_WITH_SECULAR_MOMENTUM_DEFECT";
  } else {
    summary.verdict = "RELATIVE_ATTRACTION_PHASE_BALANCE_UNRESOLVED";
  }

  int failures = 0;
  const auto check = [&](bool condition, const std::string& label) {
    std::cout << (condition ? "  PASS  " : "  FAIL  ") << label << '\n';
    if (!condition) ++failures;
  };
  check(summary.phase_arms == 168, "all 168 locked phase arms attempted");
  check(summary.arms.size() == 168, "all 168 locked phase arms recorded");
  check(summary.periodic_arms == 3, "all three integer-period arms attempted");
  check(!summary.verdict.empty(), "campaign produced a locked verdict");
  write_record(summary);
  std::cout << "protocol_sha256=" << protocol_sha256 << '\n'
            << "verdict=" << summary.verdict << '\n'
            << "matter_classification=" << summary.matter_classification << '\n'
            << "pseudomomentum_classification="
            << summary.pseudomomentum_classification << '\n'
            << "M8_matter=" << summary.resolutions[0].maximum_mean_matter << '\n'
            << "M16_matter=" << summary.resolutions[1].maximum_mean_matter << '\n'
            << "M32_matter=" << summary.resolutions[2].maximum_mean_matter << '\n'
            << "M8_pseudomomentum="
            << summary.resolutions[0].maximum_mean_pseudomomentum << '\n'
            << "M16_pseudomomentum="
            << summary.resolutions[1].maximum_mean_pseudomomentum << '\n'
            << "M32_pseudomomentum="
            << summary.resolutions[2].maximum_mean_pseudomomentum << '\n'
            << "minimum_inward_impulse=" << summary.minimum_inward_impulse << '\n'
            << "minimum_separation_decrease="
            << summary.minimum_separation_decrease << '\n'
            << "worst_integer_periodicity="
            << summary.worst_integer_periodicity << '\n'
            << "failures=" << failures << '\n';
  return failures == 0 ? 0 : 1;
}
