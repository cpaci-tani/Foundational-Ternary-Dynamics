/**
 * Scenario Descriptions — Metadata for scenario dropdowns and info panels.
 *
 * Extracted from the main dashboard controller. Each scale's scenario
 * descriptions live here.
 * Human coders: add new scenario descriptions here when adding scenarios.
 */

export const QUANTUM_SCENARIO_DESCRIPTIONS = {
    'quantum-born-rule': 'Native Genesis Response: samples the engine’s thresholded genesis events against a seeded flux profile. This is a genesis diagnostic; it does not implement wave-function collapse or prove a Born distribution.',
    'quantum-double-slit': 'Classical Two-Source Interference: two coherent transverse flux sources form a native wave-interference pattern with genesis disabled. It does not measure accumulated single-particle impacts.',
    'quantum-eraser': 'State-Grid Transmission Prototype: evolves flux around a selected locked-state grid. No which-way observable, polarization measurement, or erasure operation is implemented.',
    'quantum-tunnel': 'State-Wall Transmission: measures native flux reflection and transmission at a wall of locked manifested sites. The wall is not a calibrated Schrödinger potential.',
    'quantum-well': 'Imposed Standing Harmonics: visualizes selected harmonics between marker walls. The walls do not impose a wave boundary condition or derive an n² spectrum.',
    'quantum-entangle': 'Tagged Pair Correlation: initializes opposite states and opposite flux with one shared pair ID. This is exact classical pair bookkeeping, not a Bell-entanglement demonstration.',
    'quantum-aharonov-bohm': 'Solenoid Two-Path Topology: provides the geometry for a future holonomy test. No gauge-invariant phase observable is presently extracted.',
    'quantum-casimir': 'Parallel-Plate Vacuum Null Setup: provides plates and a seeded noise field. No vacuum-ensemble subtraction or plate-force estimator is presently implemented.',
    'quantum-zeno': 'Unobserved Decay Control: supplies a near-threshold baseline for a future intervention test. The engine has no measurement operator, so no Zeno effect is currently tested.',
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
// Allowed tags (the canonical CLAUDE.md § Epistemic Tags vocabulary —
// this file uses the subset listed here; all are canonical, none invented):
//   [THEOREM]                       — rigorously proven from FTD axioms
//   [DERIVED]                       — explicit chain from axioms/prior theorems
//   [SELECTION]                     — argued from consistency, not uniquely proven
//   [STRONGLY MOTIVATED CONJECTURE] — empirical match + structural uniqueness,
//                                     no derivation chain (e.g. the FTD-0013
//                                     x_+ ↔ 1/α identification and the mass
//                                     ratios downstream of it, per LEDGER)
//   [PARAMETRIC]                    — SM formula filled with FTD numbers
//   [CONJECTURE]                    — proposed interpretation requiring validation
//   [IMPOSED] / [EXTERNAL INPUT]    — parameter choice / external value
//   [EMERGENT]                      — behavior arising from dynamics
//   [PARTIAL]                       — partially established
//   [OPEN]                          — unresolved question
//   [AXIOM]                         — structural postulate
//   [DEMO] / [NOT SHOWN] / [WARNING] — UI-scenario annotations (non-epistemic)
//
// DO NOT INVENT NEW TAGS. DO NOT UPGRADE A TAG WITHOUT A CORRESPONDING
// THEORY DOCUMENT / LEDGER ROW. Labelling a [SELECTION] or a
// [STRONGLY MOTIVATED CONJECTURE] as [THEOREM] in this file constitutes an
// epistemic regression that the FTD project explicitly forbids. Tags here
// must match the canonical LEDGER (docs/theory/07_assessment/core_ledgers/
// LEDGER.md); when in doubt, align to LEDGER, do not re-decide status.
// ─────────────────────────────────────────────────────────────────────
export const S0_SEED_SCENARIO_METADATA = {
    's0-seed-dynamical-flux-dressing': {
        title: 'Dynamical flux dressing — native source response',
        desc: 'A single locked positive ternary site starts with exactly zero flux and zero wave momentum. On each tick the existing state–flux coupling drives -G_C grad(s), so the surrounding field is produced by the engine rather than painted into the initializer. “Aura” is only informal shorthand for this source-conditioned dynamical field.',
        epistemic: [
            ['Source term', '[DERIVED]', 'The first-tick six-face response is the direct discrete variation of the written +G_C s div(J) coupling.'],
            ['Dynamical field', '[EMERGENT]', 'Nonzero J and wave momentum arise from the frozen production tick starting from zero field.'],
            ['Radial dressing', '[CLOSED NEGATIVE]', 'FTD-0476 measured radial alignment 0.73760 against the locked 0.75 gate.'],
            ['Attached dressing / wake / released field', '[CLOSED NEGATIVE]', 'The prescribed-motion and source-off histories miss the locked attachment, wake, and outgoing-field gates.'],
            ['Reciprocal aura / radiation mechanism', '[OPEN]', 'A future dynamically caused source history must close matter work, field energy, momentum, and any detached component.'],
            ['Electromagnetic or photon identity', '[OPEN]', 'No gauge redundancy, Ward identity, detector response, quantization, or empirical normalization is supplied by this scenario.'],
            ['Flux lines', '[CONJECTURE]', 'Rendered lines are integral curves of the sampled vector field, not literal substrate filaments.'],
        ],
    },
    's0-seed-moving-source-reciprocity': {
        title: 'Driven polarity — qualified sub-voxel response',
        desc: 'An initially resting unlocked polarity is spatially separated from a finite transverse packet. The existing selected G_C s grad|J| extension produces a deterministic 0.203598-cell response at L=65 through 72 ticks, but no integer hop. The display is therefore a qualified force-response discriminator, not a moving-source, wake, photon, or radiation scenario.',
        epistemic: [
            ['Sub-voxel response', '[EMERGENT WITHIN SELECTED EXTENSION]', 'The mobile arms respond by 0.203598 cells with no scripted velocity; source-only and locked controls remain at rest and the exact repeat residual is zero.'],
            ['Manifested motion', '[CLOSED NEGATIVE]', 'Neither polarity reaches the preregistered half-cell threshold or produces an integer movement event in 72 ticks.'],
            ['Accelerating force', '[SELECTION]', 'FTD-0435 rejects this G_C s grad|J| channel as ordinary electric qE and finds a mixed-polarity self-field/interference response.'],
            ['Dressing / wake / detached field', '[CLOSED NEGATIVE]', 'Near-field fractions are 0.438–0.465, the trailing/leading ratio is 1.0, and mean-radius growth is 1.102 rather than the locked 2.0 gate.'],
            ['Energy-momentum reciprocity', '[CLOSED NEGATIVE FOR REGISTERED AUDIT]', 'Normalized residuals reach 1.961e-6 in energy and 2.417e-3 in selected momentum, above the locked 1e-6 gates.'],
            ['Flux lines', '[CONJECTURE]', 'Lines are instantaneous integral curves of J, not paths, strings, or histories.'],
            ['-wave_vel and Poynting overlays', '[DIAGNOSTIC]', 'These are field-change and energy-flow proxies in the selected lattice dictionary, not detector-calibrated electromagnetic observables.'],
        ],
    },
    // Audit-3 + Audit-4 2026-04-28 metadata removals: s0-seed-{electron,
    // muon, tau, photon, w-boson, z-boson, higgs-boson, positron, pion,
    // proton-l4, neutron, electron-l3, neutrino, quark, antiquark,
    // proton-candidate, symmetry-regression}. All canonical entries
    // moved to s0-vacuum-* (or removed for being CI artefacts).
    /* removed metadata kept in comment for reference only (audit history):
    's0-seed-electron': {
        title: 'Electron seed (unit negative charge + dressing)',
        desc: 'Single s=\u22121 site at the lattice center with radial-inward flux envelope of scale \\(K_B\\). This is the DERIV_DARK_SECTOR \u00a75.2 particle definition in the dispositional layer: { state, flux envelope, id }. No vortex, no topology \u2014 just a charged seed.',
        epistemic: [
            ['Configuration', '[SELECTION]', 'DERIV_DARK_SECTOR \u00a75.2 seed+envelope picture. Structurally motivated, not uniquely proven.'],
            ['Name "electron"', '[IMPOSED]', 'FTD derives \\(m_e\\) but has no structural test for \u201celectron-ness\u201d. The label is engineered, not derived.'],
            ['Mass \\(m_e\\)', '[STRONGLY MOTIVATED CONJECTURE]', '\\(m_e = m_P \\sqrt{2\\pi} (16/3) \\alpha^{11}\\) (0.19% error per FTD-0015). Inherits FTD-0013 status: [STRONGLY MOTIVATED CONJECTURE] per LEDGER, not [THEOREM]. (Reference-only — this metadata block is commented out; retagged + error corrected 0.27%→0.19% for accuracy, audit P1-9, 2026-05-27.)'],
        ],
    },
    's0-seed-photon': {
        title: 'Photon seed (transverse massless flux wave at \\(c = 1/\\sqrt{3}\\))',
        desc: 'State-0 everywhere (no matter) with a \\(J_z\\)-polarized Gaussian flux pulse launched at \\(x \\approx N/4\\), propagating in +x. The wave speed \\(c = 1/\\sqrt{3}\\) is a [SELECTION] (FTD-0407): the production 18-point stencil permits c up to sqrt(3)/2, so 1/sqrt(3) is conservative rather than forced.',
        epistemic: [
            ['Wave propagation', '[SELECTION]', 'Massless transverse flux wave at \\(c = 1/\\sqrt{3}\\) is the selected lattice speed (FTD-0407); the stencil stability ceiling is sqrt(3)/2, so the value is conservative, not forced.'],
            ['Polarization (2 modes)', '[THEOREM]', 'Two transverse modes enforced by the Gauss constraint \\(\\nabla \\cdot J = 0\\).'],
            ['Name "photon"', '[SELECTION]', 'Identifying the transverse flux wave with the SM photon is structurally consistent but not uniquely forced.'],
        ],
    },
    's0-seed-muon': {
        title: 'Muon seed (2nd-generation lepton, \\(m_\\mu/m_e = 207\\))',
        desc: 'Same topology as the electron seed \u2014 unit s=\u22121 core with radial-inward flux envelope at scale \\(K_B\\) \u2014 with a 20% amplitude boost to VISUALLY convey the heavier rest-mass. Note: the mass ratio 207 is derived from framework integers \\(3 b_3 (b_3+N_c) - N_c\\), but FTD has NO spatial form for lepton mass. The envelope scale you see here is a visualization choice, not a theory prescription.',
        epistemic: [
            ['Configuration', '[SELECTION]', 'Same envelope shape as electron \u2014 visualization choice. FTD does not prescribe a spatial form for lepton mass.'],
            ['Name "muon"', '[IMPOSED]', 'Structural test for \u201cmuon-ness\u201d absent \u2014 label is engineered.'],
            ['Mass ratio \\(m_\\mu/m_e = 207\\)', '[STRONGLY MOTIVATED CONJECTURE]', '\\(3 b_3 (b_3+N_c) - N_c = 3 \\cdot 7 \\cdot 10 - 3 = 207\\), a framework-integer match 0.11% from experimental 206.77. The integer arithmetic is exact, but the physical identification inherits FTD-0013 (\\(x_+ \\leftrightarrow 1/\\alpha\\)) status: [STRONGLY MOTIVATED CONJECTURE] per LEDGER, not [THEOREM]. Retagged 2026-05-27 (audit P1-9).'],
            ['Amplitude scaling', '[SELECTION]', 'Envelope amplitude slightly boosted over electron to suggest higher field concentration; chosen to stay below \\(K_\\mathrm{GENESIS}\\) so no spurious genesis fires.'],
        ],
    },
    's0-seed-tau': {
        title: 'Tau seed (3rd-generation lepton, \\(m_\\tau/m_e = 3477\\))',
        desc: 'Same topology as electron/muon \u2014 unit s=\u22121 core with radial-inward flux envelope at scale \\(K_B\\) \u2014 with a 50% amplitude boost to VISUALLY suggest heavier rest-mass. The ratio 3477 is derived from framework integers \\((N_\\mathrm{eff}+N_\\mathrm{base}) \\cdot \\mu_\\mathrm{ratio} - 2 N_c b_3\\), but FTD has NO spatial form for lepton mass. The envelope you see is a visualization choice.',
        epistemic: [
            ['Configuration', '[SELECTION]', 'Same envelope shape as electron \u2014 visualization choice. Spatial form not prescribed by theory.'],
            ['Name "tau"', '[IMPOSED]', 'Label only \u2014 no structural test.'],
            ['Mass ratio \\(m_\\tau/m_e = 3477\\)', '[STRONGLY MOTIVATED CONJECTURE]', '\\((N_\\mathrm{eff}+N_\\mathrm{base}) \\cdot \\mu_\\mathrm{ratio} - 2 N_c b_3 = 17 \\cdot 207 - 42 = 3477\\), matching experimental 3477.23 to 0.01%. The integer arithmetic is exact, but the physical identification inherits FTD-0013 status: [STRONGLY MOTIVATED CONJECTURE] per LEDGER, not [THEOREM]. Retagged 2026-05-27 (audit P1-9).'],
            ['Amplitude scaling', '[SELECTION]', 'Envelope amplitude boosted to illustrate a more concentrated flux energy; chosen to stay below \\(K_\\mathrm{GENESIS}\\) (visual only, not a mass representation).'],
        ],
    },
    // ─────────────────────────────────────────────────────────────
    // LHC Standard Model physics (added 2026-04-17)
    // ─────────────────────────────────────────────────────────────
    // These 13 scenarios cover the particle content the LHC measures:
    // Higgs (field + boson), electroweak bosons (W, Z), gluon, all 6
    // quark flavours, plus two process demos (beta decay, e⁺e⁻
    // annihilation). Every scenario tags itself honestly — FTD has
    // derivable mass ratios but NO spatial form for any SM particle.
    // The envelopes here are visualization choices.
    // ─────────────────────────────────────────────────────────────

    's0-seed-higgs-boson': {
        title: 'Higgs boson seed (\\(m_H \\approx 125\\) GeV)',
        desc: 'Scalar (spin-0, charge-0) concentrated lump at the lattice centre. Amplitude scaled to hint at the Higgs mass relative to \\(m_e\\) via the FTD formula \\(m_H/m_e = N_\\mathrm{eff}/\\alpha^2\\) (the \\(m_H = 124.75\\) GeV parametric insertion). Spatial form is [SELECTION] \u2014 FTD has no theory of boson spatial shape; the localised Gaussian here is a visualisation choice so the scalar can be seen distinctly from the leptons.',
        epistemic: [
            ['Configuration', '[SELECTION]', 'Localised scalar envelope \u2014 visualisation choice. No theory prescription for Higgs spatial form.'],
            ['Mass \\(m_H = 124.75\\) GeV', '[STRUCTURALLY MOTIVATED PARAMETRIC]', '\\(m_H = (N_\\mathrm{eff}/\\alpha^2) m_e = 124.75\\) GeV. Against the canonical edition PDG-2024 125.20 +/- 0.11 GeV (REF_EXTERNAL_CONSTANTS.md) this is -4.1 sigma, i.e. experimentally EXCLUDED as an exact relation (LEDGER FTD-0017/FTD-0348). Standard electroweak mass formula filled with FTD numbers, not a derivation.'],
            ['Self-coupling \\(\\lambda_H\\)', '[PARAMETRIC]', '\\(\\lambda_H = m_H^2 / (2v^2) \\approx 0.129\\) is the Standard-Model tree-level relation filled with the FTD \\(m_H\\) and VEV — a parametric insertion, no FTD axiom enters. Retagged from [DERIVED] 2026-05-27 (audit P1-10).'],
            ['Name "Higgs"', '[IMPOSED]', 'Label. No structural test for Higgs-ness in the seed configuration.'],
        ],
    },
    's0-seed-higgs-field': {
        title: 'Higgs field vacuum (uniform VEV with fluctuations)',
        desc: 'Uniform low-amplitude flux background throughout the lattice with small random fluctuations \u2014 a visualisation of the Higgs field vacuum expectation value. In FTD the \u201cHiggs VEV\u201d is \\(v = m_P \\sqrt{2\\pi} \\alpha^8 = 246.09\\) GeV (0.05% from experimental); on the lattice we scale it to \\(K_B\\) so it\'s visible. This scenario does not include the Mexican-hat potential dynamics \u2014 it just shows the vacuum you\'d perturb.',
        epistemic: [
            ['Configuration', '[SELECTION]', 'Uniform flux at a tunable amplitude represents the VEV background. Lattice-rescaled for visibility.'],
            ['Higgs VEV \\(v = 246.09\\) GeV', '[STRUCTURALLY MOTIVATED PARAMETRIC]', '\\(v = m_P \\sqrt{2\\pi} \\alpha^8\\) (0.05% from experimental 246.22 GeV). A rung of the same alpha-ladder as m_e, whose anchor is [SMC] (FTD-0015); a rung cannot be [THEOREM] while alpha is [SMC] (particle_masses.h:103-109). The number 246.09 is separately booked [IMPOSED] as an SM reference input, a different object from the formula.'],
            ['Symmetry breaking', '[NOT SHOWN]', 'Mexican-hat potential dynamics NOT implemented in the engine \u2014 this scenario is purely the vacuum state.'],
        ],
    },
    's0-seed-w-boson': {
        title: 'W boson seed (charged weak mediator, \\(m_W \\approx 80.4\\) GeV)',
        desc: 'Charged (unit s=+1) lump with radial flux envelope at a scale chosen to visually mark the W+/W\u2212 weak gauge boson. Pairs with dual-substrate \\(J_L\\) dominance to reflect the W\'s left-chiral coupling. Mass \\(m_W\\) follows from FTD\'s Weinberg angle via \\(m_W = m_Z \\cos\\theta_W = m_Z \\sqrt{10/13}\\) \u2014 ratio derived, absolute scale from \\(m_Z\\) input.',
        epistemic: [
            ['Configuration', '[SELECTION]', 'Localised charged envelope. Chirality hint is valid only with dual_substrate toggle on.'],
            ['\\(m_W/m_Z\\) ratio', '[THEOREM]', '\\(\\cos\\theta_W = \\sqrt{1 - 3/13} = \\sqrt{10/13} \\approx 0.877\\) follows from \\(\\sin^2\\theta_W = N_c/N_\\mathrm{eff}\\).'],
            ['\\(m_W\\) absolute', '[IMPOSED]', 'Requires \\(m_Z = 91.19\\) GeV input (external).'],
            ['W/Z distinction', '[SELECTION]', 'In FTD the W is an OPERATOR, not a configuration. The seed is a visualisation of the excitation.'],
        ],
    },
    's0-seed-z-boson': {
        title: 'Z boson seed (neutral weak mediator, \\(m_Z = 91.19\\) GeV)',
        desc: 'Neutral (s=0 core surrounded by flux) localised envelope marking the \\(Z^0\\) electroweak gauge boson. Paired with the W scenario for weak-sector visualisation. \\(m_Z\\) is used as an INPUT in the ontic chain (\\(\\sin^2\\theta_W = N_c/N_\\mathrm{eff}\\) ties \\(m_W\\) to \\(m_Z\\)).',
        epistemic: [
            ['Configuration', '[SELECTION]', 'Neutral lump; field concentrated around an unfilled core.'],
            ['\\(m_Z = 91.1876\\) GeV', '[EXTERNAL INPUT]', 'Used as the electroweak mass scale in ontic.h:467. Not derived from \\(G^*\\).'],
            ['Name', '[IMPOSED]', 'Structural test for Z-ness absent.'],
        ],
    },
    */
    's0-seed-gluon': {
        title: 'Gluon-labelled vector packet (gluon identity rejected)',
        desc: 'Transverse flux wave similar to the photon seed, but amplitude is dominated by one Cartesian axis to suggest color charge (the lattice\'s dominant-flux-axis \u2194 color labelling). It evolves under only the source-free vector wave map at \\(c = 1/\\sqrt{3}\\), massless. No color substrate, gauge connection, self-interaction, or confinement is enabled, so this packet is not an SM gluon and instantiates none of the SU(3) gauge content of the BCC-to-SU(3) structure.',
        epistemic: [
            ['Propagation', '[SELECTION]', 'Massless transverse wave at \\(c = 1/\\sqrt{3}\\) (selected lattice speed, FTD-0407).'],
            ['Color label', '[IMPOSED]', '\\(\\mathbb{Z}_3\\) color labelling from the dominant flux axis; the seed biases one axis to make the label visible. It is a display label, not a gauge charge.'],
            ['Gluon / SU(3) gauge content', '[CLOSED NEGATIVE]', 'Registry [CLOSED NEGATIVE] gluon identity: the seed carries no color substrate, gauge connection, self-interaction, or confinement, so identifying it with the SM gluon is rejected. (The separate BCC-to-SU(3) factorization concerns the gluon propagator structure, not this vector packet.)'],
        ],
    },

    's0-seed-up-quark': {
        title: 'Up quark seed (u, 1st gen, charge +2/3)',
        desc: 'Colored s=+1 particle at the centre with narrow Gaussian flux envelope, color-labelled via dominant-axis assignment (R = color 1). Represents the up quark for visualisation only. Individual quark mass (~2.16 MeV MS-bar) is NOT derived in FTD \u2014 see TRACKER §4.1. Amplitude chosen small relative to \\(K_B\\).',
        epistemic: [
            ['Configuration', '[SELECTION]', 'Small coloured envelope; color label R from dominant flux axis.'],
            ['Mass \\(m_u\\)', '[OPEN]', 'Individual quark masses not derivable from framework integers; inserted externally.'],
            ['Up-quark identity', '[CLOSED NEGATIVE]', 'Registry [CLOSED NEGATIVE] quark identity: an inert + marker and imposed color label on a source-free A=0.5 vector wave template carry no fractional charge, quark mass, color gauge field, or confinement, so the up-quark identification is rejected.'],
            ['Charge +2/3', '[IMPOSED]', 'Ternary state is \u00b11; fractional charge is a label, not a substrate property.'],
        ],
    },
    's0-seed-down-quark': {
        title: 'Down quark seed (d, 1st gen, charge \u22121/3)',
        desc: 'Colored s=\u22121 particle (green color label) at the centre with narrow flux envelope. Visualisation partner for up-quark; individual mass not derived.',
        epistemic: [
            ['Configuration', '[SELECTION]', 'Small coloured envelope; color G from dominant flux axis.'],
            ['Mass \\(m_d\\)', '[OPEN]', 'Not derivable from framework integers.'],
            ['Down-quark identity', '[CLOSED NEGATIVE]', 'Registry [CLOSED NEGATIVE] quark identity: an inert - marker and imposed color label on a source-free A=0.5 vector wave template carry no fractional charge, quark mass, color gauge field, or confinement, so the down-quark identification is rejected.'],
            ['Charge \u22121/3', '[IMPOSED]', 'Fractional charge is a label.'],
        ],
    },
    's0-seed-strange-quark': {
        title: 'Strange quark seed (s, 2nd gen, charge \u22121/3)',
        desc: 'Heavier variant of the down quark \u2014 same color/charge, slightly larger envelope to hint at the higher mass (~93 MeV). Generation hierarchy and individual mass both not derived from FTD.',
        epistemic: [
            ['Configuration', '[SELECTION]', 'Amplitude boosted vs down to suggest heavier mass.'],
            ['Mass \\(m_s\\)', '[OPEN]', 'Not derivable.'],
            ['Generation hierarchy', '[OPEN]', 'FTD derives 3 generations via Moore shells but not individual flavour masses.'],
            ['Strange-quark identity', '[CLOSED NEGATIVE]', 'Registry [CLOSED NEGATIVE] quark identity: an inert marker and imposed color label on a source-free A=0.7 vector wave template carry no strangeness, fractional charge, mass pole, or confinement, so the strange-quark identification is rejected.'],
        ],
    },
    's0-seed-charm-quark': {
        title: 'Charm quark seed (c, 2nd gen, charge +2/3, m \u2248 1.27 GeV)',
        desc: 'Heavier partner to up, same color pattern, larger envelope. Charmonium (J/\u03c8) is the cc\u0304 bound state \u2014 see the pair scenario.',
        epistemic: [
            ['Configuration', '[SELECTION]', 'Amplitude boost over up.'],
            ['Mass \\(m_c\\)', '[OPEN]', 'Not derivable.'],
            ['Charm-quark identity', '[CLOSED NEGATIVE]', 'Registry [CLOSED NEGATIVE] quark identity: an inert + marker and imposed color label on a source-free A=1.0 vector wave template carry no charm flavor, fractional charge, mass pole, or confinement, so the charm-quark identification is rejected.'],
        ],
    },
    's0-seed-bottom-quark': {
        title: 'Bottom quark seed (b, 3rd gen, charge \u22121/3, m \u2248 4.18 GeV)',
        desc: 'Heavier variant of the strange/down sector. Bottomonium (\u03a5) is the bb\u0304 bound state.',
        epistemic: [
            ['Configuration', '[SELECTION]', 'Larger envelope vs strange.'],
            ['Mass \\(m_b\\)', '[OPEN]', 'Not derivable.'],
            ['Bottom-quark identity', '[CLOSED NEGATIVE]', 'Registry [CLOSED NEGATIVE] quark identity: an inert - marker and imposed color label on a source-free A=1.4 vector wave template carry no bottom flavor, fractional charge, mass pole, or confinement, so the bottom-quark identification is rejected.'],
        ],
    },
    's0-seed-top-quark': {
        title: 'Top quark seed (t, 3rd gen, \\(m \\approx 172.8\\) GeV \\(\\approx v_\\mathrm{Higgs}\\))',
        desc: 'Heaviest fermion; mass nearly matches the Higgs VEV. In FTD this supports a "Yukawa at unity" story: \\(y_t \\approx 1\\) if \\(m_t \\approx v\\). The seed is a strongly-concentrated lump.',
        epistemic: [
            ['Configuration', '[SELECTION]', 'Heaviest envelope in the quark catalog.'],
            ['\\(m_t \\approx v_\\mathrm{Higgs}\\)', '[SELECTION]', 'Supports Yukawa-at-unity interpretation; not uniquely forced.'],
            ['Generation structure', '[PARTIAL]', 'FTD derives 3 generations; individual top mass remains [OPEN].'],
            ['Top-quark identity', '[CLOSED NEGATIVE]', 'Registry [CLOSED NEGATIVE] quark identity: an inert + marker and imposed color label on a source-free A=2.5 vector wave template carry no fractional charge, quark mass, color gauge field, or confinement, so the top-quark identification is rejected.'],
        ],
    },
    's0-seed-beta-decay': {
        title: 'Beta decay demo (n \u2192 p + e\u207b + \u03bd\u0304, dynamic)',
        desc: 'Seeds a neutron triad (2 negative + 1 positive cluster) with weak_transmutation enabled. Over ticks the flux stress on one vertex can exceed WEAK_THRESHOLD = K_GENESIS, flipping polarity (\u2212 \u2192 +) \u2014 the electron marker and neutrino already exist at tick zero, so no daughter is created or emitted; this is not the n\u2192p W-mediated decay. Separately inject an electron and neutrino to represent the leptonic output. Demo only \u2014 does not simulate the full weak-current matrix element.',
        epistemic: [
            ['Configuration', '[SELECTION]', 'Neutron triad geometry + enabled toggles. Initial condition only.'],
            ['Dynamics', '[EMERGENT]', 'Weak transmutation is a genuine dynamical effect when the stress threshold is exceeded.'],
            ['Leptonic output', '[DEMO]', 'e\u207b + \u03bd\u0304 are preseeded, not produced dynamically. A coupled 4-body weak decay is not in the engine.'],
            ['Coefficients', '[IMPOSED]', 'WEAK_THRESHOLD and stress threshold are from electroweak theory.'],
            ['Beta-decay identity', '[CLOSED NEGATIVE]', 'Registry [CLOSED NEGATIVE] beta-decay identity: the electron and neutrino already exist at tick zero and no daughter creation or emission occurs, so no n-to-p W-mediated decay is simulated.'],
        ],
    },
    's0-seed-ee-annihilation': {
        title: 'Opposite-polarity collision \u2014 state removal, no photon pair',
        desc: 'An electron and positron seeded on opposing sides of the lattice, given initial velocities toward each other. On collision the engine\'s phase_movement logic flags opposite-sign contact and removes both states, redistributing the pre-existing field norm to the 6 face-neighbours. This is a deterministic native state-removal event, not e\u207a + e\u207b \u2192 \u03b3\u03b3: the wave momentum remains exactly zero, so no outgoing wave or photon pair is produced.',
        epistemic: [
            ['Configuration', '[SELECTION]', 'Initial positions + velocities chosen so collision happens within ~20 ticks.'],
            ['Collision-removal dynamics', '[EMERGENT]', 'The opposite-sign collision branch in phase_movement removes both states and spreads their pre-existing flux over the six-face shell.'],
            ['e+e- / two-photon final state', '[CLOSED NEGATIVE]', 'Registry [CLOSED NEGATIVE] e+e-/photon identity: the validation test records exactly zero wave momentum and no outgoing radiation, so no photon pair is created and the flux redistribution is not an annihilation-to-gamma-gamma channel.'],
        ],
    },

    // s0-seed-proton-candidate metadata removed 2026-04-28 (audit removal):
    // superseded by s0-seed-proton-l4 / s0-vacuum-proton.
    /* removed entry preserved below in a comment for reference only:
    's0-seed-proton-candidate': {
        title: 'Proton candidate (3-site positive cluster) \u2014 NOT "uud"',
        desc: 'Three s=+1 particles on an equilateral triangle at the lattice center with weak radial-outward flux dressing. The "u-u-d" story is NOT encoded: FTD has no color axis, no flavor label, no orientation-dependent quark identity. This scenario tests only whether a 3-body positive cluster persists under substrate dynamics.',
        epistemic: [
            ['3-site cluster configuration', '[SELECTION]', 'Consistent with baryon number 3. Triangle geometry is one choice among many \u2014 not uniquely forced.'],
            ['Name "proton"', '[IMPOSED]', 'A label on the cluster. Do NOT read color, flavor, or quark identity from the triangle vertices.'],
            ['Mass ratio \\(m_p/m_e\\)', '[STRONGLY MOTIVATED CONJECTURE]', '\\(m_p/m_e = N_\\mathrm{eff}/\\alpha + N_\\mathrm{base} N_\\mathrm{eff} + N_c = 1836.47\\) (174 ppm). Inherits FTD-0013/FTD-0016 status: [STRONGLY MOTIVATED CONJECTURE] per LEDGER, not [THEOREM]. (Reference-only — this metadata block is commented out; retagged for accuracy, audit P1-9, 2026-05-27.)'],
            ['LANDMINE', '[WARNING]', 'Do NOT interpret J_x-dominant flux as "red quark" or map vertices to u/d. The BCC\u2192SU(3) link is about the gluon propagator, not per-quark orientation.'],
        ],
    },
    */

    // ── Moore Seeds (geometric) ──────────────────────────────────────
    // Theory: THEOREM_MOORE_LAYER_DECOMPOSITION.md
    // C++ constructors: ftd::ctor::octahedron, cuboctahedron, stella_octangula, moore_cell

    's0-seed-octahedron': {
        title: 'Octahedron \u2014 Moore shell 1 (SC face-neighbors, 6 sites)',
        desc: 'The 6 face-sharing neighbors of the center voxel, forming a regular octahedron. This is Shell 1 of the Moore neighborhood decomposition at L2 distance 1, corresponding to the simple-cubic (SC) sublattice. In the Moore Layer Theorem, this shell maps to the U(1) gauge sector.',
        epistemic: [
            ['Geometry (6 sites at distance 1)', '[THEOREM]', 'The 6 face-neighbors of any site on a cubic lattice form a regular octahedron. This is geometric fact, not a model choice.'],
            ['U(1) sector identification', '[SELECTION]', 'Mapping shell 1 to U(1) follows from the Moore Layer Theorem decomposition. Structurally motivated, not uniquely proven.'],
        ],
    },
    's0-seed-cuboctahedron': {
        title: 'Cuboctahedron \u2014 Moore shell 2 (FCC edge-neighbors, 12 sites)',
        desc: 'The 12 edge-sharing neighbors at L2 distance \u221a2, forming a cuboctahedron. Shell 2 of the Moore neighborhood, corresponding to the face-centered-cubic (FCC) sublattice. In the Moore Layer Theorem, this shell maps to the SU(2) gauge sector.',
        epistemic: [
            ['Geometry (12 sites at distance \u221a2)', '[THEOREM]', 'The 12 edge-neighbors form a cuboctahedron \u2014 geometric fact of the cubic lattice.'],
            ['SU(2) sector identification', '[SELECTION]', 'Mapping shell 2 to SU(2) follows from the Moore Layer Theorem. Structurally consistent, not uniquely forced.'],
        ],
    },
    's0-seed-stella-octangula': {
        title: 'Stella octangula \u2014 Moore shell 3 (BCC corner-neighbors, 8 sites)',
        desc: 'The 8 corner-neighbors at L2 distance \u221a3, forming a stella octangula (two interpenetrating tetrahedra). Shell 3 of the Moore neighborhood, corresponding to the body-centered-cubic (BCC) sublattice. In the Moore Layer Theorem, this shell maps to the SU(3) gauge sector. The BCC eigenvalue produces both Watson\u2019s integral W\u2083 and the SU(3) gauge group via the triple-cosine product (DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md).',
        epistemic: [
            ['Geometry (8 sites at distance \u221a3)', '[THEOREM]', 'The 8 corner-neighbors form a stella octangula \u2014 geometric fact of the cubic lattice.'],
            ['SU(3) sector identification', '[SELECTION]', 'The BCC sublattice eigenvalue factorizes into a triple-cosine product (that factorization itself is [THEOREM]; see DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md). The identification of that product with the SM strong-force SU(3) gauge group is structurally consistent but not uniquely forced — matching the U(1)/SU(2) identifications in the sibling scenarios above (audit P0-16 correction, 2026-05-27).'],
        ],
    },
    's0-seed-moore-cell': {
        title: 'Moore cell \u2014 full 26-site neighborhood',
        desc: 'All 26 neighbors of a central voxel: the union of octahedron (6) + cuboctahedron (12) + stella octangula (8). This is the complete Moore neighborhood that defines local causality in FTD. The center voxel is shown as negative (anchor), all neighbors as positive.',
        epistemic: [
            ['26-site Moore neighborhood', '[AXIOM]', 'The 26-connected Moore neighborhood is postulated as the causal neighborhood in FTD (Postulate 4: local causality).'],
            ['6 + 12 + 8 = 26 decomposition', '[THEOREM]', 'The three shells are disjoint and exhaustive \u2014 this is the THEOREM_MOORE_LAYER_DECOMPOSITION.'],
        ],
    },
    's0-seed-moore-decomposition': {
        title: 'Moore decomposition — 3 shells visualized by polarity',
        desc: 'All three Moore shells shown simultaneously with alternating states so each shell is visually distinguishable: Shell 1 (octahedron, 6 sites) = positive, Shell 2 (cuboctahedron, 12 sites) = negative, Shell 3 (stella octangula, 8 sites) = positive, Center = negative. This lets you see the U(1) \u00d7 SU(2) \u00d7 SU(3) decomposition as three concentric geometric layers.',
        epistemic: [
            ['Polyhedral decomposition', '[THEOREM]', 'THEOREM_MOORE_LAYER_DECOMPOSITION: the 26-site Moore neighborhood decomposes uniquely into octahedron + cuboctahedron + stella octangula at distances 1, \u221a2, \u221a3.'],
            ['Gauge group mapping', '[SELECTION]', 'U(1) \u00d7 SU(2) \u00d7 SU(3) identification follows from the theorem but is a selection principle, not a uniqueness proof.'],
        ],
    },
    's0-seed-h2-bond-formation': {
        title: 'Two-nucleus Coulomb cohort \u2014 mobile pair lost, no bond',
        desc: 'Places two locked hydrogen-like source clusters close together with two shared mobile negative markers in the centre, under Poisson force and movement. The comparison has been run: the six locked source sites persist while both central mobile markers are removed by tick 64. No dynamic bond forms and no bond-energy observable is computed, so this is a prepared two-nucleus cohort, not covalent bond formation.',
        epistemic: [
            ['Configuration', '[SELECTION]', 'Initial cluster spacing and shared-marker placement are selected to probe bonding dynamics.'],
            ['Covalent bond formation', '[CLOSED NEGATIVE]', 'Registry [CLOSED NEGATIVE] H2 bond formation: under Poisson force and movement both central mobile markers are lost by tick 64 and no bond-energy observable is produced, so no stable covalent bond forms.'],
        ],
    },
    's0-seed-spark-of-life': {
        title: 'Patterned genesis burst — six events, no life or autocatalysis',
        desc: 'Seeds a locked mineral-pore ring, simple precursor charge pairs, an unlocked central catalytic triad, and a deterministic six-axis flux spark. The run has been measured: exactly six genesis events fire by tick 8 (counts 27/33/33/33 at ticks 1/8/16/32) and then the pattern stops, with no further growth, replication, chemistry, metabolism, heredity, or autocatalytic observable. This is a finite thresholded genesis response, not abiogenesis or life.',
        epistemic: [
            ['Initial condition', '[SELECTION]', 'Mineral pore, precursor pairs, central triad, and daughter pockets are selected to stage a plausible threshold-crossing setup.'],
            ['Threshold manifestation', '[EMERGENT]', 'The six manifested voxels arise from the existing genesis dynamics once local flux density exceeds K_GENESIS.'],
            ['Life / autocatalysis identity', '[CLOSED NEGATIVE]', 'Registry [CLOSED NEGATIVE] life/autocatalysis identity: after the six genesis events there is zero further growth, replication, or autocatalytic observable, so no life-like or autocatalytic behavior is exhibited.'],
            ['Real abiogenesis, biochemistry, replication', '[NOT SHOWN]', 'No molecular chemistry, metabolism, genetic replication, or evolutionary selection is implemented in this Scale 0 scenario.'],
        ],
    },
    's0-seed-gravitational-lensing': {
        title: 'Radial background + packet — lensing null',
        desc: 'Places a mass-like radial background beside an off-axis transverse flux packet and compares its trajectory against a matched no-mass control. The comparison has been run: the full profile equals the independently evolved background plus packet to below 1e-12, and the radial background induces exactly no additional deflection of the packet. The frozen engine has no gravity-to-wave vertex, so no native lensing occurs.',
        epistemic: [
            ['Radial background', '[SELECTION]', 'A lattice-scale mass-like radial inflow profile stands in for the deflecting mass.'],
            ['Gravity-to-wave lensing', '[CLOSED NEGATIVE]', 'Registry [CLOSED NEGATIVE] native gravity-to-wave lensing: against the matched no-mass control the background produces exactly no additional trajectory deflection, because no gravity-to-wave vertex exists in the frozen engine.'],
        ],
    },
    's0-seed-quark-gluon-plasma': {
        title: 'Thermal transport/outflow cohort (QGP identity failed)',
        desc: 'Eight neutral color-labelled markers are seeded with high velocities in a tight central region under an elevated Langevin bath. With the color force off, the markers are transported through the bath and deplete via the open particle boundary (counts 8/8/8/1 at ticks 8/16/32/64; the journal records movement events and zero annihilations). This is finite thermal transport, not a deconfined quark-gluon plasma: the depletion is boundary outflow, not deconfinement.',
        epistemic: [
            ['Configuration', '[SELECTION]', 'High initial kinetic energy and a warm Langevin bath represent a QGP-like initial state.'],
            ['Substrate transport / thermalization', '[EMERGENT]', 'Langevin-driven transport of the markers through the bath is genuine substrate dynamics.'],
            ['QGP / deconfinement identity', '[CLOSED NEGATIVE]', 'Registry [CLOSED NEGATIVE] QGP/deconfinement identity: with color force off and zero annihilation events, the site depletion is open particle-boundary outflow, so no deconfinement transition is demonstrated.'],
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
