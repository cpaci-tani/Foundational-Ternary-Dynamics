// FTD-0676: held-out canonical pre-contact mode-decay discriminator.
#define FTD_0674_EMBEDDED
#include "test_recovery_reservoir_donor.cpp"
#undef FTD_0674_EMBEDDED

namespace {

constexpr char decay_protocol_sha256[] =
    "1DCD7CEB1FCF429FDF63CE7251D713C76D9E5B9F80DBD75B4D061E715564A6B6";
constexpr char decay_parent_json_sha256[] =
    "1848283E5AF91B076E7DD69CB24B4677159FED8594F2C78A5D8D858F441044CB";
constexpr char decay_parent_csv_sha256[] =
    "DEA0582DD2E135071524CBAB6F532A74FCCEE49D2F88E163966E6CC6DE4364E9";
constexpr double parent_energy_decay_rate = 0.006537123419844565;
constexpr double decay_momentum = 5e-7;
constexpr int decay_horizon = 80;
constexpr int fit_start = 8;
constexpr int fit_end = 64;

struct DecayFit {
  bool valid = false;
  int samples = 0;
  double intercept = 0.0;
  double slope = 0.0;
  double gamma_energy = 0.0;
  double rss_constant = INFINITY;
  double rss_linear = INFINITY;
  double bic_constant = INFINITY;
  double bic_linear = INFINITY;
  double delta_bic = -INFINITY;
  double r_squared = -INFINITY;
};

struct DecayArm {
  int sign = 0;
  bool initialized = false;
  bool forward = false;
  bool reverse = false;
  bool sector = false;
  bool exact = false;
  double maximum_momentum = 0.0;
  double initial_target = 0.0;
  double max_energy_drift = 0.0;
  double max_common = 0.0;
  double max_observer = 0.0;
  double recovery = INFINITY;
  double decline = -INFINITY;
  double parent_rate_relative_difference = INFINITY;
  DecayFit fit{};
  std::vector<DonorTick> ticks;
};

bool decay_parent_fingerprint() {
  const auto root = std::filesystem::path(__FILE__).parent_path().parent_path();
  return file_contains(
             root / "results/ftd_0674/ftd_0674_recovery_reservoir_donor_v1.json",
             donor_protocol_sha256,
             "RECOVERY_RESERVOIR_DONOR_EXECUTION_INVALID")
      && file_contains(
             root / "results/ftd_0674/ftd_0674_recovery_reservoir_donor_ticks_v1.csv",
             "ftd_id,protocol_sha256,sign,tick,target",
             "FTD-0674");
}

ftd::eft::ConnectedMooreBlockState decay_excitation(
    const ftd::eft::ConnectedMooreBlockState& reference,
    const FullModes& modes,
    int sign) {
  double nominal = 0.0;
  auto state = regional_excitation(reference, modes, sign, nominal);
  const double actual = maximum_momentum(state);
  const double scale = actual > 0.0 ? decay_momentum / actual : 0.0;
  for (auto& constituent : state.constituents)
    constituent.momentum *= scale;
  return state;
}

const DonorTick* decay_tick(const DecayArm& arm, int tick) {
  for (const auto& record : arm.ticks)
    if (record.tick == tick) return &record;
  return nullptr;
}

DecayFit fit_decay(const DecayArm& arm) {
  DecayFit fit;
  std::vector<std::pair<double, double>> samples;
  for (const auto& tick : arm.ticks) {
    if (tick.tick < fit_start || tick.tick > fit_end) continue;
    if (!tick.valid || !(tick.target > 0.0)
        || !std::isfinite(tick.target)) return fit;
    samples.emplace_back(static_cast<double>(tick.tick),
                         std::log(tick.target));
  }
  if (samples.size() != static_cast<std::size_t>(fit_end - fit_start + 1))
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
  const double mean = sy / n;
  fit.rss_constant = 0.0;
  fit.rss_linear = 0.0;
  for (const auto& [x, y] : samples) {
    const double constant_residual = y - mean;
    const double linear_residual = y - fit.intercept - fit.slope * x;
    fit.rss_constant += constant_residual * constant_residual;
    fit.rss_linear += linear_residual * linear_residual;
  }
  if (!(fit.rss_constant > 0.0) || !(fit.rss_linear > 0.0)) return fit;
  fit.samples = static_cast<int>(samples.size());
  fit.gamma_energy = -fit.slope;
  fit.r_squared = 1.0 - fit.rss_linear / fit.rss_constant;
  fit.bic_constant = n * std::log(fit.rss_constant / n) + std::log(n);
  fit.bic_linear = n * std::log(fit.rss_linear / n) + 2.0 * std::log(n);
  fit.delta_bic = fit.bic_constant - fit.bic_linear;
  fit.valid = std::isfinite(fit.gamma_energy)
      && std::isfinite(fit.delta_bic) && std::isfinite(fit.r_squared);
  return fit;
}

std::array<DecayArm, 2> run_decay(
    const FullModes& full,
    double beta,
    const ftd::eft::ConnectedMooreBlockOptions& options,
    bool& initial_fields_equal) {
  std::array<DecayArm, 2> arms;
  arms[0].sign = -1;
  arms[1].sign = +1;
  const auto basis = donor_modes(full);
  const auto control_initial = regional_reference();
  std::array<ftd::eft::ConnectedMooreBlockState, 3> initial;
  initial[0] = control_initial;
  for (int sign = 0; sign < 2; ++sign) {
    initial[sign + 1] = decay_excitation(
        control_initial, full, arms[sign].sign);
    arms[sign].maximum_momentum = maximum_momentum(initial[sign + 1]);
    const auto raw = ftd::eft::evaluate_connected_reservoir_decomposition(
        control_initial, initial[sign + 1], basis, {6, 7}, beta,
        options, 1e-10);
    arms[sign].initial_target = raw.target_mode_energy;
    arms[sign].initialized = raw.valid
        && arms[sign].initial_target > 0.0
        && std::abs(arms[sign].maximum_momentum - decay_momentum) <= 1e-15;
  }
  initial_fields_equal = arms[0].initialized && arms[1].initialized;
  for (int path = 1; path < 3 && initial_fields_equal; ++path)
    initial_fields_equal = equal_face_bits(initial[0].electric,
        initial[path].electric)
        && equal_edge_bits(initial[0].magnetic_half,
                           initial[path].magnetic_half);
  if (!initial_fields_equal) return arms;

  std::array<ftd::eft::ConnectedMooreBlockState, 3> state = initial;
  std::array<double, 3> initial_energy{};
  std::array<std::vector<int>, 3> initial_sector;
  for (int path = 0; path < 3; ++path) {
    initial_energy[path] = energy_parts(state[path], beta, options).total;
    initial_sector[path] = sector_signature(state[path]);
  }
  for (int sign = 0; sign < 2; ++sign) {
    arms[sign].sector = true;
    auto record = observe_donor(0, state[0], state[sign + 1], basis,
        arms[sign].initial_target, beta, options);
    arms[sign].max_observer = record.observer_residual;
    arms[sign].ticks.push_back(std::move(record));
  }

  std::array<ftd::eft::ConnectedMooreBlockSolveCache, 3> forward_cache;
  std::array<ftd::eft::ConnectedMooreBlockSolveCache, 3> reverse_cache;
  bool forward = true;
  for (int tick = 1; tick <= decay_horizon && forward; ++tick) {
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
      auto record = observe_donor(tick, state[0], state[path], basis,
          arms[sign].initial_target, beta, options);
      arms[sign].max_observer = std::max(
          arms[sign].max_observer, record.observer_residual);
      arms[sign].max_common = std::max(arms[sign].max_common, common);
      arms[sign].max_energy_drift = std::max({
          arms[sign].max_energy_drift,
          std::abs(control_energy - initial_energy[0]),
          std::abs(energy_parts(state[path], beta, options).total
                   - initial_energy[path])});
      arms[sign].ticks.push_back(std::move(record));
    }
    for (const auto& arm : arms)
      if (arm.ticks.empty() || !arm.ticks.back().valid) forward = false;
    if (tick % 10 == 0)
      std::cout << "completed decay tick " << tick << std::endl;
  }

  bool reverse = forward;
  for (int tick = decay_horizon; tick >= 1 && reverse; --tick) {
    std::array<ftd::eft::ConnectedMooreBlockStepResult, 3> steps;
#pragma omp parallel for num_threads(3)
    for (int path = 0; path < 3; ++path)
      steps[path] = ftd::eft::solve_connected_moore_block_reverse(
          state[path], options, &reverse_cache[path]);
    for (int path = 0; path < 3; ++path) {
      const double residual = common_residual(steps[path]);
      for (auto& arm : arms)
        arm.max_common = std::max(arm.max_common, residual);
      if (!steps[path].valid || !steps[path].common_action_gates_pass
          || residual > 1e-10) {
        reverse = false;
        break;
      }
      state[path] = steps[path].earlier;
    }
    if (tick % 10 == 0)
      std::cout << "reversed decay tick " << tick << std::endl;
  }

  for (int sign = 0; sign < 2; ++sign) {
    const int path = sign + 1;
    auto& arm = arms[sign];
    arm.forward = forward && arm.ticks.size() == decay_horizon + 1;
    arm.reverse = reverse;
    arm.recovery = std::max(
        ftd::eft::connected_moore_block_state_max_difference(
            initial[0], state[0]),
        ftd::eft::connected_moore_block_state_max_difference(
            initial[path], state[path]));
    arm.fit = fit_decay(arm);
    const auto* start = decay_tick(arm, fit_start);
    const auto* end = decay_tick(arm, fit_end);
    if (start != nullptr && end != nullptr && start->target > 0.0)
      arm.decline = 1.0 - end->target / start->target;
    if (arm.fit.valid)
      arm.parent_rate_relative_difference = std::abs(
          arm.fit.gamma_energy - parent_energy_decay_rate)
          / parent_energy_decay_rate;
    arm.exact = arm.initialized && arm.forward && arm.reverse && arm.sector
        && arm.max_energy_drift <= 1e-10 && arm.max_common <= 1e-10
        && arm.max_observer <= 1e-8 && arm.recovery <= 1e-8
        && arm.fit.valid;
  }
  return arms;
}

double target_history_rms(const std::array<DecayArm, 2>& arms) {
  if (arms[0].ticks.size() != arms[1].ticks.size()
      || arms[0].ticks.empty()) return INFINITY;
  double sum = 0.0;
  for (std::size_t index = 0; index < arms[0].ticks.size(); ++index) {
    const double difference = arms[0].ticks[index].target
        - arms[1].ticks[index].target;
    sum += difference * difference;
  }
  return std::sqrt(sum / static_cast<double>(arms[0].ticks.size()));
}

std::string classify_decay(const std::array<DecayArm, 2>& arms,
                           bool exact,
                           double rate_difference,
                           double history_rms) {
  if (!exact) return "CANONICAL_PRECONTACT_MODE_DECAY_EXECUTION_INVALID";
  const bool decline = arms[0].decline >= 0.20 && arms[1].decline >= 0.20;
  const bool exponential = arms[0].fit.gamma_energy > 0.0
      && arms[1].fit.gamma_energy > 0.0
      && arms[0].fit.delta_bic >= 10.0 && arms[1].fit.delta_bic >= 10.0
      && arms[0].fit.r_squared >= 0.995
      && arms[1].fit.r_squared >= 0.995;
  if (!decline) return "CANONICAL_PRECONTACT_TRANSFER_ABSENT";
  if (!exponential) return "CANONICAL_PRECONTACT_TRANSFER_NONEXPONENTIAL";
  const bool amplitude = arms[0].parent_rate_relative_difference <= 0.05
      && arms[1].parent_rate_relative_difference <= 0.05;
  const bool polarity = rate_difference <= 1e-4 && history_rms <= 1e-5;
  if (!amplitude || !polarity)
    return "CANONICAL_PRECONTACT_TRANSFER_AMPLITUDE_OR_POLARITY_DEPENDENT";
  return "CANONICAL_PRECONTACT_EXPONENTIAL_TRANSFER_CONSTRUCTIVE";
}

void write_decay(const std::array<DecayArm, 2>& arms,
                 bool parent,
                 bool initial_fields_equal,
                 bool exact,
                 double rate_difference,
                 double history_rms,
                 const std::string& verdict) {
  const auto directory = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results/ftd_0676";
  std::filesystem::create_directories(directory);
  std::ofstream json(
      directory / "ftd_0676_canonical_precontact_mode_decay_v1.json");
  json << std::setprecision(17) << std::boolalpha
       << "{\n  \"ftd_id\": \"FTD-0676\",\n"
       << "  \"protocol_sha256\": \"" << decay_protocol_sha256 << "\",\n"
       << "  \"parent_json_sha256\": \"" << decay_parent_json_sha256 << "\",\n"
       << "  \"parent_csv_sha256\": \"" << decay_parent_csv_sha256 << "\",\n"
       << "  \"verdict\": \"" << verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"parent_pass\": " << parent << ",\n"
       << "  \"initial_fields_bitwise_equal\": "
       << initial_fields_equal << ",\n"
       << "  \"exact_execution_pass\": " << exact << ",\n"
       << "  \"volume\": " << regional_volume << ",\n"
       << "  \"horizon\": " << decay_horizon << ",\n"
       << "  \"fit_start_tick\": " << fit_start << ",\n"
       << "  \"fit_end_tick\": " << fit_end << ",\n"
       << "  \"maximum_constituent_momentum_amplitude\": "
       << decay_momentum << ",\n"
       << "  \"parent_energy_decay_rate\": "
       << parent_energy_decay_rate << ",\n"
       << "  \"polarity_rate_relative_difference\": "
       << rate_difference << ",\n"
       << "  \"polarity_target_history_rms\": " << history_rms << ",\n";
  for (int sign = 0; sign < 2; ++sign) {
    const auto& arm = arms[sign];
    const std::string prefix = arm.sign < 0 ? "negative" : "positive";
    json << "  \"" << prefix << "_exact\": " << arm.exact << ",\n"
         << "  \"" << prefix << "_maximum_momentum\": "
         << arm.maximum_momentum << ",\n"
         << "  \"" << prefix << "_initial_target\": "
         << arm.initial_target << ",\n"
         << "  \"" << prefix << "_gamma_energy\": "
         << arm.fit.gamma_energy << ",\n"
         << "  \"" << prefix << "_delta_bic\": "
         << arm.fit.delta_bic << ",\n"
         << "  \"" << prefix << "_r_squared\": "
         << arm.fit.r_squared << ",\n"
         << "  \"" << prefix << "_decline_tick8_tick64\": "
         << arm.decline << ",\n"
         << "  \"" << prefix << "_parent_rate_relative_difference\": "
         << arm.parent_rate_relative_difference << ",\n"
         << "  \"" << prefix << "_max_observer_residual\": "
         << arm.max_observer << ",\n"
         << "  \"" << prefix << "_max_energy_drift\": "
         << arm.max_energy_drift << ",\n"
         << "  \"" << prefix << "_max_common_residual\": "
         << arm.max_common << ",\n"
         << "  \"" << prefix << "_recovery\": " << arm.recovery
         << (sign == 0 ? ",\n" : "\n");
  }
  json << "}\n";

  std::ofstream csv(
      directory / "ftd_0676_canonical_precontact_mode_decay_ticks_v1.csv");
  csv << "ftd_id,protocol_sha256,sign,tick,target,other,nonlinear,"
         "dynamic_field,field_interference,kinetic,binding,field,total,"
         "observer_residual,observer_valid\n";
  for (const auto& arm : arms)
    for (const auto& tick : arm.ticks)
      csv << std::setprecision(17) << "FTD-0676," << decay_protocol_sha256
          << ',' << arm.sign << ',' << tick.tick << ',' << tick.target << ','
          << tick.other << ',' << tick.nonlinear << ',' << tick.dynamic_field
          << ',' << tick.field_interference << ',' << tick.kinetic << ','
          << tick.binding << ',' << tick.field << ',' << tick.total << ','
          << tick.observer_residual << ',' << (tick.valid ? 1 : 0) << '\n';
}

}  // namespace

#ifndef FTD_0676_EMBEDDED
int main() {
  const bool parent = decay_parent_fingerprint();
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  ftd::eft::ConnectedMooreBlockOptions options;
  options.allow_shared_anchor_chart = true;
  options.use_sparse_local_current = true;
  FullModes modes;
  if (parent && normalization.valid) {
    const auto reference = load_refined_state(0);
    const auto analytic = analytic_at(
        "canonical_precontact_mode_decay", 0, reference,
        normalization.mapped_field_work_coefficient, options);
    if (analytic.valid) modes = full_modes(analytic.hessian);
  }
  bool initial_fields_equal = false;
  std::array<DecayArm, 2> arms;
  if (modes.valid && modes.modes[6].group == modes.modes[7].group)
    arms = run_decay(modes,
        normalization.mapped_field_work_coefficient,
        options, initial_fields_equal);
  const bool exact = parent && normalization.valid && modes.valid
      && initial_fields_equal && arms[0].exact && arms[1].exact;
  const double rate_scale = std::max({
      std::abs(arms[0].fit.gamma_energy),
      std::abs(arms[1].fit.gamma_energy), 1e-300});
  const double rate_difference = std::abs(
      arms[0].fit.gamma_energy - arms[1].fit.gamma_energy) / rate_scale;
  const double history_rms = target_history_rms(arms);
  const std::string verdict = classify_decay(
      arms, exact, rate_difference, history_rms);
  write_decay(arms, parent, initial_fields_equal, exact,
              rate_difference, history_rms, verdict);
  std::cout << std::setprecision(17)
            << "protocol_sha256=" << decay_protocol_sha256 << '\n'
            << "verdict=" << verdict << '\n'
            << "execution=" << exact
            << " rate_difference=" << rate_difference
            << " history_rms=" << history_rms << '\n';
  for (const auto& arm : arms)
    std::cout << "sign=" << arm.sign
              << " pmax=" << arm.maximum_momentum
              << " gamma_E=" << arm.fit.gamma_energy
              << " delta_bic=" << arm.fit.delta_bic
              << " r2=" << arm.fit.r_squared
              << " decline=" << arm.decline
              << " parent_difference="
              << arm.parent_rate_relative_difference
              << " observer=" << arm.max_observer
              << " inverse=" << arm.recovery << '\n';
  return verdict == "CANONICAL_PRECONTACT_MODE_DECAY_EXECUTION_INVALID"
      ? 1 : 0;
}
#endif
