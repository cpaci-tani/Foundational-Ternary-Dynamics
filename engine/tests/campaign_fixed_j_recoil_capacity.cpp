/**
 * @file campaign_fixed_j_recoil_capacity.cpp
 * @brief FTD-0453 fixed-J central-recoil energy-capacity gate.
 */

#include "ftd/eft/discrete_interaction_work.h"
#include "ftd/eft/fixed_j_recoil_capacity.h"
#include "ftd/eft/half_tick_link_exchange.h"

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
constexpr double kPositiveEnergyGate = 1e-8;

void configure(ftd::RenderBridge& bridge) {
  bridge.force_cpu();
  bridge.toggles.disable_all();
  bridge.toggles.strict_validation = true;
}

int active_axes(const ftd::eft::CubicVector& displacement) {
  int result = 0;
  for (int value : displacement) result += value != 0 ? 1 : 0;
  return result;
}

ftd::Vec3 as_vec(const ftd::eft::CubicVector& value) {
  return {static_cast<double>(value[0]), static_cast<double>(value[1]),
          static_cast<double>(value[2])};
}

void apply_update(ftd::RenderBridge& bridge,
                  const std::vector<ftd::Vec3>& update, double sign) {
  for (std::size_t index = 0; index < update.size(); ++index)
    bridge.voxels()[index].wave_vel += update[index] * sign;
}

struct OrbitStats {
  int count = 0;
  long double minimum = std::numeric_limits<long double>::infinity();
  long double maximum = -std::numeric_limits<long double>::infinity();
};

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  std::cout << "FTD-0453 fixed-J recoil capacity v1\n";
  std::cout << "protocol,L," << kL << ",directions,26,work," << kWork
            << ",speed," << kSpeed << ",gate," << kGate
            << ",positive_energy_gate," << kPositiveEnergyGate << '\n';
  std::cout << "variational_problem,min_0p5_u2_plus_b_dot_u_subject_to_Au_equals_recoil\n";

  int direction_count = 0;
  bool finite = true;
  bool systems_valid = true;
  bool all_positive = true;
  double worst_work_residual = 0.0;
  double worst_momentum_residual = 0.0;
  long double worst_energy_formula_residual = 0.0L;
  double worst_reverse_momentum = 0.0;
  long double worst_reverse_energy = 0.0L;
  double worst_reverse_wave = 0.0;
  long double smallest_minimum = std::numeric_limits<long double>::infinity();
  std::array<OrbitStats, 3> orbits{};

  for (int dx = -1; dx <= 1; ++dx)
    for (int dy = -1; dy <= 1; ++dy)
      for (int dz = -1; dz <= 1; ++dz) {
        if (dx == 0 && dy == 0 && dz == 0) continue;
        ++direction_count;
        const ftd::eft::CubicVector displacement{{dx, dy, dz}};
        const int shell = active_axes(displacement);
        const double g = kWork / (ftd::G_C * static_cast<double>(shell));
        ftd::RenderBridge bridge(kL);
        configure(bridge);
        const int sx = kCenter;
        const int sy = kCenter;
        const int sz = kCenter;
        const int tx = sx + dx;
        const int ty = sy + dy;
        const int tz = sz + dz;
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
          if (axis == 0) flux.x = 2.0 * g * sign;
          if (axis == 1) flux.y = 2.0 * g * sign;
          if (axis == 2) flux.z = 2.0 * g * sign;
          bridge.voxel_at(rx, ry, rz).flux = flux;
        }

        const int source = bridge.lattice().index(sx, sy, sz);
        const int target = bridge.lattice().index(tx, ty, tz);
        const double measured_work = ftd::eft::discrete_hop_work(
            +1, bridge.divergence_flux(source),
            bridge.divergence_flux(target));
        const double work_residual = measured_work - kWork;
        worst_work_residual = std::max(
            worst_work_residual, std::abs(work_residual));

        const auto direction = as_vec(displacement)
            * (1.0 / as_vec(displacement).mag());
        const auto momentum = ftd::eft::production_flat_momentum(
            direction * kSpeed);
        const auto link = ftd::eft::make_half_tick_link_exchange(
            17, momentum, displacement, kWork);
        const auto before = ftd::eft::measure_native_wave_energy(bridge);
        const auto capacity = ftd::eft::minimize_fixed_j_recoil_energy(
            bridge, link.field_momentum_exchange);
        systems_valid = systems_valid && link.valid && capacity.valid;
        worst_momentum_residual = std::max(
            worst_momentum_residual, capacity.momentum_residual);
        smallest_minimum = std::min(
            smallest_minimum, capacity.minimum_energy_change);
        all_positive = all_positive
            && capacity.minimum_energy_change > kPositiveEnergyGate;

        apply_update(bridge, capacity.wave_vel_update, +1.0);
        const auto after = ftd::eft::measure_native_wave_energy(bridge);
        const long double measured_delta =
            after.tick_invariant - before.tick_invariant;
        worst_energy_formula_residual = std::max(
            worst_energy_formula_residual,
            std::abs(measured_delta - capacity.minimum_energy_change));
        apply_update(bridge, capacity.wave_vel_update, -1.0);
        const auto reversed = ftd::eft::measure_native_wave_energy(bridge);
        const auto reverse_momentum = ftd::eft::central_field_momentum(bridge);
        double reverse_wave = 0.0;
        for (const auto& voxel : bridge.voxels())
          reverse_wave = std::max(reverse_wave, voxel.wave_vel.mag());
        worst_reverse_wave = std::max(worst_reverse_wave, reverse_wave);
        worst_reverse_momentum = std::max(
            worst_reverse_momentum, reverse_momentum.mag());
        worst_reverse_energy = std::max(
            worst_reverse_energy,
            std::abs(reversed.tick_invariant - before.tick_invariant));

        auto& orbit = orbits[static_cast<std::size_t>(shell - 1)];
        ++orbit.count;
        orbit.minimum = std::min(orbit.minimum,
                                 capacity.minimum_energy_change);
        orbit.maximum = std::max(orbit.maximum,
                                 capacity.minimum_energy_change);
        finite = finite && before.finite && after.finite && reversed.finite
            && std::isfinite(measured_work)
            && std::isfinite(capacity.minimum_energy_change)
            && std::isfinite(capacity.determinant);

        std::cout << "direction,dx," << dx << ",dy," << dy << ",dz," << dz
                  << ",shell," << shell << ",g," << g
                  << ",work_residual," << work_residual
                  << ",recoil_mag," << link.field_momentum_exchange.mag()
                  << ",gram_det," << static_cast<double>(capacity.determinant)
                  << ",momentum_residual," << capacity.momentum_residual
                  << ",minimum_energy_change,"
                  << static_cast<double>(capacity.minimum_energy_change)
                  << ",direct_formula_residual,"
                  << static_cast<double>(capacity.direct_energy_change
                      - capacity.minimum_energy_change)
                  << ",measured_formula_residual,"
                  << static_cast<double>(measured_delta
                      - capacity.minimum_energy_change) << '\n';
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
      && worst_energy_formula_residual <= kGate;
  const bool reverse_pass = worst_reverse_wave <= kGate
      && worst_reverse_momentum <= kGate
      && worst_reverse_energy <= kGate;
  const char* verdict = "PROTOCOL_INVALID";
  if (finite && systems_valid && direction_count == 26 && closure_pass
      && reverse_pass && orbit_covariant && all_positive)
    verdict = "FIXED_J_ZERO_ENERGY_RECOIL_IMPOSSIBLE_MINIMAL_WORK_FIELD";
  else if (finite && systems_valid && closure_pass && reverse_pass
           && orbit_covariant && !all_positive)
    verdict = "FIXED_J_ZERO_ENERGY_RECOIL_EXISTS";
  else if (finite && !systems_valid)
    verdict = "CENTRAL_RECOIL_CONSTRAINT_UNREALIZABLE";

  std::cout << "summary,directions," << direction_count
            << ",worst_work_residual," << worst_work_residual
            << ",worst_momentum_residual," << worst_momentum_residual
            << ",worst_energy_formula_residual,"
            << static_cast<double>(worst_energy_formula_residual)
            << ",smallest_minimum_energy,"
            << static_cast<double>(smallest_minimum)
            << ",worst_reverse_wave," << worst_reverse_wave
            << ",worst_reverse_momentum," << worst_reverse_momentum
            << ",worst_reverse_energy,"
            << static_cast<double>(worst_reverse_energy)
            << ",finite," << (finite ? "true" : "false")
            << ",systems_valid," << (systems_valid ? "true" : "false")
            << ",closure_pass," << (closure_pass ? "true" : "false")
            << ",reverse_pass," << (reverse_pass ? "true" : "false")
            << ",orbit_covariant," << (orbit_covariant ? "true" : "false")
            << ",all_positive," << (all_positive ? "true" : "false")
            << '\n';
  std::cout << "verdict," << verdict << '\n';
  return std::string(verdict) == "PROTOCOL_INVALID" ? 1 : 0;
}

