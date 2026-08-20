#pragma once

#include "native/overlay_recorder.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

namespace ftd::native {

class ImGuiOverlay : public OverlayRecorder {
public:
    ImGuiOverlay();
    ~ImGuiOverlay() override;

    ImGuiOverlay(const ImGuiOverlay&) = delete;
    ImGuiOverlay& operator=(const ImGuiOverlay&) = delete;

    bool initialize(HWND hwnd, const PresenterUiContext& ctx);
    void shutdown();

    void begin_frame(float delta_seconds);
    void draw_debug_window();
    void end_frame();

    void record(ID3D12GraphicsCommandList* list,
                const RenderTargetInfo& rt) override;

    void rebuild_fonts(float dpi_scale);
    bool want_capture_mouse() const;
    bool want_capture_keyboard() const;

    static LRESULT wnd_proc_handler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

private:
    PresenterUiContext ctx_{};
    bool initialized_ = false;
    bool frame_open_ = false;
    float dpi_scale_ = 1.0f;
};

}  // namespace ftd::native
