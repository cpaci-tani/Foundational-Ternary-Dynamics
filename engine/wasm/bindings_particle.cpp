/**
 * @file bindings_particle.cpp
 * @brief Embind bindings for ParticleEngine (Scale 1).
 *
 * Extracted from ftd_wasm.cpp as part of W1-W3. Contains:
 *   - pe_toggle_map (pointer-to-member ParticleToggles lookup)
 *   - Data extraction (get_pe_particle_data, get_pe_diagnostics)
 *   - Injection helpers (pe_add_particle, pe_add_locked_particle)
 *   - Controls (dt, softening, damping, gravity, clear)
 *   - Force diagnostics
 *   - The ParticleEngine class_<> binding
 */

#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include "ftd/particle_engine.h"
#include "ftd/constants.h"
#include "ftd/eft/finite_port_gauss_battery.h"

using namespace emscripten;

static val vec3_value(const ftd::Vec3& v) {
    val out = val::object();
    out.set("x", v.x); out.set("y", v.y); out.set("z", v.z);
    return out;
}

static val provenance_value(const ftd::Scale1Provenance& p) {
    val out = val::object();
    out.set("sourceScale", p.source_scale);
    out.set("sourceTick", static_cast<double>(p.source_tick));
    out.set("sourceObjectId", p.source_object_id);
    out.set("sourceKind", p.source_kind);
    out.set("sourceScenario", p.source_scenario);
    out.set("sourceRevision", p.source_revision);
    out.set("status", ftd::scale1_status_id(p.status));
    out.set("qualification", ftd::scale1_qualification_id(p.qualification));
    return out;
}

static val scale1_snapshot_value(const ftd::Scale1Snapshot& snapshot) {
    val out = val::object();
    val core = val::object();
    core.set("schemaVersion", snapshot.core.schema_version);
    core.set("registryRevision", snapshot.core.registry_revision);
    core.set("tick", static_cast<double>(snapshot.core.tick));
    core.set("effectiveDt", snapshot.core.effective_dt);
    core.set("mode", ftd::scale1_mode_id(snapshot.core.mode));
    core.set("workspace", ftd::scale1_workspace_id(snapshot.core.workspace));
    core.set("scenarioClass", ftd::scale1_scenario_class_id(snapshot.core.scenario_class));
    core.set("dynamicsOwner", ftd::scale1_owner_id(snapshot.core.dynamics_owner));
    core.set("backend", snapshot.core.backend);
    core.set("scenario", snapshot.core.scenario);
    core.set("sourceRevision", snapshot.core.source_revision);
    core.set("artifactRevision", snapshot.core.artifact_revision);
    core.set("readOnly", snapshot.core.read_only);
    out.set("core", core);

    val objects = val::array();
    for (std::size_t i = 0; i < snapshot.objects.size(); ++i) {
        const auto& object = snapshot.objects[i];
        val row = val::object();
        row.set("id", object.id);
        row.set("effectiveState", object.effective_state);
        row.set("mass", object.mass);
        row.set("massAvailable", object.mass_available);
        row.set("kineticEnergy", object.kinetic_energy);
        row.set("kineticEnergyAvailable", object.kinetic_energy_available);
        row.set("effectiveRadius", object.effective_radius);
        row.set("position", vec3_value(object.position));
        row.set("velocity", vec3_value(object.velocity));
        row.set("momentum", vec3_value(object.momentum));
        row.set("locked", object.locked);
        row.set("constituent", object.constituent);
        row.set("identityAvailable", object.identity_available);
        row.set("ageTicks", static_cast<double>(object.age_ticks));
        row.set("manifestationSupportCount", object.manifestation_support_count);
        row.set("constituentCount", object.constituent_count);
        row.set("integerCenter", vec3_value(object.integer_center));
        row.set("fractionalCenter", vec3_value(object.fractional_center));
        row.set("integerCenterAvailable", object.integer_center_available);
        row.set("fractionalCenterAvailable", object.fractional_center_available);
        row.set("identityMargin", object.identity_margin);
        row.set("clockPhase", object.clock_phase);
        row.set("clockPhaseAvailable", object.clock_phase_available);
        row.set("graphMargin", object.graph_margin);
        row.set("energyMargin", object.energy_margin);
        val parents = val::array();
        for (std::size_t p = 0; p < object.parent_ids.size(); ++p) {
            parents.set(p, object.parent_ids[p]);
        }
        row.set("parentIds", parents);
        row.set("provenance", provenance_value(object.provenance));
        objects.set(i, row);
    }
    out.set("objects", objects);

    val fields = val::array();
    for (std::size_t i = 0; i < snapshot.fields.size(); ++i) {
        const auto& field = snapshot.fields[i];
        val row = val::object();
        row.set("channel", ftd::scale1_field_channel_id(field.channel));
        row.set("available", field.available);
        row.set("summaryOnly", field.summary_only);
        row.set("energy", field.energy);
        row.set("center", vec3_value(field.center));
        row.set("producerId", field.producer_id);
        row.set("unavailableReason", field.unavailable_reason);
        fields.set(i, row);
    }
    out.set("fields", fields);

    val forces = val::array();
    for (std::size_t i = 0; i < snapshot.forces.size(); ++i) {
        const auto& force = snapshot.forces[i];
        val row = val::object();
        row.set("objectId", force.object_id);
        row.set("termId", force.term_id);
        row.set("force", vec3_value(force.force));
        row.set("status", ftd::scale1_status_id(force.status));
        row.set("conservative", force.conservative);
        row.set("accounted", force.accounted);
        forces.set(i, row);
    }
    out.set("forces", forces);

    val events = val::array();
    for (std::size_t i = 0; i < snapshot.events.size(); ++i) {
        const auto& event = snapshot.events[i];
        val row = val::object();
        row.set("sequence", static_cast<double>(event.sequence));
        row.set("tick", static_cast<double>(event.tick));
        row.set("type", ftd::scale1_event_id(event.type));
        row.set("participantA", event.participant_a);
        row.set("participantB", event.participant_b);
        row.set("stateEnergyDelta", event.state_energy_delta);
        row.set("accountingComplete", event.accounting_complete);
        row.set("status", ftd::scale1_status_id(event.status));
        row.set("sourceId", event.source_id);
        events.set(i, row);
    }
    out.set("events", events);

    const auto& c = snapshot.conservation;
    val conservation = val::object();
    conservation.set("kineticEnergy", c.kinetic_energy);
    conservation.set("potentialEnergy", c.potential_energy);
    conservation.set("stateEnergy", c.state_energy);
    conservation.set("coulombPotential", c.coulomb_potential);
    conservation.set("gravityPotential", c.gravity_potential);
    conservation.set("totalMomentum", vec3_value(c.total_momentum));
    conservation.set("totalAngularMomentum", vec3_value(c.total_angular_momentum));
    conservation.set("centerOfMass", vec3_value(c.center_of_mass));
    conservation.set("coveredMask", c.covered_mask);
    conservation.set("missingMask", c.missing_mask);
    conservation.set("nonconservativeMask", c.nonconservative_mask);
    conservation.set("stateEnergyComplete", c.state_energy_complete);
    conservation.set("driftEligible", c.drift_eligible);
    conservation.set("cumulativeDampingSink", c.cumulative_damping_sink);
    conservation.set("cumulativeRadiationSink", c.cumulative_radiation_sink);
    conservation.set("cumulativeSpeedProjectionSink", c.cumulative_speed_projection_sink);
    conservation.set("cumulativeContactDelta", c.cumulative_contact_delta);
    out.set("conservation", conservation);

    val capabilities = val::array();
    for (std::size_t i = 0; i < snapshot.capability_ids.size(); ++i) {
        capabilities.set(i, snapshot.capability_ids[i]);
    }
    out.set("capabilityIds", capabilities);
    val unavailable = val::array();
    for (std::size_t i = 0; i < snapshot.unavailable_reasons.size(); ++i) {
        unavailable.set(i, snapshot.unavailable_reasons[i]);
    }
    out.set("unavailableReasons", unavailable);
    out.set("status", snapshot.status);
    return out;
}

static val get_scale1_registry() {
    val out = val::object();
    out.set("revision", ftd::SCALE1_REGISTRY_REVISION);
    val physics = val::array();
    const auto& specs = ftd::scale1_physics_registry();
    for (std::size_t i = 0; i < specs.size(); ++i) {
        const auto& spec = specs[i];
        val row = val::object();
        row.set("id", spec.id); row.set("toggle", spec.toggle_name);
        row.set("label", spec.label); row.set("summary", spec.summary);
        row.set("tier", ftd::scale1_tier_id(spec.tier));
        row.set("status", ftd::scale1_status_id(spec.status));
        row.set("canonicalSource", spec.canonical_source);
        row.set("available", spec.available);
        row.set("verifiedProfile", spec.verified_profile);
        row.set("conservative", spec.conservative);
        row.set("potentialAccounted", spec.potential_accounted);
        row.set("backendMask", spec.backend_mask);
        row.set("coverageMask", ftd::scale1_bit(spec.coverage));
        row.set("unavailableReason", spec.unavailable_reason);
        row.set("validationState", ftd::scale1_validation_state_id(spec.validation_state));
        row.set("validationEvidence", spec.validation_evidence);
        row.set("validationCriterion", spec.validation_criterion);
        physics.set(i, row);
    }
    out.set("physics", physics);

    val capabilities = val::array();
    const auto& caps = ftd::scale1_capability_registry();
    for (std::size_t i = 0; i < caps.size(); ++i) {
        const auto& spec = caps[i];
        val row = val::object();
        row.set("id", spec.id); row.set("label", spec.label);
        row.set("summary", spec.summary);
        row.set("mode", ftd::scale1_mode_id(spec.mode));
        row.set("status", ftd::scale1_status_id(spec.status));
        row.set("canonicalSource", spec.canonical_source);
        row.set("available", spec.available);
        row.set("backendMask", spec.backend_mask);
        row.set("unavailableReason", spec.unavailable_reason);
        capabilities.set(i, row);
    }
    out.set("capabilities", capabilities);

    val observers = val::array();
    const auto& observer_specs = ftd::scale1_observer_registry();
    for (std::size_t i = 0; i < observer_specs.size(); ++i) {
        const auto& spec = observer_specs[i];
        val row = val::object();
        row.set("id", spec.id); row.set("label", spec.label);
        row.set("channel", ftd::scale1_field_channel_id(spec.channel));
        row.set("status", ftd::scale1_status_id(spec.status));
        row.set("canonicalSource", spec.canonical_source);
        row.set("availableLive", spec.available_live);
        row.set("availableReplay", spec.available_replay);
        row.set("unavailableReason", spec.unavailable_reason);
        observers.set(i, row);
    }
    out.set("observers", observers);

    val scenarios = val::array();
    const auto& scenario_specs = ftd::scale1_scenario_registry();
    for (std::size_t i = 0; i < scenario_specs.size(); ++i) {
        const auto& spec = scenario_specs[i];
        val row = val::object();
        row.set("id", spec.id); row.set("label", spec.label);
        row.set("family", spec.family);
        row.set("workspace", ftd::scale1_workspace_id(spec.workspace));
        row.set("mode", ftd::scale1_mode_id(spec.mode));
        row.set("owner", ftd::scale1_owner_id(spec.owner));
        row.set("scenarioClass", ftd::scale1_scenario_class_id(spec.scenario_class));
        row.set("status", ftd::scale1_status_id(spec.status));
        row.set("canonicalSource", spec.canonical_source);
        row.set("setupId", spec.setup_id);
        row.set("summary", spec.summary);
        row.set("expectedObservable", spec.expected_observable);
        row.set("prohibitedClaim", spec.prohibited_claim);
        row.set("available", spec.available);
        row.set("interactive", spec.interactive);
        row.set("backendMask", spec.backend_mask);
        row.set("performanceClass", ftd::scale1_performance_class_id(spec.performance));
        row.set("unavailableReason", spec.unavailable_reason);
        row.set("validationState", ftd::scale1_validation_state_id(spec.validation_state));
        row.set("validationEvidence", spec.validation_evidence);
        row.set("validationCriterion", spec.validation_criterion);
        row.set("physicsMask", spec.physics_mask);
        row.set("behavior", ftd::scale1_scenario_behavior_id(spec.behavior));
        row.set("pairedScenarioId", spec.paired_scenario_id);
        scenarios.set(i, row);
    }
    out.set("scenarios", scenarios);
    return out;
}

static val get_pe_snapshot(ftd::ParticleEngine& pe, const std::string& scenario) {
    // Browser observations serialize forces through the compact typed-array
    // force endpoints. Avoid rebuilding hundreds of per-term JS objects in
    // the structural snapshot when no consumer reads them.
    return scale1_snapshot_value(pe.snapshot(scenario, "wasm32", false));
}

static val get_scale1_native_matter_replay() {
    return scale1_snapshot_value(ftd::NativeMatterObserver::m3_registered_replay());
}

static val get_scale1_live_cluster_observation(
    const val& ids, const val& supports, const val& signs,
    const val& centers, const val& velocities,
    int source_tick, const std::string& source_scenario,
    const std::string& source_revision, int lattice_size,
    double display_scale) {
    const unsigned count = ids["length"].as<unsigned>();
    if (supports["length"].as<unsigned>() != count
        || signs["length"].as<unsigned>() != count
        || centers["length"].as<unsigned>() != count * 3u
        || velocities["length"].as<unsigned>() != count * 3u) {
        throw std::invalid_argument(
            "Scale1 live observation arrays have inconsistent lengths");
    }
    std::vector<ftd::Scale1SourceClusterRecord> sources;
    sources.reserve(count);
    for (unsigned i = 0; i < count; ++i) {
        ftd::Scale1SourceClusterRecord source;
        source.source_object_id = ids[i].as<int>();
        source.source_tick = source_tick;
        source.source_scenario = source_scenario;
        source.source_revision = source_revision;
        source.lattice_size = lattice_size;
        source.manifestation_count = supports[i].as<int>();
        source.state_sign = signs[i].as<int>();
        source.centroid = {
            centers[i * 3u].as<double>(),
            centers[i * 3u + 1u].as<double>(),
            centers[i * 3u + 2u].as<double>(),
        };
        source.centroid_velocity = {
            velocities[i * 3u].as<double>(),
            velocities[i * 3u + 1u].as<double>(),
            velocities[i * 3u + 2u].as<double>(),
        };
        source.display_scale = display_scale;
        source.field_state_available = false;
        sources.push_back(std::move(source));
    }
    return scale1_snapshot_value(ftd::NativeMatterObserver::observe_source_clusters(
        sources, source_tick, source_scenario, source_revision));
}

// ── PE Particle Data Extraction ─────────────────────────────────────
// Returns positions + charge-based colors + mass-based sizes for Three.js
static val get_pe_particle_data(ftd::ParticleEngine& pe) {
    const auto& particles = static_cast<const ftd::ParticleEngine&>(pe).particles();
    int count = static_cast<int>(particles.size());

    val positions = val::global("Float32Array").new_(count * 3);
    val velocities = val::global("Float32Array").new_(count * 3);
    val colors    = val::global("Float32Array").new_(count * 3);
    val sizes     = val::global("Float32Array").new_(count);
    val masses    = val::global("Float64Array").new_(count);
    val r_eff     = val::global("Float32Array").new_(count);
    val charges   = val::global("Int8Array").new_(count);
    val ids       = val::global("Int32Array").new_(count);
    val locked    = val::global("Uint8Array").new_(count);
    val spins     = val::global("Int8Array").new_(count);
    val color_ids = val::global("Int8Array").new_(count);
    val spin_axes = val::global("Float32Array").new_(count * 3);

    for (int i = 0; i < count; ++i) {
        const auto& p = particles[i];

        positions.set(i * 3,     static_cast<float>(p.position.x));
        positions.set(i * 3 + 1, static_cast<float>(p.position.y));
        positions.set(i * 3 + 2, static_cast<float>(p.position.z));
        velocities.set(i * 3,     static_cast<float>(p.velocity.x));
        velocities.set(i * 3 + 1, static_cast<float>(p.velocity.y));
        velocities.set(i * 3 + 2, static_cast<float>(p.velocity.z));
        spin_axes.set(i * 3,     static_cast<float>(p.spin_axis.x));
        spin_axes.set(i * 3 + 1, static_cast<float>(p.spin_axis.y));
        spin_axes.set(i * 3 + 2, static_cast<float>(p.spin_axis.z));

        // Default colors by charge (overridden by JS catalog lookup)
        if (p.charge > 0) {
            colors.set(i * 3,     0.29f);
            colors.set(i * 3 + 1, 0.87f);
            colors.set(i * 3 + 2, 0.50f);
        } else if (p.charge < 0) {
            colors.set(i * 3,     0.97f);
            colors.set(i * 3 + 1, 0.44f);
            colors.set(i * 3 + 2, 0.44f);
        } else {
            colors.set(i * 3,     0.60f);
            colors.set(i * 3 + 1, 0.60f);
            colors.set(i * 3 + 2, 0.70f);
        }

        // Size proportional to log(mass/m_e) + 1
        float s = 3.0f + 2.0f * static_cast<float>(std::log10(p.mass / ftd::K_B + 1.0));
        if (s > 12.0f) s = 12.0f;
        sizes.set(i, s);

        charges.set(i, static_cast<int>(p.charge));
        ids.set(i, p.id);
        masses.set(i, p.mass);
        r_eff.set(i, static_cast<float>(p.r_eff));
        locked.set(i, p.locked ? 1 : 0);
        spins.set(i, static_cast<int>(p.spin));
        color_ids.set(i, static_cast<int>(p.color));
    }

    val result = val::object();
    result.set("positions", positions);
    result.set("velocities", velocities);
    result.set("colors", colors);
    result.set("sizes", sizes);
    result.set("charges", charges);
    result.set("ids", ids);
    result.set("masses", masses);
    result.set("rEff", r_eff);
    result.set("locked", locked);
    result.set("spins", spins);
    result.set("colorIds", color_ids);
    result.set("spinAxes", spin_axes);
    result.set("count", count);
    return result;
}

// ── PE Diagnostics ─────────────────────────────────────────────────
static val get_pe_diagnostics(ftd::ParticleEngine& pe) {
    auto d = pe.diagnostics();
    val result = val::object();
    result.set("tick",           d.tick);
    result.set("particleCount",  d.particle_count);
    result.set("totalKE",        d.total_ke);
    result.set("totalPE",        d.total_pe);
    result.set("coulombPE",      d.coulomb_pe);
    result.set("gravityPE",      d.gravity_pe);
    result.set("totalEnergy",    d.total_energy);
    result.set("momentumX",      d.total_momentum.x);
    result.set("momentumY",      d.total_momentum.y);
    result.set("momentumZ",      d.total_momentum.z);
    result.set("angMomX",        d.total_angular_momentum.x);
    result.set("angMomY",        d.total_angular_momentum.y);
    result.set("angMomZ",        d.total_angular_momentum.z);
    result.set("centerX",        d.center_of_mass.x);
    result.set("centerY",        d.center_of_mass.y);
    result.set("centerZ",        d.center_of_mass.z);
    result.set("coveredMask",    d.covered_mask);
    result.set("missingMask",    d.missing_mask);
    result.set("nonconservativeMask", d.nonconservative_mask);
    result.set("stateEnergyComplete", d.state_energy_complete);
    result.set("driftEligible", d.drift_eligible);
    result.set("cumulativeDampingSink", d.cumulative_damping_sink);
    result.set("cumulativeRadiationSink", d.cumulative_radiation_sink);
    result.set("cumulativeSpeedProjectionSink", d.cumulative_speed_projection_sink);
    result.set("cumulativeContactDelta", d.cumulative_contact_delta);
    result.set("contactEventCount", static_cast<double>(d.contact_event_count));
    result.set("speedProjectionCount", static_cast<double>(d.speed_projection_count));
    result.set("insulatorCollisionCount",
               static_cast<double>(d.insulator_collision_count));
    result.set("insulatorPortCrossingCount",
               static_cast<double>(d.insulator_port_crossing_count));
    result.set("insulatorImpulseX", d.cumulative_insulator_impulse.x);
    result.set("insulatorImpulseY", d.cumulative_insulator_impulse.y);
    result.set("insulatorImpulseZ", d.cumulative_insulator_impulse.z);
    return result;
}

// ── PE Particle injection ──────────────────────────────────────────
static int pe_add_particle(ftd::ParticleEngine& pe, int charge,
                           double x, double y, double z,
                           double vx, double vy, double vz,
                           double mass, double r_eff) {
    return pe.add_particle(static_cast<int8_t>(charge),
                           ftd::Vec3(x, y, z),
                           ftd::Vec3(vx, vy, vz),
                           mass, r_eff);
}

static int pe_add_locked_particle(ftd::ParticleEngine& pe, int charge,
                                   double x, double y, double z,
                                   double mass, double r_eff) {
    if (!std::isfinite(r_eff) || r_eff < 0.0) {
        throw std::invalid_argument(
            "ParticleEngine locked-particle effective radius must be finite and nonnegative");
    }
    int id = pe.add_locked_particle(static_cast<int8_t>(charge),
                                     ftd::Vec3(x, y, z), mass);
    // Override default r_eff (C++ default is 2.48, too large for atomic orbits)
    pe.particles().back().r_eff = r_eff;
    return id;
}

// Full-fidelity injection: catalog spin/color and spin axis in one call.
// Needed by the Zoo ([PARAMETRIC] catalog particles) and the lattice
// read-only native-matter source observer.
static int pe_add_particle_ex(ftd::ParticleEngine& pe, int charge,
                              double x, double y, double z,
                              double vx, double vy, double vz,
                              double mass, double r_eff,
                              int spin, int color,
                              double sax, double say, double saz) {
    if (!std::isfinite(sax) || !std::isfinite(say) || !std::isfinite(saz)) {
        throw std::invalid_argument("ParticleEngine spin axis must be finite");
    }
    int id = pe.add_particle(static_cast<int8_t>(charge),
                             ftd::Vec3(x, y, z),
                             ftd::Vec3(vx, vy, vz),
                             mass, r_eff,
                             static_cast<int8_t>(spin),
                             static_cast<int8_t>(color));
    pe.particles().back().spin_axis = ftd::Vec3(sax, say, saz);
    return id;
}

static int pe_add_locked_particle_ex(ftd::ParticleEngine& pe, int charge,
                                     double x, double y, double z,
                                     double mass, double r_eff,
                                     int spin, int color,
                                     double sax, double say, double saz) {
    if (!std::isfinite(r_eff) || r_eff < 0.0) {
        throw std::invalid_argument(
            "ParticleEngine locked-particle effective radius must be finite and nonnegative");
    }
    if (!std::isfinite(sax) || !std::isfinite(say) || !std::isfinite(saz)) {
        throw std::invalid_argument("ParticleEngine spin axis must be finite");
    }
    int id = pe.add_locked_particle(static_cast<int8_t>(charge),
                                    ftd::Vec3(x, y, z), mass,
                                    static_cast<int8_t>(spin),
                                    static_cast<int8_t>(color));
    auto& p = pe.particles().back();
    p.r_eff = r_eff;
    p.spin_axis = ftd::Vec3(sax, say, saz);
    return id;
}

static ftd::Particle* pe_find_by_id(ftd::ParticleEngine& pe, int id) {
    for (auto& p : pe.particles()) {
        if (p.id == id) return &p;
    }
    return nullptr;
}

static void pe_set_spin_axis(ftd::ParticleEngine& pe, int id,
                             double ax, double ay, double az) {
    if (!std::isfinite(ax) || !std::isfinite(ay) || !std::isfinite(az)
        || ax * ax + ay * ay + az * az < 1e-60) {
        throw std::invalid_argument(
            "ParticleEngine spin axis must be finite and nonzero");
    }
    if (auto* p = pe_find_by_id(pe, id)) p->spin_axis = ftd::Vec3(ax, ay, az);
}

// Injection-time velocity write (equilibrium-orbit seeding from the JS
// adapter: probe the native force, compute circular-orbit speed, write it
// back). Not a dynamics feature — scenario setup only.
static void pe_set_velocity(ftd::ParticleEngine& pe, int id,
                            double vx, double vy, double vz) {
    if (!std::isfinite(vx) || !std::isfinite(vy) || !std::isfinite(vz)) {
        throw std::invalid_argument("ParticleEngine velocity must be finite");
    }
    (void)pe.set_particle_velocity(id, ftd::Vec3(vx, vy, vz));
}

// ── PE Controls ────────────────────────────────────────────────────
static void pe_set_dt(ftd::ParticleEngine& pe, double dt) { pe.set_dt(dt); }
static double pe_get_dt(ftd::ParticleEngine& pe) { return pe.dt(); }
static void pe_set_softening(ftd::ParticleEngine& pe, double s) { pe.set_softening(s); }
static void pe_set_damping(ftd::ParticleEngine& pe, bool e) { pe.set_damping_enabled(e); }
static void pe_set_gravity(ftd::ParticleEngine& pe, bool e) { pe.set_gravity_enabled(e); }
static int pe_particle_count(ftd::ParticleEngine& pe) {
    return static_cast<int>(static_cast<const ftd::ParticleEngine&>(pe).particles().size());
}

static void pe_configure_insulating_box(ftd::ParticleEngine& pe,
                                         double cx, double cy, double cz,
                                         double hx, double hy, double hz) {
    pe.configure_insulating_box({cx, cy, cz}, {hx, hy, hz});
}

static void pe_add_insulating_port(ftd::ParticleEngine& pe, int axis, int side,
                                    double center_u, double center_v,
                                    double half_u, double half_v,
                                    int required_charge_sign,
                                    int crossing_direction) {
    pe.add_insulating_port(axis, side, center_u, center_v, half_u, half_v,
                           required_charge_sign, crossing_direction);
}

static void pe_clear_insulating_box(ftd::ParticleEngine& pe) {
    pe.clear_insulating_box();
}

static void pe_clear(ftd::ParticleEngine& pe) {
    pe.clear();
}

// ── PE Toggle getter/setter (generic, by name) ────────────────────
// Pointer-to-member map for ParticleToggles, built once from the single
// source of truth PARTICLE_TOGGLE_SPECS (ADR-0013, ticket 3.3) so a new
// toggle needs no edit here.
using PeBoolPTM = bool ftd::ParticleToggles::*;
static const std::unordered_map<std::string, PeBoolPTM>& pe_toggle_map() {
    static const std::unordered_map<std::string, PeBoolPTM> kMap = []{
        std::unordered_map<std::string, PeBoolPTM> m;
        for (const auto& s : ftd::PARTICLE_TOGGLE_SPECS) m.emplace(s.name, s.field);
        return m;
    }();
    return kMap;
}

static bool pe_set_toggle(ftd::ParticleEngine& pe, const std::string& name, bool val) {
    return pe.try_set_toggle(name, val);
}

static bool pe_get_toggle(ftd::ParticleEngine& pe, const std::string& name) {
    auto it = pe_toggle_map().find(name);
    if (it != pe_toggle_map().end()) return pe.toggles.*(it->second);
    return false;
}

// ── PE Force Diagnostic ───────────────────────────────────────────
static val get_pe_force_diag(ftd::ParticleEngine& pe, int idx) {
    val result = val::object();
    const auto& fd = pe.force_diag();
    if (idx < 0 || idx >= static_cast<int>(fd.size())) return result;
    const auto& d = fd[idx];
    result.set("coulomb_x", d.f_coulomb.x); result.set("coulomb_y", d.f_coulomb.y); result.set("coulomb_z", d.f_coulomb.z);
    result.set("gravity_x", d.f_gravity.x); result.set("gravity_y", d.f_gravity.y); result.set("gravity_z", d.f_gravity.z);
    result.set("lorentz_x", d.f_lorentz.x); result.set("lorentz_y", d.f_lorentz.y); result.set("lorentz_z", d.f_lorentz.z);
    result.set("exchange_x", d.f_exchange.x); result.set("exchange_y", d.f_exchange.y); result.set("exchange_z", d.f_exchange.z);
    result.set("strong_x", d.f_strong.x); result.set("strong_y", d.f_strong.y); result.set("strong_z", d.f_strong.z);
    result.set("radiation_x", d.f_radiation.x); result.set("radiation_y", d.f_radiation.y); result.set("radiation_z", d.f_radiation.z);
    result.set("spin_orbit_x", d.f_spin_orbit.x); result.set("spin_orbit_y", d.f_spin_orbit.y); result.set("spin_orbit_z", d.f_spin_orbit.z);
    result.set("relativistic_x", d.f_relativistic.x); result.set("relativistic_y", d.f_relativistic.y); result.set("relativistic_z", d.f_relativistic.z);
    result.set("magnetic_dipole_x", d.f_magnetic_dipole.x); result.set("magnetic_dipole_y", d.f_magnetic_dipole.y); result.set("magnetic_dipole_z", d.f_magnetic_dipole.z);
    auto tot = d.total();
    result.set("total_x", tot.x); result.set("total_y", tot.y); result.set("total_z", tot.z);
    return result;
}

static double get_pe_pair_coulomb_force_magnitude(
    ftd::ParticleEngine& pe, int i, int j) {
    return pe.compute_pair_force_diagnostic(i, j).f_coulomb.mag();
}

static val get_pe_forces(ftd::ParticleEngine& pe) {
    const auto& particles = static_cast<const ftd::ParticleEngine&>(pe).particles();
    const auto& observed = pe.observation_force_diag();
    int count = static_cast<int>(particles.size());
    val positions = val::global("Float32Array").new_(count * 3);
    // Float64: G_PE-only forces (~5e-46) underflow Float32 subnormals to
    // exactly 0, blanking the gravity overlay for every lepton scenario.
    val forces = val::global("Float64Array").new_(count * 3);
    double max_force = 0.0;

    for (int i = 0; i < count; ++i) {
        const auto& p = particles[i];
        const auto& fd = observed[i];
        auto f = fd.total();
        positions.set(i * 3,     static_cast<float>(p.position.x));
        positions.set(i * 3 + 1, static_cast<float>(p.position.y));
        positions.set(i * 3 + 2, static_cast<float>(p.position.z));
        forces.set(i * 3,     f.x);
        forces.set(i * 3 + 1, f.y);
        forces.set(i * 3 + 2, f.z);
        double mag = f.mag();
        if (mag > max_force) max_force = mag;
    }

    val result = val::object();
    result.set("positions", positions);
    result.set("forces", forces);
    result.set("count", count);
    result.set("maxForce", max_force);
    return result;
}

// Batched per-term force arrays for the F_C / F_G / F_S / F_net overlays.
// One embind crossing instead of N peGetForceDiag calls. `net` is the TRUE
// total of every enabled term (incl. exchange, Lorentz, and radiation) — the
// force the integrator actually applies, unlike the
// retired JS re-implementation which summed only five terms.
static val get_pe_force_decomposition(ftd::ParticleEngine& pe) {
    const auto& particles = static_cast<const ftd::ParticleEngine&>(pe).particles();
    const auto& observed = pe.observation_force_diag();
    int count = static_cast<int>(particles.size());

    val positions       = val::global("Float32Array").new_(count * 3);
    val coulomb         = val::global("Float64Array").new_(count * 3);
    val gravity         = val::global("Float64Array").new_(count * 3);
    val lorentz         = val::global("Float64Array").new_(count * 3);
    val exchange        = val::global("Float64Array").new_(count * 3);
    val strong          = val::global("Float64Array").new_(count * 3);
    val radiation       = val::global("Float64Array").new_(count * 3);
    val magnetic_dipole = val::global("Float64Array").new_(count * 3);
    val spin_orbit      = val::global("Float64Array").new_(count * 3);
    val net             = val::global("Float64Array").new_(count * 3);
    double max_coulomb = 0.0, max_gravity = 0.0, max_lorentz = 0.0;
    double max_exchange = 0.0, max_strong = 0.0, max_radiation = 0.0;
    double max_magnetic = 0.0, max_spin_orbit = 0.0, max_net = 0.0;

    for (int i = 0; i < count; ++i) {
        const auto& p = particles[i];
        const auto& fd = observed[i];
        auto f = fd.total();
        positions.set(i * 3,     static_cast<float>(p.position.x));
        positions.set(i * 3 + 1, static_cast<float>(p.position.y));
        positions.set(i * 3 + 2, static_cast<float>(p.position.z));
        coulomb.set(i * 3,     fd.f_coulomb.x);
        coulomb.set(i * 3 + 1, fd.f_coulomb.y);
        coulomb.set(i * 3 + 2, fd.f_coulomb.z);
        gravity.set(i * 3,     fd.f_gravity.x);
        gravity.set(i * 3 + 1, fd.f_gravity.y);
        gravity.set(i * 3 + 2, fd.f_gravity.z);
        lorentz.set(i * 3,     fd.f_lorentz.x);
        lorentz.set(i * 3 + 1, fd.f_lorentz.y);
        lorentz.set(i * 3 + 2, fd.f_lorentz.z);
        exchange.set(i * 3,     fd.f_exchange.x);
        exchange.set(i * 3 + 1, fd.f_exchange.y);
        exchange.set(i * 3 + 2, fd.f_exchange.z);
        strong.set(i * 3,     fd.f_strong.x);
        strong.set(i * 3 + 1, fd.f_strong.y);
        strong.set(i * 3 + 2, fd.f_strong.z);
        radiation.set(i * 3,     fd.f_radiation.x);
        radiation.set(i * 3 + 1, fd.f_radiation.y);
        radiation.set(i * 3 + 2, fd.f_radiation.z);
        magnetic_dipole.set(i * 3,     fd.f_magnetic_dipole.x);
        magnetic_dipole.set(i * 3 + 1, fd.f_magnetic_dipole.y);
        magnetic_dipole.set(i * 3 + 2, fd.f_magnetic_dipole.z);
        spin_orbit.set(i * 3,     fd.f_spin_orbit.x);
        spin_orbit.set(i * 3 + 1, fd.f_spin_orbit.y);
        spin_orbit.set(i * 3 + 2, fd.f_spin_orbit.z);
        net.set(i * 3,     f.x);
        net.set(i * 3 + 1, f.y);
        net.set(i * 3 + 2, f.z);
        max_coulomb    = std::max(max_coulomb,    fd.f_coulomb.mag());
        max_gravity    = std::max(max_gravity,    fd.f_gravity.mag());
        max_lorentz    = std::max(max_lorentz,    fd.f_lorentz.mag());
        max_exchange   = std::max(max_exchange,   fd.f_exchange.mag());
        max_strong     = std::max(max_strong,     fd.f_strong.mag());
        max_radiation  = std::max(max_radiation,  fd.f_radiation.mag());
        max_magnetic   = std::max(max_magnetic,   fd.f_magnetic_dipole.mag());
        max_spin_orbit = std::max(max_spin_orbit, fd.f_spin_orbit.mag());
        max_net        = std::max(max_net,        f.mag());
    }

    val result = val::object();
    result.set("positions", positions);
    result.set("coulomb", coulomb);
    result.set("gravity", gravity);
    result.set("lorentz", lorentz);
    result.set("exchange", exchange);
    result.set("strong", strong);
    result.set("radiation", radiation);
    result.set("magnetic_dipole", magnetic_dipole);
    result.set("spin_orbit", spin_orbit);
    result.set("net", net);
    result.set("maxCoulomb", max_coulomb);
    result.set("maxGravity", max_gravity);
    result.set("maxLorentz", max_lorentz);
    result.set("maxExchange", max_exchange);
    result.set("maxStrong", max_strong);
    result.set("maxRadiation", max_radiation);
    result.set("maxMagneticDipole", max_magnetic);
    result.set("maxSpinOrbit", max_spin_orbit);
    result.set("maxNet", max_net);
    result.set("count", count);
    return result;
}

static val get_pe_extended_data(ftd::ParticleEngine& pe) {
    const auto& particles = static_cast<const ftd::ParticleEngine&>(pe).particles();
    const auto& observed = pe.observation_force_diag();
    int count = static_cast<int>(particles.size());
    val ids = val::global("Int32Array").new_(count);
    val charges = val::global("Int8Array").new_(count);
    val masses = val::global("Float64Array").new_(count);
    val positions = val::global("Float64Array").new_(count * 3);
    val velocities = val::global("Float64Array").new_(count * 3);
    val forces = val::global("Float64Array").new_(count * 3);
    val accelerations = val::global("Float64Array").new_(count * 3);
    val locked = val::global("Uint8Array").new_(count);
    val r_eff = val::global("Float64Array").new_(count);
    val spins = val::global("Int8Array").new_(count);
    val color_ids = val::global("Int8Array").new_(count);
    val pair_ids = val::global("Int32Array").new_(count);

    for (int i = 0; i < count; ++i) {
        const auto& p = particles[i];
        const auto& fd = observed[i];
        auto f = fd.total();
        ids.set(i, p.id);
        charges.set(i, static_cast<int>(p.charge));
        masses.set(i, p.mass);
        locked.set(i, p.locked ? 1 : 0);
        r_eff.set(i, p.r_eff);
        spins.set(i, static_cast<int>(p.spin));
        color_ids.set(i, static_cast<int>(p.color));
        pair_ids.set(i, p.pair_id);
        positions.set(i * 3,     p.position.x);
        positions.set(i * 3 + 1, p.position.y);
        positions.set(i * 3 + 2, p.position.z);
        velocities.set(i * 3,     p.velocity.x);
        velocities.set(i * 3 + 1, p.velocity.y);
        velocities.set(i * 3 + 2, p.velocity.z);
        forces.set(i * 3,     f.x);
        forces.set(i * 3 + 1, f.y);
        forces.set(i * 3 + 2, f.z);
        if (p.mass > 1e-30 && !p.locked) {
            accelerations.set(i * 3,     f.x / p.mass);
            accelerations.set(i * 3 + 1, f.y / p.mass);
            accelerations.set(i * 3 + 2, f.z / p.mass);
        } else {
            accelerations.set(i * 3, 0.0);
            accelerations.set(i * 3 + 1, 0.0);
            accelerations.set(i * 3 + 2, 0.0);
        }
    }

    val result = val::object();
    result.set("count", count);
    result.set("ids", ids);
    result.set("charges", charges);
    result.set("masses", masses);
    result.set("positions", positions);
    result.set("velocities", velocities);
    result.set("forces", forces);
    result.set("accelerations", accelerations);
    result.set("locked", locked);
    result.set("rEff", r_eff);
    result.set("spins", spins);
    result.set("colorIds", color_ids);
    result.set("pairIds", pair_ids);
    return result;
}

// ── Versioned deterministic checkpoints ───────────────────────────
static ftd::Vec3 checkpoint_vec3(const val& value) {
    return {
        value["x"].as<double>(),
        value["y"].as<double>(),
        value["z"].as<double>(),
    };
}

static val checkpoint_provenance_value(const ftd::Scale1Provenance& p) {
    val out = provenance_value(p);
    out.set("statusCode", static_cast<int>(p.status));
    out.set("qualificationCode", static_cast<int>(p.qualification));
    return out;
}

static ftd::Scale1Provenance checkpoint_provenance(const val& value) {
    ftd::Scale1Provenance out;
    out.source_scale = value["sourceScale"].as<int>();
    out.source_tick = static_cast<std::int64_t>(value["sourceTick"].as<double>());
    out.source_object_id = value["sourceObjectId"].as<int>();
    out.source_kind = value["sourceKind"].as<std::string>();
    out.source_scenario = value["sourceScenario"].as<std::string>();
    out.source_revision = value["sourceRevision"].as<std::string>();
    out.status = static_cast<ftd::Scale1EpistemicStatus>(
        value["statusCode"].as<int>());
    out.qualification = static_cast<ftd::Scale1Qualification>(
        value["qualificationCode"].as<int>());
    return out;
}

static val export_pe_checkpoint(ftd::ParticleEngine& pe) {
    const auto saved = pe.checkpoint();
    val out = val::object();
    out.set("schema", "ftd.scale1.particle-checkpoint");
    out.set("schemaVersion", saved.schema_version);
    out.set("tick", saved.tick);
    out.set("nextId", saved.next_id);
    out.set("nextEventSequence", static_cast<double>(saved.next_event_sequence));
    out.set("dt", saved.dt);
    out.set("softening", saved.softening);

    val toggles = val::object();
    for (const auto& spec : ftd::PARTICLE_TOGGLE_SPECS) {
        toggles.set(spec.name, saved.toggles.*(spec.field));
    }
    out.set("toggles", toggles);

    val particles = val::array();
    for (std::size_t i = 0; i < saved.particles.size(); ++i) {
        const auto& p = saved.particles[i];
        val row = val::object();
        row.set("id", p.id);
        row.set("charge", static_cast<int>(p.charge));
        row.set("mass", p.mass);
        row.set("effectiveRadius", p.r_eff);
        row.set("position", vec3_value(p.position));
        row.set("velocity", vec3_value(p.velocity));
        row.set("acceleration", vec3_value(p.acceleration));
        row.set("previousAcceleration", vec3_value(p.prev_acceleration));
        row.set("spin", static_cast<int>(p.spin));
        row.set("color", static_cast<int>(p.color));
        row.set("pairId", p.pair_id);
        row.set("locked", p.locked);
        row.set("spinAxis", vec3_value(p.spin_axis));
        row.set("momentum", vec3_value(p.momentum));
        row.set("provenance", checkpoint_provenance_value(p.provenance));
        particles.set(i, row);
    }
    out.set("particles", particles);

    val events = val::array();
    for (std::size_t i = 0; i < saved.events.size(); ++i) {
        const auto& event = saved.events[i];
        val row = val::object();
        row.set("sequence", static_cast<double>(event.sequence));
        row.set("tick", static_cast<double>(event.tick));
        row.set("typeCode", static_cast<int>(event.type));
        row.set("participantA", event.participant_a);
        row.set("participantB", event.participant_b);
        row.set("stateEnergyDelta", event.state_energy_delta);
        row.set("accountingComplete", event.accounting_complete);
        row.set("statusCode", static_cast<int>(event.status));
        row.set("sourceId", event.source_id);
        events.set(i, row);
    }
    out.set("events", events);

    val box = val::object();
    box.set("enabled", saved.insulating_box.enabled);
    box.set("center", vec3_value(saved.insulating_box.center));
    box.set("halfExtents", vec3_value(saved.insulating_box.half_extents));
    val ports = val::array();
    for (std::size_t i = 0; i < saved.insulating_box.ports.size(); ++i) {
        const auto& port = saved.insulating_box.ports[i];
        val row = val::object();
        row.set("axis", port.axis);
        row.set("side", port.side);
        row.set("centerU", port.center_u);
        row.set("centerV", port.center_v);
        row.set("halfU", port.half_u);
        row.set("halfV", port.half_v);
        row.set("requiredChargeSign", port.required_charge_sign);
        row.set("crossingDirection", port.crossing_direction);
        ports.set(i, row);
    }
    box.set("ports", ports);
    out.set("insulatingBox", box);

    val ledger = val::object();
    ledger.set("cumulativeDampingSink", saved.cumulative_damping_sink);
    ledger.set("cumulativeRadiationSink", saved.cumulative_radiation_sink);
    ledger.set("cumulativeSpeedProjectionSink", saved.cumulative_speed_projection_sink);
    ledger.set("cumulativeContactDelta", saved.cumulative_contact_delta);
    ledger.set("contactEventCount", static_cast<double>(saved.contact_event_count));
    ledger.set("speedProjectionCount", static_cast<double>(saved.speed_projection_count));
    ledger.set("insulatorCollisionCount", static_cast<double>(saved.insulator_collision_count));
    ledger.set("insulatorPortCrossingCount", static_cast<double>(saved.insulator_port_crossing_count));
    ledger.set("cumulativeInsulatorImpulse", vec3_value(saved.cumulative_insulator_impulse));
    out.set("ledger", ledger);
    return out;
}

static bool restore_pe_checkpoint(ftd::ParticleEngine& pe, const val& value) {
    try {
        if (value["schema"].as<std::string>() != "ftd.scale1.particle-checkpoint") {
            throw std::invalid_argument("not a Scale 1 ParticleEngine checkpoint");
        }
        ftd::ParticleEngineCheckpoint saved;
        saved.schema_version = value["schemaVersion"].as<int>();
        saved.tick = value["tick"].as<int>();
        saved.next_id = value["nextId"].as<int>();
        saved.next_event_sequence = static_cast<std::uint64_t>(
            value["nextEventSequence"].as<double>());
        saved.dt = value["dt"].as<double>();
        saved.softening = value["softening"].as<double>();
        const val toggle_values = value["toggles"];
        for (const auto& spec : ftd::PARTICLE_TOGGLE_SPECS) {
            saved.toggles.*(spec.field) = toggle_values[spec.name].as<bool>();
        }

        const val particles = value["particles"];
        const unsigned particle_count = particles["length"].as<unsigned>();
        saved.particles.reserve(particle_count);
        for (unsigned i = 0; i < particle_count; ++i) {
            const val row = particles[i];
            ftd::Particle p;
            p.id = row["id"].as<int>();
            p.charge = static_cast<std::int8_t>(row["charge"].as<int>());
            p.mass = row["mass"].as<double>();
            p.r_eff = row["effectiveRadius"].as<double>();
            p.position = checkpoint_vec3(row["position"]);
            p.velocity = checkpoint_vec3(row["velocity"]);
            p.acceleration = checkpoint_vec3(row["acceleration"]);
            p.prev_acceleration = checkpoint_vec3(row["previousAcceleration"]);
            p.spin = static_cast<std::int8_t>(row["spin"].as<int>());
            p.color = static_cast<std::int8_t>(row["color"].as<int>());
            p.pair_id = row["pairId"].as<int>();
            p.locked = row["locked"].as<bool>();
            p.spin_axis = checkpoint_vec3(row["spinAxis"]);
            p.momentum = checkpoint_vec3(row["momentum"]);
            p.provenance = checkpoint_provenance(row["provenance"]);
            saved.particles.push_back(std::move(p));
        }

        const val events = value["events"];
        const unsigned event_count = events["length"].as<unsigned>();
        saved.events.reserve(event_count);
        for (unsigned i = 0; i < event_count; ++i) {
            const val row = events[i];
            ftd::Scale1EventRecord event;
            event.sequence = static_cast<std::uint64_t>(row["sequence"].as<double>());
            event.tick = static_cast<std::int64_t>(row["tick"].as<double>());
            event.type = static_cast<ftd::Scale1EventType>(row["typeCode"].as<int>());
            event.participant_a = row["participantA"].as<int>();
            event.participant_b = row["participantB"].as<int>();
            event.state_energy_delta = row["stateEnergyDelta"].as<double>();
            event.accounting_complete = row["accountingComplete"].as<bool>();
            event.status = static_cast<ftd::Scale1EpistemicStatus>(
                row["statusCode"].as<int>());
            event.source_id = row["sourceId"].as<std::string>();
            saved.events.push_back(std::move(event));
        }

        const val box = value["insulatingBox"];
        saved.insulating_box.enabled = box["enabled"].as<bool>();
        saved.insulating_box.center = checkpoint_vec3(box["center"]);
        saved.insulating_box.half_extents = checkpoint_vec3(box["halfExtents"]);
        const val ports = box["ports"];
        const unsigned port_count = ports["length"].as<unsigned>();
        saved.insulating_box.ports.reserve(port_count);
        for (unsigned i = 0; i < port_count; ++i) {
            const val row = ports[i];
            ftd::ParticleInsulatingPort port;
            port.axis = row["axis"].as<int>();
            port.side = row["side"].as<int>();
            port.center_u = row["centerU"].as<double>();
            port.center_v = row["centerV"].as<double>();
            port.half_u = row["halfU"].as<double>();
            port.half_v = row["halfV"].as<double>();
            port.required_charge_sign = row["requiredChargeSign"].as<int>();
            port.crossing_direction = row["crossingDirection"].as<int>();
            saved.insulating_box.ports.push_back(port);
        }

        const val ledger = value["ledger"];
        saved.cumulative_damping_sink = ledger["cumulativeDampingSink"].as<double>();
        saved.cumulative_radiation_sink = ledger["cumulativeRadiationSink"].as<double>();
        saved.cumulative_speed_projection_sink = ledger["cumulativeSpeedProjectionSink"].as<double>();
        saved.cumulative_contact_delta = ledger["cumulativeContactDelta"].as<double>();
        saved.contact_event_count = static_cast<std::uint64_t>(ledger["contactEventCount"].as<double>());
        saved.speed_projection_count = static_cast<std::uint64_t>(ledger["speedProjectionCount"].as<double>());
        saved.insulator_collision_count = static_cast<std::uint64_t>(ledger["insulatorCollisionCount"].as<double>());
        saved.insulator_port_crossing_count = static_cast<std::uint64_t>(ledger["insulatorPortCrossingCount"].as<double>());
        saved.cumulative_insulator_impulse = checkpoint_vec3(
            ledger["cumulativeInsulatorImpulse"]);

        std::string error;
        if (!pe.restore_checkpoint(saved, &error)) {
            throw std::invalid_argument(error);
        }
        return true;
    } catch (const std::exception& error) {
        throw std::invalid_argument(
            std::string("Invalid Scale 1 checkpoint: ") + error.what());
    }
}

// ── Isolated FTD-0884 finite-port field-battery observer ──────────
// This reference instrument is intentionally not coupled to ParticleEngine
// forces. It visualizes the theorem-of-record finite ready-port bank and its
// imposed positive quadratic battery while preserving every production,
// moving-source, photon, and Born firewall carried by the native witness.
class Scale1FinitePortBatteryObserver {
public:
    Scale1FinitePortBatteryObserver(int size, int capacity,
                                    double charge_amplitude,
                                    double battery_amplitude) {
        if (size < 2 || capacity < 1 || !std::isfinite(charge_amplitude)
            || charge_amplitude == 0.0 || !std::isfinite(battery_amplitude)
            || battery_amplitude == 0.0) {
            throw std::invalid_argument("invalid finite-port battery observer parameters");
        }
        ftd::eft::MatchedFaceFlux indexing(size);
        std::vector<double> charge(indexing.x.size(), 0.0);
        const int y = size / 2;
        const int z = size / 2;
        charge[static_cast<std::size_t>(indexing.index(size / 4, y, z))]
            = std::abs(charge_amplitude);
        charge[static_cast<std::size_t>(indexing.index((3 * size) / 4, y, z))]
            = -std::abs(charge_amplitude);
        std::vector<double> battery(charge.size(), std::abs(battery_amplitude));
        for (std::size_t i = 1; i < battery.size(); i += 2) battery[i] *= -1.0;
        witness_ = ftd::eft::FinitePortGaussBattery(
            size, charge, static_cast<std::size_t>(capacity), battery, 1e-12);
        if (!witness_.valid()) {
            throw std::invalid_argument("finite-port battery observer failed to initialize");
        }
    }

    bool step() { return witness_.step_fresh_layer().valid(); }
    bool reverse() { return witness_.reverse_last_layer(); }

    val snapshot() const {
        const auto& flux = witness_.flux();
        const int count = static_cast<int>(flux.x.size());
        val positions = val::global("Float32Array").new_(count * 3);
        val vectors = val::global("Float32Array").new_(count * 3);
        val magnitudes = val::global("Float32Array").new_(count);
        val charge = val::global("Float32Array").new_(count);
        double max_magnitude = 0.0;
        const double offset = (flux.L - 1) * 0.5;
        for (int x = 0; x < flux.L; ++x) {
            for (int y = 0; y < flux.L; ++y) {
                for (int z = 0; z < flux.L; ++z) {
                    const int i = flux.index(x, y, z);
                    const double fx = flux.x[static_cast<std::size_t>(i)];
                    const double fy = flux.y[static_cast<std::size_t>(i)];
                    const double fz = flux.z[static_cast<std::size_t>(i)];
                    const double magnitude = std::sqrt(fx * fx + fy * fy + fz * fz);
                    max_magnitude = std::max(max_magnitude, magnitude);
                    positions.set(i * 3, static_cast<float>((x - offset) * 2.0));
                    positions.set(i * 3 + 1, static_cast<float>((y - offset) * 2.0));
                    positions.set(i * 3 + 2, static_cast<float>((z - offset) * 2.0));
                    vectors.set(i * 3, static_cast<float>(fx));
                    vectors.set(i * 3 + 1, static_cast<float>(fy));
                    vectors.set(i * 3 + 2, static_cast<float>(fz));
                    magnitudes.set(i, static_cast<float>(magnitude));
                    charge.set(i, static_cast<float>(witness_.charge()[static_cast<std::size_t>(i)]));
                }
            }
        }
        val out = val::object();
        out.set("size", flux.L);
        out.set("count", count);
        out.set("positions", positions);
        out.set("vectors", vectors);
        out.set("magnitudes", magnitudes);
        out.set("charge", charge);
        out.set("maxMagnitude", max_magnitude);
        out.set("statusCode", static_cast<int>(witness_.status()));
        out.set("valid", witness_.valid());
        out.set("cursor", static_cast<double>(witness_.cursor()));
        out.set("capacity", static_cast<double>(witness_.port_capacity()));
        out.set("acceptedLayers", static_cast<double>(witness_.accepted_layers()));
        out.set("fieldEnergy", witness_.field_energy());
        out.set("portBankEnergy", witness_.port_bank_energy());
        out.set("batteryEnergy", witness_.battery_energy());
        out.set("totalBookedEnergy", witness_.total_booked_energy());
        out.set("finiteCyclicIndefiniteFreshness", witness_.finite_cyclic_indefinite_freshness());
        out.set("imposedBatteryLaw", witness_.imposed_battery_law());
        out.set("canonicalHamiltonianReservoirSupplied", witness_.canonical_hamiltonian_reservoir_supplied());
        out.set("movingSourceContinuitySupplied", witness_.moving_source_continuity_supplied());
        out.set("productionCouplingSupplied", witness_.production_coupling_supplied());
        out.set("nativeGstarSynchronizationSupplied", witness_.native_gstar_synchronization_supplied());
        out.set("bornWeightsUsed", witness_.born_weights_used());
        return out;
    }

private:
    ftd::eft::FinitePortGaussBattery witness_;
};

// Keep production tick dispatch out of Embind's virtual member-function
// invoker. Memory64 lowers a bound virtual method to a generic pointer-to-
// member call_indirect; a stale member descriptor or object vtable then traps
// before ParticleEngine::tick() begins ("table index is out of bounds").
// These free entry points have a fixed signature and use an explicitly
// qualified call, so the Scale-1 transaction reaches the concrete engine
// without an indirect virtual dispatch.
static void pe_tick_engine(ftd::ParticleEngine& pe) {
    pe.ftd::ParticleEngine::tick();
}

static void pe_run_engine(ftd::ParticleEngine& pe, int num_ticks) {
    for (int i = 0; i < num_ticks; ++i) {
        pe.ftd::ParticleEngine::tick();
    }
}

static int pe_current_tick(ftd::ParticleEngine& pe) {
    return pe.ftd::ParticleEngine::current_tick();
}

static std::uint64_t pe_observation_revision(ftd::ParticleEngine& pe) {
    return pe.observation_revision();
}

// ── Embind Registration ──────────────────────────────────────────────
EMSCRIPTEN_BINDINGS(ftd_module_particle) {
    class_<ftd::ParticleEngine>("ParticleEngine")
        .constructor<>()
        ;
    class_<Scale1FinitePortBatteryObserver>("Scale1FinitePortBatteryObserver")
        .constructor<int, int, double, double>()
        .function("step", &Scale1FinitePortBatteryObserver::step)
        .function("reverse", &Scale1FinitePortBatteryObserver::reverse)
        .function("snapshot", &Scale1FinitePortBatteryObserver::snapshot)
        ;

    function("getPEParticleData",   &get_pe_particle_data);
    function("peTickEngine",        &pe_tick_engine);
    function("peRunEngine",         &pe_run_engine);
    function("peCurrentTick",       &pe_current_tick);
    function("peObservationRevision", &pe_observation_revision);
    function("getPEDiagnostics",    &get_pe_diagnostics);
    function("getPESnapshot",       &get_pe_snapshot);
    function("getScale1Registry",   &get_scale1_registry);
    function("getScale1NativeMatterReplay", &get_scale1_native_matter_replay);
    function("getScale1LiveClusterObservation", &get_scale1_live_cluster_observation);
    function("getPEExtendedData",   &get_pe_extended_data);
    function("exportPECheckpoint",  &export_pe_checkpoint);
    function("restorePECheckpoint", &restore_pe_checkpoint);
    function("getPEForces",         &get_pe_forces);
    function("getPEForceDecomposition", &get_pe_force_decomposition);
    function("peAddParticle",       &pe_add_particle);
    function("peAddLockedParticle", &pe_add_locked_particle);
    function("peAddParticleEx",       &pe_add_particle_ex);
    function("peAddLockedParticleEx", &pe_add_locked_particle_ex);
    function("peSetSpinAxis",       &pe_set_spin_axis);
    function("peSetVelocity",       &pe_set_velocity);
    function("peSetDt",             &pe_set_dt);
    function("peGetDt",             &pe_get_dt);
    function("peSetSoftening",      &pe_set_softening);
    function("peSetDamping",        &pe_set_damping);
    function("peSetGravity",        &pe_set_gravity);
    function("peConfigureInsulatingBox", &pe_configure_insulating_box);
    function("peAddInsulatingPort", &pe_add_insulating_port);
    function("peClearInsulatingBox", &pe_clear_insulating_box);
    function("peSetToggle",         &pe_set_toggle);
    function("peGetToggle",         &pe_get_toggle);
    function("peGetForceDiag",      &get_pe_force_diag);
    function("getPECoulombPairForceMagnitude", &get_pe_pair_coulomb_force_magnitude);
    function("peParticleCount",     &pe_particle_count);
    function("peClear",             &pe_clear);
}
