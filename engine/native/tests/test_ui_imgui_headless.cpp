#include "native/imgui_assert.h"
#include "native/imgui_font.h"

#include "ftd/test_telemetry.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "implot.h"

#include <cfloat>
#include <cmath>
#include <string>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace {

constexpr ImVec2 kDisplaySize{1280.0f, 720.0f};
constexpr float kDeltaTime = 1.0f / 60.0f;
const char* kDebugWindow = "FTD Debug###ftd.debug";

void prepare_headless_io(ImGuiIO& io, float font_size_px) {
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.DisplaySize = kDisplaySize;
    io.DeltaTime = kDeltaTime;
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
    io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
    ftd::native::add_embedded_inter_font(io, font_size_px);
    unsigned char* pixels = nullptr;
    int width = 0;
    int height = 0;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    ftd::test::check("font atlas built", pixels != nullptr && width > 0 && height > 0);
}

void draw_debug_window() {
    ImGui::Begin(kDebugWindow, nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("FTD native desktop 1a");
    ImGui::End();
}

bool is_finite(float v) {
    return std::isfinite(v) != 0;
}

void assert_draw_data_invariants(const ImDrawData* draw_data) {
    ftd::test::check("draw data exists", draw_data != nullptr);
    ftd::test::check("display size matches",
                     draw_data->DisplaySize.x == kDisplaySize.x
                     && draw_data->DisplaySize.y == kDisplaySize.y);
    int bad_verts = 0;
    int bad_clips = 0;
    for (int n = 0; n < draw_data->CmdLists.Size; ++n) {
        const ImDrawList* list = draw_data->CmdLists[n];
        for (int i = 0; i < list->VtxBuffer.Size; ++i) {
            const ImVec2 pos = list->VtxBuffer[i].pos;
            if (!(is_finite(pos.x) && is_finite(pos.y))) {
                ++bad_verts;
            }
        }
        for (int c = 0; c < list->CmdBuffer.Size; ++c) {
            const ImVec4 clip = list->CmdBuffer[c].ClipRect;
            if (!(is_finite(clip.x) && is_finite(clip.y)
                  && is_finite(clip.z) && is_finite(clip.w))) {
                ++bad_clips;
            } else if (!(clip.x >= -1.0f && clip.y >= -1.0f
                         && clip.z <= kDisplaySize.x + 1.0f
                         && clip.w <= kDisplaySize.y + 1.0f)) {
                ++bad_clips;
            }
        }
    }
    ftd::test::check("vertex pos finite", bad_verts == 0);
    ftd::test::check("clip rects finite and inside DisplaySize", bad_clips == 0);
}

std::vector<float> capture_positions(const ImDrawData* draw_data) {
    std::vector<float> out;
    for (int n = 0; n < draw_data->CmdLists.Size; ++n) {
        const ImDrawList* list = draw_data->CmdLists[n];
        out.reserve(out.size() + static_cast<size_t>(list->VtxBuffer.Size) * 2);
        for (int i = 0; i < list->VtxBuffer.Size; ++i) {
            out.push_back(list->VtxBuffer[i].pos.x);
            out.push_back(list->VtxBuffer[i].pos.y);
        }
    }
    return out;
}

ImVec2 measure_debug_window(float scale) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    prepare_headless_io(io, ftd::native::kUiFontSizeDip * scale);
    ImGui::GetStyle().ScaleAllSizes(scale);

    ImGui::NewFrame();
    draw_debug_window();
    ImGui::Render();
    assert_draw_data_invariants(ImGui::GetDrawData());

    ImGuiWindow* window = ImGui::FindWindowByName(kDebugWindow);
    ftd::test::check("composed debug window exists", window != nullptr);
    const ImVec2 size = window->Size;
    ftd::test::check("window stays inside DisplaySize",
                     size.x <= kDisplaySize.x && size.y <= kDisplaySize.y
                     && size.x > 0.0f && size.y > 0.0f);

    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    return size;
}

int g_assert_calls = 0;

void recording_assert_handler(const char*, const char*, int) {
    ++g_assert_calls;
}

}  // namespace

int main() {
    ftd::test::init("test_ui_imgui_headless");

    ftd::test::section("IM_ASSERT dispatch is hookable without abort");
    g_assert_calls = 0;
    {
        ftd::native::ScopedImGuiAssertHandler scoped(&recording_assert_handler);
        IM_ASSERT(false && "hook probe");
        ftd::test::check("scoped handler observed the assertion", g_assert_calls == 1);
    }
    ftd::test::check("default handler restored after scope", g_assert_calls == 1);

#ifdef _OPENMP
    const int threads_before = omp_get_max_threads();
#endif

    ftd::test::section("headless init, draw-data, ### name, deterministic re-draw");
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    prepare_headless_io(io, ftd::native::kUiFontSizeDip);

#ifdef _OPENMP
    ftd::test::check("OpenMP max threads unchanged across ImGui init",
                     omp_get_max_threads() == threads_before);
#endif

    ImGui::NewFrame();
    draw_debug_window();
    ImGui::Render();
    assert_draw_data_invariants(ImGui::GetDrawData());

    ImGui::NewFrame();
    draw_debug_window();
    ImGui::Render();
    const ImDrawData* first = ImGui::GetDrawData();
    assert_draw_data_invariants(first);
    const std::vector<float> first_pos = capture_positions(first);

    ImGuiWindow* window = ImGui::FindWindowByName(kDebugWindow);
    ftd::test::check("FindWindowByName sees composed title###id", window != nullptr);
    const std::string stored_name = window ? window->Name : "";
    ftd::test::check("window name contains ###",
                     stored_name.find("###") != std::string::npos);

    ImGui::NewFrame();
    draw_debug_window();
    ImGui::Render();
    const ImDrawData* second = ImGui::GetDrawData();
    assert_draw_data_invariants(second);
    const std::vector<float> second_pos = capture_positions(second);
    ImGuiWindow* window2 = ImGui::FindWindowByName(kDebugWindow);
    ftd::test::check("composed name is stable across two draws",
                     window2 != nullptr && stored_name == window2->Name);
    ftd::test::check("re-draw vertex positions are identical",
                     first_pos == second_pos);

#ifdef _OPENMP
    ftd::test::check("OpenMP max threads unchanged after first draw",
                     omp_get_max_threads() == threads_before);
#endif

    ImPlot::DestroyContext();
    ImGui::DestroyContext();

#ifdef _OPENMP
    ftd::test::check("OpenMP max threads unchanged after DestroyContext",
                     omp_get_max_threads() == threads_before);
#endif

    ftd::test::section("DPI matrix font_size x scale + ScaleAllSizes(scale)");
    const ImVec2 size_1 = measure_debug_window(1.0f);
    const ImVec2 size_15 = measure_debug_window(1.5f);
    const ImVec2 size_2 = measure_debug_window(2.0f);
    const float ratio_15 = size_15.x / size_1.x;
    const float ratio_2 = size_2.x / size_1.x;
    ftd::test::check("1.5x scale grows content bounds near 1.5",
                     ratio_15 > 1.2f && ratio_15 < 1.8f);
    ftd::test::check("2.0x scale grows content bounds near 2.0",
                     ratio_2 > 1.6f && ratio_2 < 2.4f);
    ftd::test::check("larger scales produce larger windows",
                     size_2.x > size_15.x && size_15.x > size_1.x
                     && size_2.y >= size_15.y && size_15.y >= size_1.y);

    return ftd::test::finalize();
}
