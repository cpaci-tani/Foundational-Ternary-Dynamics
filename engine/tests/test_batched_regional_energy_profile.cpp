// FTD-0686: batched/scalar exact regional-energy equivalence.

#include "ftd/eft/batched_regional_energy_profile.h"
#include "ftd/eft/quadratic_coat_face_current.h"
#include "ftd/ontic.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {

constexpr int L = 17;
constexpr double gate = 2e-12;
int failures = 0;

void check(const std::string& label, bool condition) {
  if (condition) return;
  ++failures;
  std::cerr << "FAIL: " << label << '\n';
}

ftd::eft::MatchedFaceFlux electric_field() {
  ftd::eft::MatchedFaceFlux result(L);
  constexpr double pi = 3.14159265358979323846;
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z) {
        const auto i = static_cast<std::size_t>(result.index(x, y, z));
        result.x[i] = 0.031 * std::sin(2 * pi * (x + y) / L)
            + 0.004 * std::cos(2 * pi * z / L);
        result.y[i] = -0.027 * std::cos(2 * pi * (y + z) / L)
            + 0.003 * std::sin(2 * pi * x / L);
        result.z[i] = 0.023 * std::sin(2 * pi * (z + x) / L)
            - 0.005 * std::cos(2 * pi * y / L);
      }
  return result;
}

ftd::eft::MatchedEdgeField magnetic_field() {
  ftd::eft::MatchedEdgeField result(L);
  constexpr double pi = 3.14159265358979323846;
  for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
      for (int z = 0; z < L; ++z) {
        const auto i = static_cast<std::size_t>(result.index(x, y, z));
        result.x[i] = 0.019 * std::cos(2 * pi * (x - z) / L);
        result.y[i] = 0.013 * std::sin(2 * pi * (y - x) / L);
        result.z[i] = -0.015 * std::cos(2 * pi * (z - y) / L);
      }
  return result;
}

void source_free_step(const ftd::eft::MatchedFaceFlux& electric_before,
                      const ftd::eft::MatchedEdgeField& magnetic_before,
                      double lambda,
                      ftd::eft::MatchedFaceFlux& electric_pre,
                      ftd::eft::MatchedEdgeField& magnetic_after) {
  magnetic_after = magnetic_before;
  const auto ce = ftd::eft::matched_curl_adjoint(electric_before);
  for (std::size_t i = 0; i < magnetic_after.x.size(); ++i) {
    magnetic_after.x[i] -= lambda * ce.x[i];
    magnetic_after.y[i] -= lambda * ce.y[i];
    magnetic_after.z[i] -= lambda * ce.z[i];
  }
  electric_pre = electric_before;
  const auto cb = ftd::eft::matched_curl(magnetic_after);
  for (std::size_t i = 0; i < electric_pre.x.size(); ++i) {
    electric_pre.x[i] += lambda * cb.x[i];
    electric_pre.y[i] += lambda * cb.y[i];
    electric_pre.z[i] += lambda * cb.z[i];
  }
}

double scalar_difference(
    const ftd::eft::MatchedRegionalEnergyTransportResult& left,
    const ftd::eft::MatchedRegionalEnergyTransportResult& right) {
  return std::max({std::abs(left.energy_before - right.energy_before),
      std::abs(left.energy_pre_current - right.energy_pre_current),
      std::abs(left.energy_after - right.energy_after),
      std::abs(left.boundary_transport_into - right.boundary_transport_into),
      std::abs(left.source_exchange_into_field
               - right.source_exchange_into_field),
      std::abs(left.energy_change - right.energy_change),
      std::abs(left.magnetic_update_residual
               - right.magnetic_update_residual),
      std::abs(left.electric_pre_update_residual
               - right.electric_pre_update_residual),
      std::abs(left.global_source_free_residual
               - right.global_source_free_residual),
      std::abs(left.regional_ledger_residual
               - right.regional_ledger_residual)});
}

}  // namespace

int main() {
  const double lambda = ftd::C_SPEED;
  const auto electric_before = electric_field();
  const auto magnetic_before = magnetic_field();
  ftd::eft::MatchedFaceFlux electric_pre(L);
  ftd::eft::MatchedEdgeField magnetic_after(L);
  source_free_step(electric_before, magnetic_before, lambda,
                   electric_pre, magnetic_after);
  auto electric_after = electric_pre;
  const auto current = ftd::eft::make_quadratic_coat_face_current(
      L, {7.2, 8.15, 9.3}, {7.75, 7.55, 9.8}, +1);
  for (std::size_t i = 0; i < electric_after.x.size(); ++i) {
    electric_after.x[i] -= current.current_x[i];
    electric_after.y[i] -= current.current_y[i];
    electric_after.z[i] -= current.current_z[i];
  }
  const ftd::Vec3 center{7.0, 8.0, 9.0};
  const std::vector<int> radii{0, 2, 4, 8};
  const auto batch = ftd::eft::evaluate_batched_regional_energy_profile(
      electric_before, magnetic_before, electric_pre, magnetic_after,
      electric_after, lambda, center, radii, 1e-10);
  check("batch valid", batch.valid && batch.regions.size() == radii.size());
  double worst = 0.0;
  for (std::size_t index = 0; index < radii.size(); ++index) {
    const auto scalar = ftd::eft::evaluate_matched_regional_energy_transport(
        electric_before, magnetic_before, electric_pre, magnetic_after,
        electric_after, lambda, center, radii[index], 1e-10);
    check("scalar valid", scalar.valid);
    worst = std::max(worst, scalar_difference(batch.regions[index], scalar));
  }
  check("scalar equivalence", worst <= gate);

  const auto source_free = ftd::eft::evaluate_batched_regional_energy_profile(
      electric_before, magnetic_before, electric_pre, magnetic_after,
      electric_pre, lambda, center, radii, 1e-10);
  check("source-free valid", source_free.valid);
  for (const auto& region : source_free.regions)
    check("source-free exchange zero",
          std::abs(region.source_exchange_into_field) <= gate);

  check("unordered radii fail",
        !ftd::eft::evaluate_batched_regional_energy_profile(
            electric_before, magnetic_before, electric_pre, magnetic_after,
            electric_after, lambda, center, {4, 2}, 1e-10).valid);
  check("noninteger center fails",
        !ftd::eft::evaluate_batched_regional_energy_profile(
            electric_before, magnetic_before, electric_pre, magnetic_after,
            electric_after, lambda, {7.5, 8.0, 9.0}, radii, 1e-10).valid);
  auto nonfinite = electric_after;
  nonfinite.x[0] = std::numeric_limits<double>::quiet_NaN();
  check("nonfinite fails",
        !ftd::eft::evaluate_batched_regional_energy_profile(
            electric_before, magnetic_before, electric_pre, magnetic_after,
            nonfinite, lambda, center, radii, 1e-10).valid);

  std::cout.precision(17);
  std::cout << "worst_scalar_equivalence=" << worst
            << " batch_partition=" << batch.maximum_scalar_equivalence_residual
            << " failures=" << failures << '\n';
  return failures == 0 ? 0 : 1;
}
