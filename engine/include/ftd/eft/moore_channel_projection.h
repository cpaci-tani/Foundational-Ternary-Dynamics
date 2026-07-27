#pragma once
/**
 * @file moore_channel_projection.h
 * @brief Exact 13-channel Moore-shell to three-vector projection.
 *
 * The 26 directed Moore displacements are represented as 13 unoriented
 * channels with signed amplitudes.  Projection to a site vector retains only
 * the first directional moment.  This analysis helper does not add channels
 * to the production ontology.
 */

#include <array>
#include <cstddef>

namespace ftd::eft {

using MooreDirection = std::array<int, 3>;
using MooreChannels = std::array<int, 13>;

inline constexpr std::array<MooreDirection, 13> kMooreChannelDirections{{
    {{1, 0, 0}}, {{0, 1, 0}}, {{0, 0, 1}},
    {{1, 1, 0}}, {{1, -1, 0}}, {{1, 0, 1}}, {{1, 0, -1}},
    {{0, 1, 1}}, {{0, 1, -1}},
    {{1, 1, 1}}, {{1, 1, -1}}, {{1, -1, 1}}, {{1, -1, -1}}}};

inline MooreDirection project_moore_channels(const MooreChannels& channels) {
  MooreDirection result{{0, 0, 0}};
  for (std::size_t channel = 0; channel < channels.size(); ++channel) {
    for (std::size_t axis = 0; axis < result.size(); ++axis)
      result[axis] += channels[channel]
          * kMooreChannelDirections[channel][axis];
  }
  return result;
}

inline int channel_quadratic_norm(const MooreChannels& channels) {
  int result = 0;
  for (const int value : channels) result += value * value;
  return result;
}

inline MooreChannels diagonal_face_kernel(int diagonal_channel) {
  MooreChannels kernel{};
  if (diagonal_channel < 3 || diagonal_channel >= 13) return kernel;
  kernel[static_cast<std::size_t>(diagonal_channel)] = 1;
  const auto& direction =
      kMooreChannelDirections[static_cast<std::size_t>(diagonal_channel)];
  kernel[0] = -direction[0];
  kernel[1] = -direction[1];
  kernel[2] = -direction[2];
  return kernel;
}

}  // namespace ftd::eft
