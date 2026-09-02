#include "ftd/scale1/domain.h"

#include "ftd/constants.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace ftd {
namespace {

constexpr std::uint32_t kCpuWasm =
    scale1_bit(Scale1Backend::Cpu)
    | scale1_bit(Scale1Backend::Wasm32)
    | scale1_bit(Scale1Backend::Wasm64)
    | scale1_bit(Scale1Backend::ThreadedWasm);
constexpr std::uint32_t kAllImplemented = kCpuWasm | scale1_bit(Scale1Backend::NativeCuda);

// Bit positions are pinned to kPhysics order and serialized with every
// scenario. Keep the matching registry-order test when adding a module.
constexpr std::uint32_t kPCoulomb = 1u << 0;
constexpr std::uint32_t kPGravity = 1u << 1;
constexpr std::uint32_t kPDamping = 1u << 2;
constexpr std::uint32_t kPLorentz = 1u << 3;
constexpr std::uint32_t kPExchange = 1u << 4;
constexpr std::uint32_t kPStrong = 1u << 5;
constexpr std::uint32_t kPRadiation = 1u << 6;
constexpr std::uint32_t kPSpinOrbit = 1u << 7;
constexpr std::uint32_t kPRetiredRescale = 1u << 8;
constexpr std::uint32_t kPMagneticDipole = 1u << 9;
constexpr std::uint32_t kPRelativisticVerlet = 1u << 10;
constexpr std::uint32_t kPContactEvents = 1u << 11;
constexpr std::uint32_t kVerifiedProfile = kPCoulomb | kPRelativisticVerlet;

const std::vector<Scale1PhysicsSpec> kPhysics = {
    {"coulomb", "coulomb", "Coulomb pair force",
     "Effective softened electrostatic pair force. The form and coupling statuses are separate.",
     Scale1ModuleTier::VerifiedBaseline, Scale1EpistemicStatus::Parametric,
     "FTD-0792; Phase-G geometric Coulomb", true, true, true, true,
     kAllImplemented, Scale1Coverage::CoulombPotential, "",
     Scale1ValidationState::ContractQualified,
     "ctest:particle_engine;ctest:particle_toggles",
     "Pair sign, softened magnitude, action-reaction, energy, and momentum invariants pass."},
    {"newton_gravity", "gravity", "Newton pair gravity",
     "Effective comparison force; not the Scale-0 geometric-gravity operator and not GR.",
     Scale1ModuleTier::SelectedExtension,
     Scale1EpistemicStatus::StronglyMotivatedConjecture,
     "FTD-0131; FTD-1013--1022", true, false, true, true,
     kAllImplemented, Scale1Coverage::GravityPotential, "",
     Scale1ValidationState::KernelValidated,
     "ctest:particle_engine;ctest:particle_toggles",
     "Attractive sign, softened magnitude, pair decomposition, and conservative ledger pass for the imposed Newtonian model."},
    {"damping", "damping", "Environment damping",
     "Imposed bath-frame velocity damping with an explicit numerical sink ledger.",
     Scale1ModuleTier::ExperimentalQuarantined, Scale1EpistemicStatus::Imposed,
     "FTD-1012 (diagnostic boundary only)", true, false, false, true,
     kCpuWasm, Scale1Coverage::DampingSink, "",
     Scale1ValidationState::KernelValidated,
     "ctest:particle_engine;ctest:particle_toggles",
     "Velocity decreases monotonically and the numerical sink ledger receives the removed energy."},
    {"lorentz", "lorentz", "Lorentz extension",
     "Imported effective v cross B interaction; field energy is not closed.",
     Scale1ModuleTier::ExperimentalQuarantined, Scale1EpistemicStatus::Imposed,
     "effective extension", true, false, false, false,
     kCpuWasm, Scale1Coverage::LorentzFieldEnergy,
     "No shared field-energy ledger closes this interaction.",
     Scale1ValidationState::KernelValidated, "ctest:pe_forces:lorentz",
     "Toggle isolation, stationary null, velocity orthogonality, source dependence, and diagnostics pass; energy closure remains unavailable."},
    {"exchange", "exchange", "Exchange toy",
     "Pauli-style repulsion is not native exchange statistics.",
     Scale1ModuleTier::ExperimentalQuarantined, Scale1EpistemicStatus::Open,
     "FTD-1024", true, false, true, false,
     kCpuWasm, Scale1Coverage::ExchangePotential,
     "Exchange statistics, CAR/Fock, and a matching potential are open.",
     Scale1ValidationState::KernelValidated, "ctest:pe_forces:exchange;FTD-1024",
     "Implemented repulsion kernel passes spin/charge selectors and isolation; native exchange statistics remain open."},
    {"strong", "strong", "Color-force toy",
     "Injected color labels and an imposed short-range/string force; not native QCD.",
     Scale1ModuleTier::ExperimentalQuarantined, Scale1EpistemicStatus::Imposed,
     "FTD-0020; FTD-0025", true, false, true, false,
     kCpuWasm, Scale1Coverage::StrongPotential,
     "No production Scale-1 Hamiltonian closes this imposed force.",
     Scale1ValidationState::KernelValidated, "ctest:pe_forces:strong;FTD-0020;FTD-0025",
     "Injected-color kernel passes selector, range, direction, and isolation checks; this is not QCD recovery."},
    {"radiation", "radiation", "Radiation-reaction toy",
     "Imported acceleration damping without an emitted field channel.",
     Scale1ModuleTier::ExperimentalQuarantined, Scale1EpistemicStatus::Imposed,
     "effective extension", true, false, false, false,
     kCpuWasm, Scale1Coverage::RadiationSink,
     "An emitted channel and exact sink accounting are not represented.",
     Scale1ValidationState::KernelValidated, "ctest:pe_forces:radiation",
     "Acceleration-dependent damping and toggle isolation pass; emitted-field and exact sink closure remain unavailable."},
    {"spin_orbit", "spin_orbit", "Spin-orbit toy",
     "Imported L dot S interaction over injected spin labels.",
     Scale1ModuleTier::ExperimentalQuarantined, Scale1EpistemicStatus::Imposed,
     "effective extension", true, false, true, false,
     kCpuWasm, Scale1Coverage::SpinOrbitPotential,
     "No matching potential or native spin recovery is implemented.",
     Scale1ValidationState::KernelValidated, "ctest:pe_forces:spin_orbit",
     "Injected L-dot-S kernel passes orientation, sign, range, and isolation checks; native spin recovery remains unavailable."},
    {"isotropic_relativistic_rescale", "relativistic", "Retired isotropic force rescale",
     "A non-covariant visual approximation retained only as a disabled compatibility key.",
     Scale1ModuleTier::Retired, Scale1EpistemicStatus::Invalid,
     "Scale-1 v3 redevelopment audit", false, false, false, false,
     0, Scale1Coverage::None,
     "Retired: isotropic force scaling is not a covariant equation of motion.",
     Scale1ValidationState::InvalidRetired, "ctest:fine_structure_scale1;ctest:particle_toggles_table",
     "Every activation path rejects the retired non-covariant rescale."},
    {"magnetic_dipole", "magnetic_dipole", "Magnetic-dipole toy",
     "Imported dipole interaction over injected spin axes.",
     Scale1ModuleTier::ExperimentalQuarantined, Scale1EpistemicStatus::Imposed,
     "effective extension", true, false, true, false,
     kCpuWasm, Scale1Coverage::DipolePotential,
     "No matching potential/field ledger is implemented.",
     Scale1ValidationState::KernelValidated, "ctest:pe_forces:magnetic_dipole",
     "Dipole orientation, sign, range, source, diagnostics, and toggle-isolation checks pass; field-energy closure remains unavailable."},
    {"relativistic_momentum_verlet", "relativistic_verlet", "Relativistic momentum Verlet",
     "Imposed momentum-form numerical integrator; it is not Lorentz recovery.",
     Scale1ModuleTier::VerifiedBaseline, Scale1EpistemicStatus::Imposed,
     "numerical method", true, true, true, true,
     kCpuWasm, Scale1Coverage::Kinetic, "",
     Scale1ValidationState::ContractQualified,
     "ctest:relativistic_verlet;ctest:particle_engine",
     "Momentum/velocity coherence, finite high-speed transport, transition continuity, and speed ceiling pass."},
    {"contact_events", "contact_events", "Contact-removal event law",
     "Explicit selected removal event for opposite effective charge records; not QED annihilation.",
     Scale1ModuleTier::SelectedExtension, Scale1EpistemicStatus::Selection,
     "Scale-1 event-law selection", true, false, false, true,
     kCpuWasm, Scale1Coverage::ContactEvents, "",
     Scale1ValidationState::ConditionalEvidence,
     "ctest:particle_engine:PE8,PE15",
     "Contact selection, deterministic disjoint ordering, stable IDs, cleanup, and incomplete accounting labels pass."},
};

const std::vector<Scale1CapabilitySpec> kCapabilities = {
    {"native_matter_replay", "Native matter replay",
     "Read-only registered relational-matter evidence.", Scale1Mode::NativeMatter,
     Scale1EpistemicStatus::Measured, "FTD-0760", true, kCpuWasm, ""},
    {"native_matter_live", "Live native matter observer",
     "Coherent read-only observation of a live Scale-0 boundary record.", Scale1Mode::NativeMatter,
     Scale1EpistemicStatus::Measured, "FTD-1023 recovery contract", true, kCpuWasm,
     "Observation is available; generic particle qualification and identity continuity remain open."},
    {"effective_lab", "Effective particle laboratory",
     "Continuous-coordinate analytical-force experiments advanced on ordinal ticks.",
     Scale1Mode::EffectiveLab, Scale1EpistemicStatus::Imposed,
     "ParticleEngine", true, kCpuWasm,
     "Native CUDA kernels exist for selected pair forces, but no complete registered scenario profile is CUDA-qualified yet."},
    {"catalog_reference", "Catalog reference",
     "Reference-only Standard Model catalog; injection is explicitly parametric.",
     Scale1Mode::CatalogReference, Scale1EpistemicStatus::Parametric,
     "particle catalog", true, kCpuWasm, ""},
};

const std::vector<Scale1ObserverSpec> kObservers = {
    {"actual_field", "Actual field", Scale1FieldChannel::Actual,
     Scale1EpistemicStatus::Measured, "FTD-0760", false, true,
     "Replay exposes a registered summary; live coherent field publication is open."},
    {"selected_bound_field", "Selected bound field", Scale1FieldChannel::SelectedBound,
     Scale1EpistemicStatus::Selection, "FTD-0760; FTD-0763", false, true,
     "The bound-field observer is selected and replay-only in this release."},
    {"residual_field", "Residual field", Scale1FieldChannel::Residual,
     Scale1EpistemicStatus::Measured, "FTD-0764--0767", false, true,
     "Only summary metadata is available; do not label it a persistent wake."},
    {"outgoing_field", "Outgoing field", Scale1FieldChannel::Outgoing,
     Scale1EpistemicStatus::Open, "FTD-0765--0768", false, false,
     "No certified persistent outgoing/radiation channel exists."},
    {"background_field", "Background field", Scale1FieldChannel::Background,
     Scale1EpistemicStatus::Measured, "FTD-0760 causal-fibre discriminator", false, true,
     "Replay exposes environmental-fibre status, not a full field volume."},
};

// Scenario status and availability live here, beside the physics and observer
// registries. Frontends attach presentation/setup handlers by setup_id and may
// not change their claim status. The runnable-only program contains no runtime
// scale-handoff or placeholder scenarios.
std::uint32_t effective_physics_mask(std::string_view setup) {
    if (setup == "relativistic_integrator" || setup == "parametric_species"
        || setup == "mass_ladder") {
        return kPRelativisticVerlet;
    }
    if (setup == "damping_sink") return kPDamping | kPRelativisticVerlet;
    if (setup == "advanced_force_isolation") return kPExchange | kPRelativisticVerlet;
    if (setup == "incomplete_conservation") {
        return kPCoulomb | kPExchange | kPRelativisticVerlet;
    }
    if (setup == "contact_selection") {
        return kPCoulomb | kPRelativisticVerlet | kPContactEvents;
    }
    if (setup == "qed_magnetic_dipole") {
        return kPMagneticDipole | kPRelativisticVerlet;
    }
    if (setup == "qed_lorentz_dipole") {
        return kPLorentz | kPRelativisticVerlet;
    }
    if (setup == "qed_spin_orbit") {
        return kPCoulomb | kPSpinOrbit | kPRelativisticVerlet;
    }
    if (setup == "qed_radiation_reaction") {
        return kPCoulomb | kPRadiation | kPRelativisticVerlet;
    }
    if (setup == "quantum_exchange_eligible"
        || setup == "quantum_exchange_spinless_control"
        || setup == "quantum_exchange_range") {
        return kPExchange | kPRelativisticVerlet;
    }
    if (setup == "quantum_spin_orbit_parallel"
        || setup == "quantum_spin_orbit_antiparallel") {
        return kPSpinOrbit | kPRelativisticVerlet;
    }
    if (setup == "quantum_dipole_antiparallel"
        || setup == "quantum_dipole_transverse") {
        return kPMagneticDipole | kPRelativisticVerlet;
    }
    if (setup == "quantum_lorentz_charge_control"
        || setup == "quantum_lorentz_velocity_control") {
        return kPLorentz | kPRelativisticVerlet;
    }
    if (setup == "quantum_radiation_scattering") {
        return kPCoulomb | kPRadiation | kPRelativisticVerlet;
    }
    if (setup == "quantum_relativistic_counterstream") {
        return kPRelativisticVerlet;
    }
    if (setup == "quantum_color_triplet") {
        return kPStrong | kPRelativisticVerlet;
    }
    if (setup == "cluster_pair" || setup == "force_decomposition") {
        return kVerifiedProfile | kPGravity;
    }
    return kVerifiedProfile;
}

#define S1_REPLAY(ID, LABEL, FAMILY, WORKSPACE, STATUS, SOURCE, SETUP, SUMMARY, OBS, NO, PERF) \
    {ID, LABEL, FAMILY, WORKSPACE, Scale1Mode::NativeMatter, \
     Scale1DynamicsOwner::NativeMatterObserver, Scale1ScenarioClass::QualifiedReplay, \
     STATUS, SOURCE, SETUP, SUMMARY, OBS, NO, true, false, kCpuWasm, PERF, "", \
     Scale1ValidationState::ContractQualified, \
     "ctest:scale1_domain;playwright:scale1-particle-overlays", \
     "Registered replay satisfies the shared schema, provenance, availability, and non-overclaim contract.", \
     0, Scale1ScenarioBehavior::ReadOnlyReplay, ""}
#define S1_EFFECTIVE(ID, LABEL, FAMILY, SETUP, SUMMARY, OBS, NO, PERF) \
    {ID, LABEL, FAMILY, Scale1Workspace::ReferenceLaboratory, Scale1Mode::EffectiveLab, \
     Scale1DynamicsOwner::ParticleEngine, Scale1ScenarioClass::EffectiveReference, \
     Scale1EpistemicStatus::Imposed, "ParticleEngine effective reference", SETUP, SUMMARY, OBS, NO, \
     true, true, kCpuWasm, PERF, "", Scale1ValidationState::KernelValidated, \
     "ctest:particle_engine;ctest:pe_forces;playwright:scale1-particle-overlays", \
     "The exact registered profile must seed deterministically, remain finite, and satisfy every applicable kernel/accounting invariant.", \
     effective_physics_mask(SETUP), Scale1ScenarioBehavior::Dynamic, ""}
#define S1_CATALOG(ID, LABEL, SETUP, SUMMARY, OBS, NO) \
    {ID, LABEL, "Catalog and transition", Scale1Workspace::ReferenceLaboratory, \
     Scale1Mode::CatalogReference, Scale1DynamicsOwner::Catalog, \
     Scale1ScenarioClass::ParametricCatalog, Scale1EpistemicStatus::Parametric, \
     "Scale-1 parametric catalog", SETUP, SUMMARY, OBS, NO, true, true, kCpuWasm, \
     Scale1PerformanceClass::Light, "", Scale1ValidationState::ContractQualified, \
     "playwright:scale1-particle-overlays:catalog", \
     "Catalog values, labels, provenance, seed count, and absence of emergence claims must match the registered parametric inputs.", \
     effective_physics_mask(SETUP), Scale1ScenarioBehavior::StaticReference, ""}
#define S1_QED_EFFECTIVE(ID, LABEL, SETUP, SUMMARY, OBS, NO, PERF) \
    {ID, LABEL, "QED effective references", Scale1Workspace::QedReference, \
     Scale1Mode::EffectiveLab, Scale1DynamicsOwner::ParticleEngine, \
     Scale1ScenarioClass::EffectiveReference, Scale1EpistemicStatus::Imposed, \
     "External QED/classical-limit structure; ParticleEngine effective kernel", SETUP, \
     SUMMARY, OBS, NO, true, true, kCpuWasm, PERF, "", \
     Scale1ValidationState::KernelValidated, \
     "ctest:particle_engine;ctest:pe_forces;playwright:scale1-particle-overlays:qed-reference", \
     "The exact registered profile must seed deterministically, remain finite, and retain its explicit QED omissions.", \
     effective_physics_mask(SETUP), Scale1ScenarioBehavior::Dynamic, ""}
#define S1_QUANTUM_EFFECTIVE(ID, LABEL, SETUP, SUMMARY, OBS, NO, PERF) \
    {ID, LABEL, "Quantum-facing effective controls", Scale1Workspace::QuantumReference, \
     Scale1Mode::EffectiveLab, Scale1DynamicsOwner::ParticleEngine, \
     Scale1ScenarioClass::EffectiveReference, Scale1EpistemicStatus::Imposed, \
     "External effective structure; FTD-1024 physical-recovery boundary", SETUP, \
     SUMMARY, OBS, NO, true, true, kCpuWasm, PERF, "", \
     Scale1ValidationState::KernelValidated, \
     "ctest:scale1_domain;ctest:pe_forces;playwright:scale1-particle-overlays:quantum-reference", \
     "The exact profile, eligibility control, force signature, and explicit non-quantum boundary must pass.", \
     effective_physics_mask(SETUP), Scale1ScenarioBehavior::Dynamic, ""}
const std::vector<Scale1ScenarioSpec> kScenarios = [] {
    std::vector<Scale1ScenarioSpec> scenarios = {
    // A. Native particle evidence replay. Six observation subviews share this
    // one immutable artifact; they are not six different simulations.
    S1_REPLAY("s1-native-m3-replay", "M3 Evidence Replay", "Particle evidence replay",
        Scale1Workspace::ParticleObservatory, Scale1EpistemicStatus::Measured, "FTD-0760",
        "m3_anatomy", "One qualified finite-time relational-matter artifact with six inspectable evidence views and no Standard Model identity.",
        "Anatomy, constituent graph, field channels, centers, identity margins, and coverage ledger.",
        "Do not infer stable species, physical mass, spin, statistics, or asymptotic stability.",
        Scale1PerformanceClass::Light),

    // B. Quantum-facing effective controls. These make implemented eligibility,
    // orientation, and force signatures visible without promoting the toys to
    // physical exchange statistics, Hilbert-space dynamics, QFT, or QCD.
    S1_QUANTUM_EFFECTIVE("s1-quantum-exchange-eligible", "Exchange-eligible pair",
        "quantum_exchange_eligible",
        "Two same-charge records with the same injected spin label isolate the short-range exchange toy.",
        "Nonzero repulsive exchange component and incomplete potential coverage.",
        "An eligibility-conditioned repulsive kernel is not Pauli exclusion, antisymmetrization, or fermionic statistics.",
        Scale1PerformanceClass::Light),
    S1_QUANTUM_EFFECTIVE("s1-quantum-exchange-spinless-control", "Exchange spinless null control",
        "quantum_exchange_spinless_control",
        "A matched same-charge pair without injected spin labels tests the exchange eligibility gate.",
        "Zero exchange component with the same geometry and numerical profile.",
        "A zero toy-force control does not derive spin or a quantum selection rule.",
        Scale1PerformanceClass::Light),
    S1_QUANTUM_EFFECTIVE("s1-quantum-exchange-range", "Exchange range comparison",
        "quantum_exchange_range",
        "Separated near and far same-label pairs expose the imposed exponential range dependence.",
        "Near-pair exchange magnitude exceeds the matched far-pair magnitude.",
        "The imposed exponential is not an exchange amplitude or a derived wavefunction-overlap law.",
        Scale1PerformanceClass::Light),
    S1_QUANTUM_EFFECTIVE("s1-quantum-spin-orbit-parallel", "Spin-orbit parallel control",
        "quantum_spin_orbit_parallel",
        "A moving spin-axis record samples the imposed L dot S term with parallel orientation.",
        "Nonzero signed radial spin-orbit component and incomplete potential coverage.",
        "This is not a Dirac reduction, spectral splitting, or fine-structure derivation.",
        Scale1PerformanceClass::Light),
    S1_QUANTUM_EFFECTIVE("s1-quantum-spin-orbit-antiparallel", "Spin-orbit antiparallel control",
        "quantum_spin_orbit_antiparallel",
        "The matched moving record reverses its injected spin axis relative to orbital angular momentum.",
        "Spin-axis reversal reverses the signed spin-orbit force component.",
        "Orientation reversal in an imposed force law is not a quantum energy eigenstate.",
        Scale1PerformanceClass::Light),
    S1_QUANTUM_EFFECTIVE("s1-quantum-dipole-antiparallel", "Antiparallel dipole control",
        "quantum_dipole_antiparallel",
        "Two effective moments with opposed axial spin directions isolate the point-dipole geometry.",
        "Nonzero dipole component, injected axes, and incomplete dipole-potential coverage.",
        "Injected axes and g=2 point moments are not a recovered Pauli magnetic moment.",
        Scale1PerformanceClass::Light),
    S1_QUANTUM_EFFECTIVE("s1-quantum-dipole-transverse", "Transverse dipole control",
        "quantum_dipole_transverse",
        "Two transverse aligned effective moments test the angular branch of the point-dipole kernel.",
        "A nonzero force distinct from the axial geometry.",
        "Classical point-dipole angular response is not spin quantization.",
        Scale1PerformanceClass::Light),
    S1_QUANTUM_EFFECTIVE("s1-quantum-lorentz-charge-control", "Lorentz charge-sign control",
        "quantum_lorentz_charge_control",
        "Matched positive and negative moving records sample one fixed effective dipole field.",
        "Opposite charge labels reverse the signed Lorentz response.",
        "The imported v cross B response contains no photon state, gauge amplitude, or Landau spectrum.",
        Scale1PerformanceClass::Light),
    S1_QUANTUM_EFFECTIVE("s1-quantum-lorentz-velocity-control", "Lorentz velocity-sign control",
        "quantum_lorentz_velocity_control",
        "Matched charges with reversed velocities sample one fixed effective dipole field.",
        "Velocity reversal reverses the signed Lorentz response.",
        "A classical velocity-sign control is not quantum magnetic transport.",
        Scale1PerformanceClass::Light),
    S1_QUANTUM_EFFECTIVE("s1-quantum-radiation-scattering", "Accelerated-charge radiation control",
        "quantum_radiation_scattering",
        "A charged projectile is Coulomb-deflected while the imported acceleration-dependent sink is active.",
        "A nonzero post-acceleration radiation component and explicit missing sink coverage.",
        "There is no emitted photon record, bremsstrahlung spectrum, recoil quantum, or QED amplitude.",
        Scale1PerformanceClass::Standard),
    S1_QUANTUM_EFFECTIVE("s1-quantum-relativistic-counterstream", "Relativistic counterstream control",
        "quantum_relativistic_counterstream",
        "Two neutral effective records counterpropagate under the momentum-form relativistic integrator.",
        "Opposed sub-ceiling velocities, finite momenta, and force-free translation.",
        "Relativistic numerical kinematics is not a quantum wave packet or Lorentz-group recovery.",
        Scale1PerformanceClass::Light),
    S1_QUANTUM_EFFECTIVE("s1-quantum-color-triplet", "Three-color force toy",
        "quantum_color_triplet",
        "Three neutral, parametrically typed records exercise the engine's three-color force branches.",
        "Three distinct color labels and a nonzero strong-force component.",
        "The catalog labels, integer-charge engine records, and pair kernel are not QCD, a gauge-field state, or native confinement recovery.",
        Scale1PerformanceClass::Standard),

    // C. QED-facing effective references. These separate visualizable
    // classical/effective sectors from quantum-amplitude and loop types that
    // ParticleEngine does not own.
    S1_QED_EFFECTIVE("s1-qed-static-coulomb", "Static Coulomb sector",
        "qed_static_coulomb",
        "Two fixed opposite effective charges expose the softened electrostatic field and potential.",
        "Coulomb force direction, E-field streamlines, potential, and pair energy.",
        "This is an imported effective electrostatic limit, not photon exchange or QED recovery.",
        Scale1PerformanceClass::Light),
    S1_QED_EFFECTIVE("s1-qed-moller-reference", "Moller-style elastic reference",
        "qed_moller_reference",
        "Two equal negative effective records approach with a finite impact parameter under the Coulomb kernel.",
        "Like-charge deflection, reciprocal impulse, and covered state energy.",
        "No identical-fermion exchange amplitude, antisymmetrization, or Moller cross-section is computed.",
        Scale1PerformanceClass::Light),
    S1_QED_EFFECTIVE("s1-qed-bhabha-reference", "Bhabha-style elastic reference",
        "qed_bhabha_reference",
        "Opposite effective charges approach with contact removal disabled, isolating the elastic Coulomb comparison.",
        "Opposite-charge deflection, reciprocal impulse, and covered state energy.",
        "No s-channel annihilation, interference amplitude, or Bhabha cross-section is computed.",
        Scale1PerformanceClass::Light),
    S1_QED_EFFECTIVE("s1-qed-magnetic-dipole", "Magnetic dipole sector",
        "qed_magnetic_dipole",
        "Two spin-axis records isolate the imposed point-dipole interaction.",
        "Dipole force direction, spin axes, motion, and incomplete coverage mask.",
        "Injected spin axes and the point-dipole kernel are not a derived Pauli/QED magnetic moment.",
        Scale1PerformanceClass::Light),
    S1_QED_EFFECTIVE("s1-qed-lorentz-dipole", "Lorentz magnetic response",
        "qed_lorentz_dipole",
        "A moving effective charge passes the magnetic field of a fixed spin-axis source.",
        "Velocity-orthogonal deflection and the Lorentz force contribution.",
        "This imported v cross B response has no dynamical photon field or closed field-energy ledger.",
        Scale1PerformanceClass::Light),
    S1_QED_EFFECTIVE("s1-qed-spin-orbit", "Spin-orbit sector",
        "qed_spin_orbit",
        "A spin-axis-bearing charge orbits a fixed opposite charge with the imposed L dot S term enabled.",
        "Coulomb orbit, spin axis, spin-orbit force, and missing-potential coverage.",
        "This is an imposed effective L dot S toy, not a Dirac reduction or fine-structure derivation.",
        Scale1PerformanceClass::Standard),
    S1_QED_EFFECTIVE("s1-qed-radiation-reaction", "Radiation-reaction sector",
        "qed_radiation_reaction",
        "An accelerating effective charge follows a Coulomb orbit with the imported radiation-reaction sink enabled.",
        "Orbit deformation, net-force change, and radiation-sink incompleteness.",
        "No emitted photon record, spectrum, recoil quantum, or exact radiated-energy channel is present.",
        Scale1PerformanceClass::Standard),
    // D. Effective and parametric reference laboratory.  These are useful
    // comparison instruments, not evidence that their laws emerged natively.
    S1_EFFECTIVE("s1-charge-sign-matrix", "Charge-sign interaction matrix", "Effective reference dynamics",
        "charge_sign_matrix", "Four isolated effective pairs compare like- and opposite-sign response.",
        "Pair force direction and reciprocal impulse.", "Imported effective force is not native charge recovery.",
        Scale1PerformanceClass::Light),
    S1_EFFECTIVE("s1-coulomb-orbit", "Effective charge orbit", "Effective reference dynamics",
        "coulomb_orbit", "Light negative record orbiting a locked positive anchor in the geometric-tail window.",
        "Orbit, force decomposition, and energy coverage.", "The alpha coupling and initial condition are imported/imposed.",
        Scale1PerformanceClass::Light),
    S1_EFFECTIVE("s1-open-terminal-battery", "Directed rectangular battery discharge", "Effective reference dynamics",
        "open_terminal_battery",
        "A charge-neutral effective source places one mobile electron in the negative-electrode launch channel inside a rectangular perfect-insulator surface.",
        "During the imposed discharge orientation, electrons leave only through the negative port and may enter only through the positive port; every other wall or port encounter reflects.",
        "Terminal direction, the initial carrier velocity, walls, and locked sources are imposed effective-environment records; this is not electrochemistry, a closed conducting circuit, or substrate-derived battery behavior.",
        Scale1PerformanceClass::Standard),
    S1_EFFECTIVE("s1-finite-port-field-battery", "Finite-port Gauss field battery", "Effective reference dynamics",
        "finite_port_gauss_battery",
        "An isolated FTD-0884 finite ready-port bank advances a matched Gauss field against an imposed sign-preserving quadratic source battery.",
        "Finite capacity, field/port/battery energy ledger, exact finite-horizon reversal, and the no-production-coupling firewalls.",
        "This is a theorem-backed isolated EFT reference: the source law and scale remain imposed, and no moving-source, production ParticleEngine coupling, photon, Born weight, or indefinite recycler is supplied.",
        Scale1PerformanceClass::Light),
    S1_EFFECTIVE("s1-cluster-pair", "A Pair of Orbiting Charges", "Effective reference dynamics",
        "cluster_pair", "Two synthetic opposite effective cluster summaries in mutual orbit.",
        "Pair trajectory, force decomposition, and energy coverage.",
        "Synthetic N*K_B records are not promoted native clusters.", Scale1PerformanceClass::Light),
    S1_EFFECTIVE("s1-rutherford-scattering", "Rutherford-style scattering", "Effective reference dynamics",
        "rutherford_scattering", "Imported Coulomb-form impact-parameter comparison experiment.",
        "Deflection angle, impulse, and conservation coverage.", "This is not a substrate-derived cross-section.",
        Scale1PerformanceClass::Standard),
    S1_EFFECTIVE("s1-force-decomposition", "Pair-force decomposition", "Effective reference dynamics",
        "force_decomposition", "Separate Coulomb and selected gravity terms for one effective pair.",
        "Per-term vectors, net force, potential coverage.", "Display gain must not alter dynamics.",
        Scale1PerformanceClass::Light),
    S1_EFFECTIVE("s1-three-body", "Three-body chaos", "Effective reference dynamics",
        "three_body", "Three continuous effective records under the imported pair kernel.",
        "Trajectory divergence and conservation coverage.", "Chaotic motion is not native three-particle emergence.",
        Scale1PerformanceClass::Standard),
    S1_EFFECTIVE("s1-relativistic-integrator", "Relativistic-momentum integrator", "Effective reference dynamics",
        "relativistic_integrator", "Compare the selected momentum-form Verlet integrator near the speed clamp.",
        "Momentum, beta, projection sink, and step stability.", "Numerical momentum form is not Lorentz recovery.",
        Scale1PerformanceClass::Standard),
    S1_EFFECTIVE("s1-damping-sink", "Bath damping sink ledger", "Effective reference dynamics",
        "damping_sink", "Imposed bath damping with explicit non-conservative sink accounting.",
        "Mechanical-energy change and cumulative damping sink.", "Bath-frame damping is not native vacuum friction.",
        Scale1PerformanceClass::Light),
    S1_EFFECTIVE("s1-contact-selection", "Contact-event selection", "Effective reference dynamics",
        "contact_selection", "Deterministic closest-first disjoint contact-removal event selection.",
        "Selected pairs, event order, and contact energy delta.", "Contact removal is not QED annihilation or decay.",
        Scale1PerformanceClass::Light),
    S1_EFFECTIVE("s1-advanced-force-isolation", "Quarantined-force isolation", "Effective reference dynamics",
        "advanced_force_isolation", "Exercise one imposed extension at a time with incomplete ledgers visible.",
        "Term force and missing coverage mask.", "Quarantined toys cannot be called verified FTD physics.",
        Scale1PerformanceClass::Standard),
    S1_EFFECTIVE("s1-incomplete-conservation", "Incomplete-conservation demonstration", "Effective reference dynamics",
        "incomplete_conservation", "Demonstrate why an uncovered active term disables total-energy drift claims.",
        "Missing coverage mask and drift-eligibility transition.", "A partial sum must not be labeled total conserved energy.",
        Scale1PerformanceClass::Light),
    S1_CATALOG("s1-empty-zoo", "Catalog injection sandbox", "empty_zoo",
        "Empty reference scene accepting explicitly parametric catalog records.", "Catalog provenance and injected labels.",
        "Catalog identity is not lattice-derived identity."),
    S1_CATALOG("s1-parametric-species", "Parametric species comparison", "parametric_species",
        "Compare a small set of imported catalog masses and quantum labels.", "Mass and label ratios from the catalog.",
        "No mass, charge, spin, or species emergence claim."),
    S1_CATALOG("s1-mass-ladder", "Parametric mass ladder", "mass_ladder",
        "Visual mass-scale ladder over imported reference records.", "Catalog mass ordering and rendering scale.",
        "A displayed numerical relation is not an FTD derivation."),
    };

    for (auto& scenario : scenarios) {
        const std::string_view id = scenario.id;
        if (id == "s1-qed-static-coulomb") {
            scenario.behavior = Scale1ScenarioBehavior::StaticField;
            scenario.interactive = false;
        } else if (id == "s1-quantum-exchange-spinless-control") {
            scenario.behavior = Scale1ScenarioBehavior::NullControl;
            scenario.paired_scenario_id = "s1-quantum-exchange-eligible";
        } else if (id == "s1-quantum-exchange-eligible") {
            scenario.paired_scenario_id = "s1-quantum-exchange-spinless-control";
        } else if (id == "s1-empty-zoo") {
            scenario.behavior = Scale1ScenarioBehavior::AwaitingInput;
        } else if (id == "s1-parametric-species" || id == "s1-mass-ladder") {
            scenario.behavior = Scale1ScenarioBehavior::StaticReference;
            scenario.interactive = false;
        }
    }
    return scenarios;
}();

#undef S1_REPLAY
#undef S1_EFFECTIVE
#undef S1_CATALOG
#undef S1_QED_EFFECTIVE
#undef S1_QUANTUM_EFFECTIVE

bool finite_vec3(const Vec3& v) {
    return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
}

}  // namespace

const char* scale1_mode_id(Scale1Mode v) {
    switch (v) {
        case Scale1Mode::NativeMatter: return "native_matter";
        case Scale1Mode::EffectiveLab: return "effective_lab";
        case Scale1Mode::CatalogReference: return "catalog_reference";
    }
    return "unknown";
}

const char* scale1_owner_id(Scale1DynamicsOwner v) {
    switch (v) {
        case Scale1DynamicsOwner::NativeMatterObserver: return "native_matter_observer";
        case Scale1DynamicsOwner::ParticleEngine: return "particle_engine";
        case Scale1DynamicsOwner::Catalog: return "catalog";
    }
    return "unknown";
}

const char* scale1_workspace_id(Scale1Workspace v) {
    switch (v) {
        case Scale1Workspace::ParticleObservatory: return "particle_observatory";
        case Scale1Workspace::QuantumReference: return "quantum_reference";
        case Scale1Workspace::QedReference: return "qed_reference";
        case Scale1Workspace::ReferenceLaboratory: return "reference_laboratory";
    }
    return "unknown";
}

const char* scale1_scenario_class_id(Scale1ScenarioClass v) {
    switch (v) {
        case Scale1ScenarioClass::LiveNative: return "live_native";
        case Scale1ScenarioClass::QualifiedReplay: return "qualified_replay";
        case Scale1ScenarioClass::FalsificationReplay: return "falsification_replay";
        case Scale1ScenarioClass::SelectedCandidate: return "selected_candidate";
        case Scale1ScenarioClass::EffectiveReference: return "effective_reference";
        case Scale1ScenarioClass::ParametricCatalog: return "parametric_catalog";
    }
    return "unknown";
}

const char* scale1_scenario_behavior_id(Scale1ScenarioBehavior v) {
    switch (v) {
        case Scale1ScenarioBehavior::Dynamic: return "dynamic";
        case Scale1ScenarioBehavior::ReadOnlyReplay: return "read_only_replay";
        case Scale1ScenarioBehavior::StaticField: return "static_field";
        case Scale1ScenarioBehavior::NullControl: return "null_control";
        case Scale1ScenarioBehavior::AwaitingInput: return "awaiting_input";
        case Scale1ScenarioBehavior::StaticReference: return "static_reference";
    }
    return "unknown";
}

const char* scale1_performance_class_id(Scale1PerformanceClass v) {
    switch (v) {
        case Scale1PerformanceClass::Light: return "light";
        case Scale1PerformanceClass::Standard: return "standard";
        case Scale1PerformanceClass::Heavy: return "heavy";
    }
    return "unknown";
}

const char* scale1_validation_state_id(Scale1ValidationState v) {
    switch (v) {
        case Scale1ValidationState::ContractQualified: return "contract_qualified";
        case Scale1ValidationState::KernelValidated: return "kernel_validated";
        case Scale1ValidationState::ConditionalEvidence: return "conditional_evidence";
        case Scale1ValidationState::BoundaryConfirmed: return "boundary_confirmed";
        case Scale1ValidationState::OpenBlocked: return "open_blocked";
        case Scale1ValidationState::InvalidRetired: return "invalid_retired";
    }
    return "unknown";
}

const char* scale1_status_id(Scale1EpistemicStatus v) {
    switch (v) {
        case Scale1EpistemicStatus::Axiom: return "axiom";
        case Scale1EpistemicStatus::Theorem: return "theorem";
        case Scale1EpistemicStatus::Derived: return "derived";
        case Scale1EpistemicStatus::Selection: return "selection";
        case Scale1EpistemicStatus::Imposed: return "imposed";
        case Scale1EpistemicStatus::Measured: return "measured";
        case Scale1EpistemicStatus::Parametric: return "parametric";
        case Scale1EpistemicStatus::StronglyMotivatedConjecture: return "smc";
        case Scale1EpistemicStatus::Open: return "open";
        case Scale1EpistemicStatus::ClosedNegative: return "closed_negative";
        case Scale1EpistemicStatus::Invalid: return "invalid";
    }
    return "unknown";
}

const char* scale1_qualification_id(Scale1Qualification v) {
    switch (v) {
        case Scale1Qualification::NotEvaluated: return "not_evaluated";
        case Scale1Qualification::QualifiedSelected: return "qualified_selected";
        case Scale1Qualification::HeuristicCandidate: return "heuristic_candidate";
        case Scale1Qualification::Failed: return "failed";
        case Scale1Qualification::InvalidEvidence: return "invalid_evidence";
    }
    return "unknown";
}

const char* scale1_tier_id(Scale1ModuleTier v) {
    switch (v) {
        case Scale1ModuleTier::VerifiedBaseline: return "verified_baseline";
        case Scale1ModuleTier::SelectedExtension: return "selected_extension";
        case Scale1ModuleTier::ExperimentalQuarantined: return "experimental_quarantined";
        case Scale1ModuleTier::Retired: return "retired";
    }
    return "unknown";
}

const char* scale1_event_id(Scale1EventType v) {
    switch (v) {
        case Scale1EventType::ContactRemoval: return "contact_removal";
        case Scale1EventType::Formation: return "formation";
        case Scale1EventType::Dissociation: return "dissociation";
        case Scale1EventType::RecordExpiry: return "record_expiry";
    }
    return "unknown";
}

const char* scale1_field_channel_id(Scale1FieldChannel v) {
    switch (v) {
        case Scale1FieldChannel::Actual: return "actual";
        case Scale1FieldChannel::SelectedBound: return "selected_bound";
        case Scale1FieldChannel::Residual: return "residual";
        case Scale1FieldChannel::Outgoing: return "outgoing";
        case Scale1FieldChannel::Background: return "background";
    }
    return "unknown";
}

const std::vector<Scale1PhysicsSpec>& scale1_physics_registry() { return kPhysics; }
const std::vector<Scale1CapabilitySpec>& scale1_capability_registry() { return kCapabilities; }
const std::vector<Scale1ObserverSpec>& scale1_observer_registry() { return kObservers; }
const std::vector<Scale1ScenarioSpec>& scale1_scenario_registry() { return kScenarios; }

const Scale1PhysicsSpec* find_scale1_physics_spec(std::string_view key) {
    for (const auto& spec : kPhysics) {
        if (key == spec.id || key == spec.toggle_name) return &spec;
    }
    return nullptr;
}

const Scale1ScenarioSpec* find_scale1_scenario_spec(std::string_view id) {
    for (const auto& spec : kScenarios) {
        if (id == spec.id) return &spec;
    }
    return nullptr;
}

Scale1Snapshot NativeMatterObserver::m3_registered_replay() {
    Scale1Snapshot snapshot;
    snapshot.core.tick = 312;
    snapshot.core.effective_dt = 0.0;
    snapshot.core.mode = Scale1Mode::NativeMatter;
    snapshot.core.workspace = Scale1Workspace::ParticleObservatory;
    snapshot.core.scenario_class = Scale1ScenarioClass::QualifiedReplay;
    snapshot.core.dynamics_owner = Scale1DynamicsOwner::NativeMatterObserver;
    snapshot.core.backend = "registered_artifact";
    snapshot.core.scenario = "s1-native-m3-replay";
    snapshot.core.source_revision = "FTD-0760:L321:<100>:center:tick312";
    snapshot.core.artifact_revision = "scale1-artifact-ftd0760-m3-v1";
    snapshot.core.read_only = true;

    constexpr double relative_z = 0.91112627615012798;
    constexpr double graph_margin = 0.66984890890880078;
    constexpr double energy_margin = 0.0026670284656429436;
    constexpr double pair_energy = -0.0026670284656429436;

    Scale1Provenance provenance;
    provenance.source_scale = 0;
    provenance.source_tick = 312;
    provenance.source_object_id = 1;
    provenance.source_kind = "registered_relational_matter_replay";
    provenance.source_scenario = "FTD-0760 center arm <100> L=321";
    provenance.source_revision = snapshot.core.source_revision;
    provenance.status = Scale1EpistemicStatus::Measured;
    provenance.qualification = Scale1Qualification::QualifiedSelected;

    Scale1ObjectRecord a;
    a.id = 0;
    a.effective_state = +1;
    a.position = {0.0, 0.0, -0.5 * relative_z};
    a.integer_center = {0.0, 0.0, 0.0};
    a.fractional_center = {0.0, 0.0, -0.5 * relative_z};
    a.integer_center_available = true;
    a.fractional_center_available = true;
    a.momentum = {2.0334217669631263e-13,
                  -1.9436672954592286e-13,
                  0.039808041560382444};
    a.constituent = true;
    a.identity_available = true;
    a.age_ticks = 312;
    a.manifestation_support_count = 2;
    a.constituent_count = 2;
    a.graph_margin = graph_margin;
    a.energy_margin = energy_margin;
    a.identity_margin = std::min(graph_margin, energy_margin);
    a.parent_ids = {1};
    a.provenance = provenance;

    Scale1ObjectRecord b = a;
    b.id = 1;
    b.effective_state = -1;
    b.position.z = 0.5 * relative_z;
    b.fractional_center.z = 0.5 * relative_z;
    b.momentum = {-2.0332341233250192e-13,
                  1.9436987428123234e-13,
                  -0.039808041560393546};
    b.parent_ids = {0};
    snapshot.objects = {a, b};

    snapshot.fields.push_back({Scale1FieldChannel::Actual, true, true,
        -pair_energy, {}, "ftd0760_registered_summary", ""});
    snapshot.fields.push_back({Scale1FieldChannel::SelectedBound, true, true,
        -pair_energy, {}, "support_invariant_matter_observer", ""});
    snapshot.fields.push_back({Scale1FieldChannel::Residual, true, true,
        0.0, {}, "ftd0760_environmental_fibre_summary",
        "Summary only; FTD-0764--0767 prohibit a rigid-coat or persistent-wake reading."});
    snapshot.fields.push_back({Scale1FieldChannel::Outgoing, false, true,
        0.0, {}, "", "No certified persistent outgoing/radiation channel exists."});
    snapshot.fields.push_back({Scale1FieldChannel::Background, true, true,
        1.9073486328125e-06, {}, "ftd0760_causal_fibre_discriminator", ""});

    snapshot.conservation.potential_energy = pair_energy;
    snapshot.conservation.state_energy = pair_energy;
    snapshot.conservation.total_momentum = a.momentum + b.momentum;
    snapshot.conservation.covered_mask = 0;
    snapshot.conservation.missing_mask = scale1_bit(Scale1Coverage::Kinetic);
    snapshot.conservation.state_energy_complete = false;
    snapshot.conservation.drift_eligible = false;

    snapshot.capability_ids = {"native_matter_replay"};
    snapshot.unavailable_reasons = {
        "Read-only registered evidence: no live production generator is claimed.",
        "Mass, conserved charge, spin, statistics, particle poles, and SM identity are unavailable."
    };
    snapshot.particle_count = static_cast<int>(snapshot.objects.size());
    snapshot.total_energy = snapshot.conservation.state_energy;
    snapshot.total_pe = pair_energy;
    snapshot.status =
        "FTD-0760 selected finite-time M3 matter replay; not an SM particle";
    return snapshot;
}

Scale1Snapshot NativeMatterObserver::observe_source_clusters(
    const std::vector<Scale1SourceClusterRecord>& sources,
    std::int64_t source_tick,
    const std::string& source_scenario,
    const std::string& source_revision) {
    Scale1Snapshot snapshot;
    snapshot.core.tick = source_tick;
    snapshot.core.effective_dt = 0.0;
    snapshot.core.mode = Scale1Mode::NativeMatter;
    snapshot.core.workspace = Scale1Workspace::ParticleObservatory;
    snapshot.core.scenario_class = Scale1ScenarioClass::LiveNative;
    snapshot.core.dynamics_owner = Scale1DynamicsOwner::NativeMatterObserver;
    snapshot.core.backend = "coherent_scale0_capture";
    snapshot.core.scenario = "native-observer-record";
    snapshot.core.source_revision = source_revision;
    snapshot.core.artifact_revision = "ephemeral-coherent-capture";
    snapshot.core.read_only = true;

    bool any_field_summary = false;
    snapshot.objects.reserve(sources.size());
    for (const auto& source : sources) {
        if (source.lattice_size <= 0 || source.manifestation_count <= 0
            || source.state_sign == 0 || !finite_vec3(source.centroid)
            || !finite_vec3(source.centroid_velocity)
            || !std::isfinite(source.display_scale) || source.display_scale <= 0.0) {
            throw std::invalid_argument(
                "NativeMatterObserver received an inadmissible source cluster");
        }

        const double center = (source.lattice_size - 1) * 0.5;
        const Vec3 source_origin{center, center, center};
        Scale1ObjectRecord object;
        object.id = source.source_object_id;
        object.effective_state = source.state_sign;
        object.position = (source.centroid - source_origin) * source.display_scale;
        object.velocity = source.centroid_velocity;
        object.effective_radius = std::max(
            0.5, std::cbrt(3.0 * source.manifestation_count / (4.0 * PI))
                * source.display_scale);
        object.manifestation_support_count = source.manifestation_count;
        object.fractional_center = object.position;
        object.fractional_center_available = true;
        const Vec3 rounded{
            std::round(source.centroid.x),
            std::round(source.centroid.y),
            std::round(source.centroid.z),
        };
        object.integer_center = (rounded - source_origin) * source.display_scale;
        object.integer_center_available = true;
        object.identity_available = false;

        object.provenance.source_scale = 0;
        object.provenance.source_tick = source.source_tick;
        object.provenance.source_object_id = source.source_object_id;
        object.provenance.source_kind = "coherent_lattice_cluster_candidate";
        object.provenance.source_scenario = source.source_scenario.empty()
            ? source_scenario : source.source_scenario;
        object.provenance.source_revision = source.source_revision.empty()
            ? source_revision : source.source_revision;
        object.provenance.status = Scale1EpistemicStatus::Measured;
        object.provenance.qualification = Scale1Qualification::NotEvaluated;
        snapshot.objects.push_back(std::move(object));
        any_field_summary = any_field_summary || source.field_state_available;
    }

    snapshot.fields.push_back({Scale1FieldChannel::Actual, any_field_summary, true,
        0.0, {}, "coherent_scale0_cluster_summary",
        any_field_summary ? "Field availability is summarized; no Scale-1 field volume was copied."
                          : "Source capture carried no coherent field-state summary."});
    snapshot.fields.push_back({Scale1FieldChannel::SelectedBound, false, true,
        0.0, {}, "", "No bound-field observer has qualified this live capture."});
    snapshot.fields.push_back({Scale1FieldChannel::Residual, false, true,
        0.0, {}, "", "No residual-field decomposition was published for this capture."});
    snapshot.fields.push_back({Scale1FieldChannel::Outgoing, false, true,
        0.0, {}, "", "No outgoing/radiation channel was published for this capture."});
    snapshot.fields.push_back({Scale1FieldChannel::Background, false, true,
        0.0, {}, "", "No background-field volume was copied into Scale 1."});

    snapshot.conservation.missing_mask = scale1_bit(Scale1Coverage::Kinetic);
    snapshot.conservation.state_energy_complete = false;
    snapshot.conservation.drift_eligible = false;
    snapshot.capability_ids = {"native_matter_live"};
    snapshot.unavailable_reasons = {
        "Candidate capture only: identity continuity and particle qualification were not evaluated.",
        "Mass, kinetic energy, total momentum, field decomposition, spin, statistics, and species are unavailable."
    };
    if (sources.empty()) {
        snapshot.unavailable_reasons.push_back(
            "The coherent capture contained no manifested cluster candidates.");
    }
    snapshot.particle_count = static_cast<int>(snapshot.objects.size());
    snapshot.status = sources.empty()
        ? "Coherent Scale-0 capture: no candidate clusters"
        : "Coherent Scale-0 candidate observation; qualification not evaluated";
    return snapshot;
}

}  // namespace ftd
