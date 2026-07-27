/**
 * @file campaign_common_action_kick_reciprocity.cpp
 * @brief FTD-0468 exact source-kick/common-action momentum reciprocity.
 */

#include "ftd/eft/fixed_j_recoil_capacity.h"
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
constexpr int kDynamicL = 33;
constexpr int kDynamicTicks = 64;
constexpr int kMode = 2;
constexpr double kAmplitude = 1e-3;
constexpr double kWaveAmplitude = 0.02;
constexpr double kWavePhase = 0.37;
constexpr double kPi = 3.141592653589793238462643383279502884;
constexpr double kGate = 1e-12;
constexpr double kStaticNonzeroGate = 1e-14;
constexpr double kDynamicRmsGate = 1e-8;

enum class Fixture { SingleQuadratic, PairCubic };

void configure(ftd::RenderBridge& bridge, bool evolving) {
  bridge.force_cpu();
  bridge.toggles.disable_all();
  bridge.toggles.wave_propagation = evolving;
  bridge.toggles.coupling = evolving;
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

double component(const ftd::Vec3& value, int axis) {
  if (axis == 0) return value.x;
  if (axis == 1) return value.y;
  return value.z;
}

int periodic_offset(int coordinate, int center, int size) {
  int offset = coordinate - center;
  if (offset > size / 2) offset -= size;
  if (offset < -size / 2) offset += size;
  return offset;
}

int periodic_distance(int a, int b, int size) {
  const int direct = std::abs(a - b);
  return std::min(direct, size - direct);
}

void copy_state(const ftd::RenderBridge& source,
                ftd::RenderBridge& target) {
  for (std::size_t index = 0; index < source.voxels().size(); ++index)
    target.voxels()[index] = source.voxels()[index];
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

bool finite_vec(const ftd::Vec3& value) {
  return std::isfinite(value.x) && std::isfinite(value.y)
      && std::isfinite(value.z);
}

struct KickResult {
  ftd::Vec3 matter_impulse{};
  ftd::Vec3 measured_field_delta{};
  ftd::Vec3 direct_field_delta{};
  double closure_residual = 0.0;
  double formula_residual = 0.0;
  double inverse_residual = 0.0;
  double outside_face_support = 0.0;
  int support_sites = 0;
  bool finite = true;
};

bool face_neighbor_of_state(const ftd::RenderBridge& bridge, int index) {
  const auto c = bridge.lattice().coord(index);
  const int size = bridge.lattice().size();
  for (int source = 0; source < static_cast<int>(bridge.voxels().size());
       ++source) {
    if (bridge.voxels()[static_cast<std::size_t>(source)].state == 0) continue;
    const auto s = bridge.lattice().coord(source);
    const int dx = periodic_distance(c.x, s.x, size);
    const int dy = periodic_distance(c.y, s.y, size);
    const int dz = periodic_distance(c.z, s.z, size);
    if (dx + dy + dz == 1) return true;
  }
  return false;
}

KickResult evaluate_kick(const ftd::RenderBridge& before) {
  const int size = before.lattice().size();
  ftd::RenderBridge after(size), reverse(size);
  configure(after, false);
  configure(reverse, false);
  copy_state(before, after);

  KickResult result;
  for (int index = 0; index < static_cast<int>(before.voxels().size());
       ++index) {
    const auto gradient_state = ftd::gradient_state_op(
        before.voxels(), before.lattice(), index);
    const auto kick = gradient_state * -ftd::G_C;
    after.voxels()[static_cast<std::size_t>(index)].wave_vel += kick;
    if (kick.mag2() > 0.0) {
      ++result.support_sites;
      if (!face_neighbor_of_state(before, index))
        result.outside_face_support = std::max(
            result.outside_face_support, kick.mag());
    }

    const auto derivatives = ftd::eft::central_flux_derivatives(before,
                                                                 index);
    result.direct_field_delta.x -= kick.dot(derivatives[0]);
    result.direct_field_delta.y -= kick.dot(derivatives[1]);
    result.direct_field_delta.z -= kick.dot(derivatives[2]);

    const auto& voxel = before.voxels()[static_cast<std::size_t>(index)];
    if (voxel.state != 0) {
      result.matter_impulse += before.gradient_divergence(index)
          * (ftd::G_C * static_cast<double>(voxel.state));
    }
  }

  result.measured_field_delta = ftd::eft::central_field_momentum(after)
      - ftd::eft::central_field_momentum(before);
  result.closure_residual =
      (result.measured_field_delta + result.matter_impulse).mag();
  result.formula_residual =
      (result.measured_field_delta - result.direct_field_delta).mag();

  copy_state(after, reverse);
  for (int index = 0; index < static_cast<int>(before.voxels().size());
       ++index) {
    const auto kick = ftd::gradient_state_op(
        before.voxels(), before.lattice(), index) * -ftd::G_C;
    reverse.voxels()[static_cast<std::size_t>(index)].wave_vel -= kick;
  }
  result.inverse_residual = field_residual(before, reverse);
  result.finite = finite_vec(result.matter_impulse)
      && finite_vec(result.measured_field_delta)
      && finite_vec(result.direct_field_delta)
      && std::isfinite(result.closure_residual)
      && std::isfinite(result.formula_residual)
      && std::isfinite(result.inverse_residual)
      && std::isfinite(result.outside_face_support);
  return result;
}

void seed_static_field(ftd::RenderBridge& bridge, int axis,
                       Fixture fixture) {
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
      ftd::C_SPEED * std::abs(std::sin(0.5 * k)));
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

struct DynamicArm {
  int axis = 0;
  int orientation = 0;
  int records = 0;
  long double impulse_square_sum = 0.0L;
  double minimum_impulse = std::numeric_limits<double>::infinity();
  double maximum_impulse = 0.0;
  double worst_closure = 0.0;
  double worst_formula = 0.0;
  double worst_inverse = 0.0;
  double worst_outside = 0.0;
  int minimum_support = std::numeric_limits<int>::max();
  int maximum_support = 0;
  bool finite = true;
  bool cpu = true;

  double impulse_rms() const {
    return records > 0 ? std::sqrt(static_cast<double>(
        impulse_square_sum / static_cast<long double>(records))) : 0.0;
  }
};

DynamicArm run_dynamic_arm(int axis, int orientation) {
  ftd::RenderBridge bridge(kDynamicL);
  configure(bridge, true);
  seed_pair(bridge, axis, orientation, 8);
  seed_longitudinal_wave(bridge, axis);

  DynamicArm arm;
  arm.axis = axis;
  arm.orientation = orientation;
  arm.cpu = bridge.backend_kind() == ftd::Backend::Kind::Cpu;
  for (int tick = 0; tick < kDynamicTicks; ++tick) {
    const auto result = evaluate_kick(bridge);
    const double impulse = result.matter_impulse.mag();
    ++arm.records;
    arm.impulse_square_sum += static_cast<long double>(impulse)
        * static_cast<long double>(impulse);
    arm.minimum_impulse = std::min(arm.minimum_impulse, impulse);
    arm.maximum_impulse = std::max(arm.maximum_impulse, impulse);
    arm.worst_closure = std::max(
        arm.worst_closure, result.closure_residual);
    arm.worst_formula = std::max(
        arm.worst_formula, result.formula_residual);
    arm.worst_inverse = std::max(
        arm.worst_inverse, result.inverse_residual);
    arm.worst_outside = std::max(
        arm.worst_outside, result.outside_face_support);
    arm.minimum_support = std::min(arm.minimum_support, result.support_sites);
    arm.maximum_support = std::max(arm.maximum_support, result.support_sites);
    arm.finite = arm.finite && result.finite && std::isfinite(impulse);
    bridge.tick();
  }
  return arm;
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  std::cout << "FTD-0468 common-action kick reciprocity v1\n";

  struct StaticRow {
    int axis = 0;
    int orientation = 0;
    Fixture fixture = Fixture::SingleQuadratic;
    KickResult result;
  };
  std::vector<StaticRow> static_rows;
  for (int axis = 0; axis < 3; ++axis) {
    for (int sign : {-1, +1}) {
      ftd::RenderBridge bridge(kStaticL);
      configure(bridge, false);
      seed_single(bridge, sign);
      seed_static_field(bridge, axis, Fixture::SingleQuadratic);
      static_rows.push_back(
          {axis, sign, Fixture::SingleQuadratic, evaluate_kick(bridge)});
    }
    for (int orientation : {-1, +1}) {
      ftd::RenderBridge bridge(kStaticL);
      configure(bridge, false);
      seed_pair(bridge, axis, orientation, 6);
      seed_static_field(bridge, axis, Fixture::PairCubic);
      static_rows.push_back(
          {axis, orientation, Fixture::PairCubic, evaluate_kick(bridge)});
    }
  }

  std::vector<DynamicArm> dynamic_arms;
  for (int axis = 0; axis < 3; ++axis)
    for (int orientation : {-1, +1})
      dynamic_arms.push_back(run_dynamic_arm(axis, orientation));

  bool protocol_valid = true;
  bool closure_pass = true;
  double worst_closure = 0.0;
  double worst_formula = 0.0;
  double worst_inverse = 0.0;
  double worst_outside = 0.0;
  double worst_oddness = 0.0;
  double worst_covariance = 0.0;

  for (const auto& row : static_rows) {
    const auto& result = row.result;
    const double impulse = result.matter_impulse.mag();
    protocol_valid = protocol_valid && result.finite
        && impulse > kStaticNonzeroGate
        && result.formula_residual <= kGate
        && result.inverse_residual <= kGate
        && result.outside_face_support <= kGate
        && (row.fixture == Fixture::SingleQuadratic
                ? result.support_sites == 6 : result.support_sites == 12);
    closure_pass = closure_pass && result.closure_residual <= kGate;
    worst_closure = std::max(worst_closure, result.closure_residual);
    worst_formula = std::max(worst_formula, result.formula_residual);
    worst_inverse = std::max(worst_inverse, result.inverse_residual);
    worst_outside = std::max(worst_outside, result.outside_face_support);
    const double axial = component(result.matter_impulse, row.axis);
    worst_covariance = std::max(
        worst_covariance,
        (result.matter_impulse - axis_vector(row.axis, axial)).mag());
    std::cout << "static,fixture," << fixture_name(row.fixture)
              << ",axis," << row.axis
              << ",orientation," << row.orientation
              << ",matter_impulse," << axial
              << ",field_delta," << component(
                     result.measured_field_delta, row.axis)
              << ",closure_residual," << result.closure_residual
              << ",formula_residual," << result.formula_residual
              << ",inverse_residual," << result.inverse_residual
              << ",support_sites," << result.support_sites
              << ",outside_face_support," << result.outside_face_support
              << '\n';
  }

  for (int axis = 0; axis < 3; ++axis) {
    for (Fixture fixture : {Fixture::SingleQuadratic, Fixture::PairCubic}) {
      const StaticRow* minus = nullptr;
      const StaticRow* plus = nullptr;
      for (const auto& row : static_rows) {
        if (row.axis != axis || row.fixture != fixture) continue;
        if (row.orientation < 0) minus = &row;
        else plus = &row;
      }
      if (minus != nullptr && plus != nullptr)
        worst_oddness = std::max(
            worst_oddness,
            (minus->result.matter_impulse
             + plus->result.matter_impulse).mag());
    }
  }
  protocol_valid = protocol_valid && worst_covariance <= kGate
      && worst_oddness <= kGate;

  for (const auto& arm : dynamic_arms) {
    const bool arm_valid = arm.cpu && arm.finite
        && arm.records == kDynamicTicks
        && arm.impulse_rms() > kDynamicRmsGate
        && arm.worst_formula <= kGate
        && arm.worst_inverse <= kGate
        && arm.worst_outside <= kGate
        && arm.minimum_support == 12 && arm.maximum_support == 12;
    protocol_valid = protocol_valid && arm_valid;
    closure_pass = closure_pass && arm.worst_closure <= kGate;
    worst_closure = std::max(worst_closure, arm.worst_closure);
    worst_formula = std::max(worst_formula, arm.worst_formula);
    worst_inverse = std::max(worst_inverse, arm.worst_inverse);
    worst_outside = std::max(worst_outside, arm.worst_outside);
    std::cout << "dynamic,axis," << arm.axis
              << ",orientation," << arm.orientation
              << ",records," << arm.records
              << ",impulse_rms," << arm.impulse_rms()
              << ",minimum_impulse," << arm.minimum_impulse
              << ",maximum_impulse," << arm.maximum_impulse
              << ",worst_closure," << arm.worst_closure
              << ",worst_formula," << arm.worst_formula
              << ",worst_inverse," << arm.worst_inverse
              << ",support_min," << arm.minimum_support
              << ",support_max," << arm.maximum_support
              << ",outside_face_support," << arm.worst_outside
              << ",valid," << (arm_valid ? "true" : "false") << '\n';
  }

  std::string verdict;
  if (!protocol_valid) verdict = "PROTOCOL_INVALID";
  else if (closure_pass)
    verdict = "COMMON_ACTION_KICK_MOMENTUM_RECIPROCITY_EXACT";
  else verdict = "COMMON_ACTION_KICK_RECIPROCITY_FAILS";
  std::cout << "summary,static_records," << static_rows.size()
            << ",dynamic_records," << dynamic_arms.size() * kDynamicTicks
            << ",worst_closure," << worst_closure
            << ",worst_formula," << worst_formula
            << ",worst_inverse," << worst_inverse
            << ",worst_outside_face_support," << worst_outside
            << ",worst_oddness," << worst_oddness
            << ",worst_covariance," << worst_covariance
            << ",valid," << (protocol_valid ? "true" : "false") << '\n';
  std::cout << "verdict," << verdict << '\n';
  return protocol_valid ? 0 : 1;
}
