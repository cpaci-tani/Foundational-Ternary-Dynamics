#include "ftd/eft/ternary_block_bipole_peierls.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <limits>
#include <vector>

namespace ftd::eft {
namespace {

constexpr long double pi =
    3.141592653589793238462643383279502884L;

struct CompensatedSum {
  long double value = 0.0L;
  long double correction = 0.0L;

  void add(long double term) {
    const long double adjusted = term-correction;
    const long double next = value+adjusted;
    correction = (next-value)-adjusted;
    value = next;
  }
};

struct CompensatedComplexSum {
  CompensatedSum real;
  CompensatedSum imaginary;

  void add(const std::complex<long double>& term) {
    real.add(term.real());
    imaginary.add(term.imag());
  }

  std::complex<long double> value() const {
    return {real.value, imaginary.value};
  }
};

std::complex<long double> unit_phase(long double phase) {
  return {std::cos(phase), std::sin(phase)};
}

std::complex<long double> geometric_sum(int L, int mode, int width) {
  const long double k = 2.0L*pi*mode/L;
  CompensatedComplexSum result;
  for (int n = 0; n < width; ++n) result.add(unit_phase(-k*n));
  return result.value();
}

std::complex<long double> dirichlet_sum(int L, int mode, int width) {
  if (mode == 0) return {static_cast<long double>(width), 0.0L};
  const long double k = 2.0L*pi*mode/L;
  const long double amplitude = std::sin(0.5L*width*k)
      /std::sin(0.5L*k);
  return amplitude*unit_phase(-0.5L*(width-1)*k);
}

std::complex<long double> analytic_structure(
    int L, int width, int orientation,
    const std::array<int, 3>& mode) {
  std::complex<long double> result{1.0L, 0.0L};
  for (int axis = 0; axis < 3; ++axis)
    result *= dirichlet_sum(L, mode[axis], width);
  const long double kd = 2.0L*pi*mode[orientation]/L;
  return result*(std::complex<long double>{1.0L, 0.0L}
      -unit_phase(-kd*width));
}

std::complex<long double> direct_structure(
    int L, int width, int orientation,
    const std::array<int, 3>& mode) {
  const long double kd = 2.0L*pi*mode[orientation]/L;
  std::complex<long double> base{1.0L, 0.0L};
  // This is a direct compensated sum of each finite Cartesian site factor.
  // The source is separable, so multiplying the three one-dimensional sums
  // is exactly the explicit w^3 site sum while avoiding an unnecessary
  // cancellation-sensitive triple loop.
  for (int axis = 0; axis < 3; ++axis)
    base *= geometric_sum(L, mode[axis], width);
  const long double displacement_phase = -kd*width;
  const std::complex<long double> paired_factor =
      std::complex<long double>{1.0L, 0.0L}
      -unit_phase(displacement_phase);
  return base*paired_factor;
}

double validate_structure_factor(int L, int width, int orientation) {
  const std::array<std::array<int, 3>, 10> modes{{
      {{0,0,0}}, {{1,0,0}}, {{0,1,0}}, {{0,0,1}},
      {{1,2,3}}, {{2,3,5}}, {{L/7,L/5,L/3}},
      {{L-1,1,2}}, {{L/2,L/4,L/8}}, {{L-3,L-5,L-7}}}};
  long double worst = 0.0L;
  for (const auto& mode : modes) {
    const auto analytic = analytic_structure(L, width, orientation, mode);
    const auto direct = direct_structure(L, width, orientation, mode);
    const long double scale = std::max(
        {1.0L, std::abs(analytic), std::abs(direct)});
    worst = std::max(worst, std::abs(analytic-direct)/scale);
  }
  return static_cast<double>(worst);
}

}  // namespace

TernaryBlockBipolePeierlsResult evaluate_ternary_block_bipole_peierls(
    int L, int width, int orientation_axis, double beta) {
  TernaryBlockBipolePeierlsResult result;
  result.L = L;
  result.width = width;
  result.orientation_axis = orientation_axis;
  result.beta = beta;
  if (L < 3 || width < 1 || orientation_axis < 0
      || orientation_axis > 2 || !(beta > 0.0)
      || !std::isfinite(beta)) return result;

  const std::int64_t side = width;
  result.positive_sites = side*side*side;
  result.negative_sites = side*side*side;
  result.occupied_sites = result.positive_sites+result.negative_sites;
  result.exactly_neutral = result.positive_sites == result.negative_sites;
  result.support_does_not_wrap = 2*width < L;
  if (!result.support_does_not_wrap) return result;

  result.structure_factor_relative_residual = validate_structure_factor(
      L, width, orientation_axis);

  std::array<std::vector<long double>, 3> momentum;
  std::array<std::vector<long double>, 3> cosine;
  std::array<std::vector<long double>, 3> coat;
  std::array<std::vector<long double>, 3> box_squared;
  for (int axis = 0; axis < 3; ++axis) {
    momentum[axis].resize(L);
    cosine[axis].resize(L);
    coat[axis].resize(L);
    box_squared[axis].resize(L);
    for (int mode = 0; mode < L; ++mode) {
      momentum[axis][mode] = 2.0L*pi*mode/L;
      cosine[axis][mode] = std::cos(momentum[axis][mode]);
      coat[axis][mode] = (3.0L+cosine[axis][mode])/4.0L;
      box_squared[axis][mode] = std::norm(
          geometric_sum(L, mode, width));
    }
  }

  CompensatedSum energy_accumulator;
  std::array<CompensatedSum, 3> coefficient_accumulator{};
  std::array<CompensatedSum, 3> average_accumulator{};
  for (int mx = 0; mx < L; ++mx)
    for (int my = 0; my < L; ++my)
      for (int mz = 0; mz < L; ++mz) {
        if (mx == 0 && my == 0 && mz == 0) continue;
        const std::array<int, 3> mode{{mx,my,mz}};
        const long double lambda = 2.0L*(3.0L-cosine[0][mx]
            -cosine[1][my]-cosine[2][mz]);
        const long double split = 2.0L*(1.0L-std::cos(
            width*momentum[orientation_axis][mode[orientation_axis]]));
        const long double source_squared = split*box_squared[0][mx]
            *box_squared[1][my]*box_squared[2][mz];
        const long double coat_squared =
            coat[0][mx]*coat[0][mx]
            *coat[1][my]*coat[1][my]
            *coat[2][mz]*coat[2][mz];
        const long double energy_term = source_squared*coat_squared/lambda;
        energy_accumulator.add(energy_term);
        for (int axis = 0; axis < 3; ++axis) {
          const int m = mode[axis];
          long double transverse = 1.0L;
          for (int j = 0; j < 3; ++j)
            if (j != axis)
              transverse *= coat[j][mode[j]]*coat[j][mode[j]];
          const long double one_minus = 1.0L-cosine[axis][m];
          coefficient_accumulator[axis].add(
              source_squared*one_minus*one_minus*transverse/lambda);
          const long double ratio = one_minus*one_minus
              /((3.0L+cosine[axis][m])*(3.0L+cosine[axis][m]));
          average_accumulator[axis].add(energy_term*ratio);
        }
      }

  const long double energy_sum = energy_accumulator.value;
  const long double volume = static_cast<long double>(L)*L*L;
  const long double scale = static_cast<long double>(beta)/(2.0L*volume);
  result.energy = static_cast<double>(scale*energy_sum);
  bool finite = std::isfinite(result.energy) && result.energy > 0.0;
  for (int axis = 0; axis < 3; ++axis) {
    result.peierls_coefficient[axis] = static_cast<double>(
        scale*coefficient_accumulator[axis].value);
    result.half_cell_barrier[axis] =
        result.peierls_coefficient[axis]/16.0;
    result.pinning_index[axis] =
        result.half_cell_barrier[axis]/result.energy;
    result.spectral_average[axis] = static_cast<double>(
        average_accumulator[axis].value/energy_sum);
    result.spectral_identity_residual[axis] = std::abs(
        result.pinning_index[axis]-result.spectral_average[axis]);
    result.maximum_identity_residual = std::max(
        result.maximum_identity_residual,
        result.spectral_identity_residual[axis]);
    finite = finite && std::isfinite(result.peierls_coefficient[axis])
        && std::isfinite(result.half_cell_barrier[axis])
        && std::isfinite(result.pinning_index[axis])
        && result.peierls_coefficient[axis] > 0.0
        && result.half_cell_barrier[axis] > 0.0
        && result.pinning_index[axis] > 0.0;
  }
  result.maximum_identity_residual = std::max(
      result.maximum_identity_residual,
      result.structure_factor_relative_residual);
  result.valid = result.exactly_neutral && result.support_does_not_wrap
      && finite;
  return result;
}

}  // namespace ftd::eft
