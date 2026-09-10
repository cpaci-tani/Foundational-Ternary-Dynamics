// Scientific display names describe the prepared fields, selected rules, and
// qualified behavior. Historical identities and validation remain in the registry.
// This is presentation metadata only; IDs and native preparations are unchanged.
export const SCALE0_SCENARIO_PRESENTATION = Object.freeze({

    // Baselines & Controls
    "empty": {
        title: "Empty Lattice Null Control",
        intent: "Verify persistence of an initially field-free lattice with no manifested states.",
    },
    "s0-seed-emergent-ic4-subthreshold": {
        title: "Subthreshold Point-Seed Control A=0.5",
        intent: "Probe the zero-manifestation response to a central seed below the selected genesis threshold.",
    },

    // Waves · Propagation
    "flux-pulse": {
        title: "Transverse Wave Packet Propagation",
        intent: "A divergence-free packet probes propagation, reflection, and dispersal under the selected finite-box boundary law.",
    },
    "flux-soliton": {
        title: "High-Amplitude Wave Packet Dispersion",
        intent: "An intense transverse packet translates and broadens under the linear wave map without preserving a soliton shape.",
    },
    "light-rainbow": {
        title: "Three Transverse Wave Harmonics",
        intent: "Three imposed spatial harmonics remain transverse and divergence-free during wave evolution.",
    },
    "light-dipole": {
        title: "Oppositely Propagating Transverse Packets",
        intent: "Two transverse packets separate symmetrically along opposite directions with balanced half-space energy.",
    },
    "light-photon-race": {
        title: "Amplitude-Independent Wave Propagation",
        intent: "Two packets with a tenfold amplitude difference travel equal distances under the linear wave law.",
    },
    "s0-field-plane-wave": {
        title: "Traveling Transverse Mode (n = 4)",
        intent: "A transverse Fourier eigenmode follows the exact traveling-wave phase of the lattice update.",
    },
    "s0-field-photon-pulse": {
        title: "Broad Transverse Packet Dispersion",
        intent: "A broad transverse packet propagates and spreads, with no established photon identity or speed calibration.",
    },
    "s0-field-rf-lattice-wave": {
        title: "Transverse Lattice Mode (n = 1)",
        intent: "The fundamental periodic transverse mode advances at its discrete lattice frequency.",
    },
    "s0-field-light-lattice-wave": {
        title: "Transverse Lattice Mode (n = 6)",
        intent: "A shorter-wavelength transverse harmonic advances at its discrete lattice frequency.",
    },
    "s0-field-sound-lattice-wave": {
        title: "Longitudinal Lattice Mode (n = 4)",
        intent: "A longitudinal harmonic evolves under the vector wave equation without an acoustic medium or recovered sound speed.",
    },
    "s0-field-spacetime-forcing-boundary": {
        title: "Point Impulse and Causal Propagation",
        intent: "A localized wave seed produces a response whose support grows within the lattice causal cone.",
    },

    // Waves · Interference & Standing Modes
    "flux-dipole": {
        title: "Antisymmetric Gaussian Wave Pair",
        intent: "Two opposite Gaussian wave lobes preserve odd reflection parity under isolated wave evolution.",
    },
    "flux-standing": {
        title: "Symmetric Gaussian Wave Pair",
        intent: "An even Gaussian pair begins with zero wave momentum and preserves reflection symmetry.",
    },
    "flux-nested-standing": {
        title: "Orthogonal Symmetric Gaussian Wave Pairs",
        intent: "Two orthogonal broadband Gaussian pairs preserve reflection symmetry about both seeded axes.",
    },
    "flux-interference": {
        title: "Four-Lobe Symmetric Wave Superposition",
        intent: "Four Gaussian lobes evolve as a linear wave field with two-axis reflection symmetry.",
    },
    "flux-dual-substrate": {
        title: "Mirror-Polarized Gaussian Wave Pair",
        intent: "Mirrored Gaussian packets preserve their mixed vector-component parities under the single active wave map.",
    },
    "light-two-slit": {
        title: "Oscillatory Wave Packet Superposition",
        intent: "Two separated Gaussian sheet packets with sinusoidal carriers exhibit linear superposition and signed interference cross terms.",
    },
    "quantum-double-slit": {
        title: "Gaussian Wave Packet Superposition",
        intent: "Two coherent Gaussian sheet packets combine linearly with constructive overlap at the qualified observation plane.",
    },
    "s0-field-standing-wave": {
        title: "Standing Transverse Mode (n = 4)",
        intent: "A transverse Fourier eigenmode oscillates with fixed nodes and no traveling-mode leakage.",
    },
    "s0-field-sound-collision": {
        title: "Counterpropagating Longitudinal Wave Packets",
        intent: "Oppositely seeded longitudinal packets overlap through exact linear superposition without a collision interaction.",
    },

    // Waves · Boundaries & Barriers
    "quantum-tunnel": {
        title: "Wave Amplification by Locked State Sheets",
        intent: "Locked state sheets source a large field response through the coupling term rather than acting as a tunneling barrier.",
    },
    "quantum-well": {
        title: "Broadband Waves Through Marker Planes",
        intent: "Broadband waves propagate beyond two marker planes that impose no confining wave boundary.",
    },
    "quantum-aharonov-bohm": {
        title: "Central Tube and Two Wave Paths",
        intent: "A central tube field and two path packets evolve by linear addition without a measured phase interaction.",
    },
    "quantum-casimir": {
        title: "Transverse Mode Through Marker Planes",
        intent: "A transverse eigenmode evolves identically with and without the two inert marker planes.",
    },

    // Fields · Sources & Electric Profiles
    "s0-seed-dynamical-flux-dressing": {
        title: "Polarity-Sourced Field Propagation",
        intent: "A locked central polarity generates a causal field response from initially zero flux through the native coupling term.",
    },
    "flux-screening": {
        title: "Central Polarity and Octahedral Shell",
        intent: "One central positive state and six surrounding negative states form an inert octahedral source geometry.",
    },
    "quantum-eraser": {
        title: "Checkerboard State-Coupling Wave Amplification",
        intent: "A locked checkerboard state pattern amplifies the field through the active source-coupling term.",
    },
    "s0-field-uniform-e": {
        title: "Uniform Canonical Momentum Field",
        intent: "A uniform wave-momentum field remains unchanged with all evolution terms disabled.",
    },
    "s0-field-electric-dipole": {
        title: "Softened Opposite-Source Field Profile",
        intent: "Two opposite source markers accompany an imposed softened Coulomb-shaped flux profile.",
    },

    // Fields · Magnetic Profiles
    "s0-seed-monopole": {
        title: "Radial Inverse-Square Field Profile",
        intent: "An imposed radial vector field has constant radius-squared-weighted magnitude without a magnetic-charge mechanism.",
    },
    "s0-field-uniform-b": {
        title: "Vector Potential with Uniform Interior Curl",
        intent: "An imposed vector potential produces a constant curl away from the finite lattice faces.",
    },
    "s0-field-magnetic-dipole": {
        title: "Softened Dipole Vector Potential",
        intent: "A static softened dipole vector-potential profile displays the prescribed geometry around its axis.",
    },

    // Energy · Storage & Boundaries
    "s0-cell-capacitor": {
        title: "Gauss Projection Between Opposite Plates",
        intent: "The Gauss projection builds flux across initially empty space between two locked opposite-polarity plates.",
    },
    "s0-cell-torus": {
        title: "Periodic Ring Field Dispersion",
        intent: "A circulating ring field redistributes its energy across a periodic box while conserving the wave Hamiltonian.",
    },
    "s0-cell-torus-reverse": {
        title: "Reversed Ring Circulation Control",
        intent: "The reversed ring has opposite initial circulation and equal initial energy relative to the forward ring.",
    },
    "s0-cell-torus-scrambled": {
        title: "Alternating-Sign Ring Field Dispersion",
        intent: "Quadrant sign reversals remove net circulation while preserving the initial pointwise field magnitude.",
    },
    "s0-cell-torus-open": {
        title: "Ring Field Energy Escape",
        intent: "A ring field loses energy through the selected dispersal boundary as waves reach the box faces.",
    },
    "s0-cell-torus-walled": {
        title: "Ring Waves in a Reflective Box",
        intent: "Reflective boundaries retain total wave energy while allowing the initial ring geometry to disperse.",
    },
    "s0-cell-triad": {
        title: "Three-Axis Standing Wave Packets",
        intent: "Three crossing standing arms have equal axial flux moments and zero net field current.",
    },
    "s0-cell-torus-membrane": {
        title: "Ring Field Retention in a Clocked Shell",
        intent: "An imposed shell with a local harmonic clock term retains substantially more ring energy than its low-frequency control.",
    },

    // Energy · Driving & Transfer
    "s0-cell-torus-membrane-gated": {
        title: "Field Energy Release Through an Aperture",
        intent: "Scheduled removal of shell sites opens a port whose measured energy current accounts for reservoir discharge.",
    },
    "s0-cell-membrane-pumped": {
        title: "Pulsed Charging of a Field Reservoir",
        intent: "Twenty consecutive source increments inject accounted energy into an initially empty clocked shell before the source switches off.",
    },
    "s0-cell-membrane-transfer": {
        title: "Field Energy Transfer Through an Aperture",
        intent: "An opening between two clocked shells permits measured energy transfer from a seeded cell into an initially empty receiver.",
    },
    "s0-cell-membrane-pumped-resonant": {
        title: "Periodic Driving of a Field Reservoir",
        intent: "Source increments spaced by eight ticks inject more energy than consecutive increments at the measured constructive driving interval.",
    },

    // Collective Fields · Noise & Shear
    "flux-vacuum-foam": {
        title: "Localized Random Wave Evolution",
        intent: "A finite deterministic random wave ball evolves without ongoing noise injection and conserves the periodic wave invariant.",
    },
    "flux-thermalization": {
        title: "Random Wave Packet Spreading",
        intent: "A compact deterministic random field spreads beyond its initial support through linear propagation and dephasing.",
    },
    "flux-zero-point": {
        title: "Periodic Random Wave Field",
        intent: "A subthreshold random wave field evolves source-free in a periodic domain while conserving its discrete Hamiltonian.",
    },
    "s0-field-shear-layer": {
        title: "Wave Propagation of a Sheared Field",
        intent: "A transverse sheared flux profile splits into traveling wave components rather than undergoing viscous diffusion.",
    },

    // State Dynamics · Genesis & Decay
    "flux-cascade": {
        title: "Above-Threshold Gaussian State Creation",
        intent: "Measure the first-tick local genesis cohort from a concentrated supercritical Gaussian field.",
    },
    "flux-random-genesis": {
        title: "State Creation from Random Field Patches",
        intent: "Measure the first-tick genesis cohort from reproducible random field patches.",
    },
    "flux-genesis-between-gates": {
        title: "Three-Band State-Creation Threshold Probe",
        intent: "Compare the first genesis decision in three amplitude bands straddling the current and retired thresholds.",
    },
    "s0-seed-ew-phase-transition": {
        title: "Uniform Additive Drive and Genesis",
        intent: "Measure state creation under a prescribed nonnegative uniform field drive.",
    },
    "flux-pair-production": {
        title: "Adjacent Polarity-Pair Creation Rule",
        intent: "Test the selected one-tick transition that creates adjacent opposite states with a shared pair identifier.",
    },
    "quantum-born-rule": {
        title: "State Creation from a Polarized Gaussian",
        intent: "Measure local threshold response to a Gaussian field envelope with fixed vector orientation.",
    },
    "quantum-zeno": {
        title: "State Creation from Equal-Component Fields",
        intent: "Measure first-tick state creation from a Gaussian field with equal vector components.",
    },
    "s0-seed-beta-decay": {
        title: "Damped Weak-Stress Polarity Flips",
        intent: "Track selected polarity flips in a prepared state configuration under weak transmutation and damping.",
    },
    "s0-seed-spark-of-life": {
        title: "State Creation from a Patterned Seed",
        intent: "Measure the finite genesis response of a prescribed mixture of locked and mobile states.",
    },
    "s0-seed-emergent-ic1": {
        title: "Axial Point-Seed Genesis A=10",
        intent: "Track finite state creation following a central field injection along one lattice axis.",
    },
    "s0-seed-emergent-ic3-collision": {
        title: "Opposite Point-Seed Genesis A=5",
        intent: "Track the state response to two separated field injections with opposite axial orientations.",
    },
    "s0-seed-emergent-ic2-thermal-runaway": {
        title: "Empty Langevin Bath T=0.05",
        intent: "Test genesis response in an initially empty lattice driven by the prescribed Langevin bath.",
    },
    "s0-seed-emergent-ic1-diagonal": {
        title: "Body-Diagonal Point-Seed Genesis A=10",
        intent: "Track state creation from a central field injection directed along the cube body diagonal.",
    },
    "s0-seed-emergent-ic1-isotropic": {
        title: "Six-Axis Genesis Preparation A=10",
        intent: "Track state creation from equal outward field injections on the six face neighbors.",
    },
    "s0-seed-emergent-ic1-viz": {
        title: "Axial Genesis and Decay A=20",
        intent: "Track the decaying manifested-site count after an axial injection with Langevin driving disabled.",
    },
    "s0-seed-emergent-ic1-diagonal-viz": {
        title: "Body-Diagonal Genesis and Decay A=20",
        intent: "Track the decaying manifested-site count after a body-diagonal injection with Langevin driving disabled.",
    },
    "s0-seed-emergent-ic1-isotropic-viz": {
        title: "Six-Axis Genesis and Decay A=20",
        intent: "Track the decaying manifested-site count after six outward injections with Langevin driving disabled.",
    },
    "s0-seed-cluster-law": {
        title: "Adjustable Axial Genesis Amplitude",
        intent: "Explore axial genesis response around the qualified default amplitude A=10.",
    },
    "s0-seed-cluster-law-subknee": {
        title: "Axial Genesis Response A=12",
        intent: "Measure the finite-box response at the lowest registered amplitude of the A=12/16/40 comparison.",
    },
    "s0-seed-cluster-law-knee": {
        title: "Axial Genesis Response A=16",
        intent: "Measure the finite-box response at the middle registered amplitude of the A=12/16/40 comparison.",
    },
    "s0-seed-cluster-law-superknee": {
        title: "Axial Genesis Response A=40",
        intent: "Measure the finite-box response at the highest registered amplitude of the A=12/16/40 comparison.",
    },
    "s0-seed-thermal-ignition": {
        title: "Empty Langevin Bath T=0.03",
        intent: "Measure field excitation and genesis response under a prescribed T=0.03 Langevin bath.",
    },

    // Particle Motion & Collisions
    "s0-seed-moving-source-reciprocity": {
        title: "Packet-Driven Subvoxel Source Response",
        intent: "Measure the selected flux-gradient force response of a resting source to a separate transverse packet.",
    },
    "flux-annihilation": {
        title: "Adjacent Opposite-State Collision Removal",
        intent: "Test removal and pre-existing flux redistribution when a moving state enters an adjacent opposite state.",
    },
    "flux-meson": {
        title: "Counter-Moving Opposite-State Transport",
        intent: "Track exact free transport of two separated opposite states with counter-directed velocities.",
    },
    "flux-string-breaking": {
        title: "Separating Opposite-State Free Transport",
        intent: "Track increasing separation of two opposite states under movement alone.",
    },
    "flux-baryon": {
        title: "Threefold Tangential Marker Transport",
        intent: "Track three tangentially moving positive markers alongside a stationary opposite marker.",
    },
    "flux-cyclotron": {
        title: "Imposed Magnetic-Field Trajectory Curvature",
        intent: "Measure trajectory curvature under the selected velocity-cross-field-curl force term.",
    },
    "quantum-entangle": {
        title: "Tagged Opposite-Polarity Pair",
        intent: "Inspect two initialized opposite states with a shared pair identifier and cancelling flux.",
    },
    "s0-seed-ee-annihilation": {
        title: "Separated Opposite-State Collision Removal",
        intent: "Track a separated opposite-state pair through transport to collision removal and field redistribution.",
    },
    "s0-seed-quark-gluon-plasma": {
        title: "Langevin-Bath Marker Transport and Outflow",
        intent: "Track prepared markers through a prescribed wave bath and their loss at open particle boundaries.",
    },
    "s0-field-thomson-scattering": {
        title: "Locked-Source Wave Superposition Control",
        intent: "Compare combined beam and locked-source evolution with their independently evolved sum.",
    },
    "s0-field-thomson-unlocked-recoil": {
        title: "Plane-Wave Flux-Gradient Recoil",
        intent: "Measure source displacement driven by a plane wave through the selected flux-gradient force.",
    },

    // Particle Models · Leptons
    "s0-vacuum-electron": {
        title: "Inward Radial Field with Negative Marker",
        intent: "An inert negative marker sits at the center of an inward radial Gaussian vector field evolving under the source-free wave map.",
    },
    "s0-vacuum-muon": {
        title: "Inward Radial Field · 1.2× Amplitude",
        intent: "The negative-marker radial field is prepared at 1.2 times the base amplitude to compare otherwise identical linear wave evolution.",
    },
    "s0-vacuum-tau": {
        title: "Inward Radial Field · 1.5× Amplitude",
        intent: "The negative-marker radial field is prepared at 1.5 times the base amplitude to compare otherwise identical linear wave evolution.",
    },
    "s0-vacuum-positron": {
        title: "Outward Radial Field with Positive Marker",
        intent: "An inert positive marker sits at the center of an outward radial Gaussian vector field evolving under the source-free wave map.",
    },
    "s0-vacuum-antimuon": {
        title: "Outward Radial Field · 1.2× Amplitude",
        intent: "The positive-marker radial field is prepared at 1.2 times the base amplitude to compare otherwise identical linear wave evolution.",
    },
    "s0-vacuum-antitau": {
        title: "Outward Radial Field · 1.5× Amplitude",
        intent: "The positive-marker radial field is prepared at 1.5 times the base amplitude to compare otherwise identical linear wave evolution.",
    },
    "s0-vacuum-electron-neutrino": {
        title: "Localized Transverse Packet · Forward",
        intent: "A localized divergence-free vector packet travels in the positive x direction without producing manifested sites.",
    },
    "s0-vacuum-muon-neutrino": {
        title: "Forward Transverse Packet · 1.3× Amplitude",
        intent: "The forward localized transverse packet is prepared at 1.3 times the base amplitude to compare amplitude-independent propagation.",
    },
    "s0-vacuum-tau-neutrino": {
        title: "Forward Transverse Packet · 1.6× Amplitude",
        intent: "The forward localized transverse packet is prepared at 1.6 times the base amplitude to compare amplitude-independent propagation.",
    },
    "s0-vacuum-electron-antineutrino": {
        title: "Localized Transverse Packet · Reverse",
        intent: "A localized divergence-free vector packet travels in the negative x direction as the directional counterpart of the forward preparation.",
    },
    "s0-vacuum-muon-antineutrino": {
        title: "Reverse Transverse Packet · 1.3× Amplitude",
        intent: "The reverse localized transverse packet is prepared at 1.3 times the base amplitude to compare amplitude-independent propagation.",
    },
    "s0-vacuum-tau-antineutrino": {
        title: "Reverse Transverse Packet · 1.6× Amplitude",
        intent: "The reverse localized transverse packet is prepared at 1.6 times the base amplitude to compare amplitude-independent propagation.",
    },

    // Particle Models · Quarks
    "s0-seed-up-quark": {
        title: "Positive X-Biased Wave · A=0.5",
        intent: "An inert positive marker accompanies an outward radial wave with an added x-directed component and an imposed amplitude multiplier of 0.5.",
    },
    "s0-seed-down-quark": {
        title: "Negative Y-Biased Wave · A=0.5",
        intent: "An inert negative marker accompanies an inward radial wave with an added negative y-directed component and an imposed amplitude multiplier of 0.5.",
    },
    "s0-seed-strange-quark": {
        title: "Negative Z-Biased Wave · A=0.7",
        intent: "An inert negative marker accompanies an inward radial wave with an added negative z-directed component and an imposed amplitude multiplier of 0.7.",
    },
    "s0-seed-charm-quark": {
        title: "Positive X-Biased Wave · A=1.0",
        intent: "An inert positive marker accompanies an outward radial wave with an added x-directed component and an imposed amplitude multiplier of 1.0.",
    },
    "s0-seed-bottom-quark": {
        title: "Negative Y-Biased Wave · A=1.4",
        intent: "An inert negative marker accompanies an inward radial wave with an added negative y-directed component and an imposed amplitude multiplier of 1.4.",
    },
    "s0-seed-top-quark": {
        title: "Positive Z-Biased Wave · A=2.5",
        intent: "An inert positive marker accompanies an outward radial wave with an added z-directed component and an imposed amplitude multiplier of 2.5.",
    },
    "s0-seed-anti-up-quark": {
        title: "Negative X-Biased Wave · A=0.5",
        intent: "An inert negative marker accompanies an inward radial wave with an added negative x-directed component and an imposed amplitude multiplier of 0.5.",
    },
    "s0-seed-anti-down-quark": {
        title: "Positive Y-Biased Wave · A=0.5",
        intent: "An inert positive marker accompanies an outward radial wave with an added y-directed component and an imposed amplitude multiplier of 0.5.",
    },
    "s0-seed-anti-strange-quark": {
        title: "Positive Z-Biased Wave · A=0.7",
        intent: "An inert positive marker accompanies an outward radial wave with an added z-directed component and an imposed amplitude multiplier of 0.7.",
    },
    "s0-seed-anti-charm-quark": {
        title: "Negative X-Biased Wave · A=1.0",
        intent: "An inert negative marker accompanies an inward radial wave with an added negative x-directed component and an imposed amplitude multiplier of 1.0.",
    },
    "s0-seed-anti-bottom-quark": {
        title: "Positive Y-Biased Wave · A=1.4",
        intent: "An inert positive marker accompanies an outward radial wave with an added y-directed component and an imposed amplitude multiplier of 1.4.",
    },
    "s0-seed-anti-top-quark": {
        title: "Negative Z-Biased Wave · A=2.5",
        intent: "An inert negative marker accompanies an inward radial wave with an added negative z-directed component and an imposed amplitude multiplier of 2.5.",
    },

    // Particle Models · Bosons
    "s0-seed-higgs-field": {
        title: "Modulated Vector Field Background",
        intent: "A volume-filling vector field with deterministic sinusoidal variations evolves under the source-free wave map.",
    },
    "s0-seed-gluon": {
        title: "Gaussian Packet with Crossed Vector Components",
        intent: "A Gaussian field in the y component and an initial field rate in the x component evolve with their spatial means removed.",
    },
    "s0-vacuum-photon": {
        title: "Transverse Plane-Wave Packet",
        intent: "A divergence-free transverse plane packet travels through the isolated wave sector while genesis remains disabled.",
    },
    "s0-vacuum-w-boson": {
        title: "Anisotropic Radial Field · Positive Marker",
        intent: "An inert positive marker accompanies an outward radial Gaussian vector field whose x component is multiplied by 1.3.",
    },
    "s0-vacuum-w-minus-boson": {
        title: "Anisotropic Radial Field · Negative Marker",
        intent: "An inert negative marker accompanies the sign-reversed anisotropic radial Gaussian vector field whose x component is multiplied by 1.3.",
    },
    "s0-vacuum-z-boson": {
        title: "Inward Radial Field without Markers",
        intent: "An inward radial Gaussian vector field evolves under the source-free wave map without manifested markers.",
    },
    "s0-vacuum-higgs": {
        title: "Equal-Component Gaussian Vector Field",
        intent: "A localized Gaussian envelope with equal x, y, and z vector components evolves under the source-free wave map.",
    },

    // Particle Models · Hadrons
    "s0-vacuum-proton": {
        title: "Mixed-Polarity Triad · Net +1",
        intent: "Three unlocked sites with polarities +1, +1, and -1 evolve under the selected dressing, color-force, and movement rules.",
    },
    "s0-vacuum-neutron": {
        title: "Mixed-Polarity Triad · Net −1",
        intent: "Three unlocked sites with polarities +1, -1, and -1 evolve under the selected dressing, color-force, and movement rules.",
    },
    "s0-vacuum-pion-charged": {
        title: "Opposite-Polarity Pair with Color Forces",
        intent: "An unlocked opposite-polarity pair with imposed field dressing evolves under selected color forces and the movement collision rule.",
    },
    "s0-vacuum-pion-neutral": {
        title: "Opposite-Polarity Pair · Duplicate Preparation",
        intent: "This preparation exactly repeats the opposite-polarity color-force pair, including its initial records and subsequent evolution.",
    },
    "s0-vacuum-kaon-charged": {
        title: "Opposite-Polarity Pair · 1.88× Field Dressing",
        intent: "The unlocked opposite-polarity pair receives 1.88 times the base field dressing while retaining the same selected force and movement rules.",
    },

    // Atomic & Molecular Models
    "s0-seed-hydrogen": {
        title: "Locked Triad with Mobile Negative Marker",
        intent: "A mobile negative marker responds to a prepared three-site locked source under the Poisson-Coulomb force and movement rules.",
    },
    "s0-seed-helium": {
        title: "Tetrahedral Source Cluster with Two Markers",
        intent: "Two mobile negative markers interact with twelve locked source sites arranged as four triads at tetrahedral vertices under the Poisson-Coulomb rule.",
    },
    "s0-seed-h2-bond-formation": {
        title: "Two Locked Triads with Mobile Pair",
        intent: "Two central mobile negative markers evolve between two prepared locked triads under the Poisson-Coulomb force and movement rules.",
    },

    // Gravity & Clocks
    "s0-seed-schwarzschild": {
        title: "Static Inward Inverse-Square Field",
        intent: "Inspect an imposed inward radial vector field around a central state with evolution disabled.",
    },
    "s0-seed-gravitational-lensing": {
        title: "Radial-Field Packet Superposition Control",
        intent: "Compare packet propagation with and without a radial background under the linear wave operator.",
    },
    "s0-seed-gravitational-wave": {
        title: "Transverse Lattice Harmonic n=4",
        intent: "Track a transverse harmonic under the periodic native wave operator.",
    },
    "s0-seed-massive-body": {
        title: "Locked-Source Latency Poisson Field",
        intent: "Measure the static latency field sourced by a compact locked-state cluster under the selected Poisson law.",
    },
    "s0-seed-time-gravity-well": {
        title: "Transverse Harmonic — Gravity-Well Alias",
        intent: "Run the exact transverse-harmonic alias retained from the retired gravity-well interpretation.",
    },
    "s0-seed-time-twin-clocks": {
        title: "Transverse Harmonic — Twin-Clock Alias",
        intent: "Run the exact transverse-harmonic alias retained from the retired twin-clock interpretation.",
    },
    "s0-seed-time-horizon": {
        title: "Inward Radial Field — Horizon Alias",
        intent: "Inspect the inert inverse-square-field alias retained from the retired horizon interpretation.",
    },
    "s0-seed-de-broglie-clock": {
        title: "Imposed Klein–Gordon Block Response",
        intent: "Track a field block under wave propagation and a prescribed local mass-term restoring force.",
    },

    // Geometry & Topology
    "flux-vortex": {
        title: "Static Helical Field Ring",
        intent: "Inspect an imposed ring field with circulation and axial bias while evolution is disabled.",
    },
    "flux-triad": {
        title: "Threefold Inward-Field State Preparation",
        intent: "Inspect three positive states with imposed inward field dressing and all binding terms disabled.",
    },
    "s0-seed-wilson-loop": {
        title: "Oriented Square Field Path",
        intent: "Inspect a static vector field arranged along an oriented square lattice path.",
    },
    "s0-seed-flux-tube": {
        title: "Static Gaussian Axial Field Tube",
        intent: "Inspect a prescribed Gaussian axial field profile between two opposite endpoint states.",
    },
    "s0-seed-instanton": {
        title: "Static Localized Radial Field",
        intent: "Inspect an imposed radial three-vector profile with no topological-charge operator.",
    },
    "s0-seed-sloop": {
        title: "Twelve-Site Tangential Field Ring",
        intent: "Inspect twelve coplanar positive states with equal tangential field magnitudes and zero net vector flux.",
    },
    "s0-seed-observer-cell": {
        title: "Alternating Moore-Shell State Labels",
        intent: "Inspect prescribed alternating ternary labels on the center and three Moore shells.",
    },
    "s0-field-vortex-line": {
        title: "Azimuthal Inverse-Radius Field",
        intent: "Inspect a static tangential field whose magnitude decreases inversely with distance from its axis.",
    },
    "s0-seed-octahedron": {
        title: "Octahedral Moore Face Shell",
        intent: "Inspect the six face-neighbor states and central state forming the imposed octahedral construction.",
    },
    "s0-seed-cuboctahedron": {
        title: "Cuboctahedral Moore Edge Shell",
        intent: "Inspect the twelve edge-neighbor states and central state forming the imposed cuboctahedral construction.",
    },
    "s0-seed-stella-octangula": {
        title: "Stella Octangula Moore Corner Shell",
        intent: "Inspect the eight corner-neighbor states and central state forming the imposed shell construction.",
    },
    "s0-seed-moore-cell": {
        title: "Complete 27-Site Moore Cell",
        intent: "Inspect the center and all 26 neighboring states of the imposed Moore cell.",
    },
    "s0-seed-moore-decomposition": {
        title: "Moore Shell Decomposition 1+6+12+8",
        intent: "Inspect the center, face, edge and corner shells using prescribed alternating state labels.",
    },
});

for (const presentation of Object.values(SCALE0_SCENARIO_PRESENTATION)) {
    Object.freeze(presentation);
}
