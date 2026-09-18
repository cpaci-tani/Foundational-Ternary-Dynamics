#pragma once
/**
 * @file compatibility_analysis_helpers.h
 * @brief Helpers shared by the contact-carrier compatibility, energy-
 *        obstruction and implicit-atomic-endpoint analyzers.
 *
 * These are the carrier-position, anchor/remainder, matched-field
 * linear-combination, residual and free-carrier-momentum helpers that
 * single_slab_connection_compatibility.cpp,
 * staggered_current_split_compatibility.cpp,
 * implicit_atomic_face_action.cpp, implicit_atomic_endpoint_solve.cpp and
 * matched_contact_energy_obstruction.cpp previously each carried as their
 * own anonymous-namespace copy.  The bodies here are the originals verbatim;
 * no formula, loop bound, or evaluation order changes.
 *
 * Observer-only: nothing here touches RenderBridge state.
 */

#include "ftd/constants.h"
#include "ftd/eft/matched_gauss_transport.h"
#include "ftd/eft/overshoot_preserving_contact_rebase.h"
#include "ftd/eft/symmetric_diagonal_coupled_endpoint.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace ftd::eft::detail {

/// Absolute sub-lattice position of a contact carrier (anchor + remainder).
inline Vec3 position(const ContactCarrierRecord& carrier) {
  return {carrier.anchor.x+carrier.remainder.x,
          carrier.anchor.y+carrier.remainder.y,
          carrier.anchor.z+carrier.remainder.z};
}

/// Inverse of position(): floor anchor plus the sub-lattice remainder.
inline void decompose(const Vec3& value, Coord& anchor, Vec3& remainder) {
  anchor = {static_cast<int>(std::floor(value.x)),
            static_cast<int>(std::floor(value.y)),
            static_cast<int>(std::floor(value.z))};
  remainder = {value.x-anchor.x, value.y-anchor.y, value.z-anchor.z};
}

/// target += value, componentwise over the matched face complex.
inline void add(MatchedFaceFlux& target, const MatchedFaceFlux& value) {
  for (std::size_t i = 0; i < target.x.size(); ++i) {
    target.x[i] += value.x[i];
    target.y[i] += value.y[i];
    target.z[i] += value.z[i];
  }
}

/// target += amount*value, componentwise over the matched face complex.
inline void add_scaled(MatchedFaceFlux& target,
                       const MatchedFaceFlux& value, double amount) {
  for (std::size_t i = 0; i < target.x.size(); ++i) {
    target.x[i] += amount*value.x[i];
    target.y[i] += amount*value.y[i];
    target.z[i] += amount*value.z[i];
  }
}

/// target += amount*value, componentwise over the matched edge complex.
inline void add_scaled(MatchedEdgeField& target,
                       const MatchedEdgeField& value, double amount) {
  for (std::size_t i = 0; i < target.x.size(); ++i) {
    target.x[i] += amount*value.x[i];
    target.y[i] += amount*value.y[i];
    target.z[i] += amount*value.z[i];
  }
}

/// target *= amount, componentwise over the matched face complex.
inline void scale(MatchedFaceFlux& target, double amount) {
  for (std::size_t i = 0; i < target.x.size(); ++i) {
    target.x[i] *= amount;
    target.y[i] *= amount;
    target.z[i] *= amount;
  }
}

/// target *= amount, componentwise over the matched edge complex.
inline void scale(MatchedEdgeField& target, double amount) {
  for (std::size_t i = 0; i < target.x.size(); ++i) {
    target.x[i] *= amount;
    target.y[i] *= amount;
    target.z[i] *= amount;
  }
}

/// Max-norm difference of two site vectors; INFINITY on a length mismatch.
inline double vector_residual(const std::vector<double>& lhs,
                              const std::vector<double>& rhs) {
  if (lhs.size() != rhs.size()) return INFINITY;
  double result = 0.0;
  for (std::size_t i = 0; i < lhs.size(); ++i)
    result = std::max(result, std::abs(lhs[i]-rhs[i]));
  return result;
}

/// Relativistic momentum of a free carrier at the given lattice speed.
inline double momentum_from_speed(double speed) {
  const double h = E_REST/std::sqrt(
      1.0-speed*speed/(C_SPEED*C_SPEED));
  return h*speed/(C_SPEED*C_SPEED);
}

/// Worst residual a coupled-endpoint solve hands down to its caller.
inline double inherited_residual(
    const SymmetricDiagonalCoupledEndpointResult& value) {
  return std::max({value.root_residual, value.continuity_residual,
      value.gauss_before_residual, value.gauss_after_residual,
      value.staggered_embedding_residual, value.field_work_residual,
      value.matter_work_residual, value.total_energy_residual,
      value.displacement_residual, value.causal_excess,
      value.inverse_residual});
}

}  // namespace ftd::eft::detail
