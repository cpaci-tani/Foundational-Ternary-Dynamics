#pragma once

#include <algorithm>
#include <cstdint>

namespace ftd::native {

struct DataNeeds {
    std::uint32_t telemetry_groups = 0;
    bool energy_ledger = false;
    bool field_sample = false;
    bool spectrum = false;   // compute the flux E(k) this boundary (Spectrum panel)
    int history_depth = 0;
};

inline DataNeeds operator|(const DataNeeds& a, const DataNeeds& b) {
    DataNeeds out;
    out.telemetry_groups = a.telemetry_groups | b.telemetry_groups;
    out.energy_ledger = a.energy_ledger || b.energy_ledger;
    out.field_sample = a.field_sample || b.field_sample;
    out.spectrum = a.spectrum || b.spectrum;
    out.history_depth = std::max(a.history_depth, b.history_depth);
    return out;
}

}  // namespace ftd::native
