/**
 * @file campaign_local_support_recoil_threshold.cpp
 * @brief FTD-0456 fixed-radius travelling-wave recoil thresholds.
 */

#include "ftd/eft/discrete_interaction_work.h"
#include "ftd/eft/exact_travelling_mode.h"
#include "ftd/eft/half_tick_link_exchange.h"
#include "ftd/eft/supported_paired_recoil_capacity.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace {

constexpr double kWork = 1e-4;
constexpr double kSpeed = 0.15;
constexpr double kGate = 1e-10;
constexpr double kWorkGate = 1e-12;
constexpr double kBracketGate = 1e-8;
constexpr int kBisectionIterations = 80;
constexpr int kMode = 1;
constexpr double kPhase = 0.0;
constexpr double kHighAmplitude = 1.0;
const std::array<int, 3> kLengths{{11, 17, 33}};
const std::array<int, 2> kSigns{{-1, +1}};
const std::array<int, 4> kRadii{{1, 2, 3, -1}};

void configure(ftd::RenderBridge& bridge) {
  bridge.force_cpu();
  bridge.toggles.disable_all();
  bridge.toggles.strict_validation = true;
}

void populate_minimal_face_shape(ftd::RenderBridge& bridge, double scale) {
  const int center = bridge.lattice().size() / 2;
  bridge.voxel_at(center + 2, center, center).flux.x = 2.0 * scale;
}

void make_control(const ftd::RenderBridge& old_state,
                  ftd::RenderBridge& control) {
  const double c2 = ftd::C_WAVE * ftd::C_WAVE;
  for (int index = 0; index < static_cast<int>(old_state.voxels().size());
       ++index) {
    const auto delta = old_state.laplacian_flux(index) * c2;
    control.voxels()[static_cast<std::size_t>(index)].wave_vel =
        old_state.voxels()[static_cast<std::size_t>(index)].wave_vel + delta;
    control.voxels()[static_cast<std::size_t>(index)].flux =
        old_state.voxels()[static_cast<std::size_t>(index)].flux
        + control.voxels()[static_cast<std::size_t>(index)].wave_vel;
  }
}

int periodic_distance(int a, int b, int length) {
  const int distance = std::abs(a - b);
  return std::min(distance, length - distance);
}

std::vector<std::uint8_t> make_support(const ftd::Lattice& lattice,
                                       int source, int target, int radius) {
  std::vector<std::uint8_t> support(
      static_cast<std::size_t>(lattice.total_sites()), 0);
  if (radius < 0) {
    std::fill(support.begin(), support.end(), 1);
    return support;
  }
  const auto source_coord = lattice.coord(source);
  const auto target_coord = lattice.coord(target);
  for (int index = 0; index < static_cast<int>(lattice.total_sites()); ++index) {
    const auto coordinate = lattice.coord(index);
    const auto near = [&](const ftd::Coord& center) {
      return periodic_distance(coordinate.x, center.x, lattice.size()) <= radius
          && periodic_distance(coordinate.y, center.y, lattice.size()) <= radius
          && periodic_distance(coordinate.z, center.z, lattice.size()) <= radius;
    };
    support[static_cast<std::size_t>(index)] =
        near(source_coord) || near(target_coord) ? 1 : 0;
  }
  return support;
}

int support_count(const std::vector<std::uint8_t>& support) {
  int count = 0;
  for (std::uint8_t value : support) count += value != 0 ? 1 : 0;
  return count;
}

void apply_paired_impulse(ftd::RenderBridge& state,
                          const std::vector<ftd::Vec3>& impulse) {
  for (std::size_t index = 0; index < impulse.size(); ++index) {
    state.voxels()[index].flux += impulse[index];
    state.voxels()[index].wave_vel += impulse[index];
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

struct ArmState {
  int length = 0;
  int sign = 1;
  int center = 0;
  int source = 0;
  int target = 0;
  double unit_wave_energy = 0.0;
  ftd::Vec3 requested_recoil{};
  std::vector<ftd::Vec3> base_flux;
  std::vector<ftd::Vec3> wave_flux;
  std::vector<ftd::Vec3> wave_vel;
  ftd::RenderBridge old_state;
  ftd::RenderBridge control;

  ArmState(int length_in, int sign_in)
      : length(length_in), sign(sign_in), center(length_in / 2),
        old_state(length_in), control(length_in) {
    configure(old_state);
    configure(control);
    source = old_state.lattice().index(center, center, center);
    target = old_state.lattice().index(center + 1, center, center);
    populate_minimal_face_shape(old_state, 1.0);
    make_control(old_state, control);
    const double unit_difference = control.divergence_flux(target)
        - control.divergence_flux(source);
    const double minimal_scale = kWork / (ftd::G_C * unit_difference);

    const std::size_t count = old_state.voxels().size();
    base_flux.assign(count, {});
    wave_flux.assign(count, {});
    wave_vel.assign(count, {});
    for (auto& voxel : old_state.voxels()) voxel = {};
    populate_minimal_face_shape(old_state, minimal_scale);
    for (std::size_t index = 0; index < count; ++index)
      base_flux[index] = old_state.voxels()[index].flux;
    for (int x = 0; x < length; ++x)
      for (int y = 0; y < length; ++y)
        for (int z = 0; z < length; ++z) {
          const int index = old_state.lattice().index(x, y, z);
          const auto sample = ftd::eft::exact_axis_travelling_mode(
              x, length, kMode, kPhase, sign, 1.0);
          wave_flux[static_cast<std::size_t>(index)].y = sample.flux;
          wave_vel[static_cast<std::size_t>(index)].y = sample.wave_vel;
          old_state.voxels()[static_cast<std::size_t>(index)].flux =
              wave_flux[static_cast<std::size_t>(index)];
          old_state.voxels()[static_cast<std::size_t>(index)].wave_vel =
              wave_vel[static_cast<std::size_t>(index)];
        }
    unit_wave_energy = static_cast<double>(
        ftd::eft::measure_native_wave_energy(old_state).tick_invariant);
    const ftd::eft::CubicVector displacement{{1, 0, 0}};
    const auto momentum = ftd::eft::production_flat_momentum(
        {kSpeed, 0.0, 0.0});
    requested_recoil = ftd::eft::make_half_tick_link_exchange(
        17, momentum, displacement, kWork).field_momentum_exchange;
  }

  ftd::eft::SupportedPairedRecoilCapacity evaluate(
      double amplitude, const std::vector<std::uint8_t>& support) {
    for (std::size_t index = 0; index < old_state.voxels().size(); ++index) {
      old_state.voxels()[index].flux = base_flux[index]
          + wave_flux[index] * amplitude;
      old_state.voxels()[index].wave_vel = wave_vel[index] * amplitude;
      control.voxels()[index] = {};
    }
    make_control(old_state, control);
    return ftd::eft::minimize_supported_paired_recoil_energy(
        old_state, control, target, +1, requested_recoil, support);
  }
};

struct Result {
  int length = 0;
  int sign = 1;
  int radius = 0;
  int sites = 0;
  bool bracketed = false;
  bool valid = false;
  double threshold = 0.0;
  double threshold_wave_energy = 0.0;
  double direct_energy_residual = 0.0;
  double momentum_residual = 0.0;
  double participation_sites = 0.0;
  double outside_max = 0.0;
};

Result run_threshold(ArmState& arm, int radius) {
  Result result;
  result.length = arm.length;
  result.sign = arm.sign;
  result.radius = radius;
  const auto support = make_support(
      arm.old_state.lattice(), arm.source, arm.target, radius);
  result.sites = support_count(support);
  const auto low_capacity = arm.evaluate(0.0, support);
  const auto high_capacity = arm.evaluate(kHighAmplitude, support);
  result.valid = low_capacity.valid && high_capacity.valid;
  result.bracketed = result.valid
      && low_capacity.minimum_total_energy_change > kBracketGate
      && high_capacity.minimum_total_energy_change < -kBracketGate;
  if (!result.bracketed) return result;

  double low = 0.0;
  double high = kHighAmplitude;
  for (int iteration = 0; iteration < kBisectionIterations; ++iteration) {
    const double middle = 0.5 * (low + high);
    const auto capacity = arm.evaluate(middle, support);
    result.valid = result.valid && capacity.valid;
    if (capacity.minimum_total_energy_change <= 0.0L)
      high = middle;
    else
      low = middle;
  }
  result.threshold = high;
  result.threshold_wave_energy = high * high * arm.unit_wave_energy;
  const auto capacity = arm.evaluate(high, support);
  result.valid = result.valid && capacity.valid
      && capacity.zero_energy_solution && capacity.support_exact
      && std::abs(capacity.minimum_total_energy_change) <= kGate;

  ftd::RenderBridge event(arm.length);
  configure(event);
  for (std::size_t index = 0; index < arm.control.voxels().size(); ++index)
    event.voxels()[index] = arm.control.voxels()[index];
  apply_paired_impulse(event, capacity.zero_energy_impulse);
  result.direct_energy_residual = static_cast<double>(
      complete_event_energy_change(
          arm.control, event, arm.source, arm.target));
  const auto direct_recoil = ftd::eft::central_field_momentum(event)
      - ftd::eft::central_field_momentum(arm.control);
  result.momentum_residual =
      (direct_recoil - arm.requested_recoil).mag();

  long double norm2 = 0.0L;
  long double norm4 = 0.0L;
  for (std::size_t index = 0; index < capacity.zero_energy_impulse.size();
       ++index) {
    const auto& impulse = capacity.zero_energy_impulse[index];
    const long double site_norm2 = ftd::eft::dot_long_double(
        impulse, impulse);
    norm2 += site_norm2;
    norm4 += site_norm2 * site_norm2;
    if (support[index] == 0)
      result.outside_max = std::max(result.outside_max, impulse.mag());
  }
  result.participation_sites = norm4 > 0.0L
      ? static_cast<double>(norm2 * norm2 / norm4) : 0.0;
  const double measured_work = ftd::eft::discrete_hop_work(
      +1, arm.control.divergence_flux(arm.source),
      arm.control.divergence_flux(arm.target));
  result.valid = result.valid
      && std::abs(measured_work - kWork) <= kWorkGate
      && std::abs(result.direct_energy_residual) <= kGate
      && result.momentum_residual <= kGate
      && result.outside_max == 0.0
      && std::isfinite(result.threshold_wave_energy)
      && std::isfinite(result.participation_sites);
  return result;
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  std::cout << "FTD-0456 local-support recoil threshold v1\n";
  std::cout << "protocol,lengths,11|17|33,signs,-1|1,phase,0,mode,1,"
            << "radii,1|2|3|global,work," << kWork << ",speed," << kSpeed
            << ",bracket,0|" << kHighAmplitude
            << ",bisection_iterations," << kBisectionIterations
            << ",gate," << kGate << '\n';

  std::vector<Result> results;
  for (int length : kLengths)
    for (int sign : kSigns) {
      ArmState arm(length, sign);
      for (int radius : kRadii)
        results.push_back(run_threshold(arm, radius));
    }

  bool valid = true;
  double worst_energy_residual = 0.0;
  double worst_momentum_residual = 0.0;
  double worst_outside = 0.0;
  std::array<int, 4> bracketed_by_support{};
  for (const auto& result : results) {
    valid = valid && result.valid;
    const int support_index = result.radius < 0 ? 3 : result.radius - 1;
    bracketed_by_support[static_cast<std::size_t>(support_index)]
        += result.bracketed ? 1 : 0;
    worst_energy_residual = std::max(
        worst_energy_residual, std::abs(result.direct_energy_residual));
    worst_momentum_residual = std::max(
        worst_momentum_residual, result.momentum_residual);
    worst_outside = std::max(worst_outside, result.outside_max);
    std::cout << "arm,L," << result.length << ",sign," << result.sign
              << ",radius," << result.radius << ",support_sites,"
              << result.sites << ",bracketed,"
              << (result.bracketed ? "true" : "false")
              << ",threshold," << result.threshold
              << ",threshold_wave_energy," << result.threshold_wave_energy
              << ",participation_sites," << result.participation_sites
              << ",direct_energy_residual," << result.direct_energy_residual
              << ",momentum_residual," << result.momentum_residual
              << ",outside_max," << result.outside_max
              << ",valid," << (result.valid ? "true" : "false") << '\n';
  }

  const bool r1_all = bracketed_by_support[0] == 6;
  const bool r2_all = bracketed_by_support[1] == 6;
  const bool r3_all = bracketed_by_support[2] == 6;
  const bool global_all = bracketed_by_support[3] == 6;
  const char* verdict = "PROTOCOL_INVALID";
  if (valid && global_all && r1_all)
    verdict = "R1_LOCAL_TRAVELLING_WAVE_RECOIL_THRESHOLD_CONSTRUCTED";
  else if (valid && global_all && r2_all)
    verdict = "R2_LOCAL_TRAVELLING_WAVE_RECOIL_THRESHOLD_CONSTRUCTED";
  else if (valid && global_all && r3_all)
    verdict = "R3_LOCAL_TRAVELLING_WAVE_RECOIL_THRESHOLD_CONSTRUCTED";
  else if (valid && global_all && !r1_all && !r2_all && !r3_all)
    verdict = "NO_FIXED_R3_TRAVELLING_WAVE_RECOIL_THRESHOLD";
  else if (valid)
    verdict = "MIXED_LOCAL_SUPPORT_RECOIL_THRESHOLD";

  std::cout << "summary,arms," << results.size()
            << ",r1_bracketed," << bracketed_by_support[0]
            << ",r2_bracketed," << bracketed_by_support[1]
            << ",r3_bracketed," << bracketed_by_support[2]
            << ",global_bracketed," << bracketed_by_support[3]
            << ",worst_energy_residual," << worst_energy_residual
            << ",worst_momentum_residual," << worst_momentum_residual
            << ",worst_outside," << worst_outside
            << ",valid," << (valid ? "true" : "false") << '\n';
  std::cout << "verdict," << verdict << '\n';
  return std::string(verdict) == "PROTOCOL_INVALID" ? 1 : 0;
}

