/**
 * @file campaign_production_local_flux_carry_work.cpp
 * @brief FTD-0461 energy accounting for production's integer-hop flux carry.
 */

#include "ftd/eft/coupled_wave_tick_snapshot.h"
#include "ftd/eft/discrete_interaction_work.h"
#include "ftd/eft/localized_transverse_packet.h"
#include "ftd/eft/native_wave_energy.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>

namespace {

constexpr int kL = 33;
constexpr int kCenter = kL / 2;
constexpr int kTicks = 48;
constexpr double kInitialSpeed = 0.15;
constexpr double kInitialDressingWork = 1e-4;
constexpr double kPacketAmplitude = 0.02;
constexpr double kGate = 1e-12;

void configure(ftd::RenderBridge& bridge) {
  bridge.force_cpu();
  bridge.toggles.disable_all();
  bridge.toggles.strict_validation = true;
}

void copy_state(const ftd::RenderBridge& source,
                ftd::RenderBridge& target) {
  for (std::size_t index = 0; index < source.voxels().size(); ++index)
    target.voxels()[index] = source.voxels()[index];
}

void populate_face_shape(ftd::RenderBridge& bridge, int source,
                         double scale) {
  const auto coordinate = bridge.lattice().coord(source);
  bridge.voxel_at(coordinate.x + 2, coordinate.y, coordinate.z).flux.x
      += 2.0 * scale;
}

double interaction_energy(const ftd::RenderBridge& bridge, int site) {
  return -ftd::G_C * bridge.divergence_flux(site);
}

long double observer_hamiltonian(
    const ftd::RenderBridge& bridge, int manifested_site) {
  return ftd::eft::measure_native_wave_energy(bridge).tick_invariant
      + static_cast<long double>(interaction_energy(bridge, manifested_site));
}

void move_manifestation(ftd::RenderBridge& bridge, int source, int target) {
  auto& source_voxel = bridge.voxels()[static_cast<std::size_t>(source)];
  const auto velocity = source_voxel.velocity;
  const auto remainder = source_voxel.remainder;
  bridge.voxels()[static_cast<std::size_t>(target)].state = +1;
  bridge.voxels()[static_cast<std::size_t>(target)].velocity = velocity;
  bridge.voxels()[static_cast<std::size_t>(target)].remainder = remainder;
  source_voxel.state = 0;
  source_voxel.velocity = {};
  source_voxel.remainder = {};
}

ftd::Vec3 apply_production_flux_carry(
    ftd::RenderBridge& bridge, int source, int target) {
  auto& source_voxel = bridge.voxels()[static_cast<std::size_t>(source)];
  auto& target_voxel = bridge.voxels()[static_cast<std::size_t>(target)];
  const double rho = source_voxel.flux.mag();
  ftd::Vec3 carried{};
  if (rho > ftd::EPSILON_MAG) {
    const double transfer = std::min(rho, ftd::K_B);
    carried = source_voxel.flux * (transfer / rho);
    source_voxel.flux -= carried;
    target_voxel.flux += carried;
  }
  move_manifestation(bridge, source, target);
  return carried;
}

double event_reverse_residual(ftd::RenderBridge& event,
                              const ftd::RenderBridge& control,
                              int source, int target,
                              const ftd::Vec3& carried) {
  auto& source_voxel = event.voxels()[static_cast<std::size_t>(source)];
  auto& target_voxel = event.voxels()[static_cast<std::size_t>(target)];
  source_voxel.state = +1;
  source_voxel.velocity = target_voxel.velocity;
  source_voxel.remainder = target_voxel.remainder;
  target_voxel.state = 0;
  target_voxel.velocity = {};
  target_voxel.remainder = {};
  source_voxel.flux += carried;
  target_voxel.flux -= carried;
  double residual = 0.0;
  for (std::size_t index = 0; index < event.voxels().size(); ++index) {
    const auto& a = event.voxels()[index];
    const auto& b = control.voxels()[index];
    residual = std::max(residual, (a.flux - b.flux).mag());
    residual = std::max(residual, (a.wave_vel - b.wave_vel).mag());
    residual = std::max(residual, (a.velocity - b.velocity).mag());
    residual = std::max(residual, (a.remainder - b.remainder).mag());
    if (a.state != b.state) residual = std::max(residual, 1.0);
  }
  return residual;
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  std::cout << "FTD-0461 production local flux-carry work v1\n";

  ftd::RenderBridge state(kL), packet(kL), dressing(kL), unit(kL);
  ftd::RenderBridge unit_control(kL);
  for (auto* bridge : std::array<ftd::RenderBridge*, 5>{
           &state, &packet, &dressing, &unit, &unit_control})
    configure(*bridge);
  const int source = state.lattice().index(kCenter, kCenter, kCenter);
  const int target = state.lattice().index(kCenter + 1, kCenter, kCenter);

  ftd::eft::LocalizedPacketSpec spec;
  spec.x0 = static_cast<double>(kCenter - 6);
  spec.y0 = static_cast<double>(kCenter) - 3.0;
  spec.z0 = static_cast<double>(kCenter);
  spec.direction = +1;
  spec.amplitude = kPacketAmplitude;
  ftd::eft::seed_localized_transverse_packet(packet, spec);
  copy_state(packet, state);

  populate_face_shape(unit, source, 1.0);
  copy_state(unit, unit_control);
  ftd::eft::advance_coupled_wave_tick_snapshot(unit_control);
  const double unit_difference = unit_control.divergence_flux(target)
      - unit_control.divergence_flux(source);
  const double dressing_scale = kInitialDressingWork
      / (ftd::G_C * unit_difference);
  populate_face_shape(dressing, source, dressing_scale);
  for (std::size_t index = 0; index < state.voxels().size(); ++index) {
    state.voxels()[index].flux += dressing.voxels()[index].flux;
    state.voxels()[index].wave_vel += dressing.voxels()[index].wave_vel;
  }
  state.voxels()[static_cast<std::size_t>(source)].state = +1;
  state.voxels()[static_cast<std::size_t>(source)].velocity =
      {kInitialSpeed, 0.0, 0.0};

  double remainder_x = 0.0;
  int attempts = 0;
  bool finite = true;
  double min_carried = 1e300;
  double max_carried = 0.0;
  long double correction_square_sum = 0.0L;
  long double maximum_production_residual = 0.0L;
  long double maximum_endpoint_residual = 0.0L;
  long double worst_fixed_closure = 0.0L;
  long double worst_identity = 0.0L;
  double worst_reverse = 0.0;

  std::cout << "protocol,L," << kL << ",ticks," << kTicks
            << ",speed," << kInitialSpeed << ",gate," << kGate << '\n';
  for (int tick = 0; tick < kTicks; ++tick) {
    ftd::eft::advance_coupled_wave_tick_snapshot(state);
    remainder_x += kInitialSpeed;
    if (remainder_x < 1.0) continue;
    ++attempts;

    const double endpoint_work = ftd::eft::discrete_hop_work(
        +1, state.divergence_flux(source), state.divergence_flux(target));
    const long double before = observer_hamiltonian(state, source);

    ftd::RenderBridge fixed(kL), carried_event(kL);
    configure(fixed);
    configure(carried_event);
    copy_state(state, fixed);
    copy_state(state, carried_event);
    move_manifestation(fixed, source, target);
    const long double delta_fixed = observer_hamiltonian(fixed, target) - before;
    const ftd::Vec3 carried = apply_production_flux_carry(
        carried_event, source, target);
    const long double delta_carry =
        observer_hamiltonian(carried_event, target) - before;
    const long double correction = delta_carry - delta_fixed;
    const long double fixed_closure =
        delta_fixed + static_cast<long double>(endpoint_work);
    const long double endpoint_residual =
        delta_carry + static_cast<long double>(endpoint_work);
    const long double required_work = -delta_carry;
    const long double identity = required_work
        - (static_cast<long double>(endpoint_work) - correction);
    const double reverse = event_reverse_residual(
        carried_event, state, source, target, carried);

    const double carried_magnitude = carried.mag();
    min_carried = std::min(min_carried, carried_magnitude);
    max_carried = std::max(max_carried, carried_magnitude);
    correction_square_sum += correction * correction;
    maximum_production_residual = std::max(
        maximum_production_residual, std::abs(delta_carry));
    maximum_endpoint_residual = std::max(
        maximum_endpoint_residual, std::abs(endpoint_residual));
    worst_fixed_closure = std::max(
        worst_fixed_closure, std::abs(fixed_closure));
    worst_identity = std::max(worst_identity, std::abs(identity));
    worst_reverse = std::max(worst_reverse, reverse);
    finite = finite && std::isfinite(endpoint_work)
        && std::isfinite(static_cast<double>(delta_fixed))
        && std::isfinite(static_cast<double>(delta_carry))
        && std::isfinite(static_cast<double>(correction))
        && std::isfinite(static_cast<double>(required_work));

    std::cout << "attempt,tick," << tick << ",endpoint_work,"
              << endpoint_work << ",delta_fixed,"
              << static_cast<double>(delta_fixed) << ",delta_carry,"
              << static_cast<double>(delta_carry) << ",carry_correction,"
              << static_cast<double>(correction) << ",required_work,"
              << static_cast<double>(required_work) << ",carried_magnitude,"
              << carried_magnitude << ",fixed_closure,"
              << static_cast<double>(fixed_closure)
              << ",endpoint_residual,"
              << static_cast<double>(endpoint_residual)
              << ",reverse_residual," << reverse << '\n';
  }

  const long double rms_correction = std::sqrt(
      correction_square_sum / static_cast<long double>(attempts));
  const bool valid = finite && attempts == 42
      && worst_fixed_closure <= kGate
      && worst_identity <= kGate
      && worst_reverse <= kGate;
  std::string verdict;
  if (!valid) {
    verdict = "PROTOCOL_INVALID";
  } else if (maximum_production_residual <= kGate) {
    verdict = "PRODUCTION_LOCAL_CARRY_ENERGY_NEUTRAL";
  } else if (maximum_endpoint_residual <= kGate) {
    verdict = "ENDPOINT_WORK_CLOSES_PRODUCTION_LOCAL_CARRY";
  } else {
    verdict = "PRODUCTION_LOCAL_CARRY_REQUIRES_EXPLICIT_WORK_CORRECTION";
  }

  std::cout << "summary,attempts," << attempts << ",min_carried,"
            << min_carried << ",max_carried," << max_carried
            << ",rms_carry_correction,"
            << static_cast<double>(rms_correction)
            << ",max_production_residual,"
            << static_cast<double>(maximum_production_residual)
            << ",max_endpoint_residual,"
            << static_cast<double>(maximum_endpoint_residual)
            << ",worst_fixed_closure,"
            << static_cast<double>(worst_fixed_closure)
            << ",worst_identity," << static_cast<double>(worst_identity)
            << ",worst_reverse," << worst_reverse
            << ",valid," << (valid ? "true" : "false") << '\n';
  std::cout << "verdict," << verdict << '\n';
  return valid ? 0 : 1;
}
