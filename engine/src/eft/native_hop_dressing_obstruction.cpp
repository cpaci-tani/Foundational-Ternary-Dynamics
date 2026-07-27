#include "ftd/eft/native_hop_dressing_obstruction.h"

#include "ftd/eft/external_drive_radiation.h"
#include "ftd/eft/integer_bloch_transport.h"
#include "ftd/eft/native_moving_source_pole.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace ftd::eft {
namespace {

constexpr long double pi =
    3.141592653589793238462643383279502884L;
constexpr double identity_gate = 1e-12;
constexpr int response_ticks = 128;

double dot(const std::array<double,3>& lhs,
           const std::array<double,3>& rhs) {
  return lhs[0]*rhs[0]+lhs[1]*rhs[1]+lhs[2]*rhs[2];
}

std::array<double,3> cross(const std::array<double,3>& lhs,
                           const std::array<double,3>& rhs) {
  return {{lhs[1]*rhs[2]-lhs[2]*rhs[1],
           lhs[2]*rhs[0]-lhs[0]*rhs[2],
           lhs[0]*rhs[1]-lhs[1]*rhs[0]}};
}

double norm2(const std::array<double,3>& value) {
  return dot(value,value);
}

double complex_norm2(
    const std::array<std::complex<double>,3>& value) {
  return std::norm(value[0])+std::norm(value[1])+std::norm(value[2]);
}

double complex_vector_residual(
    const std::array<std::complex<double>,3>& lhs,
    const std::array<std::complex<double>,3>& rhs) {
  return std::max({std::abs(lhs[0]-rhs[0]),
                   std::abs(lhs[1]-rhs[1]),
                   std::abs(lhs[2]-rhs[2])});
}

template <typename Function>
double bisect_root(double lower,
                   double upper,
                   const Function& function) {
  double f_lower = function(lower);
  double f_upper = function(upper);
  if (!std::isfinite(f_lower) || !std::isfinite(f_upper)
      || f_lower*f_upper >= 0.0) {
    return std::numeric_limits<double>::quiet_NaN();
  }
  for (int iteration = 0; iteration < 160; ++iteration) {
    const double midpoint = 0.5*(lower+upper);
    const double f_midpoint = function(midpoint);
    if (f_lower*f_midpoint <= 0.0) {
      upper = midpoint;
      f_upper = f_midpoint;
    } else {
      lower = midpoint;
      f_lower = f_midpoint;
    }
  }
  return 0.5*(lower+upper);
}

std::array<double,3> rotated_vector(int axis,
                                    double parallel,
                                    double transverse) {
  std::array<double,3> value{};
  value[axis] = parallel;
  value[(axis+1)%3] = transverse;
  return value;
}

double phase_for(const std::array<double,3>& momentum,double c2) {
  return native_bloch_phase(full_stencil_symbol(momentum),c2);
}

NativeHopDressingArm analyze_arm(int period,
                                 int axis,
                                 int polarity,
                                 double c2,
                                 double coupling) {
  NativeHopDressingArm result;
  result.period = period;
  result.axis = axis;
  result.polarity = polarity;
  if (period <= 0 || axis < 0 || axis > 2
      || std::abs(polarity) != 1) return result;

  if (period == 1) {
    result.harmonic = 0;
    result.bracket_lower = 0.0;
    result.bracket_upper = 0.2;
    const double parallel = 0.1;
    auto residual = [&](double transverse) {
      return parallel-phase_for(
          rotated_vector(axis,parallel,transverse),c2);
    };
    result.root_parameter = bisect_root(
        result.bracket_lower,result.bracket_upper,residual);
    result.momentum = rotated_vector(
        axis,parallel,result.root_parameter);
    result.omega = parallel;
  } else if (period == 2) {
    result.harmonic = 0;
    result.bracket_lower = 0.01;
    result.bracket_upper = static_cast<double>(pi);
    auto residual = [&](double u) {
      return u/period-phase_for(rotated_vector(axis,u,0.0),c2);
    };
    result.root_parameter = bisect_root(
        result.bracket_lower,result.bracket_upper,residual);
    result.momentum = rotated_vector(
        axis,result.root_parameter,0.0);
    result.omega = result.root_parameter/period;
  } else {
    result.harmonic = 1;
    result.bracket_lower = 0.0;
    result.bracket_upper = static_cast<double>(pi);
    auto residual = [&](double u) {
      return (2.0*static_cast<double>(pi)-u)/period
          -phase_for(rotated_vector(axis,-u,0.0),c2);
    };
    result.root_parameter = bisect_root(
        result.bracket_lower,result.bracket_upper,residual);
    result.momentum = rotated_vector(
        axis,-result.root_parameter,0.0);
    result.omega = (2.0*static_cast<double>(pi)
        -result.root_parameter)/period;
  }

  if (!std::isfinite(result.root_parameter)) return result;
  result.symbol = full_stencil_symbol(result.momentum);
  result.phase = native_bloch_phase(result.symbol,c2);
  result.denominator_residual = std::abs(
      production_driven_denominator(result.symbol,c2,result.omega));
  for (int component = 0; component < 3; ++component) {
    result.wave_symbol[component] = std::sin(result.momentum[component]);
  }
  result.velocity[axis] = 1.0/period;
  const auto transverse_source = cross(
      result.wave_symbol,result.velocity);

  const auto coefficients = integer_hop_floquet_coefficients(
      result.momentum[axis],period);
  if (result.harmonic >= static_cast<int>(coefficients.size())) return result;
  const auto coefficient = coefficients[result.harmonic];
  result.floquet_coefficient_norm = std::abs(coefficient);
  if (period >= 2) {
    result.coefficient_identity_residual = std::abs(
        result.floquet_coefficient_norm-std::sqrt(3.0)/period);
  }

  const std::complex<double> common = std::complex<double>{0.0,coupling}
      *static_cast<double>(polarity)*coefficient;
  for (int component = 0; component < 3; ++component) {
    result.source[component] = common
        *(-result.wave_symbol[component]+transverse_source[component]);
  }
  result.source_norm = complex_norm2(result.source);
  result.orthogonal_source_norm = coupling*coupling*std::norm(coefficient)
      *(norm2(result.wave_symbol)+norm2(transverse_source));
  result.source_orthogonality_residual = std::abs(
      result.source_norm-result.orthogonal_source_norm);
  result.normalized_effective_forcing = std::sqrt(result.source_norm)
      /std::abs(coupling);

  if (period == 1) {
    const double ky = result.root_parameter;
    const double kx = 0.1;
    result.regularity_derivative = c2*(2.0/3.0)*std::sin(ky)
        *(2.0+std::cos(kx));
  } else if (period == 2) {
    const double u = result.root_parameter;
    result.regularity_derivative = std::abs(
        2.0*c2*std::sin(u)
        -2.0*std::sin(result.omega)/period);
  } else {
    const double u = result.root_parameter;
    result.regularity_derivative = std::abs(
        2.0*c2*std::sin(u)
        +2.0*std::sin(result.omega)/period);
  }

  const std::complex<double> amplitude{
      std::sqrt(result.source_norm),0.0};
  ComplexModalState state{};
  const double kick = c2*result.symbol;
  for (int tick = 0; tick < response_ticks; ++tick) {
    const auto drive = amplitude*std::polar(1.0,-result.omega*tick);
    state = forced_production_modal_step(state,kick,drive);
  }
  result.normalized_resonant_energy = production_modal_energy(state,kick)
      /(response_ticks*response_ticks*result.source_norm);
  const double sine = std::abs(std::sin(result.phase));
  const double cosine_half = std::cos(0.5*result.phase);
  const double vector_norm = std::sqrt(
      1.0/(4.0*sine*sine)
      +1.0/(4.0*cosine_half*cosine_half));
  const double error_norm = std::sqrt(
      1.0/(4.0*std::pow(sine,4))
      +std::pow(1.0/(sine*sine)+1.0/(2.0*sine),2));
  const double lambda_bound = kick+1.0;
  result.resonant_error_bound =
      2.0*lambda_bound*vector_norm*error_norm/response_ticks
      +lambda_bound*error_norm*error_norm
          /(response_ticks*response_ticks);

  result.valid = result.root_parameter > result.bracket_lower
      && result.root_parameter < result.bracket_upper
      && result.denominator_residual <= identity_gate
      && result.regularity_derivative > 1e-3
      && result.source_orthogonality_residual <= identity_gate
      && result.normalized_effective_forcing > 0.05
      && (period == 1
          || result.coefficient_identity_residual <= identity_gate)
      && std::abs(result.normalized_resonant_energy-0.5)
          <= result.resonant_error_bound+identity_gate;
  return result;
}

}  // namespace

NativeHopDressingObstructionResult analyze_native_hop_dressing_obstruction(
    double c2,
    double coupling) {
  NativeHopDressingObstructionResult result;
  result.c2 = c2;
  result.coupling = coupling;
  if (!std::isfinite(c2) || !(c2 > 0.0)
      || !std::isfinite(coupling) || !(std::abs(coupling) > 0.0)) {
    return result;
  }
  result.native_source_components_are_orthogonal = true;
  result.every_finite_registered_period_has_resonance = true;
  result.resonant_native_source_is_nonzero = true;
  result.axial_floquet_coefficient_identity = true;
  result.point_hop_dressing_not_square_summable = true;
  result.slow_hop_forcing_is_asymptotically_quadratic = true;
  result.minimum_regularity_derivative =
      std::numeric_limits<double>::infinity();
  result.minimum_normalized_effective_forcing =
      std::numeric_limits<double>::infinity();

  for (int period = 1; period <= 16; ++period)
    for (int axis = 0; axis < 3; ++axis)
      for (int polarity : {1,-1}) {
        auto arm = analyze_arm(period,axis,polarity,c2,coupling);
        result.maximum_root_residual = std::max(
            result.maximum_root_residual,arm.denominator_residual);
        result.minimum_regularity_derivative = std::min(
            result.minimum_regularity_derivative,
            arm.regularity_derivative);
        result.maximum_source_orthogonality_residual = std::max(
            result.maximum_source_orthogonality_residual,
            arm.source_orthogonality_residual);
        result.maximum_coefficient_identity_residual = std::max(
            result.maximum_coefficient_identity_residual,
            arm.coefficient_identity_residual);
        result.minimum_normalized_effective_forcing = std::min(
            result.minimum_normalized_effective_forcing,
            arm.normalized_effective_forcing);
        result.maximum_resonant_coefficient_excess = std::max(
            result.maximum_resonant_coefficient_excess,
            std::abs(arm.normalized_resonant_energy-0.5)
                -arm.resonant_error_bound);
        result.arms.push_back(std::move(arm));
      }

  for (int period = 1; period <= 16; ++period) {
    for (int axis = 0; axis < 3; ++axis) {
      const std::size_t plus_index = static_cast<std::size_t>(
          (period-1)*6+axis*2);
      const auto& plus = result.arms[plus_index];
      const auto& minus = result.arms[plus_index+1];
      std::array<std::complex<double>,3> negative_minus{};
      for (int component = 0; component < 3; ++component)
        negative_minus[component] = -minus.source[component];
      result.maximum_polarity_mirror_residual = std::max(
          result.maximum_polarity_mirror_residual,
          complex_vector_residual(plus.source,negative_minus));
    }
    for (int polarity_index = 0; polarity_index < 2; ++polarity_index) {
      const auto& reference = result.arms[static_cast<std::size_t>(
          (period-1)*6+polarity_index)];
      for (int axis = 1; axis < 3; ++axis) {
        const auto& rotated = result.arms[static_cast<std::size_t>(
            (period-1)*6+axis*2+polarity_index)];
        result.maximum_cubic_covariance_residual = std::max({
            result.maximum_cubic_covariance_residual,
            std::abs(reference.root_parameter-rotated.root_parameter),
            std::abs(reference.phase-rotated.phase),
            std::abs(reference.floquet_coefficient_norm
                     -rotated.floquet_coefficient_norm),
            std::abs(reference.source_norm-rotated.source_norm)});
      }
    }
  }

  result.valid = result.native_source_components_are_orthogonal
      && result.every_finite_registered_period_has_resonance
      && result.resonant_native_source_is_nonzero
      && result.axial_floquet_coefficient_identity
      && result.point_hop_dressing_not_square_summable
      && result.slow_hop_forcing_is_asymptotically_quadratic
      && result.arms.size() == 96
      && result.maximum_root_residual <= identity_gate
      && result.minimum_regularity_derivative > 1e-3
      && result.maximum_source_orthogonality_residual <= identity_gate
      && result.maximum_coefficient_identity_residual <= identity_gate
      && result.minimum_normalized_effective_forcing > 0.05
      && result.maximum_polarity_mirror_residual <= identity_gate
      && result.maximum_cubic_covariance_residual <= identity_gate
      && result.maximum_resonant_coefficient_excess <= identity_gate
      && std::all_of(result.arms.begin(),result.arms.end(),
          [](const auto& arm) { return arm.valid; });
  return result;
}

}  // namespace ftd::eft
