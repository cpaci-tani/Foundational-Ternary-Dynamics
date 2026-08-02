// FTD-0662: generalized-amplitude-normalized tight-frame covariance.
#define FTD_0661_EMBEDDED
#include "test_internal_mode_action_transfer_v2.cpp"
#undef FTD_0661_EMBEDDED

namespace {

constexpr char transfer_v3_protocol_sha256[] =
    "F517C08CB66B6AE2388CBE3C04E1EE5429C4B596723C41732A443649C542136D";
constexpr char transfer_v3_parent_sha256[] =
    "45E0AFAB3E986C72A06252087DDB06F662754964013109E92A205AD37C22C421";

double normalized_tight_frame_history_residual(
    const TransferSummary& summary,
    int amplitude,
    int quadrature) {
  std::array<std::array<const TransferArm*, 4>, 2> arms{};
  for (int orientation = 0; orientation < 2; ++orientation) {
    for (int polarization = 0; polarization < 4; ++polarization) {
      arms[orientation][polarization] = find_transfer_arm(
          summary, orientation, polarization, amplitude, quadrature);
      if (!arms[orientation][polarization]
          || !(arms[orientation][polarization]->ticks.front().doublet_energy > 0.0)) {
        return INFINITY;
      }
    }
  }
  long double difference = 0.0, norm_x = 0.0, norm_y = 0.0;
  auto add = [&](double x, double y) {
    difference += static_cast<long double>(x - y) * (x - y);
    norm_x += static_cast<long double>(x) * x;
    norm_y += static_cast<long double>(y) * y;
  };
  for (int tick = 0; tick <= transfer_ticks; ++tick) {
    std::array<TransferTick, 2> sums{};
    for (int orientation = 0; orientation < 2; ++orientation) {
      for (int polarization = 0; polarization < 4; ++polarization) {
        const auto* arm = arms[orientation][polarization];
        const auto& source = arm->ticks[tick];
        const double scale = arm->ticks.front().doublet_energy;
        auto& target = sums[orientation];
        target.doublet_energy += source.doublet_energy / scale;
        target.kinetic_excitation += source.kinetic_excitation / scale;
        target.binding_excitation += source.binding_excitation / scale;
        target.field_excitation += source.field_excitation / scale;
        target.dressing_excitation += source.dressing_excitation / scale;
        target.residual_field_energy += source.residual_field_energy / scale;
        target.field_interference += source.field_interference / scale;
        target.residual_norm += source.residual_norm / scale;
        target.near_norm += source.near_norm / scale;
        target.middle_norm += source.middle_norm / scale;
        target.far_norm += source.far_norm / scale;
      }
    }
    add(sums[0].doublet_energy, sums[1].doublet_energy);
    add(sums[0].kinetic_excitation, sums[1].kinetic_excitation);
    add(sums[0].binding_excitation, sums[1].binding_excitation);
    add(sums[0].field_excitation, sums[1].field_excitation);
    add(sums[0].dressing_excitation, sums[1].dressing_excitation);
    add(sums[0].residual_field_energy, sums[1].residual_field_energy);
    add(sums[0].field_interference, sums[1].field_interference);
    add(sums[0].residual_norm, sums[1].residual_norm);
    add(sums[0].near_norm, sums[1].near_norm);
    add(sums[0].middle_norm, sums[1].middle_norm);
    add(sums[0].far_norm, sums[1].far_norm);
  }
  return std::sqrt(static_cast<double>(difference))
      / std::max({1e-300, std::sqrt(static_cast<double>(norm_x)),
                  std::sqrt(static_cast<double>(norm_y))});
}

void evaluate_transfer_v3(TransferSummary& summary) {
  evaluate_transfer_v2(summary);
  summary.covariance = summary.execution;
  summary.covariance_residual = 0.0;
  for (int amplitude : {1, 2}) {
    for (int quadrature : {1, 3}) {
      const double residual = normalized_tight_frame_history_residual(
          summary, amplitude, quadrature);
      summary.covariance_residual = std::max(
          summary.covariance_residual, residual);
      summary.covariance = summary.covariance && residual <= 0.05;
    }
  }
  if (!summary.parent || !summary.normalization || !summary.eigenspace
      || !summary.coverage || !summary.execution) {
    summary.verdict = "INTERNAL_MODE_ACTION_TRANSFER_EXECUTION_INVALID";
  } else if (!summary.bounded) {
    summary.verdict = "INTERNAL_MODE_ACTION_TRANSFER_CLOSED_NEGATIVE";
  } else if (summary.transfer && summary.amplitude && summary.sign
             && summary.covariance && summary.zero
             && summary.dynamic_morphology) {
    summary.verdict = "INTERNAL_MODE_DYNAMIC_FIELD_TRANSFER_CONSTRUCTIVE";
  } else if (summary.transfer && summary.amplitude && summary.sign
             && summary.covariance && summary.zero
             && summary.local_morphology) {
    summary.verdict = "INTERNAL_MODE_LOCAL_HYBRID_TRANSFER_CONSTRUCTIVE";
  } else {
    summary.verdict = "INTERNAL_MODE_ACTION_TRANSFER_MIXED";
  }
}

void write_transfer_v3(const TransferSummary& summary) {
  const auto directory = std::filesystem::path(__FILE__).parent_path().parent_path()
      / "results/ftd_0662";
  std::filesystem::create_directories(directory);
  std::ofstream json(directory / "ftd_0662_internal_mode_action_transfer_v3.json");
  json << std::setprecision(17)
       << "{\n  \"ftd_id\": \"FTD-0662\",\n"
       << "  \"protocol_sha256\": \"" << transfer_v3_protocol_sha256 << "\",\n"
       << "  \"parent_result_sha256\": \"" << transfer_v3_parent_sha256 << "\",\n"
       << "  \"verdict\": \"" << summary.verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"arm_count\": " << summary.arms.size() << ",\n"
       << "  \"ticks_each_direction\": " << transfer_ticks << ",\n"
       << "  \"coverage_pass\": " << summary.coverage << ",\n"
       << "  \"execution_pass\": " << summary.execution << ",\n"
       << "  \"bounded_pass\": " << summary.bounded << ",\n"
       << "  \"transfer_pass\": " << summary.transfer << ",\n"
       << "  \"amplitude_pass\": " << summary.amplitude << ",\n"
       << "  \"sign_pass\": " << summary.sign << ",\n"
       << "  \"covariance_pass\": " << summary.covariance << ",\n"
       << "  \"zero_control_pass\": " << summary.zero << ",\n"
       << "  \"dynamic_morphology_pass\": " << summary.dynamic_morphology << ",\n"
       << "  \"local_morphology_pass\": " << summary.local_morphology << ",\n"
       << "  \"worst_common_residual\": " << summary.worst_common << ",\n"
       << "  \"worst_energy_drift\": " << summary.worst_energy_drift << ",\n"
       << "  \"worst_decomposition_residual\": "
       << summary.worst_decomposition_residual << ",\n"
       << "  \"worst_recovery\": " << summary.worst_recovery << ",\n"
       << "  \"minimum_doublet_ratio\": " << summary.minimum_doublet_ratio << ",\n"
       << "  \"minimum_dynamic_ratio\": " << summary.minimum_dynamic_ratio << ",\n"
       << "  \"minimum_far_fraction\": " << summary.minimum_far_fraction << ",\n"
       << "  \"maximum_far_fraction\": " << summary.maximum_far_fraction << ",\n"
       << "  \"amplitude_residual\": " << summary.amplitude_residual << ",\n"
       << "  \"sign_residual\": " << summary.sign_residual << ",\n"
       << "  \"normalized_tight_frame_covariance_residual\": "
       << summary.covariance_residual << ",\n"
       << "  \"zero_residual\": " << summary.zero_residual << "\n}\n";

  std::ofstream arms(directory / "ftd_0662_internal_mode_action_transfer_arms_v3.csv");
  arms << "ftd_id,label,orientation,polarization,amplitude,quadrature,zero,"
          "initialization,forward,reverse,bounded,redress,initial_excitation,"
          "initial_doublet_energy,min_doublet_ratio,recovered_doublet_ratio,"
          "max_dynamic_ratio,max_far_fraction,near_onset,middle_onset,far_onset,"
          "max_common,max_energy_drift,max_decomposition_residual,recovery\n";
  for (const auto& arm : summary.arms) {
    const double initial_doublet = arm.ticks.empty()
        ? NAN : arm.ticks.front().doublet_energy;
    arms << std::setprecision(17) << "FTD-0662," << arm.spec.label << ','
         << arm.spec.orientation << ',' << arm.spec.polarization << ','
         << arm.spec.amplitude << ',' << arm.spec.quadrature << ','
         << arm.spec.zero << ',' << arm.initialization << ',' << arm.forward
         << ',' << arm.reverse << ',' << arm.bounded << ',' << arm.redress << ','
         << arm.initial_excitation << ',' << initial_doublet << ','
         << arm.min_doublet_ratio << ',' << arm.recovered_doublet_ratio << ','
         << arm.max_dynamic_ratio << ',' << arm.max_far_fraction << ','
         << arm.near_onset << ',' << arm.middle_onset << ',' << arm.far_onset
         << ',' << arm.max_common << ',' << arm.max_energy_drift << ','
         << arm.max_decomposition_residual << ',' << arm.recovery << '\n';
  }
  std::ofstream ticks(directory / "ftd_0662_internal_mode_action_transfer_ticks_v3.csv");
  ticks << "ftd_id,label,tick,redress,doublet_energy,doublet_ratio,"
           "kinetic_excitation,binding_excitation,field_excitation,"
           "dressing_excitation,residual_field_energy,field_interference,"
           "total_excitation,field_decomposition_residual,total_energy_drift,"
           "other_matter_norm,residual_norm,near_norm,middle_norm,far_norm,"
           "far_fraction,common\n";
  for (const auto& arm : summary.arms) {
    for (const auto& tick : arm.ticks) {
      ticks << std::setprecision(17) << "FTD-0662," << arm.spec.label << ','
            << tick.tick << ',' << tick.redress << ',' << tick.doublet_energy
            << ',' << tick.doublet_ratio << ',' << tick.kinetic_excitation
            << ',' << tick.binding_excitation << ',' << tick.field_excitation
            << ',' << tick.dressing_excitation << ','
            << tick.residual_field_energy << ',' << tick.field_interference
            << ',' << tick.total_excitation << ','
            << tick.field_decomposition_residual << ','
            << tick.total_energy_drift << ',' << tick.other_matter_norm << ','
            << tick.residual_norm << ',' << tick.near_norm << ','
            << tick.middle_norm << ',' << tick.far_norm << ','
            << tick.far_fraction << ',' << tick.common << '\n';
    }
  }
}

}  // namespace

int main() {
  TransferSummary summary;
  summary.parent = mode_parent_fingerprint();
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  summary.normalization = normalization.valid;
  summary.beta = normalization.mapped_field_work_coefficient;
  ftd::eft::ConnectedMooreBlockOptions options;
  options.allow_shared_anchor_chart = true;
  if (summary.parent && summary.normalization) {
    for (int orientation = 0; orientation < 2; ++orientation) {
      const auto state = load_refined_state(orientation);
      const auto analytic = analytic_at(
          orientation == 0 ? "transfer_v3_x" : "transfer_v3_y",
          orientation, state, summary.beta, options);
      if (analytic.valid) summary.modes[orientation] = full_modes(analytic.hessian);
    }
    summary.eigenspace = summary.modes[0].valid && summary.modes[1].valid
        && summary.modes[0].modes[6].group == summary.modes[0].modes[7].group
        && relative_value(summary.modes[0].modes[6].hessian_eigen,
                          summary.modes[0].modes[7].hessian_eigen) <= 1e-9;
  }
  std::vector<ClockSpec> specs;
  if (summary.eigenspace) {
    for (int orientation = 0; orientation < 2; ++orientation) {
      for (int polarization = 0; polarization < 4; ++polarization) {
        for (int amplitude : {1, 2}) {
          for (int quadrature : {1, 3}) {
            ClockSpec spec;
            spec.orientation = orientation;
            spec.polarization = polarization;
            spec.amplitude = amplitude;
            spec.quadrature = quadrature;
            spec.label = "o" + std::to_string(orientation)
                + "_p" + std::to_string(polarization)
                + "_a" + std::to_string(amplitude)
                + "_q" + std::to_string(quadrature);
            specs.push_back(spec);
          }
        }
      }
      ClockSpec zero;
      zero.orientation = orientation;
      zero.zero = true;
      zero.label = "o" + std::to_string(orientation) + "_zero";
      specs.push_back(zero);
    }
  }
  constexpr std::size_t batch = 24;
  for (std::size_t start = 0; start < specs.size(); start += batch) {
    std::vector<std::future<TransferArm>> futures;
    const auto end = std::min(specs.size(), start + batch);
    for (std::size_t index = start; index < end; ++index) {
      futures.push_back(std::async(
          std::launch::async,
          [&, spec = specs[index]] {
            return run_transfer_arm(
                spec, summary.modes[spec.orientation], summary.beta, options);
          }));
    }
    for (std::size_t index = start; index < end; ++index) {
      summary.arms.push_back(futures[index - start].get());
      std::cout << "completed " << specs[index].label << std::endl;
    }
  }
  evaluate_transfer_v3(summary);
  write_transfer_v3(summary);
  std::cout << std::setprecision(17)
            << "protocol_sha256=" << transfer_v3_protocol_sha256 << '\n'
            << "verdict=" << summary.verdict << '\n'
            << "coverage=" << summary.coverage
            << " execution=" << summary.execution
            << " bounded=" << summary.bounded
            << " transfer=" << summary.transfer
            << " amplitude=" << summary.amplitude
            << " sign=" << summary.sign
            << " covariance=" << summary.covariance
            << " zero=" << summary.zero
            << " dynamic=" << summary.dynamic_morphology
            << " local=" << summary.local_morphology << '\n'
            << "normalized_tight_frame_covariance="
            << summary.covariance_residual
            << " zero_residual=" << summary.zero_residual << '\n';
  return summary.verdict == "INTERNAL_MODE_ACTION_TRANSFER_EXECUTION_INVALID"
      ? 1 : 0;
}
