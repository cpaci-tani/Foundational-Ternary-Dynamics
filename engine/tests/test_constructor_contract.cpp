/**
 * @file test_constructor_contract.cpp
 * @brief Constructor-domain metadata helper smoke test.
 */

#include "ftd/test_telemetry.h"

int main() {
    ftd::test::init("test_constructor_contract");

    const ftd::test::ConstructorContract c{
        "metadata/constructor",
        "[SPECIFICATION]",
        "context_contrast, ontic_units, individuation, closure, admissibility",
        "observable_map, backend_policy",
        "constructor contract event",
        "test process stdout",
        "backend-independent",
        "contract metadata is complete and emit-able",
        "missing metadata means the test cannot be audited as constructor-domain evidence"};

    ftd::test::section("contract event");
    ftd::test::contract(c);
    ftd::test::check("valid contract passes completeness check",
                     ftd::test::valid_contract(c));

    ftd::test::ConstructorContract incomplete = c;
    incomplete.observable_map = "";
    ftd::test::check("missing observable map is invalid",
                     !ftd::test::valid_contract(incomplete));

    return ftd::test::finalize();
}
