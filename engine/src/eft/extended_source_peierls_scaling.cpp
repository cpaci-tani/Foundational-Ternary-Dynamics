#include "ftd/eft/extended_source_peierls_scaling.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <numeric>

namespace ftd::eft {
namespace {

constexpr long double pi =
    3.141592653589793238462643383279502884L;

long double coord_component(const Coord& value, int axis) {
  return axis == 0 ? value.x : (axis == 1 ? value.y : value.z);
}

long double norm_squared(const Coord& value) {
  return static_cast<long double>(value.x)*value.x
      +static_cast<long double>(value.y)*value.y
      +static_cast<long double>(value.z)*value.z;
}

BinomialEnvelopeDiagnostics diagnose_binomial(int L, int order) {
  BinomialEnvelopeDiagnostics result;
  result.order = order;
  result.support = order+1;
  result.local_generation_steps_3d = 3*order;
  if (L < 3 || order < 1) return result;

  std::vector<long double> weights(order+1, 0.0L);
  weights[0] = std::ldexp(1.0L, -order);
  for (int n = 0; n < order; ++n)
    weights[n+1] = weights[n]
        *static_cast<long double>(order-n)/(n+1);

  long double partition = 0.0L;
  long double first = 0.0L;
  for (int n = 0; n <= order; ++n) {
    partition += weights[n];
    first += n*weights[n];
  }
  long double variance = 0.0L;
  for (int n = 0; n <= order; ++n) {
    const long double delta = n-first;
    variance += delta*delta*weights[n];
  }
  result.partition_residual = std::abs(static_cast<double>(partition-1.0L));
  result.mean_residual = std::abs(static_cast<double>(
      first-static_cast<long double>(order)/2.0L));
  result.variance_residual = std::abs(static_cast<double>(
      variance-static_cast<long double>(order)/4.0L));

  for (int mode = 0; mode < L; ++mode) {
    const long double k = 2.0L*pi*mode/L;
    std::complex<long double> direct{0.0L, 0.0L};
    for (int n = 0; n <= order; ++n) {
      const long double phase = -k*n;
      direct += weights[n]*std::complex<long double>{
          std::cos(phase), std::sin(phase)};
    }
    const std::complex<long double> step =
        0.5L*(std::complex<long double>{1.0L, 0.0L}
              +std::complex<long double>{std::cos(-k), std::sin(-k)});
    const auto expected = std::pow(step, order);
    result.fourier_residual = std::max(
        result.fourier_residual,
        static_cast<double>(std::abs(direct-expected)));
  }
  result.maximum_identity_residual = std::max({
      result.partition_residual, result.mean_residual,
      result.variance_residual, result.fourier_residual});
  result.valid = std::isfinite(result.maximum_identity_residual);
  return result;
}

void set_asymptotic_constants(ExtendedPeierlsSample& sample,
                              int order, int axis, double beta) {
  const long double root_pi_cubed = std::pow(pi, 1.5L);
  const long double m = order;
  if (sample.profile.kind
      == ExtendedPeierlsProfileKind::MonopoleBackground) {
    sample.scaled_energy_constant = static_cast<double>(
        sample.energy_zero*std::sqrt(m)/beta);
    sample.scaled_barrier_constant = static_cast<double>(
        sample.half_cell_barrier*std::pow(m, 2.5L)/beta);
    sample.scaled_relative_constant = static_cast<double>(
        sample.relative_barrier*m*m);
    sample.expected_energy_constant = static_cast<double>(
        1.0L/(4.0L*root_pi_cubed));
    sample.expected_barrier_constant = static_cast<double>(
        3.0L/(320.0L*root_pi_cubed));
    sample.expected_relative_constant = 3.0/80.0;
    return;
  }

  const long double d2 = norm_squared(sample.profile.displacement);
  const long double di = coord_component(sample.profile.displacement, axis);
  const long double directional = d2+4.0L*di*di;
  sample.scaled_energy_constant = static_cast<double>(
      sample.energy_zero*std::pow(m, 1.5L)/beta);
  sample.scaled_barrier_constant = static_cast<double>(
      sample.half_cell_barrier*std::pow(m, 3.5L)/beta);
  sample.scaled_relative_constant = static_cast<double>(
      sample.relative_barrier*m*m);
  sample.expected_energy_constant = static_cast<double>(
      d2/(6.0L*root_pi_cubed));
  sample.expected_barrier_constant = static_cast<double>(
      3.0L*directional/(224.0L*root_pi_cubed));
  sample.expected_relative_constant = static_cast<double>(
      9.0L*directional/(112.0L*d2));
}

}  // namespace

ExtendedSourcePeierlsResult evaluate_extended_source_peierls(
    int L,
    int order,
    int translation_axis,
    const std::vector<ExtendedPeierlsProfile>& profiles,
    double beta) {
  ExtendedSourcePeierlsResult result;
  result.L = L;
  result.order = order;
  result.axis = translation_axis;
  result.beta = beta;
  result.support_does_not_wrap = order+1 < L;
  if (L < 3 || L%2 == 0 || order < 1 || translation_axis < 0
      || translation_axis > 2 || profiles.empty()
      || !std::isfinite(beta) || !(beta > 0.0)) return result;
  for (const auto& profile : profiles) {
    if (profile.kind == ExtendedPeierlsProfileKind::Dipole
        && norm_squared(profile.displacement) == 0.0L) return result;
  }

  result.envelope = diagnose_binomial(L, order);
  if (!result.envelope.valid) return result;

  struct Accumulator {
    long double energy = 0.0L;
    long double coefficient = 0.0L;
    long double average_numerator = 0.0L;
  };
  std::vector<Accumulator> sums(profiles.size());

  std::vector<long double> cosine(L);
  std::vector<long double> centered(L);
  std::vector<long double> envelope_squared(L);
  std::vector<long double> momentum(L);
  for (int mode = 0; mode < L; ++mode) {
    momentum[mode] = 2.0L*pi*mode/L;
    cosine[mode] = std::cos(momentum[mode]);
    centered[mode] = (3.0L+cosine[mode])/4.0L;
    envelope_squared[mode] = std::pow(
        std::abs(std::cos(momentum[mode]/2.0L)), 2*order);
  }

  for (int mx = 0; mx < L; ++mx)
    for (int my = 0; my < L; ++my)
      for (int mz = 0; mz < L; ++mz) {
        if (mx == 0 && my == 0 && mz == 0) continue;
        const std::array<int, 3> mode{{mx, my, mz}};
        const long double lambda = 2.0L*(3.0L-cosine[mx]
            -cosine[my]-cosine[mz]);
        const long double envelope = envelope_squared[mx]
            *envelope_squared[my]*envelope_squared[mz];
        const long double centered_product =
            centered[mx]*centered[mx]
            *centered[my]*centered[my]
            *centered[mz]*centered[mz];
        long double transverse = 1.0L;
        for (int direction = 0; direction < 3; ++direction)
          if (direction != translation_axis)
            transverse *= centered[mode[direction]]
                *centered[mode[direction]];
        const long double one_minus =
            1.0L-cosine[mode[translation_axis]];
        const long double ratio = one_minus*one_minus
            /((3.0L+cosine[mode[translation_axis]])
              *(3.0L+cosine[mode[translation_axis]]));

        for (std::size_t p = 0; p < profiles.size(); ++p) {
          long double source_factor = 1.0L;
          if (profiles[p].kind == ExtendedPeierlsProfileKind::Dipole) {
            const auto& d = profiles[p].displacement;
            const long double phase = momentum[mx]*d.x
                +momentum[my]*d.y+momentum[mz]*d.z;
            source_factor = 2.0L*(1.0L-std::cos(phase));
          }
          const long double energy_term = source_factor*envelope
              *centered_product/lambda;
          const long double coefficient_term = source_factor*envelope
              *one_minus*one_minus*transverse/lambda;
          sums[p].energy += energy_term;
          sums[p].coefficient += coefficient_term;
          sums[p].average_numerator += energy_term*ratio;
        }
      }

  const long double volume = static_cast<long double>(L)*L*L;
  const long double scale = static_cast<long double>(beta)/(2.0L*volume);
  for (std::size_t p = 0; p < profiles.size(); ++p) {
    ExtendedPeierlsSample sample;
    sample.profile = profiles[p];
    sample.energy_zero = static_cast<double>(scale*sums[p].energy);
    sample.peierls_coefficient = static_cast<double>(
        scale*sums[p].coefficient);
    sample.half_cell_barrier = sample.peierls_coefficient/16.0;
    sample.relative_barrier = sample.half_cell_barrier/sample.energy_zero;
    sample.spectral_average = static_cast<double>(
        sums[p].average_numerator/sums[p].energy);
    sample.spectral_identity_residual = std::abs(
        sample.relative_barrier-sample.spectral_average);
    set_asymptotic_constants(sample, order, translation_axis, beta);
    sample.valid = std::isfinite(sample.energy_zero)
        && std::isfinite(sample.peierls_coefficient)
        && std::isfinite(sample.relative_barrier)
        && sample.energy_zero > 0.0
        && sample.peierls_coefficient > 0.0
        && sample.relative_barrier > 0.0
        && sample.relative_barrier <= 1.0;
    result.maximum_identity_residual = std::max(
        result.maximum_identity_residual,
        sample.spectral_identity_residual);
    result.samples.push_back(sample);
  }
  result.maximum_identity_residual = std::max(
      result.maximum_identity_residual,
      result.envelope.maximum_identity_residual);
  result.valid = result.support_does_not_wrap
      && result.envelope.valid
      && std::all_of(result.samples.begin(), result.samples.end(),
          [](const ExtendedPeierlsSample& sample) { return sample.valid; });
  return result;
}

}  // namespace ftd::eft
