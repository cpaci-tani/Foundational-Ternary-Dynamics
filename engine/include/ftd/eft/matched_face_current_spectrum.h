#pragma once

/**
 * @file matched_face_current_spectrum.h
 * @brief Carrier-aware Fourier observer for oriented face current (FTD-0702).
 */

#include "ftd/eft/quadratic_coat_face_current.h"
#include "ftd/eft/matched_gauss_transport.h"

#include <array>
#include <complex>
#include <vector>

namespace ftd::eft {

using FaceCurrentComplexVector = std::array<std::complex<double>, 3>;

struct MatchedFaceCurrentSpectrum {
  bool valid = false;
  int L = 0;
  std::array<double, 3> wavevector{};
  std::array<double, 3> lattice_wavevector{};
  FaceCurrentComplexVector current{};
  FaceCurrentComplexVector transverse{};
  FaceCurrentComplexVector longitudinal{};
  double normalization = 0.0;
  double input_l1 = 0.0;
  double total_power = 0.0;
  double transverse_power = 0.0;
  double longitudinal_power = 0.0;
  double transverse_fraction = 0.0;
  double projection_residual = 0.0;
  double power_partition_residual = 0.0;
};

MatchedFaceCurrentSpectrum observe_sparse_face_current_spectrum(
    int L,
    const std::vector<QuadraticCoatSparseCurrentEntry>& entries,
    const std::array<double, 3>& wavevector,
    double normalization);

MatchedFaceCurrentSpectrum observe_dense_face_current_spectrum(
    const MatchedFaceFlux& current,
    const std::array<double, 3>& wavevector,
    double normalization);

}  // namespace ftd::eft
