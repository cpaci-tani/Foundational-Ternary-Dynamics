/**
 * @file campaign_injective_local_permutation_event.cpp
 * @brief FTD-0466 injective 36-site cyclic-permutation event control.
 */

#include "ftd/eft/coupled_wave_tick_snapshot.h"
#include "ftd/eft/fixed_j_recoil_capacity.h"
#include "ftd/eft/localized_transverse_packet.h"
#include "ftd/eft/native_wave_energy.h"
#include "ftd/eft/production_hop_kinematics.h"

#include <algorithm>
#include <array>
#include <cmath>
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

std::vector<std::uint8_t> r1_union_support(
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

void cyclic_permute_r1_fields(const ftd::RenderBridge& input,
                              ftd::RenderBridge& output,
                              int source, int direction) {
  copy_state(input, output);
  const auto center = input.lattice().coord(source);
  for (int dy = -1; dy <= 1; ++dy) {
    for (int dz = -1; dz <= 1; ++dz) {
      std::array<int, 4> sites{};
      for (int position = 0; position < 4; ++position) {
        sites[static_cast<std::size_t>(position)] = input.lattice().index(
            center.x - 1 + position, center.y + dy, center.z + dz);
      }
      for (int position = 0; position < 4; ++position) {
        const int destination_position = direction > 0
            ? (position + 1) % 4 : (position + 3) % 4;
        const auto source_index = static_cast<std::size_t>(
            sites[static_cast<std::size_t>(position)]);
        const auto destination_index = static_cast<std::size_t>(
            sites[static_cast<std::size_t>(destination_position)]);
        output.voxels()[destination_index].flux =
            input.voxels()[source_index].flux;
        output.voxels()[destination_index].wave_vel =
            input.voxels()[source_index].wave_vel;
      }
    }
  }
}

double field_residual(const ftd::RenderBridge& lhs,
                      const ftd::RenderBridge& rhs) {
  double result = 0.0;
  for (std::size_t index = 0; index < lhs.voxels().size(); ++index) {
    result = std::max(result,
        (lhs.voxels()[index].flux - rhs.voxels()[index].flux).mag());
    result = std::max(result,
        (lhs.voxels()[index].wave_vel
         - rhs.voxels()[index].wave_vel).mag());
  }
  return result;
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

double deterministic_inverse_residual() {
  ftd::RenderBridge input(kL), forward(kL), reverse(kL);
  configure(input);
  configure(forward);
  configure(reverse);
  const int source = input.lattice().index(kCenter, kCenter, kCenter);
  for (int index = 0; index < static_cast<int>(input.voxels().size()); ++index) {
    const double x = static_cast<double>(index + 1);
    input.voxels()[static_cast<std::size_t>(index)].flux =
        {x * 1e-7, -x * 2e-7, x * 3e-7};
    input.voxels()[static_cast<std::size_t>(index)].wave_vel =
        {-x * 4e-7, x * 5e-7, -x * 6e-7};
  }
  cyclic_permute_r1_fields(input, forward, source, +1);
  cyclic_permute_r1_fields(forward, reverse, source, -1);
  return field_residual(input, reverse);
}

struct ArmResult {
  bool dressing = false;
  int attempts = 0;
  int kinematic_valid = 0;
  int momentum_pass = 0;
  long double required_square_sum = 0.0L;
  long double momentum_square_sum = 0.0L;
  double min_momentum_residual = std::numeric_limits<double>::infinity();
  double max_momentum_residual = 0.0;
  double max_required_abs = 0.0;
  double worst_energy_residual = 0.0;
  double worst_inverse_residual = 0.0;
  double worst_outside_support = 0.0;
  bool finite = true;

  double required_rms() const {
    return attempts > 0 ? std::sqrt(static_cast<double>(
        required_square_sum / static_cast<long double>(attempts))) : 0.0;
  }

  double momentum_rms() const {
    return attempts > 0 ? std::sqrt(static_cast<double>(
        momentum_square_sum / static_cast<long double>(attempts))) : 0.0;
  }
};

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  std::cout << "FTD-0466 injective local permutation event v1\n";
  const double fixture_inverse = deterministic_inverse_residual();

  ftd::RenderBridge packet(kL), dressing(kL), source_history(kL);
  ftd::RenderBridge unit(kL), unit_control(kL);
  for (auto* bridge : std::array<ftd::RenderBridge*, 5>{
           &packet, &dressing, &source_history, &unit, &unit_control})
    configure(*bridge);
  const int source = packet.lattice().index(kCenter, kCenter, kCenter);
  const int target = packet.lattice().index(kCenter + 1, kCenter, kCenter);
  const auto support = r1_union_support(packet.lattice(), source, target);
  const int support_count = static_cast<int>(std::count(
      support.begin(), support.end(), static_cast<std::uint8_t>(1)));

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

  std::array<ArmResult, 2> arms{};
  arms[0].dressing = false;
  arms[1].dressing = true;
  const ftd::Vec3 particle_momentum = ftd::eft::production_flat_momentum(
      {kInitialSpeed, 0.0, 0.0});
  const ftd::Vec3 displacement{1.0, 0.0, 0.0};
  double remainder_x = 0.0;
  for (int tick = 0; tick < kTicks; ++tick) {
    ftd::eft::advance_coupled_wave_tick_snapshot(packet);
    ftd::eft::advance_coupled_wave_tick_snapshot(dressing);
    ftd::eft::advance_coupled_wave_tick_snapshot(source_history);
    remainder_x += kInitialSpeed;
    if (remainder_x < 1.0) continue;

    for (int dressing_index = 0; dressing_index < 2; ++dressing_index) {
      auto& arm = arms[static_cast<std::size_t>(dressing_index)];
      ftd::RenderBridge before(kL), after(kL), reverse(kL);
      configure(before);
      configure(after);
      configure(reverse);
      copy_state(packet, before);
      if (arm.dressing) add_fields(dressing, before);
      add_fields(source_history, before);
      before.voxels()[static_cast<std::size_t>(source)].state = +1;
      before.voxels()[static_cast<std::size_t>(source)].velocity =
          {kInitialSpeed, 0.0, 0.0};

      cyclic_permute_r1_fields(before, after, source, +1);
      after.voxels()[static_cast<std::size_t>(source)].state = 0;
      after.voxels()[static_cast<std::size_t>(source)].velocity = {};
      after.voxels()[static_cast<std::size_t>(target)].state = +1;
      after.voxels()[static_cast<std::size_t>(target)].velocity =
          {kInitialSpeed, 0.0, 0.0};
      cyclic_permute_r1_fields(after, reverse, source, -1);

      const double inverse_residual = field_residual(before, reverse);
      const double outside = outside_support_fraction(before, after, support);
      const long double delta_field = observer_hamiltonian(after, target)
          - observer_hamiltonian(before, source);
      const double required_work = static_cast<double>(-delta_field);
      const auto update = ftd::eft::selected_production_hop_update(
          particle_momentum, displacement, required_work);

      ++arm.attempts;
      arm.kinematic_valid += update.valid ? 1 : 0;
      arm.required_square_sum += delta_field * delta_field;
      arm.max_required_abs = std::max(
          arm.max_required_abs, std::abs(required_work));
      arm.worst_inverse_residual = std::max(
          arm.worst_inverse_residual, inverse_residual);
      arm.worst_outside_support = std::max(
          arm.worst_outside_support, outside);
      arm.finite = arm.finite && std::isfinite(required_work)
          && std::isfinite(inverse_residual) && std::isfinite(outside);
      if (!update.valid) continue;

      const ftd::Vec3 field_delta = ftd::eft::central_field_momentum(after)
          - ftd::eft::central_field_momentum(before);
      const double momentum_residual =
          (field_delta - update.required_field_recoil).mag();
      const double energy_residual = std::abs(static_cast<double>(delta_field)
          + update.energy_after - update.energy_before);
      arm.momentum_pass += momentum_residual <= kGate ? 1 : 0;
      arm.momentum_square_sum += static_cast<long double>(momentum_residual)
          * static_cast<long double>(momentum_residual);
      arm.min_momentum_residual = std::min(
          arm.min_momentum_residual, momentum_residual);
      arm.max_momentum_residual = std::max(
          arm.max_momentum_residual, momentum_residual);
      arm.worst_energy_residual = std::max(
          arm.worst_energy_residual, energy_residual);
      arm.finite = arm.finite && std::isfinite(momentum_residual)
          && std::isfinite(energy_residual);
    }
  }

  bool protocol_valid = fixture_inverse <= kGate && support_count == 36;
  bool all_kinematic = true;
  bool all_momentum = true;
  for (const auto& arm : arms) {
    const bool arm_valid = arm.finite && arm.attempts == 42
        && arm.worst_inverse_residual <= kGate
        && arm.worst_outside_support <= kGate
        && arm.worst_energy_residual <= kGate;
    protocol_valid = protocol_valid && arm_valid;
    all_kinematic = all_kinematic && arm.kinematic_valid == arm.attempts;
    all_momentum = all_momentum && arm.momentum_pass == arm.attempts;
    std::cout << "arm,dressing," << (arm.dressing ? "on" : "off")
              << ",attempts," << arm.attempts
              << ",kinematic_valid," << arm.kinematic_valid
              << ",momentum_pass," << arm.momentum_pass
              << ",required_work_rms," << arm.required_rms()
              << ",max_required_abs," << arm.max_required_abs
              << ",momentum_residual_rms," << arm.momentum_rms()
              << ",min_momentum_residual," << arm.min_momentum_residual
              << ",max_momentum_residual," << arm.max_momentum_residual
              << ",worst_energy_residual," << arm.worst_energy_residual
              << ",worst_inverse_residual," << arm.worst_inverse_residual
              << ",worst_outside_support," << arm.worst_outside_support
              << ",valid," << (arm_valid ? "true" : "false") << '\n';
  }

  std::string verdict;
  if (!protocol_valid) verdict = "PROTOCOL_INVALID";
  else if (!all_kinematic)
    verdict = "INJECTIVE_LOCAL_PERMUTATION_KINEMATIC_VETO";
  else if (!all_momentum)
    verdict = "INJECTIVE_LOCAL_PERMUTATION_MOMENTUM_MISMATCH";
  else verdict = "INJECTIVE_LOCAL_PERMUTATION_ENERGY_MOMENTUM_CLOSES";
  std::cout << "summary,support_sites," << support_count
            << ",fixture_inverse_residual," << fixture_inverse
            << ",valid," << (protocol_valid ? "true" : "false") << '\n';
  std::cout << "verdict," << verdict << '\n';
  return protocol_valid ? 0 : 1;
}
