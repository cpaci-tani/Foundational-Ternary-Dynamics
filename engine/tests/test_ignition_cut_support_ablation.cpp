/** FTD-0587: ignition-cut support-mechanism ablation. */

#include "ftd/eft/ignition_cut_support_ablation.h"

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

std::string verdict(const ftd::eft::IgnitionCutSupportAblationResult& result) {
  if (!result.valid) return "INVALID";
  if (result.mixed_or_unresolved) return "MIXED_OR_UNRESOLVED";
  if (result.reservoir_sufficient) return "RESERVOIR_SUFFICIENT";
  if (result.causal_state_source_sufficient)
    return "CAUSAL_STATE_SOURCE_SUFFICIENT";
  if (result.gauss_constraint_sufficient)
    return "GAUSS_CONSTRAINT_SUFFICIENT";
  if (result.state_only_persistence) return "STATE_ONLY_PERSISTENCE";
  return "NO_REGISTERED_SUPPORT_MECHANISM";
}

void write_records(const ftd::eft::IgnitionCutSupportAblationResult& result,
                   const std::string& final_verdict) {
  const fs::path output_dir = fs::path(__FILE__).parent_path().parent_path()
                            / "results" / "ftd_0587";
  fs::create_directories(output_dir);

  std::ofstream csv(output_dir / "windows_msvc_cpu.csv");
  csv << std::setprecision(17)
      << "arm,L,amplitude,seed,prefix_state_hash,prefix_rng_hash,"
         "support_hash_before,support_hash_after_intervention,cut_occupancy,"
         "final_occupancy,sample_count,minimum_sample_occupancy,"
         "maximum_sample_occupancy,final_positive,final_negative,genesis_events,"
         "evaporation_events,movement_events,annihilation_events,"
         "cut_quadratic_field_norm,post_intervention_field_norm,"
         "final_quadratic_field_norm,maximum_quadratic_field_norm,"
         "maximum_flux,maximum_wave_velocity,maximum_gauss_error,"
         "maximum_velocity_before_rebase,maximum_remainder_before_rebase,"
         "maximum_velocity_after_rebase,maximum_remainder_after_rebase,"
         "occupancy_cv,radius_cv,stable,finite,all_samples_valid,size_gate,"
         "prefix_kinematics_clean,"
         "support_preserved_by_intervention\n";
  for (const auto& run : result.runs) {
    csv << ftd::eft::ignition_cut_arm_name(run.arm) << ','
        << run.lattice_size << ',' << run.amplitude << ',' << run.seed << ','
        << run.prefix_state_hash << ',' << run.prefix_rng_hash << ','
        << run.support_hash_before << ','
        << run.support_hash_after_intervention << ','
        << run.cut_occupancy << ',' << run.final_occupancy << ','
        << run.sample_count << ',' << run.minimum_sample_occupancy << ','
        << run.maximum_sample_occupancy << ','
        << run.final_positive << ',' << run.final_negative << ','
        << run.genesis_events << ',' << run.evaporation_events << ','
        << run.movement_events << ',' << run.annihilation_events << ','
        << run.cut_quadratic_field_norm << ','
        << run.post_intervention_field_norm << ','
        << run.final_quadratic_field_norm << ','
        << run.maximum_quadratic_field_norm << ',' << run.maximum_flux << ','
        << run.maximum_wave_velocity << ',' << run.maximum_gauss_error << ','
        << run.maximum_velocity_before_rebase << ','
        << run.maximum_remainder_before_rebase << ','
        << run.maximum_velocity_after_rebase << ','
        << run.maximum_remainder_after_rebase << ','
        << run.occupancy_cv << ',' << run.radius_cv << ',' << run.stable << ','
        << run.finite << ',' << run.all_samples_valid << ',' << run.size_gate
        << ',' << run.prefix_kinematics_clean << ','
        << run.support_preserved_by_intervention << '\n';
  }

  std::ofstream json(output_dir / "windows_msvc_cpu.json");
  json << std::setprecision(17)
       << "{\n"
       << "  \"identifier\": \"FTD-0587\",\n"
       << "  \"protocol\": \"ignition-cut support ablation v1\",\n"
       << "  \"date\": \"2026-07-26\",\n"
       << "  \"platform\": \"Windows 11\",\n"
       << "  \"backend\": \"CPU\",\n"
       << "  \"production_tick_modified\": false,\n"
       << "  \"preregistration_sha256\": "
          "\"C2417CD829E665C6A4936D37DFA7C83F790925E5395FA387C34C03F27C857B2B\",\n"
       << "  \"volumes\": [24, 32],\n"
       << "  \"amplitudes_in_K_GENESIS\": [12, 20, 40],\n"
       << "  \"prefix_tick\": 150,\n"
       << "  \"final_tick\": 300,\n"
       << "  \"run_count\": " << result.run_count << ",\n"
       << "  \"distinct_prefix_cells\": "
       << result.distinct_prefix_cells << ",\n"
       << "  \"prefix_hash_mismatches\": "
       << result.prefix_hash_mismatches << ",\n"
       << "  \"prefix_rng_mismatches\": "
       << result.prefix_rng_mismatches << ",\n"
       << "  \"intervention_support_mismatches\": "
       << result.intervention_support_mismatches << ",\n"
       << "  \"observer_neutral\": "
       << (result.observer_neutral ? "true" : "false") << ",\n"
       << "  \"intact_projected_reproduced\": "
       << (result.intact_projected_reproduced ? "true" : "false") << ",\n"
       << "  \"arms\": [\n";
  for (std::size_t i = 0; i < result.arms.size(); ++i) {
    const auto& arm = result.arms[i];
    json << "    {\"name\": \""
         << ftd::eft::ignition_cut_arm_name(arm.arm)
         << "\", \"runs\": " << arm.run_count
         << ", \"stable_runs\": " << arm.stable_runs
         << ", \"passing_cells\": " << arm.passing_cells
         << ", \"support_qualified\": "
         << (arm.support_qualified ? "true" : "false")
         << ", \"genesis_events\": " << arm.genesis_events
         << ", \"evaporation_events\": " << arm.evaporation_events
         << ", \"movement_events\": " << arm.movement_events
         << ", \"annihilation_events\": " << arm.annihilation_events
         << ", \"mean_cut_field_norm\": " << arm.mean_cut_field_norm
         << ", \"mean_post_intervention_field_norm\": "
         << arm.mean_post_intervention_field_norm
         << ", \"mean_final_field_norm\": "
         << arm.mean_final_field_norm
         << ", \"maximum_gauss_error\": " << arm.maximum_gauss_error
         << "}" << (i + 1 == result.arms.size() ? "\n" : ",\n");
  }
  json << "  ],\n"
       << "  \"mechanisms\": {\n"
       << "    \"reservoir_sufficient\": "
       << (result.reservoir_sufficient ? "true" : "false") << ",\n"
       << "    \"causal_state_source_sufficient\": "
       << (result.causal_state_source_sufficient ? "true" : "false") << ",\n"
       << "    \"gauss_constraint_sufficient\": "
       << (result.gauss_constraint_sufficient ? "true" : "false") << ",\n"
       << "    \"state_only_persistence\": "
       << (result.state_only_persistence ? "true" : "false") << ",\n"
       << "    \"mixed_or_unresolved\": "
       << (result.mixed_or_unresolved ? "true" : "false") << ",\n"
       << "    \"no_registered_support_mechanism\": "
       << (result.no_registered_support_mechanism ? "true" : "false")
       << "\n  },\n"
       << "  \"structural_valid\": "
       << (result.valid ? "true" : "false") << ",\n"
       << "  \"verdict\": \"" << final_verdict << "\"\n"
       << "}\n";
}
}  // namespace

int main() {
  const auto result = ftd::eft::analyze_ignition_cut_support_ablation();
  const std::string final_verdict = verdict(result);
  write_records(result, final_verdict);

  check("144 locked continuations execute",
        result.run_count == 144 && result.total_ticks == 43200);
  check("24 prefixes replay bit-exactly across all six arms",
        result.distinct_prefix_cells == 24
        && result.prefix_hash_mismatches == 0
        && result.prefix_rng_mismatches == 0);
  check("field interventions preserve ternary support and labels",
        result.intervention_support_mismatches == 0);
  check("all records are finite", result.nonfinite_runs == 0);
  check("prefix and continuation kinematics are sanitized",
        result.dirty_prefix_kinematics_runs == 0
        && result.post_rebase_kinematics_runs == 0);
  check("movement and annihilation remain forbidden",
        result.forbidden_event_runs == 0);
  check("intact projector reproduces FTD-0474 reaction-dispersal",
        result.intact_projected_reproduced
        && result.arms[2].stable_runs == 20
        && result.arms[2].passing_cells == 5);
  check("history journal is state and RNG neutral", result.observer_neutral);

  const int sufficient_count =
      static_cast<int>(result.reservoir_sufficient)
      + static_cast<int>(result.causal_state_source_sufficient)
      + static_cast<int>(result.gauss_constraint_sufficient)
      + static_cast<int>(result.state_only_persistence);
  check("mechanism classification is exhaustive",
        (sufficient_count == 0) == result.no_registered_support_mechanism
        && ((sufficient_count > 1
             || (result.arms[1].support_qualified
                 && !result.arms[0].support_qualified
                 && !result.arms[4].support_qualified)
             || (result.arms[2].support_qualified
                 && !result.arms[0].support_qualified
                 && !result.arms[5].support_qualified))
            == result.mixed_or_unresolved));
  check("production and defaults remain unchanged",
        !result.production_changed);
  check("registered FTD-0587 structural verdict closes", result.valid);

  std::cout << std::setprecision(17);
  for (const auto& arm : result.arms) {
    const char* name = ftd::eft::ignition_cut_arm_name(arm.arm);
    std::cout << name << "_stable_runs=" << arm.stable_runs << '\n'
              << name << "_passing_cells=" << arm.passing_cells << '\n'
              << name << "_support_qualified="
              << static_cast<int>(arm.support_qualified) << '\n'
              << name << "_genesis_events=" << arm.genesis_events << '\n'
              << name << "_evaporation_events="
              << arm.evaporation_events << '\n'
              << name << "_mean_cut_field_norm="
              << arm.mean_cut_field_norm << '\n'
              << name << "_mean_post_intervention_field_norm="
              << arm.mean_post_intervention_field_norm << '\n'
              << name << "_mean_final_field_norm="
              << arm.mean_final_field_norm << '\n'
              << name << "_maximum_gauss_error="
              << arm.maximum_gauss_error << '\n';
  }
  std::cout << "reservoir_sufficient="
            << static_cast<int>(result.reservoir_sufficient) << '\n'
            << "causal_state_source_sufficient="
            << static_cast<int>(result.causal_state_source_sufficient) << '\n'
            << "gauss_constraint_sufficient="
            << static_cast<int>(result.gauss_constraint_sufficient) << '\n'
            << "state_only_persistence="
            << static_cast<int>(result.state_only_persistence) << '\n'
            << "mixed_or_unresolved="
            << static_cast<int>(result.mixed_or_unresolved) << '\n'
            << "no_registered_support_mechanism="
            << static_cast<int>(result.no_registered_support_mechanism) << '\n'
            << "ignition_cut_support_ablation failures=" << failures << '\n'
            << "verdict=" << final_verdict << '\n';
  return failures == 0 ? 0 : 1;
}
