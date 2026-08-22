#include "ftd/scenario_meta.h"
#include "ftd/scenarios.h"
#include "ftd/test_telemetry.h"

#include <string>
#include <string_view>
#include <unordered_set>

int main() {
    ftd::test::init("test_scenario_meta");

    const auto& ids = ftd::scale0_scenario_ids();
    ftd::test::check("meta count is 130", ftd::scenario_meta_count() == 130);
    ftd::test::check("id list count is 130", ids.size() == 130);
    ftd::test::check("meta count matches id list",
                     ftd::scenario_meta_count() == ids.size());

    std::unordered_set<std::string> meta_ids;
    for (const auto& row : ftd::SCENARIO_META) {
        ftd::test::check("id present", row.id && row.id[0]);
        ftd::test::check("title present", row.title && row.title[0]);
        ftd::test::check("category present", row.category && row.category[0]);
        ftd::test::check("scale is 0", row.scale == 0);
        ftd::test::check("min_lattice unconstrained until authored",
                         row.min_lattice == 0);
        ftd::test::check("admitted-behavioral",
                         std::string_view(row.admission_status)
                             == "admitted-behavioral");
        ftd::test::check("find round-trips",
                         ftd::find_scenario_meta(row.id) == &row);
        meta_ids.emplace(row.id);
    }
    ftd::test::check("meta ids are unique", meta_ids.size() == ids.size());

    std::unordered_set<std::string> cpp_ids(ids.begin(), ids.end());
    ftd::test::check("meta covers every scale0 id", meta_ids == cpp_ids);

    const ftd::ScenarioMeta* hydrogen = ftd::find_scenario_meta("s0-seed-hydrogen");
    ftd::test::check("hydrogen row exists", hydrogen != nullptr);
    ftd::test::check(
        "hydrogen title does not claim the atom",
        hydrogen && std::string_view(hydrogen->title).find("Hydrogen")
                       == std::string_view::npos);
    const ftd::ScenarioMeta* massive =
        ftd::find_scenario_meta("s0-seed-massive-body");
    ftd::test::check("massive-body row exists", massive != nullptr);
    ftd::test::check(
        "massive-body is category 5",
        massive
            && std::string_view(massive->category).find("5. Macroscopic") == 0);

    ftd::test::check("unknown id is null",
                     ftd::find_scenario_meta("not-a-scenario") == nullptr);
    return ftd::test::finalize();
}
