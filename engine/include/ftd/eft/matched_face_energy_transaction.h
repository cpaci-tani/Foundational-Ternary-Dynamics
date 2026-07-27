#pragma once
/**
 * @file matched_face_energy_transaction.h
 * @brief Observer-only finite-current work ledger for the matched face complex.
 *
 * The staggered Maxwell step stores B at a half tick and preserves
 *
 *   H~(E,B) = 1/2||E||^2 + 1/2||B||^2
 *              - (lambda/2)<B,C^T E>.
 *
 * A conservative transport history supplies a face current K and applies
 * E <- E-K.  Across the complete source-free-plus-current step, the exact
 * matter work is
 *
 *   W_matter = <K,(E_before+E_after)/2>,
 *
 * so Delta H~ + W_matter = 0.  This helper measures that identity and its
 * exact algebraic inverse without modifying RenderBridge or selecting a
 * route for a Moore hop.
 */

#include "ftd/eft/matched_gauss_transport.h"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace ftd::eft {

struct MatchedFaceEnergyTransaction {
  bool valid = false;
  int current_support = 0;
  double continuity_residual = 0.0;
  double gauss_before = 0.0;
  double gauss_after = 0.0;
  double energy_before = 0.0;
  double energy_after_source_free = 0.0;
  double energy_after = 0.0;
  double source_free_residual = 0.0;
  double midpoint_work = 0.0;
  double balance_residual = 0.0;
  double current_balance_residual = 0.0;
  double pre_current_work = 0.0;
  double naive_balance_residual = 0.0;
  double naive_formula_residual = 0.0;
  double inverse_residual = 0.0;
  MatchedFaceFlux electric_after;
  MatchedEdgeField magnetic_after;
};

inline long double matched_face_dot(const MatchedFaceFlux& a,
                                    const MatchedFaceFlux& b) {
  if (a.L != b.L || a.x.size() != b.x.size()) return NAN;
  long double value = 0.0L;
  for (std::size_t index = 0; index < a.x.size(); ++index) {
    value += static_cast<long double>(a.x[index]) * b.x[index];
    value += static_cast<long double>(a.y[index]) * b.y[index];
    value += static_cast<long double>(a.z[index]) * b.z[index];
  }
  return value;
}

inline long double matched_edge_dot(const MatchedEdgeField& a,
                                    const MatchedEdgeField& b) {
  if (a.L != b.L || a.x.size() != b.x.size()) return NAN;
  long double value = 0.0L;
  for (std::size_t index = 0; index < a.x.size(); ++index) {
    value += static_cast<long double>(a.x[index]) * b.x[index];
    value += static_cast<long double>(a.y[index]) * b.y[index];
    value += static_cast<long double>(a.z[index]) * b.z[index];
  }
  return value;
}

inline double matched_face_max_difference(const MatchedFaceFlux& a,
                                          const MatchedFaceFlux& b) {
  if (a.L != b.L || a.x.size() != b.x.size()) return INFINITY;
  double value = 0.0;
  for (std::size_t index = 0; index < a.x.size(); ++index) {
    value = std::max(value, std::abs(a.x[index] - b.x[index]));
    value = std::max(value, std::abs(a.y[index] - b.y[index]));
    value = std::max(value, std::abs(a.z[index] - b.z[index]));
  }
  return value;
}

inline double matched_edge_max_difference(const MatchedEdgeField& a,
                                          const MatchedEdgeField& b) {
  if (a.L != b.L || a.x.size() != b.x.size()) return INFINITY;
  double value = 0.0;
  for (std::size_t index = 0; index < a.x.size(); ++index) {
    value = std::max(value, std::abs(a.x[index] - b.x[index]));
    value = std::max(value, std::abs(a.y[index] - b.y[index]));
    value = std::max(value, std::abs(a.z[index] - b.z[index]));
  }
  return value;
}

inline MatchedFaceFlux matched_current_field(
    const DualCellContinuity& history) {
  MatchedFaceFlux current(history.L);
  if (history.current_x.size() != current.x.size()) return current;
  current.x = history.current_x;
  current.y = history.current_y;
  current.z = history.current_z;
  return current;
}

inline double matched_modified_energy(const MatchedFaceFlux& electric,
                                      const MatchedEdgeField& magnetic_half,
                                      double lambda) {
  if (electric.L != magnetic_half.L || !std::isfinite(lambda)) return NAN;
  const auto curl_adjoint = matched_curl_adjoint(electric);
  return quadratic_energy(electric) + quadratic_energy(magnetic_half)
      - 0.5 * lambda * static_cast<double>(
          matched_edge_dot(magnetic_half, curl_adjoint));
}

inline MatchedFaceEnergyTransaction measure_matched_face_energy_transaction(
    const MatchedFaceFlux& electric_before,
    const MatchedEdgeField& magnetic_before,
    const DualCellContinuity& history,
    double wave_speed,
    double dt = 1.0,
    double tolerance = 1e-12) {
  MatchedFaceEnergyTransaction result;
  result.electric_after = MatchedFaceFlux(electric_before.L);
  result.magnetic_after = MatchedEdgeField(magnetic_before.L);
  const double lambda = wave_speed * dt;
  if (electric_before.L <= 0 || electric_before.L != magnetic_before.L
      || electric_before.L != history.L || !std::isfinite(lambda)
      || lambda < 0.0 || total_reaction_l1(history) != 0) {
    return result;
  }

  result.continuity_residual = max_continuity_residual(history);
  if (result.continuity_residual > tolerance) return result;

  const MatchedFaceFlux current = matched_current_field(history);
  for (std::size_t index = 0; index < current.x.size(); ++index) {
    result.current_support += current.x[index] != 0.0 ? 1 : 0;
    result.current_support += current.y[index] != 0.0 ? 1 : 0;
    result.current_support += current.z[index] != 0.0 ? 1 : 0;
  }

  result.gauss_before = max_gauss_residual(
      electric_before, history.rho_before);
  result.energy_before = matched_modified_energy(
      electric_before, magnetic_before, lambda);

  result.magnetic_after = magnetic_before;
  const auto electric_curl = matched_curl_adjoint(electric_before);
  for (std::size_t index = 0; index < result.magnetic_after.x.size(); ++index) {
    result.magnetic_after.x[index] -= lambda * electric_curl.x[index];
    result.magnetic_after.y[index] -= lambda * electric_curl.y[index];
    result.magnetic_after.z[index] -= lambda * electric_curl.z[index];
  }

  MatchedFaceFlux electric_pre_current = electric_before;
  const auto magnetic_curl = matched_curl(result.magnetic_after);
  for (std::size_t index = 0; index < electric_pre_current.x.size(); ++index) {
    electric_pre_current.x[index] += lambda * magnetic_curl.x[index];
    electric_pre_current.y[index] += lambda * magnetic_curl.y[index];
    electric_pre_current.z[index] += lambda * magnetic_curl.z[index];
  }
  result.energy_after_source_free = matched_modified_energy(
      electric_pre_current, result.magnetic_after, lambda);
  result.source_free_residual = result.energy_after_source_free
      - result.energy_before;

  result.electric_after = electric_pre_current;
  const auto transport = apply_conservative_current(
      result.electric_after, history, tolerance);
  if (!transport.valid) return result;

  result.energy_after = matched_modified_energy(
      result.electric_after, result.magnetic_after, lambda);
  long double midpoint_work = 0.0L;
  long double pre_current_work = 0.0L;
  for (std::size_t index = 0; index < current.x.size(); ++index) {
    midpoint_work += static_cast<long double>(current.x[index])
        * 0.5L * (electric_before.x[index]
                  + result.electric_after.x[index]);
    midpoint_work += static_cast<long double>(current.y[index])
        * 0.5L * (electric_before.y[index]
                  + result.electric_after.y[index]);
    midpoint_work += static_cast<long double>(current.z[index])
        * 0.5L * (electric_before.z[index]
                  + result.electric_after.z[index]);
    pre_current_work += static_cast<long double>(current.x[index])
        * electric_pre_current.x[index];
    pre_current_work += static_cast<long double>(current.y[index])
        * electric_pre_current.y[index];
    pre_current_work += static_cast<long double>(current.z[index])
        * electric_pre_current.z[index];
  }
  result.midpoint_work = static_cast<double>(midpoint_work);
  result.pre_current_work = static_cast<double>(pre_current_work);
  result.balance_residual = result.energy_after - result.energy_before
      + result.midpoint_work;
  result.current_balance_residual = result.energy_after
      - result.energy_after_source_free + result.midpoint_work;
  result.naive_balance_residual = result.energy_after
      - result.energy_before + result.pre_current_work;
  const double expected_naive = 0.5 * static_cast<double>(
      matched_face_dot(current, current))
      + 0.5 * lambda * static_cast<double>(
          matched_face_dot(current, magnetic_curl));
  result.naive_formula_residual = result.naive_balance_residual
      - expected_naive;

  MatchedFaceFlux recovered_pre_current = result.electric_after;
  for (std::size_t index = 0; index < current.x.size(); ++index) {
    recovered_pre_current.x[index] += current.x[index];
    recovered_pre_current.y[index] += current.y[index];
    recovered_pre_current.z[index] += current.z[index];
  }
  MatchedFaceFlux recovered_electric = recovered_pre_current;
  for (std::size_t index = 0; index < current.x.size(); ++index) {
    recovered_electric.x[index] -= lambda * magnetic_curl.x[index];
    recovered_electric.y[index] -= lambda * magnetic_curl.y[index];
    recovered_electric.z[index] -= lambda * magnetic_curl.z[index];
  }
  MatchedEdgeField recovered_magnetic = result.magnetic_after;
  const auto recovered_curl = matched_curl_adjoint(recovered_electric);
  for (std::size_t index = 0; index < recovered_magnetic.x.size(); ++index) {
    recovered_magnetic.x[index] += lambda * recovered_curl.x[index];
    recovered_magnetic.y[index] += lambda * recovered_curl.y[index];
    recovered_magnetic.z[index] += lambda * recovered_curl.z[index];
  }
  result.inverse_residual = std::max(
      matched_face_max_difference(electric_before, recovered_electric),
      matched_edge_max_difference(magnetic_before, recovered_magnetic));
  result.gauss_after = max_gauss_residual(
      result.electric_after, history.rho_after);
  result.valid = std::isfinite(result.energy_before)
      && std::isfinite(result.energy_after)
      && std::isfinite(result.midpoint_work)
      && std::isfinite(result.balance_residual)
      && std::isfinite(result.inverse_residual);
  return result;
}

}  // namespace ftd::eft
