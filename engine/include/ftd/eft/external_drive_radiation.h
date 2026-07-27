#pragma once
/**
 * @file external_drive_radiation.h
 * @brief Exact modal work and external-drive radiation observer (FTD-0559).
 */

#include <array>
#include <complex>
#include <vector>

namespace ftd::eft {

using ComplexModalState = std::array<std::complex<double>, 2>;

struct ExternalDriveWorkDiagnostics {
  bool valid = false;
  std::array<double, 3> momentum{};
  ComplexModalState initial{};
  std::complex<double> drive{};
  double kick = 0.0;
  double energy_change = 0.0;
  double source_work = 0.0;
  double residual = 0.0;
};

struct HarmonicDriveDiagnostics {
  bool valid = false;
  bool resonant = false;
  std::array<double, 3> momentum{};
  int ticks = 0;
  double kick = 0.0;
  double phase = 0.0;
  double omega = 0.0;
  ComplexModalState direct_state{};
  ComplexModalState closed_state{};
  double final_energy = 0.0;
  double cumulative_work = 0.0;
  double response_residual = 0.0;
  double work_residual = 0.0;
  double normalized_resonant_energy = 0.0;
  double resonant_error_bound = 0.0;
  double maximum_off_resonant_energy = 0.0;
  double off_resonant_energy_bound = 0.0;
};

struct FejerNormalizationDiagnostics {
  bool valid = false;
  int ticks = 0;
  int quadrature_points = 0;
  double normalized_integral = 0.0;
  double residual = 0.0;
};

struct ExternalDriveRadiationResult {
  bool valid = false;
  bool exact_source_work_identity = false;
  bool exact_retarded_response = false;
  bool finite_volume_resonance_dichotomy = false;
  bool fejer_radiation_limit = false;
  bool group_velocity_mismatch_jacobian = false;
  bool integer_hop_power_is_floquet_weighted = false;
  double c2 = 0.0;
  double maximum_work_identity_residual = 0.0;
  double maximum_response_residual = 0.0;
  double maximum_cumulative_work_residual = 0.0;
  double maximum_fejer_residual = 0.0;
  std::vector<ExternalDriveWorkDiagnostics> work_arms;
  std::vector<HarmonicDriveDiagnostics> harmonic_arms;
  std::vector<FejerNormalizationDiagnostics> fejer_arms;
};

double production_modal_energy(const ComplexModalState& state,
                               double kick);

ComplexModalState forced_production_modal_step(
    const ComplexModalState& state,
    double kick,
    const std::complex<double>& drive);

double exact_external_drive_work(
    const ComplexModalState& unforced_endpoint,
    const ComplexModalState& forced_endpoint,
    double kick,
    const std::complex<double>& drive);

ComplexModalState closed_harmonic_response(
    double phase,
    double omega,
    int ticks,
    const std::complex<double>& amplitude);

ExternalDriveRadiationResult analyze_external_drive_radiation(double c2);

}  // namespace ftd::eft
