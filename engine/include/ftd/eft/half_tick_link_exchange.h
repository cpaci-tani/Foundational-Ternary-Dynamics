#pragma once
/**
 * @file half_tick_link_exchange.h
 * @brief Selected reversible exchange ledger on an oriented Moore link.
 *
 * This analysis-only object records a particle momentum/energy exchange and
 * its equal-and-opposite field ledger at n+1/2.  It is not a field
 * Hamiltonian and does not update production J/W.
 */

#include "ftd/eft/cubic_hop_response.h"
#include "ftd/eft/moore_channel_projection.h"
#include "ftd/eft/production_hop_kinematics.h"

namespace ftd::eft {

struct MooreChannelRef {
  int index = -1;
  int orientation = 0;
  bool valid = false;
};

inline MooreChannelRef moore_channel_ref(
    const CubicVector& displacement) {
  for (int channel = 0; channel < 13; ++channel) {
    const auto& direction = kMooreChannelDirections[
        static_cast<std::size_t>(channel)];
    bool forward = true;
    bool reverse = true;
    for (int axis = 0; axis < 3; ++axis) {
      forward = forward
          && displacement[static_cast<std::size_t>(axis)]
              == direction[static_cast<std::size_t>(axis)];
      reverse = reverse
          && displacement[static_cast<std::size_t>(axis)]
              == -direction[static_cast<std::size_t>(axis)];
    }
    if (forward) return {channel, +1, true};
    if (reverse) return {channel, -1, true};
  }
  return {};
}

inline CubicVector reconstruct_channel_displacement(
    const MooreChannelRef& ref) {
  if (!ref.valid || ref.index < 0 || ref.index >= 13
      || (ref.orientation != -1 && ref.orientation != 1))
    return {};
  CubicVector result{};
  const auto& direction = kMooreChannelDirections[
      static_cast<std::size_t>(ref.index)];
  for (int axis = 0; axis < 3; ++axis)
    result[static_cast<std::size_t>(axis)] =
        ref.orientation * direction[static_cast<std::size_t>(axis)];
  return result;
}

struct HalfTickLinkExchange {
  int twice_time = 1;
  CubicVector displacement{};
  MooreChannelRef channel{};
  Vec3 particle_momentum_before{};
  Vec3 particle_momentum_after{};
  Vec3 field_momentum_exchange{};
  double work = 0.0;
  double field_energy_exchange = 0.0;
  double particle_energy_before = 0.0;
  double particle_energy_after = 0.0;
  double momentum_residual = 0.0;
  double energy_residual = 0.0;
  bool valid = false;
};

inline HalfTickLinkExchange make_half_tick_link_exchange(
    int tick, const Vec3& momentum, const CubicVector& displacement,
    double work) {
  HalfTickLinkExchange result;
  result.twice_time = 2 * tick + 1;
  result.displacement = displacement;
  result.channel = moore_channel_ref(displacement);
  result.particle_momentum_before = momentum;
  result.work = work;
  result.field_energy_exchange = -work;
  const Vec3 displacement_vec{
      static_cast<double>(displacement[0]),
      static_cast<double>(displacement[1]),
      static_cast<double>(displacement[2])};
  const auto update = selected_production_hop_update(
      momentum, displacement_vec, work);
  if (!result.channel.valid || !update.valid) return result;
  result.particle_momentum_after = update.momentum_after;
  result.field_momentum_exchange = update.required_field_recoil;
  result.particle_energy_before = update.energy_before;
  result.particle_energy_after = update.energy_after;
  result.momentum_residual =
      (result.particle_momentum_after - result.particle_momentum_before
       + result.field_momentum_exchange).mag();
  result.energy_residual = result.particle_energy_after
      - result.particle_energy_before + result.field_energy_exchange;
  result.valid = std::isfinite(result.momentum_residual)
      && std::isfinite(result.energy_residual);
  return result;
}

}  // namespace ftd::eft
