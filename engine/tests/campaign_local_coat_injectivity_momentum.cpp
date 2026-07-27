/**
 * @file campaign_local_coat_injectivity_momentum.cpp
 * @brief FTD-0465 injectivity and momentum audit of FTD-0464's R1 event.
 */

#include "ftd/eft/coupled_wave_tick_snapshot.h"
#include "ftd/eft/discrete_interaction_work.h"
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

namespace {

constexpr int kL = 33;
constexpr int kCenter = kL / 2;
constexpr int kTicks = 48;
constexpr int kRadius = 1;
constexpr double kInitialSpeed = 0.15;
constexpr double kInitialDressingWork = 1e-4;
constexpr double kPacketAmplitude = 0.02;
constexpr double kGate = 1e-12;
constexpr std::array<int, 3> kFiniteRadii{1, 2, 3};

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
  const auto coordinate = lattice.coord(index);
  const auto center = lattice.coord(source);
  return periodic_distance(coordinate.x, center.x) <= radius
      && periodic_distance(coordinate.y, center.y) <= radius
      && periodic_distance(coordinate.z, center.z) <= radius;
}

void translate_selected_fields(const ftd::RenderBridge& input,
                               ftd::RenderBridge& output,
                               int source, int radius) {
  copy_state(input, output);
  const auto& lattice = input.lattice();
  for (int index = 0; index < static_cast<int>(lattice.total_sites()); ++index) {
    if (!in_source_cube(lattice, index, source, radius)) continue;
    const auto& original = input.voxels()[static_cast<std::size_t>(index)];
    output.voxels()[static_cast<std::size_t>(index)].flux -= original.flux;
    output.voxels()[static_cast<std::size_t>(index)].wave_vel
        -= original.wave_vel;
    const auto coordinate = lattice.coord(index);
    const int destination = lattice.index(
        coordinate.x + 1, coordinate.y, coordinate.z);
    output.voxels()[static_cast<std::size_t>(destination)].flux
        += original.flux;
    output.voxels()[static_cast<std::size_t>(destination)].wave_vel
        += original.wave_vel;
  }
}

void translate_selected_source_history(
    const ftd::RenderBridge& source_history,
    ftd::RenderBridge& translated, int source, int target, int radius) {
  translate_selected_fields(source_history, translated, source, radius);
  translated.voxels()[static_cast<std::size_t>(source)].state = 0;
  translated.voxels()[static_cast<std::size_t>(source)].velocity = {};
  translated.voxels()[static_cast<std::size_t>(target)].state = +1;
  translated.voxels()[static_cast<std::size_t>(target)].velocity =
      {kInitialSpeed, 0.0, 0.0};
}

double maximum_field_norm(const ftd::RenderBridge& bridge) {
  double result = 0.0;
  for (const auto& voxel : bridge.voxels()) {
    result = std::max(result, voxel.flux.mag());
    result = std::max(result, voxel.wave_vel.mag());
  }
  return result;
}

double full_field_residual(const ftd::RenderBridge& lhs,
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

void set_component(ftd::Voxel& voxel, int component, double value) {
  ftd::Vec3* vector = component < 3 ? &voxel.flux : &voxel.wave_vel;
  const int axis = component % 3;
  if (axis == 0) vector->x = value;
  else if (axis == 1) vector->y = value;
  else vector->z = value;
}

struct KernelResult {
  int radius = 0;
  int witnesses = 0;
  int lower_bound = 0;
  double worst_image_norm = 0.0;
};

KernelResult certify_kernel(int radius) {
  KernelResult result;
  result.radius = radius;
  result.lower_bound = 6 * (2 * radius + 1) * (2 * radius + 1);
  ftd::RenderBridge geometry(kL);
  configure(geometry);
  const int source = geometry.lattice().index(kCenter, kCenter, kCenter);
  for (int dy = -radius; dy <= radius; ++dy) {
    for (int dz = -radius; dz <= radius; ++dz) {
      const int inner = geometry.lattice().index(
          kCenter + radius, kCenter + dy, kCenter + dz);
      const int outer = geometry.lattice().index(
          kCenter + radius + 1, kCenter + dy, kCenter + dz);
      for (int component = 0; component < 6; ++component) {
        ftd::RenderBridge witness(kL), image(kL);
        configure(witness);
        configure(image);
        set_component(witness.voxels()[static_cast<std::size_t>(inner)],
                      component, +1.0);
        set_component(witness.voxels()[static_cast<std::size_t>(outer)],
                      component, -1.0);
        translate_selected_fields(witness, image, source, radius);
        result.worst_image_norm = std::max(
            result.worst_image_norm, maximum_field_norm(image));
        ++result.witnesses;
      }
    }
  }
  return result;
}

void translate_all(const ftd::RenderBridge& input,
                   ftd::RenderBridge& output, int dx) {
  const auto& lattice = input.lattice();
  for (int index = 0; index < static_cast<int>(lattice.total_sites()); ++index) {
    const auto coordinate = lattice.coord(index);
    const int destination = lattice.index(
        coordinate.x + dx, coordinate.y, coordinate.z);
    output.voxels()[static_cast<std::size_t>(destination)].flux =
        input.voxels()[static_cast<std::size_t>(index)].flux;
    output.voxels()[static_cast<std::size_t>(destination)].wave_vel =
        input.voxels()[static_cast<std::size_t>(index)].wave_vel;
  }
}

double global_translation_reverse_residual() {
  ftd::RenderBridge input(kL), forward(kL), reverse(kL);
  configure(input);
  configure(forward);
  configure(reverse);
  for (int index = 0; index < static_cast<int>(input.voxels().size()); ++index) {
    const double x = static_cast<double>(index + 1);
    input.voxels()[static_cast<std::size_t>(index)].flux =
        {x * 1e-7, -x * 2e-7, x * 3e-7};
    input.voxels()[static_cast<std::size_t>(index)].wave_vel =
        {-x * 4e-7, x * 5e-7, -x * 6e-7};
  }
  translate_all(input, forward, +1);
  translate_all(forward, reverse, -1);
  return full_field_residual(input, reverse);
}

struct MomentumArm {
  bool dressing = false;
  int attempts = 0;
  int kinematic_valid = 0;
  int momentum_pass = 0;
  long double residual_square_sum = 0.0L;
  double min_residual = std::numeric_limits<double>::infinity();
  double max_residual = 0.0;
  double worst_energy_residual = 0.0;
  bool finite = true;

  double residual_rms() const {
    return attempts > 0 ? std::sqrt(static_cast<double>(
        residual_square_sum / static_cast<long double>(attempts))) : 0.0;
  }
};

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  std::cout << "FTD-0465 local coat injectivity and momentum v1\n";

  std::array<KernelResult, 3> kernels{};
  bool kernel_valid = true;
  for (std::size_t index = 0; index < kFiniteRadii.size(); ++index) {
    kernels[index] = certify_kernel(kFiniteRadii[index]);
    kernel_valid = kernel_valid
        && kernels[index].witnesses == kernels[index].lower_bound
        && kernels[index].worst_image_norm <= kGate;
    std::cout << "kernel,radius,R" << kernels[index].radius
              << ",independent_witnesses," << kernels[index].witnesses
              << ",nullity_lower_bound," << kernels[index].lower_bound
              << ",worst_image_norm," << kernels[index].worst_image_norm
              << '\n';
  }
  const double global_reverse = global_translation_reverse_residual();
  kernel_valid = kernel_valid && global_reverse <= kGate;

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

  std::array<MomentumArm, 2> arms{};
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

    ftd::RenderBridge translated(kL);
    configure(translated);
    translate_selected_source_history(
        source_history, translated, source, target, kRadius);
    for (int dressing_index = 0; dressing_index < 2; ++dressing_index) {
      auto& arm = arms[static_cast<std::size_t>(dressing_index)];
      ftd::RenderBridge external(kL), before(kL), after(kL);
      configure(external);
      configure(before);
      configure(after);
      copy_state(packet, external);
      if (arm.dressing) add_fields(dressing, external);
      copy_state(external, before);
      add_fields(source_history, before);
      before.voxels()[static_cast<std::size_t>(source)].state = +1;
      before.voxels()[static_cast<std::size_t>(source)].velocity =
          {kInitialSpeed, 0.0, 0.0};
      copy_state(external, after);
      add_fields(translated, after);
      after.voxels()[static_cast<std::size_t>(target)].state = +1;
      after.voxels()[static_cast<std::size_t>(target)].velocity =
          {kInitialSpeed, 0.0, 0.0};

      const long double delta_field = observer_hamiltonian(after, target)
          - observer_hamiltonian(before, source);
      const double required_work = static_cast<double>(-delta_field);
      const auto update = ftd::eft::selected_production_hop_update(
          particle_momentum, displacement, required_work);
      ++arm.attempts;
      arm.kinematic_valid += update.valid ? 1 : 0;
      if (!update.valid) {
        arm.finite = false;
        continue;
      }
      const ftd::Vec3 field_delta = ftd::eft::central_field_momentum(after)
          - ftd::eft::central_field_momentum(before);
      const double momentum_residual =
          (field_delta - update.required_field_recoil).mag();
      const double energy_residual = std::abs(static_cast<double>(delta_field)
          + update.energy_after - update.energy_before);
      arm.momentum_pass += momentum_residual <= kGate ? 1 : 0;
      arm.residual_square_sum += static_cast<long double>(momentum_residual)
          * static_cast<long double>(momentum_residual);
      arm.min_residual = std::min(arm.min_residual, momentum_residual);
      arm.max_residual = std::max(arm.max_residual, momentum_residual);
      arm.worst_energy_residual = std::max(
          arm.worst_energy_residual, energy_residual);
      arm.finite = arm.finite && std::isfinite(momentum_residual)
          && std::isfinite(energy_residual);
    }
  }

  bool protocol_valid = kernel_valid;
  bool all_momentum_close = true;
  for (const auto& arm : arms) {
    const bool arm_valid = arm.finite && arm.attempts == 42
        && arm.kinematic_valid == 42
        && arm.worst_energy_residual <= kGate;
    protocol_valid = protocol_valid && arm_valid;
    all_momentum_close = all_momentum_close
        && arm.momentum_pass == arm.attempts;
    std::cout << "momentum,dressing," << (arm.dressing ? "on" : "off")
              << ",attempts," << arm.attempts
              << ",kinematic_valid," << arm.kinematic_valid
              << ",momentum_pass," << arm.momentum_pass
              << ",momentum_residual_rms," << arm.residual_rms()
              << ",min_momentum_residual," << arm.min_residual
              << ",max_momentum_residual," << arm.max_residual
              << ",worst_energy_residual," << arm.worst_energy_residual
              << ",valid," << (arm_valid ? "true" : "false") << '\n';
  }

  std::string verdict;
  if (!protocol_valid) verdict = "PROTOCOL_INVALID";
  else if (all_momentum_close)
    verdict = "LOCAL_TRANSLATION_NONINJECTIVE_MOMENTUM_CLOSES";
  else verdict = "LOCAL_TRANSLATION_NONINJECTIVE_MOMENTUM_MISMATCH";
  std::cout << "summary,r1_nullity_lower_bound," << kernels[0].lower_bound
            << ",r2_nullity_lower_bound," << kernels[1].lower_bound
            << ",r3_nullity_lower_bound," << kernels[2].lower_bound
            << ",global_reverse_residual," << global_reverse
            << ",valid," << (protocol_valid ? "true" : "false") << '\n';
  std::cout << "verdict," << verdict << '\n';
  return protocol_valid ? 0 : 1;
}
