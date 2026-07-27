/**
 * @file campaign_blocked_hop_work_decomposition.cpp
 * @brief FTD-0460 exact additive decomposition of the FTD-0459 blocked work.
 */

#include "ftd/eft/coupled_wave_tick.h"
#include "ftd/eft/discrete_interaction_work.h"
#include "ftd/eft/localized_transverse_packet.h"
#include "ftd/eft/production_hop_kinematics.h"

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
constexpr double kLinearGate = 1e-12;
constexpr double kReverseGate = 1e-10;

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

void add_fields(const ftd::RenderBridge& source,
                ftd::RenderBridge& target) {
  for (std::size_t index = 0; index < source.voxels().size(); ++index) {
    target.voxels()[index].flux += source.voxels()[index].flux;
    target.voxels()[index].wave_vel += source.voxels()[index].wave_vel;
  }
}

void populate_face_shape(ftd::RenderBridge& bridge, int source,
                         double scale) {
  const auto coordinate = bridge.lattice().coord(source);
  bridge.voxel_at(coordinate.x + 2, coordinate.y, coordinate.z).flux.x
      += 2.0 * scale;
}

double forward_work(const ftd::RenderBridge& bridge, int source, int target) {
  return ftd::eft::discrete_hop_work(
      +1, bridge.divergence_flux(source), bridge.divergence_flux(target));
}

double field_residual(const ftd::RenderBridge& lhs,
                      const ftd::RenderBridge& rhs) {
  double residual = 0.0;
  for (std::size_t index = 0; index < lhs.voxels().size(); ++index) {
    residual = std::max(residual,
        (lhs.voxels()[index].flux - rhs.voxels()[index].flux).mag());
    residual = std::max(residual,
        (lhs.voxels()[index].wave_vel
         - rhs.voxels()[index].wave_vel).mag());
  }
  return residual;
}

double component_residual(const ftd::RenderBridge& full,
                          const ftd::RenderBridge& packet,
                          const ftd::RenderBridge& dressing,
                          const ftd::RenderBridge& moving_source) {
  double residual = 0.0;
  for (std::size_t index = 0; index < full.voxels().size(); ++index) {
    const auto reconstructed_j = packet.voxels()[index].flux
        + dressing.voxels()[index].flux
        + moving_source.voxels()[index].flux;
    const auto reconstructed_w = packet.voxels()[index].wave_vel
        + dressing.voxels()[index].wave_vel
        + moving_source.voxels()[index].wave_vel;
    residual = std::max(residual,
        (full.voxels()[index].flux - reconstructed_j).mag());
    residual = std::max(residual,
        (full.voxels()[index].wave_vel - reconstructed_w).mag());
  }
  return residual;
}

struct ComponentStats {
  const char* name = "";
  long double work_square_sum = 0.0L;
  int invalid_updates_rescued = 0;
};

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  std::cout << "FTD-0460 blocked-hop work decomposition v1\n";

  ftd::RenderBridge packet(kL), dressing(kL), full(kL);
  ftd::RenderBridge static_source(kL), moving_source(kL), unit(kL);
  ftd::RenderBridge unit_control(kL);
  for (auto* bridge : std::array<ftd::RenderBridge*, 7>{
           &packet, &dressing, &full, &static_source, &moving_source,
           &unit, &unit_control})
    configure(*bridge);

  const int source = full.lattice().index(kCenter, kCenter, kCenter);
  const int target = full.lattice().index(kCenter + 1, kCenter, kCenter);

  ftd::eft::LocalizedPacketSpec spec;
  spec.x0 = static_cast<double>(kCenter - 6);
  spec.y0 = static_cast<double>(kCenter) - 3.0;
  spec.z0 = static_cast<double>(kCenter);
  spec.direction = +1;
  spec.amplitude = kPacketAmplitude;
  ftd::eft::seed_localized_transverse_packet(packet, spec);

  populate_face_shape(unit, source, 1.0);
  copy_state(unit, unit_control);
  ftd::eft::advance_coupled_wave_tick(unit_control);
  const double unit_difference = unit_control.divergence_flux(target)
      - unit_control.divergence_flux(source);
  const double dressing_scale = kInitialDressingWork
      / (ftd::G_C * unit_difference);
  populate_face_shape(dressing, source, dressing_scale);

  add_fields(packet, full);
  add_fields(dressing, full);
  full.voxels()[static_cast<std::size_t>(source)].state = +1;
  full.voxels()[static_cast<std::size_t>(source)].velocity =
      {kInitialSpeed, 0.0, 0.0};
  static_source.voxels()[static_cast<std::size_t>(source)].state = +1;
  moving_source.voxels()[static_cast<std::size_t>(source)].state = +1;
  moving_source.voxels()[static_cast<std::size_t>(source)].velocity =
      {kInitialSpeed, 0.0, 0.0};

  ftd::RenderBridge initial_packet(kL), initial_dressing(kL), initial_full(kL);
  ftd::RenderBridge initial_static(kL), initial_moving(kL);
  for (auto* bridge : std::array<ftd::RenderBridge*, 5>{
           &initial_packet, &initial_dressing, &initial_full,
           &initial_static, &initial_moving})
    configure(*bridge);
  copy_state(packet, initial_packet);
  copy_state(dressing, initial_dressing);
  copy_state(full, initial_full);
  copy_state(static_source, initial_static);
  copy_state(moving_source, initial_moving);

  const ftd::Vec3 momentum = ftd::eft::production_flat_momentum(
      {kInitialSpeed, 0.0, 0.0});
  const double kinetic_energy =
      ftd::eft::production_flat_energy_from_momentum(momentum) - ftd::E_REST;
  double remainder_x = 0.0;
  int attempts = 0;
  int full_valid_count = 0;
  int source_free_valid_count = 0;
  double worst_work_closure = 0.0;
  double worst_component_closure = 0.0;
  bool finite = true;
  std::array<ComponentStats, 4> stats{{
      {"PACKET", 0.0L, 0}, {"DRESSING", 0.0L, 0},
      {"STATIC_POLARITY_SOURCE", 0.0L, 0},
      {"VELOCITY_CURL_SOURCE", 0.0L, 0}}};

  std::cout << "protocol,L," << kL << ",ticks," << kTicks
            << ",speed," << kInitialSpeed << ",kinetic_energy,"
            << kinetic_energy << ",linear_gate," << kLinearGate << '\n';
  for (int tick = 0; tick < kTicks; ++tick) {
    ftd::eft::advance_coupled_wave_tick(full);
    ftd::eft::advance_coupled_wave_tick(packet);
    ftd::eft::advance_coupled_wave_tick(dressing);
    ftd::eft::advance_coupled_wave_tick(static_source);
    ftd::eft::advance_coupled_wave_tick(moving_source);
    worst_component_closure = std::max(worst_component_closure,
        component_residual(full, packet, dressing, moving_source));

    remainder_x += kInitialSpeed;
    if (remainder_x < 1.0) continue;
    ++attempts;
    const double work_full = forward_work(full, source, target);
    const double work_packet = forward_work(packet, source, target);
    const double work_dressing = forward_work(dressing, source, target);
    const double work_static = forward_work(static_source, source, target);
    const double work_moving = forward_work(moving_source, source, target);
    const double work_curl = work_moving - work_static;
    const std::array<double, 4> contributions{{
        work_packet, work_dressing, work_static, work_curl}};
    const double work_sum = work_packet + work_dressing
        + work_static + work_curl;
    const double closure = std::abs(work_full - work_sum);
    worst_work_closure = std::max(worst_work_closure, closure);

    const ftd::Vec3 displacement{1.0, 0.0, 0.0};
    const auto full_update = ftd::eft::selected_production_hop_update(
        momentum, displacement, work_full);
    const auto source_free_update = ftd::eft::selected_production_hop_update(
        momentum, displacement, work_packet + work_dressing);
    full_valid_count += full_update.valid ? 1 : 0;
    source_free_valid_count += source_free_update.valid ? 1 : 0;
    for (std::size_t i = 0; i < stats.size(); ++i) {
      stats[i].work_square_sum += static_cast<long double>(contributions[i])
          * static_cast<long double>(contributions[i]);
      const auto removed = ftd::eft::selected_production_hop_update(
          momentum, displacement, work_full - contributions[i]);
      if (!full_update.valid && removed.valid)
        ++stats[i].invalid_updates_rescued;
    }
    finite = finite && std::isfinite(work_full) && std::isfinite(work_sum)
        && std::isfinite(closure);
    std::cout << "attempt,tick," << tick << ",full," << work_full
              << ",packet," << work_packet << ",dressing,"
              << work_dressing << ",static," << work_static << ",curl,"
              << work_curl << ",sum," << work_sum << ",closure," << closure
              << ",full_kinematic," << (full_update.valid ? "true" : "false")
              << ",source_free_kinematic,"
              << (source_free_update.valid ? "true" : "false") << '\n';
  }

  std::array<double, 4> rms{};
  for (std::size_t i = 0; i < stats.size(); ++i)
    rms[i] = std::sqrt(static_cast<double>(
        stats[i].work_square_sum / static_cast<long double>(attempts)));
  std::array<std::size_t, 4> order{{0, 1, 2, 3}};
  std::sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b) {
    return rms[a] > rms[b];
  });

  for (int tick = kTicks - 1; tick >= 0; --tick) {
    (void)tick;
    ftd::eft::reverse_coupled_wave_tick(full);
    ftd::eft::reverse_coupled_wave_tick(packet);
    ftd::eft::reverse_coupled_wave_tick(dressing);
    ftd::eft::reverse_coupled_wave_tick(static_source);
    ftd::eft::reverse_coupled_wave_tick(moving_source);
  }
  const double reverse_residual = std::max({
      field_residual(full, initial_full),
      field_residual(packet, initial_packet),
      field_residual(dressing, initial_dressing),
      field_residual(static_source, initial_static),
      field_residual(moving_source, initial_moving)});

  const bool valid = finite && attempts == 42
      && worst_work_closure <= kLinearGate
      && worst_component_closure <= kLinearGate
      && reverse_residual <= kReverseGate;
  std::string verdict = "NO_SINGLE_COMPONENT_DOMINATES_BLOCKED_HOP_WORK";
  if (valid && rms[order[0]] >= 2.0 * rms[order[1]]
      && stats[order[0]].invalid_updates_rescued > 0)
    verdict = std::string(stats[order[0]].name)
        + "_DOMINATES_BLOCKED_HOP_WORK";
  if (!valid) verdict = "PROTOCOL_INVALID";

  for (std::size_t i = 0; i < stats.size(); ++i)
    std::cout << "component," << stats[i].name << ",rms_work," << rms[i]
              << ",invalid_updates_rescued,"
              << stats[i].invalid_updates_rescued << '\n';
  std::cout << "summary,attempts," << attempts << ",full_valid_count,"
            << full_valid_count << ",source_free_valid_count,"
            << source_free_valid_count << ",worst_work_closure,"
            << worst_work_closure << ",worst_component_closure,"
            << worst_component_closure << ",reverse_residual,"
            << reverse_residual << ",valid," << (valid ? "true" : "false")
            << '\n';
  std::cout << "verdict," << verdict << '\n';
  return valid ? 0 : 1;
}
