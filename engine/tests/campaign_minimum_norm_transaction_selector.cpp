/**
 * @file campaign_minimum_norm_transaction_selector.cpp
 * @brief FTD-0458 unique cubic-covariant minimum-norm local selector.
 */

#include "ftd/eft/cubic_hop_response.h"
#include "ftd/eft/discrete_interaction_work.h"
#include "ftd/eft/half_tick_link_exchange.h"
#include "ftd/eft/localized_transverse_packet.h"
#include "ftd/eft/minimum_norm_transaction_selector.h"

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
constexpr double kWork = 1e-4;
constexpr double kSpeed = 0.15;
constexpr double kAmplitude = 0.02;
constexpr double kGate = 1e-10;
const ftd::eft::CubicVector kDisplacement{{1, 0, 0}};
const std::array<std::array<int, 3>, 6> kPermutations{{
    {{0, 1, 2}}, {{0, 2, 1}}, {{1, 0, 2}},
    {{1, 2, 0}}, {{2, 0, 1}}, {{2, 1, 0}}}};

void configure(ftd::RenderBridge& bridge) {
  bridge.force_cpu();
  bridge.toggles.disable_all();
  bridge.toggles.strict_validation = true;
}

std::vector<ftd::eft::SignedPermutation> cubic_group() {
  std::vector<ftd::eft::SignedPermutation> group;
  for (const auto& permutation : kPermutations)
    for (int sx : {-1, 1})
      for (int sy : {-1, 1})
        for (int sz : {-1, 1})
          group.push_back({permutation, {{sx, sy, sz}}});
  return group;
}

ftd::Vec3 transform_vector(const ftd::eft::SignedPermutation& transform,
                           const ftd::Vec3& value) {
  const std::array<double, 3> input{{value.x, value.y, value.z}};
  std::array<double, 3> output{};
  for (int axis = 0; axis < 3; ++axis)
    output[static_cast<std::size_t>(axis)] =
        transform.signs[static_cast<std::size_t>(axis)]
        * input[static_cast<std::size_t>(
            transform.permutation[static_cast<std::size_t>(axis)])];
  return {output[0], output[1], output[2]};
}

int centered_delta(int coordinate) {
  int delta = coordinate - kCenter;
  while (delta > kL / 2) delta -= kL;
  while (delta < -kL / 2) delta += kL;
  return delta;
}

int transformed_index(const ftd::Lattice& lattice, int index,
                      const ftd::eft::SignedPermutation& transform) {
  const auto coordinate = lattice.coord(index);
  const ftd::eft::CubicVector relative{{centered_delta(coordinate.x),
                                        centered_delta(coordinate.y),
                                        centered_delta(coordinate.z)}};
  const auto mapped = ftd::eft::apply_signed_permutation(transform, relative);
  return lattice.index(kCenter + mapped[0], kCenter + mapped[1],
                       kCenter + mapped[2]);
}

void transform_state(const ftd::RenderBridge& source,
                     ftd::RenderBridge& target,
                     const ftd::eft::SignedPermutation& transform) {
  for (std::size_t index = 0; index < target.voxels().size(); ++index)
    target.voxels()[index] = {};
  for (int index = 0; index < static_cast<int>(source.voxels().size()); ++index) {
    const int mapped = transformed_index(source.lattice(), index, transform);
    target.voxels()[static_cast<std::size_t>(mapped)].flux =
        transform_vector(transform,
            source.voxels()[static_cast<std::size_t>(index)].flux);
    target.voxels()[static_cast<std::size_t>(mapped)].wave_vel =
        transform_vector(transform,
            source.voxels()[static_cast<std::size_t>(index)].wave_vel);
  }
}

std::vector<std::uint8_t> transform_support(
    const ftd::Lattice& lattice, const std::vector<std::uint8_t>& source,
    const ftd::eft::SignedPermutation& transform) {
  std::vector<std::uint8_t> target(source.size(), 0);
  for (int index = 0; index < static_cast<int>(source.size()); ++index)
    target[static_cast<std::size_t>(
        transformed_index(lattice, index, transform))] =
        source[static_cast<std::size_t>(index)];
  return target;
}

std::vector<ftd::Vec3> transform_impulse(
    const ftd::Lattice& lattice, const std::vector<ftd::Vec3>& source,
    const ftd::eft::SignedPermutation& transform) {
  std::vector<ftd::Vec3> target(source.size());
  for (int index = 0; index < static_cast<int>(source.size()); ++index) {
    const int mapped = transformed_index(lattice, index, transform);
    target[static_cast<std::size_t>(mapped)] =
        transform_vector(transform, source[static_cast<std::size_t>(index)]);
  }
  return target;
}

double impulse_max_residual(const std::vector<ftd::Vec3>& lhs,
                            const std::vector<ftd::Vec3>& rhs) {
  double residual = 0.0;
  for (std::size_t index = 0; index < lhs.size(); ++index)
    residual = std::max(residual, (lhs[index] - rhs[index]).mag());
  return residual;
}

int periodic_distance(int a, int b) {
  const int distance = std::abs(a - b);
  return std::min(distance, kL - distance);
}

std::vector<std::uint8_t> r1_support(
    const ftd::Lattice& lattice, int source, int target) {
  std::vector<std::uint8_t> support(lattice.total_sites(), 0);
  const auto source_coord = lattice.coord(source);
  const auto target_coord = lattice.coord(target);
  for (int index = 0; index < static_cast<int>(lattice.total_sites()); ++index) {
    const auto coordinate = lattice.coord(index);
    const auto near = [&](const ftd::Coord& center) {
      return periodic_distance(coordinate.x, center.x) <= 1
          && periodic_distance(coordinate.y, center.y) <= 1
          && periodic_distance(coordinate.z, center.z) <= 1;
    };
    support[static_cast<std::size_t>(index)] =
        near(source_coord) || near(target_coord) ? 1 : 0;
  }
  return support;
}

void populate_face_shape(ftd::RenderBridge& bridge, double scale) {
  bridge.voxel_at(kCenter + 2, kCenter, kCenter).flux.x = 2.0 * scale;
}

void make_control(const ftd::RenderBridge& old_state,
                  ftd::RenderBridge& control) {
  for (std::size_t index = 0; index < old_state.voxels().size(); ++index) {
    control.voxels()[index].flux = old_state.voxels()[index].flux;
    control.voxels()[index].wave_vel = old_state.voxels()[index].wave_vel;
  }
  ftd::eft::advance_source_free_wave(control);
}

void apply_impulse(ftd::RenderBridge& state,
                   const std::vector<ftd::Vec3>& impulse, double sign) {
  for (std::size_t index = 0; index < impulse.size(); ++index) {
    state.voxels()[index].flux += impulse[index] * sign;
    state.voxels()[index].wave_vel += impulse[index] * sign;
  }
}

long double complete_event_energy_change(
    const ftd::RenderBridge& control, const ftd::RenderBridge& event,
    int source, int target) {
  const auto control_wave = ftd::eft::measure_native_wave_energy(control);
  const auto event_wave = ftd::eft::measure_native_wave_energy(event);
  const long double interaction_control = -static_cast<long double>(ftd::G_C)
      * static_cast<long double>(control.divergence_flux(source));
  const long double interaction_event = -static_cast<long double>(ftd::G_C)
      * static_cast<long double>(event.divergence_flux(target));
  return event_wave.tick_invariant - control_wave.tick_invariant
      + interaction_event - interaction_control
      + static_cast<long double>(kWork);
}

struct Seed {
  ftd::RenderBridge old_state{kL};
  ftd::RenderBridge control{kL};
  int source = 0;
  int target = 0;
  ftd::Vec3 recoil{};
  std::vector<std::uint8_t> support;
  ftd::eft::SupportedPairedRecoilCapacity capacity;
  ftd::eft::MinimumNormSelectorCertificate certificate;

  Seed() {
    configure(old_state);
    configure(control);
    source = old_state.lattice().index(kCenter, kCenter, kCenter);
    target = old_state.lattice().index(kCenter + 1, kCenter, kCenter);

    ftd::RenderBridge packet(kL);
    configure(packet);
    ftd::eft::LocalizedPacketSpec spec;
    spec.x0 = static_cast<double>(kCenter + 6);
    spec.y0 = static_cast<double>(kCenter) - 3.0;
    spec.z0 = static_cast<double>(kCenter);
    spec.direction = -1;
    ftd::eft::seed_localized_transverse_packet(packet, spec);
    for (int tick = 0; tick < 8; ++tick)
      ftd::eft::advance_source_free_wave(packet);

    populate_face_shape(old_state, 1.0);
    make_control(old_state, control);
    const double unit_difference = control.divergence_flux(target)
        - control.divergence_flux(source);
    const double scale = kWork / (ftd::G_C * unit_difference);
    for (auto& voxel : old_state.voxels()) voxel = {};
    populate_face_shape(old_state, scale);
    for (std::size_t index = 0; index < old_state.voxels().size(); ++index) {
      old_state.voxels()[index].flux +=
          packet.voxels()[index].flux * kAmplitude;
      old_state.voxels()[index].wave_vel =
          packet.voxels()[index].wave_vel * kAmplitude;
    }
    make_control(old_state, control);
    support = r1_support(old_state.lattice(), source, target);
    const auto momentum = ftd::eft::production_flat_momentum({kSpeed, 0, 0});
    recoil = ftd::eft::make_half_tick_link_exchange(
        17, momentum, kDisplacement, kWork).field_momentum_exchange;
    capacity = ftd::eft::minimize_supported_paired_recoil_energy(
        old_state, control, target, +1, recoil, support);
    certificate = ftd::eft::certify_minimum_norm_selector(
        old_state, control, target, +1, recoil, support, capacity);
  }
};

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  std::cout << "FTD-0458 minimum-norm transaction selector v1\n";
  std::cout << "protocol,L,33,group_size,48,hop,+x,support,R1,amplitude,"
            << kAmplitude << ",packet_direction,-1,packet_tick,8,work,"
            << kWork << ",speed," << kSpeed << ",gate," << kGate << '\n';

  Seed seed;
  const auto& base = seed.certificate;
  bool theorem_pass = base.valid
      && std::abs(base.radius_residual) <= kGate
      && std::abs(base.projected_minimum_residual) <= kGate
      && std::abs(base.selected_direction_residual) <= kGate
      && std::abs(base.norm2_bound_residual) <= kGate
      && std::abs(base.selected_energy_residual) <= kGate
      && base.selected_momentum_residual <= kGate
      && base.projected_coefficient_momentum_residual <= kGate
      && base.minimum_alternative_norm2_excess > kGate
      && base.worst_alternative_energy_residual <= kGate
      && base.worst_alternative_momentum_residual <= kGate;
  std::cout << "certificate,ambient_dimension," << base.ambient_dimension
            << ",constraint_rank," << base.constraint_rank
            << ",nullity," << base.nullity
            << ",zero_shell_dimension," << base.zero_shell_dimension
            << ",minimum_energy,"
            << static_cast<double>(seed.capacity.minimum_total_energy_change)
            << ",radius," << static_cast<double>(base.radius)
            << ",projected_coefficient_norm,"
            << static_cast<double>(base.projected_coefficient_norm)
            << ",norm2_bound_residual,"
            << static_cast<double>(base.norm2_bound_residual)
            << ",direction_residual,"
            << static_cast<double>(base.selected_direction_residual)
            << ",minimum_alternative_norm2_excess,"
            << static_cast<double>(base.minimum_alternative_norm2_excess)
            << ",alternative_count," << base.alternative_count
            << ",pass," << (theorem_pass ? "true" : "false") << '\n';

  const auto group = cubic_group();
  int covariance_pass_count = 0;
  double worst_work = 0.0;
  long double worst_minimum_energy = 0.0L;
  long double worst_norm2 = 0.0L;
  double worst_minimum_impulse = 0.0;
  double worst_selected_impulse = 0.0;
  long double worst_complete_energy = 0.0L;
  double worst_momentum = 0.0;
  double worst_reverse = 0.0;
  bool all_certificates = true;

  for (std::size_t arm = 0; arm < group.size(); ++arm) {
    const auto& transform = group[arm];
    ftd::RenderBridge old_state(kL);
    ftd::RenderBridge control(kL);
    configure(old_state);
    configure(control);
    transform_state(seed.old_state, old_state, transform);
    transform_state(seed.control, control, transform);
    const int source = old_state.lattice().index(kCenter, kCenter, kCenter);
    const auto displacement = ftd::eft::apply_signed_permutation(
        transform, kDisplacement);
    const int target = old_state.lattice().index(
        kCenter + displacement[0], kCenter + displacement[1],
        kCenter + displacement[2]);
    const auto support = transform_support(
        seed.old_state.lattice(), seed.support, transform);
    const auto recoil = transform_vector(transform, seed.recoil);
    const auto capacity = ftd::eft::minimize_supported_paired_recoil_energy(
        old_state, control, target, +1, recoil, support);
    const auto certificate = ftd::eft::certify_minimum_norm_selector(
        old_state, control, target, +1, recoil, support, capacity);
    const auto expected_minimum = transform_impulse(
        seed.old_state.lattice(), seed.capacity.minimum_impulse, transform);
    const auto expected_selected = transform_impulse(
        seed.old_state.lattice(), seed.capacity.zero_energy_impulse, transform);
    const double work_residual = std::abs(ftd::eft::discrete_hop_work(
        +1, control.divergence_flux(source), control.divergence_flux(target))
        - kWork);
    const long double minimum_energy_residual = std::abs(
        capacity.minimum_total_energy_change
        - seed.capacity.minimum_total_energy_change);
    const long double norm2_residual = std::abs(
        certificate.selected_norm2 - base.selected_norm2);
    const double minimum_impulse_residual = impulse_max_residual(
        capacity.minimum_impulse, expected_minimum);
    const double selected_impulse_residual = impulse_max_residual(
        capacity.zero_energy_impulse, expected_selected);

    ftd::RenderBridge event(kL);
    configure(event);
    for (std::size_t index = 0; index < control.voxels().size(); ++index)
      event.voxels()[index] = control.voxels()[index];
    apply_impulse(event, capacity.zero_energy_impulse, +1.0);
    const long double complete_energy = std::abs(complete_event_energy_change(
        control, event, source, target));
    const auto realized = ftd::eft::central_field_momentum(event)
        - ftd::eft::central_field_momentum(control);
    const double momentum_residual = (realized - recoil).mag();
    apply_impulse(event, capacity.zero_energy_impulse, -1.0);
    const double reverse_residual =
        ftd::eft::wave_state_max_residual(event, control);

    const bool arm_pass = certificate.valid
        && certificate.norm2_bound_residual <= kGate
        && certificate.selected_direction_residual <= kGate
        && certificate.minimum_alternative_norm2_excess > kGate
        && work_residual <= kGate
        && minimum_energy_residual <= kGate
        && norm2_residual <= kGate
        && minimum_impulse_residual <= kGate
        && selected_impulse_residual <= kGate
        && complete_energy <= kGate
        && momentum_residual <= kGate
        && reverse_residual <= kGate;
    covariance_pass_count += arm_pass ? 1 : 0;
    all_certificates = all_certificates && certificate.valid;
    worst_work = std::max(worst_work, work_residual);
    worst_minimum_energy = std::max(
        worst_minimum_energy, minimum_energy_residual);
    worst_norm2 = std::max(worst_norm2, norm2_residual);
    worst_minimum_impulse = std::max(
        worst_minimum_impulse, minimum_impulse_residual);
    worst_selected_impulse = std::max(
        worst_selected_impulse, selected_impulse_residual);
    worst_complete_energy = std::max(worst_complete_energy, complete_energy);
    worst_momentum = std::max(worst_momentum, momentum_residual);
    worst_reverse = std::max(worst_reverse, reverse_residual);
    std::cout << "arm,index," << arm << ",dx," << displacement[0]
              << ",dy," << displacement[1] << ",dz," << displacement[2]
              << ",minimum_energy_residual,"
              << static_cast<double>(minimum_energy_residual)
              << ",norm2_residual," << static_cast<double>(norm2_residual)
              << ",minimum_impulse_residual," << minimum_impulse_residual
              << ",selected_impulse_residual," << selected_impulse_residual
              << ",complete_energy_residual,"
              << static_cast<double>(complete_energy)
              << ",momentum_residual," << momentum_residual
              << ",reverse_residual," << reverse_residual
              << ",pass," << (arm_pass ? "true" : "false") << '\n';
  }

  const bool covariance_pass = group.size() == 48
      && covariance_pass_count == 48 && all_certificates;
  const char* verdict = "PROTOCOL_INVALID";
  if (theorem_pass && covariance_pass)
    verdict = "MINIMUM_NORM_LOCAL_SELECTOR_UNIQUE_CUBIC_COVARIANT";
  else if (theorem_pass && all_certificates)
    verdict = "MINIMUM_NORM_SELECTOR_UNIQUE_NOT_COVARIANT";
  else if (base.projected_coefficient_norm <= 1e-8L
           || base.constraint_rank < 3
           || (base.alternative_count == 5
               && base.minimum_alternative_norm2_excess <= kGate))
    verdict = "MINIMUM_NORM_SELECTOR_DEGENERATE";
  else if (seed.capacity.minimum_total_energy_change >= 0.0L)
    verdict = "MINIMUM_NORM_SELECTOR_NOT_EXISTENT";

  std::cout << "summary,group_size," << group.size()
            << ",covariance_pass_count," << covariance_pass_count
            << ",worst_work_residual," << worst_work
            << ",worst_minimum_energy_residual,"
            << static_cast<double>(worst_minimum_energy)
            << ",worst_norm2_residual," << static_cast<double>(worst_norm2)
            << ",worst_minimum_impulse_residual," << worst_minimum_impulse
            << ",worst_selected_impulse_residual," << worst_selected_impulse
            << ",worst_complete_energy_residual,"
            << static_cast<double>(worst_complete_energy)
            << ",worst_momentum_residual," << worst_momentum
            << ",worst_reverse_residual," << worst_reverse
            << ",theorem_pass," << (theorem_pass ? "true" : "false")
            << ",covariance_pass," << (covariance_pass ? "true" : "false")
            << '\n';
  std::cout << "verdict," << verdict << '\n';
  return std::string(verdict) == "PROTOCOL_INVALID" ? 1 : 0;
}

