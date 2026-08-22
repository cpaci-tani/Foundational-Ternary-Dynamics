#pragma once
//
// app/app_context.h — AppContext (the shared GUI-thread state every wnd_proc /
// RmlUi callback reaches) plus the command-emission helpers that translate UI
// actions into ScaleCommands on the bus. Split out of app/main.cpp
// (behavior-neutral).
//
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>   // HWND, POINT

#include "app/ui_model.h"             // ShellData, OverlayRow, ConfigSpec, builders
#include "native/d3d12_presenter.h"  // D3D12Presenter, Camera
#include "native/host/command_bus.h" // CommandBus
#include "native/host/run_config.h"  // RunConfig
#include "native/model/commands.h"   // Core/Scale0/Scale1 command types
#include "native/scene_rect.h"       // SceneRect
#include "native/ui_snapshot.h"      // BridgeKnobs

#include "ftd/term_toggles.h"        // TermToggles

#include <RmlUi/Core.h>              // Rml::Context, Rml::DataModelHandle

#include <atomic>
#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

namespace ftd::native::app {

// ── Everything the Win32 wnd_proc + RmlUi event callbacks need to reach. Set
//    once on the GUI thread during setup and read only on the GUI thread
//    (wnd_proc runs during DispatchMessageW on this same thread), so no locking.
struct AppContext {
    HWND hwnd = nullptr;
    Rml::Context* context = nullptr;
    ftd::native::D3D12Presenter* presenter = nullptr;
    ftd::native::Camera* camera = nullptr;
    ftd::native::CommandBus* commands = nullptr;
    ShellData* data = nullptr;
    std::atomic<bool>* paused = nullptr;
    std::atomic<bool>* quit = nullptr;
    std::string scenario_id;

    // The laid-out #viewport hole in client pixels (updated each frame after
    // Context::Update). Pointer arbitration + the presenter scene_rect use it.
    ftd::native::SceneRect viewport_rect{};
    bool dragging = false;
    POINT last{};

    // ── Click-vs-drag discrimination (GUI thread; wnd_proc only) ──
    // A press inside the viewport that releases with < kClickSlop travel is a
    // CLICK (→ pick); anything with more travel is an orbit DRAG (camera moved,
    // no pick). press_pt is the button-down point; drag_moved latches once the
    // pointer leaves the slop box.
    POINT press_pt{};
    bool press_in_viewport = false;
    bool drag_moved = false;

    // Pick request handed from wnd_proc to the GUI loop (same thread; the loop
    // owns the frame + camera + viewport rect needed to unproject the ray).
    bool pick_pending = false;
    int pick_x = 0, pick_y = 0;

    // ── Selected inspection target (GUI thread) ──
    // Re-issued as an InspectVoxel / InspectParticle1 each new snapshot so the
    // readout stays live. 0 = nothing picked, 1 = Scale-0 voxel, 2 = Scale-1
    // particle.
    int inspect_kind = 0;
    int inspect_vx = 0, inspect_vy = 0, inspect_vz = 0;  // Scale-0 voxel cell
    int inspect_pidx = -1;                               // Scale-1 particle index
    // True once at least one inspection payload has been received for the
    // current target. The published snapshot only carries the inspection on the
    // boundaries that drained a re-issue, so between those the readout keeps its
    // last (live) values rather than blanking to "reading…". Reset on a new
    // pick / scale switch / clear.
    bool insp_has_data = false;
    // Set by the walk_neigh callback when a 26-neighbour cell is clicked: the loop
    // resets the inspect re-issue seqs so the readout retargets immediately.
    bool inspect_retarget = false;

    // ── Rubber-sheet height control (GUI thread) ──
    // Data-model handle stored so the wnd_proc scroll-wheel path can dirty the
    // overlay panel after a height nudge (the RML event callbacks get their own
    // handle). last_sheet_id is the most-recently toggled-on / nudged sheet — the
    // target the viewport scroll-wheel (Shift+wheel) moves.
    Rml::DataModelHandle model{};
    bool model_ready = false;
    std::uint32_t last_sheet_id = 0;
    bool has_last_sheet = false;

    // ── Setup scenario picker (GUI thread) ──
    // The live search string (last value typed into the picker filter box). Kept
    // so a group collapse/expand re-derives visibility consistently with the
    // active filter.
    std::string scenario_filter;
    // Titles of the picker groups the user has expanded. Drives which groups
    // instantiate their item rows in rebuild_scenario_view (closed groups build
    // zero rows). Empty ⇒ every group collapsed (the default when the picker
    // opens), so an open-but-unbrowsed picker is just the 5 category headers.
    std::vector<std::string> scn_expanded_groups;

    // ── Physics-control surface (GUI thread) ──
    // Authoritative live engine state, mirrored from the Scale-0 snapshot each
    // boundary. Every control callback reads its "current" value from these
    // caches (never from the — possibly unbuilt — view rows), so command dispatch
    // is decoupled from the lazy DOM. `have_live` is false until the first
    // Scale-0 snapshot lands.
    ftd::TermToggles live_toggles;
    ftd::native::BridgeKnobs live_knobs;
    bool have_live = false;
    bool backend_cpu = false;  // drives the CPU-only gpu-warning surfacing
    // Titles of the toggle categories the user has expanded (scenario-picker
    // pattern: only an expanded category builds its item rows).
    std::vector<std::string> tog_expanded_groups;
    // The run config used to reboot the engine at a new lattice L (the one
    // knob that reboots). Seeded from the CLI at boot and updated when the
    // lattice / boundary knobs change, so a reboot re-applies the live choices.
    ftd::native::RunConfig run_config;

    // ── Diagnostics-panel running stats (GUI thread) ──
    // One cumulative-since-reset Min/Max/Avg accumulator per kDiagMetrics[] row
    // (sized lazily to diag_metric_count()). Advanced only when the metric's
    // telemetry group tick changes — so a repeated cached snapshot never over-
    // counts — and reset on scenario / scale / lattice change or a group-tick
    // regression. Accumulates every boundary (even while the panel is closed) so
    // Min/Max/Avg reflect the whole run. The steady_clock stamps drive the
    // "state t… · N ms" freshness age (the snapshot carries no wall-clock time).
    std::vector<RunningStat> diag_stats;
    int diag_last_diag_tick = -1;
    int diag_last_audit_tick = -1;
    int diag_synced_lattice = -1;
    std::chrono::steady_clock::time_point diag_stamp{};
    std::chrono::steady_clock::time_point audit_stamp{};
};


// ── Command helpers (GUI thread; drained by the sim thread). ────────────────
void push_core(AppContext* app, ftd::native::CoreCommand cmd);
void push_scale0(AppContext* app, ftd::native::Scale0Cmd cmd);
void push_scale1(AppContext* app, ftd::native::Scale1Cmd cmd);
void request_play(AppContext* app);
void request_pause(AppContext* app);
void request_play_toggle(AppContext* app);
void request_step(AppContext* app);
void request_reset(AppContext* app);
void request_switch_scale(AppContext* app, int level);
void request_toggle(AppContext* app, const std::string& name);
void request_reset_toggles(AppContext* app);
void request_telemetry_demand(AppContext* app);  // OR of all open analysis sections
void request_lattice_reboot(AppContext* app, int new_l);
void request_config_nudge(AppContext* app, const std::string& key, const std::string& dir);
void nudge_sheet_height(AppContext* app, OverlayRow* row, float delta);
void nudge_last_sheet(AppContext* app, float delta);

// Move the click-to-inspect cursor by a Moore offset (dx,dy,dz), clamped to the
// live lattice, and flag a retarget so the loop re-issues the inspection
// immediately. No-op unless a Scale-0 voxel is currently inspected (kind == 1).
// Shared by the neighbour-cell click callback and the headless --walk-neigh flag.
void walk_inspection(AppContext* app, int dx, int dy, int dz);

}  // namespace ftd::native::app
