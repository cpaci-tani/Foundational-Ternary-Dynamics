// FTD-0681: corrected-output replication of localized-basin relaxation.
#define FTD_0679_EMBEDDED
#include "test_localized_basin_relaxation_v2.cpp"
#undef FTD_0679_EMBEDDED

namespace {

constexpr char basin_v3_protocol_sha256[] =
    "6E653BBD9D133F78ACE56E2E974EA322A275930C77E147478A8D4F31299D7E3A";

struct BasinExpectedTick {
  bool present = false;
  double core_ratio = 0.0;
  double near_field = 0.0;
  double intermediate_field = 0.0;
  double far_field = 0.0;
  double total_field = 0.0;
};

using BasinExpected = std::array<
    std::array<BasinExpectedTick, basin_horizon + 1>, 2>;

double basin_v3_relative(double left, double right) {
  return std::abs(left - right)
      / std::max({std::abs(left), std::abs(right), 1e-300});
}

bool load_basin_v2_expected(BasinExpected& expected) {
  const auto path = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results/ftd_0679/ftd_0679_localized_basin_relaxation_ticks_v2.csv";
  std::ifstream input(path);
  std::string line;
  if (!std::getline(input, line)
      || line.find("core_position,core_momentum,core_phase,core_ratio")
             == std::string::npos)
    return false;
  int records = 0;
  while (std::getline(input, line)) {
    std::vector<std::string> fields;
    std::stringstream stream(line);
    std::string field;
    while (std::getline(stream, field, ',')) fields.push_back(field);
    if (fields.size() != 22 || fields[0] != "FTD-0679") return false;
    const int sign = std::stoi(fields[2]);
    const int tick = std::stoi(fields[3]);
    const int arm = sign < 0 ? 0 : 1;
    if ((sign != -1 && sign != 1) || tick < 0 || tick > basin_horizon
        || expected[arm][tick].present)
      return false;
    auto& record = expected[arm][tick];
    record.present = true;
    record.core_ratio = std::stod(fields[9]);
    record.near_field = std::stod(fields[13]);
    record.intermediate_field = std::stod(fields[14]);
    record.far_field = std::stod(fields[15]);
    record.total_field = std::stod(fields[16]);
    ++records;
  }
  return records == 2 * (basin_horizon + 1);
}

bool basin_v3_conformance(const std::array<BasinArm, 2>& arms,
                          const BasinExpected& expected,
                          double& target_residual,
                          double& replication_residual) {
  target_residual = 0.0;
  replication_residual = 0.0;
  for (int arm = 0; arm < 2; ++arm) {
    if (arms[arm].ticks.size() != basin_horizon + 1) return false;
    for (const auto& tick : arms[arm].ticks) {
      if (tick.tick < 0 || tick.tick > basin_horizon
          || !expected[arm][tick.tick].present)
        return false;
      target_residual = std::max(target_residual, basin_v3_relative(
          tick.target, arms[arm].initial_target * tick.target_ratio));
      if (tick.tick == 0) {
        target_residual = std::max({target_residual,
            basin_v3_relative(tick.target, arms[arm].initial_target),
            std::abs(tick.target_ratio - 1.0)});
      }
      const auto& prior = expected[arm][tick.tick];
      replication_residual = std::max({replication_residual,
          basin_v3_relative(tick.core_ratio, prior.core_ratio),
          basin_v3_relative(tick.near_field, prior.near_field),
          basin_v3_relative(tick.intermediate_field,
                            prior.intermediate_field),
          basin_v3_relative(tick.far_field, prior.far_field),
          basin_v3_relative(tick.total_field, prior.total_field)});
    }
  }
  return target_residual <= 1e-12 && replication_residual <= 1e-12;
}

std::string classify_basin_v3(const std::array<BasinArm, 2>& arms,
                              bool exact,
                              double rate_difference,
                              double history_rms,
                              double far_fraction_difference) {
  const std::string v2 = classify_basin_v2(
      arms, exact, rate_difference, history_rms, far_fraction_difference);
  if (v2 == "LOCALIZED_BASIN_RELAXATION_V2_EXECUTION_INVALID")
    return "LOCALIZED_BASIN_RELAXATION_V3_EXECUTION_INVALID";
  if (v2 == "LOCALIZED_BASIN_V2_INTERNAL_RELAXATION_ABSENT")
    return "LOCALIZED_BASIN_V3_INTERNAL_RELAXATION_ABSENT";
  if (v2 == "LOCALIZED_BASIN_V2_INTERNAL_RELAXATION_NONEXPONENTIAL")
    return "LOCALIZED_BASIN_V3_INTERNAL_RELAXATION_NONEXPONENTIAL";
  if (v2 == "LOCALIZED_BASIN_V2_REMOTE_FIELD_NOT_DOMINANT")
    return "LOCALIZED_BASIN_V3_REMOTE_FIELD_NOT_DOMINANT";
  if (v2 == "LOCALIZED_BASIN_V2_RELAXATION_SIGN_DEPENDENT")
    return "LOCALIZED_BASIN_V3_RELAXATION_SIGN_DEPENDENT";
  return "LOCALIZED_BASIN_V3_RELAXATION_TOWARD_CONSTRUCTIVE";
}

void write_basin_v3(const std::array<BasinArm, 2>& arms,
                    bool parent,
                    bool initial_fields_equal,
                    bool observer_preflight,
                    bool expected_loaded,
                    bool conformance,
                    bool exact,
                    double initial_core_sign_difference,
                    double target_residual,
                    double replication_residual,
                    double rate_difference,
                    double history_rms,
                    double far_fraction_difference,
                    const std::string& verdict) {
  const auto directory = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results/ftd_0681";
  std::filesystem::create_directories(directory);
  std::ofstream json(
      directory / "ftd_0681_localized_basin_relaxation_v3.json");
  json << std::setprecision(17) << std::boolalpha
       << "{\n  \"ftd_id\": \"FTD-0681\",\n"
       << "  \"protocol_sha256\": \"" << basin_v3_protocol_sha256 << "\",\n"
       << "  \"verdict\": \"" << verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"held_out\": false,\n"
       << "  \"parent_pass\": " << parent << ",\n"
       << "  \"observer_preflight_pass\": " << observer_preflight << ",\n"
       << "  \"initial_fields_bitwise_equal\": "
       << initial_fields_equal << ",\n"
       << "  \"prior_series_loaded\": " << expected_loaded << ",\n"
       << "  \"conformance_pass\": " << conformance << ",\n"
       << "  \"exact_execution_pass\": " << exact << ",\n"
       << "  \"target_contract_residual\": " << target_residual << ",\n"
       << "  \"replication_residual\": " << replication_residual << ",\n"
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
         << "  \"" << prefix << "_initial_target_energy\": "
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
         << "  \"" << prefix << "_final_near_fraction\": "
         << (final != nullptr ? final->near_fraction : INFINITY) << ",\n"
         << "  \"" << prefix << "_final_far_fraction\": "
         << (final != nullptr ? final->far_fraction : INFINITY) << ",\n"
         << "  \"" << prefix << "_max_partition_residual\": "
         << arm.maximum_partition_residual << ",\n"
         << "  \"" << prefix << "_max_energy_drift\": "
         << arm.maximum_energy_drift << ",\n"
         << "  \"" << prefix << "_max_common_residual\": "
         << arm.maximum_common_residual << ",\n"
         << "  \"" << prefix << "_recovery\": " << arm.recovery
         << (sign == 0 ? ",\n" : "\n");
  }
  json << "}\n";

  std::ofstream csv(
      directory / "ftd_0681_localized_basin_relaxation_ticks_v3.csv");
  csv << "ftd_id,protocol_sha256,sign,tick,target_energy,target_ratio,"
         "core_position,core_momentum,core_phase,core_ratio,center_offset,"
         "mean_momentum_offset,edge_difference,near_field,intermediate_field,"
         "far_field,total_field,near_fraction,far_fraction,partition_residual,"
         "reservoir_residual,valid\n";
  for (const auto& arm : arms)
    for (const auto& tick : arm.ticks)
      csv << std::setprecision(17) << "FTD-0681," << basin_v3_protocol_sha256
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

int main() {
  BasinExpected expected{};
  const bool expected_loaded = load_basin_v2_expected(expected);
  const bool parent = basin_parent_fingerprint();
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  ftd::eft::ConnectedMooreBlockOptions options;
  options.allow_shared_anchor_chart = true;
  options.use_sparse_local_current = true;
  FullModes modes;
  if (parent && normalization.valid && expected_loaded) {
    const auto reference = load_refined_state(0);
    const auto analytic = analytic_at(
        "localized_basin_relaxation_v3", 0, reference,
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
  double target_residual = INFINITY;
  double replication_residual = INFINITY;
  const bool conformance = expected_loaded && basin_v3_conformance(
      arms, expected, target_residual, replication_residual);
  const bool exact = parent && normalization.valid && modes.valid
      && modes.modes[6].group == modes.modes[7].group
      && initial_fields_equal && observer_preflight
      && initial_core_sign_difference <= 1e-12
      && arms[0].exact && arms[1].exact && conformance;
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
  const std::string verdict = classify_basin_v3(
      arms, exact, rate_difference, history_rms, far_fraction_difference);
  write_basin_v3(arms, parent, initial_fields_equal, observer_preflight,
      expected_loaded, conformance, exact, initial_core_sign_difference,
      target_residual, replication_residual, rate_difference, history_rms,
      far_fraction_difference, verdict);
  std::cout << std::setprecision(17)
            << "protocol_sha256=" << basin_v3_protocol_sha256 << '\n'
            << "verdict=" << verdict << '\n'
            << "execution=" << exact
            << " target_residual=" << target_residual
            << " replication_residual=" << replication_residual << '\n';
  for (const auto& arm : arms) {
    const auto* final = basin_tick(arm, basin_horizon);
    std::cout << "sign=" << arm.sign
              << " gamma_core=" << arm.fit.gamma
              << " delta_bic=" << arm.fit.delta_bic
              << " r2=" << arm.fit.r_squared
              << " decline=" << arm.decline
              << " near_fraction="
              << (final != nullptr ? final->near_fraction : INFINITY)
              << " far_fraction="
              << (final != nullptr ? final->far_fraction : INFINITY)
              << " inverse=" << arm.recovery << '\n';
  }
  return verdict == "LOCALIZED_BASIN_RELAXATION_V3_EXECUTION_INVALID"
      ? 1 : 0;
}
