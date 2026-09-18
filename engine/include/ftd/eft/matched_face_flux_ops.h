#pragma once
/**
 * @file matched_face_flux_ops.h
 * @brief Shared elementwise operations on the matched face flux container.
 *
 * The matched-face transaction observers each need the same three elementwise
 * primitives on MatchedFaceFlux: a scaled accumulate, a long-double inner
 * product, and a sup-norm difference.  The definitions are collected here
 * verbatim so the observers share one copy.  Arithmetic, accumulation order,
 * and the shape-mismatch sentinels (NaN for dot, infinity for max_difference)
 * are unchanged from the per-translation-unit copies they replace.
 *
 * The helpers live in their own nested namespace on purpose.  Other observers
 * in ftd::eft carry same-named elementwise helpers whose accumulation order and
 * mismatch handling differ from these, so promoting these names straight into
 * ftd::eft would make two different functions share one signature.  A consumer
 * opts in with `using namespace ftd::eft::matched_face_flux_ops;`.
 */

#include "ftd/eft/matched_gauss_transport.h"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace ftd::eft::matched_face_flux_ops {

/// target <- target + scale * value, elementwise over all three components.
inline void add_scaled(MatchedFaceFlux& target,
                       const MatchedFaceFlux& value,
                       double scale) {
  for (std::size_t i = 0; i < target.x.size(); ++i) {
    target.x[i] += scale * value.x[i];
    target.y[i] += scale * value.y[i];
    target.z[i] += scale * value.z[i];
  }
}

/// Long-double accumulated inner product.  NaN on a shape mismatch.
inline long double dot(const MatchedFaceFlux& lhs,
                       const MatchedFaceFlux& rhs) {
  if (lhs.L != rhs.L || lhs.x.size() != rhs.x.size()) return NAN;
  long double result = 0.0L;
  for (std::size_t i = 0; i < lhs.x.size(); ++i) {
    result += static_cast<long double>(lhs.x[i]) * rhs.x[i]
        + static_cast<long double>(lhs.y[i]) * rhs.y[i]
        + static_cast<long double>(lhs.z[i]) * rhs.z[i];
  }
  return result;
}

/// Elementwise sup-norm difference.  Infinity on a shape mismatch.
inline double max_difference(const MatchedFaceFlux& lhs,
                             const MatchedFaceFlux& rhs) {
  if (lhs.L != rhs.L || lhs.x.size() != rhs.x.size()) return INFINITY;
  double residual = 0.0;
  for (std::size_t i = 0; i < lhs.x.size(); ++i) {
    residual = std::max({residual,
        std::abs(lhs.x[i] - rhs.x[i]),
        std::abs(lhs.y[i] - rhs.y[i]),
        std::abs(lhs.z[i] - rhs.z[i])});
  }
  return residual;
}

}  // namespace ftd::eft::matched_face_flux_ops
