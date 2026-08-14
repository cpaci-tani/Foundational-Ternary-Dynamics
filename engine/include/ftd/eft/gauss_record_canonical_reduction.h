#pragma once

/**
 * @file gauss_record_canonical_reduction.h
 * @brief Matched Gauss-record canonical reduction witness (FTD-0877/0880).
 *
 * This isolated EFT observer uses the already selected oriented-face
 * MatchedFaceFlux complex. It does not mutate production Voxel storage or
 * claim that the live central-difference/SOR Gauss pass is an exact projector.
 */

#include "ftd/eft/matched_gauss_transport.h"

#include <cstdint>
#include <vector>

namespace ftd::eft {

enum class GaussRecordReductionStatus : std::uint8_t {
  Valid = 0,
  InvalidSize,
  InvalidTolerance,
  ShapeMismatch,
  InvalidTernaryState,
  InvalidCoupling,
  NonFiniteInput,
  IncompatibleCharge,
  SolverFailure,
};

struct GaussRecordCanonicalDecomposition {
  GaussRecordReductionStatus status = GaussRecordReductionStatus::InvalidSize;
  int L = 0;
  MatchedFaceFlux flux;
  MatchedFaceFlux momentum;
  std::vector<double> charge;
  std::vector<double> charge_momentum;
  MatchedFaceFlux longitudinal_flux;
  MatchedFaceFlux longitudinal_momentum;
  MatchedFaceFlux transverse_flux;
  MatchedFaceFlux transverse_momentum;
  double maximum_flux_reconstruction_residual = 0.0;
  double maximum_momentum_reconstruction_residual = 0.0;
  double maximum_transverse_flux_divergence = 0.0;
  double maximum_transverse_momentum_divergence = 0.0;
  double longitudinal_transverse_pairing_residual = 0.0;
  bool charge_bracket_identity_on_mean_zero_space = false;
  bool canonical_split_verified = false;
  bool uniformly_local_charge_conjugate_supplied = false;
  bool production_gauss_projector_used = false;

  bool valid() const { return status == GaussRecordReductionStatus::Valid; }
};

struct StaticTernaryGaussRecord {
  GaussRecordReductionStatus status = GaussRecordReductionStatus::InvalidSize;
  int L = 0;
  std::vector<int> ternary_state;
  std::vector<double> compatible_charge;
  double coupling = 1.0;
  double mean_state = 0.0;
  MatchedFaceFlux flux;
  MatchedFaceFlux momentum;
  double maximum_gauss_residual = 0.0;
  bool neutral_without_background = false;
  bool background_subtracted = false;
  bool static_charge_momentum_zero = false;
  bool minimum_energy_longitudinal = false;
  bool dynamic_native_preparation_supplied = false;
  bool native_gstar_synchronization_supplied = false;

  bool valid() const { return status == GaussRecordReductionStatus::Valid; }
};

struct GaussRecordPreparationLedger {
  GaussRecordReductionStatus status = GaussRecordReductionStatus::InvalidSize;
  MatchedFaceFlux input;
  MatchedFaceFlux prepared;
  MatchedFaceFlux discarded_longitudinal_discrepancy;
  MatchedFaceFlux recovered;
  std::vector<double> target_charge;
  double maximum_target_residual = 0.0;
  double maximum_recovery_residual = 0.0;
  double maximum_discrepancy_curl_adjoint = 0.0;
  bool affine_projection_idempotent = false;
  bool reversible_with_discrepancy_ledger = false;
  bool reversible_without_discrepancy_ledger = false;
  bool environment_dynamics_supplied = false;

  bool valid() const { return status == GaussRecordReductionStatus::Valid; }
};

struct ProductionGaussSymbolBoundary {
  double central_composition_half_pi = -1.0;
  double sor_18_point_half_pi = -2.0;
  double central_composition_nyquist = 0.0;
  double sor_18_point_nyquist = -4.0;
  bool operators_match = false;
  bool exact_projector = false;
  bool finite_iteration_relaxation = true;
  bool default_manifested_site_skip = true;
};

GaussRecordCanonicalDecomposition decompose_matched_gauss_canonical(
    const MatchedFaceFlux& flux,
    const MatchedFaceFlux& momentum,
    double tolerance = 1e-11,
    int max_iterations = 0);

StaticTernaryGaussRecord make_static_ternary_gauss_record(
    int L,
    const std::vector<int>& ternary_state,
    double coupling = 1.0,
    double tolerance = 1e-11,
    int max_iterations = 0);

GaussRecordPreparationLedger prepare_matched_gauss_record(
    const MatchedFaceFlux& input,
    const std::vector<double>& target_charge,
    double tolerance = 1e-11,
    int max_iterations = 0);

double gauss_canonical_symplectic_pairing(
    const MatchedFaceFlux& delta_flux_first,
    const MatchedFaceFlux& delta_momentum_first,
    const MatchedFaceFlux& delta_flux_second,
    const MatchedFaceFlux& delta_momentum_second);

double reduced_gauss_symplectic_pairing(
    const GaussRecordCanonicalDecomposition& first,
    const GaussRecordCanonicalDecomposition& second);

ProductionGaussSymbolBoundary production_gauss_symbol_boundary();

}  // namespace ftd::eft
