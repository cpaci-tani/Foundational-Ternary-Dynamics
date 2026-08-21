#pragma once
//
// app/ui_model.h — the RmlUi data-model layer of native_app: the C++ mirror of
// the shell (ShellData + its row types) plus the config-knob spec table, split
// out of app/main.cpp for readability (behavior-neutral). The builder functions
// that populate these rows from engine truth are declared here and defined in
// app/ui_model.cpp.
//
#include "ftd/term_toggles.h"     // ftd::TermToggles, ftd::ToggleSpec (builder sigs)
#include "native/ui_snapshot.h"   // ftd::native::BridgeKnobs (config builder sigs)

#include <RmlUi/Core.h>           // Rml::String, Rml::Vector

#include <string>
#include <vector>

namespace ftd::native::app {

// ── The physics-control panel is data-driven from ftd::TOGGLE_SPECS[] (all 44
//    toggles). To keep the live DOM small (fps), the toggle list is split into a
//    few named categories, each independently collapsible. Every TOGGLE_SPECS row
//    maps to exactly one category via toggle_category(); an unrecognized name
//    falls into the last bucket, so a newly-added toggle can never vanish.
inline constexpr const char* kToggleCategories[] = {
    "Core dynamics",
    "Forces & gravity",
    "Nuclear / gauge",
    "Thermal / quantum / diag",
};

// Assign a toggle (by canonical TOGGLE_SPECS name) to a category index.
int toggle_category(std::string_view n);

// Non-bool config knobs, exposed as -/+ nudge controls (RmlUi has no range
// input). `kind` selects the command + value semantics; enum kinds cycle labels.
enum class CfgKind { Double, Int, Boundary, Bcc, SiteFilter, UInt, Lattice };
struct ConfigSpec {
    const char* key;    // stable id (matches config_nudge dispatch)
    const char* label;  // human label
    CfgKind     kind;
    double      lo, hi, step;
    const char* hint;   // units / note (static subtitle)
};
const ConfigSpec* find_config_spec(std::string_view key);
Rml::String boundary_label(int v);
Rml::String bcc_label(int v);
Rml::String site_label(int v);
Rml::String config_value_str(const ConfigSpec& s, double v);

// One tick in physical seconds (electron-primary gauge: t_phys = t_P/sqrt(3), see
// CLAUDE.md). Renders a human-facing "physical time" in the status bar only.
inline constexpr double kTPhysSeconds = 3.11e-44;

// ── RmlUi data-model mirror of the shell (the bound C++ side). ──────────────
// One physics toggle row. name/desc/req come from the TOGGLE_SPECS row (static);
// on/gated are the live engine truth. req is the requires/conflicts/gpu-only
// metadata; gated lights amber when the current combo violates a rule.
struct FullToggleRow {
    Rml::String name;
    Rml::String desc;
    Rml::String req;
    bool on = false;
    bool gated = false;
    bool has_req = false;
};
// One collapsible toggle category. Items are built only while expanded (the
// scenario-picker DOM-shrink pattern), so a closed category costs ~0 DOM.
struct ToggleGroupRow {
    Rml::String title;
    bool expanded = false;
    Rml::String count;
    Rml::Vector<FullToggleRow> items;
};
// One config-knob row: label + current value + -/+ nudge. key maps to the
// ConfigSpec / command dispatch.
struct ConfigRow {
    Rml::String key;
    Rml::String label;
    Rml::String vstr;
    Rml::String hint;
};

// One overlay toggle row (Scale-0 panel). on lights the LED when the overlay is
// in the active set (multi-select). Rubber-sheet rows (is_sheet) also carry a
// slice height: when active they expose a -/+ height nudge and hstr shows it.
struct OverlayRow {
    Rml::String name;   // stable overlay id (registry name)
    Rml::String label;  // human label
    bool on = false;
    bool is_sheet = false;  // true for rubber-sheet overlays (height-adjustable)
    float height = 0.0f;    // current slice height (fraction of the lattice box)
    Rml::String hstr;       // formatted height ("0.42") for display
};
// One overlay-menu column (Volume / Fields / Forces / ...). expanded gates the
// collapsible list; items are the overlays in this column; count is the badge.
struct OverlayColumnRow {
    Rml::String title;
    bool expanded = true;
    Rml::String count;
    Rml::Vector<OverlayRow> items;
};

// One key/value line in the click-to-inspect readout. header marks a group
// divider (empty value; the key is a full-width section title).
struct InspLine {
    Rml::String k;
    Rml::String v;
    bool header = false;
};

// One Moore-neighbour cell in the 26-neighbour grid. dir labels the offset;
// val carries the state glyph + |flux| readout; state (-1/0/+1) drives colour.
struct InspNeighCell {
    Rml::String dir;
    Rml::String val;
    int state = 0;
};

// One Scale-0 scenario row in the Setup picker. current highlights the loaded
// scenario; visible gates the row under the live search filter; search is the
// lowercased "title id tags" haystack (C++ filter only, not bound into RML).
struct ScenarioRow {
    Rml::String id;
    Rml::String title;
    bool current = false;
    bool visible = true;
    Rml::String search;
};
// One category group in the picker: the 5 honest scenario classes, collapsible.
// has_visible = any item passes the filter (gates the header); show_items =
// header expanded OR a filter active (gates the list); count = visible tally.
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
    // Physics control surface (Scale-0), collapsed by default so the steady-state
    // DOM stays tiny (fps). phys_open gates the toggle categories; cfg_open the
    // config knobs; ov_open the field overlays. A closed section leaves its bound
    // array EMPTY, so the nested data-for instantiates ~0 rows.
    bool phys_open = false;
    bool cfg_open = false;
    bool ov_open = false;
    Rml::Vector<ToggleGroupRow> toggle_groups;   // the 44 toggles, categorized
    Rml::Vector<ConfigRow> config_rows;          // dt / SOR / boundary / ... + reset
    // Live validation banner: TermToggles::validate() (+ CPU runtime warnings).
    bool has_validation = false;
    Rml::String validation_msg;
    // FIELDS overlay panel (Scale-0): the 7-column multi-select menu, built from
    // the shared registry, grouped by column; empty columns omitted.
    Rml::Vector<OverlayColumnRow> overlay_columns;
    // Global force render-style (0 Arrows / 1 Heatmap / 2 Flow / 3 Glyphs).
    int force_style = 0;
    // Setup scenario picker (Scale-0): the ~130 native-catalog scenarios grouped
    // into the 5 honest classes, searchable + collapsible. scn_open gates the list
    // (COLLAPSED by default); while collapsed scenario_groups is left EMPTY.
    bool scn_open = false;
    Rml::Vector<ScenarioGroupRow> scenario_groups;
    // Click-to-inspect readout. insp_active gates the panel section; insp_title
    // names the picked entity; insp_lines are its live fields.
    bool insp_active = false;
    Rml::String insp_title;
    Rml::Vector<InspLine> insp_lines;
    // 26-Moore-neighbour sub-section (Scale-0 voxel picks only). insp_neigh_show
    // gates it (false for Scale-1 particle picks); insp_neigh_open is the collapse
    // state — while closed the three cell vectors are EMPTY and the app stops
    // issuing the 26-read gather.
    bool insp_neigh_show = false;
    bool insp_neigh_open = true;
    Rml::Vector<InspNeighCell> insp_faces;    // 6 face neighbours (shell 1)
    Rml::Vector<InspNeighCell> insp_edges;    // 12 edge neighbours (shell 2)
    Rml::Vector<InspNeighCell> insp_corners;  // 8 corner neighbours (shell 3)
    // Telemetry section (Scale-0 telemetry hub), collapsed by default (fps). The
    // <ftd-chart> traces come from app-side ring buffers; these strings are the
    // throttled current-value + freshness legend from the Scale0Snapshot channels.
    bool tel_open = false;
    // Diagnostics group (always demanded; cheap cadence-1).
    Rml::String tel_d_energy = "—";   // accounted total energy (E_curr)
    Rml::String tel_d_manif = "—";    // manifested particle count
    Rml::String tel_d_entropy = "—";  // total entropy
    Rml::String tel_d_charge = "—";   // net charge (positive - negative)
    Rml::String tel_diag_prov = "—";  // freshness: sampled tick of the diag group
    // Audit / conservation group (demanded only while the section is open).
    Rml::String tel_a_energy = "—";   // accounted total energy (audit reduction)
    Rml::String tel_a_drift = "—";    // energy drift dE/dt (conservation view)
    Rml::String tel_a_gauss = "—";    // Gauss-law residual
    Rml::String tel_audit_prov = "—"; // freshness: sampled tick of the audit group
    // Lagrangian group (demanded only while the section is open).
    Rml::String tel_l_lag = "—";      // total Lagrangian
    Rml::String tel_l_ham = "—";      // total Hamiltonian
};

// ── Toggle-panel builders (build + live-sync the toggle model). ──────────────
Rml::String toggle_req_text(const ftd::ToggleSpec& spec);
bool toggle_gated(const ftd::ToggleSpec& spec, const ftd::TermToggles& tt);
bool refresh_toggle_row(FullToggleRow& r, const ftd::TermToggles& tt);
void build_toggle_group_items(ToggleGroupRow& g, int cat, const ftd::TermToggles& tt);
int toggle_group_count(int cat);
void rebuild_toggle_groups(ShellData* data, const ftd::TermToggles& tt,
                           const std::vector<std::string>& expanded);

// ── Config-knob builders. ────────────────────────────────────────────────────
double config_current(const ConfigSpec& s, const ftd::TermToggles& tt,
                      const ftd::native::BridgeKnobs& knobs);
void build_config_rows(ShellData* data, const ftd::TermToggles& tt,
                       const ftd::native::BridgeKnobs& knobs);
bool refresh_config_rows(ShellData* data, const ftd::TermToggles& tt,
                         const ftd::native::BridgeKnobs& knobs);

// ── Overlay-panel builders. ──────────────────────────────────────────────────
Rml::String sheet_hstr(float h);
Rml::Vector<OverlayColumnRow> build_overlay_columns();
OverlayRow* find_overlay_row(ShellData* data, const Rml::String& name);

// ── Scenario-picker builders. ────────────────────────────────────────────────
// The 5 honest Scale-0 scenario classes, in catalog order — the picker headers.
inline constexpr const char* kScenarioCategories[] = {
    "1. Validated Native Dynamics",
    "2. Validated State Dynamics",
    "3. Qualified Selected Extensions",
    "4. Validated Initial Data",
    "5. Macroscopic Physics & Measurement",
};
void rebuild_scenario_view(ShellData* data, const std::string& current,
                           const std::string& raw_filter,
                           const std::vector<std::string>& expanded);
bool set_current_scenario(ShellData* data, const std::string& id);

}  // namespace ftd::native::app
