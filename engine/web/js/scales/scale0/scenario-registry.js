
import { SCALE0_SCENARIO_VALIDATION } from './scenario-validation.js';
export { SCALE0_SCENARIO_VALIDATION } from './scenario-validation.js';



// Plain-language display names, keyed by scenario id. The dropdown shows these
// so the list reads in everyday English; the original precise title is kept as
// `sourceTitle` and surfaced in the scenario description panel (see
// updateScenarioMetadata in ui/bindings.js), so no physics precision is lost.
// Ids absent here fall back to their technical title (harmless). The `id` (the
// wiring key checked by the JS↔C++ parity guard) is never touched.
const LAYMAN_NAMES = {
    'empty': 'Empty Space',
    's0-seed-dynamical-flux-dressing': 'A Charge Builds Its Own Field',
    's0-seed-moving-source-reciprocity': 'A Nudged Charge Responds',
    'flux-pulse': 'A Wave Pulse Crossing the Box',
    'flux-dipole': 'Two Mirror-Image Wave Pulses',
    'flux-standing': 'A Standing Wave',
    'flux-nested-standing': 'Crossed Standing Waves',
    'flux-soliton': 'A Strong, Self-Holding Pulse',
    'flux-interference': 'A Rippling Interference Pattern',
    'flux-vortex': 'A Spinning Ring of Field',
    'flux-dual-substrate': 'A Mirror-Polarized Wave Pair',
    'flux-cascade': 'Matter Sparks from an Intense Field',
    'flux-random-genesis': 'Matter Sparks from Random Patches',
    'flux-genesis-between-gates': 'Matter Blinks On for One Instant',
    's0-seed-ew-phase-transition': 'A Steady Push on the Field',
    'flux-pair-production': 'A Particle-Antiparticle Pair Appears',
    'flux-annihilation': 'Opposite Particles Collide',
    'flux-vacuum-foam': 'A Ball of Random Waves',
    'flux-meson': 'Two Opposite Particles Pass By',
    'flux-string-breaking': 'Opposite Charges Fly Apart',
    'flux-baryon': 'Three Charges Spread Outward',
    'flux-cyclotron': 'A Charge Curves in a Magnetic Field',
    'flux-screening': 'A Shell of Alternating Charge',
    'flux-thermalization': 'Random Waves Spreading Out',
    'flux-triad': 'Three Inward Streams of Field',
    'flux-zero-point': 'A Restless Sea of Waves',
    'light-rainbow': 'Three Colors of Light',
    'light-dipole': 'Light Radiating Both Ways',
    'light-two-slit': 'Two Light Sources Overlapping',
    'light-photon-race': 'Do Bright and Dim Light Travel Alike?',
    'quantum-born-rule': 'A Burst of New Matter',
    'quantum-double-slit': 'A Two-Source Wave Pattern',
    'quantum-eraser': 'Checkerboard-Driven Waves',
    'quantum-tunnel': 'Waves Pushing Against a Barrier',
    'quantum-well': 'Waves Between Two Walls',
    'quantum-entangle': 'A Tagged Particle Pair',
    'quantum-aharonov-bohm': 'Waves Passing Around a Tube',
    'quantum-casimir': 'Waves Between Two Plates',
    'quantum-zeno': 'A Burst of Matter, Watched',
    's0-seed-up-quark': 'Up Quark',
    's0-seed-down-quark': 'Down Quark',
    's0-seed-strange-quark': 'Strange Quark',
    's0-seed-charm-quark': 'Charm Quark',
    's0-seed-bottom-quark': 'Bottom Quark',
    's0-seed-top-quark': 'Top Quark',
    's0-seed-anti-up-quark': 'Anti-Up Quark',
    's0-seed-anti-down-quark': 'Anti-Down Quark',
    's0-seed-anti-strange-quark': 'Anti-Strange Quark',
    's0-seed-anti-charm-quark': 'Anti-Charm Quark',
    's0-seed-anti-bottom-quark': 'Anti-Bottom Quark',
    's0-seed-anti-top-quark': 'Anti-Top Quark',
    's0-seed-higgs-field': 'The Higgs Field',
    's0-seed-gluon': 'A Gluon',
    's0-seed-beta-decay': 'Beta Decay',
    's0-seed-ee-annihilation': 'An Electron Meets a Positron',
    's0-seed-quark-gluon-plasma': 'Hot Quark Soup',
    's0-seed-hydrogen': 'A Hydrogen Atom',
    's0-seed-helium': 'A Helium Atom',
    's0-seed-h2-bond-formation': 'Two Atoms Trying to Bond',
    's0-seed-spark-of-life': 'A Spark of Life',
    's0-seed-wilson-loop': 'A Square Loop of Field',
    's0-seed-flux-tube': 'A Tube of Field',
    's0-seed-monopole': 'A Single Magnetic Pole',
    's0-seed-instanton': 'A Localized Field Knot',
    's0-seed-schwarzschild': 'A Black Hole Field',
    's0-seed-gravitational-lensing': 'Gravity Bending Light',
    's0-seed-gravitational-wave': 'A Gravitational Wave',
    's0-seed-massive-body': 'A Heavy Mass Bending Space',
    's0-seed-time-gravity-well': 'A Gravity Well',
    's0-seed-time-twin-clocks': 'Twin Clocks',
    's0-seed-time-horizon': 'An Event Horizon',
    's0-seed-sloop': 'A Twelve-Point Ring',
    's0-seed-observer-cell': 'An Alternating Shell Cell',
    's0-field-plane-wave': 'A Traveling Wave',
    's0-field-standing-wave': 'A Standing Wave (Field)',
    's0-field-uniform-e': 'A Uniform Electric Field',
    's0-field-uniform-b': 'A Uniform Magnetic Field',
    's0-field-photon-pulse': 'A Pulse of Light',
    's0-field-rf-lattice-wave': 'A Radio-Wave Mode',
    's0-field-light-lattice-wave': 'A Light-Wave Mode',
    's0-field-sound-lattice-wave': 'A Sound-Wave Mode',
    's0-field-sound-collision': 'Two Sound Pulses Overlapping',
    's0-field-thomson-scattering': 'Light Scattering off a Charge',
    's0-field-thomson-unlocked-recoil': 'A Charge Recoiling from Light',
    's0-field-spacetime-forcing-boundary': 'How Far a Nudge Reaches',
    's0-field-electric-dipole': 'An Electric Dipole',
    's0-field-magnetic-dipole': 'A Magnetic Dipole',
    's0-field-vortex-line': 'A Vortex Line',
    's0-seed-octahedron': 'An Octahedron Shell',
    's0-seed-cuboctahedron': 'A Cuboctahedron Shell',
    's0-seed-stella-octangula': 'A Star-Tetrahedron Shell',
    's0-seed-moore-cell': 'The 27-Cell Neighborhood',
    's0-seed-moore-decomposition': 'The Neighborhood, Layer by Layer',
    's0-seed-emergent-ic1': 'Matter from a Single-Axis Burst',
    's0-seed-emergent-ic3-collision': 'Two Bursts Colliding',
    's0-seed-emergent-ic4-subthreshold': 'A Quiet, Below-Threshold Bath',
    's0-seed-emergent-ic2-thermal-runaway': 'A Hot, Empty Bath',
    's0-seed-emergent-ic1-diagonal': 'Matter from a Diagonal Burst',
    's0-seed-emergent-ic1-isotropic': 'Matter from an All-Directions Burst',
    's0-seed-emergent-ic1-viz': 'A Single-Axis Burst, Fading',
    's0-seed-emergent-ic1-diagonal-viz': 'A Diagonal Burst, Fading',
    's0-seed-emergent-ic1-isotropic-viz': 'An All-Directions Burst, Fading',
    's0-seed-cluster-law': 'Make Matter: Dial the Intensity',
    's0-seed-cluster-law-subknee': 'Matter Burst — Gentle',
    's0-seed-cluster-law-knee': 'Matter Burst — Medium',
    's0-seed-cluster-law-superknee': 'Matter Burst — Strong',
    's0-vacuum-electron': 'An Electron',
    's0-vacuum-muon': 'A Muon',
    's0-vacuum-tau': 'A Tau',
    's0-vacuum-positron': 'A Positron',
    's0-vacuum-antimuon': 'An Antimuon',
    's0-vacuum-antitau': 'An Antitau',
    's0-vacuum-electron-neutrino': 'An Electron Neutrino',
    's0-vacuum-muon-neutrino': 'A Muon Neutrino',
    's0-vacuum-tau-neutrino': 'A Tau Neutrino',
    's0-vacuum-electron-antineutrino': 'An Electron Antineutrino',
    's0-vacuum-muon-antineutrino': 'A Muon Antineutrino',
    's0-vacuum-tau-antineutrino': 'A Tau Antineutrino',
    's0-vacuum-photon': 'A Photon',
    's0-vacuum-w-boson': 'A W+ Boson',
    's0-vacuum-w-minus-boson': 'A W- Boson',
    's0-vacuum-z-boson': 'A Z Boson',
    's0-vacuum-higgs': 'A Higgs Boson',
    's0-vacuum-proton': 'A Proton',
    's0-vacuum-neutron': 'A Neutron',
    's0-vacuum-pion-charged': 'A Charged Pion',
    's0-vacuum-pion-neutral': 'A Neutral Pion',
    's0-vacuum-kaon-charged': 'A Charged Kaon',
    's0-seed-de-broglie-clock': 'The Clock Inside a Particle',
    's0-seed-thermal-ignition': 'A Warm, Below-Threshold Bath',
};

function makeScenario(category, id, title, tags = [], epistemicStatus = '[OPEN]') {
    const validation = SCALE0_SCENARIO_VALIDATION[id] || null;
    const admitted = validation?.level === 'behavioral';
    const qualification = admitted
        ? validation.qualification
        : 'RESEARCH SETUP — mechanically smoke-tested only; advertised behavior and physical identity are unvalidated';
    const laymanTitle = LAYMAN_NAMES[id] || null;
    const displayBase = laymanTitle || title;  // plain name if we have one, else the technical title
    // Honest visibility marker. The old verbose titles carried "… Identity
    // Rejected / Gate Failed" inline; a plain name like "An Electron" would drop
    // that and read as a confirmed result. So when a scenario's labeled physical
    // identity was tested and rejected ([CLOSED NEGATIVE]), keep a short "(model)"
    // tag on the plain name. Full status (technical name + epistemic tag +
    // validation notes) lives in the description panel.
    const identityRejected = /\[CLOSED NEGATIVE\]/.test(epistemicStatus);
    return {
        id,
        scale: 'lattice',
        sourceTitle: title,          // original technical name — preserved, shown in the description
        laymanTitle,                 // plain-language name (null if this id wasn't renamed)
        title: identityRejected ? `${displayBase} (model)` : displayBase,
        category,
        tags,
        defaultParams: {},
        requiredCapabilities: ['scale0'],
        epistemicStatus,
        admissionStatus: admitted ? 'admitted-behavioral' : 'hidden-research',
        evidenceLevel: admitted ? 'behavioral' : 'mechanical-smoke-only',
        qualification,
        validation,
        mechanicalTest: 'engine/web/tests/scale0-scenario-health.spec.js',
        load(harness, params = {}) {
            harness.setupScenario(params.id || id);
        },
    };
}

export const SCALE0_SCENARIO_CATALOG = [
    /*
     * Scenario: empty (Empty Lattice)
     * Physical purpose: Serves as the baseline state of the lattice with no initial particles or fields.
     * Parameters: None.
     * Expected behavior: The lattice remains completely quiet and empty.
     * Discrepancy: None.
     */
    makeScenario('1. Validated Native Dynamics', 'empty', 'Empty Lattice — Null Control', ['baseline'], '[AXIOM]'),
    /*
     * Scenario: s0-seed-dynamical-flux-dressing
     * Physical purpose: Visualizes a flux response generated by a manifested
     * polarity from zero initial field through the existing coupling operator.
     * Expected behavior: an outward, causal, polarity-sourced field develops.
     * Discrepancy: attachment, wake, radiation, EM identity, and quantization
     * are separate FTD-0476 gates and are not implied by the streamline image.
     */
    makeScenario('1. Validated Native Dynamics', 's0-seed-dynamical-flux-dressing', 'Dynamical Flux Dressing — Native Source Probe', ['field', 'flux', 'polarity', 'dressing', 'locality'], '[EMERGENT] source-built field in the restricted native wave/coupling sector'),
    /*
     * Scenario: s0-seed-moving-source-reciprocity
     * Physical purpose: FTD-0477 caused-motion and reciprocity discriminator.
     * The source begins at rest; a separate finite packet drives only the
     * selected flux-gradient force extension.
     * Discrepancy: this force was rejected as ordinary qE by FTD-0435, so the
     * scenario cannot establish electromagnetism or radiation by appearance.
     */
    makeScenario('3. Qualified Selected Extensions', 's0-seed-moving-source-reciprocity', 'Driven Polarity — Sub-voxel Response', ['field', 'flux', 'polarity', 'response', 'reciprocity'], '[QUALIFIED NEGATIVE] 0.203598-cell response; no hop, wake, detached field, or closed reciprocity'),
    /*
     * Scenario: flux-pulse (Localized Transverse Boundary Probe)
     * Physical purpose: Measures a divergence-free packet against the implemented finite-box boundary operators.
     * Parameters: None.
     * Expected behavior: Periodic Hamiltonian conservation and Neumann-shell momentum reversal.
     * Discrepancy: The lossy shell retained 52.9% of field norm at tick 90, failing the preregistered 75%-removal gate. These are computational finite-box laws, not physical boundaries of the ontology.
     */
    makeScenario('1. Validated Native Dynamics', 'flux-pulse', 'Transverse Packet — Finite-Box Boundary Test', ['flux', 'wave'], '[EMERGENT] under [IMPOSED] computational boundary laws'),
    /*
     * Scenario: flux-dipole (Antisymmetric Wave Pair)
     * Physical purpose: Tests odd-reflection parity under the isolated wave map.
     * Parameters: None.
     * Expected behavior: Exact odd x parity is preserved without manifestation.
     * Discrepancy: No electromagnetic dipole identity is tested.
     */
    makeScenario('1. Validated Native Dynamics', 'flux-dipole', 'Antisymmetric Gaussian Wave Pair', ['flux', 'wave'], '[EMERGENT] parity preservation under the native wave map'),
    /*
     * Scenario: flux-standing (Reflection-Even Broadband Wave Pair)
     * Physical purpose: Tests even-reflection parity from zero initial wave momentum.
     * Parameters: None.
     * Expected behavior: Even x parity is preserved under native evolution.
     * Discrepancy: The Gaussian pair is broadband, not a pure standing eigenmode.
     */
    makeScenario('1. Validated Native Dynamics', 'flux-standing', 'Reflection-Even Broadband Wave Pair', ['flux', 'wave'], '[EMERGENT] parity preservation under the native wave map'),
    /*
     * Scenario: flux-nested-standing (Orthogonal Reflection-Even Wave Pairs)
     * Physical purpose: Tests reflection preservation for two orthogonal broadband pairs.
     * Parameters: None.
     * Expected behavior: Exact even x/z parity under the isolated native wave map.
     * Discrepancy: The Gaussian pairs are not pure standing eigenmodes.
     */
    makeScenario('1. Validated Native Dynamics', 'flux-nested-standing', 'Orthogonal Reflection-Even Wave Pairs', ['flux', 'wave'], '[EMERGENT] parity preservation under the native wave map'),
    /*
     * Scenario: flux-soliton (High-Amplitude Packet Dispersion)
     * Physical purpose: Tests whether a high-amplitude packet disperses under the native wave map.
     * Parameters: None.
     * Expected behavior: Packet translation and lattice dispersion without manifestation.
     * Discrepancy: No soliton-generating nonlinearity is present, so this is not a soliton solution.
     */
    makeScenario('1. Validated Native Dynamics', 'flux-soliton', 'High-Amplitude Packet — Native Dispersion Test', ['flux', 'wave'], '[EMERGENT] under the isolated linear wave map'),
    /*
     * Scenario: flux-interference (Four-Lobe Symmetric Wave Field)
     * Physical purpose: Tests two-axis reflection preservation from four Gaussian lobes.
     * Parameters: None.
     * Expected behavior: Exact even x/z parity under isolated native evolution.
     * Discrepancy: No detector fringe law or physical interference identification is tested.
     */
    makeScenario('1. Validated Native Dynamics', 'flux-interference', 'Four-Lobe Reflection-Symmetric Wave Field', ['flux', 'wave'], '[EMERGENT] parity preservation under the native wave map'),
    /*
     * Scenario: flux-vortex (Helical Ring Vector Ansatz)
     * Physical purpose: Provides a discrete ring with imposed circulation and axial bias.
     * Parameters: None.
     * Expected behavior: Exact static three-plane support and oriented circulation.
     * Discrepancy: It demonstrates neither spin nor dynamic rotation.
     */
    makeScenario('1. Validated Native Dynamics', 'flux-vortex', 'Helical Ring — Exact Vector Ansatz', ['flux', 'geometry'], '[IMPOSED]'),
    /*
     * Scenario: flux-dual-substrate (Mirror-Polarized Wave Pair)
     * Physical purpose: Tests mixed component parity for two mirrored Gaussian wave blobs.
     * Parameters: None.
     * Expected behavior: x-even and y/z-odd parity is preserved by the native wave map.
     * Discrepancy: The dual_substrate operator is off; this is not a two-sector simulation.
     */
    makeScenario('1. Validated Native Dynamics', 'flux-dual-substrate', 'Mirror-Polarized Wave Pair — Dual Sector Not Engaged', ['flux', 'wave'], '[EMERGENT] mixed component parity under the native wave map'),
    /*
     * Scenario: flux-cascade (Supercritical Gaussian Genesis Cohort)
     * Physical purpose: Measures the first selected-law genesis response to one supercritical Gaussian field.
     * Parameters: None.
     * Expected behavior: Exact fixed-seed single-site +/- cohort on tick one.
     * Discrepancy: There is no cascade, branching, recruitment, or pair-production mechanism in this profile.
     */
    makeScenario('1. Validated Native Dynamics', 'flux-cascade', 'Supercritical Gaussian Genesis Cohort', ['genesis', 'cohort'], '[EMERGENT] under the [SELECTION] local genesis law'),
    /*
     * Scenario: flux-random-genesis (Fixed-Seed Random-Patch Genesis Cohort)
     * Physical purpose: Measures the first selected-law genesis response to eight fixed-seed random patches.
     * Parameters: Eight super-threshold patches; genesis only.
     * Expected behavior: Exact fixed-seed single-site +/- cohort on tick one.
     * Discrepancy: No pair production, vacuum fluctuation, or self-organization claim follows.
     */
    makeScenario('1. Validated Native Dynamics', 'flux-random-genesis', 'Fixed-Seed Random-Patch Genesis Cohort', ['genesis', 'random-seed', 'cohort'], '[EMERGENT] under the [SELECTION] local genesis law'),
    /*
     * Scenario: flux-genesis-between-gates (Genesis: Between the Gates)
     * Physical purpose: One-tick discriminator for the selected genesis law; exact initial cohorts at |J| = 1.5160 / 1.5250 / 1.5340 straddle K_GENESIS = 3·W_SC = 1.5164.
     * Parameters: All non-genesis physics and campaign threshold overrides are cleared; seed = 1.
     * Expected behavior: On the first tick, the lower cohort has zero hazard while the upper cohorts follow the compiled local Bernoulli hazards with ratio 2.0268.
     * Discrepancy: After accepted events, flux drain and evaporation invalidate the frozen-independent-cohort model; no sustained cascade is claimed.
     */
    makeScenario('1. Validated Native Dynamics', 'flux-genesis-between-gates', 'Genesis Gate — One-Tick Cohorts', ['genesis', 'ftd-0388'], '[EMERGENT] under [SELECTION] local genesis law'),
    makeScenario('2. Validated State Dynamics', 's0-seed-ew-phase-transition', 'Uniform Additive Drive + Genesis — Hysteresis/EW Claim Failed', ['drive', 'genesis', 'null-test'], '[EMERGENT] finite driven response; [CLOSED NEGATIVE] hysteresis/EW identity'),
    /*
     * Scenario: flux-pair-production (Native Polarity-Pair Rule — Cohort)
     * Physical purpose: One-tick test of the selected adjacent polarity-pair transition.
     * Parameters: 343 isolated +x flux sources at p=1/2; all other physics off; seed 1.
     * Expected behavior: Accepted sources become adjacent upstream -1/downstream +1 pairs with exact signed-polarity and vector-flux cancellation.
     * Discrepancy: This is not a derivation or validation of physical Schwinger production, particle identity, pair stability, or later-time dynamics.
     */
    makeScenario('1. Validated Native Dynamics', 'flux-pair-production', 'Native Polarity-Pair Rule — One-Tick Cohort', ['pair-production', 'polarity'], '[EMERGENT] under [SELECTION] pair-transition law'),
    /*
     * Scenario: flux-annihilation (Native Opposite-State Collision Rule)
     * Physical purpose: Exact two-tick test of the production collision-removal branch.
     * Parameters: Adjacent +/- states, one moving at C_SPEED, with cancelling pre-existing transverse flux; only movement enabled.
     * Expected behavior: Both states vanish and their pre-existing flux is spread over the two six-face shells.
     * Discrepancy: The rule has no rest-mass-to-flux conversion and creates no outgoing wave; physical annihilation is not established.
     */
    makeScenario('1. Validated Native Dynamics', 'flux-annihilation', 'Native Opposite-State Collision Rule', ['movement', 'polarity'], '[EMERGENT] collision behavior under the native movement rule'),
    /*
     * Scenario: flux-vacuum-foam (Finite Deterministic Random-Wave Ball)
     * Physical purpose: Tests exact replay and invariant preservation for a finite random wave seed.
     * Parameters: None.
     * Expected behavior: Deterministic source-free wave evolution.
     * Discrepancy: There is no ongoing noise source, quantum-vacuum mechanism, or spacetime foam.
     */
    makeScenario('1. Validated Native Dynamics', 'flux-vacuum-foam', 'Finite Deterministic Random-Wave Ball', ['wave', 'random-seed', 'invariant'], '[EMERGENT] source-free native wave evolution from [IMPOSED] random initial data'),
    /*
     * Scenario: flux-meson (Counter-Moving Opposite-State Pair)
     * Physical purpose: Tests native remainder/integer transport for two opposite states.
     * Parameters: None.
     * Expected behavior: Exact counter-directed free transport with inert field dressing.
     * Discrepancy: No colors, confinement term, binding, or meson identity is present.
     */
    makeScenario('2. Validated State Dynamics', 'flux-meson', 'Counter-Moving Opposite-State Pair', ['movement', 'polarity'], '[EMERGENT] native movement bookkeeping'),
    /*
     * Scenario: flux-string-breaking (Outward Opposite-Polarity Transport)
     * Physical purpose: Tests exact outward movement bookkeeping for a +/- pair.
     * Parameters: None.
     * Expected behavior: Separation increases with exactly two states.
     * Discrepancy: No string, tension, confinement, color, or pair-production mechanism is active.
     */
    makeScenario('2. Validated State Dynamics', 'flux-string-breaking', 'Outward Opposite-Polarity Transport — String Absent', ['movement', 'polarity', 'null-test'], '[EMERGENT] native movement; [CLOSED NEGATIVE] string-breaking interpretation'),
    /*
     * Scenario: flux-baryon (Threefold Tangential Transport)
     * Physical purpose: Tests movement bookkeeping for a threefold velocity seed and one stationary opposite marker.
     * Parameters: None.
     * Expected behavior: Exact face translations with all four markers unlocked.
     * Discrepancy: No binding, color, quark, or baryon identity is active.
     */
    makeScenario('2. Validated State Dynamics', 'flux-baryon', 'Threefold Tangential Free Transport', ['movement', 'polarity', 'threefold'], '[EMERGENT] native movement bookkeeping'),
    /*
     * Scenario: flux-cyclotron (Imposed-B Native Curvature Test)
     * Physical purpose: Measures the selected native velocity-cross-curl force against a no-Lorentz control.
     * Parameters: None.
     * Expected behavior: Resolved curvature toward -y with bounded unit-tick speed drift.
     * Discrepancy: The vector potential and force law are selected inputs; EM emergence and physical cyclotron identity are not established.
     */
    makeScenario('1. Validated Native Dynamics', 'flux-cyclotron', 'Imposed-B Native Curvature Test', ['field', 'polarity', 'lorentz-response'], '[EMERGENT] response under an [IMPOSED] vector potential and [SELECTED] force law'),
    /*
     * Scenario: flux-screening (Octahedral Polarity-Shell Seed)
     * Physical purpose: Displays one central positive state and the six-site negative face orbit.
     * Parameters: None.
     * Expected behavior: Exact inert 1+6 polarity geometry with imposed radial dressing.
     * Discrepancy: Net state is -5; no neutralization, dielectric response, or screening observable exists.
     */
    makeScenario('4. Validated Initial Data', 'flux-screening', 'Octahedral Polarity-Shell Seed', ['geometry', 'polarity', 'imposed-field'], '[IMPOSED] exact initial data'),
    /*
     * Scenario: flux-thermalization (Localized Random-Wave Mixing)
     * Physical purpose: Measures spatial spreading of a fixed-seed compact random wave patch.
     * Parameters: None.
     * Expected behavior: Linear propagation beyond the initial support with exact modified-H conservation.
     * Discrepancy: No thermostat, temperature, entropy observable, or equilibrium test exists.
     */
    makeScenario('1. Validated Native Dynamics', 'flux-thermalization', 'Localized Random-Wave Mixing', ['wave', 'random-seed', 'spreading'], '[EMERGENT] linear wave spreading from [IMPOSED] random initial data'),
    /*
     * Scenario: flux-triad (Threefold Inward-Flux Seed)
     * Physical purpose: Displays a prepared threefold polarity geometry with inward flux dressing.
     * Parameters: None.
     * Expected behavior: Exact inert initial data.
     * Discrepancy: No binding or stability dynamics, color structure, or baryon identity is active.
     */
    makeScenario('4. Validated Initial Data', 'flux-triad', 'Threefold Inward-Flux Seed', ['geometry', 'polarity', 'imposed-field'], '[IMPOSED] exact initial data'),
    /*
     * Scenario: flux-zero-point (Periodic Random-Wave Bath)
     * Physical purpose: Tests the exact source-free kick-drift invariant on deterministic random initial data.
     * Parameters: Fixed scenario RNG seed and amplitude 0.3 K_B.
     * Expected behavior: Nonzero finite wave bath with no manifestation.
     * Discrepancy: This is not quantum vacuum energy or a ground-state construction.
     */
    makeScenario('1. Validated Native Dynamics', 'flux-zero-point', 'Periodic Random-Wave Bath — Exact Invariant', ['substrate', 'wave'], '[EMERGENT] under the isolated finite periodic wave map'),
    /*
     * Scenario: light-rainbow (Three Transverse Harmonics)
     * Physical purpose: Initializes and verifies three divergence-free transverse harmonics.
     * Parameters: None.
     * Expected behavior: All modes remain transverse and unmanifested under native propagation.
     * Discrepancy: Relative dispersion has not yet earned a menu claim.
     */
    makeScenario('1. Validated Native Dynamics', 'light-rainbow', 'Three Harmonics — Native Transversality Test', ['wave'], '[EMERGENT]'),
    /*
     * Scenario: light-dipole (Dipole-like Radiation Proxy)
     * Physical purpose: Visualizes two oppositely directed transverse radiation lobes.
     * Parameters: None.
     * Expected behavior: Two divergence-free packets separate along opposite x directions.
     * Discrepancy: This is not the full angular Maxwell dipole-radiation solution.
     */
    makeScenario('1. Validated Native Dynamics', 'light-dipole', 'Bidirectional Transverse Lobes — Native Wave Proxy', ['light', 'wave'], '[EMERGENT] under the isolated linear wave map'),
    /*
     * Scenario: light-two-slit (Two-Source Linear Superposition)
     * Physical purpose: Tests exact superposition and a fixed screen cross-term gate.
     * Parameters: None.
     * Expected behavior: Exact linear reconstruction and both cross-term signs.
     * Discrepancy: No material slits or particles are present, and constructive
     * contrast 3.94% fails the preregistered 5% gate.
     */
    makeScenario('1. Validated Native Dynamics', 'light-two-slit', 'Two-Source Superposition — Contrast Gate Failed', ['wave', 'superposition'], '[EMERGENT] linear superposition; [CLOSED NEGATIVE] fixed contrast gate'),
    /*
     * Scenario: light-photon-race (Amplitude-Independent Wave Race)
     * Physical purpose: Compares native wave-packet propagation across a tenfold amplitude change.
     * Parameters: None.
     * Expected behavior: Both packets have equal x displacement in the linear sector.
     * Discrepancy: This establishes amplitude independence, not photon identity.
     */
    makeScenario('1. Validated Native Dynamics', 'light-photon-race', 'Wave Race — Native Amplitude-Independence Test', ['wave'], '[EMERGENT]'),
    /*
     * Scenario: quantum-born-rule (Native Genesis Response)
     * Physical purpose: Measures where native thresholded genesis responds to a seeded flux profile.
     * Parameters: None.
     * Expected behavior: Manifestation events sample the engine's genesis rule.
     * Discrepancy: No wave function, collapse operator, or Born-law proof is implemented.
     */
    makeScenario('2. Validated State Dynamics', 'quantum-born-rule', 'Fixed Gaussian Genesis Cohort — Born Claim Absent', ['genesis', 'cohort', 'null-test'], '[EMERGENT] selected genesis response; [CLOSED NEGATIVE] Born-law interpretation'),
    /*
     * Scenario: quantum-double-slit (Classical Two-Source Interference)
     * Physical purpose: Tests interference from two coherent native flux sources.
     * Parameters: None.
     * Expected behavior: Constructive and destructive classical wave interference.
     * Discrepancy: Genesis is disabled; no single-particle impact distribution is measured.
     */
    makeScenario('1. Validated Native Dynamics', 'quantum-double-slit', 'Two-Source Field — Double-Slit Fringe Gate Failed', ['wave', 'superposition', 'null-test'], '[CLOSED NEGATIVE] destructive fringe at the fixed screen'),
    /*
     * Scenario: quantum-eraser (State-Grid Transmission Prototype)
     * Physical purpose: Tests flux transmission through a selected locked-state grid.
     * Parameters: None.
     * Expected behavior: The initialized field evolves around the static grid.
     * Discrepancy: No which-way observable, polarization measurement, or erasure operation is implemented.
     */
    makeScenario('2. Validated State Dynamics', 'quantum-eraser', 'Checkerboard Coupling Source — Eraser Mechanism Absent', ['coupling', 'wave', 'checkerboard', 'null-test'], '[CLOSED NEGATIVE] quantum-eraser interpretation'),
    /*
     * Scenario: quantum-tunnel (State-Wall Transmission)
     * Physical purpose: Measures native flux transmission past a wall of locked manifested sites.
     * Parameters: None.
     * Expected behavior: Reflected and transmitted native flux can be measured across the wall.
     * Discrepancy: The wall is not a calibrated Schrodinger potential and exponential tunneling is not assumed.
     */
    makeScenario('2. Validated State Dynamics', 'quantum-tunnel', 'Locked State-Sheet Amplifier — Tunneling Gate Failed', ['coupling', 'wave', 'amplification', 'null-test'], '[CLOSED NEGATIVE] tunneling-barrier interpretation'),
    /*
     * Scenario: quantum-well (Imposed Standing Harmonics)
     * Physical purpose: Visualizes selected standing harmonics between marker walls.
     * Parameters: None.
     * Expected behavior: The imposed harmonics evolve under the native wave map.
     * Discrepancy: The marker walls do not impose a wave boundary condition or derive an n-squared spectrum.
     */
    makeScenario('1. Validated Native Dynamics', 'quantum-well', 'Broadband Harmonics — Marker Planes Do Not Confine', ['wave', 'markers', 'null-test'], '[CLOSED NEGATIVE] confinement and particle-in-a-box interpretation'),
    /*
     * Scenario: quantum-entangle (Tagged Opposite-Polarity Pair)
     * Physical purpose: Initializes a pair with shared provenance and opposite polarity/flux.
     * Parameters: None.
     * Expected behavior: Exact pair bookkeeping and classical anti-correlation are preserved at initialization.
     * Discrepancy: No Bell measurement settings or nonclassical correlation are implemented.
     */
    makeScenario('2. Validated State Dynamics', 'quantum-entangle', 'Tagged Polarity Pair — Bookkeeping Test', ['pair', 'polarity'], '[SELECTION]'),
    /*
     * Scenario: quantum-aharonov-bohm (Solenoid Two-Path Topology)
     * Physical purpose: Provides a solenoid-and-two-path topology for a future phase observable.
     * Parameters: None.
     * Expected behavior: Two wave packets propagate on opposite sides of the initialized core.
     * Discrepancy: No gauge-invariant phase or holonomy is extracted, so the Aharonov-Bohm effect is not demonstrated.
     */
    makeScenario('1. Validated Native Dynamics', 'quantum-aharonov-bohm', 'Tube + Two Paths — Aharonov–Bohm Mechanism Absent', ['wave', 'topology', 'superposition', 'null-test'], '[CLOSED NEGATIVE] Aharonov-Bohm phase interaction'),
    /*
     * Scenario: quantum-casimir (Parallel-Plate Vacuum Null Setup)
     * Physical purpose: Provides parallel plates and a seeded noise field for a force-null diagnostic.
     * Parameters: None.
     * Expected behavior: Boundary-modified native field energy can be inspected.
     * Discrepancy: No vacuum ensemble subtraction or plate-force estimator is implemented.
     */
    makeScenario('1. Validated Native Dynamics', 'quantum-casimir', 'Transparent Marker Planes — Casimir Mechanism Absent', ['wave', 'markers', 'null-test'], '[CLOSED NEGATIVE] Casimir boundary and force interpretation'),
    /*
     * Scenario: quantum-zeno (Unobserved Decay Control)
     * Physical purpose: Supplies an unobserved near-threshold control for a future measurement comparison.
     * Parameters: None.
     * Expected behavior: Native decay or manifestation proceeds without an observation intervention.
     * Discrepancy: The engine has no measurement operator, so no Zeno suppression is tested.
     */
    makeScenario('2. Validated State Dynamics', 'quantum-zeno', 'Supercritical Genesis Cohort — Zeno Mechanism Absent', ['genesis', 'cohort', 'null-test'], '[EMERGENT] selected genesis response; [CLOSED NEGATIVE] Zeno interpretation'),

    // Particle-named templates qualified only as source-free vector-wave cohorts.
    // Their polarity/color metadata do not couple to the selected operator.
    makeScenario('1. Validated Native Dynamics', 's0-seed-up-quark', 'A=0.5 Positive/Red-Labeled Wave Template — Up Identity Rejected', ['wave', 'template', 'null-test'], '[IMPOSED] template; [CLOSED NEGATIVE] quark identity'),
    makeScenario('1. Validated Native Dynamics', 's0-seed-down-quark', 'A=0.5 Negative/Green-Labeled Wave Template — Down Identity Rejected', ['wave', 'template', 'null-test'], '[IMPOSED] template; [CLOSED NEGATIVE] quark identity'),
    makeScenario('1. Validated Native Dynamics', 's0-seed-strange-quark', 'A=0.7 Negative/Blue-Labeled Wave Template — Strange Identity Rejected', ['wave', 'template', 'null-test'], '[IMPOSED] template; [CLOSED NEGATIVE] quark identity'),
    makeScenario('1. Validated Native Dynamics', 's0-seed-charm-quark', 'A=1.0 Positive/Red-Labeled Wave Template — Charm Identity Rejected', ['wave', 'template', 'null-test'], '[IMPOSED] template; [CLOSED NEGATIVE] quark identity'),
    makeScenario('1. Validated Native Dynamics', 's0-seed-bottom-quark', 'A=1.4 Negative/Green-Labeled Wave Template — Bottom Identity Rejected', ['wave', 'template', 'null-test'], '[IMPOSED] template; [CLOSED NEGATIVE] quark identity'),
    makeScenario('1. Validated Native Dynamics', 's0-seed-top-quark', 'A=2.5 Positive/Blue-Labeled Wave Template — Top Identity Rejected', ['wave', 'template', 'null-test'], '[IMPOSED] template; [CLOSED NEGATIVE] quark identity'),
    makeScenario('1. Validated Native Dynamics', 's0-seed-anti-up-quark', 'A=0.5 Negative/Red-Labeled Wave Template — Anti-Up Identity Rejected', ['wave', 'template', 'null-test'], '[IMPOSED] template; [CLOSED NEGATIVE] antiquark identity'),
    makeScenario('1. Validated Native Dynamics', 's0-seed-anti-down-quark', 'A=0.5 Positive/Green-Labeled Wave Template — Anti-Down Identity Rejected', ['wave', 'template', 'null-test'], '[IMPOSED] template; [CLOSED NEGATIVE] antiquark identity'),
    makeScenario('1. Validated Native Dynamics', 's0-seed-anti-strange-quark', 'A=0.7 Positive/Blue-Labeled Wave Template — Anti-Strange Identity Rejected', ['wave', 'template', 'null-test'], '[IMPOSED] template; [CLOSED NEGATIVE] antiquark identity'),
    makeScenario('1. Validated Native Dynamics', 's0-seed-anti-charm-quark', 'A=1.0 Negative/Red-Labeled Wave Template — Anti-Charm Identity Rejected', ['wave', 'template', 'null-test'], '[IMPOSED] template; [CLOSED NEGATIVE] antiquark identity'),
    makeScenario('1. Validated Native Dynamics', 's0-seed-anti-bottom-quark', 'A=1.4 Positive/Green-Labeled Wave Template — Anti-Bottom Identity Rejected', ['wave', 'template', 'null-test'], '[IMPOSED] template; [CLOSED NEGATIVE] antiquark identity'),
    makeScenario('1. Validated Native Dynamics', 's0-seed-anti-top-quark', 'A=2.5 Negative/Blue-Labeled Wave Template — Anti-Top Identity Rejected', ['wave', 'template', 'null-test'], '[IMPOSED] template; [CLOSED NEGATIVE] antiquark identity'),
    makeScenario('1. Validated Native Dynamics', 's0-seed-higgs-field', 'Volume-Filling Vector Background — Higgs/VEV Identity Rejected', ['wave', 'background', 'null-test'], '[IMPOSED] vector background; [CLOSED NEGATIVE] scalar/VEV identity'),
    makeScenario('1. Validated Native Dynamics', 's0-seed-gluon', 'Mixed-Polarization Vector Packet — Gluon Identity Rejected', ['wave', 'packet', 'null-test'], '[IMPOSED] vector packet; [CLOSED NEGATIVE] gluon identity'),

    // LHC Standard Model — processes (2026-04-17)
    makeScenario('2. Validated State Dynamics', 's0-seed-beta-decay', 'Prepared Weak-Stress Ramp — Products Preseeded, No Beta Decay', ['weak', 'prepared', 'null-test'], '[EMERGENT] selected polarity flips; [CLOSED NEGATIVE] beta-decay identity'),
    /* Long-baseline production-movement collision; no rest-mass radiation. */
    makeScenario('2. Validated State Dynamics', 's0-seed-ee-annihilation', 'Opposite-Polarity Collision at Tick 24 — No Photon Production', ['collision', 'movement', 'null-test'], '[EMERGENT] state removal; [CLOSED NEGATIVE] e+e-/photon identity'),
    makeScenario('2. Validated State Dynamics', 's0-seed-quark-gluon-plasma', 'Fixed-Seed Thermal Transport/Outflow — QGP Identity Failed', ['langevin', 'transport', 'null-test'], '[EMERGENT] finite transport; [CLOSED NEGATIVE] QGP/deconfinement identity'),
    makeScenario('2. Validated State Dynamics', 's0-seed-hydrogen', 'Locked Triad + Mobile Negative Marker — 64-Tick Coulomb Cohort', ['coulomb', 'prepared', 'null-test'], '[IMPOSED] sources; [CLOSED NEGATIVE] hydrogen identification'),
    makeScenario('2. Validated State Dynamics', 's0-seed-helium', 'Locked 12+2 Coulomb Cohort — Net Polarity −2, Not Helium', ['coulomb', 'prepared', 'null-test'], '[IMPOSED] sources; [CLOSED NEGATIVE] neutral helium identification'),
    makeScenario('2. Validated State Dynamics', 's0-seed-h2-bond-formation', 'Prepared Two-Nucleus Cohort — Mobile Pair Lost, No Bond', ['coulomb', 'prepared', 'null-test'], '[CLOSED NEGATIVE] H2 bond formation'),
    makeScenario('2. Validated State Dynamics', 's0-seed-spark-of-life', 'Patterned Genesis Burst — Six Events, No Life or Autocatalysis', ['genesis', 'prepared', 'null-test'], '[EMERGENT] finite genesis response; [CLOSED NEGATIVE] life/autocatalysis identity'),
    /*
     * Scenario: s0-seed-wilson-loop (Oriented Square Flux Path)
     * Physical purpose: Constructs an exact oriented square in the vector field.
     * Parameters: None.
     * Expected behavior: Inert closed-path initial data with zero vector sum.
     * Discrepancy: No link holonomy, traced Wilson observable, or confinement test is present.
     */
    makeScenario('1. Validated Native Dynamics', 's0-seed-wilson-loop', 'Oriented Square Flux Path — Not a Wilson Observable', ['seed', 'geometry'], '[IMPOSED]'),
    /*
     * Scenario: s0-seed-flux-tube (Gaussian Axial Tube)
     * Physical purpose: Seeds a Gaussian axial vector profile between opposite ternary endpoints.
     * Parameters: None.
     * Expected behavior: Exact inert profile and neutral endpoint bookkeeping.
     * Discrepancy: No q-qbar identity, energy-vs-separation law, or confinement behavior is tested.
     */
    makeScenario('1. Validated Native Dynamics', 's0-seed-flux-tube', 'Gaussian Axial Tube — Imposed Profile', ['seed', 'field'], '[IMPOSED]'),
    /*
     * Scenario: s0-seed-monopole (Radial Inverse-Square Profile)
     * Physical purpose: Seeds an exact radial inverse-square vector ansatz.
     * Parameters: None.
     * Expected behavior: Inert radial profile with fixed r-squared-weighted magnitude.
     * Discrepancy: Magnetic charge is not represented or derived.
     */
    makeScenario('1. Validated Native Dynamics', 's0-seed-monopole', 'Radial Inverse-Square Profile — Monopole Ansatz Only', ['seed', 'field'], '[IMPOSED]'),
    /*
     * Scenario: s0-seed-instanton (Localized Radial 3-Vector Profile)
     * Physical purpose: Preserves the legacy localized radial profile as exact initial data.
     * Parameters: None.
     * Expected behavior: Exact inert J=3 r-hat/(r-squared+9) profile.
     * Discrepancy: This is not an instanton and has no 4D/non-Abelian/topological content.
     */
    makeScenario('1. Validated Native Dynamics', 's0-seed-instanton', 'Localized Radial Profile — Instanton Identity Rejected', ['seed', 'field'], '[CLOSED NEGATIVE] instanton interpretation'),
    /*
     * Scenario: s0-seed-schwarzschild (Inward inverse-square ansatz)
     * Physical purpose: Preserves the legacy radial profile as exact initial data.
     * Parameters: J=-3 G_N K_B r/r^3 plus one central + marker.
     * Expected behavior: Exact inert vector profile.
     * Discrepancy: No Schwarzschild metric, curvature, latency, or gravity solution exists.
     */
    makeScenario('1. Validated Native Dynamics', 's0-seed-schwarzschild', 'Inward Inverse-Square Ansatz — Schwarzschild Identity Rejected', ['seed', 'field', 'null-test'], '[IMPOSED] ansatz; [CLOSED NEGATIVE] Schwarzschild identity'),
    /*
     * Scenario: s0-seed-gravitational-lensing (Gravitational Optical-Channel Null Test)
     * Physical purpose: Places a mass-like seed beside a transverse packet to test for native trajectory bending.
     * Parameters: None.
     * Expected behavior: The packet trajectory is measured against a no-mass control.
     * Discrepancy: The frozen engine does not yet establish a native gravity-to-wave optical coupling.
     */
    makeScenario('1. Validated Native Dynamics', 's0-seed-gravitational-lensing', 'Radial Background + Packet — Lensing Null', ['seed', 'wave', 'null-test'], '[CLOSED NEGATIVE] native gravity-to-wave lensing'),
    /*
     * Scenario: s0-seed-gravitational-wave (Exact transverse harmonic)
     * Physical purpose: Preserves the legacy entry as an exact native wave test.
     * Parameters: n=4, amplitude 0.1, +x propagation, z polarization.
     * Expected behavior: Exact kick-drift lattice eigenmode.
     * Discrepancy: No tensor, metric, source, or gravitational observable exists.
     */
    makeScenario('1. Validated Native Dynamics', 's0-seed-gravitational-wave', 'Exact Transverse Harmonic — Gravity Identity Rejected', ['seed', 'wave', 'null-test'], '[EMERGENT] native wave; [CLOSED NEGATIVE] gravity identity'),
    /*
     * Scenario: s0-seed-massive-body (Massive body (real mass))
     * Physical purpose: Seeds a massive body using real manifested mass (locked).
     * Parameters: None.
     * Expected behavior: Central dense core of locked mass that sources gravity via the Poisson equation.
     * Discrepancy: None.
     */
    makeScenario('5. Macroscopic Physics & Measurement', 's0-seed-massive-body', 'Locked Mass — Native Latency-Poisson Probe', ['seed', 'gravity'], '[EMERGENT] under [IMPOSED] gravity charge and Poisson latency law'),
    /*
     * Scenario: s0-seed-time-gravity-well (Plain-wave legacy alias)
     * Physical purpose: Exposes that the legacy entry duplicates the wave control.
     * Parameters: Exact alias of s0-seed-gravitational-wave.
     * Expected behavior: Bit-identical native harmonic evolution.
     * Discrepancy: No well, latency field, clock, or dτ/dt measurement exists.
     */
    makeScenario('1. Validated Native Dynamics', 's0-seed-time-gravity-well', 'Plain-Wave Alias — Gravity-Well Claim Failed', ['seed', 'wave', 'null-test'], '[CLOSED NEGATIVE] gravity/time interpretation'),
    /*
     * Scenario: s0-seed-time-twin-clocks (Plain-wave legacy alias)
     * Physical purpose: Exposes that the legacy entry duplicates the wave control.
     * Parameters: Exact alias of s0-seed-gravitational-wave.
     * Expected behavior: Bit-identical native harmonic evolution.
     * Discrepancy: No clocks, observers, worldlines, or Δτ comparison exists.
     */
    makeScenario('1. Validated Native Dynamics', 's0-seed-time-twin-clocks', 'Plain-Wave Alias — Twin-Clock Claim Failed', ['seed', 'wave', 'null-test'], '[CLOSED NEGATIVE] twin-clock interpretation'),
    /*
     * Scenario: s0-seed-time-horizon (Inert radial-profile alias)
     * Physical purpose: Exposes that the legacy entry duplicates the radial ansatz.
     * Parameters: Exact alias of s0-seed-schwarzschild.
     * Expected behavior: Bit-identical inert profile.
     * Discrepancy: No horizon condition, latency field, clock, or dτ/dt exists.
     */
    makeScenario('1. Validated Native Dynamics', 's0-seed-time-horizon', 'Radial-Ansatz Alias — Horizon Claim Failed', ['seed', 'field', 'null-test'], '[CLOSED NEGATIVE] horizon/time interpretation'),
    /*
     * Scenario: s0-seed-sloop (Tangential ring ansatz)
     * Physical purpose: Seeds an exact 12-site tangential-flux ring.
     * Parameters: None.
     * Expected behavior: Loop of positive charges carrying angular/circulating flux.
     * Discrepancy: Geometry alone supplies no self-reference or observer mechanism.
     */
    makeScenario('1. Validated Native Dynamics', 's0-seed-sloop', 'Tangential 12-Site Ring — Exact Ansatz', ['seed'], '[IMPOSED] exact structural initial data'),
    /*
     * Scenario: s0-seed-observer-cell (Alternating Moore-shell cell)
     * Physical purpose: Seeds exact alternating ternary labels on the 3^3 Moore cell.
     * Parameters: None.
     * Expected behavior: Central + state surrounded by -,+,- shells.
     * Discrepancy: The imposed pattern carries no observer interpretation.
     */
    makeScenario('1. Validated Native Dynamics', 's0-seed-observer-cell', 'Alternating Moore-Shell Cell — Exact Ansatz', ['seed'], '[IMPOSED] exact structural initial data'),
    /*
     * Scenario: s0-field-plane-wave (Exact Traveling Harmonic)
     * Physical purpose: Tests an exact traveling eigenmode of the native wave map.
     * Parameters: None.
     * Expected behavior: The n=4 Fourier mode follows the exact lattice pole.
     * Discrepancy: No electromagnetic or photon identity is inferred.
     */
    makeScenario('1. Validated Native Dynamics', 's0-field-plane-wave', 'Traveling Harmonic — Exact Native Mode', ['field', 'wave'], '[EMERGENT] within the frozen linear wave map'),
    /*
     * Scenario: s0-field-standing-wave (Exact Standing Harmonic)
     * Physical purpose: Tests an exact standing eigenmode of the native wave map.
     * Parameters: None.
     * Expected behavior: The n=4 mode oscillates with fixed nodes at the exact lattice pole.
     * Discrepancy: No physical cavity or photon identity is inferred.
     */
    makeScenario('1. Validated Native Dynamics', 's0-field-standing-wave', 'Standing Harmonic — Exact Native Mode', ['field', 'wave'], '[EMERGENT] within the frozen linear wave map'),
    /*
     * Scenario: s0-field-uniform-e (Uniform E-proxy initial data)
     * Physical purpose: Establishes the engine's exact uniform canonical-momentum E proxy.
     * Parameters: None.
     * Expected behavior: Every site retains wave_vel=(-0.1,0,0) with all terms off.
     * Discrepancy: No source configuration or Maxwell identification is claimed.
     */
    makeScenario('1. Validated Native Dynamics', 's0-field-uniform-e', 'Uniform Canonical-Momentum Field — E Proxy', ['field'], '[IMPOSED] exact field initial data'),
    /*
     * Scenario: s0-field-uniform-b (Uniform interior-curl ansatz)
     * Physical purpose: Establishes a vector potential with exact uniform interior z curl.
     * Parameters: None.
     * Expected behavior: curl(J)=(0,0,0.05) away from finite faces.
     * Discrepancy: The finite-face discontinuity is excluded from the claim.
     */
    makeScenario('1. Validated Native Dynamics', 's0-field-uniform-b', 'Uniform Interior Curl — B Proxy', ['field'], '[IMPOSED] exact vector-potential initial data'),
    /*
     * Scenario: s0-field-photon-pulse (Broad transverse packet candidate)
     * Physical purpose: Tests a broad transverse packet as a photon candidate.
     * Parameters: None.
     * Expected behavior: Exact transversality, followed by a speed/coherence qualification gate.
     * Discrepancy: CLOSED NEGATIVE for this seed: speed 0.462 vs C_SPEED 0.577 and width ratio 1.646 after 20 ticks.
     */
    makeScenario('1. Validated Native Dynamics', 's0-field-photon-pulse', 'Broad Transverse Packet — Photon Gate Failed', ['field', 'wave', 'null-test'], '[CLOSED NEGATIVE] current photon-pulse seed'),
    /*
     * Scenario: s0-field-rf-lattice-wave (n=1 Transverse Lattice Mode)
     * Physical purpose: Measures the lowest selected transverse spatial harmonic.
     * Parameters: None.
     * Expected behavior: Exact discrete-time lattice pole in periodic evolution.
     * Discrepancy: There is no SI radio-frequency calibration.
     */
    makeScenario('1. Validated Native Dynamics', 's0-field-rf-lattice-wave', 'n=1 Transverse Lattice Mode', ['field', 'wave', 'wave-lab'], '[EMERGENT] native linear pole'),
    /*
     * Scenario: s0-field-light-lattice-wave (n=6 Transverse Lattice Mode)
     * Physical purpose: Measures a shorter-wavelength selected transverse harmonic.
     * Parameters: None.
     * Expected behavior: Exact discrete-time lattice pole in periodic evolution.
     * Discrepancy: There is no SI optical-frequency or color calibration.
     */
    makeScenario('1. Validated Native Dynamics', 's0-field-light-lattice-wave', 'n=6 Transverse Lattice Mode', ['field', 'wave', 'wave-lab'], '[EMERGENT] native linear pole'),
    /*
     * Scenario: s0-field-sound-lattice-wave (Longitudinal n=4 Sound Gate)
     * Physical purpose: Tests whether a c/8 longitudinal seed creates a slower pole.
     * Parameters: None.
     * Expected behavior: The frozen vector wave operator re-propagates it at its native pole.
     * Discrepancy: The c/8 sound-speed interpretation fails; no medium exists in this sector.
     */
    makeScenario('1. Validated Native Dynamics', 's0-field-sound-lattice-wave', 'Longitudinal n=4 Mode — Sound-Speed Gate Failed', ['field', 'wave', 'wave-lab'], '[CLOSED NEGATIVE] c/8 sound interpretation'),
    /*
     * Scenario: s0-field-sound-collision (Longitudinal Packet Overlap)
     * Physical purpose: Tests whether two counter-seeded longitudinal packets interact on overlap.
     * Parameters: Two n=4 Gaussian-windowed lanes with opposite W signs.
     * Expected behavior: Exact native linear superposition through substantial overlap.
     * Discrepancy: No acoustic medium, sound speed, or collision interaction exists.
     */
    makeScenario('1. Validated Native Dynamics', 's0-field-sound-collision', 'Longitudinal Packet Overlap — Sound Collision Absent', ['field', 'wave', 'overlap', 'null-test'], '[CLOSED NEGATIVE] acoustic collision interaction'),
    /*
     * Scenario: s0-field-thomson-scattering (Locked-Source Superposition Null)
     * Physical purpose: Tests whether a locked negative source changes a native plane wave beyond linear addition.
     * Parameters: None.
     * Expected behavior: Deterministic four-arm field decomposition.
     * Discrepancy: No interaction residual or recoil is observed; Thomson scattering is not demonstrated.
     */
    makeScenario('1. Validated Native Dynamics', 's0-field-thomson-scattering', 'Locked-Source Superposition — Thomson Gate Failed', ['field', 'wave', 'null-test'], '[CLOSED NEGATIVE] Thomson scattering for the locked profile'),
    /*
     * Scenario: s0-field-thomson-unlocked-recoil (Native Flux-Gradient Recoil Probe)
     * Physical purpose: Tests the selected native flux-gradient force response of one mobile negative-polarity site.
     * Parameters: None.
     * Expected behavior: Deterministic beam-minus-no-beam displacement under the emergent-forces extension.
     * Discrepancy: No electron identity, Thomson cross section, QED scattering law, or universality is established.
     */
    makeScenario('1. Validated Native Dynamics', 's0-field-thomson-unlocked-recoil', 'Native Flux-Gradient Recoil Probe', ['field', 'wave', 'polarity', 'recoil'], '[EMERGENT] under the selected native flux-gradient force extension'),
    /*
     * Scenario: s0-field-spacetime-forcing-boundary (Native Point-Response Cone)
     * Physical purpose: Measures exact support growth from one production-wave point seed.
     * Parameters: None.
     * Expected behavior: One-neighborhood-per-tick support cone and exact periodic invariant.
     * Discrepancy: This does not derive a Lorentzian metric or physical spacetime.
     */
    makeScenario('1. Validated Native Dynamics', 's0-field-spacetime-forcing-boundary', 'Point Response — Native Locality Cone', ['field', 'wave', 'locality'], '[EMERGENT] finite-support cone under the production wave map'),
    /*
     * Scenario: s0-field-electric-dipole (Softened Opposite-Source Flux)
     * Physical purpose: Installs a declared softened Coulomb-shaped vector profile.
     * Parameters: None.
     * Expected behavior: Exact imported profile around two opposite ternary markers.
     * Discrepancy: The profile is imposed and is not an emergent electromagnetic solution.
     */
    makeScenario('1. Validated Native Dynamics', 's0-field-electric-dipole', 'Softened Opposite-Source Flux Ansatz', ['field'], '[IMPOSED]'),
    /*
     * Scenario: s0-field-magnetic-dipole (Softened Dipole Vector Potential)
     * Physical purpose: Installs a smooth vector-potential ansatz for a z-directed dipole moment.
     * Parameters: None.
     * Expected behavior: Exact A proportional to z-hat cross r over softened r cubed.
     * Discrepancy: This imported ansatz does not derive magnetism or a material source.
     */
    makeScenario('1. Validated Native Dynamics', 's0-field-magnetic-dipole', 'Softened Dipole Vector-Potential Ansatz', ['field'], '[IMPOSED]'),
    /*
     * Scenario: s0-field-vortex-line (Azimuthal Inverse-Radius Profile)
     * Physical purpose: Installs an exact tangential 1/r vector profile around the z-axis.
     * Parameters: None.
     * Expected behavior: Inert azimuthal profile with constant r times field magnitude.
     * Discrepancy: No electromagnetic, fluid, or quantized-vortex identity is established.
     */
    makeScenario('1. Validated Native Dynamics', 's0-field-vortex-line', 'Azimuthal Inverse-Radius Vector Profile', ['field'], '[IMPOSED]'),
    /*
     * Scenario: s0-seed-octahedron (Octahedron (6 face-neighbors))
     * Physical purpose: Seeds an octahedral arrangement of 6 face-neighboring charges.
     * Parameters: None.
     * Expected behavior: Central -1 charge surrounded by 6 positive charges.
     * Discrepancy: None.
     */
    makeScenario('1. Validated Native Dynamics', 's0-seed-octahedron', 'Moore Face Shell — Exact Octahedron', ['seed'], '[IMPOSED] exact structural initial data'),
    /*
     * Scenario: s0-seed-cuboctahedron (Cuboctahedron (12 edge-neighbors))
     * Physical purpose: Seeds a cuboctahedral arrangement of 12 edge-neighboring charges.
     * Parameters: None.
     * Expected behavior: Central -1 charge surrounded by 12 positive charges.
     * Discrepancy: None.
     */
    makeScenario('1. Validated Native Dynamics', 's0-seed-cuboctahedron', 'Moore Edge Shell — Exact Cuboctahedron', ['seed'], '[IMPOSED] exact structural initial data'),
    /*
     * Scenario: s0-seed-stella-octangula (Stella octangula (8 corners))
     * Physical purpose: Seeds a stella octangula arrangement of 8 corner charges.
     * Parameters: None.
     * Expected behavior: Central -1 charge surrounded by 8 positive charges.
     * Discrepancy: None.
     */
    makeScenario('1. Validated Native Dynamics', 's0-seed-stella-octangula', 'Moore Corner Shell — Exact Stella Octangula', ['seed'], '[IMPOSED] exact structural initial data'),
    /*
     * Scenario: s0-seed-moore-cell (Moore cell (full 26))
     * Physical purpose: Seeds a full 26-neighbor Moore cell.
     * Parameters: None.
     * Expected behavior: Central -1 charge surrounded by 26 positive charges.
     * Discrepancy: None.
     */
    makeScenario('1. Validated Native Dynamics', 's0-seed-moore-cell', 'Moore Cell — Exact 27-Site Construction', ['seed'], '[IMPOSED] exact structural initial data'),
    /*
     * Scenario: s0-seed-moore-decomposition (Moore decomposition (3 shells))
     * Physical purpose: Seeds a Moore cell decomposed into shell layers.
     * Parameters: None.
     * Expected behavior: Central -1 charge surrounded by octahedron (+1), cuboctahedron (-1), and stella octangula (+1).
     * Discrepancy: None.
     */
    makeScenario('1. Validated Native Dynamics', 's0-seed-moore-decomposition', 'Moore Cell — Exact 1+6+12+8 Decomposition', ['seed'], '[IMPOSED] exact structural initial data'),

    // FTD-0102 / FTD-0107 emergent-spectrum reproduction.
    /*
     * Scenario: s0-seed-emergent-ic1 (Emergent ic1 (FTD-0107: 25-voxel L¹-ball-radius-2 cluster))
     * Physical purpose: Emergent octahedral bound state point injection (FTD-0107).
     * Parameters: None.
     * Expected behavior: Localized central high-energy flux nucleation into a stable 25-voxel octahedron.
     * Discrepancy: None.
     */
    makeScenario('2. Validated State Dynamics', 's0-seed-emergent-ic1', 'Axial A=10 Genesis Response — 25-Site Gate Failed', ['genesis', 'axial', 'null-test'], '[EMERGENT] finite response; [CLOSED NEGATIVE] 25-site claim'),
    /*
     * Scenario: s0-seed-emergent-ic3-collision (Emergent ic3 (FTD-0107: 2-cluster collision, 2-3 voxels each))
     * Physical purpose: Two-beam collision producing stable emergent clusters (FTD-0107).
     * Parameters: None.
     * Expected behavior: Collision of two opposing flux beams producing stable clusters.
     * Discrepancy: None.
     */
    makeScenario('2. Validated State Dynamics', 's0-seed-emergent-ic3-collision', 'Opposite A=5 Genesis Sources — Collision-Product Gate Failed', ['genesis', 'two-source', 'null-test'], '[EMERGENT] finite response; [CLOSED NEGATIVE] collision-product claim'),
    /*
     * Scenario: s0-seed-emergent-ic4-subthreshold (Emergent ic4 (FTD-0107: sub-threshold, 0 voxels — negative control))
     * Physical purpose: Sub-threshold negative control point injection (FTD-0107).
     * Parameters: None.
     * Expected behavior: Dispersive decay of low-amplitude flux with zero manifested voxels.
     * Discrepancy: None.
     */
    makeScenario('2. Validated State Dynamics', 's0-seed-emergent-ic4-subthreshold', 'Subthreshold A=0.5 Bath Control', ['genesis', 'threshold', 'control'], '[EMERGENT] finite zero-response control'),
    /*
     * Scenario: s0-seed-emergent-ic2-thermal-runaway (Emergent ic2 (FTD-0107: thermal-driven runaway — unstable phase))
     * Physical purpose: Thermal-driven runaway genesis in unstable phase (FTD-0107).
     * Parameters: None.
     * Expected behavior: High thermal Langevin noise triggers runaway genesis without initial flux injection.
     * Discrepancy: None.
     */
    makeScenario('2. Validated State Dynamics', 's0-seed-emergent-ic2-thermal-runaway', 'T=0.05 Empty Bath — Runaway Gate Failed', ['langevin', 'genesis', 'null-test'], '[CLOSED NEGATIVE] runaway over qualified run'),
    /*
     * Scenario: s0-seed-emergent-ic1-diagonal (Emergent ic1 — body-diagonal injection (D3g: Z₄ vs Z₃ test))
     * Physical purpose: Body-diagonal flux point injection (D3g symmetry test).
     * Parameters: None.
     * Expected behavior: Nucleation along body diagonal, testing cluster-size efficiency.
     * Discrepancy: None.
     */
    makeScenario('2. Validated State Dynamics', 's0-seed-emergent-ic1-diagonal', 'Body-Diagonal A=10 Genesis Response', ['genesis', 'diagonal'], '[EMERGENT] finite response'),
    /*
     * Scenario: s0-seed-emergent-ic1-isotropic (Emergent ic1 — isotropic 6-axis injection (D3h: full O_h symmetry test))
     * Physical purpose: Isotropic 6-axis flux point injection (D3h O_h symmetry test).
     * Parameters: None.
     * Expected behavior: Symmetric outward expansion and nucleation.
     * Discrepancy: None.
     */
    makeScenario('2. Validated State Dynamics', 's0-seed-emergent-ic1-isotropic', 'Six-Axis A=10 Genesis Response', ['genesis', 'six-axis'], '[EMERGENT] finite response'),
    /*
     * Scenario: s0-seed-emergent-ic1-viz (Emergent ic1 — clean view (T=0, no thermal background))
     * Physical purpose: Clean visualization of axial ic1 cluster under zero temperature.
     * Parameters: None.
     * Expected behavior: Static, noise-free development of the octahedral bound state.
     * Discrepancy: None.
     */
    makeScenario('2. Validated State Dynamics', 's0-seed-emergent-ic1-viz', 'Axial A=20 T=0 Response — Decaying', ['genesis', 'axial', 'decay'], '[EMERGENT] finite deterministic response'),
    /*
     * Scenario: s0-seed-emergent-ic1-diagonal-viz (Emergent ic1 body-diagonal — clean view (T=0))
     * Physical purpose: Clean visualization of body-diagonal ic1 cluster under zero temperature.
     * Parameters: None.
     * Expected behavior: Static, noise-free development along the body diagonal.
     * Discrepancy: None.
     */
    makeScenario('2. Validated State Dynamics', 's0-seed-emergent-ic1-diagonal-viz', 'Body-Diagonal A=20 T=0 Response — Decaying', ['genesis', 'diagonal', 'decay'], '[EMERGENT] finite deterministic response'),
    /*
     * Scenario: s0-seed-emergent-ic1-isotropic-viz (Emergent ic1 isotropic — clean view (T=0))
     * Physical purpose: Clean visualization of isotropic ic1 cluster under zero temperature.
     * Parameters: None.
     * Expected behavior: Static, noise-free symmetric cluster growth.
     * Discrepancy: None.
     */
    makeScenario('2. Validated State Dynamics', 's0-seed-emergent-ic1-isotropic-viz', 'Six-Axis A=20 T=0 Response — Decaying', ['genesis', 'six-axis', 'decay'], '[EMERGENT] finite deterministic response'),
    /*
     * Scenario: s0-seed-cluster-law (Selected genesis N(A) response — interactive)
     * Physical purpose: Measures finite-box manifested count N versus selected injection amplitude A.
     * Parameters: None.
     * Expected behavior: Interactive response points from the selected production map.
     * Discrepancy: Universality, a broken power law and a forced knee are not established.
     */
    makeScenario('2. Validated State Dynamics', 's0-seed-cluster-law', 'Interactive Genesis Response — Default A=10 Qualified', ['seed', 'genesis', 'response', 'interactive'], '[EMERGENT] default point; arbitrary amplitudes [OPEN]'),
    /*
     * Scenario: s0-seed-cluster-law-subknee (N(A) law — sub-knee (A=12, geometry-limited))
     * Physical purpose: Clean visualization of cluster-law sub-knee regime (A=12).
     * Parameters: None.
     * Expected behavior: Compact 27-block cascade of ~8 voxels under zero temperature.
     * Discrepancy: None.
     */
    makeScenario('2. Validated State Dynamics', 's0-seed-cluster-law-subknee', 'Selected Genesis Response — A=12', ['seed', 'genesis', 'cluster', 'response'], '[EMERGENT] under [SELECTION] genesis/wave/Gauss map'),
    /*
     * Scenario: s0-seed-cluster-law-knee (N(A) law — the knee (A=16, 27-block escape))
     * Physical purpose: Clean visualization of cluster-law knee escape (A=16).
     * Parameters: None.
     * Expected behavior: escape from 27-block to ~21 voxels under zero temperature.
     * Discrepancy: None.
     */
    makeScenario('2. Validated State Dynamics', 's0-seed-cluster-law-knee', 'Selected Genesis Response — A=16', ['seed', 'genesis', 'cluster', 'response'], '[EMERGENT] under [SELECTION] genesis/wave/Gauss map'),
    /*
     * Scenario: s0-seed-cluster-law-superknee (N(A) law — super-knee (A=40, energy budget N=k·A²))
     * Physical purpose: Clean visualization of cluster-law super-knee regime (A=40).
     * Parameters: None.
     * Expected behavior: Large bulk-volume expansion of ~92 voxels under zero temperature.
     * Discrepancy: None.
     */
    makeScenario('2. Validated State Dynamics', 's0-seed-cluster-law-superknee', 'Selected Genesis Response — A=40', ['seed', 'genesis', 'cluster', 'response'], '[EMERGENT] under [SELECTION] genesis/wave/Gauss map'),
    // s0-seed-symmetry-regression removed 2026-04-28 (audit removal): engine CI
    // regression artefact (voxel_uniform() determinism check), not user-facing
    // physics. Fold into engine/tests/ as a ctest if still needed.

    // ── Vacuum Particles (s0-vacuum-* group, 2026-04-28) ───────────────
    // 15 single-particle-in-vacuum scenarios. See
    // engine/web/docs/SPEC_VACUUM_PARTICLE_SCENARIOS.md for the catalog.
    makeScenario('1. Validated Native Dynamics', 's0-vacuum-electron', 'Negative Marker + Radial Wave — Electron Identity Rejected', ['vacuum', 'wave', 'null-test'], '[IMPOSED] template; [CLOSED NEGATIVE] electron identity'),
    makeScenario('1. Validated Native Dynamics', 's0-vacuum-muon', '1.2x Negative-Marker Wave Copy — Muon Identity Rejected', ['vacuum', 'wave', 'null-test'], '[IMPOSED] amplitude copy; [CLOSED NEGATIVE] generation identity'),
    makeScenario('1. Validated Native Dynamics', 's0-vacuum-tau', '1.5x Negative-Marker Wave Copy — Tau Identity Rejected', ['vacuum', 'wave', 'null-test'], '[IMPOSED] amplitude copy; [CLOSED NEGATIVE] generation identity'),
    makeScenario('1. Validated Native Dynamics', 's0-vacuum-positron', 'Positive Marker + Radial Wave — Positron Identity Rejected', ['vacuum', 'wave', 'null-test'], '[IMPOSED] template; [CLOSED NEGATIVE] positron identity'),
    makeScenario('1. Validated Native Dynamics', 's0-vacuum-antimuon', '1.2x Positive-Marker Wave Copy — Antimuon Identity Rejected', ['vacuum', 'wave', 'null-test'], '[IMPOSED] amplitude copy; [CLOSED NEGATIVE] generation identity'),
    makeScenario('1. Validated Native Dynamics', 's0-vacuum-antitau', '1.5x Positive-Marker Wave Copy — Antitau Identity Rejected', ['vacuum', 'wave', 'null-test'], '[IMPOSED] amplitude copy; [CLOSED NEGATIVE] generation identity'),
    /*
     * Scenario: s0-vacuum-electron-neutrino (Neutral Transverse Packet)
     * Physical purpose: Seeds a neutral divergence-free packet for native propagation tests.
     * Parameters: None.
     * Expected behavior: Small-amplitude localized propagating neutral wave packet.
     * Discrepancy: Neutrino identity is not claimed by this validated menu entry.
     */
    makeScenario('1. Validated Native Dynamics', 's0-vacuum-electron-neutrino', 'Neutral Packet Candidate — Native Wave Test', ['vacuum', 'wave'], '[CONJECTURE] — neutral propagation is [EMERGENT]; neutrino identity is not claimed'),
    /*
     * Scenario: s0-vacuum-muon-neutrino (Neutral Packet — imposed 1.3x copy)
     * Physical purpose: Tests amplitude independence of the native neutral packet.
     * Parameters: Exact 1.3 amplitude multiplier relative to the base packet.
     * Expected behavior: Identical propagation after factoring out 1.3.
     * Discrepancy: No flavor label, mass term, oscillation, or neutrino identity exists.
     */
    makeScenario('1. Validated Native Dynamics', 's0-vacuum-muon-neutrino', 'Neutral Packet — Imposed 1.3x Amplitude', ['vacuum', 'wave', 'null-test'], '[EMERGENT] linear propagation; [CLOSED NEGATIVE] flavor interpretation'),
    /*
     * Scenario: s0-vacuum-tau-neutrino (Neutral Packet — imposed 1.6x copy)
     * Physical purpose: Tests amplitude independence of the native neutral packet.
     * Parameters: Exact 1.6 amplitude multiplier relative to the base packet.
     * Expected behavior: Identical propagation after factoring out 1.6.
     * Discrepancy: No flavor label, mass term, oscillation, or neutrino identity exists.
     */
    makeScenario('1. Validated Native Dynamics', 's0-vacuum-tau-neutrino', 'Neutral Packet — Imposed 1.6x Amplitude', ['vacuum', 'wave', 'null-test'], '[EMERGENT] linear propagation; [CLOSED NEGATIVE] flavor interpretation'),
    /*
     * Scenario: s0-vacuum-electron-antineutrino (Neutral Packet Candidate, Opposite Direction — Native Wave Test)
     * Physical purpose: Direction-mirror of s0-vacuum-electron-neutrino.
     * Parameters: None.
     * Expected behavior: Divergence-free packet, translates opposite the electron-neutrino packet.
     * Discrepancy: No flavor label, mass term, oscillation, weak interaction, or antineutrino identity exists.
     */
    makeScenario('1. Validated Native Dynamics', 's0-vacuum-electron-antineutrino', 'Neutral Packet Candidate, Opposite Direction — Native Wave Test', ['vacuum', 'wave'], '[CONJECTURE] — neutral propagation is [EMERGENT]; antineutrino identity is not claimed'),
    /*
     * Scenario: s0-vacuum-muon-antineutrino (Neutral Packet, Opposite Direction — imposed 1.3x copy)
     * Physical purpose: Tests amplitude independence of the direction-mirrored neutral packet.
     * Parameters: Exact 1.3 amplitude multiplier relative to the base antineutrino packet.
     * Expected behavior: Identical propagation after factoring out 1.3.
     * Discrepancy: No flavor label, mass term, oscillation, or antineutrino identity exists.
     */
    makeScenario('1. Validated Native Dynamics', 's0-vacuum-muon-antineutrino', 'Neutral Packet, Opposite Direction — Imposed 1.3x Amplitude', ['vacuum', 'wave', 'null-test'], '[EMERGENT] linear propagation; [CLOSED NEGATIVE] flavor interpretation'),
    /*
     * Scenario: s0-vacuum-tau-antineutrino (Neutral Packet, Opposite Direction — imposed 1.6x copy)
     * Physical purpose: Tests amplitude independence of the direction-mirrored neutral packet.
     * Parameters: Exact 1.6 amplitude multiplier relative to the base antineutrino packet.
     * Expected behavior: Identical propagation after factoring out 1.6.
     * Discrepancy: No flavor label, mass term, oscillation, or antineutrino identity exists.
     */
    makeScenario('1. Validated Native Dynamics', 's0-vacuum-tau-antineutrino', 'Neutral Packet, Opposite Direction — Imposed 1.6x Amplitude', ['vacuum', 'wave', 'null-test'], '[EMERGENT] linear propagation; [CLOSED NEGATIVE] flavor interpretation'),
    /*
     * Scenario: s0-vacuum-photon (Photon Candidate — Native Transverse Packet)
     * Physical purpose: Seeds the cleanest native transverse propagating flux mode.
     * Parameters: None.
     * Expected behavior: A divergence-free packet propagates at the linear lattice cone without manifestation.
     * Discrepancy: Photon identity requires matter coupling and operational electromagnetic observables not shown here.
     */
    makeScenario('1. Validated Native Dynamics', 's0-vacuum-photon',                 'Photon Candidate — Native Wave Test', ['vacuum', 'wave'], '[CONJECTURE] — native propagation is [EMERGENT]; photon identity is [OPEN]'),
    makeScenario('1. Validated Native Dynamics', 's0-vacuum-w-boson', 'Positive Marker + Anisotropic Vector Wave — W Identity Rejected', ['vacuum', 'wave', 'null-test'], '[IMPOSED] vector template; [CLOSED NEGATIVE] W identity'),
    makeScenario('1. Validated Native Dynamics', 's0-vacuum-w-minus-boson', 'Negative Marker + Anisotropic Vector Wave — W Identity Rejected', ['vacuum', 'wave', 'null-test'], '[IMPOSED] vector template; [CLOSED NEGATIVE] W identity'),
    makeScenario('1. Validated Native Dynamics', 's0-vacuum-z-boson', 'Inward Radial Vector Wave — Z Identity Rejected', ['vacuum', 'wave', 'null-test'], '[IMPOSED] vector template; [CLOSED NEGATIVE] Z identity'),
    makeScenario('1. Validated Native Dynamics', 's0-vacuum-higgs', 'Equal-Component Vector Blob — Scalar Higgs Identity Rejected', ['vacuum', 'wave', 'null-test'], '[IMPOSED] vector template; [CLOSED NEGATIVE] scalar Higgs identity'),
    /*
     * Scenario: s0-vacuum-proton (Proton Candidate)
     * Physical purpose: Seeds an unlocked three-constituent color-labelled candidate.
     * Parameters: None.
     * Expected behavior: The implemented color, force, and movement phases determine its evolution.
     * Discrepancy: Stability and proton identity are measurements, not imposed initial facts.
     */
    makeScenario('2. Validated State Dynamics', 's0-vacuum-proton', 'Unlocked Selected-Color Triad — Proton Stability Failed', ['vacuum', 'cohort', 'null-test'], '[CLOSED NEGATIVE] bound proton candidate'),
    /*
     * Scenario: s0-vacuum-neutron (Neutron Candidate)
     * Physical purpose: Seeds an unlocked three-constituent color-labelled candidate.
     * Parameters: None.
     * Expected behavior: The implemented color, force, and movement phases determine its evolution.
     * Discrepancy: Stability and neutron identity are measurements, not imposed initial facts.
     */
    makeScenario('2. Validated State Dynamics', 's0-vacuum-neutron', 'Alternate-Polarity Triad — Neutron Stability Failed', ['vacuum', 'cohort', 'null-test'], '[CLOSED NEGATIVE] bound neutron candidate'),
    /*
     * Scenario: s0-vacuum-pion-charged (Charged-Meson Candidate)
     * Physical purpose: Seeds an unlocked oppositely polarized two-constituent candidate.
     * Parameters: None.
     * Expected behavior: Native color and force phases determine whether the pair binds.
     * Discrepancy: Pion identity and binding are not established by initialization.
     */
    makeScenario('2. Validated State Dynamics', 's0-vacuum-pion-charged', 'Opposite-Polarity Pair — Charged-Pion Binding Failed', ['vacuum', 'collision', 'null-test'], '[CLOSED NEGATIVE] bound charged pion'),
    /*
     * Scenario: s0-vacuum-pion-neutral (Neutral-Meson Candidate)
     * Physical purpose: Seeds an unlocked opposite-state two-constituent candidate.
     * Parameters: None.
     * Expected behavior: Native color and force phases determine whether the pair binds.
     * Discrepancy: Neutral-pion identity and binding are not established by initialization.
     */
    makeScenario('2. Validated State Dynamics', 's0-vacuum-pion-neutral', 'Exact Pair Alias — Neutral-Pion Distinction Absent', ['vacuum', 'alias', 'null-test'], '[CLOSED NEGATIVE] neutral-pion distinction and binding'),
    /*
     * Scenario: s0-vacuum-kaon-charged (Heavy Charged-Meson Candidate)
     * Physical purpose: Seeds an unlocked boosted two-constituent candidate.
     * Parameters: None.
     * Expected behavior: Native color and force phases determine whether the pair binds.
     * Discrepancy: Kaon identity, mass, and binding are not established by initialization.
     */
    makeScenario('2. Validated State Dynamics', 's0-vacuum-kaon-charged', '1.88x-Dressed Pair — Kaon Binding Failed', ['vacuum', 'collision', 'null-test'], '[IMPOSED] boost; [CLOSED NEGATIVE] bound kaon'),
    /*
     * Scenario: s0-seed-de-broglie-clock (Imposed Klein-Gordon Block Clock)
     * Physical purpose: Exercises the optional local -omega0^2 J operator.
     * Parameters: omega0=0.30, J0=0.08, central 7^3 manifested block.
     * Expected behavior: The block receives the selected harmonic restoring kick.
     * Discrepancy: omega0 and the mass term are imposed; no phase-guidance force,
     * physical Compton calibration, or particle identity is derived.
     */
    makeScenario('1. Validated Native Dynamics', 's0-seed-de-broglie-clock', 'Imposed Klein–Gordon Block Clock', ['seed', 'clock', 'selected-operator'], '[IMPOSED] omega0 and mass term; operator response [DERIVED]'),
    /*
     * Scenario: s0-seed-thermal-ignition (Fixed-Temperature Langevin Bath)
     * Physical purpose: Tests the selected Langevin + genesis stack from an empty lattice.
     * Parameters: T=0.03, gamma=0.02, deterministic seed 1.
     * Expected behavior: Finite stochastic-field response with exact seeded replay.
     * Discrepancy: No ignition occurs in the qualified 100-tick L=16 run; a
     * temperature sweep and thermodynamic-limit analysis remain separate work.
     */
    makeScenario('1. Validated Native Dynamics', 's0-seed-thermal-ignition', 'Below-Threshold Langevin/Genesis Bath', ['seed', 'langevin', 'genesis', 'null-test'], '[EMERGENT] finite native response; [CLOSED NEGATIVE] ignition at the qualified point'),
];

// Public/user-facing list: evidence-gated. The full catalog remains available
// to direct research harnesses, saved runs, and provenance audits.
export const SCALE0_SCENARIOS = Object.freeze(
    SCALE0_SCENARIO_CATALOG
        .filter((scenario) => Object.hasOwn(SCALE0_SCENARIO_VALIDATION, scenario.id))
        .map((scenario) => Object.freeze({
            ...scenario,
            validation: SCALE0_SCENARIO_VALIDATION[scenario.id],
        })),
);

export const SCALE0_SCENARIO_MAP = new Map(SCALE0_SCENARIOS.map((scenario) => [scenario.id, scenario]));
const SCALE0_SCENARIO_CATALOG_MAP = new Map(
    SCALE0_SCENARIO_CATALOG.map((scenario) => [scenario.id, scenario]),
);

export function getScale0Scenario(id) {
    // Internal callers may still load a catalogued research scenario by exact
    // ID, but only SCALE0_SCENARIOS is offered in the normal menu.
    const scenario = SCALE0_SCENARIO_MAP.get(id) || SCALE0_SCENARIO_CATALOG_MAP.get(id);
    if (!scenario && id) {
        // C6: surface a typo'd / unregistered id instead of silently loading the
        // default. (Unknown ids legitimately fall back to flux-pulse, but quietly
        // doing so hides bugs like the B4 orphan.)
        console.warn(`[scenario-registry] unknown scenario id "${id}" — falling back to flux-pulse`);
    }
    return scenario || SCALE0_SCENARIO_MAP.get('flux-pulse');
}

export function populateScale0ScenarioSelect(select, selectedId = 'flux-pulse') {
    if (!select) return;
    const groups = new Map();
    for (const scenario of SCALE0_SCENARIOS) {
        if (!groups.has(scenario.category)) groups.set(scenario.category, []);
        groups.get(scenario.category).push(scenario);
    }

    select.innerHTML = '';
    for (const [category, scenarios] of groups) {
        const group = document.createElement('optgroup');
        group.label = category;
        for (const scenario of scenarios) {
            const option = document.createElement('option');
            option.value = scenario.id;
            option.textContent = scenario.title;
            option.selected = scenario.id === selectedId;
            group.appendChild(option);
        }
        select.appendChild(group);
    }
}

export function validateScale0ScenarioRegistry() {
    const seen = new Set();
    const errors = [];
    for (const scenario of SCALE0_SCENARIOS) {
        if (seen.has(scenario.id)) errors.push(`duplicate:${scenario.id}`);
        seen.add(scenario.id);
        if (scenario.scale !== 'lattice') errors.push(`scale:${scenario.id}:${scenario.scale}`);
        if (!scenario.category) errors.push(`category:${scenario.id}`);
        if (!Array.isArray(scenario.requiredCapabilities)) errors.push(`capabilities:${scenario.id}`);
        if (!scenario.validation || scenario.validation.level !== 'behavioral') {
            errors.push(`validation:${scenario.id}`);
        }
    }
    return { ok: errors.length === 0, errors, count: SCALE0_SCENARIOS.length };
}

// C5: run the validator once at module load so registry drift (duplicate ids,
// bad scale/category, malformed capabilities) surfaces as a console warning
// immediately, instead of the validator only ever being callable and never run.
// No-op output for a healthy registry (errors === []).
{
    const _registryCheck = validateScale0ScenarioRegistry();
    if (!_registryCheck.ok) {
        console.warn('[scenario-registry] registry validation failed:', _registryCheck.errors);
    }
}
