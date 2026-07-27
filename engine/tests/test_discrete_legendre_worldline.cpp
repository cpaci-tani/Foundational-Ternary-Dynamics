/** FTD-0490: gauge-covariant interior discrete Legendre map. */

#include "ftd/eft/discrete_legendre_worldline.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {

constexpr int L = 17;
constexpr double c_speed = 0.57735026918962576451;
constexpr double lambda_t = c_speed;
constexpr double rest_energy = 0.511;
constexpr double coupling = 0.73;
constexpr double gate = 1e-12;
int failures = 0;
double worst_deposit_residual = 0.0;
double worst_dispersion_residual = 0.0;
double worst_canonical_covariance = 0.0;
double worst_kinetic_invariance = 0.0;
double worst_pure_gauge_residual = 0.0;
double worst_symmetry_residual = 0.0;

void check(const std::string& label, bool pass) {
  std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
  if (!pass) ++failures;
}

double max_component(const ftd::Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

double max_difference(const ftd::Vec3& lhs, const ftd::Vec3& rhs) {
  return std::max({std::abs(lhs.x - rhs.x),
                   std::abs(lhs.y - rhs.y),
                   std::abs(lhs.z - rhs.z)});
}

ftd::eft::DualGaugePotentialSlab general_slab() {
  ftd::eft::DualGaugePotentialSlab slab(L, lambda_t);
  constexpr double pi = 3.141592653589793238462643383279502884;
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const int i = slab.index(x, y, z);
        const double px = 2.0 * pi * x / L;
        const double py = 2.0 * pi * y / L;
        const double pz = 2.0 * pi * z / L;
        slab.A_start.x[static_cast<std::size_t>(i)] =
            0.013 * std::sin(py) + 0.004 * std::cos(pz);
        slab.A_start.y[static_cast<std::size_t>(i)] =
            -0.009 * std::sin(pz) + 0.006 * std::cos(px);
        slab.A_start.z[static_cast<std::size_t>(i)] =
            0.011 * std::sin(px) - 0.005 * std::cos(py);
        slab.A_end.x[static_cast<std::size_t>(i)] =
            slab.A_start.x[static_cast<std::size_t>(i)]
            + 0.003 * std::cos(px + py);
        slab.A_end.y[static_cast<std::size_t>(i)] =
            slab.A_start.y[static_cast<std::size_t>(i)]
            - 0.002 * std::sin(py + pz);
        slab.A_end.z[static_cast<std::size_t>(i)] =
            slab.A_start.z[static_cast<std::size_t>(i)]
            + 0.004 * std::cos(pz + px);
        slab.Phi[static_cast<std::size_t>(i)] =
            0.017 * std::sin(px) + 0.007 * std::cos(py)
            - 0.003 * std::sin(pz);
      }
    }
  }
  return slab;
}

void make_gauge(std::vector<double>& chi_start,
                std::vector<double>& chi_end) {
  chi_start.assign(static_cast<std::size_t>(L * L * L), 0.0);
  chi_end = chi_start;
  constexpr double pi = 3.141592653589793238462643383279502884;
  ftd::eft::DualGaugePotentialSlab indexing(L, lambda_t);
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const int i = indexing.index(x, y, z);
        const double px = 2.0 * pi * x / L;
        const double py = 2.0 * pi * y / L;
        const double pz = 2.0 * pi * z / L;
        chi_start[static_cast<std::size_t>(i)] =
            0.031 * std::sin(px + py) + 0.011 * std::cos(pz);
        chi_end[static_cast<std::size_t>(i)] =
            -0.019 * std::cos(py + pz) + 0.023 * std::sin(px);
      }
    }
  }
}

void run_free_arm(const std::string& label, const ftd::Vec3& momentum) {
  const ftd::Vec3 start{5.35, 6.42, 7.31};
  const auto displacement = ftd::eft::free_displacement_from_momentum(
      momentum, rest_energy, c_speed, lambda_t);
  const ftd::Vec3 end = start + displacement;
  const ftd::eft::DualGaugePotentialSlab zero(L, lambda_t);
  const auto result = ftd::eft::evaluate_discrete_legendre_worldline(
      start, end, +1, rest_energy, c_speed, zero, coupling);
  worst_deposit_residual = std::max(
      worst_deposit_residual, result.deposited_action_residual);
  worst_dispersion_residual = std::max(
      worst_dispersion_residual, result.dispersion_residual);
  check(label, result.valid
      && max_difference(result.canonical_start, momentum) <= gate
      && max_difference(result.canonical_end, momentum) <= gate
      && max_difference(result.kinetic_start, momentum) <= gate
      && max_difference(result.kinetic_end, momentum) <= gate
      && result.deposited_action_residual <= gate
      && result.dispersion_residual <= gate);
}

}  // namespace

int main() {
  run_free_arm("free axial dispersion inverse", {0.12, 0.0, 0.0});
  run_free_arm("free diagonal dispersion inverse", {0.08, -0.06, 0.05});

  const ftd::Vec3 start{5.25, 6.35, 7.45};
  const ftd::Vec3 end{5.43, 6.28, 7.51};
  const auto slab = general_slab();
  std::vector<double> chi_start;
  std::vector<double> chi_end;
  make_gauge(chi_start, chi_end);
  const auto transformed_slab = ftd::eft::gauge_transform_slab(
      slab, chi_start, chi_end);

  for (int charge : {-1, +1}) {
    const auto base = ftd::eft::evaluate_discrete_legendre_worldline(
        start, end, charge, rest_energy, c_speed, slab, coupling);
    const auto transformed = ftd::eft::evaluate_discrete_legendre_worldline(
        start, end, charge, rest_energy, c_speed,
        transformed_slab, coupling);
    const double charge_coupling = coupling * charge;
    const ftd::Vec3 expected_start_shift =
        (transformed.connection_start - base.connection_start)
        * charge_coupling;
    const ftd::Vec3 expected_end_shift =
        (transformed.connection_end - base.connection_end)
        * charge_coupling;
    worst_canonical_covariance = std::max({
        worst_canonical_covariance,
        max_difference(transformed.canonical_start - base.canonical_start,
                       expected_start_shift),
        max_difference(transformed.canonical_end - base.canonical_end,
                       expected_end_shift)});
    worst_kinetic_invariance = std::max({
        worst_kinetic_invariance,
        max_difference(base.kinetic_start, transformed.kinetic_start),
        max_difference(base.kinetic_end, transformed.kinetic_end)});
    worst_deposit_residual = std::max({worst_deposit_residual,
        base.deposited_action_residual,
        transformed.deposited_action_residual});
    check(charge > 0 ? "+ gauge-covariant endpoint momenta"
                     : "- gauge-covariant endpoint momenta",
          base.valid && transformed.valid
          && worst_canonical_covariance <= gate
          && worst_kinetic_invariance <= gate
          && base.deposited_action_residual <= gate
          && transformed.deposited_action_residual <= gate);
  }

  const ftd::eft::DualGaugePotentialSlab zero(L, lambda_t);
  const auto pure_gauge = ftd::eft::gauge_transform_slab(
      zero, chi_start, chi_end);
  const auto free = ftd::eft::evaluate_discrete_legendre_worldline(
      start, end, +1, rest_energy, c_speed, zero, coupling);
  const auto pure = ftd::eft::evaluate_discrete_legendre_worldline(
      start, end, +1, rest_energy, c_speed, pure_gauge, coupling);
  worst_pure_gauge_residual = std::max(
      max_difference(free.kinetic_start, pure.kinetic_start),
      max_difference(free.kinetic_end, pure.kinetic_end));
  check("pure gauge reproduces free kinetic endpoint momenta",
        free.valid && pure.valid && worst_pure_gauge_residual <= gate);

  const auto positive = ftd::eft::evaluate_discrete_legendre_worldline(
      start, end, +1, rest_energy, c_speed, slab, coupling);
  const auto negative = ftd::eft::evaluate_discrete_legendre_worldline(
      start, end, -1, rest_energy, c_speed, slab, coupling);
  worst_symmetry_residual = std::max(
      max_component(positive.d1_interaction + negative.d1_interaction),
      max_component(positive.d2_interaction + negative.d2_interaction));
  check("polarity reversal reverses interaction endpoint derivatives",
        positive.valid && negative.valid
        && worst_symmetry_residual <= gate);

  const ftd::Vec3 shift{3.0, -2.0, 4.0};
  const auto translated = ftd::eft::evaluate_discrete_legendre_worldline(
      start + shift, end + shift, +1,
      rest_energy, c_speed, zero, coupling);
  worst_symmetry_residual = std::max({worst_symmetry_residual,
      max_difference(free.canonical_start, translated.canonical_start),
      max_difference(free.canonical_end, translated.canonical_end)});
  check("integer translation preserves free Legendre map",
        translated.valid && worst_symmetry_residual <= gate);

  const ftd::Vec3 rotated_start{start.y, start.z, start.x};
  const ftd::Vec3 rotated_end{end.y, end.z, end.x};
  const auto rotated = ftd::eft::evaluate_discrete_legendre_worldline(
      rotated_start, rotated_end, +1,
      rest_energy, c_speed, zero, coupling);
  const ftd::Vec3 expected_rotated{
      free.canonical_start.y,
      free.canonical_start.z,
      free.canonical_start.x};
  worst_symmetry_residual = std::max(
      worst_symmetry_residual,
      max_difference(rotated.canonical_start, expected_rotated));
  check("proper cubic rotation rotates free Legendre map",
        rotated.valid && worst_symmetry_residual <= gate);

  check("cross-cell segment fails closed",
        !ftd::eft::evaluate_discrete_legendre_worldline(
            {5.9, 6.3, 7.4}, {6.1, 6.3, 7.4}, +1,
            rest_energy, c_speed, zero, coupling).valid);
  check("noncausal segment fails closed",
        !ftd::eft::evaluate_discrete_legendre_worldline(
            {5.1, 6.3, 7.4}, {5.8, 6.3, 7.4}, +1,
            rest_energy, c_speed, zero, coupling).valid);
  check("zero charge fails closed",
        !ftd::eft::evaluate_discrete_legendre_worldline(
            start, end, 0, rest_energy, c_speed, zero, coupling).valid);
  auto invalid = zero;
  invalid.Phi[0] = std::numeric_limits<double>::quiet_NaN();
  check("nonfinite slab fails closed",
        !ftd::eft::evaluate_discrete_legendre_worldline(
            start, end, +1, rest_energy, c_speed, invalid, coupling).valid);

  std::cout.precision(17);
  std::cout << "worst_deposited_action_residual="
            << worst_deposit_residual << '\n'
            << "worst_dispersion_residual="
            << worst_dispersion_residual << '\n'
            << "worst_canonical_covariance_residual="
            << worst_canonical_covariance << '\n'
            << "worst_kinetic_invariance_residual="
            << worst_kinetic_invariance << '\n'
            << "worst_pure_gauge_residual="
            << worst_pure_gauge_residual << '\n'
            << "worst_symmetry_residual="
            << worst_symmetry_residual << '\n'
            << "discrete_legendre_worldline failures=" << failures << '\n'
            << "verdict=INTERIOR_DISCRETE_LEGENDRE_GAUGE_COVARIANT\n";
  return failures == 0 ? 0 : 1;
}
