#pragma once
//
// app/app_options.h — native_app command-line options (parsed once at startup).
//
// Split out of app/main.cpp for readability; behaviour is unchanged. AppOptions is
// the app-specific CLI surface (capture/headless flags, pre-open panel flags,
// simulated control edits, the --profile-ui reflow harness). It is distinct from
// native/cli_options.h's NativeDesktopCli (the session/HostOptions CLI).
//
#include "native/scale0_overlays.h"  // ftd::native::ForceStyle (parse_force_style)

#include <string>
#include <utility>
#include <vector>

namespace ftd::native::app {

struct AppOptions {
    int capture_frames = -1;   // -1 = interactive; >=0 = capture then exit
    bool start_paused = false; // default: live on launch
    int scale = 0;             // initial ScaleHost scale level (0 lattice, 1 particles)
    std::string field;         // legacy single overlay id (alias for --overlays; Scale-0)
    std::string overlays;      // comma-separated overlay ids to activate (Scale-0)
    // Initial rubber-sheet slice heights: repeatable --sheet-height <name>,<frac>
    // (e.g. --sheet-height gravPotential,0.8). Stamped into the first boundary
    // after --overlays, so the very first captured frame slices at that height.
    std::vector<std::pair<std::string, float>> sheet_heights;
    std::string png_out;       // capture PNG path override (empty = compiled default)
    bool prime_tick = true;    // run ONE tick at load so paused overlays have data
    // Simulated click-to-inspect for headless captures (interactive picking
    // can't run under --capture-frames). --inspect-voxel i,j,k selects a Scale-0
    // voxel; --inspect-particle N selects a Scale-1 particle. Fed into the same
    // live re-inspection path a real click uses, so the captured frame shows the
    // populated inspector.
    std::string inspect_voxel;         // "i,j,k" (empty = none; Scale-0 only)
    int inspect_particle = -1;         // particle index (< 0 = none; Scale-1 only)
    bool have_inspect_particle = false;
    // After --inspect-voxel selects a cell, --walk-neigh dx,dy,dz steps the
    // inspection cursor by that Moore offset (the same path a neighbour-cell click
    // drives), so a headless capture can show the walked target. Scale-0 only.
    std::string walk_neigh;            // "dx,dy,dz" (empty = none)
    // Simulated Setup-picker selection for headless captures (interactive
    // clicking can't run under --capture-frames). --pick-scenario <id> boots the
    // default scenario, then drives the SAME select_scenario → LoadScenario path
    // a real picker click uses, so the captured frame shows a scenario reloaded
    // via the picker. Scale-0 only.
    std::string pick_scenario;

    // ── Physics-control surface (headless proof of the new panel) ──
    // --open-physics / --open-config pre-open the (default-collapsed) sections so
    // a capture shows the full control surface. --open-physics leaves the toggle
    // categories COLLAPSED (4 headers whose counts sum to 44 — all grouped &
    // reachable); --expand-all-tog expands every category (all 44 rows), and
    // --expand-tog-group NAME expands one. --no-scroll disables the capture-mode
    // scroll-to-bottom so the shot shows the TOP of the panel.
    bool open_physics = false;
    bool open_config = false;
    bool open_overlays = false;
    bool expand_all_tog = false;
    std::vector<std::string> expand_tog_groups;
    bool no_scroll = false;
    // --open-telemetry pre-opens the (default-collapsed) telemetry section so a
    // capture shows the populated diagnostics + conservation charts. It also flips
    // the demand mask to include the audit/lagrangian groups (diagnostics is on
    // regardless); collapsed by default so idle fps is unaffected.
    bool open_telemetry = false;
    // Simulated control edits (interactive input can't run under --capture-frames).
    // These drive the SAME commands the −/＋ nudges and toggle clicks push, so the
    // captured snapshot reflects them: prove control works headlessly.
    std::vector<std::string> toggles_on;   // --toggle-on NAME  (repeatable)
    std::vector<std::string> toggles_off;  // --toggle-off NAME (repeatable)
    bool   set_dt = false;      double dt_value = 1.0;         // --set-dt V
    bool   set_sor = false;     int    sor_value = 0;          // --set-sor N
    bool   set_boundary = false; int   boundary_value = 0;     // --set-boundary N (0/1/2)
    int    set_lattice = 0;     // --set-lattice N (>0 ⇒ reboot at N; [4,256])
    // Global force render-style for the four Force overlays (Scale-0):
    // --force-style <arrows|heatmap|flow|glyphs>. Stamped at boot so a headless
    // capture of a Force overlay shows that style. Empty = Arrows (default).
    std::string force_style;
    // ── UI-profiling spike (--profile-ui N): run N frames timing RmlUi Update()
    //    (reflow) and presenter.render() (geometry), bucketed by whether the ~8/s
    //    status push dirtied a bound field that frame, then print a report and exit.
    //    --scn-open opens the picker; --scn-expand-cat K expands category K (1-based)
    //    so the profiler sees a large list. --profile-freeze-status forces the status
    //    push OFF — the control that isolates single-document reflow coupling from
    //    the mere presence of the DOM.
    int  profile_ui_frames = 0;
    bool scn_open = false;
    int  scn_expand_cat = 0;   // 1-based index into kScenarioCategories; 0 = none
    bool profile_freeze_status = false;
};

AppOptions parse_app_options(const std::vector<std::string>& args);

// Map a --force-style token → ForceStyle (Arrows on an empty/unknown token).
ftd::native::ForceStyle parse_force_style(const std::string& s);

}  // namespace ftd::native::app
