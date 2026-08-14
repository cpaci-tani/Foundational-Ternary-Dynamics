#pragma once

/**
 * @file reversible_checkerboard_gauss_preparation.h
 * @brief FTD-0881/0882 reversible local matched-Gauss preparation witness.
 *
 * On an even periodic matched-face probe, one cell parity at a time rotates
 * the local six-face Gauss residual into a signed environment port. Fresh
 * zero ports make each layer an affine orthogonal projection. Retaining every
 * outgoing port makes any finite history exactly reversible. This isolated
 * EFT witness does not modify production Voxel storage, Gauss projection, or
 * tick phases, and it does not synchronize to G* or read Born targets.
 */

#include "ftd/eft/matched_gauss_transport.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace ftd::eft {

enum class ReversibleCheckerboardGaussStatus : std::uint8_t {
  Valid = 0,
  InvalidSize,
  OddPeriodicSize,
  ShapeMismatch,
  IncompatibleCharge,
  NonFiniteInput,
  NoRetainedHistory,
};

struct ReversibleCheckerboardGaussLayer {
  ReversibleCheckerboardGaussStatus status =
      ReversibleCheckerboardGaussStatus::InvalidSize;
  int L = 0;
  int parity = 0;
  std::vector<double> incoming_environment;
  std::vector<double> outgoing_environment;
  std::size_t active_cells = 0;
  double maximum_active_residual_before = 0.0;
  double maximum_active_residual_after = 0.0;
  double field_energy_before = 0.0;
  double field_energy_after = 0.0;
  double incoming_environment_energy = 0.0;
  double outgoing_environment_energy = 0.0;
  double source_work = 0.0;
  double energy_ledger_residual = 0.0;
  bool fresh_environment = false;
  bool active_affine_projection_exact = false;
  bool exact_inverse_formula = false;
  bool disjoint_checkerboard_support = false;
  bool six_face_local = false;
  bool pseudoinverse_used = false;
  bool born_target_used = false;
  bool production_coupling_used = false;
  bool new_selected_type_added = false;

  bool valid() const {
    return status == ReversibleCheckerboardGaussStatus::Valid;
  }
};

/** Apply one parity layer with an explicit environment input at every cell. */
ReversibleCheckerboardGaussLayer
apply_reversible_checkerboard_gauss_layer(
    MatchedFaceFlux& flux,
    const std::vector<double>& compatible_charge,
    int parity,
    const std::vector<double>& incoming_environment,
    double tolerance = 1e-12);

/** Reverse one retained layer and recover its incoming environment ports. */
ReversibleCheckerboardGaussStatus
reverse_reversible_checkerboard_gauss_layer(
    MatchedFaceFlux& flux,
    const std::vector<double>& compatible_charge,
    const ReversibleCheckerboardGaussLayer& layer,
    std::vector<double>* recovered_incoming_environment = nullptr,
    double tolerance = 1e-12);

double checkerboard_gauss_residual_l2_squared(
    const MatchedFaceFlux& flux,
    const std::vector<double>& compatible_charge,
    int parity = -1);

double matched_face_difference_energy(
    const MatchedFaceFlux& first,
    const MatchedFaceFlux& second);

/**
 * Fresh-port preparation history. The mechanism starts from zero flux, uses
 * parity half_layer_count()%2, and retains every outgoing signed residual.
 */
class ReversibleCheckerboardGaussPreparation {
 public:
  ReversibleCheckerboardGaussPreparation() = default;
  ReversibleCheckerboardGaussPreparation(
      int size,
      const std::vector<double>& compatible_charge,
      double tolerance = 1e-12);

  ReversibleCheckerboardGaussStatus reset(
      int size,
      const std::vector<double>& compatible_charge,
      double tolerance = 1e-12);

  ReversibleCheckerboardGaussLayer step_fresh_layer();
  bool reverse_last_layer(double tolerance = 1e-10);

  bool valid() const {
    return status_ == ReversibleCheckerboardGaussStatus::Valid;
  }
  ReversibleCheckerboardGaussStatus status() const { return status_; }
  const MatchedFaceFlux& flux() const { return flux_; }
  const std::vector<double>& charge() const { return charge_; }
  const std::vector<ReversibleCheckerboardGaussLayer>& history() const {
    return history_;
  }
  std::size_t half_layer_count() const { return history_.size(); }
  double history_energy() const { return history_energy_; }
  double source_work() const { return source_work_; }
  double field_energy() const { return quadratic_energy(flux_); }
  double physical_balance_residual() const {
    return field_energy() + history_energy_ - source_work_;
  }
  double centered_energy(const MatchedFaceFlux& exact_record) const {
    return matched_face_difference_energy(flux_, exact_record)
        + history_energy_;
  }
  double maximum_gauss_residual() const;

  bool local_gate_only() const { return true; }
  bool finite_history_reversible() const { return true; }
  bool minimum_energy_record_is_limit() const { return true; }
  bool generic_fixed_finite_sweep_completion() const { return false; }
  bool limiting_field_history_energy_equal() const { return true; }
  bool environment_freshness_required() const { return true; }
  bool autonomous_environment_recycling_supplied() const { return false; }
  bool positive_source_reservoir_microdynamics_supplied() const { return false; }
  bool moving_source_continuity_supplied() const { return false; }
  bool production_coupling_supplied() const { return false; }
  bool native_gstar_synchronization_supplied() const { return false; }
  bool born_weights_used() const { return false; }
  bool new_selected_type_added() const { return false; }

 private:
  ReversibleCheckerboardGaussStatus status_ =
      ReversibleCheckerboardGaussStatus::InvalidSize;
  MatchedFaceFlux flux_;
  std::vector<double> charge_;
  std::vector<ReversibleCheckerboardGaussLayer> history_;
  double tolerance_ = 1e-12;
  double history_energy_ = 0.0;
  double source_work_ = 0.0;
};

}  // namespace ftd::eft
