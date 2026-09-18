#pragma once
/**
 * @file quadratic_coat_common.h
 * @brief Shared periodic-index and worldline-break helpers for the quadratic
 *        coupling-coat observers.
 *
 * The quadratic-coat translation units (face current, spacetime action, matter
 * work, orbit gather, composite Peierls) each carried a verbatim copy of these
 * five helpers in their own anonymous namespace.  They are collected here
 * unchanged — same arithmetic, same evaluation order, same edge cases — so the
 * observers share one definition.  Header-only and `inline` so every consumer
 * keeps the previous internal-linkage inlining behaviour.
 *
 * Consumers pull these in with using-declarations placed inside their own
 * anonymous namespace, which keeps the file-local `finite` overloads for
 * `std::vector<double>`, `MatchedFaceFlux`, and `MatchedEdgeField` in the same
 * overload set as the `Vec3` overload declared here.
 */

#include "ftd/voxel.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <vector>

namespace ftd::eft::quadratic_coat_common {

/// Periodic wrap of a possibly negative lattice coordinate into [0, L).
inline int wrap(int value, int L) {
  const int remainder = value % L;
  return remainder < 0 ? remainder + L : remainder;
}

/// Row-major flat site index with periodic wrapping applied on every axis.
inline std::size_t flat_index(int L, int x, int y, int z) {
  const auto side = static_cast<std::size_t>(L);
  const auto wx = static_cast<std::size_t>(wrap(x, L));
  const auto wy = static_cast<std::size_t>(wrap(y, L));
  const auto wz = static_cast<std::size_t>(wrap(z, L));
  return (wx * side + wy) * side + wz;
}

/// Axis-indexed component accessor: 0 selects x, 1 selects y, otherwise z.
inline double component(const Vec3& value, int axis) {
  return axis == 0 ? value.x : (axis == 1 ? value.y : value.z);
}

inline bool finite(const Vec3& value) {
  return std::isfinite(value.x) && std::isfinite(value.y)
      && std::isfinite(value.z);
}

/// Parameters in (0, 1) at which the straight segment start -> end crosses a
/// half-integer plane on any axis, together with the two endpoints.  The result
/// is sorted and de-duplicated at 32 epsilon.
inline std::vector<double> half_integer_breaks(const Vec3& start,
                                               const Vec3& end) {
  std::vector<double> breaks{0.0, 1.0};
  for (int axis = 0; axis < 3; ++axis) {
    const double p0 = component(start, axis);
    const double delta = component(end, axis) - p0;
    if (delta == 0.0) continue;
    const double lower = std::min(p0, p0 + delta);
    const double upper = std::max(p0, p0 + delta);
    const int first = static_cast<int>(std::floor(lower)) - 2;
    const int last = static_cast<int>(std::ceil(upper)) + 2;
    for (int k = first; k <= last; ++k) {
      const double plane = static_cast<double>(k) + 0.5;
      const double t = (plane - p0) / delta;
      if (t > 0.0 && t < 1.0) breaks.push_back(t);
    }
  }
  std::sort(breaks.begin(), breaks.end());
  breaks.erase(std::unique(breaks.begin(), breaks.end(),
      [](double a, double b) {
        return std::abs(a - b)
            <= 32.0 * std::numeric_limits<double>::epsilon();
      }), breaks.end());
  return breaks;
}

}  // namespace ftd::eft::quadratic_coat_common
