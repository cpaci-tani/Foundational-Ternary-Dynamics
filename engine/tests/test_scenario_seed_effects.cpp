#include "ftd/scenarios.h"
#include "support/scenario_seed_receipt.h"
#include <fstream>
#include <iostream>
#include <set>

// Make a legal protocol fixture from the registry rather than silently
// dropping a requested toggle. Dependencies belong to the test fixture;
// production rejects inconsistent recipes without changing them.
static void set_term(ftd::seed::Overrides& values, const std::string& name, bool on, int depth=0) {
    if (depth > 32) throw std::runtime_error("toggle dependency cycle");
    const auto* spec = ftd::term_toggles_detail::find_spec(name);
    if (!spec) throw std::runtime_error("unknown toggle fixture");
    values["protocol." + name] = on;
    if (on) {
        ftd::term_toggles_detail::for_each_csv(spec->requires_, [&](auto dep) { set_term(values, std::string(dep), true, depth+1); });
        ftd::term_toggles_detail::for_each_csv(spec->conflicts, [&](auto dep) { set_term(values, std::string(dep), false, depth+1); });
    } else for (const auto& other : ftd::TOGGLE_SPECS) {
        ftd::term_toggles_detail::for_each_csv(other.requires_, [&](auto dep) {
            if (dep == name) set_term(values, other.name, false, depth+1);
        });
    }
}

int main(int argc, char** argv) {
    std::ofstream manifest;
    if (argc == 3 && std::string(argv[1]) == "--manifest") manifest.open(argv[2]);
    if (manifest) manifest << "{\"schemaVersion\":2,\"size\":17,\"effects\":[";
    bool comma=false;
    int checked=0, failed=0;
    for (auto id_view : ftd::scale0_scenario_ids()) {
        const std::string id(id_view);
        try {
            ftd::RenderBridge base(17); base.force_cpu(); ftd::seed::Context defaults(id);
            if (!ftd::dispatch_scenario_seed(base, defaults)) throw std::runtime_error("missing constructor");
            const auto before = seed_receipt(base,id);
            for (const auto& property : defaults.properties) {
                bool effect=false; double changed=property.value;
                std::vector<double> probes;
                for (const auto& option : property.options) probes.push_back(option.first);
                if (probes.empty()) probes = {property.value + property.step, property.value - property.step,
                    property.value * 2, property.value * .5, 0, 1, property.recommended_max, property.recommended_min,
                    property.minimum};
                std::set<double> tried;
                for (const double value : probes) {
                    if (value == property.value || !tried.insert(value).second || value < property.minimum || value > property.maximum
                        || (property.type != "real" && std::floor(value) != value)) continue;
                    for (int fixture=0; fixture<2 && !effect; ++fixture) try {
                        ftd::seed::Overrides edits{{property.key,value}};
                        if (fixture) {
                            if (property.key.compare(0,9,"protocol.") != 0) continue;
                            for (const auto& term : ftd::TOGGLE_SPECS) if (term.backends & ftd::ToggleBackend::JS)
                                edits["protocol." + std::string(term.name)] = 0;
                            edits["protocol.stencil"] = 0; edits["protocol.bathSites"] = 3;
                            edits["protocol.boundary"] = 0; edits[property.key] = value;
                            if (property.key == "protocol.triad_binding" && value) set_term(edits,"dual_substrate",true);
                            if (property.key == "protocol.cluster_inertia" && value) set_term(edits,"forces",true);
                            if (property.key == "protocol.reflective_boundary" && value) edits["protocol.boundary"]=1;
                            if (property.key == "protocol.bathSites" && value != 3) set_term(edits,"langevin",true);
                            if (property.key == "protocol.stencil" && value) set_term(edits,"wave_propagation",true);
                        }
                        if (property.key.compare(0,9,"protocol.")==0 && ftd::term_toggles_detail::find_spec(property.key.substr(9)))
                            set_term(edits,property.key.substr(9),value != 0);
                        ftd::RenderBridge candidate(17); candidate.force_cpu(); ftd::seed::Context context(id,edits);
                        if (!ftd::dispatch_scenario_seed(candidate,context)) continue;
                        // A toggle may need companion terms to make its fixture
                        // legal. Verify the actual target member independently:
                        // a changed companion cannot hide an inert target binding.
                        if (property.key.compare(0,9,"protocol.") == 0) {
                            const auto* target = ftd::term_toggles_detail::find_spec(property.key.substr(9));
                            if (target && (candidate.toggles.*target->field) != (value != 0)) continue;
                        }
                        // Compare a paired fixture with only the target value
                        // different. Companion requirements cannot earn credit
                        // for an otherwise inert property.
                        auto comparison = before;
                        if (fixture) {
                            auto paired = edits; paired[property.key] = property.value;
                            ftd::RenderBridge control(17); control.force_cpu(); ftd::seed::Context pair_context(id,paired);
                            if (!ftd::dispatch_scenario_seed(control,pair_context)) continue;
                            comparison = seed_receipt(control,id);
                        }
                        if (seed_receipt(candidate,id) != comparison) {effect=true; changed=value; break;}
                    } catch (const std::invalid_argument&) { /* Try another declared legal fixture. */ }
                }
                ++checked;
                if (!effect) {++failed; std::cerr << "NO EFFECT " << id << ' ' << property.key << '\n';}
                if (manifest) {
                    if (comma) manifest << ','; comma=true;
                    manifest << "{\"scenarioId\":" << ftd::seed::quote(id) << ",\"key\":" << ftd::seed::quote(property.key)
                        << ",\"effectVerified\":" << (effect ? "true":"false") << ",\"default\":" << property.value
                        << ",\"testedValue\":" << changed << ",\"test\":\"test_scenario_seed_effects\"}";
                }
            }
        } catch (const std::exception& e) {++failed; std::cerr << id << ": " << e.what() << '\n';}
    }
    if (manifest) manifest << "]}\n";
    std::cout << checked << " property effect bindings checked; " << failed << " failures\n";
    return failed ? 1 : 0;
}
