#pragma once
/**
 * @file native_moving_source_pole.h
 * @brief Observer-only correction of the native moving-source pole
 *        (FTD-0558).
 */

#include "ftd/lattice.h"

#include <array>
#include <complex>
#include <vector>

namespace ftd::eft {

struct DrivenPoleDiagnostics {
  bool valid = false;
  std::array<double, 3> momentum{};
  double omega = 0.0;
  double symbol = 0.0;
  double kick = 0.0;
  double phase = 0.0;
  double denominator = 0.0;
  double direct_flux_response = 0.0;
  double closed_flux_response = 0.0;
  double direct_solve_residual = 0.0;
  double recurrence_residual = 0.0;
  double static_resolvent_residual = 0.0;
};

struct WrappedThresholdDiagnostics {
  bool valid = false;
  int L = 0;
  Coord direction{};
  std::array<int, 3> minimizing_mode{};
  double minimum_phase_speed = 0.0;
  double universal_lower_bound = 0.0;
  double lower_bound_residual = 0.0;
};

struct FloquetHopDiagnostics {
  bool valid = false;
  int period = 0;
  Coord displacement{};
  std::array<double, 3> momentum{};
  std::vector<std::complex<double>> coefficients;
  double mean_frequency = 0.0;
  double maximum_reconstruction_residual = 0.0;
  double parseval_residual = 0.0;
  double maximum_nonfundamental_amplitude = 0.0;
};

struct NativeMovingSourcePoleResult {
  bool valid = false;
  bool production_discrete_time_pole_derived = false;
  bool seven_point_any_speed_claim_refuted = false;
  bool full_stencil_positive_speed_floor = false;
  bool wrapped_alias_counterexample = false;
  bool integer_hop_requires_floquet_spectrum = false;
  double c2 = 0.0;
  double universal_speed_floor = 0.0;
  double seven_point_ratio_floor = 0.0;
  double alias_symbol_residual = 0.0;
  double alias_phase_residual = 0.0;
  double old_to_wrapped_alias_ratio = 0.0;
  double maximum_identity_residual = 0.0;
  std::vector<DrivenPoleDiagnostics> driven_modes;
  std::vector<WrappedThresholdDiagnostics> thresholds;
  std::vector<FloquetHopDiagnostics> floquet_schedules;
};

int canonical_crystal_mode(int array_mode, int L);

double production_driven_denominator(double symbol,
                                     double c2,
                                     double omega);

std::vector<std::complex<double>> integer_hop_floquet_coefficients(
    double momentum_dot_displacement,
    int period);

NativeMovingSourcePoleResult analyze_native_moving_source_pole(
    double c2);

}  // namespace ftd::eft
