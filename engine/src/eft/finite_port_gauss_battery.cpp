#include "ftd/eft/finite_port_gauss_battery.h"

#include <algorithm>
#include <cmath>
#include <numeric>

namespace ftd::eft {
namespace {

void coordinates(int L, int index, int& x, int& y, int& z) {
  z = index % L;
  const int xy = index / L;
  y = xy % L;
  x = xy / L;
}

int parity_of(int x, int y, int z) {
  return (x + y + z) & 1;
}

bool finite_values(const std::vector<double>& values) {
  return std::all_of(values.begin(), values.end(), [](double value) {
    return std::isfinite(value);
  });
}

double active_residual(
    const MatchedFaceFlux& flux,
    const std::vector<double>& charge,
    int x,
    int y,
    int z) {
  const int index = flux.index(x, y, z);
  return divergence_at(flux, x, y, z)
      - charge[static_cast<std::size_t>(index)];
}

double vector_port_energy(const std::vector<double>& values) {
  long double result = 0.0L;
  for (double value : values) {
    const long double amplitude = value;
    result += amplitude * amplitude / 12.0L;
  }
  return static_cast<double>(result);
}

bool close(double first, double second, double tolerance) {
  return std::abs(first - second)
      <= tolerance * std::max({1.0, std::abs(first), std::abs(second)});
}

}  // namespace

FinitePortGaussBattery::FinitePortGaussBattery(
    int size,
    const std::vector<double>& compatible_charge,
    std::size_t port_capacity,
    const std::vector<double>& battery_amplitudes,
    double tolerance) {
  reset(
      size, compatible_charge, port_capacity, battery_amplitudes, tolerance);
}

FinitePortGaussBatteryStatus FinitePortGaussBattery::reset(
    int size,
    const std::vector<double>& compatible_charge,
    std::size_t port_capacity,
    const std::vector<double>& battery_amplitudes,
    double tolerance) {
  flux_ = MatchedFaceFlux(size);
  charge_ = compatible_charge;
  ports_.clear();
  battery_ = battery_amplitudes;
  history_.clear();
  cursor_ = 0;
  tolerance_ = tolerance;

  const ReversibleCheckerboardGaussPreparation parent(
      size, compatible_charge, tolerance);
  if (!parent.valid()) {
    status_ = FinitePortGaussBatteryStatus::InvalidParent;
    return status_;
  }
  if (port_capacity == 0) {
    status_ = FinitePortGaussBatteryStatus::InvalidCapacity;
    return status_;
  }
  if (battery_.size() != charge_.size()) {
    status_ = FinitePortGaussBatteryStatus::BatteryShapeMismatch;
    return status_;
  }
  if (!std::isfinite(tolerance_) || tolerance_ <= 0.0
      || !finite_values(battery_)) {
    status_ = FinitePortGaussBatteryStatus::NonFiniteBattery;
    return status_;
  }
  if (std::any_of(battery_.begin(), battery_.end(), [this](double value) {
        return std::abs(value) <= tolerance_;
      })) {
    status_ = FinitePortGaussBatteryStatus::EmptyBatteryAmplitude;
    return status_;
  }
  ports_.assign(
      port_capacity, std::vector<double>(charge_.size(), 0.0));
  status_ = FinitePortGaussBatteryStatus::Valid;
  return status_;
}

double FinitePortGaussBattery::port_bank_energy() const {
  long double result = 0.0L;
  for (const auto& port : ports_) {
    result += vector_port_energy(port);
  }
  return static_cast<double>(result);
}

double FinitePortGaussBattery::battery_energy() const {
  long double result = 0.0L;
  for (double value : battery_) {
    const long double amplitude = value;
    result += amplitude * amplitude / 2.0L;
  }
  return static_cast<double>(result);
}

FinitePortGaussBatteryStep FinitePortGaussBattery::step_fresh_layer() {
  FinitePortGaussBatteryStep result;
  result.status = status_;
  result.context_blind_cursor = true;
  result.exact_real_memory_no_go_claimed = false;
  result.canonical_hamiltonian_reservoir_supplied = false;
  result.production_coupling_used = false;
  result.born_target_used = false;
  result.new_selected_type_added = false;
  if (!valid()) return result;

  result.cursor_before = cursor_;
  result.cursor_after = (cursor_ + 1U) % ports_.size();
  const auto incoming = ports_[cursor_];
  result.fresh_port = std::all_of(
      incoming.begin(), incoming.end(), [this](double value) {
        return std::abs(value) <= tolerance_;
      });
  if (!result.fresh_port) {
    result.status = FinitePortGaussBatteryStatus::NoFreshPort;
    return result;
  }

  const int parity = static_cast<int>(history_.size() & 1U);
  std::vector<double> work_by_cell(charge_.size(), 0.0);
  std::vector<double> radicands(charge_.size(), 0.0);
  for (int index = 0; index < flux_.L * flux_.L * flux_.L; ++index) {
    int x = 0, y = 0, z = 0;
    coordinates(flux_.L, index, x, y, z);
    if (parity_of(x, y, z) != parity) continue;
    const std::size_t offset = static_cast<std::size_t>(index);
    const double old_residual = active_residual(flux_, charge_, x, y, z);
    work_by_cell[offset] = charge_[offset]
        * (incoming[offset] - old_residual) / 6.0;
    radicands[offset] = battery_[offset] * battery_[offset]
        - 2.0 * work_by_cell[offset];
    if (!std::isfinite(radicands[offset])
        || radicands[offset] <= tolerance_ * tolerance_) {
      result.status = FinitePortGaussBatteryStatus::BatteryReserveDepleted;
      return result;
    }
  }
  result.reserve_checked_before_mutation = true;
  result.battery_energy_before = battery_energy();
  result.port_bank_energy_before = port_bank_energy();
  result.combined_energy_before = total_booked_energy();

  result.layer = apply_reversible_checkerboard_gauss_layer(
      flux_, charge_, parity, incoming, tolerance_);
  if (!result.layer.valid()) {
    status_ = FinitePortGaussBatteryStatus::ParentLayerFailure;
    result.status = status_;
    return result;
  }
  bool signs_preserved = true;
  for (int index = 0; index < flux_.L * flux_.L * flux_.L; ++index) {
    int x = 0, y = 0, z = 0;
    coordinates(flux_.L, index, x, y, z);
    if (parity_of(x, y, z) != parity) continue;
    const std::size_t offset = static_cast<std::size_t>(index);
    const double before = battery_[offset];
    battery_[offset] = std::copysign(std::sqrt(radicands[offset]), before);
    signs_preserved &= std::signbit(battery_[offset]) == std::signbit(before);
  }
  ports_[cursor_] = result.layer.outgoing_environment;
  cursor_ = result.cursor_after;

  result.battery_energy_after = battery_energy();
  result.port_bank_energy_after = port_bank_energy();
  result.combined_energy_after = total_booked_energy();
  result.source_work = std::accumulate(
      work_by_cell.begin(), work_by_cell.end(), 0.0);
  result.battery_work_residual = result.battery_energy_after
      - result.battery_energy_before + result.source_work;
  result.combined_energy_residual = result.combined_energy_after
      - result.combined_energy_before;
  result.battery_sign_preserved = signs_preserved;
  result.unique_sign_preserving_quadratic_update = true;
  result.full_state_inverse_available = true;
  const double scale = std::max({
      1.0,
      std::abs(result.combined_energy_before),
      std::abs(result.combined_energy_after),
      std::abs(result.source_work),
  });
  if (!close(result.battery_work_residual, 0.0, 50.0 * tolerance_)
      || !close(result.combined_energy_residual, 0.0, 50.0 * tolerance_ * scale)) {
    status_ = FinitePortGaussBatteryStatus::ParentLayerFailure;
    result.status = status_;
    return result;
  }
  result.status = FinitePortGaussBatteryStatus::Valid;
  history_.push_back(result);
  return result;
}

bool FinitePortGaussBattery::reverse_last_layer(double tolerance) {
  if (!valid() || history_.empty()) {
    if (valid()) status_ = FinitePortGaussBatteryStatus::NoHistory;
    return false;
  }
  const auto step = history_.back();
  if (cursor_ != step.cursor_after || step.cursor_before >= ports_.size()) {
    status_ = FinitePortGaussBatteryStatus::HistoryMismatch;
    return false;
  }
  const auto outgoing = ports_[step.cursor_before];
  const int parity = step.layer.parity;
  std::vector<double> recovered(charge_.size(), 0.0);
  std::vector<double> prior_squared(charge_.size(), 0.0);
  for (int index = 0; index < flux_.L * flux_.L * flux_.L; ++index) {
    int x = 0, y = 0, z = 0;
    coordinates(flux_.L, index, x, y, z);
    if (parity_of(x, y, z) != parity) continue;
    const std::size_t offset = static_cast<std::size_t>(index);
    recovered[offset] = active_residual(flux_, charge_, x, y, z);
    const double work = charge_[offset]
        * (recovered[offset] + outgoing[offset]) / 6.0;
    prior_squared[offset] = battery_[offset] * battery_[offset] + 2.0 * work;
    if (!std::isfinite(prior_squared[offset])
        || prior_squared[offset] <= tolerance * tolerance) {
      status_ = FinitePortGaussBatteryStatus::HistoryMismatch;
      return false;
    }
  }

  std::vector<double> parent_recovered;
  if (reverse_reversible_checkerboard_gauss_layer(
          flux_, charge_, step.layer, &parent_recovered, tolerance)
      != ReversibleCheckerboardGaussStatus::Valid) {
    status_ = FinitePortGaussBatteryStatus::HistoryMismatch;
    return false;
  }
  for (int index = 0; index < flux_.L * flux_.L * flux_.L; ++index) {
    int x = 0, y = 0, z = 0;
    coordinates(flux_.L, index, x, y, z);
    if (parity_of(x, y, z) != parity) continue;
    const std::size_t offset = static_cast<std::size_t>(index);
    battery_[offset] = std::copysign(
        std::sqrt(prior_squared[offset]), battery_[offset]);
  }
  ports_[step.cursor_before] = std::move(parent_recovered);
  cursor_ = step.cursor_before;
  history_.pop_back();
  status_ = FinitePortGaussBatteryStatus::Valid;
  return true;
}

}  // namespace ftd::eft

