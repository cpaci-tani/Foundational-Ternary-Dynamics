#include "ftd/scale1/field_mediation_probe.h"

#include <cmath>
#include <iostream>
#include <limits>
#include <string>

int main() {
    using ftd::scale1::FieldMediationDispatch;
    using ftd::scale1::FieldMediationProbe;
    int failures = 0;
    const auto check = [&failures](const char* label, bool condition) {
        if (!condition) { ++failures; std::cerr << "FAIL: " << label << '\n'; }
    };

    FieldMediationProbe probe;
    std::string error;
    check("finite carrier dispatch succeeds",
          probe.dispatch({3, 4, 9, {0, 0, 0}, {2.1, -1, 0}, -2.0}, &error)
          && error.empty());
    check("carrier is booked in the field before arrival",
          probe.pending_count() == 1 && probe.field_energy() == 2.0
          && probe.delivered_energy() == 0.0 && probe.ledger_residual() == 0.0);
    check("radius-one Moore ceiling forbids early arrival",
          probe.advance_to(5).empty() && probe.pending_count() == 1);
    const auto arrivals = probe.advance_to(6);
    check("carrier arrives after ceil Linfinity distance ticks",
          arrivals.size() == 1 && arrivals[0].moore_distance == 3
          && arrivals[0].emission_tick == 3 && arrivals[0].arrival_tick == 6);
    check("delivery transfers the complete booked energy",
          probe.field_energy() == 0.0 && probe.delivered_energy() == 2.0
          && probe.delivered_count() == 1 && probe.ledger_residual() == 0.0);
    check("scope firewalls stay explicit",
          !arrivals[0].applied_to_particle_engine
          && !arrivals[0].reciprocal_recoil_supplied
          && !arrivals[0].photon_identified
          && !arrivals[0].born_weight_used
          && !FieldMediationProbe::production_coupling_supplied()
          && !FieldMediationProbe::qed_amplitude_supplied()
          && !FieldMediationProbe::moving_source_tracking_supplied());

    FieldMediationDispatch invalid{6, 1, 2, {}, {1, 0, 0},
        std::numeric_limits<double>::infinity()};
    check("nonfinite dispatch fails without mutation",
          !probe.dispatch(invalid, &error) && !error.empty()
          && probe.pending_count() == 0 && probe.dispatched_energy() == 2.0);
    check("observer clock is monotonic", probe.advance_to(5).empty()
          && probe.current_tick() == 6);

    std::cout << "Scale1 field-mediation observer: "
              << (failures ? "FAIL" : "PASS") << '\n';
    return failures;
}
