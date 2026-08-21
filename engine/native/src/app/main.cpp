// native_app — the live windowed FTD native application (M-UI-1..M-UI-3 fused).
//
// A real Win32 window whose swapchain is owned by D3D12Presenter. Two threads,
// the same split the native_desktop reference (native_desktop/src/main.cpp) uses:
//   • sim thread   — owns a ScaleHost (Scale 0 behind the ScaleHost/ScaleAdapter
//                    seam), ticks it, drains the CommandBus at the tick boundary,
//                    publishes a HostSnapshot + a NativeFrame.
//   • GUI thread   — owns RmlUi (RmlD3D12Renderer + Rml::Context) and the
//                    presenter. Each frame it acquires the published snapshot,
//                    pushes it into the RmlUi data model, lays out (Context::
//                    Update), then calls presenter.render(...). The RmlUi shell
//                    is drawn INSIDE the presenter's OverlayRecorder seam so the
//                    CSS chrome composites over the live 3D scene in one frame.
//
// The shell's transparent `#viewport` element marks the 3D hole: the presenter's
// scene_rect is set to that element's laid-out rectangle each frame, so the
// interop-free CPU-gathered lattice/particle/flux scene renders exactly there
// while RmlUi draws the toolbar / Setup panel / Physics-terms panel / status bar
// around it.
//
// On the GPU backend at Scale 0 this app wires the CUDA<->D3D12 zero-copy
// interop path (mirroring the native_desktop reference): the presenter exposes
// a shared particle buffer + a shared cross-API fence, CUDA imports both, and
// the interop gather writes device-resident particles straight into that
// buffer. The sim thread requests a gather + advances the shared fence each
// round; the GUI thread waits on that fence and draws the device-resident
// particles (presenter.render(..., interop_particle_count > 0)). The CPU sprite
// path (interop_particle_count = 0) remains the fallback for the CPU backend,
// for Scale 1 (ParticleEngine has no device buffer), and for any frame whose
// gather has not yet landed — host.capture() still supplies frame.particles +
// flux + metadata every round, so a fallback frame is never blank.
//
// --capture-frames N: boot, run the sim while rendering, request a composited
// back-buffer readback, save it as a PNG, and exit 0 — the headless proof that
// chrome + live scene + real data all land in one frame.

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
// NOTE: deliberately NOT <windowsx.h> — its message-cracker macros
// (GetNextSibling / GetFirstChild / GetLastChild / …) collide with RmlUi's
// Rml::Element methods of the same name and break <RmlUi/Core.h>. The only
// windowsx.h helpers this file needs are the signed LPARAM coordinate
// extractors, defined locally below.
#include <shellapi.h>
#include <wincodec.h>
#include <wrl/client.h>

#ifndef _WIN64
#error "native_app is a Win64 (x86-64) target"
#endif

#include "native/cli_options.h"
#include "native/d3d12_presenter.h"
#include "native/dpi_support.h"
#include "native/host/command_bus.h"
#include "native/host/scale_host.h"
#include "native/model/commands.h"
#include "native/model/snapshot.h"
#include "native/native_frame.h"
#include "native/scale0_overlays.h"
#include "native/scenario_catalog.h"  // ftd::native scenario catalog (Setup picker)
#include "native/scene_rect.h"

#include "ftd/term_toggles.h"
#include "ftd/visual_field_sample.h"   // ftd::VisualFieldKind (FIELDS overlay menu)
#include "ftd/visual_snapshot.h"   // ftd::kMaxVisualParticleCapture (interop buffer sizing)

#include "ui/rml_d3d12_renderer.h"
#include "ui/ftd_chart_element.h"

#include <RmlUi/Core.h>
#include <RmlUi/Core/Factory.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

using Microsoft::WRL::ComPtr;
using ftd::native::ui::RmlD3D12Renderer;
using ftd::native::ui::RmlD3D12System;

namespace {

// Signed LPARAM coordinate extractors (would come from <windowsx.h>, which we
// cannot include — see the include block above).
inline int lparam_x(LPARAM lp) { return static_cast<int>(static_cast<short>(LOWORD(lp))); }
inline int lparam_y(LPARAM lp) { return static_cast<int>(static_cast<short>(HIWORD(lp))); }

// Max pointer travel (client px) between button-down and button-up that still
// counts as a CLICK (→ scene pick) rather than a DRAG (→ camera orbit).
constexpr int kClickSlop = 4;

// ── The physics-control panel is data-driven from ftd::TOGGLE_SPECS[] (all 44
//    toggles) plus the non-bool config knobs on TermToggles / the bridge. To
//    keep the live DOM small (fps), the whole panel is collapsible: the toggle
//    list is split into a few named categories (each independently collapsible,
//    building its rows only when open — the scenario-picker pattern), and the
//    config knobs live behind their own collapse. Nothing here is a
//    hand-maintained subset: every TOGGLE_SPECS row is placed into a category, so
//    adding a toggle to the table makes it appear automatically.

// The toggle categories, in panel order. Every TOGGLE_SPECS row maps to exactly
// one via toggle_category(); an unrecognized name falls into the last bucket, so
// a newly-added toggle can never vanish from the panel.
constexpr const char* kToggleCategories[] = {
    "Core dynamics",
    "Forces & gravity",
    "Nuclear / gauge",
    "Thermal / quantum / diag",
};

// Assign a toggle (by canonical TOGGLE_SPECS name) to a category index. Pure
// presentation; the set of toggles shown is still exactly TOGGLE_SPECS.
int toggle_category(std::string_view n) {
    // Forces & gravity.
    if (n == "forces" || n == "gravity" || n == "poisson_coulomb" || n == "lorentz_force"
        || n == "emergent_forces" || n == "exchange_force" || n == "latency_field"
        || n == "field_energy_gravity" || n == "cluster_inertia" || n == "geometric_gravity"
        || n == "absorbing_boundary" || n == "reflective_boundary")
        return 1;
    // Nuclear / gauge.
    if (n == "color_forces" || n == "strong_stress_energy" || n == "weak_transmutation"
        || n == "strong_force" || n == "triad_binding" || n == "confinement"
        || n == "su2_gauge" || n == "su3_gauge" || n == "pair_production")
        return 2;
    // Thermal / quantum / diagnostics.
    if (n == "larmor_radiation" || n == "langevin" || n == "de_broglie_clock"
        || n == "db_clock_coulomb" || n == "knot_tracking" || n == "strict_validation")
        return 3;
    // Everything else: core dynamics + integrators + gauss variants + EW sweep.
    return 0;
}

// Non-bool config knobs, exposed as −/＋ nudge controls (RmlUi has no range
// input; this mirrors the rubber-sheet height affordance). `kind` selects the
// command + value semantics; the enum kinds cycle through their labels.
enum class CfgKind { Double, Int, Boundary, Bcc, SiteFilter, UInt, Lattice };
struct ConfigSpec {
    const char* key;    // stable id (matches config_nudge dispatch)
    const char* label;  // human label
    CfgKind     kind;
    double      lo, hi, step;
    const char* hint;   // units / note (static subtitle)
};
constexpr ConfigSpec kConfigSpecs[] = {
    {"lattice",          "Lattice L",        CfgKind::Lattice,    4.0, 256.0, 8.0,  "reboots the scenario"},
    {"dt",               "dt (time step)",   CfgKind::Double,     1.0, 20.0,  0.5,  "≥1 unless symplectic"},
    {"sor",              "SOR iterations",   CfgKind::Int,        1.0, 60.0,  1.0,  "Poisson solver depth"},
    {"boundary",         "Flux boundary",    CfgKind::Boundary,   0.0, 2.0,   1.0,  "field wrap law"},
    {"langevin_T",       "Langevin T",       CfgKind::Double,     0.0, 5.0,   0.1,  "thermostat temperature"},
    {"langevin_gamma",   "Langevin gamma",   CfgKind::Double,     0.0, 1.0,   0.01, "OU friction"},
    {"langevin_seed",    "Langevin seed",    CfgKind::UInt,       0.0, 1.0e6, 1.0,  "RNG reproducibility"},
    {"langevin_site",    "Langevin sites",   CfgKind::SiteFilter, 0.0, 3.0,   1.0,  "parity filter"},
    {"bcc_stencil",      "BCC stencil",      CfgKind::Bcc,        0.0, 3.0,   1.0,  "sublattice Laplacian"},
    {"coulomb_coupling", "Coulomb coupling", CfgKind::Double,     0.0, 5.0,   0.1,  "Gauss source scale"},
    {"coulomb_source",   "Coulomb source Z", CfgKind::Double,     1.0, 4.0,   1.0,  "nuclear charge Z"},
    {"omega0",           "omega0 (clock)",   CfgKind::Double,     0.0, 2.0,   0.1,  "de Broglie frequency"},
    {"kinetic_drain",    "Kinetic drain",    CfgKind::Double,     0.0, 1.0,   0.05, "genesis kinetic drain"},
};
const ConfigSpec* find_config_spec(std::string_view key) {
    for (const auto& s : kConfigSpecs)
        if (key == s.key) return &s;
    return nullptr;
}
Rml::String boundary_label(int v) {
    switch (v) { case 0: return "Periodic"; case 1: return "Reflective"; default: return "Dispersal"; }
}
Rml::String bcc_label(int v) {
    switch (v) { case 1: return "SC"; case 2: return "FCC"; case 3: return "BCC"; default: return "FULL"; }
}
Rml::String site_label(int v) {
    switch (v) { case 0: return "SC"; case 1: return "BCC"; case 2: return "FCC"; default: return "ALL"; }
}
Rml::String config_value_str(const ConfigSpec& s, double v) {
    switch (s.kind) {
        case CfgKind::Boundary:   return boundary_label(static_cast<int>(std::lround(v)));
        case CfgKind::Bcc:        return bcc_label(static_cast<int>(std::lround(v)));
        case CfgKind::SiteFilter: return site_label(static_cast<int>(std::lround(v)));
        case CfgKind::Int:
        case CfgKind::UInt:
        case CfgKind::Lattice: {
            char b[32];
            std::snprintf(b, sizeof(b), "%d", static_cast<int>(std::lround(v)));
            return Rml::String(b);
        }
        default: {
            char b[32];
            std::snprintf(b, sizeof(b), "%.2f", v);
            return Rml::String(b);
        }
    }
}

// One tick in physical seconds (electron-primary gauge: t_phys = t_P/√3, see
// CLAUDE.md). Used only to render a human-facing "physical time" in the status
// bar; nothing physical depends on it.
constexpr double kTPhysSeconds = 3.11e-44;

// ── The FIELDS overlay panel (Scale-0 only) mirrors the web's 7-column,
//    multi-select overlay menu. Every selectable overlay is a row in the shared
//    registry (native/scale0_overlays.h): the panel groups those rows by column
//    and the --overlays CLI resolves them by their stable `name`. Adding an
//    overlay in a later tranche is a single registry row — no wiring here moves.

// Split a comma-separated "--overlays a,b,c" value into trimmed tokens.
std::vector<std::string> split_csv(const std::string& s) {
    std::vector<std::string> out;
    std::string cur;
    for (char c : s) {
        if (c == ',') {
            if (!cur.empty()) out.push_back(cur);
            cur.clear();
        } else if (c != ' ') {
            cur.push_back(c);
        }
    }
    if (!cur.empty()) out.push_back(cur);
    return out;
}

// ── RmlUi data-model mirror of UiSnapshot (the bound C++ side of the shell) ──
// One physics toggle row (bound into the shell). `name`/`desc`/`req` come from
// the TOGGLE_SPECS row (static); `on`/`gated` are the live engine truth. `req`
// is the discoverable requires/conflicts/gpu-only metadata; `gated` lights amber
// when the current combo already violates a rule involving this enabled toggle.
struct FullToggleRow {
    Rml::String name;
    Rml::String desc;
    Rml::String req;
    bool on = false;
    bool gated = false;
    bool has_req = false;
};
// One collapsible toggle category. Items are built only while `expanded` (the
// scenario-picker DOM-shrink pattern), so a closed category costs ~0 DOM.
struct ToggleGroupRow {
    Rml::String title;
    bool expanded = false;
    Rml::String count;
    Rml::Vector<FullToggleRow> items;
};
// One config-knob row: label + current value + −/＋ nudge. `key` maps to the
// ConfigSpec / command dispatch. (The reset-to-defaults button is a static RML
// element, not a data row.)
struct ConfigRow {
    Rml::String key;
    Rml::String label;
    Rml::String vstr;
    Rml::String hint;
};

// One overlay toggle row bound into the shell (Scale-0 panel). `on` lights the
// LED when the overlay is in the active set (multi-select). Rubber-sheet rows
// (`is_sheet`) also carry a slice height: when active they expose a −/＋ height
// nudge in the panel and `hstr` shows the value (e.g. "y 0.42").
struct OverlayRow {
    Rml::String name;   // stable overlay id (registry name)
    Rml::String label;  // human label
    bool on = false;
    bool is_sheet = false;  // true for rubber-sheet overlays (height-adjustable)
    float height = 0.0f;    // current slice height (fraction of the lattice box)
    Rml::String hstr;       // formatted height ("0.42") for display
};
// One overlay-menu column (Volume / Fields / Forces / …). `expanded` gates the
// collapsible list; `items` are the overlays grouped into this column.
struct OverlayColumnRow {
    Rml::String title;
    bool expanded = true;
    Rml::Vector<OverlayRow> items;
};

// One key/value line in the click-to-inspect readout (bound into the shell).
struct InspLine {
    Rml::String k;
    Rml::String v;
};

// One Scale-0 scenario row in the Setup picker (bound into the shell). `current`
// highlights the loaded scenario; `visible` gates the row under the live search
// filter. `search` is the lowercased "title id tags" haystack — used by the
// C++ filter, not bound into RML.
struct ScenarioRow {
    Rml::String id;
    Rml::String title;
    bool current = false;
    bool visible = true;
    Rml::String search;
};
// One category group in the picker: the 5 honest scenario classes, collapsible.
// `has_visible` = any item passes the filter (gates the header); `show_items` =
// header expanded OR a filter is active (gates the item list). `count` is the
// visible-item tally shown in the header.
struct ScenarioGroupRow {
    Rml::String title;
    bool expanded = true;
    bool has_visible = true;
    bool show_items = true;
    Rml::String count;
    Rml::Vector<ScenarioRow> items;
};

struct ShellData {
    int tick = 0;
    int active_scale = 0;   // drives the toolbar scale-switcher highlight
    int particle_count = 0;
    Rml::String physical_time = "0 s";
    Rml::String total_energy = "0.0";
    Rml::String s1_ke = "0.0";   // Scale-1 kinetic energy (particle readout)
    Rml::String s1_pe = "0.0";   // Scale-1 potential energy (particle readout)
    Rml::String scenario;
    Rml::String backend = "CPU";
    Rml::String lattice = "0";
    int fps = 0;
    bool running = false;
    // ── Physics control surface (Scale-0). Data-driven from ftd::TOGGLE_SPECS[]
    //    (all 44 toggles) + the config knobs, both collapsed by default so the
    //    steady-state DOM stays tiny (fps). `phys_open` gates the toggle
    //    categories; `cfg_open` gates the config knobs. While a section (or a
    //    category) is closed its bound array is left EMPTY, so the nested
    //    data-for instantiates ~0 rows — the same DOM-shrink discipline the
    //    scenario picker uses.
    bool phys_open = false;
    bool cfg_open = false;
    Rml::Vector<ToggleGroupRow> toggle_groups;   // the 44 toggles, categorized
    Rml::Vector<ConfigRow> config_rows;          // dt / SOR / boundary / … + reset
    // Live validation banner: TermToggles::validate() (+ CPU runtime warnings)
    // surfaced from the authoritative engine state. `has_validation` gates the
    // banner; `validation_msg` is the first offending rule.
    bool has_validation = false;
    Rml::String validation_msg;
    // FIELDS overlay panel (Scale-0). The 7-column, multi-select overlay menu
    // (mirrors the web). Built from the shared registry, grouped by column;
    // empty columns are omitted. Each row's `on` reflects the active set.
    Rml::Vector<OverlayColumnRow> overlay_columns;
    // Setup scenario picker (Scale-0). The ~130 native-catalog scenarios grouped
    // into the 5 honest classes, searchable + collapsible. A pick issues the
    // LoadScenario core command (the Reset path) — a live scenario reboot.
    // `scn_open` gates the whole list: COLLAPSED by default (the "Scenarios ▾"
    // header expands it). While collapsed, `scenario_groups` is left EMPTY so the
    // data-for instantiates ~0 scenario <div>s — the ~130-item DOM (which forced a
    // full-document RmlUi relayout + geometry regen every frame) simply does not
    // exist until the user opens the picker.
    bool scn_open = false;
    Rml::Vector<ScenarioGroupRow> scenario_groups;
    // Click-to-inspect readout. `insp_active` gates the panel section (data-if);
    // `insp_title` names the picked entity; `insp_lines` are its live fields.
    bool insp_active = false;
    Rml::String insp_title;
    Rml::Vector<InspLine> insp_lines;
    // ── Telemetry section (Scale-0 engine telemetry hub) ──────────────────────
    // Collapsible + COLLAPSED by default (fps: the chart elements + legend rows
    // live behind data-if="tel_open", so a closed section costs ~0 GUI work and
    // the audit/lagrangian scheduler groups aren't demanded). The <ftd-chart>
    // traces come from app-side ring buffers (registry-bound, drawn through the
    // D3D12 line path); these strings are the throttled current-value + freshness
    // legend read from the published Scale0Snapshot telemetry channels.
    bool tel_open = false;
    // Diagnostics group (always demanded; cheap cadence-1).
    Rml::String tel_d_energy = "—";   // accounted total energy (E_curr)
    Rml::String tel_d_manif = "—";    // manifested particle count
    Rml::String tel_d_entropy = "—";  // total entropy
    Rml::String tel_d_charge = "—";   // net charge (positive − negative)
    Rml::String tel_diag_prov = "—";  // freshness: sampled tick of the diag group
    // Audit / conservation group (demanded only while the section is open).
    Rml::String tel_a_energy = "—";   // accounted total energy (audit reduction)
    Rml::String tel_a_drift = "—";    // energy drift dE/dt (the conservation view)
    Rml::String tel_a_gauss = "—";    // Gauss-law residual Σ|div J − s|²
    Rml::String tel_audit_prov = "—"; // freshness: sampled tick of the audit group
    // Lagrangian group (demanded only while the section is open).
    Rml::String tel_l_lag = "—";      // total Lagrangian ℒ
    Rml::String tel_l_ham = "—";      // total Hamiltonian ℋ
};

// ── Physics-control helpers (build + live-sync the toggle/config model) ──────

// The static requires/conflicts/gpu-only metadata for one TOGGLE_SPECS row,
// formatted as the discoverable subtitle shown under the toggle label. Built
// once (per row); does not depend on live state.
Rml::String toggle_req_text(const ftd::ToggleSpec& spec) {
    std::string out;
    if (spec.requires_ && *spec.requires_) {
        out += "needs ";
        out += spec.requires_;
    }
    if (spec.conflicts && *spec.conflicts) {
        if (!out.empty()) out += " · ";
        out += "conflicts ";
        out += spec.conflicts;
    }
    if (spec.gpu_only_warning && *spec.gpu_only_warning) {
        if (!out.empty()) out += " · ";
        out += "GPU-only";
    }
    return Rml::String(out);
}

// Live "gated" flag for an ENABLED toggle: true when the current combo already
// violates a requires/conflicts rule involving it (a required dep is off, or a
// conflicting toggle is on). Surfaces validation per-row without blocking the
// click (the engine's own validate() is authoritative; strict_validation off ⇒
// stderr-warn). Reads the live TermToggles the snapshot published.
bool toggle_gated(const ftd::ToggleSpec& spec, const ftd::TermToggles& tt) {
    if (!(tt.*(spec.field))) return false;  // only an enabled toggle can be gated
    bool gated = false;
    ftd::term_toggles_detail::for_each_csv(spec.requires_, [&](std::string_view dep) {
        const ftd::ToggleSpec* ds = ftd::term_toggles_detail::find_spec(dep);
        if (ds && !(tt.*(ds->field))) gated = true;
    });
    if (spec.conflicts && *spec.conflicts) {
        const ftd::ToggleSpec* cs = ftd::term_toggles_detail::find_spec(spec.conflicts);
        if (cs && (tt.*(cs->field))) gated = true;
    }
    return gated;
}

// Fill a FullToggleRow's live fields (on/gated) from the engine truth. Returns
// true if anything changed (so the caller can dirty the binding).
bool refresh_toggle_row(FullToggleRow& r, const ftd::TermToggles& tt) {
    const ftd::ToggleSpec* spec = ftd::term_toggles_detail::find_spec(r.name.c_str());
    if (!spec) return false;
    bool changed = false;
    const bool on = tt.*(spec->field);
    if (r.on != on) { r.on = on; changed = true; }
    const bool gated = toggle_gated(*spec, tt);
    if (r.gated != gated) { r.gated = gated; changed = true; }
    return changed;
}

// Build one category's item rows from TOGGLE_SPECS (only the toggles that map to
// this category), seeded from the live engine truth `tt`.
void build_toggle_group_items(ToggleGroupRow& g, int cat, const ftd::TermToggles& tt) {
    g.items.clear();
    for (const ftd::ToggleSpec& spec : ftd::TOGGLE_SPECS) {
        if (toggle_category(spec.name) != cat) continue;
        FullToggleRow r;
        r.name = spec.name;
        r.desc = spec.description ? Rml::String(spec.description) : Rml::String();
        r.req = toggle_req_text(spec);
        r.has_req = !r.req.empty();
        r.on = tt.*(spec.field);
        r.gated = toggle_gated(spec, tt);
        g.items.push_back(std::move(r));
    }
}

// Count how many TOGGLE_SPECS rows fall into a category (for the header tally).
int toggle_group_count(int cat) {
    int n = 0;
    for (const ftd::ToggleSpec& spec : ftd::TOGGLE_SPECS)
        if (toggle_category(spec.name) == cat) ++n;
    return n;
}

// Rebuild the toggle-category headers. Items are built only for categories whose
// title is in `expanded` (the DOM-shrink pattern); a closed category holds 0
// item rows. Called on a STRUCTURAL change (panel open, category expand/collapse)
// — never per frame; the live on/gated values update in place via
// refresh_toggle_row below.
void rebuild_toggle_groups(ShellData* data, const ftd::TermToggles& tt,
                           const std::vector<std::string>& expanded) {
    if (!data) return;
    data->toggle_groups.clear();
    if (!data->phys_open) return;  // closed section ⇒ no headers, no rows
    for (int cat = 0; cat < static_cast<int>(std::size(kToggleCategories)); ++cat) {
        ToggleGroupRow g;
        g.title = kToggleCategories[cat];
        g.expanded = std::find(expanded.begin(), expanded.end(),
                               std::string(kToggleCategories[cat])) != expanded.end();
        g.count = std::to_string(toggle_group_count(cat));
        if (g.expanded) build_toggle_group_items(g, cat, tt);
        data->toggle_groups.push_back(std::move(g));
    }
}

// The live numeric value of one config knob, read from the authoritative engine
// truth (TermToggles + the published bridge knobs). Enum kinds return the raw
// integer mode. Lattice/dt/SOR come from the knobs; the rest from TermToggles.
double config_current(const ConfigSpec& s, const ftd::TermToggles& tt,
                      const ftd::native::BridgeKnobs& knobs) {
    const std::string_view k(s.key);
    if (k == "lattice") return static_cast<double>(knobs.lattice_size);
    if (k == "dt") return knobs.dt;
    if (k == "sor") return static_cast<double>(knobs.sor_iterations);
    if (k == "boundary") return static_cast<double>(static_cast<int>(tt.flux_boundary));
    if (k == "langevin_T") return tt.langevin_T;
    if (k == "langevin_gamma") return tt.langevin_gamma;
    if (k == "langevin_seed") return static_cast<double>(tt.langevin_seed);
    if (k == "langevin_site") return static_cast<double>(static_cast<int>(tt.langevin_site_filter));
    if (k == "bcc_stencil") return static_cast<double>(static_cast<int>(tt.bcc_stencil));
    if (k == "coulomb_coupling") return tt.coulomb_charge_coupling;
    if (k == "coulomb_source") return tt.coulomb_source_scale;
    if (k == "omega0") return tt.omega0;
    if (k == "kinetic_drain") return tt.kinetic_drain;
    return 0.0;
}

// Build the config-knob rows (one per ConfigSpec) + a trailing reset button row,
// seeded from the live engine truth. Called when the config section opens.
void build_config_rows(ShellData* data, const ftd::TermToggles& tt,
                       const ftd::native::BridgeKnobs& knobs) {
    if (!data) return;
    data->config_rows.clear();
    if (!data->cfg_open) return;
    for (const ConfigSpec& s : kConfigSpecs) {
        ConfigRow r;
        r.key = s.key;
        r.label = s.label;
        r.hint = s.hint;
        r.vstr = config_value_str(s, config_current(s, tt, knobs));
        data->config_rows.push_back(std::move(r));
    }
}

// Refresh the built config rows' displayed values from the live engine truth.
// Returns true if any value string changed (so the caller can dirty).
bool refresh_config_rows(ShellData* data, const ftd::TermToggles& tt,
                         const ftd::native::BridgeKnobs& knobs) {
    if (!data) return false;
    bool changed = false;
    for (ConfigRow& r : data->config_rows) {
        const ConfigSpec* s = find_config_spec(r.key.c_str());
        if (!s) continue;
        Rml::String v = config_value_str(*s, config_current(*s, tt, knobs));
        if (r.vstr != v) { r.vstr = v; changed = true; }
    }
    return changed;
}

// Format a sheet height fraction for the panel (2 decimals, e.g. "0.42").
Rml::String sheet_hstr(float h) {
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%.2f", h);
    return Rml::String(buf);
}

// Build the Scale-0 overlay panel model: walk the 7 columns in menu order and
// collect each column's registry rows. Columns with no rows (the tranches that
// have not landed their overlays yet, e.g. Quantum / Stress-Energy) are omitted
// entirely, so the panel grows a column automatically when a registry row for
// it appears.
Rml::Vector<OverlayColumnRow> build_overlay_columns() {
    Rml::Vector<OverlayColumnRow> cols;
    for (std::uint32_t c = 0; c < static_cast<std::uint32_t>(ftd::native::OverlayColumn::Count);
         ++c) {
        const auto column = static_cast<ftd::native::OverlayColumn>(c);
        OverlayColumnRow row;
        row.title = ftd::native::overlay_column_title(column);
        row.expanded = true;
        for (const ftd::native::OverlayDescriptor& d : ftd::native::kOverlayRegistry) {
            if (d.column != column) continue;
            OverlayRow r;
            r.name = d.name;
            r.label = d.label;
            r.on = false;
            r.is_sheet = (d.render == ftd::native::OverlayRender::Sheet);
            r.height = d.y_frac;                 // seeded to the registry default
            r.hstr = sheet_hstr(d.y_frac);
            row.items.push_back(std::move(r));
        }
        if (!row.items.empty()) cols.push_back(std::move(row));
    }
    return cols;
}

// Find an overlay row (by stable name) across all columns; nullptr on miss.
OverlayRow* find_overlay_row(ShellData* data, const Rml::String& name) {
    if (!data) return nullptr;
    for (OverlayColumnRow& col : data->overlay_columns)
        for (OverlayRow& r : col.items)
            if (r.name == name) return &r;
    return nullptr;
}

// ASCII-lowercase a string (search haystack / filter normalization). Multibyte
// UTF-8 bytes pass through unchanged; the picker filter matches on ASCII.
std::string to_lower(std::string s) {
    for (char& c : s) c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));
    return s;
}

// The 5 honest Scale-0 scenario classes, in catalog order — the group headers of
// the Setup picker. Every native-catalog row's `category` is one of these.
constexpr const char* kScenarioCategories[] = {
    "1. Validated Native Dynamics",
    "2. Validated State Dynamics",
    "3. Qualified Selected Extensions",
    "4. Validated Initial Data",
    "5. Macroscopic Physics & Measurement",
};

// Rebuild the Setup picker view (data->scenario_groups) from the native scenario
// catalog (native/scenario_catalog.h), honoring the live search filter and the
// per-group expanded set. This is the DOM-shrink half of the framerate fix: only
// a group that is EXPANDED (or force-opened by an active filter) instantiates its
// matching item rows, and a filter keeps only matching rows — so the live
// scenario DOM stays small in every browsing state (0 rows when the picker is
// closed, a handful when one group is open or a filter is active, instead of all
// ~130 <div>s at once). Every rebuild is a user-action event (open / filter /
// group toggle), never per-frame. `current` highlights the loaded scenario;
// `expanded` holds the titles of the groups the user has opened.
void rebuild_scenario_view(ShellData* data, const std::string& current,
                           const std::string& raw_filter,
                           const std::vector<std::string>& expanded) {
    if (!data) return;
    data->scenario_groups.clear();
    const std::string f = to_lower(raw_filter);
    const bool filtering = !f.empty();
    for (const char* cat : kScenarioCategories) {
        // A filter force-opens every matching group; otherwise honor the user's
        // per-group expanded state. Closed groups instantiate zero item rows.
        const bool is_open =
            filtering
            || std::find(expanded.begin(), expanded.end(), std::string(cat))
                   != expanded.end();
        ScenarioGroupRow g;
        g.title = cat;
        bool any_in_cat = false;
        int matches = 0;
        for (const ftd::native::ScenarioMeta& m : ftd::native::SCENARIO_META) {
            if (std::string_view(m.category) != cat) continue;
            any_in_cat = true;
            const bool hit =
                !filtering
                || to_lower(std::string(m.title) + " " + m.id + " "
                            + (m.tags ? m.tags : ""))
                           .find(f) != std::string::npos;
            if (!hit) continue;
            ++matches;
            if (is_open) {  // only an open group builds its (matching) rows
                ScenarioRow r;
                r.id = m.id;
                r.title = m.title;
                r.current = (current == m.id);
                r.visible = true;
                g.items.push_back(std::move(r));
            }
        }
        if (!any_in_cat) continue;  // omit empty classes
        g.expanded = is_open;
        g.has_visible = (matches > 0);
        g.show_items = g.has_visible && is_open;
        g.count = std::to_string(matches);
        data->scenario_groups.push_back(std::move(g));
    }
}

// Set the picker's current-row highlight to the loaded scenario id. Returns true
// iff a `current` flag actually changed (so the caller can dirty the binding).
bool set_current_scenario(ShellData* data, const std::string& id) {
    if (!data) return false;
    bool changed = false;
    for (ScenarioGroupRow& g : data->scenario_groups)
        for (ScenarioRow& s : g.items) {
            const bool cur = (s.id == id);
            if (s.current != cur) { s.current = cur; changed = true; }
        }
    return changed;
}

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
};

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
void request_telemetry_demand(AppContext* app, bool panel_open) {
    ftd::native::DataNeeds needs;
    needs.telemetry_groups = ftd::TELEMETRY_DIAGNOSTICS;
    if (panel_open)
        needs.telemetry_groups |= ftd::TELEMETRY_AUDIT | ftd::TELEMETRY_LAGRANGIAN;
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

// ── Win32 → RmlUi input plumbing ──
int rml_key_modifiers() {
    int m = 0;
    if (GetKeyState(VK_CONTROL) & 0x8000) m |= Rml::Input::KM_CTRL;
    if (GetKeyState(VK_SHIFT) & 0x8000) m |= Rml::Input::KM_SHIFT;
    if (GetKeyState(VK_MENU) & 0x8000) m |= Rml::Input::KM_ALT;
    return m;
}

// True when the given client point is inside the laid-out #viewport hole, i.e.
// the pointer is over the 3D scene and should drive the camera rather than the
// (transparent) RML element that marks the hole.
bool over_viewport(const AppContext* app, int x, int y) {
    return ftd::native::scene_contains_client(app->viewport_rect, x, y);
}

AppContext* app_from_hwnd(HWND hwnd) {
    return reinterpret_cast<AppContext*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
}

LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    AppContext* app = app_from_hwnd(hwnd);
    Rml::Context* ctx = app ? app->context : nullptr;
    switch (msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_DPICHANGED:
            ftd::native::apply_dpi_suggested_rect(hwnd, lparam);
            return 0;
        case WM_MOUSEMOVE: {
            const int x = lparam_x(lparam), y = lparam_y(lparam);
            if (ctx) ctx->ProcessMouseMove(x, y, rml_key_modifiers());
            if (app->dragging) {
                app->camera->yaw += (x - app->last.x) * 0.01f;
                app->camera->pitch += (y - app->last.y) * 0.01f;
                app->camera->pitch = std::max(-1.4f, std::min(1.4f, app->camera->pitch));
                app->last = {x, y};
                // Once the pointer leaves the slop box this press is an orbit
                // drag, not a click — suppress the pick on release.
                if (std::abs(x - app->press_pt.x) > kClickSlop
                    || std::abs(y - app->press_pt.y) > kClickSlop)
                    app->drag_moved = true;
            }
            return 0;
        }
        case WM_LBUTTONDOWN: {
            const int x = lparam_x(lparam), y = lparam_y(lparam);
            if (ctx) ctx->ProcessMouseButtonDown(0, rml_key_modifiers());
            if (over_viewport(app, x, y)) {
                app->dragging = true;
                app->last = {x, y};
                app->press_pt = {x, y};
                app->press_in_viewport = true;
                app->drag_moved = false;
                SetCapture(hwnd);
            } else {
                app->press_in_viewport = false;
            }
            return 0;
        }
        case WM_LBUTTONUP: {
            const int x = lparam_x(lparam), y = lparam_y(lparam);
            if (ctx) ctx->ProcessMouseButtonUp(0, rml_key_modifiers());
            app->dragging = false;
            if (GetCapture() == hwnd) ReleaseCapture();
            // A press+release inside the viewport with negligible travel is a
            // CLICK → request a scene pick. The GUI loop (which owns the frame,
            // camera, and viewport rect) unprojects + picks; wnd_proc only flags
            // it. A drag (camera already orbited) is ignored here.
            if (app->press_in_viewport && !app->drag_moved && over_viewport(app, x, y)) {
                app->pick_pending = true;
                app->pick_x = x;
                app->pick_y = y;
            }
            app->press_in_viewport = false;
            return 0;
        }
        case WM_RBUTTONDOWN:
            if (ctx) ctx->ProcessMouseButtonDown(1, rml_key_modifiers());
            return 0;
        case WM_RBUTTONUP:
            if (ctx) ctx->ProcessMouseButtonUp(1, rml_key_modifiers());
            return 0;
        case WM_CAPTURECHANGED:
        case WM_KILLFOCUS:
            if (app) app->dragging = false;
            return DefWindowProcW(hwnd, msg, wparam, lparam);
        case WM_MOUSEWHEEL: {
            POINT pt{lparam_x(lparam), lparam_y(lparam)};
            ScreenToClient(hwnd, &pt);
            const int delta = GET_WHEEL_DELTA_WPARAM(wparam);
            const bool shift = (GET_KEYSTATE_WPARAM(wparam) & MK_SHIFT) != 0;
            if (over_viewport(app, pt.x, pt.y)) {
                if (shift) {
                    // Shift+wheel over the scene sweeps the most-recently active
                    // sheet up/down through the lattice (tactile height control).
                    // Plain wheel keeps the camera zoom below — so this does NOT
                    // break the existing orbit-camera controls.
                    nudge_last_sheet(app, delta > 0 ? 0.03f : -0.03f);
                } else {
                    app->camera->distance *= (delta > 0) ? 0.9f : 1.1f;
                    app->camera->distance = std::max(4.0f, std::min(512.0f, app->camera->distance));
                }
            } else if (ctx) {
                ctx->ProcessMouseWheel(Rml::Vector2f(0.0f, delta > 0 ? -1.0f : 1.0f),
                                       rml_key_modifiers());
            }
            return 0;
        }
        default:
            return DefWindowProcW(hwnd, msg, wparam, lparam);
    }
}

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

// ── PNG writer (WIC, RGBA8 rows) — mirrors test_ui_rml_smoke.cpp's save_png ──
std::wstring widen(const std::string& s) {
    if (s.empty()) return std::wstring();
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()), nullptr, 0);
    std::wstring w(static_cast<size_t>(n), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()), w.data(), n);
    return w;
}

bool save_png(const std::wstring& path, const std::uint8_t* rgba, UINT w, UINT h,
              UINT row_pitch) {
    ComPtr<IWICImagingFactory> factory;
    if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                                IID_PPV_ARGS(&factory))))
        return false;
    ComPtr<IWICStream> stream;
    if (FAILED(factory->CreateStream(&stream))) return false;
    if (FAILED(stream->InitializeFromFilename(path.c_str(), GENERIC_WRITE))) return false;
    ComPtr<IWICBitmapEncoder> encoder;
    if (FAILED(factory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &encoder))) return false;
    if (FAILED(encoder->Initialize(stream.Get(), WICBitmapEncoderNoCache))) return false;
    ComPtr<IWICBitmapFrameEncode> frame;
    ComPtr<IPropertyBag2> props;
    if (FAILED(encoder->CreateNewFrame(&frame, &props))) return false;
    if (FAILED(frame->Initialize(props.Get()))) return false;
    if (FAILED(frame->SetSize(w, h))) return false;
    WICPixelFormatGUID fmt = GUID_WICPixelFormat32bppBGRA;
    if (FAILED(frame->SetPixelFormat(&fmt))) return false;
    ComPtr<IWICBitmap> source;
    if (FAILED(factory->CreateBitmapFromMemory(w, h, GUID_WICPixelFormat32bppRGBA, row_pitch,
                                               row_pitch * h, const_cast<BYTE*>(rgba), &source)))
        return false;
    if (FAILED(frame->WriteSource(source.Get(), nullptr))) return false;
    if (FAILED(frame->Commit())) return false;
    if (FAILED(encoder->Commit())) return false;
    return true;
}

// Bind stdout/stderr to the launching console so --capture-frames logging is
// visible for a WIN32-subsystem exe (copied from the native_desktop reference).
bool bind_crt_to_std_handle(DWORD std_id, int crt_fd) {
    HANDLE handle = GetStdHandle(std_id);
    if (handle == nullptr || handle == INVALID_HANDLE_VALUE) return false;
    const DWORD type = GetFileType(handle);
    if (type != FILE_TYPE_DISK && type != FILE_TYPE_PIPE && type != FILE_TYPE_CHAR) return false;
    const int fd = _open_osfhandle(reinterpret_cast<intptr_t>(handle), _O_TEXT);
    if (fd < 0) return false;
    _dup2(fd, crt_fd);
    return true;
}
void attach_parent_console_if_any() {
    const bool out_bound = bind_crt_to_std_handle(STD_OUTPUT_HANDLE, 1);
    bind_crt_to_std_handle(STD_ERROR_HANDLE, 2);
    if (out_bound) { std::cout.clear(); std::cerr.clear(); return; }
    if (!AttachConsole(ATTACH_PARENT_PROCESS)) return;
    FILE* s = nullptr;
    freopen_s(&s, "CONOUT$", "w", stdout);
    s = nullptr;
    freopen_s(&s, "CONOUT$", "w", stderr);
    std::cout.clear();
    std::cerr.clear();
}

std::string wide_to_utf8(const wchar_t* wide) {
    if (!wide || wide[0] == L'\0') return {};
    const int bytes = WideCharToMultiByte(CP_UTF8, 0, wide, -1, nullptr, 0, nullptr, nullptr);
    if (bytes <= 1) return {};
    std::string out(static_cast<size_t>(bytes - 1), '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide, -1, out.data(), bytes, nullptr, nullptr);
    return out;
}
std::vector<std::string> utf8_args() {
    int argc = 0;
    LPWSTR* wargv = CommandLineToArgvW(GetCommandLineW(), &argc);
    std::vector<std::string> args;
    if (!wargv) return {"native_app"};
    for (int i = 0; i < argc; ++i) args.push_back(wide_to_utf8(wargv[i]));
    LocalFree(wargv);
    if (args.empty()) args.emplace_back("native_app");
    return args;
}

std::string upper(std::string s) {
    for (char& c : s) c = static_cast<char>(::toupper(static_cast<unsigned char>(c)));
    return s;
}
std::string fmt(const char* f, double v) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), f, v);
    return buf;
}
std::string fmt3(const char* f, double a, double b, double c) {
    char buf[96];
    std::snprintf(buf, sizeof(buf), f, a, b, c);
    return buf;
}

void apply_camera_for_lattice(ftd::native::Camera& cam, int lattice) {
    const float c = static_cast<float>(lattice) * 0.5f;
    cam.target_x = cam.target_y = cam.target_z = c;
    cam.distance = static_cast<float>(lattice) * 1.8f;
}

// ── Click-to-inspect: unproject a scene click to a world ray, then pick ──────
struct PickRay {
    float ox = 0.0f, oy = 0.0f, oz = 0.0f;   // origin (camera eye)
    float dx = 0.0f, dy = 1.0f, dz = 0.0f;   // unit direction (into the scene)
};

// Build a world-space ray from a click at (client_x, client_y) inside `rect`,
// using the SAME orbit-camera math D3D12Presenter::render() uses (look_at + DX
// perspective, row-vector convention). No 4×4 inverse: the eye + view basis are
// reconstructed directly and the view-space ray direction (nx·tan·aspect,
// ny·tan, 1) is rotated into world by that basis. Ray = origin + t·dir, t ≥ 0.
PickRay make_pick_ray(const ftd::native::Camera& cam, const ftd::native::SceneRect& rect,
                      int client_x, int client_y) {
    const float w = rect.width > 0 ? static_cast<float>(rect.width) : 1.0f;
    const float h = rect.height > 0 ? static_cast<float>(rect.height) : 1.0f;
    const float ndc_x = 2.0f * (static_cast<float>(client_x - rect.x)) / w - 1.0f;
    const float ndc_y = 1.0f - 2.0f * (static_cast<float>(client_y - rect.y)) / h;
    const float aspect = w / h;
    const float tan_half = std::tan(cam.fov_y * 0.5f);

    // eye + forward — identical to the presenter's eye_{x,y,z} + look_at forward.
    const float cp = std::cos(cam.pitch);
    const float ex = cam.target_x + cam.distance * cp * std::sin(cam.yaw);
    const float ey = cam.target_y + cam.distance * std::sin(cam.pitch);
    const float ez = cam.target_z + cam.distance * cp * std::cos(cam.yaw);
    float fx = cam.target_x - ex, fy = cam.target_y - ey, fz = cam.target_z - ez;
    float fl = std::sqrt(fx * fx + fy * fy + fz * fz);
    if (fl < 1e-6f) fl = 1.0f;
    fx /= fl; fy /= fl; fz /= fl;
    // right = normalize(cross(forward, up)) with up = (0,1,0) → (-fz, 0, fx).
    float sx = -fz, sy = 0.0f, sz = fx;
    float sl = std::sqrt(sx * sx + sy * sy + sz * sz);
    if (sl < 1e-6f) sl = 1.0f;
    sx /= sl; sy /= sl; sz /= sl;
    // up2 = cross(right, forward).
    const float ux = sy * fz - sz * fy;
    const float uy = sz * fx - sx * fz;
    const float uz = sx * fy - sy * fx;

    const float vx = ndc_x * tan_half * aspect;
    const float vy = ndc_y * tan_half;
    float dx = vx * sx + vy * ux + fx;
    float dy = vx * sy + vy * uy + fy;
    float dz = vx * sz + vy * uz + fz;
    float dl = std::sqrt(dx * dx + dy * dy + dz * dz);
    if (dl < 1e-6f) dl = 1.0f;
    return PickRay{ex, ey, ez, dx / dl, dy / dl, dz / dl};
}

// Perpendicular distance from world point P to the ray; t_out = distance along
// the (unit) direction (in front of the camera when > 0).
float ray_perp(const PickRay& r, float px, float py, float pz, float& t_out) {
    const float wx = px - r.ox, wy = py - r.oy, wz = pz - r.oz;
    const float t = wx * r.dx + wy * r.dy + wz * r.dz;
    t_out = t;
    const float cx = wx - t * r.dx, cy = wy - t * r.dy, cz = wz - t * r.dz;
    return std::sqrt(cx * cx + cy * cy + cz * cz);
}

// A sample is "hit" when it is in front of the camera and within a narrow
// angular cone (0.05·t ≈ 2.9°), with a 1.2-unit floor so nearby samples stay
// easy to click. The cone (rather than a fixed world radius) keeps distant
// samples clickable under perspective.
inline bool ray_hits(float perp, float t) {
    return t > 0.0f && perp < std::max(1.2f, 0.05f * t);
}

// Scale 0: nearest rendered sample to the ray → its lattice cell. Manifested
// particles are preferred; the ambient flux cloud is the fallback so a click on
// a field-only region still resolves a cell. Returns false (→ clear the
// inspector) when nothing is near the ray — a click on empty space.
bool pick_scale0(const ftd::native::NativeFrame& frame, const PickRay& ray, int L,
                 int& vx, int& vy, int& vz) {
    auto scan = [&](const std::vector<ftd::native::NativeParticle>& pts, float& best_perp,
                    float& bx, float& by, float& bz) {
        bool any = false;
        for (const ftd::native::NativeParticle& p : pts) {
            float t = 0.0f;
            const float perp = ray_perp(ray, p.x, p.y, p.z, t);
            if (ray_hits(perp, t) && perp < best_perp) {
                best_perp = perp; bx = p.x; by = p.y; bz = p.z; any = true;
            }
        }
        return any;
    };
    float bp = 1e30f, bx = 0.0f, by = 0.0f, bz = 0.0f;
    bool hit = scan(frame.particles, bp, bx, by, bz);
    if (!hit) hit = scan(frame.flux, bp, bx, by, bz);
    if (!hit) return false;
    const int hi = std::max(0, L - 1);
    vx = std::min(hi, std::max(0, static_cast<int>(std::floor(bx))));
    vy = std::min(hi, std::max(0, static_cast<int>(std::floor(by))));
    vz = std::min(hi, std::max(0, static_cast<int>(std::floor(bz))));
    return true;
}

// Scale 1: nearest particle to the ray → its index. frame.particles is 1:1 with
// the engine's particle list (Scale1Adapter::capture() preserves order), so the
// index feeds InspectParticle1 directly. Returns false (→ clear) on a miss.
bool pick_scale1(const ftd::native::NativeFrame& frame, const PickRay& ray, int& pidx) {
    float bp = 1e30f;
    int best = -1;
    for (std::size_t i = 0; i < frame.particles.size(); ++i) {
        const ftd::native::NativeParticle& p = frame.particles[i];
        float t = 0.0f;
        const float perp = ray_perp(ray, p.x, p.y, p.z, t);
        if (ray_hits(perp, t) && perp < bp) { bp = perp; best = static_cast<int>(i); }
    }
    if (best < 0) return false;
    pidx = best;
    return true;
}

// Parse "i,j,k" (the --inspect-voxel argument) into three ints. Returns false on
// a malformed value (the flag is then ignored with a warning).
bool parse_ijk(const std::string& s, int& i, int& j, int& k) {
    return std::sscanf(s.c_str(), "%d,%d,%d", &i, &j, &k) == 3;
}

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
};

AppOptions parse_app_options(const std::vector<std::string>& args) {
    AppOptions o;
    for (size_t i = 1; i < args.size(); ++i) {
        if (args[i] == "--capture-frames" && i + 1 < args.size()) {
            o.capture_frames = std::max(1, std::atoi(args[++i].c_str()));
        } else if (args[i] == "--paused") {
            o.start_paused = true;
        } else if (args[i] == "--run") {
            o.start_paused = false;
        } else if (args[i] == "--scale" && i + 1 < args.size()) {
            o.scale = std::max(0, std::atoi(args[++i].c_str()));
        } else if (args[i] == "--field" && i + 1 < args.size()) {
            o.field = args[++i];
        } else if (args[i] == "--overlays" && i + 1 < args.size()) {
            o.overlays = args[++i];
        } else if (args[i] == "--sheet-height" && i + 1 < args.size()) {
            // "<name>,<frac>" — the comma splits the overlay name from the height.
            const std::string spec = args[++i];
            const auto comma = spec.find(',');
            if (comma != std::string::npos && comma > 0) {
                const std::string nm = spec.substr(0, comma);
                const float frac = static_cast<float>(std::atof(spec.c_str() + comma + 1));
                o.sheet_heights.emplace_back(nm, frac);
            }
        } else if (args[i] == "--no-prime-tick") {
            o.prime_tick = false;
        } else if (args[i] == "--prime-tick") {
            o.prime_tick = true;
        } else if (args[i] == "--png-out" && i + 1 < args.size()) {
            o.png_out = args[++i];
        } else if (args[i] == "--inspect-voxel" && i + 1 < args.size()) {
            o.inspect_voxel = args[++i];
        } else if (args[i] == "--inspect-particle" && i + 1 < args.size()) {
            o.inspect_particle = std::atoi(args[++i].c_str());
            o.have_inspect_particle = true;
        } else if (args[i] == "--pick-scenario" && i + 1 < args.size()) {
            o.pick_scenario = args[++i];
        } else if (args[i] == "--open-physics") {
            o.open_physics = true;
        } else if (args[i] == "--open-config") {
            o.open_config = true;
        } else if (args[i] == "--open-controls") {   // both sections
            o.open_physics = true;
            o.open_config = true;
        } else if (args[i] == "--expand-all-tog") {
            o.expand_all_tog = true;
        } else if (args[i] == "--expand-tog-group" && i + 1 < args.size()) {
            o.expand_tog_groups.push_back(args[++i]);
        } else if (args[i] == "--no-scroll") {
            o.no_scroll = true;
        } else if (args[i] == "--open-telemetry") {
            o.open_telemetry = true;
        } else if (args[i] == "--toggle-on" && i + 1 < args.size()) {
            o.toggles_on.push_back(args[++i]);
        } else if (args[i] == "--toggle-off" && i + 1 < args.size()) {
            o.toggles_off.push_back(args[++i]);
        } else if (args[i] == "--set-dt" && i + 1 < args.size()) {
            o.set_dt = true;
            o.dt_value = std::atof(args[++i].c_str());
        } else if (args[i] == "--set-sor" && i + 1 < args.size()) {
            o.set_sor = true;
            o.sor_value = std::atoi(args[++i].c_str());
        } else if (args[i] == "--set-boundary" && i + 1 < args.size()) {
            o.set_boundary = true;
            o.boundary_value = std::atoi(args[++i].c_str());
        } else if (args[i] == "--set-lattice" && i + 1 < args.size()) {
            o.set_lattice = std::atoi(args[++i].c_str());
        }
    }
    return o;
}

int run_app(const std::vector<std::string>& args) {
    // ── CLI ──────────────────────────────────────────────────────────────────
    std::vector<const char*> argv;
    for (const std::string& a : args) argv.push_back(a.c_str());
    const auto parsed = ftd::native::parse_native_cli(static_cast<int>(argv.size()), argv.data());
    const AppOptions app_opts = parse_app_options(args);
    const bool capture_mode = app_opts.capture_frames >= 0;

    if (!ftd::native::enable_per_monitor_v2_dpi())
        throw std::runtime_error("Per-monitor-V2 DPI awareness unavailable");

    ftd::native::NativeEngineOptions engine_opts = parsed.options;
    std::cout << "native_app: L=" << engine_opts.lattice_size
              << " scenario=" << engine_opts.scenario
              << (engine_opts.force_cpu ? " cpu" : " gpu-default")
              << (capture_mode ? " [capture]" : "") << "\n" << std::flush;

    // ── Scale host (Scale 0/1 behind the ScaleHost/ScaleAdapter seam) ───────────
    ftd::native::HostOptions host_opts;
    host_opts.scale_level = app_opts.scale;
    host_opts.scenario = engine_opts.scenario;
    // Scale 1 has its own seed vocabulary; the Scale-0 default scenario would boot
    // the ParticleEngine into its fallback cloud anyway, but naming a Scale-1 id
    // keeps the status/scenario readout honest.
    if (app_opts.scale == 1 && host_opts.scenario.rfind("s1-", 0) != 0)
        host_opts.scenario = "s1-hydrogen-cloud";
    host_opts.run.lattice_size = engine_opts.lattice_size;
    host_opts.run.force_cpu = engine_opts.force_cpu;
    host_opts.run.flux_boundary = engine_opts.flux_boundary;
    const std::string initial_scenario = host_opts.scenario;
    const int initial_scale = host_opts.scale_level;
    ftd::native::ScaleHost host(std::move(host_opts));
    std::cout << "backend=" << host.backend_name() << " status=" << host.status()
              << "\n" << std::flush;

    ftd::native::CommandBus commands;
    std::atomic<bool> running{true};
    std::atomic<bool> paused{app_opts.start_paused};
    std::atomic<bool> quit_flag{false};
    std::atomic<int> tick_hz{capture_mode ? 240 : 60};

    ftd::native::Camera camera;
    ftd::native::NativeViewOptions view_opts;
    apply_camera_for_lattice(camera, host.lattice_size());

    // Resolve the initial active-overlay set (Scale-0 only) from --overlays
    // (comma-separated) and the legacy --field alias. Unknown names warn + skip.
    // The resolved names drive BOTH the first-boundary stamp (so frame 0 already
    // composites them) AND the panel's initial lit LEDs.
    std::vector<std::string> initial_overlays;
    if (app_opts.scale == 0) {
        auto add_overlay = [&](const std::string& name) {
            if (const auto* d = ftd::native::overlay_by_name(name)) {
                if (std::find(initial_overlays.begin(), initial_overlays.end(), name)
                    == initial_overlays.end())
                    initial_overlays.push_back(d->name);
            } else {
                std::cerr << "native_app: unknown overlay '" << name << "' (ignored)\n"
                          << std::flush;
            }
        };
        for (const std::string& n : split_csv(app_opts.overlays)) add_overlay(n);
        if (!app_opts.field.empty()) add_overlay(app_opts.field);
    }

    // Prime the loop control + publish one snapshot before the sim thread starts.
    // The initial overlays (Scale-0) are stamped into this first boundary so the
    // very first captured frame already composites them.
    {
        host.set_loop_control({app_opts.start_paused, !app_opts.start_paused, 0});
        ftd::native::CommandBus stamp;
        for (const std::string& name : initial_overlays) {
            if (const auto* d = ftd::native::overlay_by_name(name)) {
                stamp.push(ftd::native::scale0_command(ftd::native::SetOverlay{
                    static_cast<std::uint32_t>(d->id), true}));
            }
        }
        // Initial sheet slice heights (--sheet-height), stamped AFTER SetOverlay
        // so they override the y_frac seed the toggle-on installs.
        if (app_opts.scale == 0) {
            for (const auto& [nm, frac] : app_opts.sheet_heights) {
                const auto* d = ftd::native::overlay_by_name(nm);
                if (d && d->render == ftd::native::OverlayRender::Sheet) {
                    stamp.push(ftd::native::scale0_command(ftd::native::SetSheetHeight{
                        static_cast<std::uint32_t>(d->id), frac}));
                } else {
                    std::cerr << "native_app: --sheet-height '" << nm
                              << "' is not a rubber-sheet overlay (ignored)\n" << std::flush;
                }
            }
        }
        host.process_ui_boundary(stamp);
    }
    // Prime-tick-on-load (default ON): run exactly ONE tick before the sim thread
    // starts so overlays have field data to render even while the app is paused
    // (mirrors the web `primeTickOnLoad`). tick_once() advances one tick
    // regardless of the pause state set above; the sim thread then honors pause.
    if (app_opts.prime_tick) {
        const auto primed = host.tick_once();
        if (!primed.ok)
            std::cerr << "native_app: prime tick failed: " << primed.message << "\n"
                      << std::flush;
    }
    ftd::native::NativeFrame latest = host.capture();

    // ── Window ─────────────────────────────────────────────────────────────────
    WNDCLASSW wc{};
    wc.lpfnWndProc = wnd_proc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"FtdNativeApp";
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    RegisterClassW(&wc);

    const DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
    HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"FTD Native", style, CW_USEDEFAULT,
                                CW_USEDEFAULT, 1600, 900, nullptr, nullptr, wc.hInstance, nullptr);
    if (!hwnd) throw std::runtime_error("CreateWindowExW failed");

    RECT client{};
    GetClientRect(hwnd, &client);
    std::uint32_t win_w = static_cast<std::uint32_t>(std::max<LONG>(1, client.right));
    std::uint32_t win_h = static_cast<std::uint32_t>(std::max<LONG>(1, client.bottom));

    // ── Presenter (owns the swapchain on this HWND) ─────────────────────────────
    ftd::native::D3D12Presenter presenter;
    presenter.initialize(hwnd, win_w, win_h);

    // ── RmlUi renderer + context (declared AFTER presenter so it destructs
    //    FIRST — its D3D12 resources must be released while the device lives) ──
    RmlD3D12System rml_system;
    RmlD3D12Renderer rml_renderer;
    ftd::native::PresenterUiContext ui_ctx = presenter.ui_backend_context();
    if (!ui_ctx.device || !ui_ctx.queue)
        throw std::runtime_error("presenter did not expose a UI device/queue");
    rml_renderer.initialize(ui_ctx.device, ui_ctx.queue, widen(FTD_RMLUI_HLSL_PATH).c_str());

    Rml::SetSystemInterface(&rml_system);
    Rml::SetRenderInterface(&rml_renderer);
    if (!Rml::Initialise()) throw std::runtime_error("Rml::Initialise failed");

    // The shell RCSS styles some elements "Inter" and some "JetBrains Mono".
    // Load the vendored Inter face under both families (a JetBrains Mono face is
    // not vendored yet — same shim the smoke test uses).
    if (!Rml::LoadFontFace(FTD_RML_FONT_PATH))
        throw std::runtime_error("LoadFontFace(Inter-Regular.ttf) failed");
    {
        static std::vector<Rml::byte> mono;
        std::FILE* f = nullptr;
        if (_wfopen_s(&f, widen(FTD_RML_FONT_PATH).c_str(), L"rb") == 0 && f) {
            std::fseek(f, 0, SEEK_END);
            long n = std::ftell(f);
            std::fseek(f, 0, SEEK_SET);
            if (n > 0) {
                mono.resize(static_cast<size_t>(n));
                mono.resize(std::fread(mono.data(), 1, mono.size(), f));
            }
            std::fclose(f);
        }
        if (!mono.empty())
            Rml::LoadFontFace(Rml::Span<const Rml::byte>(mono.data(), mono.size()),
                              "JetBrains Mono", Rml::Style::FontStyle::Normal,
                              Rml::Style::FontWeight::Auto, false);
    }

    const UINT dpi0 = GetDpiForWindow(hwnd);
    float dpi_scale = dpi0 > 0 ? static_cast<float>(dpi0) / 96.0f : 1.0f;
    Rml::Context* context =
        Rml::CreateContext("main", Rml::Vector2i(static_cast<int>(win_w), static_cast<int>(win_h)));
    if (!context) throw std::runtime_error("Rml::CreateContext failed");
    context->SetDensityIndependentPixelRatio(dpi_scale);

    // ── Data model (must exist before LoadDocument so data-model binds) ─────────
    ShellData data;
    data.scenario = initial_scenario;
    data.active_scale = initial_scale;
    // Physics control surface: COLLAPSED on boot (data.phys_open / data.cfg_open
    // both false), so the bound toggle_groups / config_rows arrays start EMPTY —
    // the 44 toggle rows + config knobs are instantiated lazily the first time the
    // user opens a section (see the toggle_physics / toggle_config callbacks),
    // keeping the boot/normal-use DOM tiny. The --open-physics / --open-config CLI
    // flags pre-open them for headless captures (handled after `app` is built).
    // FIELDS overlay panel (7 columns, multi-select) + initial lit LEDs (mirrors
    // the overlay stamp above so the panel matches the geometry from frame 0).
    data.overlay_columns = build_overlay_columns();
    for (const std::string& name : initial_overlays)
        if (OverlayRow* r = find_overlay_row(&data, Rml::String(name.c_str())))
            r->on = true;
    // Setup scenario picker: starts COLLAPSED (data.scn_open == false), so the
    // bound scenario_groups array is left EMPTY on boot — the ~130 scenario <div>s
    // are instantiated lazily the first time the user opens the picker (see the
    // toggle_scn_picker callback). This keeps the boot/normal-use DOM tiny.
    // Reflect any --sheet-height overrides in the panel rows (mirrors the
    // SetSheetHeight commands stamped above) so the shown value starts correct.
    for (const auto& [nm, frac] : app_opts.sheet_heights) {
        if (OverlayRow* r = find_overlay_row(&data, Rml::String(nm.c_str()))) {
            const float h = std::clamp(frac, 0.0f, 0.999f);
            r->height = h;
            r->hstr = sheet_hstr(h);
        }
    }

    // ── Telemetry ring buffers + the <ftd-chart> registry/instancer ────────────
    // The app owns every series (GUI thread), pushing one scalar per published
    // snapshot below; the custom elements read them read-only. Declared here so
    // all outlive Rml::Shutdown() (which releases the instanced elements back
    // through the instancer). The registry maps each chart's `id` → its coloured
    // series set; the trace colours mirror the RCSS legend-chip classes (.s0..s4)
    // so the legend and the traces agree. Registered after Rml::Initialise(),
    // before LoadDocument parses <ftd-chart>.
    //
    // Sources (all from the published Scale0Snapshot):
    //   chart-diag  ← telemetry.diagnostics (scheduler DIAGNOSTICS group) + the
    //                 always-live energy_ledger; the base "what is the field doing"
    //                 view (energy · manifested · entropy · net charge).
    //   chart-audit ← telemetry.audit (scheduler AUDIT group) + energy_ledger.dE_dt;
    //                 the "is energy conserved" view (accounted E · drift · Gauss).
    //   chart-lagr  ← telemetry.lagrangian (scheduler LAGRANGIAN group); ℒ and ℋ.
    using ftd::native::ui::ChartSeries;
    ChartSeries diag_energy(240), diag_manif(240), diag_entropy(240), diag_charge(240);
    ChartSeries aud_energy(240), aud_drift(240), aud_gauss(240);
    ChartSeries lag_lag(240), lag_ham(240);
    ftd::native::ui::ChartRegistry chart_registry;
    {
        using C = Rml::Colourb;
        const C kBlue(106, 168, 224);    // .s0
        const C kGreen(78, 203, 138);    // .s1
        const C kAmber(224, 166, 58);    // .s2
        const C kRed(224, 106, 106);     // .s3
        const C kViolet(169, 138, 224);  // .s4
        chart_registry.binding("chart-diag").series = {
            {&diag_energy, kBlue}, {&diag_manif, kGreen},
            {&diag_entropy, kAmber}, {&diag_charge, kRed}};
        chart_registry.binding("chart-audit").series = {
            {&aud_energy, kBlue}, {&aud_drift, kAmber}, {&aud_gauss, kRed}};
        chart_registry.binding("chart-lagr").series = {
            {&lag_lag, kBlue}, {&lag_ham, kViolet}};
    }
    ftd::native::ui::FtdChartInstancer chart_instancer(&chart_registry);
    Rml::Factory::RegisterElementInstancer("ftd-chart", &chart_instancer);

    AppContext app;
    app.hwnd = hwnd;
    app.context = nullptr;  // published after LoadDocument
    app.presenter = &presenter;
    app.camera = &camera;
    app.commands = &commands;
    app.data = &data;
    app.paused = &paused;
    app.quit = &quit_flag;
    app.scenario_id = initial_scenario;
    // Seed the lattice-reboot run config from the launch options, so a lattice
    // nudge re-applies the same backend/boundary the app booted with (the reboot
    // reloads the CURRENT scenario at the new L).
    app.run_config.lattice_size = host.lattice_size();
    app.run_config.force_cpu = engine_opts.force_cpu;
    app.run_config.flux_boundary = engine_opts.flux_boundary;
    app.backend_cpu = engine_opts.force_cpu;

    // Seed the scroll-wheel height target to the last active sheet (if any), so
    // Shift+wheel over the scene is tactile from frame 0 in headless captures.
    for (const std::string& name : initial_overlays) {
        const auto* d = ftd::native::overlay_by_name(name);
        if (d && d->render == ftd::native::OverlayRender::Sheet) {
            app.last_sheet_id = static_cast<std::uint32_t>(d->id);
            app.has_last_sheet = true;
        }
    }

    // Simulated initial pick (headless captures — interactive picking cannot run
    // under --capture-frames). Honored only when it matches the initial scale
    // (voxel ↔ Scale 0, particle ↔ Scale 1). This seeds the SAME selection state
    // a real click sets, so the GUI loop's live re-inspection populates the
    // inspector before the capture fires.
    if (initial_scale == 0 && !app_opts.inspect_voxel.empty()) {
        int ix = 0, iy = 0, iz = 0;
        if (parse_ijk(app_opts.inspect_voxel, ix, iy, iz)) {
            app.inspect_kind = 1;
            app.inspect_vx = ix;
            app.inspect_vy = iy;
            app.inspect_vz = iz;
        } else {
            std::cerr << "native_app: bad --inspect-voxel '" << app_opts.inspect_voxel
                      << "' (want i,j,k; ignored)\n" << std::flush;
        }
    } else if (initial_scale == 1 && app_opts.have_inspect_particle) {
        app.inspect_kind = 2;
        app.inspect_pidx = app_opts.inspect_particle;
    }

    // Simulated Setup-picker selection (headless captures — interactive clicking
    // cannot run under --capture-frames). --pick-scenario <id> drives the SAME
    // select_scenario → LoadScenario path a real click uses: it records the id as
    // the Reset target, moves the highlight, and enqueues the reload the sim
    // thread applies at its next boundary, so the captured frame shows a scenario
    // loaded via the picker. Scale-0 only (the catalog is Scale-0 scenarios).
    if (initial_scale == 0 && !app_opts.pick_scenario.empty()) {
        const std::string& pick = app_opts.pick_scenario;
        if (ftd::native::find_scenario_meta(pick)) {
            // Open the picker for the headless capture and demonstrate the intended
            // fast-browse path: type-to-filter narrows the list to the matches (a
            // small DOM), then select loads. Seeding the filter with the target id
            // force-opens its group with only the matching row(s), so the captured
            // frame shows a searched list with the target highlighted — the exact
            // select_scenario → LoadScenario path a real picker click drives.
            data.scn_open = true;
            app.scenario_filter = pick;
            rebuild_scenario_view(&data, pick, app.scenario_filter,
                                  app.scn_expanded_groups);
            app.scenario_id = pick;
            set_current_scenario(&data, pick);
            data.scenario = pick;
            push_core(&app, ftd::native::LoadScenario{pick});
            std::cout << "native_app: picker select_scenario -> " << pick << "\n"
                      << std::flush;
        } else {
            std::cerr << "native_app: --pick-scenario '" << pick
                      << "' not in catalog; ignored\n" << std::flush;
        }
    }

    Rml::DataModelConstructor ctor = context->CreateDataModel("shell");
    if (!ctor) throw std::runtime_error("CreateDataModel(shell) failed");
    // Physics control surface: the 44-toggle rows (categorized) + config knobs.
    if (auto trow = ctor.RegisterStruct<FullToggleRow>()) {
        trow.RegisterMember("name", &FullToggleRow::name);
        trow.RegisterMember("desc", &FullToggleRow::desc);
        trow.RegisterMember("req", &FullToggleRow::req);
        trow.RegisterMember("on", &FullToggleRow::on);
        trow.RegisterMember("gated", &FullToggleRow::gated);
        trow.RegisterMember("has_req", &FullToggleRow::has_req);
    }
    ctor.RegisterArray<Rml::Vector<FullToggleRow>>();
    if (auto tgrp = ctor.RegisterStruct<ToggleGroupRow>()) {
        tgrp.RegisterMember("title", &ToggleGroupRow::title);
        tgrp.RegisterMember("expanded", &ToggleGroupRow::expanded);
        tgrp.RegisterMember("count", &ToggleGroupRow::count);
        tgrp.RegisterMember("items", &ToggleGroupRow::items);
    }
    ctor.RegisterArray<Rml::Vector<ToggleGroupRow>>();
    if (auto crow = ctor.RegisterStruct<ConfigRow>()) {
        crow.RegisterMember("key", &ConfigRow::key);
        crow.RegisterMember("label", &ConfigRow::label);
        crow.RegisterMember("vstr", &ConfigRow::vstr);
        crow.RegisterMember("hint", &ConfigRow::hint);
    }
    ctor.RegisterArray<Rml::Vector<ConfigRow>>();
    if (auto orow = ctor.RegisterStruct<OverlayRow>()) {
        orow.RegisterMember("name", &OverlayRow::name);
        orow.RegisterMember("label", &OverlayRow::label);
        orow.RegisterMember("on", &OverlayRow::on);
        orow.RegisterMember("is_sheet", &OverlayRow::is_sheet);
        orow.RegisterMember("hstr", &OverlayRow::hstr);
    }
    ctor.RegisterArray<Rml::Vector<OverlayRow>>();
    if (auto ocol = ctor.RegisterStruct<OverlayColumnRow>()) {
        ocol.RegisterMember("title", &OverlayColumnRow::title);
        ocol.RegisterMember("expanded", &OverlayColumnRow::expanded);
        ocol.RegisterMember("items", &OverlayColumnRow::items);
    }
    ctor.RegisterArray<Rml::Vector<OverlayColumnRow>>();
    if (auto irow = ctor.RegisterStruct<InspLine>()) {
        irow.RegisterMember("k", &InspLine::k);
        irow.RegisterMember("v", &InspLine::v);
    }
    ctor.RegisterArray<Rml::Vector<InspLine>>();
    if (auto srow = ctor.RegisterStruct<ScenarioRow>()) {
        srow.RegisterMember("id", &ScenarioRow::id);
        srow.RegisterMember("title", &ScenarioRow::title);
        srow.RegisterMember("current", &ScenarioRow::current);
        srow.RegisterMember("visible", &ScenarioRow::visible);
    }
    ctor.RegisterArray<Rml::Vector<ScenarioRow>>();
    if (auto sgrp = ctor.RegisterStruct<ScenarioGroupRow>()) {
        sgrp.RegisterMember("title", &ScenarioGroupRow::title);
        sgrp.RegisterMember("expanded", &ScenarioGroupRow::expanded);
        sgrp.RegisterMember("has_visible", &ScenarioGroupRow::has_visible);
        sgrp.RegisterMember("show_items", &ScenarioGroupRow::show_items);
        sgrp.RegisterMember("count", &ScenarioGroupRow::count);
        sgrp.RegisterMember("items", &ScenarioGroupRow::items);
    }
    ctor.RegisterArray<Rml::Vector<ScenarioGroupRow>>();
    ctor.Bind("tick", &data.tick);
    ctor.Bind("active_scale", &data.active_scale);
    ctor.Bind("particle_count", &data.particle_count);
    ctor.Bind("physical_time", &data.physical_time);
    ctor.Bind("total_energy", &data.total_energy);
    ctor.Bind("s1_ke", &data.s1_ke);
    ctor.Bind("s1_pe", &data.s1_pe);
    ctor.Bind("scenario", &data.scenario);
    ctor.Bind("backend", &data.backend);
    ctor.Bind("lattice", &data.lattice);
    ctor.Bind("fps", &data.fps);
    ctor.Bind("running", &data.running);
    ctor.Bind("phys_open", &data.phys_open);
    ctor.Bind("cfg_open", &data.cfg_open);
    ctor.Bind("toggle_groups", &data.toggle_groups);
    ctor.Bind("config_rows", &data.config_rows);
    ctor.Bind("has_validation", &data.has_validation);
    ctor.Bind("validation_msg", &data.validation_msg);
    ctor.Bind("overlay_columns", &data.overlay_columns);
    ctor.Bind("scn_open", &data.scn_open);
    ctor.Bind("scenario_groups", &data.scenario_groups);
    ctor.Bind("insp_active", &data.insp_active);
    ctor.Bind("insp_title", &data.insp_title);
    ctor.Bind("insp_lines", &data.insp_lines);
    // Telemetry section (collapsible; charts + legend live behind tel_open).
    ctor.Bind("tel_open", &data.tel_open);
    ctor.Bind("tel_d_energy", &data.tel_d_energy);
    ctor.Bind("tel_d_manif", &data.tel_d_manif);
    ctor.Bind("tel_d_entropy", &data.tel_d_entropy);
    ctor.Bind("tel_d_charge", &data.tel_d_charge);
    ctor.Bind("tel_diag_prov", &data.tel_diag_prov);
    ctor.Bind("tel_a_energy", &data.tel_a_energy);
    ctor.Bind("tel_a_drift", &data.tel_a_drift);
    ctor.Bind("tel_a_gauss", &data.tel_a_gauss);
    ctor.Bind("tel_audit_prov", &data.tel_audit_prov);
    ctor.Bind("tel_l_lag", &data.tel_l_lag);
    ctor.Bind("tel_l_ham", &data.tel_l_ham);
    ctor.BindEventCallback("run", [&app](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) {
        request_play_toggle(&app);
    });
    ctor.BindEventCallback("pause", [&app](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) {
        request_pause(&app);
    });
    ctor.BindEventCallback("step", [&app](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) {
        request_step(&app);
    });
    ctor.BindEventCallback("reset", [&app](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) {
        request_reset(&app);
    });
    ctor.BindEventCallback("toggle", [&app](Rml::DataModelHandle, Rml::Event&,
                                            const Rml::VariantList& v) {
        if (!v.empty()) request_toggle(&app, v[0].Get<Rml::String>());
    });
    // Physics-terms section — expand/collapse the whole toggle-category list. On
    // open, build the category headers (items are built per-category on demand);
    // on close, drop every row so the DOM holds ~0 toggle <div>s (the picker's
    // DOM-shrink discipline). This is the fps guard for the 44-toggle panel.
    ctor.BindEventCallback("toggle_physics", [&app](Rml::DataModelHandle h, Rml::Event&,
                                                    const Rml::VariantList&) {
        if (!app.data) return;
        app.data->phys_open = !app.data->phys_open;
        if (app.data->phys_open)
            rebuild_toggle_groups(app.data, app.live_toggles, app.tog_expanded_groups);
        else
            app.data->toggle_groups.clear();
        h.DirtyVariable("phys_open");
        h.DirtyVariable("toggle_groups");
    });
    // Expand/collapse one toggle CATEGORY (its header is the affordance). Only an
    // expanded category builds its item rows, so the live DOM stays small.
    ctor.BindEventCallback("toggle_tog_group", [&app](Rml::DataModelHandle h, Rml::Event&,
                                                      const Rml::VariantList& v) {
        if (v.empty() || !app.data) return;
        const std::string title(v[0].Get<Rml::String>().c_str());
        auto& ex = app.tog_expanded_groups;
        auto it = std::find(ex.begin(), ex.end(), title);
        if (it == ex.end()) ex.push_back(title);
        else ex.erase(it);
        rebuild_toggle_groups(app.data, app.live_toggles, ex);
        h.DirtyVariable("toggle_groups");
    });
    // Config section — expand/collapse the config knobs + lattice + reset. On
    // open, build the knob rows from the live engine truth; on close, drop them.
    ctor.BindEventCallback("toggle_config", [&app](Rml::DataModelHandle h, Rml::Event&,
                                                   const Rml::VariantList&) {
        if (!app.data) return;
        app.data->cfg_open = !app.data->cfg_open;
        if (app.data->cfg_open)
            build_config_rows(app.data, app.live_toggles, app.live_knobs);
        else
            app.data->config_rows.clear();
        h.DirtyVariable("cfg_open");
        h.DirtyVariable("config_rows");
    });
    // Config knob nudge: v[0] = knob key, v[1] = "-"/"+". Steps/cycles the value
    // (reading current from the live mirror) and pushes the matching command.
    ctor.BindEventCallback("config_nudge", [&app](Rml::DataModelHandle, Rml::Event&,
                                                  const Rml::VariantList& v) {
        if (v.size() < 2) return;
        request_config_nudge(&app, std::string(v[0].Get<Rml::String>().c_str()),
                             std::string(v[1].Get<Rml::String>().c_str()));
    });
    // Reset every term toggle to its canonical default (ResetToDefaults).
    ctor.BindEventCallback("reset_toggles", [&app](Rml::DataModelHandle, Rml::Event&,
                                                   const Rml::VariantList&) {
        request_reset_toggles(&app);
    });
    // Telemetry section — expand/collapse the charts. Opening it flips the demand
    // mask to add the heavier AUDIT + LAGRANGIAN scheduler groups (diagnostics is
    // demanded regardless); closing it drops back to diagnostics-only so idle fps
    // stays high. The charts/legend are gated by data-if="tel_open", so a closed
    // section holds ~0 chart DOM (the physics/scenario-picker DOM-shrink pattern).
    ctor.BindEventCallback("toggle_telemetry", [&app](Rml::DataModelHandle h, Rml::Event&,
                                                      const Rml::VariantList&) {
        if (!app.data) return;
        app.data->tel_open = !app.data->tel_open;
        request_telemetry_demand(&app, app.data->tel_open);
        h.DirtyVariable("tel_open");
    });
    ctor.BindEventCallback("scale_lattice", [&app](Rml::DataModelHandle, Rml::Event&,
                                                   const Rml::VariantList&) {
        request_switch_scale(&app, 0);
    });
    ctor.BindEventCallback("scale_particles", [&app](Rml::DataModelHandle, Rml::Event&,
                                                     const Rml::VariantList&) {
        request_switch_scale(&app, 1);
    });
    // Overlay toggle: flip one overlay's membership in the active set. Pushes a
    // SetOverlay Scale-0 command (multi-select — the adapter composites all
    // active overlays) and lights/clears the row's LED. The bound `on` state is
    // owned here (no snapshot round-trip carries it back), so dirty the array.
    ctor.BindEventCallback("set_overlay", [&app](Rml::DataModelHandle h, Rml::Event&,
                                                 const Rml::VariantList& v) {
        if (v.empty() || !app.data) return;
        const Rml::String name = v[0].Get<Rml::String>();
        const ftd::native::OverlayDescriptor* d =
            ftd::native::overlay_by_name(name.c_str());
        OverlayRow* row = find_overlay_row(app.data, name);
        if (!d || !row) return;
        row->on = !row->on;
        push_scale0(&app, ftd::native::SetOverlay{static_cast<std::uint32_t>(d->id),
                                                  row->on});
        // Toggling a sheet ON makes it the scroll-wheel target and resets its
        // shown height to the registry default (matching the adapter's seed).
        if (row->is_sheet && row->on) {
            row->height = d->y_frac;
            row->hstr = sheet_hstr(d->y_frac);
            app.last_sheet_id = static_cast<std::uint32_t>(d->id);
            app.has_last_sheet = true;
        }
        h.DirtyVariable("overlay_columns");
    });
    // Rubber-sheet height nudge: the panel's −/＋ affordance for one active sheet.
    // v[0] = overlay name, v[1] = "-" or "+". Steps the slice height by ±0.05 via
    // the shared nudge helper (clamps, updates the row, pushes SetSheetHeight).
    ctor.BindEventCallback("sheet_height_nudge", [&app](Rml::DataModelHandle, Rml::Event&,
                                                        const Rml::VariantList& v) {
        if (v.size() < 2 || !app.data) return;
        const Rml::String name = v[0].Get<Rml::String>();
        const Rml::String dir = v[1].Get<Rml::String>();
        OverlayRow* row = find_overlay_row(app.data, name);
        if (!row) return;
        nudge_sheet_height(&app, row, dir == "+" ? 0.05f : -0.05f);
    });
    // Collapse/expand one overlay column (its header is the affordance). Pure
    // view-state; flips `expanded` and re-lays the column list via data-if.
    ctor.BindEventCallback("toggle_overlay_col", [&app](Rml::DataModelHandle h, Rml::Event&,
                                                        const Rml::VariantList& v) {
        if (v.empty() || !app.data) return;
        const Rml::String title = v[0].Get<Rml::String>();
        for (OverlayColumnRow& col : app.data->overlay_columns) {
            if (col.title == title) { col.expanded = !col.expanded; break; }
        }
        h.DirtyVariable("overlay_columns");
    });
    // Setup scenario picker — expand/collapse the whole list ("Scenarios ▾"
    // header). Collapsing DROPS every row (scenario_groups.clear()) so the DOM
    // holds ~0 scenario <div>s; expanding rebuilds the list from the catalog and
    // re-applies the active search filter + current-row highlight. This is the
    // DOM-shrink half of the framerate fix: the ~130-item list only exists while
    // the picker is open.
    ctor.BindEventCallback("toggle_scn_picker", [&app](Rml::DataModelHandle h, Rml::Event&,
                                                       const Rml::VariantList&) {
        if (!app.data) return;
        app.data->scn_open = !app.data->scn_open;
        if (app.data->scn_open) {
            rebuild_scenario_view(app.data, app.scenario_id, app.scenario_filter,
                                  app.scn_expanded_groups);
        } else {
            app.data->scenario_groups.clear();  // collapse → 0 scenario rows in the DOM
        }
        h.DirtyVariable("scn_open");
        h.DirtyVariable("scenario_groups");
    });
    // Setup scenario picker — select one scenario. Issues the LoadScenario core
    // command (the exact path Reset uses) so the host reboots the engine into it
    // (a LIVE switch; overlays/scene refresh on the reload). Optimistically moves
    // the current-row highlight and records the id as the new Reset target; the
    // per-frame sync confirms it against the effective (possibly W9-corrected)
    // scenario the host actually loaded.
    ctor.BindEventCallback("select_scenario", [&app](Rml::DataModelHandle h, Rml::Event&,
                                                     const Rml::VariantList& v) {
        if (v.empty() || !app.data) return;
        const std::string id = std::string(v[0].Get<Rml::String>().c_str());
        set_current_scenario(app.data, id);
        app.scenario_id = id;
        push_core(&app, ftd::native::LoadScenario{id});
        h.DirtyVariable("scenario_groups");
    });
    // Collapse/expand one scenario category group (its header is the affordance).
    // Flips the group's membership in the expanded set and rebuilds the view, so
    // an expanded group instantiates its rows and a collapsed group drops them.
    ctor.BindEventCallback("toggle_scenario_group", [&app](Rml::DataModelHandle h, Rml::Event&,
                                                           const Rml::VariantList& v) {
        if (v.empty() || !app.data) return;
        const std::string title(v[0].Get<Rml::String>().c_str());
        auto& ex = app.scn_expanded_groups;
        auto it = std::find(ex.begin(), ex.end(), title);
        if (it == ex.end()) ex.push_back(title);
        else ex.erase(it);
        rebuild_scenario_view(app.data, app.scenario_id, app.scenario_filter, ex);
        h.DirtyVariable("scenario_groups");
    });
    // Live search over the picker: the change event carries the input's new text.
    // Rebuilds the view keeping only matching rows (matching groups force-open),
    // so typing SHRINKS the live DOM to the matches rather than hiding 130 rows.
    ctor.BindEventCallback("filter_scenarios", [&app](Rml::DataModelHandle h, Rml::Event& ev,
                                                      const Rml::VariantList&) {
        if (!app.data) return;
        app.scenario_filter = std::string(ev.GetParameter<Rml::String>("value", "").c_str());
        rebuild_scenario_view(app.data, app.scenario_id, app.scenario_filter,
                              app.scn_expanded_groups);
        h.DirtyVariable("scenario_groups");
    });
    // Inspector close affordance (the × in the readout header): drop the current
    // selection so the GUI loop stops re-issuing the inspect command and hides
    // the section next frame. Clear the bound fields here too for immediacy.
    ctor.BindEventCallback("clear_inspect", [&app](Rml::DataModelHandle h, Rml::Event&,
                                                   const Rml::VariantList&) {
        app.inspect_kind = 0;
        app.inspect_pidx = -1;
        app.insp_has_data = false;
        if (app.data && app.data->insp_active) {
            app.data->insp_active = false;
            app.data->insp_lines.clear();
            h.DirtyVariable("insp_active");
            h.DirtyVariable("insp_lines");
        }
    });
    Rml::DataModelHandle model = ctor.GetModelHandle();
    // Publish the handle to the app so wnd_proc's scroll-wheel height nudge can
    // dirty the overlay panel (the RML event callbacks get their own handle).
    app.model = model;
    app.model_ready = true;

    // ── CLI-driven physics-control state (headless proof of the panel) ──────────
    // Seed the live caches from the host's boot snapshot so the pre-opened panel
    // shows real engine values from frame 0 (the per-frame sync keeps them live).
    if (app_opts.scale == 0) {
        if (auto boot = host.publisher().acquire()) {
            if (const ftd::native::Scale0Snapshot* s0 = boot->scale0()) {
                app.live_toggles = s0->term_toggles;
                app.live_knobs = s0->knobs;
                app.have_live = true;
                app.backend_cpu = (s0->env.backend == ftd::native::BackendKindUi::Cpu);
            }
        }
        // --open-physics: open the section (categories collapsed by default — 4
        // headers whose counts sum to 44). --expand-all-tog / --expand-tog-group
        // expand the requested categories so their toggle rows show.
        if (app_opts.open_physics) {
            data.phys_open = true;
            app.tog_expanded_groups.clear();
            if (app_opts.expand_all_tog) {
                for (const char* c : kToggleCategories)
                    app.tog_expanded_groups.emplace_back(c);
            } else {
                for (const std::string& g : app_opts.expand_tog_groups)
                    app.tog_expanded_groups.push_back(g);
            }
            rebuild_toggle_groups(&data, app.live_toggles, app.tog_expanded_groups);
        }
        if (app_opts.open_config) {
            data.cfg_open = true;
            build_config_rows(&data, app.live_toggles, app.live_knobs);
        }
        // Simulated control edits — the SAME commands the toggle clicks / config
        // nudges push, so the captured snapshot reflects them (control works).
        for (const std::string& n : app_opts.toggles_on) {
            if (ftd::term_toggles_detail::find_spec(n))
                push_scale0(&app, ftd::native::SetToggle{n, true});
            else std::cerr << "native_app: unknown toggle '" << n << "' (ignored)\n" << std::flush;
        }
        for (const std::string& n : app_opts.toggles_off) {
            if (ftd::term_toggles_detail::find_spec(n))
                push_scale0(&app, ftd::native::SetToggle{n, false});
            else std::cerr << "native_app: unknown toggle '" << n << "' (ignored)\n" << std::flush;
        }
        if (app_opts.set_dt) push_scale0(&app, ftd::native::SetDt{app_opts.dt_value});
        if (app_opts.set_sor) push_scale0(&app, ftd::native::SetSorIterations{app_opts.sor_value});
        if (app_opts.set_boundary) {
            const int b = std::clamp(app_opts.boundary_value, 0, 2);
            app.run_config.flux_boundary = b;
            push_scale0(&app, ftd::native::SetBoundary{static_cast<ftd::FluxBoundaryMode>(b)});
        }
        if (app_opts.set_lattice > 0) request_lattice_reboot(&app, app_opts.set_lattice);
    }

    // Telemetry: pre-open the section for a headless capture if requested, then
    // publish the initial demand. DIAGNOSTICS is demanded regardless of the section
    // state (cheap cadence-1 base charts); AUDIT + LAGRANGIAN follow when the
    // section is open. Issued unconditionally (any start scale) so a later switch
    // to Scale 0 already has a live demand on the host.
    if (app_opts.open_telemetry) data.tel_open = true;
    request_telemetry_demand(&app, data.tel_open);

    Rml::ElementDocument* doc = context->LoadDocument(FTD_RML_SHELL_PATH);
    if (!doc) throw std::runtime_error("LoadDocument(shell.rml) failed");
    doc->Show();

    // Publish the RmlUi context + overlay into the presenter's frame path.
    app.context = context;
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&app));
    RmlOverlay overlay(&rml_renderer, &context);
    presenter.set_overlay_recorder(&overlay);

    // ── CUDA↔D3D12 zero-copy interop (Scale-0 GPU particle path) ────────────────
    // The presenter owns a D3D12_HEAP_FLAG_SHARED particle buffer + a
    // D3D12_FENCE_FLAG_SHARED cross-API fence; CUDA imports both and the interop
    // gather writes device-resident particles straight into that buffer. Lifecycle
    // mirrors the native_desktop reference: create + import once and keep the NT
    // handles open for the whole process, so every post-reload re-import targets
    // the SAME D3D12 resources (the presenter and its shared resources are never
    // recreated by a reload — only the CUDA GpuEngine is). Interop is Scale-0-only:
    // Scale 1's ParticleEngine has no device buffer and its adapter's interop
    // methods are no-ops, so try_enable_interop() there returns false and the CPU
    // sprite path is used.
    //
    // Handles are created whenever the backend is GPU (regardless of the INITIAL
    // scale) so that a later switch INTO Scale 0 can re-import them; the CUDA
    // import (try_enable_interop) only actually succeeds while Scale 0 is active.
    std::atomic<bool> interop_active{false};
    HANDLE interop_buf_handle = nullptr;
    HANDLE interop_fence_handle = nullptr;
    std::uint64_t interop_buffer_bytes = 0;
    if (!engine_opts.force_cpu) {
        interop_buf_handle =
            presenter.create_shared_particle_buffer(ftd::kMaxVisualParticleCapture);
        interop_fence_handle =
            interop_buf_handle ? presenter.create_shared_fence() : nullptr;
        if (interop_buf_handle && interop_fence_handle) {
            interop_buffer_bytes = presenter.shared_particle_buffer_bytes();
            interop_active.store(host.try_enable_interop(
                interop_buf_handle, interop_buffer_bytes, interop_fence_handle));
        }
        std::cout << "interop: "
                  << (interop_active.load()
                          ? "enabled (Scale-0 device-resident particles)"
                          : "unavailable, using CPU particle path")
                  << "\n" << std::flush;
    }

    int camera_lattice = host.lattice_size();

    // ── Sim thread ──────────────────────────────────────────────────────────────
    std::mutex frame_mu;
    // Interop poll result produced EXCLUSIVELY on the sim thread (the only thread
    // allowed to touch host/bridge/GpuEngine — a reload can tear the GpuEngine down
    // mid-flight with no locking) and consumed on the GUI thread under frame_mu,
    // paired with `latest`. -1 = "no gather ready this round" / interop inactive.
    int latest_interop_count = -1;
    std::uint64_t latest_interop_fence = 0;
    std::thread sim([&] {
        // Sim-thread-local fence bookkeeping. interop_fence_counter is the strictly
        // increasing value each gather is signaled under; it is NEVER reset across
        // reloads, because the D3D12-side shared fence object survives every reload
        // (only the CUDA GpuEngine is rebuilt) and keeps its completed value — a
        // reset would make the first post-reload gather signal an already-passed
        // value, which cudaSignalExternalSemaphoresAsync rejects and a D3D12
        // queue->Wait would treat as already-satisfied (defeating the sync).
        // pending_interop_fence remembers which value the most recently REQUESTED
        // gather used, so a polled count is always paired with the exact fence value
        // that produced it.
        std::uint64_t interop_fence_counter = 0;
        std::uint64_t pending_interop_fence = 0;
        while (running.load()) {
            const auto start = std::chrono::steady_clock::now();
            try {
                const auto loop = host.loop_control();
                const bool need_work = loop.pending_steps > 0 || !loop.pause;
                if (need_work) {
                    const auto r = host.tick_once();
                    if (loop.pending_steps > 0) host.consume_pending_step();
                    if (!r.ok) throw std::runtime_error(r.message);
                }
                host.process_ui_boundary(commands);
                paused.store(host.loop_control().pause);

                // A reload / scale-switch rebuilt the active engine. Re-establish
                // interop iff Scale 0 is active and the shared handles exist. Two
                // paths converge here: a same-scale Scale-0 reload clears the
                // adapter's interop (fresh GpuEngine) and ScaleHost reports
                // ReloadStatus::InteropReimportRequired; a switch BACK to Scale 0
                // reports Success on a fresh adapter (was_interop == false). Both
                // need the identical re-import, and a switch to Scale 1 (no device
                // buffer) simply leaves interop off. import_d3d12_* are documented
                // safe to call more than once, so re-importing the same handles into
                // the fresh GpuEngine is within contract.
                if (host.applied_reload()) {
                    bool now_active = false;
                    if (interop_buf_handle && interop_fence_handle
                        && host.active_scale() == 0) {
                        now_active = host.try_enable_interop(
                            interop_buf_handle, interop_buffer_bytes,
                            interop_fence_handle);
                    }
                    interop_active.store(now_active);
                }

                if (need_work || host.applied_reload() || host.applied_host_write()) {
                    // Poll the PREVIOUS gather's count and request the NEXT gather —
                    // only ever on this thread. A thrown GPU error also lands in the
                    // catch below rather than unwinding past the still-joinable sim
                    // thread.
                    int polled_interop_count = -1;
                    std::uint64_t polled_interop_fence = 0;
                    if (interop_active.load()) {
                        polled_interop_count = host.poll_interop_particle_count();
                        polled_interop_fence = pending_interop_fence;
                        const std::uint64_t fv = ++interop_fence_counter;
                        if (host.request_interop_gather(fv)) {
                            pending_interop_fence = fv;
                        } else {
                            // The cross-API fence signal failed: the GUI thread's
                            // wait_shared_fence() would otherwise block forever on a
                            // value never signaled. Drop to the CPU particle path
                            // for the rest of the session (host.capture() below
                            // still supplies frame.particles).
                            interop_active.store(false);
                            std::cerr << "interop: gather/fence-signal failed "
                                         "mid-session, falling back to the CPU "
                                         "particle path\n" << std::flush;
                        }
                    }
                    ftd::native::NativeFrame next = host.capture();
                    std::lock_guard<std::mutex> lock(frame_mu);
                    latest = std::move(next);
                    latest_interop_count = polled_interop_count;
                    latest_interop_fence = polled_interop_fence;
                }
            } catch (const std::exception& ex) {
                std::lock_guard<std::mutex> lock(frame_mu);
                latest.status = ex.what();
            }
            const int hz = std::max(1, tick_hz.load());
            const auto budget = std::chrono::milliseconds(1000 / hz);
            const auto elapsed = std::chrono::steady_clock::now() - start;
            if (elapsed < budget) std::this_thread::sleep_for(budget - elapsed);
        }
    });

    // ── GUI loop ──────────────────────────────────────────────────────────────
    LARGE_INTEGER qpc_freq{}, qpc_last{};
    QueryPerformanceFrequency(&qpc_freq);
    QueryPerformanceCounter(&qpc_last);
    double fps_accum = 0.0;
    int fps_frames = 0;
    int smoothed_fps = 0;

    ftd::native::CaptureToken capture_token{};
    bool capture_requested = false;
    bool capture_saved = false;
    bool capture_ok = false;
    // True once a device-resident interop gather has EVER landed this session.
    // Gates the 0-particle-field arm-gate fallback below: if interop is enabled
    // but no gather ever lands (a pure-field scenario — where the rubber sheets
    // shine), arm the capture a short grace past the warmup instead of hanging to
    // the 40 s deadline. Once a gather DOES land, this stays true and the arm
    // reverts to the exact prior behavior (wait for draw_interop_count > 0).
    bool interop_gathered_once = false;
    int frame_no = 0;
    // Capture output path: --png-out overrides the compiled default so a single
    // build can emit distinct captures (e.g. fields_vec.png vs fields_scalar.png).
    const std::string png_out =
        app_opts.png_out.empty() ? std::string(FTD_APP_PNG_OUT) : app_opts.png_out;

    // Telemetry ring-buffer feed bookkeeping (GUI thread). chart_series_scale
    // tracks the scale the buffer currently holds (reset the trace on a switch);
    // last_pushed_seq dedups pushes to one scalar per published snapshot.
    int chart_series_scale = initial_scale;
    std::uint64_t last_pushed_seq = 0;
    bool pushed_any = false;

    // Setup-picker highlight sync (GUI thread). Tracks the last scenario the
    // picker was highlighted for so the current-row highlight follows the
    // effective (host-authoritative) scenario after a Reset, initial boot, live
    // pick, or a W9 validation-reject that changed the loaded id.
    std::string synced_scenario = initial_scenario;

    // Click-to-inspect bookkeeping (GUI thread). last_inspect_seq dedups the
    // per-boundary re-issue to one inspect command per new snapshot;
    // last_inspect_scale drops the selection on a scale switch.
    std::uint64_t last_inspect_seq = 0;
    int last_inspect_scale = initial_scale;

    // The interop StructuredBuffer SRV (heap slot 0) only needs binding ONCE for
    // the lifetime of the never-recreated shared buffer. This catch-up covers both
    // the startup-active case and a later inactive→active reload transition; a
    // D3D12 presenter call must stay on this (GUI) thread, so it lives here rather
    // than in the sim-thread reload block above.
    bool interop_srv_bound = false;

    // ── Cosmetic status-bar throttle (GUI thread) ───────────────────────────
    // The continuously-changing status readouts (tick, particle count, fps,
    // physical time, total/S1 energy) are pushed into the data model on a ~120 ms
    // cadence rather than every frame. Each such push DirtyVariable()s a bound
    // field, and a single dirty status field forces RmlUi to re-lay-out the whole
    // document and regenerate geometry on the next Context::Update()/Render();
    // at 60..145 fps that full reflow ran up to ~145×/s. Throttling caps the
    // status-driven reflow near ~8/s. Everything that changes on USER ACTION or
    // structure (scenario, toggles, overlays, inspector, sheet heights,
    // active_scale, running) stays IMMEDIATE below — those writes are all
    // change-guarded, so a steady state costs nothing.
    constexpr auto kStatusPushInterval = std::chrono::milliseconds(120);
    auto last_status_push = std::chrono::steady_clock::now();
    bool status_first = true;

    const auto loop_start = std::chrono::steady_clock::now();
    bool quit = false;
    MSG msg{};
    while (!quit) {
        if (capture_mode) {
            if (frame_no % 30 == 0)
                std::cerr << "[frame " << frame_no << "] tick=" << data.tick
                          << " particles=" << data.particle_count << "\n" << std::flush;
            if (std::chrono::steady_clock::now() - loop_start > std::chrono::seconds(40)) {
                std::cerr << "capture: deadline exceeded at frame " << frame_no << "\n" << std::flush;
                break;
            }
        }
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) quit = true;
            if (msg.message == WM_KEYDOWN) {
                if (msg.wParam == VK_ESCAPE) quit = true;
                else if (msg.wParam == VK_SPACE) request_play_toggle(&app);
                else if (msg.wParam == 'R') request_reset(&app);
                else if (msg.wParam == 'S') request_step(&app);
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        if (quit || quit_flag.load()) break;

        // Frame-polled resize (keeps swapchain resizes off the wnd_proc reentrancy
        // path). GetClientRect is physical pixels on a per-monitor-V2 window.
        GetClientRect(hwnd, &client);
        const std::uint32_t cw = static_cast<std::uint32_t>(std::max<LONG>(1, client.right));
        const std::uint32_t ch = static_cast<std::uint32_t>(std::max<LONG>(1, client.bottom));
        if (cw != presenter.width() || ch != presenter.height()) {
            presenter.wait_idle();
            presenter.resize(cw, ch);
            context->SetDimensions(Rml::Vector2i(static_cast<int>(cw), static_cast<int>(ch)));
        }
        const UINT dpi = GetDpiForWindow(hwnd);
        const float scale = dpi > 0 ? static_cast<float>(dpi) / 96.0f : 1.0f;
        if (scale != dpi_scale) {
            dpi_scale = scale;
            context->SetDensityIndependentPixelRatio(dpi_scale);
        }

        // Acquire published state. The interop count/fence come straight out of
        // the same frame_mu-protected snapshot as `frame` — populated by the sim
        // thread, so the fence value is exactly the one request_interop_gather()
        // used for the gather this count was polled from.
        ftd::native::NativeFrame frame;
        int this_frame_interop_count = -1;
        std::uint64_t this_frame_fence_value = 0;
        {
            std::lock_guard<std::mutex> lock(frame_mu);
            frame = latest;
            this_frame_interop_count = latest_interop_count;
            this_frame_fence_value = latest_interop_fence;
        }
        const std::uint32_t draw_interop_count =
            this_frame_interop_count > 0
                ? static_cast<std::uint32_t>(this_frame_interop_count)
                : 0u;
        if (capture_mode && frame_no % 30 == 0) {
            std::cerr << "[frame " << frame_no << "] interop="
                      << (interop_active.load() ? "on" : "off")
                      << " interop_count=" << this_frame_interop_count
                      << " draw_count=" << draw_interop_count << "\n" << std::flush;
        }
        std::shared_ptr<const ftd::native::HostSnapshot> snap = host.publisher().acquire();

        if (frame.lattice_size > 0 && frame.lattice_size != camera_lattice) {
            apply_camera_for_lattice(camera, frame.lattice_size);
            camera_lattice = frame.lattice_size;
        }

        // fps (smoothed over ~0.4 s).
        LARGE_INTEGER qpc_now{};
        QueryPerformanceCounter(&qpc_now);
        double dt = qpc_freq.QuadPart
                        ? double(qpc_now.QuadPart - qpc_last.QuadPart) / double(qpc_freq.QuadPart)
                        : 1.0 / 60.0;
        qpc_last = qpc_now;
        if (dt <= 0.0 || dt > 0.5) dt = 1.0 / 60.0;
        fps_accum += dt;
        ++fps_frames;
        if (fps_accum >= 0.4) {
            smoothed_fps = static_cast<int>(fps_frames / fps_accum + 0.5);
            fps_accum = 0.0;
            fps_frames = 0;
        }

        // ── Push snapshot → data model (dirty only what changed) ──
        auto set_int = [&](const char* name, int& dst, int val) {
            if (dst != val) { dst = val; model.DirtyVariable(name); }
        };
        auto set_bool = [&](const char* name, bool& dst, bool val) {
            if (dst != val) { dst = val; model.DirtyVariable(name); }
        };
        auto set_str = [&](const char* name, Rml::String& dst, const std::string& val) {
            if (dst != val) { dst = val; model.DirtyVariable(name); }
        };

        // Throttle gate for the cosmetic status readouts (see kStatusPushInterval).
        const auto now_status = std::chrono::steady_clock::now();
        const bool push_status =
            status_first || (now_status - last_status_push) >= kStatusPushInterval;
        if (push_status) { last_status_push = now_status; status_first = false; }

        if (push_status) set_int("tick", data.tick, frame.tick);
        if (snap) set_int("active_scale", data.active_scale, snap->active_scale);
        if (push_status)
            set_int("particle_count", data.particle_count,
                    static_cast<int>(frame.total_manifested));
        set_bool("running", data.running, !paused.load());
        if (push_status) set_int("fps", data.fps, smoothed_fps);
        set_str("scenario", data.scenario, frame.scenario.empty() ? app.scenario_id
                                                                   : frame.scenario);
        // Keep the picker's current-row highlight on the actually-loaded scenario
        // (Reset / initial boot / live pick / W9-corrected id). Idempotent; dirties
        // only when a highlight flag actually flips.
        {
            const std::string eff =
                frame.scenario.empty() ? app.scenario_id : frame.scenario;
            if (eff != synced_scenario) {
                synced_scenario = eff;
                if (set_current_scenario(&data, eff))
                    model.DirtyVariable("scenario_groups");
            }
        }
        set_str("backend", data.backend, upper(frame.backend.empty() ? host.backend_name()
                                                                      : frame.backend));
        set_str("lattice", data.lattice, std::to_string(frame.lattice_size));
        if (push_status)
            set_str("physical_time", data.physical_time,
                    fmt("%.2e s", static_cast<double>(frame.tick) * kTPhysSeconds));
        // Reflect the adapter's authoritative sheet slice heights (frame-carried)
        // in the panel rows, so a CLI --sheet-height or an adapter clamp shows up
        // live. Only dirties when a value actually changed (steady state = free).
        {
            bool sheets_changed = false;
            for (const ftd::native::NativeSheetHeight& sh : frame.sheet_heights) {
                const auto* d = ftd::native::overlay_by_id(sh.overlay_id);
                if (!d) continue;
                OverlayRow* r = find_overlay_row(&data, Rml::String(d->name));
                if (!r) continue;
                if (std::abs(r->height - sh.height) > 1.0e-4f) {
                    r->height = sh.height;
                    r->hstr = sheet_hstr(sh.height);
                    sheets_changed = true;
                }
            }
            if (sheets_changed) model.DirtyVariable("overlay_columns");
        }
        // Energy + toggles come from whichever ScaleSnapshot alternative is live.
        // Scale 0 carries the full energy ledger + term-toggle state; Scale 1
        // carries a small particle-diagnostics payload feeding the readout panel.
        // Every deref is guarded so the wrong variant is never read after a switch.
        // `chart_energy` is the scalar the telemetry ring buffer plots this frame.
        double chart_energy = 0.0;
        bool chart_energy_valid = false;
        if (const ftd::native::Scale0Snapshot* s0 = snap ? snap->scale0() : nullptr) {
            chart_energy = s0->energy_ledger.E_curr;
            chart_energy_valid = true;
            if (push_status)
                set_str("total_energy", data.total_energy,
                        fmt("%.1f", s0->energy_ledger.E_curr));

            // ── Physics-control live sync (authoritative engine truth) ──
            // Mirror the full TermToggles + published knobs into the app caches
            // (every control callback reads its "current" value from these), then
            // refresh whatever view rows are currently built. All writes are
            // change-guarded, so a steady state costs nothing and never dirties
            // the (reflow-heavy) bindings.
            app.live_toggles = s0->term_toggles;
            app.live_knobs = s0->knobs;
            app.have_live = true;
            app.backend_cpu = (s0->env.backend == ftd::native::BackendKindUi::Cpu);

            bool toggles_changed = false;
            for (ToggleGroupRow& g : data.toggle_groups)
                for (FullToggleRow& r : g.items)
                    if (refresh_toggle_row(r, s0->term_toggles)) toggles_changed = true;
            if (toggles_changed) model.DirtyVariable("toggle_groups");

            if (refresh_config_rows(&data, s0->term_toggles, s0->knobs))
                model.DirtyVariable("config_rows");

            // Live validation banner: the engine's own validate() (+ CPU runtime
            // warnings), so an invalid combo the user builds is surfaced rather
            // than silently stderr-warned. Change-guarded (rarely non-empty).
            std::string verr;
            s0->term_toggles.validate(&verr);
            if (app.backend_cpu) {
                const std::string warn = s0->term_toggles.cpu_runtime_warnings();
                if (!warn.empty()) {
                    if (!verr.empty()) verr += "\n";
                    verr += warn;
                }
            }
            // Surface just the first line (the banner is one row).
            std::string vfirst = verr.substr(0, verr.find('\n'));
            const bool has_v = !vfirst.empty();
            if (data.has_validation != has_v) {
                data.has_validation = has_v;
                model.DirtyVariable("has_validation");
            }
            set_str("validation_msg", data.validation_msg, vfirst);

            // ── Telemetry legend / current-value + freshness (Scale-0) ──
            // Throttled with the cosmetic status readouts and only while the
            // section is open (the values live behind data-if="tel_open", and a
            // dirtied bound field forces a document reflow — the same fps guard the
            // status bar uses). The values come from the published Scale0Snapshot
            // telemetry channels the scheduler now fills; the "t=" provenance is the
            // per-group sampled tick, so a slow audit group reads its own freshness
            // rather than the fast diagnostics tick.
            if (data.tel_open && push_status) {
                const ftd::TelemetrySnapshot& tm = s0->telemetry;
                const ftd::Diagnostics& dg = tm.diagnostics;
                const ftd::EnergyAudit& au = tm.audit;
                const ftd::TelemetryLagrangian& lg = tm.lagrangian;
                set_str("tel_d_energy", data.tel_d_energy,
                        fmt("%.2f", s0->energy_ledger.E_curr));
                set_str("tel_d_manif", data.tel_d_manif,
                        std::to_string(dg.manifested_count));
                set_str("tel_d_entropy", data.tel_d_entropy,
                        fmt("%.3f", dg.total_entropy));
                set_str("tel_d_charge", data.tel_d_charge,
                        std::to_string(dg.positive_count - dg.negative_count));
                set_str("tel_diag_prov", data.tel_diag_prov,
                        "t=" + std::to_string(tm.diagnostics_meta.tick));
                set_str("tel_a_energy", data.tel_a_energy,
                        fmt("%.2f", au.total_energy));
                set_str("tel_a_drift", data.tel_a_drift,
                        fmt("%+.4f", s0->energy_ledger.dE_dt));
                set_str("tel_a_gauss", data.tel_a_gauss,
                        fmt("%.2e", au.gauss_violation));
                set_str("tel_audit_prov", data.tel_audit_prov,
                        "t=" + std::to_string(tm.audit_meta.tick));
                set_str("tel_l_lag", data.tel_l_lag,
                        fmt("%.2f", lg.total_lagrangian));
                set_str("tel_l_ham", data.tel_l_ham,
                        fmt("%.2f", lg.total_hamiltonian));
            }
        } else if (const ftd::native::Scale1Snapshot* s1 = snap ? snap->scale1() : nullptr) {
            chart_energy = s1->total_energy;
            chart_energy_valid = true;
            if (push_status) {
                set_str("total_energy", data.total_energy, fmt("%.3f", s1->total_energy));
                set_str("s1_ke", data.s1_ke, fmt("%.3f", s1->total_ke));
                set_str("s1_pe", data.s1_pe, fmt("%.3f", s1->total_pe));
            }
        }

        // ── Feed the telemetry ring buffers (snapshot-only, GUI thread) ──
        // Reset every series on a scale switch so Scale-0 telemetry is never
        // plotted next to Scale-1 telemetry; push exactly one scalar per NEW
        // published snapshot (dedup by seq — the GUI loop runs faster than the sim
        // tick). The DIAGNOSTICS channels are fed on every Scale-0 boundary so the
        // base charts are already populated when the section is opened; the heavier
        // AUDIT + LAGRANGIAN channels are fed only while the section is open (their
        // scheduler groups are only demanded then).
        if (snap) {
            if (snap->active_scale != chart_series_scale) {
                diag_energy.clear(); diag_manif.clear(); diag_entropy.clear();
                diag_charge.clear();
                aud_energy.clear(); aud_drift.clear(); aud_gauss.clear();
                lag_lag.clear(); lag_ham.clear();
                chart_series_scale = snap->active_scale;
                last_pushed_seq = 0;
                pushed_any = false;
            }
            if (chart_energy_valid && (!pushed_any || snap->seq != last_pushed_seq)) {
                // Energy trace is dual-scale (Scale-0 accounted E / Scale-1 total).
                diag_energy.push(static_cast<float>(chart_energy));
                if (const ftd::native::Scale0Snapshot* s0 = snap->scale0()) {
                    const ftd::Diagnostics& dg = s0->telemetry.diagnostics;
                    diag_manif.push(static_cast<float>(dg.manifested_count));
                    diag_entropy.push(static_cast<float>(dg.total_entropy));
                    diag_charge.push(
                        static_cast<float>(dg.positive_count - dg.negative_count));
                    if (data.tel_open) {
                        const ftd::EnergyAudit& au = s0->telemetry.audit;
                        aud_energy.push(static_cast<float>(au.total_energy));
                        aud_drift.push(static_cast<float>(s0->energy_ledger.dE_dt));
                        aud_gauss.push(static_cast<float>(au.gauss_violation));
                        const ftd::TelemetryLagrangian& lg = s0->telemetry.lagrangian;
                        lag_lag.push(static_cast<float>(lg.total_lagrangian));
                        lag_ham.push(static_cast<float>(lg.total_hamiltonian));
                    }
                }
                last_pushed_seq = snap->seq;
                pushed_any = true;
            }
        }

        // ── Click-to-inspect (GUI thread) ────────────────────────────────────
        // (a) resolve a pending scene click into a selection (unproject + pick),
        // (b) re-issue the inspect command once per new boundary so the readout
        // stays LIVE, (c) mirror the published inspection into the data model.
        // Runs before Context::Update() so the readout dirties lay out this frame;
        // the pick reuses the previous frame's viewport_rect (stable frame-to-
        // frame, and always valid once the shell has laid out at least once).
        const int cur_scale = snap ? snap->active_scale : data.active_scale;
        bool scroll_physics_bottom = capture_mode && !app_opts.no_scroll;
        if (cur_scale != last_inspect_scale) {
            // Scale switch: the old target index is meaningless on the new scale.
            app.inspect_kind = 0;
            app.inspect_pidx = -1;
            app.insp_has_data = false;
            last_inspect_scale = cur_scale;
            last_inspect_seq = 0;
        }
        if (app.pick_pending) {
            app.pick_pending = false;
            const PickRay ray =
                make_pick_ray(camera, app.viewport_rect, app.pick_x, app.pick_y);
            if (cur_scale == 0) {
                const int L = frame.lattice_size > 0 ? frame.lattice_size : camera_lattice;
                int vx = 0, vy = 0, vz = 0;
                if (pick_scale0(frame, ray, L, vx, vy, vz)) {
                    app.inspect_kind = 1;
                    app.inspect_vx = vx;
                    app.inspect_vy = vy;
                    app.inspect_vz = vz;
                    app.insp_has_data = false;      // fresh target — reset the latch
                    scroll_physics_bottom = true;   // reveal the readout on a hit
                } else {
                    app.inspect_kind = 0;           // empty space → clear
                }
            } else if (cur_scale == 1) {
                int pidx = -1;
                if (pick_scale1(frame, ray, pidx)) {
                    app.inspect_kind = 2;
                    app.inspect_pidx = pidx;
                    app.insp_has_data = false;
                    scroll_physics_bottom = true;
                } else {
                    app.inspect_kind = 0;
                }
            }
            last_inspect_seq = 0;   // force an immediate re-issue for the new target
        }
        // Re-issue the inspect command once per NEW published snapshot so the
        // adapter refreshes the inspection payload every boundary (live data).
        if (snap && app.inspect_kind != 0 && snap->seq != last_inspect_seq) {
            if (app.inspect_kind == 1) {
                push_scale0(&app, ftd::native::InspectVoxel{app.inspect_vx, app.inspect_vy,
                                                            app.inspect_vz});
            } else if (app.inspect_kind == 2) {
                push_scale1(&app, ftd::native::InspectParticle1{app.inspect_pidx});
            }
            last_inspect_seq = snap->seq;
        }
        // Mirror the published inspection into the bound readout fields.
        if (app.inspect_kind == 0) {
            if (data.insp_active) {
                data.insp_active = false;
                data.insp_lines.clear();
                model.DirtyVariable("insp_active");
                model.DirtyVariable("insp_lines");
            }
        } else {
            Rml::String title;
            Rml::Vector<InspLine> lines;
            bool have_now = false;   // this snapshot carries fresh inspection data
            if (app.inspect_kind == 1) {
                title = fmt3("Voxel (%.0f, %.0f, %.0f)",
                             static_cast<double>(app.inspect_vx),
                             static_cast<double>(app.inspect_vy),
                             static_cast<double>(app.inspect_vz));
                const ftd::native::Scale0Snapshot* s0 = snap ? snap->scale0() : nullptr;
                if (s0 && s0->voxel_present) {
                    const ftd::VoxelInspection& vi = s0->voxel;
                    const ftd::Vec3& J = vi.voxel.flux;
                    const double jmag = std::sqrt(J.x * J.x + J.y * J.y + J.z * J.z);
                    const double cmag = std::sqrt(vi.curl.x * vi.curl.x + vi.curl.y * vi.curl.y
                                                  + vi.curl.z * vi.curl.z);
                    lines.push_back(InspLine{"State", std::to_string(static_cast<int>(vi.voxel.state))});
                    lines.push_back(InspLine{"Flux J", fmt3("%.3f, %.3f, %.3f", J.x, J.y, J.z)});
                    lines.push_back(InspLine{"|J|", fmt("%.4f", jmag)});
                    lines.push_back(InspLine{"Div J", fmt("%.4f", vi.divergence)});
                    lines.push_back(InspLine{"|Curl|", fmt("%.4f", cmag)});
                    have_now = true;
                }
            } else if (app.inspect_kind == 2) {
                title = "Particle #" + std::to_string(app.inspect_pidx);
                const ftd::native::Scale1Snapshot* s1 = snap ? snap->scale1() : nullptr;
                if (s1 && s1->insp_present) {
                    const double vmag = std::sqrt(s1->insp_vel[0] * s1->insp_vel[0]
                                                  + s1->insp_vel[1] * s1->insp_vel[1]
                                                  + s1->insp_vel[2] * s1->insp_vel[2]);
                    const std::string chg = (s1->insp_charge >= 0 ? "+" : "")
                                            + std::to_string(s1->insp_charge);
                    lines.push_back(InspLine{"Charge", chg});
                    lines.push_back(InspLine{"Pos", fmt3("%.2f, %.2f, %.2f", s1->insp_pos[0],
                                                         s1->insp_pos[1], s1->insp_pos[2])});
                    lines.push_back(InspLine{"Vel", fmt3("%.3f, %.3f, %.3f", s1->insp_vel[0],
                                                         s1->insp_vel[1], s1->insp_vel[2])});
                    lines.push_back(InspLine{"|v|", fmt("%.4f", vmag)});
                    lines.push_back(InspLine{"Locked", s1->insp_locked ? "yes" : "no"});
                    have_now = true;
                }
            }
            if (data.insp_title != title) {
                data.insp_title = title;
                model.DirtyVariable("insp_title");
            }
            // Refresh the lines only on a snapshot that actually carries the
            // inspection (they change every boundary — live data). Between the
            // sparse re-issues keep the last values; show "reading..." only until
            // the very first payload for this target arrives.
            if (have_now) {
                data.insp_lines = std::move(lines);
                model.DirtyVariable("insp_lines");
                app.insp_has_data = true;
            } else if (!app.insp_has_data) {
                data.insp_lines.clear();
                data.insp_lines.push_back(InspLine{"", "reading..."});
                model.DirtyVariable("insp_lines");
            }
            if (!data.insp_active) {
                data.insp_active = true;
                model.DirtyVariable("insp_active");
            }
        }

        // Lay out, then map the #viewport hole rect for the scene + input.
        context->Update();
        if (Rml::Element* vp = doc->GetElementById("viewport")) {
            const Rml::Vector2f off = vp->GetAbsoluteOffset(Rml::BoxArea::Border);
            const int w = static_cast<int>(vp->GetOffsetWidth());
            const int h = static_cast<int>(vp->GetOffsetHeight());
            if (w > 0 && h > 0) {
                app.viewport_rect = {static_cast<int>(off.x), static_cast<int>(off.y),
                                     static_cast<std::uint32_t>(w),
                                     static_cast<std::uint32_t>(h)};
                presenter.set_scene_rect(app.viewport_rect);
            }
        }

        // The right panel now overflows the body-row height (PHYSICS TERMS +
        // FIELDS + telemetry chart + inspector). Interactively it scrolls by
        // wheel; in a headless capture (no wheel) and on a fresh pick, scroll it
        // to the bottom so the inspector + chart are in view. Post-Update so
        // GetScrollHeight is valid; SetScrollTop dirties the child offsets, which
        // render() recomputes, so the same frame shows the scroll. No-op when the
        // panel fits.
        if (scroll_physics_bottom) {
            if (Rml::Element* phys = doc->GetElementById("physics"))
                phys->SetScrollTop(phys->GetScrollHeight());
        }

        // ── Capture mode: after warmup, request + poll a composited readback ──
        // A capture is ARMED by exactly one render() after request_capture (that
        // frame records the copy into the readback buffer and signals the capture
        // fence). D3D12Presenter::render() re-arms the still-pending capture on
        // EVERY subsequent call — bumping the target fence value one ahead of what
        // the GPU has completed — so if we kept rendering while polling, poll would
        // never converge. Once armed, stop rendering and just poll until the fence
        // completes. (Pre-existing presenter behavior; the app owns the arm-once
        // discipline.)
        const bool capture_armed = capture_mode && capture_requested;
        if (capture_mode && !capture_saved) {
            // When interop is active, hold off arming until a device-resident
            // gather has actually landed (draw_interop_count > 0), so the captured
            // frame renders the interop particles rather than the CPU fallback that
            // shows before the first gather completes.
            if (draw_interop_count > 0) interop_gathered_once = true;
            // Arm-gate fallback (pure-field scenarios): if interop is enabled but
            // NO gather has ever landed a short grace past the warmup, arm anyway
            // (the CPU particle path — empty here — still supplies the frame), so a
            // 0-particle field capture writes a PNG instead of hitting the 40 s
            // deadline. Disabled the instant a gather lands (interop_gathered_once),
            // so behavior is unchanged whenever particles DO gather.
            constexpr int kInteropGraceFrames = 60;
            const bool interop_zero_particle_grace =
                interop_active.load() && !interop_gathered_once
                && frame_no >= app_opts.capture_frames + kInteropGraceFrames;
            const bool interop_ready_or_na =
                !interop_active.load() || draw_interop_count > 0
                || interop_zero_particle_grace;
            if (!capture_requested && frame_no >= app_opts.capture_frames
                && interop_ready_or_na) {
                capture_token = presenter.request_capture(ftd::native::CaptureRegion::FullWindow);
                capture_requested = true;
            }
            if (capture_requested) {
                ftd::native::CaptureResult res = presenter.poll_capture(capture_token);
                if (res.status == ftd::native::CaptureStatus::Ready) {
                    const bool ok = save_png(widen(png_out), res.bytes.data(), res.width,
                                             res.height, res.row_pitch);
                    std::cout << "capture: " << (ok ? "wrote " : "FAILED ") << png_out
                              << " (" << res.width << "x" << res.height
                              << ", tick=" << data.tick << ", particles=" << data.particle_count
                              << ", interop=" << (interop_active.load() ? "on" : "off")
                              << ", interop_count=" << this_frame_interop_count
                              << ", fps=" << smoothed_fps
                              << ", lattice=" << frame.lattice_size
                              << ", energy=" << data.total_energy.c_str() << ")\n";
                    // The readback already completed (poll returned Ready) and the
                    // PNG is committed to disk. Clean-exit attempt: mark the capture
                    // done and fall through to the normal run_app teardown (sim join,
                    // GPU idle, RmlUi shutdown, D3D12 release) + a normal return,
                    // rather than hard-terminating here.
                    std::cout.flush();
                    std::cerr.flush();
                    capture_ok = ok;
                    capture_saved = true;
                    running.store(false);
                    quit = true;
                    break;
                } else if (res.status == ftd::native::CaptureStatus::Failed) {
                    std::cerr << "capture: failed: " << res.error << "\n" << std::flush;
                    capture_saved = true;
                    quit = true;
                }
            }
        }

        // Skip render only AFTER the capture is armed (armed on a prior frame),
        // so the presenter does not re-arm the pending readback. The arming frame
        // itself (capture_armed still false here) renders normally.
        if (!capture_armed) {
            try {
                // Bind the interop SRV once (idempotent, GUI-thread-only D3D12
                // call), then make the render queue wait until CUDA has signaled
                // the shared fence for the gather whose count we are about to draw.
                if (interop_active.load() && !interop_srv_bound) {
                    presenter.bind_interop_particle_srv();
                    interop_srv_bound = true;
                }
                if (draw_interop_count != 0) {
                    presenter.wait_shared_fence(this_frame_fence_value);
                }
                presenter.render(frame, camera, view_opts, draw_interop_count);
            } catch (const std::exception& ex) {
                std::cerr << "render threw at frame " << frame_no << ": " << ex.what() << "\n"
                          << std::flush;
                running.store(false);
                sim.join();
                throw;
            }
        }
        ++frame_no;

        // Interactive: yield a little so the sim thread gets CPU; capture mode
        // spins to reach the readback fast.
        if (!capture_mode) std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // ── Teardown ────────────────────────────────────────────────────────────────
    running.store(false);
    sim.join();
    presenter.set_overlay_recorder(nullptr);
    app.context = nullptr;
    context = nullptr;  // stop the overlay from re-entering RmlUi during shutdown
    presenter.wait_idle();
    // The sim thread (the only thread that re-imports these after startup) is
    // joined and the GPU is idle, so nothing on either API still references the
    // shared buffer/fence. CUDA imported (did not take ownership of) these NT
    // handles, so closing them here frees the handle without touching the
    // still-live CUDA external objects. (In --capture-frames mode the process is
    // hard-terminated before reaching here; the OS reclaims the handles.)
    if (interop_fence_handle) CloseHandle(interop_fence_handle);
    if (interop_buf_handle) CloseHandle(interop_buf_handle);
    Rml::Shutdown();  // renderer (declared after presenter) still alive here

    if (capture_mode) return (capture_saved && capture_ok) ? 0 : 2;
    return 0;
}

}  // namespace

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
    attach_parent_console_if_any();
    HRESULT co = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    int rc = 1;
    try {
        rc = run_app(utf8_args());
    } catch (const std::exception& ex) {
        std::cerr << "native_app: " << ex.what() << "\n";
        MessageBoxA(nullptr, ex.what(), "FTD Native App", MB_ICONERROR);
        rc = 1;
    }
    if (SUCCEEDED(co)) CoUninitialize();
    // Clean-exit attempt (see run_app teardown): all meaningful teardown has run
    // above (GPU idle, RmlUi shut down, the D3D12 device + swapchain released as
    // run_app's locals unwound). Return normally and let the CRT exit path run.
    std::cout.flush();
    std::cerr.flush();
    return rc;
}
