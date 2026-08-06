#pragma once
/**
 * @file transported_chart_morphology.h
 * @brief Observer-only transported field morphology (FTD-0764/0766).
 */

#include "ftd/eft/state_only_matter_field_observer.h"

#include <array>
#include <complex>
#include <vector>

namespace ftd::eft {

struct TransportedChartMode {
  int nx = 0;
  int ny = 0;
  int nz = 0;
};

struct TransportedChartModeCoefficient {
  TransportedChartMode mode{};
  std::complex<double> actual{};
  std::complex<double> bound{};
  std::complex<double> residual{};
  std::complex<double> interference{};
  std::complex<double> near_residual{};
};

struct TransportedChartMorphologyOptions {
  int support_half_width = 4;
  int near_radius = 8;
  int outer_radius = 48;
  double wave_speed = C_SPEED;
  double dt = 1.0;
  double poisson_tolerance = 1e-13;
  int poisson_max_iterations = 4096;
  double gate_tolerance = 1e-12;
  // A zero vector disables the FTD-0766 longitudinal partition.  A nonzero
  // vector is normalized internally and defines positive/leading motion.
  Vec3 longitudinal_direction{};
  double longitudinal_dead_band = 0.5;
  std::vector<TransportedChartMode> modes;
};

struct ResidualLongitudinalPartition {
  double trailing = 0.0;
  double neutral = 0.0;
  double leading = 0.0;

  double total() const { return trailing+neutral+leading; }
  double asymmetry() const {
    const double directed=trailing+leading;
    return directed>0.0?(trailing-leading)/directed:0.0;
  }
};

struct TransportedChartMorphologyObservation {
  bool valid = false;
  bool state_only = true;
  bool observer_only_nonlocal_phase = true;
  int L = 0;
  Vec3 center{};
  Vec3 support_center{};
  Vec3 fractional_center_offset{};
  double actual_energy = 0.0;
  double bound_energy = 0.0;
  double residual_energy = 0.0;
  double interference_energy = 0.0;
  double near_residual_energy = 0.0;
  double outer_residual_energy = 0.0;
  Vec3 near_residual_first_moment{};
  Vec3 outer_residual_first_moment{};
  double near_residual_second_moment = 0.0;
  double outer_residual_second_moment = 0.0;
  double near_residual_rms_radius = 0.0;
  double outer_residual_rms_radius = 0.0;
  bool longitudinal_partition_enabled = false;
  Vec3 longitudinal_direction{};
  ResidualLongitudinalPartition near_longitudinal{};
  ResidualLongitudinalPartition outer_longitudinal{};
  double longitudinal_partition_residual = 0.0;
  double energy_reconstruction_residual = 0.0;
  double maximum_mode_reconstruction_residual = 0.0;
  double bound_gauss_residual = 0.0;
  std::vector<TransportedChartModeCoefficient> coefficients;
};

struct TransportedChartMorphologyComparison {
  bool valid = false;
  double actual_distance = 0.0;
  double bound_distance = 0.0;
  double residual_distance = 0.0;
  double near_residual_distance = 0.0;
  double actual_energy_ratio = 0.0;
  double bound_energy_ratio = 0.0;
  double residual_energy_ratio = 0.0;
  double near_residual_energy_ratio = 0.0;
  Vec3 near_first_moment_change{};
  Vec3 outer_first_moment_change{};
};

std::vector<TransportedChartMode> make_transport_modes(
    const std::array<std::array<int, 3>, 3>& basis,
    const std::vector<int>& harmonics = {1, 2, 4, 8, 16, 32});

TransportedChartMorphologyObservation
observe_transported_chart_morphology(
    const ConnectedMooreBlockState& state,
    const ConnectedMooreBlockOptions& action_options,
    const TransportedChartMorphologyOptions& observer_options);

TransportedChartMorphologyComparison compare_transported_chart_morphology(
    const TransportedChartMorphologyObservation& reference,
    const TransportedChartMorphologyObservation& later,
    double tolerance = 1e-12);

}  // namespace ftd::eft
