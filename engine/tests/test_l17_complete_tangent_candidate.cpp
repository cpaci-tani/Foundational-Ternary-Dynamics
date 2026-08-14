// Shared implementation for the SHA-locked FTD-0774 candidate and the
// target-blind FTD-0829 certificate-repair successor.

#define FTD_0640_EMBEDDED
#include "test_connected_block_analytic_matter_modes.cpp"
#undef FTD_0640_EMBEDDED

#include "support/connected_moore_tangent_codec.h"

#include <cstdlib>
#include <atomic>
#include <complex>
#include <filesystem>
#include <future>
#include <limits>
#include <map>
#include <mutex>
#include <optional>
#include <set>
#include <tuple>

namespace {

using ftd0774::ChartVector;
using ftd0774::CodecResult;
using ftd0774::RetractionResult;
using ftd0774::SmallMatrix;
using ftd0774::TangentMetric;
using ftd::eft::ConnectedMooreBlockState;
using ftd::eft::MatchedEdgeField;
using ftd::eft::MatchedFaceFlux;

#ifdef FTD_0832_NONSINGULAR_PRODUCT_CHART
constexpr char kFtdId[] = "FTD-0832";
constexpr char kProtocolSha256[] =
    "2CE5516F7C0D4AF06649D54DD50C1E680C5BEE7CBEDFA10BDB09C2669FA81805";
#elif defined(FTD_0831_REPRESENTABILITY_FLOOR)
constexpr char kFtdId[] = "FTD-0831";
constexpr char kProtocolSha256[] =
    "A7BA4CEE3CC57AEC23CA9B9F60B0330C1E5B09EDBFC07FD5CC3E441AC736B3A1";
#elif defined(FTD_0830_HARMONIC_REINSERTION_REPAIR)
constexpr char kFtdId[] = "FTD-0830";
constexpr char kProtocolSha256[] =
    "A0D660F846D6C9AF43D94D475F1890E3D90D3967C6218B065ABDD9AA3BBFA5EC";
#elif defined(FTD_0829_CERTIFICATE_REPAIR)
constexpr char kFtdId[] = "FTD-0829";
constexpr char kProtocolSha256[] =
    "04C771A53E0A749492359255C613BD72A693A399920C0F3CA0FAE757931F361F";
#else
constexpr char kFtdId[] = "FTD-0774";
constexpr char kProtocolSha256[] =
    "0604AF560EA193BDE9E339ADB3FB28C0631B43D204186BEDA977EB700DD7F27E";
#endif
constexpr char kSourceCommit[] =
    "93748ac2021e4db5a9b8583cc28493332c716ac0";
constexpr double kH0 = 2e-6;
constexpr double kH1 = 1e-6;
constexpr double kHE = 2e-4;
constexpr double kInternalPhase = 1.0911648733663635;

struct ParentFile {
  std::string relative;
  std::string expected;
};

const std::array<ParentFile, 6> kParents{{
    {"engine/results/ftd_0638/ftd_0638_connected_block_analytic_static_refinement_v1.json",
     "435493EDC8E5DA5B34CF416EB6445C537A1F6ED9ABFCE02BB032DE2486C1B18C"},
    {"engine/results/ftd_0638/ftd_0638_connected_block_analytic_static_refinement_states_v1.csv",
     "8A717BC9DFE3A43FB21A6B46EF723BD2649D5F1F5BC2174BBA6027D25550214F"},
    {"engine/results/ftd_0639/ftd_0639_connected_block_analytic_dynamical_rest_v1.json",
     "DFA39E27F0317165D2A85E7778BBC7DA5691D1449DEEF20B4990C2AB9A1E7BD6"},
    {"engine/results/ftd_0640/ftd_0640_connected_block_analytic_matter_modes_v1.json",
     "AB43D342CFE48BEF452955E56B1EDC34F9EE51911F7D899932E7E542877E6B9A"},
    {"engine/results/ftd_0640/ftd_0640_connected_block_analytic_matter_modes_modes_v1.csv",
     "FE9F916443F8A8BF8F04B53067741919B203AF4C726D9DD67134B0BB43ECEFFD"},
    {"engine/results/ftd_0641/ftd_0641_connected_block_independent_field_modes_v1.json",
     "EA24EF12476533DB8395C0E64C1E381A6605662EAA9ED35C1E38D66D560189E6"},
}};

const std::array<ParentFile,4> kCompiledClosure{{
    {"engine/tests/test_connected_block_analytic_matter_modes.cpp",
     "EEBED821848AE462761385B9BE029C2CC0980289C22DF5357CFF305DEE27D8C4"},
    {"engine/tests/test_connected_block_analytic_dynamical_rest.cpp",
     "4FB13FEEC9320722B9CD67CD9443FC50F744FBA922369DD09865E67931CE3FF7"},
    {"engine/tests/test_connected_block_analytic_static_refinement.cpp",
     "4860362B11F4D1AE18B1DEB87CBB6988B0FF850C30CCE765FDC68D91CBD23B5D"},
    {"engine/tests/test_connected_block_analytic_envelope_hessian.cpp",
     "933AF8B366BF7350E208EB83188A90A4203A099CBC93E4F185685198B6928560"},
}};

std::filesystem::path engine_root() {
  return std::filesystem::path(__FILE__).parent_path().parent_path();
}

std::filesystem::path repo_root() { return engine_root().parent_path(); }

std::filesystem::path result_root() {
#ifdef FTD_0832_NONSINGULAR_PRODUCT_CHART
  return engine_root() / "results/ftd_0832";
#elif defined(FTD_0831_REPRESENTABILITY_FLOOR)
  return engine_root() / "results/ftd_0831";
#elif defined(FTD_0830_HARMONIC_REINSERTION_REPAIR)
  return engine_root() / "results/ftd_0830";
#elif defined(FTD_0829_CERTIFICATE_REPAIR)
  return engine_root() / "results/ftd_0829";
#else
  return engine_root() / "results/ftd_0774";
#endif
}

std::filesystem::path protocol_path() {
#ifdef FTD_0832_NONSINGULAR_PRODUCT_CHART
  return repo_root() /
      "docs/theory/10_eft_program/preregistrations/constituent_complete_matter/PREREG_L17_COMPLETE_TANGENT_NONSINGULAR_PRODUCT_CHART_v5.md";
#elif defined(FTD_0831_REPRESENTABILITY_FLOOR)
  return repo_root() /
      "docs/theory/10_eft_program/preregistrations/constituent_complete_matter/PREREG_L17_COMPLETE_TANGENT_REPRESENTABILITY_FLOOR_v4.md";
#elif defined(FTD_0830_HARMONIC_REINSERTION_REPAIR)
  return repo_root() /
      "docs/theory/10_eft_program/preregistrations/constituent_complete_matter/PREREG_L17_COMPLETE_TANGENT_HARMONIC_REINSERTION_REPAIR_v3.md";
#elif defined(FTD_0829_CERTIFICATE_REPAIR)
  return repo_root() /
      "docs/theory/10_eft_program/preregistrations/constituent_complete_matter/PREREG_L17_COMPLETE_TANGENT_CERTIFICATE_REPAIR_v2.md";
#else
  return repo_root() /
      "docs/theory/10_eft_program/preregistrations/constituent_complete_matter/PREREG_L17_COMPLETE_TANGENT_CANDIDATE_v1.md";
#endif
}

std::string quote(const std::filesystem::path& path) {
  return std::string("\"") + path.string() + "\"";
}

ftd::eft::ConnectedMooreBlockOptions locked_options() {
  ftd::eft::ConnectedMooreBlockOptions options;
  options.wave_speed = ftd::C_SPEED;
  options.dt = 1.0;
  options.binding_stiffness = 1.0;
  options.binding_law = ftd::eft::ConnectedBindingLaw::FixedEdgeQuartic;
  options.compact_pair_well_depth = 0.01;
  options.compact_pair_cutoff_distance_squared = 1.5;
  options.constituent_mass_scale = 1.0;
  options.polarity_scale = 1.0;
  options.field_energy_scale = 1.0;
  options.gate_tolerance = 1e-10;
  options.solve_tolerance = 2e-13;
  options.finite_difference_scale = 2e-7;
  options.max_iterations = 64;
  options.allow_shared_anchor_chart = true;
  options.use_sparse_local_current = true;
  options.use_local_residual_evaluation = true;
  options.use_low_rank_identity_broyden = false;
  options.use_matrix_free_newton_krylov = false;
  options.defer_volume_diagnostics = false;
  options.measure_final_root_regularity = false;
  options.root_momentum_seed.clear();
  return options;
}

bool options_are_locked(const ftd::eft::ConnectedMooreBlockOptions& o) {
  return o.wave_speed == ftd::C_SPEED && o.dt == 1.0
      && o.binding_stiffness == 1.0
      && o.binding_law == ftd::eft::ConnectedBindingLaw::FixedEdgeQuartic
      && o.compact_pair_well_depth == 0.01
      && o.compact_pair_cutoff_distance_squared == 1.5
      && o.constituent_mass_scale == 1.0 && o.polarity_scale == 1.0
      && o.field_energy_scale == 1.0 && o.gate_tolerance == 1e-10
      && o.solve_tolerance == 2e-13 && o.finite_difference_scale == 2e-7
      && o.max_iterations == 64 && o.allow_shared_anchor_chart
      && o.use_sparse_local_current && o.use_local_residual_evaluation
      && !o.use_low_rank_identity_broyden
      && !o.use_matrix_free_newton_krylov
      && !o.defer_volume_diagnostics && !o.measure_final_root_regularity
      && o.root_momentum_seed.empty();
}

struct StoredMode {
  bool valid = false;
  int mode = -1;
  int group = -1;
  double hessian_eigen = NAN;
  double lambda = NAN;
  double omega = NAN;
  double phase = NAN;
  std::vector<double> vector;
};

std::vector<std::string> split_csv(const std::string& line) {
  std::vector<std::string> fields;
  std::stringstream row(line);
  std::string field;
  while (std::getline(row, field, ',')) fields.push_back(field);
  if(!line.empty()&&line.back()==',')fields.emplace_back();
  return fields;
}

std::array<StoredMode, 2> load_locked_modes() {
  std::array<StoredMode, 2> result;
  const auto path = engine_root()
      / "results/ftd_0640/ftd_0640_connected_block_analytic_matter_modes_modes_v1.csv";
  std::ifstream input(path);
  std::string line;
  std::getline(input, line);
  while (std::getline(input, line)) {
    const auto fields = split_csv(line);
    if (fields.size() != 10 + ftd0774::kMatterDimension) continue;
    if (std::stoi(fields[1]) != 0) continue;
    const int mode = std::stoi(fields[2]);
    if (mode != 6 && mode != 7) continue;
    auto& out = result[mode - 6];
    out.mode = mode;
    out.group = std::stoi(fields[3]);
    out.hessian_eigen = std::stod(fields[4]);
    out.lambda = std::stod(fields[5]);
    out.omega = std::stod(fields[6]);
    out.phase = std::stod(fields[7]);
    out.vector.resize(ftd0774::kMatterDimension);
    for (int i = 0; i < ftd0774::kMatterDimension; ++i)
      out.vector[i] = std::stod(fields[10 + i]);
    const bool finite = std::all_of(out.vector.begin(), out.vector.end(),
        [](double value) { return std::isfinite(value); });
    out.valid = finite && out.group == 4
        && std::abs(out.phase - kInternalPhase) <= 1e-12;
  }
  long double gram00 = 0.0L, gram01 = 0.0L, gram11 = 0.0L;
  if (result[0].valid && result[1].valid) {
    for (int i = 0; i < ftd0774::kMatterDimension; ++i) {
      gram00 += result[0].vector[i] * ftd::M_INERTIAL * result[0].vector[i];
      gram01 += result[0].vector[i] * ftd::M_INERTIAL * result[1].vector[i];
      gram11 += result[1].vector[i] * ftd::M_INERTIAL * result[1].vector[i];
    }
    const bool mass_gram = std::abs(static_cast<double>(gram00) - 1.0) <= 1e-10
        && std::abs(static_cast<double>(gram11) - 1.0) <= 1e-10
        && std::abs(static_cast<double>(gram01)) <= 1e-10;
    result[0].valid = result[0].valid && mass_gram;
    result[1].valid = result[1].valid && mass_gram;
  }
  return result;
}

MatchedFaceFlux locked_field_shape() {
  MatchedEdgeField potential(ftd0774::kL);
  int accepted = -1;
  for (int axis = 0, transverse = 0; axis < 3; ++axis) {
    MatchedEdgeField candidate(ftd0774::kL);
    for (int x = 0; x < ftd0774::kL; ++x)
      for (int y = 0; y < ftd0774::kL; ++y)
        for (int z = 0; z < ftd0774::kL; ++z) {
          const auto i = static_cast<std::size_t>(candidate.index(x, y, z));
          const double value = std::cos(2.0 * ftd::PI * x / ftd0774::kL);
          if (axis == 0) candidate.x[i] = value;
          else if (axis == 1) candidate.y[i] = value;
          else candidate.z[i] = value;
        }
    const auto curl = ftd::eft::matched_curl(candidate);
    double maximum = 0.0;
    for (std::size_t i = 0; i < ftd0774::kVolume; ++i)
      maximum = std::max({maximum, std::abs(curl.x[i]),
          std::abs(curl.y[i]), std::abs(curl.z[i])});
    if (maximum > 1e-12) {
      if (transverse == 0) { potential = std::move(candidate); accepted = axis; break; }
      ++transverse;
    }
  }
  auto result = ftd::eft::matched_curl(potential);
  double maximum = 0.0;
  for (std::size_t i = 0; i < ftd0774::kVolume; ++i)
    maximum = std::max({maximum, std::abs(result.x[i]),
        std::abs(result.y[i]), std::abs(result.z[i])});
  if (accepted < 0 || !(maximum > 0.0)) return MatchedFaceFlux{};
  for (std::size_t i = 0; i < ftd0774::kVolume; ++i) {
    result.x[i] /= maximum;
    result.y[i] /= maximum;
    result.z[i] /= maximum;
  }
  return result;
}

struct FieldControl {
  bool pass = false;
  double phase = INFINITY;
  double phase_relative = INFINITY;
  double recurrence = INFINITY;
  double recovery_relative = INFINITY;
  double recovery_face_absolute = INFINITY;
  double recovery_edge_absolute = INFINITY;
  double divergence = INFINITY;
  double energy_drift = INFINITY;
  double target_amplitude = 1e-7;
  double initial_signal_energy = NAN;
  double initial_background_energy = NAN;
  std::vector<double> q;
  std::vector<double> divergence_series;
  std::vector<double> signal_energy_series;
  std::vector<double> background_energy_series;
};

void advance_source_free(MatchedFaceFlux& electric, MatchedEdgeField& magnetic) {
  const auto curl_e = ftd::eft::matched_curl_adjoint(electric);
  ftd0774::add_scaled(magnetic, curl_e, -ftd::C_SPEED);
  const auto curl_b = ftd::eft::matched_curl(magnetic);
  ftd0774::add_scaled(electric, curl_b, ftd::C_SPEED);
}

void reverse_source_free(MatchedFaceFlux& electric, MatchedEdgeField& magnetic) {
  const auto curl_b = ftd::eft::matched_curl(magnetic);
  ftd0774::add_scaled(electric, curl_b, -ftd::C_SPEED);
  const auto curl_e = ftd::eft::matched_curl_adjoint(electric);
  ftd0774::add_scaled(magnetic, curl_e, ftd::C_SPEED);
}

FieldControl run_field_control(const ConnectedMooreBlockState& reference) {
  FieldControl result;
  constexpr double target = 1e-7;
  result.target_amplitude=target;
  const auto unit = locked_field_shape();
  if (unit.L != ftd0774::kL) return result;
  auto background_e = reference.electric;
  auto background_b = reference.magnetic_half;
  auto electric = background_e;
  auto magnetic = background_b;
  ftd0774::add_scaled(electric, unit, target);
  const auto initial_e = electric;
  const auto initial_b = magnetic;
  const double energy0 = ftd::eft::matched_modified_energy(
      electric, magnetic, ftd::C_SPEED);
  const double background0 = ftd::eft::matched_modified_energy(
      background_e, background_b, ftd::C_SPEED);
  result.initial_signal_energy=energy0;
  result.initial_background_energy=background0;
  const double denominator = ftd0774::face_dot(unit, unit);
  auto& q=result.q;
  q.reserve(256);
  result.divergence_series.reserve(256);
  result.signal_energy_series.reserve(256);
  result.background_energy_series.reserve(256);
  result.recurrence = 0.0;
  result.divergence = 0.0;
  result.energy_drift = 0.0;
  for (int tick = 0; tick < 256; ++tick) {
    advance_source_free(electric, magnetic);
    advance_source_free(background_e, background_b);
    const auto delta = ftd0774::face_difference(electric, background_e);
    q.push_back(ftd0774::face_dot(delta, unit) / denominator);
    const double divergence=ftd::eft::max_divergence(delta);
    const double signal_energy=ftd::eft::matched_modified_energy(
        electric,magnetic,ftd::C_SPEED);
    const double background_energy=ftd::eft::matched_modified_energy(
        background_e,background_b,ftd::C_SPEED);
    result.divergence_series.push_back(divergence);
    result.signal_energy_series.push_back(signal_energy);
    result.background_energy_series.push_back(background_energy);
    result.divergence = std::max(result.divergence,divergence);
    result.energy_drift = std::max({result.energy_drift,
        std::abs(signal_energy-energy0),
        std::abs(background_energy-background0)});
  }
  long double numerator = 0.0L, phase_denominator = 0.0L;
  for (int i = 1; i < 255; ++i) {
    numerator += q[i] * (q[i + 1] + q[i - 1]);
    phase_denominator += 2.0L * q[i] * q[i];
  }
  const double predicted = 2.0 * std::asin(
      ftd::C_SPEED * std::sin(ftd::PI / ftd0774::kL));
  if (phase_denominator > 0.0L) {
    const double cosine_ratio=static_cast<double>(numerator/phase_denominator);
    if(std::isfinite(cosine_ratio)&&cosine_ratio>=-1.0&&cosine_ratio<=1.0)
      result.phase=std::acos(cosine_ratio);
  }
  result.phase_relative = std::abs(result.phase - predicted) / predicted;
  const double cosine = std::cos(predicted);
  for (int i = 1; i < 255; ++i)
    result.recurrence = std::max(result.recurrence,
        std::abs(q[i + 1] + q[i - 1] - 2.0 * cosine * q[i]) / target);
  for (int tick = 0; tick < 256; ++tick)
    reverse_source_free(electric, magnetic);
  result.recovery_face_absolute=
      ftd::eft::matched_face_max_difference(electric,initial_e);
  result.recovery_edge_absolute=
      ftd::eft::matched_edge_max_difference(magnetic,initial_b);
  result.recovery_relative=std::max(result.recovery_face_absolute,
      result.recovery_edge_absolute)/target;
  result.pass = result.phase_relative <= 1e-8 && result.recurrence <= 1e-8
      && result.recovery_relative <= 1e-8;
  return result;
}

struct PreflightRow {
  std::string record_kind;
  std::string probe;
  double h = NAN;
  std::string direction;
  int sign = 0;
  bool valid = false;
  double common_residual = NAN;
  double energy_drift = NAN;
  double recovery = NAN;
  double gauss_pre = NAN;
  double gauss_clean = NAN;
  double hodge_correction = NAN;
  double reconstruction = NAN;
  double harmonic_face = NAN;
  double harmonic_edge = NAN;
  double sigma_min = NAN;
  double condition = NAN;
  double scale_difference = NAN;
  double observer_regression = NAN;
  int jacobian_refreshes = 0;
  int jacobian_reuses = 0;
  int cache_fallbacks = 0;
  double k_norm = NAN;
  double energy_slope = NAN;
  double energy_second = NAN;
  double energy_relative = NAN;
  double derivative_scale_relative = NAN;
  double composition_residual = NAN;
  std::string detail;
};

void csv_number(std::ostream& output, double value) {
  if (std::isfinite(value)) output << std::setprecision(17) << value;
}

void write_preflight_row(std::ostream& output, const PreflightRow& row) {
  output << row.record_kind << ',' << row.probe << ',';
  csv_number(output, row.h);
  output << ',' << row.direction << ',' << row.sign << ',' << row.valid << ',';
  csv_number(output,row.common_residual);output<<',';csv_number(output,row.energy_drift);output<<',';
  csv_number(output,row.recovery);output<<',';csv_number(output,row.gauss_pre);output<<',';
  csv_number(output,row.gauss_clean);output<<',';csv_number(output,row.hodge_correction);output<<',';
  csv_number(output,row.reconstruction);output<<',';csv_number(output,row.harmonic_face);output<<',';
  csv_number(output,row.harmonic_edge);output<<',';csv_number(output,row.sigma_min);output<<',';
  csv_number(output,row.condition);output<<',';csv_number(output,row.scale_difference);output<<',';
  csv_number(output,row.observer_regression);output<<','<<row.jacobian_refreshes<<','
      <<row.jacobian_reuses<<','<<row.cache_fallbacks<<',';
  csv_number(output,row.k_norm);output<<',';csv_number(output,row.energy_slope);output<<',';
  csv_number(output,row.energy_second);output<<',';csv_number(output,row.energy_relative);output<<',';
  csv_number(output,row.derivative_scale_relative);output<<',';
  csv_number(output,row.composition_residual);output<<','<<row.detail<<'\n';
}

using TangentStepResult = ftd::eft::ConnectedMooreBlockStepResult;
using TangentSolveCache = ftd::eft::ConnectedMooreBlockSolveCache;

TangentStepResult solve_tangent_endpoint(
    const ConnectedMooreBlockState& input,
    const ftd::eft::ConnectedMooreBlockOptions& options, bool forward,
    TangentSolveCache* cache = nullptr) {
  return forward
      ? ftd::eft::solve_connected_moore_block_forward(input, options, cache)
      : ftd::eft::solve_connected_moore_block_reverse(input, options, cache);
}

const ConnectedMooreBlockState& tangent_endpoint_state(
    const TangentStepResult& step, bool forward) {
  return forward ? step.later : step.earlier;
}

bool tangent_step_accepted(const TangentStepResult& step) {
  return step.valid && step.common_action_gates_pass
      && common_residual(step) <= 1e-10
      && std::abs(total_after(step) - total_before(step)) <= 1e-12
      && step.site_hops == 0;
}

struct SignedEndpointEvaluation {
  bool valid = false;
  bool endpoint_pass = false;
  bool regularity_pass = true;
  bool cache_pass = true;
  bool chart_pass = false;
  bool direct_step_pass = false;
  bool inverse_step_pass = false;
  bool observer_step_pass = false;
  bool population_step_pass = false;
  bool reuse_step_pass = false;
  bool cache_valid_after_population = false;
  bool cache_semantics = false;
  double common = INFINITY;
  double energy_drift = INFINITY;
  double recovery = INFINITY;
  double sigma_min = NAN;
  double condition = NAN;
  double regularity_scale = NAN;
  double observer_regression = NAN;
  int jacobian_refreshes = 0;
  int jacobian_reuses = 0;
  int cache_fallbacks = 0;
  int population_iterations = 0;
  int population_refreshes = 0;
  int population_reuses = 0;
  int population_fallbacks = 0;
  int reuse_refreshes = 0;
  int reuse_reuses = 0;
  int reuse_fallbacks = 0;
  double cache_agreement = INFINITY;
  double direct_population_agreement = INFINITY;
  double direct_reuse_agreement = INFINITY;
  ConnectedMooreBlockState output{ftd0774::kL};
};

SignedEndpointEvaluation evaluate_signed_endpoint(
    const ConnectedMooreBlockState& input,
    const ftd::eft::ConnectedMooreBlockOptions& options, bool forward,
    bool controls) {
  SignedEndpointEvaluation result;
  const auto direct = solve_tangent_endpoint(input, options, forward, nullptr);
  result.direct_step_pass = tangent_step_accepted(direct);
  result.common = common_residual(direct);
  result.energy_drift = std::abs(total_after(direct) - total_before(direct));
  if (!result.direct_step_pass) return result;
  result.output = tangent_endpoint_state(direct, forward);
  const bool direct_chart = ftd0774::same_metadata(input, result.output)
      && sector_signature(input) == sector_signature(result.output);
  result.chart_pass = direct_chart;

  const auto inverse = solve_tangent_endpoint(
      result.output, options, !forward, nullptr);
  result.inverse_step_pass = tangent_step_accepted(inverse);
  result.common = std::max(result.common, common_residual(inverse));
  result.energy_drift = std::max(result.energy_drift,
      std::abs(total_after(inverse) - total_before(inverse)));
  if (result.inverse_step_pass)
    result.recovery = ftd::eft::connected_moore_block_state_max_difference(
        input, tangent_endpoint_state(inverse, !forward));
  result.endpoint_pass = direct_chart && result.inverse_step_pass
      && result.common <= 1e-10 && result.energy_drift <= 1e-12
      && result.recovery <= 1e-10;
  if (!controls) {
    result.valid = result.endpoint_pass;
    return result;
  }

  auto observed_options = options;
  observed_options.measure_final_root_regularity = true;
  const auto observed = solve_tangent_endpoint(
      input, observed_options, forward, nullptr);
  result.observer_step_pass=tangent_step_accepted(observed);
  if (result.observer_step_pass) {
    result.observer_regression =
        ftd::eft::connected_moore_block_state_max_difference(
            result.output, tangent_endpoint_state(observed, forward));
    result.sigma_min = observed.solve.final_minimum_singular_value;
    result.condition = observed.solve.final_condition_number;
    result.regularity_scale =
        observed.solve.regularity_scale_relative_difference;
  }
  result.regularity_pass = result.observer_step_pass
      && observed.solve.final_root_regularity_measured
      && std::isfinite(result.sigma_min) && result.sigma_min >= 1e-3
      && std::isfinite(result.condition) && result.condition <= 1e4
      && std::isfinite(result.regularity_scale)
      && result.regularity_scale <= 1e-5
      && result.observer_regression <= 1e-12;

  TangentSolveCache cache;
  const auto population = solve_tangent_endpoint(
      input, options, forward, &cache);
  const bool cache_valid_after_population = cache.valid;
  const auto reuse = solve_tangent_endpoint(input, options, forward, &cache);
  result.population_step_pass=tangent_step_accepted(population);
  result.reuse_step_pass=tangent_step_accepted(reuse);
  result.cache_valid_after_population=cache_valid_after_population;
  result.population_iterations=population.solve.iterations;
  result.population_refreshes=population.solve.jacobian_refreshes;
  result.population_reuses=population.solve.jacobian_reuses;
  result.population_fallbacks=population.solve.cache_fallbacks;
  result.reuse_refreshes=reuse.solve.jacobian_refreshes;
  result.reuse_reuses=reuse.solve.jacobian_reuses;
  result.reuse_fallbacks=reuse.solve.cache_fallbacks;
  result.jacobian_refreshes = population.solve.jacobian_refreshes
      + reuse.solve.jacobian_refreshes;
  result.jacobian_reuses = population.solve.jacobian_reuses
      + reuse.solve.jacobian_reuses;
  result.cache_fallbacks = population.solve.cache_fallbacks
      + reuse.solve.cache_fallbacks;
  if (result.population_step_pass && result.reuse_step_pass) {
    result.direct_population_agreement=
        ftd::eft::connected_moore_block_state_max_difference(
            result.output,tangent_endpoint_state(population,forward));
    result.direct_reuse_agreement=
        ftd::eft::connected_moore_block_state_max_difference(
            result.output,tangent_endpoint_state(reuse,forward));
    result.cache_agreement=std::max(result.direct_population_agreement,
        result.direct_reuse_agreement);
  }
  bool cache_semantics = true;
  if (population.solve.jacobian_refreshes > 0
      && cache_valid_after_population)
    cache_semantics = reuse.solve.jacobian_reuses > 0;
  if (population.solve.iterations == 0 && !cache_valid_after_population)
    cache_semantics = population.solve.jacobian_refreshes == 0
        && population.solve.jacobian_reuses == 0
        && reuse.solve.jacobian_refreshes == 0
        && reuse.solve.jacobian_reuses == 0;
  result.cache_semantics=cache_semantics;
  result.cache_pass = result.population_step_pass
      && result.reuse_step_pass && result.cache_agreement <= 1e-10
      && result.cache_fallbacks == 0 && cache_semantics;
  result.valid = result.endpoint_pass && result.regularity_pass
      && result.cache_pass;
  return result;
}

struct DerivativeEvaluation {
  bool valid = false;
  bool forward = true;
  double h = NAN;
  RetractionResult plus;
  RetractionResult minus;
  SignedEndpointEvaluation plus_endpoint;
  SignedEndpointEvaluation minus_endpoint;
  CodecResult codec;
  ChartVector value;
};

template <class Chart>
DerivativeEvaluation evaluate_derivative(
    const Chart& chart, const ChartVector& tangent, double h, bool forward,
    const ftd::eft::ConnectedMooreBlockOptions& options, bool controls) {
  DerivativeEvaluation result;
  result.forward = forward;
  result.h = h;
  result.plus = ftd0774::retract(chart, tangent, h);
  result.minus = ftd0774::retract(chart, tangent, -h);
  if (!result.plus.valid || !result.minus.valid) return result;
  result.plus_endpoint = evaluate_signed_endpoint(
      result.plus.state, options, forward, controls);
  result.minus_endpoint = evaluate_signed_endpoint(
      result.minus.state, options, forward, controls);
  if (!result.plus_endpoint.valid || !result.minus_endpoint.valid)
    return result;
  result.codec = ftd0774::encode_centered(chart,
      result.plus_endpoint.output, result.minus_endpoint.output, h);
  result.value = result.codec.value;
  result.valid = result.codec.valid && ftd0774::finite_chart(result.value);
  return result;
}

double chart_max_abs(const ChartVector& value) {
  double result = 0.0;
  for (double entry : ftd0774::flatten(value))
    result = std::max(result, std::abs(entry));
  return result;
}

std::string codec_detail(const CodecResult& codec, bool pass) {
  std::ostringstream output;
  output << (pass ? "pass" : "fail") << std::hexfloat;
  const auto write_triplet = [&](const std::array<double,3>& values) {
    output << values[0] << '/' << values[1] << '/' << values[2];
  };
  output << ";face_raw="; write_triplet(codec.face_raw);
  output << ";face_rebuilt="; write_triplet(codec.face_rebuilt);
  output << ";edge_raw="; write_triplet(codec.edge_raw);
  output << ";edge_rebuilt="; write_triplet(codec.edge_rebuilt);
  output << ";tangent_source_mean_abs=" << codec.tangent_source_mean_abs
      << ";tangent_source_mean_rel=" << codec.tangent_source_mean_rel
      << ";hodge_source_mean_abs=" << codec.hodge_source_mean_abs
      << ";hodge_source_mean_rel=" << codec.hodge_source_mean_rel;
#ifdef FTD_0829_CERTIFICATE_REPAIR
  output << ";hodge_compatibility_reference="
      << codec.hodge_compatibility_reference;
#endif
#ifdef FTD_0831_REPRESENTABILITY_FLOOR
  output << ";face_completed_max=" << codec.face_completed_max;
#endif
#ifdef FTD_0832_NONSINGULAR_PRODUCT_CHART
  output << ";complete_chart_norm=" << codec.complete_chart_norm
      << ";chart_dx_square=" << codec.chart_dx_square
      << ";chart_dp_square=" << codec.chart_dp_square
      << ";chart_electric_square=" << codec.chart_electric_square
      << ";chart_magnetic_square=" << codec.chart_magnetic_square
      << ";hodge_correction_absolute="
      << codec.hodge_correction_absolute
      << ";reconstruction_absolute="
      << codec.reconstruction_absolute;
#endif
  return output.str();
}

struct FullGradientAudit {
  bool valid = false;
  double dx = INFINITY;
  double dp = INFINITY;
  double eT = INFINITY;
  double b = INFINITY;
  double global = INFINITY;
};

struct ExecutionStatusRow {
  std::uint64_t evaluation_id = 0;
  std::string record_kind;
  std::string construction;
  std::string stage;
  std::string operation;
  std::optional<int> power;
  std::optional<int> column;
  double h = NAN;
  std::string direction;
  int sign = 0;
  bool valid = false;
  std::optional<bool> metadata;
  std::optional<bool> sector;
  std::optional<bool> finite;
  double gauss = NAN;
  double poisson_absolute = NAN;
  std::optional<bool> endpoint_chart;
  double common_residual = NAN;
  double energy_drift = NAN;
  double recovery = NAN;
  double codec_gauss_pre = NAN;
  double codec_gauss_clean = NAN;
  double hodge_correction = NAN;
  double reconstruction = NAN;
  double harmonic_face = NAN;
  double harmonic_edge = NAN;
  double tangent_source_mean_rel = NAN;
  double hodge_source_mean_rel = NAN;
  double tangent_poisson_relative = NAN;
  double hodge_poisson_relative = NAN;
  std::string detail;
};

struct OperatorAuditKey {
  std::string construction;
  std::string stage;
  std::string operation;
  std::optional<int> power;
  std::optional<int> column;
  double h = NAN;
  bool forward = true;
};

class OperatorAuditLedger {
 public:
  std::uint64_t reserve() { return next_.fetch_add(1); }

  void record(std::uint64_t id, const OperatorAuditKey& key,
              const DerivativeEvaluation& value) {
    std::vector<ExecutionStatusRow> group;
    group.reserve(5);
    const auto base = [&](const char* kind, int sign) {
      ExecutionStatusRow row;
      row.evaluation_id = id;
      row.record_kind = kind;
      row.construction = key.construction;
      row.stage = key.stage;
      row.operation = key.operation;
      row.power = key.power;
      row.column = key.column;
      row.h = key.h;
      row.direction = key.forward ? "forward" : "reverse";
      row.sign = sign;
      return row;
    };
    const auto retraction_row = [&](const RetractionResult& retract_value,
                                    int sign) {
      auto row = base("retraction", sign);
      row.valid = retract_value.valid;
      row.metadata = retract_value.metadata;
      row.sector = retract_value.sector;
      row.finite = retract_value.finite;
      row.gauss = retract_value.gauss_residual;
      row.poisson_absolute = retract_value.poisson_absolute_residual;
      return row;
    };
    const auto endpoint_row = [&](const SignedEndpointEvaluation& endpoint,
                                  int sign) {
      auto row = base("endpoint", sign);
      row.valid = endpoint.endpoint_pass;
      row.metadata = endpoint.direct_step_pass;
      row.sector = endpoint.inverse_step_pass;
      row.endpoint_chart = endpoint.chart_pass;
      row.common_residual = endpoint.common;
      row.energy_drift = endpoint.energy_drift;
      row.recovery = endpoint.recovery;
      return row;
    };
    group.push_back(retraction_row(value.plus, +1));
    group.push_back(endpoint_row(value.plus_endpoint, +1));
    group.push_back(retraction_row(value.minus, -1));
    group.push_back(endpoint_row(value.minus_endpoint, -1));
    auto codec = base("codec", 0);
    codec.valid = value.codec.valid && ftd0774::finite_chart(value.value);
    codec.finite = ftd0774::finite_chart(value.value);
    codec.codec_gauss_pre = value.codec.preclean_divergence;
    codec.codec_gauss_clean = value.codec.cleaned_divergence;
    codec.hodge_correction = value.codec.hodge_correction;
    codec.reconstruction = value.codec.reconstruction;
    codec.harmonic_face = value.codec.face_harmonic;
    codec.harmonic_edge = value.codec.edge_harmonic;
    codec.tangent_source_mean_rel = value.codec.tangent_source_mean_rel;
    codec.hodge_source_mean_rel = value.codec.hodge_source_mean_rel;
    codec.tangent_poisson_relative =
        value.codec.tangent_poisson_relative_residual;
    codec.hodge_poisson_relative =
        value.codec.hodge_poisson_relative_residual;
    codec.detail = codec_detail(value.codec, codec.valid);
    group.push_back(std::move(codec));
    {
      std::lock_guard<std::mutex> lock(mutex_);
      groups_.push_back({id, std::move(group)});
    }
    if (!value.valid) failed_.store(true);
  }

  void record_numeric_failure(const OperatorAuditKey& key,
                              double diagnostic,
                              const std::string& detail,
                              double secondary = NAN,
                              std::optional<bool> finite_override = std::nullopt) {
    ExecutionStatusRow row;
    row.evaluation_id=reserve();
    row.record_kind="numeric";
    row.construction=key.construction;
    row.stage=key.stage;
    row.operation=key.operation;
    row.power=key.power;
    row.column=key.column;
    row.h=key.h;
    row.direction=key.forward?"forward":"reverse";
    row.valid=false;
    row.finite=finite_override.has_value()
        ?*finite_override:std::isfinite(diagnostic);
    row.gauss=diagnostic;
    row.poisson_absolute=secondary;
    row.detail=detail;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      groups_.push_back({row.evaluation_id,{std::move(row)}});
    }
    failed_.store(true);
  }

  void record_bookkeeping_failure(
      const OperatorAuditKey& key,int generated,int accepted,int prior,
      int last_power,int last_start,int last_end,int deflations,
      bool happy,bool exhausted) {
    ExecutionStatusRow row;
    row.evaluation_id=reserve();
    row.record_kind="numeric";
    row.construction=key.construction;
    row.stage=key.stage;
    row.operation=key.operation;
    row.h=key.h;
    row.valid=false;
    row.finite=true;
    row.power=generated;
    row.column=accepted;
    row.gauss=prior;
    row.poisson_absolute=last_power;
    row.common_residual=last_start;
    row.energy_drift=last_end;
    row.recovery=deflations;
    row.codec_gauss_pre=happy?1.0:0.0;
    row.codec_gauss_clean=exhausted?1.0:0.0;
    row.detail="inconsistent_krylov_bookkeeping";
    {
      std::lock_guard<std::mutex> lock(mutex_);
      groups_.push_back({row.evaluation_id,{std::move(row)}});
    }
    failed_.store(true);
  }

  bool failed() const { return failed_.load(); }
  std::uint64_t size() const { return next_.load(); }

  std::vector<ExecutionStatusRow> ordered_rows(
      bool complete,const std::string& terminal_construction="all",
      const std::string& complete_operation="run_complete",
      const std::string& abort_operation="run_abort",
      bool preserve_reservation_order=false) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto groups = groups_;
    if (preserve_reservation_order) {
      std::sort(groups.begin(), groups.end(), [](const auto& left,
                                                 const auto& right) {
        return left.first < right.first;
      });
    } else {
      std::sort(groups.begin(), groups.end(), [](const auto& left,
                                                 const auto& right) {
        const auto& a=left.second.front();
        const auto& b=right.second.front();
        return std::tie(a.construction,a.stage,a.operation,a.power,a.column,
                        a.direction)
            <std::tie(b.construction,b.stage,b.operation,b.power,b.column,
                      b.direction);
      });
    }
    std::vector<ExecutionStatusRow> rows;
    rows.reserve(groups.size() * 5 + 1);
    std::uint64_t normalized_id=0;
    for (auto& group : groups) {
      for(auto& row:group.second)row.evaluation_id=normalized_id;
      rows.insert(rows.end(), group.second.begin(), group.second.end());
      ++normalized_id;
    }
    ExecutionStatusRow terminal;
    terminal.evaluation_id = normalized_id;
    terminal.record_kind = "terminal";
    terminal.construction = terminal_construction;
    terminal.stage = "final";
    terminal.operation = complete ? complete_operation : abort_operation;
    terminal.valid = complete;
    rows.push_back(std::move(terminal));
    return rows;
  }

 private:
  mutable std::mutex mutex_;
  std::atomic<std::uint64_t> next_{0};
  std::atomic<bool> failed_{false};
  std::vector<std::pair<std::uint64_t,std::vector<ExecutionStatusRow>>> groups_;
};

struct CacheControlRow {
  std::string probe;
  double h = NAN;
  std::string direction;
  int sign = 0;
  bool retraction_metadata = false;
  bool retraction_sector = false;
  bool retraction_finite = false;
  double retraction_gauss = INFINITY;
  double retraction_poisson_absolute = INFINITY;
  bool direct_accepted = false;
  bool inverse_accepted = false;
  bool observer_accepted = false;
  bool endpoint_chart = false;
  bool population_accepted = false;
  bool reuse_accepted = false;
  double direct_population_agreement = INFINITY;
  double direct_reuse_agreement = INFINITY;
  int population_iterations = 0;
  bool cache_valid_after_population = false;
  int population_refreshes = 0;
  int population_reuses = 0;
  int population_fallbacks = 0;
  int reuse_refreshes = 0;
  int reuse_reuses = 0;
  int reuse_fallbacks = 0;
  bool cache_semantics = false;
  bool valid = false;
};

FullGradientAudit audit_full_gradient(
    const ConnectedMooreBlockState& reference, double beta, double lambda,
    double analytic_dx) {
  FullGradientAudit result;
  result.dx = analytic_dx;
  result.dp = 0.0;
  for (const auto& particle : reference.constituents) {
    const double energy =
        ftd::eft::production_flat_energy_from_momentum(particle.momentum);
    if (!(energy > 0.0) || !std::isfinite(energy)) return result;
    const Vec3 gradient = particle.momentum
        * (ftd::C_SPEED*ftd::C_SPEED/energy);
    result.dp = std::max({result.dp,std::abs(gradient.x),
                          std::abs(gradient.y),std::abs(gradient.z)});
  }

  MatchedFaceFlux electric_gradient = reference.electric;
  const auto curl_b = ftd::eft::matched_curl(reference.magnetic_half);
  ftd0774::add_scaled(electric_gradient,curl_b,-0.5*lambda);
  for (std::size_t i=0;i<ftd0774::kVolume;++i) {
    electric_gradient.x[i] *= beta;
    electric_gradient.y[i] *= beta;
    electric_gradient.z[i] *= beta;
  }
  auto e_longitudinal = ftd0774::solve_longitudinal(
      ftd0774::divergence(electric_gradient),1e-13,4096);
  if (!e_longitudinal.valid) return result;
  ftd0774::enforce_zero_face_means(e_longitudinal.field);
  // Recompute after exact periodic-gradient mean enforcement so all three
  // uniform transverse directions remain part of the projected gradient.
  const auto projected_e = ftd0774::face_difference(
      electric_gradient,e_longitudinal.field);
  result.eT = ftd0774::face_max_abs(projected_e);

  MatchedEdgeField magnetic_gradient = reference.magnetic_half;
  const auto curl_e = ftd::eft::matched_curl_adjoint(reference.electric);
  ftd0774::add_scaled(magnetic_gradient,curl_e,-0.5*lambda);
  for (std::size_t i=0;i<ftd0774::kVolume;++i) {
    magnetic_gradient.x[i] *= beta;
    magnetic_gradient.y[i] *= beta;
    magnetic_gradient.z[i] *= beta;
  }
  result.b = ftd0774::edge_max_abs(magnetic_gradient);
  result.global = std::max({result.dx,result.dp,result.eT,result.b});
  result.valid = std::isfinite(result.global)
      && result.dx <= 1e-10 && result.dp <= 1e-10
      && result.eT <= 1e-10 && result.b <= 1e-10;
  return result;
}

struct TangentSummary {
  bool protocol_locked = false;
  bool provenance = false;
  bool source_gate = false;
  bool representative = false;
  bool options = false;
  bool hessian = false;
  bool gradient = false;
  bool seed_metric = false;
  bool energy_form = false;
  bool endpoint_preflight = false;
  bool regularity = false;
  bool cache_control = false;
  bool field_control = false;
  bool preflight = false;
  bool artifact_schema = false;
  bool krylov_executed = false;
  bool krylov_resolved = false;
  int eligible_candidates = 0;
  int qualified_candidates = 0;
  std::string selected_candidate;
  std::string verdict = "L17_COMPLETE_TANGENT_EXECUTION_INVALID";
  std::string companion;
  double beta = NAN;
  double lambda = ftd::C_SPEED;
  double hessian_antisymmetry = NAN;
  double hessian_eigen_residual = NAN;
  double hessian_orthogonality = NAN;
  double hessian_min = NAN;
  double hessian_max = NAN;
  double field_lower_bound = NAN;
  double maximum_gradient = NAN;
  double gradient_dx = NAN;
  double gradient_dp = NAN;
  double gradient_eT = NAN;
  double gradient_b = NAN;
  double b0_gram_residual = NAN;
  double maximum_energy_slope = NAN;
  double maximum_energy_relative = NAN;
  double maximum_common = NAN;
  double maximum_energy_drift = NAN;
  double maximum_recovery = NAN;
  double maximum_scale_relative = NAN;
  double maximum_composition = NAN;
  double maximum_adjoint = NAN;
  double minimum_sigma = NAN;
  double maximum_condition = NAN;
  double maximum_regularity_scale = NAN;
  double maximum_observer_regression = NAN;
  double maximum_codec_divergence = NAN;
  double maximum_hodge_correction = NAN;
  double maximum_reconstruction = NAN;
  double maximum_harmonic = NAN;
  double field_phase_relative = NAN;
  double field_recurrence = NAN;
  double field_recovery = NAN;
  std::map<std::string, int> dimensions;
  std::map<std::string, std::string> artifact_hashes;
};

constexpr char kPreflightHeader[] =
    "record_kind,probe,h,direction,sign,valid,common_residual,energy_drift,recovery,gauss_pre,gauss_clean,hodge_correction,reconstruction,harmonic_face,harmonic_edge,sigma_min,condition,scale_difference,observer_regression,jacobian_refreshes,jacobian_reuses,cache_fallbacks,k_norm,energy_slope,energy_second,energy_relative,derivative_scale_relative,composition_residual,detail";
constexpr char kHessianHeader[] = "row,column,value";
constexpr char kProjectedHeader[] =
    "construction,stage,dimension,matrix,row,column,value";
constexpr char kClustersHeader[] =
    "construction,stage,cluster_id,rank,index,mu,phase,seed_overlap,seed_linked,eligible,in_window,candidate_id";
constexpr char kCandidateMetricsHeader[] =
    "candidate_id,construction,stage,dimension,cluster_id,rank,mu_min,mu_max,phase_mean,phase_split,seed_overlap,ritz_residual,prior_angle,h1_angle,sign_angle,rotation_angle,t_invariance,tinv_invariance,tinv_t_residual,t_tinv_residual,adjoint_residual,orthogonality_residual,modulus_residual,conjugacy_residual,conjugacy_separation,intertwining_residual,gram_min,gram_max,gram_ratio,qualified,detail";
constexpr char kCandidateIndexHeader[] =
    "candidate_id,construction,stage,vector_kind,column,chart_dimension,byte_offset,byte_length";
constexpr char kGramHeader[] =
    "candidate_id,construction,stage,block,row,column,value";
constexpr char kExecutionStatusHeader[] =
    "evaluation_id,record_kind,construction,stage,operation,power,column,h,direction,sign,valid,metadata,sector,finite,gauss,poisson_absolute,endpoint_chart,common_residual,energy_drift,recovery,codec_gauss_pre,codec_gauss_clean,hodge_correction,reconstruction,harmonic_face,harmonic_edge,tangent_source_mean_rel,hodge_source_mean_rel,tangent_poisson_relative,hodge_poisson_relative,detail";
constexpr char kRuntimeHeader[] = "name,value";
constexpr char kEnergyControlHeader[] =
    "probe,sign,h,metadata,sector,finite,gauss,poisson_absolute,increment";
constexpr char kCacheControlHeader[] =
    "probe,h,direction,sign,retraction_metadata,retraction_sector,retraction_finite,retraction_gauss,retraction_poisson_absolute,direct_accepted,inverse_accepted,observer_accepted,endpoint_chart,population_accepted,reuse_accepted,direct_population_agreement,direct_reuse_agreement,population_iterations,cache_valid_after_population,population_refreshes,population_reuses,population_fallbacks,reuse_refreshes,reuse_reuses,reuse_fallbacks,cache_semantics,valid";
constexpr char kFieldControlHeader[] =
    "record_kind,tick,target_amplitude,q,divergence,signal_energy,background_energy,recovery_face_absolute,recovery_edge_absolute,recovery_relative,maximum_divergence,maximum_energy_drift";
constexpr char kKrylovStatusHeader[] =
    "construction,generated_power_count,accepted_dimension,prior_dimension,last_nonempty_power,last_nonempty_start,last_nonempty_end,deflation_count,happy_breakdown,exhausted_16_powers,bookkeeping_valid,projected_final_present,terminal_t_invariance,terminal_tinv_invariance,terminal_invariance_eligible";

void write_optional_bool(std::ostream& output,
                         const std::optional<bool>& value) {
  if (value.has_value()) output << (*value ? 1 : 0);
}

bool write_execution_status(const std::filesystem::path& stem,
                            const OperatorAuditLedger& ledger,
                            bool complete,
                            const std::string& suffix="_execution_status.csv",
                            const std::string& terminal_construction="all",
                            const std::string& complete_operation="run_complete",
                            const std::string& abort_operation="run_abort",
                            bool preserve_reservation_order=false) {
  std::ofstream output(stem.string()+suffix);
  if(!output)return false;
  output << kExecutionStatusHeader << '\n';
  for (const auto& row : ledger.ordered_rows(complete,terminal_construction,
                                             complete_operation,
                                             abort_operation,
                                             preserve_reservation_order)) {
    output << row.evaluation_id << ',' << row.record_kind << ','
        << row.construction << ',' << row.stage << ',' << row.operation << ',';
    if (row.power.has_value()) output << *row.power;
    output << ',';
    if (row.column.has_value()) output << *row.column;
    output << ','; csv_number(output,row.h);
    output << ',' << row.direction << ',' << row.sign << ',' << row.valid << ',';
    write_optional_bool(output,row.metadata); output << ',';
    write_optional_bool(output,row.sector); output << ',';
    write_optional_bool(output,row.finite); output << ',';
    csv_number(output,row.gauss); output << ',';
    csv_number(output,row.poisson_absolute); output << ',';
    write_optional_bool(output,row.endpoint_chart); output << ',';
    csv_number(output,row.common_residual); output << ',';
    csv_number(output,row.energy_drift); output << ',';
    csv_number(output,row.recovery); output << ',';
    csv_number(output,row.codec_gauss_pre); output << ',';
    csv_number(output,row.codec_gauss_clean); output << ',';
    csv_number(output,row.hodge_correction); output << ',';
    csv_number(output,row.reconstruction); output << ',';
    csv_number(output,row.harmonic_face); output << ',';
    csv_number(output,row.harmonic_edge); output << ',';
    csv_number(output,row.tangent_source_mean_rel); output << ',';
    csv_number(output,row.hodge_source_mean_rel); output << ',';
    csv_number(output,row.tangent_poisson_relative); output << ',';
    csv_number(output,row.hodge_poisson_relative); output << ','
        << row.detail << '\n';
  }
  output.flush();
  return static_cast<bool>(output);
}

bool write_runtime_audit(
    const std::filesystem::path& stem,
    const ftd::eft::ConnectedMooreBlockOptions& options,
    const ConnectedMooreBlockState& reference,
    const ftd::eft::ConnectedMooreBlockInitialization& initialized,
    const ftd::eft::FaceFluxNormalization& normalization,double beta,
    const Deposit& density,
    const std::array<StoredMode,2>& modes) {
  std::ofstream output(stem.string()+"_runtime.csv");
  output<<kRuntimeHeader<<'\n'<<std::setprecision(17);
  const auto number=[&](const char* name,double value){
    output<<name<<',';csv_number(output,value);output<<'\n';
  };
  const auto integer=[&](const char* name,std::int64_t value){
    output<<name<<','<<value<<'\n';
  };
  const auto boolean=[&](const char* name,bool value){
    output<<name<<','<<(value?1:0)<<'\n';
  };
  number("option_wave_speed",options.wave_speed);
  number("option_dt",options.dt);
  number("option_binding_stiffness",options.binding_stiffness);
  output<<"option_binding_law,FixedEdgeQuartic\n";
  number("option_compact_pair_well_depth",options.compact_pair_well_depth);
  number("option_compact_pair_cutoff_distance_squared",
      options.compact_pair_cutoff_distance_squared);
  number("option_constituent_mass_scale",options.constituent_mass_scale);
  number("option_polarity_scale",options.polarity_scale);
  number("option_field_energy_scale",options.field_energy_scale);
  number("option_gate_tolerance",options.gate_tolerance);
  number("option_solve_tolerance",options.solve_tolerance);
  number("option_finite_difference_scale",options.finite_difference_scale);
  integer("option_max_iterations",options.max_iterations);
  boolean("option_allow_shared_anchor_chart",options.allow_shared_anchor_chart);
  boolean("option_use_sparse_local_current",options.use_sparse_local_current);
  boolean("option_use_local_residual_evaluation",
      options.use_local_residual_evaluation);
  boolean("option_use_low_rank_identity_broyden",
      options.use_low_rank_identity_broyden);
  boolean("option_use_matrix_free_newton_krylov",
      options.use_matrix_free_newton_krylov);
  boolean("option_defer_volume_diagnostics",options.defer_volume_diagnostics);
  boolean("option_measure_final_root_regularity",
      options.measure_final_root_regularity);
  integer("option_root_momentum_seed_size",
      static_cast<std::int64_t>(options.root_momentum_seed.size()));
  integer("reference_L",reference.electric.L);
  boolean("initialized_valid",initialized.valid);
  boolean("initializer_graph_connected",initialized.graph_connected);
  boolean("initializer_graph_local",initialized.graph_local);
  boolean("initializer_site_projection_valid",
      initialized.site_projection_valid);
  integer("initializer_poisson_iterations",initialized.poisson_iterations);
  number("initializer_poisson_residual",initialized.poisson_residual);
  number("initializer_gauss_residual",initialized.gauss_residual);
  number("initializer_curl_adjoint_residual",
      initialized.curl_adjoint_residual);
  integer("constituent_count",
      static_cast<std::int64_t>(reference.constituents.size()));
  integer("orientation_axis",reference.orientation_axis);
  integer("width",reference.width);
  boolean("normalization_valid",normalization.valid);
  number("normalization_field_scale",normalization.field_scale);
  number("normalization_current_scale",normalization.current_scale);
  number("normalization_energy_scale",normalization.energy_scale);
  number("normalization_native_susceptibility",
      normalization.native_susceptibility);
  number("normalization_mapped_susceptibility",
      normalization.mapped_susceptibility);
  number("normalization_native_action_work_coefficient",
      normalization.native_action_work_coefficient);
  number("normalization_mapped_field_work_coefficient",
      normalization.mapped_field_work_coefficient);
  number("normalization_susceptibility_residual",
      normalization.susceptibility_residual);
  number("normalization_work_residual",normalization.work_residual);
  number("normalization_work_coefficient",
      normalization.mapped_field_work_coefficient);
  number("beta",beta);
  boolean("density_jet_valid",density.valid);
  number("density_charge_residual",density.charge_residual);
  number("derivative_charge_residual",density.derivative_charge_residual);
  number("density_derivative_moment_residual",
      density.derivative_moment_residual);
  boolean("mode6_valid",modes[0].valid);
  boolean("mode7_valid",modes[1].valid);
  for(int index=0;index<2;++index){
    const std::string prefix=index==0?"mode6_":"mode7_";
    integer((prefix+"number").c_str(),modes[index].mode);
    integer((prefix+"group").c_str(),modes[index].group);
    number((prefix+"phase").c_str(),modes[index].phase);
  }
  double gram00=NAN,gram01=NAN,gram11=NAN;
  if(modes[0].vector.size()==ftd0774::kMatterDimension
      &&modes[1].vector.size()==ftd0774::kMatterDimension){
    long double g00=0.0L,g01=0.0L,g11=0.0L;
    for(int i=0;i<ftd0774::kMatterDimension;++i){
      g00+=modes[0].vector[i]*ftd::M_INERTIAL*modes[0].vector[i];
      g01+=modes[0].vector[i]*ftd::M_INERTIAL*modes[1].vector[i];
      g11+=modes[1].vector[i]*ftd::M_INERTIAL*modes[1].vector[i];
    }
    gram00=static_cast<double>(g00);
    gram01=static_cast<double>(g01);
    gram11=static_cast<double>(g11);
  }
  number("mode_mass_gram_00",gram00);
  number("mode_mass_gram_01",gram01);
  number("mode_mass_gram_11",gram11);
  output.flush();
  return static_cast<bool>(output);
}

void write_energy_control_row(
    std::ostream& output,const std::string& probe,int sign,double h,
    const RetractionResult& retracted,double increment) {
  output<<probe<<','<<sign<<','<<std::setprecision(17)<<h<<','
      <<retracted.metadata<<','<<retracted.sector<<','<<retracted.finite<<',';
  csv_number(output,retracted.gauss_residual);output<<',';
  csv_number(output,retracted.poisson_absolute_residual);output<<',';
  csv_number(output,increment);output<<'\n';
}

void write_cache_control_row(std::ostream& output,
                             const CacheControlRow& row) {
  output<<row.probe<<','<<std::setprecision(17)<<row.h<<','<<row.direction
      <<','<<row.sign<<','<<row.retraction_metadata<<','
      <<row.retraction_sector<<','<<row.retraction_finite<<',';
  csv_number(output,row.retraction_gauss);output<<',';
  csv_number(output,row.retraction_poisson_absolute);output<<','
      <<row.direct_accepted<<','<<row.inverse_accepted<<','
      <<row.observer_accepted<<','<<row.endpoint_chart<<','
      <<row.population_accepted<<','
      <<row.reuse_accepted<<',';
  csv_number(output,row.direct_population_agreement);output<<',';
  csv_number(output,row.direct_reuse_agreement);output<<','
      <<row.population_iterations<<','<<row.cache_valid_after_population<<','
      <<row.population_refreshes<<','<<row.population_reuses<<','
      <<row.population_fallbacks<<','<<row.reuse_refreshes<<','
      <<row.reuse_reuses<<','<<row.reuse_fallbacks<<','
      <<row.cache_semantics<<','<<row.valid<<'\n';
}

bool write_field_control_artifact(const std::filesystem::path& stem,
                                  const FieldControl& field) {
  if(field.q.size()!=256||field.divergence_series.size()!=256
      ||field.signal_energy_series.size()!=256
      ||field.background_energy_series.size()!=256)return false;
  std::ofstream output(stem.string()+"_field_control.csv");
  if(!output)return false;
  output<<kFieldControlHeader<<'\n'<<std::setprecision(17);
  output<<"initial,-1,"<<field.target_amplitude<<",,,"
      <<field.initial_signal_energy<<','<<field.initial_background_energy
      <<",,,,,\n";
  for(int tick=0;tick<256;++tick)
    output<<"sample,"<<tick<<','<<field.target_amplitude<<','<<field.q[tick]
        <<','<<field.divergence_series[tick]<<','
        <<field.signal_energy_series[tick]<<','
        <<field.background_energy_series[tick]<<",,,,,\n";
  output<<"summary,256,"<<field.target_amplitude<<",,,,,"
      <<field.recovery_face_absolute<<','<<field.recovery_edge_absolute<<','
      <<field.recovery_relative<<','<<field.divergence<<','
      <<field.energy_drift<<'\n';
  output.flush();
  return static_cast<bool>(output);
}

bool write_header_only(const std::filesystem::path& path,
                       const std::string& header) {
  std::ofstream output(path,std::ios::trunc);
  if(!output)return false;
  output<<header<<'\n';
  output.flush();
  return static_cast<bool>(output);
}

bool initialize_artifacts(const std::filesystem::path& stem) {
  std::error_code error;
  std::filesystem::create_directories(stem.parent_path(),error);
  if(error)return false;
  bool valid=true;
  valid=write_header_only(stem.string()+"_preflight.csv",kPreflightHeader)
      &&valid;
  valid=write_header_only(stem.string()+"_hessian.csv",kHessianHeader)&&valid;
  valid=write_header_only(stem.string()+"_projected_matrices.csv",
      kProjectedHeader)&&valid;
  valid=write_header_only(stem.string()+"_clusters.csv",kClustersHeader)
      &&valid;
  valid=write_header_only(stem.string()+"_candidate_metrics.csv",
      kCandidateMetricsHeader)&&valid;
  {
    std::ofstream binary(stem.string()+"_candidate_vectors.bin",
        std::ios::binary|std::ios::trunc);
    valid=static_cast<bool>(binary)&&valid;
  }
  valid=write_header_only(stem.string()+"_candidate_vectors_index.csv",
      kCandidateIndexHeader)&&valid;
  valid=write_header_only(stem.string()+"_gram_blocks.csv",kGramHeader)&&valid;
  valid=write_header_only(stem.string()+"_execution_status.csv",
      kExecutionStatusHeader)&&valid;
  valid=write_header_only(stem.string()+"_preflight_derivative_status.csv",
      kExecutionStatusHeader)&&valid;
  valid=write_header_only(stem.string()+"_runtime.csv",kRuntimeHeader)&&valid;
  valid=write_header_only(stem.string()+"_energy_control.csv",
      kEnergyControlHeader)&&valid;
  valid=write_header_only(stem.string()+"_cache_control.csv",
      kCacheControlHeader)&&valid;
  valid=write_header_only(stem.string()+"_field_control.csv",
      kFieldControlHeader)&&valid;
  valid=write_header_only(stem.string()+"_krylov_status.csv",
      kKrylovStatusHeader)&&valid;
  return valid;
}

void write_hessian(const std::filesystem::path& stem, const Arm& analytic) {
  std::ofstream output(stem.string()+"_hessian.csv");
  output << kHessianHeader << '\n' << std::setprecision(17);
  for (int row = 0; row < ftd0774::kMatterDimension; ++row)
    for (int column = 0; column < ftd0774::kMatterDimension; ++column)
      output << row << ',' << column << ',' << analytic.hessian[row][column] << '\n';
  for (int index = 0; index < ftd0774::kMatterDimension; ++index)
    output << -1 << ',' << index << ',' << analytic.eigenvalues[index] << '\n';
}

void write_matrix_block(std::ostream& output, const std::string& candidate,
                        const std::string& construction,
                        const std::string& stage, const std::string& block,
                        const SmallMatrix& matrix) {
  output << std::setprecision(17);
  for (std::size_t row = 0; row < matrix.size(); ++row)
    for (std::size_t column = 0; column < matrix[row].size(); ++column)
      output << candidate << ',' << construction << ',' << stage << ','
          << block << ',' << row << ',' << column << ','
          << matrix[row][column] << '\n';
}

struct TangentImages {
  bool valid = false;
  ChartVector t;
  ChartVector tinv;
  ChartVector s;
};

struct TangentCluster {
  int cluster_id = -1;
  std::vector<int> indices;
  std::vector<double> mus;
  std::vector<double> phases;
  double seed_overlap = NAN;
  bool seed_linked = false;
  bool in_window = false;
  bool eligible = false;
  std::string candidate_id;
};

struct TangentProjectedStage {
  std::string construction;
  std::string stage;
  int dimension = 0;
  bool valid = false;
  bool exact_domain = false;
  bool terminal_invariant = false;
  double t_invariance = INFINITY;
  double tinv_invariance = INFINITY;
  std::vector<ChartVector> basis;
  std::vector<ChartVector> seeds;
  std::vector<TangentImages> images;
  std::map<std::string,SmallMatrix> matrices;
  std::vector<TangentCluster> clusters;
};

struct TangentKrylovRun {
  std::string construction;
  double h = NAN;
  bool execution_valid = false;
  bool happy_breakdown = false;
  bool exhausted = false;
  bool bookkeeping_valid = false;
  int generated_power_count = 0;
  int prior_dimension = 0;
  int last_nonempty_power = -1;
  int last_nonempty_start = 0;
  int last_nonempty_end = 0;
  int deflation_count = 0;
  std::vector<ChartVector> seeds;
  std::vector<ChartVector> basis;
  std::vector<TangentProjectedStage> stages;
};

struct TangentCandidate {
  std::string candidate_id;
  std::string construction;
  std::string stage;
  int dimension = 0;
  int cluster_id = -1;
  std::vector<double> mus;
  std::vector<ChartVector> u;
  std::vector<ChartVector> su;
  std::vector<ChartVector> tu;
  std::vector<ChartVector> tinvu;
  std::vector<ChartVector> tinv_tu;
  std::vector<ChartVector> t_tinvu;
  std::map<std::string,SmallMatrix> grams;
  double mu_min = NAN;
  double mu_max = NAN;
  double phase_mean = INFINITY;
  double phase_split = INFINITY;
  double seed_overlap = NAN;
  double ritz_residual = INFINITY;
  double prior_angle = INFINITY;
  double h1_angle = INFINITY;
  double sign_angle = INFINITY;
  double rotation_angle = INFINITY;
  double t_invariance = INFINITY;
  double tinv_invariance = INFINITY;
  double tinv_t_residual = INFINITY;
  double t_tinv_residual = INFINITY;
  double adjoint_residual = INFINITY;
  double orthogonality_residual = INFINITY;
  double modulus_residual = INFINITY;
  double conjugacy_residual = INFINITY;
  double conjugacy_separation = -INFINITY;
  double intertwining_residual = INFINITY;
  double gram_min = NAN;
  double gram_max = NAN;
  double gram_ratio = NAN;
  bool core_qualified = false;
  bool qualified = false;
  std::string matched_prior;
  std::string matched_h1;
  std::string matched_sign;
  std::string matched_rotation;
};

struct TangentPipeline {
  bool execution_valid = false;
  bool projected_valid = false;
  bool solve_resolved = false;
  bool seeded_rank_gt_four = false;
  std::vector<TangentKrylovRun> runs;
  std::vector<TangentCandidate> candidates;
};

bool tangent_matrix_finite(const SmallMatrix& matrix) {
  return !matrix.empty() && std::all_of(matrix.begin(),matrix.end(),
      [](const auto& row) {
        return !row.empty() && std::all_of(row.begin(),row.end(),
            [](double value){return std::isfinite(value);});
      });
}

SmallMatrix tangent_block_gram(const TangentMetric& metric,
                               const std::vector<ChartVector>& left,
                               const std::vector<ChartVector>& right) {
  SmallMatrix result(left.size(),std::vector<double>(right.size(),0.0));
  for (std::size_t i=0;i<left.size();++i)
    for (std::size_t j=0;j<right.size();++j)
      result[i][j]=ftd0774::inner(metric,left[i],right[j]);
  return result;
}

SmallMatrix tangent_average(const SmallMatrix& left,
                            const SmallMatrix& right) {
  if(left.size()!=right.size())return{};
  SmallMatrix result=left;
  for(std::size_t i=0;i<result.size();++i){
    if(result[i].size()!=right[i].size())return{};
    for(std::size_t j=0;j<result[i].size();++j)
      result[i][j]=0.5*(left[i][j]+right[i][j]);
  }
  return result;
}

std::vector<ChartVector> tangent_linear_combination(
    const std::vector<ChartVector>& basis,const SmallMatrix& coefficients) {
  if(basis.empty()||coefficients.size()!=basis.size())return{};
  const std::size_t columns=coefficients.front().size();
  std::vector<ChartVector> result(columns);
  for(std::size_t row=0;row<basis.size();++row){
    if(coefficients[row].size()!=columns)return{};
    for(std::size_t column=0;column<columns;++column)
      ftd0774::axpy(result[column],basis[row],coefficients[row][column]);
  }
  return result;
}

std::vector<ChartVector> tangent_block_difference(
    const std::vector<ChartVector>& left,
    const std::vector<ChartVector>& right) {
  if(left.size()!=right.size())return{};
  std::vector<ChartVector> result;
  result.reserve(left.size());
  for(std::size_t i=0;i<left.size();++i)
    result.push_back(ftd0774::difference(left[i],right[i]));
  return result;
}

double tangent_block_k_norm(const TangentMetric& metric,
                            const std::vector<ChartVector>& block) {
  long double square=0.0L;
  for(const auto& column:block){
    const double value=ftd0774::inner(metric,column,column);
    if(!std::isfinite(value)||value<0.0)return INFINITY;
    square+=value;
  }
  const double total=static_cast<double>(square);
  if(!std::isfinite(total)||total<0.0)return INFINITY;
  return std::sqrt(total);
}

template<class Chart>
DerivativeEvaluation audited_derivative(
    const Chart& chart,const ChartVector& value,double h,bool forward,
    const ftd::eft::ConnectedMooreBlockOptions& options,
    OperatorAuditLedger& ledger,const OperatorAuditKey& key) {
  const std::uint64_t id=ledger.reserve();
  const auto evaluated=evaluate_derivative(chart,value,h,forward,options,false);
  ledger.record(id,key,evaluated);
  return evaluated;
}

template<class Chart>
TangentImages audited_images(
    const Chart& chart,const ChartVector& value,double h,
    const ftd::eft::ConnectedMooreBlockOptions& options,
    OperatorAuditLedger& ledger,OperatorAuditKey key) {
  TangentImages result;
  key.h=h;
  key.forward=true;
  const auto forward=audited_derivative(
      chart,value,h,true,options,ledger,key);
  key.forward=false;
  const auto reverse=audited_derivative(
      chart,value,h,false,options,ledger,key);
  if(!forward.valid||!reverse.valid)return result;
  result.t=forward.value;
  result.tinv=reverse.value;
  result.s=ftd0774::scaled(result.t,0.5);
  ftd0774::axpy(result.s,result.tinv,0.5);
  result.valid=ftd0774::finite_chart(result.s);
  if(!result.valid){
    key.operation="operator_average";
    key.forward=true;
    ledger.record_numeric_failure(key,NAN,"nonfinite_operator_average");
  }
  return result;
}

template<class Chart>
bool tangent_filter_block(
    const Chart& chart,const std::vector<ChartVector>& input,int power,
    double h,const std::string& construction,
    const ftd::eft::ConnectedMooreBlockOptions& options,
    OperatorAuditLedger& ledger,std::vector<ChartVector>& output) {
  std::vector<std::future<std::pair<bool,ChartVector>>> futures;
  for(int column=0;column<4;++column)
    futures.push_back(std::async(std::launch::async,[&,column]() {
      OperatorAuditKey key{construction,"krylov","filter_v",power,column,h,true};
      const auto first=audited_images(
          chart,input[column],h,options,ledger,key);
      if(!first.valid)return std::make_pair(false,ChartVector{});
      ChartVector w=first.s;
      ftd0774::axpy(w,input[column],-std::cos(kInternalPhase));
      key.operation="filter_w";
      const auto second=audited_images(chart,w,h,options,ledger,key);
      if(!second.valid)return std::make_pair(false,ChartVector{});
      ChartVector correction=second.s;
      ftd0774::axpy(correction,w,-std::cos(kInternalPhase));
      ChartVector filtered=input[column];
      ftd0774::axpy(filtered,correction,-0.25);
      const bool finite=ftd0774::finite_chart(filtered);
      if(!finite){
        key.operation="filter_output";
        ledger.record_numeric_failure(key,NAN,"nonfinite_filter_output");
      }
      return std::make_pair(finite,std::move(filtered));
    }));
  output.assign(4,ChartVector{});
  bool valid=true;
  for(int column=0;column<4;++column){
    auto value=futures[column].get();
    valid=valid&&value.first;
    output[column]=std::move(value.second);
  }
  return valid&&!ledger.failed();
}

template<class Chart>
TangentKrylovRun tangent_build_krylov(
    const Chart& chart,const TangentMetric& metric,
    const ftd::eft::ConnectedMooreBlockOptions& options,
    const std::string& construction,double h,
    const std::vector<ChartVector>& seeds,OperatorAuditLedger& ledger) {
  TangentKrylovRun result;
  result.construction=construction;
  result.h=h;
  result.seeds=seeds;
  std::vector<ChartVector> raw=seeds;
  bool valid=raw.size()==4;
  if(!valid)ledger.record_numeric_failure(
      {construction,"krylov","seed_block",std::nullopt,std::nullopt,h,true},
      static_cast<double>(raw.size()),"invalid_seed_block_dimension");
  for(int power=0;valid&&power<16;++power){
    const int block_start=static_cast<int>(result.basis.size());
    int accepted_in_block=0;
    for(int column=0;column<4;++column){
      ChartVector candidate=raw[column];
      for(int pass=0;pass<2;++pass)
        for(const auto& basis_column:result.basis)
          ftd0774::axpy(candidate,basis_column,
              -ftd0774::inner(metric,basis_column,candidate));
      double square=ftd0774::inner(metric,candidate,candidate);
      if(!std::isfinite(square)||square<0.0){
        ledger.record_numeric_failure(
            {construction,"krylov","mgs_norm",power,column,h,true},
            square,"nonfinite_or_negative_k_norm");
        valid=false;break;
      }
      const double candidate_norm=std::sqrt(square);
      if(candidate_norm<1e-12)continue;
      if(result.basis.size()>=64){
        ledger.record_numeric_failure(
            {construction,"krylov","basis_cap",power,column,h,true},
            static_cast<double>(result.basis.size()),
            "acceptable_column_above_cap",candidate_norm);
        valid=false;break;
      }
      candidate=ftd0774::scaled(candidate,1.0/candidate_norm);
      if(!ftd0774::finite_chart(candidate)){
        ledger.record_numeric_failure(
            {construction,"krylov","normalized_basis",power,column,h,true},
            NAN,"nonfinite_normalized_basis");
        valid=false;break;
      }
      result.basis.push_back(std::move(candidate));
      ++accepted_in_block;
    }
    result.generated_power_count=power+1;
    if(!valid)break;
    if(accepted_in_block==0){
      result.happy_breakdown=true;
      break;
    }
    result.last_nonempty_power=power;
    result.last_nonempty_start=block_start;
    result.last_nonempty_end=static_cast<int>(result.basis.size());
    if(result.basis.size()>=64)break;
    if(power==15){result.exhausted=true;break;}
    std::vector<ChartVector> next;
    if(!tangent_filter_block(chart,raw,power+1,h,construction,
          options,ledger,next)){valid=false;break;}
    raw=std::move(next);
  }
  if(valid&&!result.happy_breakdown&&result.generated_power_count==16)
    result.exhausted=true;
  const int accepted=static_cast<int>(result.basis.size());
  result.deflation_count=4*result.generated_power_count-accepted;
  result.prior_dimension=accepted==64?48:
      (accepted<=4?0:result.last_nonempty_start);
  const bool structural=result.generated_power_count>=1
      &&result.generated_power_count<=16&&accepted>=0&&accepted<=64
      &&result.last_nonempty_power>=-1
      &&result.last_nonempty_power<result.generated_power_count
      &&result.last_nonempty_start>=0
      &&result.last_nonempty_start<=result.last_nonempty_end
      &&result.last_nonempty_end==accepted
      &&result.deflation_count==4*result.generated_power_count-accepted
      &&result.prior_dimension==(accepted==64?48:
          (accepted<=4?0:result.last_nonempty_start))
      &&result.prior_dimension>=0
      &&(accepted==0?result.prior_dimension==0:
          result.prior_dimension<accepted)
      &&!(result.happy_breakdown&&result.exhausted)
      &&(!result.exhausted||result.generated_power_count==16)
      &&(result.happy_breakdown||result.exhausted||accepted==64);
  result.bookkeeping_valid=structural;
  if(valid&&!structural)
    ledger.record_bookkeeping_failure(
        {construction,"krylov","bookkeeping",std::nullopt,std::nullopt,h,true},
        result.generated_power_count,accepted,result.prior_dimension,
        result.last_nonempty_power,result.last_nonempty_start,
        result.last_nonempty_end,result.deflation_count,
        result.happy_breakdown,result.exhausted);
  result.execution_valid=valid&&!ledger.failed()&&structural;
  return result;
}

template<class Chart>
TangentProjectedStage tangent_project_stage(
    const Chart& chart,const TangentMetric& metric,
    const ftd::eft::ConnectedMooreBlockOptions& options,
    const TangentKrylovRun& run,const std::string& stage_name,int dimension,
    OperatorAuditLedger& ledger) {
  TangentProjectedStage result;
  result.construction=run.construction;
  result.stage=stage_name;
  result.dimension=dimension;
  result.seeds=run.seeds;
  if(dimension<=0||dimension>static_cast<int>(run.basis.size()))return result;
  result.basis.assign(run.basis.begin(),run.basis.begin()+dimension);
  result.images.resize(dimension);
  constexpr int batch_size=24;
  for(int start=0;start<dimension&&!ledger.failed();start+=batch_size){
    const int end=std::min(start+batch_size,dimension);
    std::vector<std::future<TangentImages>> futures;
    futures.reserve(end-start);
    for(int column=start;column<end;++column)
      futures.push_back(std::async(std::launch::async,[&,column]() {
        OperatorAuditKey key{run.construction,stage_name,"basis_image",
            std::nullopt,column,run.h,true};
        return audited_images(chart,result.basis[column],run.h,
            options,ledger,key);
      }));
    for(int column=start;column<end;++column)
      result.images[column]=futures[column-start].get();
  }
  if(ledger.failed()||std::any_of(result.images.begin(),result.images.end(),
      [](const TangentImages& image){return !image.valid;}))return result;
  std::vector<ChartVector> tv,tinvv,sv;
  tv.reserve(dimension);tinvv.reserve(dimension);sv.reserve(dimension);
  for(const auto& image:result.images){
    tv.push_back(image.t);tinvv.push_back(image.tinv);sv.push_back(image.s);
  }
  auto& matrices=result.matrices;
  matrices["V_K_V"]=tangent_block_gram(metric,result.basis,result.basis);
  matrices["A_T"]=tangent_block_gram(metric,result.basis,tv);
  matrices["A_TINV"]=tangent_block_gram(metric,result.basis,tinvv);
  matrices["A_S"]=tangent_block_gram(metric,result.basis,sv);
  matrices["SEED"]=tangent_block_gram(metric,result.basis,result.seeds);
  matrices["TV_K_TV"]=tangent_block_gram(metric,tv,tv);
  matrices["TINV_V_K_TINV_V"]=tangent_block_gram(metric,tinvv,tinvv);
  const auto projected_tv=tangent_linear_combination(
      result.basis,matrices["A_T"]);
  const auto projected_tinv=tangent_linear_combination(
      result.basis,matrices["A_TINV"]);
  const auto t_residual=tangent_block_difference(tv,projected_tv);
  const auto tinv_residual=tangent_block_difference(tinvv,projected_tinv);
  matrices["T_RESIDUAL_K_T_RESIDUAL"]=
      tangent_block_gram(metric,t_residual,t_residual);
  matrices["TINV_RESIDUAL_K_TINV_RESIDUAL"]=
      tangent_block_gram(metric,tinv_residual,tinv_residual);
  bool finite=true;
  for(const auto& [name,matrix]:matrices)finite=finite&&tangent_matrix_finite(matrix);
  if(!finite){
    ledger.record_numeric_failure(
        {run.construction,stage_name,"projected_nonfinite",std::nullopt,
         std::nullopt,run.h,true},NAN,"nonfinite_projected_matrix");
    return result;
  }
  SmallMatrix gram_delta=matrices["V_K_V"];
  for(int i=0;i<dimension;++i)gram_delta[i][i]-=1.0;
  const double gram_residual=ftd0774::frobenius(gram_delta);
  const double asymmetry=ftd0774::relative_frobenius(
      matrices["A_S"],ftd0774::transpose(matrices["A_S"]));
  const auto operator_average=tangent_average(
      matrices["A_T"],matrices["A_TINV"]);
  const double operator_agreement=ftd0774::relative_frobenius(
      matrices["A_S"],operator_average);
  const auto symmetric=tangent_average(
      matrices["A_S"],ftd0774::transpose(matrices["A_S"]));
  const auto eig=ftd0774::symmetric_eigen(symmetric);
  if(!eig.valid||eig.values.size()!=static_cast<std::size_t>(dimension)){
    ledger.record_numeric_failure(
        {run.construction,stage_name,"symmetric_eigensolver",std::nullopt,
         std::nullopt,run.h,true},eig.residual,
        "invalid_symmetric_eigensolver",NAN,false);
    return result;
  }
  bool expanded_domain=true;
  result.exact_domain=true;
  for(double mu:eig.values){
    expanded_domain=expanded_domain&&mu>=-1.0-2e-4&&mu<=1.0+2e-4;
    result.exact_domain=result.exact_domain&&mu>=-1.0&&mu<=1.0;
  }
  const double tv_norm=tangent_block_k_norm(metric,tv);
  const double tinv_norm=tangent_block_k_norm(metric,tinvv);
  const double tv_residual_norm=tangent_block_k_norm(metric,t_residual);
  const double tinv_residual_norm=tangent_block_k_norm(metric,tinv_residual);
  const bool invariant_well_formed=std::isfinite(tv_norm)
      &&std::isfinite(tinv_norm)&&std::isfinite(tv_residual_norm)
      &&std::isfinite(tinv_residual_norm)&&tv_norm>0.0&&tinv_norm>0.0;
  if(invariant_well_formed){
    result.t_invariance=tv_residual_norm/tv_norm;
    result.tinv_invariance=tinv_residual_norm/tinv_norm;
  }
  result.terminal_invariant=invariant_well_formed
      &&result.t_invariance<=2e-4&&result.tinv_invariance<=2e-4;
  result.valid=gram_residual<=1e-10&&asymmetry<=1e-4
      &&operator_agreement<=2e-4&&expanded_domain&&result.exact_domain;
  if(result.exact_domain){
    std::vector<double> phases(dimension);
    std::vector<int> order(dimension);
    for(int i=0;i<dimension;++i){phases[i]=std::acos(eig.values[i]);order[i]=i;}
    std::sort(order.begin(),order.end(),[&](int left,int right){
      return phases[left]<phases[right];
    });
    std::vector<std::vector<int>> groups;
    for(int index:order){
      if(groups.empty()||phases[index]-phases[groups.back().back()]>5e-4)
        groups.push_back({index});
      else groups.back().push_back(index);
    }
    for(std::size_t cluster_index=0;cluster_index<groups.size();++cluster_index){
      TangentCluster cluster;
      cluster.cluster_id=static_cast<int>(cluster_index);
      cluster.indices=groups[cluster_index];
      cluster.seed_overlap=0.0;
      cluster.in_window=true;
      for(int index:cluster.indices){
        cluster.mus.push_back(eig.values[index]);
        cluster.phases.push_back(phases[index]);
        cluster.in_window=cluster.in_window
            &&std::abs(phases[index]-kInternalPhase)<=0.08;
        for(int seed=0;seed<4;++seed){
          long double projection=0.0L;
          for(int row=0;row<dimension;++row)
            projection+=static_cast<long double>(eig.vectors[row][index])
                *matrices["SEED"][row][seed];
          cluster.seed_overlap+=static_cast<double>(projection*projection);
        }
      }
      cluster.seed_linked=cluster.seed_overlap>=0.10;
      cluster.eligible=cluster.in_window&&cluster.seed_linked
          &&cluster.indices.size()==4;
      if(cluster.eligible){
        std::ostringstream id;
        id<<run.construction<<'_'<<stage_name<<"_c"
          <<std::setw(3)<<std::setfill('0')<<cluster.cluster_id;
        cluster.candidate_id=id.str();
      }
      result.clusters.push_back(std::move(cluster));
    }
  }
  return result;
}

template<class Chart>
bool tangent_project_run(
    const Chart& chart,const TangentMetric& metric,
    const ftd::eft::ConnectedMooreBlockOptions& options,
    TangentKrylovRun& run,OperatorAuditLedger& ledger) {
  if(!run.execution_valid)return false;
  if(run.basis.size()<4)return true;
  if(run.prior_dimension>0){
    auto prior=tangent_project_stage(chart,metric,options,run,"prior",
        run.prior_dimension,ledger);
    if(ledger.failed())return false;
    run.stages.push_back(std::move(prior));
  }
  auto final_stage=tangent_project_stage(chart,metric,options,run,"final",
      static_cast<int>(run.basis.size()),ledger);
  if(ledger.failed())return false;
  if(!std::isfinite(final_stage.t_invariance)
      ||!std::isfinite(final_stage.tinv_invariance)){
    ledger.record_numeric_failure(
        {run.construction,"final","terminal_invariance",std::nullopt,
         std::nullopt,run.h,true},final_stage.t_invariance,
        "nonfinite_terminal_invariance");
    return false;
  }
  SmallMatrix status(1,std::vector<double>(13,0.0));
  status[0][0]=run.generated_power_count;
  status[0][1]=run.basis.size();
  status[0][2]=run.prior_dimension;
  status[0][3]=run.last_nonempty_power;
  status[0][4]=run.last_nonempty_start;
  status[0][5]=run.last_nonempty_end;
  status[0][6]=run.deflation_count;
  status[0][7]=run.happy_breakdown?1.0:0.0;
  status[0][8]=run.exhausted?1.0:0.0;
  status[0][9]=final_stage.t_invariance;
  status[0][10]=final_stage.tinv_invariance;
  status[0][11]=final_stage.terminal_invariant?1.0:0.0;
  status[0][12]=run.bookkeeping_valid?1.0:0.0;
  final_stage.matrices["KRYLOV_STATUS"]=std::move(status);
  run.stages.push_back(std::move(final_stage));
  return true;
}

struct TangentPairingAudit {
  double residual = INFINITY;
  double separation = -INFINITY;
  bool products_positive = false;
};

TangentPairingAudit tangent_conjugate_pairing(
    const std::array<std::complex<double>,4>& values) {
  const std::array<std::array<std::pair<int,int>,2>,3> pairings{{
      {{{0,1},{2,3}}},{{{0,2},{1,3}}},{{{0,3},{1,2}}}}};
  std::array<std::pair<double,int>,3> scores;
  for(int p=0;p<3;++p){
    double score=0.0;
    for(const auto& [i,j]:pairings[p])
      score=std::max(score,std::abs(values[i]-std::conj(values[j]))
          /std::max({std::abs(values[i]),std::abs(values[j]),1e-30}));
    scores[p]={score,p};
  }
  std::sort(scores.begin(),scores.end());
  TangentPairingAudit result;
  result.residual=scores[0].first;
  result.separation=scores[1].first-scores[0].first;
  result.products_positive=true;
  for(const auto& [i,j]:pairings[scores[0].second]){
    const auto product=values[i]*values[j];
    result.products_positive=result.products_positive&&product.real()>0.0
        &&std::abs(product.imag())
            <=1e-8*std::max(std::abs(product),1e-30);
  }
  return result;
}

template<class Chart>
std::optional<TangentCandidate> tangent_build_candidate(
    const Chart& chart,const TangentMetric& metric,
    const ftd::eft::ConnectedMooreBlockOptions& options,
    const TangentKrylovRun& run,const TangentProjectedStage& stage,
    const TangentCluster& cluster,OperatorAuditLedger& ledger) {
  if(!cluster.eligible||cluster.indices.size()!=4)return std::nullopt;
  const auto symmetric=tangent_average(stage.matrices.at("A_S"),
      ftd0774::transpose(stage.matrices.at("A_S")));
  const auto eig=ftd0774::symmetric_eigen(symmetric);
  if(!eig.valid){
    ledger.record_numeric_failure(
        {run.construction,stage.stage,"candidate_symmetric_eigensolver",
         std::nullopt,std::nullopt,run.h,true},eig.residual,
        "invalid_symmetric_eigensolver",NAN,false);
    return std::nullopt;
  }
  SmallMatrix coefficients(stage.dimension,std::vector<double>(4,0.0));
  for(int column=0;column<4;++column)
    for(int row=0;row<stage.dimension;++row)
      coefficients[row][column]=eig.vectors[row][cluster.indices[column]];
  TangentCandidate result;
  result.candidate_id=cluster.candidate_id;
  result.construction=run.construction;
  result.stage=stage.stage;
  result.dimension=stage.dimension;
  result.cluster_id=cluster.cluster_id;
  result.mus=cluster.mus;
  result.mu_min=*std::min_element(result.mus.begin(),result.mus.end());
  result.mu_max=*std::max_element(result.mus.begin(),result.mus.end());
  result.u=tangent_linear_combination(stage.basis,coefficients);
  if(result.u.size()!=4){
    ledger.record_numeric_failure(
        {run.construction,stage.stage,"candidate_dimension",std::nullopt,
         std::nullopt,run.h,true},static_cast<double>(result.u.size()),
        "invalid_candidate_dimension");
    return std::nullopt;
  }

  std::array<TangentImages,4> images;
  std::vector<std::future<TangentImages>> futures;
  for(int column=0;column<4;++column)
    futures.push_back(std::async(std::launch::async,[&,column]() {
      OperatorAuditKey key{run.construction,stage.stage,
          "candidate_image:"+cluster.candidate_id,std::nullopt,column,
          run.h,true};
      return audited_images(chart,result.u[column],run.h,options,ledger,key);
    }));
  for(int column=0;column<4;++column)images[column]=futures[column].get();
  if(ledger.failed()||std::any_of(images.begin(),images.end(),
      [](const TangentImages& image){return !image.valid;}))return std::nullopt;
  for(const auto& image:images){
    result.tu.push_back(image.t);
    result.tinvu.push_back(image.tinv);
    result.su.push_back(image.s);
  }

  struct CompositionPair {bool valid=false;ChartVector tinv_t;ChartVector t_tinv;};
  std::array<CompositionPair,4> compositions;
  std::vector<std::future<CompositionPair>> composition_futures;
  for(int column=0;column<4;++column)
    composition_futures.push_back(std::async(std::launch::async,[&,column]() {
      CompositionPair pair;
      OperatorAuditKey key{run.construction,stage.stage,
          "candidate_composition:"+cluster.candidate_id,std::nullopt,column,
          run.h,false};
      const auto reverse=audited_derivative(chart,result.tu[column],run.h,
          false,options,ledger,key);
      key.forward=true;
      const auto forward=audited_derivative(chart,result.tinvu[column],run.h,
          true,options,ledger,key);
      if(reverse.valid&&forward.valid){
        pair.valid=true;
        pair.tinv_t=reverse.value;
        pair.t_tinv=forward.value;
      }
      return pair;
    }));
  for(int column=0;column<4;++column)
    compositions[column]=composition_futures[column].get();
  if(ledger.failed()||std::any_of(compositions.begin(),compositions.end(),
      [](const CompositionPair& pair){return !pair.valid;}))return std::nullopt;
  for(auto& pair:compositions){
    result.tinv_tu.push_back(std::move(pair.tinv_t));
    result.t_tinvu.push_back(std::move(pair.t_tinv));
  }

  auto& grams=result.grams;
  grams["RAW_CANDIDATE_GRAM"]=tangent_block_gram(metric,result.u,result.u);
  grams["U_K_U"]=grams["RAW_CANDIDATE_GRAM"];
  grams["U_K_B0"]=tangent_block_gram(metric,result.u,run.seeds);
  grams["U_K_TU"]=tangent_block_gram(metric,result.u,result.tu);
  grams["U_K_TINVU"]=tangent_block_gram(metric,result.u,result.tinvu);
  grams["TU_K_TU"]=tangent_block_gram(metric,result.tu,result.tu);
  grams["TINVU_K_TINVU"]=tangent_block_gram(metric,result.tinvu,result.tinvu);
  grams["U_K_TINV_TU"]=tangent_block_gram(metric,result.u,result.tinv_tu);
  grams["U_K_T_TINVU"]=tangent_block_gram(metric,result.u,result.t_tinvu);
  grams["R_T_R"]=ftd0774::multiply(
      ftd0774::transpose(grams["U_K_TU"]),grams["U_K_TU"]);
  for(const auto& [name,matrix]:grams)
    if(!tangent_matrix_finite(matrix)){
      ledger.record_numeric_failure(
          {run.construction,stage.stage,"candidate_gram",std::nullopt,
           std::nullopt,run.h,true},NAN,"nonfinite_candidate_gram");
      return std::nullopt;
    }

  result.seed_overlap=std::pow(ftd0774::frobenius(grams["U_K_B0"]),2);
  long double ritz_square=0.0L;
  for(int column=0;column<4;++column){
    auto residual=result.su[column];
    ftd0774::axpy(residual,result.u[column],-result.mus[column]);
    const double square=ftd0774::inner(metric,residual,residual);
    if(!std::isfinite(square)||square<0.0){
      ledger.record_numeric_failure(
          {run.construction,stage.stage,"candidate_ritz_norm",std::nullopt,
           column,run.h,true},square,
          "nonfinite_or_negative_candidate_norm");
      return std::nullopt;
    }
    ritz_square+=square;
  }
  const double mean_ritz_square=static_cast<double>(ritz_square/4.0L);
  if(!std::isfinite(mean_ritz_square)||mean_ritz_square<0.0){
    ledger.record_numeric_failure(
        {run.construction,stage.stage,"candidate_ritz_total",std::nullopt,
         std::nullopt,run.h,true},mean_ritz_square,
        "nonfinite_or_negative_candidate_total_norm");
    return std::nullopt;
  }
  result.ritz_residual=std::sqrt(mean_ritz_square);
  const auto projected_t=tangent_linear_combination(
      result.u,grams["U_K_TU"]);
  const auto projected_tinv=tangent_linear_combination(
      result.u,grams["U_K_TINVU"]);
  const double t_residual_norm=tangent_block_k_norm(metric,
      tangent_block_difference(result.tu,projected_t));
  const double tinv_residual_norm=tangent_block_k_norm(metric,
      tangent_block_difference(result.tinvu,projected_tinv));
  const double tinv_t_norm=tangent_block_k_norm(metric,
      tangent_block_difference(result.tinv_tu,result.u));
  const double t_tinv_norm=tangent_block_k_norm(metric,
      tangent_block_difference(result.t_tinvu,result.u));
  const double tu_norm=tangent_block_k_norm(metric,result.tu);
  const double tinvu_norm=tangent_block_k_norm(metric,result.tinvu);
  const double u_norm=tangent_block_k_norm(metric,result.u);
  if(!std::isfinite(t_residual_norm)||!std::isfinite(tinv_residual_norm)
      ||!std::isfinite(tinv_t_norm)||!std::isfinite(t_tinv_norm)
      ||!std::isfinite(tu_norm)||!std::isfinite(tinvu_norm)
      ||!std::isfinite(u_norm)||tu_norm<=0.0||tinvu_norm<=0.0
      ||u_norm<=0.0){
    ledger.record_numeric_failure(
        {run.construction,stage.stage,"candidate_block_norm",std::nullopt,
         std::nullopt,run.h,true},u_norm,
        "nonfinite_negative_or_zero_candidate_block_norm",tu_norm);
    return std::nullopt;
  }
  result.t_invariance=t_residual_norm/tu_norm;
  result.tinv_invariance=tinv_residual_norm/tinvu_norm;
  result.tinv_t_residual=tinv_t_norm/u_norm;
  result.t_tinv_residual=t_tinv_norm/u_norm;
  result.adjoint_residual=ftd0774::relative_frobenius(
      grams["U_K_TINVU"],ftd0774::transpose(grams["U_K_TU"]));
  SmallMatrix orthogonality=grams["R_T_R"];
  for(int i=0;i<4;++i)orthogonality[i][i]-=1.0;
  result.orthogonality_residual=ftd0774::frobenius(orthogonality);

  const auto real_eigen=ftd0774::general_real4_eigenvalues(
      grams["U_K_TU"],1e-10);
  if(!real_eigen.valid){
    ledger.record_numeric_failure(
        {run.construction,stage.stage,"general_real4_eigensolver",
         std::nullopt,std::nullopt,run.h,true},real_eigen.residual,
        "invalid_general_real4_eigensolver",NAN,
        std::isfinite(real_eigen.residual));
    return std::nullopt;
  }
  const auto pairing=tangent_conjugate_pairing(real_eigen.values);
  result.conjugacy_residual=pairing.residual;
  result.conjugacy_separation=pairing.separation;
  result.modulus_residual=0.0;
  std::array<double,4> phases{};
  bool imaginary=true,phase_window=true;
  for(int i=0;i<4;++i){
    const auto value=real_eigen.values[i];
    imaginary=imaginary&&std::abs(value.imag())>=1e-6;
    result.modulus_residual=std::max(result.modulus_residual,
        std::abs(std::abs(value)-1.0));
    phases[i]=std::atan2(std::abs(value.imag()),value.real());
    phase_window=phase_window
        &&std::abs(phases[i]-kInternalPhase)<=0.08;
  }
  result.phase_mean=std::accumulate(phases.begin(),phases.end(),0.0)/4.0;
  result.phase_split=*std::max_element(phases.begin(),phases.end())
      -*std::min_element(phases.begin(),phases.end());
  const auto raw_symmetric=tangent_average(grams["RAW_CANDIDATE_GRAM"],
      ftd0774::transpose(grams["RAW_CANDIDATE_GRAM"]));
  const auto raw_eigen=ftd0774::symmetric_eigen(raw_symmetric);
  if(!raw_eigen.valid||raw_eigen.values.size()!=4){
    ledger.record_numeric_failure(
        {run.construction,stage.stage,"candidate_gram_eigensolver",
         std::nullopt,std::nullopt,run.h,true},raw_eigen.residual,
        "invalid_symmetric_eigensolver",NAN,false);
    return std::nullopt;
  }
  result.gram_min=raw_eigen.values.front();
  result.gram_max=raw_eigen.values.back();
  result.gram_ratio=std::isfinite(result.gram_max)&&result.gram_max>0.0
      ?result.gram_min/result.gram_max:NAN;
  SmallMatrix u_identity=grams["U_K_U"];
  for(int i=0;i<4;++i)u_identity[i][i]-=1.0;
  const double raw_symmetry=ftd0774::relative_frobenius(
      grams["RAW_CANDIDATE_GRAM"],
      ftd0774::transpose(grams["RAW_CANDIDATE_GRAM"]));
  const bool finite_metrics=std::isfinite(result.ritz_residual)
      &&std::isfinite(result.t_invariance)
      &&std::isfinite(result.tinv_invariance)
      &&std::isfinite(result.tinv_t_residual)
      &&std::isfinite(result.t_tinv_residual)
      &&std::isfinite(result.adjoint_residual)
      &&std::isfinite(result.orthogonality_residual)
      &&std::isfinite(result.modulus_residual)
      &&std::isfinite(result.conjugacy_residual)
      &&std::isfinite(result.conjugacy_separation)
      &&std::isfinite(result.phase_mean)&&std::isfinite(result.phase_split)
      &&std::isfinite(result.gram_ratio);
  if(!finite_metrics){
    ledger.record_numeric_failure(
        {run.construction,stage.stage,"candidate_metric",std::nullopt,
         std::nullopt,run.h,true},NAN,"nonfinite_candidate_metric");
    return std::nullopt;
  }
  result.core_qualified=ftd0774::frobenius(u_identity)<=1e-10
      &&raw_symmetry<=1e-10
      &&std::abs(result.seed_overlap-cluster.seed_overlap)<=2e-8
      &&result.ritz_residual<=2e-4
      &&result.t_invariance<=2e-4&&result.tinv_invariance<=2e-4
      &&result.tinv_t_residual<=1e-4&&result.t_tinv_residual<=1e-4
      &&result.adjoint_residual<=2e-4
      &&result.orthogonality_residual<=2e-4
      &&real_eigen.valid&&imaginary
      &&result.conjugacy_residual<=1e-8
      &&result.conjugacy_separation>1e-10&&pairing.products_positive
      &&result.modulus_residual<=2e-4&&phase_window
      &&result.phase_split<=1e-4&&result.seed_overlap>=0.10
      &&result.gram_min>0.0&&result.gram_max>0.0
      &&result.gram_ratio>=1e-6;
  return result;
}

struct TangentMatch {
  int index = -1;
  double overlap = -INFINITY;
  double angle = INFINITY;
  bool valid = false;
};

TangentMatch tangent_match_candidate(
    const TangentMetric& metric,const std::vector<TangentCandidate>& candidates,
    const TangentCandidate& source,const std::string& construction,
    const std::string& stage,double tolerance) {
  struct Score{double overlap;int index;double angle;};
  std::vector<Score> scores;
  for(std::size_t i=0;i<candidates.size();++i){
    const auto& target=candidates[i];
    if(target.candidate_id==source.candidate_id
        &&target.construction==source.construction&&target.stage==source.stage)
      continue;
    if(target.construction!=construction||target.stage!=stage)continue;
    const auto cross=tangent_block_gram(metric,source.u,target.u);
    scores.push_back({std::pow(ftd0774::frobenius(cross),2),
        static_cast<int>(i),ftd0774::principal_sine(cross)});
  }
  std::sort(scores.begin(),scores.end(),[](const Score& left,const Score& right){
    if(left.overlap!=right.overlap)return left.overlap>right.overlap;
    return left.index<right.index;
  });
  TangentMatch result;
  if(scores.empty())return result;
  result.index=scores.front().index;
  result.overlap=scores.front().overlap;
  result.angle=scores.front().angle;
  const bool unique=scores.size()==1
      ||scores[0].overlap-scores[1].overlap>1e-8;
  result.valid=result.overlap>3.9&&unique&&result.angle<=tolerance;
  return result;
}

TangentProjectedStage* tangent_find_stage(TangentKrylovRun& run,
                                          const std::string& stage) {
  for(auto& value:run.stages)if(value.stage==stage)return&value;
  return nullptr;
}

const TangentKrylovRun* tangent_find_run(
    const std::vector<TangentKrylovRun>& runs,const std::string& construction) {
  for(const auto& run:runs)if(run.construction==construction)return&run;
  return nullptr;
}

template<class Chart>
TangentPipeline tangent_run_pipeline(
    const Chart& chart,const TangentMetric& metric,
    const ftd::eft::ConnectedMooreBlockOptions& options,
    const std::vector<ChartVector>& primary_seeds,
    OperatorAuditLedger& ledger) {
  TangentPipeline pipeline;
  std::vector<ChartVector> sign_seeds;
  for(const auto& seed:primary_seeds)sign_seeds.push_back(ftd0774::scaled(seed,-1.0));
  std::vector<ChartVector> rotation_seeds(4);
  const double inverse_sqrt_two=1.0/std::sqrt(2.0);
  for(int pair=0;pair<2;++pair){
    const int a=2*pair,b=a+1;
    rotation_seeds[a]=ftd0774::scaled(primary_seeds[a],inverse_sqrt_two);
    ftd0774::axpy(rotation_seeds[a],primary_seeds[b],inverse_sqrt_two);
    rotation_seeds[b]=ftd0774::scaled(primary_seeds[a],-inverse_sqrt_two);
    ftd0774::axpy(rotation_seeds[b],primary_seeds[b],inverse_sqrt_two);
  }
  const std::array<std::tuple<std::string,double,const std::vector<ChartVector>*>,4>
      specifications{{
          {"primary",kH0,&primary_seeds},{"h1",kH1,&primary_seeds},
          {"sign",kH0,&sign_seeds},{"rotation",kH0,&rotation_seeds}}};
  for(const auto& [name,h,seeds]:specifications){
    auto run=tangent_build_krylov(
        chart,metric,options,name,h,*seeds,ledger);
    if(!run.execution_valid||!tangent_project_run(
          chart,metric,options,run,ledger)){
      pipeline.runs.push_back(std::move(run));
      return pipeline;
    }
    pipeline.runs.push_back(std::move(run));
  }
  for(auto& run:pipeline.runs)
    for(auto& stage:run.stages){
      for(const auto& cluster:stage.clusters){
        pipeline.seeded_rank_gt_four=pipeline.seeded_rank_gt_four
            ||(cluster.in_window&&cluster.seed_linked&&cluster.indices.size()>4);
        if(!cluster.eligible)continue;
        auto candidate=tangent_build_candidate(
            chart,metric,options,run,stage,cluster,ledger);
        if(!candidate.has_value()||ledger.failed())return pipeline;
        pipeline.candidates.push_back(std::move(*candidate));
      }
    }

  std::map<int,bool> prior_match_ok;
  std::map<int,bool> prior_core_ok;
  for(std::size_t index=0;index<pipeline.candidates.size();++index){
    auto& candidate=pipeline.candidates[index];
    if(candidate.stage!="final")continue;
    const auto match=tangent_match_candidate(metric,pipeline.candidates,
        candidate,candidate.construction,"prior",1e-3);
    if(match.index>=0&&(!std::isfinite(match.overlap)
                       ||!std::isfinite(match.angle))){
      ledger.record_numeric_failure(
          {candidate.construction,"final","candidate_match",std::nullopt,
           std::nullopt,candidate.construction=="h1"?kH1:kH0,true},
          match.angle,"nonfinite_candidate_match");
      return pipeline;
    }
    bool pass=match.valid;
    bool core=match.valid&&match.index>=0
        &&pipeline.candidates[match.index].core_qualified;
    if(candidate.dimension==4&&match.index<0){
      pass=true;core=true;candidate.prior_angle=INFINITY;
    }
    else if(match.index>=0){
      candidate.prior_angle=match.angle;
      candidate.matched_prior=pipeline.candidates[match.index].candidate_id;
      candidate.grams["U_K_PRIOR"]=tangent_block_gram(
          metric,candidate.u,pipeline.candidates[match.index].u);
    }
    prior_match_ok[static_cast<int>(index)]=pass;
    prior_core_ok[static_cast<int>(index)]=core;
  }

  bool any_resolved=false;
  for(std::size_t index=0;index<pipeline.candidates.size();++index){
    auto& candidate=pipeline.candidates[index];
    if(candidate.construction!="primary"||candidate.stage!="final")continue;
    const auto h1=tangent_match_candidate(
        metric,pipeline.candidates,candidate,"h1","final",1e-2);
    const auto sign=tangent_match_candidate(
        metric,pipeline.candidates,candidate,"sign","final",1e-6);
    const auto rotation=tangent_match_candidate(
        metric,pipeline.candidates,candidate,"rotation","final",1e-6);
    if((h1.index>=0&&(!std::isfinite(h1.overlap)||!std::isfinite(h1.angle)))
        ||(sign.index>=0&&(!std::isfinite(sign.overlap)||!std::isfinite(sign.angle)))
        ||(rotation.index>=0&&(!std::isfinite(rotation.overlap)
                              ||!std::isfinite(rotation.angle)))){
      ledger.record_numeric_failure(
          {"primary","final","candidate_match",std::nullopt,std::nullopt,
           kH0,true},NAN,"nonfinite_candidate_match");
      return pipeline;
    }
    candidate.h1_angle=h1.angle;
    candidate.sign_angle=sign.angle;
    candidate.rotation_angle=rotation.angle;
    if(h1.index>=0){
      const auto& matched=pipeline.candidates[h1.index];
      candidate.matched_h1=matched.candidate_id;
      candidate.grams["U_K_H1"]=tangent_block_gram(metric,candidate.u,matched.u);
      const auto r=candidate.grams["U_K_TU"];
      const auto r_h1=matched.grams.at("U_K_TU");
      const auto c=candidate.grams["U_K_H1"];
      candidate.grams["C_H1"]=c;
      candidate.grams["R_H1"]=r_h1;
      const auto left=ftd0774::multiply(r,c);
      const auto right=ftd0774::multiply(c,r_h1);
      SmallMatrix difference=left;
      for(std::size_t row=0;row<difference.size();++row)
        for(std::size_t column=0;column<difference[row].size();++column)
          difference[row][column]-=right[row][column];
      candidate.intertwining_residual=ftd0774::frobenius(difference)
          /std::max(ftd0774::frobenius(r),1e-30);
      if(!std::isfinite(candidate.intertwining_residual)){
        ledger.record_numeric_failure(
            {"primary","final","intertwining",std::nullopt,std::nullopt,
             kH0,true},candidate.intertwining_residual,
            "nonfinite_intertwining_residual");
        return pipeline;
      }
    }
    if(sign.index>=0){
      const auto& matched=pipeline.candidates[sign.index];
      candidate.matched_sign=matched.candidate_id;
      candidate.grams["U_K_SIGN"]=tangent_block_gram(
          metric,candidate.u,matched.u);
    }
    if(rotation.index>=0){
      const auto& matched=pipeline.candidates[rotation.index];
      candidate.matched_rotation=matched.candidate_id;
      candidate.grams["U_K_ROT45"]=tangent_block_gram(
          metric,candidate.u,matched.u);
    }
    const bool h1_prior=h1.index>=0
        &&prior_match_ok.count(h1.index)&&prior_match_ok[h1.index];
    const bool sign_prior=sign.index>=0
        &&prior_match_ok.count(sign.index)&&prior_match_ok[sign.index];
    const bool rotation_prior=rotation.index>=0
        &&prior_match_ok.count(rotation.index)&&prior_match_ok[rotation.index];
    const bool primary_prior_core=prior_core_ok.count(static_cast<int>(index))
        &&prior_core_ok[static_cast<int>(index)];
    const bool h1_prior_core=h1.index>=0&&prior_core_ok.count(h1.index)
        &&prior_core_ok[h1.index];
    const bool sign_prior_core=sign.index>=0&&prior_core_ok.count(sign.index)
        &&prior_core_ok[sign.index];
    const bool rotation_prior_core=rotation.index>=0
        &&prior_core_ok.count(rotation.index)&&prior_core_ok[rotation.index];
    const bool resolved=prior_match_ok.count(static_cast<int>(index))
        &&prior_match_ok[static_cast<int>(index)]&&h1.valid&&h1_prior
        &&sign.valid&&sign_prior&&rotation.valid&&rotation_prior;
    any_resolved=any_resolved||resolved;
    const bool h1_core=h1.index>=0&&pipeline.candidates[h1.index].core_qualified;
    const bool sign_core=sign.index>=0
        &&pipeline.candidates[sign.index].core_qualified;
    const bool rotation_core=rotation.index>=0
        &&pipeline.candidates[rotation.index].core_qualified;
    const bool h1_phase=h1.index>=0
        &&std::abs(candidate.phase_mean
                    -pipeline.candidates[h1.index].phase_mean)<=1e-3;
    candidate.qualified=resolved&&candidate.core_qualified
        &&primary_prior_core&&h1_core&&h1_prior_core
        &&sign_core&&sign_prior_core&&rotation_core&&rotation_prior_core
        &&h1_phase
        &&std::isfinite(candidate.intertwining_residual)
        &&candidate.intertwining_residual<=1e-3;
  }
  pipeline.projected_valid=true;
  bool terminals=true;
  for(const auto& run:pipeline.runs){
    bool found_final=false;
    for(const auto& stage:run.stages){
      pipeline.projected_valid=pipeline.projected_valid&&stage.valid;
      if(stage.stage=="final"){
        found_final=true;
        terminals=terminals&&stage.terminal_invariant;
      }
    }
    terminals=terminals&&found_final;
  }
  pipeline.execution_valid=!ledger.failed()
      &&std::all_of(pipeline.runs.begin(),pipeline.runs.end(),
          [](const TangentKrylovRun& run){return run.execution_valid;});
  pipeline.solve_resolved=pipeline.projected_valid&&terminals
      &&!pipeline.seeded_rank_gt_four&&any_resolved;
  return pipeline;
}

bool write_tangent_pipeline_artifacts(const std::filesystem::path& stem,
                                      const TangentPipeline& pipeline) {
  std::ofstream projected(stem.string()+"_projected_matrices.csv",std::ios::app);
  std::ofstream clusters(stem.string()+"_clusters.csv",std::ios::app);
  std::ofstream metrics(stem.string()+"_candidate_metrics.csv",std::ios::app);
  std::ofstream index(stem.string()+"_candidate_vectors_index.csv",std::ios::app);
  std::ofstream grams(stem.string()+"_gram_blocks.csv",std::ios::app);
  std::ofstream krylov(stem.string()+"_krylov_status.csv");
  std::ofstream binary(stem.string()+"_candidate_vectors.bin",
                       std::ios::binary|std::ios::trunc);
  if(!projected||!clusters||!metrics||!index||!grams||!krylov||!binary)
    return false;
  projected<<std::setprecision(17);
  clusters<<std::setprecision(17);
  metrics<<std::setprecision(17);
  krylov<<kKrylovStatusHeader<<'\n'<<std::setprecision(17);
  for(const auto& run:pipeline.runs){
    const TangentProjectedStage* final_stage=nullptr;
    for(const auto& stage:run.stages)
      if(stage.stage=="final")final_stage=&stage;
    krylov<<run.construction<<','<<run.generated_power_count<<','
        <<run.basis.size()<<','<<run.prior_dimension<<','
        <<run.last_nonempty_power<<','<<run.last_nonempty_start<<','
        <<run.last_nonempty_end<<','<<run.deflation_count<<','
        <<run.happy_breakdown<<','<<run.exhausted<<','
        <<run.bookkeeping_valid<<','<<(final_stage!=nullptr)<<',';
    if(final_stage){
      csv_number(krylov,final_stage->t_invariance);krylov<<',';
      csv_number(krylov,final_stage->tinv_invariance);krylov<<','
          <<final_stage->terminal_invariant;
    }else{
      krylov<<",,0";
    }
    krylov<<'\n';
    for(const auto& stage:run.stages){
      for(const auto& [name,matrix]:stage.matrices)
        for(std::size_t row=0;row<matrix.size();++row)
          for(std::size_t column=0;column<matrix[row].size();++column){
            if(!std::isfinite(matrix[row][column]))return false;
            projected<<run.construction<<','<<stage.stage<<','<<stage.dimension
                <<','<<name<<','<<row<<','<<column<<','
                <<matrix[row][column]<<'\n';
          }
      for(const auto& cluster:stage.clusters)
        for(std::size_t member=0;member<cluster.indices.size();++member){
          if(!std::isfinite(cluster.mus[member])
              ||!std::isfinite(cluster.phases[member])
              ||!std::isfinite(cluster.seed_overlap))return false;
          clusters<<run.construction<<','<<stage.stage<<','
              <<cluster.cluster_id<<','<<cluster.indices.size()<<','
              <<cluster.indices[member]<<','<<cluster.mus[member]<<','
              <<cluster.phases[member]<<','<<cluster.seed_overlap<<','
              <<cluster.seed_linked<<','<<cluster.eligible<<','
              <<cluster.in_window<<','<<cluster.candidate_id<<'\n';
        }
    }
  }
  std::uintmax_t offset=0;
  const std::uint16_t endian_probe=1;
  if(*reinterpret_cast<const unsigned char*>(&endian_probe)!=1)return false;
  for(const auto& candidate:pipeline.candidates){
    const std::array<std::pair<const char*,const std::vector<ChartVector>*>,6>
        vector_blocks{{{"U",&candidate.u},{"SU",&candidate.su},
          {"TU",&candidate.tu},{"TINVU",&candidate.tinvu},
          {"TINV_TU",&candidate.tinv_tu},{"T_TINVU",&candidate.t_tinvu}}};
    for(const auto& [kind,block]:vector_blocks){
      if(block->size()!=4)return false;
      for(int column=0;column<4;++column){
        const auto flat=ftd0774::flatten((*block)[column]);
        if(flat.size()!=ftd0774::kRawChartDimension
            ||!std::all_of(flat.begin(),flat.end(),
                [](double value){return std::isfinite(value);}))return false;
        const auto length=flat.size()*sizeof(double);
        binary.write(reinterpret_cast<const char*>(flat.data()),
            static_cast<std::streamsize>(length));
        if(!binary)return false;
        index<<candidate.candidate_id<<','<<candidate.construction<<','
            <<candidate.stage<<','<<kind<<','<<column<<','
            <<ftd0774::kRawChartDimension<<','<<offset<<','<<length<<'\n';
        offset+=length;
      }
    }
    for(const auto& [name,matrix]:candidate.grams)
      write_matrix_block(grams,candidate.candidate_id,candidate.construction,
          candidate.stage,name,matrix);
    metrics<<candidate.candidate_id<<','<<candidate.construction<<','
        <<candidate.stage<<','<<candidate.dimension<<','
        <<candidate.cluster_id<<",4,";
    const std::array<double,23> values{{candidate.mu_min,candidate.mu_max,
        candidate.phase_mean,candidate.phase_split,candidate.seed_overlap,
        candidate.ritz_residual,candidate.prior_angle,candidate.h1_angle,
        candidate.sign_angle,candidate.rotation_angle,candidate.t_invariance,
        candidate.tinv_invariance,candidate.tinv_t_residual,
        candidate.t_tinv_residual,candidate.adjoint_residual,
        candidate.orthogonality_residual,candidate.modulus_residual,
        candidate.conjugacy_residual,candidate.conjugacy_separation,
        candidate.intertwining_residual,candidate.gram_min,candidate.gram_max,
        candidate.gram_ratio}};
    for(double value:values){csv_number(metrics,value);metrics<<',';}
    metrics<<candidate.qualified<<','
        <<(candidate.core_qualified?"core_pass":"core_gate")<<'\n';
  }
  projected.flush();clusters.flush();metrics.flush();index.flush();grams.flush();
  krylov.flush();
  binary.flush();
  return static_cast<bool>(projected)&&static_cast<bool>(clusters)
      &&static_cast<bool>(metrics)&&static_cast<bool>(index)
      &&static_cast<bool>(grams)&&static_cast<bool>(krylov)
      &&static_cast<bool>(binary);
}

void invalidate_postflight_summary(TangentSummary& summary) {
  summary.artifact_schema=false;
  summary.krylov_executed=false;
  summary.krylov_resolved=false;
  summary.eligible_candidates=0;
  summary.qualified_candidates=0;
  summary.selected_candidate.clear();
  summary.dimensions.clear();
  summary.artifact_hashes.clear();
  summary.verdict="L17_COMPLETE_TANGENT_EXECUTION_INVALID";
  summary.companion.clear();
}

bool reset_postflight_artifacts(const std::filesystem::path& stem) {
  std::vector<std::string> retained_grams;
  {
    std::ifstream input(stem.string()+"_gram_blocks.csv");
    std::string line;
    if(!input||!std::getline(input,line)||line!=kGramHeader)return false;
    const std::set<std::string> allowed{{"PROBE_K_PROBE","PROBE_K_T_H0",
        "TINV_H0_K_PROBE","PROBE_K_T_H1","TINV_H1_K_PROBE"}};
    while(std::getline(input,line)){
      if(line.empty())continue;
      const auto fields=split_csv(line);
      if(fields.size()==7&&fields[1]=="preflight"&&fields[2]=="final"
          &&allowed.count(fields[3])==1)
        retained_grams.push_back(line);
    }
  }
  if(retained_grams.size()!=5U*16U*16U)return false;
  bool valid=true;
  valid=write_header_only(stem.string()+"_projected_matrices.csv",
      kProjectedHeader)&&valid;
  valid=write_header_only(stem.string()+"_clusters.csv",kClustersHeader)
      &&valid;
  valid=write_header_only(stem.string()+"_candidate_metrics.csv",
      kCandidateMetricsHeader)&&valid;
  valid=write_header_only(stem.string()+"_candidate_vectors_index.csv",
      kCandidateIndexHeader)&&valid;
  valid=write_header_only(stem.string()+"_krylov_status.csv",
      kKrylovStatusHeader)&&valid;
  {
    std::ofstream binary(stem.string()+"_candidate_vectors.bin",
        std::ios::binary|std::ios::trunc);
    valid=static_cast<bool>(binary)&&valid;
  }
  {
    std::ofstream grams(stem.string()+"_gram_blocks.csv",std::ios::trunc);
    if(!grams)valid=false;
    else{
      grams<<kGramHeader<<'\n';
      for(const auto& row:retained_grams)grams<<row<<'\n';
      grams.flush();
      valid=static_cast<bool>(grams)&&valid;
    }
  }
  return valid;
}

bool abort_postflight(const std::filesystem::path& stem,
                      TangentSummary& summary,OperatorAuditLedger& ledger,
                      const std::string& operation,double diagnostic,
                      const std::string& detail) {
  ledger.record_numeric_failure(
      {"artifact","final",operation,std::nullopt,std::nullopt,kH0,true},
      diagnostic,detail,1.0,true);
  invalidate_postflight_summary(summary);
  const bool reset=reset_postflight_artifacts(stem);
  if(!reset)
    ledger.record_numeric_failure(
        {"artifact","final","artifact_reset",std::nullopt,std::nullopt,
         kH0,true},4.0,"postflight_artifact_reset_failure",1.0,true);
  const bool status=write_execution_status(stem,ledger,false);
  return reset&&status;
}

void json_number(std::ostream& output, double value) {
  if (std::isfinite(value)) output << std::setprecision(17) << value;
  else output << "null";
}

std::uintmax_t file_bytes(const std::filesystem::path& path) {
  std::error_code error;
  const auto value = std::filesystem::file_size(path, error);
  return error ? 0 : value;
}

std::vector<std::pair<std::string, std::filesystem::path>> primitive_files(
    const std::filesystem::path& stem) {
  return {
    {"preflight",stem.string()+"_preflight.csv"},
    {"hessian",stem.string()+"_hessian.csv"},
    {"projected_matrices",stem.string()+"_projected_matrices.csv"},
    {"clusters",stem.string()+"_clusters.csv"},
    {"candidate_metrics",stem.string()+"_candidate_metrics.csv"},
    {"candidate_vectors",stem.string()+"_candidate_vectors.bin"},
    {"candidate_vectors_index",stem.string()+"_candidate_vectors_index.csv"},
    {"gram_blocks",stem.string()+"_gram_blocks.csv"},
    {"execution_status",stem.string()+"_execution_status.csv"},
    {"preflight_derivative_status",
        stem.string()+"_preflight_derivative_status.csv"},
    {"runtime",stem.string()+"_runtime.csv"},
    {"energy_control",stem.string()+"_energy_control.csv"},
    {"cache_control",stem.string()+"_cache_control.csv"},
    {"field_control",stem.string()+"_field_control.csv"},
    {"krylov_status",stem.string()+"_krylov_status.csv"},
  };
}

bool write_json(const std::filesystem::path& stem, const TangentSummary& s) {
  std::ofstream out(stem.string()+".json");
  if (!out) return false;
  out << std::boolalpha << "{\n"
      << "  \"ftd_id\": \"" << kFtdId << "\",\n"
      << "  \"protocol_sha256\": \"" << kProtocolSha256 << "\",\n"
      << "  \"source_commit\": \"" << kSourceCommit << "\",\n"
      << "  \"production_changed\": false,\n"
      << "  \"verdict\": \"" << s.verdict << "\",\n"
      << "  \"companion_verdict\": ";
  if (s.companion.empty()) out << "null"; else out << '"' << s.companion << '"';
  out << ",\n  \"protocol_locked\": " << s.protocol_locked
      << ",\n  \"provenance_pass\": " << s.provenance
      << ",\n  \"source_gate_pass\": " << s.source_gate
      << ",\n  \"representative_pass\": " << s.representative
      << ",\n  \"options_pass\": " << s.options
      << ",\n  \"chart_raw_dimension\": " << ftd0774::kRawChartDimension
      << ",\n  \"chart_independent_dimension\": " << ftd0774::kIndependentChartDimension
      << ",\n  \"hessian_pass\": " << s.hessian
      << ",\n  \"gradient_pass\": " << s.gradient
      << ",\n  \"seed_metric_pass\": " << s.seed_metric
      << ",\n  \"energy_form_pass\": " << s.energy_form
      << ",\n  \"endpoint_preflight_pass\": " << s.endpoint_preflight
      << ",\n  \"regularity_pass\": " << s.regularity
      << ",\n  \"cache_control_pass\": " << s.cache_control
      << ",\n  \"field_control_pass\": " << s.field_control
      << ",\n  \"preflight_pass\": " << s.preflight
      << ",\n  \"artifact_schema_pass\": " << s.artifact_schema
      << ",\n  \"krylov_executed\": " << s.krylov_executed
      << ",\n  \"krylov_resolved\": " << s.krylov_resolved
      << ",\n  \"eligible_candidate_count\": " << s.eligible_candidates
      << ",\n  \"qualified_candidate_count\": " << s.qualified_candidates
      << ",\n  \"selected_candidate_id\": ";
  if (s.selected_candidate.empty()) out << "null";
  else out << '"' << s.selected_candidate << '"';
  out << ",\n  \"beta\": "; json_number(out,s.beta);
  out << ",\n  \"lambda\": "; json_number(out,s.lambda);
  out << ",\n  \"internal_phase\": " << std::setprecision(17)
      << kInternalPhase;
  out << ",\n  \"mu0\": " << std::setprecision(17)
      << std::cos(kInternalPhase);
  out << ",\n  \"h0\": " << std::setprecision(17) << kH0
      << ",\n  \"h1\": " << kH1 << ",\n  \"hE\": " << kHE
      << ",\n  \"hessian\": {\"antisymmetry\": ";json_number(out,s.hessian_antisymmetry);
  out << ", \"eigen_residual\": ";json_number(out,s.hessian_eigen_residual);
  out << ", \"orthogonality\": ";json_number(out,s.hessian_orthogonality);
  out << ", \"lambda_min\": ";json_number(out,s.hessian_min);
  out << ", \"lambda_max\": ";json_number(out,s.hessian_max);
  out << ", \"field_lower_bound\": ";json_number(out,s.field_lower_bound);
  out << "},\n  \"preflight_maxima\": {"
      << "\"gradient\": ";json_number(out,s.maximum_gradient);
  out << ", \"gradient_dx\": ";json_number(out,s.gradient_dx);
  out << ", \"gradient_dp\": ";json_number(out,s.gradient_dp);
  out << ", \"gradient_eT\": ";json_number(out,s.gradient_eT);
  out << ", \"gradient_b\": ";json_number(out,s.gradient_b);
  out << ", \"b0_gram\": ";json_number(out,s.b0_gram_residual);
  out << ", \"energy_slope\": ";json_number(out,s.maximum_energy_slope);
  out << ", \"energy_relative\": ";json_number(out,s.maximum_energy_relative);
  out << ", \"common_residual\": ";json_number(out,s.maximum_common);
  out << ", \"energy_drift\": ";json_number(out,s.maximum_energy_drift);
  out << ", \"recovery\": ";json_number(out,s.maximum_recovery);
  out << ", \"scale_relative\": ";json_number(out,s.maximum_scale_relative);
  out << ", \"composition\": ";json_number(out,s.maximum_composition);
  out << ", \"adjoint\": ";json_number(out,s.maximum_adjoint);
  out << ", \"minimum_sigma\": ";json_number(out,s.minimum_sigma);
  out << ", \"maximum_condition\": ";json_number(out,s.maximum_condition);
  out << ", \"regularity_scale\": ";json_number(out,s.maximum_regularity_scale);
  out << ", \"observer_regression\": ";json_number(out,s.maximum_observer_regression);
  out << ", \"codec_divergence\": ";json_number(out,s.maximum_codec_divergence);
  out << ", \"hodge_correction\": ";json_number(out,s.maximum_hodge_correction);
  out << ", \"reconstruction\": ";json_number(out,s.maximum_reconstruction);
  out << ", \"harmonic\": ";json_number(out,s.maximum_harmonic);
  out << ", \"field_phase_relative\": ";json_number(out,s.field_phase_relative);
  out << ", \"field_recurrence\": ";json_number(out,s.field_recurrence);
  out << ", \"field_recovery\": ";json_number(out,s.field_recovery);
  out << "},\n  \"construction_dimensions\": {";
  const std::array<std::string,6> dimension_names{{"primary_prior","primary_final",
      "h1_prior","h1_final","sign_final","rotation_final"}};
  for (std::size_t i=0;i<dimension_names.size();++i) {
    if (i) out << ", ";
    out << '"' << dimension_names[i] << "\": ";
    const auto found=s.dimensions.find(dimension_names[i]);
    if(found==s.dimensions.end())out<<"null";else out<<found->second;
  }
  out << "},\n  \"artifact_sha256\": {";
  bool first=true;for(const auto& entry:s.artifact_hashes){if(!first)out<<", ";first=false;out<<'"'<<entry.first<<"\": \""<<entry.second<<'"';}
  out << "}\n}\n";
  out.flush();
  return static_cast<bool>(out);
}

std::optional<std::size_t> validated_row_count(
    const std::filesystem::path& path,const std::string& header) {
  std::ifstream input(path,std::ios::binary);
  if (!input) return std::nullopt;
  std::string line;
  if (!std::getline(input,line)) return std::nullopt;
  if (!line.empty() && line.back()=='\r') line.pop_back();
  if (line!=header) return std::nullopt;
  const std::size_t field_count=1+static_cast<std::size_t>(
      std::count(header.begin(),header.end(),','));
  std::size_t rows=0;
  while (std::getline(input,line)) if (!line.empty()) {
    if (!line.empty() && line.back()=='\r') line.pop_back();
    if (1+static_cast<std::size_t>(std::count(line.begin(),line.end(),','))
        !=field_count) return std::nullopt;
    ++rows;
  }
  return rows;
}

bool header_and_minimum_rows(const std::filesystem::path& path,
                             const std::string& header,
                             std::size_t minimum_rows) {
  const auto rows=validated_row_count(path,header);
  return rows.has_value()&&*rows>=minimum_rows;
}

bool header_and_allowed_rows(const std::filesystem::path& path,
                             const std::string& header,
                             const std::set<std::size_t>& allowed) {
  const auto rows=validated_row_count(path,header);
  return rows.has_value()&&allowed.count(*rows)==1;
}

bool file_contains(const std::filesystem::path& path,
                   const std::string& token) {
  std::ifstream input(path,std::ios::binary);
  if (!input) return false;
  const std::string bytes((std::istreambuf_iterator<char>(input)),{});
  return bytes.find(token)!=std::string::npos;
}

bool parse_exact_int(const std::string& text,int& value) {
  if(text.empty())return false;
  try{
    std::size_t consumed=0;
    const long long parsed=std::stoll(text,&consumed);
    if(consumed!=text.size()||parsed<std::numeric_limits<int>::min()
        ||parsed>std::numeric_limits<int>::max())return false;
    value=static_cast<int>(parsed);
    return true;
  }catch(...){return false;}
}

bool parse_finite_double(const std::string& text,double& value) {
  if(text.empty())return false;
  try{
    std::size_t consumed=0;
    value=std::stod(text,&consumed);
    return consumed==text.size()&&std::isfinite(value);
  }catch(...){return false;}
}

bool parse_binary(const std::string& text,bool& value) {
  if(text=="0"){value=false;return true;}
  if(text=="1"){value=true;return true;}
  return false;
}

bool validate_status_payload(const std::filesystem::path& path,
                             const std::string& terminal_construction,
                             const std::string& terminal_operation,
                             bool terminal_valid,
                             std::optional<int> expected_groups=std::nullopt) {
  std::ifstream input(path);
  std::string line;
  if(!input||!std::getline(input,line)||line!=kExecutionStatusHeader)
    return false;
  struct Group {int retractions=0;int endpoints=0;int codecs=0;int numeric=0;};
  std::map<int,Group> groups;
  int terminal_id=-1;
  int terminal_count=0;
  while(std::getline(input,line)){
    if(line.empty())continue;
    const auto fields=split_csv(line);
    if(fields.size()!=31)return false;
    int id=-1;
    if(!parse_exact_int(fields[0],id)||id<0)return false;
    if(fields[1]=="terminal"){
      bool valid=false;
      if(!parse_binary(fields[10],valid)||valid!=terminal_valid
          ||fields[2]!=terminal_construction||fields[3]!="final"
          ||fields[4]!=terminal_operation)return false;
      terminal_id=id;
      ++terminal_count;
      continue;
    }
    auto& group=groups[id];
    if(fields[1]=="retraction")++group.retractions;
    else if(fields[1]=="endpoint")++group.endpoints;
    else if(fields[1]=="codec")++group.codecs;
    else if(fields[1]=="numeric")++group.numeric;
    else return false;
  }
  if(terminal_count!=1||terminal_id!=static_cast<int>(groups.size()))
    return false;
  int expected_id=0;
  for(const auto& [id,group]:groups){
    if(id!=expected_id++)return false;
    const bool derivative=group.retractions==2&&group.endpoints==2
        &&group.codecs==1&&group.numeric==0;
    const bool numeric=group.numeric==1&&group.retractions==0
        &&group.endpoints==0&&group.codecs==0;
    if(!derivative&&!numeric)return false;
  }
  return !expected_groups.has_value()
      ||static_cast<int>(groups.size())==*expected_groups;
}

bool validate_projected_payload(const std::filesystem::path& stem) {
  std::ifstream input(stem.string()+"_projected_matrices.csv");
  std::string line;
  if(!input||!std::getline(input,line)||line!=kProjectedHeader)return false;
  struct Block {int dimension=0;int rows=0;int columns=0;
    std::set<std::pair<int,int>> cells;};
  std::map<std::string,Block> blocks;
  std::map<std::string,int> stages;
  const std::set<std::string> square_names{{"V_K_V","A_S","A_T",
      "A_TINV","TV_K_TV","TINV_V_K_TINV_V",
      "T_RESIDUAL_K_T_RESIDUAL","TINV_RESIDUAL_K_TINV_RESIDUAL"}};
  while(std::getline(input,line)){
    if(line.empty())continue;
    const auto fields=split_csv(line);
    if(fields.size()!=7)return false;
    int dimension=0,row=0,column=0;
    double value=NAN;
    if(!parse_exact_int(fields[2],dimension)||dimension<=0||dimension>64
        ||!parse_exact_int(fields[4],row)||!parse_exact_int(fields[5],column)
        ||row<0||column<0||!parse_finite_double(fields[6],value))return false;
    if(fields[0].empty()||(fields[1]!="prior"&&fields[1]!="final"))
      return false;
    int expected_rows=0,expected_columns=0;
    if(square_names.count(fields[3])){
      expected_rows=dimension;expected_columns=dimension;
    }else if(fields[3]=="SEED"){
      expected_rows=dimension;expected_columns=4;
    }else if(fields[3]=="KRYLOV_STATUS"){
      if(fields[1]!="final")return false;
      expected_rows=1;expected_columns=13;
    }else return false;
    if(row>=expected_rows||column>=expected_columns)return false;
    const std::string stage_key=fields[0]+"|"+fields[1];
    if(stages.count(stage_key)&&stages[stage_key]!=dimension)return false;
    stages[stage_key]=dimension;
    const std::string key=stage_key+"|"+fields[3];
    auto& block=blocks[key];
    if(block.dimension!=0&&block.dimension!=dimension)return false;
    block.dimension=dimension;block.rows=expected_rows;
    block.columns=expected_columns;
    if(!block.cells.insert({row,column}).second)return false;
  }
  for(const auto& [key,block]:blocks)
    if(block.cells.size()!=static_cast<std::size_t>(block.rows*block.columns))
      return false;
  for(const auto& [stage_key,dimension]:stages){
    (void)dimension;
    for(const char* name:{"V_K_V","A_S","A_T","A_TINV","SEED",
        "TV_K_TV","TINV_V_K_TINV_V","T_RESIDUAL_K_T_RESIDUAL",
        "TINV_RESIDUAL_K_TINV_RESIDUAL"})
      if(blocks.count(stage_key+"|"+name)!=1)return false;
    const bool final=stage_key.size()>=6
        &&stage_key.substr(stage_key.size()-6)=="|final";
    if((blocks.count(stage_key+"|KRYLOV_STATUS")==1)!=final)return false;
  }
  return true;
}

bool validate_cluster_payload(const std::filesystem::path& stem) {
  std::ifstream input(stem.string()+"_clusters.csv");
  std::string line;
  if(!input||!std::getline(input,line)||line!=kClustersHeader)return false;
  struct Cluster {int rank=0;std::set<int> indices;bool eligible=false;
    std::string candidate;};
  std::map<std::string,Cluster> clusters;
  while(std::getline(input,line)){
    if(line.empty())continue;
    const auto fields=split_csv(line);
    if(fields.size()!=12)return false;
    int cluster_id=0,rank=0,index=0;
    double mu=NAN,phase=NAN,overlap=NAN;
    bool seed=false,eligible=false,window=false;
    if(fields[0].empty()||fields[1].empty()
        ||!parse_exact_int(fields[2],cluster_id)||cluster_id<0
        ||!parse_exact_int(fields[3],rank)||rank<=0
        ||!parse_exact_int(fields[4],index)||index<0
        ||!parse_finite_double(fields[5],mu)
        ||!parse_finite_double(fields[6],phase)
        ||!parse_finite_double(fields[7],overlap)
        ||!parse_binary(fields[8],seed)||!parse_binary(fields[9],eligible)
        ||!parse_binary(fields[10],window))return false;
    const std::string key=fields[0]+"|"+fields[1]+"|"+fields[2];
    auto& cluster=clusters[key];
    if(cluster.rank!=0&&(cluster.rank!=rank||cluster.eligible!=eligible
                         ||cluster.candidate!=fields[11]))return false;
    cluster.rank=rank;cluster.eligible=eligible;cluster.candidate=fields[11];
    if(!cluster.indices.insert(index).second
        ||(eligible&&cluster.candidate.empty())
        ||(!eligible&&!cluster.candidate.empty()))return false;
  }
  for(const auto& [key,cluster]:clusters){
    (void)key;
    if(cluster.indices.size()!=static_cast<std::size_t>(cluster.rank))
      return false;
  }
  return true;
}

bool validate_gram_payload(const std::filesystem::path& stem,
                           bool require_preflight) {
  std::ifstream input(stem.string()+"_gram_blocks.csv");
  std::string line;
  if(!input||!std::getline(input,line)||line!=kGramHeader)return false;
  struct Block {int dimension=0;std::set<std::pair<int,int>> cells;};
  std::map<std::string,Block> blocks;
  while(std::getline(input,line)){
    if(line.empty())continue;
    const auto fields=split_csv(line);
    int row=0,column=0;
    double value=NAN;
    if(fields.size()!=7||fields[1].empty()||fields[2].empty()
        ||fields[3].empty()||!parse_exact_int(fields[4],row)
        ||!parse_exact_int(fields[5],column)
        ||!parse_finite_double(fields[6],value))return false;
    const bool preflight=fields[1]=="preflight";
    const int dimension=preflight?16:4;
    if(row<0||column<0||row>=dimension||column>=dimension
        ||(preflight&&!fields[0].empty())||(!preflight&&fields[0].empty()))
      return false;
    const std::string key=fields[0]+"|"+fields[1]+"|"+fields[2]+"|"
        +fields[3];
    auto& block=blocks[key];
    if(block.dimension!=0&&block.dimension!=dimension)return false;
    block.dimension=dimension;
    if(!block.cells.insert({row,column}).second)return false;
  }
  for(const auto& [key,block]:blocks){
    (void)key;
    if(block.cells.size()!=static_cast<std::size_t>(
        block.dimension*block.dimension))return false;
  }
  const std::array<const char*,5> preflight_names{{"PROBE_K_PROBE",
      "PROBE_K_T_H0","TINV_H0_K_PROBE","PROBE_K_T_H1",
      "TINV_H1_K_PROBE"}};
  for(const char* name:preflight_names){
    const bool present=blocks.count(std::string("|preflight|final|")+name)==1;
    if(present!=require_preflight)return false;
  }
  return true;
}

bool validate_krylov_status_payload(const std::filesystem::path& stem,
                                    bool expected_complete) {
  std::ifstream input(stem.string()+"_krylov_status.csv");
  std::string line;
  if(!input||!std::getline(input,line)||line!=kKrylovStatusHeader)return false;
  std::set<std::string> constructions;
  int rows=0;
  while(std::getline(input,line)){
    if(line.empty())continue;
    const auto fields=split_csv(line);
    if(fields.size()!=15)return false;
    int generated=0,accepted=0,prior=0,last_power=0,last_start=0,last_end=0,
        deflations=0;
    bool happy=false,exhausted=false,bookkeeping=false,projected=false,
        eligible=false;
    if(!parse_exact_int(fields[1],generated)||!parse_exact_int(fields[2],accepted)
        ||!parse_exact_int(fields[3],prior)
        ||!parse_exact_int(fields[4],last_power)
        ||!parse_exact_int(fields[5],last_start)
        ||!parse_exact_int(fields[6],last_end)
        ||!parse_exact_int(fields[7],deflations)
        ||!parse_binary(fields[8],happy)||!parse_binary(fields[9],exhausted)
        ||!parse_binary(fields[10],bookkeeping)
        ||!parse_binary(fields[11],projected)
        ||!parse_binary(fields[14],eligible))return false;
    const int expected_prior=accepted==64?48:(accepted<=4?0:last_start);
    const bool structural=generated>=1&&generated<=16&&accepted>=0
        &&accepted<=64&&prior==expected_prior&&prior>=0
        &&(accepted==0?prior==0:prior<accepted)&&last_power>=-1
        &&last_power<generated&&last_start>=0&&last_start<=last_end
        &&last_end==accepted&&deflations==4*generated-accepted
        &&!(happy&&exhausted)&&(!exhausted||generated==16)
        &&(happy||exhausted||accepted==64)&&bookkeeping;
    if(!structural||!constructions.insert(fields[0]).second)return false;
    if(accepted<4){
      if(projected||eligible||!fields[12].empty()||!fields[13].empty())
        return false;
    }else{
      double t=NAN,tinv=NAN;
      if(!projected||!parse_finite_double(fields[12],t)
          ||!parse_finite_double(fields[13],tinv)
          ||eligible!=(t<=2e-4&&tinv<=2e-4))return false;
      if(!file_contains(stem.string()+"_projected_matrices.csv",
          fields[0]+",final,"))return false;
    }
    if(prior>0&&!file_contains(stem.string()+"_projected_matrices.csv",
        fields[0]+",prior,"))return false;
    ++rows;
  }
  const std::set<std::string> expected{{"primary","h1","sign","rotation"}};
  return expected_complete?rows==4&&constructions==expected:rows==0;
}

std::set<std::string> candidate_metric_keys(
    const std::filesystem::path& stem, bool& valid) {
  std::set<std::string> keys;
  std::ifstream input(stem.string()+"_candidate_metrics.csv");
  std::string line;
  valid=static_cast<bool>(input)&&std::getline(input,line)
      &&line==kCandidateMetricsHeader;
  while (valid&&std::getline(input,line)) {
    if (line.empty()) continue;
    const auto fields=split_csv(line);
    if (fields.size()!=31 || fields[0].empty()
        || fields[1].empty() || fields[2].empty()) {
      valid=false;
      break;
    }
    if (!keys.insert(fields[0]+"|"+fields[1]+"|"+fields[2]).second) {
      valid=false;
      break;
    }
  }
  return keys;
}

bool validate_vector_payload(const std::filesystem::path& stem,
                             const TangentSummary& summary) {
  const auto binary_path=std::filesystem::path(
      stem.string()+"_candidate_vectors.bin");
  const auto index_path=std::filesystem::path(
      stem.string()+"_candidate_vectors_index.csv");
  std::error_code error;
  const std::uintmax_t binary_size=std::filesystem::file_size(binary_path,error);
  if (error) return false;
  std::ifstream input(index_path);
  if (!input) return false;
  std::string line;
  if (!std::getline(input,line) || line!=kCandidateIndexHeader) return false;
  std::uintmax_t expected_offset=0;
  std::map<std::string,std::set<std::string>> records;
  const std::set<std::string> required_kinds{{
      "U","SU","TU","TINVU","TINV_TU","T_TINVU"}};
  try {
    while (std::getline(input,line)) {
      if (line.empty()) continue;
      const auto fields=split_csv(line);
      if (fields.size()!=8) return false;
      const auto dimension=static_cast<std::uintmax_t>(std::stoull(fields[5]));
      const auto offset=static_cast<std::uintmax_t>(std::stoull(fields[6]));
      const auto length=static_cast<std::uintmax_t>(std::stoull(fields[7]));
      if (dimension!=ftd0774::kRawChartDimension
          || length!=dimension*sizeof(double) || offset!=expected_offset
          || offset+length>binary_size) return false;
      const std::string key=fields[0]+"|"+fields[1]+"|"+fields[2];
      if (fields[0].empty() || fields[1].empty() || fields[2].empty()
          || required_kinds.count(fields[3])!=1) return false;
      const int column=std::stoi(fields[4]);
      if (column<0 || column>=4
          || !records[key].insert(fields[3]+"|"+fields[4]).second)
        return false;
      expected_offset+=length;
    }
  } catch (...) { return false; }
  if (expected_offset!=binary_size) return false;
  bool metrics_valid=false;
  const auto metric_keys=candidate_metric_keys(stem,metrics_valid);
  if (!metrics_valid) return false;
  if (metric_keys.size()<static_cast<std::size_t>(summary.eligible_candidates)
      || records.size()!=metric_keys.size()) return false;
  for (const auto& key : metric_keys) {
    const auto found=records.find(key);
    if (found==records.end() || found->second.size()!=24) return false;
    for (const auto& kind : required_kinds)
      for (int column=0;column<4;++column)
        if (found->second.count(kind+"|"+std::to_string(column))!=1)
          return false;
  }
  return true;
}

bool validate_primitive_schema(const std::filesystem::path& stem,
                               const TangentSummary& summary) {
  const bool detailed_preflight=file_contains(
      stem.string()+"_preflight_derivative_status.csv",
      ",terminal,preflight,final,preflight_record_complete,");
  const std::size_t hessian_rows=static_cast<std::size_t>(
      ftd0774::kMatterDimension*ftd0774::kMatterDimension
      +ftd0774::kMatterDimension);
  bool valid=header_and_allowed_rows(stem.string()+"_preflight.csv",
      kPreflightHeader,detailed_preflight?std::set<std::size_t>{215}:
          std::set<std::size_t>{0,4,22})
      && header_and_allowed_rows(stem.string()+"_hessian.csv",
          kHessianHeader,{0,hessian_rows})
      && header_and_minimum_rows(stem.string()+"_projected_matrices.csv",
          kProjectedHeader,0)
      && header_and_minimum_rows(stem.string()+"_clusters.csv",
          kClustersHeader,0)
      && header_and_minimum_rows(stem.string()+"_candidate_metrics.csv",
          kCandidateMetricsHeader,0)
      && header_and_minimum_rows(stem.string()+"_candidate_vectors_index.csv",
          kCandidateIndexHeader,0)
      && header_and_minimum_rows(stem.string()+"_gram_blocks.csv",
          kGramHeader,detailed_preflight?1280:0)
      && header_and_allowed_rows(stem.string()+"_runtime.csv",
          kRuntimeHeader,{0,60})
      && header_and_allowed_rows(stem.string()+"_energy_control.csv",
          kEnergyControlHeader,{0,32})
      && header_and_allowed_rows(stem.string()+"_cache_control.csv",
          kCacheControlHeader,{0,128})
      && header_and_allowed_rows(stem.string()+"_field_control.csv",
          kFieldControlHeader,{0,258})
      && header_and_allowed_rows(
          stem.string()+"_preflight_derivative_status.csv",
          kExecutionStatusHeader,{0,491})
      && validate_vector_payload(stem,summary)
      && validate_projected_payload(stem)
      && validate_cluster_payload(stem)
      && validate_gram_payload(stem,detailed_preflight)
      && validate_krylov_status_payload(stem,summary.krylov_executed);
  if(detailed_preflight)
    valid=valid&&validate_status_payload(
        stem.string()+"_preflight_derivative_status.csv","preflight",
        "preflight_record_complete",true,98);
  if(summary.krylov_executed)
    valid=valid&&validate_status_payload(stem.string()+"_execution_status.csv",
        "all","run_complete",true);
  else if(summary.preflight)
    valid=valid&&validate_status_payload(stem.string()+"_execution_status.csv",
        "all","run_abort",false);
  else
    valid=valid&&header_and_allowed_rows(stem.string()+"_execution_status.csv",
        kExecutionStatusHeader,{0});
  for (const auto& [name,path] : primitive_files(stem)) {
    const auto found=summary.artifact_hashes.find(name);
    valid=valid && std::filesystem::exists(path)
        && found!=summary.artifact_hashes.end() && found->second.size()==64;
  }
  if (summary.krylov_executed) {
    const auto projected=std::filesystem::path(
        stem.string()+"_projected_matrices.csv");
    const auto projected_rows=validated_row_count(projected,kProjectedHeader);
    if(projected_rows.has_value()&&*projected_rows>0)
      for (const char* name : {"V_K_V","A_S","A_T","A_TINV","SEED",
            "TV_K_TV","TINV_V_K_TINV_V","T_RESIDUAL_K_T_RESIDUAL",
            "TINV_RESIDUAL_K_TINV_RESIDUAL","KRYLOV_STATUS"})
        valid=valid&&file_contains(projected,std::string(",")+name+",");
  }
  if (summary.eligible_candidates>0) {
    const auto grams=std::filesystem::path(stem.string()+"_gram_blocks.csv");
    valid=valid&&file_contains(grams,",RAW_CANDIDATE_GRAM,")
        &&header_and_minimum_rows(stem.string()+"_candidate_metrics.csv",
            kCandidateMetricsHeader,
            static_cast<std::size_t>(summary.eligible_candidates));
  }
  if (summary.verdict!="L17_COMPLETE_TANGENT_EXECUTION_INVALID")
    valid=valid&&summary.preflight&&summary.krylov_executed;
  return valid;
}

bool write_hash_manifest(const std::filesystem::path& stem) {
  const auto hashes_path = std::filesystem::path(stem.string()+"_hashes.csv");
  std::ofstream hashes(hashes_path);
  if (!hashes) return false;
  hashes << "artifact,sha256,bytes\n";
  const auto protocol = protocol_path();
  const auto runner = engine_root()/"tests/test_l17_complete_tangent_candidate.cpp";
  const auto support = engine_root()/"tests/support/connected_moore_tangent_codec.h";
  std::vector<std::pair<std::string,std::filesystem::path>> fixed{{
      {"protocol",protocol},{"runner",runner},{"support",support},
      {"json",stem.string()+".json"}}};
#ifdef FTD_0829_CERTIFICATE_REPAIR
  fixed.push_back({"successor_wrapper",engine_root()/"tests"/
#ifdef FTD_0832_NONSINGULAR_PRODUCT_CHART
      "test_l17_complete_tangent_nonsingular_product_chart_v5.cpp"});
#elif defined(FTD_0831_REPRESENTABILITY_FLOOR)
      "test_l17_complete_tangent_representability_floor_v4.cpp"});
#elif defined(FTD_0830_HARMONIC_REINSERTION_REPAIR)
      "test_l17_complete_tangent_harmonic_reinsertion_repair_v3.cpp"});
#else
      "test_l17_complete_tangent_certificate_repair_v2.cpp"});
#endif
  fixed.push_back({"proof",repo_root()/
      "scripts/proofs/proof_l17_complete_tangent_candidate.py"});
  fixed.push_back({"proof_wrapper",repo_root()/"scripts/proofs"/
#ifdef FTD_0832_NONSINGULAR_PRODUCT_CHART
      "proof_l17_complete_tangent_nonsingular_product_chart_v5.py"});
#elif defined(FTD_0831_REPRESENTABILITY_FLOOR)
      "proof_l17_complete_tangent_representability_floor_v4.py"});
#elif defined(FTD_0830_HARMONIC_REINSERTION_REPAIR)
      "proof_l17_complete_tangent_harmonic_reinsertion_repair_v3.py"});
#else
      "proof_l17_complete_tangent_certificate_repair_v2.py"});
#endif
#endif
  for (std::size_t i = 0; i < kCompiledClosure.size(); ++i)
    fixed.push_back({"compiled_closure_"+std::to_string(i),
                     repo_root()/kCompiledClosure[i].relative});
  for(const auto& [name,path]:fixed)
    hashes<<name<<','<<ftd0774::sha256_file(path.string())<<','<<file_bytes(path)<<'\n';
  for(const auto& [name,path]:primitive_files(stem))
    hashes<<name<<','<<ftd0774::sha256_file(path.string())<<','<<file_bytes(path)<<'\n';
  hashes.flush();
  return static_cast<bool>(hashes);
}

bool validate_hash_manifest(const std::filesystem::path& stem) {
  std::ifstream input(stem.string()+"_hashes.csv");
  std::string line;
  if (!std::getline(input,line) || line!="artifact,sha256,bytes") return false;
  const auto protocol = protocol_path();
  std::map<std::string,std::filesystem::path> expected{{"protocol",protocol},
      {"runner",engine_root()/"tests/test_l17_complete_tangent_candidate.cpp"},
      {"support",engine_root()/"tests/support/connected_moore_tangent_codec.h"},
      {"json",stem.string()+".json"}};
#ifdef FTD_0829_CERTIFICATE_REPAIR
  expected["successor_wrapper"]=engine_root()/"tests"/
#ifdef FTD_0832_NONSINGULAR_PRODUCT_CHART
      "test_l17_complete_tangent_nonsingular_product_chart_v5.cpp";
#elif defined(FTD_0831_REPRESENTABILITY_FLOOR)
      "test_l17_complete_tangent_representability_floor_v4.cpp";
#elif defined(FTD_0830_HARMONIC_REINSERTION_REPAIR)
      "test_l17_complete_tangent_harmonic_reinsertion_repair_v3.cpp";
#else
      "test_l17_complete_tangent_certificate_repair_v2.cpp";
#endif
  expected["proof"]=repo_root()/
      "scripts/proofs/proof_l17_complete_tangent_candidate.py";
  expected["proof_wrapper"]=repo_root()/"scripts/proofs"/
#ifdef FTD_0832_NONSINGULAR_PRODUCT_CHART
      "proof_l17_complete_tangent_nonsingular_product_chart_v5.py";
#elif defined(FTD_0831_REPRESENTABILITY_FLOOR)
      "proof_l17_complete_tangent_representability_floor_v4.py";
#elif defined(FTD_0830_HARMONIC_REINSERTION_REPAIR)
      "proof_l17_complete_tangent_harmonic_reinsertion_repair_v3.py";
#else
      "proof_l17_complete_tangent_certificate_repair_v2.py";
#endif
#endif
  for(std::size_t i=0;i<kCompiledClosure.size();++i)
    expected["compiled_closure_"+std::to_string(i)]=
        repo_root()/kCompiledClosure[i].relative;
  for(const auto& [name,path]:primitive_files(stem))expected[name]=path;
  std::set<std::string> seen;
  while (std::getline(input,line)) {
    if (line.empty()) continue;
    const auto fields=split_csv(line);
    const auto found=fields.size()==3?expected.find(fields[0]):expected.end();
    if (found==expected.end()||fields[1].size()!=64
        ||!seen.insert(fields[0]).second
        ||fields[1]!=ftd0774::sha256_file(found->second.string()))
      return false;
    try {
      std::size_t consumed=0;
      const auto bytes=std::stoull(fields[2],&consumed);
      if(consumed!=fields[2].size()||bytes!=file_bytes(found->second))
        return false;
    } catch (...) { return false; }
  }
  return seen.size()==expected.size();
}

bool refresh_primitive_hashes(const std::filesystem::path& stem,
                              TangentSummary& summary) {
  summary.artifact_hashes.clear();
  bool valid=true;
  for (const auto& [name, path] : primitive_files(stem)) {
    const auto hash=ftd0774::sha256_file(path.string());
    summary.artifact_hashes[name]=hash;
    valid=valid&&hash.size()==64;
  }
  return valid;
}

bool write_and_validate_final_manifest(const std::filesystem::path& stem,
                                       TangentSummary& summary) {
  return write_json(stem,summary)&&write_hash_manifest(stem)
      &&validate_hash_manifest(stem);
}

bool finalize_artifacts(const std::filesystem::path& stem,
                        TangentSummary& summary,
                        OperatorAuditLedger* execution_ledger=nullptr) {
  bool hashes=refresh_primitive_hashes(stem,summary);
  summary.artifact_schema=hashes&&validate_primitive_schema(stem,summary);
  if(!summary.artifact_schema&&execution_ledger){
    const bool sanitized=abort_postflight(stem,summary,*execution_ledger,
        "artifact_schema",2.0,"postflight_artifact_schema_failure");
    hashes=refresh_primitive_hashes(stem,summary);
    summary.artifact_schema=sanitized&&hashes
        &&validate_primitive_schema(stem,summary);
  }
  if(!summary.artifact_schema){
    summary.verdict="L17_COMPLETE_TANGENT_EXECUTION_INVALID";
    summary.companion.clear();
  }

  bool valid=write_and_validate_final_manifest(stem,summary);
  if(!valid&&execution_ledger){
    const bool sanitized=abort_postflight(stem,summary,*execution_ledger,
        "artifact_manifest",3.0,"postflight_manifest_or_hash_failure");
    hashes=refresh_primitive_hashes(stem,summary);
    summary.artifact_schema=sanitized&&hashes
        &&validate_primitive_schema(stem,summary);
    valid=write_and_validate_final_manifest(stem,summary);
  }
  if(!valid){
    summary.artifact_schema=false;
    summary.verdict="L17_COMPLETE_TANGENT_EXECUTION_INVALID";
    summary.companion.clear();
    (void)write_json(stem,summary);
    (void)write_hash_manifest(stem);
  }
  return valid&&summary.artifact_schema;
}

}  // namespace

int main() {
#ifdef FTD_0832_NONSINGULAR_PRODUCT_CHART
  const auto stem = result_root()/
      "ftd_0832_l17_complete_tangent_nonsingular_product_chart_v5";
#elif defined(FTD_0831_REPRESENTABILITY_FLOOR)
  const auto stem = result_root()/
      "ftd_0831_l17_complete_tangent_representability_floor_v4";
#elif defined(FTD_0830_HARMONIC_REINSERTION_REPAIR)
  const auto stem = result_root()/
      "ftd_0830_l17_complete_tangent_harmonic_reinsertion_repair_v3";
#elif defined(FTD_0829_CERTIFICATE_REPAIR)
  const auto stem = result_root()/
      "ftd_0829_l17_complete_tangent_certificate_repair_v2";
#else
  const auto stem = result_root()/"ftd_0774_l17_complete_tangent_candidate_v1";
#endif
  if(!initialize_artifacts(stem)){
    std::cerr<<kFtdId<<" artifact initialization failed\n";
    return 1;
  }
  TangentSummary summary;
  const auto locked_protocol_path = protocol_path();
  summary.protocol_locked =
      ftd0774::sha256_file(locked_protocol_path.string()) == kProtocolSha256;
  summary.provenance = summary.protocol_locked;
  for (const auto& parent : kParents)
    summary.provenance = summary.provenance
        && ftd0774::sha256_file((repo_root()/parent.relative).string())
            == parent.expected;
  for (const auto& source : kCompiledClosure)
    summary.provenance = summary.provenance
        && ftd0774::sha256_file((repo_root()/source.relative).string())
            == source.expected;
  const std::string source_command = "git -C " + quote(repo_root())
      + " diff --quiet " + kSourceCommit
      + " -- engine/include/ftd engine/src/eft";
  summary.source_gate = std::system(source_command.c_str()) == 0;
  const auto options = locked_options();
  summary.options = options_are_locked(options);
  if (!summary.provenance || !summary.source_gate || !summary.options) {
    finalize_artifacts(stem, summary);
    std::cout << "protocol_sha256=" << kProtocolSha256 << '\n'
              << "verdict=" << summary.verdict << '\n'
              << "preflight=provenance/source/options\n";
    return 1;
  }

  const auto reference = load_refined_state(0);
  const auto initialized = ftd::eft::initialize_connected_moore_block(
      17, 2, 0, 0, 0.5, 1e-13, 4096);
  summary.representative = reference.electric.L == 17
      && reference.constituents.size() == 16 && reference.orientation_axis == 0
      && reference.width == 2;
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  summary.beta = normalization.mapped_field_work_coefficient
      * options.field_energy_scale;
  const auto density_gate = deposit(reference);
  const auto modes = load_locked_modes();
  const bool runtime_written=write_runtime_audit(stem,options,reference,
      initialized,normalization,summary.beta,density_gate,modes);
  summary.representative = summary.representative
      &&initialized.valid&&normalization.valid
      &&std::isfinite(normalization.mapped_field_work_coefficient)
      &&std::isfinite(summary.beta)&&summary.beta>0.0&&runtime_written;
  if (!summary.representative) {
    finalize_artifacts(stem, summary);
    std::cout << "protocol_sha256=" << kProtocolSha256 << '\n'
              << "verdict=" << summary.verdict << '\n'
              << "preflight=representative\n";
    return 1;
  }

  const auto analytic = analytic_at("ftd0774", 0, reference, summary.beta, options);
  summary.hessian_antisymmetry = analytic.antisymmetry;
  summary.hessian_eigen_residual = analytic.eigen_residual;
  summary.hessian_orthogonality = analytic.orthogonality;
  summary.hessian_min = analytic.min_eigen;
  summary.hessian_max = analytic.max_eigen;
  summary.field_lower_bound = summary.beta * (1.0 - std::cos(ftd::PI / 34.0));
  const auto full_gradient = audit_full_gradient(reference,summary.beta,
      ftd::C_SPEED,analytic.gradient_inf);
  summary.gradient_dx = full_gradient.dx;
  summary.gradient_dp = full_gradient.dp;
  summary.gradient_eT = full_gradient.eT;
  summary.gradient_b = full_gradient.b;
  summary.maximum_gradient = full_gradient.global;
  summary.gradient = density_gate.valid&&full_gradient.valid
      &&std::isfinite(density_gate.derivative_charge_residual)
      &&density_gate.derivative_charge_residual<=1e-12;
  summary.hessian = analytic.hessian.size() == ftd0774::kMatterDimension
      && analytic.eigenvalues.size() == ftd0774::kMatterDimension
      && analytic.antisymmetry <= 1e-12 && analytic.eigen_residual <= 1e-7
      && analytic.orthogonality <= 1e-10 && analytic.min_eigen > 1e-5
      && summary.field_lower_bound > 0.0;
  if (summary.hessian) write_hessian(stem, analytic);
  {
    std::ofstream gradient_rows(stem.string()+"_preflight.csv",std::ios::app);
    const std::array<std::pair<const char*,double>,4> blocks{{
        {"dx",summary.gradient_dx},{"dp",summary.gradient_dp},
        {"eT",summary.gradient_eT},{"b",summary.gradient_b}}};
    for (const auto& [name,value] : blocks) {
      PreflightRow row;
      row.record_kind="gradient";
      row.probe=name;
      row.valid=std::isfinite(value) && value<=1e-10
          && std::isfinite(summary.maximum_gradient)
          && summary.maximum_gradient<=1e-10;
      row.k_norm=value;
      row.energy_slope=summary.maximum_gradient;
      row.detail=row.valid?"pass":"block_gradient_gate";
      write_preflight_row(gradient_rows,row);
    }
  }
  summary.hessian = summary.hessian && modes[0].valid && modes[1].valid;
  if (!summary.hessian || !summary.gradient) {
    finalize_artifacts(stem, summary);
    std::cout << "protocol_sha256=" << kProtocolSha256 << '\n'
              << "verdict=" << summary.verdict << '\n'
              << "preflight=hessian/gradient\n";
    return 1;
  }

  TangentMetric metric{analytic.hessian, summary.beta, ftd::C_SPEED};
  std::vector<ChartVector> raw_seeds(4);
  raw_seeds[0].dx = modes[0].vector;
  raw_seeds[1].dx = modes[1].vector;
  for (int i = 0; i < ftd0774::kMatterDimension; ++i) {
    raw_seeds[2].dp[i] = ftd::M_INERTIAL * modes[0].vector[i];
    raw_seeds[3].dp[i] = ftd::M_INERTIAL * modes[1].vector[i];
  }
  std::vector<ChartVector> seeds;
  for (auto seed : raw_seeds) {
    if (!ftd0774::k_mgs_two_pass(metric, seed, seeds, 0.0)) {
      finalize_artifacts(stem, summary);
      std::cout << "protocol_sha256=" << kProtocolSha256 << '\n'
                << "verdict=" << summary.verdict << '\n'
                << "preflight=seed_metric\n";
      return 1;
    }
    seeds.push_back(std::move(seed));
  }

  std::vector<std::string> probe_names{"q6","q7","p6","p7"};
  std::vector<ChartVector> probes = seeds;
  const std::array<std::array<int,4>,4> signs{{
      {{1,1,1,1}},{{1,-1,1,-1}},{{1,1,-1,-1}},{{1,-1,-1,1}}}};
  for (int row = 0; row < 4; ++row) {
    ChartVector mixed;
    for (int column = 0; column < 4; ++column)
      ftd0774::axpy(mixed, seeds[column], 0.5 * signs[row][column]);
    probes.push_back(std::move(mixed));
    probe_names.push_back("matter_mix_"+std::to_string(row));
  }
  ChartVector f_e;
  f_e.e = locked_field_shape();
  ChartVector f_b;
  f_b.b = ftd::eft::matched_curl_adjoint(f_e.e);
  ChartVector h_e, h_b;
#ifdef FTD_0832_NONSINGULAR_PRODUCT_CHART
  h_e.e_harmonic[0] = 1.0;
#else
  std::fill(h_e.e.x.begin(), h_e.e.x.end(), 1.0);
#endif
  std::fill(h_b.b.x.begin(), h_b.b.x.end(), 1.0);
  bool field_vectors = f_e.e.L == 17
      && ftd0774::normalize(metric, f_e)
      && ftd0774::normalize(metric, f_b)
      && ftd0774::normalize(metric, h_e)
      && ftd0774::normalize(metric, h_b)
      && ftd::eft::max_divergence(f_e.e) <= 1e-12
      && ftd::eft::max_divergence(
          ftd0774::completed_electric(h_e)) <= 1e-12;
  ChartVector fe_plus_fb=f_e;ftd0774::axpy(fe_plus_fb,f_b,1.0);
  ChartVector fe_minus_fb=f_e;ftd0774::axpy(fe_minus_fb,f_b,-1.0);
  ChartVector q6_plus_fe=seeds[0];ftd0774::axpy(q6_plus_fe,f_e,1.0);
  ChartVector p6_plus_fb=seeds[2];ftd0774::axpy(p6_plus_fb,f_b,1.0);
  field_vectors = field_vectors
      && ftd0774::normalize(metric,fe_plus_fb)
      && ftd0774::normalize(metric,fe_minus_fb)
      && ftd0774::normalize(metric,q6_plus_fe)
      && ftd0774::normalize(metric,p6_plus_fb);
  if (!field_vectors) {
    finalize_artifacts(stem, summary);
    std::cout << "protocol_sha256=" << kProtocolSha256 << '\n'
              << "verdict=" << summary.verdict << '\n'
              << "preflight=field_probes\n";
    return 1;
  }
  const std::array<std::pair<const char*,ChartVector*>,8> fields{{
      {"f_e",&f_e},{"f_b",&f_b},{"f_e_plus_f_b",&fe_plus_fb},
      {"f_e_minus_f_b",&fe_minus_fb},{"h_E_x",&h_e},{"h_B_x",&h_b},
      {"q6_plus_f_e",&q6_plus_fe},{"p6_plus_f_b",&p6_plus_fb}}};
  for (const auto& [name,value] : fields) {
    probe_names.push_back(name);
    probes.push_back(*value);
  }
  if (probes.size() != 16) {
    finalize_artifacts(stem, summary);
    return 1;
  }

  const auto chart = ftd0774::TangentChart(reference,
      [](const ConnectedMooreBlockState& state){return deposit(state);},
      [](const auto& point){return position(point);},
      [](const Vec3& x){return point_at(x);},
      [](const ConnectedMooreBlockState& state){return sector_signature(state);});

  SmallMatrix probe_gram(16,std::vector<double>(16,0.0));
  for(int i=0;i<16;++i)for(int j=0;j<16;++j)
    probe_gram[i][j]=ftd0774::inner(metric,probes[i],probes[j]);
  long double b0_gram_square = 0.0L;
  for (int i=0;i<4;++i) for (int j=0;j<4;++j) {
    const long double residual = probe_gram[i][j]-(i==j?1.0:0.0);
    b0_gram_square += residual*residual;
  }
  summary.b0_gram_residual = std::sqrt(static_cast<double>(b0_gram_square));
  summary.seed_metric = std::isfinite(summary.b0_gram_residual)
      && summary.b0_gram_residual<=1e-10;
  {
    std::ofstream grams(stem.string()+"_gram_blocks.csv",std::ios::app);
    write_matrix_block(grams,"","preflight","final","PROBE_K_PROBE",probe_gram);
  }

  std::ofstream preflight(stem.string()+"_preflight.csv",std::ios::app);
  PreflightRow seed_row;
  seed_row.record_kind="seed_metric";
  seed_row.probe="B0";
  seed_row.valid=summary.seed_metric;
  seed_row.k_norm=summary.b0_gram_residual;
  seed_row.detail=seed_row.valid?"pass":"seed_metric_gate";
  write_preflight_row(preflight,seed_row);
  summary.energy_form = true;
  summary.maximum_energy_slope = 0.0;
  summary.maximum_energy_relative = 0.0;
  std::ofstream energy_control(stem.string()+"_energy_control.csv");
  energy_control<<kEnergyControlHeader<<'\n';
  for (int index = 0; index < 16; ++index) {
    const auto plus = ftd0774::retract(chart, probes[index], kHE);
    const auto minus = ftd0774::retract(chart, probes[index], -kHE);
    const double delta_plus = ftd0774::stable_energy_increment(
        chart, plus, probes[index], kHE, summary.beta, ftd::C_SPEED, options);
    const double delta_minus = ftd0774::stable_energy_increment(
        chart, minus, probes[index], -kHE, summary.beta, ftd::C_SPEED, options);
    write_energy_control_row(energy_control,probe_names[index],+1,kHE,
        plus,delta_plus);
    write_energy_control_row(energy_control,probe_names[index],-1,kHE,
        minus,delta_minus);
    const double slope = (delta_plus-delta_minus)/(2.0*kHE);
    const double second = (delta_plus+delta_minus)/(kHE*kHE);
    const double expected = ftd0774::inner(metric,probes[index],probes[index]);
    const bool positive_expected=std::isfinite(expected)&&expected>0.0;
    const double relative = positive_expected
        ?std::abs(second-expected)/expected:INFINITY;
    const double k_norm = ftd0774::norm(metric,probes[index]);
    const bool pass = plus.valid && minus.valid && positive_expected
        && std::isfinite(slope)
        && std::isfinite(second) && std::isfinite(k_norm) && k_norm>0.0
        && std::abs(slope)<=1e-8 && relative<=1e-6;
    summary.energy_form = summary.energy_form && pass;
    summary.maximum_energy_slope=std::max(summary.maximum_energy_slope,std::abs(slope));
    summary.maximum_energy_relative=std::max(summary.maximum_energy_relative,relative);
    PreflightRow row;row.record_kind="energy";row.probe=probe_names[index];
    row.h=kHE;row.valid=pass;row.gauss_pre=std::max(plus.gauss_residual,minus.gauss_residual);
    row.k_norm=k_norm;row.energy_slope=slope;row.energy_second=second;
    row.energy_relative=relative;row.detail=pass?"pass":"energy_form_gate";
    write_preflight_row(preflight,row);
  }
  energy_control.flush();
  summary.energy_form=summary.energy_form&&static_cast<bool>(energy_control);
  const FieldControl field_control=run_field_control(reference);
  summary.field_control=field_control.pass
      &&write_field_control_artifact(stem,field_control);
  summary.field_phase_relative=field_control.phase_relative;
  summary.field_recurrence=field_control.recurrence;
  summary.field_recovery=field_control.recovery_relative;
  PreflightRow field_row;field_row.record_kind="field_control";
  field_row.probe="family100_n1_p0_e0";field_row.valid=summary.field_control;
  field_row.recovery=field_control.recovery_relative;
  field_row.energy_drift=field_control.energy_drift;
  field_row.composition_residual=field_control.recurrence;
  field_row.detail=field_control.pass?"pass":"source_free_field_control";
  write_preflight_row(preflight,field_row);
  preflight.close();

  // The expensive endpoint/root/cache preflight is fail-closed and begins
  // only after every algebraic and isolated-field gate has passed.
  if (!summary.seed_metric || !summary.energy_form || !summary.field_control) {
    finalize_artifacts(stem, summary);
    std::cout << std::setprecision(17)
              << "protocol_sha256=" << kProtocolSha256 << '\n'
              << "verdict=" << summary.verdict << '\n'
              << "energy_form=" << summary.energy_form
              << " max_slope=" << summary.maximum_energy_slope
              << " max_relative=" << summary.maximum_energy_relative << '\n'
              << "field_control=" << summary.field_control
              << " phase=" << summary.field_phase_relative
              << " recurrence=" << summary.field_recurrence
              << " recovery=" << summary.field_recovery << '\n';
    return 1;
  }

  std::array<std::array<DerivativeEvaluation,16>,2> forward_derivatives;
  std::array<std::array<DerivativeEvaluation,16>,2> reverse_derivatives;
  OperatorAuditLedger preflight_derivative_ledger;
  struct PreflightTaskSpec { int scale=0; int probe=0; bool forward=true; };
  std::vector<PreflightTaskSpec> task_specs;
  for (int scale = 0; scale < 2; ++scale)
    for (int probe = 0; probe < 16; ++probe) {
      task_specs.push_back({scale,probe,true});
      task_specs.push_back({scale,probe,false});
    }
  constexpr std::size_t endpoint_batch = 24;
  for (std::size_t start = 0; start < task_specs.size();
       start += endpoint_batch) {
    const std::size_t end = std::min(start + endpoint_batch, task_specs.size());
    std::vector<std::future<DerivativeEvaluation>> futures;
    futures.reserve(end - start);
    for (std::size_t index = start; index < end; ++index) {
      const auto spec = task_specs[index];
      futures.push_back(std::async(std::launch::async,[&,spec]() {
        const double h = spec.scale == 0 ? kH0 : kH1;
        return evaluate_derivative(
            chart, probes[spec.probe], h, spec.forward, options, true);
      }));
    }
    for (std::size_t index = start; index < end; ++index) {
      const auto spec = task_specs[index];
      auto value = futures[index-start].get();
      if (spec.forward)
        forward_derivatives[spec.scale][spec.probe] = std::move(value);
      else
        reverse_derivatives[spec.scale][spec.probe] = std::move(value);
      const auto& stored=spec.forward
          ?forward_derivatives[spec.scale][spec.probe]
          :reverse_derivatives[spec.scale][spec.probe];
      OperatorAuditKey key{"preflight","probe",
          "probe:"+probe_names[spec.probe],spec.scale,spec.probe,
          spec.scale==0?kH0:kH1,spec.forward};
      preflight_derivative_ledger.record(
          preflight_derivative_ledger.reserve(),key,stored);
    }
    std::cout << "completed endpoint preflight " << end << '/'
              << task_specs.size() << std::endl;
  }

  summary.endpoint_preflight = true;
  summary.regularity = true;
  summary.cache_control = true;
  summary.maximum_common = 0.0;
  summary.maximum_energy_drift = 0.0;
  summary.maximum_recovery = 0.0;
  summary.maximum_scale_relative = 0.0;
  summary.maximum_composition = 0.0;
  summary.maximum_adjoint = 0.0;
  summary.minimum_sigma = INFINITY;
  summary.maximum_condition = 0.0;
  summary.maximum_regularity_scale = 0.0;
  summary.maximum_observer_regression = 0.0;
  summary.maximum_codec_divergence = 0.0;
  summary.maximum_hodge_correction = 0.0;
  summary.maximum_reconstruction = 0.0;
  summary.maximum_harmonic = 0.0;
  bool cache_population_reuse_witness = false;
  std::vector<PreflightRow> endpoint_rows;
  std::vector<CacheControlRow> cache_rows;

  const auto accumulate_signed = [&](const SignedEndpointEvaluation& endpoint,
                                     const RetractionResult& retracted,
                                     const std::string& probe, double h,
                                     bool direction, int sign) {
    summary.endpoint_preflight = summary.endpoint_preflight
        && retracted.valid && endpoint.endpoint_pass;
    summary.regularity = summary.regularity && endpoint.regularity_pass;
    summary.cache_control = summary.cache_control && endpoint.cache_pass;
    cache_population_reuse_witness=cache_population_reuse_witness
        ||(endpoint.population_refreshes>0
           &&endpoint.cache_valid_after_population
           &&endpoint.reuse_reuses>0);
    summary.maximum_common = std::max(summary.maximum_common,endpoint.common);
    summary.maximum_energy_drift = std::max(
        summary.maximum_energy_drift,endpoint.energy_drift);
    summary.maximum_recovery = std::max(
        summary.maximum_recovery,endpoint.recovery);
    if (std::isfinite(endpoint.sigma_min))
      summary.minimum_sigma = std::min(summary.minimum_sigma,endpoint.sigma_min);
    if (std::isfinite(endpoint.condition))
      summary.maximum_condition = std::max(
          summary.maximum_condition,endpoint.condition);
    if (std::isfinite(endpoint.regularity_scale))
      summary.maximum_regularity_scale = std::max(
          summary.maximum_regularity_scale,endpoint.regularity_scale);
    if (std::isfinite(endpoint.observer_regression))
      summary.maximum_observer_regression = std::max(
          summary.maximum_observer_regression,endpoint.observer_regression);
    PreflightRow row;
    row.record_kind = "endpoint";
    row.probe = probe;
    row.h = h;
    row.direction = direction ? "forward" : "reverse";
    row.sign = sign;
    row.valid = retracted.valid && endpoint.valid;
    row.common_residual = endpoint.common;
    row.energy_drift = endpoint.energy_drift;
    row.recovery = endpoint.recovery;
    row.gauss_pre = retracted.gauss_residual;
    row.sigma_min = endpoint.sigma_min;
    row.condition = endpoint.condition;
    row.scale_difference = endpoint.regularity_scale;
    row.observer_regression = endpoint.observer_regression;
    row.jacobian_refreshes = endpoint.jacobian_refreshes;
    row.jacobian_reuses = endpoint.jacobian_reuses;
    row.cache_fallbacks = endpoint.cache_fallbacks;
    row.detail = row.valid ? "pass" : "endpoint_root_or_cache_gate";
    endpoint_rows.push_back(std::move(row));
    CacheControlRow cache_row;
    cache_row.probe=probe;
    cache_row.h=h;
    cache_row.direction=direction?"forward":"reverse";
    cache_row.sign=sign;
    cache_row.retraction_metadata=retracted.metadata;
    cache_row.retraction_sector=retracted.sector;
    cache_row.retraction_finite=retracted.finite;
    cache_row.retraction_gauss=retracted.gauss_residual;
    cache_row.retraction_poisson_absolute=
        retracted.poisson_absolute_residual;
    cache_row.direct_accepted=endpoint.direct_step_pass;
    cache_row.inverse_accepted=endpoint.inverse_step_pass;
    cache_row.observer_accepted=endpoint.observer_step_pass;
    cache_row.endpoint_chart=endpoint.chart_pass;
    cache_row.population_accepted=endpoint.population_step_pass;
    cache_row.reuse_accepted=endpoint.reuse_step_pass;
    cache_row.direct_population_agreement=
        endpoint.direct_population_agreement;
    cache_row.direct_reuse_agreement=endpoint.direct_reuse_agreement;
    cache_row.population_iterations=endpoint.population_iterations;
    cache_row.cache_valid_after_population=
        endpoint.cache_valid_after_population;
    cache_row.population_refreshes=endpoint.population_refreshes;
    cache_row.population_reuses=endpoint.population_reuses;
    cache_row.population_fallbacks=endpoint.population_fallbacks;
    cache_row.reuse_refreshes=endpoint.reuse_refreshes;
    cache_row.reuse_reuses=endpoint.reuse_reuses;
    cache_row.reuse_fallbacks=endpoint.reuse_fallbacks;
    cache_row.cache_semantics=endpoint.cache_semantics;
    cache_row.valid=retracted.valid&&endpoint.endpoint_pass
        &&endpoint.cache_pass;
    cache_rows.push_back(std::move(cache_row));
  };

  const auto accumulate_codec = [&](const DerivativeEvaluation& derivative) {
    summary.endpoint_preflight = summary.endpoint_preflight && derivative.valid;
    summary.maximum_codec_divergence = std::max(
        summary.maximum_codec_divergence,
        std::max(derivative.codec.preclean_divergence,
                 derivative.codec.cleaned_divergence));
    summary.maximum_hodge_correction = std::max(
        summary.maximum_hodge_correction,derivative.codec.hodge_correction);
    summary.maximum_reconstruction = std::max(
        summary.maximum_reconstruction,derivative.codec.reconstruction);
    summary.maximum_harmonic = std::max(summary.maximum_harmonic,
        std::max(derivative.codec.face_harmonic,
                 derivative.codec.edge_harmonic));
  };

  for (int scale = 0; scale < 2; ++scale) {
    const double h = scale == 0 ? kH0 : kH1;
    for (int probe = 0; probe < 16; ++probe) {
      auto& forward = forward_derivatives[scale][probe];
      auto& reverse = reverse_derivatives[scale][probe];
      accumulate_signed(forward.plus_endpoint,forward.plus,
          probe_names[probe],h,true,+1);
      accumulate_signed(forward.minus_endpoint,forward.minus,
          probe_names[probe],h,true,-1);
      accumulate_signed(reverse.plus_endpoint,reverse.plus,
          probe_names[probe],h,false,+1);
      accumulate_signed(reverse.minus_endpoint,reverse.minus,
          probe_names[probe],h,false,-1);
      accumulate_codec(forward);
      accumulate_codec(reverse);
    }
  }
  summary.cache_control = summary.cache_control
      &&cache_population_reuse_witness;
  {
    std::ofstream cache_output(stem.string()+"_cache_control.csv");
    cache_output<<kCacheControlHeader<<'\n';
    for(const auto& row:cache_rows)write_cache_control_row(cache_output,row);
    cache_output.flush();
    summary.cache_control=summary.cache_control
        &&cache_rows.size()==128&&static_cast<bool>(cache_output);
  }

  std::array<DerivativeEvaluation,16> reverse_forward_compositions;
  std::array<DerivativeEvaluation,16> forward_reverse_compositions;
  for (int probe = 0; probe < 16; ++probe) {
    if (forward_derivatives[0][probe].valid)
      reverse_forward_compositions[probe] = evaluate_derivative(
          chart,forward_derivatives[0][probe].value,kH0,false,options,false);
    if (reverse_derivatives[0][probe].valid)
      forward_reverse_compositions[probe] = evaluate_derivative(
          chart,reverse_derivatives[0][probe].value,kH0,true,options,false);
    {
      OperatorAuditKey key{"preflight","composition","reverse_forward",
          std::nullopt,probe,kH0,false};
      preflight_derivative_ledger.record(preflight_derivative_ledger.reserve(),
          key,reverse_forward_compositions[probe]);
      key.operation="forward_reverse";
      key.forward=true;
      preflight_derivative_ledger.record(preflight_derivative_ledger.reserve(),
          key,forward_reverse_compositions[probe]);
    }
    accumulate_codec(reverse_forward_compositions[probe]);
    accumulate_codec(forward_reverse_compositions[probe]);
  }

  std::array<double,16> forward_scale{}, reverse_scale{};
  std::array<double,16> reverse_forward_residual{}, forward_reverse_residual{};
  for (int probe = 0; probe < 16; ++probe) {
    forward_scale[probe] = ftd0774::norm(metric,ftd0774::difference(
        forward_derivatives[0][probe].value,
        forward_derivatives[1][probe].value))
        / std::max(ftd0774::norm(metric,
              forward_derivatives[1][probe].value),1e-30);
    reverse_scale[probe] = ftd0774::norm(metric,ftd0774::difference(
        reverse_derivatives[0][probe].value,
        reverse_derivatives[1][probe].value))
        / std::max(ftd0774::norm(metric,
              reverse_derivatives[1][probe].value),1e-30);
    reverse_forward_residual[probe] = ftd0774::norm(metric,
        ftd0774::difference(reverse_forward_compositions[probe].value,
                            probes[probe]))
        / std::max(ftd0774::norm(metric,probes[probe]),1e-30);
    forward_reverse_residual[probe] = ftd0774::norm(metric,
        ftd0774::difference(forward_reverse_compositions[probe].value,
                            probes[probe]))
        / std::max(ftd0774::norm(metric,probes[probe]),1e-30);
    summary.maximum_scale_relative = std::max({summary.maximum_scale_relative,
        forward_scale[probe],reverse_scale[probe]});
    summary.maximum_composition = std::max({summary.maximum_composition,
        reverse_forward_residual[probe],forward_reverse_residual[probe]});
    summary.endpoint_preflight = summary.endpoint_preflight
        && std::isfinite(forward_scale[probe]) && forward_scale[probe] <= 1e-3
        && std::isfinite(reverse_scale[probe]) && reverse_scale[probe] <= 1e-3
        && reverse_forward_compositions[probe].valid
        && forward_reverse_compositions[probe].valid
        && reverse_forward_residual[probe] <= 1e-4
        && forward_reverse_residual[probe] <= 1e-4;
  }

  SmallMatrix probe_k_t_h0(16,std::vector<double>(16,0.0));
  SmallMatrix tinv_h0_k_probe(16,std::vector<double>(16,0.0));
  SmallMatrix probe_k_t_h1(16,std::vector<double>(16,0.0));
  SmallMatrix tinv_h1_k_probe(16,std::vector<double>(16,0.0));
  for (int scale = 0; scale < 2; ++scale)
    for (int row = 0; row < 16; ++row)
      for (int column = 0; column < 16; ++column) {
        const double left = ftd0774::inner(metric,probes[row],
            forward_derivatives[scale][column].value);
        const double right = ftd0774::inner(metric,
            reverse_derivatives[scale][row].value,probes[column]);
        const double residual = std::abs(left-right)/std::max(
            ftd0774::norm(metric,probes[row])
                *ftd0774::norm(metric,probes[column]),1e-30);
        summary.maximum_adjoint = std::max(summary.maximum_adjoint,residual);
        summary.endpoint_preflight = summary.endpoint_preflight
            && std::isfinite(residual) && residual <= 1e-4;
        if (scale == 0) {
          probe_k_t_h0[row][column] = left;
          tinv_h0_k_probe[row][column] = right;
        } else {
          probe_k_t_h1[row][column] = left;
          tinv_h1_k_probe[row][column] = right;
        }
      }
  {
    std::ofstream grams(stem.string()+"_gram_blocks.csv",std::ios::app);
    write_matrix_block(grams,"","preflight","final",
        "PROBE_K_T_H0",probe_k_t_h0);
    write_matrix_block(grams,"","preflight","final",
        "TINV_H0_K_PROBE",tinv_h0_k_probe);
    write_matrix_block(grams,"","preflight","final",
        "PROBE_K_T_H1",probe_k_t_h1);
    write_matrix_block(grams,"","preflight","final",
        "TINV_H1_K_PROBE",tinv_h1_k_probe);
  }

  ChartVector zero;
  const auto zero_forward = evaluate_derivative(
      chart,zero,kH0,true,options,false);
  const auto zero_reverse = evaluate_derivative(
      chart,zero,kH0,false,options,false);
  {
    OperatorAuditKey key{"preflight","zero","zero",std::nullopt,0,
        kH0,true};
    preflight_derivative_ledger.record(
        preflight_derivative_ledger.reserve(),key,zero_forward);
    key.forward=false;
    preflight_derivative_ledger.record(
        preflight_derivative_ledger.reserve(),key,zero_reverse);
  }
  const double zero_endpoint_difference = std::max(
      ftd::eft::connected_moore_block_state_max_difference(
          zero_forward.plus_endpoint.output,
          zero_forward.minus_endpoint.output),
      ftd::eft::connected_moore_block_state_max_difference(
          zero_reverse.plus_endpoint.output,
          zero_reverse.minus_endpoint.output));
  const double fixed_excursion = std::max({
      ftd::eft::connected_moore_block_state_max_difference(
          reference,zero_forward.plus_endpoint.output),
      ftd::eft::connected_moore_block_state_max_difference(
          reference,zero_reverse.plus_endpoint.output)});
  const double zero_tangent = std::max(
      chart_max_abs(zero_forward.value),chart_max_abs(zero_reverse.value));
  const bool zero_control = zero_forward.valid && zero_reverse.valid
      && zero_endpoint_difference == 0.0 && zero_tangent == 0.0
      && fixed_excursion <= 1e-10;
  summary.endpoint_preflight = summary.endpoint_preflight && zero_control;

  const bool preflight_record_complete=preflight_derivative_ledger.size()==98;
#ifdef FTD_0829_CERTIFICATE_REPAIR
  constexpr bool preserve_preflight_order=true;
#else
  constexpr bool preserve_preflight_order=false;
#endif
  const bool preflight_status_written=write_execution_status(
      stem,preflight_derivative_ledger,preflight_record_complete,
      "_preflight_derivative_status.csv","preflight",
      "preflight_record_complete","preflight_record_abort",
      preserve_preflight_order);
  summary.endpoint_preflight=summary.endpoint_preflight
      &&preflight_record_complete&&preflight_status_written;

  preflight.open(stem.string()+"_preflight.csv",std::ios::app);
  for (const auto& row : endpoint_rows) write_preflight_row(preflight,row);
  for (int scale = 0; scale < 2; ++scale) {
    const double h = scale == 0 ? kH0 : kH1;
    for (int probe = 0; probe < 16; ++probe)
      for (int direction = 0; direction < 2; ++direction) {
        const auto& derivative = direction == 0
            ? forward_derivatives[scale][probe]
            : reverse_derivatives[scale][probe];
        PreflightRow row;
        row.record_kind = "derivative";
        row.probe = probe_names[probe];
        row.h = h;
        row.direction = direction == 0 ? "forward" : "reverse";
        row.valid = derivative.valid;
        row.gauss_pre = derivative.codec.preclean_divergence;
        row.gauss_clean = derivative.codec.cleaned_divergence;
        row.hodge_correction = derivative.codec.hodge_correction;
        row.reconstruction = derivative.codec.reconstruction;
        row.harmonic_face = derivative.codec.face_harmonic;
        row.harmonic_edge = derivative.codec.edge_harmonic;
        row.k_norm = ftd0774::norm(metric,derivative.value);
        row.derivative_scale_relative = direction == 0
            ? forward_scale[probe] : reverse_scale[probe];
        if (scale == 0) row.composition_residual = direction == 0
            ? reverse_forward_residual[probe]
            : forward_reverse_residual[probe];
        row.detail = codec_detail(derivative.codec,row.valid);
        write_preflight_row(preflight,row);
      }
  }
  PreflightRow zero_row;
  zero_row.record_kind="zero_control";
  zero_row.probe="zero";
  zero_row.h=kH0;
  zero_row.valid=zero_control;
  zero_row.recovery=fixed_excursion;
  zero_row.k_norm=zero_tangent;
  zero_row.composition_residual=zero_endpoint_difference;
  zero_row.detail=zero_control?"pass":"zero_or_fixed_point_gate";
  write_preflight_row(preflight,zero_row);
  preflight.close();

  if (!std::isfinite(summary.minimum_sigma)) summary.minimum_sigma = NAN;
  summary.preflight = summary.seed_metric && summary.energy_form && summary.field_control
      && summary.endpoint_preflight && summary.regularity
      && summary.cache_control;
  if (!summary.preflight) {
    finalize_artifacts(stem, summary);
    std::cout << std::setprecision(17)
              << "protocol_sha256=" << kProtocolSha256 << '\n'
              << "verdict=" << summary.verdict << '\n'
              << "endpoint=" << summary.endpoint_preflight
              << " regularity=" << summary.regularity
              << " cache=" << summary.cache_control
              << " common=" << summary.maximum_common
              << " drift=" << summary.maximum_energy_drift
              << " recovery=" << summary.maximum_recovery
              << " scale=" << summary.maximum_scale_relative
              << " composition=" << summary.maximum_composition
              << " adjoint=" << summary.maximum_adjoint
              << " sigma_min=" << summary.minimum_sigma
              << " condition=" << summary.maximum_condition
              << " regularity_scale=" << summary.maximum_regularity_scale
              << " observer=" << summary.maximum_observer_regression
              << " codec_divergence=" << summary.maximum_codec_divergence
              << " hodge=" << summary.maximum_hodge_correction
              << " reconstruction=" << summary.maximum_reconstruction
              << " harmonic=" << summary.maximum_harmonic << '\n';
    return 1;
  }

  OperatorAuditLedger execution_ledger;
  auto pipeline=tangent_run_pipeline(
      chart,metric,options,seeds,execution_ledger);
  const bool postflight_complete=pipeline.execution_valid;
  if(!postflight_complete){
    (void)abort_postflight(stem,summary,execution_ledger,
        "postflight_execution",0.0,"postflight_execution_failure");
    (void)finalize_artifacts(stem,summary,&execution_ledger);
    std::cout<<"protocol_sha256="<<kProtocolSha256<<'\n'
             <<"verdict="<<summary.verdict<<'\n'
             <<"postflight_execution=false evaluations="
             <<execution_ledger.size()<<'\n';
    return 1;
  }

  summary.krylov_executed=true;
  summary.krylov_resolved=pipeline.solve_resolved;
  for(const auto& run:pipeline.runs)
    for(const auto& stage:run.stages){
      std::string label;
      if(run.construction=="primary")label="primary_"+stage.stage;
      else if(run.construction=="h1")label="h1_"+stage.stage;
      else if(stage.stage=="final")label=run.construction+"_final";
      if(!label.empty())summary.dimensions[label]=stage.dimension;
      if(run.construction=="primary"&&stage.stage=="final")
        for(const auto& cluster:stage.clusters)
          if(cluster.eligible)++summary.eligible_candidates;
    }
  std::vector<const TangentCandidate*> qualified;
  for(const auto& candidate:pipeline.candidates)
    if(candidate.qualified){qualified.push_back(&candidate);}
  summary.qualified_candidates=static_cast<int>(qualified.size());
  if(!qualified.empty()){
    std::sort(qualified.begin(),qualified.end(),[](const auto* left,
                                                   const auto* right){
      if(left->seed_overlap!=right->seed_overlap)
        return left->seed_overlap>right->seed_overlap;
      if(left->ritz_residual!=right->ritz_residual)
        return left->ritz_residual<right->ritz_residual;
      return left->phase_mean<right->phase_mean;
    });
    summary.selected_candidate=qualified.front()->candidate_id;
  }
  if(!pipeline.solve_resolved)
    summary.verdict="L17_FIRST_DOUBLET_TANGENT_SOLVE_UNRESOLVED";
  else if(qualified.empty())
    summary.verdict="L17_FIRST_DOUBLET_LOCKED_CANDIDATES_NOT_QUALIFIED";
  else
    summary.verdict=
        "L17_FIRST_DOUBLET_POSITIVE_TANGENT_CANDIDATE_CONSTRUCTIVE";
  summary.companion="PRODUCTION_NATIVE_BRIDGE_OPEN";

  if(!write_tangent_pipeline_artifacts(stem,pipeline)){
    (void)abort_postflight(stem,summary,execution_ledger,
        "artifact_write",1.0,"postflight_artifact_write_failure");
    (void)finalize_artifacts(stem,summary,&execution_ledger);
    std::cout<<"protocol_sha256="<<kProtocolSha256<<'\n'
             <<"verdict="<<summary.verdict<<'\n'
             <<"postflight_artifact_write=false evaluations="
             <<execution_ledger.size()<<'\n';
    return 1;
  }
  if(!write_execution_status(stem,execution_ledger,true)){
    (void)abort_postflight(stem,summary,execution_ledger,
        "status_write",1.5,"postflight_status_write_failure");
    (void)finalize_artifacts(stem,summary,&execution_ledger);
    std::cout<<"protocol_sha256="<<kProtocolSha256<<'\n'
             <<"verdict="<<summary.verdict<<'\n'
             <<"postflight_status_write=false evaluations="
             <<execution_ledger.size()<<'\n';
    return 1;
  }
  const bool artifacts_valid=finalize_artifacts(
      stem,summary,&execution_ledger);
  std::cout<<"protocol_sha256="<<kProtocolSha256<<'\n'
           <<"verdict="<<summary.verdict<<'\n'
           <<"krylov_resolved="<<summary.krylov_resolved
           <<" eligible="<<summary.eligible_candidates
           <<" qualified="<<summary.qualified_candidates
           <<" evaluations="<<execution_ledger.size()<<'\n';
  return artifacts_valid
      &&summary.verdict!="L17_COMPLETE_TANGENT_EXECUTION_INVALID"?0:1;
}
