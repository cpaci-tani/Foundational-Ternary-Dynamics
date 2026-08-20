#include "native/command_queue.h"
#include "ui/command_palette.h"
#include "ui/panel_registry.h"

#include "ftd/test_telemetry.h"

#include <algorithm>
#include <atomic>
#include <string>
#include <variant>
#include <vector>

int main() {
    ftd::test::init("test_ui_command_palette");

    const auto registry = ftd::native::PanelRegistry::make_default();
    ftd::native::PaletteHostState host;
    host.paused = true;
    host.scenario = "s0-seed-hydrogen";
    host.theme_name = "Graphite";
    host.workspace = ftd::native::WorkspaceKind::Experiment;
    registry.for_each([&](const ftd::native::Panel& panel) {
        host.panel_open[panel.id()] = true;
        host.panel_floating[panel.id()] =
            panel.default_slot() == ftd::native::DockSlot::Viewport;
    });

    const auto catalog =
        ftd::native::build_phase3b_catalog(registry, host);

    ftd::test::section("Phase 3b catalog is panels + actions");
    bool saw_toggle = false;
    bool saw_field = false;
    bool saw_scenario = false;
    int panel_count = 0;
    int action_count = 0;
    std::vector<std::string> panel_ids;
    for (const auto& entry : catalog) {
        if (entry.kind == ftd::native::PaletteKind::Panel) {
            ++panel_count;
            panel_ids.push_back(entry.id);
        } else if (entry.kind == ftd::native::PaletteKind::Action) {
            ++action_count;
        } else if (entry.kind == ftd::native::PaletteKind::Toggle) {
            saw_toggle = true;
        } else if (entry.kind == ftd::native::PaletteKind::Field) {
            saw_field = true;
        } else if (entry.kind == ftd::native::PaletteKind::Scenario) {
            saw_scenario = true;
        }
    }
    ftd::test::check("every registered panel is indexed",
                     panel_count == static_cast<int>(registry.panels().size()));
    ftd::test::check("actions are indexed", action_count >= 10);
    ftd::test::check("toggles wait for Phase 4", !saw_toggle);
    ftd::test::check("fields wait for Phase 5", !saw_field);
    ftd::test::check("scenarios wait for Phase 7a", !saw_scenario);
    ftd::test::check("telemetry panel is indexed",
                     std::find(panel_ids.begin(), panel_ids.end(), "telemetry")
                         != panel_ids.end());

    auto find_id = [&](const std::string& id) -> const ftd::native::PaletteEntry* {
        for (const auto& entry : catalog) {
            if (entry.id == id) return &entry;
        }
        return nullptr;
    };
    const auto* pause = find_id("action.pause");
    const auto* resume = find_id("action.resume");
    ftd::test::check("pause action exists", pause != nullptr);
    ftd::test::check("resume action exists", resume != nullptr);
    ftd::test::check("pause is disabled while already paused",
                     pause && pause->state.find("disabled:") == 0);
    ftd::test::check("resume is enabled while paused",
                     resume && resume->state == "enabled");
    const auto* telemetry = find_id("telemetry");
    ftd::test::check("open docked panel is visible",
                     telemetry && telemetry->state == "visible");
    const auto* play = find_id("play_bar");
    ftd::test::check("open play bar is floating",
                     play && play->state == "floating");

    ftd::test::section("§4.3 ranking");
    const auto empty = ftd::native::rank_palette("", catalog);
    ftd::test::check("empty query keeps every entry", empty.size() == catalog.size());
    ftd::test::check("empty query puts actions before panels", [&] {
        bool saw_panel = false;
        for (const auto& match : empty) {
            if (match.entry.kind == ftd::native::PaletteKind::Panel) {
                saw_panel = true;
            } else if (match.entry.kind == ftd::native::PaletteKind::Action
                       && saw_panel) {
                return false;
            }
        }
        return true;
    }());

    const auto pause_q = ftd::native::rank_palette("pause", catalog);
    ftd::test::check("pause query returns matches", !pause_q.empty());
    ftd::test::check("exact-prefix pause action ranks first",
                     !pause_q.empty() && pause_q.front().entry.id == "action.pause"
                         && pause_q.front().prefix);

    std::vector<ftd::native::PaletteEntry> mixed;
    mixed.push_back({ftd::native::PaletteKind::Panel, "audit", "Audit",
                     "visible", ""});
    mixed.push_back({ftd::native::PaletteKind::Action, "action.apply",
                     "Apply lattice", "enabled", ""});
    mixed.push_back({ftd::native::PaletteKind::Panel, "telemetry",
                     "Telemetry", "hidden", ""});
    mixed.push_back({ftd::native::PaletteKind::Toggle, "toggles.x",
                     "Example toggle", "disabled", ""});
    const auto a_q = ftd::native::rank_palette("a", mixed);
    ftd::test::check("prefix 'a' leads with Apply then Audit",
                     a_q.size() >= 2 && a_q[0].prefix && a_q[1].prefix
                         && a_q[0].entry.id == "action.apply"
                         && a_q[1].entry.id == "audit");
    ftd::test::check("non-prefix 'a' matches stay after the prefix group", [&] {
        bool seen_non_prefix = false;
        for (const auto& match : a_q) {
            if (match.prefix && seen_non_prefix) return false;
            if (!match.prefix) seen_non_prefix = true;
            if (match.entry.id == "telemetry") return false;
        }
        return true;
    }());

    mixed.push_back({ftd::native::PaletteKind::Panel, "fields", "Fields",
                     "visible", ""});
    const auto tel = ftd::native::rank_palette("tel", mixed);
    ftd::test::check("fuzzy 'tel' matches Telemetry",
                     !tel.empty() && tel.front().entry.id == "telemetry");

    std::vector<ftd::native::PaletteEntry> tied;
    tied.push_back({ftd::native::PaletteKind::Action, "action.b", "Beta",
                    "enabled", ""});
    tied.push_back({ftd::native::PaletteKind::Action, "action.a", "Alpha",
                    "enabled", ""});
    tied.push_back({ftd::native::PaletteKind::Action, "action.c", "Alpha",
                    "enabled", ""});
    const auto alpha = ftd::native::rank_palette("alpha", tied);
    ftd::test::check("equal titles keep source order",
                     alpha.size() >= 2 && alpha[0].entry.id == "action.a"
                         && alpha[1].entry.id == "action.c");

    ftd::test::section("effects");
    ftd::native::CommandQueue queue;
    ftd::native::ViewChrome chrome;
    std::atomic<bool> paused{true};
    bool particles = true;
    bool quit = false;
    bool reset_camera = false;
    chrome.paused = &paused;
    chrome.particles = &particles;
    chrome.request_quit = &quit;
    chrome.reset_camera = &reset_camera;

    const auto resume_effect = ftd::native::effect_for(*resume);
    ftd::native::apply_palette_effect(resume_effect, host, queue, &chrome);
    const auto drained = queue.drain();
    ftd::test::check("resume pushes Run",
                     drained.size() == 1
                         && std::holds_alternative<ftd::native::Run>(
                             drained[0].command));
    ftd::test::check("resume flips the chrome pause flag", !paused.load());

    host.panel_open["log"] = false;
    const auto* log = find_id("log");
    ftd::test::check("closed panel reports hidden when catalog rebuilt", [&] {
        ftd::native::PaletteHostState closed = host;
        closed.panel_open["log"] = false;
        const auto rebuilt =
            ftd::native::build_phase3b_catalog(registry, closed);
        for (const auto& entry : rebuilt) {
            if (entry.id == "log") return entry.state == "hidden";
        }
        return false;
    }());
    ftd::test::check("show-panel effect names the panel",
                     log && ftd::native::effect_for(*log).kind
                         == ftd::native::PaletteEffectKind::ShowPanel
                     && ftd::native::effect_for(*log).arg == "log");

    const auto pause_effect = ftd::native::effect_for(*pause);
    ftd::test::check("disabled pause does not emit a command",
                     pause_effect.kind == ftd::native::PaletteEffectKind::None);

    (void)quit;
    return ftd::test::finalize();
}
