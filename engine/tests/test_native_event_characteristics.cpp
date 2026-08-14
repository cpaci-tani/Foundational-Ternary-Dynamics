/** FTD-0858 isolated native event-acceptance/characteristic verifier. */

#include "ftd/eft/native_event_characteristics.h"
#include "ftd/eft/reciprocal_record_port.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string>

namespace {

using namespace ftd::eft;

int failures = 0;

void check(const std::string& label, bool condition) {
  if (condition) return;
  ++failures;
  std::cerr << "FAIL: " << label << '\n';
}

bool close(double first, double second, double tolerance = 1e-12) {
  return std::abs(first - second)
      <= tolerance * std::max({1.0, std::abs(first), std::abs(second)});
}

}  // namespace

int main() {
  NativeGenesisAcceptanceInput genesis;
  genesis.enabled = true;
  genesis.record = 0;
  genesis.common_magnitude = 3.0;
  genesis.genesis_threshold = 2.0;
  genesis.manifestation_scale = 0.5;
  genesis.uniform_draw = 0.1;
  const auto genesis_yes = evaluate_native_genesis_acceptance(genesis);
  check("genesis predicate is valid", genesis_yes.valid());
  check("genesis threshold reproduces source law",
      close(genesis_yes.acceptance_threshold, 1.0 - std::exp(-2.0)));
  check("genesis accepts a draw below the source threshold", genesis_yes.accepted);

  genesis.uniform_draw = 0.99;
  check("genesis rejects a draw above the source threshold",
      !evaluate_native_genesis_acceptance(genesis).accepted);
  genesis.common_magnitude = 2.0;
  genesis.uniform_draw = 0.0;
  check("genesis is strict above threshold",
      !evaluate_native_genesis_acceptance(genesis).accepted);
  genesis.common_magnitude = 3.0;
  genesis.record = 1;
  check("genesis requires a void record",
      !evaluate_native_genesis_acceptance(genesis).accepted);

  NativeEvaporationAcceptanceInput evaporation;
  evaporation.event_processing_enabled = true;
  evaporation.record = 1;
  evaporation.common_energy_site_plus_faces = 0.25;
  evaporation.manifestation_scale = 1.0;
  evaporation.evaporation_rate = 0.8;
  evaporation.proper_time_rate = 0.5;
  evaporation.uniform_draw = 0.1;
  const auto evaporation_yes =
      evaluate_native_evaporation_acceptance(evaporation);
  check("evaporation predicate is valid", evaporation_yes.valid());
  check("evaporation threshold reproduces source law",
      close(evaporation_yes.acceptance_threshold, std::exp(-0.25) * 0.4));
  check("evaporation accepts a draw below the source threshold",
      evaporation_yes.accepted);
  evaporation.locked = true;
  check("locked records do not evaporate",
      !evaluate_native_evaporation_acceptance(evaporation).accepted);
  evaporation.locked = false;
  evaporation.record = 0;
  check("void records do not evaporate",
      !evaluate_native_evaporation_acceptance(evaporation).accepted);

  check("ordered event none is retained",
      classify_ordered_native_event(false, false) == OrderedNativeEvent::None);
  check("ordered genesis is retained",
      classify_ordered_native_event(true, false) == OrderedNativeEvent::Genesis);
  check("ordered evaporation is retained",
      classify_ordered_native_event(false, true) == OrderedNativeEvent::Evaporation);
  check("same-tick genesis then evaporation is not collapsed",
      classify_ordered_native_event(true, true)
          == OrderedNativeEvent::GenesisThenEvaporation);

  const auto cr_a = common_relative_scalar(1.0, 1.0);
  const auto cr_b = common_relative_scalar(2.0, 0.0);
  check("common projection has a relative kernel",
      close(cr_a.common, cr_b.common) && !close(cr_a.relative, cr_b.relative));
  const auto bilateral = bilateral_scalar(cr_b.common, cr_b.relative);
  check("common-relative chart is invertible",
      close(bilateral.left, 2.0) && close(bilateral.right, 0.0));

  const auto chart = relative_characteristic_chart(1.25, -0.75);
  check("relative characteristic chart is valid", chart.valid());
  check("characteristic chart closes energy", close(chart.energy_residual, 0.0));
  check("characteristic chart closes signed current",
      close(chart.current_residual, 0.0));
  const double recovered_p =
      (chart.incoming + chart.outgoing) / std::sqrt(2.0);
  const double recovered_g =
      (chart.incoming - chart.outgoing) / std::sqrt(2.0);
  check("characteristic inverse recovers relative momentum",
      close(recovered_p, chart.relative_momentum));
  check("characteristic inverse recovers oriented strain",
      close(recovered_g, chart.oriented_strain));

  const auto orientation_reverse =
      relative_characteristic_chart(1.25, 0.75);
  check("spatial orientation reversal swaps the ports",
      close(orientation_reverse.incoming, chart.outgoing)
      && close(orientation_reverse.outgoing, chart.incoming));
  const auto time_reverse =
      relative_characteristic_chart(-1.25, -0.75);
  check("time reversal gives minus outgoing-incoming",
      close(time_reverse.incoming, -chart.outgoing)
      && close(time_reverse.outgoing, -chart.incoming));

  const double pi = std::acos(-1.0);
  const auto dispersion = production_axial_c18_dispersion(pi);
  check("production axial dispersion is valid", dispersion.valid());
  check("production c squared is one third",
      close(dispersion.wave_speed_squared, 1.0 / 3.0));
  check("production pi-mode eigenvalue is four thirds",
      close(dispersion.source_eigenvalue, 4.0 / 3.0));
  check("production determinant is one",
      close(dispersion.production_determinant, 1.0));
  check("one-cell shift trace defect is eight thirds at pi",
      close(dispersion.one_cell_shift_trace_defect, 8.0 / 3.0));

  ReciprocalRecordPortInput emission;
  emission.record = 1;
  emission.event_energy = 1.5;
  emission.eligibility = RecordPortEligibility::Exchange;
  const auto exchanged = scatter_reciprocal_record_port(emission);
  check("reference scatterer remains valid", exchanged.valid());
  check("reference scatterer signal work remains zero",
      close(exchanged.energy_residual, 0.0));

  NativeGenesisAcceptanceInput bad_genesis = genesis;
  bad_genesis.record = 0;
  bad_genesis.uniform_draw = 1.0;
  check("invalid genesis draw fails closed",
      evaluate_native_genesis_acceptance(bad_genesis).status
          == NativeEventCharacteristicStatus::InvalidDraw);
  NativeEvaporationAcceptanceInput bad_evaporation = evaporation;
  bad_evaporation.record = 1;
  bad_evaporation.proper_time_rate = 1.1;
  check("invalid proper-time rate fails closed",
      evaluate_native_evaporation_acceptance(bad_evaporation).status
          == NativeEventCharacteristicStatus::InvalidProperTimeRate);
  check("nonfinite characteristic coordinate fails closed",
      relative_characteristic_chart(
          std::numeric_limits<double>::infinity(), 0.0).status
          == NativeEventCharacteristicStatus::InvalidCoordinate);
  check("nonpositive wave speed fails closed",
      axial_c18_dispersion(0.2, 0.0).status
          == NativeEventCharacteristicStatus::InvalidWaveSpeed);

  std::cout << "FTD-0858 native event characteristics EFT: "
            << (failures == 0 ? "PASS" : "FAIL") << '\n';
  std::cout << "scope=SOURCE_ACCEPTANCE_PLUS_CHARACTERISTIC_CHART\n";
  std::cout << "common_to_relative_transducer=OPEN\n";
  std::cout << "protected_production_rail=OPEN\n";
  std::cout << "production_integration=NONE\n";
  return failures == 0 ? 0 : 1;
}

