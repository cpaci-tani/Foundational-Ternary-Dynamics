/** FTD-0492: centered weak trace and branch-action discriminator. */

#include "ftd/eft/centered_knot_trace.h"
#include "ftd/eft/discrete_legendre_worldline.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {

constexpr int L = 17;
constexpr ftd::Coord knot{8, 8, 8};
constexpr double c_speed = 0.57735026918962576451;
constexpr double rest_energy = 0.511;
constexpr double coupling = 0.73;
constexpr double gate = 1e-12;
int failures = 0;
double worst_trace_residual = 0.0;
double worst_covariance_residual = 0.0;
double worst_gauss_residual = 0.0;
double worst_branch_formula_residual = 0.0;
double worst_gauge_residual = 0.0;
double measured_branch_mismatch = 0.0;

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

ftd::eft::MatchedFaceFlux local_field(
    ftd::Coord site, const ftd::Vec3& incoming,
    const ftd::Vec3& outgoing) {
  ftd::eft::MatchedFaceFlux field(L);
  const int i = field.index(site.x, site.y, site.z);
  const int ix = field.index(site.x - 1, site.y, site.z);
  const int iy = field.index(site.x, site.y - 1, site.z);
  const int iz = field.index(site.x, site.y, site.z - 1);
  field.x[static_cast<std::size_t>(i)] = outgoing.x;
  field.y[static_cast<std::size_t>(i)] = outgoing.y;
  field.z[static_cast<std::size_t>(i)] = outgoing.z;
  field.x[static_cast<std::size_t>(ix)] = incoming.x;
  field.y[static_cast<std::size_t>(iy)] = incoming.y;
  field.z[static_cast<std::size_t>(iz)] = incoming.z;
  return field;
}

ftd::eft::DualGaugePotentialSlab self_bias_slab(
    int polarity, const ftd::Vec3& bias) {
  ftd::eft::DualGaugePotentialSlab slab(L, c_speed);
  const double self = static_cast<double>(polarity) / 6.0;
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const int i = slab.index(x, y, z);
        const double ex = bias.x + (x == knot.x ? self
            : (x == knot.x - 1 ? -self : 0.0));
        const double ey = bias.y + (y == knot.y ? self
            : (y == knot.y - 1 ? -self : 0.0));
        const double ez = bias.z + (z == knot.z ? self
            : (z == knot.z - 1 ? -self : 0.0));
        slab.A_end.x[static_cast<std::size_t>(i)] = -c_speed * ex;
        slab.A_end.y[static_cast<std::size_t>(i)] = -c_speed * ey;
        slab.A_end.z[static_cast<std::size_t>(i)] = -c_speed * ez;
      }
    }
  }
  return slab;
}

void make_gauge(const ftd::eft::DualGaugePotentialSlab& indexing,
                std::vector<double>& chi_start,
                std::vector<double>& chi_end) {
  chi_start.assign(static_cast<std::size_t>(L * L * L), 0.0);
  chi_end = chi_start;
  constexpr double pi = 3.141592653589793238462643383279502884;
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

int sign_of(double value) { return value > 0.0 ? +1 : -1; }

}  // namespace

int main() {
  const ftd::Vec3 incoming{-0.31, 0.27, -0.19};
  const ftd::Vec3 outgoing{0.47, -0.11, 0.53};
  const auto field = local_field(knot, incoming, outgoing);
  const auto trace = ftd::eft::evaluate_centered_knot_trace(field, knot);
  const ftd::Vec3 expected = (incoming + outgoing) * 0.5;
  worst_trace_residual = std::max({trace.incident_average_residual,
      trace.weight_sum_residual,
      max_difference(trace.centered, expected)});
  check("unique invariant weights are eight copies of 1/8",
        trace.valid && trace.weight_sum_residual == 0.0
        && std::all_of(trace.invariant_weights.begin(),
                       trace.invariant_weights.end(),
                       [](double weight) { return weight == 0.125; }));
  check("incident-cell average equals centered face trace",
        worst_trace_residual <= gate);

  const ftd::Coord shifted_site{11, 6, 12};
  const auto shifted = ftd::eft::evaluate_centered_knot_trace(
      local_field(shifted_site, incoming, outgoing), shifted_site);
  worst_covariance_residual = max_difference(
      trace.centered, shifted.centered);
  check("integer translation preserves centered trace",
        shifted.valid && worst_covariance_residual <= gate);

  const ftd::Vec3 rotated_in{incoming.y, incoming.z, incoming.x};
  const ftd::Vec3 rotated_out{outgoing.y, outgoing.z, outgoing.x};
  const ftd::Coord rotated_site{knot.y, knot.z, knot.x};
  const auto rotated = ftd::eft::evaluate_centered_knot_trace(
      local_field(rotated_site, rotated_in, rotated_out), rotated_site);
  const ftd::Vec3 expected_rotated{
      trace.centered.y, trace.centered.z, trace.centered.x};
  worst_covariance_residual = std::max(
      worst_covariance_residual,
      max_difference(rotated.centered, expected_rotated));
  check("cyclic cubic rotation rotates centered trace",
        rotated.valid && worst_covariance_residual <= gate);

  // Reflect x through the knot: polar x component changes sign and the two
  // x-directed face traces exchange; y/z polar components retain sign.
  const ftd::Vec3 reflected_in{-outgoing.x, incoming.y, incoming.z};
  const ftd::Vec3 reflected_out{-incoming.x, outgoing.y, outgoing.z};
  const auto reflected = ftd::eft::evaluate_centered_knot_trace(
      local_field(knot, reflected_in, reflected_out), knot);
  const ftd::Vec3 expected_reflected{
      -trace.centered.x, trace.centered.y, trace.centered.z};
  worst_covariance_residual = std::max(
      worst_covariance_residual,
      max_difference(reflected.centered, expected_reflected));
  check("coordinate reflection transforms centered polar trace",
        reflected.valid && worst_covariance_residual <= gate);

  const ftd::Vec3 bias{0.4, 0.5, 0.6};
  for (int polarity : {-1, +1}) {
    const auto slab = self_bias_slab(polarity, bias);
    const auto electric = ftd::eft::slab_electric_field(slab);
    const auto centered = ftd::eft::evaluate_centered_knot_trace(
        electric, knot);
    worst_gauss_residual = std::max(worst_gauss_residual,
        std::abs(centered.divergence - polarity));
    check(polarity > 0
              ? "+ self trace cancels and bias survives"
              : "- self trace cancels and bias survives",
          centered.valid
          && max_difference(centered.centered, bias) <= gate
          && std::abs(centered.divergence - polarity) <= gate);

    const ftd::Vec3 centered_momentum = bias
        * (coupling * polarity * c_speed / 2.0);
    const auto displacement = ftd::eft::free_displacement_from_momentum(
        centered_momentum, rest_energy, c_speed, c_speed);
    const ftd::Coord sign{
        sign_of(displacement.x),
        sign_of(displacement.y),
        sign_of(displacement.z)};
    constexpr double epsilon = 1e-8;
    const ftd::Vec3 start{
        knot.x + epsilon * sign.x,
        knot.y + epsilon * sign.y,
        knot.z + epsilon * sign.z};
    const ftd::Vec3 end = start + displacement;
    const auto ordinary = ftd::eft::evaluate_discrete_legendre_worldline(
        start, end, polarity, rest_energy,
        c_speed, slab, coupling);
    const ftd::Vec3 branch_residual = ordinary.kinetic_start;
    const ftd::Vec3 expected_branch_residual{
        -coupling * c_speed * sign.x / 12.0,
        -coupling * c_speed * sign.y / 12.0,
        -coupling * c_speed * sign.z / 12.0};
    std::cout << "  INFO  polarity=" << polarity
              << " centered_p=(" << centered_momentum.x << ','
              << centered_momentum.y << ',' << centered_momentum.z << ')'
              << " ordinary_P0=(" << ordinary.kinetic_start.x << ','
              << ordinary.kinetic_start.y << ','
              << ordinary.kinetic_start.z << ')'
              << " residual=(" << branch_residual.x << ','
              << branch_residual.y << ',' << branch_residual.z << ")\n";
    measured_branch_mismatch = std::max(
        measured_branch_mismatch, max_component(branch_residual));
    worst_branch_formula_residual = std::max(
        worst_branch_formula_residual,
        max_difference(branch_residual, expected_branch_residual));

    std::vector<double> chi_start;
    std::vector<double> chi_end;
    make_gauge(slab, chi_start, chi_end);
    const auto gauged_slab = ftd::eft::gauge_transform_slab(
        slab, chi_start, chi_end);
    const auto gauged = ftd::eft::evaluate_discrete_legendre_worldline(
        start, end, polarity, rest_energy,
        c_speed, gauged_slab, coupling);
    worst_gauge_residual = std::max(
        worst_gauge_residual,
        max_difference(ordinary.kinetic_start, gauged.kinetic_start));
    check(polarity > 0
              ? "+ centered endpoint is not ordinary branch derivative"
              : "- centered endpoint is not ordinary branch derivative",
          ordinary.valid && gauged.valid
          && measured_branch_mismatch > gate
          && worst_branch_formula_residual <= gate
          && worst_gauge_residual <= gate);
  }

  check("invalid field fails closed",
        !ftd::eft::evaluate_centered_knot_trace(
            ftd::eft::MatchedFaceFlux(), knot).valid);

  std::cout.precision(17);
  std::cout << "worst_trace_residual=" << worst_trace_residual << '\n'
            << "worst_covariance_residual="
            << worst_covariance_residual << '\n'
            << "worst_gauss_residual=" << worst_gauss_residual << '\n'
            << "measured_branch_mismatch="
            << measured_branch_mismatch << '\n'
            << "expected_branch_mismatch="
            << coupling * c_speed / 12.0 << '\n'
            << "worst_branch_formula_residual="
            << worst_branch_formula_residual << '\n'
            << "worst_gauge_residual=" << worst_gauge_residual << '\n'
            << "centered_knot_trace failures=" << failures << '\n'
            << "verdict=CENTERED_TRACE_UNIQUE_LOCAL_BUT_NOT_BRANCH_ACTION_DERIVATIVE\n";
  return failures == 0 ? 0 : 1;
}
