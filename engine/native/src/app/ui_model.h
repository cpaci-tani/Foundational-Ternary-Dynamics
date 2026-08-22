#pragma once
//
// app/ui_model.h — the RmlUi data-model layer of native_app: the C++ mirror of
// the shell (ShellData + its row types) plus the config-knob spec table, split
// out of app/main.cpp for readability (behavior-neutral). The builder functions
// that populate these rows from engine truth are declared here and defined in
// app/ui_model.cpp.
//
#include "ftd/term_toggles.h"     // ftd::TermToggles, ftd::ToggleSpec (builder sigs)
#include "ftd/render_bridge_diagnostics.h"  // ftd::Diagnostics/EnergyAudit/EnergyLedger (DiagInputs)
#include "native/ui_snapshot.h"   // ftd::native::BridgeKnobs (config builder sigs)

#include <RmlUi/Core.h>           // Rml::String, Rml::Vector

#include <cmath>
#include <limits>
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
    int dx = 0, dy = 0, dz = 0;  // Moore offset — click-to-walk the inspection cursor here
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
    // Epistemic-status badge: the leading "[TAG]" of the catalog row's
    // epistemic_status (e.g. "EMERGENT", "IMPOSED", "QUALIFIED NEGATIVE"), plus a
    // colour class (0 neutral / 1 derived-emergent / 2 imposed-selection /
    // 3 conjecture-open / 4 negative). Surfaces the scientific status in the picker.
    Rml::String tag;
    int tag_cls = 0;
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

// One Diagnostics-panel metric row: Metric | Value | Unit | Min | Max | Avg.
// value is the live formatted reading; vmin/vmax/vavg come from the per-metric
// RunningStat (rendered "—" when has_stat is false — pair/triple/vector rows —
// or the accumulator is still empty). variant tints the value (1 green / 2 red).
struct InstrumentRow {
    Rml::String label;
    Rml::String value = "—";
    Rml::String unit;
    Rml::String vmin = "—";
    Rml::String vmax = "—";
    Rml::String vavg = "—";
    int variant = 0;
    bool has_stat = true;
};
// One collapsible Diagnostics section (Particle State / Energy Budget / ...).
struct InstrumentSection {
    Rml::String title;
    bool expanded = true;
    Rml::Vector<InstrumentRow> rows;
};

// One alive knot row in the Knots panel table (from the engine KnotTracker).
// sign_cls tints the sign chip: 1 positive (green), 2 negative (red).
struct KnotUiRow {
    Rml::String id;
    Rml::String sign;
    Rml::String age;
    Rml::String size;
    Rml::String flux;
    Rml::String org;
    int sign_cls = 0;
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
    // Toolbar (nav-bar) dropdowns: scale switcher + scenario selector. Each gates
    // its own popup menu (data-if); a closed scenario menu leaves scenario_groups
    // EMPTY so the ~130 rows never sit in the idle DOM.
    bool scale_dd_open = false;
    bool scn_dd_open = false;
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
    // ── Gravity instrument (Scale-0 analysis panel) ───────────────────────────
    // A collapsible instrument reading the engine's GravityMetricAgg (the REAL
    // C++ latency field from the Poisson solver — voxel.latency, not the |J|² web
    // proxy): potential L, lapse f = 1−L², γ, time dilation, and the active-voxel
    // count. Demands the TELEMETRY_GRAVITY scheduler group only while open (fps).
    bool grav_open = false;
    Rml::String grav_l_max = "—";     // max voxel.latency (gravity potential L)
    Rml::String grav_l_mean = "—";    // mean L over voxels with L>0
    Rml::String grav_f_min = "—";     // min lapse f = 1 − L_max²  (deepest dilation)
    Rml::String grav_gamma = "—";     // max gamma_ftd()
    Rml::String grav_dilation = "—";  // (1 − √f_min)·100 %  (time dilation)
    Rml::String grav_voxels = "—";    // voxels with latency > 0
    Rml::String grav_prov = "—";      // freshness: sampled tick of the gravity group
    Rml::String grav_status = "";     // "term off" / "on, no field yet" note
    bool grav_inactive = false;       // gate the status note (requested && !active, or off)
    // ── Time instrument (causal-clock, Scale-0 analysis panel) ────────────────
    // The clock-perspective on the SAME latency field: the causal clock runs at
    // dτ/dt = √(1−L²) (the clock hypothesis), so the deepest well's clock rate is
    // √f_min and its accumulated dilation is (1−√f_min)·100 %. Demands the gravity
    // group (shares GravityMetricAgg with the Gravity panel).
    bool time_open = false;
    Rml::String time_dtau = "—";      // clock rate dτ/dt = √f_min  (slowest clock)
    Rml::String time_dilation = "—";  // (1 − √f_min)·100 %  (time dilation)
    Rml::String time_gamma = "—";     // γ max
    Rml::String time_f = "—";         // lapse f_min = 1 − L_max²
    Rml::String time_prov = "—";      // freshness: sampled tick of the gravity group
    // ── Thermodynamics instrument (Scale-0 analysis panel) ────────────────────
    // The lattice's thermal state: the Langevin bath setpoint T (langevin_T), the
    // equipartition kinetic temperature T_kin = ⟨½|wave_vel|²⟩/(3/2) = E_wave /
    // (1.5·L³) with k_B ≡ 1, total wave energy, entropy, and manifested count.
    // Demands the AUDIT group (for E_wave) while open; entropy/count are cadence-1.
    bool thermo_open = false;
    Rml::String therm_bath = "—";     // Langevin bath temperature (langevin_T)
    Rml::String therm_kin = "—";      // kinetic temperature T_kin (equipartition)
    Rml::String therm_wave = "—";     // total wave energy Σ ½|wave_vel|²
    Rml::String therm_entropy = "—";  // total entropy
    Rml::String therm_manif = "—";    // manifested (condensed) site count
    Rml::String therm_prov = "—";     // freshness: sampled tick of the audit group
    // ── Spectrum instrument (lattice spectroscopy, Scale-0 analysis panel) ────
    // The spatial energy spectrum E(k) of the flux field, computed by an adapter-
    // side 3D FFT (native/spectrum.h; demand.spectrum). The <ftd-chart> plots the
    // log E(k) curve from an app ring buffer; these readouts are the scalars.
    bool spectrum_open = false;
    Rml::String spec_peak_k = "—";    // |k| of the dominant (max-E) mode
    Rml::String spec_power = "—";     // total power Σ|J|² (Parseval)
    Rml::String spec_slope = "—";     // log-log spectral index
    Rml::String spec_grid = "—";      // FFT grid M = nextPow2(L)
    Rml::String spec_prov = "—";      // freshness note
    // Spectrum+ field-topology readouts (demand the AUDIT group while open).
    Rml::String spec_gauss = "—";     // Gauss residual Σ(divJ−s)²
    Rml::String spec_chi = "—";       // chirality Σχ
    Rml::String spec_efield = "—";    // E-field energy
    Rml::String spec_bfield = "—";    // B-field energy
    // ── Knots panel (active_panel==10): engine KnotTracker telemetry ──────────
    // Aggregate counts + the top alive knots (by size). knot_note carries the
    // "enable/unavailable" status (empty when live). Built only while active.
    Rml::Vector<KnotUiRow> knot_rows;
    Rml::String knot_alive = "—";       // alive count
    Rml::String knot_charge = "—";      // net charge Σ sign
    Rml::String knot_lifecycle = "—";   // "B born · D died · F split · G merged"
    Rml::String knot_note = "";         // status note (blocked / waiting); "" = live
    bool knot_has_note = false;
    // ── Panel rail (the left instrument-panel switcher) ───────────────────────
    // active_panel selects which LEFT panel is shown; every panel block is
    // data-if="active_panel==N" (0 Scenario · 1 Diagnostics · 2 Telemetry ·
    // 3 Gravity · 4 Time · 5 Thermo · 6 Spectrum). select_panel(N) sets it and
    // DERIVES the *_open gates above, so request_telemetry_demand + all the
    // per-frame sync blocks keep working unchanged. diag_active mirrors
    // (active_panel==1) for the demand + Diagnostics refresh guard.
    int active_panel = 1;      // default to Diagnostics (the rich metric dashboard)
    bool diag_active = true;
    // ── Diagnostics panel (active_panel==1): the rich Min/Max/Avg metric tables.
    // Built only while diag_active (DOM-shrink); the ~28-row nested data-for
    // dirties only on a real value change. The per-group freshness "state t… · N
    // ms" is kept in the two flat strings (NOT in the array) so the ~8/s age tick
    // never reflows the table.
    Rml::Vector<InstrumentSection> diag_sections;
    Rml::String diag_fresh_diag = "—";
    Rml::String diag_fresh_audit = "—";
    // ── Epistemic-status box (per active scenario, pinned above the panels) ────
    // Split of the active scenario's ScenarioMeta.epistemic_status: the leading
    // "[TAG]" (epi_tag, brackets kept), a colour class (epi_cls: 1 emergent /
    // 2 imposed / 3 open / 4 negative, reusing the .scn-tag palette), the
    // qualification clause after the ']' (epi_body), and the scenario title.
    // epi_tag == "" hides the box (Scale-1 / unknown scenario).
    Rml::String epi_tag;
    int epi_cls = 0;
    Rml::String epi_body;
    Rml::String epi_title;
    // ── Resizable side-panel widths ───────────────────────────────────────────
    // Bound to #setup / #physics via data-style-width; updated by the .resizer
    // drag callbacks (resize_left / resize_right). Strings so RCSS reads them as
    // lengths (e.g. "380dp").
    Rml::String setup_width = "380dp";
    Rml::String physics_width = "248dp";
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

// ── Epistemic-status box ─────────────────────────────────────────────────────
// Fill data->epi_tag / epi_cls / epi_body / epi_title from the catalog metadata
// of scenario `id` (looked up via find_scenario_meta). Clears them (hiding the
// box) when the id has no Scale-0 catalog entry. Returns true if any field
// changed. Reuses the picker's tag parsing (leading_tag / classify_tag).
bool fill_epistemic(ShellData* data, const std::string& id);

// ── Diagnostics-panel instrument tables + per-metric running stats. ──────────
// A cumulative-since-reset Min/Max/Avg accumulator for one metric. Advanced only
// when the metric's telemetry group tick changes (so a repeated cached snapshot
// doesn't over-count), reset on scenario/scale/lattice change or tick regression
// — mirroring the web diagnostics table's RunningStats.
struct RunningStat {
    double mn = std::numeric_limits<double>::infinity();
    double mx = -std::numeric_limits<double>::infinity();
    double sum = 0.0;
    long long count = 0;
    void push(double v) {
        if (!std::isfinite(v)) return;
        if (v < mn) mn = v;
        if (v > mx) mx = v;
        sum += v; ++count;
    }
    double avg() const { return count ? sum / static_cast<double>(count) : 0.0; }
};

// The live engine reductions a Diagnostics row extractor reads (from the
// Scale-0 snapshot: telemetry.diagnostics / telemetry.audit / energy_ledger).
struct DiagInputs {
    const ftd::Diagnostics& dg;
    const ftd::EnergyAudit& au;
    const ftd::EnergyLedger& led;
};

// Number of descriptor rows (kDiagMetrics). The stats vector is sized to this.
std::size_t diag_metric_count();
// Build the Diagnostics section+row skeletons (labels/units/structure) once when
// the panel becomes active; leaves diag_sections EMPTY when !diag_active.
void build_instrument_sections(ShellData* data);
// Accumulate the running stats for one boundary. diag_adv / audit_adv say whether
// each group's tick advanced this boundary; a metric is pushed only when its own
// group advanced. Resizes `stats` to diag_metric_count() on first use.
void accumulate_diag_stats(std::vector<RunningStat>& stats, const DiagInputs& in,
                           bool diag_adv, bool audit_adv);
// Clear every accumulator (used at scenario/scale/lattice reset).
void reset_diag_stats(std::vector<RunningStat>& stats);
// Refresh the live Value + Min/Max/Avg strings of the built rows from the snapshot
// + stats (change-guarded). Returns true if any string changed (caller dirties
// "diag_sections" once). No-op when diag_sections is empty.
bool refresh_instrument_sections(ShellData* data, const DiagInputs& in,
                                 const std::vector<RunningStat>& stats);

}  // namespace ftd::native::app
