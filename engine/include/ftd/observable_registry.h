#pragma once
/**
 * @file observable_registry.h
 * @brief Seed registry for constructor-domain observable maps.
 *
 * This is a descriptor catalogue, not a second runtime observable system.
 * Runtime measurements remain in existing sources such as sim::Observable,
 * energy/continuity ledgers, and EFT operator-moment helpers. The registry
 * gives tests and campaigns stable names for the observable maps they claim.
 */

#include <string_view>
#include <vector>

namespace ftd {

struct ObservableDescriptor {
    const char* name;
    const char* domain;
    const char* source;
    const char* units_or_dimension;
    const char* backend_support;
    const char* epistemic_tag;
};

inline const std::vector<ObservableDescriptor>& native_observable_registry() {
    static const std::vector<ObservableDescriptor> registry = {
        {"continuity_residual",
         "transport/conservation",
         "ftd::eft::max_continuity_residual(DualCellContinuity)",
         "signed charge per blocked cell",
         "cpu+gpu-ledger",
         "[MEASUREMENT]"},
        {"reaction_l1",
         "reaction/conservation",
         "ftd::eft::total_reaction_l1(DualCellContinuity)",
         "signed charge L1",
         "cpu+gpu-ledger",
         "[MEASUREMENT]"},
        {"current_l1",
         "transport/continuity",
         "ftd::eft::total_current_l1(DualCellContinuity)",
         "oriented current L1",
         "cpu+gpu-ledger",
         "[MEASUREMENT]"},
        {"field_energy",
         "energy/observable",
         "EnergyAudit::field_energy / sim::FieldEnergyAudit",
         "lattice field-energy units",
         "cpu+gpu-sync",
         "[MEASUREMENT]"},
        {"state_histogram",
         "instantiation/observable",
         "sim::StateHistogram",
         "counts over {-1,0,+1}",
         "cpu+pipeline",
         "[MEASUREMENT]"},
        {"flux_correlator",
         "many-body/correlation",
         "sim::FluxCorrelator",
         "J dot J correlation",
         "cpu+pipeline",
         "[MEASUREMENT]"},
        {"blocked_operator_moments",
         "blocking/EFT",
         "ftd::eft::measure_operator_moments(DualCellContinuity)",
         "dimensionless blocked moments",
         "cpu+gpu-ledger",
         "[MEASUREMENT]"},
        {"gauss_violation",
         "constraint/gauge",
         "EnergyAudit::gauss_violation / EFT Gauss identity helpers",
         "divJ-rho residual",
         "cpu+gpu-sync",
         "[MEASUREMENT]"},
    };
    return registry;
}

inline const ObservableDescriptor* find_native_observable(std::string_view name) {
    for (const auto& d : native_observable_registry()) {
        if (std::string_view(d.name) == name) return &d;
    }
    return nullptr;
}

inline bool native_observable_names_unique() {
    const auto& registry = native_observable_registry();
    for (std::size_t i = 0; i < registry.size(); ++i) {
        for (std::size_t j = i + 1; j < registry.size(); ++j) {
            if (std::string_view(registry[i].name) == registry[j].name) {
                return false;
            }
        }
    }
    return true;
}

}  // namespace ftd
