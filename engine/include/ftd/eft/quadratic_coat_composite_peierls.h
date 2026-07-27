#pragma once
/**
 * @file quadratic_coat_composite_peierls.h
 * @brief Observer-only rigid-composite Peierls analysis (FTD-0553).
 */

#include "ftd/eft/quadratic_coat_orbit_gather.h"

#include <cstddef>
#include <utility>
#include <vector>

namespace ftd::eft {

struct QuadraticCompositeConstituent {
  Coord offset{};
  int polarity = 0;
};

struct QuadraticCompositePeierlsSample {
  double fraction = 0.0;
  double spectral_energy = 0.0;
  double poisson_energy = 0.0;
  double predicted_energy = 0.0;
  double poisson_residual = 0.0;
  double gauss_residual = 0.0;
  double curl_residual = 0.0;
  double neutrality_residual = 0.0;
  double partition_residual = 0.0;
  double first_moment_residual = 0.0;
  double spectral_poisson_residual = 0.0;
  double quartic_residual = 0.0;
};

struct QuadraticCompositePeierlsWork {
  double fraction_before = 0.0;
  double fraction_after = 0.0;
  double field_energy_change = 0.0;
  double current_work = 0.0;
  double spectral_energy_change = 0.0;
  double net_force_component = 0.0;
  double continuity_residual = 0.0;
  double endpoint_density_residual = 0.0;
  double gather_adjoint_residual = 0.0;
  double field_work_residual = 0.0;
  double spectral_work_residual = 0.0;
};

struct QuadraticCompositePeierlsResult {
  bool valid = false;
  bool neutral = false;
  bool distinct_primitive_sites = false;
  bool axis_invariant = false;
  int L = 0;
  int axis = -1;
  int maximum_poisson_iterations = 0;
  std::size_t positive_spectral_terms = 0;
  Coord origin{};
  double beta = 0.0;
  double spectral_energy_zero = 0.0;
  double peierls_coefficient = 0.0;
  double barrier = 0.0;
  double maximum_identity_residual = 0.0;
  std::vector<QuadraticCompositeConstituent> constituents;
  std::vector<QuadraticCompositePeierlsSample> samples;
  std::vector<QuadraticCompositePeierlsWork> work_samples;
};

/**
 * Evaluate the exact finite-volume spectral law and an independent periodic
 * Poisson reconstruction for a rigid integer-offset composite. The common
 * subcell translation is along one coordinate axis. This observer never
 * mutates production state.
 */
QuadraticCompositePeierlsResult
evaluate_quadratic_composite_peierls(
    int L,
    const std::vector<QuadraticCompositeConstituent>& constituents,
    const Coord& origin,
    int axis,
    double beta,
    const std::vector<double>& fractions,
    const std::vector<std::pair<double, double>>& work_intervals,
    double poisson_tolerance = 1e-13,
    int poisson_max_iterations = 4096);

}  // namespace ftd::eft
