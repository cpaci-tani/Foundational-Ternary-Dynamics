/** FTD-0588: collective common-history and asynchronous source bounds. */

#include "ftd/eft/collective_source_history_bound.h"

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

std::string verdict(
    const ftd::eft::CollectiveSourceHistoryBoundResult& result) {
  if (!result.valid) return "INVALID";
  if (result.five_source_residual_tail_observed)
    return "COMMON_N_LE_5_ASYNC_N_LE_4_CLOSED_"
           "N5_RESIDUAL_TAIL_GENESIS_OBSERVED";
  return "COMMON_N_LE_5_ASYNC_N_LE_4_CLOSED_"
         "N5_RESIDUAL_TAIL_UNRESOLVED";
}

void write_records(
    const ftd::eft::CollectiveSourceHistoryBoundResult& result,
    const std::string& final_verdict) {
  const fs::path output_dir = fs::path(__FILE__).parent_path().parent_path()
                            / "results" / "ftd_0588";
  fs::create_directories(output_dir);

  std::ofstream csv(output_dir / "windows_msvc_cpu.csv");
  csv << std::setprecision(17)
      << "history,L,N,polarity,chirality,translation,seed,ticks,"
         "genesis_events,evaporation_events,first_genesis_tick,"
         "originals_remaining_before_first_genesis,"
         "all_originals_removed_tick,analytic_bound,"
         "maximum_flux_in_analytic_scope,maximum_bound_excess,"
         "maximum_velocity,maximum_remainder,all_originals_removed,"
         "analytic_scope_respected,valid\n";
  for (const auto& arm : result.arms) {
    csv << ftd::eft::collective_source_history_name(arm.history) << ','
        << arm.lattice_size << ',' << arm.source_count << ','
        << arm.polarity << ',' << arm.chirality << ',' << arm.translation
        << ',' << arm.seed << ',' << arm.ticks << ',' << arm.genesis_events
        << ',' << arm.evaporation_events << ',' << arm.first_genesis_tick
        << ',' << arm.originals_remaining_before_first_genesis << ','
        << arm.all_originals_removed_tick << ',' << arm.analytic_bound << ','
        << arm.maximum_flux_in_analytic_scope << ','
        << arm.maximum_bound_excess << ',' << arm.maximum_velocity << ','
        << arm.maximum_remainder << ',' << arm.all_originals_removed << ','
        << arm.analytic_scope_respected << ',' << arm.valid << '\n';
  }

  std::ofstream json(output_dir / "windows_msvc_cpu.json");
  json << std::setprecision(17)
       << "{\n"
       << "  \"identifier\": \"FTD-0588\",\n"
       << "  \"protocol\": \"collective source-history bound v1\",\n"
       << "  \"date\": \"2026-07-26\",\n"
       << "  \"platform\": \"Windows 11\",\n"
       << "  \"backend\": \"CPU\",\n"
       << "  \"production_tick_modified\": false,\n"
       << "  \"preregistration_sha256\": "
          "\"06DE9E8B896272044D847FF5BEC53A342928E3B210B61AC4D3AD605D9D36692E\",\n"
       << "  \"spectral_volumes\": [\n";
  for (std::size_t i = 0; i < result.volumes.size(); ++i) {
    const auto& volume = result.volumes[i];
    json << "    {\"L\": " << volume.lattice_size
         << ", \"maximum_mode_eigenvalue\": "
         << volume.maximum_mode_eigenvalue
         << ", \"one_source_step_triangle_bound\": "
         << volume.one_source_step_triangle_bound
         << ", \"common_step_coefficient\": "
         << volume.common_step_coefficient
         << ", \"common_pulse_five_source_bound\": "
         << volume.common_pulse_five_source_bound
         << ", \"common_pulse_six_source_bound\": "
         << volume.common_pulse_six_source_bound
         << ", \"common_five_source_margin\": "
         << volume.common_five_source_margin
         << ", \"asynchronous_four_source_bound\": "
         << volume.asynchronous_four_source_bound
         << ", \"asynchronous_four_source_margin\": "
         << volume.asynchronous_four_source_margin
         << ", \"five_source_while_original_remains_bound\": "
         << volume.five_source_while_original_remains_bound
         << ", \"five_source_while_original_remains_margin\": "
         << volume.five_source_while_original_remains_margin
         << ", \"five_source_all_removed_envelope\": "
         << volume.five_source_all_removed_envelope
         << ", \"five_source_all_removed_margin\": "
         << volume.five_source_all_removed_margin
         << ", \"maximum_gradient_stencil_ratio\": "
         << volume.maximum_gradient_stencil_ratio << "}"
         << (i + 1 == result.volumes.size() ? "\n" : ",\n");
  }
  json << "  ],\n"
       << "  \"common_history_arms\": " << result.common_history_arms
       << ",\n"
       << "  \"native_unlocked_arms\": " << result.native_unlocked_arms
       << ",\n"
       << "  \"total_arms\": " << result.total_arms << ",\n"
       << "  \"total_ticks\": " << result.total_ticks << ",\n"
       << "  \"common_history_genesis_events\": "
       << result.common_history_genesis_events << ",\n"
       << "  \"asynchronous_four_source_genesis_events\": "
       << result.asynchronous_four_source_genesis_events << ",\n"
       << "  \"unlocked_five_source_genesis_events\": "
       << result.unlocked_five_source_genesis_events << ",\n"
       << "  \"five_source_residual_tail_genesis_events\": "
       << result.five_source_residual_tail_genesis_events << ",\n"
       << "  \"analytic_contradiction_events\": "
       << result.analytic_contradiction_events << ",\n"
       << "  \"unlocked_arms_all_sources_removed\": "
       << result.unlocked_arms_all_sources_removed << ",\n"
       << "  \"evaporation_events\": " << result.evaporation_events
       << ",\n"
       << "  \"maximum_parseval_error\": "
       << result.maximum_parseval_error << ",\n"
       << "  \"maximum_bound_excess\": "
       << result.maximum_bound_excess << ",\n"
       << "  \"maximum_velocity\": " << result.maximum_velocity << ",\n"
       << "  \"maximum_remainder\": " << result.maximum_remainder
       << ",\n"
       << "  \"observer_neutral\": "
       << (result.observer_neutral ? "true" : "false") << ",\n"
       << "  \"structural_valid\": "
       << (result.valid ? "true" : "false") << ",\n"
       << "  \"verdict\": \"" << final_verdict << "\"\n"
       << "}\n";
}
}  // namespace

int main() {
  const auto result = ftd::eft::analyze_collective_source_history_bound();
  const std::string final_verdict = verdict(result);
  write_records(result, final_verdict);

  check("four stable registered spectral volumes execute",
        result.spectral_volume_count == 4
        && result.volumes.size() == 4);
  check("gradient symbol is strictly dominated away from zero mode",
        result.stencil_dominance_derived);
  check("finite-group Parseval closes on all live fixtures",
        result.finite_group_parseval_derived
        && result.maximum_parseval_error <= 1e-12);
  check("common source histories close through N=5",
        result.common_history_n_le_five_closed
        && result.common_history_minimum_sources_not_excluded == 6);
  check("N=6 common pulse is not uniformly excluded",
        result.volumes.back().common_pulse_six_source_bound
            > 1.5163860591519780);
  check("arbitrary one-time removals close through N=4",
        result.asynchronous_n_le_four_closed
        && result.asynchronous_minimum_sources_not_excluded == 5);
  check("five sources remain closed until the last original vanishes",
        result.five_source_while_original_remains_closed
        && result.volumes.back().five_source_while_original_remains_margin
            > 0.0
        && result.volumes.back().five_source_all_removed_margin < 0.0);
  check("64 common and 64 unlocked arms execute",
        result.common_history_arms == 64
        && result.native_unlocked_arms == 64
        && result.total_arms == 128
        && result.total_ticks == 16384);
  check("common histories produce no descendant genesis",
        result.common_history_genesis_events == 0);
  check("asynchronous four-source histories produce no genesis",
        result.asynchronous_four_source_genesis_events == 0);
  check("registered N=5 residual branch is exercised",
        result.unlocked_arms_all_sources_removed > 0);
  check("no analytic contradiction is observed",
        result.analytic_contradiction_events == 0
        && result.maximum_bound_excess <= 1e-12);
  check("N=5 residual verdict is exhaustive",
        result.five_source_residual_tail_observed
            != result.five_source_residual_tail_unresolved);
  check("matter kinematics stay sanitized",
        result.maximum_velocity == 0.0
        && result.maximum_remainder == 0.0);
  check("history journal is state and RNG neutral", result.observer_neutral);
  check("production and defaults remain unchanged",
        !result.production_changed);
  check("registered FTD-0588 verdict closes", result.valid);

  std::cout << std::setprecision(17);
  for (const auto& volume : result.volumes) {
    std::cout << "L" << volume.lattice_size
              << "_maximum_mode_eigenvalue="
              << volume.maximum_mode_eigenvalue << '\n'
              << "L" << volume.lattice_size
              << "_one_source_step_triangle_bound="
              << volume.one_source_step_triangle_bound << '\n'
              << "L" << volume.lattice_size
              << "_common_step_coefficient="
              << volume.common_step_coefficient << '\n'
              << "L" << volume.lattice_size
              << "_common_pulse_five_source_bound="
              << volume.common_pulse_five_source_bound << '\n'
              << "L" << volume.lattice_size
              << "_common_pulse_six_source_bound="
              << volume.common_pulse_six_source_bound << '\n'
              << "L" << volume.lattice_size
              << "_common_five_source_margin="
              << volume.common_five_source_margin << '\n'
              << "L" << volume.lattice_size
              << "_asynchronous_four_source_bound="
              << volume.asynchronous_four_source_bound << '\n'
              << "L" << volume.lattice_size
              << "_asynchronous_four_source_margin="
              << volume.asynchronous_four_source_margin << '\n'
              << "L" << volume.lattice_size
              << "_five_source_while_original_remains_bound="
              << volume.five_source_while_original_remains_bound << '\n'
              << "L" << volume.lattice_size
              << "_five_source_while_original_remains_margin="
              << volume.five_source_while_original_remains_margin << '\n'
              << "L" << volume.lattice_size
              << "_five_source_all_removed_envelope="
              << volume.five_source_all_removed_envelope << '\n'
              << "L" << volume.lattice_size
              << "_five_source_all_removed_margin="
              << volume.five_source_all_removed_margin << '\n'
              << "L" << volume.lattice_size
              << "_maximum_gradient_stencil_ratio="
              << volume.maximum_gradient_stencil_ratio << '\n';
  }
  std::cout << "common_history_arms=" << result.common_history_arms << '\n'
            << "native_unlocked_arms=" << result.native_unlocked_arms << '\n'
            << "total_arms=" << result.total_arms << '\n'
            << "total_ticks=" << result.total_ticks << '\n'
            << "common_history_genesis_events="
            << result.common_history_genesis_events << '\n'
            << "asynchronous_four_source_genesis_events="
            << result.asynchronous_four_source_genesis_events << '\n'
            << "unlocked_five_source_genesis_events="
            << result.unlocked_five_source_genesis_events << '\n'
            << "five_source_residual_tail_genesis_events="
            << result.five_source_residual_tail_genesis_events << '\n'
            << "analytic_contradiction_events="
            << result.analytic_contradiction_events << '\n'
            << "unlocked_arms_all_sources_removed="
            << result.unlocked_arms_all_sources_removed << '\n'
            << "evaporation_events=" << result.evaporation_events << '\n'
            << "maximum_parseval_error="
            << result.maximum_parseval_error << '\n'
            << "maximum_bound_excess="
            << result.maximum_bound_excess << '\n'
            << "maximum_velocity=" << result.maximum_velocity << '\n'
            << "maximum_remainder=" << result.maximum_remainder << '\n'
            << "collective_source_history_bound failures=" << failures
            << '\n'
            << "verdict=" << final_verdict << '\n';
  return failures == 0 ? 0 : 1;
}
