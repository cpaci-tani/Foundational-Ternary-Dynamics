// FTD-0666: out-of-sample L=17 extension of the FTD-0665 return threshold.
#define FTD_0664_EMBEDDED
#include "test_volume_scaled_internal_mode_transfer.cpp"
#undef FTD_0664_EMBEDDED

namespace {

constexpr char return_protocol_sha256[] =
    "4AFD79B3207C16A37EBDF96197EFCDA64ADFD5410DB0825D6085280791D8FDEC";
constexpr char return_parent_json_sha256[] =
    "3D9C7F4601C4932458F351A1DE412A6E6E849E2514691C2C21093944BEE9B5B2";
constexpr int return_volume = 17;
constexpr int return_horizon = 100;

bool return_parent_fingerprint() {
  const auto path = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results/ftd_0665/ftd_0665_volume_scaled_internal_mode_transfer_v2.json";
  std::ifstream input(path, std::ios::binary);
  const std::string bytes((std::istreambuf_iterator<char>(input)), {});
  return bytes.find("E8E627DEE418186A96A951290B61396D5C3D18B40C0AF6B18A37B26289FFE9B8")
          != std::string::npos
      && bytes.find("VOLUME_SCALED_PRE_RETURN_TRANSFER_V2_CONSTRUCTIVE")
          != std::string::npos;
}

std::array<VolumeArm, 2> run_return_extension(
    const FullModes& modes,
    double beta,
    const ftd::eft::ConnectedMooreBlockOptions& options) {
  std::array<VolumeArm, 2> arms;
  for (int sign = 0; sign < 2; ++sign) {
    arms[sign].volume = return_volume;
    arms[sign].sign = sign == 0 ? -1 : 1;
  }
  const auto control_initial = volume_reference(return_volume);
  std::array<ftd::eft::ConnectedMooreBlockState, 3> initial;
  initial[0] = control_initial;
  for (int sign = 0; sign < 2; ++sign) {
    initial[sign + 1] = volume_excitation(
        control_initial, modes, arms[sign].sign, arms[sign].initial_doublet);
    arms[sign].initialized = control_initial.electric.L == return_volume
        && initial[sign + 1].electric.L == return_volume
        && arms[sign].initial_doublet > 0.0;
  }
  if (!arms[0].initialized || !arms[1].initialized) return arms;

  std::array<ftd::eft::ConnectedMooreBlockState, 3> state = initial;
  std::array<double, 3> initial_energy{};
  for (int path = 0; path < 3; ++path) {
    initial_energy[path] = energy_parts(state[path], beta, options).total;
  }
  const std::array<std::vector<int>, 3> initial_sector{{
      sector_signature(state[0]), sector_signature(state[1]),
      sector_signature(state[2])}};
  for (auto& arm : arms) {
    arm.sector = true;
    arm.redressed = true;
    arm.ticks.push_back(observe_volume(
        0, state[0], state[arm.sign < 0 ? 1 : 2], modes,
        arm.initial_doublet, beta, 0.0, 0.0));
  }

  std::array<ftd::eft::ConnectedMooreBlockSolveCache, 3> forward_cache;
  std::array<ftd::eft::ConnectedMooreBlockSolveCache, 3> reverse_cache;
  bool forward = true;
  for (int tick = 1; tick <= return_horizon && forward; ++tick) {
    double common = 0.0;
    for (int path = 0; path < 3; ++path) {
      const auto step = ftd::eft::solve_connected_moore_block_forward(
          state[path], options, &forward_cache[path]);
      const double residual = common_residual(step);
      common = std::max(common, residual);
      if (!step.valid || !step.common_action_gates_pass || residual > 1e-10) {
        forward = false;
        break;
      }
      state[path] = step.later;
      for (auto& arm : arms) {
        arm.sector = arm.sector
            && sector_signature(state[path]) == initial_sector[path];
      }
    }
    if (!forward) break;
    for (int sign = 0; sign < 2; ++sign) {
      const int path = sign + 1;
      const double drift = std::max(
          std::abs(energy_parts(state[0], beta, options).total
                   - initial_energy[0]),
          std::abs(energy_parts(state[path], beta, options).total
                   - initial_energy[path]));
      auto record = observe_volume(
          tick, state[0], state[path], modes, arms[sign].initial_doublet,
          beta, drift, common);
      arms[sign].redressed = arms[sign].redressed
          && std::isfinite(record.decomposition_residual);
      arms[sign].max_energy_drift = std::max(
          arms[sign].max_energy_drift, record.energy_drift);
      arms[sign].max_common = std::max(
          arms[sign].max_common, record.common_residual);
      arms[sign].max_decomposition = std::max(
          arms[sign].max_decomposition, record.decomposition_residual);
      arms[sign].ticks.push_back(record);
    }
  }

  bool reverse = forward;
  for (int tick = return_horizon; tick >= 1 && reverse; --tick) {
    for (int path = 0; path < 3; ++path) {
      const auto step = ftd::eft::solve_connected_moore_block_reverse(
          state[path], options, &reverse_cache[path]);
      const double residual = common_residual(step);
      for (auto& arm : arms) arm.max_common = std::max(arm.max_common, residual);
      if (!step.valid || !step.common_action_gates_pass || residual > 1e-10) {
        reverse = false;
        break;
      }
      state[path] = step.earlier;
    }
  }
  for (int sign = 0; sign < 2; ++sign) {
    const int path = sign + 1;
    arms[sign].recovery = std::max(
        ftd::eft::connected_moore_block_state_max_difference(
            initial[0], state[0]),
        ftd::eft::connected_moore_block_state_max_difference(
            initial[path], state[path]));
    if (!arms[sign].ticks.empty()
        && arms[sign].ticks.front().doublet_ratio > 0.0) {
      const double normalization = arms[sign].ticks.front().doublet_ratio;
      arms[sign].initial_doublet *= normalization;
      for (auto& tick : arms[sign].ticks) {
        tick.doublet_ratio /= normalization;
        tick.dynamic_energy_ratio /= normalization;
        tick.dynamic_norm_ratio /= normalization;
      }
    }
    arms[sign].executed = forward && reverse && arms[sign].initialized
        && arms[sign].redressed && arms[sign].sector
        && static_cast<int>(arms[sign].ticks.size()) == return_horizon + 1
        && arms[sign].max_common <= 1e-10
        && arms[sign].max_energy_drift <= 1e-10
        && arms[sign].max_decomposition <= 1e-10
        && arms[sign].recovery <= 1e-8;
    bool below = false;
    for (const auto& tick : arms[sign].ticks) {
      if (tick.doublet_ratio < 0.60) below = true;
      if (below && tick.tick >= return_volume && tick.doublet_ratio > 0.80) {
        arms[sign].return_tick = tick.tick;
        break;
      }
    }
  }
  return arms;
}

void write_return(
    const std::array<VolumeArm, 2>& arms,
    const std::string& verdict) {
  const auto directory = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results/ftd_0666";
  std::filesystem::create_directories(directory);
  std::ofstream json(directory / "ftd_0666_internal_mode_return_time_v1.json");
  json << std::setprecision(17)
       << "{\n  \"ftd_id\": \"FTD-0666\",\n"
       << "  \"protocol_sha256\": \"" << return_protocol_sha256 << "\",\n"
       << "  \"parent_json_sha256\": \""
       << return_parent_json_sha256 << "\",\n"
       << "  \"verdict\": \"" << verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"return_tick_negative\": " << arms[0].return_tick << ",\n"
       << "  \"return_tick_positive\": " << arms[1].return_tick << ",\n"
       << "  \"recovery_negative\": " << arms[0].recovery << ",\n"
       << "  \"recovery_positive\": " << arms[1].recovery << "\n}\n";
  std::ofstream ticks(directory / "ftd_0666_internal_mode_return_time_ticks_v1.csv");
  ticks << "ftd_id,sign,tick,doublet_ratio,dynamic_energy_ratio,"
           "dynamic_norm_ratio,radius_second_moment,decomposition_residual,"
           "energy_drift,common_residual\n";
  for (const auto& arm : arms) {
    for (const auto& tick : arm.ticks) {
      ticks << std::setprecision(17) << "FTD-0666," << arm.sign << ','
            << tick.tick << ',' << tick.doublet_ratio << ','
            << tick.dynamic_energy_ratio << ',' << tick.dynamic_norm_ratio << ','
            << tick.radius_second_moment << ','
            << tick.decomposition_residual << ',' << tick.energy_drift << ','
            << tick.common_residual << '\n';
    }
  }
}

}  // namespace

int main() {
  const bool parent = return_parent_fingerprint();
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  ftd::eft::ConnectedMooreBlockOptions options;
  options.allow_shared_anchor_chart = true;
  FullModes modes;
  if (parent && normalization.valid) {
    const auto reference = load_refined_state(0);
    const auto analytic = analytic_at(
        "return_time", 0, reference,
        normalization.mapped_field_work_coefficient, options);
    if (analytic.valid) modes = full_modes(analytic.hessian);
  }
  std::array<VolumeArm, 2> arms;
  if (modes.valid && modes.modes[6].group == modes.modes[7].group) {
    arms = run_return_extension(
        modes, normalization.mapped_field_work_coefficient, options);
  }
  const bool execution = parent && normalization.valid && modes.valid
      && arms[0].executed && arms[1].executed;
  const bool prediction = execution
      && arms[0].return_tick >= 74 && arms[0].return_tick <= 78
      && arms[1].return_tick >= 74 && arms[1].return_tick <= 78
      && std::abs(arms[0].return_tick - arms[1].return_tick) <= 1;
  const std::string verdict = !execution
      ? "INTERNAL_MODE_RETURN_TIME_EXECUTION_INVALID"
      : prediction
          ? "INTERNAL_MODE_ABSOLUTE_RETURN_TIME_CONSTRUCTIVE"
          : "INTERNAL_MODE_RETURN_TIME_MIXED";
  write_return(arms, verdict);
  std::cout << std::setprecision(17)
            << "protocol_sha256=" << return_protocol_sha256 << '\n'
            << "verdict=" << verdict << '\n'
            << "execution=" << execution << " prediction=" << prediction
            << " returns=" << arms[0].return_tick << ','
            << arms[1].return_tick << " recoveries=" << arms[0].recovery
            << ',' << arms[1].recovery << '\n';
  return arms[0].ticks.size() == return_horizon + 1
      && arms[1].ticks.size() == return_horizon + 1 ? 0 : 1;
}
