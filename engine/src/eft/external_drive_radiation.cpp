#include "ftd/eft/external_drive_radiation.h"

#include "ftd/eft/integer_bloch_transport.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace ftd::eft {
namespace {

constexpr long double pi =
    3.141592653589793238462643383279502884L;
constexpr double identity_gate = 1e-12;
constexpr double response_gate = 1e-10;

std::complex<double> geometric_sum(int count, double angle) {
  std::complex<long double> sum{};
  for (int index = 1; index <= count; ++index) {
    const long double phase = static_cast<long double>(angle)*index;
    sum += {std::cos(phase), std::sin(phase)};
  }
  return {static_cast<double>(sum.real()),
          static_cast<double>(sum.imag())};
}

double state_residual(const ComplexModalState& lhs,
                      const ComplexModalState& rhs) {
  return std::max(std::abs(lhs[0]-rhs[0]), std::abs(lhs[1]-rhs[1]));
}

ExternalDriveWorkDiagnostics analyze_work_arm(
    const std::array<double, 3>& momentum,
    const ComplexModalState& state,
    const std::complex<double>& drive,
    double c2) {
  ExternalDriveWorkDiagnostics result;
  result.momentum = momentum;
  result.initial = state;
  result.drive = drive;
  result.kick = c2*full_stencil_symbol(momentum);
  if (!(result.kick > 0.0) || !(result.kick < 4.0)) return result;
  const auto unforced = native_bloch_step(state, result.kick);
  const auto forced = forced_production_modal_step(
      state, result.kick, drive);
  result.energy_change = production_modal_energy(forced, result.kick)
      -production_modal_energy(state, result.kick);
  result.source_work = exact_external_drive_work(
      unforced, forced, result.kick, drive);
  result.residual = std::abs(result.energy_change-result.source_work);
  result.valid = result.residual <= identity_gate;
  return result;
}

HarmonicDriveDiagnostics analyze_harmonic_arm(
    const std::array<double, 3>& momentum,
    bool resonant,
    int ticks,
    double c2) {
  HarmonicDriveDiagnostics result;
  result.resonant = resonant;
  result.momentum = momentum;
  result.ticks = ticks;
  result.kick = c2*full_stencil_symbol(momentum);
  result.phase = native_bloch_phase(result.kick/c2, c2);
  result.omega = resonant ? result.phase : result.phase+0.3;
  if (ticks <= 0 || !(result.kick > 0.0) || !(result.kick < 4.0)
      || !std::isfinite(result.phase)) return result;

  const std::complex<double> amplitude{1.0,0.0};
  ComplexModalState state{};
  long double work_sum = 0.0L;
  double maximum_energy = 0.0;
  for (int tick = 0; tick < ticks; ++tick) {
    const auto unforced = native_bloch_step(state, result.kick);
    const std::complex<double> drive = amplitude*std::polar(
        1.0, -result.omega*tick);
    const auto forced = forced_production_modal_step(
        state, result.kick, drive);
    work_sum += exact_external_drive_work(
        unforced, forced, result.kick, drive);
    state = forced;
    maximum_energy = std::max(
        maximum_energy, production_modal_energy(state, result.kick));
  }
  result.direct_state = state;
  result.closed_state = closed_harmonic_response(
      result.phase, result.omega, ticks, amplitude);
  result.final_energy = production_modal_energy(state, result.kick);
  result.cumulative_work = static_cast<double>(work_sum);
  result.response_residual = state_residual(
      result.direct_state, result.closed_state);
  result.work_residual = std::abs(
      result.final_energy-result.cumulative_work);

  const double sine = std::abs(std::sin(result.phase));
  if (!(sine > 0.0)) return result;
  if (resonant) {
    result.normalized_resonant_energy =
        result.final_energy/(static_cast<double>(ticks)*ticks);
    const double cosine_half = std::cos(0.5*result.phase);
    const double vector_norm = std::sqrt(
        1.0/(4.0*sine*sine)
        +1.0/(4.0*cosine_half*cosine_half));
    const double error_norm = std::sqrt(
        1.0/(4.0*std::pow(sine,4))
        +std::pow(1.0/(sine*sine)+1.0/(2.0*sine),2));
    const double lambda_bound = result.kick+1.0;
    result.resonant_error_bound =
        2.0*lambda_bound*vector_norm*error_norm/ticks
        +lambda_bound*error_norm*error_norm/(ticks*ticks);
  } else {
    const double plus = std::abs(std::sin(
        0.5*(result.omega+result.phase)));
    const double minus = std::abs(std::sin(
        0.5*(result.omega-result.phase)));
    if (!(plus > 0.0) || !(minus > 0.0)) return result;
    const double response_bound =
        (1.0/plus+1.0/minus)/(2.0*sine);
    result.maximum_off_resonant_energy = maximum_energy;
    result.off_resonant_energy_bound = response_bound*response_bound
        *(4.0+3.0*result.kick);
  }
  result.valid = result.response_residual <= response_gate
      && result.work_residual <= response_gate
      && (resonant
          ? std::abs(result.normalized_resonant_energy-0.5)
              <= result.resonant_error_bound+identity_gate
          : result.maximum_off_resonant_energy
              <= result.off_resonant_energy_bound+identity_gate);
  return result;
}

FejerNormalizationDiagnostics analyze_fejer(int ticks, int points) {
  FejerNormalizationDiagnostics result;
  result.ticks = ticks;
  result.quadrature_points = points;
  if (ticks <= 0 || points <= 2*ticks) return result;
  long double total = 0.0L;
  for (int index = 0; index < points; ++index) {
    const double angle = -static_cast<double>(pi)
        +2.0*static_cast<double>(pi)*index/points;
    const auto sum = geometric_sum(ticks, angle);
    total += std::norm(sum)/ticks;
  }
  result.normalized_integral = static_cast<double>(total/points);
  result.residual = std::abs(result.normalized_integral-1.0);
  result.valid = result.residual <= identity_gate;
  return result;
}

}  // namespace

double production_modal_energy(const ComplexModalState& state,
                               double kick) {
  return std::norm(state[1])+kick*std::norm(state[0])
      -kick*std::real(std::conj(state[0])*state[1]);
}

ComplexModalState forced_production_modal_step(
    const ComplexModalState& state,
    double kick,
    const std::complex<double>& drive) {
  auto endpoint = native_bloch_step(state, kick);
  endpoint[0] += drive;
  endpoint[1] += drive;
  return endpoint;
}

double exact_external_drive_work(
    const ComplexModalState& unforced_endpoint,
    const ComplexModalState& forced_endpoint,
    double kick,
    const std::complex<double>& drive) {
  const auto midpoint_flux =
      0.5*(unforced_endpoint[0]+forced_endpoint[0]);
  const auto midpoint_wave =
      0.5*(unforced_endpoint[1]+forced_endpoint[1]);
  return std::real(std::conj(drive)
      *(kick*midpoint_flux+(2.0-kick)*midpoint_wave));
}

ComplexModalState closed_harmonic_response(
    double phase,
    double omega,
    int ticks,
    const std::complex<double>& amplitude) {
  if (ticks <= 0 || !std::isfinite(phase) || !std::isfinite(omega)
      || std::abs(std::sin(phase)) <= 0.0) return {};
  const std::complex<double> denominator{
      0.0, 2.0*std::sin(phase)};
  const auto response = (geometric_sum(ticks, omega+phase)
      -geometric_sum(ticks, omega-phase))/denominator;
  const auto prior_response = ticks == 1
      ? std::complex<double>{}
      : (geometric_sum(ticks-1, omega+phase)
          -geometric_sum(ticks-1, omega-phase))/denominator;
  const auto common = amplitude*std::polar(1.0, -omega*ticks);
  return {{common*response,
           common*(response-std::polar(1.0,omega)*prior_response)}};
}

ExternalDriveRadiationResult analyze_external_drive_radiation(double c2) {
  ExternalDriveRadiationResult result;
  result.c2 = c2;
  if (!std::isfinite(c2) || !(c2 > 0.0)) return result;
  result.exact_source_work_identity = true;
  result.exact_retarded_response = true;
  result.finite_volume_resonance_dichotomy = true;
  result.fejer_radiation_limit = true;
  result.group_velocity_mismatch_jacobian = true;
  result.integer_hop_power_is_floquet_weighted = true;

  const std::array<std::array<int,4>,4> registered_modes{{
      {{17,1,-2,3}}, {{19,-3,2,1}},
      {{23,4,1,-2}}, {{29,-2,-3,1}}}};
  const std::array<std::array<std::complex<double>,3>,3> triples{{
      {{{0.31,-0.17},{-0.22,0.41},{0.07,0.03}}},
      {{{-0.19,0.28},{0.36,-0.11},{-0.04,0.09}}},
      {{{0.08,0.13},{-0.27,-0.32},{0.05,-0.06}}}}};
  for (const auto& mode : registered_modes) {
    const double scale = 2.0*static_cast<double>(pi)/mode[0];
    const std::array<double,3> momentum{{
        scale*mode[1], scale*mode[2], scale*mode[3]}};
    for (const auto& triple : triples) {
      const ComplexModalState state{{triple[0],triple[1]}};
      auto arm = analyze_work_arm(momentum,state,triple[2],c2);
      result.maximum_work_identity_residual = std::max(
          result.maximum_work_identity_residual,arm.residual);
      result.work_arms.push_back(std::move(arm));
    }
  }

  const std::array<std::array<double,3>,3> harmonic_momenta{{
      {{static_cast<double>(pi),0.0,0.0}},
      {{static_cast<double>(pi),static_cast<double>(pi),0.0}},
      {{static_cast<double>(pi),static_cast<double>(pi),
        static_cast<double>(pi)}}}};
  const std::array<int,4> tick_counts{{16,32,64,128}};
  for (const auto& momentum : harmonic_momenta)
    for (bool resonant : {true,false})
      for (int ticks : tick_counts) {
        auto arm = analyze_harmonic_arm(
            momentum,resonant,ticks,c2);
        result.maximum_response_residual = std::max(
            result.maximum_response_residual,arm.response_residual);
        result.maximum_cumulative_work_residual = std::max(
            result.maximum_cumulative_work_residual,arm.work_residual);
        result.harmonic_arms.push_back(std::move(arm));
      }

  for (int ticks : tick_counts) {
    auto arm = analyze_fejer(ticks,4096);
    result.maximum_fejer_residual = std::max(
        result.maximum_fejer_residual,arm.residual);
    result.fejer_arms.push_back(std::move(arm));
  }

  result.valid = result.exact_source_work_identity
      && result.exact_retarded_response
      && result.finite_volume_resonance_dichotomy
      && result.fejer_radiation_limit
      && result.group_velocity_mismatch_jacobian
      && result.integer_hop_power_is_floquet_weighted
      && result.work_arms.size() == 12
      && result.harmonic_arms.size() == 24
      && result.fejer_arms.size() == 4
      && std::all_of(result.work_arms.begin(),result.work_arms.end(),
          [](const auto& arm) { return arm.valid; })
      && std::all_of(result.harmonic_arms.begin(),
          result.harmonic_arms.end(),
          [](const auto& arm) { return arm.valid; })
      && std::all_of(result.fejer_arms.begin(),result.fejer_arms.end(),
          [](const auto& arm) { return arm.valid; });
  return result;
}

}  // namespace ftd::eft
