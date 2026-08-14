#pragma once

/**
 * @file finite_port_gauss_battery.h
 * @brief FTD-0883/0884 finite ready-port bank and positive battery witness.
 *
 * This isolated EFT reference couples the FTD-0882 reversible checkerboard
 * Gauss layer to an explicit finite cyclic bank of signed environment ports
 * and a sign-preserving quadratic source battery. A bank of capacity C gives
 * C fresh layers from its prepared all-zero state; it is not an indefinite
 * recycler. The battery law is imposed on existing continuous carrier types
 * and is not claimed to be a canonical Hamiltonian or production mechanism.
 */

#include "ftd/eft/reversible_checkerboard_gauss_preparation.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace ftd::eft {

enum class FinitePortGaussBatteryStatus : std::uint8_t {
  Valid = 0,
  InvalidParent,
  InvalidCapacity,
  BatteryShapeMismatch,
  NonFiniteBattery,
  EmptyBatteryAmplitude,
  NoFreshPort,
  BatteryReserveDepleted,
  ParentLayerFailure,
  NoHistory,
  HistoryMismatch,
};

struct FinitePortGaussBatteryStep {
  FinitePortGaussBatteryStatus status =
      FinitePortGaussBatteryStatus::InvalidParent;
  std::size_t cursor_before = 0;
  std::size_t cursor_after = 0;
  ReversibleCheckerboardGaussLayer layer;
  double battery_energy_before = 0.0;
  double battery_energy_after = 0.0;
  double port_bank_energy_before = 0.0;
  double port_bank_energy_after = 0.0;
  double combined_energy_before = 0.0;
  double combined_energy_after = 0.0;
  double source_work = 0.0;
  double battery_work_residual = 0.0;
  double combined_energy_residual = 0.0;
  bool fresh_port = false;
  bool reserve_checked_before_mutation = false;
  bool battery_sign_preserved = false;
  bool unique_sign_preserving_quadratic_update = false;
  bool full_state_inverse_available = false;
  bool context_blind_cursor = true;
  bool exact_real_memory_no_go_claimed = false;
  bool canonical_hamiltonian_reservoir_supplied = false;
  bool production_coupling_used = false;
  bool born_target_used = false;
  bool new_selected_type_added = false;

  bool valid() const { return status == FinitePortGaussBatteryStatus::Valid; }
};

class FinitePortGaussBattery {
 public:
  FinitePortGaussBattery() = default;
  FinitePortGaussBattery(
      int size,
      const std::vector<double>& compatible_charge,
      std::size_t port_capacity,
      const std::vector<double>& battery_amplitudes,
      double tolerance = 1e-12);

  FinitePortGaussBatteryStatus reset(
      int size,
      const std::vector<double>& compatible_charge,
      std::size_t port_capacity,
      const std::vector<double>& battery_amplitudes,
      double tolerance = 1e-12);

  FinitePortGaussBatteryStep step_fresh_layer();
  bool reverse_last_layer(double tolerance = 1e-10);

  bool valid() const { return status_ == FinitePortGaussBatteryStatus::Valid; }
  FinitePortGaussBatteryStatus status() const { return status_; }
  const MatchedFaceFlux& flux() const { return flux_; }
  const std::vector<double>& charge() const { return charge_; }
  const std::vector<std::vector<double>>& ports() const { return ports_; }
  const std::vector<double>& battery_amplitudes() const { return battery_; }
  const std::vector<FinitePortGaussBatteryStep>& history() const {
    return history_;
  }
  std::size_t cursor() const { return cursor_; }
  std::size_t port_capacity() const { return ports_.size(); }
  std::size_t accepted_layers() const { return history_.size(); }

  double field_energy() const { return quadratic_energy(flux_); }
  double port_bank_energy() const;
  double battery_energy() const;
  double total_booked_energy() const {
    return field_energy() + port_bank_energy() + battery_energy();
  }

  bool finite_cyclic_indefinite_freshness() const { return false; }
  bool exact_real_memory_no_go_claimed() const { return false; }
  bool imposed_battery_law() const { return true; }
  bool canonical_hamiltonian_reservoir_supplied() const { return false; }
  bool moving_source_continuity_supplied() const { return false; }
  bool production_coupling_supplied() const { return false; }
  bool native_gstar_synchronization_supplied() const { return false; }
  bool born_weights_used() const { return false; }
  bool new_selected_type_added() const { return false; }

 private:
  FinitePortGaussBatteryStatus status_ =
      FinitePortGaussBatteryStatus::InvalidParent;
  MatchedFaceFlux flux_;
  std::vector<double> charge_;
  std::vector<std::vector<double>> ports_;
  std::vector<double> battery_;
  std::vector<FinitePortGaussBatteryStep> history_;
  std::size_t cursor_ = 0;
  double tolerance_ = 1e-12;
};

}  // namespace ftd::eft

