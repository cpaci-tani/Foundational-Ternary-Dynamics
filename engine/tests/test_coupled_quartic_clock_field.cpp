/**
 * FTD-0770: Coupled Quartic Clock Field v1 selected-extension verifier.
 */

#include "ftd/eft/coupled_quartic_clock_field.h"
#include "ftd/ontic/lemniscate.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {

constexpr double exact_gate = 1e-12;
constexpr double period_gate = 2e-6;
constexpr double dispersion_gate = 2e-3;
constexpr double step_size = 0.002;
constexpr std::size_t chain_size = 64;
constexpr double phase_amplitude = 1e-4;
int failures = 0;

void check(const std::string& label, bool condition) {
  if (condition) return;
  ++failures;
  std::cerr << "FAIL: " << label << '\n';
}

double integer_power(double base, int exponent) {
  double result = 1.0;
  for (int i = 0; i < exponent; ++i) result *= base;
  return result;
}

double oscillator_energy(double q, double p, int exponent) {
  return 0.5 * (p * p + integer_power(q, exponent));
}

double oscillator_force(double q, int exponent) {
  return -0.5 * static_cast<double>(exponent)
      * integer_power(q, exponent - 1);
}

struct PeriodMeasurement {
  double measured_period = 0.0;
  double predicted_period = 0.0;
  double relative_period_error = 0.0;
  double relative_energy_drift = 0.0;
  bool valid = false;
};

PeriodMeasurement measure_period(int exponent, double energy) {
  PeriodMeasurement result;
  const auto law = ftd::eft::make_even_power_clock_law(exponent);
  result.predicted_period = law.period(energy);
  if (!law.valid || !(result.predicted_period > 0.0)) return result;

  const int steps_per_period = 32768;
  const double dt = result.predicted_period
      / static_cast<double>(steps_per_period);
  double q = std::pow(2.0 * energy, 1.0 / static_cast<double>(exponent));
  double p = 0.0;
  const double initial_energy = oscillator_energy(q, p, exponent);
  double maximum_energy_drift = 0.0;
  double previous_q = q;
  double elapsed = 0.0;

  for (int step = 0; step < steps_per_period; ++step) {
    p += 0.5 * dt * oscillator_force(q, exponent);
    const double next_q = q + dt * p;
    p += 0.5 * dt * oscillator_force(next_q, exponent);
    elapsed += dt;
    const double current_energy = oscillator_energy(next_q, p, exponent);
    maximum_energy_drift = std::max(maximum_energy_drift,
        std::abs(current_energy - initial_energy) / initial_energy);
    if (previous_q >= 0.0 && next_q < 0.0) {
      const double crossing_fraction = previous_q / (previous_q - next_q);
      const double crossing_time = elapsed - dt + crossing_fraction * dt;
      result.measured_period = 4.0 * crossing_time;
      result.relative_period_error = std::abs(
          result.measured_period / result.predicted_period - 1.0);
      result.relative_energy_drift = maximum_energy_drift;
      result.valid = true;
      return result;
    }
    previous_q = next_q;
    q = next_q;
  }
  return result;
}

double mode_projection(const ftd::eft::CoupledClockField& field, int mode) {
  double mean = 0.0;
  for (const auto& site : field.sites()) mean += site.phase;
  mean /= static_cast<double>(field.sites().size());
  const double wave_number = 2.0 * ftd::ontic::PI
      * static_cast<double>(mode) / static_cast<double>(field.sites().size());
  double projection = 0.0;
  for (std::size_t i = 0; i < field.sites().size(); ++i) {
    projection += (field.sites()[i].phase - mean)
        * std::cos(wave_number * static_cast<double>(i));
  }
  return 2.0 * projection / static_cast<double>(field.sites().size());
}

ftd::eft::CoupledClockField make_chain(int exponent, int mode) {
  const auto law = ftd::eft::make_even_power_clock_law(exponent);
  const double background_energy = 0.5;
  const double background_action = law.action_from_energy(background_energy);
  std::vector<ftd::eft::ClockSite> sites(chain_size);
  const double wave_number = 2.0 * ftd::ontic::PI
      * static_cast<double>(mode) / static_cast<double>(chain_size);
  for (std::size_t i = 0; i < chain_size; ++i) {
    sites[i].action = background_action;
    sites[i].phase = phase_amplitude
        * std::cos(wave_number * static_cast<double>(i));
  }
  std::vector<ftd::eft::ClockEdge> edges;
  edges.reserve(chain_size);
  for (std::size_t i = 0; i < chain_size; ++i) {
    edges.push_back({i, (i + 1) % chain_size, 0.0});
  }
  return {law, std::move(sites), std::move(edges), 0.2};
}

struct DispersionMeasurement {
  double measured_frequency = 0.0;
  double predicted_frequency = 0.0;
  double relative_error = 0.0;
  double action_drift = 0.0;
  double relative_energy_drift = 0.0;
  bool valid = false;
};

DispersionMeasurement measure_dispersion(int exponent, int mode) {
  DispersionMeasurement result;
  auto field = make_chain(exponent, mode);
  if (!field.valid()) return result;
  const double background_action = field.sites().front().action;
  const double wave_number = 2.0 * ftd::ontic::PI
      * static_cast<double>(mode) / static_cast<double>(chain_size);
  result.predicted_frequency = 2.0 * std::sqrt(
      field.stiffness() * field.law().curvature(background_action))
      * std::sin(0.5 * wave_number);
  if (!(result.predicted_frequency > 0.0)) return result;

  const double initial_projection = mode_projection(field, mode);
  const double initial_action = field.total_action();
  const double initial_energy = field.hamiltonian();
  double maximum_action_drift = 0.0;
  double maximum_energy_drift = 0.0;
  double previous_projection = initial_projection;
  double elapsed = 0.0;
  const double maximum_time = ftd::ontic::PI / result.predicted_frequency;

  while (elapsed < maximum_time) {
    if (!field.step(step_size)) return result;
    elapsed += step_size;
    const double projection = mode_projection(field, mode);
    maximum_action_drift = std::max(maximum_action_drift,
        std::abs(field.total_action() - initial_action));
    maximum_energy_drift = std::max(maximum_energy_drift,
        std::abs(field.hamiltonian() - initial_energy) / initial_energy);
    if (previous_projection > 0.0 && projection <= 0.0) {
      const double fraction = previous_projection
          / (previous_projection - projection);
      const double crossing_time = elapsed - step_size + fraction * step_size;
      result.measured_frequency = 0.5 * ftd::ontic::PI / crossing_time;
      result.relative_error = std::abs(
          result.measured_frequency / result.predicted_frequency - 1.0);
      result.action_drift = maximum_action_drift;
      result.relative_energy_drift = maximum_energy_drift;
      result.valid = true;
      return result;
    }
    previous_projection = projection;
  }
  return result;
}

double run_quadratic_control() {
  auto field = make_chain(2, 3);
  const double initial_projection = mode_projection(field, 3);
  for (int step = 0; step < 12500; ++step) {
    if (!field.step(step_size)) return std::numeric_limits<double>::infinity();
  }
  return std::abs(mode_projection(field, 3) - initial_projection);
}

}  // namespace

int main() {
  const auto quadratic = ftd::eft::make_even_power_clock_law(2);
  const auto quartic = ftd::eft::make_even_power_clock_law(4);
  const auto sextic = ftd::eft::make_even_power_clock_law(6);
  check("even-power laws valid", quadratic.valid && quartic.valid && sextic.valid);
  check("odd power rejected", !ftd::eft::make_even_power_clock_law(3).valid);

  const double quartic_action_target = ftd::ontic::G_STAR
      / (3.0 * std::sqrt(ftd::ontic::PI));
  const double quartic_period_target = std::sqrt(ftd::ontic::PI)
      * ftd::ontic::G_STAR;
  check("quartic action coefficient contains canonical G*",
      std::abs(quartic.action_coefficient - quartic_action_target)
          <= exact_gate);
  check("quartic amplitude-one period contains canonical G*",
      std::abs(quartic.unit_shell_period - quartic_period_target)
          <= exact_gate);
  check("Tstar belongs to E=1/2 shell",
      std::abs(quartic.period(0.5) - quartic_period_target) <= exact_gate);
  check("E=1 period is not Tstar",
      std::abs(quartic.period(1.0) - quartic_period_target) > 1e-3);

  double maximum_period_error = 0.0;
  double maximum_single_energy_drift = 0.0;
  double quartic_invariant_min = std::numeric_limits<double>::infinity();
  double quartic_invariant_max = 0.0;
  const std::array<double, 3> doubled_energies{{1.0 / 16.0, 1.0, 16.0}};
  for (int exponent : {2, 4, 6}) {
    for (double doubled_energy : doubled_energies) {
      const auto measurement = measure_period(exponent, 0.5 * doubled_energy);
      maximum_period_error = std::max(
          maximum_period_error, measurement.relative_period_error);
      maximum_single_energy_drift = std::max(
          maximum_single_energy_drift, measurement.relative_energy_drift);
      check("single-clock measurement valid", measurement.valid);
      check("single-clock period gate",
          measurement.valid && measurement.relative_period_error <= period_gate);
      check("single-clock energy gate",
          measurement.valid && measurement.relative_energy_drift <= period_gate);
      if (exponent == 4 && measurement.valid) {
        const double invariant = measurement.measured_period
            * std::pow(doubled_energy, 0.25);
        quartic_invariant_min = std::min(quartic_invariant_min, invariant);
        quartic_invariant_max = std::max(quartic_invariant_max, invariant);
      }
    }
  }
  const double quartic_invariant_spread =
      (quartic_invariant_max - quartic_invariant_min) / quartic_period_target;
  check("quartic scaling-invariant spread", quartic_invariant_spread <= period_gate);

  const double eta = 0.4;
  check("quadratic ratio control",
      std::abs(ftd::eft::linear_wave_cycle_ratio_squared(2, eta))
          <= exact_gate);
  check("quartic ratio cancels G*",
      std::abs(ftd::eft::linear_wave_cycle_ratio_squared(4, eta)
          - eta / 4.0) <= exact_gate);
  check("sextic ratio control",
      std::abs(ftd::eft::linear_wave_cycle_ratio_squared(6, eta)
          - eta / 3.0) <= exact_gate);
  for (const auto& law : {quadratic, quartic, sextic}) {
    const double action = law.action_from_energy(0.5);
    const double direct_ratio = 0.2 * law.curvature(action)
        / (law.frequency(action) * law.frequency(action));
    check("direct wave-cycle ratio matches reduced control",
        std::abs(direct_ratio
            - ftd::eft::linear_wave_cycle_ratio_squared(law.exponent, eta))
            <= exact_gate);
  }
  check("axial continuum factor",
      ftd::eft::axial_continuum_neighbor_factor() == 1.0);
  check("Moore continuum factor",
      ftd::eft::full_moore_continuum_neighbor_factor(3) == 9.0);

  double maximum_dispersion_error = 0.0;
  double maximum_action_drift = 0.0;
  double maximum_chain_energy_drift = 0.0;
  for (int exponent : {4, 6}) {
    for (int mode : {1, 3, 8}) {
      const auto measurement = measure_dispersion(exponent, mode);
      maximum_dispersion_error = std::max(
          maximum_dispersion_error, measurement.relative_error);
      maximum_action_drift = std::max(
          maximum_action_drift, measurement.action_drift);
      maximum_chain_energy_drift = std::max(
          maximum_chain_energy_drift, measurement.relative_energy_drift);
      check("dispersion measurement valid", measurement.valid);
      check("dispersion gate",
          measurement.valid && measurement.relative_error <= dispersion_gate);
      check("chain total-action gate",
          measurement.valid && measurement.action_drift <= 1e-11);
      check("chain Hamiltonian gate", measurement.valid
          && measurement.relative_energy_drift <= 5e-7);
    }
  }
  const double quadratic_phase_drift = run_quadratic_control();
  check("quadratic phase perturbation is non-propagating",
      quadratic_phase_drift <= 1e-10);

  const double equal_action = quartic.action_from_energy(0.5);
  std::vector<ftd::eft::ClockSite> compliance_sites{
      {equal_action, 0.0, 0.2}, {equal_action, 0.0, -0.35}};
  ftd::eft::CoupledClockField compliance_field(
      quartic, compliance_sites, {}, 0.0);
  const double compliance_ratio = compliance_field.phase_rate(0)
      / compliance_field.phase_rate(1);
  const double compliance_target = std::exp(-(0.2 - (-0.35)));
  check("imposed compliance rate law", compliance_field.valid()
      && std::abs(compliance_ratio / compliance_target - 1.0) <= 1e-13);

  const std::vector<ftd::eft::ClockEdge> flat_square{
      {0, 1, 0.0}, {1, 2, 0.0}, {2, 3, 0.0}, {3, 0, 0.0}};
  auto flux_square = flat_square;
  flux_square[0].connection = 0.3;
  const auto flat_integrability =
      ftd::eft::analyze_connection_integrability(4, flat_square);
  const auto flux_integrability =
      ftd::eft::analyze_connection_integrability(4, flux_square);
  check("zero-holonomy square integrable",
      flat_integrability.valid && flat_integrability.integrable);
  check("nonzero-holonomy square not integrable",
      flux_integrability.valid && !flux_integrability.integrable
      && std::abs(flux_integrability.maximum_cycle_residual - 0.3)
          <= exact_gate);

  std::vector<ftd::eft::ClockSite> gauge_sites{
      {equal_action, 0.1, 0.0}, {equal_action, -0.2, 0.0},
      {equal_action, 0.35, 0.0}, {equal_action, -0.45, 0.0}};
  const std::array<double, 4> gauge_shift{{0.2, -0.4, 0.7, -0.1}};
  ftd::eft::CoupledClockField original_gauge_field(
      quartic, gauge_sites, flux_square, 0.2);
  auto transformed_sites = gauge_sites;
  auto transformed_edges = flux_square;
  for (std::size_t i = 0; i < transformed_sites.size(); ++i) {
    transformed_sites[i].phase += gauge_shift[i];
  }
  for (auto& edge : transformed_edges) {
    edge.connection += gauge_shift[edge.tail] - gauge_shift[edge.head];
  }
  ftd::eft::CoupledClockField transformed_gauge_field(
      quartic, transformed_sites, transformed_edges, 0.2);
  const auto transformed_integrability =
      ftd::eft::analyze_connection_integrability(4, transformed_edges);
  check("gauge transformation preserves Hamiltonian",
      std::abs(original_gauge_field.hamiltonian()
          - transformed_gauge_field.hamiltonian()) <= exact_gate);
  check("gauge transformation preserves holonomy obstruction",
      transformed_integrability.valid && !transformed_integrability.integrable
      && std::abs(transformed_integrability.maximum_cycle_residual - 0.3)
          <= exact_gate);

  const auto fixed_connections = original_gauge_field.edges();
  check("fixed-background holonomy step executes",
      original_gauge_field.step(0.01));
  double maximum_connection_change = 0.0;
  for (std::size_t i = 0; i < fixed_connections.size(); ++i) {
    maximum_connection_change = std::max(maximum_connection_change,
        std::abs(original_gauge_field.edges()[i].connection
            - fixed_connections[i].connection));
  }
  check("fixed-background holonomy is kinematic input",
      maximum_connection_change == 0.0);

  std::vector<ftd::eft::ClockSite> boundary_sites{
      {1e-8, 0.5 * ftd::ontic::PI, 0.0},
      {1.0, 0.0, 0.0}};
  ftd::eft::CoupledClockField boundary_field(
      quartic, boundary_sites, {{0, 1, 0.0}}, 1.0);
  const auto boundary_before = boundary_field.sites();
  check("positive-action boundary crossing rejected", !boundary_field.step(1.0));
  check("rejected step rolls back phase",
      boundary_field.sites()[0].phase == boundary_before[0].phase
      && boundary_field.sites()[1].phase == boundary_before[1].phase);
  check("rejected step rolls back action",
      boundary_field.sites()[0].action == boundary_before[0].action
      && boundary_field.sites()[1].action == boundary_before[1].action);

  std::cout.precision(17);
  std::cout << "protocol_sha256="
            << "384C67CF1D6B96829C46C144414B1B5F43E8AE1FCD4FB4D83AA132EFB6616AB4"
            << '\n'
            << "quartic_action_coefficient=" << quartic.action_coefficient << '\n'
            << "quartic_unit_shell_period=" << quartic.unit_shell_period << '\n'
            << "maximum_period_relative_error=" << maximum_period_error << '\n'
            << "maximum_single_energy_relative_drift="
            << maximum_single_energy_drift << '\n'
            << "quartic_invariant_relative_spread="
            << quartic_invariant_spread << '\n'
            << "maximum_dispersion_relative_error="
            << maximum_dispersion_error << '\n'
            << "maximum_total_action_absolute_drift="
            << maximum_action_drift << '\n'
            << "maximum_chain_energy_relative_drift="
            << maximum_chain_energy_drift << '\n'
            << "quadratic_phase_mode_drift=" << quadratic_phase_drift << '\n'
            << "compliance_rate_ratio=" << compliance_ratio << '\n'
            << "nonzero_cycle_residual="
            << flux_integrability.maximum_cycle_residual << '\n'
            << "maximum_fixed_connection_change="
            << maximum_connection_change << '\n'
            << "verdict=COUPLED_QUARTIC_CLOCK_FIELD_V1_CONDITIONAL_THEOREMS_PASS"
            << '\n'
            << "gstar_verdict=GSTAR_LINEAR_SIGNATURE_ABSENT" << '\n'
            << "holonomy_verdict=FIXED_BACKGROUND_HOLONOMY_KINEMATIC_ONLY"
            << '\n'
            << "coupled_quartic_clock_field failures=" << failures << '\n';
  return failures == 0 ? 0 : 1;
}
