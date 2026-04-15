/**
 * Scenario Descriptions — Metadata for scenario dropdowns and info panels.
 *
 * Extracted from the main dashboard controller. Each scale's scenario
 * descriptions live here.
 * Human coders: add new scenario descriptions here when adding scenarios.
 */

// Scale 4 (Consciousness) scenario descriptions
export const CS_SCENARIO_DESCRIPTIONS = {
    'cs-threshold':      'Flux starts below the consciousness threshold K_C \u2248 3.60. As flux energy builds, the discriminant \u0394_k passes through zero, and roots transition from real (physics) through degenerate (measurement) to complex (consciousness). Watch the Domain indicator change.',
    'cs-high-coupling':  'Four-source flux interference with coupling and forces enabled. High flux density pushes well above K_C, producing strong consciousness intensity. The holographic figure becomes vivid as the observer\u2019s self-model stabilizes.',
    'cs-self-ref':       'Standing wave pattern: the observer meets itself. sLoop depth = 1 \u2014 a fixed point of the gap equation x\u00B2 = K(x \u2212 G*). The lattice determines its own coupling.',
    'cs-nested-sloop':   'Two orthogonal standing waves: aware of self-awareness. sLoop depth = 2. This is the algebraic expression of recursive self-referential closure.',
    'cs-chirality':      'Dual substrate with left/right asymmetric injection demonstrating parity violation. The chirality split mirrors the 3:1 alternating handedness of the dyadic Fourier shells.',
    'cs-boundary-orbit': 'Mandelbrot iteration at c = 1/G* \u2248 0.338, tracking the edge of chaos. The fixed points of z \u2192 z\u00B2 + c are exactly the consciousness roots y = 2.19 \u00B1 2.86i.',
    'cs-entangled':      'Full coupling with Bell parameter S = 2\u221A2 \u2248 2.83. All forces, genesis, and movement enabled. Demonstrates observer-lattice entanglement via complexification + sLoop coupling.',
    'cs-flow':           'Fast vortex pattern with effective \u03B8 < 52.54\u00B0 (object-dominant flow state). The holographic figure responds with rapid, outward-focused dynamics.',
    'cs-meditation':     'Gentle centered pulse with effective \u03B8 > 52.54\u00B0 (subject-dominant contemplative state). The observer turns inward, producing slow, resonant breathing patterns.',
};

export const QUANTUM_SCENARIO_DESCRIPTIONS = {
    'quantum-born-rule': 'Born Rule Convergence: Tests that manifestation probability converges to |J|^2. Runs N trials with random-phase Gaussian flux, accumulates positions, verifies Born rule emergence from deterministic lattice dynamics. [THEOREM]',
    'quantum-double-slit': 'Quantitative Double-Slit: Two coherent sources create interference on a detector screen. Measures fringe visibility V = (Imax-Imin)/(Imax+Imin) and fringe spacing. Tests wave-particle duality from the two-layer ontology.',
    'quantum-tunnel': 'Quantum Tunneling: Flux packet encounters a potential barrier (locked charge wall). Measures transmission T vs barrier width W. Expected: T proportional to exp(-2 kappa W), demonstrating evanescent wave penetration.',
    'quantum-well': 'Particle in a Box: Broadband flux confined between reflective walls. FFT of time series reveals discrete frequency peaks at f_n proportional to n^2, demonstrating energy quantization from boundary conditions.',
    'quantum-entangle': 'Entanglement Correlation: Pair production creates correlated +1/-1 particles. Measures spin-charge correlation C(d) vs separation distance. Tests decoherence length and correlation decay.',
    'quantum-aharonov-bohm': 'Aharonov-Bohm Effect: Two flux packets traverse paths around a confined solenoid. Phase shift at convergence point is proportional to enclosed flux, even though B=0 outside. Tests topological phase.',
    'quantum-casimir': 'Casimir Effect: Two parallel reflective plates in a vacuum foam background. Energy density between plates differs from outside, creating boundary-modified vacuum pressure. Expected: F proportional to 1/d^4.',
    'quantum-zeno': 'Quantum Zeno Effect: Near-threshold flux with periodic "measurement" (flux sampling). Frequent measurement suppresses decay (manifestation). Tests the measurement-inhibition prediction.',
};

// ─────────────────────────────────────────────────────────────────────
// Scale 0 — FTD-derived SM particle seed scenarios.
//
// EPISTEMIC WARNING: This table is load-bearing. Every entry has its
// status classified per the FTD epistemic tag system
// (docs/theory/REF_EPISTEMIC_LABELS.md). Read each tag before acting on
// the data. In particular, do NOT conflate [THEOREM] for a derived mass
// with a [THEOREM] for a lattice configuration — FTD derives m_e but
// not a structural test for "electron-ness". The configuration that
// realizes m_e is [SELECTION], not [THEOREM].
//
// Allowed tags:
//   [THEOREM]    — rigorously proven from FTD axioms
//   [SELECTION]  — argued from consistency, not uniquely proven
//   [CONJECTURE] — proposed interpretation requiring validation
//   [IMPOSED]    — parameter choice or model calibration
//   [OPEN]       — unresolved question
//
// DO NOT INVENT NEW TAGS. DO NOT UPGRADE A TAG WITHOUT A CORRESPONDING
// THEORY DOCUMENT. Labelling a [SELECTION] as [THEOREM] in this file
// constitutes an epistemic regression that the FTD project explicitly
// forbids.
// ─────────────────────────────────────────────────────────────────────
export const S0_SEED_SCENARIO_METADATA = {
    's0-seed-electron': {
        title: 'Electron seed (unit negative charge + dressing)',
        desc: 'Single s=\u22121 site at the lattice center with radial-inward flux envelope of scale K_B. This is the DERIV_DARK_SECTOR \u00a75.2 particle definition in the dispositional layer: { state, flux envelope, id }. No vortex, no topology \u2014 just a charged seed.',
        epistemic: [
            ['Configuration', '[SELECTION]', 'DERIV_DARK_SECTOR \u00a75.2 seed+envelope picture. Structurally motivated, not uniquely proven.'],
            ['Name "electron"', '[IMPOSED]', 'FTD derives m_e but has no structural test for "electron-ness". The label is engineered, not derived.'],
            ['Mass m_e', '[THEOREM]', 'm_e = m_P\u00b7\u221a(2\u03c0)\u00b7(16/3)\u00b7\u03b1\u00b9\u00b9 (0.27% error). Derived, but mass does NOT encode spatial structure.'],
        ],
    },
    's0-seed-photon': {
        title: 'Photon seed (transverse massless flux wave at c = 1/\u221a3)',
        desc: 'State-0 everywhere (no matter) with a J_z-polarized Gaussian flux pulse launched at x \u2248 N/4, propagating in +x. The wave speed c = 1/\u221a3 emerges from CFL stability on the cubic lattice.',
        epistemic: [
            ['Wave propagation', '[THEOREM]', 'Massless transverse flux wave at c = 1/\u221a3 follows from the cubic-lattice wave equation + CFL stability.'],
            ['Polarization (2 modes)', '[THEOREM]', 'Two transverse modes enforced by the Gauss constraint \u2207\u00b7J = 0.'],
            ['Name "photon"', '[SELECTION]', 'Identifying the transverse flux wave with the SM photon is structurally consistent but not uniquely forced.'],
        ],
    },
    's0-seed-proton-candidate': {
        title: 'Proton candidate (3-site positive cluster) \u2014 NOT "uud"',
        desc: 'Three s=+1 particles on an equilateral triangle at the lattice center with weak radial-outward flux dressing. The "u-u-d" story is NOT encoded: FTD has no color axis, no flavor label, no orientation-dependent quark identity. This scenario tests only whether a 3-body positive cluster persists under substrate dynamics.',
        epistemic: [
            ['3-site cluster configuration', '[SELECTION]', 'Consistent with baryon number 3. Triangle geometry is one choice among many \u2014 not uniquely forced.'],
            ['Name "proton"', '[IMPOSED]', 'A label on the cluster. Do NOT read color, flavor, or quark identity from the triangle vertices.'],
            ['Mass ratio m_p/m_e', '[THEOREM]', 'm_p/m_e = N_eff/\u03b1 + N_base\u00b7N_eff + N_c = 1836.47 (174 ppm). The ratio is derived, but has NO spatial expression.'],
            ['LANDMINE', '[WARNING]', 'Do NOT interpret J_x-dominant flux as "red quark" or map vertices to u/d. The BCC\u2192SU(3) link is about the gluon propagator, not per-quark orientation.'],
        ],
    },
};

/**
 * Render S0 seed metadata as a plain-text block suitable for the
 * <details> description panel. Returns an empty string if no entry
 * exists for this scenario.
 */
export function formatS0SeedMetadata(name) {
    const meta = S0_SEED_SCENARIO_METADATA[name];
    if (!meta) return '';
    const lines = [meta.title, '', meta.desc, '', 'Epistemic status:'];
    for (const [field, tag, note] of meta.epistemic) {
        lines.push(`  \u2022 ${field}: ${tag}`);
        lines.push(`      ${note}`);
    }
    return lines.join('\n');
}
