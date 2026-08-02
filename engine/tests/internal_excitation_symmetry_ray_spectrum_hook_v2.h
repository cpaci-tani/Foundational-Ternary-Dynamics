#pragma once

// FTD-0699 correction layer over the immutable FTD-0698 observation core.

#include "internal_excitation_symmetry_ray_spectrum_hook.h"

namespace ftd0699 {

constexpr char protocol_sha256[] =
    "C1609A6060C5148A0D5B4B6334B862E2212C2C55B22579A25BC34858F7610858";

inline void observe(
    int tick,
    const ftd::eft::ConnectedMooreBlockState& control,
    const ftd::eft::ConnectedMooreBlockState& negative,
    const ftd::eft::ConnectedMooreBlockState& positive,
    const ftd::eft::ConnectedMooreBlockStepResult* control_step,
    const ftd::eft::ConnectedMooreBlockStepResult* negative_step,
    const ftd::eft::ConnectedMooreBlockStepResult* positive_step,
    const ftd::eft::ConnectedMooreBlockOptions& options) {
  ftd0698::observe(tick, control, negative, positive,
                   control_step, negative_step, positive_step, options);
}

inline void write_csv(const std::filesystem::path& directory) {
  std::ofstream output(directory /
      "ftd_0699_internal_excitation_symmetry_ray_spectrum_ticks_v2.csv");
  output << "ftd_id,tick,sign,ray,harmonic,omega,field_total,"
            "field_transverse,field_longitudinal,current_total,"
            "current_transverse,current_longitudinal,projection_residual,"
            "etx_re,etx_im,ety_re,ety_im,etz_re,etz_im,"
            "btx_re,btx_im,bty_re,bty_im,btz_re,btz_im,"
            "kx_re,kx_im,ky_re,ky_im,kz_re,kz_im\n";
  output << std::setprecision(17);
  for (const auto& row : ftd0698::recorder.rows) {
    output << "FTD-0699," << row.tick << ',' << row.sign << ',' << row.ray
           << ',' << row.harmonic << ',' << row.omega << ','
           << row.field_total << ',' << row.field_transverse << ','
           << row.field_longitudinal << ',' << row.current_total << ','
           << row.current_transverse << ',' << row.current_longitudinal << ','
           << row.projection_residual;
    for (const auto& vector : {row.electric_transverse,
                               row.magnetic_transverse,
                               row.current_coefficient})
      for (const auto& value : vector)
        output << ',' << value.real() << ',' << value.imag();
    output << '\n';
  }
}

template <typename Arms>
inline void finalize(bool parent_exact, double phase, const Arms&) {
  auto& recorder = ftd0698::recorder;
  recorder.finalized = true;
  recorder.summaries.clear();
  recorder.sign_field_residual = 0.0;
  recorder.sign_current_residual = 0.0;
  const std::size_t expected_rows = static_cast<std::size_t>(
      ftd0698::expected_horizon + 1) * 2 * 3 * ftd0698::harmonic_count;
  recorder.valid = recorder.valid && recorder.rows.size() == expected_rows
      && recorder.maximum_projection_residual <= 1e-12
      && std::abs(phase - 1.0911648733663635) <= 2e-15;

  for (int ray = 0; ray < 3; ++ray) {
    recorder.summaries.push_back(ftd0698::summarize(phase, -1, ray));
    recorder.summaries.push_back(ftd0698::summarize(phase, 1, ray));
  }
  bool eligible = true;
  bool constructive = true;
  bool peak_agreement = true;
  for (const auto& summary : recorder.summaries) {
    eligible = eligible && summary.eligible >= 8;
    constructive = constructive && summary.constructive;
  }
  for (int ray = 0; ray < 3; ++ray)
    peak_agreement = peak_agreement && std::abs(
        recorder.summaries[2 * ray].peak_harmonic
        - recorder.summaries[2 * ray + 1].peak_harmonic) <= 1;

  for (int ray = 0; ray < 3; ++ray) {
    std::array<double, ftd0698::harmonic_count> negative_field{};
    std::array<double, ftd0698::harmonic_count> positive_field{};
    std::array<double, ftd0698::harmonic_count> negative_current{};
    std::array<double, ftd0698::harmonic_count> positive_current{};
    for (int harmonic = 1; harmonic <= ftd0698::harmonic_count; ++harmonic) {
      const auto index = static_cast<std::size_t>(harmonic - 1);
      for (int tick = 1; tick <= ftd0698::expected_horizon; ++tick) {
        const auto* negative = ftd0698::find_row(tick, -1, ray, harmonic);
        const auto* positive = ftd0698::find_row(tick, 1, ray, harmonic);
        if (negative == nullptr || positive == nullptr) {
          recorder.valid = false;
          continue;
        }
        negative_field[index] += negative->field_transverse;
        positive_field[index] += positive->field_transverse;
        negative_current[index] += negative->current_total;
        positive_current[index] += positive->current_total;
      }
    }
    const double maximum_negative = *std::max_element(
        negative_current.begin(), negative_current.end());
    const double maximum_positive = *std::max_element(
        positive_current.begin(), positive_current.end());
    for (int harmonic = 0; harmonic < ftd0698::harmonic_count; ++harmonic) {
      if (negative_current[harmonic] < 1e-6 * maximum_negative
          || positive_current[harmonic] < 1e-6 * maximum_positive)
        continue;
      recorder.sign_field_residual = std::max(
          recorder.sign_field_residual, ftd0698::relative(
              negative_field[harmonic], positive_field[harmonic]));
      recorder.sign_current_residual = std::max(
          recorder.sign_current_residual, ftd0698::relative(
              negative_current[harmonic], positive_current[harmonic]));
    }
  }
  recorder.execution = parent_exact && recorder.valid && eligible
      && recorder.sign_field_residual <= 1e-4
      && recorder.sign_current_residual <= 1e-4;
  recorder.verdict = recorder.execution
      ? (constructive && peak_agreement
          ? "SYMMETRY_RAY_RESONANT_TRANSFER_CONSTRUCTIVE"
          : "SYMMETRY_RAY_SPECTRAL_TRANSFER_MIXED")
      : "INTERNAL_EXCITATION_SYMMETRY_RAY_SPECTRUM_EXECUTION_INVALID";

  const auto directory = std::filesystem::path(__FILE__).parent_path()
      .parent_path() / "results/ftd_0699";
  std::filesystem::create_directories(directory);
  write_csv(directory);
  std::ofstream json(directory /
      "ftd_0699_internal_excitation_symmetry_ray_spectrum_v2.json");
  json << std::setprecision(17)
       << "{\n  \"ftd_id\": \"FTD-0699\",\n"
       << "  \"protocol_sha256\": \"" << protocol_sha256 << "\",\n"
       << "  \"parent_invalid_result_sha256\": "
          "\"7421A810BA13A7592CF91ADFBFC27710A0FF25A5D19146C9FF784E4F8C6492EC\",\n"
       << "  \"verdict\": \"" << recorder.verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"execution_pass\": " << (recorder.execution ? 1 : 0) << ",\n"
       << "  \"row_count\": " << recorder.rows.size() << ",\n"
       << "  \"maximum_projection_residual\": "
       << recorder.maximum_projection_residual << ",\n"
       << "  \"sign_field_residual\": " << recorder.sign_field_residual
       << ",\n  \"sign_current_residual\": "
       << recorder.sign_current_residual << ",\n"
       << "  \"internal_phase\": " << phase << ",\n"
       << "  \"ray_summaries\": [\n";
  for (std::size_t index = 0; index < recorder.summaries.size(); ++index) {
    const auto& summary = recorder.summaries[index];
    json << "    {\"sign\": " << summary.sign
         << ", \"ray\": " << summary.ray
         << ", \"eligible\": " << summary.eligible
         << ", \"closest_harmonic\": " << summary.closest_harmonic
         << ", \"closest_omega\": " << summary.closest_omega
         << ", \"peak_harmonic\": " << summary.peak_harmonic
         << ", \"peak_omega\": " << summary.peak_omega
         << ", \"peak_detuning\": " << summary.peak_detuning
         << ", \"allowed_detuning\": " << summary.allowed_detuning
         << ", \"contrast\": " << summary.contrast
         << ", \"near_resonant\": " << (summary.near_resonant ? 1 : 0)
         << ", \"constructive\": " << (summary.constructive ? 1 : 0)
         << '}' << (index + 1 == recorder.summaries.size() ? "\n" : ",\n");
  }
  json << "  ]\n}\n";

  std::cout << std::setprecision(17)
            << "spectral_verdict=" << recorder.verdict
            << " rows=" << recorder.rows.size()
            << " phase=" << phase
            << " projection=" << recorder.maximum_projection_residual
            << " eligible_sign_field=" << recorder.sign_field_residual
            << " eligible_sign_current=" << recorder.sign_current_residual
            << '\n';
  for (const auto& summary : recorder.summaries)
    std::cout << "spectral sign=" << summary.sign << " ray=" << summary.ray
              << " eligible=" << summary.eligible
              << " closest=" << summary.closest_harmonic
              << " peak=" << summary.peak_harmonic
              << " peak_omega=" << summary.peak_omega
              << " detuning=" << summary.peak_detuning
              << " allowed=" << summary.allowed_detuning
              << " contrast=" << summary.contrast
              << " near=" << summary.near_resonant << '\n';
}

inline bool execution_valid() {
  return ftd0698::recorder.finalized && ftd0698::recorder.execution;
}

}  // namespace ftd0699
