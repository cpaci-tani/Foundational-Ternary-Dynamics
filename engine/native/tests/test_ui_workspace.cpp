#include "ui/workspace.h"

#include "ftd/test_telemetry.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>

int main() {
    ftd::test::init("test_ui_workspace");

    const auto dir = std::filesystem::temp_directory_path() / "ftd_ui_workspace_l1";
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    const auto path = (dir / "experiment.workspace").string();

    ftd::test::section("round-trip");
    auto state = ftd::native::default_workspace(
        ftd::native::WorkspaceKind::Experiment);
    state.theme_name = "Contrast";
    state.imgui_ini = "[Window][Scenarios###scenarios]\nPos=0,0\n";
    ftd::test::check("save ok", ftd::native::save_workspace_file(path, state));
    const auto loaded = ftd::native::load_workspace_file(
        path, ftd::native::WorkspaceKind::Experiment);
    ftd::test::check("load ok", loaded.ok && !loaded.used_fallback);
    ftd::test::check("theme round-trips", loaded.state.theme_name == "Contrast");
    ftd::test::check("open-set includes scenarios",
                     std::find(loaded.state.open_ids.begin(), loaded.state.open_ids.end(),
                               "scenarios")
                         != loaded.state.open_ids.end());
    ftd::test::check("setup tab round-trips", loaded.state.setup_tab == "scenarios");
    ftd::test::check("ini dump round-trips",
                     loaded.state.imgui_ini.find("Scenarios###scenarios")
                         != std::string::npos);

    ftd::test::section("corrupt file falls back to the built-in recipe");
    {
        std::ofstream out(path, std::ios::binary | std::ios::trunc);
        out << "this is not a workspace\nkind=Nope\n";
    }
    const auto corrupt = ftd::native::load_workspace_file(
        path, ftd::native::WorkspaceKind::Experiment);
    ftd::test::check("corrupt uses fallback", corrupt.used_fallback);
    ftd::test::check("corrupt is not ok", !corrupt.ok);
    ftd::test::check("fallback still Experiment",
                     corrupt.state.kind
                         == ftd::native::WorkspaceKind::Experiment);
    ftd::test::check("fallback keeps default open-set",
                     !corrupt.state.open_ids.empty());

    ftd::test::section("setup host docking is read from the ini section");
    ftd::test::check(
        "missing setup window is not docked",
        !ftd::native::setup_host_is_docked_in_ini("[Window][scenarios]\n"));
    ftd::test::check(
        "floating setup is not docked",
        !ftd::native::setup_host_is_docked_in_ini(
            "[Window][setup]\nPos=73,187\nSize=276,1213\nCollapsed=0\n"));
    ftd::test::check(
        "setup with DockId is docked",
        ftd::native::setup_host_is_docked_in_ini(
            "[Window][setup]\nPos=0,21\nSize=336,819\nDockId=0x00000009,0\n"));

    std::filesystem::remove_all(dir, ec);
    return ftd::test::finalize();
}
