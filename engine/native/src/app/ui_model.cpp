// app/ui_model.cpp — builders for the RmlUi data model (see app/ui_model.h).

#include "app/ui_model.h"

#include "app/app_util.h"             // to_lower (scenario filter)
#include "native/scale0_overlays.h"   // overlay registry (build_overlay_columns)
#include "native/scenario_catalog.h"  // SCENARIO_META (rebuild_scenario_view)

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <string_view>

namespace ftd::native::app {

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

namespace {
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
}  // namespace

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

// The static requires/conflicts/gpu-only metadata for one TOGGLE_SPECS row,
// formatted as the discoverable subtitle shown under the toggle label.
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
// violates a requires/conflicts rule involving it.
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

// Rebuild the toggle-category headers. Items are built only for expanded
// categories (DOM-shrink). Called on a STRUCTURAL change, never per frame.
void rebuild_toggle_groups(ShellData* data, const ftd::TermToggles& tt,
                           const std::vector<std::string>& expanded) {
    if (!data) return;
    data->toggle_groups.clear();
    if (!data->phys_open) return;  // closed section => no headers, no rows
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
// truth (TermToggles + the published bridge knobs).
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

// Build the config-knob rows (one per ConfigSpec), seeded from live engine truth.
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
// collect each column's registry rows. Empty columns are omitted.
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
        row.count = std::to_string(row.items.size());  // section-head badge tally
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

namespace {
// The leading "[TAG]" of an epistemic_status string ("[EMERGENT] ..." -> "EMERGENT").
// Empty when there is no leading bracket.
Rml::String leading_tag(const char* status) {
    if (!status || status[0] != '[') return {};
    const char* end = std::strchr(status, ']');
    if (!end || end <= status + 1) return {};
    return Rml::String(status + 1, static_cast<std::size_t>(end - status - 1));
}
// Colour class for a status tag: 1 derived/emergent, 2 imposed/selection,
// 3 conjecture/open, 4 negative, 0 neutral. "NEGATIVE" wins (CLOSED/QUALIFIED
// NEGATIVE are negatives regardless of any other word present).
int classify_tag(const Rml::String& t) {
    if (t.empty()) return 0;
    auto has = [&](const char* s) { return t.find(s) != Rml::String::npos; };
    if (has("NEGATIVE")) return 4;
    if (has("THEOREM") || has("DERIVED") || has("EMERGENT")) return 1;
    if (has("IMPOSED") || has("SELECTION") || has("AXIOM") || has("PARAMETRIC")) return 2;
    if (has("CONJECTURE") || has("OPEN") || has("QUALIFIED")) return 3;
    return 0;
}
}  // namespace

// Rebuild the Setup picker view from the native scenario catalog, honoring the
// live search filter and the per-group expanded set (DOM-shrink: only an open or
// filter-matched group instantiates its item rows).
void rebuild_scenario_view(ShellData* data, const std::string& current,
                           const std::string& raw_filter,
                           const std::vector<std::string>& expanded) {
    if (!data) return;
    data->scenario_groups.clear();
    const std::string f = to_lower(raw_filter);
    const bool filtering = !f.empty();
    for (const char* cat : kScenarioCategories) {
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
                r.tag = leading_tag(m.epistemic_status);
                r.tag_cls = classify_tag(r.tag);
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

// Fill the epistemic-status box from scenario `id`'s catalog metadata. Splits
// epistemic_status into the leading [TAG] (bare, matching the picker badge), its
// colour class, and the qualification clause after the ']'. Clears every field
// (hiding the box) when the id has no Scale-0 catalog entry (Scale-1 / unknown).
bool fill_epistemic(ShellData* data, const std::string& id) {
    if (!data) return false;
    Rml::String tag, body, title;
    int cls = 0;
    if (const ftd::native::ScenarioMeta* m = ftd::native::find_scenario_meta(id)) {
        tag = leading_tag(m->epistemic_status);
        cls = classify_tag(tag);
        title = m->title ? m->title : "";
        if (m->epistemic_status) {
            const char* end = std::strchr(m->epistemic_status, ']');
            const char* rest = end ? end + 1 : m->epistemic_status;
            while (*rest == ' ') ++rest;   // trim the space after ']'
            body = rest;
        }
    }
    bool changed = false;
    if (data->epi_tag != tag)     { data->epi_tag = std::move(tag);     changed = true; }
    if (data->epi_cls != cls)     { data->epi_cls = cls;                changed = true; }
    if (data->epi_body != body)   { data->epi_body = std::move(body);   changed = true; }
    if (data->epi_title != title) { data->epi_title = std::move(title); changed = true; }
    return changed;
}

// ═══ Diagnostics-panel instrument tables ══════════════════════════════════════
namespace {
enum class DiagGroup { Diagnostics, Audit };
enum class DiagFmt { Scalar, Int, Pair, Triple, Vector };
// One metric descriptor. a/b/c are captureless extractors (decay to function
// pointers). group selects which telemetry-group tick gates its stat sampling.
struct DiagMetric {
    int section;
    const char* label;
    const char* unit;
    DiagGroup group;
    DiagFmt fmt;
    int variant;
    bool has_stat;   // false for pair/triple/vector rows (no Min/Max/Avg)
    double (*a)(const DiagInputs&);
    double (*b)(const DiagInputs&);
    double (*c)(const DiagInputs&);
};
constexpr const char* kDiagSectionTitles[] = {
    "Particle State", "Energy Budget", "Electromagnetic", "Constraints", "Dual Substrate",
};
// ORDERED BY SECTION so a single running index aligns build order with refresh
// order (the parallel-index invariant refresh_instrument_sections relies on).
const DiagMetric kDiagMetrics[] = {
    // ── 0. Particle State (group Diagnostics) ──
    {0, "Manifested",   "ct", DiagGroup::Diagnostics, DiagFmt::Int, 0, true,
        [](const DiagInputs& in){ return (double)in.dg.manifested_count; }, nullptr, nullptr},
    {0, "Positive",     "ct", DiagGroup::Diagnostics, DiagFmt::Int, 1, true,
        [](const DiagInputs& in){ return (double)in.dg.positive_count; }, nullptr, nullptr},
    {0, "Negative",     "ct", DiagGroup::Diagnostics, DiagFmt::Int, 2, true,
        [](const DiagInputs& in){ return (double)in.dg.negative_count; }, nullptr, nullptr},
    {0, "Charge (net)", "ct", DiagGroup::Diagnostics, DiagFmt::Int, 0, true,
        [](const DiagInputs& in){ return (double)(in.dg.positive_count - in.dg.negative_count); }, nullptr, nullptr},
    {0, "Spin Up/Down", "ct", DiagGroup::Diagnostics, DiagFmt::Pair, 0, false,
        [](const DiagInputs& in){ return (double)in.dg.spin_up_count; },
        [](const DiagInputs& in){ return (double)in.dg.spin_down_count; }, nullptr},
    {0, "Color R/G/B",  "ct", DiagGroup::Diagnostics, DiagFmt::Triple, 0, false,
        [](const DiagInputs& in){ return (double)in.dg.color_count[1]; },
        [](const DiagInputs& in){ return (double)in.dg.color_count[2]; },
        [](const DiagInputs& in){ return (double)in.dg.color_count[3]; }},
    {0, "Colorless",    "ct", DiagGroup::Diagnostics, DiagFmt::Int, 0, true,
        [](const DiagInputs& in){ return (double)in.dg.color_count[0]; }, nullptr, nullptr},
    // ── 1. Energy Budget ──
    {1, "Dynamic Energy",   "E*", DiagGroup::Audit,       DiagFmt::Scalar, 0, true,
        [](const DiagInputs& in){ return in.au.dynamic_energy; }, nullptr, nullptr},
    {1, "Accounted Energy", "E*", DiagGroup::Audit,       DiagFmt::Scalar, 0, true,
        [](const DiagInputs& in){ return in.au.total_energy; }, nullptr, nullptr},
    {1, "Particle Rest E",  "E*", DiagGroup::Audit,       DiagFmt::Scalar, 0, true,
        [](const DiagInputs& in){ return in.au.particle_rest_energy; }, nullptr, nullptr},
    {1, "Energy Drift",     "%",  DiagGroup::Diagnostics, DiagFmt::Scalar, 0, true,
        [](const DiagInputs& in){ return in.led.drift_frac * 100.0; }, nullptr, nullptr},
    {1, "Field |J|²",  "E*", DiagGroup::Audit,       DiagFmt::Scalar, 0, true,
        [](const DiagInputs& in){ return in.au.field_energy; }, nullptr, nullptr},
    {1, "Wave |w|²",   "E*", DiagGroup::Audit,       DiagFmt::Scalar, 0, true,
        [](const DiagInputs& in){ return in.au.wave_energy; }, nullptr, nullptr},
    {1, "Particle KE",      "E*", DiagGroup::Audit,       DiagFmt::Scalar, 0, true,
        [](const DiagInputs& in){ return in.au.particle_ke; }, nullptr, nullptr},
    {1, "Coulomb PE",       "E*", DiagGroup::Audit,       DiagFmt::Scalar, 0, true,
        [](const DiagInputs& in){ return in.au.coulomb_pe; }, nullptr, nullptr},
    {1, "Total Flux",       "|J|",DiagGroup::Diagnostics, DiagFmt::Scalar, 0, true,
        [](const DiagInputs& in){ return in.dg.total_flux; }, nullptr, nullptr},
    {1, "Entropy",          "nat",DiagGroup::Diagnostics, DiagFmt::Scalar, 0, true,
        [](const DiagInputs& in){ return in.dg.total_entropy; }, nullptr, nullptr},
    // ── 2. Electromagnetic ──
    {2, "E-Field ½|E|²", "E*", DiagGroup::Audit,       DiagFmt::Scalar, 0, true,
        [](const DiagInputs& in){ return in.au.E_field_energy; }, nullptr, nullptr},
    {2, "B-Field",          "E*", DiagGroup::Audit,       DiagFmt::Scalar, 0, true,
        [](const DiagInputs& in){ return in.au.B_field_energy; }, nullptr, nullptr},
    {2, "Poynting |S|",     "S",  DiagGroup::Audit,       DiagFmt::Scalar, 0, true,
        [](const DiagInputs& in){ const auto& p = in.au.total_poynting;
            return std::sqrt(p.x*p.x + p.y*p.y + p.z*p.z); }, nullptr, nullptr},
    {2, "Angular Mom",      "L",  DiagGroup::Diagnostics, DiagFmt::Vector, 0, false,
        [](const DiagInputs& in){ return (double)in.dg.total_angular_momentum.x; },
        [](const DiagInputs& in){ return (double)in.dg.total_angular_momentum.y; },
        [](const DiagInputs& in){ return (double)in.dg.total_angular_momentum.z; }},
    // ── 3. Constraints (group Audit) ──
    {3, "Gauss Σ(divJ−s)²", "", DiagGroup::Audit, DiagFmt::Scalar, 0, true,
        [](const DiagInputs& in){ return in.au.gauss_violation; }, nullptr, nullptr},
    {3, "Max Gauss err",    "",   DiagGroup::Audit, DiagFmt::Scalar, 0, true,
        [](const DiagInputs& in){ return in.au.max_gauss_error; }, nullptr, nullptr},
    {3, "Self-field inj",   "E*", DiagGroup::Audit, DiagFmt::Scalar, 0, true,
        [](const DiagInputs& in){ return in.au.self_field_injection; }, nullptr, nullptr},
    // ── 4. Dual Substrate (group Audit) ──
    {4, "E_L",       "E*", DiagGroup::Audit, DiagFmt::Scalar, 0, true,
        [](const DiagInputs& in){ return in.au.E_L_total; }, nullptr, nullptr},
    {4, "E_R",       "E*", DiagGroup::Audit, DiagFmt::Scalar, 0, true,
        [](const DiagInputs& in){ return in.au.E_R_total; }, nullptr, nullptr},
    {4, "Chirality", "χ", DiagGroup::Audit, DiagFmt::Scalar, 0, true,
        [](const DiagInputs& in){ return in.au.chirality_total; }, nullptr, nullptr},
    {4, "Wave L/R",  "E*", DiagGroup::Audit, DiagFmt::Pair, 0, false,
        [](const DiagInputs& in){ return in.au.wv_L_total; },
        [](const DiagInputs& in){ return in.au.wv_R_total; }, nullptr},
};
constexpr std::size_t kN = sizeof(kDiagMetrics) / sizeof(kDiagMetrics[0]);

// Value formatter (mirrors the web formatters.js): ints exact; 0 → "0"; |v|≥1e4
// or <1e-3 → "%.2e"; else "%.6g".
Rml::String diag_fmt(double v, DiagFmt fmt) {
    if (fmt == DiagFmt::Int) return std::to_string(static_cast<long long>(std::llround(v)));
    if (v == 0.0) return "0";
    char buf[40];
    const double a = std::fabs(v);
    if (a >= 1e4 || a < 1e-3) std::snprintf(buf, sizeof(buf), "%.2e", v);
    else                      std::snprintf(buf, sizeof(buf), "%.6g", v);
    return Rml::String(buf);
}
Rml::String format_metric(const DiagMetric& m, const DiagInputs& in) {
    switch (m.fmt) {
        case DiagFmt::Int:    return diag_fmt(m.a(in), DiagFmt::Int);
        case DiagFmt::Scalar: return diag_fmt(m.a(in), DiagFmt::Scalar);
        case DiagFmt::Pair:   return diag_fmt(m.a(in), DiagFmt::Int) + " / " + diag_fmt(m.b(in), DiagFmt::Int);
        case DiagFmt::Triple: return diag_fmt(m.a(in), DiagFmt::Int) + " / " + diag_fmt(m.b(in), DiagFmt::Int)
                                     + " / " + diag_fmt(m.c(in), DiagFmt::Int);
        case DiagFmt::Vector: return diag_fmt(m.a(in), DiagFmt::Scalar) + ", " + diag_fmt(m.b(in), DiagFmt::Scalar)
                                     + ", " + diag_fmt(m.c(in), DiagFmt::Scalar);
    }
    return "0";
}
}  // namespace

std::size_t diag_metric_count() { return kN; }

void build_instrument_sections(ShellData* data) {
    if (!data) return;
    data->diag_sections.clear();
    if (!data->diag_active) return;   // DOM-shrink: no rows unless the panel is active
    const int nsec = static_cast<int>(sizeof(kDiagSectionTitles) / sizeof(kDiagSectionTitles[0]));
    for (int s = 0; s < nsec; ++s) {
        InstrumentSection sec;
        sec.title = kDiagSectionTitles[s];
        sec.expanded = true;
        for (const DiagMetric& m : kDiagMetrics) {
            if (m.section != s) continue;
            InstrumentRow row;
            row.label = m.label;
            row.unit = (m.unit && m.unit[0]) ? Rml::String(m.unit) : Rml::String("—");
            row.variant = m.variant;
            row.has_stat = m.has_stat;
            if (!m.has_stat) { row.vmin = "—"; row.vmax = "—"; row.vavg = "—"; }
            sec.rows.push_back(std::move(row));
        }
        data->diag_sections.push_back(std::move(sec));
    }
}

void reset_diag_stats(std::vector<RunningStat>& stats) {
    for (RunningStat& s : stats) s = RunningStat{};
}

void accumulate_diag_stats(std::vector<RunningStat>& stats, const DiagInputs& in,
                           bool diag_adv, bool audit_adv) {
    if (stats.size() != kN) stats.assign(kN, RunningStat{});
    for (std::size_t i = 0; i < kN; ++i) {
        const DiagMetric& m = kDiagMetrics[i];
        if (!m.has_stat) continue;
        const bool adv = (m.group == DiagGroup::Audit) ? audit_adv : diag_adv;
        if (adv) stats[i].push(m.a(in));
    }
}

bool refresh_instrument_sections(ShellData* data, const DiagInputs& in,
                                 const std::vector<RunningStat>& stats) {
    if (!data || data->diag_sections.empty()) return false;
    bool changed = false;
    std::size_t idx = 0;   // parallel to kDiagMetrics (section-ordered — see build)
    for (InstrumentSection& sec : data->diag_sections) {
        for (InstrumentRow& row : sec.rows) {
            if (idx >= kN) break;
            const DiagMetric& m = kDiagMetrics[idx];
            Rml::String val = format_metric(m, in);
            if (row.value != val) { row.value = std::move(val); changed = true; }
            if (m.has_stat && idx < stats.size() && stats[idx].count > 0) {
                Rml::String mn = diag_fmt(stats[idx].mn, DiagFmt::Scalar);
                Rml::String mx = diag_fmt(stats[idx].mx, DiagFmt::Scalar);
                Rml::String av = diag_fmt(stats[idx].avg(), DiagFmt::Scalar);
                if (row.vmin != mn) { row.vmin = std::move(mn); changed = true; }
                if (row.vmax != mx) { row.vmax = std::move(mx); changed = true; }
                if (row.vavg != av) { row.vavg = std::move(av); changed = true; }
            }
            ++idx;
        }
    }
    return changed;
}

}  // namespace ftd::native::app
