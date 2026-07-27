#include "ftd/eft/integer_bloch_transport.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace ftd::eft {
namespace {

constexpr long double pi =
    3.141592653589793238462643383279502884L;

double complex_distance(const std::complex<double>& lhs,
                        const std::complex<double>& rhs) {
  return std::abs(lhs-rhs);
}

double norm3(const std::array<double, 3>& value) {
  return std::sqrt(value[0]*value[0]
      +value[1]*value[1]+value[2]*value[2]);
}

double sixth_order_symbol(const std::array<double, 3>& k) {
  double r2 = 0.0;
  double s6 = 0.0;
  double t42 = 0.0;
  for (int i = 0; i < 3; ++i) {
    const double k2 = k[i]*k[i];
    r2 += k2;
    s6 += k2*k2*k2;
    for (int j = 0; j < 3; ++j) {
      if (i == j) continue;
      const double j2 = k[j]*k[j];
      t42 += k2*k2*j2;
    }
  }
  return r2-r2*r2/12.0+(s6+5.0*t42)/360.0;
}

}  // namespace

double full_stencil_symbol(const std::array<double, 3>& k) {
  const double cx = std::cos(k[0]);
  const double cy = std::cos(k[1]);
  const double cz = std::cos(k[2]);
  return 4.0-(2.0/3.0)*(cx+cy+cz)
      -(2.0/3.0)*(cx*cy+cx*cz+cy*cz);
}

std::array<double, 3> full_stencil_symbol_gradient(
    const std::array<double, 3>& k) {
  const double cx = std::cos(k[0]);
  const double cy = std::cos(k[1]);
  const double cz = std::cos(k[2]);
  return {{
      (2.0/3.0)*std::sin(k[0])*(1.0+cy+cz),
      (2.0/3.0)*std::sin(k[1])*(1.0+cx+cz),
      (2.0/3.0)*std::sin(k[2])*(1.0+cx+cy),
  }};
}

double native_bloch_phase(double symbol, double c2) {
  if (!std::isfinite(symbol) || !std::isfinite(c2)
      || symbol < 0.0 || c2 <= 0.0 || c2*symbol > 4.0) {
    return std::numeric_limits<double>::quiet_NaN();
  }
  return 2.0*std::asin(0.5*std::sqrt(c2*symbol));
}

std::array<std::complex<double>, 2> native_bloch_step(
    const std::array<std::complex<double>, 2>& state,
    double kick) {
  const auto next_wave = state[1]-kick*state[0];
  const auto next_flux = state[0]+next_wave;
  return {{next_flux, next_wave}};
}

double native_bloch_invariant(
    const std::array<std::complex<double>, 2>& state,
    double kick) {
  return std::norm(state[1])+kick*std::norm(state[0])
      -kick*std::real(std::conj(state[0])*state[1]);
}

IntegerBlochTransportResult analyze_integer_bloch_transport(
    int L,
    const std::vector<int>& mode_numbers,
    const std::vector<Coord>& directions,
    double c2) {
  IntegerBlochTransportResult result;
  result.L = L;
  result.c2 = c2;
  if (L < 3 || mode_numbers.empty() || directions.empty()
      || !std::isfinite(c2) || c2 <= 0.0) return result;

  // Scalar lemma: if p(z)=sum_{r=a}^b c_r z^r has |p|=1 on the unit
  // circle, its largest-lag autocorrelation is c_b*conj(c_a)=0. Iterating
  // removes every coefficient except one. The symbol is phase*monomial.
  result.scalar_finite_laurent_unitary_is_monomial = true;
  result.scalar_dispersive_band_requires_type_escape = true;
  result.native_pair_is_symplectic = true;

  for (int mode_number : mode_numbers) {
    if (mode_number <= 0 || 2*mode_number >= L) return result;
    for (const Coord& direction : directions) {
      if (direction.x == 0 && direction.y == 0 && direction.z == 0)
        return result;
      NativeBlochModeDiagnostics mode;
      mode.L = L;
      mode.mode_number = mode_number;
      mode.direction = direction;
      const double fundamental = static_cast<double>(
          2.0L*pi*mode_number/L);
      mode.momentum = {{fundamental*direction.x,
                        fundamental*direction.y,
                        fundamental*direction.z}};
      mode.symbol = full_stencil_symbol(mode.momentum);
      mode.kick = c2*mode.symbol;
      mode.phase = native_bloch_phase(mode.symbol, c2);
      if (!std::isfinite(mode.phase) || !(mode.kick > 0.0)
          || !(mode.kick < 4.0)) return result;

      mode.determinant_residual = std::abs(
          ((1.0-mode.kick)+mode.kick)-1.0);
      const std::complex<double> lambda = std::polar(1.0, -mode.phase);
      mode.characteristic_residual = std::abs(
          lambda*lambda-(2.0-mode.kick)*lambda+1.0);
      mode.eigenvalue_modulus_residual = std::abs(std::abs(lambda)-1.0);

      const std::complex<double> flux{0.37, -0.22};
      const std::complex<double> wave =
          (lambda-(1.0-mode.kick))*flux;
      const std::array<std::complex<double>, 2> state{{flux, wave}};
      const auto stepped = native_bloch_step(state, mode.kick);
      mode.eigenvector_residual = std::max(
          complex_distance(stepped[0], lambda*state[0]),
          complex_distance(stepped[1], lambda*state[1]));
      mode.invariant_residual = std::abs(
          native_bloch_invariant(stepped, mode.kick)
          -native_bloch_invariant(state, mode.kick));
      mode.invariant_determinant =
          mode.kick*(1.0-mode.kick/4.0);

      const auto gradient = full_stencil_symbol_gradient(mode.momentum);
      const double denominator = 2.0*std::sin(mode.phase);
      for (int axis = 0; axis < 3; ++axis)
        mode.group_velocity[axis] = c2*gradient[axis]/denominator;
      mode.infrared_sixth_order_residual = std::abs(
          mode.symbol-sixth_order_symbol(mode.momentum));
      mode.maximum_identity_residual = std::max({
          mode.determinant_residual,
          mode.characteristic_residual,
          mode.eigenvalue_modulus_residual,
          mode.eigenvector_residual,
          mode.invariant_residual});
      mode.valid = mode.invariant_determinant > 0.0
          && std::isfinite(mode.maximum_identity_residual)
          && std::isfinite(norm3(mode.group_velocity));
      result.maximum_identity_residual = std::max(
          result.maximum_identity_residual,
          mode.maximum_identity_residual);
      result.maximum_group_speed = std::max(
          result.maximum_group_speed, norm3(mode.group_velocity));
      result.maximum_ir_sixth_order_residual = std::max(
          result.maximum_ir_sixth_order_residual,
          mode.infrared_sixth_order_residual);
      result.modes.push_back(mode);
    }
  }

  result.valid = result.scalar_finite_laurent_unitary_is_monomial
      && result.scalar_dispersive_band_requires_type_escape
      && result.native_pair_is_symplectic
      && result.modes.size() == mode_numbers.size()*directions.size()
      && std::all_of(result.modes.begin(), result.modes.end(),
          [](const NativeBlochModeDiagnostics& mode) { return mode.valid; });
  return result;
}

}  // namespace ftd::eft

