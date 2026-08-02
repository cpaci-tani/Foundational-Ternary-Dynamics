// FTD-0678: fresh localized-basin relaxation discriminator.
#define FTD_0676_EMBEDDED
#include "test_canonical_precontact_mode_decay.cpp"
#undef FTD_0676_EMBEDDED

#include "ftd/eft/localized_basin_observer.h"

namespace {

constexpr char basin_protocol_sha256[] =
    "3876E9CBF7017E68426C26E6829D4751513F183D3CA3EB6536C73E0718FFD156";
constexpr char basin_parent_json_sha256[] =
    "6592E523EDDC37648A39FE39CFF02FF4371555CAEF6DE830D822114D98858206";
constexpr char basin_parent_csv_sha256[] =
    "D1BB98C6C178201D9B8A289FD5E3026439D57239BEDDE235FE9010A44B888AA4";
constexpr double basin_momentum = 2.5e-7;
constexpr int basin_horizon = 80;
constexpr int basin_fit_start = 8;
constexpr int basin_fit_end = 64;
constexpr int basin_inner_radius = 8;
constexpr int basin_outer_radius = 24;

struct BasinTick {
  int tick = 0;
  bool valid = false;
  double target = 0.0;
  double target_ratio = INFINITY;
  double core_position = INFINITY;
  double core_momentum = INFINITY;
  double core_phase = INFINITY;
  double core_ratio = INFINITY;
  double center_offset = INFINITY;
  double mean_momentum_offset = INFINITY;
  double edge_difference = INFINITY;
  double near_field = INFINITY;
  double intermediate_field = INFINITY;
  double far_field = INFINITY;
  double total_field = INFINITY;
  double near_fraction = INFINITY;
  double far_fraction = INFINITY;
  double partition_residual = INFINITY;
  double reservoir_residual = INFINITY;
};

struct BasinFit {
  bool valid = false;
  int samples = 0;
  double intercept = 0.0;
  double slope = 0.0;
  double gamma = 0.0;
  double rss_constant = INFINITY;
  double rss_linear = INFINITY;
  double delta_bic = -INFINITY;
  double r_squared = -INFINITY;
};

struct BasinArm {
  int sign = 0;
  bool initialized = false;
  bool forward = false;
  bool reverse = false;
  bool sector = false;
  bool exact = false;
  double maximum_momentum = 0.0;
  double initial_target = 0.0;
  double initial_core = 0.0;
  double initial_identity_residual = INFINITY;
  double maximum_partition_residual = 0.0;
  double maximum_reservoir_residual = 0.0;
  double maximum_energy_drift = 0.0;
  double maximum_common_residual = 0.0;
  double recovery = INFINITY;
  double decline = -INFINITY;
  BasinFit fit{};
  std::vector<BasinTick> ticks;
};

bool basin_parent_fingerprint() {
  const auto root = std::filesystem::path(__FILE__).parent_path().parent_path();
  return file_contains(
             root / "results/ftd_0676/ftd_0676_canonical_precontact_mode_decay_v1.json",
             decay_protocol_sha256,
             "CANONICAL_PRECONTACT_EXPONENTIAL_TRANSFER_CONSTRUCTIVE")
      && file_contains(
             root / "results/ftd_0676/ftd_0676_canonical_precontact_mode_decay_ticks_v1.csv",
             "ftd_id,protocol_sha256,sign,tick,target",
             "FTD-0676");
}

ftd::eft::ConnectedMooreBlockState basin_excitation(
    const ftd::eft::ConnectedMooreBlockState& reference,
    const FullModes& modes,
    int sign) {
  double nominal = 0.0;
  auto state = regional_excitation(reference, modes, sign, nominal);
  const double actual = maximum_momentum(state);
  const double scale = actual > 0.0 ? basin_momentum / actual : 0.0;
  for (auto& constituent : state.constituents)
    constituent.momentum *= scale;
  return state;
}

BasinTick observe_basin(
    int tick,
    const ftd::eft::ConnectedMooreBlockState& control,
    const ftd::eft::ConnectedMooreBlockState& excited,
    const std::vector<ftd::eft::ConnectedTangentMode>& basis,
    double initial_target,
    double initial_core,
    double omega,
    double beta,
    const ftd::eft::ConnectedMooreBlockOptions& options) {
  BasinTick result;
  result.tick = tick;
  const auto reservoir = observe_donor(
      tick, control, excited, basis, initial_target, beta, options);
  const auto localized = ftd::eft::observe_localized_basin(
      control, excited, center(control), basin_inner_radius,
      basin_outer_radius, omega, beta, options.wave_speed,
      ftd::M_INERTIAL, 1e-12);
  // observe_donor() returns the target ratio, not the dimensional energy.
  result.target_ratio = reservoir.target;
  result.target = initial_target * reservoir.target;
  result.core_position = localized.internal_position_metric;
  result.core_momentum = localized.internal_momentum_metric;
  result.core_phase = localized.core_phase_metric;
  result.core_ratio = initial_core > 0.0
      ? result.core_phase / initial_core : INFINITY;
  result.center_offset = localized.center_offset_norm;
  result.mean_momentum_offset = localized.mean_momentum_offset_norm;
  result.edge_difference = localized.maximum_edge_length_difference;
  result.near_field = localized.near_dynamic_field;
  result.intermediate_field = localized.intermediate_dynamic_field;
  result.far_field = localized.far_dynamic_field;
  result.total_field = localized.total_dynamic_field;
  result.near_fraction = localized.near_fraction;
  result.far_fraction = localized.far_fraction;
  result.partition_residual = localized.field_partition_residual;
  result.reservoir_residual = reservoir.observer_residual;
  result.valid = reservoir.valid && localized.valid
      && std::isfinite(result.core_ratio)
      && std::isfinite(result.target_ratio);
  return result;
}

const BasinTick* basin_tick(const BasinArm& arm, int tick) {
  for (const auto& record : arm.ticks)
    if (record.tick == tick) return &record;
  return nullptr;
}

BasinFit fit_basin(const BasinArm& arm) {
  BasinFit fit;
  std::vector<std::pair<double, double>> samples;
  for (const auto& tick : arm.ticks) {
    if (tick.tick < basin_fit_start || tick.tick > basin_fit_end) continue;
    if (!tick.valid || !(tick.core_ratio > 0.0)
        || !std::isfinite(tick.core_ratio)) return fit;
    samples.emplace_back(static_cast<double>(tick.tick),
                         std::log(tick.core_ratio));
  }
  if (samples.size()
      != static_cast<std::size_t>(basin_fit_end - basin_fit_start + 1))
    return fit;
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
  if (!(denominator > 0.0)) return fit;
  fit.slope = (n * sxy - sx * sy) / denominator;
  fit.intercept = (sy - fit.slope * sx) / n;
  const double mean_y = sy / n;
  fit.rss_constant = 0.0;
  fit.rss_linear = 0.0;
  for (const auto& [x, y] : samples) {
    const double constant_residual = y - mean_y;
    const double linear_residual = y - fit.intercept - fit.slope * x;
    fit.rss_constant += constant_residual * constant_residual;
    fit.rss_linear += linear_residual * linear_residual;
  }
  if (!(fit.rss_constant > 0.0) || !(fit.rss_linear > 0.0)) return fit;
  fit.samples = static_cast<int>(samples.size());
  fit.gamma = -fit.slope;
  fit.r_squared = 1.0 - fit.rss_linear / fit.rss_constant;
  fit.delta_bic = n * std::log(fit.rss_constant / n) + std::log(n)
      - (n * std::log(fit.rss_linear / n) + 2.0 * std::log(n));
  fit.valid = std::isfinite(fit.gamma) && std::isfinite(fit.delta_bic)
      && std::isfinite(fit.r_squared);
  return fit;
}

std::array<BasinArm, 2> run_basin(
    const FullModes& full,
    double beta,
    const ftd::eft::ConnectedMooreBlockOptions& options,
    bool& initial_fields_equal,
    bool& observer_preflight) {
  std::array<BasinArm, 2> arms;
  arms[0].sign = -1;
  arms[1].sign = +1;
  const auto basis = donor_modes(full);
  const double omega = 0.5 * (
      full.modes[6].omega + full.modes[7].omega);
  const auto control_initial = regional_reference();
  const auto preflight = ftd::eft::observe_localized_basin(
      control_initial, control_initial, center(control_initial),
      basin_inner_radius, basin_outer_radius, omega, beta,
      options.wave_speed, ftd::M_INERTIAL, 1e-12);
  observer_preflight = preflight.valid && preflight.core_phase_metric == 0.0
      && preflight.total_dynamic_field == 0.0;

  std::array<ftd::eft::ConnectedMooreBlockState, 3> initial;
  initial[0] = control_initial;
  for (int sign = 0; sign < 2; ++sign)
    initial[sign + 1] = basin_excitation(
        control_initial, full, arms[sign].sign);

  initial_fields_equal = observer_preflight;
  for (int path = 1; path < 3 && initial_fields_equal; ++path)
    initial_fields_equal = equal_face_bits(initial[0].electric,
        initial[path].electric)
        && equal_edge_bits(initial[0].magnetic_half,
                           initial[path].magnetic_half);
  if (!initial_fields_equal) return arms;

  for (int sign = 0; sign < 2; ++sign) {
    const int path = sign + 1;
    arms[sign].maximum_momentum = maximum_momentum(initial[path]);
    const auto reservoir = ftd::eft::evaluate_connected_reservoir_decomposition(
        initial[0], initial[path], basis, {6, 7}, beta, options, 1e-10);
    const auto localized = ftd::eft::observe_localized_basin(
        initial[0], initial[path], center(initial[0]), basin_inner_radius,
        basin_outer_radius, omega, beta, options.wave_speed,
        ftd::M_INERTIAL, 1e-12);
    arms[sign].initial_target = reservoir.target_mode_energy;
    arms[sign].initial_core = localized.core_phase_metric;
    const double identity_scale = std::max({
        std::abs(arms[sign].initial_core),
        2.0 * std::abs(arms[sign].initial_target), 1e-300});
    arms[sign].initial_identity_residual = std::abs(
        arms[sign].initial_core - 2.0 * arms[sign].initial_target)
        / identity_scale;
    arms[sign].initialized = reservoir.valid && localized.valid
        && arms[sign].initial_target > 0.0
        && arms[sign].initial_core > 0.0
        && std::abs(arms[sign].maximum_momentum - basin_momentum) <= 1e-15
        && arms[sign].initial_identity_residual <= 1e-8;
  }
  if (!arms[0].initialized || !arms[1].initialized) return arms;

  std::array<ftd::eft::ConnectedMooreBlockState, 3> state = initial;
  std::array<double, 3> initial_energy{};
  std::array<std::vector<int>, 3> initial_sector;
  for (int path = 0; path < 3; ++path) {
    initial_energy[path] = energy_parts(state[path], beta, options).total;
    initial_sector[path] = sector_signature(state[path]);
  }
  for (int sign = 0; sign < 2; ++sign) {
    arms[sign].sector = true;
    auto tick = observe_basin(0, state[0], state[sign + 1], basis,
        arms[sign].initial_target, arms[sign].initial_core, omega, beta,
        options);
    arms[sign].maximum_partition_residual = tick.partition_residual;
    arms[sign].maximum_reservoir_residual = tick.reservoir_residual;
    arms[sign].ticks.push_back(std::move(tick));
  }

  std::array<ftd::eft::ConnectedMooreBlockSolveCache, 3> forward_cache;
  std::array<ftd::eft::ConnectedMooreBlockSolveCache, 3> reverse_cache;
  bool forward = true;
  for (int tick = 1; tick <= basin_horizon && forward; ++tick) {
    std::array<ftd::eft::ConnectedMooreBlockStepResult, 3> steps;
#pragma omp parallel for num_threads(3)
    for (int path = 0; path < 3; ++path)
      steps[path] = ftd::eft::solve_connected_moore_block_forward(
          state[path], options, &forward_cache[path]);
    double common = 0.0;
    for (int path = 0; path < 3; ++path) {
      common = std::max(common, common_residual(steps[path]));
      if (!steps[path].valid || !steps[path].common_action_gates_pass
          || common_residual(steps[path]) > 1e-10) {
        forward = false;
        break;
      }
      state[path] = steps[path].later;
      for (auto& arm : arms)
        arm.sector = arm.sector
            && sector_signature(state[path]) == initial_sector[path];
    }
    if (!forward) break;

    const double control_energy = energy_parts(state[0], beta, options).total;
#pragma omp parallel for num_threads(2)
    for (int sign = 0; sign < 2; ++sign) {
      const int path = sign + 1;
      auto record = observe_basin(tick, state[0], state[path], basis,
          arms[sign].initial_target, arms[sign].initial_core, omega, beta,
          options);
      arms[sign].maximum_partition_residual = std::max(
          arms[sign].maximum_partition_residual,
          record.partition_residual);
      arms[sign].maximum_reservoir_residual = std::max(
          arms[sign].maximum_reservoir_residual,
          record.reservoir_residual);
      arms[sign].maximum_common_residual = std::max(
          arms[sign].maximum_common_residual, common);
      arms[sign].maximum_energy_drift = std::max({
          arms[sign].maximum_energy_drift,
          std::abs(control_energy - initial_energy[0]),
          std::abs(energy_parts(state[path], beta, options).total
                   - initial_energy[path])});
      arms[sign].ticks.push_back(std::move(record));
    }
    for (const auto& arm : arms)
      if (arm.ticks.empty() || !arm.ticks.back().valid) forward = false;
    if (tick % 10 == 0)
      std::cout << "completed basin tick " << tick << std::endl;
  }

  bool reverse = forward;
  for (int tick = basin_horizon; tick >= 1 && reverse; --tick) {
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
          || residual > 1e-10) {
        reverse = false;
        break;
      }
      state[path] = steps[path].earlier;
    }
    if (tick % 10 == 0)
      std::cout << "reversed basin tick " << tick << std::endl;
  }

  for (int sign = 0; sign < 2; ++sign) {
    const int path = sign + 1;
    auto& arm = arms[sign];
    arm.forward = forward && arm.ticks.size() == basin_horizon + 1;
    arm.reverse = reverse;
    arm.recovery = std::max(
        ftd::eft::connected_moore_block_state_max_difference(
            initial[0], state[0]),
        ftd::eft::connected_moore_block_state_max_difference(
            initial[path], state[path]));
    arm.fit = fit_basin(arm);
    const auto* start = basin_tick(arm, basin_fit_start);
    const auto* end = basin_tick(arm, basin_fit_end);
    if (start != nullptr && end != nullptr && start->core_ratio > 0.0)
      arm.decline = 1.0 - end->core_ratio / start->core_ratio;
    arm.exact = arm.initialized && arm.forward && arm.reverse && arm.sector
        && arm.maximum_partition_residual <= 1e-12
        && arm.maximum_energy_drift <= 1e-10
        && arm.maximum_common_residual <= 1e-10
        && arm.recovery <= 1e-8 && arm.fit.valid;
  }
  return arms;
}

double basin_history_rms(const std::array<BasinArm, 2>& arms) {
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

std::string classify_basin(const std::array<BasinArm, 2>& arms,
                           bool exact,
                           double rate_difference,
                           double history_rms,
                           double far_fraction_difference) {
  if (!exact) return "LOCALIZED_BASIN_RELAXATION_EXECUTION_INVALID";
  if (arms[0].decline < 0.20 || arms[1].decline < 0.20)
    return "LOCALIZED_BASIN_INTERNAL_RELAXATION_ABSENT";
  const bool exponential = arms[0].fit.gamma > 0.0
      && arms[1].fit.gamma > 0.0
      && arms[0].fit.delta_bic >= 10.0 && arms[1].fit.delta_bic >= 10.0
      && arms[0].fit.r_squared >= 0.995
      && arms[1].fit.r_squared >= 0.995;
  if (!exponential)
    return "LOCALIZED_BASIN_INTERNAL_RELAXATION_NONEXPONENTIAL";
  const auto* final_negative = basin_tick(arms[0], basin_horizon);
  const auto* final_positive = basin_tick(arms[1], basin_horizon);
  const bool remote = final_negative != nullptr && final_positive != nullptr
      && final_negative->far_field > final_negative->near_field
      && final_positive->far_field > final_positive->near_field
      && final_negative->far_field > 0.0 && final_positive->far_field > 0.0;
  if (!remote) return "LOCALIZED_BASIN_REMOTE_FIELD_NOT_DOMINANT";
  if (rate_difference > 1e-4 || history_rms > 1e-5
      || far_fraction_difference > 1e-4)
    return "LOCALIZED_BASIN_RELAXATION_SIGN_DEPENDENT";
  return "LOCALIZED_BASIN_RELAXATION_TOWARD_CONSTRUCTIVE";
}

void write_basin(const std::array<BasinArm, 2>& arms,
                 bool parent,
                 bool initial_fields_equal,
                 bool observer_preflight,
                 bool exact,
                 double rate_difference,
                 double history_rms,
                 double far_fraction_difference,
                 const std::string& verdict) {
  const auto directory = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results/ftd_0678";
  std::filesystem::create_directories(directory);
  std::ofstream json(
      directory / "ftd_0678_localized_basin_relaxation_v1.json");
  json << std::setprecision(17) << std::boolalpha
       << "{\n  \"ftd_id\": \"FTD-0678\",\n"
       << "  \"protocol_sha256\": \"" << basin_protocol_sha256 << "\",\n"
       << "  \"parent_json_sha256\": \"" << basin_parent_json_sha256 << "\",\n"
       << "  \"parent_csv_sha256\": \"" << basin_parent_csv_sha256 << "\",\n"
       << "  \"verdict\": \"" << verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"parent_pass\": " << parent << ",\n"
       << "  \"observer_preflight_pass\": " << observer_preflight << ",\n"
       << "  \"initial_fields_bitwise_equal\": "
       << initial_fields_equal << ",\n"
       << "  \"exact_execution_pass\": " << exact << ",\n"
       << "  \"volume\": " << regional_volume << ",\n"
       << "  \"horizon\": " << basin_horizon << ",\n"
       << "  \"fit_start_tick\": " << basin_fit_start << ",\n"
       << "  \"fit_end_tick\": " << basin_fit_end << ",\n"
       << "  \"inner_radius\": " << basin_inner_radius << ",\n"
       << "  \"outer_radius\": " << basin_outer_radius << ",\n"
       << "  \"maximum_constituent_momentum_amplitude\": "
       << basin_momentum << ",\n"
       << "  \"polarity_rate_relative_difference\": "
       << rate_difference << ",\n"
       << "  \"polarity_core_history_rms\": " << history_rms << ",\n"
       << "  \"polarity_far_fraction_difference\": "
       << far_fraction_difference << ",\n";
  for (int sign = 0; sign < 2; ++sign) {
    const auto& arm = arms[sign];
    const auto* final = basin_tick(arm, basin_horizon);
    const std::string prefix = arm.sign < 0 ? "negative" : "positive";
    json << "  \"" << prefix << "_exact\": " << arm.exact << ",\n"
         << "  \"" << prefix << "_maximum_momentum\": "
         << arm.maximum_momentum << ",\n"
         << "  \"" << prefix << "_initial_target\": "
         << arm.initial_target << ",\n"
         << "  \"" << prefix << "_initial_core\": "
         << arm.initial_core << ",\n"
         << "  \"" << prefix << "_initial_identity_residual\": "
         << arm.initial_identity_residual << ",\n"
         << "  \"" << prefix << "_gamma_core\": "
         << arm.fit.gamma << ",\n"
         << "  \"" << prefix << "_delta_bic\": "
         << arm.fit.delta_bic << ",\n"
         << "  \"" << prefix << "_r_squared\": "
         << arm.fit.r_squared << ",\n"
         << "  \"" << prefix << "_decline_tick8_tick64\": "
         << arm.decline << ",\n"
         << "  \"" << prefix << "_final_near_field\": "
         << (final != nullptr ? final->near_field : INFINITY) << ",\n"
         << "  \"" << prefix << "_final_far_field\": "
         << (final != nullptr ? final->far_field : INFINITY) << ",\n"
         << "  \"" << prefix << "_final_far_fraction\": "
         << (final != nullptr ? final->far_fraction : INFINITY) << ",\n"
         << "  \"" << prefix << "_max_partition_residual\": "
         << arm.maximum_partition_residual << ",\n"
         << "  \"" << prefix << "_max_reservoir_residual\": "
         << arm.maximum_reservoir_residual << ",\n"
         << "  \"" << prefix << "_max_energy_drift\": "
         << arm.maximum_energy_drift << ",\n"
         << "  \"" << prefix << "_max_common_residual\": "
         << arm.maximum_common_residual << ",\n"
         << "  \"" << prefix << "_recovery\": " << arm.recovery
         << (sign == 0 ? ",\n" : "\n");
  }
  json << "}\n";

  std::ofstream csv(
      directory / "ftd_0678_localized_basin_relaxation_ticks_v1.csv");
  csv << "ftd_id,protocol_sha256,sign,tick,target,target_ratio,"
         "core_position,core_momentum,core_phase,core_ratio,center_offset,"
         "mean_momentum_offset,edge_difference,near_field,intermediate_field,"
         "far_field,total_field,near_fraction,far_fraction,partition_residual,"
         "reservoir_residual,valid\n";
  for (const auto& arm : arms)
    for (const auto& tick : arm.ticks)
      csv << std::setprecision(17) << "FTD-0678," << basin_protocol_sha256
          << ',' << arm.sign << ',' << tick.tick << ',' << tick.target << ','
          << tick.target_ratio << ',' << tick.core_position << ','
          << tick.core_momentum << ',' << tick.core_phase << ','
          << tick.core_ratio << ',' << tick.center_offset << ','
          << tick.mean_momentum_offset << ',' << tick.edge_difference << ','
          << tick.near_field << ',' << tick.intermediate_field << ','
          << tick.far_field << ',' << tick.total_field << ','
          << tick.near_fraction << ',' << tick.far_fraction << ','
          << tick.partition_residual << ',' << tick.reservoir_residual << ','
          << (tick.valid ? 1 : 0) << '\n';
}

}  // namespace

#ifndef FTD_0678_EMBEDDED
int main() {
  const bool parent = basin_parent_fingerprint();
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  ftd::eft::ConnectedMooreBlockOptions options;
  options.allow_shared_anchor_chart = true;
  options.use_sparse_local_current = true;
  FullModes modes;
  if (parent && normalization.valid) {
    const auto reference = load_refined_state(0);
    const auto analytic = analytic_at(
        "localized_basin_relaxation", 0, reference,
        normalization.mapped_field_work_coefficient, options);
    if (analytic.valid) modes = full_modes(analytic.hessian);
  }
  bool initial_fields_equal = false;
  bool observer_preflight = false;
  std::array<BasinArm, 2> arms;
  if (modes.valid && modes.modes[6].group == modes.modes[7].group)
    arms = run_basin(modes, normalization.mapped_field_work_coefficient,
        options, initial_fields_equal, observer_preflight);
  const bool exact = parent && normalization.valid && modes.valid
      && modes.modes[6].group == modes.modes[7].group
      && initial_fields_equal && observer_preflight
      && arms[0].exact && arms[1].exact;
  const double rate_scale = std::max({
      std::abs(arms[0].fit.gamma), std::abs(arms[1].fit.gamma), 1e-300});
  const double rate_difference = std::abs(
      arms[0].fit.gamma - arms[1].fit.gamma) / rate_scale;
  const double history_rms = basin_history_rms(arms);
  const auto* final_negative = basin_tick(arms[0], basin_horizon);
  const auto* final_positive = basin_tick(arms[1], basin_horizon);
  const double far_fraction_difference = final_negative != nullptr
          && final_positive != nullptr
      ? std::abs(final_negative->far_fraction - final_positive->far_fraction)
      : INFINITY;
  const std::string verdict = classify_basin(
      arms, exact, rate_difference, history_rms, far_fraction_difference);
  write_basin(arms, parent, initial_fields_equal, observer_preflight, exact,
              rate_difference, history_rms, far_fraction_difference, verdict);
  std::cout << std::setprecision(17)
            << "protocol_sha256=" << basin_protocol_sha256 << '\n'
            << "verdict=" << verdict << '\n'
            << "execution=" << exact
            << " rate_difference=" << rate_difference
            << " history_rms=" << history_rms
            << " far_fraction_difference=" << far_fraction_difference << '\n';
  for (const auto& arm : arms) {
    const auto* final = basin_tick(arm, basin_horizon);
    std::cout << "sign=" << arm.sign
              << " pmax=" << arm.maximum_momentum
              << " identity=" << arm.initial_identity_residual
              << " gamma_core=" << arm.fit.gamma
              << " delta_bic=" << arm.fit.delta_bic
              << " r2=" << arm.fit.r_squared
              << " decline=" << arm.decline
              << " final_near="
              << (final != nullptr ? final->near_field : INFINITY)
              << " final_far="
              << (final != nullptr ? final->far_field : INFINITY)
              << " far_fraction="
              << (final != nullptr ? final->far_fraction : INFINITY)
              << " partition=" << arm.maximum_partition_residual
              << " inverse=" << arm.recovery << '\n';
  }
  return verdict == "LOCALIZED_BASIN_RELAXATION_EXECUTION_INVALID" ? 1 : 0;
}
#endif
