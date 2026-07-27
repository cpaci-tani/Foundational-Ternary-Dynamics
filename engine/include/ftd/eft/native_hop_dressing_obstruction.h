#pragma once
/**
 * @file native_hop_dressing_obstruction.h
 * @brief Observer-only periodic point-hop dressing obstruction (FTD-0560).
 */

#include <array>
#include <complex>
#include <vector>

namespace ftd::eft {

struct NativeHopDressingArm {
  bool valid = false;
  int period = 0;
  int axis = 0;
  int polarity = 0;
  int harmonic = 0;
  std::array<double, 3> momentum{};
  std::array<double, 3> wave_symbol{};
  std::array<double, 3> velocity{};
  std::array<std::complex<double>, 3> source{};
  double root_parameter = 0.0;
  double bracket_lower = 0.0;
  double bracket_upper = 0.0;
  double symbol = 0.0;
  double phase = 0.0;
  double omega = 0.0;
  double denominator_residual = 0.0;
  double regularity_derivative = 0.0;
  double floquet_coefficient_norm = 0.0;
  double coefficient_identity_residual = 0.0;
  double source_norm = 0.0;
  double orthogonal_source_norm = 0.0;
  double source_orthogonality_residual = 0.0;
  double normalized_effective_forcing = 0.0;
  double normalized_resonant_energy = 0.0;
  double resonant_error_bound = 0.0;
};

struct NativeHopDressingObstructionResult {
  bool valid = false;
  bool native_source_components_are_orthogonal = false;
  bool every_finite_registered_period_has_resonance = false;
  bool resonant_native_source_is_nonzero = false;
  bool axial_floquet_coefficient_identity = false;
  bool point_hop_dressing_not_square_summable = false;
  bool slow_hop_forcing_is_asymptotically_quadratic = false;
  double c2 = 0.0;
  double coupling = 0.0;
  double maximum_root_residual = 0.0;
  double minimum_regularity_derivative = 0.0;
  double maximum_source_orthogonality_residual = 0.0;
  double maximum_coefficient_identity_residual = 0.0;
  double minimum_normalized_effective_forcing = 0.0;
  double maximum_polarity_mirror_residual = 0.0;
  double maximum_cubic_covariance_residual = 0.0;
  double maximum_resonant_coefficient_excess = 0.0;
  std::vector<NativeHopDressingArm> arms;
};

NativeHopDressingObstructionResult analyze_native_hop_dressing_obstruction(
    double c2,
    double coupling);

}  // namespace ftd::eft
