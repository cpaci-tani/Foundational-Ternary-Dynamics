#pragma once
/**
 * @file matched_field_helpers.h
 * @brief Shared validity/reduction helpers for matched face/edge fields.
 *
 * These are the small, behaviour-identical predicates and reductions that the
 * matched-complex measurement translation units each used to carry as a
 * private copy.  They are gathered here verbatim so the definitions cannot
 * drift apart.  Nothing here allocates, mutates, or depends on engine state.
 *
 * The helpers live in their own namespace rather than ftd::eft::detail:
 * matched_poisson.h already declares a *different* ftd::eft::detail::dot()
 * over std::vector<double> that accumulates in double, whereas the reduction
 * here deliberately accumulates in long double.
 */

#include "ftd/eft/matched_gauss_transport.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace ftd::eft::matched_field_helpers {

inline bool finite(const std::vector<double>& values) {
  return std::all_of(values.begin(), values.end(),
      [](double value) { return std::isfinite(value); });
}

inline bool finite(const MatchedFaceFlux& field) {
  return finite(field.x) && finite(field.y) && finite(field.z);
}

inline bool finite(const MatchedEdgeField& field) {
  return finite(field.x) && finite(field.y) && finite(field.z);
}

inline bool valid_size(const MatchedFaceFlux& field, int L) {
  const auto count = static_cast<std::size_t>(L) * L * L;
  return field.L == L && field.x.size() == count
      && field.y.size() == count && field.z.size() == count;
}

inline bool valid_size(const MatchedEdgeField& field, int L) {
  const auto count = static_cast<std::size_t>(L) * L * L;
  return field.L == L && field.x.size() == count
      && field.y.size() == count && field.z.size() == count;
}

inline long double dot(const std::vector<double>& lhs,
                       const std::vector<double>& rhs) {
  long double result = 0.0L;
  for (std::size_t i = 0; i < lhs.size(); ++i)
    result += static_cast<long double>(lhs[i]) * rhs[i];
  return result;
}

inline long double dot(const MatchedFaceFlux& lhs,
                       const MatchedFaceFlux& rhs) {
  return dot(lhs.x, rhs.x) + dot(lhs.y, rhs.y) + dot(lhs.z, rhs.z);
}

inline long double dot(const MatchedEdgeField& lhs,
                       const MatchedEdgeField& rhs) {
  return dot(lhs.x, rhs.x) + dot(lhs.y, rhs.y) + dot(lhs.z, rhs.z);
}

inline double max_difference(const std::vector<double>& lhs,
                             const std::vector<double>& rhs) {
  double result = 0.0;
  for (std::size_t i = 0; i < lhs.size(); ++i)
    result = std::max(result, std::abs(lhs[i] - rhs[i]));
  return result;
}

inline double max_difference(const MatchedFaceFlux& lhs,
                             const MatchedFaceFlux& rhs) {
  return std::max({max_difference(lhs.x, rhs.x),
                   max_difference(lhs.y, rhs.y),
                   max_difference(lhs.z, rhs.z)});
}

inline double max_difference(const MatchedEdgeField& lhs,
                             const MatchedEdgeField& rhs) {
  return std::max({max_difference(lhs.x, rhs.x),
                   max_difference(lhs.y, rhs.y),
                   max_difference(lhs.z, rhs.z)});
}

}  // namespace ftd::eft::matched_field_helpers
