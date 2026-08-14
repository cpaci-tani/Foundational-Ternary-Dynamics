/** FTD-0883/0884 finite port Gauss battery EFT verifier. */

#include "ftd/eft/finite_port_gauss_battery.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {

int checks = 0;
int failures = 0;

void check(const std::string& label, bool condition) {
  ++checks;
  if (!condition) {
    ++failures;
    std::cerr << "FAIL  " << label << '\n';
  }
}

bool close(double first, double second, double tolerance = 1e-9) {
  return std::abs(first - second)
      <= tolerance * std::max({1.0, std::abs(first), std::abs(second)});
}

double maximum_face_magnitude(const ftd::eft::MatchedFaceFlux& field) {
  double result = 0.0;
  for (std::size_t index = 0; index < field.x.size(); ++index) {
    result = std::max({
        result,
        std::abs(field.x[index]),
        std::abs(field.y[index]),
        std::abs(field.z[index]),
    });
  }
  return result;
}

double maximum_port_magnitude(
    const std::vector<std::vector<double>>& ports) {
  double result = 0.0;
  for (const auto& port : ports) {
    for (double value : port) result = std::max(result, std::abs(value));
  }
  return result;
}

}  // namespace

int main() {
  using namespace ftd::eft;
  constexpr int L = 4;
  constexpr std::size_t capacity = 4;
  MatchedFaceFlux indexing(L);
  std::vector<double> charge(indexing.x.size(), 0.0);
  charge[static_cast<std::size_t>(indexing.index(0, 0, 0))] = +1.0;
  charge[static_cast<std::size_t>(indexing.index(1, 0, 0))] = -1.0;
  std::vector<double> battery(charge.size(), 10.0);
  for (std::size_t index = 1; index < battery.size(); index += 2) {
    battery[index] = -10.0;
  }

  FinitePortGaussBattery witness(L, charge, capacity, battery, 1e-12);
  check("finite port battery initializes", witness.valid()
      && witness.port_capacity() == capacity
      && witness.cursor() == 0
      && close(witness.field_energy(), 0.0)
      && close(witness.port_bank_energy(), 0.0));
  const double initial_total = witness.total_booked_energy();
  const auto initial_battery = witness.battery_amplitudes();

  for (std::size_t layer = 0; layer < capacity; ++layer) {
    const auto step = witness.step_fresh_layer();
    check("prepared bank supplies one fresh projected layer",
        step.valid()
        && step.fresh_port
        && step.layer.valid()
        && step.layer.active_affine_projection_exact
        && step.cursor_before == layer
        && step.cursor_after == (layer + 1U) % capacity);
    check("battery reserve and sign-preserving law close",
        step.reserve_checked_before_mutation
        && step.battery_sign_preserved
        && step.unique_sign_preserving_quadratic_update
        && step.full_state_inverse_available
        && close(step.battery_work_residual, 0.0, 1e-9));
    check("complete booked energy is conserved",
        close(step.combined_energy_residual, 0.0, 1e-9)
        && close(witness.total_booked_energy(), initial_total, 1e-9));
    check("scope firewall remains explicit",
        step.context_blind_cursor
        && !step.exact_real_memory_no_go_claimed
        && !step.canonical_hamiltonian_reservoir_supplied
        && !step.production_coupling_used
        && !step.born_target_used
        && !step.new_selected_type_added);
  }

  check("capacity layers consume all prepared ready coordinates",
      witness.accepted_layers() == capacity
      && witness.cursor() == 0
      && maximum_port_magnitude(witness.ports()) > 0.0);
  const auto field_before_capacity_failure = witness.flux();
  const auto ports_before_capacity_failure = witness.ports();
  const auto battery_before_capacity_failure = witness.battery_amplitudes();
  const double total_before_capacity_failure = witness.total_booked_energy();
  const auto exhausted = witness.step_fresh_layer();
  check("returning cursor fails the finite fresh-port gate",
      exhausted.status == FinitePortGaussBatteryStatus::NoFreshPort
      && witness.accepted_layers() == capacity
      && witness.cursor() == 0
      && close(witness.total_booked_energy(), total_before_capacity_failure));
  check("capacity failure mutates no field bank or battery",
      witness.flux().x == field_before_capacity_failure.x
      && witness.flux().y == field_before_capacity_failure.y
      && witness.flux().z == field_before_capacity_failure.z
      && witness.ports() == ports_before_capacity_failure
      && witness.battery_amplitudes() == battery_before_capacity_failure);

  while (witness.accepted_layers() > 0) {
    check("stored signed bank step reverses", witness.reverse_last_layer());
  }
  check("complete field bank battery and cursor recover",
      witness.cursor() == 0
      && maximum_face_magnitude(witness.flux()) <= 1e-9
      && maximum_port_magnitude(witness.ports()) <= 1e-9
      && close(witness.total_booked_energy(), initial_total, 1e-9));
  bool batteries_recovered = true;
  for (std::size_t index = 0; index < battery.size(); ++index) {
    batteries_recovered &= close(
        witness.battery_amplitudes()[index], initial_battery[index], 1e-9);
  }
  check("signed quadratic batteries recover exactly", batteries_recovered);
  check("reference scope does not overclaim",
      !witness.finite_cyclic_indefinite_freshness()
      && !witness.exact_real_memory_no_go_claimed()
      && witness.imposed_battery_law()
      && !witness.canonical_hamiltonian_reservoir_supplied()
      && !witness.moving_source_continuity_supplied()
      && !witness.production_coupling_supplied()
      && !witness.native_gstar_synchronization_supplied()
      && !witness.born_weights_used()
      && !witness.new_selected_type_added());

  std::vector<double> depleted_battery(charge.size(), 0.01);
  FinitePortGaussBattery depleted(
      L, charge, capacity, depleted_battery, 1e-12);
  const auto depletion = depleted.step_fresh_layer();
  check("insufficient positive reserve fails before mutation",
      depletion.status == FinitePortGaussBatteryStatus::BatteryReserveDepleted
      && depletion.cursor_before == 0
      && depleted.cursor() == 0
      && depleted.accepted_layers() == 0
      && maximum_face_magnitude(depleted.flux()) == 0.0
      && maximum_port_magnitude(depleted.ports()) == 0.0
      && depleted.battery_amplitudes() == depleted_battery);

  check("zero capacity fails closed",
      FinitePortGaussBattery(L, charge, 0, battery).status()
          == FinitePortGaussBatteryStatus::InvalidCapacity);
  check("battery shape mismatch fails closed",
      FinitePortGaussBattery(L, charge, capacity, std::vector<double>(1, 1.0)).status()
          == FinitePortGaussBatteryStatus::BatteryShapeMismatch);
  auto empty_battery = battery;
  empty_battery[0] = 0.0;
  check("empty battery amplitude fails closed",
      FinitePortGaussBattery(L, charge, capacity, empty_battery).status()
          == FinitePortGaussBatteryStatus::EmptyBatteryAmplitude);
  auto nonfinite_battery = battery;
  nonfinite_battery[0] = std::numeric_limits<double>::infinity();
  check("nonfinite battery fails closed",
      FinitePortGaussBattery(L, charge, capacity, nonfinite_battery).status()
          == FinitePortGaussBatteryStatus::NonFiniteBattery);

  std::cout << "FTD-0883/0884 finite port Gauss battery EFT: "
            << (checks - failures) << '/' << checks << " PASS\n";
  std::cout << "fresh_layers=FINITE_CAPACITY\n"
            << "source_battery=POSITIVE_REVERSIBLE_IMPOSED\n"
            << "finite_indefinite_recycling=NO_IN_REGISTERED_CLASS\n"
            << "hamiltonian_production_gstar_born=OPEN_UNTOUCHED\n";
  return failures == 0 ? 0 : 1;
}

