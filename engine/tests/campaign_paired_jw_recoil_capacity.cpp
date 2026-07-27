/**
 * @file campaign_paired_jw_recoil_capacity.cpp
 * @brief FTD-0454 simultaneous J/W recoil-capacity gate.
 */

#include "ftd/eft/discrete_interaction_work.h"
#include "ftd/eft/half_tick_link_exchange.h"
#include "ftd/eft/paired_jw_recoil_capacity.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>

namespace {

constexpr int kL = 11;
constexpr int kCenter = kL / 2;
constexpr double kWork = 1e-4;
constexpr double kSpeed = 0.15;
constexpr double kGate = 1e-12;
constexpr double kPositiveGate = 1e-8;

void configure(ftd::RenderBridge& bridge) {
  bridge.force_cpu();
  bridge.toggles.disable_all();
  bridge.toggles.strict_validation = true;
}

int active_axes(const ftd::eft::CubicVector& displacement) {
  int count = 0;
  for (int value : displacement) count += value != 0 ? 1 : 0;
  return count;
}

ftd::Vec3 as_vec(const ftd::eft::CubicVector& value) {
  return {static_cast<double>(value[0]), static_cast<double>(value[1]),
          static_cast<double>(value[2])};
}

void populate_shape(ftd::RenderBridge& bridge,
                    const ftd::eft::CubicVector& displacement,
                    double scale) {
  const int tx = kCenter + displacement[0];
  const int ty = kCenter + displacement[1];
  const int tz = kCenter + displacement[2];
  for (int axis = 0; axis < 3; ++axis) {
    const int sign = displacement[static_cast<std::size_t>(axis)];
    if (sign == 0) continue;
    int rx = tx;
    int ry = ty;
    int rz = tz;
    if (axis == 0) rx += sign;
    if (axis == 1) ry += sign;
    if (axis == 2) rz += sign;
    ftd::Vec3 flux{};
    if (axis == 0) flux.x = 2.0 * scale * sign;
    if (axis == 1) flux.y = 2.0 * scale * sign;
    if (axis == 2) flux.z = 2.0 * scale * sign;
    bridge.voxel_at(rx, ry, rz).flux = flux;
  }
}

void make_control(const ftd::RenderBridge& old_state,
                  ftd::RenderBridge& control) {
  const double c2 = ftd::C_WAVE * ftd::C_WAVE;
  for (int index = 0; index < static_cast<int>(old_state.voxels().size());
       ++index) {
    const auto delta = old_state.laplacian_flux(index) * c2;
    control.voxels()[static_cast<std::size_t>(index)].wave_vel = delta;
    control.voxels()[static_cast<std::size_t>(index)].flux =
        old_state.voxels()[static_cast<std::size_t>(index)].flux + delta;
  }
}

void apply_paired_impulse(ftd::RenderBridge& state,
                          const std::vector<ftd::Vec3>& impulse,
                          double sign) {
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

struct OrbitStats {
  int count = 0;
  long double minimum = std::numeric_limits<long double>::infinity();
  long double maximum = -std::numeric_limits<long double>::infinity();
};

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  std::cout << "FTD-0454 paired J/W recoil capacity v1\n";
  std::cout << "protocol,L," << kL << ",directions,26,work," << kWork
            << ",speed," << kSpeed << ",gate," << kGate
            << ",positive_gate," << kPositiveGate << '\n';
  std::cout << "event,W_event=W_control+S,J_event=J_control+S\n";

  int direction_count = 0;
  int positive_count = 0;
  int nonpositive_count = 0;
  bool finite = true;
  bool systems_valid = true;
  bool zero_solutions_pass = true;
  double worst_work_residual = 0.0;
  double worst_momentum_residual = 0.0;
  long double worst_minimum_formula_residual = 0.0L;
  long double worst_zero_energy_residual = 0.0L;
  double worst_zero_momentum_residual = 0.0;
  long double worst_reverse_energy = 0.0L;
  double worst_reverse_state = 0.0;
  long double smallest_minimum = std::numeric_limits<long double>::infinity();
  std::array<OrbitStats, 3> orbits{};

  for (int dx = -1; dx <= 1; ++dx)
    for (int dy = -1; dy <= 1; ++dy)
      for (int dz = -1; dz <= 1; ++dz) {
        if (dx == 0 && dy == 0 && dz == 0) continue;
        ++direction_count;
        const ftd::eft::CubicVector displacement{{dx, dy, dz}};
        const int shell = active_axes(displacement);
        ftd::RenderBridge unit_old(kL);
        ftd::RenderBridge unit_control(kL);
        configure(unit_old);
        configure(unit_control);
        populate_shape(unit_old, displacement, 1.0);
        make_control(unit_old, unit_control);
        const int source = unit_old.lattice().index(
            kCenter, kCenter, kCenter);
        const int target = unit_old.lattice().index(
            kCenter + dx, kCenter + dy, kCenter + dz);
        const double unit_divergence_difference =
            unit_control.divergence_flux(target)
            - unit_control.divergence_flux(source);
        const double scale = kWork
            / (ftd::G_C * unit_divergence_difference);

        ftd::RenderBridge old_state(kL);
        ftd::RenderBridge control(kL);
        configure(old_state);
        configure(control);
        populate_shape(old_state, displacement, scale);
        make_control(old_state, control);
        const double measured_work = ftd::eft::discrete_hop_work(
            +1, control.divergence_flux(source),
            control.divergence_flux(target));
        worst_work_residual = std::max(
            worst_work_residual, std::abs(measured_work - kWork));

        const auto direction_vec = as_vec(displacement);
        const auto direction = direction_vec * (1.0 / direction_vec.mag());
        const auto momentum = ftd::eft::production_flat_momentum(
            direction * kSpeed);
        const auto link = ftd::eft::make_half_tick_link_exchange(
            17, momentum, displacement, kWork);
        const auto capacity = ftd::eft::minimize_paired_jw_recoil_energy(
            old_state, control, target, +1,
            link.field_momentum_exchange);
        systems_valid = systems_valid && link.valid && capacity.valid;
        smallest_minimum = std::min(
            smallest_minimum, capacity.minimum_total_energy_change);
        if (capacity.minimum_total_energy_change > kPositiveGate)
          ++positive_count;
        else
          ++nonpositive_count;

        ftd::RenderBridge event_minimum(kL);
        configure(event_minimum);
        for (std::size_t index = 0; index < control.voxels().size(); ++index)
          event_minimum.voxels()[index] = control.voxels()[index];
        apply_paired_impulse(event_minimum, capacity.minimum_impulse, +1.0);
        const auto direct_recoil = ftd::eft::central_field_momentum(event_minimum)
            - ftd::eft::central_field_momentum(control);
        const double momentum_residual =
            (direct_recoil - link.field_momentum_exchange).mag();
        const long double direct_minimum = complete_event_energy_change(
            control, event_minimum, source, target);
        worst_momentum_residual = std::max(
            worst_momentum_residual, momentum_residual);
        worst_minimum_formula_residual = std::max(
            worst_minimum_formula_residual,
            std::abs(direct_minimum
                     - capacity.minimum_total_energy_change));
        apply_paired_impulse(event_minimum, capacity.minimum_impulse, -1.0);
        const long double reverse_energy = complete_event_energy_change(
            control, event_minimum, source, source) - kWork;
        double reverse_state = 0.0;
        for (std::size_t index = 0; index < control.voxels().size(); ++index) {
          reverse_state = std::max(reverse_state,
              (event_minimum.voxels()[index].flux
               - control.voxels()[index].flux).mag());
          reverse_state = std::max(reverse_state,
              (event_minimum.voxels()[index].wave_vel
               - control.voxels()[index].wave_vel).mag());
        }
        worst_reverse_state = std::max(worst_reverse_state, reverse_state);
        worst_reverse_energy = std::max(
            worst_reverse_energy, std::abs(reverse_energy));

        long double direct_zero = 0.0L;
        double direct_zero_momentum = 0.0;
        if (capacity.zero_energy_solution) {
          ftd::RenderBridge event_zero(kL);
          configure(event_zero);
          for (std::size_t index = 0; index < control.voxels().size(); ++index)
            event_zero.voxels()[index] = control.voxels()[index];
          apply_paired_impulse(
              event_zero, capacity.zero_energy_impulse, +1.0);
          direct_zero = complete_event_energy_change(
              control, event_zero, source, target);
          const auto zero_recoil = ftd::eft::central_field_momentum(event_zero)
              - ftd::eft::central_field_momentum(control);
          direct_zero_momentum =
              (zero_recoil - link.field_momentum_exchange).mag();
          worst_zero_energy_residual = std::max(
              worst_zero_energy_residual, std::abs(direct_zero));
          worst_zero_momentum_residual = std::max(
              worst_zero_momentum_residual, direct_zero_momentum);
          zero_solutions_pass = zero_solutions_pass
              && capacity.covariant_null_norm > kGate
              && std::abs(direct_zero) <= kGate
              && direct_zero_momentum <= kGate;
        } else if (capacity.minimum_total_energy_change <= 0.0L) {
          zero_solutions_pass = false;
        }

        auto& orbit = orbits[static_cast<std::size_t>(shell - 1)];
        ++orbit.count;
        orbit.minimum = std::min(
            orbit.minimum, capacity.minimum_total_energy_change);
        orbit.maximum = std::max(
            orbit.maximum, capacity.minimum_total_energy_change);
        finite = finite && std::isfinite(scale)
            && std::isfinite(measured_work)
            && std::isfinite(capacity.minimum_total_energy_change)
            && std::isfinite(direct_minimum)
            && std::isfinite(momentum_residual)
            && std::isfinite(reverse_state);

        std::cout << "direction,dx," << dx << ",dy," << dy << ",dz," << dz
                  << ",shell," << shell << ",scale," << scale
                  << ",work_residual," << measured_work - kWork
                  << ",recoil_mag," << link.field_momentum_exchange.mag()
                  << ",minimum_energy,"
                  << static_cast<double>(capacity.minimum_total_energy_change)
                  << ",direct_minimum_residual,"
                  << static_cast<double>(direct_minimum
                      - capacity.minimum_total_energy_change)
                  << ",momentum_residual," << momentum_residual
                  << ",null_norm,"
                  << static_cast<double>(capacity.covariant_null_norm)
                  << ",zero_solution,"
                  << (capacity.zero_energy_solution ? "true" : "false")
                  << ",zero_energy_residual," << static_cast<double>(direct_zero)
                  << ",zero_momentum_residual," << direct_zero_momentum << '\n';
      }

  bool orbit_covariant = true;
  for (int shell = 1; shell <= 3; ++shell) {
    const auto& orbit = orbits[static_cast<std::size_t>(shell - 1)];
    const long double spread = orbit.maximum - orbit.minimum;
    orbit_covariant = orbit_covariant && spread <= kGate;
    std::cout << "orbit,shell," << shell << ",count," << orbit.count
              << ",minimum," << static_cast<double>(orbit.minimum)
              << ",maximum," << static_cast<double>(orbit.maximum)
              << ",spread," << static_cast<double>(spread) << '\n';
  }

  const bool closure_pass = worst_work_residual <= kGate
      && worst_momentum_residual <= kGate
      && worst_minimum_formula_residual <= kGate
      && worst_reverse_state <= kGate
      && worst_reverse_energy <= kGate;
  const bool all_positive = positive_count == 26;
  const bool all_constructive = nonpositive_count == 26
      && zero_solutions_pass
      && worst_zero_energy_residual <= kGate
      && worst_zero_momentum_residual <= kGate;

  const char* verdict = "PROTOCOL_INVALID";
  if (finite && systems_valid && direction_count == 26 && closure_pass
      && orbit_covariant && all_positive)
    verdict = "PAIRED_JW_ZERO_ENERGY_RECOIL_IMPOSSIBLE_MINIMAL_WORK_FIELD";
  else if (finite && systems_valid && direction_count == 26 && closure_pass
           && orbit_covariant && all_constructive)
    verdict = "PAIRED_JW_ZERO_ENERGY_RECOIL_CONSTRUCTED";
  else if (finite && systems_valid && direction_count == 26 && closure_pass
           && orbit_covariant)
    verdict = "MIXED_PAIRED_JW_RECOIL_CAPACITY";
  else if (finite && !systems_valid)
    verdict = "CENTRAL_PAIRED_RECOIL_CONSTRAINT_UNREALIZABLE";

  std::cout << "summary,directions," << direction_count
            << ",positive_count," << positive_count
            << ",nonpositive_count," << nonpositive_count
            << ",smallest_minimum," << static_cast<double>(smallest_minimum)
            << ",worst_work_residual," << worst_work_residual
            << ",worst_momentum_residual," << worst_momentum_residual
            << ",worst_minimum_formula_residual,"
            << static_cast<double>(worst_minimum_formula_residual)
            << ",worst_zero_energy_residual,"
            << static_cast<double>(worst_zero_energy_residual)
            << ",worst_zero_momentum_residual,"
            << worst_zero_momentum_residual
            << ",worst_reverse_state," << worst_reverse_state
            << ",worst_reverse_energy,"
            << static_cast<double>(worst_reverse_energy)
            << ",finite," << (finite ? "true" : "false")
            << ",systems_valid," << (systems_valid ? "true" : "false")
            << ",closure_pass," << (closure_pass ? "true" : "false")
            << ",orbit_covariant," << (orbit_covariant ? "true" : "false")
            << ",zero_solutions_pass,"
            << (zero_solutions_pass ? "true" : "false") << '\n';
  std::cout << "verdict," << verdict << '\n';
  return std::string(verdict) == "PROTOCOL_INVALID" ? 1 : 0;
}

