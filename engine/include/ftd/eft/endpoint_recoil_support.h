#pragma once
/**
 * @file endpoint_recoil_support.h
 * @brief Exact endpoint splits of a longitudinal hop recoil (analysis only).
 */

#include "ftd/eft/cubic_hop_response.h"

namespace ftd::eft {

struct EndpointRecoil {
  CubicVector source{};
  CubicVector target{};
};

inline EndpointRecoil split_endpoint_recoil(const CubicVector& total,
                                             int source_numerator,
                                             int denominator) {
  EndpointRecoil result;
  if (denominator <= 0 || source_numerator < 0
      || source_numerator > denominator)
    return result;
  for (int axis = 0; axis < 3; ++axis) {
    const int component = total[static_cast<std::size_t>(axis)];
    result.source[static_cast<std::size_t>(axis)] =
        component * source_numerator / denominator;
    result.target[static_cast<std::size_t>(axis)] =
        component * (denominator - source_numerator) / denominator;
  }
  return result;
}

inline CubicVector total_endpoint_recoil(const EndpointRecoil& recoil) {
  return {{recoil.source[0] + recoil.target[0],
           recoil.source[1] + recoil.target[1],
           recoil.source[2] + recoil.target[2]}};
}

inline int endpoint_quadratic_norm(const EndpointRecoil& recoil) {
  return norm2(recoil.source) + norm2(recoil.target);
}

inline EndpointRecoil exchange_endpoints(const EndpointRecoil& recoil) {
  return {recoil.target, recoil.source};
}

inline EndpointRecoil apply_signed_permutation(
    const SignedPermutation& transform, const EndpointRecoil& recoil) {
  return {apply_signed_permutation(transform, recoil.source),
          apply_signed_permutation(transform, recoil.target)};
}

}  // namespace ftd::eft
