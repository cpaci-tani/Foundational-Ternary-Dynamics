/**
 * @file campaign_link_action_work_compatibility.cpp
 * @brief FTD-0470 exact finite-link work versus centered site-gradient force.
 */

#include "ftd/eft/link_action_work.h"
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

constexpr int kPolynomialL = 17;
constexpr int kFourierL = 32;
constexpr int kDynamicL = 33;
constexpr int kDynamicTicks = 64;
constexpr int kDynamicMode = 2;
constexpr double kPolynomialAmplitude = 1e-3;
constexpr double kFourierAmplitude = 0.01;
constexpr double kWaveAmplitude = 0.02;
constexpr double kPhase = 0.37;
constexpr double kPi = 3.141592653589793238462643383279502884;
constexpr double kGate = 1e-12;
constexpr double kNonzeroGate = 1e-10;
constexpr double kDynamicMismatchGate = 1e-10;

enum class Potential { Affine, Quadratic, Cubic };

void configure(ftd::RenderBridge& bridge, bool evolving) {
  bridge.force_cpu();
  bridge.toggles.disable_all();
  bridge.toggles.wave_propagation = evolving;
  bridge.toggles.coupling = evolving;
  bridge.toggles.strict_validation = true;
}

int coordinate_component(const ftd::Coord& coordinate, int axis) {
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

const char* potential_name(Potential potential) {
  if (potential == Potential::Affine) return "affine";
  if (potential == Potential::Quadratic) return "quadratic";
  return "cubic";
}

void seed_polynomial_primitive(ftd::RenderBridge& bridge, int axis,
                               Potential potential) {
  const int size = bridge.lattice().size();
  const int center = size / 2;
  for (int index = 0; index < static_cast<int>(bridge.voxels().size());
       ++index) {
    const auto coordinate = bridge.lattice().coord(index);
    const double r = static_cast<double>(periodic_offset(
        coordinate_component(coordinate, axis), center, size));
    double primitive = 0.0;
    if (potential == Potential::Affine) {
      // D_c[(a/2) r^2] = a r.
      primitive = 0.5 * kPolynomialAmplitude * r * r;
    } else if (potential == Potential::Quadratic) {
      // D_c[(a/3)(r^3-r)] = a r^2.
      primitive = (kPolynomialAmplitude / 3.0) * (r * r * r - r);
    } else {
      // D_c[(a/4)r^4-(a/2)r^2] = a r^3.
      primitive = 0.25 * kPolynomialAmplitude * r * r * r * r
          - 0.5 * kPolynomialAmplitude * r * r;
    }
    bridge.voxels()[static_cast<std::size_t>(index)].flux =
        axis_vector(axis, primitive);
  }
}

void seed_fourier_primitive(ftd::RenderBridge& bridge, int axis, int mode) {
  const double k = 2.0 * kPi * static_cast<double>(mode)
      / static_cast<double>(bridge.lattice().size());
  const double denominator = std::sin(k);
  for (int index = 0; index < static_cast<int>(bridge.voxels().size());
       ++index) {
    const auto coordinate = bridge.lattice().coord(index);
    const double x = static_cast<double>(
        coordinate_component(coordinate, axis));
    // D_c[-A cos(kx+phase)/sin(k)] = A sin(kx+phase).
    const double primitive = -kFourierAmplitude
        * std::cos(k * x + kPhase) / denominator;
    bridge.voxels()[static_cast<std::size_t>(index)].flux =
        axis_vector(axis, primitive);
  }
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
  const double k = 2.0 * kPi * static_cast<double>(kDynamicMode)
      / static_cast<double>(size);
  const double omega = 2.0 * std::asin(
      ftd::C_SPEED * std::abs(std::sin(0.5 * k)));
  for (int index = 0; index < static_cast<int>(bridge.voxels().size());
       ++index) {
    const auto coordinate = bridge.lattice().coord(index);
    const double x = static_cast<double>(
        coordinate_component(coordinate, axis));
    const auto wave = travelling_component(k * x + kPhase, omega);
    auto& voxel = bridge.voxels()[static_cast<std::size_t>(index)];
    voxel.flux = axis_vector(axis, wave[0]);
    voxel.wave_vel = axis_vector(axis, wave[1]);
  }
}

struct PairSites {
  std::array<int, 2> index{};
  std::array<int, 2> charge{};
};

PairSites seed_pair(ftd::RenderBridge& bridge, int axis, int orientation,
                    int separation) {
  const int center = bridge.lattice().size() / 2;
  std::array<int, 3> low{{center, center, center}};
  std::array<int, 3> high{{center, center, center}};
  low[static_cast<std::size_t>(axis)] -= separation / 2;
  high[static_cast<std::size_t>(axis)] += separation / 2;
  PairSites result;
  result.charge = orientation > 0 ? std::array<int, 2>{{+1, -1}}
                                  : std::array<int, 2>{{-1, +1}};
  bridge.inject_particle(low[0], low[1], low[2],
                         static_cast<std::int8_t>(result.charge[0]), {});
  bridge.inject_particle(high[0], high[1], high[2],
                         static_cast<std::int8_t>(result.charge[1]), {});
  result.index[0] = bridge.lattice().index(low[0], low[1], low[2]);
  result.index[1] = bridge.lattice().index(high[0], high[1], high[2]);
  bridge.voxels()[static_cast<std::size_t>(result.index[0])].locked = true;
  bridge.voxels()[static_cast<std::size_t>(result.index[1])].locked = true;
  return result;
}

struct DynamicArm {
  int records = 0;
  long double exact_square_sum = 0.0L;
  long double defect_square_sum = 0.0L;
  double maximum_relative_defect = 0.0;
  double worst_link_residual = 0.0;
  int centered_matches = 0;
  bool finite = true;
  bool cpu = true;

  double exact_rms() const {
    return records > 0 ? std::sqrt(static_cast<double>(
        exact_square_sum / static_cast<long double>(records))) : 0.0;
  }

  double defect_rms() const {
    return records > 0 ? std::sqrt(static_cast<double>(
        defect_square_sum / static_cast<long double>(records))) : 0.0;
  }
};

DynamicArm run_dynamic_arm(int axis, int orientation) {
  ftd::RenderBridge bridge(kDynamicL);
  configure(bridge, true);
  const PairSites pair = seed_pair(bridge, axis, orientation, 8);
  seed_longitudinal_wave(bridge, axis);

  DynamicArm arm;
  arm.cpu = bridge.backend_kind() == ftd::Backend::Kind::Cpu;
  for (int tick = 0; tick < kDynamicTicks; ++tick) {
    for (int particle = 0; particle < 2; ++particle) {
      for (int direction : {-1, +1}) {
        const auto result = ftd::eft::measure_face_link_action_work(
            bridge, pair.index[static_cast<std::size_t>(particle)], axis,
            direction, pair.charge[static_cast<std::size_t>(particle)]);
        ++arm.records;
        arm.exact_square_sum +=
            static_cast<long double>(result.exact_work) * result.exact_work;
        arm.defect_square_sum += static_cast<long double>(
            result.centered_defect) * result.centered_defect;
        if (std::abs(result.exact_work) > kNonzeroGate) {
          arm.maximum_relative_defect = std::max(
              arm.maximum_relative_defect,
              std::abs(result.centered_defect / result.exact_work));
        }
        arm.worst_link_residual = std::max(arm.worst_link_residual,
                                            std::abs(result.link_residual));
        if (std::abs(result.centered_defect) <= kGate)
          ++arm.centered_matches;
        arm.finite = arm.finite && result.valid;
      }
    }
    bridge.tick();
  }
  return arm;
}

}  // namespace

int main() {
  std::cout << std::setprecision(17);
  std::cout << "FTD-0470 link-action work compatibility v1\n";

  bool protocol_valid = true;
  bool exact_operator_pass = true;
  bool centered_counterexample_pass = true;
  double worst_control_defect = 0.0;
  double worst_cubic_formula = 0.0;
  double worst_fourier_formula = 0.0;
  double worst_link_residual = 0.0;
  double maximum_fourier_relative_defect = 0.0;

  for (int axis = 0; axis < 3; ++axis) {
    for (int direction : {-1, +1}) {
      for (int charge : {-1, +1}) {
        for (Potential potential : {Potential::Affine,
                                    Potential::Quadratic,
                                    Potential::Cubic}) {
          ftd::RenderBridge bridge(kPolynomialL);
          configure(bridge, false);
          const int center = kPolynomialL / 2;
          bridge.inject_particle(center, center, center,
                                 static_cast<std::int8_t>(charge), {});
          const int source = bridge.lattice().index(center, center, center);
          bridge.voxels()[static_cast<std::size_t>(source)].locked = true;
          seed_polynomial_primitive(bridge, axis, potential);
          const auto result = ftd::eft::measure_face_link_action_work(
              bridge, source, axis, direction, charge);
          const double control_defect = potential == Potential::Cubic
              ? result.centered_defect - 1.5 * result.exact_work
              : result.centered_defect;
          if (potential == Potential::Cubic)
            worst_cubic_formula = std::max(worst_cubic_formula,
                                             std::abs(control_defect));
          else
            worst_control_defect = std::max(worst_control_defect,
                                             std::abs(control_defect));
          worst_link_residual = std::max(worst_link_residual,
                                          std::abs(result.link_residual));
          protocol_valid = protocol_valid && result.valid
              && bridge.backend_kind() == ftd::Backend::Kind::Cpu
              && std::abs(result.exact_work) > kNonzeroGate;
          exact_operator_pass = exact_operator_pass
              && std::abs(control_defect) <= kGate
              && std::abs(result.link_residual) <= kGate;
          if (potential == Potential::Cubic)
            centered_counterexample_pass = centered_counterexample_pass
                && std::abs(result.centered_defect) > kNonzeroGate;
          std::cout << "polynomial,potential," << potential_name(potential)
                    << ",axis," << axis
                    << ",direction," << direction
                    << ",charge," << charge
                    << ",exact_work," << result.exact_work
                    << ",centered_work," << result.centered_site_work
                    << ",defect," << result.centered_defect
                    << ",formula_residual," << control_defect
                    << ",link_residual," << result.link_residual << '\n';
        }
      }
    }
  }

  constexpr std::array<int, 6> kModes{{1, 2, 4, 8, 12, 15}};
  for (int axis = 0; axis < 3; ++axis) {
    for (int direction : {-1, +1}) {
      for (int charge : {-1, +1}) {
        for (int mode : kModes) {
          ftd::RenderBridge bridge(kFourierL);
          configure(bridge, false);
          const int center = kFourierL / 2;
          bridge.inject_particle(center, center, center,
                                 static_cast<std::int8_t>(charge), {});
          const int source = bridge.lattice().index(center, center, center);
          bridge.voxels()[static_cast<std::size_t>(source)].locked = true;
          seed_fourier_primitive(bridge, axis, mode);
          const auto result = ftd::eft::measure_face_link_action_work(
              bridge, source, axis, direction, charge);
          const double k = 2.0 * kPi * static_cast<double>(mode)
              / static_cast<double>(kFourierL);
          const double expected_ratio =
              std::cos(0.5 * k) * std::cos(0.5 * k);
          const double measured_ratio =
              result.centered_site_work / result.exact_work;
          const double formula_residual = result.centered_site_work
              - expected_ratio * result.exact_work;
          const double relative_defect = std::abs(
              result.centered_defect / result.exact_work);
          worst_fourier_formula = std::max(worst_fourier_formula,
                                            std::abs(formula_residual));
          maximum_fourier_relative_defect = std::max(
              maximum_fourier_relative_defect, relative_defect);
          worst_link_residual = std::max(worst_link_residual,
                                          std::abs(result.link_residual));
          protocol_valid = protocol_valid && result.valid
              && bridge.backend_kind() == ftd::Backend::Kind::Cpu
              && std::abs(result.exact_work) > kNonzeroGate;
          exact_operator_pass = exact_operator_pass
              && std::abs(formula_residual) <= kGate
              && std::abs(result.link_residual) <= kGate;
          centered_counterexample_pass = centered_counterexample_pass
              && (mode == 1 || relative_defect > kNonzeroGate);
          std::cout << "fourier,mode," << mode
                    << ",axis," << axis
                    << ",direction," << direction
                    << ",charge," << charge
                    << ",exact_work," << result.exact_work
                    << ",centered_work," << result.centered_site_work
                    << ",measured_ratio," << measured_ratio
                    << ",expected_ratio," << expected_ratio
                    << ",relative_defect," << relative_defect
                    << ",formula_residual," << formula_residual
                    << ",link_residual," << result.link_residual << '\n';
        }
      }
    }
  }

  bool dynamic_nontrivial = true;
  bool dynamic_mismatch = true;
  int dynamic_records = 0;
  double minimum_dynamic_exact_rms = std::numeric_limits<double>::infinity();
  double minimum_dynamic_defect_rms = std::numeric_limits<double>::infinity();
  double maximum_dynamic_relative_defect = 0.0;
  for (int axis = 0; axis < 3; ++axis) {
    for (int orientation : {-1, +1}) {
      const auto arm = run_dynamic_arm(axis, orientation);
      dynamic_records += arm.records;
      minimum_dynamic_exact_rms = std::min(minimum_dynamic_exact_rms,
                                            arm.exact_rms());
      minimum_dynamic_defect_rms = std::min(minimum_dynamic_defect_rms,
                                             arm.defect_rms());
      maximum_dynamic_relative_defect = std::max(
          maximum_dynamic_relative_defect, arm.maximum_relative_defect);
      worst_link_residual = std::max(worst_link_residual,
                                      arm.worst_link_residual);
      protocol_valid = protocol_valid && arm.finite && arm.cpu;
      dynamic_nontrivial = dynamic_nontrivial
          && arm.exact_rms() > kNonzeroGate;
      dynamic_mismatch = dynamic_mismatch
          && arm.defect_rms() > kDynamicMismatchGate;
      exact_operator_pass = exact_operator_pass
          && arm.worst_link_residual <= kGate;
      std::cout << "dynamic,axis," << axis
                << ",orientation," << orientation
                << ",records," << arm.records
                << ",exact_work_rms," << arm.exact_rms()
                << ",centered_defect_rms," << arm.defect_rms()
                << ",maximum_relative_defect,"
                << arm.maximum_relative_defect
                << ",centered_matches," << arm.centered_matches
                << ",worst_link_residual," << arm.worst_link_residual
                << '\n';
    }
  }

  std::string verdict;
  if (!protocol_valid || !dynamic_nontrivial) {
    verdict = "PROTOCOL_INVALID";
  } else if (exact_operator_pass && centered_counterexample_pass
             && dynamic_mismatch) {
    verdict = "SITE_GRADIENT_IS_IR_APPROXIMATION_LINK_DIFFERENCE_IS_EXACT";
  } else if (exact_operator_pass && !dynamic_mismatch) {
    verdict = "NATIVE_HISTORIES_ACCIDENTALLY_CLOSE_SITE_GRADIENT";
  } else {
    verdict = "LINK_ACTION_WORK_IDENTITY_FAILS";
  }

  std::cout << "summary,worst_control_defect," << worst_control_defect
            << ",worst_cubic_formula," << worst_cubic_formula
            << ",worst_fourier_formula," << worst_fourier_formula
            << ",worst_link_residual," << worst_link_residual
            << ",maximum_fourier_relative_defect,"
            << maximum_fourier_relative_defect
            << ",dynamic_records," << dynamic_records
            << ",minimum_dynamic_exact_rms," << minimum_dynamic_exact_rms
            << ",minimum_dynamic_defect_rms," << minimum_dynamic_defect_rms
            << ",maximum_dynamic_relative_defect,"
            << maximum_dynamic_relative_defect
            << ",valid," << (protocol_valid ? "true" : "false") << '\n';
  std::cout << "verdict," << verdict << '\n';
  return protocol_valid ? 0 : 1;
}
