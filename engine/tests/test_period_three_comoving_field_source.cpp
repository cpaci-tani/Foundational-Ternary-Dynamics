// FTD-0716 source side: deposit the locked FTD-0715 three-phase trajectory
// and write the exact affine RHS for the translated three-tick field equation.

#define FTD_0712_EMBEDDED
#include "test_resonant_internal_gait_cancellation.cpp"
#undef FTD_0712_EMBEDDED

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

constexpr char period3field_protocol_sha256[] =
    "5F74489C3BD5F7DCC28B99442DE13FBA36AC9110F9099065FF70C65F6041BE19";
constexpr char period3field_parent_protocol_sha256[] =
    "668C2D55EBB59572CE6C1E01928E4AE9A94E0913C964F5E45A69CCC8B5C2B4F9";
constexpr int period3field_ticks = 3;

struct Period3FieldPair {
  ftd::eft::MatchedFaceFlux electric;
  ftd::eft::MatchedEdgeField magnetic;
  explicit Period3FieldPair(int L = 0) : electric(L), magnetic(L) {}
};

struct Period3FieldSummary {
  bool parent = false;
  bool reconstruction = false;
  bool reference = false;
  bool currents = false;
  bool source = false;
  int segments = 0;
  int field_dof = 0;
  double continuity_residual = INFINITY;
  double causal_excess = INFINITY;
  double position_cycle_residual = INFINITY;
  double source_l2 = INFINITY;
  double source_maximum = INFINITY;
  std::vector<double> rhs;
  std::string verdict = "PERIOD_THREE_COMOVING_FIELD_SOURCE_EXECUTION_INVALID";
};

bool period3field_load(std::array<Vec3, count>& delta) {
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
  if (loaded != count) return false;
  Vec3 sum{};
  for (const auto& value : delta) sum += value;
  return sum.mag() <= 1e-14;
}

void period3field_add_segment(
    ftd::eft::MatchedFaceFlux& target,
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
void period3field_add_scaled(Field& target, const Field& source, double scale) {
  for (std::size_t i = 0; i < target.x.size(); ++i) {
    target.x[i] += scale * source.x[i];
    target.y[i] += scale * source.y[i];
    target.z[i] += scale * source.z[i];
  }
}

template <typename Field>
Field period3field_translate_field(const Field& source, int dx) {
  Field result(source.L);
  const int L = source.L;
  for (int x = 0; x < L; ++x) for (int y = 0; y < L; ++y)
    for (int z = 0; z < L; ++z) {
      const int from = source.index(x, y, z);
      const int to = result.index(x + dx, y, z);
      result.x[to] = source.x[from];
      result.y[to] = source.y[from];
      result.z[to] = source.z[from];
    }
  return result;
}

Period3FieldPair period3field_translate(const Period3FieldPair& source, int dx) {
  Period3FieldPair result(source.electric.L);
  result.electric = period3field_translate_field(source.electric, dx);
  result.magnetic = period3field_translate_field(source.magnetic, dx);
  return result;
}

Period3FieldPair period3field_advance(
    const Period3FieldPair& initial,
    const std::array<ftd::eft::MatchedFaceFlux, period3field_ticks>& current) {
  Period3FieldPair state = initial;
  for (int tick = 0; tick < period3field_ticks; ++tick) {
    const auto curl_t = ftd::eft::matched_curl_adjoint(state.electric);
    period3field_add_scaled(state.magnetic, curl_t, -ftd::C_SPEED);
    const auto curl = ftd::eft::matched_curl(state.magnetic);
    period3field_add_scaled(state.electric, curl, +ftd::C_SPEED);
    period3field_add_scaled(state.electric, current[tick], -1.0);
  }
  return state;
}

std::vector<double> period3field_pack(const Period3FieldPair& field) {
  const std::size_t volume = field.electric.x.size();
  std::vector<double> result(6 * volume);
  const std::array<const std::vector<double>*, 6> parts{{
      &field.electric.x, &field.electric.y, &field.electric.z,
      &field.magnetic.x, &field.magnetic.y, &field.magnetic.z}};
  for (std::size_t part = 0; part < parts.size(); ++part)
    std::copy(parts[part]->begin(), parts[part]->end(),
        result.begin() + static_cast<std::ptrdiff_t>(part * volume));
  return result;
}

void period3field_run(
    Period3FieldSummary& summary,
    const std::array<Vec3, count>& delta,
    const ftd::eft::ConnectedMooreBlockState& reference) {
  const int L = reference.electric.L;
  std::array<ftd::eft::MatchedFaceFlux, period3field_ticks> current{{
      ftd::eft::MatchedFaceFlux(L), ftd::eft::MatchedFaceFlux(L),
      ftd::eft::MatchedFaceFlux(L)}};
  summary.continuity_residual = 0.0;
  summary.causal_excess = 0.0;
  summary.position_cycle_residual = 0.0;
  bool valid = true;
  for (int particle = 0; particle < count; ++particle) {
    const Vec3 x0 = position(reference.constituents[particle]);
    const Vec3 x1 = x0 + Vec3{1.0 / 3.0, 0.0, 0.0} + delta[particle];
    const Vec3 x2 = x0 + Vec3{2.0 / 3.0, 0.0, 0.0} - delta[particle];
    const Vec3 x3 = x0 + Vec3{1.0, 0.0, 0.0};
    const std::array<Vec3, 4> positions{{x0, x1, x2, x3}};
    summary.position_cycle_residual = std::max(
        summary.position_cycle_residual,
        (positions[3] - positions[0] - Vec3{1.0, 0.0, 0.0}).mag());
    for (int tick = 0; tick < period3field_ticks; ++tick) {
      const auto segment = ftd::eft::make_quadratic_coat_face_current(
          L, positions[tick], positions[tick + 1],
          reference.charges[particle], false);
      ++summary.segments;
      valid = valid && segment.valid;
      summary.continuity_residual = std::max(
          summary.continuity_residual, segment.continuity_residual);
      summary.causal_excess = std::max(
          summary.causal_excess, segment.causal_excess);
      if (segment.valid) period3field_add_segment(current[tick], segment);
    }
  }
  summary.currents = valid && summary.segments == count * period3field_ticks
      && summary.continuity_residual <= 1e-12
      && summary.causal_excess <= 1e-12
      && summary.position_cycle_residual <= 1e-14;
  if (!summary.currents) return;

  Period3FieldPair zero(L);
  const auto affine = period3field_translate(
      period3field_advance(zero, current), -1);
  summary.rhs = period3field_pack(affine);
  long double squared = 0.0L;
  summary.source_maximum = 0.0;
  for (double& value : summary.rhs) {
    value = -value;
    squared += static_cast<long double>(value) * value;
    summary.source_maximum = std::max(summary.source_maximum, std::abs(value));
  }
  summary.source_l2 = std::sqrt(static_cast<double>(squared));
  summary.field_dof = static_cast<int>(summary.rhs.size());
  summary.source = summary.field_dof == 6 * L * L * L
      && std::isfinite(summary.source_l2) && summary.source_l2 > 0.0
      && std::isfinite(summary.source_maximum);
}

void period3field_write(const Period3FieldSummary& summary) {
  const auto directory = std::filesystem::path(__FILE__).parent_path()
      .parent_path() / "results/ftd_0716";
  std::filesystem::create_directories(directory);
  std::ofstream json(directory /
      "ftd_0716_period_three_comoving_field_source_v1.json");
  json << std::setprecision(17)
      << "{\n  \"ftd_id\": \"FTD-0716\",\n"
      << "  \"protocol_sha256\": \"" << period3field_protocol_sha256
      << "\",\n  \"parent_protocol_sha256\": \""
      << period3field_parent_protocol_sha256 << "\",\n"
      << "  \"verdict\": \"" << summary.verdict << "\",\n"
      << "  \"production_changed\": false,\n"
      << "  \"volume\": " << preflight_volume << ",\n"
      << "  \"parent_pass\": " << summary.parent << ",\n"
      << "  \"reconstruction_pass\": " << summary.reconstruction << ",\n"
      << "  \"reference_pass\": " << summary.reference << ",\n"
      << "  \"current_pass\": " << summary.currents << ",\n"
      << "  \"source_pass\": " << summary.source << ",\n"
      << "  \"segments\": " << summary.segments << ",\n"
      << "  \"field_dof\": " << summary.field_dof << ",\n"
      << "  \"continuity_residual\": " << summary.continuity_residual
      << ",\n  \"causal_excess\": " << summary.causal_excess << ",\n"
      << "  \"position_cycle_residual\": "
      << summary.position_cycle_residual << ",\n"
      << "  \"source_l2\": " << summary.source_l2 << ",\n"
      << "  \"source_maximum\": " << summary.source_maximum << "\n}\n";

  std::ofstream source(directory /
      "ftd_0716_period_three_comoving_field_source_v1.csv");
  source << "x,y,z,rhs_electric_x,rhs_electric_y,rhs_electric_z,"
            "rhs_magnetic_x,rhs_magnetic_y,rhs_magnetic_z\n";
  const std::size_t volume = static_cast<std::size_t>(preflight_volume)
      * preflight_volume * preflight_volume;
  if (summary.rhs.size() != 6 * volume) return;
  for (int x = 0; x < preflight_volume; ++x)
    for (int y = 0; y < preflight_volume; ++y)
      for (int z = 0; z < preflight_volume; ++z) {
        const std::size_t i = static_cast<std::size_t>(
            (x * preflight_volume + y) * preflight_volume + z);
        source << x << ',' << y << ',' << z;
        for (int component = 0; component < 6; ++component)
          source << ',' << std::setprecision(17)
                 << summary.rhs[static_cast<std::size_t>(component) * volume + i];
        source << '\n';
      }
}

}  // namespace

int main() {
  Period3FieldSummary summary;
  const auto results = std::filesystem::path(__FILE__).parent_path()
      .parent_path() / "results";
  summary.parent = gait_parent_fingerprint(
      results / "ftd_0715/ftd_0715_period_three_internal_momentum_lift_v1.json",
      period3field_parent_protocol_sha256,
      "PERIOD_THREE_MOMENTUM_LIFT_CONSTRUCTIVE");
  std::array<Vec3, count> delta{};
  summary.reconstruction = period3field_load(delta);
  const auto reference = gait_reference(summary.reference);
  if (summary.parent && summary.reconstruction && summary.reference)
    period3field_run(summary, delta, reference);
  if (summary.parent && summary.reconstruction && summary.reference
      && summary.currents && summary.source)
    summary.verdict = "PERIOD_THREE_COMOVING_FIELD_SOURCE_CONSTRUCTIVE";
  period3field_write(summary);
  std::cout << std::setprecision(17)
      << "protocol_sha256=" << period3field_protocol_sha256 << '\n'
      << "verdict=" << summary.verdict << '\n'
      << "segments=" << summary.segments
      << " continuity=" << summary.continuity_residual
      << " causal=" << summary.causal_excess << '\n'
      << "source_l2=" << summary.source_l2
      << " source_max=" << summary.source_maximum << '\n';
  return summary.verdict ==
      "PERIOD_THREE_COMOVING_FIELD_SOURCE_EXECUTION_INVALID" ? 1 : 0;
}

