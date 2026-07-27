/** FTD-0544: exact matched midpoint field-energy identity. */

#include "ftd/eft/matched_midpoint_poynting.h"
#include "ftd/ontic.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace {

constexpr int L = 17;
constexpr double gate = 1e-12;
int failures = 0;
int arms = 0;
double worst_midpoint = 0.0;
double worst_update = 0.0;
double worst_adjoint = 0.0;
double worst_poynting = 0.0;
double worst_gauss = 0.0;

void check(const std::string& label, bool condition) {
  if (condition) return;
  ++failures;
  std::cerr << "FAIL: " << label << '\n';
}

ftd::eft::MatchedFaceFlux make_electric() {
  ftd::eft::MatchedFaceFlux result(L);
  constexpr double pi = 3.1415926535897932384626433832795;
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const auto i = static_cast<std::size_t>(result.index(x, y, z));
        const double px = 2.0 * pi * x / L;
        const double py = 2.0 * pi * y / L;
        const double pz = 2.0 * pi * z / L;
        result.x[i] = 0.031 * std::sin(px + py) + 0.017;
        result.y[i] = -0.027 * std::cos(py + pz) - 0.011;
        result.z[i] = 0.023 * std::sin(pz + px) + 0.007;
      }
    }
  }
  return result;
}

ftd::eft::MatchedEdgeField make_magnetic() {
  ftd::eft::MatchedEdgeField result(L);
  constexpr double pi = 3.1415926535897932384626433832795;
  for (int x = 0; x < L; ++x) {
    for (int y = 0; y < L; ++y) {
      for (int z = 0; z < L; ++z) {
        const auto i = static_cast<std::size_t>(result.index(x, y, z));
        const double px = 2.0 * pi * x / L;
        const double py = 2.0 * pi * y / L;
        const double pz = 2.0 * pi * z / L;
        result.x[i] = 0.019 * std::cos(px - pz);
        result.y[i] = 0.013 * std::sin(py - px);
        result.z[i] = -0.015 * std::cos(pz - py);
      }
    }
  }
  return result;
}

ftd::eft::MatchedMidpointPoyntingResult run(
    const std::string& label,
    const ftd::Vec3& start,
    const ftd::Vec3& end,
    int charge,
    const ftd::eft::MatchedFaceFlux& electric,
    const ftd::eft::MatchedEdgeField& magnetic) {
  ++arms;
  const auto current = ftd::eft::make_quadratic_coat_face_current(
      L, start, end, charge);
  const auto result = ftd::eft::evaluate_matched_midpoint_poynting(
      electric, magnetic, current, ftd::C_SPEED);
  worst_midpoint = std::max({worst_midpoint,
      result.electric_midpoint_residual,
      result.magnetic_midpoint_residual});
  worst_update = std::max({worst_update,
      result.ampere_residual, result.faraday_residual});
  worst_adjoint = std::max(worst_adjoint, result.adjoint_residual);
  worst_poynting = std::max(worst_poynting, result.poynting_residual);
  worst_gauss = std::max(worst_gauss, result.gauss_transport_residual);
  check(label, current.valid && result.valid
      && result.electric_midpoint_residual <= gate
      && result.magnetic_midpoint_residual <= gate
      && result.ampere_residual <= gate
      && result.faraday_residual <= gate
      && result.adjoint_residual <= gate
      && result.poynting_residual <= gate
      && result.gauss_transport_residual <= gate);
  return result;
}

}  // namespace

int main() {
  const auto electric = make_electric();
  const auto magnetic = make_magnetic();
  const std::vector<std::pair<ftd::Vec3, ftd::Vec3>> paths{
      {{5.2, 6.7, 7.4}, {5.2, 6.7, 7.4}},
      {{5.1, 6.2, 7.7}, {5.8, 6.2, 7.7}},
      {{5.2, 6.15, 7.3}, {5.75, 5.55, 7.3}},
      {{5.2, 6.7, 7.4}, {5.75, 7.1, 6.8}},
      {{5.8, 6.2, 7.7}, {6.25, 5.6, 8.2}},
      {{16.9, 8.25, 9.5}, {0.1, 8.25, 9.5}}};
  for (int charge : {-1, +1}) {
    for (std::size_t i = 0; i < paths.size(); ++i) {
      run(std::string(charge > 0 ? "+" : "-")
              + " arm " + std::to_string(i),
          paths[i].first, paths[i].second, charge,
          electric, magnetic);
    }
  }

  const auto positive = run("polarity + control",
      {4.2, 5.7, 6.4}, {4.75, 6.1, 5.8}, +1, electric, magnetic);
  const auto negative = run("polarity - control",
      {4.2, 5.7, 6.4}, {4.75, 6.1, 5.8}, -1, electric, magnetic);
  const double polarity = std::abs(
      (positive.field_energy_after - positive.field_energy_before)
      + (negative.field_energy_after - negative.field_energy_before));
  check("polarity work mirror", polarity <= gate);

  const auto reverse = run("reversal control",
      {4.75, 6.1, 5.8}, {4.2, 5.7, 6.4}, +1, electric, magnetic);
  const double reversal = std::abs(
      (positive.field_energy_after - positive.field_energy_before)
      + (reverse.field_energy_after - reverse.field_energy_before));
  check("reversal work mirror", reversal <= gate);

  auto bad_electric = electric;
  bad_electric.x[0] = std::numeric_limits<double>::quiet_NaN();
  const auto current = ftd::eft::make_quadratic_coat_face_current(
      L, {4.2, 5.7, 6.4}, {4.75, 6.1, 5.8}, +1);
  check("nonfinite field fails closed",
      !ftd::eft::evaluate_matched_midpoint_poynting(
          bad_electric, magnetic, current, ftd::C_SPEED).valid);
  check("zero duration fails closed",
      !ftd::eft::evaluate_matched_midpoint_poynting(
          electric, magnetic, current, 0.0).valid);
  const auto invalid_current = ftd::eft::make_quadratic_coat_face_current(
      L, {4.2, 5.7, 6.4}, {5.4, 6.1, 5.8}, +1);
  check("invalid current fails closed",
      !ftd::eft::evaluate_matched_midpoint_poynting(
          electric, magnetic, invalid_current, ftd::C_SPEED).valid);

  std::cout.precision(17);
  std::cout << "arms=" << arms << '\n'
            << "worst_midpoint_residual=" << worst_midpoint << '\n'
            << "worst_update_residual=" << worst_update << '\n'
            << "worst_adjoint_residual=" << worst_adjoint << '\n'
            << "worst_poynting_residual=" << worst_poynting << '\n'
            << "worst_gauss_transport_residual=" << worst_gauss << '\n'
            << "polarity_residual=" << polarity << '\n'
            << "reversal_residual=" << reversal << '\n'
            << "matched_midpoint_poynting failures=" << failures << '\n';
  return failures == 0 ? 0 : 1;
}
