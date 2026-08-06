// FTD-0684/0685: fresh causal excitation-separation discriminator.
#define FTD_0678_EMBEDDED
#include "test_localized_basin_relaxation.cpp"
#undef FTD_0678_EMBEDDED

#include "ftd/eft/component_aware_radial_field_profile.h"
#include "ftd/eft/batched_regional_energy_profile.h"
#include "ftd/eft/matched_regional_energy_transport.h"

#ifdef FTD_SEPARATION_EXTRA_HEADER
#include FTD_SEPARATION_EXTRA_HEADER
#endif

namespace {

#ifndef FTD_SEPARATION_PROTOCOL_SHA256
#define FTD_SEPARATION_PROTOCOL_SHA256 \
  "CA82DDAFC93AB7FB339EB3D2186B3C974E8602EE8BA4AD295FC9E3DE6A6A589E"
#endif
#ifndef FTD_SEPARATION_IDENTIFIER
#define FTD_SEPARATION_IDENTIFIER "FTD-0684"
#endif
#ifndef FTD_SEPARATION_RESULT_DIRECTORY
#define FTD_SEPARATION_RESULT_DIRECTORY "results/ftd_0684"
#endif
#ifndef FTD_SEPARATION_RESULT_STEM
#define FTD_SEPARATION_RESULT_STEM "ftd_0684_causal_excitation_separation_v1"
#endif
#ifndef FTD_SEPARATION_CENTER_TOLERANCE
#define FTD_SEPARATION_CENTER_TOLERANCE 0.0
#endif
#ifndef FTD_SEPARATION_USE_BATCHED_REGIONAL
#define FTD_SEPARATION_USE_BATCHED_REGIONAL 0
#endif
#ifndef FTD_SEPARATION_VOLUME
#define FTD_SEPARATION_VOLUME 129
#endif
#ifndef FTD_SEPARATION_HORIZON
#define FTD_SEPARATION_HORIZON 112
#endif
#ifndef FTD_SEPARATION_LATE_START
#define FTD_SEPARATION_LATE_START 88
#endif
#ifndef FTD_SEPARATION_LATE_END
#define FTD_SEPARATION_LATE_END 112
#endif
#ifndef FTD_SEPARATION_PROGRESS_INTERVAL
#define FTD_SEPARATION_PROGRESS_INTERVAL 8
#endif
#ifndef FTD_SEPARATION_OBSERVATION_CADENCE
#define FTD_SEPARATION_OBSERVATION_CADENCE 1
#endif
#ifndef FTD_SEPARATION_USE_LOCAL_RESIDUAL
#define FTD_SEPARATION_USE_LOCAL_RESIDUAL 0
#endif
#ifndef FTD_SEPARATION_MOMENTUM
#define FTD_SEPARATION_MOMENTUM 1.25e-7
#endif
#ifndef FTD_SEPARATION_EXTRA_OBSERVE
#define FTD_SEPARATION_EXTRA_OBSERVE(...) do {} while (false)
#endif
#ifndef FTD_SEPARATION_EXTRA_FINALIZE
#define FTD_SEPARATION_EXTRA_FINALIZE(...) do {} while (false)
#endif
#ifndef FTD_SEPARATION_EXTRA_EXECUTION_VALID
#define FTD_SEPARATION_EXTRA_EXECUTION_VALID true
#endif

constexpr char separation_protocol_sha256[] =
    FTD_SEPARATION_PROTOCOL_SHA256;
constexpr char separation_identifier[] = FTD_SEPARATION_IDENTIFIER;
constexpr char separation_result_directory[] = FTD_SEPARATION_RESULT_DIRECTORY;
constexpr char separation_result_stem[] = FTD_SEPARATION_RESULT_STEM;
constexpr int separation_volume = FTD_SEPARATION_VOLUME;
constexpr int separation_horizon = FTD_SEPARATION_HORIZON;
constexpr int separation_source_radius = 8;
constexpr int separation_contact_tick =
    separation_volume - 2 * separation_source_radius;
constexpr double separation_momentum = FTD_SEPARATION_MOMENTUM;
constexpr int separation_late_start = FTD_SEPARATION_LATE_START;
constexpr int separation_late_end = FTD_SEPARATION_LATE_END;
constexpr std::array<int, 6> separation_radii{{8, 16, 24, 32, 40, 48}};
constexpr double separation_arrival_threshold = 0.001;

struct SeparationTick {
  int tick = 0;
  bool valid = false;
  double target_energy = 0.0;
  double target_ratio = INFINITY;
  double target_contract_residual = INFINITY;
  double core_position = INFINITY;
  double core_momentum = INFINITY;
  double core_phase = INFINITY;
  double core_ratio = INFINITY;
  double field_total = INFINITY;
  double field_mean_radius = INFINITY;
  double field_rms_radius = INFINITY;
  double radius_50 = INFINITY;
  double radius_90 = INFINITY;
  double radius_99 = INFINITY;
  std::array<double, 6> cumulative_field{};
  std::array<double, 6> boundary_transport_into{};
  std::array<double, 6> source_exchange_into_field{};
  std::array<double, 6> regional_energy_after{};
  int source_support_radius = 0;
  double profile_residual = INFINITY;
  double regional_residual = INFINITY;
  double source_exchange_span = INFINITY;
  double energy_drift = INFINITY;
  double common_residual = INFINITY;
};

struct SeparationLinearFit {
  bool valid = false;
  int samples = 0;
  double intercept = 0.0;
  double slope = 0.0;
  double r_squared = -INFINITY;
};

struct SeparationLateStats {
  bool valid = false;
  double mean = INFINITY;
  double coefficient_of_variation = INFINITY;
  double slope = INFINITY;
  double relative_slope = INFINITY;
};

struct SeparationArm {
  int sign = 0;
  bool initialized = false;
  bool forward = false;
  bool reverse = false;
  bool sector = false;
  bool exact = false;
  double initial_target = 0.0;
  double initial_core = 0.0;
  double maximum_momentum = 0.0;
  double maximum_profile_residual = 0.0;
  double maximum_regional_residual = 0.0;
  double maximum_source_exchange_span = 0.0;
  double maximum_energy_drift = 0.0;
  double maximum_common_residual = 0.0;
  double maximum_target_contract_residual = 0.0;
  double recovery = INFINITY;
  std::array<double, 6> cumulative_outward{};
  std::array<double, 6> cumulative_inward{};
  std::array<double, 6> block_source_exchange{};
  std::array<double, 6> last_regional_energy{};
  std::array<int, 6> arrival_tick{{-1, -1, -1, -1, -1, -1}};
  SeparationLinearFit shell_fit{};
  SeparationLateStats late_core{};
  SeparationLateStats late_near{};
  std::string spatial_class = "DISTRIBUTED_FIELD_MIXED";
  std::string late_class = "CONTINUING_EXCITATION_TRANSFER";
  std::vector<SeparationTick> ticks;
};

ftd::eft::ConnectedMooreBlockState separation_reference() {
  const auto base = load_refined_state(0);
  if (base.electric.L != L)
    return ftd::eft::ConnectedMooreBlockState{};
  auto geometry = base;
  geometry.electric = ftd::eft::MatchedFaceFlux(separation_volume);
  geometry.magnetic_half = ftd::eft::MatchedEdgeField(separation_volume);
  const Vec3 base_center = center(base);
  const Vec3 target_center{
      0.5 * static_cast<double>(separation_volume - 1),
      0.5 * static_cast<double>(separation_volume - 1),
      0.5 * static_cast<double>(separation_volume - 1)};
  for (int particle = 0; particle < count; ++particle) {
    const Vec3 x = target_center
        + (position(base.constituents[particle]) - base_center);
    geometry.constituents[particle] = point_at_volume(x, separation_volume);
    geometry.constituents[particle].momentum = {};
  }
  const auto dressed = ftd::eft::redress_connected_moore_block_with_fibre_limit(
      geometry, 8, 1e-13, 4096);
  return dressed.valid ? dressed.state
                       : ftd::eft::ConnectedMooreBlockState{};
}

ftd::eft::ConnectedMooreBlockState separation_excitation(
    const ftd::eft::ConnectedMooreBlockState& reference,
    const FullModes& modes,
    int sign) {
  double nominal = 0.0;
  auto state = volume_excitation(reference, modes, sign, nominal);
  const double actual = maximum_momentum(state);
  const double scale = actual > 0.0 ? separation_momentum / actual : 0.0;
  for (auto& constituent : state.constituents)
    constituent.momentum *= scale;
  return state;
}

SeparationLinearFit separation_linear_fit(
    const std::vector<std::pair<double, double>>& samples) {
  SeparationLinearFit result;
  if (samples.size() < 2) return result;
  const double n = static_cast<double>(samples.size());
  double sx = 0.0;
  double sy = 0.0;
  double sxx = 0.0;
  double sxy = 0.0;
  for (const auto& [x, y] : samples) {
    sx += x;
    sy += y;
    sxx += x * x;
    sxy += x * y;
  }
  const double denominator = n * sxx - sx * sx;
  if (!(denominator > 0.0)) return result;
  result.slope = (n * sxy - sx * sy) / denominator;
  result.intercept = (sy - result.slope * sx) / n;
  const double mean_y = sy / n;
  double residual = 0.0;
  double total = 0.0;
  for (const auto& [x, y] : samples) {
    const double fit = result.intercept + result.slope * x;
    residual += (y - fit) * (y - fit);
    total += (y - mean_y) * (y - mean_y);
  }
  result.samples = static_cast<int>(samples.size());
  result.r_squared = total > 0.0 ? 1.0 - residual / total
                                 : (residual == 0.0 ? 1.0 : -INFINITY);
  result.valid = std::isfinite(result.slope)
      && std::isfinite(result.intercept)
      && std::isfinite(result.r_squared);
  return result;
}

SeparationLateStats separation_late_stats(
    const SeparationArm& arm,
    bool core) {
  SeparationLateStats result;
  std::vector<std::pair<double, double>> samples;
  for (const auto& tick : arm.ticks) {
    if (tick.tick < separation_late_start || tick.tick > separation_late_end)
      continue;
    const double value = core ? tick.core_ratio
        : tick.cumulative_field[0] / arm.initial_core;
    if (!std::isfinite(value)) return result;
    samples.emplace_back(static_cast<double>(tick.tick), value);
  }
  if (samples.size() != static_cast<std::size_t>(
          (separation_late_end - separation_late_start)
              / FTD_SEPARATION_OBSERVATION_CADENCE + 1))
    return result;
  double sum = 0.0;
  for (const auto& sample : samples) sum += sample.second;
  result.mean = sum / static_cast<double>(samples.size());
  double variance = 0.0;
  for (const auto& sample : samples)
    variance += (sample.second - result.mean)
        * (sample.second - result.mean);
  variance /= static_cast<double>(samples.size());
  const auto fit = separation_linear_fit(samples);
  result.coefficient_of_variation = result.mean != 0.0
      ? std::sqrt(variance) / std::abs(result.mean)
      : (variance == 0.0 ? 0.0 : INFINITY);
  result.slope = fit.slope;
  result.relative_slope = std::abs(result.slope)
      / std::max(std::abs(result.mean), 1e-300);
  result.valid = fit.valid && std::isfinite(result.mean)
      && std::isfinite(result.coefficient_of_variation)
      && std::isfinite(result.relative_slope);
  return result;
}

SeparationTick observe_separation_tick(
    int tick,
    const ftd::eft::ConnectedMooreBlockState& control_before,
    const ftd::eft::ConnectedMooreBlockState& excited_before,
    const ftd::eft::ConnectedMooreBlockState& control_after,
    const ftd::eft::ConnectedMooreBlockState& excited_after,
    const ftd::eft::ConnectedMooreBlockStepResult* control_step,
    const ftd::eft::ConnectedMooreBlockStepResult* excited_step,
    const std::vector<ftd::eft::ConnectedTangentMode>& basis,
    double initial_target,
    double initial_core,
    double omega,
    double beta,
    const Vec3& origin,
    const ftd::eft::ConnectedMooreBlockOptions& options,
    double energy_drift,
    double common) {
  SeparationTick record;
  record.tick = tick;
  const auto reservoir = observe_donor(
      tick, control_after, excited_after, basis,
      initial_target, beta, options);
  record.target_ratio = reservoir.target;
  record.target_energy = initial_target * record.target_ratio;
  record.target_contract_residual = std::abs(
      record.target_energy - initial_target * record.target_ratio);

  const auto localized = ftd::eft::observe_localized_basin(
      control_after, excited_after, origin, separation_source_radius,
      separation_radii.back(), omega, beta, options.wave_speed,
      ftd::M_INERTIAL, 1e-12);
  record.core_position = localized.internal_position_metric;
  record.core_momentum = localized.internal_momentum_metric;
  record.core_phase = localized.core_phase_metric;
  record.core_ratio = initial_core > 0.0
      ? record.core_phase / initial_core : INFINITY;

  const auto profile = ftd::eft::observe_component_aware_radial_field_profile(
      control_after.electric, control_after.magnetic_half,
      excited_after.electric, excited_after.magnetic_half,
      origin, beta, options.wave_speed, 1e-12);
  record.field_total = profile.total_norm;
  record.field_mean_radius = profile.mean_radius;
  record.field_rms_radius = profile.rms_radius;
  record.radius_50 = 0.5 * profile.doubled_radius_50;
  record.radius_90 = 0.5 * profile.doubled_radius_90;
  record.radius_99 = 0.5 * profile.doubled_radius_99;
  for (std::size_t radius = 0; radius < separation_radii.size(); ++radius) {
    const int bin = 2 * separation_radii[radius];
    record.cumulative_field[radius] =
        profile.cumulative_norm_by_doubled_radius[
            static_cast<std::size_t>(bin)];
  }
  record.profile_residual = std::max({profile.partition_residual,
      profile.cumulative_residual, profile.monotonicity_residual});

  record.regional_residual = 0.0;
  record.source_exchange_span = 0.0;
  if (tick > 0 && control_step != nullptr && excited_step != nullptr) {
    const auto electric_before = subtract_face(
        excited_before.electric, control_before.electric);
    const auto magnetic_before = subtract_edge(
        excited_before.magnetic_half, control_before.magnetic_half);
    const auto electric_after = subtract_face(
        excited_after.electric, control_after.electric);
    const auto magnetic_after = subtract_edge(
        excited_after.magnetic_half, control_after.magnetic_half);
    ftd::eft::MatchedFaceFlux electric_pre_current(separation_volume);
    source_free_intermediate(
        electric_before, magnetic_after, electric_pre_current);
    const double scale = beta / initial_target;
    double minimum_exchange = INFINITY;
    double maximum_exchange = -INFINITY;
#if FTD_SEPARATION_USE_BATCHED_REGIONAL
    const std::vector<int> registered_radii(
        separation_radii.begin(), separation_radii.end());
    const auto batch = ftd::eft::evaluate_batched_regional_energy_profile(
        electric_before, magnetic_before, electric_pre_current,
        magnetic_after, electric_after, options.wave_speed,
        origin, registered_radii, 1e-10);
    if (batch.regions.size() != separation_radii.size()) {
      record.regional_residual = INFINITY;
      return record;
    }
#endif
    for (std::size_t radius = 0; radius < separation_radii.size(); ++radius) {
#if FTD_SEPARATION_USE_BATCHED_REGIONAL
      const auto& regional = batch.regions[radius];
#else
      const auto regional =
          ftd::eft::evaluate_matched_regional_energy_transport(
              electric_before, magnetic_before, electric_pre_current,
              magnetic_after, electric_after, options.wave_speed,
              origin, separation_radii[radius], 1e-10);
#endif
      record.boundary_transport_into[radius] =
          scale * regional.boundary_transport_into;
      record.source_exchange_into_field[radius] =
          scale * regional.source_exchange_into_field;
      record.regional_energy_after[radius] = scale * regional.energy_after;
      record.regional_residual = std::max({record.regional_residual,
          regional.magnetic_update_residual,
          regional.electric_pre_update_residual,
          regional.global_source_free_residual,
          regional.partition_residual,
          regional.regional_ledger_residual});
      minimum_exchange = std::min(
          minimum_exchange, record.source_exchange_into_field[radius]);
      maximum_exchange = std::max(
          maximum_exchange, record.source_exchange_into_field[radius]);
      if (!regional.valid) record.regional_residual = INFINITY;
    }
#if FTD_SEPARATION_USE_BATCHED_REGIONAL
    if (!batch.valid) record.regional_residual = INFINITY;
#endif
    record.source_exchange_span = maximum_exchange - minimum_exchange;
    record.source_support_radius = std::max(
        segment_support_radius(*control_step, origin),
        segment_support_radius(*excited_step, origin));
  }
  record.energy_drift = energy_drift;
  record.common_residual = common;
  record.valid = reservoir.valid && localized.valid && profile.valid
      && std::isfinite(record.core_ratio)
      && std::isfinite(record.target_ratio)
      && record.target_contract_residual <= 1e-12
      && record.profile_residual <= 1e-12
      && record.regional_residual <= 1e-10
      && record.source_exchange_span <= 1e-10
      && record.source_support_radius <= separation_source_radius
      && energy_drift <= 1e-10 && common <= 1e-10;
  return record;
}

double separation_global_source_exchange(
    const ftd::eft::ConnectedMooreBlockState& control_before,
    const ftd::eft::ConnectedMooreBlockState& excited_before,
    const ftd::eft::ConnectedMooreBlockState& control_after,
    const ftd::eft::ConnectedMooreBlockState& excited_after,
    double beta,
    double initial_target,
    double wave_speed) {
  const auto electric_before = subtract_face(
      excited_before.electric, control_before.electric);
  const auto magnetic_after = subtract_edge(
      excited_after.magnetic_half, control_after.magnetic_half);
  const auto electric_after = subtract_face(
      excited_after.electric, control_after.electric);
  ftd::eft::MatchedFaceFlux electric_pre(separation_volume);
  source_free_intermediate(electric_before, magnetic_after, electric_pre);
  return beta / initial_target * (
      ftd::eft::matched_modified_energy(
          electric_after, magnetic_after, wave_speed)
      - ftd::eft::matched_modified_energy(
          electric_pre, magnetic_after, wave_speed));
}

void classify_separation(SeparationArm& arm) {
  std::vector<std::pair<double, double>> arrival_samples;
  bool all_arrived = true;
  bool strictly_ordered = true;
  for (std::size_t radius = 0; radius < separation_radii.size(); ++radius) {
    if (arm.arrival_tick[radius] < 0) all_arrived = false;
    if (radius > 0 && (arm.arrival_tick[radius - 1] < 0
        || arm.arrival_tick[radius] <= arm.arrival_tick[radius - 1]))
      strictly_ordered = false;
    if (radius > 0 && arm.arrival_tick[radius] >= 0)
      arrival_samples.emplace_back(
          static_cast<double>(separation_radii[radius]),
          static_cast<double>(arm.arrival_tick[radius]));
  }
  arm.shell_fit = separation_linear_fit(arrival_samples);
  const double shell_speed = arm.shell_fit.valid && arm.shell_fit.slope > 0.0
      ? 1.0 / arm.shell_fit.slope : -INFINITY;
  if (all_arrived && strictly_ordered && arm.shell_fit.valid
      && arm.shell_fit.samples == 5 && arm.shell_fit.r_squared >= 0.98
      && shell_speed > 0.0) {
    arm.spatial_class = "ORDERED_OUTWARD_PACKET";
  } else if (arm.arrival_tick.back() < 0 && !arm.ticks.empty()
             && arm.ticks.back().radius_90 <= 16.0) {
    arm.spatial_class = "LOCALIZED_EXCITATION_FIELD";
  } else {
    arm.spatial_class = "DISTRIBUTED_FIELD_MIXED";
  }

  arm.late_core = separation_late_stats(arm, true);
  arm.late_near = separation_late_stats(arm, false);
  const bool plateau = arm.late_core.valid && arm.late_near.valid
      && arm.late_core.mean >= 0.01 && arm.late_near.mean >= 0.01
      && arm.late_core.coefficient_of_variation <= 0.10
      && arm.late_near.coefficient_of_variation <= 0.10
      && arm.late_core.relative_slope <= 0.001
      && arm.late_near.relative_slope <= 0.001;
  const bool recovered = !arm.ticks.empty()
      && arm.ticks.back().core_ratio <= 0.01
      && arm.ticks.back().cumulative_field[0] / arm.initial_core <= 0.01;
  if (plateau)
    arm.late_class = "EXCITATION_BOUND_PLATEAU";
  else if (recovered)
    arm.late_class = "CONTROL_REST_RECOVERED";
  else
    arm.late_class = "CONTINUING_EXCITATION_TRANSFER";
}

double separation_core_history_rms(
    const std::array<SeparationArm, 2>& arms) {
  if (arms[0].ticks.size() != arms[1].ticks.size()
      || arms[0].ticks.empty()) return INFINITY;
  double sum = 0.0;
  for (std::size_t index = 0; index < arms[0].ticks.size(); ++index) {
    const double difference = arms[0].ticks[index].core_ratio
        - arms[1].ticks[index].core_ratio;
    sum += difference * difference;
  }
  return std::sqrt(sum / static_cast<double>(arms[0].ticks.size()));
}

double separation_final_fraction_difference(
    const std::array<SeparationArm, 2>& arms) {
  if (arms[0].ticks.empty() || arms[1].ticks.empty()) return INFINITY;
  const auto& left = arms[0].ticks.back();
  const auto& right = arms[1].ticks.back();
  double result = 0.0;
  for (std::size_t radius = 0; radius < separation_radii.size(); ++radius) {
    const double left_fraction = left.field_total > 0.0
        ? left.cumulative_field[radius] / left.field_total : 0.0;
    const double right_fraction = right.field_total > 0.0
        ? right.cumulative_field[radius] / right.field_total : 0.0;
    result = std::max(result, std::abs(left_fraction - right_fraction));
  }
  return result;
}

void write_separation(const std::array<SeparationArm, 2>& arms,
                      bool parent,
                      bool preflight,
                      bool initial_fields_equal,
                      double core_history_rms,
                      double final_fraction_difference,
                      const std::string& verdict) {
  const auto safe = [](double value) {
    return std::isfinite(value) ? value : -1.0;
  };
  const auto directory = std::filesystem::path(__FILE__).parent_path().parent_path()
      / separation_result_directory;
  std::filesystem::create_directories(directory);
  std::ofstream json(directory / (std::string(separation_result_stem) + ".json"));
  json << std::setprecision(17) << std::boolalpha
       << "{\n  \"ftd_id\": \"" << separation_identifier << "\",\n"
       << "  \"protocol_sha256\": \"" << separation_protocol_sha256 << "\",\n"
       << "  \"verdict\": \"" << verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"parent_pass\": " << parent << ",\n"
       << "  \"observer_preflight_pass\": " << preflight << ",\n"
       << "  \"initial_fields_bitwise_equal\": " << initial_fields_equal << ",\n"
       << "  \"volume\": " << separation_volume << ",\n"
       << "  \"horizon\": " << separation_horizon << ",\n"
       << "  \"contact_tick\": " << separation_contact_tick << ",\n"
       << "  \"maximum_constituent_momentum\": " << separation_momentum << ",\n"
       << "  \"core_history_rms\": " << safe(core_history_rms) << ",\n"
       << "  \"final_fraction_difference\": "
       << safe(final_fraction_difference) << ",\n";
  for (int index = 0; index < 2; ++index) {
    const auto& arm = arms[index];
    const std::string prefix = arm.sign < 0 ? "negative" : "positive";
    const double shell_speed = arm.shell_fit.valid && arm.shell_fit.slope > 0.0
        ? 1.0 / arm.shell_fit.slope : -1.0;
    json << "  \"" << prefix << "_exact\": " << arm.exact << ",\n"
         << "  \"" << prefix << "_spatial_class\": \""
         << arm.spatial_class << "\",\n"
         << "  \"" << prefix << "_late_class\": \""
         << arm.late_class << "\",\n"
         << "  \"" << prefix << "_shell_speed\": " << shell_speed << ",\n"
         << "  \"" << prefix << "_shell_fit_r_squared\": "
         << safe(arm.shell_fit.r_squared) << ",\n"
         << "  \"" << prefix << "_arrival_ticks\": [";
    for (std::size_t radius = 0; radius < arm.arrival_tick.size(); ++radius)
      json << (radius == 0 ? "" : ",") << arm.arrival_tick[radius];
    json << "],\n"
         << "  \"" << prefix << "_late_core_mean\": "
         << safe(arm.late_core.mean) << ",\n"
         << "  \"" << prefix << "_late_core_relative_slope\": "
         << safe(arm.late_core.relative_slope) << ",\n"
         << "  \"" << prefix << "_late_near_mean\": "
         << safe(arm.late_near.mean) << ",\n"
         << "  \"" << prefix << "_late_near_relative_slope\": "
         << safe(arm.late_near.relative_slope) << ",\n"
         << "  \"" << prefix << "_max_profile_residual\": "
         << safe(arm.maximum_profile_residual) << ",\n"
         << "  \"" << prefix << "_max_regional_residual\": "
         << safe(arm.maximum_regional_residual) << ",\n"
         << "  \"" << prefix << "_max_source_exchange_span\": "
         << safe(arm.maximum_source_exchange_span) << ",\n"
         << "  \"" << prefix << "_max_energy_drift\": "
         << safe(arm.maximum_energy_drift) << ",\n"
         << "  \"" << prefix << "_max_common_residual\": "
         << safe(arm.maximum_common_residual) << ",\n"
         << "  \"" << prefix << "_recovery\": " << safe(arm.recovery)
         << (index == 0 ? ",\n" : "\n");
  }
  json << "}\n";

  std::ofstream csv(
      directory / (std::string(separation_result_stem) + "_ticks.csv"));
  csv << "ftd_id,protocol_sha256,sign,tick,radius,target_energy,target_ratio,"
         "target_contract_residual,core_position,core_momentum,core_phase,"
         "core_ratio,field_total,cumulative_field,field_mean_radius,"
         "field_rms_radius,r50,r90,r99,boundary_transport_into,"
         "source_exchange_into_field,cumulative_outward,cumulative_inward,"
         "arrival_tick,source_support_radius,profile_residual,"
         "regional_residual,source_exchange_span,energy_drift,common_residual,"
         "valid\n";
  for (const auto& arm : arms) {
    std::array<double, 6> outward{};
    std::array<double, 6> inward{};
    for (const auto& tick : arm.ticks) {
      for (std::size_t radius = 0; radius < separation_radii.size(); ++radius) {
        outward[radius] += std::max(
            -tick.boundary_transport_into[radius], 0.0);
        inward[radius] += std::max(
            tick.boundary_transport_into[radius], 0.0);
        csv << std::setprecision(17) << separation_identifier << ','
            << separation_protocol_sha256 << ',' << arm.sign << ','
            << tick.tick << ',' << separation_radii[radius] << ','
            << tick.target_energy << ',' << tick.target_ratio << ','
            << tick.target_contract_residual << ',' << tick.core_position << ','
            << tick.core_momentum << ',' << tick.core_phase << ','
            << tick.core_ratio << ',' << tick.field_total << ','
            << tick.cumulative_field[radius] << ',' << tick.field_mean_radius
            << ',' << tick.field_rms_radius << ',' << tick.radius_50 << ','
            << tick.radius_90 << ',' << tick.radius_99 << ','
            << tick.boundary_transport_into[radius] << ','
            << tick.source_exchange_into_field[radius] << ','
            << outward[radius] << ',' << inward[radius] << ','
            << arm.arrival_tick[radius] << ',' << tick.source_support_radius
            << ',' << tick.profile_residual << ',' << tick.regional_residual
            << ',' << tick.source_exchange_span << ',' << tick.energy_drift
            << ',' << tick.common_residual << ',' << (tick.valid ? 1 : 0)
            << '\n';
      }
    }
  }
}

}  // namespace

int main() {
  const bool parent = basin_parent_fingerprint();
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  ftd::eft::ConnectedMooreBlockOptions options;
  options.allow_shared_anchor_chart = true;
  options.use_sparse_local_current = true;
  options.use_local_residual_evaluation =
      FTD_SEPARATION_USE_LOCAL_RESIDUAL != 0;
  FullModes modes;
  if (parent && normalization.valid) {
    const auto reference = load_refined_state(0);
    const auto analytic = analytic_at(
        "causal_excitation_separation_v1", 0, reference,
        normalization.mapped_field_work_coefficient, options);
    if (analytic.valid) modes = full_modes(analytic.hessian);
  }

  const auto control_initial = separation_reference();
  const double origin_coordinate =
      0.5 * static_cast<double>(separation_volume - 1);
  const Vec3 origin{origin_coordinate, origin_coordinate, origin_coordinate};
  const double omega = modes.valid
      ? 0.5 * (modes.modes[6].omega + modes.modes[7].omega) : 0.0;
  const double internal_phase = modes.valid
      ? 0.5 * (modes.modes[6].phase + modes.modes[7].phase) : 0.0;
  const auto profile_preflight =
      ftd::eft::observe_component_aware_radial_field_profile(
          control_initial.electric, control_initial.magnetic_half,
          control_initial.electric, control_initial.magnetic_half,
          origin, normalization.mapped_field_work_coefficient,
          options.wave_speed, 1e-12);
  const auto basin_preflight = ftd::eft::observe_localized_basin(
      control_initial, control_initial, origin, separation_source_radius,
      separation_radii.back(), omega,
      normalization.mapped_field_work_coefficient, options.wave_speed,
      ftd::M_INERTIAL, 1e-12);
  const Vec3 center_residual = center(control_initial) - origin;
  const bool preflight = separation_contact_tick == separation_horizon + 1
      && profile_preflight.valid && profile_preflight.zero_profile
      && basin_preflight.valid && basin_preflight.core_phase_metric == 0.0
      && center_residual.mag() <= FTD_SEPARATION_CENTER_TOLERANCE;

  std::array<SeparationArm, 2> arms;
  arms[0].sign = -1;
  arms[1].sign = +1;
  std::array<ftd::eft::ConnectedMooreBlockState, 3> initial;
  initial[0] = control_initial;
  if (modes.valid && modes.modes[6].group == modes.modes[7].group) {
    initial[1] = separation_excitation(control_initial, modes, -1);
    initial[2] = separation_excitation(control_initial, modes, +1);
  }
  bool initial_fields_equal = preflight;
  for (int path = 1; path < 3 && initial_fields_equal; ++path)
    initial_fields_equal = equal_face_bits(initial[0].electric,
        initial[path].electric)
        && equal_edge_bits(initial[0].magnetic_half,
                           initial[path].magnetic_half);

  const auto basis = modes.valid ? donor_modes(modes)
                                 : std::vector<ftd::eft::ConnectedTangentMode>{};
  for (int sign = 0; sign < 2 && initial_fields_equal; ++sign) {
    const int path = sign + 1;
    arms[sign].maximum_momentum = maximum_momentum(initial[path]);
    const auto reservoir = ftd::eft::evaluate_connected_reservoir_decomposition(
        initial[0], initial[path], basis, {6, 7},
        normalization.mapped_field_work_coefficient, options, 1e-10);
    const auto localized = ftd::eft::observe_localized_basin(
        initial[0], initial[path], origin, separation_source_radius,
        separation_radii.back(), omega,
        normalization.mapped_field_work_coefficient, options.wave_speed,
        ftd::M_INERTIAL, 1e-12);
    arms[sign].initial_target = reservoir.target_mode_energy;
    arms[sign].initial_core = localized.core_phase_metric;
    arms[sign].initialized = reservoir.valid && localized.valid
        && arms[sign].initial_target > 0.0 && arms[sign].initial_core > 0.0
        && std::abs(arms[sign].maximum_momentum - separation_momentum)
               <= 1e-15;
  }

  std::array<ftd::eft::ConnectedMooreBlockState, 3> state = initial;
  std::array<double, 3> initial_energy{};
  std::array<std::vector<int>, 3> initial_sector;
  for (int path = 0; path < 3; ++path) {
    initial_energy[path] = energy_parts(
        state[path], normalization.mapped_field_work_coefficient,
        options).total;
    initial_sector[path] = sector_signature(state[path]);
  }
  for (int sign = 0; sign < 2; ++sign) {
    arms[sign].sector = true;
    auto record = observe_separation_tick(
        0, state[0], state[sign + 1], state[0], state[sign + 1],
        nullptr, nullptr, basis, arms[sign].initial_target,
        arms[sign].initial_core, omega,
        normalization.mapped_field_work_coefficient, origin, options,
        0.0, 0.0);
    arms[sign].ticks.push_back(std::move(record));
  }
  FTD_SEPARATION_EXTRA_OBSERVE(
      0, state[0], state[1], state[2], nullptr, nullptr, nullptr, options);

  std::array<ftd::eft::ConnectedMooreBlockSolveCache, 3> forward_cache;
  std::array<ftd::eft::ConnectedMooreBlockSolveCache, 3> reverse_cache;
  bool forward = preflight && initial_fields_equal
      && arms[0].initialized && arms[1].initialized;
  for (int tick = 1; tick <= separation_horizon && forward; ++tick) {
    std::array<ftd::eft::ConnectedMooreBlockStepResult, 3> steps;
#pragma omp parallel for num_threads(3)
    for (int path = 0; path < 3; ++path)
      steps[path] = ftd::eft::solve_connected_moore_block_forward(
          state[path], options, &forward_cache[path]);
    double common = 0.0;
    for (int path = 0; path < 3; ++path) {
      common = std::max(common, common_residual(steps[path]));
      if (!steps[path].valid || !steps[path].common_action_gates_pass
          || common_residual(steps[path]) > 1e-10
          || (FTD_SEPARATION_USE_LOCAL_RESIDUAL != 0
              && (steps[path].solve.full_candidate_materializations != 1
                  || steps[path].solve.materialized_residual_difference
                         > 1e-14))) {
        forward = false;
        break;
      }
    }
    if (!forward) break;

    std::array<double, 3> drift{};
    for (int path = 0; path < 3; ++path)
      drift[path] = std::abs(energy_parts(
          steps[path].later, normalization.mapped_field_work_coefficient,
          options).total - initial_energy[path]);
    std::array<double, 2> source_exchange{};
#pragma omp parallel for num_threads(2)
    for (int sign = 0; sign < 2; ++sign) {
      const int path = sign + 1;
      source_exchange[sign] = separation_global_source_exchange(
          state[0], state[path], steps[0].later, steps[path].later,
          normalization.mapped_field_work_coefficient,
          arms[sign].initial_target, options.wave_speed);
      for (std::size_t radius = 0; radius < separation_radii.size(); ++radius)
        arms[sign].block_source_exchange[radius] += source_exchange[sign];
      arms[sign].maximum_energy_drift = std::max(
          arms[sign].maximum_energy_drift, std::max(drift[0], drift[path]));
      arms[sign].maximum_common_residual = std::max(
          arms[sign].maximum_common_residual, common);
      arms[sign].sector = arms[sign].sector
          && sector_signature(steps[path].later) == initial_sector[path];
    }
    const bool sampled = tick % FTD_SEPARATION_OBSERVATION_CADENCE == 0;
    if (sampled) {
#pragma omp parallel for num_threads(2)
      for (int sign = 0; sign < 2; ++sign) {
      const int path = sign + 1;
      auto record = observe_separation_tick(
          tick, state[0], state[path], steps[0].later, steps[path].later,
          &steps[0], &steps[path], basis, arms[sign].initial_target,
          arms[sign].initial_core, omega,
          normalization.mapped_field_work_coefficient, origin, options,
          std::max(drift[0], drift[path]), common);
      for (std::size_t radius = 0; radius < separation_radii.size(); ++radius) {
        record.source_exchange_into_field[radius] =
            arms[sign].block_source_exchange[radius];
        record.boundary_transport_into[radius] =
            record.regional_energy_after[radius]
            - arms[sign].last_regional_energy[radius]
            - arms[sign].block_source_exchange[radius];
        record.source_exchange_span = 0.0;
        arms[sign].last_regional_energy[radius] =
            record.regional_energy_after[radius];
        arms[sign].block_source_exchange[radius] = 0.0;
      }
      arms[sign].maximum_profile_residual = std::max(
          arms[sign].maximum_profile_residual, record.profile_residual);
      arms[sign].maximum_regional_residual = std::max(
          arms[sign].maximum_regional_residual, record.regional_residual);
      arms[sign].maximum_source_exchange_span = std::max(
          arms[sign].maximum_source_exchange_span,
          record.source_exchange_span);
      arms[sign].maximum_target_contract_residual = std::max(
          arms[sign].maximum_target_contract_residual,
          record.target_contract_residual);
      for (std::size_t radius = 0; radius < separation_radii.size(); ++radius) {
        arms[sign].cumulative_outward[radius] += std::max(
            -record.boundary_transport_into[radius], 0.0);
        arms[sign].cumulative_inward[radius] += std::max(
            record.boundary_transport_into[radius], 0.0);
        if (arms[sign].arrival_tick[radius] < 0
            && arms[sign].cumulative_outward[radius]
                   >= separation_arrival_threshold)
          arms[sign].arrival_tick[radius] = tick;
      }
      arms[sign].ticks.push_back(std::move(record));
    }
    }
    if (sampled)
      for (const auto& arm : arms)
        if (arm.ticks.empty() || !arm.ticks.back().valid) forward = false;
    FTD_SEPARATION_EXTRA_OBSERVE(
        tick, steps[0].later, steps[1].later, steps[2].later,
        &steps[0], &steps[1], &steps[2], options);
    for (int path = 0; path < 3; ++path) state[path] = std::move(steps[path].later);
    if (tick % FTD_SEPARATION_PROGRESS_INTERVAL == 0)
      std::cout << "completed separation tick " << tick << std::endl;
  }

  bool reverse = forward;
  for (int tick = separation_horizon; tick >= 1 && reverse; --tick) {
    std::array<ftd::eft::ConnectedMooreBlockStepResult, 3> steps;
#pragma omp parallel for num_threads(3)
    for (int path = 0; path < 3; ++path)
      steps[path] = ftd::eft::solve_connected_moore_block_reverse(
          state[path], options, &reverse_cache[path]);
    for (int path = 0; path < 3; ++path) {
      const double residual = common_residual(steps[path]);
      for (auto& arm : arms)
        arm.maximum_common_residual = std::max(
            arm.maximum_common_residual, residual);
      if (!steps[path].valid || !steps[path].common_action_gates_pass
          || residual > 1e-10
          || (FTD_SEPARATION_USE_LOCAL_RESIDUAL != 0
              && (steps[path].solve.full_candidate_materializations != 1
                  || steps[path].solve.materialized_residual_difference
                         > 1e-14))) {
        reverse = false;
        break;
      }
      state[path] = std::move(steps[path].earlier);
    }
    if (tick % FTD_SEPARATION_PROGRESS_INTERVAL == 0)
      std::cout << "reversed separation tick " << tick << std::endl;
  }

  for (int sign = 0; sign < 2; ++sign) {
    const int path = sign + 1;
    auto& arm = arms[sign];
    arm.forward = forward && arm.ticks.size()
        == static_cast<std::size_t>(
            separation_horizon / FTD_SEPARATION_OBSERVATION_CADENCE + 1);
    arm.reverse = reverse;
    arm.recovery = std::max(
        ftd::eft::connected_moore_block_state_max_difference(
            initial[0], state[0]),
        ftd::eft::connected_moore_block_state_max_difference(
            initial[path], state[path]));
    classify_separation(arm);
    arm.exact = arm.initialized && arm.forward && arm.reverse && arm.sector
        && arm.maximum_profile_residual <= 1e-12
        && arm.maximum_regional_residual <= 1e-10
        && arm.maximum_source_exchange_span <= 1e-10
        && arm.maximum_energy_drift <= 1e-10
        && arm.maximum_common_residual <= 1e-10
        && arm.maximum_target_contract_residual <= 1e-12
        && arm.recovery <= 1e-8;
  }

  const double history_rms = separation_core_history_rms(arms);
  const double fraction_difference = separation_final_fraction_difference(arms);
  bool arrival_agreement = true;
  for (std::size_t radius = 0; radius < separation_radii.size(); ++radius) {
    const int left = arms[0].arrival_tick[radius];
    const int right = arms[1].arrival_tick[radius];
    arrival_agreement = arrival_agreement
        && ((left < 0 && right < 0)
            || (left >= 0 && right >= 0 && std::abs(left - right) <= 1));
  }
  double speed_difference = 0.0;
  bool speed_agreement = arms[0].shell_fit.valid == arms[1].shell_fit.valid;
  if (arms[0].shell_fit.valid && arms[1].shell_fit.valid
      && arms[0].shell_fit.slope > 0.0 && arms[1].shell_fit.slope > 0.0) {
    const double left = 1.0 / arms[0].shell_fit.slope;
    const double right = 1.0 / arms[1].shell_fit.slope;
    speed_difference = std::abs(left - right)
        / std::max({std::abs(left), std::abs(right), 1e-300});
  } else if (arms[0].shell_fit.valid || arms[1].shell_fit.valid) {
    speed_agreement = false;
  }
  const bool polarity = history_rms <= 1e-4
      && fraction_difference <= 1e-4 && arrival_agreement
      && speed_agreement && speed_difference <= 1e-4
      && arms[0].spatial_class == arms[1].spatial_class
      && arms[0].late_class == arms[1].late_class;
  const bool exact = parent && preflight && initial_fields_equal
      && arms[0].exact && arms[1].exact && polarity;
  const std::string verdict = exact
      ? arms[0].spatial_class + " + " + arms[0].late_class
      : "CAUSAL_EXCITATION_SEPARATION_EXECUTION_INVALID";
  FTD_SEPARATION_EXTRA_FINALIZE(exact, internal_phase, arms);
  write_separation(arms, parent, preflight, initial_fields_equal,
      history_rms, fraction_difference, verdict);

  std::cout << std::setprecision(17)
            << "protocol_sha256=" << separation_protocol_sha256 << '\n'
            << "verdict=" << verdict << '\n'
            << "execution=" << exact
            << " history_rms=" << history_rms
            << " final_fraction_difference=" << fraction_difference << '\n';
  for (const auto& arm : arms) {
    const double shell_speed = arm.shell_fit.valid && arm.shell_fit.slope > 0.0
        ? 1.0 / arm.shell_fit.slope : -1.0;
    std::cout << "sign=" << arm.sign
              << " spatial=" << arm.spatial_class
              << " late=" << arm.late_class
              << " arrivals=";
    for (int tick : arm.arrival_tick) std::cout << tick << ':';
    std::cout << " shell_speed=" << shell_speed
              << " shell_r2=" << arm.shell_fit.r_squared
              << " late_core_mean=" << arm.late_core.mean
              << " late_near_mean=" << arm.late_near.mean
              << " recovery=" << arm.recovery << '\n';
  }
  return exact && FTD_SEPARATION_EXTRA_EXECUTION_VALID ? 0 : 1;
}
