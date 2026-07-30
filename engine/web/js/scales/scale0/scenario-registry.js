
/**
 * Hard admission gate for the normal Scale-0 menu.
 *
 * A scenario appears in SCALE0_SCENARIOS only when an automated test asserts
 * the behavior named here. A mount/telemetry smoke test is not sufficient.
 * Implementations without an entry remain in SCALE0_SCENARIO_CATALOG for
 * provenance and direct research use, but are not presented to users.
 */
export const SCALE0_SCENARIO_VALIDATION = Object.freeze({
    'empty': Object.freeze({
        level: 'behavioral',
        qualification: 'NULL CONTROL — no physical identification',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'The empty lattice remains field-free and unmanifested across native ticks.',
    }),
    'flux-pulse': Object.freeze({
        level: 'behavioral',
        qualification: 'FINITE-BOX NATIVE BOUNDARY PROBE — computational boundary behavior, not ontology or electromagnetism',
        test: 'engine/tests/test_boundary_scenario_physics.cpp',
        target: 'boundary_scenario_physics',
        assertion: 'The transverse packet stays inside the exact stencil cone, conserves the periodic discrete Hamiltonian, and reverses flux momentum under the Neumann ghost shell. The lossy shell retains 52.9% of field norm at tick 90 and is explicitly not certified as an absorbing/radiating boundary.',
    }),
    'flux-genesis-between-gates': Object.freeze({
        level: 'behavioral',
        qualification: 'ONE-TICK SELECTED GENESIS-LAW TEST — validates the implemented threshold/hazard, not particle identity or a sustained cascade',
        test: 'engine/tests/test_genesis_scenario_physics.cpp',
        target: 'genesis_scenario_physics',
        assertion: 'At L=24 and seed 1, exact initial cohorts give 0/49/120 site-resolved genesis events for hazards 0/0.0168973/0.034247; later ticks are not frozen independent cohorts because genesis drains flux and the master rule also runs evaporation.',
    }),
    'flux-pair-production': Object.freeze({
        level: 'behavioral',
        qualification: 'ONE-TICK SELECTED POLARITY-PAIR RULE — validates the implemented stochastic transition, not Schwinger production or particle identity',
        test: 'engine/tests/test_reaction_scenario_physics.cpp',
        target: 'reaction_scenario_physics',
        assertion: 'At L=24 and seed 1, 343 isolated sources with exact hazard p=1/2 produce 170 events within the preregistered six-sigma gate; every event creates adjacent -/+ states with a shared pair ID, distinct particle IDs, and zero pairwise signed-state and vector-flux sums.',
    }),
    'flux-annihilation': Object.freeze({
        level: 'behavioral',
        qualification: 'NATIVE OPPOSITE-STATE COLLISION RULE — state removal and pre-existing-flux redistribution, not physical annihilation radiation',
        test: 'engine/tests/test_reaction_scenario_physics.cpp',
        target: 'reaction_scenario_physics',
        assertion: 'One + state crosses into an adjacent - state on tick two; both states vanish, total vector flux remains zero, the six-face redistribution gives field-norm ratio 1/6, and no wave momentum or rest-mass radiation is created.',
    }),
    'light-rainbow': Object.freeze({
        level: 'behavioral',
        qualification: 'NATIVE TRANSVERSALITY TEST — not physical color or spectroscopy',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'All three initialized harmonics are exactly transverse, divergence-free, energetic, and remain unmanifested.',
    }),
    'light-photon-race': Object.freeze({
        level: 'behavioral',
        qualification: 'NATIVE LINEARITY TEST — not a photon-identification test',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'Two packets separated by a tenfold amplitude change translate equally under the native linear-wave map.',
    }),
    's0-field-plane-wave': Object.freeze({
        level: 'behavioral',
        qualification: 'EXACT NATIVE TRAVELING EIGENMODE — not an electromagnetic identification',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'The n=4 transverse Fourier mode preserves amplitude and follows the exact kick-drift lattice phase to numerical precision.',
    }),
    's0-field-standing-wave': Object.freeze({
        level: 'behavioral',
        qualification: 'EXACT NATIVE STANDING EIGENMODE — no material cavity or photon identity implied',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'The n=4 transverse Fourier mode follows the exact standing-mode oscillation while retaining fixed nodes and zero traveling-mode leakage.',
    }),
    'quantum-entangle': Object.freeze({
        level: 'behavioral',
        qualification: 'INITIAL-DATA BOOKKEEPING TEST — not quantum entanglement',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'The initialized pair contains exactly two opposite states with one shared pair ID and cancelling flux.',
    }),
    's0-vacuum-electron-neutrino': Object.freeze({
        level: 'behavioral',
        qualification: 'NEUTRAL WAVE-PACKET CANDIDATE — not a neutrino identification',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'The neutral packet is divergence-free, translates dynamically, and remains unmanifested.',
    }),
    's0-vacuum-muon-neutrino': Object.freeze({
        level: 'behavioral',
        qualification: 'IMPOSED 1.3x NEUTRAL WAVE-PACKET COPY — CLOSED NEGATIVE for a distinct muon-neutrino mode',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'The packet is exactly 1.3 times the base neutral packet initially and after 12 ticks, with identical centroid motion and no manifestation; amplitude coding supplies no flavor physics.',
    }),
    's0-vacuum-tau-neutrino': Object.freeze({
        level: 'behavioral',
        qualification: 'IMPOSED 1.6x NEUTRAL WAVE-PACKET COPY — CLOSED NEGATIVE for a distinct tau-neutrino mode',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'The packet is exactly 1.6 times the base neutral packet initially and after 12 ticks, with identical centroid motion and no manifestation; amplitude coding supplies no flavor physics.',
    }),
    's0-vacuum-electron-antineutrino': Object.freeze({
        level: 'behavioral',
        qualification: 'NEUTRAL WAVE-PACKET CANDIDATE, OPPOSITE-DIRECTION MIRROR — not an antineutrino identification',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'The neutral packet is divergence-free, translates dynamically in the direction opposite the electron-neutrino packet, and remains unmanifested.',
    }),
    's0-vacuum-muon-antineutrino': Object.freeze({
        level: 'behavioral',
        qualification: 'IMPOSED 1.3x NEUTRAL WAVE-PACKET COPY, OPPOSITE-DIRECTION MIRROR — CLOSED NEGATIVE for a distinct muon-antineutrino mode',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'The packet is exactly 1.3 times the base antineutrino packet initially and after 12 ticks, with identical centroid motion and no manifestation; amplitude coding supplies no flavor physics.',
    }),
    's0-vacuum-tau-antineutrino': Object.freeze({
        level: 'behavioral',
        qualification: 'IMPOSED 1.6x NEUTRAL WAVE-PACKET COPY, OPPOSITE-DIRECTION MIRROR — CLOSED NEGATIVE for a distinct tau-antineutrino mode',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'The packet is exactly 1.6 times the base antineutrino packet initially and after 12 ticks, with identical centroid motion and no manifestation; amplitude coding supplies no flavor physics.',
    }),
    's0-vacuum-photon': Object.freeze({
        level: 'behavioral',
        qualification: 'PHOTON CANDIDATE — propagation tested; physical identity open',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'The transverse packet propagates near C_SPEED, remains coherent and unmanifested, and uses an isolated wave sector.',
    }),
    's0-seed-dynamical-flux-dressing': Object.freeze({
        level: 'behavioral',
        qualification: 'NATIVE POLARITY-SOURCED FLUX RESPONSE — locked radial-dressing, attachment, wake, and release labels failed',
        test: 'engine/tests/test_dynamic_flux_dressing_scenario.cpp',
        target: 'dynamic_flux_dressing_scenario',
        assertion: 'One locked central polarity starts with exactly zero field; the native -G_C grad(s) coupling creates the exact outward six-face first-tick response and a causal polarity-odd field. FTD-0476 does not qualify the resulting morphology as a radial dressing, attached aura, wake, or released radiation field.',
    }),
    's0-seed-moving-source-reciprocity': Object.freeze({
        level: 'behavioral',
        qualification: 'QUALIFIED NEGATIVE — selected force gives a 0.203598-cell sub-voxel response but no manifested hop; dressing, wake, detached-field, and reciprocity gates fail',
        test: 'engine/tests/test_reciprocal_moving_source_scenario.cpp',
        target: 'reciprocal_moving_source_scenario',
        assertion: 'A separate finite transverse packet causes equal-magnitude polarity-related sub-voxel responses under the selected G_C s grad|J| extension, while matched source-only and locked controls remain at rest. At L=65 through 72 ticks neither polarity performs an integer movement event, so this is not a moving-source, wake, or radiation demonstration.',
    }),
    's0-seed-octahedron': Object.freeze({
        level: 'behavioral',
        qualification: 'EXACT INERT INITIAL-DATA CONSTRUCTION — a Moore face-shell orbit, not a particle or gauge-group derivation',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'One central - state and the six radius-squared-1 face sites are seeded exactly, with no field dressing and every production term off.',
    }),
    's0-seed-cuboctahedron': Object.freeze({
        level: 'behavioral',
        qualification: 'EXACT INERT INITIAL-DATA CONSTRUCTION — a Moore edge-shell orbit, not a particle or gauge-group derivation',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'One central - state and the twelve radius-squared-2 edge sites are seeded exactly, with no field dressing and every production term off.',
    }),
    's0-seed-stella-octangula': Object.freeze({
        level: 'behavioral',
        qualification: 'EXACT INERT INITIAL-DATA CONSTRUCTION — a Moore corner-shell orbit, not a particle or generation derivation',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'One central - state and the eight radius-squared-3 corner sites are seeded exactly, with no field dressing and every production term off.',
    }),
    's0-seed-moore-cell': Object.freeze({
        level: 'behavioral',
        qualification: 'EXACT INERT 27-SITE CONSTRUCTION — Moore-neighborhood bookkeeping only',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'The center plus 6 face, 12 edge, and 8 corner sites form the exact 27-site Moore cell and remain unchanged with all production terms off.',
    }),
    's0-seed-moore-decomposition': Object.freeze({
        level: 'behavioral',
        qualification: 'EXACT INERT SHELL DECOMPOSITION — alternating ternary labels are imposed visualization labels',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'The exact 1+6+12+8 shell decomposition is seeded with declared alternating ternary labels, no field dressing, and all production terms off.',
    }),
    's0-seed-cluster-law-subknee': Object.freeze({
        level: 'behavioral',
        qualification: 'SELECTED GENESIS AMPLITUDE RESPONSE AT A=12 — finite-box native behavior, not a universal N(A) law',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'Under the isolated wave+Gauss+genesis map at T=0, the A=12 seed produces the smallest nonzero member of the preregistered A=12/16/40 response ordering and is stable from ticks 200 to 220.',
    }),
    's0-seed-cluster-law-knee': Object.freeze({
        level: 'behavioral',
        qualification: 'SELECTED GENESIS AMPLITUDE RESPONSE AT A=16 — finite-box native behavior; no geometric-knee claim',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'Under the isolated wave+Gauss+genesis map at T=0, the A=16 response lies strictly between the A=12 and A=40 counts and is stable from ticks 200 to 220.',
    }),
    's0-seed-cluster-law-superknee': Object.freeze({
        level: 'behavioral',
        qualification: 'SELECTED GENESIS AMPLITUDE RESPONSE AT A=40 — finite-box native behavior; no N proportional to A-squared claim',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'Under the isolated wave+Gauss+genesis map at T=0, the A=40 seed produces the largest member of the preregistered A=12/16/40 response ordering and is stable from ticks 200 to 220.',
    }),
    's0-seed-massive-body': Object.freeze({
        level: 'behavioral',
        qualification: 'SELECTED LOCKED-MASS LATENCY-POISSON PROBE — imposed gravity charge and static source, not GR or dynamical gravitation',
        test: 'engine/tests/test_scenario_behavior.cpp',
        supplementalTest: 'engine/web/tests/scale0-massbody.spec.js',
        target: 'scenario_behavior',
        assertion: 'Exactly 33 locked manifested sites source an isolated positive latency solution that decreases from center to boundary, remains sub-horizon, and does not grow or move.',
    }),
    'flux-zero-point': Object.freeze({
        level: 'behavioral',
        qualification: 'FINITE PERIODIC RANDOM-WAVE BATH — exact discrete invariant; not quantum vacuum energy or a ground-state derivation',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'A deterministic subthreshold random J/W bath evolves under only the periodic bare wave map, conserves its exact kick-drift modified Hamiltonian, and remains unmanifested for 200 ticks.',
    }),
    'flux-soliton': Object.freeze({
        level: 'behavioral',
        qualification: 'HIGH-AMPLITUDE NATIVE DISPERSION TEST — explicitly not a soliton or particle',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'The isolated divergence-free high-amplitude packet remains unmanifested, moves in +x, and broadens by more than a factor of two over 20 ticks, falsifying soliton-like shape preservation.',
    }),
    'light-dipole': Object.freeze({
        level: 'behavioral',
        qualification: 'BIDIRECTIONAL TRANSVERSE-LOBE PROXY — native wave separation, not electromagnetic dipole radiation',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'Two isolated transverse packets separate symmetrically in opposite x directions, more than doubling the x-width with half-space energy imbalance below 1e-10 and no manifestation.',
    }),
    's0-field-uniform-e': Object.freeze({
        level: 'behavioral',
        qualification: 'EXACT INERT CANONICAL-MOMENTUM FIELD — uniform engine E-proxy initial data, not a sourced electromagnetic solution',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'Every site has exactly wave_vel=(-0.1,0,0), zero J and zero state under an all-terms-off profile, and the configuration remains unchanged.',
    }),
    's0-field-uniform-b': Object.freeze({
        level: 'behavioral',
        qualification: 'EXACT INERT VECTOR-POTENTIAL ANSATZ — uniform interior curl only; finite-face behavior is not certified',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'The imposed vector potential has interior curl B=(0,0,0.05) to machine precision, zero transverse curl, and remains unchanged with all production terms off.',
    }),
    's0-seed-sloop': Object.freeze({
        level: 'behavioral',
        qualification: 'EXACT INERT TANGENTIAL-RING ANSATZ — imposed geometry; no self-reference or consciousness claim',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'Twelve positive sites lie in one plane with equal K_B flux magnitude, zero net vector flux and no wave momentum, while all production terms remain off.',
    }),
    's0-seed-observer-cell': Object.freeze({
        level: 'behavioral',
        qualification: 'EXACT INERT ALTERNATING MOORE-SHELL CELL — imposed ternary labels; no observer claim',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'The exact 1+6+12+8 Moore shells carry imposed +,-,+,- labels with zero field dressing and all production terms off.',
    }),
    's0-seed-wilson-loop': Object.freeze({
        level: 'behavioral',
        qualification: 'EXACT INERT ORIENTED SQUARE PATH — no link holonomy, trace, Wilson observable, or confinement claim',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'The field occupies exactly the 8R sites of an oriented square path, has zero vector sum and no ternary corner charges, and remains inert with all production terms off.',
    }),
    's0-seed-flux-tube': Object.freeze({
        level: 'behavioral',
        qualification: 'EXACT INERT GAUSSIAN AXIAL TUBE — imposed profile with neutral ternary endpoints; no q-qbar or confinement claim',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'Every field value matches the declared thresholded transverse Gaussian along the selected axial interval, with exactly two opposite endpoint states and all production terms off.',
    }),
    's0-seed-monopole': Object.freeze({
        level: 'behavioral',
        qualification: 'EXACT INERT RADIAL INVERSE-SQUARE PROFILE — monopole-shaped imported ansatz, not evidence for magnetic charge',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'After correcting the prior azimuthal implementation, every noncentral vector is exactly radial with r-squared times magnitude equal to 1/(4 pi), with no manifested sites and all production terms off.',
    }),
    's0-seed-instanton': Object.freeze({
        level: 'behavioral',
        qualification: 'EXACT INERT RADIAL 3-VECTOR PROFILE — explicitly not an instanton; no Euclidean time, non-Abelian connection, or topological charge',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'The setup exactly realizes J=3 r-hat/(r-squared+9) in three spatial dimensions; the automated gate rejects any Yang-Mills-instanton interpretation.',
    }),
    's0-field-electric-dipole': Object.freeze({
        level: 'behavioral',
        qualification: 'EXACT INERT SOFTENED OPPOSITE-SOURCE FLUX PROFILE — selected Coulomb-shaped ansatz, not emergent electromagnetism',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'Every vector equals the declared superposition alpha/(4 pi) times r/(r-squared+1)^(3/2), with exactly two opposite ternary source markers and all production terms off.',
    }),
    's0-field-magnetic-dipole': Object.freeze({
        level: 'behavioral',
        qualification: 'EXACT INERT SOFTENED DIPOLE VECTOR-POTENTIAL ANSATZ — imported shape, not a native magnetic-dipole derivation',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'The corrected field equals A=(K_B/4pi)(z-hat cross r)/(r-squared+1)^(3/2) at every site; the former stack of rings through every z-plane was removed.',
    }),
    's0-field-vortex-line': Object.freeze({
        level: 'behavioral',
        qualification: 'EXACT INERT AZIMUTHAL INVERSE-RADIUS PROFILE — no electromagnetic, fluid, or quantized-vortex identity',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'Every non-axis vector is exactly tangential and r times its magnitude is constant on all z slices, with all production terms off.',
    }),
    'flux-dipole': Object.freeze({
        level: 'behavioral',
        qualification: 'ANTISYMMETRIC NATIVE WAVE PAIR — odd-parity Gaussian vector blobs, not an electromagnetic dipole',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'The isolated periodic wave map preserves the exact odd x-reflection parity of both J and W for 12 ticks without manifestation.',
    }),
    'flux-standing': Object.freeze({
        level: 'behavioral',
        qualification: 'REFLECTION-EVEN BROADBAND WAVE PAIR — zero-initial-momentum standing proxy, not a pure normal mode',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'The pair starts with exact even x-reflection parity and zero W; the isolated periodic wave map preserves that parity for 12 ticks without manifestation.',
    }),
    'flux-vortex': Object.freeze({
        level: 'behavioral',
        qualification: 'EXACT INERT HELICAL-RING VECTOR ANSATZ — positive imposed circulation and axial bias; no spin or quantization claim',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'All support lies on the declared three-plane ring, with positive oriented circulation, positive axial vector sum, zero wave momentum, and all production terms off.',
    }),
    's0-field-rf-lattice-wave': Object.freeze({
        level: 'behavioral',
        qualification: 'SELECTED n=1 TRANSVERSE LATTICE MODE — exact native discrete-time pole in lattice units; no radio-frequency calibration',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'The periodic plane-average n=1 harmonic advances one tick at omega=2 asin(C_SPEED sin(k/2)) with relative error below 1e-11 and remains unmanifested.',
    }),
    's0-field-light-lattice-wave': Object.freeze({
        level: 'behavioral',
        qualification: 'SELECTED n=6 TRANSVERSE LATTICE MODE — exact native discrete-time pole in lattice units; no light or color calibration',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'The periodic plane-average n=6 harmonic advances one tick at omega=2 asin(C_SPEED sin(k/2)) with relative error below 1e-11 and remains unmanifested.',
    }),
    's0-field-sound-lattice-wave': Object.freeze({
        level: 'behavioral',
        qualification: 'LONGITUDINAL n=4 NATIVE MODE — CLOSED NEGATIVE for the c/8 sound-speed interpretation; the frozen engine has no acoustic medium',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'The longitudinal plane average satisfies the native C_SPEED recurrence to numerical precision and rejects the seeded c/8 recurrence by normalized residual 0.0801.',
    }),
    's0-field-sound-collision': Object.freeze({
        level: 'behavioral',
        qualification: 'COUNTER-SEEDED LONGITUDINAL WAVE OVERLAP — exact linear superposition; CLOSED NEGATIVE for an acoustic collision interaction',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'At L=48 and tick 20 the independently reconstructed lanes have normalized spatial overlap 0.804443, while the combined evolution differs from their pointwise sum by only 6.02e-16 and remains unmanifested.',
    }),
    'quantum-double-slit': Object.freeze({
        level: 'behavioral',
        qualification: 'CLASSICAL TWO-SOURCE LINEAR-SUPERPOSITION CONTROL — CLOSED NEGATIVE for a destructive double-slit fringe at the fixed screen',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'At L=48 and tick 20 the field equals the sum of its two independently evolved sources to relative residual 7.74e-16; the fixed x=L/2 screen has constructive cross-term fraction 0.461954 but destructive fraction zero.',
    }),
    'quantum-born-rule': Object.freeze({
        level: 'behavioral',
        qualification: 'FIXED-ENVELOPE ONE-TICK GENESIS COHORT — selected local threshold/hazard response; no Born probability law or measurement claim',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'At L=32 with genesis as the only active term, the fixed-orientation Gaussian J/W envelope produces exactly 36 manifested single-site events in one tick, zero pair IDs, and bit-exact independent replay.',
    }),
    'quantum-zeno': Object.freeze({
        level: 'behavioral',
        qualification: 'SUPERCRITICAL ONE-TICK GENESIS COHORT — CLOSED NEGATIVE for a Zeno interpretation because no measurement or intervention exists',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'At L=32 with genesis as the only active term, the supercritical isotropic J/W envelope produces exactly 491 manifested single-site events in one tick, zero pair IDs, and bit-exact independent replay.',
    }),
    'quantum-well': Object.freeze({
        level: 'behavioral',
        qualification: 'IMPOSED BROADBAND HARMONICS WITH INERT MARKER PLANES — CLOSED NEGATIVE for confinement or particle-in-a-box quantization',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'At L=32, removing both 32x32 locked marker planes changes the eight-tick field evolution by exactly zero, while 32.5541% of field energy propagates outside the marked interval.',
    }),
    'quantum-aharonov-bohm': Object.freeze({
        level: 'behavioral',
        qualification: 'CENTRAL-TUBE PLUS TWO-PATH LINEAR-WAVE GEOMETRY — CLOSED NEGATIVE for an Aharonov-Bohm interaction or phase observable',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'At L=48 the tube/path decomposition is exact initially; after 12 projected-wave ticks, the full field differs from the separately evolved tube-plus-path sum by 6.13e-16, remains unmanifested, and has normalized divergence 9.06e-17.',
    }),
    'quantum-casimir': Object.freeze({
        level: 'behavioral',
        qualification: 'TRANSVERSE EIGENMODE PLUS INERT PARALLEL MARKER PLANES — CLOSED NEGATIVE for a Casimir boundary or force mechanism',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'At L=32, removing both locked 32x32 marker planes changes the 12-tick transverse eigenmode evolution by exactly zero; the plates remain static and no force, boundary restriction, or vacuum subtraction exists.',
    }),
    'quantum-tunnel': Object.freeze({
        level: 'behavioral',
        qualification: 'LOCKED THREE-PLANE STATE-COUPLING AMPLIFIER — CLOSED NEGATIVE for a tunneling-barrier or evanescent-transmission interpretation',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'At L=32 and tick 28, right-side field energy is 14091.6 with the locked wall versus 0.909727 without it, a ratio of 15489.9; the relative wall/control field difference is 81.6898.',
    }),
    'quantum-eraser': Object.freeze({
        level: 'behavioral',
        qualification: 'LOCKED CHECKERBOARD STATE-COUPLING SOURCE RESPONSE — CLOSED NEGATIVE for a quantum-eraser or measurement interpretation',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'At L=32 and tick 28, downstream field energy is 754.473 with the locked checkerboard versus 49.2727 without it, a ratio of 15.3122; the relative full-field difference is 4.58949.',
    }),
    's0-field-spacetime-forcing-boundary': Object.freeze({
        level: 'behavioral',
        qualification: 'NATIVE POINT-RESPONSE LOCALITY CONE — production wave behavior only; no Lorentzian metric or spacetime derivation',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'For eight ticks the exact support never advances by more than one lattice neighborhood per tick, reaches Chebyshev radius eight, conserves the periodic modified Hamiltonian to 6e-15, and remains unmanifested.',
    }),
    'flux-interference': Object.freeze({
        level: 'behavioral',
        qualification: 'FOUR-LOBE REFLECTION-SYMMETRIC BROADBAND WAVE FIELD — no detector fringe or physical interference-pattern claim',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'The isolated periodic wave map preserves exact even x and z reflection symmetries for 12 ticks without manifestation.',
    }),
    'flux-nested-standing': Object.freeze({
        level: 'behavioral',
        qualification: 'ORTHOGONAL REFLECTION-EVEN BROADBAND WAVE PAIRS — not pure standing eigenmodes',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'The isolated periodic wave map preserves the exact even x and z reflection symmetries of both orthogonal Gaussian pairs for 12 ticks without manifestation.',
    }),
    'flux-dual-substrate': Object.freeze({
        level: 'behavioral',
        qualification: 'MIRROR-POLARIZED NATIVE WAVE PAIR — dual-substrate operator is off; no two-sector ontology claim',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'The native periodic wave map preserves exact x-even and y/z-odd component parity for J and W over 12 ticks; dual_substrate and every non-wave term remain disabled.',
    }),
    'flux-meson': Object.freeze({
        level: 'behavioral',
        qualification: 'COUNTER-MOVING OPPOSITE-STATE TRANSPORT PROBE — no colors, confinement, bound state, or meson identity',
        test: 'engine/tests/test_scenario_velocity_wiring.cpp',
        target: 'scenario_velocity_wiring',
        assertion: 'With movement as the only active production term, the two states preserve velocities +/-0.05, accumulate exact signed remainders, translate by one y lattice unit after 21 ticks, and vacate their original sites.',
    }),
    'light-two-slit': Object.freeze({
        level: 'behavioral',
        qualification: 'TWO-SOURCE NATIVE LINEAR SUPERPOSITION — no barrier, slit, photon, or quantum claim; preregistered bidirectional contrast gate failed',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'At the fixed L=48 screen and tick 20, pointwise superposition residual is 6.77e-16 and both cross-term signs occur, but constructive contrast 0.0394 is below the 0.05 gate while destructive contrast is 0.0805.',
    }),
    's0-field-thomson-scattering': Object.freeze({
        level: 'behavioral',
        qualification: 'LOCKED-SOURCE LINEAR-SUPERPOSITION NULL — Thomson scattering and recoil are closed negative for this profile',
        test: 'engine/tests/campaign_thomson_recoil_observatory.cpp',
        target: 'thomson_recoil_observatory',
        assertion: 'A four-arm 200-tick decomposition is exactly repeatable and gives plus-minus-beam-minus-source max residual 3.86e-16 (relative L2 3.28e-15), with zero charge velocity; verdict LINEAR_SUPERPOSITION_NO_RECOIL_OBSERVED.',
    }),
    's0-field-thomson-unlocked-recoil': Object.freeze({
        level: 'behavioral',
        qualification: 'NATIVE FLUX-GRADIENT RECOIL RESPONSE — selected emergent-forces extension; no electron, Thomson, cross-section, or QED identity',
        test: 'engine/tests/campaign_thomson_unlocked_recoil.cpp',
        target: 'thomson_unlocked_recoil',
        assertion: 'At L=33 over 200 ticks, the exact lattice plane wave produces deterministic beam-minus-source displacement 0.1697027 and maximum speed 0.00370375 with the native emergent-force path; the legacy force path remains at 1.48e-16 displacement and exact repeat residual is zero.',
    }),
    'flux-cyclotron': Object.freeze({
        level: 'behavioral',
        qualification: 'IMPOSED-B NATIVE MAGNETIC-CURVATURE TEST — validates the selected v×curl(J) engine term, not emergent electromagnetism or a physical cyclotron',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'With Bz=1, v0=(0.12,0,0), and alpha*B*dt<0.01, the 80-tick Lorentz branch ends at (31.522,18.8348,24) with vy=-0.111379 and 1.224% speed drift, while the otherwise identical no-Lorentz control remains exactly straight at y=24.',
    }),
    'flux-screening': Object.freeze({
        level: 'behavioral',
        qualification: 'PREPARED OCTAHEDRAL POLARITY-SHELL GEOMETRY — net state is -5; no neutrality, medium response, or screening claim',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'The inert profile contains exactly one central + state and six face-orbit - states (net -5), a nonzero imposed compact radial dressing, and bit-exact primitive state/field persistence over eight ticks.',
    }),
    'flux-triad': Object.freeze({
        level: 'behavioral',
        qualification: 'PREPARED THREEFOLD INWARD-FLUX SEED — no binding, stability, color, or baryon claim',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'The inert L=48 profile contains exactly three unlocked + states at the rounded 120-degree positions, nonzero imposed inward dressing, and bit-exact primitive persistence over eight ticks with every binding term off.',
    }),
    'flux-thermalization': Object.freeze({
        level: 'behavioral',
        qualification: 'DETERMINISTIC LOCALIZED RANDOM-WAVE MIXING — linear spreading/dephasing only; no temperature, bath, entropy, or thermal-equilibrium claim',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'At L=32 the fixed-seed compact J/W patch starts entirely within Chebyshev radius 6; after 12 native-wave ticks 4.90346% of instantaneous field energy lies outside that radius while the exact periodic modified Hamiltonian drifts only 2.08e-14 and no state manifests.',
    }),
    'flux-vacuum-foam': Object.freeze({
        level: 'behavioral',
        qualification: 'FINITE DETERMINISTIC RANDOM-WAVE BALL — no ongoing stochastic source, quantum vacuum, virtual particles, or spacetime-foam claim',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'Two independent L=24 dispatches produce bit-exact fixed-seed J/W initial data and remain bit-exact after 12 source-free wave ticks; the periodic modified Hamiltonian is conserved below 1e-12 and the field remains unmanifested.',
    }),
    'flux-cascade': Object.freeze({
        level: 'behavioral',
        qualification: 'SUPERCRITICAL GAUSSIAN ONE-TICK GENESIS COHORT — independent selected-law events; no cascade, branching, recruitment, or pair-production claim',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'At L=32 with genesis as the only active term, two dispatches and first ticks replay exactly; the Gaussian seed produces 105 positive and 102 negative single-site events, with zero pair IDs.',
    }),
    'flux-random-genesis': Object.freeze({
        level: 'behavioral',
        qualification: 'FIXED-SEED RANDOM-PATCH ONE-TICK GENESIS COHORT — selected local hazard only; no spontaneous vacuum-pair or self-organization claim',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'At L=32 with genesis as the only active term, two fixed-seed random-patch setups and first ticks replay exactly; the cohort contains 179 positive and 168 negative single-site events, with zero pair IDs.',
    }),
    'flux-string-breaking': Object.freeze({
        level: 'behavioral',
        qualification: 'OUTWARD OPPOSITE-POLARITY FREE-TRANSPORT CONTROL — string breaking is absent; no color, confinement, tension, or pair-production term',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'With movement as the only active term, the +/- states at vx=-/+0.3 each translate one outward lattice face after four ticks, retain exact signed 0.2 remainders, and remain the only two states; no pair is produced.',
    }),
    'flux-baryon': Object.freeze({
        level: 'behavioral',
        qualification: 'THREEFOLD TANGENTIAL FREE-TRANSPORT CONTROL — no binding, color, quark, or baryon identity',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'With movement only, the three seeded + markers reach exact sites (32,25), (19,30), and (20,17) in the x-z plane after 30 ticks; the stationary opposite marker remains at (28,28,24), all four remain unlocked, and no binding occurs.',
    }),
    's0-field-photon-pulse': Object.freeze({
        level: 'behavioral',
        qualification: 'TRANSVERSE PACKET PHOTON GATE CLOSED NEGATIVE — exact transversality survives, but centroid speed and coherence fail',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'At L=48 over 20 isolated-wave ticks, the unmanifested transverse packet moves +x at centroid speed 0.461588 rather than C_SPEED=0.577350 and broadens by width ratio 1.64584, failing both preregistered photon-candidate gates.',
    }),
    's0-seed-thermal-ignition': Object.freeze({
        level: 'behavioral',
        qualification: 'FIXED-TEMPERATURE LANGEVIN/GENESIS BATH PROBE — CLOSED NEGATIVE for ignition at T=0.03 over the qualified finite run',
        test: 'engine/tests/test_scenario_behavior.cpp',
        target: 'scenario_behavior',
        assertion: 'On an initially empty L=16 lattice at imposed T=0.03 and gamma=0.02, two 100-tick CPU runs replay bit-exactly, produce finite field excitation 1000.82, and manifest 0 of 4096 sites; this single point establishes neither condensation nor a phase transition.',
    }),
    's0-seed-emergent-ic1': Object.freeze({
        level: 'behavioral', qualification: 'AXIAL A=10 FINITE GENESIS RESPONSE — CLOSED NEGATIVE for the advertised 25-site octahedron',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'At L=24, T=0.005, the fixed axial A=10 profile replays bit-exactly and has 3 manifested sites at both ticks 100 and 120, not 25.',
    }),
    's0-seed-emergent-ic3-collision': Object.freeze({
        level: 'behavioral', qualification: 'OPPOSITE A=5 TWO-SOURCE GENESIS RESPONSE — CLOSED NEGATIVE for two 2–3-site collision products',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'At L=24, T=0.005, the fixed opposite-source profile replays bit-exactly and has exactly 2 manifested sites at ticks 100 and 120.',
    }),
    's0-seed-emergent-ic4-subthreshold': Object.freeze({
        level: 'behavioral', qualification: 'SUBTHRESHOLD A=0.5 BATH CONTROL — zero manifestation over the qualified finite run',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'At L=24, T=0.005, the central 0.5*K_GENESIS seed replays bit-exactly and has zero manifested sites at ticks 100 and 120.',
    }),
    's0-seed-emergent-ic2-thermal-runaway': Object.freeze({
        level: 'behavioral', qualification: 'EMPTY T=0.05 LANGEVIN/GENESIS BATH — CLOSED NEGATIVE for thermal runaway over the qualified finite run',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'At L=24 with T=0.05 and no injected field, the fixed-seed history replays bit-exactly and has zero manifested sites at ticks 100 and 120.',
    }),
    's0-seed-emergent-ic1-diagonal': Object.freeze({
        level: 'behavioral', qualification: 'BODY-DIAGONAL A=10 FINITE GENESIS RESPONSE — no Z3 efficiency or 33-site-cluster result',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'At L=24, T=0.005, the fixed body-diagonal A=10 profile replays bit-exactly and has 1 manifested site at both ticks 100 and 120.',
    }),
    's0-seed-emergent-ic1-isotropic': Object.freeze({
        level: 'behavioral', qualification: 'SIX-AXIS A=10 FINITE GENESIS RESPONSE — no O_h bound-state or efficiency interpretation',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'At L=24, T=0.005, the fixed six-axis A=10 profile replays bit-exactly and has 8 manifested sites at both ticks 100 and 120.',
    }),
    's0-seed-emergent-ic1-viz': Object.freeze({
        level: 'behavioral', qualification: 'AXIAL A=20 ZERO-TEMPERATURE GENESIS RESPONSE — deterministic but not static or stable',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'At L=24 and T=0, the fixed axial A=20 response replays bit-exactly but decays from 22 manifested sites at tick 100 to 20 at tick 120.',
    }),
    's0-seed-emergent-ic1-diagonal-viz': Object.freeze({
        level: 'behavioral', qualification: 'BODY-DIAGONAL A=20 ZERO-TEMPERATURE GENESIS RESPONSE — deterministic but not static or stable',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'At L=24 and T=0, the fixed body-diagonal A=20 response replays bit-exactly but decays from 22 manifested sites at tick 100 to 20 at tick 120.',
    }),
    's0-seed-emergent-ic1-isotropic-viz': Object.freeze({
        level: 'behavioral', qualification: 'SIX-AXIS A=20 ZERO-TEMPERATURE GENESIS RESPONSE — deterministic but not static or stable',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'At L=24 and T=0, the fixed six-axis A=20 response replays bit-exactly but decays from 20 manifested sites at tick 100 to 18 at tick 120.',
    }),
    's0-seed-cluster-law': Object.freeze({
        level: 'behavioral', qualification: 'INTERACTIVE A=10 SELECTED GENESIS RESPONSE — one qualified default point, not a universal N(A) law',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'At the default A=10, L=24, T=0.005 point, the isolated wave+Gauss+genesis+Langevin profile replays bit-exactly and has 3 manifested sites at ticks 100 and 120; arbitrary interactive amplitudes remain unqualified.',
    }),
    's0-seed-de-broglie-clock': Object.freeze({
        level: 'behavioral',
        qualification: 'IMPOSED KLEIN–GORDON BLOCK CLOCK — selected mass term; no pilot-wave guidance or physical Compton-frequency derivation',
        test: 'engine/tests/test_scenario_behavior.cpp',
        supplementalTest: 'engine/tests/test_de_broglie_clock.cpp',
        target: 'scenario_behavior',
        assertion: 'The actual 7^3 block scenario isolates wave_propagation plus de_broglie_clock at imposed omega0=0.30; its center receives the exact first-tick -omega0^2 J kick and replays bit-exactly. The separate k=0 operator test verifies bounded oscillation at 2pi/omega0.',
    }),
    's0-seed-gravitational-wave': Object.freeze({
        level: 'behavioral',
        qualification: 'EXACT n=4 TRANSVERSE NATIVE HARMONIC — CLOSED NEGATIVE for a gravitational-wave identity',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'The isolated periodic wave follows the exact kick-drift lattice pole, conserves modified H below 1e-12, and contains no state, tensor, metric, mass source, or gravity operator.',
    }),
    's0-seed-time-gravity-well': Object.freeze({
        level: 'behavioral',
        qualification: 'EXACT ALIAS OF THE PLAIN n=4 WAVE — CLOSED NEGATIVE for a gravity-well or proper-time mechanism',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'The entry is bit-identical to s0-seed-gravitational-wave before and after 11 ticks and enables only wave_propagation; it supplies no well, clock, or latency observable.',
    }),
    's0-seed-time-twin-clocks': Object.freeze({
        level: 'behavioral',
        qualification: 'EXACT ALIAS OF THE PLAIN n=4 WAVE — CLOSED NEGATIVE for a twin-clock comparison',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'The entry is bit-identical to s0-seed-gravitational-wave before and after 11 ticks and contains no pair of clocks, observers, trajectories, or proper-time comparison.',
    }),
    's0-seed-schwarzschild': Object.freeze({
        level: 'behavioral',
        qualification: 'EXACT INERT INWARD INVERSE-SQUARE VECTOR ANSATZ — CLOSED NEGATIVE for a Schwarzschild metric or engine-gravity solution',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'Every noncentral vector equals -3 G_N K_B r/r^3, one central + marker is present, and all production terms are off; no metric, curvature, latency, horizon, or gravitational dynamics is computed.',
    }),
    's0-seed-time-horizon': Object.freeze({
        level: 'behavioral',
        qualification: 'EXACT ALIAS OF THE INERT INVERSE-SQUARE ANSATZ — CLOSED NEGATIVE for a horizon or time-dilation mechanism',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'The entry is bit-identical to the Schwarzschild-named inert ansatz before and after eight ticks and contains no clock, latency field, null surface, or proper-time observable.',
    }),
    's0-seed-gravitational-lensing': Object.freeze({
        level: 'behavioral',
        qualification: 'RADIAL-BACKGROUND PLUS TRANSVERSE-PACKET LINEAR NULL — CLOSED NEGATIVE for gravitational lensing',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'At L=48 over 16 isolated wave ticks, the full profile equals independently evolved radial-background plus packet components below 1e-12. The packet has an intrinsic 32.0000 to 31.8889 centroid drift, but the radial background induces exactly no additional deflection because no gravity-to-wave vertex exists.',
    }),
    's0-seed-up-quark': Object.freeze({
        level: 'behavioral', qualification: 'SELECTED + MARKER, COLOR-1 LABEL, AND A=0.5 AXIS-BIASED WAVE TEMPLATE — no up-quark identity',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'One inert + marker and imposed color label 1 accompany a source-free vector wave whose exact modified Hamiltonian is conserved; no fractional charge, quark mass, color gauge field, or confinement is present.',
    }),
    's0-seed-down-quark': Object.freeze({
        level: 'behavioral', qualification: 'SELECTED - MARKER, COLOR-2 LABEL, AND A=0.5 AXIS-BIASED WAVE TEMPLATE — no down-quark identity',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'One inert - marker and imposed color label 2 accompany a source-free vector wave with the same norm as the A=0.5 positive control; no fractional charge, mass, gauge field, or confinement is present.',
    }),
    's0-seed-strange-quark': Object.freeze({
        level: 'behavioral', qualification: 'SELECTED - MARKER, COLOR-3 LABEL, AND A=0.7 WAVE TEMPLATE — no strange-quark identity',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'The field norm follows only the imposed A-squared amplitude code and conserves the source-free wave invariant; no strangeness, flavor-changing channel, mass pole, or confinement exists.',
    }),
    's0-seed-charm-quark': Object.freeze({
        level: 'behavioral', qualification: 'SELECTED + MARKER, COLOR-1 LABEL, AND A=1.0 WAVE TEMPLATE — no charm-quark identity',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'The field norm follows only the imposed A-squared amplitude code and conserves the source-free wave invariant; no charm flavor, mass pole, decay, or confinement exists.',
    }),
    's0-seed-bottom-quark': Object.freeze({
        level: 'behavioral', qualification: 'SELECTED - MARKER, COLOR-2 LABEL, AND A=1.4 WAVE TEMPLATE — no bottom-quark identity',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'The field norm follows only the imposed A-squared amplitude code and conserves the source-free wave invariant; no bottom flavor, mass pole, decay, or confinement exists.',
    }),
    's0-seed-top-quark': Object.freeze({
        level: 'behavioral', qualification: 'SELECTED + MARKER, COLOR-3 LABEL, AND A=2.5 WAVE TEMPLATE — no top-quark identity',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'The field norm follows only the imposed A-squared amplitude code and conserves the source-free wave invariant; the marker remains stable rather than exhibiting a top decay and no mass pole exists.',
    }),
    's0-seed-anti-up-quark': Object.freeze({
        level: 'behavioral', qualification: 'SELECTED - MARKER, COLOR-1 LABEL, AND A=0.5 AXIS-BIASED WAVE TEMPLATE — no anti-up-quark identity',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'One inert - marker and imposed color label 1 accompany a source-free vector wave whose exact modified Hamiltonian is conserved; no fractional charge, antiquark mass, color gauge field, or confinement is present.',
    }),
    's0-seed-anti-down-quark': Object.freeze({
        level: 'behavioral', qualification: 'SELECTED + MARKER, COLOR-2 LABEL, AND A=0.5 AXIS-BIASED WAVE TEMPLATE — no anti-down-quark identity',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'One inert + marker and imposed color label 2 accompany a source-free vector wave with the same norm as the A=0.5 negative control; no fractional charge, mass, gauge field, or confinement is present.',
    }),
    's0-seed-anti-strange-quark': Object.freeze({
        level: 'behavioral', qualification: 'SELECTED + MARKER, COLOR-3 LABEL, AND A=0.7 WAVE TEMPLATE — no anti-strange-quark identity',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'The field norm follows only the imposed A-squared amplitude code and conserves the source-free wave invariant; no strangeness, flavor-changing channel, mass pole, or confinement exists.',
    }),
    's0-seed-anti-charm-quark': Object.freeze({
        level: 'behavioral', qualification: 'SELECTED - MARKER, COLOR-1 LABEL, AND A=1.0 WAVE TEMPLATE — no anti-charm-quark identity',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'The field norm follows only the imposed A-squared amplitude code and conserves the source-free wave invariant; no charm flavor, mass pole, decay, or confinement exists.',
    }),
    's0-seed-anti-bottom-quark': Object.freeze({
        level: 'behavioral', qualification: 'SELECTED + MARKER, COLOR-2 LABEL, AND A=1.4 WAVE TEMPLATE — no anti-bottom-quark identity',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'The field norm follows only the imposed A-squared amplitude code and conserves the source-free wave invariant; no bottom flavor, mass pole, decay, or confinement exists.',
    }),
    's0-seed-anti-top-quark': Object.freeze({
        level: 'behavioral', qualification: 'SELECTED - MARKER, COLOR-3 LABEL, AND A=2.5 WAVE TEMPLATE — no anti-top-quark identity',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'The field norm follows only the imposed A-squared amplitude code and conserves the source-free wave invariant; the marker remains stable rather than exhibiting a top decay and no mass pole exists.',
    }),
    's0-vacuum-electron': Object.freeze({
        level: 'behavioral', qualification: 'NEGATIVE MARKER PLUS SELECTED RADIAL FREE-WAVE TEMPLATE — no electron identity',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'One inert negative marker accompanies the base radial vector wave; no mass, spinor, Coulomb response, charge measurement, or matter pole is established.',
    }),
    's0-vacuum-muon': Object.freeze({
        level: 'behavioral', qualification: 'EXACT 1.2x COPY OF THE NEGATIVE-MARKER RADIAL WAVE — CLOSED NEGATIVE for a distinct muon mode',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'After factoring out 1.2, the field is identical to the base negative-marker template initially and after 12 ticks; no muon mass, lifetime, or flavor operator exists.',
    }),
    's0-vacuum-tau': Object.freeze({
        level: 'behavioral', qualification: 'EXACT 1.5x COPY OF THE NEGATIVE-MARKER RADIAL WAVE — CLOSED NEGATIVE for a distinct tau mode',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'After factoring out 1.5, the field is identical to the base negative-marker template initially and after 12 ticks; no tau mass, lifetime, or flavor operator exists.',
    }),
    's0-vacuum-positron': Object.freeze({
        level: 'behavioral', qualification: 'POSITIVE MARKER PLUS SELECTED RADIAL FREE-WAVE TEMPLATE — no positron identity',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'One inert positive marker accompanies the base radial vector wave; no mass, spinor, Coulomb response, charge measurement, or matter pole is established.',
    }),
    's0-vacuum-antimuon': Object.freeze({
        level: 'behavioral', qualification: 'EXACT 1.2x COPY OF THE POSITIVE-MARKER RADIAL WAVE — CLOSED NEGATIVE for a distinct antimuon mode',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'After factoring out 1.2, the field is identical to the base positive-marker template initially and after 12 ticks; no antimuon mass, lifetime, or flavor operator exists.',
    }),
    's0-vacuum-antitau': Object.freeze({
        level: 'behavioral', qualification: 'EXACT 1.5x COPY OF THE POSITIVE-MARKER RADIAL WAVE — CLOSED NEGATIVE for a distinct antitau mode',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'After factoring out 1.5, the field is identical to the base positive-marker template initially and after 12 ticks; no antitau mass, lifetime, or flavor operator exists.',
    }),
    's0-seed-higgs-field': Object.freeze({
        level: 'behavioral', qualification: 'IMPOSED VOLUME-FILLING VECTOR BACKGROUND — no scalar Higgs field or VEV mechanism',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'The deterministic nonuniform three-vector background evolves as a finite source-free wave and conserves modified H; it has no scalar degree of freedom, symmetry breaking, Yukawa coupling, or Higgs observable.',
    }),
    's0-seed-gluon': Object.freeze({
        level: 'behavioral', qualification: 'SELECTED MIXED-POLARIZATION VECTOR PACKET — no gluon or color-gauge identity',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'The unmanifested packet evolves under only the source-free vector wave map and conserves modified H; no color substrate, gauge connection, self-interaction, or confinement is enabled.',
    }),
    's0-vacuum-w-boson': Object.freeze({
        level: 'behavioral', qualification: 'POSITIVE MARKER PLUS SELECTED ANISOTROPIC VECTOR WAVE — no W-boson identity',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'One inert positive marker accompanies a source-free anisotropic vector template; no weak charge, mass pole, polarization representation, or decay channel exists.',
    }),
    's0-vacuum-z-boson': Object.freeze({
        level: 'behavioral', qualification: 'SELECTED INWARD RADIAL VECTOR WAVE — no Z-boson identity',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'The unmanifested radial template conserves the source-free wave invariant; no neutral weak current, mass pole, polarization representation, or decay channel exists.',
    }),
    's0-vacuum-higgs': Object.freeze({
        level: 'behavioral', qualification: 'SELECTED EQUAL-COMPONENT VECTOR BLOB — no scalar Higgs-boson identity',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'The unmanifested equal-component vector template evolves finitely and conserves modified H; it is a three-vector, not a scalar field, and has no Higgs mass pole or decay channel.',
    }),
    's0-vacuum-proton': Object.freeze({
        level: 'behavioral', qualification: 'UNLOCKED SELECTED-COLOR TRIAD — CLOSED NEGATIVE as a stable proton candidate',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'With only static-dressing force, selected color force, and movement active at L=24, the 3-site triad has 3/1/0 sites at ticks 8/16/32 and replays exactly; no bound proton mode survives.',
    }),
    's0-vacuum-neutron': Object.freeze({
        level: 'behavioral', qualification: 'ALTERNATE-POLARITY SELECTED-COLOR TRIAD — CLOSED NEGATIVE as a stable neutron candidate',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'The 3-site triad falls to one site at tick 8 and zero by tick 64 at L=24 under the isolated selected force/movement profile; no bound neutron mode survives.',
    }),
    's0-vacuum-pion-charged': Object.freeze({
        level: 'behavioral', qualification: 'UNLOCKED OPPOSITE-POLARITY PAIR — CLOSED NEGATIVE as a bound charged pion',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'Both selected-color sites are removed by tick 8 under the isolated force/color/movement profile; the residual field is static and no bound meson survives.',
    }),
    's0-vacuum-pion-neutral': Object.freeze({
        level: 'behavioral', qualification: 'EXACT ALIAS OF THE CHARGED-PION-LABELLED PAIR — neutral-pion distinction absent',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'Initial data and 16-tick evolution are bit-identical to s0-vacuum-pion-charged, and both sites are removed by tick 8; no neutral-specific degree of freedom exists.',
    }),
    's0-vacuum-kaon-charged': Object.freeze({
        level: 'behavioral', qualification: '1.88x-DRESSED UNLOCKED PAIR — CLOSED NEGATIVE as a bound kaon',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'The imposed 1.88 dressing boost does not bind the pair: both selected-color sites are removed by tick 8 and no kaon flavor or mass operator exists.',
    }),
    's0-seed-ee-annihilation': Object.freeze({
        level: 'behavioral', qualification: 'LONG-BASELINE NATIVE OPPOSITE-POLARITY COLLISION — not electron/positron or photon production',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'At L=24 the selected pair reaches collision removal exactly at tick 24 under movement alone; both states vanish, pre-existing field norm is redistributed downward, wave momentum remains exactly zero, and the 64-tick history replays exactly.',
    }),
    's0-seed-hydrogen': Object.freeze({
        level: 'behavioral', qualification: 'PREPARED LOCKED-TRIAD COULOMB COHORT — finite persistence, not a hydrogen atom',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'At L=24 the locked 3-site source and one mobile negative marker persist for 64 ticks under only Poisson-Coulomb force and movement with exact replay; no orbital pole, spectrum, binding energy, or emergent proton is shown.',
    }),
    's0-seed-helium': Object.freeze({
        level: 'behavioral', qualification: 'PREPARED LOCKED 12+2 COULOMB COHORT — CLOSED NEGATIVE as neutral helium',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'The imposed 12 locked plus 2 mobile sites persist for 64 ticks, but their signed state is -2 rather than neutral; no alpha-particle emergence, exchange term, orbital pole, spectrum, or helium binding energy exists.',
    }),
    's0-seed-h2-bond-formation': Object.freeze({
        level: 'behavioral', qualification: 'PREPARED TWO-NUCLEUS COULOMB COHORT — CLOSED NEGATIVE for bond formation',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'The six locked source sites persist, while both central mobile negative markers are removed by tick 64 under Poisson force and movement; no dynamic bond forms and no bond-energy observable is computed.',
    }),
    's0-seed-ew-phase-transition': Object.freeze({
        level: 'behavioral', qualification: 'NONNEGATIVE UNIFORM ADDITIVE DRIVE + GENESIS — CLOSED NEGATIVE for hysteresis or an EW transition',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'At L=16 the first step is uniform D(0)=0.025 to projection precision, counts are 0/0/0/2068 at ticks 16/24/32/64, and replay is exact. Because D(t)=(sin(0.01t)+1)*0.025 is never negative, the setup has no down-sweep and cannot form a hysteresis loop.',
    }),
    's0-seed-beta-decay': Object.freeze({
        level: 'behavioral', qualification: 'PREPARED WEAK-STRESS RAMP — CLOSED NEGATIVE as beta decay',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'At L=24 the alleged electron marker and neutral packet already exist at tick zero. With only dual substrate and weak transmutation active, stored wave momentum grows the field, the first polarity flip occurs at tick 54, seven flips occur by tick 64, and all four sites remain; no daughter creation or emission occurs.',
    }),
    's0-seed-quark-gluon-plasma': Object.freeze({
        level: 'behavioral', qualification: 'FIXED-SEED LANGEVIN TRANSPORT/OUTFLOW COHORT — CLOSED NEGATIVE as QGP',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'Eight neutral color-labelled markers at speed 0.5*C are transported through a T=0.02 wave bath with color force off. Counts are 8/8/8/1 at ticks 8/16/32/64; the journal records 145 movement and zero annihilation events, so depletion is open particle-boundary outflow, not deconfinement.',
    }),
    's0-seed-spark-of-life': Object.freeze({
        level: 'behavioral', qualification: 'PATTERNED FINITE GENESIS RESPONSE — CLOSED NEGATIVE for life or autocatalysis',
        test: 'engine/tests/test_scenario_behavior.cpp', target: 'scenario_behavior',
        assertion: 'At L=24 the prepared 16 locked + 8 mobile + 3 central sites produce exactly six genesis events by tick 8, giving counts 27/33/33/33 at ticks 1/8/16/32. There are zero evaporation and annihilation events and no further growth, replication, chemistry, metabolism, heredity, or autocatalytic observable.',
    }),
});

function makeScenario(category, id, title, tags = [], epistemicStatus = '[OPEN]') {
    const validation = SCALE0_SCENARIO_VALIDATION[id] || null;
    const admitted = validation?.level === 'behavioral';
    const qualification = admitted
        ? validation.qualification
        : 'RESEARCH SETUP — mechanically smoke-tested only; advertised behavior and physical identity are unvalidated';
    return {
        id,
        scale: 'lattice',
        sourceTitle: title,
        title: admitted ? title : `${title} — Research Setup (Behavior Unvalidated)`,
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
    makeScenario('4. Macroscopic Physics & Measurement', 's0-seed-massive-body', 'Locked Mass — Native Latency-Poisson Probe', ['seed', 'gravity'], '[EMERGENT] under [IMPOSED] gravity charge and Poisson latency law'),
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
