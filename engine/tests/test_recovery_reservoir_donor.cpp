// FTD-0674: identify the exact donor of the causal-envelope recovery.
#define FTD_0672_EMBEDDED
#include "test_causal_regional_field_flow.cpp"
#undef FTD_0672_EMBEDDED

#include "ftd/eft/connected_reservoir_decomposition.h"

namespace {

constexpr char donor_protocol_sha256[] =
    "EC89065A9996C233978E164533D878200275B203646921222500928062C60383";
constexpr char donor_parent_json_sha256[] =
    "E3EFB78EC36F32FEFE7627A3EE368E2A5A700BCE0890FBEF1E27D2D8E9B414D3";
constexpr char donor_parent_csv_sha256[] =
    "C4339D5985F4EB36DFE2F0DDF28A4151C805D4EC5913ED08EAF1A601F84F5C8E";
constexpr int donor_horizon = 80;
constexpr int donor_start_tick = 72;
constexpr int donor_end_tick = 78;

struct DonorTick {
  int tick = 0;
  double target = 0.0;
  double other = 0.0;
  double nonlinear = 0.0;
  double dynamic_field = 0.0;
  double field_interference = 0.0;
  double kinetic = 0.0;
  double binding = 0.0;
  double field = 0.0;
  double total = 0.0;
  double observer_residual = INFINITY;
  bool valid = false;
};

struct DonorArm {
  int sign = 0;
  bool initialized = false;
  bool forward = false;
  bool reverse = false;
  bool sector = false;
  bool executed = false;
  double initial_target = 0.0;
  double maximum_momentum = 0.0;
  double max_energy_drift = 0.0;
  double max_common = 0.0;
  double max_observer = 0.0;
  double recovery = INFINITY;
  std::array<double, 5> delta{};
  std::array<double, 4> donor{};
  std::array<double, 4> fraction{};
  double donor_total = 0.0;
  double interval_closure = INFINITY;
  double target_recovery = -INFINITY;
  double binding_change = INFINITY;
  std::string donor_class = "RESERVOIR_DONOR_MIXED";
  std::string binding_class = "BINDING_BALANCED";
  std::vector<DonorTick> ticks;
};

bool donor_parent_fingerprint() {
  const auto root = std::filesystem::path(__FILE__).parent_path().parent_path();
  return file_contains(
             root / "results/ftd_0672/ftd_0672_causal_regional_field_flow_v1.json",
             regional_protocol_sha256,
             "CAUSAL_REGIONAL_FIELD_FLOW_MIXED")
      && file_contains(
             root / "results/ftd_0672/ftd_0672_causal_regional_field_flow_ticks_v1.csv",
             "ftd_id,protocol_sha256,sign,tick,radius,doublet_ratio",
             "FTD-0672");
}

std::vector<ftd::eft::ConnectedTangentMode> donor_modes(
    const FullModes& modes) {
  std::vector<ftd::eft::ConnectedTangentMode> result;
  result.reserve(modes.modes.size());
  for (const auto& mode : modes.modes)
    result.push_back({mode.omega, mode.vector});
  return result;
}

ftd::eft::ConnectedMooreBlockState donor_excitation(
    const ftd::eft::ConnectedMooreBlockState& reference,
    const FullModes& modes,
    int sign,
    double& nominal) {
  auto state = regional_excitation(reference, modes, sign, nominal);
  double maximum = 0.0;
  for (auto& constituent : state.constituents)
    maximum = std::max({maximum,
        std::abs(constituent.momentum.x),
        std::abs(constituent.momentum.y),
        std::abs(constituent.momentum.z)});
  const double scale = maximum > 0.0 ? 1e-6 / maximum : 0.0;
  for (auto& constituent : state.constituents)
    constituent.momentum *= scale;
  nominal *= scale * scale;
  return state;
}

double maximum_momentum(const ftd::eft::ConnectedMooreBlockState& state) {
  double result = 0.0;
  for (const auto& constituent : state.constituents)
    result = std::max({result,
        std::abs(constituent.momentum.x),
        std::abs(constituent.momentum.y),
        std::abs(constituent.momentum.z)});
  return result;
}

DonorTick observe_donor(
    int tick,
    const ftd::eft::ConnectedMooreBlockState& control,
    const ftd::eft::ConnectedMooreBlockState& excited,
    const std::vector<ftd::eft::ConnectedTangentMode>& modes,
    double initial_target,
    double beta,
    const ftd::eft::ConnectedMooreBlockOptions& options) {
  DonorTick record;
  record.tick = tick;
  const auto result = ftd::eft::evaluate_connected_reservoir_decomposition(
      control, excited, modes, {6, 7}, beta, options, 1e-10);
  if (!result.valid || !(initial_target > 0.0)) return record;
  const double scale = 1.0 / initial_target;
  record.target = scale * result.target_mode_energy;
  record.other = scale * result.other_mode_energy;
  record.nonlinear = scale * result.matter_nonlinear_remainder;
  record.dynamic_field = scale * result.dynamic_field_energy;
  record.field_interference = scale * result.field_interference;
  record.kinetic = scale * result.kinetic_difference;
  record.binding = scale * result.binding_difference;
  record.field = scale * result.field_difference;
  record.total = scale * result.total_difference;
  record.observer_residual = scale * std::max({
      result.mode_orthonormality_residual * initial_target,
      result.field_decomposition_residual,
      result.matter_decomposition_residual,
      result.complete_decomposition_residual});
  record.valid = std::isfinite(record.total)
      && record.observer_residual <= 1e-8;
  return record;
}

const DonorTick* donor_tick(const DonorArm& arm, int tick) {
  for (const auto& record : arm.ticks)
    if (record.tick == tick) return &record;
  return nullptr;
}

std::array<double, 5> additive_values(const DonorTick& record) {
  return {record.target, record.other, record.nonlinear,
          record.dynamic_field, record.field_interference};
}

void classify_donor(DonorArm& arm) {
  const auto* start = donor_tick(arm, donor_start_tick);
  const auto* end = donor_tick(arm, donor_end_tick);
  if (start == nullptr || end == nullptr) return;
  const auto before = additive_values(*start);
  const auto after = additive_values(*end);
  arm.interval_closure = 0.0;
  for (std::size_t index = 0; index < arm.delta.size(); ++index) {
    arm.delta[index] = after[index] - before[index];
    arm.interval_closure += arm.delta[index];
  }
  arm.interval_closure = std::abs(arm.interval_closure);
  arm.target_recovery = arm.delta[0];
  for (std::size_t index = 0; index < arm.donor.size(); ++index) {
    arm.donor[index] = std::max(-arm.delta[index + 1], 0.0);
    arm.donor_total += arm.donor[index];
  }
  if (arm.donor_total > 0.0)
    for (std::size_t index = 0; index < arm.fraction.size(); ++index)
      arm.fraction[index] = arm.donor[index] / arm.donor_total;

  int dominant = -1;
  for (int index = 0; index < 4; ++index)
    if (arm.fraction[static_cast<std::size_t>(index)] >= 0.60)
      dominant = dominant < 0 ? index : -2;
  constexpr std::array<const char*, 4> labels{{
      "OTHER_TANGENT_MODE_DONOR",
      "NONLINEAR_MATTER_REMAINDER_DONOR",
      "DYNAMIC_FIELD_SELF_ENERGY_DONOR",
      "FIELD_INTERFERENCE_DONOR"}};
  if (dominant >= 0) {
    arm.donor_class = labels[static_cast<std::size_t>(dominant)];
  } else {
    int distributed = 0;
    for (double fraction : arm.fraction)
      if (fraction >= 0.20) ++distributed;
    if (distributed >= 2)
      arm.donor_class = "DISTRIBUTED_RESERVOIR_DONOR";
  }
  arm.binding_change = end->binding - start->binding;
  if (arm.binding_change <= -0.01)
    arm.binding_class = "BINDING_DECREASE";
  else if (arm.binding_change >= 0.01)
    arm.binding_class = "BINDING_INCREASE";
}

std::array<DonorArm, 2> run_donor(
    const FullModes& full,
    double beta,
    const ftd::eft::ConnectedMooreBlockOptions& options,
    bool& initial_fields_equal) {
  std::array<DonorArm, 2> arms;
  arms[0].sign = -1;
  arms[1].sign = +1;
  const auto basis = donor_modes(full);
  const auto control_initial = regional_reference();
  std::array<ftd::eft::ConnectedMooreBlockState, 3> initial;
  initial[0] = control_initial;
  for (int sign = 0; sign < 2; ++sign) {
    double nominal = 0.0;
    initial[sign + 1] = donor_excitation(
        control_initial, full, arms[sign].sign, nominal);
    arms[sign].maximum_momentum = maximum_momentum(initial[sign + 1]);
    const auto raw = ftd::eft::evaluate_connected_reservoir_decomposition(
        control_initial, initial[sign + 1], basis, {6, 7}, beta,
        options, 1e-10);
    arms[sign].initial_target = raw.target_mode_energy;
    arms[sign].initialized = raw.valid
        && arms[sign].initial_target > 0.0
        && std::abs(arms[sign].maximum_momentum - 1e-6) <= 1e-15
        && std::isfinite(nominal);
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
    arms[sign].ticks.push_back(observe_donor(
        0, state[0], state[sign + 1], basis,
        arms[sign].initial_target, beta, options));
  }

  std::array<ftd::eft::ConnectedMooreBlockSolveCache, 3> forward_cache;
  std::array<ftd::eft::ConnectedMooreBlockSolveCache, 3> reverse_cache;
  bool forward = true;
  for (int tick = 1; tick <= donor_horizon && forward; ++tick) {
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
          std::abs(energy_parts(state[0], beta, options).total
                   - initial_energy[0]),
          std::abs(energy_parts(state[path], beta, options).total
                   - initial_energy[path])});
      arms[sign].ticks.push_back(std::move(record));
    }
    for (const auto& arm : arms)
      if (arm.ticks.empty() || !arm.ticks.back().valid) forward = false;
    if (tick % 10 == 0)
      std::cout << "completed donor tick " << tick << std::endl;
  }

  bool reverse = forward;
  for (int tick = donor_horizon; tick >= 1 && reverse; --tick) {
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
      std::cout << "reversed donor tick " << tick << std::endl;
  }

  for (int sign = 0; sign < 2; ++sign) {
    const int path = sign + 1;
    auto& arm = arms[sign];
    arm.forward = forward && arm.ticks.size() == donor_horizon + 1;
    arm.reverse = reverse;
    arm.recovery = std::max(
        ftd::eft::connected_moore_block_state_max_difference(
            initial[0], state[0]),
        ftd::eft::connected_moore_block_state_max_difference(
            initial[path], state[path]));
    classify_donor(arm);
    arm.executed = arm.initialized && arm.forward && arm.reverse && arm.sector
        && arm.max_energy_drift <= 1e-10 && arm.max_common <= 1e-10
        && arm.max_observer <= 1e-8 && arm.recovery <= 1e-8
        && arm.target_recovery >= 0.05
        && *std::max_element(arm.donor.begin(), arm.donor.end()) >= 0.01
        && arm.interval_closure <= 1e-8;
  }
  return arms;
}

bool donor_polarity_consistent(const std::array<DonorArm, 2>& arms) {
  if (arms[0].donor_class != arms[1].donor_class
      || arms[0].binding_class != arms[1].binding_class) return false;
  for (int tick : {donor_start_tick, donor_end_tick}) {
    const auto* left = donor_tick(arms[0], tick);
    const auto* right = donor_tick(arms[1], tick);
    if (left == nullptr || right == nullptr) return false;
    const auto lv = additive_values(*left);
    const auto rv = additive_values(*right);
    for (std::size_t index = 0; index < lv.size(); ++index)
      if (std::abs(lv[index] - rv[index]) > 1e-4) return false;
    if (std::abs(left->kinetic - right->kinetic) > 1e-4
        || std::abs(left->binding - right->binding) > 1e-4
        || std::abs(left->field - right->field) > 1e-4) return false;
  }
  for (std::size_t index = 0; index < 5; ++index)
    if (std::abs(arms[0].delta[index] - arms[1].delta[index]) > 1e-4)
      return false;
  for (std::size_t index = 0; index < 4; ++index)
    if (std::abs(arms[0].fraction[index] - arms[1].fraction[index]) > 1e-4)
      return false;
  return true;
}

void write_donor(const std::array<DonorArm, 2>& arms,
                 bool parent,
                 bool initial_fields_equal,
                 bool polarity,
                 const std::string& verdict) {
  const auto directory = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results/ftd_0674";
  std::filesystem::create_directories(directory);
  std::ofstream json(
      directory / "ftd_0674_recovery_reservoir_donor_v1.json");
  json << std::setprecision(17) << std::boolalpha
       << "{\n  \"ftd_id\": \"FTD-0674\",\n"
       << "  \"protocol_sha256\": \"" << donor_protocol_sha256 << "\",\n"
       << "  \"parent_json_sha256\": \"" << donor_parent_json_sha256 << "\",\n"
       << "  \"parent_csv_sha256\": \"" << donor_parent_csv_sha256 << "\",\n"
       << "  \"verdict\": \"" << verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"parent_pass\": " << parent << ",\n"
       << "  \"initial_fields_bitwise_equal\": "
       << initial_fields_equal << ",\n"
       << "  \"polarity_consistent\": " << polarity << ",\n"
       << "  \"volume\": " << regional_volume << ",\n"
       << "  \"horizon\": " << donor_horizon << ",\n"
       << "  \"donor_start_tick\": " << donor_start_tick << ",\n"
       << "  \"donor_end_tick\": " << donor_end_tick << ",\n"
       << "  \"maximum_constituent_momentum_amplitude\": 1e-6,\n";
  for (int sign = 0; sign < 2; ++sign) {
    const auto& arm = arms[sign];
    const std::string prefix = sign == 0 ? "negative" : "positive";
    json << "  \"" << prefix << "_executed\": " << arm.executed << ",\n"
         << "  \"" << prefix << "_donor_class\": \""
         << arm.donor_class << "\",\n"
         << "  \"" << prefix << "_binding_class\": \""
         << arm.binding_class << "\",\n"
         << "  \"" << prefix << "_initial_target\": "
         << arm.initial_target << ",\n"
         << "  \"" << prefix << "_target_recovery\": "
         << arm.target_recovery << ",\n"
         << "  \"" << prefix << "_interval_closure\": "
         << arm.interval_closure << ",\n"
         << "  \"" << prefix << "_binding_change\": "
         << arm.binding_change << ",\n"
         << "  \"" << prefix << "_donor_total\": "
         << arm.donor_total << ",\n"
         << "  \"" << prefix << "_max_observer_residual\": "
         << arm.max_observer << ",\n"
         << "  \"" << prefix << "_max_energy_drift\": "
         << arm.max_energy_drift << ",\n"
         << "  \"" << prefix << "_max_common_residual\": "
         << arm.max_common << ",\n"
         << "  \"" << prefix << "_recovery\": " << arm.recovery << ",\n";
    constexpr std::array<const char*, 5> additive{{
        "target", "other", "nonlinear", "dynamic_field", "field_interference"}};
    for (std::size_t index = 0; index < additive.size(); ++index)
      json << "  \"" << prefix << "_delta_" << additive[index] << "\": "
           << arm.delta[index] << ",\n";
    for (std::size_t index = 0; index < 4; ++index)
      json << "  \"" << prefix << "_donor_fraction_"
           << additive[index + 1] << "\": " << arm.fraction[index] << ",\n";
  }
  json << "  \"schema_complete\": true\n}\n";

  std::ofstream csv(
      directory / "ftd_0674_recovery_reservoir_donor_ticks_v1.csv");
  csv << "ftd_id,protocol_sha256,sign,tick,target,other,nonlinear,"
         "dynamic_field,field_interference,kinetic,binding,field,total,"
         "observer_residual,observer_valid\n";
  for (const auto& arm : arms)
    for (const auto& tick : arm.ticks)
      csv << std::setprecision(17) << "FTD-0674," << donor_protocol_sha256
          << ',' << arm.sign << ',' << tick.tick << ',' << tick.target << ','
          << tick.other << ',' << tick.nonlinear << ',' << tick.dynamic_field
          << ',' << tick.field_interference << ',' << tick.kinetic << ','
          << tick.binding << ',' << tick.field << ',' << tick.total << ','
          << tick.observer_residual << ',' << (tick.valid ? 1 : 0) << '\n';
}

}  // namespace

#ifndef FTD_0674_EMBEDDED
int main() {
  const bool parent = donor_parent_fingerprint();
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  ftd::eft::ConnectedMooreBlockOptions options;
  options.allow_shared_anchor_chart = true;
  options.use_sparse_local_current = true;
  FullModes modes;
  if (parent && normalization.valid) {
    const auto reference = load_refined_state(0);
    const auto analytic = analytic_at(
        "recovery_reservoir_donor", 0, reference,
        normalization.mapped_field_work_coefficient, options);
    if (analytic.valid) modes = full_modes(analytic.hessian);
  }
  bool initial_fields_equal = false;
  std::array<DonorArm, 2> arms;
  if (modes.valid && modes.modes[6].group == modes.modes[7].group)
    arms = run_donor(modes,
        normalization.mapped_field_work_coefficient,
        options, initial_fields_equal);
  const bool execution = parent && normalization.valid && modes.valid
      && initial_fields_equal && arms[0].executed && arms[1].executed;
  const bool polarity = execution && donor_polarity_consistent(arms);
  const std::string verdict = polarity
      ? "RECOVERY_RESERVOIR_" + arms[0].donor_class
      : "RECOVERY_RESERVOIR_DONOR_EXECUTION_INVALID";
  write_donor(arms, parent, initial_fields_equal, polarity, verdict);
  std::cout << std::setprecision(17)
            << "protocol_sha256=" << donor_protocol_sha256 << '\n'
            << "verdict=" << verdict << '\n'
            << "execution=" << execution << " polarity=" << polarity << '\n';
  for (const auto& arm : arms)
    std::cout << "sign=" << arm.sign
              << " pmax=" << arm.maximum_momentum
              << " initial_target=" << arm.initial_target
              << " class=" << arm.donor_class
              << " binding=" << arm.binding_class
              << " target=" << arm.target_recovery
              << " delta=(" << arm.delta[1] << ',' << arm.delta[2] << ','
              << arm.delta[3] << ',' << arm.delta[4] << ')'
              << " fraction=(" << arm.fraction[0] << ',' << arm.fraction[1]
              << ',' << arm.fraction[2] << ',' << arm.fraction[3] << ')'
              << " closure=" << arm.interval_closure
              << " observer=" << arm.max_observer
              << " inverse=" << arm.recovery << '\n';
  return execution && polarity ? 0 : 1;
}
#endif
