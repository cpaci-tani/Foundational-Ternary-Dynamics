#pragma once
/**
 * @file extended_source_peierls_scaling.h
 * @brief Observer-only spectral pinning analysis for extended sources (FTD-0555).
 */

#include "ftd/lattice.h"

#include <vector>

namespace ftd::eft {

enum class ExtendedPeierlsProfileKind {
  MonopoleBackground,
  Dipole,
};

struct ExtendedPeierlsProfile {
  ExtendedPeierlsProfileKind kind =
      ExtendedPeierlsProfileKind::MonopoleBackground;
  Coord displacement{};
};

struct BinomialEnvelopeDiagnostics {
  bool valid = false;
  int order = 0;
  int support = 0;
  int local_generation_steps_3d = 0;
  double partition_residual = 0.0;
  double mean_residual = 0.0;
  double variance_residual = 0.0;
  double fourier_residual = 0.0;
  double maximum_identity_residual = 0.0;
};

struct ExtendedPeierlsSample {
  bool valid = false;
  ExtendedPeierlsProfile profile{};
  double energy_zero = 0.0;
  double peierls_coefficient = 0.0;
  double half_cell_barrier = 0.0;
  double relative_barrier = 0.0;
  double spectral_average = 0.0;
  double spectral_identity_residual = 0.0;
  double scaled_energy_constant = 0.0;
  double scaled_barrier_constant = 0.0;
  double scaled_relative_constant = 0.0;
  double expected_energy_constant = 0.0;
  double expected_barrier_constant = 0.0;
  double expected_relative_constant = 0.0;
};

struct ExtendedSourcePeierlsResult {
  bool valid = false;
  bool support_does_not_wrap = false;
  int L = 0;
  int order = 0;
  int axis = -1;
  double beta = 0.0;
  BinomialEnvelopeDiagnostics envelope{};
  std::vector<ExtendedPeierlsSample> samples;
  double maximum_identity_residual = 0.0;
};

/**
 * Evaluate the exact FTD-0541 compact-coat energy and Peierls coefficient for
 * locally generated tensor-binomial source envelopes. This is a read-only
 * spectral observer and never mutates RenderBridge or production state.
 */
ExtendedSourcePeierlsResult evaluate_extended_source_peierls(
    int L,
    int order,
    int translation_axis,
    const std::vector<ExtendedPeierlsProfile>& profiles,
    double beta);

}  // namespace ftd::eft
