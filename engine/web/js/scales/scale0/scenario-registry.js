
function makeScenario(category, id, title, tags = [], epistemicStatus = '[OPEN]') {
    return {
        id,
        scale: 'lattice',
        title,
        category,
        tags,
        defaultParams: {},
        requiredCapabilities: ['scale0'],
        epistemicStatus,
        load(harness, params = {}) {
            harness.setupScenario(params.id || id);
        },
    };
}

export const SCALE0_SCENARIOS = [
    /*
     * Scenario: empty (Empty Lattice)
     * Physical purpose: Serves as the baseline state of the lattice with no initial particles or fields.
     * Parameters: None.
     * Expected behavior: The lattice remains completely quiet and empty.
     * Discrepancy: None.
     */
    makeScenario('1. Foundational Dynamics & Substrate', 'empty', 'Empty Lattice', ['baseline']),
    /*
     * Scenario: flux-pulse (Flux Pulse)
     * Physical purpose: Demonstrates single wave/flux pulse propagation through the discrete lattice.
     * Parameters: None.
     * Expected behavior: Spherical expansion of flux pulse outward from center.
     * Discrepancy: None.
     */
    makeScenario('1. Foundational Dynamics & Substrate', 'flux-pulse', 'Flux Pulse', ['flux', 'wave']),
    /*
     * Scenario: flux-dipole (Flux Dipole)
     * Physical purpose: Simulates a positive and negative flux dipole pair.
     * Parameters: None.
     * Expected behavior: Dipole field configuration that propagates and interacts.
     * Discrepancy: None.
     */
    makeScenario('1. Foundational Dynamics & Substrate', 'flux-dipole', 'Flux Dipole', ['flux', 'wave']),
    /*
     * Scenario: flux-standing (Standing Wave)
     * Physical purpose: Establishes a standing wave pattern in the lattice.
     * Parameters: None.
     * Expected behavior: Constructive and destructive interference forming stable standing wave peaks and troughs.
     * Discrepancy: None.
     */
    makeScenario('1. Foundational Dynamics & Substrate', 'flux-standing', 'Standing Wave', ['flux', 'wave']),
    /*
     * Scenario: flux-nested-standing (Nested Standing)
     * Physical purpose: Establishes a nested standing wave pattern across orthogonal dimensions.
     * Parameters: None.
     * Expected behavior: Orthogonally overlapping standing waves creating a grid-like interference pattern.
     * Discrepancy: None.
     */
    makeScenario('1. Foundational Dynamics & Substrate', 'flux-nested-standing', 'Nested Standing', ['flux', 'wave']),
    /*
     * Scenario: flux-soliton (Soliton)
     * Physical purpose: Simulates non-dispersive soliton wave propagation.
     * Parameters: None.
     * Expected behavior: Soliton maintains shape during propagation without dispersing or producing particle pairs.
     * Discrepancy: None.
     */
    makeScenario('1. Foundational Dynamics & Substrate', 'flux-soliton', 'Soliton', ['flux', 'wave']),
    /*
     * Scenario: flux-interference (4-Source Interference)
     * Physical purpose: Demonstrates interference patterns using four point-like flux sources.
     * Parameters: None.
     * Expected behavior: Multi-source constructive and destructive wave interference patterns.
     * Discrepancy: None.
     */
    makeScenario('1. Foundational Dynamics & Substrate', 'flux-interference', '4-Source Interference', ['flux', 'wave']),
    /*
     * Scenario: flux-vortex (Flux Vortex (Spin))
     * Physical purpose: Models a rotating flux vortex to demonstrate spin.
     * Parameters: None.
     * Expected behavior: Coherent angular momentum/spin rotation of the flux field.
     * Discrepancy: None.
     */
    makeScenario('1. Foundational Dynamics & Substrate', 'flux-vortex', 'Flux Vortex (Spin)', ['flux', 'spin']),
    /*
     * Scenario: flux-dual-substrate (Dual Substrate)
     * Physical purpose: Explores wave propagation under a dual-substrate (two independent vacuum states) configuration.
     * Parameters: None.
     * Expected behavior: Distinct propagation characteristics and interference patterns on the dual substrate.
     * Discrepancy: None.
     */
    makeScenario('1. Foundational Dynamics & Substrate', 'flux-dual-substrate', 'Dual Substrate', ['flux', 'dual-substrate']),
    /*
     * Scenario: flux-cascade (Genesis Cascade)
     * Physical purpose: Models cascading genesis events where high energy triggers a chain reaction of particle creation.
     * Parameters: None.
     * Expected behavior: Rapid series of pair-production events cascading outward.
     * Discrepancy: None.
     */
    makeScenario('2. Genesis & Emergence', 'flux-cascade', 'Genesis Cascade', ['genesis']),
    /*
     * Scenario: flux-random-genesis (Random Genesis)
     * Physical purpose: Demonstrates spontaneous particle creation from random vacuum fluctuations.
     * Parameters: None.
     * Expected behavior: Stochastic nucleation of particle-antiparticle pairs across the lattice.
     * Discrepancy: None.
     */
    makeScenario('2. Genesis & Emergence', 'flux-random-genesis', 'Random Genesis', ['genesis']),
    /*
     * Scenario: flux-genesis-between-gates (Genesis: Between the Gates)
     * Physical purpose: Empirical discriminator for the FTD-0388 genesis-gate adoption — three frozen uniform-flux bands at |J| = 1.5160 / 1.5250 / 1.5340 straddle the adopted gate K_GENESIS = 3·W_SC = 1.5164 and the retired 3·K_B = 1.533 gate.
     * Parameters: Field-freezing toggle overrides (config/toggles.js) keep the band amplitudes exact; genesis is the only active dynamics.
     * Expected behavior: Left band stays void forever; middle band nucleates matter the pre-FTD-0388 engine could never create; right band nucleates ~2x faster.
     * Discrepancy: None.
     */
    makeScenario('2. Genesis & Emergence', 'flux-genesis-between-gates', 'Genesis: Between the Gates', ['genesis', 'ftd-0388'], '[SELECTION]'),
    /*
     * Scenario: s0-seed-ew-phase-transition (EW Phase Transition (Hysteresis))
     * Physical purpose: Simulates the Electroweak phase transition showing hysteresis using a uniform flux sweep.
     * Parameters: None.
     * Expected behavior: Gradual shift in state field as uniform flux is sinusoidally swept.
     * Discrepancy: None.
     */
    makeScenario('2. Genesis & Emergence', 's0-seed-ew-phase-transition', 'EW Phase Transition (Hysteresis)', ['seed', 'genesis', 'hysteresis'], '[DERIVED]'),
    /*
     * Scenario: flux-pair-production (Pair Production)
     * Physical purpose: Demonstrates particle-antiparticle pair creation from a high-energy field.
     * Parameters: None.
     * Expected behavior: Local flux exceeds threshold and collapses into stable discrete particle-antiparticle pairs.
     * Discrepancy: None.
     */
    makeScenario('2. Genesis & Emergence', 'flux-pair-production', 'Pair Production', ['genesis']),
    /*
     * Scenario: flux-annihilation (Pair Annihilation)
     * Physical purpose: Simulates particle-antiparticle pair annihilation.
     * Parameters: None.
     * Expected behavior: Particles move together, collide, and annihilate, releasing their mass energy as outgoing flux waves.
     * Discrepancy: None.
     */
    makeScenario('2. Genesis & Emergence', 'flux-annihilation', 'Pair Annihilation', ['genesis']),
    /*
     * Scenario: flux-vacuum-foam (Vacuum Fluctuations)
     * Physical purpose: Models quantum vacuum fluctuations (spacetime foam) at the Planck scale.
     * Parameters: None.
     * Expected behavior: Continuous, stochastic boiling and fluctuation of the background flux field.
     * Discrepancy: None.
     */
    makeScenario('2. Genesis & Emergence', 'flux-vacuum-foam', 'Vacuum Fluctuations', ['genesis']),
    /*
     * Scenario: flux-meson (Meson (Confinement))
     * Physical purpose: Demonstrates color confinement in a quark-antiquark meson system.
     * Parameters: None.
     * Expected behavior: The flux remains confined to a string-like region between the quarks, preventing them from separating freely.
     * Discrepancy: None.
     */
    makeScenario('3. Particles & The Standard Model', 'flux-meson', 'Meson (Confinement)', ['confinement']),
    /*
     * Scenario: flux-string-breaking (String Breaking)
     * Physical purpose: Models QCD string breaking when quarks are pulled apart with high energy.
     * Parameters: None.
     * Expected behavior: As the quarks separate, the tension in the connecting string increases until it snaps, producing a new quark-antiquark pair from the vacuum.
     * Discrepancy: None.
     */
    makeScenario('3. Particles & The Standard Model', 'flux-string-breaking', 'String Breaking', ['confinement']),
    /*
     * Scenario: flux-baryon (Baryon (3-Quark))
     * Physical purpose: Models a 3-quark baryon bound state.
     * Parameters: None.
     * Expected behavior: Stable 3-quark configuration bound by the central flux structure.
     * Discrepancy: None.
     */
    makeScenario('3. Particles & The Standard Model', 'flux-baryon', 'Baryon (3-Quark)', ['confinement']),
    /*
     * Scenario: flux-cyclotron (Cyclotron Motion)
     * Physical purpose: Simulates cyclotron motion of a charged particle in a magnetic field.
     * Parameters: None.
     * Expected behavior: The particle undergoes circular orbital motion due to the Lorentz-like force.
     * Discrepancy: None.
     */
    makeScenario('1. Foundational Dynamics & Substrate', 'flux-cyclotron', 'Cyclotron Motion', ['substrate']),
    /*
     * Scenario: flux-screening (Charge Screening)
     * Physical purpose: Demonstrates electric charge screening in a dielectric or plasma-like medium.
     * Parameters: None.
     * Expected behavior: The net electric field at large distances is reduced (screened) by the surrounding opposite charges.
     * Discrepancy: None.
     */
    makeScenario('1. Foundational Dynamics & Substrate', 'flux-screening', 'Charge Screening', ['substrate']),
    /*
     * Scenario: flux-thermalization (Thermalization)
     * Physical purpose: Simulates a system of random waves relaxing towards thermal equilibrium.
     * Parameters: None.
     * Expected behavior: Energy diffuses and thermalizes, distributing evenly across the lattice modes over time.
     * Discrepancy: None.
     */
    makeScenario('1. Foundational Dynamics & Substrate', 'flux-thermalization', 'Thermalization', ['substrate']),
    /*
     * Scenario: flux-triad (Triad Formation)
     * Physical purpose: Simulates the formation of a stable three-body triad structure.
     * Parameters: None.
     * Expected behavior: Particles and fields bind together into a stable triad structure.
     * Discrepancy: None.
     */
    makeScenario('1. Foundational Dynamics & Substrate', 'flux-triad', 'Triad Formation', ['substrate']),
    /*
     * Scenario: flux-zero-point (Zero-Point Energy)
     * Physical purpose: Models the irreducible quantum ground-state vacuum energy.
     * Parameters: None.
     * Expected behavior: Persistent low-amplitude random background flux that does not trigger genesis.
     * Discrepancy: None.
     */
    makeScenario('1. Foundational Dynamics & Substrate', 'flux-zero-point', 'Zero-Point Energy', ['substrate', 'vacuum']),
    /*
     * Scenario: light-rainbow (Rainbow (3 Colors))
     * Physical purpose: Demonstrates dispersion of light waves of three different wavelengths/colors.
     * Parameters: None.
     * Expected behavior: Dispersion and spatial separation of the wave frequencies.
     * Discrepancy: None.
     */
    makeScenario('4. Macroscopic Physics & Measurement', 'light-rainbow', 'Rainbow (3 Colors)', ['light', 'em']),
    /*
     * Scenario: light-dipole (Dipole Radiation)
     * Physical purpose: Simulates classical electromagnetic dipole radiation.
     * Parameters: None.
     * Expected behavior: Spherically propagating radiation fields representing dipole emissions.
     * Discrepancy: None.
     */
    makeScenario('4. Macroscopic Physics & Measurement', 'light-dipole', 'Dipole Radiation', ['light', 'em']),
    /*
     * Scenario: light-two-slit (Two-Slit Interference)
     * Physical purpose: Simulates the classic double-slit wave interference experiment.
     * Parameters: None.
     * Expected behavior: Interference fringes propagating downstream of the double slits.
     * Discrepancy: None.
     */
    makeScenario('4. Macroscopic Physics & Measurement', 'light-two-slit', 'Two-Slit Interference', ['light', 'em']),
    /*
     * Scenario: light-photon-race (Photon Race)
     * Physical purpose: Compares propagation characteristics of photons/wave packets of different amplitudes.
     * Parameters: None.
     * Expected behavior: Propagation of the two wave packets across the lattice, demonstrating non-linear or linear wave speeds.
     * Discrepancy: None.
     */
    makeScenario('4. Macroscopic Physics & Measurement', 'light-photon-race', 'Photon Race', ['light', 'em']),
    /*
     * Scenario: quantum-born-rule (Born Rule Test)
     * Physical purpose: Tests the quantum Born rule / wave-function collapse.
     * Parameters: None.
     * Expected behavior: Localized wave packet collapses stochastically into particles with Born-rule probability.
     * Discrepancy: None.
     */
    makeScenario('5. Quantum Lab & Foundations', 'quantum-born-rule', 'Born Rule Test', ['quantum']),
    /*
     * Scenario: quantum-double-slit (Double-Slit (Quantitative))
     * Physical purpose: Quantitatively simulates double-slit quantum particle interference.
     * Parameters: None.
     * Expected behavior: Emergence of interference pattern from accumulated single particle impacts.
     * Discrepancy: None.
     */
    makeScenario('5. Quantum Lab & Foundations', 'quantum-double-slit', 'Double-Slit (Quantitative)', ['quantum']),
    /*
     * Scenario: quantum-eraser (Quantum Eraser (which-way))
     * Physical purpose: Simulates the quantum eraser experiment with path polarization markers.
     * Parameters: None.
     * Expected behavior: Restoring interference pattern when path information is erased by polarizer grid.
     * Discrepancy: None.
     */
    makeScenario('5. Quantum Lab & Foundations', 'quantum-eraser', 'Quantum Eraser (which-way)', ['quantum']),
    /*
     * Scenario: quantum-tunnel (Quantum Tunneling)
     * Physical purpose: Demonstrates quantum tunneling of a wave packet through a potential barrier.
     * Parameters: None.
     * Expected behavior: Partial transmission of a wave packet through a thin potential barrier.
     * Discrepancy: None.
     */
    makeScenario('5. Quantum Lab & Foundations', 'quantum-tunnel', 'Quantum Tunneling', ['quantum']),
    /*
     * Scenario: quantum-well (Particle in a Box)
     * Physical purpose: Simulates a particle in a 1D potential well (infinite square well).
     * Parameters: None.
     * Expected behavior: Discrete standing wave harmonics bound by potential walls.
     * Discrepancy: None.
     */
    makeScenario('5. Quantum Lab & Foundations', 'quantum-well', 'Particle in a Box', ['quantum']),
    /*
     * Scenario: quantum-entangle (Entanglement Correlation)
     * Physical purpose: Tests entanglement correlation between two separated particles.
     * Parameters: None.
     * Expected behavior: Correlated behavior of two particles originating from a single high-energy flux burst.
     * Discrepancy: None.
     */
    makeScenario('5. Quantum Lab & Foundations', 'quantum-entangle', 'Entanglement Correlation', ['quantum', 'entangle']),
    /*
     * Scenario: quantum-aharonov-bohm (Aharonov-Bohm Effect)
     * Physical purpose: Simulates the Aharonov-Bohm effect where a vector potential shifts phase.
     * Parameters: None.
     * Expected behavior: Phase shift in the interference pattern of two paths wrapping a solenoid.
     * Discrepancy: None.
     */
    makeScenario('5. Quantum Lab & Foundations', 'quantum-aharonov-bohm', 'Aharonov-Bohm Effect', ['quantum']),
    /*
     * Scenario: quantum-casimir (Casimir Effect)
     * Physical purpose: Models the Casimir attraction force between parallel plates.
     * Parameters: None.
     * Expected behavior: Modification of vacuum fluctuations between plates leading to net attractive force.
     * Discrepancy: None.
     */
    makeScenario('5. Quantum Lab & Foundations', 'quantum-casimir', 'Casimir Effect', ['quantum']),
    /*
     * Scenario: quantum-zeno (Quantum Zeno Effect)
     * Physical purpose: Simulates the Quantum Zeno effect (frequent observation slows evolution).
     * Parameters: None.
     * Expected behavior: Continuous measurement/interaction freezes quantum state transition.
     * Discrepancy: None.
     */
    makeScenario('5. Quantum Lab & Foundations', 'quantum-zeno', 'Quantum Zeno Effect', ['quantum']),

    // LHC Standard Model — quark flavours (2026-04-17)
    /*
     * Scenario: s0-seed-up-quark (Up quark (u, 1st gen, +2/3))
     * Physical purpose: Seeds a valence up quark (+2/3 charge, 1st gen).
     * Parameters: None.
     * Expected behavior: Localized +1 charge and fractional color-polarized flux envelope.
     * Discrepancy: None.
     */
    makeScenario('3. Particles & The Standard Model', 's0-seed-up-quark', 'Up quark (u, 1st gen, +2/3)', ['seed', 'sm'], '[CONJECTURE]'),
    /*
     * Scenario: s0-seed-down-quark (Down quark (d, 1st gen, −1/3))
     * Physical purpose: Seeds a valence down quark (-1/3 charge, 1st gen).
     * Parameters: None.
     * Expected behavior: Localized -1 charge and fractional color-polarized flux envelope.
     * Discrepancy: None.
     */
    makeScenario('3. Particles & The Standard Model', 's0-seed-down-quark', 'Down quark (d, 1st gen, −1/3)', ['seed', 'sm'], '[CONJECTURE]'),
    /*
     * Scenario: s0-seed-strange-quark (Strange quark (s, 2nd gen))
     * Physical purpose: Seeds a strange quark (2nd gen).
     * Parameters: None.
     * Expected behavior: Localized -1 charge and intermediate-strength color-polarized flux envelope.
     * Discrepancy: None.
     */
    makeScenario('3. Particles & The Standard Model', 's0-seed-strange-quark', 'Strange quark (s, 2nd gen)', ['seed', 'sm'], '[CONJECTURE]'),
    /*
     * Scenario: s0-seed-charm-quark (Charm quark (c, 2nd gen, m≈1.27 GeV))
     * Physical purpose: Seeds a charm quark (2nd gen).
     * Parameters: None.
     * Expected behavior: Localized +1 charge and higher-strength color-polarized flux envelope.
     * Discrepancy: None.
     */
    makeScenario('3. Particles & The Standard Model', 's0-seed-charm-quark', 'Charm quark (c, 2nd gen, m≈1.27 GeV)', ['seed', 'sm'], '[CONJECTURE]'),
    /*
     * Scenario: s0-seed-bottom-quark (Bottom quark (b, 3rd gen, m≈4.2 GeV))
     * Physical purpose: Seeds a bottom quark (3rd gen).
     * Parameters: None.
     * Expected behavior: Localized -1 charge and high-strength color-polarized flux envelope.
     * Discrepancy: None.
     */
    makeScenario('3. Particles & The Standard Model', 's0-seed-bottom-quark', 'Bottom quark (b, 3rd gen, m≈4.2 GeV)', ['seed', 'sm'], '[CONJECTURE]'),
    /*
     * Scenario: s0-seed-top-quark (Top quark (t, 3rd gen, m≈v_Higgs))
     * Physical purpose: Seeds a top quark (3rd gen).
     * Parameters: None.
     * Expected behavior: Localized +1 charge and very high-strength color-polarized flux envelope.
     * Discrepancy: None.
     */
    makeScenario('3. Particles & The Standard Model', 's0-seed-top-quark', 'Top quark (t, 3rd gen, m≈v_Higgs)', ['seed', 'sm'], '[CONJECTURE]'),

    // LHC Standard Model — gauge + Higgs (2026-04-17)
    /*
     * Scenario: s0-seed-higgs-field (Higgs field vacuum (VEV background))
     * Physical purpose: Seeds a Higgs field vacuum expectation value (VEV) background.
     * Parameters: None.
     * Expected behavior: Spatially uniform static background flux with minor sinusoidal noise.
     * Discrepancy: None.
     */
    makeScenario('3. Particles & The Standard Model', 's0-seed-higgs-field', 'Higgs field vacuum (VEV background)', ['seed', 'sm'], '[CONJECTURE]'),
    /*
     * Scenario: s0-seed-gluon (Gluon (massless, colored))
     * Physical purpose: Seeds a massless, colored gauge boson (gluon).
     * Parameters: None.
     * Expected behavior: A localized transverse color-polarized wave packet propagating across the lattice.
     * Discrepancy: None.
     */
    makeScenario('3. Particles & The Standard Model', 's0-seed-gluon', 'Gluon (massless, colored)', ['seed', 'sm'], '[CONJECTURE]'),

    // LHC Standard Model — processes (2026-04-17)
    /*
     * Scenario: s0-seed-beta-decay (Beta decay (n → p + e⁻ + ν̅, dynamic))
     * Physical purpose: Models dynamic nuclear beta decay.
     * Parameters: None.
     * Expected behavior: A neutron triad transmuting into a proton triad, emitting an electron and an antineutrino.
     * Discrepancy: None.
     */
    makeScenario('3. Particles & The Standard Model', 's0-seed-beta-decay', 'Beta decay (n → p + e⁻ + ν̅, dynamic)', ['seed', 'sm', 'process'], '[CONJECTURE]'),
    /*
     * Scenario: s0-seed-ee-annihilation (e⁺ e⁻ annihilation (collision → flux burst))
     * Physical purpose: Simulates electron-positron high-energy annihilation.
     * Parameters: None.
     * Expected behavior: Opposing charges collide and dissolve, generating a spherical high-energy flux burst.
     * Discrepancy: None.
     */
    makeScenario('3. Particles & The Standard Model', 's0-seed-ee-annihilation', 'e⁺ e⁻ annihilation (collision → flux burst)', ['seed', 'sm', 'process'], '[CONJECTURE]'),
    /*
     * Scenario: s0-seed-quark-gluon-plasma (Quark-gluon plasma (QGP, thermal deconfined))
     * Physical purpose: Models a thermal deconfined Quark-Gluon Plasma (QGP).
     * Parameters: None.
     * Expected behavior: Multi-particle high-velocity gas of quarks and gluons under high Langevin temperature.
     * Discrepancy: None.
     */
    makeScenario('3. Particles & The Standard Model', 's0-seed-quark-gluon-plasma', 'Quark-gluon plasma (QGP, thermal deconfined)', ['seed', 'sm', 'process'], '[CONJECTURE]'),
    /*
     * Scenario: s0-seed-hydrogen (Hydrogen atom)
     * Physical purpose: Seeds a hydrogen atom (proton triad and electron cloud).
     * Parameters: None.
     * Expected behavior: A localized three-quark triad at the center with a single electron cloud bound to it.
     * Discrepancy: None.
     */
    makeScenario('4. Macroscopic Physics & Measurement', 's0-seed-hydrogen', 'Hydrogen atom', ['seed', 'atom'], '[CONJECTURE]'),
    /*
     * Scenario: s0-seed-helium (Helium atom (⁴He, 2p+2n + 1s²))
     * Physical purpose: Seeds a helium atom (four-nucleon nucleus and two electron clouds).
     * Parameters: None.
     * Expected behavior: Tetrahedrally arranged nucleons with two surrounding bound electron clouds.
     * Discrepancy: None.
     */
    makeScenario('4. Macroscopic Physics & Measurement', 's0-seed-helium', 'Helium atom (⁴He, 2p+2n + 1s²)', ['seed', 'atom'], '[CONJECTURE]'),
    /*
     * Scenario: s0-seed-h2-bond-formation (H₂ covalent bond formation (dynamic))
     * Physical purpose: Models covalent bond formation in H2.
     * Parameters: None.
     * Expected behavior: Two adjacent proton triads sharing a pair of electron clouds.
     * Discrepancy: None.
     */
    makeScenario('4. Macroscopic Physics & Measurement', 's0-seed-h2-bond-formation', 'H₂ covalent bond formation (dynamic)', ['seed'], '[CONJECTURE]'),
    /*
     * Scenario: s0-seed-spark-of-life (Spark of Life (abiogenesis threshold))
     * Physical purpose: Pedagogical demo of pre-biotic autocatalysis and threshold genesis.
     * Parameters: None.
     * Expected behavior: High-energy deterministic flux spark interacting with a mineral-pore boundary and precursor charge pairs.
     * Discrepancy: None.
     */
    makeScenario('4. Macroscopic Physics & Measurement', 's0-seed-spark-of-life', 'Spark of Life (abiogenesis threshold)', ['seed', 'genesis', 'life', 'abiogenesis', 'autocatalytic', 'demo'], '[DEMO]'),
    /*
     * Scenario: s0-seed-wilson-loop (Wilson loop)
     * Physical purpose: Implements a rectangular Wilson loop to probe confinement.
     * Parameters: None.
     * Expected behavior: A closed rectangular path of localized gauge flux with four corner charges.
     * Discrepancy: None.
     */
    makeScenario('5. Quantum Lab & Foundations', 's0-seed-wilson-loop', 'Wilson loop', ['seed'], '[CONJECTURE]'),
    /*
     * Scenario: s0-seed-flux-tube (Flux tube (q-qbar))
     * Physical purpose: Seeds a quark-antiquark flux tube string.
     * Parameters: None.
     * Expected behavior: Spherically dressed opposite charges connected by a dense flux tube.
     * Discrepancy: None.
     */
    makeScenario('5. Quantum Lab & Foundations', 's0-seed-flux-tube', 'Flux tube (q-qbar)', ['seed'], '[CONJECTURE]'),
    /*
     * Scenario: s0-seed-monopole (Magnetic monopole)
     * Physical purpose: Seeds a magnetic monopole configuration.
     * Parameters: None.
     * Expected behavior: Radial magnetic field structure emanating from center.
     * Discrepancy: None.
     */
    makeScenario('5. Quantum Lab & Foundations', 's0-seed-monopole', 'Magnetic monopole', ['seed'], '[CONJECTURE]'),
    /*
     * Scenario: s0-seed-instanton (Instanton)
     * Physical purpose: Seeds a Yang-Mills instanton configuration.
     * Parameters: None.
     * Expected behavior: Localized topological field configuration representing tunneling between vacuum states.
     * Discrepancy: None.
     */
    makeScenario('5. Quantum Lab & Foundations', 's0-seed-instanton', 'Instanton', ['seed'], '[CONJECTURE]'),
    /*
     * Scenario: s0-seed-schwarzschild (Schwarzschild well)
     * Physical purpose: Seeds a Schwarzschild gravitational mass well.
     * Parameters: None.
     * Expected behavior: Central mass charge with a radial 1/r^2 flux force field.
     * Discrepancy: None.
     */
    makeScenario('4. Macroscopic Physics & Measurement', 's0-seed-schwarzschild', 'Schwarzschild well', ['seed', 'gravity'], '[CONJECTURE]'),
    /*
     * Scenario: s0-seed-gravitational-lensing (Gravitational lensing (dynamic bending))
     * Physical purpose: Models light bending (gravitational lensing) around a Schwarzschild mass.
     * Parameters: None.
     * Expected behavior: Off-axis photon packet propagating past a central mass, experiencing trajectory deflection.
     * Discrepancy: None.
     */
    makeScenario('4. Macroscopic Physics & Measurement', 's0-seed-gravitational-lensing', 'Gravitational lensing (dynamic bending)', ['seed'], '[CONJECTURE]'),
    /*
     * Scenario: s0-seed-gravitational-wave (Gravitational wave)
     * Physical purpose: Seeds a gravitational wave propagation scenario.
     * Parameters: None.
     * Expected behavior: Transverse sinusoidal oscillations in the flux field propagating across the lattice.
     * Discrepancy: None.
     */
    makeScenario('4. Macroscopic Physics & Measurement', 's0-seed-gravitational-wave', 'Gravitational wave', ['seed', 'gravity'], '[CONJECTURE]'),
    /*
     * Scenario: s0-seed-massive-body (Massive body (real mass))
     * Physical purpose: Seeds a massive body using real manifested mass (locked).
     * Parameters: None.
     * Expected behavior: Central dense core of locked mass that sources gravity via the Poisson equation.
     * Discrepancy: None.
     */
    makeScenario('4. Macroscopic Physics & Measurement', 's0-seed-massive-body', 'Massive body (real mass)', ['seed', 'gravity'], '[DERIVED]'),
    /*
     * Scenario: s0-seed-time-gravity-well (Gravity well (dτ/dt across a well))
     * Physical purpose: Models clock slowdown inside a gravitational well (Time Observatory).
     * Parameters: None.
     * Expected behavior: Sinusoidal flux wave producing localized clock dilation dτ/dt.
     * Discrepancy: None.
     */
    makeScenario('4. Macroscopic Physics & Measurement', 's0-seed-time-gravity-well', 'Gravity well (dτ/dt across a well)', ['seed', 'time', 'gravity'], '[CONJECTURE]'),
    /*
     * Scenario: s0-seed-time-twin-clocks (Twin clocks (Δτ deep vs far))
     * Physical purpose: Compares elapsed time (twin clocks paradox) in different gravity regions.
     * Parameters: None.
     * Expected behavior: Distinct local proper times dτ/dt accumulated at different spatial coordinates.
     * Discrepancy: None.
     */
    makeScenario('4. Macroscopic Physics & Measurement', 's0-seed-time-twin-clocks', 'Twin clocks (Δτ deep vs far)', ['seed', 'time', 'gravity'], '[CONJECTURE]'),
    /*
     * Scenario: s0-seed-time-horizon (Horizon well (deep dilation))
     * Physical purpose: Models deep time dilation near a black hole horizon.
     * Parameters: None.
     * Expected behavior: Strong central mass well showing near-zero dτ/dt dilation at the center.
     * Discrepancy: None.
     */
    makeScenario('4. Macroscopic Physics & Measurement', 's0-seed-time-horizon', 'Horizon well (deep dilation)', ['seed', 'time', 'gravity'], '[CONJECTURE]'),
    /*
     * Scenario: s0-seed-sloop (sLoop (self-referential ring))
     * Physical purpose: Seeds a self-referential sLoop ring.
     * Parameters: None.
     * Expected behavior: Loop of positive charges carrying angular/circulating flux.
     * Discrepancy: None.
     */
    makeScenario('5. Quantum Lab & Foundations', 's0-seed-sloop', 'sLoop (self-referential ring)', ['seed'], '[CONJECTURE]'),
    /*
     * Scenario: s0-seed-observer-cell (Observer cell (3³ lattice))
     * Physical purpose: Seeds an observer cell configuration on a 3^3 lattice.
     * Parameters: None.
     * Expected behavior: Central -1 charge surrounded by shells of +1, -1, and +1 charges.
     * Discrepancy: None.
     */
    makeScenario('5. Quantum Lab & Foundations', 's0-seed-observer-cell', 'Observer cell (3³ lattice)', ['seed'], '[CONJECTURE]'),
    /*
     * Scenario: s0-field-plane-wave (Plane wave)
     * Physical purpose: Establishes a planar electromagnetic wave.
     * Parameters: None.
     * Expected behavior: Plane wave propagating along the x-axis with transverse oscillations.
     * Discrepancy: None.
     */
    makeScenario('1. Foundational Dynamics & Substrate', 's0-field-plane-wave', 'Plane wave', ['field']),
    /*
     * Scenario: s0-field-standing-wave (Standing wave)
     * Physical purpose: Establishes a planar electromagnetic standing wave.
     * Parameters: None.
     * Expected behavior: Stationary sinusoidal node-antinode pattern in the transverse flux.
     * Discrepancy: None.
     */
    makeScenario('1. Foundational Dynamics & Substrate', 's0-field-standing-wave', 'Standing wave', ['field']),
    /*
     * Scenario: s0-field-uniform-e (Uniform E field)
     * Physical purpose: Establishes a uniform electric field.
     * Parameters: None.
     * Expected behavior: Spatially uniform vector potential flux background pointing along x-axis.
     * Discrepancy: None.
     */
    makeScenario('1. Foundational Dynamics & Substrate', 's0-field-uniform-e', 'Uniform E field', ['field']),
    /*
     * Scenario: s0-field-uniform-b (Uniform B field)
     * Physical purpose: Establishes a uniform magnetic field.
     * Parameters: None.
     * Expected behavior: Rotational flux field pattern representing a uniform magnetic field.
     * Discrepancy: None.
     */
    makeScenario('1. Foundational Dynamics & Substrate', 's0-field-uniform-b', 'Uniform B field', ['field']),
    /*
     * Scenario: s0-field-photon-pulse (Photon pulse)
     * Physical purpose: Seeds a propagating photon pulse packet.
     * Parameters: None.
     * Expected behavior: Coherent localized wave packet propagating at light speed along the x-axis.
     * Discrepancy: None.
     */
    makeScenario('1. Foundational Dynamics & Substrate', 's0-field-photon-pulse', 'Photon pulse', ['field']),
    /*
     * Scenario: s0-field-rf-lattice-wave (RF lattice wave)
     * Physical purpose: Models a radio-frequency lattice wave.
     * Parameters: None.
     * Expected behavior: Oscillating wave in the lattice.
     * Discrepancy: None.
     */
    makeScenario('1. Foundational Dynamics & Substrate', 's0-field-rf-lattice-wave', 'RF lattice wave', ['field', 'rf', 'wave', 'wave-lab'], '[INSTRUMENT]'),
    /*
     * Scenario: s0-field-light-lattice-wave (Light lattice wave)
     * Physical purpose: Models a light-frequency lattice wave.
     * Parameters: None.
     * Expected behavior: Oscillating wave in the lattice.
     * Discrepancy: None.
     */
    makeScenario('1. Foundational Dynamics & Substrate', 's0-field-light-lattice-wave', 'Light lattice wave', ['field', 'light', 'wave', 'wave-lab'], '[INSTRUMENT]'),
    /*
     * Scenario: s0-field-sound-lattice-wave (Sound lattice proxy)
     * Physical purpose: Models an acoustic-frequency lattice wave.
     * Parameters: None.
     * Expected behavior: Acoustic wave propagation proxy.
     * Discrepancy: None.
     */
    makeScenario('1. Foundational Dynamics & Substrate', 's0-field-sound-lattice-wave', 'Sound lattice proxy', ['field', 'sound', 'wave', 'wave-lab'], '[INSTRUMENT]'),
    /*
     * Scenario: s0-field-sound-collision (Sound lattice collision)
     * Physical purpose: Models acoustic wave packet collisions.
     * Parameters: None.
     * Expected behavior: Colliding acoustic wave packets.
     * Discrepancy: None.
     */
    makeScenario('1. Foundational Dynamics & Substrate', 's0-field-sound-collision', 'Sound lattice collision', ['field', 'sound', 'wave'], '[INSTRUMENT]'),
    /*
     * Scenario: s0-field-thomson-scattering (Flux recoil locked)
     * Physical purpose: Simulates classical electromagnetic Thomson scattering off a locked charge.
     * Parameters: None.
     * Expected behavior: Plane wave scattering off a stationary central electron.
     * Discrepancy: None.
     */
    makeScenario('1. Foundational Dynamics & Substrate', 's0-field-thomson-scattering', 'Flux recoil locked', ['field', 'light', 'charge', 'thomson'], '[INSTRUMENT]'),
    /*
     * Scenario: s0-field-thomson-unlocked-recoil (Flux recoil unlocked)
     * Physical purpose: Simulates Thomson scattering with an unlocked recoiling charge.
     * Parameters: None.
     * Expected behavior: Plane wave scattering off a mobile central electron which recoils under emergent forces.
     * Discrepancy: None.
     */
    makeScenario('1. Foundational Dynamics & Substrate', 's0-field-thomson-unlocked-recoil', 'Flux recoil unlocked', ['field', 'light', 'charge', 'thomson'], '[MEASUREMENT]'),
    /*
     * Scenario: s0-field-spacetime-forcing-boundary (Spacetime forcing boundary (FTD-0253))
     * Physical purpose: Models spacetime forcing boundary conditions (FTD-0253).
     * Parameters: None.
     * Expected behavior: Forcing of localized flux at the center.
     * Discrepancy: None.
     */
    makeScenario('1. Foundational Dynamics & Substrate', 's0-field-spacetime-forcing-boundary', 'Spacetime forcing boundary (FTD-0253)', ['field', 'spacetime', 'locality', 'demo'], '[DEMO]+[BOUNDARY]'),
    /*
     * Scenario: s0-field-electric-dipole (Electric dipole)
     * Physical purpose: Establishes a static electric dipole field.
     * Parameters: None.
     * Expected behavior: Dipole field lines connecting a positive and negative charge.
     * Discrepancy: None.
     */
    makeScenario('1. Foundational Dynamics & Substrate', 's0-field-electric-dipole', 'Electric dipole', ['field']),
    /*
     * Scenario: s0-field-magnetic-dipole (Magnetic dipole)
     * Physical purpose: Establishes a static magnetic dipole field.
     * Parameters: None.
     * Expected behavior: Circulating loop flux representing a magnetic dipole.
     * Discrepancy: None.
     */
    makeScenario('1. Foundational Dynamics & Substrate', 's0-field-magnetic-dipole', 'Magnetic dipole', ['field']),
    /*
     * Scenario: s0-field-vortex-line (Vortex line)
     * Physical purpose: Models an electromagnetic or fluid vortex line.
     * Parameters: None.
     * Expected behavior: Tangential rotational flux line about the z-axis.
     * Discrepancy: None.
     */
    makeScenario('1. Foundational Dynamics & Substrate', 's0-field-vortex-line', 'Vortex line', ['field']),
    /*
     * Scenario: s0-seed-octahedron (Octahedron (6 face-neighbors))
     * Physical purpose: Seeds an octahedral arrangement of 6 face-neighboring charges.
     * Parameters: None.
     * Expected behavior: Central -1 charge surrounded by 6 positive charges.
     * Discrepancy: None.
     */
    makeScenario('1. Foundational Dynamics & Substrate', 's0-seed-octahedron', 'Octahedron (6 face-neighbors)', ['seed'], '[CONJECTURE]'),
    /*
     * Scenario: s0-seed-cuboctahedron (Cuboctahedron (12 edge-neighbors))
     * Physical purpose: Seeds a cuboctahedral arrangement of 12 edge-neighboring charges.
     * Parameters: None.
     * Expected behavior: Central -1 charge surrounded by 12 positive charges.
     * Discrepancy: None.
     */
    makeScenario('1. Foundational Dynamics & Substrate', 's0-seed-cuboctahedron', 'Cuboctahedron (12 edge-neighbors)', ['seed'], '[CONJECTURE]'),
    /*
     * Scenario: s0-seed-stella-octangula (Stella octangula (8 corners))
     * Physical purpose: Seeds a stella octangula arrangement of 8 corner charges.
     * Parameters: None.
     * Expected behavior: Central -1 charge surrounded by 8 positive charges.
     * Discrepancy: None.
     */
    makeScenario('1. Foundational Dynamics & Substrate', 's0-seed-stella-octangula', 'Stella octangula (8 corners)', ['seed'], '[CONJECTURE]'),
    /*
     * Scenario: s0-seed-moore-cell (Moore cell (full 26))
     * Physical purpose: Seeds a full 26-neighbor Moore cell.
     * Parameters: None.
     * Expected behavior: Central -1 charge surrounded by 26 positive charges.
     * Discrepancy: None.
     */
    makeScenario('1. Foundational Dynamics & Substrate', 's0-seed-moore-cell', 'Moore cell (full 26)', ['seed'], '[CONJECTURE]'),
    /*
     * Scenario: s0-seed-moore-decomposition (Moore decomposition (3 shells))
     * Physical purpose: Seeds a Moore cell decomposed into shell layers.
     * Parameters: None.
     * Expected behavior: Central -1 charge surrounded by octahedron (+1), cuboctahedron (-1), and stella octangula (+1).
     * Discrepancy: None.
     */
    makeScenario('1. Foundational Dynamics & Substrate', 's0-seed-moore-decomposition', 'Moore decomposition (3 shells)', ['seed'], '[CONJECTURE]'),

    // FTD-0102 / FTD-0107 emergent-spectrum reproduction.
    /*
     * Scenario: s0-seed-emergent-ic1 (Emergent ic1 (FTD-0107: 25-voxel L¹-ball-radius-2 cluster))
     * Physical purpose: Emergent octahedral bound state point injection (FTD-0107).
     * Parameters: None.
     * Expected behavior: Localized central high-energy flux nucleation into a stable 25-voxel octahedron.
     * Discrepancy: None.
     */
    makeScenario('2. Genesis & Emergence', 's0-seed-emergent-ic1', 'Emergent ic1 (FTD-0107: 25-voxel L¹-ball-radius-2 cluster)', ['seed', 'emergent', 'cluster'], '[STRUCTURAL HYPOTHESIS]'),
    /*
     * Scenario: s0-seed-emergent-ic3-collision (Emergent ic3 (FTD-0107: 2-cluster collision, 2-3 voxels each))
     * Physical purpose: Two-beam collision producing stable emergent clusters (FTD-0107).
     * Parameters: None.
     * Expected behavior: Collision of two opposing flux beams producing stable clusters.
     * Discrepancy: None.
     */
    makeScenario('2. Genesis & Emergence', 's0-seed-emergent-ic3-collision', 'Emergent ic3 (FTD-0107: 2-cluster collision, 2-3 voxels each)', ['seed', 'emergent', 'cluster', 'collision'], '[STRUCTURAL HYPOTHESIS]'),
    /*
     * Scenario: s0-seed-emergent-ic4-subthreshold (Emergent ic4 (FTD-0107: sub-threshold, 0 voxels — negative control))
     * Physical purpose: Sub-threshold negative control point injection (FTD-0107).
     * Parameters: None.
     * Expected behavior: Dispersive decay of low-amplitude flux with zero manifested voxels.
     * Discrepancy: None.
     */
    makeScenario('2. Genesis & Emergence', 's0-seed-emergent-ic4-subthreshold', 'Emergent ic4 (FTD-0107: sub-threshold, 0 voxels — negative control)', ['seed', 'emergent', 'control'], '[STRUCTURAL HYPOTHESIS]'),
    /*
     * Scenario: s0-seed-emergent-ic2-thermal-runaway (Emergent ic2 (FTD-0107: thermal-driven runaway — unstable phase))
     * Physical purpose: Thermal-driven runaway genesis in unstable phase (FTD-0107).
     * Parameters: None.
     * Expected behavior: High thermal Langevin noise triggers runaway genesis without initial flux injection.
     * Discrepancy: None.
     */
    makeScenario('2. Genesis & Emergence', 's0-seed-emergent-ic2-thermal-runaway', 'Emergent ic2 (FTD-0107: thermal-driven runaway — unstable phase)', ['seed', 'emergent', 'runaway', 'thermal'], '[STRUCTURAL HYPOTHESIS]'),
    /*
     * Scenario: s0-seed-emergent-ic1-diagonal (Emergent ic1 — body-diagonal injection (D3g: Z₄ vs Z₃ test))
     * Physical purpose: Body-diagonal flux point injection (D3g symmetry test).
     * Parameters: None.
     * Expected behavior: Nucleation along body diagonal, testing cluster-size efficiency.
     * Discrepancy: None.
     */
    makeScenario('2. Genesis & Emergence', 's0-seed-emergent-ic1-diagonal', 'Emergent ic1 — body-diagonal injection (D3g: Z₄ vs Z₃ test)', ['seed', 'emergent', 'cluster', 'D3g', 'diagonal'], '[STRUCTURAL HYPOTHESIS]'),
    /*
     * Scenario: s0-seed-emergent-ic1-isotropic (Emergent ic1 — isotropic 6-axis injection (D3h: full O_h symmetry test))
     * Physical purpose: Isotropic 6-axis flux point injection (D3h O_h symmetry test).
     * Parameters: None.
     * Expected behavior: Symmetric outward expansion and nucleation.
     * Discrepancy: None.
     */
    makeScenario('2. Genesis & Emergence', 's0-seed-emergent-ic1-isotropic', 'Emergent ic1 — isotropic 6-axis injection (D3h: full O_h symmetry test)', ['seed', 'emergent', 'cluster', 'D3h', 'isotropic'], '[STRUCTURAL HYPOTHESIS]'),
    /*
     * Scenario: s0-seed-emergent-ic1-viz (Emergent ic1 — clean view (T=0, no thermal background))
     * Physical purpose: Clean visualization of axial ic1 cluster under zero temperature.
     * Parameters: None.
     * Expected behavior: Static, noise-free development of the octahedral bound state.
     * Discrepancy: None.
     */
    makeScenario('2. Genesis & Emergence', 's0-seed-emergent-ic1-viz', 'Emergent ic1 — clean view (T=0, no thermal background)', ['seed', 'emergent', 'cluster', 'viz', 'clean'], '[VISUALISATION]'),
    /*
     * Scenario: s0-seed-emergent-ic1-diagonal-viz (Emergent ic1 body-diagonal — clean view (T=0))
     * Physical purpose: Clean visualization of body-diagonal ic1 cluster under zero temperature.
     * Parameters: None.
     * Expected behavior: Static, noise-free development along the body diagonal.
     * Discrepancy: None.
     */
    makeScenario('2. Genesis & Emergence', 's0-seed-emergent-ic1-diagonal-viz', 'Emergent ic1 body-diagonal — clean view (T=0)', ['seed', 'emergent', 'cluster', 'viz', 'clean', 'diagonal'], '[VISUALISATION]'),
    /*
     * Scenario: s0-seed-emergent-ic1-isotropic-viz (Emergent ic1 isotropic — clean view (T=0))
     * Physical purpose: Clean visualization of isotropic ic1 cluster under zero temperature.
     * Parameters: None.
     * Expected behavior: Static, noise-free symmetric cluster growth.
     * Discrepancy: None.
     */
    makeScenario('2. Genesis & Emergence', 's0-seed-emergent-ic1-isotropic-viz', 'Emergent ic1 isotropic — clean view (T=0)', ['seed', 'emergent', 'cluster', 'viz', 'clean', 'isotropic'], '[VISUALISATION]'),
    /*
     * Scenario: s0-seed-cluster-law (Genesis-Burst N(A) Law — interactive (FTD-0269))
     * Physical purpose: Sweeps cluster size N(A) vs injection amplitude A (FTD-0269).
     * Parameters: None.
     * Expected behavior: Interactive genesis-burst swept over broken power-law regimes.
     * Discrepancy: None.
     */
    makeScenario('2. Genesis & Emergence', 's0-seed-cluster-law', 'Genesis-Burst N(A) Law — interactive (FTD-0269)', ['seed', 'genesis', 'cluster', 'na-law', 'interactive'], '[MEASURED — BOUNDARY, FTD-0269]'),
    /*
     * Scenario: s0-seed-cluster-law-subknee (N(A) law — sub-knee (A=12, geometry-limited))
     * Physical purpose: Clean visualization of cluster-law sub-knee regime (A=12).
     * Parameters: None.
     * Expected behavior: Compact 27-block cascade of ~8 voxels under zero temperature.
     * Discrepancy: None.
     */
    makeScenario('2. Genesis & Emergence', 's0-seed-cluster-law-subknee', 'N(A) law — sub-knee (A=12, geometry-limited)', ['seed', 'genesis', 'cluster', 'na-law', 'subknee', 'viz'], '[VISUALISATION]'),
    /*
     * Scenario: s0-seed-cluster-law-knee (N(A) law — the knee (A=16, 27-block escape))
     * Physical purpose: Clean visualization of cluster-law knee escape (A=16).
     * Parameters: None.
     * Expected behavior: escape from 27-block to ~21 voxels under zero temperature.
     * Discrepancy: None.
     */
    makeScenario('2. Genesis & Emergence', 's0-seed-cluster-law-knee', 'N(A) law — the knee (A=16, 27-block escape)', ['seed', 'genesis', 'cluster', 'na-law', 'knee', 'viz'], '[VISUALISATION]'),
    /*
     * Scenario: s0-seed-cluster-law-superknee (N(A) law — super-knee (A=40, energy budget N=k·A²))
     * Physical purpose: Clean visualization of cluster-law super-knee regime (A=40).
     * Parameters: None.
     * Expected behavior: Large bulk-volume expansion of ~92 voxels under zero temperature.
     * Discrepancy: None.
     */
    makeScenario('2. Genesis & Emergence', 's0-seed-cluster-law-superknee', 'N(A) law — super-knee (A=40, energy budget N=k·A²)', ['seed', 'genesis', 'cluster', 'na-law', 'superknee', 'viz'], '[VISUALISATION]'),
    // s0-seed-symmetry-regression removed 2026-04-28 (audit removal): engine CI
    // regression artefact (voxel_uniform() determinism check), not user-facing
    // physics. Fold into engine/tests/ as a ctest if still needed.

    // ── Vacuum Particles (s0-vacuum-* group, 2026-04-28) ───────────────
    // 15 single-particle-in-vacuum scenarios. See
    // engine/web/docs/SPEC_VACUUM_PARTICLE_SCENARIOS.md for the catalog.
    /*
     * Scenario: s0-vacuum-electron (Electron in vacuum (e⁻))
     * Physical purpose: Seeds a physical electron in vacuum (e-).
     * Parameters: None.
     * Expected behavior: Central -1 charge surrounded by inward-pointing Coulomb dressing flux.
     * Discrepancy: None.
     */
    makeScenario('3. Particles & The Standard Model', 's0-vacuum-electron',          'Electron in vacuum (e⁻)',                 ['vacuum', 'lepton'],   '[CONJECTURE]'),
    /*
     * Scenario: s0-vacuum-muon (Muon in vacuum (μ⁻))
     * Physical purpose: Seeds a physical muon in vacuum (mu-).
     * Parameters: None.
     * Expected behavior: Central -1 charge with boosted Coulomb dressing flux.
     * Discrepancy: None.
     */
    makeScenario('3. Particles & The Standard Model', 's0-vacuum-muon',              'Muon in vacuum (μ⁻)',                     ['vacuum', 'lepton'],   '[CONJECTURE]'),
    /*
     * Scenario: s0-vacuum-tau (Tau in vacuum (τ⁻))
     * Physical purpose: Seeds a physical tau lepton in vacuum (tau-).
     * Parameters: None.
     * Expected behavior: Central -1 charge with heavily boosted Coulomb dressing flux.
     * Discrepancy: None.
     */
    makeScenario('3. Particles & The Standard Model', 's0-vacuum-tau',               'Tau in vacuum (τ⁻)',                      ['vacuum', 'lepton'],   '[CONJECTURE]'),
    /*
     * Scenario: s0-vacuum-electron-neutrino (Electron neutrino in vacuum (ν_e))
     * Physical purpose: Seeds an electron neutrino in vacuum (nu_e).
     * Parameters: None.
     * Expected behavior: Small-amplitude localized propagating neutral wave packet.
     * Discrepancy: None.
     */
    makeScenario('3. Particles & The Standard Model', 's0-vacuum-electron-neutrino', 'Electron neutrino in vacuum (ν_e)',       ['vacuum', 'lepton', 'neutrino'], '[CONJECTURE]'),
    /*
     * Scenario: s0-vacuum-muon-neutrino (Muon neutrino in vacuum (ν_μ))
     * Physical purpose: Seeds a muon neutrino in vacuum (nu_mu).
     * Parameters: None.
     * Expected behavior: Intermediate-amplitude localized propagating neutral wave packet.
     * Discrepancy: None.
     */
    makeScenario('3. Particles & The Standard Model', 's0-vacuum-muon-neutrino',     'Muon neutrino in vacuum (ν_μ)',           ['vacuum', 'lepton', 'neutrino'], '[CONJECTURE]'),
    /*
     * Scenario: s0-vacuum-tau-neutrino (Tau neutrino in vacuum (ν_τ))
     * Physical purpose: Seeds a tau neutrino in vacuum (nu_tau).
     * Parameters: None.
     * Expected behavior: Large-amplitude localized propagating neutral wave packet.
     * Discrepancy: None.
     */
    makeScenario('3. Particles & The Standard Model', 's0-vacuum-tau-neutrino',      'Tau neutrino in vacuum (ν_τ)',            ['vacuum', 'lepton', 'neutrino'], '[CONJECTURE]'),
    /*
     * Scenario: s0-vacuum-photon (Photon in vacuum (γ))
     * Physical purpose: Seeds a physical photon in vacuum.
     * Parameters: None.
     * Expected behavior: Propagating electromagnetic wave packet with genesis disabled to avoid pair production.
     * Discrepancy: None.
     */
    makeScenario('3. Particles & The Standard Model', 's0-vacuum-photon',            'Photon in vacuum (γ)',                    ['vacuum', 'gauge'],    '[CONJECTURE]'),
    /*
     * Scenario: s0-vacuum-w-boson (W boson in vacuum (W±))
     * Physical purpose: Seeds a charged W boson in vacuum (W+/-).
     * Parameters: None.
     * Expected behavior: Localized +1 charge with a short-range heavy dressing field.
     * Discrepancy: None.
     */
    makeScenario('3. Particles & The Standard Model', 's0-vacuum-w-boson',           'W boson in vacuum (W±)',                  ['vacuum', 'gauge'],    '[CONJECTURE]'),
    /*
     * Scenario: s0-vacuum-z-boson (Z boson in vacuum (Z⁰))
     * Physical purpose: Seeds a neutral Z boson in vacuum (Z0).
     * Parameters: None.
     * Expected behavior: Localized neutral heavy dressing field without central charge.
     * Discrepancy: None.
     */
    makeScenario('3. Particles & The Standard Model', 's0-vacuum-z-boson',           'Z boson in vacuum (Z⁰)',                  ['vacuum', 'gauge'],    '[CONJECTURE]'),
    /*
     * Scenario: s0-vacuum-higgs (Higgs boson in vacuum (H))
     * Physical purpose: Seeds a physical Higgs boson in vacuum (H).
     * Parameters: None.
     * Expected behavior: Localized isotropic scalar dressing flux.
     * Discrepancy: None.
     */
    makeScenario('3. Particles & The Standard Model', 's0-vacuum-higgs',             'Higgs boson in vacuum (H)',               ['vacuum', 'gauge'],    '[CONJECTURE]'),
    /*
     * Scenario: s0-vacuum-proton (Proton in vacuum (p))
     * Physical purpose: Seeds a physical proton in vacuum (p).
     * Parameters: None.
     * Expected behavior: A three-quark triad (+1, +1, -1) forming a stable bound baryon.
     * Discrepancy: None.
     */
    makeScenario('3. Particles & The Standard Model', 's0-vacuum-proton',            'Proton in vacuum (p)',                    ['vacuum', 'baryon'],   '[CONJECTURE]'),
    /*
     * Scenario: s0-vacuum-neutron (Neutron in vacuum (n))
     * Physical purpose: Seeds a physical neutron in vacuum (n).
     * Parameters: None.
     * Expected behavior: A three-quark triad (+1, -1, -1) forming a stable bound baryon.
     * Discrepancy: None.
     */
    makeScenario('3. Particles & The Standard Model', 's0-vacuum-neutron',           'Neutron in vacuum (n)',                   ['vacuum', 'baryon'],   '[CONJECTURE]'),
    /*
     * Scenario: s0-vacuum-pion-charged (Charged pion in vacuum (π±))
     * Physical purpose: Seeds a physical charged pion in vacuum (pi+/-).
     * Parameters: None.
     * Expected behavior: A bound quark-antiquark meson pair with charges (+1, -1).
     * Discrepancy: None.
     */
    makeScenario('3. Particles & The Standard Model', 's0-vacuum-pion-charged',      'Charged pion in vacuum (π±)',             ['vacuum', 'meson'],    '[CONJECTURE]'),
    /*
     * Scenario: s0-vacuum-pion-neutral (Neutral pion in vacuum (π⁰))
     * Physical purpose: Seeds a physical neutral pion in vacuum (pi0).
     * Parameters: None.
     * Expected behavior: A bound quark-antiquark meson pair with neutral charges.
     * Discrepancy: None.
     */
    makeScenario('3. Particles & The Standard Model', 's0-vacuum-pion-neutral',      'Neutral pion in vacuum (π⁰)',             ['vacuum', 'meson'],    '[CONJECTURE]'),
    /*
     * Scenario: s0-vacuum-kaon-charged (Charged kaon in vacuum (K±))
     * Physical purpose: Seeds a physical charged kaon in vacuum (K+/-).
     * Parameters: None.
     * Expected behavior: A bound quark-antiquark meson pair with boosted mass energy.
     * Discrepancy: None.
     */
    makeScenario('3. Particles & The Standard Model', 's0-vacuum-kaon-charged',      'Charged kaon in vacuum (K±)',             ['vacuum', 'meson'],    '[CONJECTURE]'),
    /*
     * Scenario: s0-seed-de-broglie-clock (De Broglie Clock (pilot wave) — interactive (FTD-0271))
     * Physical purpose: Simulates the De Broglie internal compton clock (FTD-0271).
     * Parameters: None.
     * Expected behavior: A central manifested block oscillates at the Compton frequency.
     * Discrepancy: None.
     */
    makeScenario('5. Quantum Lab & Foundations', 's0-seed-de-broglie-clock', 'De Broglie Clock (pilot wave) — interactive (FTD-0271)', ['seed', 'quantum', 'de-broglie', 'pilot-wave', 'interactive'], '[CONDITIONAL — DERIVED-GIVEN-IMPOSED-INPUT, FTD-0271]'),
    /*
     * Scenario: s0-seed-thermal-ignition (Thermal Ignition — lattice condensation (FTD-0274))
     * Physical purpose: Simulates first-order lattice thermal condensation (FTD-0274).
     * Parameters: None.
     * Expected behavior: Abrupt transition at T_up where the entire lattice manifests as a condensate.
     * Discrepancy: None.
     */
    makeScenario('5. Quantum Lab & Foundations', 's0-seed-thermal-ignition', 'Thermal Ignition — lattice condensation (FTD-0274)', ['seed', 'thermal', 'temperature', 'condensation', 'first-order', 'interactive'], '[MEASURED — BOUNDARY, FTD-0274]'),
];

export const SCALE0_SCENARIO_MAP = new Map(SCALE0_SCENARIOS.map((scenario) => [scenario.id, scenario]));

export function getScale0Scenario(id) {
    const scenario = SCALE0_SCENARIO_MAP.get(id);
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
