#pragma once
// Scale-0 scenario descriptors for the native shell.
// Titles, categories, tags, and epistemic strings are copied from
// engine/web/js/scales/scale0/scenario-registry.js sourceTitle rows.
// Ids must stay set-equal with ftd::scale0_scenario_ids().
// description and min_lattice are unauthored (SPEC_UI_V2 §5.1).

#include <cstddef>
#include <string_view>

namespace ftd {

struct ScenarioMeta {
    const char* id;
    const char* title;
    const char* category;
    const char* description;
    const char* tags;
    const char* epistemic_status;
    const char* admission_status;
    int scale;
    int min_lattice;
};

inline constexpr ScenarioMeta SCENARIO_META[] = {
    {"empty", "Empty Lattice — Null Control", "1. Validated Native Dynamics", "", "baseline", "[AXIOM]", "admitted-behavioral", 0, 0},
    {"s0-seed-dynamical-flux-dressing", "Dynamical Flux Dressing — Native Source Probe", "1. Validated Native Dynamics", "", "field,flux,polarity,dressing,locality", "[EMERGENT] source-built field in the restricted native wave/coupling sector", "admitted-behavioral", 0, 0},
    {"s0-seed-moving-source-reciprocity", "Driven Polarity — Sub-voxel Response", "3. Qualified Selected Extensions", "", "field,flux,polarity,response,reciprocity", "[QUALIFIED NEGATIVE] 0.203598-cell response; no hop, wake, detached field, or closed reciprocity", "admitted-behavioral", 0, 0},
    {"flux-pulse", "Transverse Packet — Finite-Box Boundary Test", "1. Validated Native Dynamics", "", "flux,wave", "[EMERGENT] under [IMPOSED] computational boundary laws", "admitted-behavioral", 0, 0},
    {"flux-dipole", "Antisymmetric Gaussian Wave Pair", "1. Validated Native Dynamics", "", "flux,wave", "[EMERGENT] parity preservation under the native wave map", "admitted-behavioral", 0, 0},
    {"flux-standing", "Reflection-Even Broadband Wave Pair", "1. Validated Native Dynamics", "", "flux,wave", "[EMERGENT] parity preservation under the native wave map", "admitted-behavioral", 0, 0},
    {"flux-nested-standing", "Orthogonal Reflection-Even Wave Pairs", "1. Validated Native Dynamics", "", "flux,wave", "[EMERGENT] parity preservation under the native wave map", "admitted-behavioral", 0, 0},
    {"flux-soliton", "High-Amplitude Packet — Native Dispersion Test", "1. Validated Native Dynamics", "", "flux,wave", "[EMERGENT] under the isolated linear wave map", "admitted-behavioral", 0, 0},
    {"flux-interference", "Four-Lobe Reflection-Symmetric Wave Field", "1. Validated Native Dynamics", "", "flux,wave", "[EMERGENT] parity preservation under the native wave map", "admitted-behavioral", 0, 0},
    {"flux-vortex", "Helical Ring — Exact Vector Ansatz", "1. Validated Native Dynamics", "", "flux,geometry", "[IMPOSED]", "admitted-behavioral", 0, 0},
    {"flux-dual-substrate", "Mirror-Polarized Wave Pair — Dual Sector Not Engaged", "1. Validated Native Dynamics", "", "flux,wave", "[EMERGENT] mixed component parity under the native wave map", "admitted-behavioral", 0, 0},
    {"flux-cascade", "Supercritical Gaussian Genesis Cohort", "1. Validated Native Dynamics", "", "genesis,cohort", "[EMERGENT] under the [SELECTION] local genesis law", "admitted-behavioral", 0, 0},
    {"flux-random-genesis", "Fixed-Seed Random-Patch Genesis Cohort", "1. Validated Native Dynamics", "", "genesis,random-seed,cohort", "[EMERGENT] under the [SELECTION] local genesis law", "admitted-behavioral", 0, 0},
    {"flux-genesis-between-gates", "Genesis Gate — One-Tick Cohorts", "1. Validated Native Dynamics", "", "genesis,ftd-0388", "[EMERGENT] under [SELECTION] local genesis law", "admitted-behavioral", 0, 0},
    {"s0-seed-ew-phase-transition", "Uniform Additive Drive + Genesis — Hysteresis/EW Claim Failed", "2. Validated State Dynamics", "", "drive,genesis,null-test", "[EMERGENT] finite driven response; [CLOSED NEGATIVE] hysteresis/EW identity", "admitted-behavioral", 0, 0},
    {"flux-pair-production", "Native Polarity-Pair Rule — One-Tick Cohort", "1. Validated Native Dynamics", "", "pair-production,polarity", "[EMERGENT] under [SELECTION] pair-transition law", "admitted-behavioral", 0, 0},
    {"flux-annihilation", "Native Opposite-State Collision Rule", "1. Validated Native Dynamics", "", "movement,polarity", "[EMERGENT] collision behavior under the native movement rule", "admitted-behavioral", 0, 0},
    {"flux-vacuum-foam", "Finite Deterministic Random-Wave Ball", "1. Validated Native Dynamics", "", "wave,random-seed,invariant", "[EMERGENT] source-free native wave evolution from [IMPOSED] random initial data", "admitted-behavioral", 0, 0},
    {"flux-meson", "Counter-Moving Opposite-State Pair", "2. Validated State Dynamics", "", "movement,polarity", "[EMERGENT] native movement bookkeeping", "admitted-behavioral", 0, 0},
    {"flux-string-breaking", "Outward Opposite-Polarity Transport — String Absent", "2. Validated State Dynamics", "", "movement,polarity,null-test", "[EMERGENT] native movement; [CLOSED NEGATIVE] string-breaking interpretation", "admitted-behavioral", 0, 0},
    {"flux-baryon", "Threefold Tangential Free Transport", "2. Validated State Dynamics", "", "movement,polarity,threefold", "[EMERGENT] native movement bookkeeping", "admitted-behavioral", 0, 0},
    {"flux-cyclotron", "Imposed-B Native Curvature Test", "1. Validated Native Dynamics", "", "field,polarity,lorentz-response", "[EMERGENT] response under an [IMPOSED] vector potential and [SELECTED] force law", "admitted-behavioral", 0, 0},
    {"flux-screening", "Octahedral Polarity-Shell Seed", "4. Validated Initial Data", "", "geometry,polarity,imposed-field", "[IMPOSED] exact initial data", "admitted-behavioral", 0, 0},
    {"flux-thermalization", "Localized Random-Wave Mixing", "1. Validated Native Dynamics", "", "wave,random-seed,spreading", "[EMERGENT] linear wave spreading from [IMPOSED] random initial data", "admitted-behavioral", 0, 0},
    {"flux-triad", "Threefold Inward-Flux Seed", "4. Validated Initial Data", "", "geometry,polarity,imposed-field", "[IMPOSED] exact initial data", "admitted-behavioral", 0, 0},
    {"flux-zero-point", "Periodic Random-Wave Bath — Exact Invariant", "1. Validated Native Dynamics", "", "substrate,wave", "[EMERGENT] under the isolated finite periodic wave map", "admitted-behavioral", 0, 0},
    {"light-rainbow", "Three Harmonics — Native Transversality Test", "1. Validated Native Dynamics", "", "wave", "[EMERGENT]", "admitted-behavioral", 0, 0},
    {"light-dipole", "Bidirectional Transverse Lobes — Native Wave Proxy", "1. Validated Native Dynamics", "", "light,wave", "[EMERGENT] under the isolated linear wave map", "admitted-behavioral", 0, 0},
    {"light-two-slit", "Two-Source Superposition — Contrast Gate Failed", "1. Validated Native Dynamics", "", "wave,superposition", "[EMERGENT] linear superposition; [CLOSED NEGATIVE] fixed contrast gate", "admitted-behavioral", 0, 0},
    {"light-photon-race", "Wave Race — Native Amplitude-Independence Test", "1. Validated Native Dynamics", "", "wave", "[EMERGENT]", "admitted-behavioral", 0, 0},
    {"quantum-born-rule", "Fixed Gaussian Genesis Cohort — Born Claim Absent", "2. Validated State Dynamics", "", "genesis,cohort,null-test", "[EMERGENT] selected genesis response; [CLOSED NEGATIVE] Born-law interpretation", "admitted-behavioral", 0, 0},
    {"quantum-double-slit", "Two-Source Field — Double-Slit Fringe Gate Failed", "1. Validated Native Dynamics", "", "wave,superposition,null-test", "[CLOSED NEGATIVE] destructive fringe at the fixed screen", "admitted-behavioral", 0, 0},
    {"quantum-eraser", "Checkerboard Coupling Source — Eraser Mechanism Absent", "2. Validated State Dynamics", "", "coupling,wave,checkerboard,null-test", "[CLOSED NEGATIVE] quantum-eraser interpretation", "admitted-behavioral", 0, 0},
    {"quantum-tunnel", "Locked State-Sheet Amplifier — Tunneling Gate Failed", "2. Validated State Dynamics", "", "coupling,wave,amplification,null-test", "[CLOSED NEGATIVE] tunneling-barrier interpretation", "admitted-behavioral", 0, 0},
    {"quantum-well", "Broadband Harmonics — Marker Planes Do Not Confine", "1. Validated Native Dynamics", "", "wave,markers,null-test", "[CLOSED NEGATIVE] confinement and particle-in-a-box interpretation", "admitted-behavioral", 0, 0},
    {"quantum-entangle", "Tagged Polarity Pair — Bookkeeping Test", "2. Validated State Dynamics", "", "pair,polarity", "[SELECTION]", "admitted-behavioral", 0, 0},
    {"quantum-aharonov-bohm", "Tube + Two Paths — Aharonov–Bohm Mechanism Absent", "1. Validated Native Dynamics", "", "wave,topology,superposition,null-test", "[CLOSED NEGATIVE] Aharonov-Bohm phase interaction", "admitted-behavioral", 0, 0},
    {"quantum-casimir", "Transparent Marker Planes — Casimir Mechanism Absent", "1. Validated Native Dynamics", "", "wave,markers,null-test", "[CLOSED NEGATIVE] Casimir boundary and force interpretation", "admitted-behavioral", 0, 0},
    {"quantum-zeno", "Supercritical Genesis Cohort — Zeno Mechanism Absent", "2. Validated State Dynamics", "", "genesis,cohort,null-test", "[EMERGENT] selected genesis response; [CLOSED NEGATIVE] Zeno interpretation", "admitted-behavioral", 0, 0},
    {"s0-seed-up-quark", "A=0.5 Positive/Red-Labeled Wave Template — Up Identity Rejected", "1. Validated Native Dynamics", "", "wave,template,null-test", "[IMPOSED] template; [CLOSED NEGATIVE] quark identity", "admitted-behavioral", 0, 0},
    {"s0-seed-down-quark", "A=0.5 Negative/Green-Labeled Wave Template — Down Identity Rejected", "1. Validated Native Dynamics", "", "wave,template,null-test", "[IMPOSED] template; [CLOSED NEGATIVE] quark identity", "admitted-behavioral", 0, 0},
    {"s0-seed-strange-quark", "A=0.7 Negative/Blue-Labeled Wave Template — Strange Identity Rejected", "1. Validated Native Dynamics", "", "wave,template,null-test", "[IMPOSED] template; [CLOSED NEGATIVE] quark identity", "admitted-behavioral", 0, 0},
    {"s0-seed-charm-quark", "A=1.0 Positive/Red-Labeled Wave Template — Charm Identity Rejected", "1. Validated Native Dynamics", "", "wave,template,null-test", "[IMPOSED] template; [CLOSED NEGATIVE] quark identity", "admitted-behavioral", 0, 0},
    {"s0-seed-bottom-quark", "A=1.4 Negative/Green-Labeled Wave Template — Bottom Identity Rejected", "1. Validated Native Dynamics", "", "wave,template,null-test", "[IMPOSED] template; [CLOSED NEGATIVE] quark identity", "admitted-behavioral", 0, 0},
    {"s0-seed-top-quark", "A=2.5 Positive/Blue-Labeled Wave Template — Top Identity Rejected", "1. Validated Native Dynamics", "", "wave,template,null-test", "[IMPOSED] template; [CLOSED NEGATIVE] quark identity", "admitted-behavioral", 0, 0},
    {"s0-seed-anti-up-quark", "A=0.5 Negative/Red-Labeled Wave Template — Anti-Up Identity Rejected", "1. Validated Native Dynamics", "", "wave,template,null-test", "[IMPOSED] template; [CLOSED NEGATIVE] antiquark identity", "admitted-behavioral", 0, 0},
    {"s0-seed-anti-down-quark", "A=0.5 Positive/Green-Labeled Wave Template — Anti-Down Identity Rejected", "1. Validated Native Dynamics", "", "wave,template,null-test", "[IMPOSED] template; [CLOSED NEGATIVE] antiquark identity", "admitted-behavioral", 0, 0},
    {"s0-seed-anti-strange-quark", "A=0.7 Positive/Blue-Labeled Wave Template — Anti-Strange Identity Rejected", "1. Validated Native Dynamics", "", "wave,template,null-test", "[IMPOSED] template; [CLOSED NEGATIVE] antiquark identity", "admitted-behavioral", 0, 0},
    {"s0-seed-anti-charm-quark", "A=1.0 Negative/Red-Labeled Wave Template — Anti-Charm Identity Rejected", "1. Validated Native Dynamics", "", "wave,template,null-test", "[IMPOSED] template; [CLOSED NEGATIVE] antiquark identity", "admitted-behavioral", 0, 0},
    {"s0-seed-anti-bottom-quark", "A=1.4 Positive/Green-Labeled Wave Template — Anti-Bottom Identity Rejected", "1. Validated Native Dynamics", "", "wave,template,null-test", "[IMPOSED] template; [CLOSED NEGATIVE] antiquark identity", "admitted-behavioral", 0, 0},
    {"s0-seed-anti-top-quark", "A=2.5 Negative/Blue-Labeled Wave Template — Anti-Top Identity Rejected", "1. Validated Native Dynamics", "", "wave,template,null-test", "[IMPOSED] template; [CLOSED NEGATIVE] antiquark identity", "admitted-behavioral", 0, 0},
    {"s0-seed-higgs-field", "Volume-Filling Vector Background — Higgs/VEV Identity Rejected", "1. Validated Native Dynamics", "", "wave,background,null-test", "[IMPOSED] vector background; [CLOSED NEGATIVE] scalar/VEV identity", "admitted-behavioral", 0, 0},
    {"s0-seed-gluon", "Mixed-Polarization Vector Packet — Gluon Identity Rejected", "1. Validated Native Dynamics", "", "wave,packet,null-test", "[IMPOSED] vector packet; [CLOSED NEGATIVE] gluon identity", "admitted-behavioral", 0, 0},
    {"s0-seed-beta-decay", "Prepared Weak-Stress Ramp — Products Preseeded, No Beta Decay", "2. Validated State Dynamics", "", "weak,prepared,null-test", "[EMERGENT] selected polarity flips; [CLOSED NEGATIVE] beta-decay identity", "admitted-behavioral", 0, 0},
    {"s0-seed-ee-annihilation", "Opposite-Polarity Collision at Tick 24 — No Photon Production", "2. Validated State Dynamics", "", "collision,movement,null-test", "[EMERGENT] state removal; [CLOSED NEGATIVE] e+e-/photon identity", "admitted-behavioral", 0, 0},
    {"s0-seed-quark-gluon-plasma", "Fixed-Seed Thermal Transport/Outflow — QGP Identity Failed", "2. Validated State Dynamics", "", "langevin,transport,null-test", "[EMERGENT] finite transport; [CLOSED NEGATIVE] QGP/deconfinement identity", "admitted-behavioral", 0, 0},
    {"s0-seed-hydrogen", "Locked Triad + Mobile Negative Marker — 64-Tick Coulomb Cohort", "2. Validated State Dynamics", "", "coulomb,prepared,null-test", "[IMPOSED] sources; [CLOSED NEGATIVE] hydrogen identification", "admitted-behavioral", 0, 0},
    {"s0-seed-helium", "Locked 12+2 Coulomb Cohort — Net Polarity −2, Not Helium", "2. Validated State Dynamics", "", "coulomb,prepared,null-test", "[IMPOSED] sources; [CLOSED NEGATIVE] neutral helium identification", "admitted-behavioral", 0, 0},
    {"s0-seed-h2-bond-formation", "Prepared Two-Nucleus Cohort — Mobile Pair Lost, No Bond", "2. Validated State Dynamics", "", "coulomb,prepared,null-test", "[CLOSED NEGATIVE] H2 bond formation", "admitted-behavioral", 0, 0},
    {"s0-seed-spark-of-life", "Patterned Genesis Burst — Six Events, No Life or Autocatalysis", "2. Validated State Dynamics", "", "genesis,prepared,null-test", "[EMERGENT] finite genesis response; [CLOSED NEGATIVE] life/autocatalysis identity", "admitted-behavioral", 0, 0},
    {"s0-seed-wilson-loop", "Oriented Square Flux Path — Not a Wilson Observable", "1. Validated Native Dynamics", "", "seed,geometry", "[IMPOSED]", "admitted-behavioral", 0, 0},
    {"s0-seed-flux-tube", "Gaussian Axial Tube — Imposed Profile", "1. Validated Native Dynamics", "", "seed,field", "[IMPOSED]", "admitted-behavioral", 0, 0},
    {"s0-seed-monopole", "Radial Inverse-Square Profile — Monopole Ansatz Only", "1. Validated Native Dynamics", "", "seed,field", "[IMPOSED]", "admitted-behavioral", 0, 0},
    {"s0-seed-instanton", "Localized Radial Profile — Instanton Identity Rejected", "1. Validated Native Dynamics", "", "seed,field", "[CLOSED NEGATIVE] instanton interpretation", "admitted-behavioral", 0, 0},
    {"s0-seed-schwarzschild", "Inward Inverse-Square Ansatz — Schwarzschild Identity Rejected", "1. Validated Native Dynamics", "", "seed,field,null-test", "[IMPOSED] ansatz; [CLOSED NEGATIVE] Schwarzschild identity", "admitted-behavioral", 0, 0},
    {"s0-seed-gravitational-lensing", "Radial Background + Packet — Lensing Null", "1. Validated Native Dynamics", "", "seed,wave,null-test", "[CLOSED NEGATIVE] native gravity-to-wave lensing", "admitted-behavioral", 0, 0},
    {"s0-seed-gravitational-wave", "Exact Transverse Harmonic — Gravity Identity Rejected", "1. Validated Native Dynamics", "", "seed,wave,null-test", "[EMERGENT] native wave; [CLOSED NEGATIVE] gravity identity", "admitted-behavioral", 0, 0},
    {"s0-seed-massive-body", "Locked Mass — Native Latency-Poisson Probe", "5. Macroscopic Physics & Measurement", "", "seed,gravity", "[EMERGENT] under [IMPOSED] gravity charge and Poisson latency law", "admitted-behavioral", 0, 0},
    {"s0-seed-time-gravity-well", "Plain-Wave Alias — Gravity-Well Claim Failed", "1. Validated Native Dynamics", "", "seed,wave,null-test", "[CLOSED NEGATIVE] gravity/time interpretation", "admitted-behavioral", 0, 0},
    {"s0-seed-time-twin-clocks", "Plain-Wave Alias — Twin-Clock Claim Failed", "1. Validated Native Dynamics", "", "seed,wave,null-test", "[CLOSED NEGATIVE] twin-clock interpretation", "admitted-behavioral", 0, 0},
    {"s0-seed-time-horizon", "Radial-Ansatz Alias — Horizon Claim Failed", "1. Validated Native Dynamics", "", "seed,field,null-test", "[CLOSED NEGATIVE] horizon/time interpretation", "admitted-behavioral", 0, 0},
    {"s0-seed-sloop", "Tangential 12-Site Ring — Exact Ansatz", "1. Validated Native Dynamics", "", "seed", "[IMPOSED] exact structural initial data", "admitted-behavioral", 0, 0},
    {"s0-seed-observer-cell", "Alternating Moore-Shell Cell — Exact Ansatz", "1. Validated Native Dynamics", "", "seed", "[IMPOSED] exact structural initial data", "admitted-behavioral", 0, 0},
    {"s0-field-plane-wave", "Traveling Harmonic — Exact Native Mode", "1. Validated Native Dynamics", "", "field,wave", "[EMERGENT] within the frozen linear wave map", "admitted-behavioral", 0, 0},
    {"s0-field-standing-wave", "Standing Harmonic — Exact Native Mode", "1. Validated Native Dynamics", "", "field,wave", "[EMERGENT] within the frozen linear wave map", "admitted-behavioral", 0, 0},
    {"s0-field-uniform-e", "Uniform Canonical-Momentum Field — E Proxy", "1. Validated Native Dynamics", "", "field", "[IMPOSED] exact field initial data", "admitted-behavioral", 0, 0},
    {"s0-field-uniform-b", "Uniform Interior Curl — B Proxy", "1. Validated Native Dynamics", "", "field", "[IMPOSED] exact vector-potential initial data", "admitted-behavioral", 0, 0},
    {"s0-field-photon-pulse", "Broad Transverse Packet — Photon Gate Failed", "1. Validated Native Dynamics", "", "field,wave,null-test", "[CLOSED NEGATIVE] current photon-pulse seed", "admitted-behavioral", 0, 0},
    {"s0-field-rf-lattice-wave", "n=1 Transverse Lattice Mode", "1. Validated Native Dynamics", "", "field,wave,wave-lab", "[EMERGENT] native linear pole", "admitted-behavioral", 0, 0},
    {"s0-field-light-lattice-wave", "n=6 Transverse Lattice Mode", "1. Validated Native Dynamics", "", "field,wave,wave-lab", "[EMERGENT] native linear pole", "admitted-behavioral", 0, 0},
    {"s0-field-sound-lattice-wave", "Longitudinal n=4 Mode — Sound-Speed Gate Failed", "1. Validated Native Dynamics", "", "field,wave,wave-lab", "[CLOSED NEGATIVE] c/8 sound interpretation", "admitted-behavioral", 0, 0},
    {"s0-field-sound-collision", "Longitudinal Packet Overlap — Sound Collision Absent", "1. Validated Native Dynamics", "", "field,wave,overlap,null-test", "[CLOSED NEGATIVE] acoustic collision interaction", "admitted-behavioral", 0, 0},
    {"s0-field-thomson-scattering", "Locked-Source Superposition — Thomson Gate Failed", "1. Validated Native Dynamics", "", "field,wave,null-test", "[CLOSED NEGATIVE] Thomson scattering for the locked profile", "admitted-behavioral", 0, 0},
    {"s0-field-thomson-unlocked-recoil", "Native Flux-Gradient Recoil Probe", "1. Validated Native Dynamics", "", "field,wave,polarity,recoil", "[EMERGENT] under the selected native flux-gradient force extension", "admitted-behavioral", 0, 0},
    {"s0-field-spacetime-forcing-boundary", "Point Response — Native Locality Cone", "1. Validated Native Dynamics", "", "field,wave,locality", "[EMERGENT] finite-support cone under the production wave map", "admitted-behavioral", 0, 0},
    {"s0-field-electric-dipole", "Softened Opposite-Source Flux Ansatz", "1. Validated Native Dynamics", "", "field", "[IMPOSED]", "admitted-behavioral", 0, 0},
    {"s0-field-magnetic-dipole", "Softened Dipole Vector-Potential Ansatz", "1. Validated Native Dynamics", "", "field", "[IMPOSED]", "admitted-behavioral", 0, 0},
    {"s0-field-vortex-line", "Azimuthal Inverse-Radius Vector Profile", "1. Validated Native Dynamics", "", "field", "[IMPOSED]", "admitted-behavioral", 0, 0},
    {"s0-seed-octahedron", "Moore Face Shell — Exact Octahedron", "1. Validated Native Dynamics", "", "seed", "[IMPOSED] exact structural initial data", "admitted-behavioral", 0, 0},
    {"s0-seed-cuboctahedron", "Moore Edge Shell — Exact Cuboctahedron", "1. Validated Native Dynamics", "", "seed", "[IMPOSED] exact structural initial data", "admitted-behavioral", 0, 0},
    {"s0-seed-stella-octangula", "Moore Corner Shell — Exact Stella Octangula", "1. Validated Native Dynamics", "", "seed", "[IMPOSED] exact structural initial data", "admitted-behavioral", 0, 0},
    {"s0-seed-moore-cell", "Moore Cell — Exact 27-Site Construction", "1. Validated Native Dynamics", "", "seed", "[IMPOSED] exact structural initial data", "admitted-behavioral", 0, 0},
    {"s0-seed-moore-decomposition", "Moore Cell — Exact 1+6+12+8 Decomposition", "1. Validated Native Dynamics", "", "seed", "[IMPOSED] exact structural initial data", "admitted-behavioral", 0, 0},
    {"s0-seed-emergent-ic1", "Axial A=10 Genesis Response — 25-Site Gate Failed", "2. Validated State Dynamics", "", "genesis,axial,null-test", "[EMERGENT] finite response; [CLOSED NEGATIVE] 25-site claim", "admitted-behavioral", 0, 0},
    {"s0-seed-emergent-ic3-collision", "Opposite A=5 Genesis Sources — Collision-Product Gate Failed", "2. Validated State Dynamics", "", "genesis,two-source,null-test", "[EMERGENT] finite response; [CLOSED NEGATIVE] collision-product claim", "admitted-behavioral", 0, 0},
    {"s0-seed-emergent-ic4-subthreshold", "Subthreshold A=0.5 Bath Control", "2. Validated State Dynamics", "", "genesis,threshold,control", "[EMERGENT] finite zero-response control", "admitted-behavioral", 0, 0},
    {"s0-seed-emergent-ic2-thermal-runaway", "T=0.05 Empty Bath — Runaway Gate Failed", "2. Validated State Dynamics", "", "langevin,genesis,null-test", "[CLOSED NEGATIVE] runaway over qualified run", "admitted-behavioral", 0, 0},
    {"s0-seed-emergent-ic1-diagonal", "Body-Diagonal A=10 Genesis Response", "2. Validated State Dynamics", "", "genesis,diagonal", "[EMERGENT] finite response", "admitted-behavioral", 0, 0},
    {"s0-seed-emergent-ic1-isotropic", "Six-Axis A=10 Genesis Response", "2. Validated State Dynamics", "", "genesis,six-axis", "[EMERGENT] finite response", "admitted-behavioral", 0, 0},
    {"s0-seed-emergent-ic1-viz", "Axial A=20 T=0 Response — Decaying", "2. Validated State Dynamics", "", "genesis,axial,decay", "[EMERGENT] finite deterministic response", "admitted-behavioral", 0, 0},
    {"s0-seed-emergent-ic1-diagonal-viz", "Body-Diagonal A=20 T=0 Response — Decaying", "2. Validated State Dynamics", "", "genesis,diagonal,decay", "[EMERGENT] finite deterministic response", "admitted-behavioral", 0, 0},
    {"s0-seed-emergent-ic1-isotropic-viz", "Six-Axis A=20 T=0 Response — Decaying", "2. Validated State Dynamics", "", "genesis,six-axis,decay", "[EMERGENT] finite deterministic response", "admitted-behavioral", 0, 0},
    {"s0-seed-cluster-law", "Interactive Genesis Response — Default A=10 Qualified", "2. Validated State Dynamics", "", "seed,genesis,response,interactive", "[EMERGENT] default point; arbitrary amplitudes [OPEN]", "admitted-behavioral", 0, 0},
    {"s0-seed-cluster-law-subknee", "Selected Genesis Response — A=12", "2. Validated State Dynamics", "", "seed,genesis,cluster,response", "[EMERGENT] under [SELECTION] genesis/wave/Gauss map", "admitted-behavioral", 0, 0},
    {"s0-seed-cluster-law-knee", "Selected Genesis Response — A=16", "2. Validated State Dynamics", "", "seed,genesis,cluster,response", "[EMERGENT] under [SELECTION] genesis/wave/Gauss map", "admitted-behavioral", 0, 0},
    {"s0-seed-cluster-law-superknee", "Selected Genesis Response — A=40", "2. Validated State Dynamics", "", "seed,genesis,cluster,response", "[EMERGENT] under [SELECTION] genesis/wave/Gauss map", "admitted-behavioral", 0, 0},
    {"s0-vacuum-electron", "Negative Marker + Radial Wave — Electron Identity Rejected", "1. Validated Native Dynamics", "", "vacuum,wave,null-test", "[IMPOSED] template; [CLOSED NEGATIVE] electron identity", "admitted-behavioral", 0, 0},
    {"s0-vacuum-muon", "1.2x Negative-Marker Wave Copy — Muon Identity Rejected", "1. Validated Native Dynamics", "", "vacuum,wave,null-test", "[IMPOSED] amplitude copy; [CLOSED NEGATIVE] generation identity", "admitted-behavioral", 0, 0},
    {"s0-vacuum-tau", "1.5x Negative-Marker Wave Copy — Tau Identity Rejected", "1. Validated Native Dynamics", "", "vacuum,wave,null-test", "[IMPOSED] amplitude copy; [CLOSED NEGATIVE] generation identity", "admitted-behavioral", 0, 0},
    {"s0-vacuum-positron", "Positive Marker + Radial Wave — Positron Identity Rejected", "1. Validated Native Dynamics", "", "vacuum,wave,null-test", "[IMPOSED] template; [CLOSED NEGATIVE] positron identity", "admitted-behavioral", 0, 0},
    {"s0-vacuum-antimuon", "1.2x Positive-Marker Wave Copy — Antimuon Identity Rejected", "1. Validated Native Dynamics", "", "vacuum,wave,null-test", "[IMPOSED] amplitude copy; [CLOSED NEGATIVE] generation identity", "admitted-behavioral", 0, 0},
    {"s0-vacuum-antitau", "1.5x Positive-Marker Wave Copy — Antitau Identity Rejected", "1. Validated Native Dynamics", "", "vacuum,wave,null-test", "[IMPOSED] amplitude copy; [CLOSED NEGATIVE] generation identity", "admitted-behavioral", 0, 0},
    {"s0-vacuum-electron-neutrino", "Neutral Packet Candidate — Native Wave Test", "1. Validated Native Dynamics", "", "vacuum,wave", "[CONJECTURE] — neutral propagation is [EMERGENT]; neutrino identity is not claimed", "admitted-behavioral", 0, 0},
    {"s0-vacuum-muon-neutrino", "Neutral Packet — Imposed 1.3x Amplitude", "1. Validated Native Dynamics", "", "vacuum,wave,null-test", "[EMERGENT] linear propagation; [CLOSED NEGATIVE] flavor interpretation", "admitted-behavioral", 0, 0},
    {"s0-vacuum-tau-neutrino", "Neutral Packet — Imposed 1.6x Amplitude", "1. Validated Native Dynamics", "", "vacuum,wave,null-test", "[EMERGENT] linear propagation; [CLOSED NEGATIVE] flavor interpretation", "admitted-behavioral", 0, 0},
    {"s0-vacuum-electron-antineutrino", "Neutral Packet Candidate, Opposite Direction — Native Wave Test", "1. Validated Native Dynamics", "", "vacuum,wave", "[CONJECTURE] — neutral propagation is [EMERGENT]; antineutrino identity is not claimed", "admitted-behavioral", 0, 0},
    {"s0-vacuum-muon-antineutrino", "Neutral Packet, Opposite Direction — Imposed 1.3x Amplitude", "1. Validated Native Dynamics", "", "vacuum,wave,null-test", "[EMERGENT] linear propagation; [CLOSED NEGATIVE] flavor interpretation", "admitted-behavioral", 0, 0},
    {"s0-vacuum-tau-antineutrino", "Neutral Packet, Opposite Direction — Imposed 1.6x Amplitude", "1. Validated Native Dynamics", "", "vacuum,wave,null-test", "[EMERGENT] linear propagation; [CLOSED NEGATIVE] flavor interpretation", "admitted-behavioral", 0, 0},
    {"s0-vacuum-photon", "Photon Candidate — Native Wave Test", "1. Validated Native Dynamics", "", "vacuum,wave", "[CONJECTURE] — native propagation is [EMERGENT]; photon identity is [OPEN]", "admitted-behavioral", 0, 0},
    {"s0-vacuum-w-boson", "Positive Marker + Anisotropic Vector Wave — W Identity Rejected", "1. Validated Native Dynamics", "", "vacuum,wave,null-test", "[IMPOSED] vector template; [CLOSED NEGATIVE] W identity", "admitted-behavioral", 0, 0},
    {"s0-vacuum-w-minus-boson", "Negative Marker + Anisotropic Vector Wave — W Identity Rejected", "1. Validated Native Dynamics", "", "vacuum,wave,null-test", "[IMPOSED] vector template; [CLOSED NEGATIVE] W identity", "admitted-behavioral", 0, 0},
    {"s0-vacuum-z-boson", "Inward Radial Vector Wave — Z Identity Rejected", "1. Validated Native Dynamics", "", "vacuum,wave,null-test", "[IMPOSED] vector template; [CLOSED NEGATIVE] Z identity", "admitted-behavioral", 0, 0},
    {"s0-vacuum-higgs", "Equal-Component Vector Blob — Scalar Higgs Identity Rejected", "1. Validated Native Dynamics", "", "vacuum,wave,null-test", "[IMPOSED] vector template; [CLOSED NEGATIVE] scalar Higgs identity", "admitted-behavioral", 0, 0},
    {"s0-vacuum-proton", "Unlocked Selected-Color Triad — Proton Stability Failed", "2. Validated State Dynamics", "", "vacuum,cohort,null-test", "[CLOSED NEGATIVE] bound proton candidate", "admitted-behavioral", 0, 0},
    {"s0-vacuum-neutron", "Alternate-Polarity Triad — Neutron Stability Failed", "2. Validated State Dynamics", "", "vacuum,cohort,null-test", "[CLOSED NEGATIVE] bound neutron candidate", "admitted-behavioral", 0, 0},
    {"s0-vacuum-pion-charged", "Opposite-Polarity Pair — Charged-Pion Binding Failed", "2. Validated State Dynamics", "", "vacuum,collision,null-test", "[CLOSED NEGATIVE] bound charged pion", "admitted-behavioral", 0, 0},
    {"s0-vacuum-pion-neutral", "Exact Pair Alias — Neutral-Pion Distinction Absent", "2. Validated State Dynamics", "", "vacuum,alias,null-test", "[CLOSED NEGATIVE] neutral-pion distinction and binding", "admitted-behavioral", 0, 0},
    {"s0-vacuum-kaon-charged", "1.88x-Dressed Pair — Kaon Binding Failed", "2. Validated State Dynamics", "", "vacuum,collision,null-test", "[IMPOSED] boost; [CLOSED NEGATIVE] bound kaon", "admitted-behavioral", 0, 0},
    {"s0-seed-de-broglie-clock", "Imposed Klein–Gordon Block Clock", "1. Validated Native Dynamics", "", "seed,clock,selected-operator", "[IMPOSED] omega0 and mass term; operator response [DERIVED]", "admitted-behavioral", 0, 0},
    {"s0-seed-thermal-ignition", "Below-Threshold Langevin/Genesis Bath", "1. Validated Native Dynamics", "", "seed,langevin,genesis,null-test", "[EMERGENT] finite native response; [CLOSED NEGATIVE] ignition at the qualified point", "admitted-behavioral", 0, 0},
};

inline constexpr std::size_t scenario_meta_count() {
    return sizeof(SCENARIO_META) / sizeof(SCENARIO_META[0]);
}

inline const ScenarioMeta* find_scenario_meta(std::string_view id) {
    for (const auto& row : SCENARIO_META) {
        if (id == row.id) return &row;
    }
    return nullptr;
}

}  // namespace ftd
