/** FTD-0856 isolated reciprocal record-port reference verifier. */

#include "ftd/eft/reciprocal_record_port.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string>

namespace {

using ftd::eft::RecordPortEligibility;
using ftd::eft::RecordPortStatus;
using ftd::eft::ReciprocalRecordPortInput;
using ftd::eft::ReciprocalRecordPortResult;
using ftd::eft::scatter_reciprocal_record_port;

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

void check_invariants(
    const std::string& label, const ReciprocalRecordPortResult& result) {
  check(label + " valid", result.valid());
  check(label + " energy", close(result.energy_residual, 0.0));
  check(label + " signed content", close(result.signed_content_residual, 0.0));
}

}  // namespace

int main() {
  constexpr double B = 1.75;
  const double amplitude = std::sqrt(2.0 * B);

  for (std::int8_t sign : {std::int8_t{-1}, std::int8_t{1}}) {
    ReciprocalRecordPortInput hold;
    hold.record = sign;
    hold.event_energy = B;
    const auto held = scatter_reciprocal_record_port(hold);
    check_invariants("closed occupied", held);
    check("closed gate preserves record", held.record_after == sign);
    check("closed gate emits nothing", held.outgoing == 0.0);
    check("closed gate reports no exchange", !held.gate_exchanged);

    ReciprocalRecordPortInput emission = hold;
    emission.eligibility = RecordPortEligibility::Exchange;
    const auto emitted = scatter_reciprocal_record_port(emission);
    check_invariants("open emission", emitted);
    check("emission clears record", emitted.record_after == 0);
    check("emission writes normalized rail amplitude",
        close(emitted.outgoing, static_cast<double>(sign) * amplitude));
    check("emission reports exchange", emitted.gate_exchanged);

    ReciprocalRecordPortInput absorption;
    absorption.record = 0;
    absorption.incoming = static_cast<double>(sign) * amplitude;
    absorption.event_energy = B;
    absorption.eligibility = RecordPortEligibility::Exchange;
    const auto absorbed = scatter_reciprocal_record_port(absorption);
    check_invariants("open absorption", absorbed);
    check("absorption manifests signed record", absorbed.record_after == sign);
    check("absorption leaves no outgoing pulse", absorbed.outgoing == 0.0);

    ReciprocalRecordPortInput reflection = absorption;
    reflection.eligibility = RecordPortEligibility::Hold;
    const auto reflected = scatter_reciprocal_record_port(reflection);
    check_invariants("closed incident", reflected);
    check("closed incident does not actualize", reflected.record_after == 0);
    check("closed incident exits unchanged",
        close(reflected.outgoing, reflection.incoming));

    ReciprocalRecordPortInput reverse;
    reverse.record = emitted.record_after;
    reverse.incoming = emitted.outgoing;
    reverse.event_energy = B;
    reverse.eligibility = RecordPortEligibility::Exchange;
    const auto restored = scatter_reciprocal_record_port(reverse);
    check_invariants("exchange involution", restored);
    check("exchange involution restores record", restored.record_after == sign);
    check("exchange involution restores incoming", restored.outgoing == 0.0);
  }

  ReciprocalRecordPortInput simultaneous;
  simultaneous.record = 1;
  simultaneous.incoming = -amplitude;
  simultaneous.event_energy = B;
  simultaneous.eligibility = RecordPortEligibility::Exchange;
  const auto swapped = scatter_reciprocal_record_port(simultaneous);
  check_invariants("simultaneous signed swap", swapped);
  check("incoming sign becomes record", swapped.record_after == -1);
  check("old matter becomes outgoing", close(swapped.outgoing, amplitude));

  ReciprocalRecordPortInput invalid;
  check("zero event energy fails closed",
      scatter_reciprocal_record_port(invalid).status
          == RecordPortStatus::InvalidEnergy);
  invalid.event_energy = -1.0;
  check("negative event energy fails closed",
      scatter_reciprocal_record_port(invalid).status
          == RecordPortStatus::InvalidEnergy);
  invalid.event_energy = std::numeric_limits<double>::infinity();
  check("infinite event energy fails closed",
      scatter_reciprocal_record_port(invalid).status
          == RecordPortStatus::InvalidEnergy);
  invalid.event_energy = B;
  invalid.record = 2;
  check("nonternary record fails closed",
      scatter_reciprocal_record_port(invalid).status
          == RecordPortStatus::InvalidRecord);
  invalid.record = 0;
  invalid.incoming = 0.5 * amplitude;
  check("nonquantized incoming pulse fails closed",
      scatter_reciprocal_record_port(invalid).status
          == RecordPortStatus::InvalidIncomingAmplitude);
  invalid.incoming = 0.0;
  invalid.tolerance = -1.0;
  check("negative tolerance fails closed",
      scatter_reciprocal_record_port(invalid).status
          == RecordPortStatus::InvalidTolerance);
  invalid.tolerance = 1e-12;
  invalid.eligibility = static_cast<RecordPortEligibility>(7);
  check("invalid eligibility fails closed",
      scatter_reciprocal_record_port(invalid).status
          == RecordPortStatus::InvalidEligibility);

  std::cout << "FTD-0856 reciprocal record-port EFT: "
            << (failures == 0 ? "PASS" : "FAIL") << '\n';
  std::cout << "scope=SELECTED_REFERENCE_PHYSICAL_ELIGIBILITY_OPEN\n";
  std::cout << "production_integration=NONE\n";
  return failures == 0 ? 0 : 1;
}
