#pragma once
/**
 * @file continuous_translation_locality.h
 * @brief Observer for the exact-translation/locality trilemma (FTD-0554).
 */

#include <cstddef>
#include <utility>
#include <vector>

namespace ftd::eft {

struct BandlimitedShiftSample {
  double fraction = 0.0;
  std::vector<double> weights;
  int support = 0;
  double minimum_weight = 0.0;
  double imaginary_residual = 0.0;
  double partition_residual = 0.0;
  double norm_residual = 0.0;
  double fourier_phase_residual = 0.0;
};

struct BandlimitedContinuitySample {
  double fraction_before = 0.0;
  double fraction_after = 0.0;
  int separation = 0;
  int density_change_support = 0;
  int current_support = 0;
  double current_imaginary_residual = 0.0;
  double continuity_residual = 0.0;
};

struct ContinuousTranslationLocalityResult {
  bool valid = false;
  bool finite_laurent_unitary_is_monomial = false;
  bool continuous_finite_range_shift_group_impossible = false;
  int L = 0;
  int minimum_noninteger_support = 0;
  int minimum_density_change_support = 0;
  int minimum_current_support = 0;
  double most_negative_weight = 0.0;
  double cardinal_residual = 0.0;
  double maximum_group_residual = 0.0;
  double maximum_energy_residual = 0.0;
  double maximum_identity_residual = 0.0;
  std::vector<BandlimitedShiftSample> shift_samples;
  std::vector<BandlimitedContinuitySample> continuity_samples;
};

ContinuousTranslationLocalityResult analyze_continuous_translation_locality(
    int L,
    const std::vector<double>& fractions,
    const std::vector<std::pair<double, double>>& composition_pairs,
    const std::vector<int>& neutral_separations,
    const std::vector<std::pair<double, double>>& continuity_moves,
    double beta,
    double support_tolerance = 1e-13);

}  // namespace ftd::eft
