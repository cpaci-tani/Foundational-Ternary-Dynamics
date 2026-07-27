/**
 * @file campaign_guide_cross_energy_decomposition.cpp
 * @brief FTD-0463 packet/source versus dressing/source wave cross energy.
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

long double wave_energy(const ftd::RenderBridge& bridge) {
  return ftd::eft::measure_native_wave_energy(bridge).tick_invariant;
}

long double wave_cross_energy(const ftd::RenderBridge& a,
                              const ftd::RenderBridge& b) {
  ftd::RenderBridge combined(kL);
  configure(combined);
  copy_state(a, combined);
  add_fields(b, combined);
  return wave_energy(combined) - wave_energy(a) - wave_energy(b);
}

void translate_fields(const ftd::RenderBridge& source,
                      ftd::RenderBridge& translated) {
  const auto& lattice = source.lattice();
  for (int index = 0; index < static_cast<int>(lattice.total_sites()); ++index) {
    const auto coordinate = lattice.coord(index);
    const int destination = lattice.index(
        coordinate.x + 1, coordinate.y, coordinate.z);
    translated.voxels()[static_cast<std::size_t>(destination)].flux =
        source.voxels()[static_cast<std::size_t>(index)].flux;
    translated.voxels()[static_cast<std::size_t>(destination)].wave_vel =
        source.voxels()[static_cast<std::size_t>(index)].wave_vel;
  }
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  std::cout << "FTD-0463 guide cross-energy decomposition v1\n";

  ftd::RenderBridge packet(kL), dressing(kL), external(kL);
  ftd::RenderBridge source_history(kL), unit(kL), unit_control(kL);
  for (auto* bridge : std::array<ftd::RenderBridge*, 6>{
           &packet, &dressing, &external, &source_history, &unit,
           &unit_control})
    configure(*bridge);
  const int source = packet.lattice().index(kCenter, kCenter, kCenter);
  const int target = packet.lattice().index(kCenter + 1, kCenter, kCenter);

  ftd::eft::LocalizedPacketSpec spec;
  spec.x0 = static_cast<double>(kCenter - 6);
  spec.y0 = static_cast<double>(kCenter) - 3.0;
  spec.z0 = static_cast<double>(kCenter);
  spec.direction = +1;
  spec.amplitude = kPacketAmplitude;
  ftd::eft::seed_localized_transverse_packet(packet, spec);
  populate_face_shape(unit, source, 1.0);
  copy_state(unit, unit_control);
  ftd::eft::advance_coupled_wave_tick_snapshot(unit_control);
  const double unit_difference = unit_control.divergence_flux(target)
      - unit_control.divergence_flux(source);
  const double dressing_scale = kInitialDressingWork
      / (ftd::G_C * unit_difference);
  populate_face_shape(dressing, source, dressing_scale);
  copy_state(packet, external);
  add_fields(dressing, external);
  source_history.voxels()[static_cast<std::size_t>(source)].state = +1;
  source_history.voxels()[static_cast<std::size_t>(source)].velocity =
      {kInitialSpeed, 0.0, 0.0};

  double remainder_x = 0.0;
  int attempts = 0;
  bool finite = true;
  long double packet_square_sum = 0.0L;
  long double dressing_square_sum = 0.0L;
  long double total_square_sum = 0.0L;
  double worst_cross_additivity = 0.0;
  double worst_work_additivity = 0.0;

  std::cout << "protocol,L," << kL << ",ticks," << kTicks
            << ",speed," << kInitialSpeed << ",gate," << kGate << '\n';
  for (int tick = 0; tick < kTicks; ++tick) {
    ftd::eft::advance_coupled_wave_tick_snapshot(packet);
    ftd::eft::advance_coupled_wave_tick_snapshot(dressing);
    ftd::eft::advance_coupled_wave_tick_snapshot(external);
    ftd::eft::advance_coupled_wave_tick_snapshot(source_history);
    remainder_x += kInitialSpeed;
    if (remainder_x < 1.0) continue;
    ++attempts;

    ftd::RenderBridge translated(kL);
    configure(translated);
    translate_fields(source_history, translated);
    const long double packet_before = wave_cross_energy(packet, source_history);
    const long double packet_after = wave_cross_energy(packet, translated);
    const long double dressing_before = wave_cross_energy(
        dressing, source_history);
    const long double dressing_after = wave_cross_energy(dressing, translated);
    const long double external_before = wave_cross_energy(
        external, source_history);
    const long double external_after = wave_cross_energy(external, translated);
    const long double delta_packet = packet_after - packet_before;
    const long double delta_dressing = dressing_after - dressing_before;
    const long double delta_external = external_after - external_before;
    const double cross_additivity = static_cast<double>(std::abs(
        delta_external - delta_packet - delta_dressing));

    const double packet_work = ftd::eft::discrete_hop_work(
        +1, packet.divergence_flux(source), packet.divergence_flux(target));
    const double dressing_work = ftd::eft::discrete_hop_work(
        +1, dressing.divergence_flux(source),
        dressing.divergence_flux(target));
    const double external_work = ftd::eft::discrete_hop_work(
        +1, external.divergence_flux(source),
        external.divergence_flux(target));
    const double work_additivity = std::abs(
        external_work - packet_work - dressing_work);
    const double rigid_required_work = static_cast<double>(
        static_cast<long double>(external_work) - delta_external);

    packet_square_sum += delta_packet * delta_packet;
    dressing_square_sum += delta_dressing * delta_dressing;
    total_square_sum += delta_external * delta_external;
    worst_cross_additivity = std::max(
        worst_cross_additivity, cross_additivity);
    worst_work_additivity = std::max(worst_work_additivity, work_additivity);
    finite = finite && std::isfinite(static_cast<double>(delta_packet))
        && std::isfinite(static_cast<double>(delta_dressing))
        && std::isfinite(static_cast<double>(delta_external))
        && std::isfinite(rigid_required_work);

    std::cout << "attempt,tick," << tick << ",delta_packet_cross,"
              << static_cast<double>(delta_packet)
              << ",delta_dressing_cross,"
              << static_cast<double>(delta_dressing)
              << ",delta_external_cross,"
              << static_cast<double>(delta_external)
              << ",packet_work," << packet_work
              << ",dressing_work," << dressing_work
              << ",external_work," << external_work
              << ",rigid_required_work," << rigid_required_work
              << ",cross_additivity_residual," << cross_additivity
              << ",work_additivity_residual," << work_additivity << '\n';
  }

  const double packet_rms = std::sqrt(static_cast<double>(
      packet_square_sum / static_cast<long double>(attempts)));
  const double dressing_rms = std::sqrt(static_cast<double>(
      dressing_square_sum / static_cast<long double>(attempts)));
  const double total_rms = std::sqrt(static_cast<double>(
      total_square_sum / static_cast<long double>(attempts)));
  const bool valid = finite && attempts == 42
      && worst_cross_additivity <= kGate
      && worst_work_additivity <= kGate;
  std::string verdict;
  if (!valid) verdict = "PROTOCOL_INVALID";
  else if (packet_rms >= 2.0 * dressing_rms)
    verdict = "PACKET_SOURCE_CROSS_DOMINATES";
  else if (dressing_rms >= 2.0 * packet_rms)
    verdict = "DRESSING_SOURCE_CROSS_DOMINATES";
  else verdict = "MIXED_GUIDE_CROSS_ENERGY";

  std::cout << "summary,attempts," << attempts << ",packet_cross_rms,"
            << packet_rms << ",dressing_cross_rms," << dressing_rms
            << ",external_cross_rms," << total_rms
            << ",worst_cross_additivity," << worst_cross_additivity
            << ",worst_work_additivity," << worst_work_additivity
            << ",valid," << (valid ? "true" : "false") << '\n';
  std::cout << "verdict," << verdict << '\n';
  return valid ? 0 : 1;
}
