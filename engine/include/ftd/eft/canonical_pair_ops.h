#pragma once

/**
 * @file canonical_pair_ops.h
 * @brief Shared elementary operations on ftd::eft::CanonicalCarrierPair.
 *
 * Canonical definitions of the finiteness predicate, the harmonic pair action
 * S = (q^2 + p^2)/2 evaluated through std::hypot, and the canonical phase
 * rotation used by the selected ftd::eft phase-reference witnesses.  Each body
 * is lifted verbatim from the witness translation units that previously held
 * private copies; the arithmetic, its operand order, and its edge-case
 * handling are unchanged.
 *
 * This header carries no physics of its own.  It introduces no constant, no
 * coupling, no cadence, no threshold, and no Voxel consumer; it only removes
 * duplicate spellings of expressions the witnesses already evaluated.
 */

#include "ftd/eft/phase_referenced_action_rail.h"

#include <cmath>

namespace ftd::eft {

/** True only when both canonical coordinates are finite. */
inline bool finite_pair(const CanonicalCarrierPair& pair) {
  return std::isfinite(pair.q) && std::isfinite(pair.p);
}

/** Harmonic action S = (q^2 + p^2)/2 of one canonical pair. */
inline double pair_action(const CanonicalCarrierPair& pair) {
  const double radius = std::hypot(pair.q, pair.p);
  return 0.5 * radius * radius;
}

/**
 * Canonical phase rotation by the supplied advance:
 *   q' =  cos(phase) q + sin(phase) p
 *   p' = -sin(phase) q + cos(phase) p
 * The map is symplectic with unit Jacobian determinant.
 */
inline CanonicalCarrierPair rotate_pair(
    const CanonicalCarrierPair& pair,
    double phase) {
  const double cosine = std::cos(phase);
  const double sine = std::sin(phase);
  return {
      cosine * pair.q + sine * pair.p,
      -sine * pair.q + cosine * pair.p,
  };
}

}  // namespace ftd::eft
