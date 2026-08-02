// FTD-0679: corrected localized-basin relaxation discriminator.
#define FTD_0678_EMBEDDED
#include "test_localized_basin_relaxation.cpp"
#undef FTD_0678_EMBEDDED

namespace {

constexpr char basin_v2_protocol_sha256[] =
    "697FC9058FA9AD3A48F10833CAA744C9260570DB3A5AF8F2F8CE97B32C65DF95";

std::array<BasinArm, 2> run_basin_v2(
    const FullModes& full,
    double beta,
    const ftd::eft::ConnectedMooreBlockOptions& options,
    bool& initial_fields_equal,
    bool& observer_preflight,
    double& initial_core_sign_difference) {
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
    const double comparison_scale = std::max({
        std::abs(arms[sign].initial_core),
        2.0 * std::abs(arms[sign].initial_target), 1e-300});
    arms[sign].initial_identity_residual = std::abs(
        arms[sign].initial_core - 2.0 * arms[sign].initial_target)
        / comparison_scale;
    arms[sign].initialized = reservoir.valid && localized.valid
        && std::isfinite(arms[sign].initial_target)
        && std::isfinite(arms[sign].initial_core)
        && arms[sign].initial_target > 0.0
        && arms[sign].initial_core > 0.0
        && std::abs(arms[sign].maximum_momentum - basin_momentum) <= 1e-15;
  }
  const double core_scale = std::max({std::abs(arms[0].initial_core),
      std::abs(arms[1].initial_core), 1e-300});
  initial_core_sign_difference = std::abs(
      arms[0].initial_core - arms[1].initial_core) / core_scale;
  if (!arms[0].initialized || !arms[1].initialized
      || initial_core_sign_difference > 1e-12) return arms;

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
    arms[sign].initialized = arms[sign].initialized && tick.valid
        && std::abs(tick.core_ratio - 1.0) <= 1e-12;
    arms[sign].ticks.push_back(std::move(tick));
  }
  if (!arms[0].initialized || !arms[1].initialized) return arms;

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
      std::cout << "completed basin-v2 tick " << tick << std::endl;
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
      std::cout << "reversed basin-v2 tick " << tick << std::endl;
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

std::string classify_basin_v2(const std::array<BasinArm, 2>& arms,
                              bool exact,
                              double rate_difference,
                              double history_rms,
                              double far_fraction_difference) {
  const std::string v1 = classify_basin(
      arms, exact, rate_difference, history_rms, far_fraction_difference);
  if (v1 == "LOCALIZED_BASIN_RELAXATION_EXECUTION_INVALID")
    return "LOCALIZED_BASIN_RELAXATION_V2_EXECUTION_INVALID";
  if (v1 == "LOCALIZED_BASIN_INTERNAL_RELAXATION_ABSENT")
    return "LOCALIZED_BASIN_V2_INTERNAL_RELAXATION_ABSENT";
  if (v1 == "LOCALIZED_BASIN_INTERNAL_RELAXATION_NONEXPONENTIAL")
    return "LOCALIZED_BASIN_V2_INTERNAL_RELAXATION_NONEXPONENTIAL";
  if (v1 == "LOCALIZED_BASIN_REMOTE_FIELD_NOT_DOMINANT")
    return "LOCALIZED_BASIN_V2_REMOTE_FIELD_NOT_DOMINANT";
  if (v1 == "LOCALIZED_BASIN_RELAXATION_SIGN_DEPENDENT")
    return "LOCALIZED_BASIN_V2_RELAXATION_SIGN_DEPENDENT";
  return "LOCALIZED_BASIN_V2_RELAXATION_TOWARD_CONSTRUCTIVE";
}

void write_basin_v2(const std::array<BasinArm, 2>& arms,
                    bool parent,
                    bool initial_fields_equal,
                    bool observer_preflight,
                    bool exact,
                    double initial_core_sign_difference,
                    double rate_difference,
                    double history_rms,
                    double far_fraction_difference,
                    const std::string& verdict) {
  const auto directory = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results/ftd_0679";
  std::filesystem::create_directories(directory);
  std::ofstream json(
      directory / "ftd_0679_localized_basin_relaxation_v2.json");
  json << std::setprecision(17) << std::boolalpha
       << "{\n  \"ftd_id\": \"FTD-0679\",\n"
       << "  \"protocol_sha256\": \"" << basin_v2_protocol_sha256 << "\",\n"
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
       << "  \"initial_core_sign_relative_difference\": "
       << initial_core_sign_difference << ",\n"
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
         << "  \"" << prefix << "_quotient_comparison_diagnostic\": "
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
      directory / "ftd_0679_localized_basin_relaxation_ticks_v2.csv");
  csv << "ftd_id,protocol_sha256,sign,tick,target,target_ratio,"
         "core_position,core_momentum,core_phase,core_ratio,center_offset,"
         "mean_momentum_offset,edge_difference,near_field,intermediate_field,"
         "far_field,total_field,near_fraction,far_fraction,partition_residual,"
         "reservoir_residual,valid\n";
  for (const auto& arm : arms)
    for (const auto& tick : arm.ticks)
      csv << std::setprecision(17) << "FTD-0679," << basin_v2_protocol_sha256
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

#ifndef FTD_0679_EMBEDDED
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
        "localized_basin_relaxation_v2", 0, reference,
        normalization.mapped_field_work_coefficient, options);
    if (analytic.valid) modes = full_modes(analytic.hessian);
  }
  bool initial_fields_equal = false;
  bool observer_preflight = false;
  double initial_core_sign_difference = INFINITY;
  std::array<BasinArm, 2> arms;
  if (modes.valid && modes.modes[6].group == modes.modes[7].group)
    arms = run_basin_v2(modes,
        normalization.mapped_field_work_coefficient, options,
        initial_fields_equal, observer_preflight,
        initial_core_sign_difference);
  const bool exact = parent && normalization.valid && modes.valid
      && modes.modes[6].group == modes.modes[7].group
      && initial_fields_equal && observer_preflight
      && initial_core_sign_difference <= 1e-12
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
  const std::string verdict = classify_basin_v2(
      arms, exact, rate_difference, history_rms, far_fraction_difference);
  write_basin_v2(arms, parent, initial_fields_equal, observer_preflight, exact,
      initial_core_sign_difference, rate_difference, history_rms,
      far_fraction_difference, verdict);
  std::cout << std::setprecision(17)
            << "protocol_sha256=" << basin_v2_protocol_sha256 << '\n'
            << "verdict=" << verdict << '\n'
            << "execution=" << exact
            << " initial_core_sign_difference="
            << initial_core_sign_difference
            << " rate_difference=" << rate_difference
            << " history_rms=" << history_rms
            << " far_fraction_difference=" << far_fraction_difference << '\n';
  for (const auto& arm : arms) {
    const auto* final = basin_tick(arm, basin_horizon);
    std::cout << "sign=" << arm.sign
              << " pmax=" << arm.maximum_momentum
              << " quotient_diagnostic=" << arm.initial_identity_residual
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
  return verdict == "LOCALIZED_BASIN_RELAXATION_V2_EXECUTION_INVALID"
      ? 1 : 0;
}
#endif
