#include "native/command_queue.h"
#include "native/imgui_font.h"
#include "ui/ui_shell.h"

#include "ftd/test_telemetry.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "implot.h"

#include <atomic>
#include <filesystem>
#include <string>
#include <variant>

#ifdef _OPENMP
#include <omp.h>
#endif

int main() {
    ftd::test::init("test_ui_shell_draw");

#ifdef _OPENMP
    const int threads_before = omp_get_max_threads();
#endif

    const auto dir = std::filesystem::temp_directory_path() / "ftd_ui_shell_l1";
    std::error_code ec;
    std::filesystem::remove_all(dir, ec);
    std::filesystem::create_directories(dir, ec);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.DisplaySize = ImVec2(1920.0f, 1080.0f);
    io.DeltaTime = 1.0f / 60.0f;
    ftd::native::add_embedded_inter_font(io, 15.0f);
    unsigned char* pixels = nullptr;
    int w = 0;
    int h = 0;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &w, &h);
    ftd::test::check("font atlas built", pixels != nullptr && w > 0 && h > 0);

    ftd::native::UiShell shell(dir.string());
    ftd::native::CommandQueue commands;
    ftd::native::UiSnapshot snapshot;
    snapshot.frame.scenario = "s0-seed-hydrogen";
    snapshot.frame.tick = 7;
    snapshot.knobs.lattice_size = 32;
    snapshot.env.backend = ftd::native::BackendKindUi::Cpu;
    std::atomic<int> tick_hz{20};
    std::atomic<bool> paused{true};
    bool particles = true;
    bool flux = true;
    bool box = true;
    bool reset_camera = false;
    bool quit = false;
    ftd::native::ViewChrome chrome;
    chrome.particles = &particles;
    chrome.flux = &flux;
    chrome.lattice_box = &box;
    chrome.tick_hz = &tick_hz;
    chrome.paused = &paused;
    chrome.reset_camera = &reset_camera;
    chrome.request_quit = &quit;

    ImGui::NewFrame();
    shell.draw(snapshot, commands, chrome);
    ImGui::Render();

    ftd::test::check("Experiment workspace is default",
                     shell.workspace() == ftd::native::WorkspaceKind::Experiment);
    ftd::test::check("scenarios panel starts open", shell.panel_open("scenarios"));
    ftd::test::check("substrate panel starts open", shell.panel_open("substrate"));
    const ftd::native::Panel* scenarios = shell.registry().find("scenarios");
    ftd::test::check("registry has scenarios", scenarios != nullptr);
    const ImGuiWindow* setup = ImGui::FindWindowByName(
        ftd::native::kSetupHostWindowName);
    ftd::test::check("setup stack host was begun", setup != nullptr);
    ftd::test::check("setup stack is docked into the left node",
                     setup != nullptr && setup->DockIsActive);
    const std::string scenarios_name = ftd::native::window_name(*scenarios);
    ftd::test::check(
        "scenarios is hosted in the stack, not a free window",
        ImGui::FindWindowByName(scenarios_name.c_str()) == nullptr);
    const ftd::native::Panel* run = shell.registry().find("run_config");
    ftd::test::check("registry has run config", run != nullptr);
    ftd::test::check(
        "run config is hosted in the stack, not a free window",
        ImGui::FindWindowByName(ftd::native::window_name(*run).c_str())
            == nullptr);
    ftd::test::check("central scene rect is non-empty",
                     shell.scene_rect().width > 0 && shell.scene_rect().height > 0);
    ftd::test::check("play bar panel starts open", shell.panel_open("play_bar"));
    const ftd::native::Panel* play = shell.registry().find("play_bar");
    ftd::test::check("registry has play bar", play != nullptr);
    const ImGuiWindow* play_win = play ? ImGui::FindWindowByName(
        ftd::native::window_name(*play).c_str())
                                       : nullptr;
    ftd::test::check("play bar window was begun", play_win != nullptr);
    ftd::test::check("play bar floats over the viewport, not a dock",
                     play_win != nullptr && !play_win->DockIsActive);
    ftd::test::check(
        "play bar is not dockable",
        play_win != nullptr && (play_win->Flags & ImGuiWindowFlags_NoDocking) != 0);
    ftd::test::check("theme is Graphite", shell.theme().name == "Graphite");

    shell.open_palette();
    ImGui::NewFrame();
    shell.draw(snapshot, commands, chrome);
    ImGui::Render();
    ftd::test::check("open_palette marks the palette open", shell.palette_open());
    const ImGuiWindow* palette = ImGui::FindWindowByName("Command palette###palette");
    ftd::test::check("command palette popup was begun", palette != nullptr);
    ftd::test::check(
        "resume from the palette pushes Run",
        shell.run_palette_entry("action.resume", snapshot, commands, chrome));
    const auto drained = commands.drain();
    ftd::test::check("palette resume queued Run",
                     !drained.empty()
                         && std::holds_alternative<ftd::native::Run>(
                             drained.front().command));

    const ImGuiWindow* host = ImGui::FindWindowByName("FTD Dockspace###ftd.dock");
    ftd::test::check("dock host exists", host != nullptr);
    ftd::test::check(
        "dock host is transparent so the lattice shows through the central node",
        host != nullptr
            && (host->Flags & ImGuiWindowFlags_NoBackground) != 0);

    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    std::filesystem::remove_all(dir, ec);

#ifdef _OPENMP
    ftd::test::check("OpenMP thread count unchanged",
                     omp_get_max_threads() == threads_before);
#endif
    return ftd::test::finalize();
}
