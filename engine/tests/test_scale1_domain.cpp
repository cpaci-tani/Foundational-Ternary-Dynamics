#include "ftd/particle_engine.h"
#include "ftd/scale1/domain.h"

#include <cmath>
#include <iostream>
#include <set>
#include <string>

namespace {
int failures = 0;

void check(const char* label, bool condition) {
    std::cout << (condition ? "  PASS  " : "  FAIL  ") << label << '\n';
    if (!condition) ++failures;
}
}

int main() {
    using namespace ftd;
    std::cout << "== Scale-1 shared domain contract ==\n";

    {
        const auto& registry = scale1_physics_registry();
        std::set<std::string> ids;
        std::set<std::string> toggles;
        bool validation_records_complete = true;
        bool state_matches_availability = true;
        const char* expected_toggle_order[] = {
            "coulomb", "gravity", "damping", "lorentz", "exchange", "strong",
            "radiation", "spin_orbit", "relativistic", "magnetic_dipole",
            "relativistic_verlet", "contact_events",
        };
        bool toggle_order_pinned = registry.size() == 12;
        std::size_t index = 0;
        for (const auto& spec : registry) {
            ids.insert(spec.id);
            toggles.insert(spec.toggle_name);
            validation_records_complete = validation_records_complete
                && spec.validation_evidence && *spec.validation_evidence
                && spec.validation_criterion && *spec.validation_criterion;
            state_matches_availability = state_matches_availability
                && (spec.available
                    ? spec.validation_state != Scale1ValidationState::InvalidRetired
                    : spec.validation_state == Scale1ValidationState::InvalidRetired);
            if (index >= 12 || std::string(spec.toggle_name) != expected_toggle_order[index]) {
                toggle_order_pinned = false;
            }
            ++index;
        }
        check("physics registry has one row per toggle key", registry.size() == 12);
        check("physics ids are unique", ids.size() == registry.size());
        check("toggle names are unique", toggles.size() == registry.size());
        check("physics-mask bit order is pinned to the toggle registry", toggle_order_pinned);
        check("every physics module has evidence and a pass criterion",
              validation_records_complete);
        check("physics validation verdict matches module availability",
              state_matches_availability);

        const auto* retired = find_scale1_physics_spec("relativistic");
        check("isotropic rescale is retired and unavailable",
              retired && retired->tier == Scale1ModuleTier::Retired
              && retired->status == Scale1EpistemicStatus::Invalid
              && !retired->available
              && retired->validation_state == Scale1ValidationState::InvalidRetired);
    }

    {
        const auto& scenarios = scale1_scenario_registry();
        std::set<std::string> ids;
        std::set<std::string> workspaces;
        int effective = 0;
        int qed_rows = 0;
        int quantum_rows = 0;
        bool all_available = true;
        bool validation_records_complete = true;
        bool masks_valid = true;
        bool verdicts_runnable = true;
        constexpr std::uint32_t all_physics_bits = (1u << 12) - 1u;
        constexpr std::uint32_t retired_rescale_bit = 1u << 8;
        for (const auto& spec : scenarios) {
            ids.insert(spec.id);
            workspaces.insert(scale1_workspace_id(spec.workspace));
            all_available = all_available && spec.available;
            if (spec.available
                && spec.scenario_class == Scale1ScenarioClass::EffectiveReference) {
                ++effective;
            }
            if (spec.workspace == Scale1Workspace::QedReference) {
                ++qed_rows;
            }
            if (spec.workspace == Scale1Workspace::QuantumReference) {
                ++quantum_rows;
            }
            validation_records_complete = validation_records_complete
                && spec.validation_evidence && *spec.validation_evidence
                && spec.validation_criterion && *spec.validation_criterion;
            masks_valid = masks_valid
                && (spec.physics_mask & ~all_physics_bits) == 0
                && (spec.physics_mask & retired_rescale_bit) == 0;
            verdicts_runnable = verdicts_runnable
                && spec.validation_state != Scale1ValidationState::OpenBlocked
                && spec.validation_state != Scale1ValidationState::InvalidRetired;

            for (std::size_t bit = 0; bit < scale1_physics_registry().size(); ++bit) {
                if ((spec.physics_mask & (1u << bit)) == 0) continue;
                const auto& physics = scale1_physics_registry()[bit];
                masks_valid = masks_valid && physics.available
                    && (physics.backend_mask & spec.backend_mask) == spec.backend_mask;
            }
        }
        check("particle-scale program has 36 runnable scenarios",
              scenarios.size() == 36);
        check("scenario ids are unique", ids.size() == scenarios.size());
        check("all four live particle workspaces are represented", workspaces.size() == 4);
        check("the live registry contains no disabled scenarios", all_available);
        check("thirty-two runnable effective reference scenarios are explicit", effective == 32);
        check("QED workspace contains seven runnable forms", qed_rows == 7);
        check("quantum workspace contains twelve runnable controls", quantum_rows == 12);
        check("all 36 scenarios carry qualification evidence and criteria",
              validation_records_complete);
        check("scenario physics profiles contain only available backend-qualified modules",
              masks_valid);
        check("every registered scenario has a runnable validation verdict",
              verdicts_runnable);
        const auto* native = find_scale1_scenario_spec("s1-native-m3-replay");
        check("registered M3 replay is qualified, read-only evidence",
              native && native->scenario_class == Scale1ScenarioClass::QualifiedReplay
              && native->available && !native->interactive
              && native->physics_mask == 0
              && native->behavior == Scale1ScenarioBehavior::ReadOnlyReplay
              && native->validation_state == Scale1ValidationState::ContractQualified);
        check("duplicate M3 replay pseudo-scenarios are consolidated",
              find_scale1_scenario_spec("s1-constituent-graph") == nullptr
              && find_scale1_scenario_spec("s1-field-decomposition") == nullptr
              && find_scale1_scenario_spec("s1-center-observers") == nullptr
              && find_scale1_scenario_spec("s1-identity-margins") == nullptr
              && find_scale1_scenario_spec("s1-coverage-ledger") == nullptr);
        check("retired scale-handoff scenarios are absent",
              find_scale1_scenario_spec("s1-promoted-lattice") == nullptr
              && find_scale1_scenario_spec("s1-voxel-debug") == nullptr
              && find_scale1_scenario_spec("s1-scale2-handoff") == nullptr);

        const auto* qed_static = find_scale1_scenario_spec("s1-qed-static-coulomb");
        const auto* qed_magnetic = find_scale1_scenario_spec("s1-qed-magnetic-dipole");
        check("QED references retain effective/imposed ownership and exact profiles",
              qed_static && qed_static->available
              && qed_static->workspace == Scale1Workspace::QedReference
              && qed_static->status == Scale1EpistemicStatus::Imposed
              && qed_static->physics_mask == ((1u << 0) | (1u << 10))
              && qed_magnetic && qed_magnetic->available
              && qed_magnetic->physics_mask == ((1u << 9) | (1u << 10)));
        check("static Coulomb reference declares locked-field behavior",
              qed_static && qed_static->behavior == Scale1ScenarioBehavior::StaticField
              && !qed_static->interactive);
        check("disabled QED placeholders are absent from the live registry",
              find_scale1_scenario_spec("s1-qed-annihilation-boundary") == nullptr
              && find_scale1_scenario_spec("s1-qed-loop-observables-boundary") == nullptr);

        const auto* quantum_exchange = find_scale1_scenario_spec("s1-quantum-exchange-eligible");
        const auto* quantum_null = find_scale1_scenario_spec("s1-quantum-exchange-spinless-control");
        const auto* quantum_spin_orbit = find_scale1_scenario_spec("s1-quantum-spin-orbit-parallel");
        const auto* quantum_radiation = find_scale1_scenario_spec("s1-quantum-radiation-scattering");
        const auto* quantum_color = find_scale1_scenario_spec("s1-quantum-color-triplet");
        check("quantum reference controls retain imposed ownership and exact profiles",
              quantum_exchange && quantum_exchange->available
              && quantum_exchange->workspace == Scale1Workspace::QuantumReference
              && quantum_exchange->status == Scale1EpistemicStatus::Imposed
              && quantum_exchange->physics_mask == ((1u << 4) | (1u << 10))
              && quantum_spin_orbit
              && quantum_spin_orbit->physics_mask == ((1u << 7) | (1u << 10))
              && quantum_radiation
              && quantum_radiation->physics_mask == ((1u << 0) | (1u << 6) | (1u << 10))
              && quantum_color
              && quantum_color->physics_mask == ((1u << 5) | (1u << 10)));
        check("exchange A/B pair and null expectation are native-owned",
              quantum_exchange && quantum_null
              && std::string(quantum_exchange->paired_scenario_id) == quantum_null->id
              && std::string(quantum_null->paired_scenario_id) == quantum_exchange->id
              && quantum_null->behavior == Scale1ScenarioBehavior::NullControl);

        const auto* empty_zoo = find_scale1_scenario_spec("s1-empty-zoo");
        const auto* species = find_scale1_scenario_spec("s1-parametric-species");
        const auto* ladder = find_scale1_scenario_spec("s1-mass-ladder");
        check("catalog scenarios declare waiting versus static-reference behavior",
              empty_zoo && empty_zoo->behavior == Scale1ScenarioBehavior::AwaitingInput
              && empty_zoo->interactive
              && species && species->behavior == Scale1ScenarioBehavior::StaticReference
              && !species->interactive
              && ladder && ladder->behavior == Scale1ScenarioBehavior::StaticReference
              && !ladder->interactive);

        const auto* gravity_pair = find_scale1_scenario_spec("s1-cluster-pair");
        const auto* open_battery = find_scale1_scenario_spec("s1-open-terminal-battery");
        const auto* damping = find_scale1_scenario_spec("s1-damping-sink");
        const auto* exchange = find_scale1_scenario_spec("s1-advanced-force-isolation");
        const auto* contact = find_scale1_scenario_spec("s1-contact-selection");
        check("effective scenarios carry exact native-owned physics profiles",
              gravity_pair && gravity_pair->physics_mask == ((1u << 0) | (1u << 1) | (1u << 10))
              && open_battery
              && open_battery->physics_mask == ((1u << 0) | (1u << 10))
              && open_battery->behavior == Scale1ScenarioBehavior::Dynamic
              && damping && damping->physics_mask == ((1u << 2) | (1u << 10))
              && exchange && exchange->physics_mask == ((1u << 4) | (1u << 10))
              && contact && contact->physics_mask == ((1u << 0) | (1u << 10) | (1u << 11)));
    }

    Scale1SourceClusterRecord source;
    source.source_object_id = 17;
    source.source_tick = 91;
    source.source_scenario = "test-source";
    source.lattice_size = 33;
    source.manifestation_count = 27;
    source.state_sign = +1;
    source.centroid = {17.0, 16.0, 15.0};
    source.centroid_velocity = {0.01, 0.0, 0.0};
    source.display_scale = 2.0;
    source.constituent_relations_available = true;
    source.field_state_available = true;

    {
        auto observed = NativeMatterObserver::observe_source_clusters(
            {source}, source.source_tick, source.source_scenario, "capture-v1");
        check("coherent source observation remains read-only and source-owned",
              observed.core.read_only
              && observed.core.scenario_class == Scale1ScenarioClass::LiveNative
              && observed.core.dynamics_owner == Scale1DynamicsOwner::NativeMatterObserver
              && observed.objects.size() == 1);
        check("live cluster is a candidate, not a qualified particle",
              !observed.objects[0].identity_available
              && observed.objects[0].provenance.qualification
                    == Scale1Qualification::NotEvaluated
              && !observed.objects[0].mass_available);
        check("live observation retains both center charts without fabricated mass",
              observed.objects[0].integer_center_available
              && observed.objects[0].fractional_center_available
              && observed.objects[0].manifestation_support_count
                    == source.manifestation_count);
        check("live observation does not fabricate a conservation closure",
              !observed.conservation.state_energy_complete
              && !observed.conservation.drift_eligible);
    }

    {
        auto replay = NativeMatterObserver::m3_registered_replay();
        check("native replay is read-only and Scale-0 sourced",
              replay.core.read_only
              && replay.core.dynamics_owner == Scale1DynamicsOwner::NativeMatterObserver
              && replay.objects.size() == 2
              && replay.objects[0].provenance.source_scale == 0);
        check("native replay exposes identity and both center observers",
              replay.objects[0].identity_available
              && replay.objects[0].integer_center_available
              && replay.objects[0].fractional_center_available
              && replay.objects[0].identity_margin > 0.0);
        check("native replay does not invent recovered mass",
              !replay.objects[0].mass_available && !replay.objects[1].mass_available);
        check("native replay exposes incomplete conservation honestly",
              !replay.conservation.state_energy_complete
              && replay.conservation.missing_mask != 0);
        bool outgoing_unavailable = false;
        for (const auto& field : replay.fields) {
            if (field.channel == Scale1FieldChannel::Outgoing) {
                outgoing_unavailable = !field.available
                    && !field.unavailable_reason.empty();
            }
        }
        check("unrecovered outgoing channel is unavailable, not synthesized",
              outgoing_unavailable);
    }

    {
        ParticleEngine engine;
        const int id = engine.add_particle(+1, {2.0, 0.0, -2.0});
        const auto snapshot = engine.snapshot("effective-test", "cpu");
        check("direct effective record remains explicitly non-native",
              id >= 0 && snapshot.objects.size() == 1
              && snapshot.objects[0].provenance.source_scale == 1);
        check("snapshot carries the current schema and registry",
              snapshot.core.schema_version == SCALE1_SNAPSHOT_SCHEMA_VERSION
              && snapshot.core.registry_revision == SCALE1_REGISTRY_REVISION);
        check("verified baseline has complete state-energy coverage",
              snapshot.conservation.state_energy_complete
              && snapshot.conservation.missing_mask == 0);

        engine.toggles.exchange = true;
        const auto incomplete = engine.snapshot("effective-test", "cpu");
        check("quarantined force invalidates conservation completeness",
              !incomplete.conservation.state_energy_complete
              && (incomplete.conservation.missing_mask
                  & scale1_bit(Scale1Coverage::ExchangePotential)) != 0);
    }

    std::cout << (failures == 0 ? "ALL PASS\n" : "FAILURES: " + std::to_string(failures) + "\n");
    return failures == 0 ? 0 : 1;
}
