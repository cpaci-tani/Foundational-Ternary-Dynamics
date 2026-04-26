/**
 * @file test_native_observable_registry.cpp
 * @brief Seed observable-registry contract test.
 */

#include "ftd/observable_registry.h"
#include "ftd/test_telemetry.h"

#include <string_view>

int main() {
    ftd::test::init("test_native_observable_registry");

    ftd::test::contract({
        "observable/registry",
        "[SPECIFICATION]",
        "observable_map, epistemic_tag, backend_policy",
        "sim::Observable descriptors, EFT ledger descriptors",
        "native_observable_registry()",
        "engine-wide descriptor catalogue",
        "backend-independent registry; individual entries declare support",
        "seed observables are present, unique, and fully described",
        "missing descriptor means later physics claims lack a stable observable map"});

    ftd::test::section("seed entries");
    ftd::test::check("observable names are unique",
                     ftd::native_observable_names_unique());

    const char* required[] = {
        "continuity_residual",
        "reaction_l1",
        "current_l1",
        "field_energy",
        "state_histogram",
        "flux_correlator",
        "blocked_operator_moments",
        "gauss_violation",
    };

    for (const char* name : required) {
        const auto* d = ftd::find_native_observable(name);
        ftd::test::check(name, d != nullptr);
        if (d) {
            ftd::test::check("descriptor has domain", d->domain && *d->domain);
            ftd::test::check("descriptor has source", d->source && *d->source);
            ftd::test::check("descriptor has units/dimension",
                             d->units_or_dimension && *d->units_or_dimension);
            ftd::test::check("descriptor has backend support",
                             d->backend_support && *d->backend_support);
            ftd::test::check("descriptor has epistemic tag",
                             d->epistemic_tag && *d->epistemic_tag);
        }
    }

    return ftd::test::finalize();
}
