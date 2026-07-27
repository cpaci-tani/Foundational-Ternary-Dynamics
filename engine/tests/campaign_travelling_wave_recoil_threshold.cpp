/**
 * @file campaign_travelling_wave_recoil_threshold.cpp
 * @brief FTD-0455 exact travelling-wave recoil-capacity threshold.
 */

#include "ftd/eft/discrete_interaction_work.h"
#include "ftd/eft/exact_travelling_mode.h"
#include "ftd/eft/half_tick_link_exchange.h"
#include "ftd/eft/paired_jw_recoil_capacity.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {

constexpr double kWork = 1e-4;
constexpr double kSpeed = 0.15;
constexpr double kGate = 1e-10;
constexpr double kBracketGate = 1e-8;
constexpr int kBisectionIterations = 80;
constexpr int kMode = 1;
constexpr double kHighAmplitude = 1.0;
constexpr double kPi = 3.141592653589793238462643383279502884;
const std::array<int, 3> kLengths{{11, 17, 33}};
const std::array<double, 2> kPhases{{0.0, 0.5 * kPi}};
const std::array<int, 2> kSigns{{-1, +1}};

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
  double phase = 0.0;
  int sign = 1;
  int center = 0;
  int source = 0;
  int target = 0;
  double minimal_scale = 0.0;
  double unit_wave_energy = 0.0;
  ftd::Vec3 requested_recoil{};
  std::vector<ftd::Vec3> base_flux;
  std::vector<ftd::Vec3> wave_flux;
  std::vector<ftd::Vec3> wave_vel;
  ftd::RenderBridge old_state;
  ftd::RenderBridge control;

  ArmState(int length_in, double phase_in, int sign_in)
      : length(length_in), phase(phase_in), sign(sign_in),
        center(length_in / 2),
        old_state(length_in), control(length_in) {
    configure(old_state);
    configure(control);
    source = old_state.lattice().index(center, center, center);
    target = old_state.lattice().index(center + 1, center, center);

    populate_minimal_face_shape(old_state, 1.0);
    make_control(old_state, control);
    const double unit_difference = control.divergence_flux(target)
        - control.divergence_flux(source);
    minimal_scale = kWork / (ftd::G_C * unit_difference);

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
              x, length, kMode, phase, sign, 1.0);
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

  ftd::eft::PairedJWRecoilCapacity evaluate(double amplitude) {
    for (std::size_t index = 0; index < old_state.voxels().size(); ++index) {
      old_state.voxels()[index].flux = base_flux[index]
          + wave_flux[index] * amplitude;
      old_state.voxels()[index].wave_vel = wave_vel[index] * amplitude;
      control.voxels()[index] = {};
    }
    make_control(old_state, control);
    return ftd::eft::minimize_paired_jw_recoil_energy(
        old_state, control, target, +1, requested_recoil);
  }
};

struct ThresholdResult {
  int length = 0;
  double phase = 0.0;
  int sign = 1;
  bool bracketed = false;
  bool valid = false;
  double threshold = 0.0;
  double threshold_wave_energy = 0.0;
  double a_sqrt_l = 0.0;
  double minimum_zero_side = 0.0;
  double direct_energy_residual = 0.0;
  double momentum_residual = 0.0;
  double participation_sites = 0.0;
  double participation_fraction = 0.0;
  double local_norm_fraction = 0.0;
};

bool in_local_union(const ftd::Lattice& lattice, int index,
                    int source, int target) {
  const auto c = lattice.coord(index);
  const auto s = lattice.coord(source);
  const auto t = lattice.coord(target);
  const auto periodic_distance = [&](int a, int b) {
    int distance = std::abs(a - b);
    return std::min(distance, lattice.size() - distance);
  };
  const bool near_source = periodic_distance(c.x, s.x) <= 1
      && periodic_distance(c.y, s.y) <= 1
      && periodic_distance(c.z, s.z) <= 1;
  const bool near_target = periodic_distance(c.x, t.x) <= 1
      && periodic_distance(c.y, t.y) <= 1
      && periodic_distance(c.z, t.z) <= 1;
  return near_source || near_target;
}

ThresholdResult run_arm(int length, double phase, int sign) {
  ArmState arm(length, phase, sign);
  ThresholdResult result;
  result.length = length;
  result.phase = phase;
  result.sign = sign;
  const auto low_capacity = arm.evaluate(0.0);
  const auto high_capacity = arm.evaluate(kHighAmplitude);
  result.valid = low_capacity.valid && high_capacity.valid;
  result.bracketed = result.valid
      && low_capacity.minimum_total_energy_change > kBracketGate
      && high_capacity.minimum_total_energy_change < -kBracketGate;
  if (!result.bracketed) return result;

  double low = 0.0;
  double high = kHighAmplitude;
  for (int iteration = 0; iteration < kBisectionIterations; ++iteration) {
    const double middle = 0.5 * (low + high);
    const auto capacity = arm.evaluate(middle);
    result.valid = result.valid && capacity.valid;
    if (capacity.minimum_total_energy_change <= 0.0L)
      high = middle;
    else
      low = middle;
  }
  result.threshold = high;
  const auto threshold_capacity = arm.evaluate(high);
  result.valid = result.valid && threshold_capacity.valid
      && threshold_capacity.zero_energy_solution;
  result.minimum_zero_side = static_cast<double>(
      threshold_capacity.minimum_total_energy_change);
  result.threshold_wave_energy = high * high * arm.unit_wave_energy;
  result.a_sqrt_l = high * std::sqrt(static_cast<double>(length));

  ftd::RenderBridge event(length);
  configure(event);
  for (std::size_t index = 0; index < arm.control.voxels().size(); ++index)
    event.voxels()[index] = arm.control.voxels()[index];
  apply_paired_impulse(event, threshold_capacity.zero_energy_impulse);
  result.direct_energy_residual = static_cast<double>(
      complete_event_energy_change(
          arm.control, event, arm.source, arm.target));
  const auto direct_recoil = ftd::eft::central_field_momentum(event)
      - ftd::eft::central_field_momentum(arm.control);
  result.momentum_residual =
      (direct_recoil - arm.requested_recoil).mag();

  long double norm2 = 0.0L;
  long double norm4 = 0.0L;
  long double local_norm2 = 0.0L;
  for (std::size_t index = 0;
       index < threshold_capacity.zero_energy_impulse.size(); ++index) {
    const long double site_norm2 = ftd::eft::dot_long_double(
        threshold_capacity.zero_energy_impulse[index],
        threshold_capacity.zero_energy_impulse[index]);
    norm2 += site_norm2;
    norm4 += site_norm2 * site_norm2;
    if (in_local_union(arm.old_state.lattice(), static_cast<int>(index),
                       arm.source, arm.target))
      local_norm2 += site_norm2;
  }
  result.participation_sites = norm4 > 0.0L
      ? static_cast<double>(norm2 * norm2 / norm4) : 0.0;
  result.participation_fraction = result.participation_sites
      / static_cast<double>(arm.old_state.voxels().size());
  result.local_norm_fraction = norm2 > 0.0L
      ? static_cast<double>(local_norm2 / norm2) : 0.0;
  result.valid = result.valid
      && std::abs(result.minimum_zero_side) <= kGate
      && std::abs(result.direct_energy_residual) <= kGate
      && result.momentum_residual <= kGate
      && std::isfinite(result.threshold_wave_energy)
      && std::isfinite(result.participation_sites)
      && std::isfinite(result.local_norm_fraction);
  return result;
}

double coefficient_of_variation(const std::vector<double>& values) {
  double mean = 0.0;
  for (double value : values) mean += value;
  mean /= static_cast<double>(values.size());
  double variance = 0.0;
  for (double value : values) variance += (value - mean) * (value - mean);
  variance /= static_cast<double>(values.size());
  return mean != 0.0 ? std::sqrt(variance) / std::abs(mean) : 0.0;
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  std::cout << "FTD-0455 travelling-wave recoil threshold v1\n";
  std::cout << "protocol,lengths,11|17|33,phases,0|pi_over_2,signs,-1|1,"
            << "mode," << kMode << ",work," << kWork << ",speed," << kSpeed
            << ",amplitude_bracket,0|" << kHighAmplitude
            << ",bisection_iterations," << kBisectionIterations
            << ",gate," << kGate << '\n';

  std::vector<ThresholdResult> results;
  for (int length : kLengths)
    for (double phase : kPhases)
      for (int sign : kSigns)
        results.push_back(run_arm(length, phase, sign));

  int bracketed_count = 0;
  bool valid = true;
  double worst_energy_residual = 0.0;
  double worst_momentum_residual = 0.0;
  for (const auto& result : results) {
    bracketed_count += result.bracketed ? 1 : 0;
    valid = valid && result.valid;
    worst_energy_residual = std::max(
        worst_energy_residual, std::abs(result.direct_energy_residual));
    worst_momentum_residual = std::max(
        worst_momentum_residual, result.momentum_residual);
    std::cout << "arm,L," << result.length << ",phase," << result.phase
              << ",sign," << result.sign
              << ",bracketed," << (result.bracketed ? "true" : "false")
              << ",threshold," << result.threshold
              << ",minimum_zero_side," << result.minimum_zero_side
              << ",threshold_wave_energy," << result.threshold_wave_energy
              << ",a_sqrt_l," << result.a_sqrt_l
              << ",participation_sites," << result.participation_sites
              << ",participation_fraction," << result.participation_fraction
              << ",local_norm_fraction," << result.local_norm_fraction
              << ",direct_energy_residual," << result.direct_energy_residual
              << ",momentum_residual," << result.momentum_residual
              << ",valid," << (result.valid ? "true" : "false") << '\n';
  }

  std::vector<double> mean_energy;
  std::vector<double> mean_a_sqrt_l;
  std::vector<double> mean_participation_fraction;
  std::vector<double> mean_local_fraction;
  for (int length : kLengths) {
    double energy = 0.0;
    double a_sqrt_l = 0.0;
    double participation = 0.0;
    double local = 0.0;
    int count = 0;
    for (const auto& result : results) {
      if (result.length != length || !result.bracketed) continue;
      energy += result.threshold_wave_energy;
      a_sqrt_l += result.a_sqrt_l;
      participation += result.participation_fraction;
      local += result.local_norm_fraction;
      ++count;
    }
    if (count > 0) {
      energy /= count;
      a_sqrt_l /= count;
      participation /= count;
      local /= count;
    }
    mean_energy.push_back(energy);
    mean_a_sqrt_l.push_back(a_sqrt_l);
    mean_participation_fraction.push_back(participation);
    mean_local_fraction.push_back(local);
    std::cout << "volume_summary,L," << length
              << ",mean_threshold_wave_energy," << energy
              << ",mean_a_sqrt_l," << a_sqrt_l
              << ",mean_participation_fraction," << participation
              << ",mean_local_norm_fraction," << local << '\n';
  }

  const double energy_cv = coefficient_of_variation(mean_energy);
  const double a_sqrt_l_cv = coefficient_of_variation(mean_a_sqrt_l);
  bool participation_extended = true;
  for (double value : mean_participation_fraction)
    participation_extended = participation_extended && value >= 0.05;
  const bool local_fraction_small = mean_local_fraction.back() < 0.10;
  const bool all_cross = bracketed_count == 12;
  const bool global_signature = all_cross && energy_cv <= 0.20
      && a_sqrt_l_cv <= 0.20 && participation_extended
      && local_fraction_small;

  const char* verdict = "PROTOCOL_INVALID";
  if (valid && all_cross && global_signature)
    verdict = "TRAVELLING_WAVE_GLOBAL_RESERVOIR_THRESHOLD_CONSTRUCTED";
  else if (valid && all_cross)
    verdict = "TRAVELLING_WAVE_THRESHOLD_CONSTRUCTED_SCALING_MIXED";
  else if (valid && bracketed_count == 0)
    verdict = "NO_TRAVELLING_WAVE_THRESHOLD_IN_REGISTERED_BRACKET";
  else if (valid)
    verdict = "MIXED_TRAVELLING_WAVE_THRESHOLD_ARMS";

  std::cout << "summary,arms," << results.size()
            << ",bracketed_count," << bracketed_count
            << ",energy_cv," << energy_cv
            << ",a_sqrt_l_cv," << a_sqrt_l_cv
            << ",participation_extended,"
            << (participation_extended ? "true" : "false")
            << ",local_fraction_small,"
            << (local_fraction_small ? "true" : "false")
            << ",worst_energy_residual," << worst_energy_residual
            << ",worst_momentum_residual," << worst_momentum_residual
            << ",valid," << (valid ? "true" : "false") << '\n';
  std::cout << "verdict," << verdict << '\n';
  return std::string(verdict) == "PROTOCOL_INVALID" ? 1 : 0;
}

