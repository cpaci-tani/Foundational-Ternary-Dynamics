#include "native/imgui_overlay.h"

#include "native/imgui_font.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "implot.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace ftd::native {
namespace {

void imgui_alloc_srv(ImGui_ImplDX12_InitInfo* info,
                     D3D12_CPU_DESCRIPTOR_HANDLE* cpu,
                     D3D12_GPU_DESCRIPTOR_HANDLE* gpu) {
    auto* ctx = static_cast<PresenterUiContext*>(info->UserData);
    if (ctx && ctx->alloc_srv) {
        ctx->alloc_srv(ctx, cpu, gpu);
    }
}

void imgui_free_srv(ImGui_ImplDX12_InitInfo* info,
                    D3D12_CPU_DESCRIPTOR_HANDLE cpu,
                    D3D12_GPU_DESCRIPTOR_HANDLE gpu) {
    auto* ctx = static_cast<PresenterUiContext*>(info->UserData);
    if (ctx && ctx->free_srv) {
        ctx->free_srv(ctx, cpu, gpu);
    }
}

}  // namespace

ImGuiOverlay::ImGuiOverlay() = default;

ImGuiOverlay::~ImGuiOverlay() {
    shutdown();
}

bool ImGuiOverlay::initialize(HWND hwnd, const PresenterUiContext& ctx) {
    if (initialized_ || hwnd == nullptr || ctx.device == nullptr || ctx.queue == nullptr
        || ctx.srv_heap == nullptr || ctx.alloc_srv == nullptr || ctx.free_srv == nullptr) {
        return false;
    }
    ctx_ = ctx;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;
    const UINT dpi = GetDpiForWindow(hwnd);
    dpi_scale_ = dpi > 0 ? static_cast<float>(dpi) / 96.0f : 1.0f;
    add_embedded_inter_font(io, kUiFontSizeDip * dpi_scale_);
    // Theme + ScaleAllSizes are applied by UiShell / apply_theme. Do not
    // reset ImGuiStyle here — ScaleAllSizes is not idempotent.

    if (!ImGui_ImplWin32_Init(hwnd)) {
        ImPlot::DestroyContext();
        ImGui::DestroyContext();
        return false;
    }

    ImGui_ImplDX12_InitInfo init{};
    init.Device = ctx_.device;
    init.CommandQueue = ctx_.queue;
    init.NumFramesInFlight = ctx_.num_frames_in_flight;
    init.RTVFormat = ctx_.rtv_format;
    init.DSVFormat = ctx_.dsv_format;
    init.SrvDescriptorHeap = ctx_.srv_heap;
    init.UserData = &ctx_;
    init.SrvDescriptorAllocFn = &imgui_alloc_srv;
    init.SrvDescriptorFreeFn = &imgui_free_srv;
    if (!ImGui_ImplDX12_Init(&init)) {
        ImGui_ImplWin32_Shutdown();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();
        return false;
    }
    initialized_ = true;
    return true;
}

void ImGuiOverlay::shutdown() {
    if (!initialized_) {
        return;
    }
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    initialized_ = false;
    frame_open_ = false;
}

void ImGuiOverlay::begin_frame(float delta_seconds) {
    if (!initialized_) {
        return;
    }
    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = delta_seconds > 0.0f ? delta_seconds : (1.0f / 60.0f);
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    frame_open_ = true;
}

void ImGuiOverlay::draw_debug_window() {
    if (!initialized_ || !frame_open_) {
        return;
    }
    ImGui::SetNextWindowPos(ImVec2(348.0f, 16.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("FTD Debug###ftd.debug", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::TextUnformatted("FTD native desktop");
    ImGui::Text("ImGui %s + ImPlot %s", IMGUI_VERSION, IMPLOT_VERSION);
    ImGui::End();
}

void ImGuiOverlay::end_frame() {
    if (!initialized_ || !frame_open_) {
        return;
    }
    ImGui::Render();
    frame_open_ = false;
}

void ImGuiOverlay::record(ID3D12GraphicsCommandList* list, const RenderTargetInfo&) {
    if (!initialized_ || list == nullptr) {
        return;
    }
    ImDrawData* draw = ImGui::GetDrawData();
    if (draw != nullptr) {
        ImGui_ImplDX12_RenderDrawData(draw, list);
    }
}

void ImGuiOverlay::rebuild_fonts(float dpi_scale) {
    if (!initialized_) {
        return;
    }
    dpi_scale_ = dpi_scale > 0.0f ? dpi_scale : 1.0f;
    ImGui_ImplDX12_InvalidateDeviceObjects();
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    add_embedded_inter_font(io, kUiFontSizeDip * dpi_scale_);
    ImGui_ImplDX12_CreateDeviceObjects();
}

bool ImGuiOverlay::want_capture_mouse() const {
    return initialized_ && ImGui::GetIO().WantCaptureMouse;
}

bool ImGuiOverlay::want_capture_keyboard() const {
    return initialized_ && ImGui::GetIO().WantCaptureKeyboard;
}

LRESULT ImGuiOverlay::wnd_proc_handler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    return ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);
}

}  // namespace ftd::native
