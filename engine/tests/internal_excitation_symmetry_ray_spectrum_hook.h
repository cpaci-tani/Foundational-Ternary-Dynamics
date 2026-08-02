#pragma once

// FTD-0698 hook for test_causal_excitation_separation_v1.cpp.

#include "ftd/eft/matched_symmetry_ray_spectrum.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <numeric>
#include <string>
#include <vector>

namespace ftd0698 {

constexpr char protocol_sha256[] =
    "2C48B1788191F83C3C92F01F1AF78F21F4590F4D27F0CF381C1D3BFEECFFF5E0";
constexpr int expected_horizon = 96;
constexpr int harmonic_count = 56;

struct Row {
  int tick = 0;
  int sign = 0;
  int ray = 0;
  int harmonic = 0;
  double omega = 0.0;
  double field_total = 0.0;
  double field_transverse = 0.0;
  double field_longitudinal = 0.0;
  double current_total = 0.0;
  double current_transverse = 0.0;
  double current_longitudinal = 0.0;
  double projection_residual = 0.0;
  ftd::eft::MatchedComplexVector electric_transverse{};
  ftd::eft::MatchedComplexVector magnetic_transverse{};
  ftd::eft::MatchedComplexVector current_coefficient{};
};

struct RaySummary {
  int sign = 0;
  int ray = 0;
  int eligible = 0;
  int closest_harmonic = 0;
  int peak_harmonic = 0;
  double closest_omega = 0.0;
  double peak_omega = 0.0;
  double peak_detuning = INFINITY;
  double allowed_detuning = 0.0;
  double contrast = 0.0;
  bool near_resonant = false;
  bool constructive = false;
};

struct Recorder {
  bool valid = true;
  bool finalized = false;
  bool execution = false;
  double maximum_projection_residual = 0.0;
  double sign_field_residual = 0.0;
  double sign_current_residual = 0.0;
  std::string verdict =
      "INTERNAL_EXCITATION_SYMMETRY_RAY_SPECTRUM_EXECUTION_INVALID";
  std::vector<Row> rows;
  std::vector<RaySummary> summaries;
};

inline Recorder recorder;

inline std::vector<ftd::eft::MatchedSymmetryRayRequest> requests() {
  std::vector<int> harmonics(harmonic_count);
  std::iota(harmonics.begin(), harmonics.end(), 1);
  return {{{{1, 0, 0}}, harmonics},
          {{{1, 1, 0}}, harmonics},
          {{{1, 1, 1}}, harmonics}};
}

inline ftd::eft::MatchedFaceFlux aggregate_current(
    const ftd::eft::ConnectedMooreBlockStepResult* step,
    int L,
    double scale) {
  ftd::eft::MatchedFaceFlux result(L);
  if (step == nullptr) return result;
  for (const auto& segment : step->segments) {
    if (segment.dense_materialized) {
      for (std::size_t index = 0; index < result.x.size(); ++index) {
        result.x[index] += scale * segment.current_x[index];
        result.y[index] += scale * segment.current_y[index];
        result.z[index] += scale * segment.current_z[index];
      }
    } else {
      for (const auto& entry : segment.sparse_current) {
        const auto index = static_cast<std::size_t>(result.index(
            entry.face.x, entry.face.y, entry.face.z));
        auto& component = entry.axis == 0 ? result.x
            : (entry.axis == 1 ? result.y : result.z);
        component[index] += scale * entry.value;
      }
    }
  }
  return result;
}

inline double omega(const ftd::eft::MatchedWavevectorSpectrum& spectrum) {
  double sum = 0.0;
  for (double component : spectrum.lattice_wavevector)
    sum += 0.25 * component * component;
  return 2.0 * std::asin(std::sqrt(sum / 3.0));
}

inline void append(int tick,
                   int sign,
                   const ftd::eft::MatchedSymmetryRayBatch& field,
                   const ftd::eft::MatchedSymmetryRayBatch& current) {
  if (!field.valid || !current.valid
      || field.spectra.size() != 3 * harmonic_count
      || current.spectra.size() != field.spectra.size()) {
    recorder.valid = false;
    return;
  }
  for (std::size_t index = 0; index < field.spectra.size(); ++index) {
    const auto& f = field.spectra[index];
    const auto& k = current.spectra[index];
    if (!f.valid || !k.valid || f.mode != k.mode) {
      recorder.valid = false;
      return;
    }
    Row row;
    row.tick = tick;
    row.sign = sign;
    row.ray = static_cast<int>(index / harmonic_count);
    row.harmonic = static_cast<int>(index % harmonic_count) + 1;
    row.omega = omega(f);
    row.field_total = f.total_power;
    row.field_transverse = f.transverse_power;
    row.field_longitudinal = f.longitudinal_power;
    row.current_total = k.total_power;
    row.current_transverse = k.transverse_power;
    row.current_longitudinal = k.longitudinal_power;
    row.projection_residual = std::max({
        f.electric_projection_residual, f.magnetic_projection_residual,
        k.electric_projection_residual, k.magnetic_projection_residual});
    row.electric_transverse = f.electric_transverse;
    row.magnetic_transverse = f.magnetic_transverse;
    row.current_coefficient = k.electric;
    recorder.maximum_projection_residual = std::max(
        recorder.maximum_projection_residual, row.projection_residual);
    recorder.rows.push_back(std::move(row));
  }
}

inline void observe(
    int tick,
    const ftd::eft::ConnectedMooreBlockState& control,
    const ftd::eft::ConnectedMooreBlockState& negative,
    const ftd::eft::ConnectedMooreBlockState& positive,
    const ftd::eft::ConnectedMooreBlockStepResult* control_step,
    const ftd::eft::ConnectedMooreBlockStepResult* negative_step,
    const ftd::eft::ConnectedMooreBlockStepResult* positive_step,
    const ftd::eft::ConnectedMooreBlockOptions& options) {
  if (!recorder.valid) return;
  const auto ray_requests = requests();
  const auto zero_b = ftd::eft::MatchedEdgeField(control.electric.L);
  const auto control_current = aggregate_current(
      control_step, control.electric.L, options.polarity_scale);
  const std::array<const ftd::eft::ConnectedMooreBlockState*, 2> candidates{{
      &negative, &positive}};
  const std::array<const ftd::eft::ConnectedMooreBlockStepResult*, 2> steps{{
      negative_step, positive_step}};
  for (int arm = 0; arm < 2; ++arm) {
    const auto field = ftd::eft::observe_batched_matched_symmetry_ray_spectra(
        control.electric, control.magnetic_half,
        candidates[arm]->electric, candidates[arm]->magnetic_half,
        ray_requests, options.wave_speed);
    const auto candidate_current = aggregate_current(
        steps[arm], control.electric.L, options.polarity_scale);
    const auto current = ftd::eft::observe_batched_matched_symmetry_ray_spectra(
        control_current, zero_b, candidate_current, zero_b,
        ray_requests, options.wave_speed);
    append(tick, arm == 0 ? -1 : 1, field, current);
  }
}

inline double relative(double left, double right) {
  if (left == 0.0 && right == 0.0) return 0.0;
  return std::abs(left - right)
      / std::max({std::abs(left), std::abs(right), 1e-300});
}

inline const Row* find_row(int tick, int sign, int ray, int harmonic) {
  const std::size_t per_tick = 2 * 3 * harmonic_count;
  const std::size_t sign_offset = sign < 0 ? 0 : 3 * harmonic_count;
  const std::size_t index = static_cast<std::size_t>(tick) * per_tick
      + sign_offset + static_cast<std::size_t>(ray) * harmonic_count
      + static_cast<std::size_t>(harmonic - 1);
  return index < recorder.rows.size() ? &recorder.rows[index] : nullptr;
}

inline RaySummary summarize(double phi, int sign, int ray) {
  RaySummary result;
  result.sign = sign;
  result.ray = ray;
  std::array<double, harmonic_count> field{};
  std::array<double, harmonic_count> current{};
  std::array<double, harmonic_count> frequencies{};
  for (int harmonic = 1; harmonic <= harmonic_count; ++harmonic) {
    for (int tick = 1; tick <= expected_horizon; ++tick) {
      const auto* row = find_row(tick, sign, ray, harmonic);
      if (row == nullptr) return result;
      field[static_cast<std::size_t>(harmonic - 1)] += row->field_transverse;
      current[static_cast<std::size_t>(harmonic - 1)] += row->current_total;
      frequencies[static_cast<std::size_t>(harmonic - 1)] = row->omega;
    }
  }
  const double maximum_current = *std::max_element(
      current.begin(), current.end());
  if (!(maximum_current > 0.0)) return result;
  int closest = 0;
  for (int index = 1; index < harmonic_count; ++index)
    if (std::abs(frequencies[index] - phi)
        < std::abs(frequencies[closest] - phi))
      closest = index;
  result.closest_harmonic = closest + 1;
  result.closest_omega = frequencies[closest];
  const double left_spacing = closest > 0
      ? std::abs(frequencies[closest] - frequencies[closest - 1]) : 0.0;
  const double right_spacing = closest + 1 < harmonic_count
      ? std::abs(frequencies[closest + 1] - frequencies[closest]) : 0.0;
  result.allowed_detuning = std::max(left_spacing, right_spacing);

  std::vector<double> response;
  std::vector<int> eligible_index;
  for (int index = 0; index < harmonic_count; ++index) {
    if (current[index] < 1e-6 * maximum_current) continue;
    response.push_back(field[index] / current[index]);
    eligible_index.push_back(index);
  }
  result.eligible = static_cast<int>(response.size());
  if (response.empty()) return result;
  const auto maximum = std::max_element(response.begin(), response.end());
  const auto maximum_index = static_cast<std::size_t>(
      std::distance(response.begin(), maximum));
  const int peak = eligible_index[maximum_index];
  result.peak_harmonic = peak + 1;
  result.peak_omega = frequencies[peak];
  result.peak_detuning = std::abs(result.peak_omega - phi);
  std::vector<double> ordered = response;
  std::sort(ordered.begin(), ordered.end());
  const double median = ordered.size() % 2 == 0
      ? 0.5 * (ordered[ordered.size() / 2 - 1]
               + ordered[ordered.size() / 2])
      : ordered[ordered.size() / 2];
  result.contrast = median > 0.0 ? *maximum / median : INFINITY;
  result.near_resonant = result.peak_detuning <= result.allowed_detuning;
  result.constructive = result.eligible >= 8 && result.near_resonant
      && result.contrast >= 5.0;
  return result;
}

inline void write_csv(const std::filesystem::path& directory) {
  std::ofstream output(directory /
      "ftd_0698_internal_excitation_symmetry_ray_spectrum_ticks_v1.csv");
  output << "ftd_id,tick,sign,ray,harmonic,omega,field_total,"
            "field_transverse,field_longitudinal,current_total,"
            "current_transverse,current_longitudinal,projection_residual,"
            "etx_re,etx_im,ety_re,ety_im,etz_re,etz_im,"
            "btx_re,btx_im,bty_re,bty_im,btz_re,btz_im,"
            "kx_re,kx_im,ky_re,ky_im,kz_re,kz_im\n";
  output << std::setprecision(17);
  for (const auto& row : recorder.rows) {
    output << "FTD-0698," << row.tick << ',' << row.sign << ',' << row.ray
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
inline void finalize(bool parent_exact,
                     double phi,
                     const Arms&) {
  recorder.finalized = true;
  const std::size_t expected_rows = static_cast<std::size_t>(
      expected_horizon + 1) * 2 * 3 * harmonic_count;
  recorder.valid = recorder.valid && recorder.rows.size() == expected_rows
      && recorder.maximum_projection_residual <= 1e-12;
  for (int ray = 0; ray < 3; ++ray) {
    recorder.summaries.push_back(summarize(phi, -1, ray));
    recorder.summaries.push_back(summarize(phi, 1, ray));
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
    for (int harmonic = 1; harmonic <= harmonic_count; ++harmonic) {
      double negative_field = 0.0;
      double positive_field = 0.0;
      double negative_current = 0.0;
      double positive_current = 0.0;
      for (int tick = 1; tick <= expected_horizon; ++tick) {
        const auto* negative = find_row(tick, -1, ray, harmonic);
        const auto* positive = find_row(tick, 1, ray, harmonic);
        if (negative == nullptr || positive == nullptr) {
          recorder.valid = false;
          continue;
        }
        negative_field += negative->field_transverse;
        positive_field += positive->field_transverse;
        negative_current += negative->current_total;
        positive_current += positive->current_total;
      }
      recorder.sign_field_residual = std::max(recorder.sign_field_residual,
          relative(negative_field, positive_field));
      recorder.sign_current_residual = std::max(recorder.sign_current_residual,
          relative(negative_current, positive_current));
    }
  }
  recorder.execution = parent_exact && recorder.valid && eligible
      && recorder.sign_field_residual <= 1e-4
      && recorder.sign_current_residual <= 1e-4;
  if (recorder.execution)
    recorder.verdict = constructive && peak_agreement
        ? "SYMMETRY_RAY_RESONANT_TRANSFER_CONSTRUCTIVE"
        : "SYMMETRY_RAY_SPECTRAL_TRANSFER_MIXED";

  const auto directory = std::filesystem::path(__FILE__).parent_path()
      .parent_path() / "results/ftd_0698";
  std::filesystem::create_directories(directory);
  write_csv(directory);
  std::ofstream json(directory /
      "ftd_0698_internal_excitation_symmetry_ray_spectrum_v1.json");
  json << std::setprecision(17)
       << "{\n  \"ftd_id\": \"FTD-0698\",\n"
       << "  \"protocol_sha256\": \"" << protocol_sha256 << "\",\n"
       << "  \"verdict\": \"" << recorder.verdict << "\",\n"
       << "  \"production_changed\": false,\n"
       << "  \"execution_pass\": " << (recorder.execution ? 1 : 0) << ",\n"
       << "  \"row_count\": " << recorder.rows.size() << ",\n"
       << "  \"maximum_projection_residual\": "
       << recorder.maximum_projection_residual << ",\n"
       << "  \"sign_field_residual\": " << recorder.sign_field_residual
       << ",\n  \"sign_current_residual\": "
       << recorder.sign_current_residual << ",\n"
       << "  \"internal_phase\": " << phi << ",\n"
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
            << " projection=" << recorder.maximum_projection_residual
            << " sign_field=" << recorder.sign_field_residual
            << " sign_current=" << recorder.sign_current_residual << '\n';
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
  return recorder.finalized && recorder.execution;
}

}  // namespace ftd0698
