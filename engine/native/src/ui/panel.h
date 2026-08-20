#pragma once

#include "native/command_queue.h"
#include "native/ui_demand.h"
#include "native/ui_snapshot.h"
#include "ui/history.h"
#include "ui/theme.h"

#include "imgui.h"

#include <atomic>
#include <string>

namespace ftd::native {

// Viewport is overlay chrome (the play bar), drawn over the central node — not a dock.
enum class DockSlot { Setup, Instruments, Physics, Viewport };

// Shell-owned chrome the Win32 strip used to hold. Panels may write these;
// they never touch the presenter or the session.
struct ViewChrome {
    bool* particles = nullptr;
    bool* flux = nullptr;
    bool* lattice_box = nullptr;
    std::atomic<int>* tick_hz = nullptr;
    std::atomic<bool>* paused = nullptr;
    bool* reset_camera = nullptr;
    bool* request_quit = nullptr;
    bool interop_active = false;
};

struct PanelContext {
    const UiSnapshot& snapshot;
    CommandSink& commands;
    const Theme& theme;
    const History& history;
    bool* open = nullptr;
    float dpi_scale = 1.0f;
    ViewChrome* chrome = nullptr;
};

struct Panel {
    virtual ~Panel() = default;
    virtual const char* id() const = 0;
    virtual const char* title() const = 0;
    virtual DockSlot default_slot() const = 0;
    virtual DataNeeds needs() const { return {}; }
    virtual ImGuiWindowFlags flags() const { return 0; }
    virtual void draw_contents(PanelContext&) = 0;
};

inline std::string window_name(const Panel& panel) {
    return std::string(panel.title()) + "###" + panel.id();
}

}  // namespace ftd::native
