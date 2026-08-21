#pragma once
//
// app/app_input.h — the Win32 window procedure and the RmlOverlay recorder that
// draws the RmlUi shell into the presenter's command list. Split out of
// app/main.cpp (behavior-neutral). The input helpers (lparam extractors,
// rml_key_modifiers, over_viewport, app_from_hwnd) stay file-local in the .cpp.
//
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include "native/d3d12_presenter.h"   // ftd::native::OverlayRecorder, RenderTargetInfo
#include "ui/rml_d3d12_renderer.h"    // ftd::native::ui::RmlD3D12Renderer

#include <RmlUi/Core.h>               // Rml::Context

namespace ftd::native::app {

using ftd::native::ui::RmlD3D12Renderer;
// ── OverlayRecorder: draw the RmlUi shell into the presenter's command list ──
// Called from inside D3D12Presenter::render(), after the 3D scene is recorded
// and with the full-window RTV bound. begin_frame rebinds the UI heap / PSO /
// ortho / full viewport+scissor, Context::Render() emits geometry through the
// RenderInterface into `list`, end_frame() stops routing.
class RmlOverlay : public ftd::native::OverlayRecorder {
public:
    RmlOverlay(RmlD3D12Renderer* renderer, Rml::Context** context)
        : renderer_(renderer), context_(context) {}
    void record(ID3D12GraphicsCommandList* list,
                const ftd::native::RenderTargetInfo& rt) override {
        if (!renderer_ || !*context_) return;
        renderer_->begin_frame(list, rt.width, rt.height);
        (*context_)->Render();
        renderer_->end_frame();
    }

private:
    RmlD3D12Renderer* renderer_;
    Rml::Context** context_;
};

// Win32 window procedure (registered by run_app). Reaches its AppContext via
// GWLP_USERDATA; forwards pointer/scroll/DPI events to RmlUi, orbits the camera,
// sweeps the active rubber sheet on Shift+wheel, and flags scene-click picks.
LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

}  // namespace ftd::native::app