#pragma once
/**
 * @file scale1/domain.h
 * @brief Shared scientific contract for Scale 1.
 *
 * Scale 1 has two deliberately separate dynamics owners:
 *   - NativeMatterObserver: read-only views of qualified Scale-0 records;
 *   - ParticleEngine: an effective continuous-coordinate laboratory.
 *
 * This schema is consumed by the native host and WASM.  Frontends may format
 * these records, but they must not infer claim status, force applicability,
 * or conservation coverage on their own.
 */

#include "ftd/voxel.h"

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace ftd {

inline constexpr std::uint32_t SCALE1_SNAPSHOT_SCHEMA_VERSION = 3;
inline constexpr const char* SCALE1_REGISTRY_REVISION = "scale1-v4-particle-context-r10-field-battery";

enum class Scale1Mode : std::uint8_t {
    NativeMatter = 0,
    EffectiveLab,
    CatalogReference,
};

enum class Scale1DynamicsOwner : std::uint8_t {
    NativeMatterObserver = 0,
    ParticleEngine,
    Catalog,
};

// A workspace is a presentation/research context.  It is deliberately
// distinct from Scale1Mode, which names the dynamics owner.  Several particle
// workspaces can therefore observe the same immutable native record without
// starting a second point-particle universe.
enum class Scale1Workspace : std::uint8_t {
    ParticleObservatory = 0,
    QuantumReference,
    QedReference,
    ReferenceLaboratory,
};

enum class Scale1ScenarioClass : std::uint8_t {
    LiveNative = 0,
    QualifiedReplay,
    FalsificationReplay,
    SelectedCandidate,
    EffectiveReference,
    ParametricCatalog,
};

enum class Scale1PerformanceClass : std::uint8_t {
    Light = 0,
    Standard,
    Heavy,
};

// User-visible runtime behavior is part of the scientific scenario contract.
// It prevents a static field, null control, or read-only evidence artifact from
// being mistaken for a broken time-evolving simulation.
enum class Scale1ScenarioBehavior : std::uint8_t {
    Dynamic = 0,
    ReadOnlyReplay,
    StaticField,
    NullControl,
    AwaitingInput,
    StaticReference,
};

/**
 * What the registered evidence actually validates.
 *
 * This is deliberately separate from epistemic status.  A numerically correct
 * imposed force kernel may be KernelValidated without becoming derived FTD
 * physics, while a closed-negative scenario may be BoundaryConfirmed without
 * becoming runnable positive evidence.
 */
enum class Scale1ValidationState : std::uint8_t {
    ContractQualified = 0,
    KernelValidated,
    ConditionalEvidence,
    BoundaryConfirmed,
    OpenBlocked,
    InvalidRetired,
};

enum class Scale1EpistemicStatus : std::uint8_t {
    Axiom = 0,
    Theorem,
    Derived,
    Selection,
    Imposed,
    Measured,
    Parametric,
    StronglyMotivatedConjecture,
    Open,
    ClosedNegative,
    Invalid,
};

enum class Scale1Qualification : std::uint8_t {
    NotEvaluated = 0,
    QualifiedSelected,
    HeuristicCandidate,
    Failed,
    InvalidEvidence,
};

enum class Scale1ModuleTier : std::uint8_t {
    VerifiedBaseline = 0,
    SelectedExtension,
    ExperimentalQuarantined,
    Retired,
};

enum class Scale1EventType : std::uint8_t {
    ContactRemoval = 0,
    Formation,
    Dissociation,
    RecordExpiry,
};

enum class Scale1FieldChannel : std::uint8_t {
    Actual = 0,
    SelectedBound,
    Residual,
    Outgoing,
    Background,
};

enum class Scale1Backend : std::uint32_t {
    Cpu = 1u << 0,
    NativeCuda = 1u << 1,
    Wasm32 = 1u << 2,
    Wasm64 = 1u << 3,
    ThreadedWasm = 1u << 4,
};

enum class Scale1Coverage : std::uint32_t {
    None = 0,
    Kinetic = 1u << 0,
    CoulombPotential = 1u << 1,
    GravityPotential = 1u << 2,
    DampingSink = 1u << 3,
    RadiationSink = 1u << 4,
    ContactEvents = 1u << 5,
    ExchangePotential = 1u << 6,
    StrongPotential = 1u << 7,
    LorentzFieldEnergy = 1u << 8,
    DipolePotential = 1u << 9,
    SpinOrbitPotential = 1u << 10,
    SpeedProjectionSink = 1u << 11,
};

constexpr std::uint32_t scale1_bit(Scale1Backend v) {
    return static_cast<std::uint32_t>(v);
}
constexpr std::uint32_t scale1_bit(Scale1Coverage v) {
    return static_cast<std::uint32_t>(v);
}
const char* scale1_mode_id(Scale1Mode value);
const char* scale1_owner_id(Scale1DynamicsOwner value);
const char* scale1_workspace_id(Scale1Workspace value);
const char* scale1_scenario_class_id(Scale1ScenarioClass value);
const char* scale1_scenario_behavior_id(Scale1ScenarioBehavior value);
const char* scale1_performance_class_id(Scale1PerformanceClass value);
const char* scale1_validation_state_id(Scale1ValidationState value);
const char* scale1_status_id(Scale1EpistemicStatus value);
const char* scale1_qualification_id(Scale1Qualification value);
const char* scale1_tier_id(Scale1ModuleTier value);
const char* scale1_event_id(Scale1EventType value);
const char* scale1_field_channel_id(Scale1FieldChannel value);

struct Scale1PhysicsSpec {
    const char* id = "";
    const char* toggle_name = "";
    const char* label = "";
    const char* summary = "";
    Scale1ModuleTier tier = Scale1ModuleTier::ExperimentalQuarantined;
    Scale1EpistemicStatus status = Scale1EpistemicStatus::Imposed;
    const char* canonical_source = "";
    bool available = false;
    bool verified_profile = false;
    bool conservative = false;
    bool potential_accounted = false;
    std::uint32_t backend_mask = 0;
    Scale1Coverage coverage = Scale1Coverage::None;
    const char* unavailable_reason = "";
    Scale1ValidationState validation_state = Scale1ValidationState::OpenBlocked;
    const char* validation_evidence = "";
    const char* validation_criterion = "";
};

struct Scale1CapabilitySpec {
    const char* id = "";
    const char* label = "";
    const char* summary = "";
    Scale1Mode mode = Scale1Mode::EffectiveLab;
    Scale1EpistemicStatus status = Scale1EpistemicStatus::Open;
    const char* canonical_source = "";
    bool available = false;
    std::uint32_t backend_mask = 0;
    const char* unavailable_reason = "";
};

struct Scale1ObserverSpec {
    const char* id = "";
    const char* label = "";
    Scale1FieldChannel channel = Scale1FieldChannel::Actual;
    Scale1EpistemicStatus status = Scale1EpistemicStatus::Open;
    const char* canonical_source = "";
    bool available_live = false;
    bool available_replay = false;
    const char* unavailable_reason = "";
};

/**
 * One canonical scenario-manifest row.
 *
 * The native registry owns scientific status, availability, provenance, and
 * dynamics ownership.  A frontend may attach a setup handler to setup_id, but
 * it must not redefine these fields or turn an unavailable row into a live
 * result.
 */
struct Scale1ScenarioSpec {
    const char* id = "";
    const char* label = "";
    const char* family = "";
    Scale1Workspace workspace = Scale1Workspace::ParticleObservatory;
    Scale1Mode mode = Scale1Mode::NativeMatter;
    Scale1DynamicsOwner owner = Scale1DynamicsOwner::NativeMatterObserver;
    Scale1ScenarioClass scenario_class = Scale1ScenarioClass::QualifiedReplay;
    Scale1EpistemicStatus status = Scale1EpistemicStatus::Open;
    const char* canonical_source = "";
    const char* setup_id = "";
    const char* summary = "";
    const char* expected_observable = "";
    const char* prohibited_claim = "";
    bool available = false;
    bool interactive = false;
    std::uint32_t backend_mask = 0;
    Scale1PerformanceClass performance = Scale1PerformanceClass::Light;
    const char* unavailable_reason = "";
    Scale1ValidationState validation_state = Scale1ValidationState::OpenBlocked;
    const char* validation_evidence = "";
    const char* validation_criterion = "";
    // Bit i corresponds to row i of scale1_physics_registry().  This profile is
    // native-owned; frontends may choose numerical display parameters but may
    // not silently add or remove physical modules from a scenario.
    std::uint32_t physics_mask = 0;
    Scale1ScenarioBehavior behavior = Scale1ScenarioBehavior::Dynamic;
    // Optional registered A/B peer. Empty means this is not a paired control.
    const char* paired_scenario_id = "";
};

const std::vector<Scale1PhysicsSpec>& scale1_physics_registry();
const std::vector<Scale1CapabilitySpec>& scale1_capability_registry();
const std::vector<Scale1ObserverSpec>& scale1_observer_registry();
const std::vector<Scale1ScenarioSpec>& scale1_scenario_registry();
const Scale1PhysicsSpec* find_scale1_physics_spec(std::string_view id_or_toggle);
const Scale1ScenarioSpec* find_scale1_scenario_spec(std::string_view id);

struct Scale1Provenance {
    int source_scale = 1;
    std::int64_t source_tick = 0;
    std::int32_t source_object_id = -1;
    std::string source_kind = "effective_seed";
    std::string source_scenario;
    std::string source_revision = SCALE1_REGISTRY_REVISION;
    Scale1EpistemicStatus status = Scale1EpistemicStatus::Imposed;
    Scale1Qualification qualification = Scale1Qualification::NotEvaluated;
};

struct Scale1ObjectRecord {
    std::int32_t id = -1;
    int effective_state = 0;
    double mass = 0.0;
    bool mass_available = false;
    double kinetic_energy = 0.0;
    bool kinetic_energy_available = false;
    double effective_radius = 0.0;
    Vec3 position;
    Vec3 velocity;
    Vec3 momentum;
    bool locked = false;
    bool constituent = false;
    bool identity_available = false;
    std::int64_t age_ticks = 0;
    int manifestation_support_count = 0;
    int constituent_count = 0;
    Vec3 integer_center;
    Vec3 fractional_center;
    bool integer_center_available = false;
    bool fractional_center_available = false;
    double identity_margin = 0.0;
    double clock_phase = 0.0;
    bool clock_phase_available = false;
    double graph_margin = 0.0;
    double energy_margin = 0.0;
    std::vector<std::int32_t> parent_ids;
    Scale1Provenance provenance;
};

struct Scale1FieldRecord {
    Scale1FieldChannel channel = Scale1FieldChannel::Actual;
    bool available = false;
    bool summary_only = true;
    double energy = 0.0;
    Vec3 center;
    std::string producer_id;
    std::string unavailable_reason;
};

struct Scale1ForceRecord {
    std::int32_t object_id = -1;
    std::string term_id;
    Vec3 force;
    Scale1EpistemicStatus status = Scale1EpistemicStatus::Imposed;
    bool conservative = false;
    bool accounted = false;
};

struct Scale1EventRecord {
    std::uint64_t sequence = 0;
    std::int64_t tick = 0;
    Scale1EventType type = Scale1EventType::ContactRemoval;
    std::int32_t participant_a = -1;
    std::int32_t participant_b = -1;
    double state_energy_delta = 0.0;
    bool accounting_complete = false;
    Scale1EpistemicStatus status = Scale1EpistemicStatus::Selection;
    std::string source_id;
};

struct Scale1ConservationRecord {
    double kinetic_energy = 0.0;
    double potential_energy = 0.0;
    double state_energy = 0.0;
    double coulomb_potential = 0.0;
    double gravity_potential = 0.0;
    Vec3 total_momentum;
    Vec3 total_angular_momentum;
    Vec3 center_of_mass;
    std::uint32_t covered_mask = 0;
    std::uint32_t missing_mask = 0;
    std::uint32_t nonconservative_mask = 0;
    bool state_energy_complete = false;
    bool drift_eligible = false;
    double cumulative_damping_sink = 0.0;
    double cumulative_radiation_sink = 0.0;
    double cumulative_speed_projection_sink = 0.0;
    double cumulative_contact_delta = 0.0;
};

struct Scale1SnapshotCore {
    std::uint32_t schema_version = SCALE1_SNAPSHOT_SCHEMA_VERSION;
    std::string registry_revision = SCALE1_REGISTRY_REVISION;
    std::int64_t tick = 0;
    double effective_dt = 0.0;
    Scale1Mode mode = Scale1Mode::NativeMatter;
    Scale1Workspace workspace = Scale1Workspace::ParticleObservatory;
    Scale1ScenarioClass scenario_class = Scale1ScenarioClass::QualifiedReplay;
    Scale1DynamicsOwner dynamics_owner = Scale1DynamicsOwner::NativeMatterObserver;
    std::string backend = "cpu";
    std::string scenario;
    std::string source_revision;
    std::string artifact_revision;
    bool read_only = true;
};

struct Scale1Snapshot {
    Scale1SnapshotCore core;
    std::vector<Scale1ObjectRecord> objects;
    std::vector<Scale1FieldRecord> fields;
    std::vector<Scale1ForceRecord> forces;
    std::vector<Scale1EventRecord> events;
    Scale1ConservationRecord conservation;
    std::vector<std::string> capability_ids;
    std::vector<std::string> unavailable_reasons;

    // Compatibility fields for the current native RML surface.  They mirror
    // the structured records above and are never an independent producer.
    int particle_count = 0;
    double total_energy = 0.0;
    double total_ke = 0.0;
    double total_pe = 0.0;
    std::string status;
    bool insp_present = false;
    int insp_index = -1;
    int insp_charge = 0;
    bool insp_locked = false;
    double insp_pos[3] = {0.0, 0.0, 0.0};
    double insp_vel[3] = {0.0, 0.0, 0.0};
};

struct Scale1SourceClusterRecord {
    std::int32_t source_object_id = -1;
    std::int64_t source_tick = 0;
    std::string source_scenario;
    std::string source_revision;
    int lattice_size = 0;
    int manifestation_count = 0;
    int state_sign = 0;
    Vec3 centroid;
    Vec3 centroid_velocity;
    double display_scale = 1.0;
    bool constituent_relations_available = false;
    bool field_state_available = false;
    bool bound_residual_available = false;
    bool current_history_available = false;
    bool spin_color_available = false;
};

class NativeMatterObserver {
public:
    // Immutable replay of one registered FTD-0760 candidate row.  It is an
    // observation fixture, not a live generator and not an SM particle.
    static Scale1Snapshot m3_registered_replay();

    // Read-only observation of one coherent Scale-0 capture boundary. Source
    // clusters are candidate records only: no mass, species, stability, or
    // generic-particle qualification is inferred by this conversion.
    static Scale1Snapshot observe_source_clusters(
        const std::vector<Scale1SourceClusterRecord>& sources,
        std::int64_t source_tick,
        const std::string& source_scenario,
        const std::string& source_revision);
};

}  // namespace ftd
