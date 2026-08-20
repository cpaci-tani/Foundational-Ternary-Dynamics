#include "ui/theme.h"

#include "ftd/test_telemetry.h"

#include "imgui.h"
#include "implot.h"

int main() {
    ftd::test::init("test_ui_theme_parse");

    ftd::test::section("empty input is rejected");
    const auto empty = ftd::native::parse_theme("");
    ftd::test::check("empty not ok", !empty.ok);
    ftd::test::check("empty error names empty input",
                     empty.error.find("empty") != std::string::npos);

    ftd::test::section("unknown key is rejected");
    const auto unknown = ftd::native::parse_theme("name = Graphite\nnope = 1\n");
    ftd::test::check("unknown not ok", !unknown.ok);
    ftd::test::check("unknown error names the key",
                     unknown.error.find("nope") != std::string::npos);

    ftd::test::section("Graphite tokens parse");
    const auto graphite = ftd::native::parse_theme(
        "name = Graphite\n"
        "surface_0 = #17181b\n"
        "accent = #5b8db8\n");
    ftd::test::check("graphite ok", graphite.ok);
    ftd::test::check("graphite name", graphite.theme.name == "Graphite");
    ftd::test::check("surface_0 red", graphite.theme.surface_0.r < 0.12f);
    ftd::test::check("accent blue component", graphite.theme.accent.b > 0.6f);

    ftd::test::section("apply_theme writes Graphite into a live context");
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ftd::native::apply_theme(ftd::native::make_graphite(), 1.0f);
    const ImVec4 bg = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
    ftd::test::check("WindowBg matches Graphite surface_0",
                     bg.x < 0.12f && bg.y < 0.12f && bg.z < 0.14f);
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    return ftd::test::finalize();
}
