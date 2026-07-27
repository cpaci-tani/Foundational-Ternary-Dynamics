/**
 * @file campaign_nested_source_history_translation.cpp
 * @brief FTD-0464 fixed local source-history translation, dressing on/off.
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
#include <limits>
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
constexpr double kParentGlobalOnRms = 0.00023156579861414742;
constexpr std::array<int, 4> kRadii{1, 2, 3, -1};

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

int periodic_distance(int a, int b) {
  const int distance = std::abs(a - b);
  return std::min(distance, kL - distance);
}

bool in_source_cube(const ftd::Lattice& lattice, int index,
                    int source, int radius) {
  if (radius < 0) return true;
  const auto coordinate = lattice.coord(index);
  const auto center = lattice.coord(source);
  return periodic_distance(coordinate.x, center.x) <= radius
      && periodic_distance(coordinate.y, center.y) <= radius
      && periodic_distance(coordinate.z, center.z) <= radius;
}

std::vector<std::uint8_t> event_support(
    const ftd::Lattice& lattice, int source, int radius) {
  std::vector<std::uint8_t> support(lattice.total_sites(), 0);
  for (int index = 0; index < static_cast<int>(lattice.total_sites()); ++index) {
    if (!in_source_cube(lattice, index, source, radius)) continue;
    const auto coordinate = lattice.coord(index);
    const int destination = lattice.index(
        coordinate.x + 1, coordinate.y, coordinate.z);
    support[static_cast<std::size_t>(index)] = 1;
    support[static_cast<std::size_t>(destination)] = 1;
  }
  return support;
}

void translate_selected_source_history(
    const ftd::RenderBridge& source_history,
    ftd::RenderBridge& translated, int source, int target, int radius) {
  copy_state(source_history, translated);
  const auto& lattice = source_history.lattice();
  for (int index = 0; index < static_cast<int>(lattice.total_sites()); ++index) {
    if (!in_source_cube(lattice, index, source, radius)) continue;
    const auto& original = source_history.voxels()[static_cast<std::size_t>(index)];
    translated.voxels()[static_cast<std::size_t>(index)].flux -= original.flux;
    translated.voxels()[static_cast<std::size_t>(index)].wave_vel
        -= original.wave_vel;
    const auto coordinate = lattice.coord(index);
    const int destination = lattice.index(
        coordinate.x + 1, coordinate.y, coordinate.z);
    translated.voxels()[static_cast<std::size_t>(destination)].flux
        += original.flux;
    translated.voxels()[static_cast<std::size_t>(destination)].wave_vel
        += original.wave_vel;
  }
  translated.voxels()[static_cast<std::size_t>(source)].state = 0;
  translated.voxels()[static_cast<std::size_t>(source)].velocity = {};
  translated.voxels()[static_cast<std::size_t>(target)].state = +1;
  translated.voxels()[static_cast<std::size_t>(target)].velocity =
      {kInitialSpeed, 0.0, 0.0};
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

double selected_history_norm_fraction(
    const ftd::RenderBridge& source_history, int source, int radius) {
  long double selected = 0.0L;
  long double total = 0.0L;
  const auto& lattice = source_history.lattice();
  for (int index = 0; index < static_cast<int>(lattice.total_sites()); ++index) {
    const auto& voxel = source_history.voxels()[static_cast<std::size_t>(index)];
    const long double norm2 = static_cast<long double>(voxel.flux.mag2())
        + static_cast<long double>(voxel.wave_vel.mag2());
    total += norm2;
    if (in_source_cube(lattice, index, source, radius)) selected += norm2;
  }
  return total > 0.0L ? static_cast<double>(selected / total) : 0.0;
}

struct ArmResult {
  bool dressing = false;
  int radius = 0;
  int attempts = 0;
  int valid_count = 0;
  long double required_square_sum = 0.0L;
  long double self_square_sum = 0.0L;
  long double cross_square_sum = 0.0L;
  long double moved_fraction_sum = 0.0L;
  double min_moved_fraction = 1.0;
  double max_moved_fraction = 0.0;
  double max_required_abs = 0.0;
  double worst_identity = 0.0;
  double worst_outside = 0.0;
  bool finite = true;

  double required_rms() const {
    return attempts > 0 ? std::sqrt(static_cast<double>(
        required_square_sum / static_cast<long double>(attempts))) : 0.0;
  }

  double self_rms() const {
    return attempts > 0 ? std::sqrt(static_cast<double>(
        self_square_sum / static_cast<long double>(attempts))) : 0.0;
  }

  double cross_rms() const {
    return attempts > 0 ? std::sqrt(static_cast<double>(
        cross_square_sum / static_cast<long double>(attempts))) : 0.0;
  }

  double mean_moved_fraction() const {
    return attempts > 0 ? static_cast<double>(
        moved_fraction_sum / static_cast<long double>(attempts)) : 0.0;
  }
};

std::string radius_label(int radius) {
  return radius < 0 ? "global" : "R" + std::to_string(radius);
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  std::cout << "FTD-0464 nested source-history translation v1\n";

  ftd::RenderBridge packet(kL), dressing(kL), source_history(kL);
  ftd::RenderBridge unit(kL), unit_control(kL);
  for (auto* bridge : std::array<ftd::RenderBridge*, 5>{
           &packet, &dressing, &source_history, &unit, &unit_control})
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
  source_history.voxels()[static_cast<std::size_t>(source)].state = +1;
  source_history.voxels()[static_cast<std::size_t>(source)].velocity =
      {kInitialSpeed, 0.0, 0.0};

  std::array<ArmResult, 8> arms{};
  for (int dressing_index = 0; dressing_index < 2; ++dressing_index) {
    for (int radius_index = 0; radius_index < 4; ++radius_index) {
      auto& arm = arms[static_cast<std::size_t>(dressing_index * 4
          + radius_index)];
      arm.dressing = dressing_index != 0;
      arm.radius = kRadii[static_cast<std::size_t>(radius_index)];
    }
  }
  std::array<std::vector<std::uint8_t>, 4> supports{
      event_support(packet.lattice(), source, kRadii[0]),
      event_support(packet.lattice(), source, kRadii[1]),
      event_support(packet.lattice(), source, kRadii[2]),
      event_support(packet.lattice(), source, kRadii[3])};

  const ftd::Vec3 momentum = ftd::eft::production_flat_momentum(
      {kInitialSpeed, 0.0, 0.0});
  const ftd::Vec3 displacement{1.0, 0.0, 0.0};
  double remainder_x = 0.0;

  std::cout << "protocol,L," << kL << ",ticks," << kTicks
            << ",speed," << kInitialSpeed << ",radii,R1|R2|R3|global"
            << ",dressing,off|on,gate," << kGate << '\n';
  for (int tick = 0; tick < kTicks; ++tick) {
    ftd::eft::advance_coupled_wave_tick_snapshot(packet);
    ftd::eft::advance_coupled_wave_tick_snapshot(dressing);
    ftd::eft::advance_coupled_wave_tick_snapshot(source_history);
    remainder_x += kInitialSpeed;
    if (remainder_x < 1.0) continue;

    std::array<ftd::RenderBridge, 2> external{
        ftd::RenderBridge(kL), ftd::RenderBridge(kL)};
    for (auto& bridge : external) configure(bridge);
    copy_state(packet, external[0]);
    copy_state(packet, external[1]);
    add_fields(dressing, external[1]);

    std::array<ftd::RenderBridge, 2> before{
        ftd::RenderBridge(kL), ftd::RenderBridge(kL)};
    for (int dressing_index = 0; dressing_index < 2; ++dressing_index) {
      configure(before[static_cast<std::size_t>(dressing_index)]);
      copy_state(external[static_cast<std::size_t>(dressing_index)],
                 before[static_cast<std::size_t>(dressing_index)]);
      add_fields(source_history,
                 before[static_cast<std::size_t>(dressing_index)]);
      before[static_cast<std::size_t>(dressing_index)]
          .voxels()[static_cast<std::size_t>(source)].state = +1;
      before[static_cast<std::size_t>(dressing_index)]
          .voxels()[static_cast<std::size_t>(source)].velocity =
          {kInitialSpeed, 0.0, 0.0};
    }

    std::array<long double, 2> external_wave{};
    std::array<long double, 2> before_wave{};
    std::array<long double, 2> before_h{};
    for (int dressing_index = 0; dressing_index < 2; ++dressing_index) {
      const auto arm_index = static_cast<std::size_t>(dressing_index);
      external_wave[arm_index] = wave_energy(external[arm_index]);
      before_wave[arm_index] = wave_energy(before[arm_index]);
      before_h[arm_index] = before_wave[arm_index]
          - static_cast<long double>(ftd::G_C)
              * static_cast<long double>(
                  before[arm_index].divergence_flux(source));
    }

    const long double source_before_h = observer_hamiltonian(
        source_history, source);
    const long double source_before_wave = wave_energy(source_history);
    for (int radius_index = 0; radius_index < 4; ++radius_index) {
      const int radius = kRadii[static_cast<std::size_t>(radius_index)];
      ftd::RenderBridge translated(kL);
      configure(translated);
      translate_selected_source_history(
          source_history, translated, source, target, radius);
      const long double source_after_h = observer_hamiltonian(
          translated, target);
      const long double source_after_wave = wave_energy(translated);
      const long double delta_self = source_after_h - source_before_h;
      const double moved_fraction = selected_history_norm_fraction(
          source_history, source, radius);

      for (int dressing_index = 0; dressing_index < 2; ++dressing_index) {
        auto& arm = arms[static_cast<std::size_t>(dressing_index * 4
            + radius_index)];
        const auto& ext = external[static_cast<std::size_t>(dressing_index)];
        const auto& event_before = before[static_cast<std::size_t>(
            dressing_index)];
        const auto arm_index = static_cast<std::size_t>(dressing_index);
        ftd::RenderBridge event_after(kL);
        configure(event_after);
        copy_state(ext, event_after);
        add_fields(translated, event_after);
        event_after.voxels()[static_cast<std::size_t>(target)].state = +1;
        event_after.voxels()[static_cast<std::size_t>(target)].velocity =
            {kInitialSpeed, 0.0, 0.0};

        const long double after_wave = wave_energy(event_after);
        const long double cross_before = before_wave[arm_index]
            - external_wave[arm_index]
            - source_before_wave;
        const long double cross_after = after_wave - external_wave[arm_index]
            - source_after_wave;
        const long double delta_cross = cross_after - cross_before;
        const double external_work = ftd::eft::discrete_hop_work(
            +1, ext.divergence_flux(source), ext.divergence_flux(target));
        const long double after_h = after_wave
            - static_cast<long double>(ftd::G_C)
                * static_cast<long double>(
                    event_after.divergence_flux(target));
        const long double delta_event = after_h - before_h[arm_index];
        const long double identity = delta_event
            - (delta_self + delta_cross
               - static_cast<long double>(external_work));
        const double required_work = static_cast<double>(-delta_event);
        const auto update = ftd::eft::selected_production_hop_update(
            momentum, displacement, required_work);
        const double outside = outside_support_fraction(
            event_before, event_after,
            supports[static_cast<std::size_t>(radius_index)]);

        ++arm.attempts;
        arm.valid_count += update.valid ? 1 : 0;
        arm.required_square_sum += delta_event * delta_event;
        arm.self_square_sum += delta_self * delta_self;
        arm.cross_square_sum += delta_cross * delta_cross;
        arm.moved_fraction_sum += moved_fraction;
        arm.min_moved_fraction = std::min(
            arm.min_moved_fraction, moved_fraction);
        arm.max_moved_fraction = std::max(
            arm.max_moved_fraction, moved_fraction);
        arm.max_required_abs = std::max(
            arm.max_required_abs, std::abs(required_work));
        arm.worst_identity = std::max(
            arm.worst_identity, static_cast<double>(std::abs(identity)));
        arm.worst_outside = std::max(arm.worst_outside, outside);
        arm.finite = arm.finite && std::isfinite(required_work)
            && std::isfinite(static_cast<double>(delta_self))
            && std::isfinite(static_cast<double>(delta_cross))
            && std::isfinite(moved_fraction) && std::isfinite(outside);
      }
    }
  }

  bool protocol_valid = true;
  bool local_off_full = false;
  bool local_on_full = false;
  bool global_off_full = false;
  bool global_on_full = false;
  double parent_global_on_rms_residual = 0.0;
  for (const auto& arm : arms) {
    const bool arm_valid = arm.finite && arm.attempts == 42
        && arm.worst_identity <= kGate && arm.worst_outside <= kGate;
    protocol_valid = protocol_valid && arm_valid;
    const bool full = arm.valid_count == arm.attempts;
    if (arm.radius < 0) {
      if (arm.dressing) {
        global_on_full = full;
        parent_global_on_rms_residual = std::abs(
            arm.required_rms() - kParentGlobalOnRms);
      } else {
        global_off_full = full;
      }
    } else if (arm.dressing) {
      local_on_full = local_on_full || full;
    } else {
      local_off_full = local_off_full || full;
    }
    std::cout << "arm,dressing," << (arm.dressing ? "on" : "off")
              << ",radius," << radius_label(arm.radius)
              << ",attempts," << arm.attempts
              << ",valid_count," << arm.valid_count
              << ",required_work_rms," << arm.required_rms()
              << ",max_required_abs," << arm.max_required_abs
              << ",self_delta_rms," << arm.self_rms()
              << ",cross_delta_rms," << arm.cross_rms()
              << ",mean_moved_norm_fraction," << arm.mean_moved_fraction()
              << ",min_moved_norm_fraction," << arm.min_moved_fraction
              << ",max_moved_norm_fraction," << arm.max_moved_fraction
              << ",worst_identity," << arm.worst_identity
              << ",worst_outside_support," << arm.worst_outside
              << ",valid," << (arm_valid ? "true" : "false") << '\n';
  }
  protocol_valid = protocol_valid
      && parent_global_on_rms_residual <= kGate
      && global_on_full;

  std::string verdict;
  if (!protocol_valid) {
    verdict = "PROTOCOL_INVALID";
  } else if (local_off_full && local_on_full) {
    verdict = "LOCAL_TRANSLATION_SUFFICIENT_DRESSING_INDEPENDENT";
  } else if (local_on_full) {
    verdict = "LOCAL_TRANSLATION_SUFFICIENT_DRESSING_DEPENDENT";
  } else if (local_off_full) {
    verdict = "LOCAL_TRANSLATION_SUFFICIENT_DRESSING_OBSTRUCTS_REGISTERED_ARM";
  } else if (global_off_full || global_on_full) {
    verdict = "GLOBAL_TRANSLATION_ONLY";
  } else {
    verdict = "NO_FULL_RECOVERY";
  }

  std::cout << "summary,local_off_full,"
            << (local_off_full ? "true" : "false")
            << ",local_on_full," << (local_on_full ? "true" : "false")
            << ",global_off_full," << (global_off_full ? "true" : "false")
            << ",global_on_full," << (global_on_full ? "true" : "false")
            << ",parent_global_on_rms_residual,"
            << parent_global_on_rms_residual
            << ",valid," << (protocol_valid ? "true" : "false") << '\n';
  std::cout << "verdict," << verdict << '\n';
  return protocol_valid ? 0 : 1;
}
