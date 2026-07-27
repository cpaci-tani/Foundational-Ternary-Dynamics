/**
 * @file campaign_rigid_source_history_translation.cpp
 * @brief FTD-0462 rigid source-history translation versus production carry.
 */

#include "ftd/eft/coupled_wave_tick_snapshot.h"
#include "ftd/eft/discrete_interaction_work.h"
#include "ftd/eft/localized_transverse_packet.h"
#include "ftd/eft/native_wave_energy.h"
#include "ftd/eft/production_hop_kinematics.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

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

long double observer_hamiltonian(
    const ftd::RenderBridge& bridge, int manifested_site) {
  return wave_energy(bridge)
      - static_cast<long double>(ftd::G_C)
          * static_cast<long double>(bridge.divergence_flux(manifested_site));
}

double component_residual(const ftd::RenderBridge& full,
                          const ftd::RenderBridge& external,
                          const ftd::RenderBridge& source_history) {
  double residual = 0.0;
  for (std::size_t index = 0; index < full.voxels().size(); ++index) {
    residual = std::max(residual,
        (full.voxels()[index].flux - external.voxels()[index].flux
         - source_history.voxels()[index].flux).mag());
    residual = std::max(residual,
        (full.voxels()[index].wave_vel - external.voxels()[index].wave_vel
         - source_history.voxels()[index].wave_vel).mag());
  }
  return residual;
}

void translate_source_history(const ftd::RenderBridge& source_history,
                              ftd::RenderBridge& translated,
                              int target) {
  const auto& lattice = source_history.lattice();
  for (int index = 0; index < static_cast<int>(lattice.total_sites()); ++index) {
    const auto coordinate = lattice.coord(index);
    const int destination = lattice.index(
        coordinate.x + 1, coordinate.y, coordinate.z);
    translated.voxels()[static_cast<std::size_t>(destination)].flux =
        source_history.voxels()[static_cast<std::size_t>(index)].flux;
    translated.voxels()[static_cast<std::size_t>(destination)].wave_vel =
        source_history.voxels()[static_cast<std::size_t>(index)].wave_vel;
  }
  translated.voxels()[static_cast<std::size_t>(target)].state = +1;
  translated.voxels()[static_cast<std::size_t>(target)].velocity =
      {kInitialSpeed, 0.0, 0.0};
}

double translation_map_residual(
    const ftd::RenderBridge& source_history,
    const ftd::RenderBridge& translated) {
  const auto& lattice = source_history.lattice();
  double residual = 0.0;
  for (int index = 0; index < static_cast<int>(lattice.total_sites()); ++index) {
    const auto coordinate = lattice.coord(index);
    const int destination = lattice.index(
        coordinate.x + 1, coordinate.y, coordinate.z);
    residual = std::max(residual,
        (source_history.voxels()[static_cast<std::size_t>(index)].flux
         - translated.voxels()[static_cast<std::size_t>(destination)].flux)
            .mag());
    residual = std::max(residual,
        (source_history.voxels()[static_cast<std::size_t>(index)].wave_vel
         - translated.voxels()[static_cast<std::size_t>(destination)].wave_vel)
            .mag());
  }
  return residual;
}

void move_manifestation(ftd::RenderBridge& bridge, int source, int target) {
  const auto velocity = bridge.voxels()[static_cast<std::size_t>(source)].velocity;
  bridge.voxels()[static_cast<std::size_t>(target)].state = +1;
  bridge.voxels()[static_cast<std::size_t>(target)].velocity = velocity;
  bridge.voxels()[static_cast<std::size_t>(source)].state = 0;
  bridge.voxels()[static_cast<std::size_t>(source)].velocity = {};
}

void apply_production_local_carry(
    ftd::RenderBridge& bridge, int source, int target) {
  auto& source_voxel = bridge.voxels()[static_cast<std::size_t>(source)];
  auto& target_voxel = bridge.voxels()[static_cast<std::size_t>(target)];
  const double rho = source_voxel.flux.mag();
  if (rho > ftd::EPSILON_MAG) {
    const double transfer = std::min(rho, ftd::K_B);
    const ftd::Vec3 carried = source_voxel.flux * (transfer / rho);
    source_voxel.flux -= carried;
    target_voxel.flux += carried;
  }
  move_manifestation(bridge, source, target);
}

int periodic_distance(int a, int b) {
  const int distance = std::abs(a - b);
  return std::min(distance, kL - distance);
}

std::vector<std::uint8_t> r1_support(
    const ftd::Lattice& lattice, int source, int target) {
  std::vector<std::uint8_t> support(lattice.total_sites(), 0);
  const auto a = lattice.coord(source);
  const auto b = lattice.coord(target);
  for (int index = 0; index < static_cast<int>(lattice.total_sites()); ++index) {
    const auto c = lattice.coord(index);
    const auto near = [&](const ftd::Coord& center) {
      return periodic_distance(c.x, center.x) <= 1
          && periodic_distance(c.y, center.y) <= 1
          && periodic_distance(c.z, center.z) <= 1;
    };
    support[static_cast<std::size_t>(index)] = near(a) || near(b) ? 1 : 0;
  }
  return support;
}

double outside_support_fraction(
    const ftd::RenderBridge& before, const ftd::RenderBridge& after,
    const std::vector<std::uint8_t>& support) {
  long double total = 0.0L;
  long double outside = 0.0L;
  for (std::size_t index = 0; index < before.voxels().size(); ++index) {
    const auto delta_j = after.voxels()[index].flux
        - before.voxels()[index].flux;
    const auto delta_w = after.voxels()[index].wave_vel
        - before.voxels()[index].wave_vel;
    const long double norm2 = static_cast<long double>(delta_j.mag2())
        + static_cast<long double>(delta_w.mag2());
    total += norm2;
    if (support[index] == 0) outside += norm2;
  }
  return total > 0.0L ? static_cast<double>(outside / total) : 0.0;
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  std::cout << "FTD-0462 rigid source-history translation v1\n";

  ftd::RenderBridge packet(kL), dressing(kL), external(kL);
  ftd::RenderBridge source_history(kL), full(kL), unit(kL), unit_control(kL);
  for (auto* bridge : std::array<ftd::RenderBridge*, 7>{
           &packet, &dressing, &external, &source_history, &full,
           &unit, &unit_control})
    configure(*bridge);
  const int source = full.lattice().index(kCenter, kCenter, kCenter);
  const int target = full.lattice().index(kCenter + 1, kCenter, kCenter);
  const auto support = r1_support(full.lattice(), source, target);

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
  copy_state(external, full);
  full.voxels()[static_cast<std::size_t>(source)].state = +1;
  full.voxels()[static_cast<std::size_t>(source)].velocity =
      {kInitialSpeed, 0.0, 0.0};
  source_history.voxels()[static_cast<std::size_t>(source)].state = +1;
  source_history.voxels()[static_cast<std::size_t>(source)].velocity =
      {kInitialSpeed, 0.0, 0.0};

  const ftd::Vec3 momentum = ftd::eft::production_flat_momentum(
      {kInitialSpeed, 0.0, 0.0});
  const ftd::Vec3 displacement{1.0, 0.0, 0.0};
  double remainder_x = 0.0;
  int attempts = 0;
  int partial_valid_count = 0;
  int rigid_valid_count = 0;
  bool finite = true;
  double worst_component_closure = 0.0;
  long double worst_self_translation = 0.0L;
  long double worst_cross_identity = 0.0L;
  double worst_translation_map = 0.0;
  double maximum_outside_fraction = 0.0;
  double minimum_outside_fraction = 1.0;
  long double rigid_work_square_sum = 0.0L;
  long double partial_work_square_sum = 0.0L;

  std::cout << "protocol,L," << kL << ",ticks," << kTicks
            << ",speed," << kInitialSpeed << ",gate," << kGate << '\n';
  for (int tick = 0; tick < kTicks; ++tick) {
    ftd::eft::advance_coupled_wave_tick_snapshot(full);
    ftd::eft::advance_coupled_wave_tick_snapshot(external);
    ftd::eft::advance_coupled_wave_tick_snapshot(source_history);
    worst_component_closure = std::max(worst_component_closure,
        component_residual(full, external, source_history));
    remainder_x += kInitialSpeed;
    if (remainder_x < 1.0) continue;
    ++attempts;

    ftd::RenderBridge translated_source(kL), rigid_event(kL), partial_event(kL);
    configure(translated_source);
    configure(rigid_event);
    configure(partial_event);
    translate_source_history(source_history, translated_source, target);
    copy_state(external, rigid_event);
    add_fields(translated_source, rigid_event);
    rigid_event.voxels()[static_cast<std::size_t>(target)].state = +1;
    rigid_event.voxels()[static_cast<std::size_t>(target)].velocity =
        {kInitialSpeed, 0.0, 0.0};
    copy_state(full, partial_event);
    apply_production_local_carry(partial_event, source, target);

    const long double before = observer_hamiltonian(full, source);
    const long double delta_rigid =
        observer_hamiltonian(rigid_event, target) - before;
    const long double delta_partial =
        observer_hamiltonian(partial_event, target) - before;
    const long double self_translation = observer_hamiltonian(
        translated_source, target) - observer_hamiltonian(source_history, source);
    const long double cross_before = wave_energy(full) - wave_energy(external)
        - wave_energy(source_history);
    const long double cross_after = wave_energy(rigid_event)
        - wave_energy(external) - wave_energy(translated_source);
    const long double delta_cross = cross_after - cross_before;
    const double external_work = ftd::eft::discrete_hop_work(
        +1, external.divergence_flux(source),
        external.divergence_flux(target));
    const long double cross_identity = delta_rigid
        - (delta_cross - static_cast<long double>(external_work));
    const double required_rigid = static_cast<double>(-delta_rigid);
    const double required_partial = static_cast<double>(-delta_partial);
    const auto rigid_update = ftd::eft::selected_production_hop_update(
        momentum, displacement, required_rigid);
    const auto partial_update = ftd::eft::selected_production_hop_update(
        momentum, displacement, required_partial);
    rigid_valid_count += rigid_update.valid ? 1 : 0;
    partial_valid_count += partial_update.valid ? 1 : 0;
    const double outside = outside_support_fraction(full, rigid_event, support);
    maximum_outside_fraction = std::max(maximum_outside_fraction, outside);
    minimum_outside_fraction = std::min(minimum_outside_fraction, outside);
    worst_self_translation = std::max(
        worst_self_translation, std::abs(self_translation));
    worst_cross_identity = std::max(
        worst_cross_identity, std::abs(cross_identity));
    worst_translation_map = std::max(worst_translation_map,
        translation_map_residual(source_history, translated_source));
    rigid_work_square_sum += static_cast<long double>(required_rigid)
        * static_cast<long double>(required_rigid);
    partial_work_square_sum += static_cast<long double>(required_partial)
        * static_cast<long double>(required_partial);
    finite = finite && std::isfinite(required_rigid)
        && std::isfinite(required_partial) && std::isfinite(outside)
        && std::isfinite(static_cast<double>(delta_cross));

    const double total_endpoint_work = ftd::eft::discrete_hop_work(
        +1, full.divergence_flux(source), full.divergence_flux(target));
    std::cout << "attempt,tick," << tick << ",total_endpoint_work,"
              << total_endpoint_work << ",external_work," << external_work
              << ",delta_cross," << static_cast<double>(delta_cross)
              << ",delta_rigid," << static_cast<double>(delta_rigid)
              << ",required_rigid_work," << required_rigid
              << ",required_partial_work," << required_partial
              << ",rigid_kinematic," << (rigid_update.valid ? "true" : "false")
              << ",partial_kinematic,"
              << (partial_update.valid ? "true" : "false")
              << ",outside_r1_fraction," << outside
              << ",self_translation_residual,"
              << static_cast<double>(self_translation)
              << ",cross_identity_residual,"
              << static_cast<double>(cross_identity) << '\n';
  }

  const bool valid = finite && attempts == 42
      && worst_component_closure <= kGate
      && worst_self_translation <= kGate
      && worst_cross_identity <= kGate
      && worst_translation_map <= kGate;
  std::string verdict;
  if (!valid) {
    verdict = "PROTOCOL_INVALID";
  } else {
    std::string recovery = "NO_RECOVERY";
    if (rigid_valid_count == attempts) recovery = "FULL_RECOVERY";
    else if (rigid_valid_count > partial_valid_count)
      recovery = "PARTIAL_RECOVERY";
    const std::string locality = maximum_outside_fraction <= kGate
        ? "LOCAL" : "NONLOCAL_EVENT_SUPPORT";
    verdict = "RIGID_SOURCE_HISTORY_TRANSLATION_" + recovery + "_" + locality;
  }
  const long double rigid_rms = std::sqrt(
      rigid_work_square_sum / static_cast<long double>(attempts));
  const long double partial_rms = std::sqrt(
      partial_work_square_sum / static_cast<long double>(attempts));
  std::cout << "summary,attempts," << attempts << ",partial_valid_count,"
            << partial_valid_count << ",rigid_valid_count," << rigid_valid_count
            << ",rigid_required_work_rms," << static_cast<double>(rigid_rms)
            << ",partial_required_work_rms," << static_cast<double>(partial_rms)
            << ",min_outside_r1_fraction," << minimum_outside_fraction
            << ",max_outside_r1_fraction," << maximum_outside_fraction
            << ",worst_component_closure," << worst_component_closure
            << ",worst_self_translation,"
            << static_cast<double>(worst_self_translation)
            << ",worst_cross_identity,"
            << static_cast<double>(worst_cross_identity)
            << ",worst_translation_map," << worst_translation_map
            << ",valid," << (valid ? "true" : "false") << '\n';
  std::cout << "verdict," << verdict << '\n';
  return valid ? 0 : 1;
}
