#include "native/parameter_journal.h"
#include "native/command_applier.h"

#include "ftd/render_bridge.h"
#include "ftd/term_toggles.h"

#include <utility>

namespace ftd::native {
namespace {

JValue make_bool(bool v) {
    JValue out;
    out.kind = JKind::Bool;
    out.b = v;
    return out;
}
JValue make_double(double v) {
    JValue out;
    out.kind = JKind::Double;
    out.d = v;
    return out;
}
JValue make_uint(unsigned v) {
    JValue out;
    out.kind = JKind::UInt;
    out.u = v;
    return out;
}
JValue make_enum(int v) {
    JValue out;
    out.kind = JKind::Enum;
    out.e = v;
    return out;
}
JValue make_boundary(ftd::FluxBoundaryMode mode) {
    JValue out;
    out.kind = JKind::Boundary;
    out.e = static_cast<int>(mode);
    return out;
}
JValue make_scenario(std::string id) {
    JValue out;
    out.kind = JKind::ScenarioId;
    out.s = std::move(id);
    return out;
}

UiCommand command_from_entry(const JournalEntry& entry) {
    const std::string& key = entry.key;
    const JValue& req = entry.requested;
    if (key == "bridge.dt") return SetDt{req.d};
    if (key == "bridge.sor_iterations") return SetSorIterations{req.e};
    if (key == "bridge.genesis_threshold_override") {
        return SetDouble{DoubleKey::genesis_threshold_override, req.d};
    }
    if (key == "bridge.manifest_scale_override") {
        return SetDouble{DoubleKey::manifest_scale_override, req.d};
    }
    if (key == "bridge.manifest_use_temperature") {
        return SetBoolConfig{BoolCfgKey::manifest_use_temperature, req.b};
    }
    if (key == "bridge.lattice_size") return SetLatticeSize{req.e};
    if (key == "run.staged_lattice_size") return SetLatticeSize{req.e};
    if (key == "run.scenario") return LoadScenario{req.s};
    if (key == "toggles.flux_boundary") {
        return SetBoundary{static_cast<ftd::FluxBoundaryMode>(req.e)};
    }
    if (key == "toggles.bcc_stencil") {
        return SetEnum{EnumKey::bcc_stencil, req.e};
    }
    if (key == "toggles.langevin_site_filter") {
        return SetEnum{EnumKey::langevin_site_filter, req.e};
    }
    if (key == "toggles.langevin_T") {
        return SetDouble{DoubleKey::langevin_T, req.d};
    }
    if (key == "toggles.langevin_gamma") {
        return SetDouble{DoubleKey::langevin_gamma, req.d};
    }
    if (key == "toggles.langevin_seed") {
        return SetUInt{UIntKey::langevin_seed, req.u};
    }
    if (key == "toggles.coulomb_charge_coupling") {
        return SetDouble{DoubleKey::coulomb_charge_coupling, req.d};
    }
    if (key == "toggles.coulomb_source_scale") {
        return SetDouble{DoubleKey::coulomb_source_scale, req.d};
    }
    if (key == "toggles.omega0") return SetDouble{DoubleKey::omega0, req.d};
    if (key == "toggles.kinetic_drain") {
        return SetDouble{DoubleKey::kinetic_drain, req.d};
    }
    const char prefix[] = "toggles.";
    if (key.rfind(prefix, 0) == 0) {
        return SetToggle{key.substr(sizeof(prefix) - 1), req.b};
    }
    return ResetToDefaults{};
}

}  // namespace

void ParameterJournal::append(JournalEntry entry) {
    entries_.push_back(std::move(entry));
}

void ParameterJournal::clear() { entries_.clear(); }

JValue read_journal_key(const RenderBridge& bridge, const std::string& key) {
    if (key == "bridge.dt") return make_double(bridge.dt());
    if (key == "bridge.sor_iterations") return make_enum(bridge.sor_iterations());
    if (key == "bridge.genesis_threshold_override") {
        return make_double(bridge.genesis_threshold_override);
    }
    if (key == "bridge.manifest_scale_override") {
        return make_double(bridge.manifest_scale_override);
    }
    if (key == "bridge.manifest_use_temperature") {
        return make_bool(bridge.manifest_use_temperature);
    }
    if (key == "bridge.lattice_size") return make_enum(bridge.lattice().size());
    if (key == "toggles.flux_boundary") return make_boundary(bridge.toggles.flux_boundary);
    if (key == "toggles.bcc_stencil") {
        return make_enum(static_cast<int>(bridge.toggles.bcc_stencil));
    }
    if (key == "toggles.langevin_site_filter") {
        return make_enum(static_cast<int>(bridge.toggles.langevin_site_filter));
    }
    if (key == "toggles.langevin_T") return make_double(bridge.toggles.langevin_T);
    if (key == "toggles.langevin_gamma") {
        return make_double(bridge.toggles.langevin_gamma);
    }
    if (key == "toggles.langevin_seed") return make_uint(bridge.toggles.langevin_seed);
    if (key == "toggles.coulomb_charge_coupling") {
        return make_double(bridge.toggles.coulomb_charge_coupling);
    }
    if (key == "toggles.coulomb_source_scale") {
        return make_double(bridge.toggles.coulomb_source_scale);
    }
    if (key == "toggles.omega0") return make_double(bridge.toggles.omega0);
    if (key == "toggles.kinetic_drain") return make_double(bridge.toggles.kinetic_drain);
    const char prefix[] = "toggles.";
    if (key.rfind(prefix, 0) == 0) {
        const auto* spec = ftd::term_toggles_detail::find_spec(key.substr(sizeof(prefix) - 1));
        if (spec) return make_bool(bridge.toggles.*(spec->field));
    }
    return {};
}

bool same_term_toggles(const ftd::TermToggles& a, const ftd::TermToggles& b) {
    for (const auto& spec : ftd::TOGGLE_SPECS) {
        if (a.*(spec.field) != b.*(spec.field)) return false;
    }
    return a.bcc_stencil == b.bcc_stencil
        && a.langevin_site_filter == b.langevin_site_filter
        && a.langevin_T == b.langevin_T
        && a.langevin_gamma == b.langevin_gamma
        && a.langevin_seed == b.langevin_seed
        && a.coulomb_charge_coupling == b.coulomb_charge_coupling
        && a.coulomb_source_scale == b.coulomb_source_scale
        && a.omega0 == b.omega0
        && a.kinetic_drain == b.kinetic_drain
        && a.flux_boundary == b.flux_boundary;
}

bool same_bridge_knobs(const RenderBridge& a, const RenderBridge& b) {
    return a.lattice().size() == b.lattice().size()
        && a.dt() == b.dt()
        && a.sor_iterations() == b.sor_iterations()
        && a.genesis_threshold_override == b.genesis_threshold_override
        && a.manifest_scale_override == b.manifest_scale_override
        && a.manifest_use_temperature == b.manifest_use_temperature;
}

void ParameterJournal::replay_requests(RenderBridge& bridge,
                                       NativeEngineSession* session) {
    ParameterJournal ignored;
    for (const auto& entry : entries_) {
        if (entry.key.rfind("harness.", 0) == 0) continue;
        QueuedCommand item;
        item.command = command_from_entry(entry);
        LoopControl ignored_loop;
        apply_mutation_on_bridge(bridge, session, item, ignored, entry.tick_applied,
                                 ignored_loop);
    }
}

}  // namespace ftd::native
