/**
 * @file campaign_symmetric_half_tick_energy.cpp
 * @brief FTD-0469 symmetric half-tick transaction energy/momentum/reversal
 *        gate for the common-action kick pair (parent FTD-0468).
 *
 * v2 (2026-07-25): energy residual gates are relative to the arm's ledger
 * magnitude max(1, |E_shadow_0|).  The v1 absolute gate compared 1e-12
 * against ledgers of magnitude ~56 on the pair-cubic fixtures and tripped
 * on double-accumulation noise at 3.8e-14 relative; see
 * PREREG_SYMMETRIC_HALF_TICK_ENERGY_v2.md.  Momentum and reversal gates
 * remain absolute.
 */

#include "ftd/eft/production_hop_kinematics.h"
#include "ftd/eft/symmetric_half_tick_transaction.h"
#include "ftd/field_operators.h"
#include "ftd/render_bridge.h"

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

constexpr int kStaticL = 17;
constexpr int kStaticTicks = 256;
constexpr int kDynamicL = 33;
constexpr int kDynamicTicks = 64;
constexpr int kMode = 2;
constexpr double kAmplitude = 1e-3;
constexpr double kWaveAmplitude = 0.02;
constexpr double kWavePhase = 0.37;
constexpr double kPi = 3.141592653589793238462643383279502884;
constexpr double kGate = 1e-12;
constexpr double kImpulseFloor = 1e-14;

enum class Fixture { SingleQuadratic, PairCubic };

void configure(ftd::RenderBridge& bridge) {
  bridge.force_cpu();
  bridge.toggles.disable_all();
  bridge.toggles.strict_validation = true;
}

int axis_coordinate(const ftd::Coord& coordinate, int axis) {
  if (axis == 0) return coordinate.x;
  if (axis == 1) return coordinate.y;
  return coordinate.z;
}

ftd::Vec3 axis_vector(int axis, double value) {
  if (axis == 0) return {value, 0.0, 0.0};
  if (axis == 1) return {0.0, value, 0.0};
  return {0.0, 0.0, value};
}

int periodic_offset(int coordinate, int center, int size) {
  int offset = coordinate - center;
  if (offset > size / 2) offset -= size;
  if (offset < -size / 2) offset += size;
  return offset;
}

void copy_state(const ftd::RenderBridge& source, ftd::RenderBridge& target) {
  for (std::size_t index = 0; index < source.voxels().size(); ++index)
    target.voxels()[index] = source.voxels()[index];
}

double field_restore_residual(const ftd::RenderBridge& lhs,
                              const ftd::RenderBridge& rhs) {
  double result = 0.0;
  for (std::size_t index = 0; index < lhs.voxels().size(); ++index) {
    result = std::max(result,
        (lhs.voxels()[index].flux - rhs.voxels()[index].flux).mag());
    result = std::max(result,
        (lhs.voxels()[index].wave_vel - rhs.voxels()[index].wave_vel).mag());
  }
  return result;
}

void seed_static_field(ftd::RenderBridge& bridge, int axis, Fixture fixture) {
  const int center = bridge.lattice().size() / 2;
  const int size = bridge.lattice().size();
  for (int index = 0; index < static_cast<int>(bridge.voxels().size());
       ++index) {
    const auto c = bridge.lattice().coord(index);
    const double r = static_cast<double>(periodic_offset(
        axis_coordinate(c, axis), center, size));
    const double value = fixture == Fixture::SingleQuadratic
        ? kAmplitude * r * r
        : kAmplitude * r * r * r;
    auto& voxel = bridge.voxels()[static_cast<std::size_t>(index)];
    voxel.flux = axis_vector(axis, value);
    const double p0 = 2.0 * kPi * static_cast<double>(c.x + 2 * c.y + 3 * c.z)
        / static_cast<double>(size);
    voxel.wave_vel = {0.003 * std::sin(p0),
                      0.004 * std::cos(p0),
                      -0.002 * std::sin(2.0 * p0)};
  }
}

void seed_single(ftd::RenderBridge& bridge, int sign) {
  const int center = bridge.lattice().size() / 2;
  bridge.inject_particle(center, center, center,
                         static_cast<std::int8_t>(sign), {});
  const int index = bridge.lattice().index(center, center, center);
  bridge.voxels()[static_cast<std::size_t>(index)].locked = true;
}

void seed_pair(ftd::RenderBridge& bridge, int axis, int orientation,
               int separation) {
  const int center = bridge.lattice().size() / 2;
  std::array<int, 3> low{{center, center, center}};
  std::array<int, 3> high{{center, center, center}};
  low[static_cast<std::size_t>(axis)] -= separation / 2;
  high[static_cast<std::size_t>(axis)] += separation / 2;
  const int low_sign = orientation > 0 ? +1 : -1;
  const int high_sign = -low_sign;
  bridge.inject_particle(low[0], low[1], low[2],
                         static_cast<std::int8_t>(low_sign), {});
  bridge.inject_particle(high[0], high[1], high[2],
                         static_cast<std::int8_t>(high_sign), {});
  bridge.voxels()[static_cast<std::size_t>(bridge.lattice().index(
      low[0], low[1], low[2]))].locked = true;
  bridge.voxels()[static_cast<std::size_t>(bridge.lattice().index(
      high[0], high[1], high[2]))].locked = true;
}

std::array<double, 2> travelling_component(double phase, double omega) {
  const double sine = std::sin(phase);
  const double cosine = std::cos(phase);
  return {{kWaveAmplitude * sine,
           kWaveAmplitude * ((1.0 - std::cos(omega)) * sine
                             - std::sin(omega) * cosine)}};
}

void seed_longitudinal_wave(ftd::RenderBridge& bridge, int axis) {
  const int size = bridge.lattice().size();
  const double k = 2.0 * kPi * static_cast<double>(kMode)
      / static_cast<double>(size);
  const double omega = 2.0 * std::asin(
      ftd::C_WAVE * std::abs(std::sin(0.5 * k)));
  for (int index = 0; index < static_cast<int>(bridge.voxels().size());
       ++index) {
    const auto c = bridge.lattice().coord(index);
    const int coordinate = axis_coordinate(c, axis);
    const auto wave = travelling_component(
        k * static_cast<double>(coordinate) + kWavePhase, omega);
    auto& voxel = bridge.voxels()[static_cast<std::size_t>(index)];
    voxel.flux = axis_vector(axis, wave[0]);
    voxel.wave_vel = axis_vector(axis, wave[1]);
  }
}

const char* fixture_name(Fixture fixture) {
  return fixture == Fixture::SingleQuadratic ? "single_quadratic"
                                              : "pair_cubic";
}

struct ArmResult {
  int records = 0;
  double ledger_scale = 1.0;
  double worst_shadow_rel = 0.0;
  double worst_naive_identity_rel = 0.0;
  double worst_total_momentum = 0.0;
  double impulse_accumulated = 0.0;
  double particle_energy_delta = 0.0;
  double field_ledger_delta = 0.0;
  double reversal_residual = 0.0;
  bool finite = true;
  bool cpu = true;
};

ArmResult run_symmetric_arm(ftd::RenderBridge& bridge, int ticks) {
  ArmResult arm;
  arm.cpu = bridge.backend_kind() == ftd::Backend::Kind::Cpu;

  const int size = bridge.lattice().size();
  ftd::RenderBridge initial(size);
  configure(initial);
  copy_state(bridge, initial);

  ftd::Vec3 momentum{};
  const auto ledger0 = ftd::eft::measure_shadow_ledger(bridge);
  arm.ledger_scale = std::max(1.0,
      std::abs(static_cast<double>(ledger0.shadow)));
  const auto field_momentum0 = ftd::eft::central_field_momentum(bridge);
  const double particle_energy0 =
      ftd::eft::production_flat_energy_from_momentum(momentum);

  for (int tick = 0; tick < ticks; ++tick) {
    ftd::eft::advance_symmetric_half_tick(bridge, momentum);
    ++arm.records;
    const auto ledger = ftd::eft::measure_shadow_ledger(bridge);
    arm.worst_shadow_rel = std::max(arm.worst_shadow_rel,
        std::abs(static_cast<double>(ledger.shadow - ledger0.shadow))
            / arm.ledger_scale);
    arm.worst_naive_identity_rel = std::max(
        arm.worst_naive_identity_rel,
        std::abs(static_cast<double>(
            (ledger.naive - ledger0.naive)
            - (ledger0.counterterm - ledger.counterterm)))
            / arm.ledger_scale);
    const auto field_momentum = ftd::eft::central_field_momentum(bridge);
    arm.worst_total_momentum = std::max(arm.worst_total_momentum,
        ((field_momentum - field_momentum0) + momentum).mag());
    arm.finite = arm.finite && ledger.finite;
  }

  arm.impulse_accumulated = momentum.mag();
  arm.particle_energy_delta =
      ftd::eft::production_flat_energy_from_momentum(momentum)
      - particle_energy0;
  const auto ledger_end = ftd::eft::measure_shadow_ledger(bridge);
  arm.field_ledger_delta =
      static_cast<double>(ledger_end.shadow - ledger0.shadow);

  for (int tick = 0; tick < ticks; ++tick)
    ftd::eft::reverse_symmetric_half_tick(bridge, momentum);
  arm.reversal_residual = std::max(
      field_restore_residual(initial, bridge), momentum.mag());
  return arm;
}

struct ProductionControl {
  double ledger_scale = 1.0;
  double worst_invariant_rel = 0.0;
  double naive_excursion = 0.0;
  bool finite = true;
};

ProductionControl run_production_control(ftd::RenderBridge& bridge,
                                         int ticks) {
  ProductionControl control;
  ftd::Vec3 momentum{};
  const auto ledger0 = ftd::eft::measure_shadow_ledger(bridge);
  const long double invariant0 = ledger0.naive
      + ftd::eft::production_counterterm(bridge);
  control.ledger_scale = std::max(1.0,
      std::abs(static_cast<double>(invariant0)));
  for (int tick = 0; tick < ticks; ++tick) {
    ftd::eft::advance_production_ordering(bridge, momentum);
    const auto ledger = ftd::eft::measure_shadow_ledger(bridge);
    const long double invariant = ledger.naive
        + ftd::eft::production_counterterm(bridge);
    control.worst_invariant_rel = std::max(control.worst_invariant_rel,
        std::abs(static_cast<double>(invariant - invariant0))
            / control.ledger_scale);
    control.naive_excursion = std::max(control.naive_excursion,
        std::abs(static_cast<double>(ledger.naive - ledger0.naive)));
    control.finite = control.finite && ledger.finite;
  }
  return control;
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  std::cout << "FTD-0469 symmetric half-tick transaction energy gate v2\n";

  bool protocol_valid = true;
  bool closure_pass = true;
  double worst_shadow = 0.0;
  double worst_naive_identity = 0.0;
  double worst_momentum = 0.0;
  double worst_reversal = 0.0;

  int static_records = 0;
  for (int axis = 0; axis < 3; ++axis) {
    for (int orientation : {-1, +1}) {
      for (Fixture fixture : {Fixture::SingleQuadratic, Fixture::PairCubic}) {
        ftd::RenderBridge bridge(kStaticL);
        configure(bridge);
        if (fixture == Fixture::SingleQuadratic)
          seed_single(bridge, orientation);
        else
          seed_pair(bridge, axis, orientation, 6);
        seed_static_field(bridge, axis, fixture);
        const auto arm = run_symmetric_arm(bridge, kStaticTicks);
        static_records += arm.records;
        protocol_valid = protocol_valid && arm.finite && arm.cpu
            && arm.impulse_accumulated > kImpulseFloor;
        closure_pass = closure_pass
            && arm.worst_shadow_rel <= kGate
            && arm.worst_naive_identity_rel <= kGate
            && arm.worst_total_momentum <= kGate
            && arm.reversal_residual <= kGate;
        worst_shadow = std::max(worst_shadow, arm.worst_shadow_rel);
        worst_naive_identity = std::max(worst_naive_identity,
                                        arm.worst_naive_identity_rel);
        worst_momentum = std::max(worst_momentum, arm.worst_total_momentum);
        worst_reversal = std::max(worst_reversal, arm.reversal_residual);
        std::cout << "static,fixture," << fixture_name(fixture)
                  << ",axis," << axis
                  << ",orientation," << orientation
                  << ",ticks," << arm.records
                  << ",ledger_scale," << arm.ledger_scale
                  << ",worst_shadow_rel," << arm.worst_shadow_rel
                  << ",worst_naive_identity_rel,"
                  << arm.worst_naive_identity_rel
                  << ",worst_total_momentum," << arm.worst_total_momentum
                  << ",impulse," << arm.impulse_accumulated
                  << ",particle_energy_delta," << arm.particle_energy_delta
                  << ",field_ledger_delta," << arm.field_ledger_delta
                  << ",reversal_residual," << arm.reversal_residual
                  << '\n';
      }
    }
  }

  int dynamic_records = 0;
  for (int axis = 0; axis < 3; ++axis) {
    for (int orientation : {-1, +1}) {
      ftd::RenderBridge bridge(kDynamicL);
      configure(bridge);
      seed_pair(bridge, axis, orientation, 8);
      seed_longitudinal_wave(bridge, axis);
      const auto arm = run_symmetric_arm(bridge, kDynamicTicks);
      dynamic_records += arm.records;
      protocol_valid = protocol_valid && arm.finite && arm.cpu
          && arm.impulse_accumulated > kImpulseFloor;
      closure_pass = closure_pass
          && arm.worst_shadow_rel <= kGate
          && arm.worst_naive_identity_rel <= kGate
          && arm.worst_total_momentum <= kGate
          && arm.reversal_residual <= kGate;
      worst_shadow = std::max(worst_shadow, arm.worst_shadow_rel);
      worst_naive_identity = std::max(worst_naive_identity,
                                      arm.worst_naive_identity_rel);
      worst_momentum = std::max(worst_momentum, arm.worst_total_momentum);
      worst_reversal = std::max(worst_reversal, arm.reversal_residual);
      std::cout << "dynamic,axis," << axis
                << ",orientation," << orientation
                << ",ticks," << arm.records
                << ",ledger_scale," << arm.ledger_scale
                << ",worst_shadow_rel," << arm.worst_shadow_rel
                << ",worst_naive_identity_rel," << arm.worst_naive_identity_rel
                << ",worst_total_momentum," << arm.worst_total_momentum
                << ",impulse," << arm.impulse_accumulated
                << ",particle_energy_delta," << arm.particle_energy_delta
                << ",field_ledger_delta," << arm.field_ledger_delta
                << ",reversal_residual," << arm.reversal_residual
                << '\n';
    }
  }

  double worst_production_invariant = 0.0;
  for (int axis = 0; axis < 3; ++axis) {
    ftd::RenderBridge bridge(kStaticL);
    configure(bridge);
    seed_pair(bridge, axis, +1, 6);
    seed_static_field(bridge, axis, Fixture::PairCubic);
    const auto control = run_production_control(bridge, kStaticTicks);
    protocol_valid = protocol_valid && control.finite;
    closure_pass = closure_pass && control.worst_invariant_rel <= kGate;
    worst_production_invariant = std::max(worst_production_invariant,
                                          control.worst_invariant_rel);
    std::cout << "production_control,axis," << axis
              << ",ledger_scale," << control.ledger_scale
              << ",worst_invariant_rel," << control.worst_invariant_rel
              << ",naive_excursion," << control.naive_excursion
              << '\n';
  }

  std::string verdict;
  if (!protocol_valid) verdict = "PROTOCOL_INVALID";
  else if (closure_pass)
    verdict = "SYMMETRIC_HALF_TICK_SHADOW_ENERGY_EXACT";
  else verdict = "SYMMETRIC_HALF_TICK_ENERGY_FAILS";
  std::cout << "summary,static_records," << static_records
            << ",dynamic_records," << dynamic_records
            << ",worst_shadow_rel," << worst_shadow
            << ",worst_naive_identity_rel," << worst_naive_identity
            << ",worst_total_momentum," << worst_momentum
            << ",worst_reversal," << worst_reversal
            << ",worst_production_invariant_rel," << worst_production_invariant
            << ",valid," << (protocol_valid ? "true" : "false") << '\n';
  std::cout << "verdict," << verdict << '\n';
  return protocol_valid ? 0 : 1;
}
