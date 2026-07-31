#pragma once

/**
 * @file matched_symmetry_ray_spectrum.h
 * @brief Carrier-aware Fourier observer for the matched face/edge field.
 *
 * FTD-0696 observer only.  The reported quadratic power is Fourier
 * morphology, not the exact staggered modified energy.
 */

#include "ftd/eft/matched_gauss_transport.h"

#include <array>
#include <complex>
#include <vector>

namespace ftd::eft {

using MatchedComplexVector = std::array<std::complex<double>, 3>;

struct MatchedWavevectorSpectrum {
  bool valid = false;
  int L = 0;
  std::array<int, 3> mode{};
  std::array<double, 3> wavevector{};
  std::array<double, 3> lattice_wavevector{};
  MatchedComplexVector electric{};
  MatchedComplexVector magnetic{};
  MatchedComplexVector electric_transverse{};
  MatchedComplexVector electric_longitudinal{};
  MatchedComplexVector magnetic_transverse{};
  MatchedComplexVector magnetic_longitudinal{};
  double electric_power = 0.0;
  double magnetic_power = 0.0;
  double transverse_power = 0.0;
  double longitudinal_power = 0.0;
  double total_power = 0.0;
  double electric_projection_residual = 0.0;
  double magnetic_projection_residual = 0.0;
};

/**
 * Fourier-analyse candidate-reference at one nonzero periodic wavevector.
 * Component phases use the physical face/edge carrier coordinates.
 */
MatchedWavevectorSpectrum observe_matched_wavevector_spectrum(
    const MatchedFaceFlux& reference_electric,
    const MatchedEdgeField& reference_magnetic,
    const MatchedFaceFlux& candidate_electric,
    const MatchedEdgeField& candidate_magnetic,
    const std::array<int, 3>& mode,
    double wave_speed);

struct MatchedSymmetryRayRequest {
  std::array<int, 3> direction{};
  std::vector<int> harmonics;
};

struct MatchedSymmetryRayBatch {
  bool valid = false;
  int L = 0;
  std::vector<MatchedWavevectorSpectrum> spectra;
};

/**
 * Algebraically equivalent modular-bin evaluation for several harmonics on
 * each requested integer ray. Output is ray-major, then harmonic-major.
 */
MatchedSymmetryRayBatch observe_batched_matched_symmetry_ray_spectra(
    const MatchedFaceFlux& reference_electric,
    const MatchedEdgeField& reference_magnetic,
    const MatchedFaceFlux& candidate_electric,
    const MatchedEdgeField& candidate_magnetic,
    const std::vector<MatchedSymmetryRayRequest>& requests,
    double wave_speed);

}  // namespace ftd::eft
