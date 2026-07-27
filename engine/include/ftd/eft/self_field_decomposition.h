#pragma once
/**
 * @file self_field_decomposition.h
 * @brief Global matched Hodge observer for self-field locality (FTD-0488).
 */

#include "ftd/eft/matched_gauss_transport.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace ftd::eft {

struct MatchedSelfFieldDecomposition {
  bool valid = false;
  bool neutral = false;
  int L = 0;
  long long source_sum = 0;
  MatchedMinimumEnergyResult minimum{};
  MatchedFaceFlux longitudinal{};
  MatchedFaceFlux transverse{};
  double input_gauss_residual = 0.0;
  double transverse_divergence_residual = 0.0;
  double longitudinal_curl_residual = 0.0;
  double orthogonality_residual = 0.0;
  double energy_split_residual = 0.0;
  int longitudinal_support = 0;
};

inline long double matched_face_pairing(
    const MatchedFaceFlux& lhs, const MatchedFaceFlux& rhs) {
  if (lhs.L != rhs.L || lhs.x.size() != rhs.x.size()) return NAN;
  long double result = 0.0L;
  for (std::size_t i = 0; i < lhs.x.size(); ++i) {
    result += static_cast<long double>(lhs.x[i]) * rhs.x[i];
    result += static_cast<long double>(lhs.y[i]) * rhs.y[i];
    result += static_cast<long double>(lhs.z[i]) * rhs.z[i];
  }
  return result;
}

inline long double periodic_divergence_sum(const MatchedFaceFlux& field) {
  if (field.L <= 0) return NAN;
  long double result = 0.0L;
  for (int x = 0; x < field.L; ++x) {
    for (int y = 0; y < field.L; ++y) {
      for (int z = 0; z < field.L; ++z) {
        result += divergence_at(field, x, y, z);
      }
    }
  }
  return result;
}

inline MatchedSelfFieldDecomposition decompose_matched_self_field(
    const MatchedFaceFlux& total,
    const std::vector<int>& source,
    double tolerance = 1e-12,
    int max_iterations = 0) {
  MatchedSelfFieldDecomposition result;
  result.L = total.L;
  result.longitudinal = MatchedFaceFlux(total.L);
  result.transverse = MatchedFaceFlux(total.L);
  const std::size_t count = total.L > 0
      ? static_cast<std::size_t>(total.L * total.L * total.L) : 0;
  if (total.L <= 0 || source.size() != count
      || total.x.size() != count || total.y.size() != count
      || total.z.size() != count || !(tolerance > 0.0)
      || !std::isfinite(tolerance)) {
    return result;
  }
  const auto finite = [](const std::vector<double>& values) {
    return std::all_of(values.begin(), values.end(),
        [](double value) { return std::isfinite(value); });
  };
  if (!finite(total.x) || !finite(total.y) || !finite(total.z)) return result;
  for (int value : source) result.source_sum += value;
  result.neutral = result.source_sum == 0;

  MatchedGaussDynamics solver(total.L);
  result.minimum = solver.initialize_minimum_energy(
      source, tolerance, max_iterations);
  if (!result.minimum.valid) return result;
  result.longitudinal = solver.electric();
  for (std::size_t i = 0; i < count; ++i) {
    result.transverse.x[i] = total.x[i] - result.longitudinal.x[i];
    result.transverse.y[i] = total.y[i] - result.longitudinal.y[i];
    result.transverse.z[i] = total.z[i] - result.longitudinal.z[i];
  }
  result.input_gauss_residual = max_gauss_residual(total, source);
  result.transverse_divergence_residual = max_divergence(result.transverse);
  result.longitudinal_curl_residual = max_curl_adjoint(result.longitudinal);
  result.orthogonality_residual = std::abs(static_cast<double>(
      matched_face_pairing(result.longitudinal, result.transverse)));
  result.energy_split_residual = std::abs(
      quadratic_energy(total) - quadratic_energy(result.longitudinal)
      - quadratic_energy(result.transverse));
  for (std::size_t i = 0; i < count; ++i) {
    if (std::abs(result.longitudinal.x[i]) > tolerance) {
      ++result.longitudinal_support;
    }
    if (std::abs(result.longitudinal.y[i]) > tolerance) {
      ++result.longitudinal_support;
    }
    if (std::abs(result.longitudinal.z[i]) > tolerance) {
      ++result.longitudinal_support;
    }
  }
  result.valid = result.neutral && result.minimum.converged
      && std::isfinite(result.input_gauss_residual)
      && std::isfinite(result.transverse_divergence_residual)
      && std::isfinite(result.orthogonality_residual)
      && std::isfinite(result.energy_split_residual);
  return result;
}

}  // namespace ftd::eft
