// FTD-0717: replay the independently solved FTD-0715 matter momenta and
// FTD-0716 minimum-norm field, then measure common-action defects.

#define FTD_0712_EMBEDDED
#include "test_resonant_internal_gait_cancellation.cpp"
#undef FTD_0712_EMBEDDED

#include "ftd/eft/coupled_matched_face_transaction.h"
#include "ftd/eft/face_flux_normalization.h"
#include "ftd/eft/matched_face_momentum_transaction.h"
#include "ftd/eft/spline_poynting_momentum.h"

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

constexpr char common3_protocol_sha256[] =
    "BCAE18C3786A02266910F80875DD13FD0E3337A91635A01F83252170B5BD294B";
constexpr char common3_matter_protocol_sha256[] =
    "668C2D55EBB59572CE6C1E01928E4AE9A94E0913C964F5E45A69CCC8B5C2B4F9";
constexpr char common3_field_protocol_sha256[] =
    "5F74489C3BD5F7DCC28B99442DE13FBA36AC9110F9099065FF70C65F6041BE19";
constexpr int common3_ticks = 3;

struct Common3Field {
  ftd::eft::MatchedFaceFlux electric;
  ftd::eft::MatchedEdgeField magnetic;
  explicit Common3Field(int L = 0) : electric(L), magnetic(L) {}
};

struct Common3MatterTick {
  int count = 0;
  double energy_before = 0.0;
  double energy_after = 0.0;
  Vec3 impulse{};
};

struct Common3Tick {
  int tick = 0;
  double matter_energy_change = 0.0;
  double field_energy_change = 0.0;
  double total_energy_residual = INFINITY;
  Vec3 matter_impulse{};
  Vec3 local_field_change{};
  Vec3 spline_field_change{};
  Vec3 local_defect{};
  Vec3 spline_defect{};
  double gauss_before = INFINITY;
  double gauss_after = INFINITY;
};

struct Common3Summary {
  bool matter_parent = false;
  bool field_parent = false;
  bool reconstruction = false;
  bool reference = false;
  bool field_load = false;
  bool matter_load = false;
  bool currents = false;
  bool normalization = false;
  bool replay = false;
  bool translated_return = false;
  bool gauss = false;
  bool energy = false;
  bool local_momentum = false;
  bool spline_momentum = false;
  int segments = 0;
  double beta = INFINITY;
  double continuity_residual = INFINITY;
  double causal_excess = INFINITY;
  double maximum_gauss_residual = INFINITY;
  double maximum_total_energy_residual = INFINITY;
  double maximum_local_momentum_defect = INFINITY;
  double maximum_spline_momentum_defect = INFINITY;
  double electric_return_residual = INFINITY;
  double magnetic_return_residual = INFINITY;
  double complete_return_residual = INFINITY;
  std::array<Common3Tick, common3_ticks> ticks{};
  std::string verdict = "PERIOD_THREE_COMMON_ACTION_PREFLIGHT_EXECUTION_INVALID";
};

double common3_energy(const Vec3& momentum) {
  return std::sqrt(ftd::E_REST * ftd::E_REST
      + ftd::C_SPEED * ftd::C_SPEED * momentum.mag2());
}

bool common3_load_delta(std::array<Vec3, count>& delta) {
  const auto path = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results/ftd_0713/ftd_0713_causal_bound_internal_gait_state_v1.csv";
  std::ifstream input(path);
  std::string line;
  std::getline(input, line);
  int loaded = 0;
  while (std::getline(input, line)) {
    std::stringstream row(line);
    std::array<std::string, 4> fields;
    for (auto& field : fields) std::getline(row, field, ',');
    const int particle = std::stoi(fields[0]);
    if (particle < 0 || particle >= count) return false;
    delta[particle] = {std::stod(fields[1]), std::stod(fields[2]),
                       std::stod(fields[3])};
    ++loaded;
  }
  Vec3 sum{};
  for (const auto& value : delta) sum += value;
  return loaded == count && sum.mag() <= 1e-14;
}

bool common3_load_matter(std::array<Common3MatterTick, common3_ticks>& ticks) {
  const auto path = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results/ftd_0715/ftd_0715_period_three_internal_momentum_lift_segments_v1.csv";
  std::ifstream input(path);
  std::string line;
  std::getline(input, line);
  int loaded = 0;
  while (std::getline(input, line)) {
    std::stringstream row(line);
    std::array<std::string, 20> fields;
    for (auto& field : fields) std::getline(row, field, ',');
    const int tick = std::stoi(fields[1]);
    if (tick < 0 || tick >= common3_ticks) return false;
    const Vec3 before{std::stod(fields[8]), std::stod(fields[9]),
                      std::stod(fields[10])};
    const Vec3 after{std::stod(fields[11]), std::stod(fields[12]),
                     std::stod(fields[13])};
    ticks[tick].energy_before += common3_energy(before);
    ticks[tick].energy_after += common3_energy(after);
    ticks[tick].impulse += after - before;
    ++ticks[tick].count;
    ++loaded;
  }
  return loaded == count * common3_ticks
      && ticks[0].count == count && ticks[1].count == count
      && ticks[2].count == count;
}

bool common3_load_field(Common3Field& field) {
  const auto path = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results/ftd_0716/ftd_0716_period_three_comoving_field_correction_v1.csv";
  std::ifstream input(path);
  std::string line;
  std::getline(input, line);
  std::vector<bool> seen(field.electric.x.size(), false);
  int loaded = 0;
  while (std::getline(input, line)) {
    std::stringstream row(line);
    std::array<std::string, 9> values;
    for (auto& value : values) std::getline(row, value, ',');
    const int x = std::stoi(values[0]);
    const int y = std::stoi(values[1]);
    const int z = std::stoi(values[2]);
    const auto i = static_cast<std::size_t>(field.electric.index(x, y, z));
    if (i >= seen.size() || seen[i]) return false;
    seen[i] = true;
    field.electric.x[i] = std::stod(values[3]);
    field.electric.y[i] = std::stod(values[4]);
    field.electric.z[i] = std::stod(values[5]);
    field.magnetic.x[i] = std::stod(values[6]);
    field.magnetic.y[i] = std::stod(values[7]);
    field.magnetic.z[i] = std::stod(values[8]);
    ++loaded;
  }
  return loaded == static_cast<int>(seen.size())
      && std::all_of(seen.begin(), seen.end(), [](bool value) { return value; });
}

void common3_add_segment(ftd::eft::MatchedFaceFlux& target,
                         const ftd::eft::QuadraticCoatFaceCurrent& segment) {
  for (const auto& entry : segment.sparse_current) {
    const auto i = static_cast<std::size_t>(target.index(
        entry.face.x, entry.face.y, entry.face.z));
    auto& component = entry.axis == 0 ? target.x
        : (entry.axis == 1 ? target.y : target.z);
    component[i] += entry.value;
  }
}

template <typename Field>
void common3_add_scaled(Field& target, const Field& source, double scale) {
  for (std::size_t i = 0; i < target.x.size(); ++i) {
    target.x[i] += scale * source.x[i];
    target.y[i] += scale * source.y[i];
    target.z[i] += scale * source.z[i];
  }
}

template <typename Field>
Field common3_translate_field(const Field& source, int dx) {
  Field result(source.L);
  for (int x = 0; x < source.L; ++x) for (int y = 0; y < source.L; ++y)
    for (int z = 0; z < source.L; ++z) {
      const int from = source.index(x, y, z);
      const int to = result.index(x + dx, y, z);
      result.x[to] = source.x[from];
      result.y[to] = source.y[from];
      result.z[to] = source.z[from];
    }
  return result;
}

std::vector<double> common3_density(
    const ftd::eft::ConnectedMooreBlockState& reference,
    const std::array<Vec3, count>& delta, int phase) {
  const int L = reference.electric.L;
  std::vector<double> density(static_cast<std::size_t>(L) * L * L, 0.0);
  for (int particle = 0; particle < count; ++particle) {
    Vec3 position_at = position(reference.constituents[particle]);
    if (phase == 1) position_at += Vec3{1.0 / 3.0, 0.0, 0.0} + delta[particle];
    if (phase == 2) position_at += Vec3{2.0 / 3.0, 0.0, 0.0} - delta[particle];
    if (phase == 3) position_at += Vec3{1.0, 0.0, 0.0};
    const auto coat = ftd::eft::make_quadratic_polarity_coat(
        position_at, reference.charges[particle]);
    if (!coat.valid) return {};
    for (std::size_t item = 0; item < coat.weight_count; ++item) {
      const auto& entry = coat.weights[item];
      density[static_cast<std::size_t>(reference.electric.index(
          entry.site.x, entry.site.y, entry.site.z))] += entry.weight;
    }
  }
  return density;
}

bool common3_make_currents(
    const ftd::eft::ConnectedMooreBlockState& reference,
    const std::array<Vec3, count>& delta,
    std::array<ftd::eft::MatchedFaceFlux, common3_ticks>& current,
    Common3Summary& summary) {
  summary.continuity_residual = 0.0;
  summary.causal_excess = 0.0;
  bool valid = true;
  for (int particle = 0; particle < count; ++particle) {
    const Vec3 x0 = position(reference.constituents[particle]);
    const std::array<Vec3, 4> x{{
        x0,
        x0 + Vec3{1.0 / 3.0, 0.0, 0.0} + delta[particle],
        x0 + Vec3{2.0 / 3.0, 0.0, 0.0} - delta[particle],
        x0 + Vec3{1.0, 0.0, 0.0}}};
    for (int tick = 0; tick < common3_ticks; ++tick) {
      const auto segment = ftd::eft::make_quadratic_coat_face_current(
          reference.electric.L, x[tick], x[tick + 1],
          reference.charges[particle], false);
      ++summary.segments;
      valid = valid && segment.valid;
      summary.continuity_residual = std::max(
          summary.continuity_residual, segment.continuity_residual);
      summary.causal_excess = std::max(summary.causal_excess,
          segment.causal_excess);
      if (segment.valid) common3_add_segment(current[tick], segment);
    }
  }
  return valid && summary.segments == count * common3_ticks
      && summary.continuity_residual <= 1e-12
      && summary.causal_excess <= 1e-12;
}

void common3_run(
    Common3Summary& summary, Common3Field state,
    const std::array<Common3MatterTick, common3_ticks>& matter,
    const std::array<ftd::eft::MatchedFaceFlux, common3_ticks>& current,
    const std::array<std::vector<double>, common3_ticks + 1>& density) {
  const Common3Field initial = state;
  summary.maximum_gauss_residual = 0.0;
  summary.maximum_total_energy_residual = 0.0;
  summary.maximum_local_momentum_defect = 0.0;
  summary.maximum_spline_momentum_defect = 0.0;
  for (int tick = 0; tick < common3_ticks; ++tick) {
    auto& row = summary.ticks[tick];
    row.tick = tick;
    row.gauss_before = ftd::eft::max_fractional_gauss_residual(
        state.electric, density[tick]);
    const double field_energy_before = summary.beta
        * ftd::eft::matched_modified_energy(
            state.electric, state.magnetic, ftd::C_SPEED);
    const Vec3 local_before = ftd::eft::matched_local_translation_momentum(
        state.electric, state.magnetic) * summary.beta;
    const auto spline_before = ftd::eft::measure_spline_poynting_momentum(
        state.electric, state.magnetic, ftd::C_SPEED, 1.0, summary.beta);

    const auto curl_t = ftd::eft::matched_curl_adjoint(state.electric);
    common3_add_scaled(state.magnetic, curl_t, -ftd::C_SPEED);
    const auto curl = ftd::eft::matched_curl(state.magnetic);
    common3_add_scaled(state.electric, curl, +ftd::C_SPEED);
    common3_add_scaled(state.electric, current[tick], -1.0);

    row.gauss_after = ftd::eft::max_fractional_gauss_residual(
        state.electric, density[tick + 1]);
    const double field_energy_after = summary.beta
        * ftd::eft::matched_modified_energy(
            state.electric, state.magnetic, ftd::C_SPEED);
    const Vec3 local_after = ftd::eft::matched_local_translation_momentum(
        state.electric, state.magnetic) * summary.beta;
    const auto spline_after = ftd::eft::measure_spline_poynting_momentum(
        state.electric, state.magnetic, ftd::C_SPEED, 1.0, summary.beta);
    if (!spline_before.valid || !spline_after.valid) return;

    row.matter_energy_change = matter[tick].energy_after
        - matter[tick].energy_before;
    row.field_energy_change = field_energy_after - field_energy_before;
    row.total_energy_residual = std::abs(
        row.matter_energy_change + row.field_energy_change);
    row.matter_impulse = matter[tick].impulse;
    row.local_field_change = local_after - local_before;
    row.spline_field_change = spline_after.momentum - spline_before.momentum;
    row.local_defect = row.matter_impulse + row.local_field_change;
    row.spline_defect = row.matter_impulse + row.spline_field_change;
    summary.maximum_gauss_residual = std::max({
        summary.maximum_gauss_residual, row.gauss_before, row.gauss_after});
    summary.maximum_total_energy_residual = std::max(
        summary.maximum_total_energy_residual, row.total_energy_residual);
    summary.maximum_local_momentum_defect = std::max(
        summary.maximum_local_momentum_defect, row.local_defect.mag());
    summary.maximum_spline_momentum_defect = std::max(
        summary.maximum_spline_momentum_defect, row.spline_defect.mag());
  }
  const auto expected_electric = common3_translate_field(initial.electric, 1);
  const auto expected_magnetic = common3_translate_field(initial.magnetic, 1);
  summary.electric_return_residual = ftd::eft::matched_face_max_difference(
      state.electric, expected_electric);
  summary.magnetic_return_residual = ftd::eft::matched_edge_max_difference(
      state.magnetic, expected_magnetic);
  summary.complete_return_residual = std::max(
      summary.electric_return_residual, summary.magnetic_return_residual);
  summary.replay = std::isfinite(summary.maximum_gauss_residual)
      && std::isfinite(summary.maximum_total_energy_residual)
      && std::isfinite(summary.maximum_local_momentum_defect)
      && std::isfinite(summary.maximum_spline_momentum_defect)
      && std::isfinite(summary.complete_return_residual);
  summary.translated_return = summary.complete_return_residual <= 1e-10;
  summary.gauss = summary.maximum_gauss_residual <= 1e-10;
  summary.energy = summary.maximum_total_energy_residual <= 1e-10;
  summary.local_momentum = summary.maximum_local_momentum_defect <= 1e-10;
  summary.spline_momentum = summary.maximum_spline_momentum_defect <= 1e-10;
}

void common3_classify(Common3Summary& summary) {
  const bool execution = summary.matter_parent && summary.field_parent
      && summary.reconstruction && summary.reference && summary.field_load
      && summary.matter_load && summary.currents && summary.normalization
      && summary.replay && summary.translated_return;
  if (!execution) {
    summary.verdict = "PERIOD_THREE_COMMON_ACTION_PREFLIGHT_EXECUTION_INVALID";
  } else if (summary.gauss && summary.energy && summary.local_momentum
      && summary.spline_momentum) {
    summary.verdict =
        "PERIOD_THREE_MINIMUM_NORM_COMMON_ACTION_PREFLIGHT_CONSTRUCTIVE";
  } else {
    summary.verdict =
        "PERIOD_THREE_MINIMUM_NORM_FIELD_REQUIRES_COUPLED_SELECTION";
  }
}

void common3_write(const Common3Summary& summary) {
  const auto directory = std::filesystem::path(__FILE__).parent_path()
      .parent_path() / "results/ftd_0717";
  std::filesystem::create_directories(directory);
  std::ofstream json(directory / "ftd_0717_period_three_common_action_preflight_v1.json");
  json << std::setprecision(17)
      << "{\n  \"ftd_id\": \"FTD-0717\",\n"
      << "  \"protocol_sha256\": \"" << common3_protocol_sha256 << "\",\n"
      << "  \"matter_parent_protocol_sha256\": \""
      << common3_matter_protocol_sha256 << "\",\n"
      << "  \"field_parent_protocol_sha256\": \""
      << common3_field_protocol_sha256 << "\",\n"
      << "  \"verdict\": \"" << summary.verdict << "\",\n"
      << "  \"production_changed\": false,\n"
      << "  \"matter_parent_pass\": " << summary.matter_parent << ",\n"
      << "  \"field_parent_pass\": " << summary.field_parent << ",\n"
      << "  \"reconstruction_pass\": " << summary.reconstruction << ",\n"
      << "  \"reference_pass\": " << summary.reference << ",\n"
      << "  \"field_load_pass\": " << summary.field_load << ",\n"
      << "  \"matter_load_pass\": " << summary.matter_load << ",\n"
      << "  \"current_pass\": " << summary.currents << ",\n"
      << "  \"normalization_pass\": " << summary.normalization << ",\n"
      << "  \"replay_pass\": " << summary.replay << ",\n"
      << "  \"translated_return_pass\": " << summary.translated_return << ",\n"
      << "  \"gauss_pass\": " << summary.gauss << ",\n"
      << "  \"energy_pass\": " << summary.energy << ",\n"
      << "  \"local_momentum_pass\": " << summary.local_momentum << ",\n"
      << "  \"spline_momentum_pass\": " << summary.spline_momentum << ",\n"
      << "  \"segments\": " << summary.segments << ",\n"
      << "  \"beta\": " << summary.beta << ",\n"
      << "  \"continuity_residual\": " << summary.continuity_residual << ",\n"
      << "  \"causal_excess\": " << summary.causal_excess << ",\n"
      << "  \"maximum_gauss_residual\": " << summary.maximum_gauss_residual << ",\n"
      << "  \"maximum_total_energy_residual\": "
      << summary.maximum_total_energy_residual << ",\n"
      << "  \"maximum_local_momentum_defect\": "
      << summary.maximum_local_momentum_defect << ",\n"
      << "  \"maximum_spline_momentum_defect\": "
      << summary.maximum_spline_momentum_defect << ",\n"
      << "  \"electric_return_residual\": "
      << summary.electric_return_residual << ",\n"
      << "  \"magnetic_return_residual\": "
      << summary.magnetic_return_residual << ",\n"
      << "  \"complete_return_residual\": "
      << summary.complete_return_residual << "\n}\n";

  std::ofstream ticks(directory / "ftd_0717_period_three_common_action_ticks_v1.csv");
  ticks << "tick,matter_energy_change,field_energy_change,total_energy_residual,"
           "matter_impulse_x,matter_impulse_y,matter_impulse_z,"
           "local_field_change_x,local_field_change_y,local_field_change_z,"
           "local_defect_x,local_defect_y,local_defect_z,"
           "spline_field_change_x,spline_field_change_y,spline_field_change_z,"
           "spline_defect_x,spline_defect_y,spline_defect_z,gauss_before,gauss_after\n";
  for (const auto& row : summary.ticks)
    ticks << row.tick << ',' << std::setprecision(17)
          << row.matter_energy_change << ',' << row.field_energy_change << ','
          << row.total_energy_residual << ',' << row.matter_impulse.x << ','
          << row.matter_impulse.y << ',' << row.matter_impulse.z << ','
          << row.local_field_change.x << ',' << row.local_field_change.y << ','
          << row.local_field_change.z << ',' << row.local_defect.x << ','
          << row.local_defect.y << ',' << row.local_defect.z << ','
          << row.spline_field_change.x << ',' << row.spline_field_change.y << ','
          << row.spline_field_change.z << ',' << row.spline_defect.x << ','
          << row.spline_defect.y << ',' << row.spline_defect.z << ','
          << row.gauss_before << ',' << row.gauss_after << '\n';
}

}  // namespace

int main() {
  Common3Summary summary;
  const auto results = std::filesystem::path(__FILE__).parent_path()
      .parent_path() / "results";
  summary.matter_parent = gait_parent_fingerprint(
      results / "ftd_0715/ftd_0715_period_three_internal_momentum_lift_v1.json",
      common3_matter_protocol_sha256,
      "PERIOD_THREE_MOMENTUM_LIFT_CONSTRUCTIVE");
  summary.field_parent = gait_parent_fingerprint(
      results / "ftd_0716/ftd_0716_period_three_comoving_field_solvability_v1.json",
      common3_field_protocol_sha256,
      "PERIOD_THREE_COMOVING_FIELD_SOLUTION_REGULAR");
  std::array<Vec3, count> delta{};
  summary.reconstruction = common3_load_delta(delta);
  const auto reference = gait_reference(summary.reference);
  Common3Field field(preflight_volume);
  summary.field_load = common3_load_field(field);
  std::array<Common3MatterTick, common3_ticks> matter{};
  summary.matter_load = common3_load_matter(matter);
  std::array<ftd::eft::MatchedFaceFlux, common3_ticks> current{{
      ftd::eft::MatchedFaceFlux(preflight_volume),
      ftd::eft::MatchedFaceFlux(preflight_volume),
      ftd::eft::MatchedFaceFlux(preflight_volume)}};
  if (summary.reconstruction && summary.reference)
    summary.currents = common3_make_currents(reference, delta, current, summary);
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  summary.normalization = normalization.valid;
  summary.beta = normalization.mapped_field_work_coefficient;
  std::array<std::vector<double>, common3_ticks + 1> density{};
  if (summary.reference && summary.reconstruction)
    for (int phase = 0; phase <= common3_ticks; ++phase)
      density[phase] = common3_density(reference, delta, phase);
  if (summary.matter_parent && summary.field_parent && summary.reconstruction
      && summary.reference && summary.field_load && summary.matter_load
      && summary.currents && summary.normalization
      && std::all_of(density.begin(), density.end(),
          [](const auto& values) { return !values.empty(); }))
    common3_run(summary, field, matter, current, density);
  common3_classify(summary);
  common3_write(summary);
  std::cout << std::setprecision(17)
      << "protocol_sha256=" << common3_protocol_sha256 << '\n'
      << "verdict=" << summary.verdict << '\n'
      << "gauss=" << summary.maximum_gauss_residual
      << " energy=" << summary.maximum_total_energy_residual << '\n'
      << "local_momentum=" << summary.maximum_local_momentum_defect
      << " spline_momentum=" << summary.maximum_spline_momentum_defect << '\n'
      << "return=" << summary.complete_return_residual << '\n';
  return summary.verdict ==
      "PERIOD_THREE_COMMON_ACTION_PREFLIGHT_EXECUTION_INVALID" ? 1 : 0;
}

