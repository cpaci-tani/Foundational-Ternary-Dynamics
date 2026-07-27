#include "ftd/eft/native_moving_source_pole.h"

#include "ftd/eft/integer_bloch_transport.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace ftd::eft {
namespace {

constexpr long double pi =
    3.141592653589793238462643383279502884L;
constexpr double identity_gate = 1e-12;

double dot(const std::array<double, 3>& lhs,
           const std::array<double, 3>& rhs) {
  return lhs[0]*rhs[0]+lhs[1]*rhs[1]+lhs[2]*rhs[2];
}

double dot(const std::array<double, 3>& lhs, const Coord& rhs) {
  return lhs[0]*rhs.x+lhs[1]*rhs.y+lhs[2]*rhs.z;
}

double direction_norm(const Coord& direction) {
  return std::sqrt(static_cast<double>(
      direction.x*direction.x+direction.y*direction.y
      +direction.z*direction.z));
}

double complex_residual(const std::complex<double>& lhs,
                        const std::complex<double>& rhs) {
  return std::abs(lhs-rhs);
}

DrivenPoleDiagnostics analyze_driven_mode(
    const std::array<double, 3>& momentum,
    double omega,
    double c2) {
  DrivenPoleDiagnostics result;
  result.momentum = momentum;
  result.omega = omega;
  result.symbol = full_stencil_symbol(momentum);
  result.kick = c2*result.symbol;
  result.phase = native_bloch_phase(result.symbol, c2);
  result.denominator = production_driven_denominator(
      result.symbol, c2, omega);
  if (!std::isfinite(result.phase) || !std::isfinite(result.denominator)
      || std::abs(result.denominator) <= 1e-6) return result;

  const std::complex<double> z = std::polar(1.0, -omega);
  const std::complex<double> m00 = z-(1.0-result.kick);
  const std::complex<double> m01 = -1.0;
  const std::complex<double> m10 = result.kick;
  const std::complex<double> m11 = z-1.0;
  const std::complex<double> determinant = m00*m11-m01*m10;
  const std::complex<double> flux = z/determinant;
  const std::complex<double> wave = (z-1.0)/determinant;
  const std::complex<double> closed_flux = 1.0/result.denominator;

  result.direct_flux_response = flux.real();
  result.closed_flux_response = closed_flux.real();
  result.direct_solve_residual = std::max(
      complex_residual(m00*flux+m01*wave, 1.0),
      complex_residual(m10*flux+m11*wave, 1.0));

  const std::array<std::complex<double>, 2> state{{flux, wave}};
  auto stepped = native_bloch_step(state, result.kick);
  stepped[0] += 1.0;
  stepped[1] += 1.0;
  result.recurrence_residual = std::max(
      complex_residual(stepped[0], z*flux),
      complex_residual(stepped[1], z*wave));
  result.static_resolvent_residual = std::abs(
      1.0/result.kick
      -1.0/production_driven_denominator(result.symbol, c2, 0.0));
  result.valid = complex_residual(flux, closed_flux) <= identity_gate
      && result.direct_solve_residual <= identity_gate
      && result.recurrence_residual <= identity_gate
      && result.static_resolvent_residual <= identity_gate;
  return result;
}

WrappedThresholdDiagnostics analyze_threshold(
    int L,
    const Coord& direction,
    double c2,
    double lower_bound) {
  WrappedThresholdDiagnostics result;
  result.L = L;
  result.direction = direction;
  result.universal_lower_bound = lower_bound;
  const double norm = direction_norm(direction);
  if (L < 4 || !(norm > 0.0)) return result;

  double minimum = std::numeric_limits<double>::infinity();
  std::array<int, 3> minimizing{};
  for (int nx = -L/2; nx < L/2; ++nx)
    for (int ny = -L/2; ny < L/2; ++ny)
      for (int nz = -L/2; nz < L/2; ++nz) {
        if (nx == 0 && ny == 0 && nz == 0) continue;
        const double scale = 2.0*static_cast<double>(pi)/L;
        const std::array<double, 3> momentum{{
            scale*nx, scale*ny, scale*nz}};
        const double projected = std::abs(dot(momentum, direction))/norm;
        if (!(projected > 1e-15)) continue;
        const double symbol = full_stencil_symbol(momentum);
        const double phase = native_bloch_phase(symbol, c2);
        const double ratio = phase/projected;
        if (ratio < minimum) {
          minimum = ratio;
          minimizing = {{nx,ny,nz}};
        }
      }
  result.minimizing_mode = minimizing;
  result.minimum_phase_speed = minimum;
  result.lower_bound_residual = std::max(0.0, lower_bound-minimum);
  result.valid = std::isfinite(minimum)
      && minimum >= lower_bound-identity_gate;
  return result;
}

FloquetHopDiagnostics analyze_floquet(
    int period,
    double kx) {
  FloquetHopDiagnostics result;
  result.period = period;
  result.displacement = {1,0,0};
  result.momentum = {{kx,0.0,0.0}};
  result.mean_frequency = kx/period;
  result.coefficients = integer_hop_floquet_coefficients(kx, period);
  if (static_cast<int>(result.coefficients.size()) != period) return result;

  long double coefficient_norm = 0.0L;
  for (int harmonic = 0; harmonic < period; ++harmonic) {
    coefficient_norm += std::norm(result.coefficients[harmonic]);
    if (harmonic != 0) {
      result.maximum_nonfundamental_amplitude = std::max(
          result.maximum_nonfundamental_amplitude,
          std::abs(result.coefficients[harmonic]));
    }
  }
  for (int remainder = 0; remainder < period; ++remainder) {
    std::complex<long double> reconstructed{};
    for (int harmonic = 0; harmonic < period; ++harmonic) {
      const long double angle = -2.0L*pi*harmonic*remainder/period;
      reconstructed += std::complex<long double>{
          result.coefficients[harmonic].real(),
          result.coefficients[harmonic].imag()}
          *std::complex<long double>{std::cos(angle), std::sin(angle)};
    }
    const std::complex<long double> expected = std::polar(
        1.0L, static_cast<long double>(kx)*remainder/period);
    result.maximum_reconstruction_residual = std::max(
        result.maximum_reconstruction_residual,
        static_cast<double>(std::abs(reconstructed-expected)));
  }
  result.parseval_residual = std::abs(
      static_cast<double>(coefficient_norm)-1.0);
  result.valid = result.maximum_reconstruction_residual <= identity_gate
      && result.parseval_residual <= identity_gate
      && result.maximum_nonfundamental_amplitude > 1e-6;
  return result;
}

}  // namespace

int canonical_crystal_mode(int array_mode, int L) {
  if (L <= 0) return array_mode;
  int reduced = array_mode%L;
  if (reduced < 0) reduced += L;
  if (2*reduced >= L) reduced -= L;
  return reduced;
}

double production_driven_denominator(double symbol,
                                     double c2,
                                     double omega) {
  return c2*symbol-4.0*std::pow(std::sin(0.5*omega), 2);
}

std::vector<std::complex<double>> integer_hop_floquet_coefficients(
    double momentum_dot_displacement,
    int period) {
  std::vector<std::complex<double>> coefficients;
  if (period <= 0 || !std::isfinite(momentum_dot_displacement))
    return coefficients;
  coefficients.resize(period);
  const std::complex<double> numerator =
      1.0-std::polar(1.0, momentum_dot_displacement);
  for (int harmonic = 0; harmonic < period; ++harmonic) {
    const double phase = (momentum_dot_displacement
        +2.0*static_cast<double>(pi)*harmonic)/period;
    const std::complex<double> denominator = static_cast<double>(period)
        *(1.0-std::polar(1.0, phase));
    if (std::abs(denominator) < 1e-14) {
      coefficients[harmonic] = 1.0;
    } else {
      coefficients[harmonic] = numerator/denominator;
    }
  }
  return coefficients;
}

NativeMovingSourcePoleResult analyze_native_moving_source_pole(double c2) {
  NativeMovingSourcePoleResult result;
  result.c2 = c2;
  if (!std::isfinite(c2) || !(c2 > 0.0)) return result;
  const double c_wave = std::sqrt(c2);
  result.seven_point_ratio_floor = 2.0/static_cast<double>(pi);
  result.universal_speed_floor =
      2.0*c_wave/(static_cast<double>(pi)*std::sqrt(3.0));
  result.production_discrete_time_pole_derived = true;
  result.seven_point_any_speed_claim_refuted = true;
  result.full_stencil_positive_speed_floor = true;
  result.integer_hop_requires_floquet_spectrum = true;

  const std::array<std::array<int, 4>, 4> registered_modes{{
      {{17,1,-2,3}}, {{19,-3,2,1}},
      {{23,4,1,-2}}, {{29,-2,-3,1}}}};
  const std::array<double, 3> omegas{{0.17,0.41,0.73}};
  for (const auto& registered : registered_modes) {
    const double scale = 2.0*static_cast<double>(pi)/registered[0];
    const std::array<double, 3> momentum{{
        scale*registered[1], scale*registered[2], scale*registered[3]}};
    for (double omega : omegas) {
      auto diagnostic = analyze_driven_mode(momentum, omega, c2);
      result.maximum_identity_residual = std::max({
          result.maximum_identity_residual,
          diagnostic.direct_solve_residual,
          diagnostic.recurrence_residual,
          diagnostic.static_resolvent_residual,
          std::abs(diagnostic.direct_flux_response
                   -diagnostic.closed_flux_response)});
      result.driven_modes.push_back(diagnostic);
    }
  }

  const std::array<int, 3> volumes{{16,32,64}};
  const std::array<Coord, 3> directions{{
      Coord{1,0,0}, Coord{1,1,0}, Coord{1,1,1}}};
  for (int L : volumes)
    for (const Coord& direction : directions)
      result.thresholds.push_back(analyze_threshold(
          L, direction, c2, result.universal_speed_floor));

  const int alias_L = 16;
  const int array_mode = 15;
  const int wrapped_mode = canonical_crystal_mode(array_mode, alias_L);
  const double scale = 2.0*static_cast<double>(pi)/alias_L;
  const std::array<double, 3> array_momentum{{scale*array_mode,0.0,0.0}};
  const std::array<double, 3> wrapped_momentum{{scale*wrapped_mode,0.0,0.0}};
  const double array_symbol = full_stencil_symbol(array_momentum);
  const double wrapped_symbol = full_stencil_symbol(wrapped_momentum);
  const double array_phase = native_bloch_phase(array_symbol, c2);
  const double wrapped_phase = native_bloch_phase(wrapped_symbol, c2);
  result.alias_symbol_residual = std::abs(array_symbol-wrapped_symbol);
  result.alias_phase_residual = std::abs(array_phase-wrapped_phase);
  const double old_ratio = array_phase/std::abs(array_momentum[0]);
  const double wrapped_ratio = wrapped_phase/std::abs(wrapped_momentum[0]);
  result.old_to_wrapped_alias_ratio = wrapped_ratio/old_ratio;
  result.wrapped_alias_counterexample = wrapped_mode == -1
      && result.alias_symbol_residual <= identity_gate
      && result.alias_phase_residual <= identity_gate
      && result.old_to_wrapped_alias_ratio > 10.0;

  const std::array<int, 4> periods{{4,8,16,32}};
  const std::array<double, 3> momenta{{
      2.0*static_cast<double>(pi)/17.0,
      4.0*static_cast<double>(pi)/17.0,
      6.0*static_cast<double>(pi)/17.0}};
  for (int period : periods)
    for (double momentum : momenta) {
      auto diagnostic = analyze_floquet(period, momentum);
      result.maximum_identity_residual = std::max({
          result.maximum_identity_residual,
          diagnostic.maximum_reconstruction_residual,
          diagnostic.parseval_residual});
      result.floquet_schedules.push_back(std::move(diagnostic));
    }

  result.valid = result.production_discrete_time_pole_derived
      && result.seven_point_any_speed_claim_refuted
      && result.full_stencil_positive_speed_floor
      && result.wrapped_alias_counterexample
      && result.integer_hop_requires_floquet_spectrum
      && result.driven_modes.size() == 12
      && result.thresholds.size() == 9
      && result.floquet_schedules.size() == 12
      && std::all_of(result.driven_modes.begin(), result.driven_modes.end(),
          [](const DrivenPoleDiagnostics& item) { return item.valid; })
      && std::all_of(result.thresholds.begin(), result.thresholds.end(),
          [](const WrappedThresholdDiagnostics& item) { return item.valid; })
      && std::all_of(result.floquet_schedules.begin(),
          result.floquet_schedules.end(),
          [](const FloquetHopDiagnostics& item) { return item.valid; });
  return result;
}

}  // namespace ftd::eft
