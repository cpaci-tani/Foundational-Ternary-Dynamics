/** FTD-0589: exact rectangular-pulse and arbitrary-removal bound. */

#include "ftd/eft/removal_time_pulse_bound.h"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

namespace {
int failures = 0;

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

std::string verdict(const ftd::eft::RemovalTimePulseBoundResult& result) {
  if (!result.valid) return "INVALID";
  return "ARBITRARY_REMOVAL_N_LE_6_CLOSED_NEXT_COUNT_7_UNRESOLVED";
}

void write_records(const ftd::eft::RemovalTimePulseBoundResult& result,
                   const std::string& final_verdict) {
  const fs::path output_dir = fs::path(__FILE__).parent_path().parent_path()
                            / "results" / "ftd_0589";
  fs::create_directories(output_dir);

  std::ofstream csv(output_dir / "windows_msvc_cpu.csv");
  csv << std::setprecision(17)
      << "history,L,N,polarity,shape_variant,seed,ticks,genesis_events,"
         "evaporation_events,all_originals_removed_tick,analytic_bound,"
         "maximum_flux,maximum_bound_excess,maximum_velocity,"
         "maximum_remainder,all_originals_removed,"
         "analytic_scope_respected,valid\n";
  for (const auto& arm : result.arms) {
    csv << ftd::eft::removal_history_name(arm.history) << ','
        << arm.lattice_size << ',' << arm.source_count << ','
        << arm.polarity << ',' << arm.shape_variant << ',' << arm.seed << ','
        << arm.ticks << ',' << arm.genesis_events << ','
        << arm.evaporation_events << ',' << arm.all_originals_removed_tick
        << ',' << arm.analytic_bound << ',' << arm.maximum_flux << ','
        << arm.maximum_bound_excess << ',' << arm.maximum_velocity << ','
        << arm.maximum_remainder << ',' << arm.all_originals_removed << ','
        << arm.analytic_scope_respected << ',' << arm.valid << '\n';
  }

  std::ofstream json(output_dir / "windows_msvc_cpu.json");
  json << std::setprecision(17)
       << "{\n"
       << "  \"identifier\": \"FTD-0589\",\n"
       << "  \"protocol\": \"removal-time pulse bound v1\",\n"
       << "  \"date\": \"2026-07-26\",\n"
       << "  \"platform\": \"Windows 11\",\n"
       << "  \"backend\": \"CPU\",\n"
       << "  \"production_tick_modified\": false,\n"
       << "  \"preregistration_sha256\": "
          "\"F438DBB1950E009641B1332D57B23B2EDFC23CD522A4E23C17E5FCC967AF5A33\",\n"
       << "  \"spectral_volumes\": [\n";
  for (std::size_t i = 0; i < result.volumes.size(); ++i) {
    const auto& volume = result.volumes[i];
    json << "    {\"L\": " << volume.lattice_size
         << ", \"maximum_mode_eigenvalue\": "
         << volume.maximum_mode_eigenvalue
         << ", \"one_source_step_triangle_bound\": "
         << volume.one_source_step_triangle_bound
         << ", \"exact_one_source_pulse_bound\": "
         << volume.exact_one_source_pulse_bound
         << ", \"common_step_coefficient\": "
         << volume.common_step_coefficient
         << ", \"uniform_closed_source_count\": "
         << volume.uniform_closed_source_count
         << ", \"first_source_count_not_excluded\": "
         << volume.first_source_count_not_excluded
         << ", \"maximizing_removed_at_closed_count\": "
         << volume.maximizing_removed_at_closed_count
         << ", \"maximizing_removed_at_first_open_count\": "
         << volume.maximizing_removed_at_first_open_count
         << ", \"closed_count_history_bound\": "
         << volume.closed_count_history_bound
         << ", \"closed_count_margin\": "
         << volume.closed_count_margin
         << ", \"first_open_count_history_bound\": "
         << volume.first_open_count_history_bound
         << ", \"first_open_count_margin\": "
         << volume.first_open_count_margin
         << ", \"continuous_relaxation_at_closed_count\": "
         << volume.continuous_relaxation_at_closed_count << "}"
         << (i + 1 == result.volumes.size() ? "\n" : ",\n");
  }
  json << "  ],\n"
       << "  \"uniform_closed_source_count\": "
       << result.uniform_closed_source_count << ",\n"
       << "  \"first_source_count_not_excluded\": "
       << result.first_source_count_not_excluded << ",\n"
       << "  \"pulse_identity_checks\": "
       << result.pulse_identity_checks << ",\n"
       << "  \"maximum_pulse_identity_residual\": "
       << result.maximum_pulse_identity_residual << ",\n"
       << "  \"gram_checks\": " << result.gram_checks << ",\n"
       << "  \"maximum_gram_residual\": "
       << result.maximum_gram_residual << ",\n"
       << "  \"maximum_translation_residual\": "
       << result.maximum_translation_residual << ",\n"
       << "  \"proper_cubic_rotation_arms\": "
       << result.proper_cubic_rotation_arms << ",\n"
       << "  \"maximum_cubic_covariance_residual\": "
       << result.maximum_cubic_covariance_residual << ",\n"
       << "  \"prescribed_history_arms\": "
       << result.prescribed_history_arms << ",\n"
       << "  \"native_unlocked_arms\": "
       << result.native_unlocked_arms << ",\n"
       << "  \"total_arms\": " << result.total_arms << ",\n"
       << "  \"total_ticks\": " << result.total_ticks << ",\n"
       << "  \"genesis_events\": " << result.genesis_events << ",\n"
       << "  \"evaporation_events\": "
       << result.evaporation_events << ",\n"
       << "  \"analytic_contradiction_events\": "
       << result.analytic_contradiction_events << ",\n"
       << "  \"unlocked_cells_with_complete_removal\": "
       << result.unlocked_cells_with_complete_removal << ",\n"
       << "  \"maximum_observed_flux\": "
       << result.maximum_observed_flux << ",\n"
       << "  \"maximum_bound_excess\": "
       << result.maximum_bound_excess << ",\n"
       << "  \"maximum_velocity\": " << result.maximum_velocity << ",\n"
       << "  \"maximum_remainder\": "
       << result.maximum_remainder << ",\n"
       << "  \"observer_neutral\": "
       << (result.observer_neutral ? "true" : "false") << ",\n"
       << "  \"structural_valid\": "
       << (result.valid ? "true" : "false") << ",\n"
       << "  \"verdict\": \"" << final_verdict << "\"\n"
       << "}\n";
}
}  // namespace

int main() {
  const auto result = ftd::eft::analyze_removal_time_pulse_bound();
  const std::string final_verdict = verdict(result);
  write_records(result, final_verdict);

  check("four registered spectral volumes execute",
        result.spectral_volume_count == 4 && result.volumes.size() == 4);
  check("constant step terms cancel in the exact pulse identity",
        result.exact_pulse_identity_derived
        && result.pulse_identity_checks == 8736
        && result.maximum_pulse_identity_residual <= 1e-12);
  check("continuous removal-history relaxation is valid",
        result.continuous_relaxation_derived);
  check("arbitrary one-time removals close through N=6",
        result.arbitrary_removal_n_le_six_closed
        && result.uniform_closed_source_count == 6);
  check("N=7 is the first count not excluded by this bound",
        result.seven_source_bound_inconclusive
        && result.first_source_count_not_excluded == 7);
  check("worst registered N=6 margin remains positive",
        result.volumes.back().closed_count_margin > 0.15
        && result.volumes.back().first_open_count_margin < 0.0);
  check("exact removal-time Gram identity closes",
        result.gram_identity_verified && result.gram_checks == 48
        && result.maximum_gram_residual <= 1e-12);
  check("integer translations preserve the residual kernel",
        result.translation_covariant
        && result.maximum_translation_residual <= 1e-12);
  check("all 24 proper cubic rotations preserve the residual kernel",
        result.cubic_covariant && result.proper_cubic_rotation_arms == 24
        && result.maximum_cubic_covariance_residual <= 1e-12);
  check("64 prescribed and 32 native-unlocked arms execute",
        result.prescribed_history_arms == 64
        && result.native_unlocked_arms == 32
        && result.total_arms == 96
        && result.total_ticks == 12288);
  check("all four unlocked volume/count cells exercise complete removal",
        result.residual_branch_exercised
        && result.unlocked_cells_with_complete_removal == 4);
  check("no theorem-closed arm produces descendant genesis",
        result.genesis_events == 0
        && result.analytic_contradiction_events == 0);
  check("all live histories respect the theorem bound",
        result.maximum_observed_flux > 0.0
        && result.maximum_bound_excess <= 1e-12);
  check("matter kinematics remain sanitized",
        result.maximum_velocity == 0.0
        && result.maximum_remainder == 0.0);
  check("history observer is state and RNG neutral", result.observer_neutral);
  check("production and defaults remain unchanged",
        !result.production_changed);
  check("registered FTD-0589 verdict closes", result.valid);

  std::cout << std::setprecision(17);
  for (const auto& volume : result.volumes) {
    std::cout << "L" << volume.lattice_size
              << "_maximum_mode_eigenvalue="
              << volume.maximum_mode_eigenvalue << '\n'
              << "L" << volume.lattice_size
              << "_one_source_step_triangle_bound="
              << volume.one_source_step_triangle_bound << '\n'
              << "L" << volume.lattice_size
              << "_exact_one_source_pulse_bound="
              << volume.exact_one_source_pulse_bound << '\n'
              << "L" << volume.lattice_size
              << "_common_step_coefficient="
              << volume.common_step_coefficient << '\n'
              << "L" << volume.lattice_size
              << "_uniform_closed_source_count="
              << volume.uniform_closed_source_count << '\n'
              << "L" << volume.lattice_size
              << "_first_source_count_not_excluded="
              << volume.first_source_count_not_excluded << '\n'
              << "L" << volume.lattice_size
              << "_maximizing_removed_at_closed_count="
              << volume.maximizing_removed_at_closed_count << '\n'
              << "L" << volume.lattice_size
              << "_maximizing_removed_at_first_open_count="
              << volume.maximizing_removed_at_first_open_count << '\n'
              << "L" << volume.lattice_size
              << "_closed_count_history_bound="
              << volume.closed_count_history_bound << '\n'
              << "L" << volume.lattice_size
              << "_closed_count_margin="
              << volume.closed_count_margin << '\n'
              << "L" << volume.lattice_size
              << "_first_open_count_history_bound="
              << volume.first_open_count_history_bound << '\n'
              << "L" << volume.lattice_size
              << "_first_open_count_margin="
              << volume.first_open_count_margin << '\n'
              << "L" << volume.lattice_size
              << "_continuous_relaxation_at_closed_count="
              << volume.continuous_relaxation_at_closed_count << '\n';
  }
  std::cout << "uniform_closed_source_count="
            << result.uniform_closed_source_count << '\n'
            << "first_source_count_not_excluded="
            << result.first_source_count_not_excluded << '\n'
            << "pulse_identity_checks=" << result.pulse_identity_checks
            << '\n'
            << "maximum_pulse_identity_residual="
            << result.maximum_pulse_identity_residual << '\n'
            << "gram_checks=" << result.gram_checks << '\n'
            << "maximum_gram_residual=" << result.maximum_gram_residual
            << '\n'
            << "maximum_translation_residual="
            << result.maximum_translation_residual << '\n'
            << "proper_cubic_rotation_arms="
            << result.proper_cubic_rotation_arms << '\n'
            << "maximum_cubic_covariance_residual="
            << result.maximum_cubic_covariance_residual << '\n'
            << "prescribed_history_arms="
            << result.prescribed_history_arms << '\n'
            << "native_unlocked_arms=" << result.native_unlocked_arms
            << '\n'
            << "total_arms=" << result.total_arms << '\n'
            << "total_ticks=" << result.total_ticks << '\n'
            << "genesis_events=" << result.genesis_events << '\n'
            << "evaporation_events=" << result.evaporation_events << '\n'
            << "analytic_contradiction_events="
            << result.analytic_contradiction_events << '\n'
            << "unlocked_cells_with_complete_removal="
            << result.unlocked_cells_with_complete_removal << '\n'
            << "maximum_observed_flux="
            << result.maximum_observed_flux << '\n'
            << "maximum_bound_excess=" << result.maximum_bound_excess
            << '\n'
            << "maximum_velocity=" << result.maximum_velocity << '\n'
            << "maximum_remainder=" << result.maximum_remainder << '\n'
            << "removal_time_pulse_bound failures=" << failures << '\n'
            << "verdict=" << final_verdict << '\n';
  return failures == 0 ? 0 : 1;
}
