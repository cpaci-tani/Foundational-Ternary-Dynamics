/** FTD-0552: isolated quadratic-coat self-force discriminator. */

#include "ftd/eft/quadratic_coat_discrete_gradient_transaction.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

namespace {

constexpr double gate = 1e-12;
int failures = 0;

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

std::size_t index(int L, int x, int y, int z) {
  const auto wrap = [L](int value) {
    const int remainder = value%L;
    return remainder < 0 ? remainder+L : remainder;
  };
  return (static_cast<std::size_t>(wrap(x))*L+wrap(y))*L+wrap(z);
}

void negative_laplacian(int L, const std::vector<double>& input,
                        std::vector<double>& output) {
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z) {
        const auto i = index(L, x, y, z);
        output[i] = 6.0*input[i]
            -input[index(L, x+1, y, z)]-input[index(L, x-1, y, z)]
            -input[index(L, x, y+1, z)]-input[index(L, x, y-1, z)]
            -input[index(L, x, y, z+1)]-input[index(L, x, y, z-1)];
      }
}

long double dot(const std::vector<double>& lhs,
                const std::vector<double>& rhs) {
  long double result = 0.0L;
  for (std::size_t i = 0; i < lhs.size(); ++i)
    result += static_cast<long double>(lhs[i])*rhs[i];
  return result;
}

double max_abs(const std::vector<double>& values) {
  double result = 0.0;
  for (double value : values) result = std::max(result, std::abs(value));
  return result;
}

struct Fixture {
  ftd::eft::CoupledMatchedFaceState state;
  std::vector<double> stationary;
  double poisson_residual = INFINITY;
  double curl_residual = INFINITY;
  bool valid = false;
  explicit Fixture(int L)
      : state(L), stationary(static_cast<std::size_t>(L)*L*L, 0.0) {}
};

Fixture make_fixture(int L, int charge, const ftd::Vec3& remainder) {
  Fixture fixture(L);
  fixture.state.matter.anchor = {L/2, L/2, L/2};
  fixture.state.matter.remainder = remainder;
  const ftd::Vec3 position{L/2+remainder.x, L/2+remainder.y,
                           L/2+remainder.z};
  const auto coat = ftd::eft::make_quadratic_polarity_coat(position, charge);
  if (!coat.valid) return fixture;
  const std::size_t count = fixture.stationary.size();
  const double background = -static_cast<double>(charge)/count;
  std::fill(fixture.stationary.begin(), fixture.stationary.end(), background);
  std::vector<double> source = fixture.stationary;
  for (std::size_t item = 0; item < coat.weight_count; ++item) {
    const auto& weight = coat.weights[item];
    source[index(L, weight.site.x, weight.site.y, weight.site.z)]
        += weight.weight;
  }
  const long double mean = std::accumulate(
      source.begin(), source.end(), 0.0L)/count;
  for (double& value : source) value -= static_cast<double>(mean);
  std::vector<double> potential(count, 0.0);
  std::vector<double> residual = source;
  std::vector<double> direction = residual;
  std::vector<double> image(count, 0.0);
  long double rr = dot(residual, residual);
  bool converged = max_abs(residual) <= 1e-13;
  for (int iteration = 1; !converged && iteration <= 48*L; ++iteration) {
    negative_laplacian(L, direction, image);
    const long double denominator = dot(direction, image);
    if (!(denominator > 0.0L)) break;
    const long double alpha = rr/denominator;
    for (std::size_t i = 0; i < count; ++i) {
      potential[i] += static_cast<double>(alpha*direction[i]);
      residual[i] -= static_cast<double>(alpha*image[i]);
    }
    fixture.poisson_residual = max_abs(residual);
    converged = fixture.poisson_residual <= 1e-13;
    if (converged) break;
    const long double rr_next = dot(residual, residual);
    const long double beta = rr_next/rr;
    for (std::size_t i = 0; i < count; ++i)
      direction[i] = residual[i]+static_cast<double>(beta*direction[i]);
    rr = rr_next;
  }
  if (!converged) return fixture;
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z) {
        const int i = fixture.state.electric.index(x, y, z);
        fixture.state.electric.x[i] = potential[i]
            -potential[index(L, x+1, y, z)];
        fixture.state.electric.y[i] = potential[i]
            -potential[index(L, x, y+1, z)];
        fixture.state.electric.z[i] = potential[i]
            -potential[index(L, x, y, z+1)];
      }
  fixture.curl_residual = ftd::eft::max_curl_adjoint(
      fixture.state.electric);
  fixture.valid = fixture.poisson_residual <= 1e-13
      && fixture.curl_residual <= gate
      && ftd::eft::max_fractional_gauss_residual(
          fixture.state.electric, source) <= gate;
  return fixture;
}

ftd::Vec3 effective_position(const ftd::eft::MatchedMatterPoint& matter) {
  return {matter.anchor.x+matter.remainder.x,
          matter.anchor.y+matter.remainder.y,
          matter.anchor.z+matter.remainder.z};
}

double periodic_position_difference(const ftd::Vec3& lhs,
                                    const ftd::Vec3& rhs, int L) {
  const auto component = [L](double value) {
    return value-std::round(value/L)*L;
  };
  return std::sqrt(std::pow(component(lhs.x-rhs.x), 2)
      +std::pow(component(lhs.y-rhs.y), 2)
      +std::pow(component(lhs.z-rhs.z), 2));
}

struct ArmResult {
  bool algebra_pass = false;
  double max_displacement = 0.0;
  double max_momentum = 0.0;
  double max_identity = 0.0;
  double accumulated_energy = 0.0;
  ftd::Vec3 first_impulse{};
};

ArmResult run_arm(Fixture fixture, int charge) {
  ArmResult result;
  ftd::eft::QuadraticCoatDGOptions options;
  options.gate_tolerance = gate;
  options.solve_tolerance = 2e-14;
  options.max_iterations = 48;
  options.infer_inverse = false;
  const ftd::Vec3 start = effective_position(fixture.state.matter);
  const auto normalization = ftd::eft::measure_face_flux_normalization();
  const double lambda = options.wave_speed*options.dt;
  const double initial_energy =
      ftd::eft::production_flat_energy_from_momentum(
          fixture.state.matter.momentum)
      +normalization.mapped_field_work_coefficient
          *ftd::eft::matched_modified_energy(
              fixture.state.electric, fixture.state.magnetic_half, lambda);
  bool pass = fixture.valid;
  for (int tick = 0; tick < 64; ++tick) {
    const auto transaction =
        ftd::eft::solve_quadratic_coat_dg_transaction(
            fixture.state, charge, fixture.stationary, options);
    if (tick == 0) result.first_impulse = transaction.total_impulse;
    result.max_identity = std::max({result.max_identity,
        transaction.solve_residual, transaction.continuity_residual,
        transaction.gauss_before_residual, transaction.gauss_after_residual,
        transaction.total_energy_residual, transaction.inverse_residual});
    pass = pass && transaction.valid && transaction.gates_pass;
    fixture.state = transaction.after;
    result.max_displacement = std::max(result.max_displacement,
        periodic_position_difference(
            effective_position(fixture.state.matter), start,
            fixture.state.electric.L));
    result.max_momentum = std::max(result.max_momentum,
        fixture.state.matter.momentum.mag());
  }
  const double final_energy =
      ftd::eft::production_flat_energy_from_momentum(
          fixture.state.matter.momentum)
      +normalization.mapped_field_work_coefficient
          *ftd::eft::matched_modified_energy(
              fixture.state.electric, fixture.state.magnetic_half, lambda);
  result.accumulated_energy = std::abs(final_energy-initial_energy);
  result.algebra_pass = pass && result.max_identity <= gate
      && result.accumulated_energy <= gate;
  return result;
}

double vec_residual(const ftd::Vec3& lhs, const ftd::Vec3& rhs,
                    double sign = 1.0) {
  return std::max({std::abs(lhs.x-sign*rhs.x),
      std::abs(lhs.y-sign*rhs.y), std::abs(lhs.z-sign*rhs.z)});
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  const std::array<ftd::Vec3, 3> positions{{
      {0.0, 0.0, 0.0}, {0.5, 0.0, 0.0},
      {0.173, -0.219, 0.287}}};
  int arms = 0;
  int static_arms = 0;
  bool algebra_pass = true;
  double worst_poisson = 0.0;
  double worst_curl = 0.0;
  double largest_displacement = 0.0;
  double largest_momentum = 0.0;
  double worst_identity = 0.0;
  double worst_accumulated_energy = 0.0;
  double polarity_residual = 0.0;
  for (int L_value : {17, 33}) {
    for (const auto& remainder : positions) {
      std::array<ArmResult, 2> polarity{};
      for (int polarity_index = 0; polarity_index < 2; ++polarity_index) {
        const int charge = polarity_index == 0 ? -1 : +1;
        const Fixture fixture = make_fixture(L_value, charge, remainder);
        worst_poisson = std::max(worst_poisson, fixture.poisson_residual);
        worst_curl = std::max(worst_curl, fixture.curl_residual);
        polarity[polarity_index] = run_arm(fixture, charge);
        const ArmResult& arm = polarity[polarity_index];
        ++arms;
        algebra_pass = algebra_pass && fixture.valid && arm.algebra_pass;
        const bool is_static = arm.max_displacement < gate
            && arm.max_momentum < gate;
        if (is_static) ++static_arms;
        largest_displacement = std::max(
            largest_displacement, arm.max_displacement);
        largest_momentum = std::max(largest_momentum, arm.max_momentum);
        worst_identity = std::max(worst_identity, arm.max_identity);
        worst_accumulated_energy = std::max(
            worst_accumulated_energy, arm.accumulated_energy);
      }
      polarity_residual = std::max(polarity_residual,
          vec_residual(polarity[0].first_impulse,
                       polarity[1].first_impulse));
    }
  }
  check("all 12 static-control histories preserve transaction algebra",
        algebra_pass && arms == 12 && worst_identity <= gate
        && worst_accumulated_energy <= gate);
  check("polarity mirror gives the same self-force impulse",
        polarity_residual <= gate);
  const bool self_force_absent = static_arms == arms
      && largest_displacement < gate && largest_momentum < gate;
  check("self-force discriminator reaches a registered classification",
        self_force_absent || (algebra_pass && static_arms < arms));

  const char* verdict = !algebra_pass
      ? "QUADRATIC_COAT_MULTITICK_ALGEBRA_FAILS"
      : (self_force_absent
          ? "QUADRATIC_COAT_SELF_FORCE_ABSENT"
          : "UNSUBTRACTED_QUADRATIC_SELF_FORCE_PRESENT");
  std::cout << "arms," << arms << '\n'
            << "static_arms," << static_arms << '\n'
            << "worst_poisson_residual," << worst_poisson << '\n'
            << "worst_curl_residual," << worst_curl << '\n'
            << "worst_identity_residual," << worst_identity << '\n'
            << "worst_accumulated_energy_residual,"
            << worst_accumulated_energy << '\n'
            << "largest_displacement," << largest_displacement << '\n'
            << "largest_momentum," << largest_momentum << '\n'
            << "polarity_residual," << polarity_residual << '\n'
            << "verdict," << verdict << '\n';
  return failures == 0 ? 0 : 1;
}
