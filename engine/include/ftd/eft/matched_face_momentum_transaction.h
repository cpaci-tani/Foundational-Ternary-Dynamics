#pragma once
/**
 * @file matched_face_momentum_transaction.h
 * @brief Observer-only local translation pseudomomentum for matched fields.
 *
 * For the source-free staggered map
 *
 *   B' = B-lambda C^T E,
 *   E' = E+lambda C B',
 *
 * periodic central translation D_i commutes with C and is skew-adjoint.  The
 * local quadratic quantity
 *
 *   P_i(E,B) = <E,D_i C B>
 *
 * is therefore conserved exactly.  A conservative current update E<-E-K
 * changes it by
 *
 *   Delta P_i = -<K,D_i C B'>.
 *
 * This is a selected minimal local pseudomomentum, not a unique continuum
 * momentum or a production particle-recoil law.
 */

#include "ftd/eft/matched_face_energy_transaction.h"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace ftd::eft {

struct MatchedFaceMomentumTransaction {
  bool valid = false;
  MatchedFaceEnergyTransaction energy;
  Vec3 momentum_before{};
  Vec3 momentum_after_source_free{};
  Vec3 momentum_after{};
  Vec3 source_free_residual{};
  Vec3 field_momentum_change{};
  Vec3 predicted_field_change{};
  Vec3 required_matter_impulse{};
  double formula_residual = 0.0;
};

inline MatchedFaceFlux matched_central_derivative(
    const MatchedFaceFlux& field, int axis) {
  MatchedFaceFlux derivative(field.L);
  if (field.L <= 0 || axis < 0 || axis > 2) return derivative;
  for (int x = 0; x < field.L; ++x) {
    for (int y = 0; y < field.L; ++y) {
      for (int z = 0; z < field.L; ++z) {
        int xp = x, yp = y, zp = z;
        int xm = x, ym = y, zm = z;
        if (axis == 0) { ++xp; --xm; }
        if (axis == 1) { ++yp; --ym; }
        if (axis == 2) { ++zp; --zm; }
        const int i = field.index(x, y, z);
        const int plus = field.index(xp, yp, zp);
        const int minus = field.index(xm, ym, zm);
        derivative.x[static_cast<std::size_t>(i)] = 0.5
            * (field.x[static_cast<std::size_t>(plus)]
               - field.x[static_cast<std::size_t>(minus)]);
        derivative.y[static_cast<std::size_t>(i)] = 0.5
            * (field.y[static_cast<std::size_t>(plus)]
               - field.y[static_cast<std::size_t>(minus)]);
        derivative.z[static_cast<std::size_t>(i)] = 0.5
            * (field.z[static_cast<std::size_t>(plus)]
               - field.z[static_cast<std::size_t>(minus)]);
      }
    }
  }
  return derivative;
}

inline Vec3 matched_local_translation_momentum(
    const MatchedFaceFlux& electric,
    const MatchedEdgeField& magnetic_half) {
  if (electric.L <= 0 || electric.L != magnetic_half.L) return {};
  const auto magnetic_curl = matched_curl(magnetic_half);
  Vec3 momentum{};
  for (int axis = 0; axis < 3; ++axis) {
    const auto derivative = matched_central_derivative(
        magnetic_curl, axis);
    const double value = static_cast<double>(
        matched_face_dot(electric, derivative));
    if (axis == 0) momentum.x = value;
    if (axis == 1) momentum.y = value;
    if (axis == 2) momentum.z = value;
  }
  return momentum;
}

inline MatchedFaceMomentumTransaction
measure_matched_face_momentum_transaction(
    const MatchedFaceFlux& electric_before,
    const MatchedEdgeField& magnetic_before,
    const DualCellContinuity& history,
    double wave_speed,
    double dt = 1.0,
    double tolerance = 1e-12) {
  MatchedFaceMomentumTransaction result;
  result.energy = measure_matched_face_energy_transaction(
      electric_before, magnetic_before, history, wave_speed, dt, tolerance);
  if (!result.energy.valid) return result;

  const auto current = matched_current_field(history);
  MatchedFaceFlux electric_after_source_free = result.energy.electric_after;
  for (std::size_t index = 0; index < current.x.size(); ++index) {
    electric_after_source_free.x[index] += current.x[index];
    electric_after_source_free.y[index] += current.y[index];
    electric_after_source_free.z[index] += current.z[index];
  }

  result.momentum_before = matched_local_translation_momentum(
      electric_before, magnetic_before);
  result.momentum_after_source_free = matched_local_translation_momentum(
      electric_after_source_free, result.energy.magnetic_after);
  result.momentum_after = matched_local_translation_momentum(
      result.energy.electric_after, result.energy.magnetic_after);
  result.source_free_residual = result.momentum_after_source_free
      - result.momentum_before;
  result.field_momentum_change = result.momentum_after
      - result.momentum_before;

  const auto magnetic_curl = matched_curl(result.energy.magnetic_after);
  for (int axis = 0; axis < 3; ++axis) {
    const auto derivative = matched_central_derivative(
        magnetic_curl, axis);
    const double value = -static_cast<double>(
        matched_face_dot(current, derivative));
    if (axis == 0) result.predicted_field_change.x = value;
    if (axis == 1) result.predicted_field_change.y = value;
    if (axis == 2) result.predicted_field_change.z = value;
  }
  result.required_matter_impulse = result.field_momentum_change * -1.0;
  result.formula_residual = (result.field_momentum_change
      - result.predicted_field_change).mag();
  const auto finite_vec = [](const Vec3& value) {
    return std::isfinite(value.x) && std::isfinite(value.y)
        && std::isfinite(value.z);
  };
  result.valid = finite_vec(result.momentum_before)
      && finite_vec(result.momentum_after_source_free)
      && finite_vec(result.momentum_after)
      && finite_vec(result.predicted_field_change)
      && std::isfinite(result.formula_residual);
  return result;
}

}  // namespace ftd::eft
