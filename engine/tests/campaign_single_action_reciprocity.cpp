/**
 * @file campaign_single_action_reciprocity.cpp
 * @brief FTD-0467 common-action source/force reciprocity audit.
 */

#include "ftd/field_operators.h"
#include "ftd/lagrangian.h"
#include "ftd/render_bridge.h"
#include "ftd/render_bridge_phases.h"

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

constexpr int kL = 17;
constexpr int kCenter = kL / 2;
constexpr double kAmplitude = 1e-3;
constexpr double kBackground = 0.1;
constexpr double kGate = 1e-12;
constexpr double kPi = 3.141592653589793238462643383279502884;

enum class Fixture { Quadratic, Affine };
enum class Branch { Legacy, Emergent };

void configure(ftd::RenderBridge& bridge) {
  bridge.force_cpu();
  bridge.toggles.disable_all();
  bridge.toggles.forces = true;
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

int periodic_offset(int coordinate) {
  int offset = coordinate - kCenter;
  if (offset > kL / 2) offset -= kL;
  if (offset < -kL / 2) offset += kL;
  return offset;
}

double component(const ftd::Vec3& value, int axis) {
  if (axis == 0) return value.x;
  if (axis == 1) return value.y;
  return value.z;
}

double max_abs_component(const ftd::Vec3& value) {
  return std::max({std::abs(value.x), std::abs(value.y),
                   std::abs(value.z)});
}

void seed_fixture(ftd::RenderBridge& bridge, int axis, Fixture fixture) {
  for (int index = 0; index < static_cast<int>(bridge.voxels().size());
       ++index) {
    const auto coordinate = bridge.lattice().coord(index);
    const double r = static_cast<double>(
        periodic_offset(axis_coordinate(coordinate, axis)));
    const double value = fixture == Fixture::Quadratic
        ? kAmplitude * r * r
        : kBackground + kAmplitude * r;
    auto& voxel = bridge.voxels()[static_cast<std::size_t>(index)];
    voxel.flux = axis_vector(axis, value);
    voxel.wave_vel = {};
  }
}

ftd::Vec3 tier2_density_gradient(const ftd::RenderBridge& bridge,
                                 int index) {
  const auto c = bridge.lattice().coord(index);
  const auto density = [&](int x, int y, int z) {
    return bridge.voxels()[static_cast<std::size_t>(
        bridge.lattice().index(x, y, z))].density();
  };
  return {
      (density(c.x + 2, c.y, c.z) - density(c.x - 2, c.y, c.z)) * 0.25,
      (density(c.x, c.y + 2, c.z) - density(c.x, c.y - 2, c.z)) * 0.25,
      (density(c.x, c.y, c.z + 2) - density(c.x, c.y, c.z - 2)) * 0.25};
}

struct ForceRow {
  int axis = 0;
  int sign = 0;
  Fixture fixture = Fixture::Quadratic;
  Branch branch = Branch::Legacy;
  ftd::Vec3 grad_div;
  ftd::Vec3 grad_density;
  ftd::Vec3 action;
  ftd::Vec3 helper;
  ftd::Vec3 measured;
  ftd::Vec3 coded_expected;
  double formula_residual = 0.0;
  double action_residual = 0.0;
  bool cpu_backend = false;
};

ForceRow run_force_row(int axis, int sign, Fixture fixture, Branch branch) {
  ftd::RenderBridge bridge(kL);
  configure(bridge);
  bridge.toggles.emergent_forces = branch == Branch::Emergent;
  bridge.toggles.poisson_coulomb = false;
  bridge.inject_particle(kCenter, kCenter, kCenter,
                         static_cast<std::int8_t>(sign), {});
  const int probe = bridge.lattice().index(kCenter, kCenter, kCenter);
  bridge.voxels()[static_cast<std::size_t>(probe)].locked = true;
  seed_fixture(bridge, axis, fixture);

  ForceRow row;
  row.axis = axis;
  row.sign = sign;
  row.fixture = fixture;
  row.branch = branch;
  row.cpu_backend = bridge.backend_kind() == ftd::Backend::Kind::Cpu;
  row.grad_div = bridge.gradient_divergence(probe);
  row.grad_density = tier2_density_gradient(bridge, probe);
  row.action = row.grad_div * (ftd::G_C * static_cast<double>(sign));
  row.helper = ftd::coupling_force(static_cast<std::int8_t>(sign),
                                   row.grad_div);
  row.coded_expected = branch == Branch::Legacy
      ? row.grad_div * (-ftd::ALPHA * static_cast<double>(sign))
      : row.grad_density * (ftd::G_C * static_cast<double>(sign));

  ftd::phase_forces_build_color_cache(bridge);
  ftd::phase_forces_main_loop(bridge);
  row.measured = bridge.force_diag_at(probe).f_coulomb;
  row.formula_residual = (row.measured - row.coded_expected).mag();
  row.action_residual = (row.measured - row.action).mag();
  return row;
}

struct SourceResult {
  double source_residual = 0.0;
  long double adjoint_residual = 0.0L;
  bool cpu_backend = false;
};

SourceResult run_source_and_adjoint() {
  ftd::RenderBridge source(kL);
  configure(source);
  source.toggles.coupling = true;
  source.inject_particle(kCenter, kCenter, kCenter, +1, {});
  const int center = source.lattice().index(kCenter, kCenter, kCenter);
  source.voxels()[static_cast<std::size_t>(center)].locked = true;
  ftd::phase_read_main_loop(source);
  ftd::phase_write_main_loop(source);

  SourceResult result;
  result.cpu_backend =
      source.backend_kind() == ftd::Backend::Kind::Cpu;
  for (int index = 0; index < static_cast<int>(source.voxels().size());
       ++index) {
    const auto expected = ftd::gradient_state_op(
        source.voxels(), source.lattice(), index) * -ftd::G_C;
    result.source_residual = std::max(
        result.source_residual,
        (source.voxels()[static_cast<std::size_t>(index)].wave_vel
         - expected).mag());
  }

  ftd::RenderBridge adjoint(kL);
  configure(adjoint);
  result.cpu_backend = result.cpu_backend
      && adjoint.backend_kind() == ftd::Backend::Kind::Cpu;
  adjoint.inject_particle(kCenter, kCenter, kCenter, +1, {});
  adjoint.voxels()[static_cast<std::size_t>(center)].locked = true;
  for (int index = 0; index < static_cast<int>(adjoint.voxels().size());
       ++index) {
    const auto c = adjoint.lattice().coord(index);
    const double x = 2.0 * kPi * static_cast<double>(c.x) / kL;
    const double y = 2.0 * kPi * static_cast<double>(c.y) / kL;
    const double z = 2.0 * kPi * static_cast<double>(c.z) / kL;
    adjoint.voxels()[static_cast<std::size_t>(index)].flux = {
        0.011 * std::sin(x) + 0.007 * std::cos(y),
        0.013 * std::sin(y) - 0.005 * std::cos(z),
        0.017 * std::sin(z) + 0.003 * std::cos(x)};
  }
  long double field_pairing = 0.0L;
  long double interaction = 0.0L;
  for (int index = 0; index < static_cast<int>(adjoint.voxels().size());
       ++index) {
    const auto source_value = ftd::gradient_state_op(
        adjoint.voxels(), adjoint.lattice(), index) * -ftd::G_C;
    field_pairing += static_cast<long double>(
        adjoint.voxels()[static_cast<std::size_t>(index)].flux.dot(
            source_value));
    interaction += static_cast<long double>(ftd::G_C)
        * static_cast<long double>(
            adjoint.voxels()[static_cast<std::size_t>(index)].state)
        * static_cast<long double>(adjoint.divergence_flux(index));
  }
  result.adjoint_residual = std::abs(field_pairing - interaction);
  return result;
}

struct PoissonResult {
  int axis = 0;
  double j_independence_residual = 0.0;
  double quadratic_action_residual = 0.0;
  double affine_action_residual = 0.0;
  bool cpu_backend = false;
};

ftd::Vec3 run_poisson_fixture(int axis, Fixture fixture,
                              ftd::Vec3& action,
                              bool& cpu_backend) {
  ftd::RenderBridge bridge(kL);
  configure(bridge);
  cpu_backend = bridge.backend_kind() == ftd::Backend::Kind::Cpu;
  bridge.toggles.poisson_coulomb = true;
  bridge.inject_particle(kCenter, kCenter, kCenter, +1, {});
  std::array<int, 3> neutralizer{{kCenter, kCenter, kCenter}};
  neutralizer[static_cast<std::size_t>(axis)] += 6;
  bridge.inject_particle(neutralizer[0], neutralizer[1], neutralizer[2],
                         -1, {});
  const int probe = bridge.lattice().index(kCenter, kCenter, kCenter);
  bridge.voxels()[static_cast<std::size_t>(probe)].locked = true;
  seed_fixture(bridge, axis, fixture);
  action = bridge.gradient_divergence(probe) * ftd::G_C;
  ftd::phase_forces_solve_potentials(bridge);
  ftd::phase_forces_build_color_cache(bridge);
  ftd::phase_forces_main_loop(bridge);
  return bridge.force_diag_at(probe).f_coulomb;
}

PoissonResult run_poisson_axis(int axis) {
  ftd::Vec3 quadratic_action;
  ftd::Vec3 affine_action;
  bool quadratic_cpu = false;
  bool affine_cpu = false;
  const auto quadratic = run_poisson_fixture(
      axis, Fixture::Quadratic, quadratic_action, quadratic_cpu);
  const auto affine = run_poisson_fixture(axis, Fixture::Affine,
                                          affine_action, affine_cpu);
  PoissonResult result;
  result.axis = axis;
  result.j_independence_residual = (quadratic - affine).mag();
  result.quadratic_action_residual = (quadratic - quadratic_action).mag();
  result.affine_action_residual = (affine - affine_action).mag();
  result.cpu_backend = quadratic_cpu && affine_cpu;
  return result;
}

const char* fixture_name(Fixture fixture) {
  return fixture == Fixture::Quadratic ? "quadratic" : "affine";
}

const char* branch_name(Branch branch) {
  return branch == Branch::Legacy ? "legacy" : "emergent";
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  std::cout << "FTD-0467 single-action reciprocity v1\n";

  const auto source = run_source_and_adjoint();
  std::vector<ForceRow> rows;
  for (int axis = 0; axis < 3; ++axis)
    for (int sign : {-1, +1})
      for (Fixture fixture : {Fixture::Quadratic, Fixture::Affine})
        for (Branch branch : {Branch::Legacy, Branch::Emergent})
          rows.push_back(run_force_row(axis, sign, fixture, branch));

  std::array<PoissonResult, 3> poisson{};
  for (int axis = 0; axis < 3; ++axis)
    poisson[static_cast<std::size_t>(axis)] = run_poisson_axis(axis);

  double worst_formula = 0.0;
  double worst_covariance = 0.0;
  double worst_oddness = 0.0;
  bool finite = std::isfinite(source.source_residual)
      && std::isfinite(static_cast<double>(source.adjoint_residual));
  bool cpu_backend = source.cpu_backend;
  bool legacy_action = true;
  bool emergent_action = true;
  double helper_action_residual = 0.0;

  for (const auto& row : rows) {
    worst_formula = std::max(worst_formula, row.formula_residual);
    helper_action_residual = std::max(
        helper_action_residual, (row.helper - row.action).mag());
    finite = finite && std::isfinite(row.formula_residual)
        && std::isfinite(row.action_residual);
    cpu_backend = cpu_backend && row.cpu_backend;
    if (row.branch == Branch::Legacy)
      legacy_action = legacy_action && row.action_residual <= kGate;
    else
      emergent_action = emergent_action && row.action_residual <= kGate;

    const double axial = component(row.measured, row.axis);
    const auto axial_vector = axis_vector(row.axis, axial);
    worst_covariance = std::max(
        worst_covariance, (row.measured - axial_vector).mag());
    std::cout << "force,axis," << row.axis
              << ",sign," << row.sign
              << ",fixture," << fixture_name(row.fixture)
              << ",branch," << branch_name(row.branch)
              << ",grad_div," << component(row.grad_div, row.axis)
              << ",grad_density," << component(row.grad_density, row.axis)
              << ",action," << component(row.action, row.axis)
              << ",helper," << component(row.helper, row.axis)
              << ",measured," << axial
              << ",formula_residual," << row.formula_residual
              << ",action_residual," << row.action_residual << '\n';
  }

  for (int axis = 0; axis < 3; ++axis) {
    for (Fixture fixture : {Fixture::Quadratic, Fixture::Affine}) {
      for (Branch branch : {Branch::Legacy, Branch::Emergent}) {
        const ForceRow* minus = nullptr;
        const ForceRow* plus = nullptr;
        for (const auto& row : rows) {
          if (row.axis != axis || row.fixture != fixture
              || row.branch != branch)
            continue;
          if (row.sign < 0) minus = &row;
          else plus = &row;
        }
        if (minus != nullptr && plus != nullptr)
          worst_oddness = std::max(
              worst_oddness, (minus->measured + plus->measured).mag());
      }
    }
  }

  bool poisson_independent = true;
  bool poisson_action = true;
  double worst_poisson_independence = 0.0;
  double min_poisson_action_residual =
      std::numeric_limits<double>::infinity();
  for (const auto& result : poisson) {
    worst_poisson_independence = std::max(
        worst_poisson_independence, result.j_independence_residual);
    min_poisson_action_residual = std::min(
        min_poisson_action_residual,
        std::min(result.quadratic_action_residual,
                 result.affine_action_residual));
    poisson_independent = poisson_independent
        && result.j_independence_residual <= kGate;
    poisson_action = poisson_action
        && result.quadratic_action_residual <= kGate
        && result.affine_action_residual <= kGate;
    finite = finite && std::isfinite(result.j_independence_residual)
        && std::isfinite(result.quadratic_action_residual)
        && std::isfinite(result.affine_action_residual);
    cpu_backend = cpu_backend && result.cpu_backend;
    std::cout << "poisson,axis," << result.axis
              << ",j_independence_residual,"
              << result.j_independence_residual
              << ",quadratic_action_residual,"
              << result.quadratic_action_residual
              << ",affine_action_residual,"
              << result.affine_action_residual << '\n';
  }

  const bool protocol_valid = finite && cpu_backend
      && source.source_residual <= kGate
      && source.adjoint_residual <= kGate
      && worst_formula <= kGate
      && worst_covariance <= kGate
      && worst_oddness <= kGate
      && poisson_independent;
  const bool action_found = legacy_action || emergent_action || poisson_action;
  std::string verdict;
  if (!protocol_valid) verdict = "PROTOCOL_INVALID";
  else if (action_found) verdict = "NATIVE_SINGLE_ACTION_RECIPROCITY_FOUND";
  else verdict = "NO_PRODUCTION_FORCE_BRANCH_IS_NATIVE_SINGLE_ACTION_PARTNER";

  std::cout << "summary,source_residual," << source.source_residual
            << ",adjoint_residual,"
            << static_cast<double>(source.adjoint_residual)
            << ",worst_formula_residual," << worst_formula
            << ",worst_covariance_residual," << worst_covariance
            << ",worst_oddness_residual," << worst_oddness
            << ",helper_action_residual," << helper_action_residual
            << ",poisson_j_independence_residual,"
            << worst_poisson_independence
            << ",min_poisson_action_residual,"
            << min_poisson_action_residual
            << ",legacy_action," << (legacy_action ? "true" : "false")
            << ",emergent_action," << (emergent_action ? "true" : "false")
             << ",poisson_action," << (poisson_action ? "true" : "false")
             << ",cpu_backend," << (cpu_backend ? "true" : "false")
             << ",valid," << (protocol_valid ? "true" : "false") << '\n';
  std::cout << "verdict," << verdict << '\n';
  return protocol_valid ? 0 : 1;
}
