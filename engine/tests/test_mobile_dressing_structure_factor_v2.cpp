// FTD-0656: corrected full rerun of the mobile dressing structure factor.

#define FTD_0655_EMBEDDED
#include "test_mobile_dressing_structure_factor.cpp"
#undef FTD_0655_EMBEDDED

namespace {

constexpr char protocol_sha256_v6[] =
    "898AF1958713038FC945D09DD4DEA434A213BC6F79DE44006F64D35A208C99E3";

std::string verdict_v6(const DressingSummary& summary) {
  if (!summary.coverage || !summary.execution || !summary.exact
      || !summary.coherence)
    return "MOBILE_DRESSING_STRUCTURE_FACTOR_V2_EXECUTION_INVALID";
  if (!summary.matter)
    return "MOBILE_MATTER_STRUCTURE_FACTOR_V2_CLOSED_NEGATIVE";
  if (summary.field && summary.mirror && summary.cubic && summary.width_trend)
    return "MOBILE_DRESSED_STRUCTURE_FACTOR_V2_CONSTRUCTIVE";
  return "MOBILE_CORE_FIELD_DRESSING_V2_MIXED";
}

void write_v6(const DressingSummary& summary, const std::string& verdict) {
  const auto dir = std::filesystem::path(__FILE__).parent_path().parent_path()
      /"results/ftd_0656";
  std::filesystem::create_directories(dir);
  std::ofstream json(dir/"ftd_0656_mobile_dressing_structure_factor_v2.json");
  json << std::setprecision(17)
       << "{\n  \"ftd_id\": \"FTD-0656\",\n"
       << "  \"protocol_sha256\": \"" << protocol_sha256_v6 << "\",\n"
       << "  \"verdict\": \"" << verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"arm_count\": " << summary.arms.size() << ",\n"
       << "  \"coverage_pass\": " << summary.coverage << ",\n"
       << "  \"execution_pass\": " << summary.execution << ",\n"
       << "  \"exact_pass\": " << summary.exact << ",\n"
       << "  \"coherence_pass\": " << summary.coherence << ",\n"
       << "  \"matter_pass\": " << summary.matter << ",\n"
       << "  \"field_pass\": " << summary.field << ",\n"
       << "  \"mirror_pass\": " << summary.mirror << ",\n"
       << "  \"cubic_pass\": " << summary.cubic << ",\n"
       << "  \"width_trend_pass\": " << summary.width_trend << ",\n"
       << "  \"worst_action\": " << summary.worst_action << ",\n"
       << "  \"worst_recovery\": " << summary.worst_recovery << ",\n"
       << "  \"worst_strain\": " << summary.worst_strain << ",\n"
       << "  \"mirror_residual\": " << summary.mirror_residual << ",\n"
       << "  \"cubic_residual\": " << summary.cubic_residual << ",\n"
       << "  \"width2_velocity_mismatch\": " << summary.max_velocity_mismatch.at(2) << ",\n"
       << "  \"width3_velocity_mismatch\": " << summary.max_velocity_mismatch.at(3) << ",\n"
       << "  \"width4_velocity_mismatch\": " << summary.max_velocity_mismatch.at(4) << ",\n"
       << "  \"width2_relative_phase_rms\": " << summary.max_relative_phase_rms.at(2) << ",\n"
       << "  \"width3_relative_phase_rms\": " << summary.max_relative_phase_rms.at(3) << ",\n"
       << "  \"width4_relative_phase_rms\": " << summary.max_relative_phase_rms.at(4) << ",\n"
       << "  \"width2_field_cv\": " << summary.max_field_cv.at(2) << ",\n"
       << "  \"width3_field_cv\": " << summary.max_field_cv.at(3) << ",\n"
       << "  \"width4_field_cv\": " << summary.max_field_cv.at(4) << "\n}\n";

  std::ofstream arms(dir/"ftd_0656_mobile_dressing_structure_factor_arms_v2.csv");
  arms << "ftd_id,label,kind,family,width,speed,initialized,forward,reverse,exact,coherent,matter_pass,field_pass,center_velocity,matter_velocity,field_velocity,matter_mean,field_mean,matter_phase_rms,field_phase_rms,matter_cv,field_cv,relative_phase_rms,matter_center_mismatch,field_matter_mismatch,max_action,max_strain,recovery\n";
  for (const auto& result : summary.arms) {
    const auto& arm = result.dynamics;
    arms << std::setprecision(17) << "FTD-0656," << arm.spec.label << ','
         << arm.spec.kind << ',' << arm.spec.family << ',' << arm.spec.width
         << ',' << arm.spec.speed << ',' << arm.initialized << ',' << arm.forward
         << ',' << arm.reverse << ',' << arm.exact << ',' << arm.coherent << ','
         << result.matter_pass << ',' << result.field_pass << ','
         << result.center_velocity << ',' << result.matter.phase_velocity << ','
         << result.field.phase_velocity << ',' << result.matter.mean_amplitude
         << ',' << result.field.mean_amplitude << ',' << result.matter.phase_rms
         << ',' << result.field.phase_rms << ',' << result.matter.amplitude_cv
         << ',' << result.field.amplitude_cv << ',' << result.relative_phase_rms
         << ',' << result.matter_center_mismatch << ','
         << result.field_matter_mismatch << ',' << arm.maximum_action << ','
         << arm.maximum_relative_edge_strain << ',' << arm.recovery << '\n';
  }

  std::ofstream series(dir/"ftd_0656_mobile_dressing_structure_factor_series_v2.csv");
  series << "ftd_id,label,tick,matter_real,matter_imag,field_real,field_imag,center_x,center_y,center_z\n";
  for (const auto& result : summary.arms)
    for (const auto& sample : result.samples)
      series << std::setprecision(17) << "FTD-0656,"
             << result.dynamics.spec.label << ',' << sample.tick << ','
             << sample.matter.real() << ',' << sample.matter.imag() << ','
             << sample.field.real() << ',' << sample.field.imag() << ','
             << sample.center.x << ',' << sample.center.y << ','
             << sample.center.z << '\n';
}

}  // namespace

int main() {
  DressingSummary summary;
  const auto specs = specs_v5();
  constexpr std::size_t batch = 6;
  for (std::size_t start = 0; start < specs.size(); start += batch) {
    std::vector<std::future<DressingArm>> futures;
    const std::size_t end = std::min(specs.size(),start+batch);
    for (std::size_t i = start; i < end; ++i)
      futures.push_back(std::async(std::launch::async,
          [spec=specs[i]]() { return run_arm_v5(spec); }));
    for (std::size_t i = start; i < end; ++i) {
      summary.arms.push_back(futures[i-start].get());
      std::cout << "completed " << specs[i].label << std::endl;
    }
  }
  evaluate_v5(summary);
  const std::string verdict = verdict_v6(summary);
  write_v6(summary,verdict);
  std::cout << std::setprecision(17)
            << "protocol_sha256=" << protocol_sha256_v6 << '\n'
            << "verdict=" << verdict << '\n'
            << "coverage=" << summary.coverage
            << " execution=" << summary.execution
            << " exact=" << summary.exact
            << " coherence=" << summary.coherence
            << " matter=" << summary.matter
            << " field=" << summary.field
            << " mirror=" << summary.mirror
            << " cubic=" << summary.cubic
            << " trend=" << summary.width_trend << '\n';
  return verdict == "MOBILE_DRESSING_STRUCTURE_FACTOR_V2_EXECUTION_INVALID"
      ? 1 : 0;
}
