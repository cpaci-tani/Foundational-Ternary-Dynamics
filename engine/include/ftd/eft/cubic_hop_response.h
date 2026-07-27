#pragma once
/**
 * @file cubic_hop_response.h
 * @brief Cubic-covariant isolated-hop work response (analysis only).
 *
 * If the only local polar vector is a Moore displacement d and the response
 * is invariant under d's full cubic stabilizer, its direction is forced to
 * span(d).  Imposing F dot d = W then gives F = W d / |d|^2.
 */

#include <array>
#include <cstddef>

namespace ftd::eft {

using CubicVector = std::array<int, 3>;

struct SignedPermutation {
  std::array<int, 3> permutation{{0, 1, 2}};
  std::array<int, 3> signs{{1, 1, 1}};
};

inline CubicVector apply_signed_permutation(
    const SignedPermutation& transform, const CubicVector& value) {
  CubicVector output{};
  for (int axis = 0; axis < 3; ++axis) {
    output[static_cast<std::size_t>(axis)] =
        transform.signs[static_cast<std::size_t>(axis)]
        * value[static_cast<std::size_t>(
            transform.permutation[static_cast<std::size_t>(axis)])];
  }
  return output;
}

inline int norm2(const CubicVector& value) {
  return value[0] * value[0] + value[1] * value[1]
      + value[2] * value[2];
}

inline int dot(const CubicVector& a, const CubicVector& b) {
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

inline CubicVector integer_work_response(const CubicVector& displacement,
                                         int work_multiple_of_six) {
  CubicVector response{};
  const int length_squared = norm2(displacement);
  if (length_squared <= 0 || work_multiple_of_six % 6 != 0) return response;
  for (int axis = 0; axis < 3; ++axis)
    response[static_cast<std::size_t>(axis)] =
        work_multiple_of_six
        * displacement[static_cast<std::size_t>(axis)] / length_squared;
  return response;
}

}  // namespace ftd::eft
