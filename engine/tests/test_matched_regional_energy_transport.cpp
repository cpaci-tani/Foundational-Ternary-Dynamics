/** FTD-0671: exact regional matched-field energy transport identity. */

#include "ftd/eft/matched_regional_energy_transport.h"
#include "ftd/eft/quadratic_coat_face_current.h"
#include "ftd/ontic.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>

namespace {

constexpr int L = 17;
constexpr double gate = 1e-12;
constexpr double comparison_gate = 2e-12;
int failures = 0;

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
        result.x[i] = 0.031 * std::sin(px + py) + 0.004 * std::cos(pz);
        result.y[i] = -0.027 * std::cos(py + pz) + 0.003 * std::sin(px);
        result.z[i] = 0.023 * std::sin(pz + px) - 0.005 * std::cos(py);
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
        result.x[i] = 0.019 * std::cos(px - pz) + 0.002 * std::sin(py);
        result.y[i] = 0.013 * std::sin(py - px) - 0.003 * std::cos(pz);
        result.z[i] = -0.015 * std::cos(pz - py) + 0.004 * std::sin(px);
      }
    }
  }
  return result;
}

void source_free_step(const ftd::eft::MatchedFaceFlux& electric_before,
                      const ftd::eft::MatchedEdgeField& magnetic_before,
                      double lambda,
                      ftd::eft::MatchedFaceFlux& electric_pre_current,
                      ftd::eft::MatchedEdgeField& magnetic_after) {
  magnetic_after = magnetic_before;
  const auto electric_curl = ftd::eft::matched_curl_adjoint(electric_before);
  for (std::size_t i = 0; i < magnetic_after.x.size(); ++i) {
    magnetic_after.x[i] -= lambda * electric_curl.x[i];
    magnetic_after.y[i] -= lambda * electric_curl.y[i];
    magnetic_after.z[i] -= lambda * electric_curl.z[i];
  }
  electric_pre_current = electric_before;
  const auto magnetic_curl = ftd::eft::matched_curl(magnetic_after);
  for (std::size_t i = 0; i < electric_pre_current.x.size(); ++i) {
    electric_pre_current.x[i] += lambda * magnetic_curl.x[i];
    electric_pre_current.y[i] += lambda * magnetic_curl.y[i];
    electric_pre_current.z[i] += lambda * magnetic_curl.z[i];
  }
}

template <typename Field>
Field translated(const Field& field, int dx, int dy, int dz) {
  Field result(field.L);
  for (int x = 0; x < field.L; ++x) {
    for (int y = 0; y < field.L; ++y) {
      for (int z = 0; z < field.L; ++z) {
        const auto old_i = static_cast<std::size_t>(field.index(x, y, z));
        const auto new_i = static_cast<std::size_t>(result.index(
            x + dx, y + dy, z + dz));
        result.x[new_i] = field.x[old_i];
        result.y[new_i] = field.y[old_i];
        result.z[new_i] = field.z[old_i];
      }
    }
  }
  return result;
}

template <typename Field>
Field cyclic_rotated(const Field& field) {
  Field result(field.L);
  for (int x = 0; x < field.L; ++x) {
    for (int y = 0; y < field.L; ++y) {
      for (int z = 0; z < field.L; ++z) {
        const auto old_i = static_cast<std::size_t>(field.index(x, y, z));
        const auto new_i = static_cast<std::size_t>(result.index(y, z, x));
        result.x[new_i] = field.y[old_i];
        result.y[new_i] = field.z[old_i];
        result.z[new_i] = field.x[old_i];
      }
    }
  }
  return result;
}

double max_scalar_difference(
    const ftd::eft::MatchedRegionalEnergyTransportResult& lhs,
    const ftd::eft::MatchedRegionalEnergyTransportResult& rhs) {
  return std::max({
      std::abs(lhs.energy_before - rhs.energy_before),
      std::abs(lhs.energy_pre_current - rhs.energy_pre_current),
      std::abs(lhs.energy_after - rhs.energy_after),
      std::abs(lhs.boundary_transport_into - rhs.boundary_transport_into),
      std::abs(lhs.source_exchange_into_field
               - rhs.source_exchange_into_field),
      std::abs(lhs.energy_change - rhs.energy_change)});
}

}  // namespace

int main() {
  const double lambda = ftd::C_SPEED;
  const auto electric_before = make_electric();
  const auto magnetic_before = make_magnetic();
  ftd::eft::MatchedFaceFlux electric_pre_current(L);
  ftd::eft::MatchedEdgeField magnetic_after(L);
  source_free_step(electric_before, magnetic_before, lambda,
                   electric_pre_current, magnetic_after);

  const auto current = ftd::eft::make_quadratic_coat_face_current(
      L, {7.2, 8.15, 9.3}, {7.75, 7.55, 9.8}, +1);
  check("current valid", current.valid);
  auto electric_after = electric_pre_current;
  for (std::size_t i = 0; i < electric_after.x.size(); ++i) {
    electric_after.x[i] -= current.current_x[i];
    electric_after.y[i] -= current.current_y[i];
    electric_after.z[i] -= current.current_z[i];
  }

  const ftd::Vec3 center{7.5, 8.0, 9.0};
  double worst_residual = 0.0;
  for (double radius : {2.0, 4.0, 8.0, 9.0}) {
    const auto result = ftd::eft::evaluate_matched_regional_energy_transport(
        electric_before, magnetic_before, electric_pre_current,
        magnetic_after, electric_after, lambda, center, radius);
    worst_residual = std::max({worst_residual,
        result.magnetic_update_residual,
        result.electric_pre_update_residual,
        result.global_source_free_residual,
        result.partition_residual,
        result.regional_ledger_residual});
    check("regional identity radius " + std::to_string(radius),
          result.valid);
    if (radius == 9.0) {
      check("full-region boundary transport vanishes",
            std::abs(result.boundary_transport_into) <= gate);
    }
  }

  const auto reference = ftd::eft::evaluate_matched_regional_energy_transport(
      electric_before, magnetic_before, electric_pre_current,
      magnetic_after, electric_after, lambda, center, 4.0);
  const auto snapshot = ftd::eft::measure_matched_regional_energy(
      electric_after,magnetic_after,lambda,center,4.0);
  check("regional snapshot partition",
        snapshot.valid && snapshot.partition_residual <= gate
        && std::abs(snapshot.total_energy
                    -snapshot.inside_energy-snapshot.outside_energy) <= gate);
  check("regional snapshot agrees with transaction endpoint",
        std::abs(snapshot.inside_energy-reference.energy_after) <= gate);

  constexpr int dx = 2;
  constexpr int dy = -3;
  constexpr int dz = 1;
  const auto translated_result =
      ftd::eft::evaluate_matched_regional_energy_transport(
          translated(electric_before, dx, dy, dz),
          translated(magnetic_before, dx, dy, dz),
          translated(electric_pre_current, dx, dy, dz),
          translated(magnetic_after, dx, dy, dz),
          translated(electric_after, dx, dy, dz), lambda,
          {center.x + dx, center.y + dy, center.z + dz}, 4.0);
  const double translation_residual = max_scalar_difference(
      reference, translated_result);
  check("integer translation covariance",
        translated_result.valid && translation_residual <= comparison_gate);

  const auto rotated_result =
      ftd::eft::evaluate_matched_regional_energy_transport(
          cyclic_rotated(electric_before), cyclic_rotated(magnetic_before),
          cyclic_rotated(electric_pre_current),
          cyclic_rotated(magnetic_after), cyclic_rotated(electric_after),
          lambda, {center.y, center.z, center.x}, 4.0);
  const double rotation_residual = max_scalar_difference(
      reference, rotated_result);
  check("proper cubic covariance",
        rotated_result.valid && rotation_residual <= comparison_gate);

  const auto source_free = ftd::eft::evaluate_matched_regional_energy_transport(
      electric_before, magnetic_before, electric_pre_current,
      magnetic_after, electric_pre_current, lambda, center, 4.0);
  check("source-free source exchange vanishes",
        source_free.valid
        && std::abs(source_free.source_exchange_into_field) <= gate);

  auto broken_pre_current = electric_pre_current;
  broken_pre_current.x[0] += 1e-4;
  check("inconsistent source-free update fails closed",
        !ftd::eft::evaluate_matched_regional_energy_transport(
            electric_before, magnetic_before, broken_pre_current,
            magnetic_after, electric_after, lambda, center, 4.0).valid);

  auto nonfinite = electric_after;
  nonfinite.z[0] = std::numeric_limits<double>::quiet_NaN();
  check("nonfinite field fails closed",
        !ftd::eft::evaluate_matched_regional_energy_transport(
            electric_before, magnetic_before, electric_pre_current,
            magnetic_after, nonfinite, lambda, center, 4.0).valid);
  check("nonfinite center fails closed",
        !ftd::eft::evaluate_matched_regional_energy_transport(
            electric_before, magnetic_before, electric_pre_current,
            magnetic_after, electric_after, lambda,
            {std::numeric_limits<double>::infinity(), 0.0, 0.0}, 4.0).valid);

  std::cout.precision(17);
  std::cout << "worst_identity_residual=" << worst_residual << '\n'
            << "translation_residual=" << translation_residual << '\n'
            << "rotation_residual=" << rotation_residual << '\n'
            << "matched_regional_energy_transport failures="
            << failures << '\n';
  return failures == 0 ? 0 : 1;
}
