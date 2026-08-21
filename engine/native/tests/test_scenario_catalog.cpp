// test_scenario_catalog.cpp — proves engine/native's OWN Scale-0 scenario
// catalog (native/scenario_catalog.h, ftd::native) stays set-equal with the
// canonical engine registry ftd::scale0_scenario_ids(). This is the guard that
// stops the self-contained native copy from silently drifting from the engine's
// real Scale-0 scenario set now that the native tree no longer depends on the
// untracked engine header engine/include/ftd/scenario_meta.h.
//
// Ported from the held-off engine/tests/test_scenario_meta.cpp, retargeted at
// the ftd::native catalog.

#include "native/scenario_catalog.h"   // ftd::native catalog under test
#include "ftd/scenarios.h"             // ftd::scale0_scenario_ids() (canonical)
#include "ftd/test_telemetry.h"

#include <cstdio>
#include <string>
#include <string_view>
#include <unordered_set>

int main() {
    ftd::test::init("test_scenario_catalog");

    const auto& ids = ftd::scale0_scenario_ids();
    const std::size_t catalog_n = ftd::native::scenario_meta_count();

    std::printf("[scenario_catalog] native catalog rows       = %zu\n", catalog_n);
    std::printf("[scenario_catalog] scale0_scenario_ids().size = %zu\n", ids.size());

    ftd::test::check("catalog count matches scale0_scenario_ids()",
                     catalog_n == ids.size());

    std::unordered_set<std::string> catalog_ids;
    for (const auto& row : ftd::native::SCENARIO_META) {
        ftd::test::check("id present", row.id && row.id[0]);
        ftd::test::check("title present", row.title && row.title[0]);
        ftd::test::check("category present", row.category && row.category[0]);
        ftd::test::check("scale is 0", row.scale == 0);
        ftd::test::check("admitted-behavioral",
                         std::string_view(row.admission_status)
                             == "admitted-behavioral");
        ftd::test::check("find round-trips",
                         ftd::native::find_scenario_meta(row.id) == &row);
        catalog_ids.emplace(row.id);
    }
    ftd::test::check("catalog ids are unique", catalog_ids.size() == catalog_n);

    // Set-equality, both directions.
    std::unordered_set<std::string> engine_ids(ids.begin(), ids.end());
    ftd::test::check("catalog covers every scale0 id (⊇)",
                     catalog_ids == engine_ids);

    std::size_t missing_from_catalog = 0;
    for (const auto& id : ids)
        if (!ftd::native::find_scenario_meta(id)) ++missing_from_catalog;
    ftd::test::check("no engine id missing from catalog",
                     missing_from_catalog == 0);

    std::size_t catalog_not_in_engine = 0;
    for (const auto& row : ftd::native::SCENARIO_META)
        if (engine_ids.find(row.id) == engine_ids.end()) ++catalog_not_in_engine;
    ftd::test::check("no catalog id absent from engine",
                     catalog_not_in_engine == 0);

    ftd::test::check("unknown id resolves to null",
                     ftd::native::find_scenario_meta("not-a-scenario") == nullptr);

    const bool set_equal =
        (catalog_ids == engine_ids) && (catalog_n == ids.size());
    std::printf("[scenario_catalog] SET-EQUALITY = %s (%zu scenarios)\n",
                set_equal ? "PASS" : "FAIL", catalog_n);

    return ftd::test::finalize();
}
