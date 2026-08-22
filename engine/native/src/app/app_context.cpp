// app/app_context.cpp — command-emission helpers (see app/app_context.h).

#include "app/app_context.h"

#include "native/scale0_overlays.h"   // overlay_by_name / overlay_by_id

#include <algorithm>
#include <cmath>
#include <utility>

namespace ftd::native::app {
// ── Command helpers (run on the GUI thread; drained by the sim thread) ──
// The bus carries scale-generic ScaleCommands: loop control / reload are core
// commands the host handles; toggles are a Scale-0 payload the adapter handles.
void push_core(AppContext* app, ftd::native::CoreCommand cmd) {
    if (app && app->commands)
        app->commands->push(ftd::native::core_command(std::move(cmd)));
}
void push_scale0(AppContext* app, ftd::native::Scale0Cmd cmd) {
    if (app && app->commands)
        app->commands->push(ftd::native::scale0_command(std::move(cmd)));
}
void push_scale1(AppContext* app, ftd::native::Scale1Cmd cmd) {
    if (app && app->commands)
        app->commands->push(ftd::native::scale1_command(std::move(cmd)));
}

void request_play(AppContext* app) {
    push_core(app, ftd::native::Run{});
    if (app->paused) app->paused->store(false);
}
void request_pause(AppContext* app) {
    push_core(app, ftd::native::Pause{});
    if (app->paused) app->paused->store(true);
}
void request_play_toggle(AppContext* app) {
    if (app->paused && app->paused->load()) request_play(app);
    else request_pause(app);
}
void request_step(AppContext* app) {
    push_core(app, ftd::native::Pause{});
    push_core(app, ftd::native::Step{1});
    if (app->paused) app->paused->store(true);
}
void request_reset(AppContext* app) {
    if (!app->scenario_id.empty())
        push_core(app, ftd::native::LoadScenario{app->scenario_id});
}
// Push a core SwitchScale: the host tears down the active adapter, rebuilds via
// make_scale_adapter(level), and reboots into the given per-scale seed id. The
// app tracks the active scenario so Reset targets the newly active scale.
void request_switch_scale(AppContext* app, int level) {
    const char* scenario = (level == 1) ? "s1-hydrogen-cloud" : "s0-seed-hydrogen";
    push_core(app, ftd::native::SwitchScale{level, scenario});
    app->scenario_id = scenario;
}
void request_toggle(AppContext* app, const std::string& name) {
    // Read the CURRENT value from the authoritative live engine mirror, not the
    // (possibly unbuilt) view rows — flip it and push a SetToggle Scale-0 command.
    const ftd::ToggleSpec* spec = ftd::term_toggles_detail::find_spec(name);
    const bool cur = spec ? (app->live_toggles.*(spec->field)) : false;
    // Optimistically flip the matching built row for immediate LED feedback (the
    // snapshot confirms it next boundary).
    if (app->data) {
        for (ToggleGroupRow& g : app->data->toggle_groups)
            for (FullToggleRow& r : g.items)
                if (r.name == name) { r.on = !cur; break; }
        if (app->model_ready) app->model.DirtyVariable("toggle_groups");
    }
    push_scale0(app, ftd::native::SetToggle{name, !cur});
}

// Reset all term toggles to their canonical defaults (ftd::TermToggles{}). Issues
// the ResetToDefaults Scale-0 command; the snapshot reflects it next boundary.
void request_reset_toggles(AppContext* app) {
    push_scale0(app, ftd::native::ResetToDefaults{});
}

// Publish the desired telemetry-group demand to the host (a core SetTelemetryDemand
// command). The DIAGNOSTICS group is always requested — it is the cheap cadence-1
// reduction that keeps the base charts populated even while the panel is closed —
// and the heavier AUDIT + LAGRANGIAN groups are requested only while the telemetry
// section is open. The host stores this on demand_ and the Scale-0 adapter maps it
// onto the NativeTelemetryScheduler in build_snapshot(), so the scheduler starts
// producing real diagnostics/audit/lagrangian CachedView data (it is otherwise
// inert with a zero mask). Harmless on Scale 1 (its adapter ignores the mask).
void request_telemetry_demand(AppContext* app) {
    // OR together the demand of every open analysis section (DIAGNOSTICS is always
    // on — cheap cadence-1). Reads the live section-open flags off the shell data.
    ftd::native::DataNeeds needs;
    needs.telemetry_groups = ftd::TELEMETRY_DIAGNOSTICS;
    if (app && app->data) {
        if (app->data->diag_active)
            needs.telemetry_groups |= ftd::TELEMETRY_AUDIT;   // Energy-Budget rows
        if (app->data->tel_open)
            needs.telemetry_groups |= ftd::TELEMETRY_AUDIT | ftd::TELEMETRY_LAGRANGIAN;
        if (app->data->thermo_open)
            needs.telemetry_groups |= ftd::TELEMETRY_AUDIT;   // E_wave for T_kin
        if (app->data->grav_open || app->data->time_open)
            needs.telemetry_groups |= ftd::TELEMETRY_GRAVITY;
        if (app->data->spectrum_open) {
            needs.spectrum = true;   // adapter computes the flux E(k) this boundary
            needs.telemetry_groups |= ftd::TELEMETRY_AUDIT;   // Spectrum+ topology
        }
        // Field slices: Flux-slice (8) shows |J|; Gravity (3) / Time (4) show the
        // latency field; Thermo (5) shows |J|. Same slice subsystem, field-selected.
        const int ap = app->data->active_panel;
        if (ap == 8)                  { needs.slice = true; needs.slice_field = 0; }
        else if (ap == 3 || ap == 4)  { needs.slice = true; needs.slice_field = 1; }
        else if (ap == 5)             { needs.slice = true; needs.slice_field = 0; }
    }
    push_core(app, ftd::native::SetTelemetryDemand{needs});
}

// Reboot the current scenario at a new lattice L (the one knob that reboots the
// engine). Clamps L to [4,256], stamps it into the app's live RunConfig, and
// pushes the core SetRunConfig command — the host reloads the active scenario on
// the fresh lattice (interop re-establishes via the applied_reload() path).
void request_lattice_reboot(AppContext* app, int new_l) {
    new_l = std::max(4, std::min(256, new_l));
    if (new_l == app->run_config.lattice_size) return;
    app->run_config.lattice_size = new_l;
    push_core(app, ftd::native::SetRunConfig{app->run_config});
}

// Dispatch a config-knob nudge (`dir` = "+" / "-"). Reads the CURRENT value from
// the live engine mirror, steps/cycles by the ConfigSpec, and pushes the matching
// Scale-0 (or core, for lattice) command. Optimistic view update is unnecessary —
// the next snapshot refreshes the displayed value (change-guarded).
void request_config_nudge(AppContext* app, const std::string& key, const std::string& dir) {
    const ConfigSpec* s = find_config_spec(key);
    if (!s) return;
    const double cur = config_current(*s, app->live_toggles, app->live_knobs);
    const double sign = (dir == "+") ? 1.0 : -1.0;

    // Enum knobs cycle through [lo,hi] (wrapping); numeric knobs step + clamp.
    const bool is_enum = (s->kind == CfgKind::Boundary || s->kind == CfgKind::Bcc
                          || s->kind == CfgKind::SiteFilter);
    double next;
    if (is_enum) {
        const int lo = static_cast<int>(std::lround(s->lo));
        const int hi = static_cast<int>(std::lround(s->hi));
        const int span = hi - lo + 1;
        int v = static_cast<int>(std::lround(cur)) + (dir == "+" ? 1 : -1);
        v = lo + ((v - lo) % span + span) % span;  // wrap into [lo,hi]
        next = static_cast<double>(v);
    } else {
        next = std::clamp(cur + sign * s->step, s->lo, s->hi);
    }

    switch (s->kind) {
        case CfgKind::Lattice:
            request_lattice_reboot(app, static_cast<int>(std::lround(next)));
            break;
        case CfgKind::Double: {
            ftd::native::DoubleKey dk = ftd::native::DoubleKey::langevin_T;
            if (key == "langevin_T") dk = ftd::native::DoubleKey::langevin_T;
            else if (key == "langevin_gamma") dk = ftd::native::DoubleKey::langevin_gamma;
            else if (key == "coulomb_coupling") dk = ftd::native::DoubleKey::coulomb_charge_coupling;
            else if (key == "coulomb_source") dk = ftd::native::DoubleKey::coulomb_source_scale;
            else if (key == "omega0") dk = ftd::native::DoubleKey::omega0;
            else if (key == "kinetic_drain") dk = ftd::native::DoubleKey::kinetic_drain;
            if (key == "dt") push_scale0(app, ftd::native::SetDt{next});
            else push_scale0(app, ftd::native::SetDouble{dk, next});
            break;
        }
        case CfgKind::Int:  // only SOR uses Int
            push_scale0(app, ftd::native::SetSorIterations{static_cast<int>(std::lround(next))});
            break;
        case CfgKind::UInt:  // only langevin_seed
            push_scale0(app, ftd::native::SetUInt{ftd::native::UIntKey::langevin_seed,
                                                  static_cast<unsigned>(std::lround(next))});
            break;
        case CfgKind::Boundary:
            app->run_config.flux_boundary = static_cast<int>(std::lround(next));
            push_scale0(app, ftd::native::SetBoundary{
                static_cast<ftd::FluxBoundaryMode>(static_cast<int>(std::lround(next)))});
            break;
        case CfgKind::Bcc:
            push_scale0(app, ftd::native::SetEnum{ftd::native::EnumKey::bcc_stencil,
                                                  static_cast<int>(std::lround(next))});
            break;
        case CfgKind::SiteFilter:
            push_scale0(app, ftd::native::SetEnum{ftd::native::EnumKey::langevin_site_filter,
                                                  static_cast<int>(std::lround(next))});
            break;
    }
}

// Nudge one active rubber-sheet's slice height by `delta` (fraction of the box):
// clamp, optimistically update the panel row for immediate feedback, push the
// SetSheetHeight Scale-0 command (adapter re-slices + repositions on the next
// capture — live even while paused, since SetSheetHeight is a frame-refresh
// write), and mark this the last-touched sheet (the scroll-wheel target). Shared
// by the panel −/＋ callback and the viewport scroll-wheel path.
void nudge_sheet_height(AppContext* app, OverlayRow* row, float delta) {
    if (!app || !row || !row->is_sheet) return;
    const ftd::native::OverlayDescriptor* d = ftd::native::overlay_by_name(row->name.c_str());
    if (!d) return;
    const float h = std::clamp(row->height + delta, 0.0f, 0.999f);
    row->height = h;
    row->hstr = sheet_hstr(h);
    push_scale0(app, ftd::native::SetSheetHeight{static_cast<std::uint32_t>(d->id), h});
    app->last_sheet_id = static_cast<std::uint32_t>(d->id);
    app->has_last_sheet = true;
    if (app->model_ready) app->model.DirtyVariable("overlay_columns");
}

// Nudge whichever sheet the scroll-wheel currently targets (the most-recently
// toggled-on / adjusted sheet, if it is still active). No-op when no sheet is
// the target. Called from wnd_proc (GUI thread) for Shift+wheel over the scene.
void nudge_last_sheet(AppContext* app, float delta) {
    if (!app || !app->has_last_sheet || !app->data) return;
    const ftd::native::OverlayDescriptor* d =
        ftd::native::overlay_by_id(app->last_sheet_id);
    if (!d) return;
    OverlayRow* row = find_overlay_row(app->data, Rml::String(d->name));
    if (!row || !row->on || !row->is_sheet) return;  // target no longer an active sheet
    nudge_sheet_height(app, row, delta);
}

// Move the inspection cursor by a Moore offset, clamped to the live lattice, and
// flag a retarget (the loop re-issues InspectVoxel/Force/Neighbors for the new
// voxel). No-op unless a Scale-0 voxel is inspected.
void walk_inspection(AppContext* app, int dx, int dy, int dz) {
    if (!app || app->inspect_kind != 1) return;
    const int L = (app->have_live && app->live_knobs.lattice_size > 0)
                      ? static_cast<int>(app->live_knobs.lattice_size)
                      : std::max(1, app->run_config.lattice_size);
    const int hi = std::max(0, L - 1);
    app->inspect_vx = std::min(hi, std::max(0, app->inspect_vx + dx));
    app->inspect_vy = std::min(hi, std::max(0, app->inspect_vy + dy));
    app->inspect_vz = std::min(hi, std::max(0, app->inspect_vz + dz));
    app->insp_has_data = false;    // fresh target — reset the readout latch
    app->inspect_retarget = true;  // loop resets the re-issue seqs (immediate refresh)
}

}  // namespace ftd::native::app
