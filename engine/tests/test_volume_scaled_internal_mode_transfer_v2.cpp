// FTD-0665: corrected mass-weighted normalization and accumulated recovery gate.
#define FTD_0664_EMBEDDED
#include "test_volume_scaled_internal_mode_transfer.cpp"
#undef FTD_0664_EMBEDDED

namespace {

constexpr char volume_v2_protocol_sha256[] =
    "E8E627DEE418186A96A951290B61396D5C3D18B40C0AF6B18A37B26289FFE9B8";
constexpr char volume_v2_parent_json_sha256[] =
    "EB6228CCE248DBF83822C87E957A35D057DA82311461CF192F24CF06E150A6A8";

std::array<VolumeArm, 2> run_volume_v2(
    int size,
    const FullModes& modes,
    double beta,
    const ftd::eft::ConnectedMooreBlockOptions& options) {
  auto arms = run_volume(size, modes, beta, options);
  for (auto& arm : arms) {
    if (arm.ticks.empty() || !(arm.ticks.front().doublet_ratio > 0.0)) {
      arm.executed = false;
      continue;
    }
    const double normalization = arm.ticks.front().doublet_ratio;
    arm.initial_doublet *= normalization;
    for (auto& tick : arm.ticks) {
      tick.doublet_ratio /= normalization;
      tick.dynamic_energy_ratio /= normalization;
      tick.dynamic_norm_ratio /= normalization;
    }
    arm.executed = arm.initialized && arm.redressed && arm.sector
        && static_cast<int>(arm.ticks.size()) == 4 * size + 1
        && arm.max_common <= 1e-10
        && arm.max_energy_drift <= 1e-10
        && arm.max_decomposition <= 1e-10
        && arm.recovery <= 1e-8;
    arm.return_tick = -1;
    bool below = false;
    for (const auto& tick : arm.ticks) {
      if (tick.doublet_ratio < 0.60) below = true;
      if (below && tick.tick >= size && tick.doublet_ratio > 0.80) {
        arm.return_tick = tick.tick;
        break;
      }
    }
  }
  return arms;
}

void evaluate_volume_v2(VolumeSummary& summary) {
  summary.execution = summary.parent && summary.normalization
      && summary.eigenspace && summary.arms.size() == 6
      && std::all_of(summary.arms.begin(), summary.arms.end(),
          [](const VolumeArm& arm) { return arm.executed; });
  if (!summary.execution) {
    summary.verdict = "VOLUME_SCALED_INTERNAL_TRANSFER_V2_EXECUTION_INVALID";
    return;
  }
  long double locality_sum = 0.0;
  int locality_count = 0;
  summary.emission = true;
  summary.outward = true;
  for (int sign : {-1, 1}) {
    const auto* base = find_volume_arm(summary, 17, sign);
    for (int size : {25, 33}) {
      const auto* arm = find_volume_arm(summary, size, sign);
      for (int tick = 0; tick <= pre_return_tick; ++tick) {
        for (int channel = 0; channel < 2; ++channel) {
          const double a = channel == 0
              ? base->ticks[tick].doublet_ratio
              : base->ticks[tick].dynamic_energy_ratio;
          const double b = channel == 0
              ? arm->ticks[tick].doublet_ratio
              : arm->ticks[tick].dynamic_energy_ratio;
          locality_sum += static_cast<long double>(a - b) * (a - b);
          ++locality_count;
        }
      }
    }
    for (int size : volume_sizes) {
      const auto* arm = find_volume_arm(summary, size, sign);
      const auto& early = arm->ticks[4];
      const auto& pre = arm->ticks[pre_return_tick];
      summary.emission = summary.emission
          && pre.dynamic_energy_ratio > 0.0
          && pre.dynamic_norm_ratio > 0.0;
      summary.outward = summary.outward
          && pre.radius_second_moment - early.radius_second_moment >= 4.0;
    }
  }
  summary.locality_residual = std::sqrt(
      static_cast<double>(locality_sum / std::max(1, locality_count)));
  summary.locality = summary.locality_residual <= 0.05;

  std::vector<double> scaled_returns;
  int return_count = 0;
  for (const auto& arm : summary.arms) {
    if (arm.return_tick >= 0) {
      ++return_count;
      scaled_returns.push_back(
          static_cast<double>(arm.return_tick) / arm.volume);
    }
  }
  if (return_count == static_cast<int>(summary.arms.size())) {
    const double mean = std::accumulate(
        scaled_returns.begin(), scaled_returns.end(), 0.0)
        / scaled_returns.size();
    long double variance = 0.0;
    for (double value : scaled_returns) {
      variance += static_cast<long double>(value - mean) * (value - mean);
    }
    summary.return_scaled_cv = std::sqrt(
        static_cast<double>(variance / scaled_returns.size())) / mean;
    summary.return_classification = summary.return_scaled_cv <= 0.25
        ? "SCALED_RETURN" : "MIXED_RETURN";
  } else if (return_count == 0) {
    summary.return_classification = "NO_RETURN_IN_WINDOW";
  } else {
    summary.return_classification = "MIXED_RETURN";
  }
  summary.verdict = summary.locality && summary.emission && summary.outward
      ? "VOLUME_SCALED_PRE_RETURN_TRANSFER_V2_CONSTRUCTIVE"
      : "VOLUME_SCALED_INTERNAL_TRANSFER_V2_MIXED";
}

void write_volume_v2(const VolumeSummary& summary) {
  const auto directory = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results/ftd_0665";
  std::filesystem::create_directories(directory);
  std::ofstream json(directory / "ftd_0665_volume_scaled_internal_mode_transfer_v2.json");
  json << std::setprecision(17)
       << "{\n  \"ftd_id\": \"FTD-0665\",\n"
       << "  \"protocol_sha256\": \"" << volume_v2_protocol_sha256 << "\",\n"
       << "  \"parent_json_sha256\": \""
       << volume_v2_parent_json_sha256 << "\",\n"
       << "  \"verdict\": \"" << summary.verdict << "\",\n"
       << "  \"return_classification\": \""
       << summary.return_classification << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"arm_count\": " << summary.arms.size() << ",\n"
       << "  \"execution_pass\": " << summary.execution << ",\n"
       << "  \"locality_pass\": " << summary.locality << ",\n"
       << "  \"emission_pass\": " << summary.emission << ",\n"
       << "  \"outward_pass\": " << summary.outward << ",\n"
       << "  \"locality_rms\": " << summary.locality_residual << ",\n"
       << "  \"return_scaled_cv\": " << summary.return_scaled_cv << "\n}\n";

  std::ofstream arms(directory / "ftd_0665_volume_scaled_internal_mode_transfer_arms_v2.csv");
  arms << "ftd_id,volume,sign,initialized,executed,redressed,sector,"
          "horizon,return_tick,initial_doublet,max_energy_drift,max_common,"
          "max_decomposition,recovery,pre_doublet_ratio,pre_dynamic_energy_ratio,"
          "pre_dynamic_norm_ratio,r2_tick4,r2_tick16\n";
  for (const auto& arm : summary.arms) {
    arms << std::setprecision(17) << "FTD-0665," << arm.volume << ','
         << arm.sign << ',' << arm.initialized << ',' << arm.executed << ','
         << arm.redressed << ',' << arm.sector << ',' << 4 * arm.volume << ','
         << arm.return_tick << ',' << arm.initial_doublet << ','
         << arm.max_energy_drift << ',' << arm.max_common << ','
         << arm.max_decomposition << ',' << arm.recovery << ','
         << arm.ticks[pre_return_tick].doublet_ratio << ','
         << arm.ticks[pre_return_tick].dynamic_energy_ratio << ','
         << arm.ticks[pre_return_tick].dynamic_norm_ratio << ','
         << arm.ticks[4].radius_second_moment << ','
         << arm.ticks[pre_return_tick].radius_second_moment << '\n';
  }
  std::ofstream ticks(directory / "ftd_0665_volume_scaled_internal_mode_transfer_ticks_v2.csv");
  ticks << "ftd_id,volume,sign,tick,doublet_ratio,dynamic_energy_ratio,"
           "dynamic_norm_ratio,radius_second_moment,decomposition_residual,"
           "energy_drift,common_residual\n";
  for (const auto& arm : summary.arms) {
    for (const auto& tick : arm.ticks) {
      ticks << std::setprecision(17) << "FTD-0665," << arm.volume << ','
            << arm.sign << ',' << tick.tick << ',' << tick.doublet_ratio << ','
            << tick.dynamic_energy_ratio << ',' << tick.dynamic_norm_ratio << ','
            << tick.radius_second_moment << ','
            << tick.decomposition_residual << ',' << tick.energy_drift << ','
            << tick.common_residual << '\n';
    }
  }
}

}  // namespace

int main() {
  VolumeSummary summary;
  summary.parent = volume_parent_fingerprint();
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  summary.normalization = normalization.valid;
  summary.beta = normalization.mapped_field_work_coefficient;
  ftd::eft::ConnectedMooreBlockOptions options;
  options.allow_shared_anchor_chart = true;
  if (summary.parent && summary.normalization) {
    const auto reference = load_refined_state(0);
    const auto analytic = analytic_at(
        "volume_transfer_v2", 0, reference, summary.beta, options);
    if (analytic.valid) summary.modes = full_modes(analytic.hessian);
    summary.eigenspace = summary.modes.valid
        && summary.modes.modes[6].group == summary.modes.modes[7].group;
  }
  if (summary.eigenspace) {
    std::array<std::future<std::array<VolumeArm, 2>>, 3> futures;
    for (std::size_t index = 0; index < volume_sizes.size(); ++index) {
      futures[index] = std::async(
          std::launch::async,
          [&, size = volume_sizes[index]] {
            return run_volume_v2(size, summary.modes, summary.beta, options);
          });
    }
    for (std::size_t index = 0; index < volume_sizes.size(); ++index) {
      const auto volume_arms = futures[index].get();
      summary.arms.push_back(volume_arms[0]);
      summary.arms.push_back(volume_arms[1]);
      std::cout << "completed v2 L=" << volume_sizes[index] << std::endl;
    }
  }
  evaluate_volume_v2(summary);
  write_volume_v2(summary);
  std::cout << std::setprecision(17)
            << "protocol_sha256=" << volume_v2_protocol_sha256 << '\n'
            << "verdict=" << summary.verdict << '\n'
            << "execution=" << summary.execution
            << " locality=" << summary.locality
            << " emission=" << summary.emission
            << " outward=" << summary.outward
            << " locality_rms=" << summary.locality_residual << '\n'
            << "return=" << summary.return_classification
            << " scaled_cv=" << summary.return_scaled_cv << '\n';
  for (const auto& arm : summary.arms) {
    if (arm.ticks.size() > static_cast<std::size_t>(pre_return_tick)) {
      std::cout << "L=" << arm.volume << " sign=" << arm.sign
                << " executed=" << arm.executed
                << " pre_doublet="
                << arm.ticks[pre_return_tick].doublet_ratio
                << " pre_dynamic="
                << arm.ticks[pre_return_tick].dynamic_energy_ratio
                << " r2=" << arm.ticks[4].radius_second_moment << "->"
                << arm.ticks[pre_return_tick].radius_second_moment
                << " return=" << arm.return_tick
                << " recovery=" << arm.recovery << '\n';
    }
  }
  return summary.verdict
             == "VOLUME_SCALED_INTERNAL_TRANSFER_V2_EXECUTION_INVALID"
      ? 1 : 0;
}
