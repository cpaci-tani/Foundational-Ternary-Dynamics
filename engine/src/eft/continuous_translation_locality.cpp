#include "ftd/eft/continuous_translation_locality.h"

#include <algorithm>
#include <cmath>
#include <complex>
#include <limits>
#include <numeric>

namespace ftd::eft {
namespace {

constexpr double pi = 3.1415926535897932384626433832795;

int wrap(int value, int L) {
  const int remainder = value%L;
  return remainder < 0 ? remainder+L : remainder;
}

std::vector<int> modes(int L) {
  const int half = (L-1)/2;
  std::vector<int> result;
  result.reserve(L);
  for (int mode = -half; mode <= half; ++mode) result.push_back(mode);
  return result;
}

std::vector<std::complex<long double>> forward(
    const std::vector<double>& values) {
  const int L = static_cast<int>(values.size());
  const auto registered_modes = modes(L);
  std::vector<std::complex<long double>> result(L);
  for (int slot = 0; slot < L; ++slot) {
    const long double k = 2.0L*pi*registered_modes[slot]/L;
    for (int n = 0; n < L; ++n) {
      const long double phase = -k*n;
      result[slot] += static_cast<long double>(values[n])
          *std::complex<long double>{std::cos(phase), std::sin(phase)};
    }
  }
  return result;
}

struct InverseResult {
  std::vector<double> real;
  double imaginary_residual = 0.0;
};

InverseResult inverse(
    const std::vector<std::complex<long double>>& spectrum) {
  const int L = static_cast<int>(spectrum.size());
  const auto registered_modes = modes(L);
  InverseResult result;
  result.real.assign(L, 0.0);
  for (int n = 0; n < L; ++n) {
    std::complex<long double> value{0.0L, 0.0L};
    for (int slot = 0; slot < L; ++slot) {
      const long double k = 2.0L*pi*registered_modes[slot]/L;
      const long double phase = k*n;
      value += spectrum[slot]
          *std::complex<long double>{std::cos(phase), std::sin(phase)};
    }
    value /= static_cast<long double>(L);
    result.real[n] = static_cast<double>(value.real());
    result.imaginary_residual = std::max(
        result.imaginary_residual, std::abs(static_cast<double>(value.imag())));
  }
  return result;
}

InverseResult bandlimited_kernel(int L, double fraction) {
  const auto registered_modes = modes(L);
  std::vector<std::complex<long double>> spectrum(L);
  for (int slot = 0; slot < L; ++slot) {
    const long double k = 2.0L*pi*registered_modes[slot]/L;
    const long double phase = -k*fraction;
    spectrum[slot] = {std::cos(phase), std::sin(phase)};
  }
  return inverse(spectrum);
}

double max_difference(const std::vector<double>& lhs,
                      const std::vector<double>& rhs) {
  double result = 0.0;
  for (std::size_t i = 0; i < lhs.size(); ++i)
    result = std::max(result, std::abs(lhs[i]-rhs[i]));
  return result;
}

int support(const std::vector<double>& values, double tolerance) {
  return static_cast<int>(std::count_if(values.begin(), values.end(),
      [tolerance](double value) { return std::abs(value) > tolerance; }));
}

std::vector<double> neutral_density(int L, double fraction, int separation) {
  const auto positive = bandlimited_kernel(L, fraction).real;
  const auto negative = bandlimited_kernel(L, fraction+separation).real;
  std::vector<double> result(L);
  for (int n = 0; n < L; ++n) result[n] = positive[n]-negative[n];
  return result;
}

double neutral_spectral_energy(const std::vector<double>& density,
                               double beta) {
  const int L = static_cast<int>(density.size());
  const auto registered_modes = modes(L);
  const auto spectrum = forward(density);
  long double energy = 0.0L;
  for (int slot = 0; slot < L; ++slot) {
    const long double k = 2.0L*pi*registered_modes[slot]/L;
    const long double lambda = 2.0L*(1.0L-std::cos(k));
    if (lambda <= 0.0L) continue;
    energy += std::norm(spectrum[slot])/lambda;
  }
  return static_cast<double>(static_cast<long double>(beta)
      *energy/(2.0L*L));
}

}  // namespace

ContinuousTranslationLocalityResult analyze_continuous_translation_locality(
    int L,
    const std::vector<double>& fractions,
    const std::vector<std::pair<double, double>>& composition_pairs,
    const std::vector<int>& neutral_separations,
    const std::vector<std::pair<double, double>>& continuity_moves,
    double beta,
    double support_tolerance) {
  ContinuousTranslationLocalityResult result;
  result.L = L;
  result.minimum_noninteger_support = L;
  result.minimum_density_change_support = L;
  result.minimum_current_support = L;
  if (L < 3 || L%2 == 0 || fractions.empty()
      || neutral_separations.empty() || continuity_moves.empty()
      || !std::isfinite(beta) || !(beta > 0.0)
      || !(support_tolerance > 0.0)) return result;

  // If p(z)=sum_{a..b} c_n z^n has constant modulus on the unit circle,
  // its extreme nonzero autocorrelation coefficient is c_b*conj(c_a)=0.
  // Hence a=b and p is a phase times a monomial. A continuous family of such
  // finite-range unitary symbols cannot change its integer monomial degree.
  result.finite_laurent_unitary_is_monomial = true;
  result.continuous_finite_range_shift_group_impossible = true;

  const auto zero = bandlimited_kernel(L, 0.0);
  const auto one = bandlimited_kernel(L, 1.0);
  std::vector<double> delta_zero(L, 0.0);
  std::vector<double> delta_one(L, 0.0);
  delta_zero[0] = 1.0;
  delta_one[1] = 1.0;
  result.cardinal_residual = std::max(
      max_difference(zero.real, delta_zero),
      max_difference(one.real, delta_one));
  result.maximum_identity_residual = std::max({
      result.maximum_identity_residual, zero.imaginary_residual,
      one.imaginary_residual, result.cardinal_residual});

  const auto registered_modes = modes(L);
  for (double fraction : fractions) {
    if (!std::isfinite(fraction)) return result;
    const auto kernel = bandlimited_kernel(L, fraction);
    BandlimitedShiftSample sample;
    sample.fraction = fraction;
    sample.weights = kernel.real;
    sample.imaginary_residual = kernel.imaginary_residual;
    sample.support = support(sample.weights, support_tolerance);
    sample.minimum_weight = *std::min_element(
        sample.weights.begin(), sample.weights.end());
    sample.partition_residual = std::abs(static_cast<double>(
        std::accumulate(sample.weights.begin(), sample.weights.end(), 0.0L)
        -1.0L));
    long double norm = 0.0L;
    for (double weight : sample.weights)
      norm += static_cast<long double>(weight)*weight;
    sample.norm_residual = std::abs(static_cast<double>(norm-1.0L));
    const auto spectrum = forward(sample.weights);
    for (int slot = 0; slot < L; ++slot) {
      const long double k = 2.0L*pi*registered_modes[slot]/L;
      const long double phase = -k*fraction;
      const std::complex<long double> expected{
          std::cos(phase), std::sin(phase)};
      sample.fourier_phase_residual = std::max(
          sample.fourier_phase_residual,
          static_cast<double>(std::abs(spectrum[slot]-expected)));
    }
    if (std::abs(fraction-std::round(fraction)) > 1e-12) {
      result.minimum_noninteger_support = std::min(
          result.minimum_noninteger_support, sample.support);
      result.most_negative_weight = std::min(
          result.most_negative_weight, sample.minimum_weight);
    }
    result.maximum_identity_residual = std::max({
        result.maximum_identity_residual, sample.imaginary_residual,
        sample.partition_residual, sample.norm_residual,
        sample.fourier_phase_residual});
    result.shift_samples.push_back(std::move(sample));
  }

  for (const auto& pair : composition_pairs) {
    const auto start = bandlimited_kernel(L, pair.second);
    auto spectrum = forward(start.real);
    for (int slot = 0; slot < L; ++slot) {
      const long double k = 2.0L*pi*registered_modes[slot]/L;
      const long double phase = -k*pair.first;
      spectrum[slot] *= std::complex<long double>{
          std::cos(phase), std::sin(phase)};
    }
    const auto composed = inverse(spectrum);
    const auto expected = bandlimited_kernel(L, pair.first+pair.second);
    result.maximum_group_residual = std::max({
        result.maximum_group_residual,
        max_difference(composed.real, expected.real),
        composed.imaginary_residual, expected.imaginary_residual});
  }
  result.maximum_identity_residual = std::max(
      result.maximum_identity_residual, result.maximum_group_residual);

  for (int separation : neutral_separations) {
    double reference_energy = NAN;
    for (double fraction : fractions) {
      const double energy = neutral_spectral_energy(
          neutral_density(L, fraction, separation), beta);
      if (!std::isfinite(reference_energy)) reference_energy = energy;
      result.maximum_energy_residual = std::max(
          result.maximum_energy_residual,
          std::abs(energy-reference_energy));
    }
  }
  result.maximum_identity_residual = std::max(
      result.maximum_identity_residual, result.maximum_energy_residual);

  for (int separation : neutral_separations) {
    for (const auto& move : continuity_moves) {
      BandlimitedContinuitySample sample;
      sample.fraction_before = move.first;
      sample.fraction_after = move.second;
      sample.separation = separation;
      const auto before = neutral_density(L, move.first, separation);
      const auto after = neutral_density(L, move.second, separation);
      std::vector<double> change(L);
      for (int n = 0; n < L; ++n) change[n] = after[n]-before[n];
      sample.density_change_support = support(change, support_tolerance);
      auto change_spectrum = forward(change);
      std::vector<std::complex<long double>> current_spectrum(L);
      for (int slot = 0; slot < L; ++slot) {
        const long double k = 2.0L*pi*registered_modes[slot]/L;
        const std::complex<long double> divergence{
            1.0L-std::cos(k), std::sin(k)};
        if (std::abs(divergence) > 1e-18L)
          current_spectrum[slot] = -change_spectrum[slot]/divergence;
      }
      const auto current = inverse(current_spectrum);
      sample.current_imaginary_residual = current.imaginary_residual;
      sample.current_support = support(current.real, support_tolerance);
      for (int n = 0; n < L; ++n) {
        const double divergence = current.real[n]
            -current.real[wrap(n-1, L)];
        sample.continuity_residual = std::max(
            sample.continuity_residual,
            std::abs(change[n]+divergence));
      }
      result.minimum_density_change_support = std::min(
          result.minimum_density_change_support,
          sample.density_change_support);
      result.minimum_current_support = std::min(
          result.minimum_current_support, sample.current_support);
      result.maximum_identity_residual = std::max({
          result.maximum_identity_residual,
          sample.current_imaginary_residual,
          sample.continuity_residual});
      result.continuity_samples.push_back(sample);
    }
  }

  result.valid = result.finite_laurent_unitary_is_monomial
      && result.continuous_finite_range_shift_group_impossible
      && result.shift_samples.size() == fractions.size()
      && result.continuity_samples.size()
          == neutral_separations.size()*continuity_moves.size()
      && std::isfinite(result.maximum_identity_residual);
  return result;
}

}  // namespace ftd::eft
