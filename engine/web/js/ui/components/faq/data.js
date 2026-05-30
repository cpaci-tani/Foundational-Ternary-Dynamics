/**
 * FAQ sidebar data — 16 canonical hard problems framed through the FTD lens.
 * See docs/superpowers/specs/2026-04-18-faq-panel-design.md.
 *
 * Tag vocabulary for ftdAngle bullets:
 *   THEOREM     rigorously proven from FTD axioms
 *   SELECTION   argued from consistency, not uniquely proven
 *   PARAMETRIC  SM formula with FTD numbers inserted
 *   CONJECTURE  proposed interpretation requiring validation
 *   OPEN        framed by FTD but unresolved
 */

// Expanded 2026-05-27 (audit P1-16) to include 'CLOSED NEGATIVE' and
// 'PARTIAL' so FAQ entries can mark retracted identifications honestly
// (e.g. FTD-0131 G_N closed-negative status) without forcing a less
// accurate tag. Allowed set mirrors CLAUDE.md § Epistemic Tags.
export const FAQ_TAGS = Object.freeze([
    'THEOREM',
    'SELECTION',
    'PARAMETRIC',
    'CONJECTURE',
    'OPEN',
    'CLOSED NEGATIVE',
    'PARTIAL',
]);

export const FAQ_SECTIONS = Object.freeze([
    {
        id: 'physics',
        title: 'Physics',
        description: 'Twelve hard problems of modern physics, framed through the FTD lens.',
        entries: [
            {
                id: 'hard-problem-reference frame context',
                question: 'Why is there subjective experience at all?',
                shortQuestion: 'The hard problem of reference frame context',
                problem: [
                    'Chalmers\' "hard problem": even if every functional brain process were fully modeled, the question "why is there something it is like to be that system?" remains untouched. No combination of information-processing steps seems, on its face, to entail experience.',
                ],
                mainstreamStruggle: [
                    'Integrated Information Theory, Global Workspace, Higher-Order-Thought, and related frameworks characterize neural correlates of reference frame context. They are models of when experience occurs, not explanations of why there is experience at all. The explanatory gap remains open.',
                ],
                ftdAngle: [
                    { tag: 'SELECTION', text: 'Reference frame structure enters the formalism as the phase angle \\(\\theta_C\\) of the master-quadratic extension. The structural fraction \\(\\cos^2(\\theta_C) = G^{\\ast}/8\\) follows from the ternary-state algebra. (Per REF_REFERENCE_FRAME_VOCABULARY 2026-05-01, "reference frame structure" denotes the structural closure; "reference frame context" is the colloquial pointer.)' },
                    { tag: 'CONJECTURE', text: 'The sLoop self-reference ring is proposed as the structural locus where reference closes on itself — a candidate substrate for frame-relative frame dynamics, not an explanation of qualia.' },
                    { tag: 'OPEN', text: 'FTD offers a geometry for where reference frame structure could live in the formalism; it does not derive qualia (subjective experience) from that geometry.' },
                ],
                stillOpen: [
                    'No operational test distinguishes the "FTD reference frame structure phase" from a purely functional account.',
                    'The relationship between the sLoop structure and the subjective quality of experience is asserted, not derived.',
                ],
                theoryRefs: [
                    'docs/theory/01_reference/REF_REFERENCE_FRAME_VOCABULARY.md',
                ],
            },
            {
                id: 'measurement-problem',
                question: 'Why does the wave function appear to collapse when measured?',
                shortQuestion: 'Measurement / collapse',
                problem: [
                    'The Schrodinger equation is linear and deterministic, yet every observation appears to yield a definite classical outcome. No formal collapse mechanism is contained in the equation itself. This is the "measurement problem" at the heart of foundational quantum mechanics.',
                ],
                mainstreamStruggle: [
                    'Mainstream interpretations each pay a price: Copenhagen posits an unexplained classical/quantum split; Many-Worlds keeps linearity but multiplies ontology; GRW and Penrose OR add nonlinear physics without empirical support; QBism relocates the question to epistemic agents. None of these is experimentally favoured over the others.',
                ],
                ftdAngle: [
                    { tag: 'CONJECTURE', text: 'Measurement is not a separate physical process: it is the transition from flux-field dispositional structure to state-field actualization at a discrete tick. What looks like collapse is just the manifestation step of the two-layer ontology.' },
                    { tag: 'SELECTION', text: 'The Born rule (\\(P \\propto |J|^2\\)) emerges as a statistical consequence of lattice dynamics; benchmarked in engine/tests/benchmark_engine_theory.cpp with a 10x lattice bias that is currently unaccounted for.' },
                    { tag: 'OPEN', text: 'The precise dynamical trigger for manifestation — what determines the "when" of a tick for a given voxel — is an axiom, not a derivation.' },
                ],
                stillOpen: [
                    'FTD reframes collapse as manifestation but does not derive the specific measurement-outcome probabilities from a deeper principle.',
                    'No unambiguous experimental discriminator separates "FTD manifestation" from textbook Copenhagen collapse.',
                ],
                theoryRefs: [
                    'docs/theory/03_derivations/DERIV_QM_FROM_LATTICE.md',
                    'docs/theory/03_derivations/DERIV_SINGLET_FROM_VOID_EVENT.md',
                ],
            },
            {
                id: 'arrow-of-time',
                question: 'Why does time appear to have a direction?',
                shortQuestion: 'Arrow of time',
                problem: [
                    'Fundamental physical laws are almost entirely time-symmetric. Yet macroscopic phenomena — memory, causation, entropy growth, the expansion of the universe — all exhibit a strong preferred direction. Where does this asymmetry come from if the underlying equations do not contain it?',
                ],
                mainstreamStruggle: [
                    'The standard answer points to a low-entropy initial state ("Past Hypothesis"), but this relocates the question rather than resolves it: why was the early universe in such a low-entropy state in the first place? No first-principles derivation exists.',
                ],
                ftdAngle: [
                    { tag: 'SELECTION', text: 'The lattice tick is intrinsically ordered: state at t+1 is computed from state at t via a non-invertible manifestation step. Time-asymmetry is built into the tick cycle, not derived from initial conditions.' },
                    { tag: 'CONJECTURE', text: 'The distinction between the commutative Euler reflection product (yielding \\(\\pi\\), time-symmetric) and the non-commutative ratio (yielding \\(G^{\\ast}\\), time-asymmetric) is proposed as the formal root of the arrow.' },
                ],
                stillOpen: [
                    'FTD builds asymmetry into its update rule at the axiomatic level. It does not show that any strictly time-symmetric axiomatisation is impossible.',
                    'The connection between lattice time and observed thermodynamic entropy is stated but not quantitatively derived.',
                ],
                theoryRefs: [
                    'docs/papers/src/PAPER_RATIO_AND_THE_ARROW.tex',
                ],
            },
            {
                id: 'matter-antimatter-asymmetry',
                question: 'Why is there any matter at all?',
                shortQuestion: 'Matter-antimatter asymmetry',
                problem: [
                    'Standard cosmological models predict that the Big Bang should have produced equal amounts of matter and antimatter, which would have annihilated leaving only radiation. Yet we observe a universe dominated by matter. This is the baryogenesis puzzle.',
                ],
                mainstreamStruggle: [
                    'Sakharov\'s three conditions (baryon number violation, C and CP violation, out-of-equilibrium) are necessary but not sufficient. The observed CP violation in the Standard Model is too small by many orders of magnitude to account for the observed asymmetry. New physics is required but not empirically found.',
                ],
                ftdAngle: [
                    { tag: 'SELECTION', text: 'The Moore Layer decomposition produces a +/− structural symmetry at the postulate level (s ∈ {−1, 0, +1}): for every occupied +1 voxel there is a structurally equivalent −1 voxel. This is a structural symmetry of the state space, not a derivation of why our observed sector lacks annihilation; that step is a separate argument.' },
                    { tag: 'CONJECTURE', text: 'The observed "matter dominance" is reinterpreted as an observer-selection effect within a co-located +/− partition — we inhabit the +1 sublattice and measure from it.' },
                ],
                stillOpen: [
                    'FTD reframes the puzzle topologically but does not produce a quantitative matter-antimatter ratio to compare against the observed 10^-10 baryon-to-photon asymmetry.',
                    'The observer-selection framing is untested.',
                ],
                theoryRefs: [
                    'docs/theory/08_structural/THEOREM_MOORE_LAYER_DECOMPOSITION.md',
                ],
            },
            {
                id: 'dark-matter',
                question: 'What is dark matter?',
                shortQuestion: 'Dark matter',
                problem: [
                    'Galactic rotation curves, gravitational lensing, and CMB observations all point to a gravitating component that interacts weakly if at all with light. Decades of direct-detection searches for WIMPs, axions, and other candidates have produced null results.',
                ],
                mainstreamStruggle: [
                    'Mainstream cosmology adopts dark matter as a parametric addition — \\(\\Lambda\\)-CDM works phenomenologically but the underlying particle is unidentified. MOND and modified gravity theories capture some rotation curves but not cluster lensing. Nothing is satisfactory.',
                ],
                ftdAngle: [
                    { tag: 'SELECTION', text: 'The Moore Layer decomposition of the 27-site neighborhood identifies 17 states that, on the proposed identification, do not participate in the electromagnetic sector. Under uniform voxel-counting the structural ratio is \\(17/10 \\approx 1.70\\) (DM:baryon); under uniform fraction it is \\(17/27 \\approx 63\\%\\). Neither matches Planck 2018\'s \\(\\Omega_\\mathrm{DM}/\\Omega_b \\approx 5.37\\).' },
                    { tag: 'SELECTION', text: 'A natural weighting candidate ("W5") gives a substantially sharper match (2026-05-27 analysis, post-hoc): weight the 12 cuboctahedral sites by \\(N_\\mathrm{base} = 4\\) (reflecting the cuboct ↔ fermion identification — 12 sites = 3 generations × 4 fermions × internal multiplicity 4). Result: weighted DM = \\(1 + 12 \\cdot 4 + 4 = 53\\); BARYON = \\(6 + 4 = 10\\); ratio = \\(53/10 = 5.30\\) (1.4% from Planck), and \\(\\Omega_\\mathrm{DM}/\\Omega_m = 53/63 = 0.841\\) (0.2% from Planck). Of 9 natural FTD weighting candidates tested, only W5 lands within Planck\'s 1σ band. Status is [SELECTION] — structurally motivated but POST-HOC; pre-registered confirmation against an independent observable is the next step.' },
                    { tag: 'CONJECTURE', text: 'Dark matter is proposed to be the unexcited subset of the same lattice structure that produces visible matter — not a separate substance.' },
                ],
                stillOpen: [
                    'The W5 cuboctahedron × \\(N_\\mathrm{base}\\) weighting was found post-hoc (no pre-registration); a confirmation test against an independent cosmological observable (CMB acoustic-peak position, BBN \\(^4\\)He mass fraction, or matter-radiation equality redshift) under the same weighting is required before promoting it beyond [SELECTION].',
                    '"Why N_base and not N_eff" lacks a first-principles derivation — N_eff = 13 weighting gives 16.1 (catastrophically off), so the choice of N_base = 4 specifically is not yet derived from FTD axioms.',
                    'FTD does not currently explain galactic rotation-curve shapes or halo profiles quantitatively.',
                    'No direct-detection prediction distinguishes FTD dark matter from a sterile-neutrino or other weakly-interacting candidate.',
                ],
                theoryRefs: [
                    'docs/theory/08_structural/THEOREM_MOORE_LAYER_DECOMPOSITION.md',
                    'docs/theory/10_eft_program/EXPLR_DM_BARYON_W5_WEIGHTING.md',
                ],
            },
            {
                id: 'dark-energy',
                question: 'What drives accelerating cosmic expansion?',
                shortQuestion: 'Dark energy',
                problem: [
                    'Type Ia supernova observations show that the expansion of the universe is accelerating. The standard interpretation adds a cosmological constant \\(\\Lambda\\) whose observed value is \\(10^{120}\\) smaller than naive vacuum-energy estimates. This is the "worst prediction in physics".',
                ],
                mainstreamStruggle: [
                    'Quintessence, modified-gravity, and anthropic-landscape approaches each introduce additional parameters to accommodate the observed \\(\\Lambda\\) without explaining its size. The discrepancy between QFT vacuum energy and observation is essentially unresolved.',
                ],
                ftdAngle: [
                    { tag: 'PARAMETRIC', text: 'In FTD cosmology \\(\\Omega_\\Lambda = 2/3\\) and \\(\\Omega_m = 1/3\\) at the ontological level — consistent with observations to within a few percent.' },
                    { tag: 'CONJECTURE', text: 'Dark energy is proposed to be the net effect of flux-field relaxation between manifestation events: a background dispositional stress, not a vacuum-energy density in the QFT sense.' },
                ],
                stillOpen: [
                    'No prediction of the \\(\\Lambda\\) scale in natural units; the \\(2/3\\) ratio is ontological, not a prediction of magnitude.',
                    'No quantitative account of why the observed value is so small compared to any natural cutoff.',
                ],
                theoryRefs: [
                    'docs/theory/03_derivations/DERIV_STELLAR_LIFECYCLE_LATTICE.md',
                ],
            },
            {
                id: 'fine-tuning',
                question: 'Why do fundamental constants have the values they do?',
                shortQuestion: 'Fine-tuning',
                problem: [
                    'Many parameters of the Standard Model and cosmology appear fine-tuned: the cosmological constant, the Higgs mass, the ratio of electroweak to Planck scales, the near-flatness of the universe. Small changes to these constants would preclude complex chemistry or life.',
                ],
                mainstreamStruggle: [
                    'The two main responses are the multiverse/anthropic landscape (which makes no sharp predictions) and the search for a unique underlying theory (which has not been found). Neither is empirically decisive.',
                ],
                ftdAngle: [
                    { tag: 'CONJECTURE', text: 'The master quadratic \\(x^2 - 16G^{*2}x + 16G^{*3} = 0\\) is a [THEOREM]; its larger root \\(x_+ \\approx 137.036\\) matches \\(1/\\alpha\\) to 1.26 ppm at tree level. The physical identification \\(x_+ = 1/\\alpha\\) is a [STRONGLY MOTIVATED CONJECTURE] supported by Bayes-factor and structural-uniqueness scans (\\(\\sim 4\\times 10^5\\):1) but no derivation chain from axioms to \\(\\alpha\\) currently exists.' },
                    { tag: 'SELECTION', text: 'The Moore Layer decomposition motivates gauge-group structure, generation count, and a dark-state count from the neighborhood geometry. The argument is consistency-based, not a uniqueness theorem.' },
                    { tag: 'PARAMETRIC', text: 'Masses like \\(m_e\\) and \\(m_H\\) are computed by inserting FTD-derived \\(\\alpha\\) and integers into Standard-Model formulas; agreements are parametric, not first-principles predictions.' },
                ],
                stillOpen: [
                    'Not every Standard-Model parameter is derived from FTD structure; the CKM matrix and neutrino masses are untouched.',
                    'The Planck scale itself is imposed as a unit, not explained.',
                ],
                theoryRefs: [
                    'docs/theory/03_derivations/DERIV_ALPHA_FROM_PHASE_STRUCTURE.md',
                    'docs/theory/08_structural/THEOREM_MOORE_LAYER_DECOMPOSITION.md',
                ],
            },
            {
                id: 'hierarchy-problem',
                question: 'Why is gravity so absurdly weak?',
                shortQuestion: 'Hierarchy problem',
                problem: [
                    'The ratio of the gravitational to electromagnetic force between two protons is about 10^-37. Equivalently, the Higgs mass is 17 orders of magnitude smaller than the Planck mass. Without extreme fine-tuning, quantum corrections should drive the Higgs mass up to the Planck scale.',
                ],
                mainstreamStruggle: [
                    'Supersymmetry was the leading proposed solution but has not been observed at the LHC at accessible scales. Extra-dimensional, compositeness, and anthropic approaches each introduce new structure without decisive empirical support.',
                ],
                ftdAngle: [
                    { tag: 'CLOSED NEGATIVE', text: 'The reading \\(G_N = 1/(b_3 + N_c)^2 = 0.01\\) as an identification with the physical Newton constant is RETIRED per FTD-0131 (SPEC_DOCTRINE_LEDGER §10): under any natural calibration it is off by ~10^20 to 10^43 from the measured value.' },
                    { tag: 'PARTIAL', text: 'The surviving claim is the substrate-derived gravitational fine-structure ratio for one electron: \\(\\alpha_G(e,e) = (m_e/m_P)^2 \\approx 1.745\\times10^{-45}\\) (predicted, 0.38% match to measured 1.752×10^-45), conditional on one flagged interpretive step (clock hypothesis). See DERIV_NEWTON_FROM_SUBSTRATE.md and FTD-0131.' },
                    { tag: 'CONJECTURE', text: 'Gravity\'s weakness is reinterpreted as a long-range relaxation of the flux field across many manifestation events — weakness as structural dilution, not a mass-scale hierarchy.' },
                ],
                stillOpen: [
                    'FTD-0131 closes the framework-integer reading of G_N as [CLOSED NEGATIVE]; the substrate α_G(e,e) match is partial (one flagged step).',
                    'The connection between the 17-digit hierarchy and the FTD cell topology remains qualitative.',
                ],
                theoryRefs: [
                    'docs/theory/03_derivations/DERIV_NEWTON_FROM_SUBSTRATE.md',
                    'docs/theory/01_reference/SPEC_DOCTRINE_LEDGER.md',
                ],
            },
            {
                id: 'quantum-gravity',
                question: 'How do quantum mechanics and general relativity fit together?',
                shortQuestion: 'Quantum gravity',
                problem: [
                    'QM and GR are each extraordinarily successful in their domain but use incompatible mathematical frameworks. Attempts to quantize GR directly produce non-renormalisable infinities. A consistent theory that reduces to both in the right limits has not been found.',
                ],
                mainstreamStruggle: [
                    'String theory requires supersymmetry and extra dimensions with no direct experimental test. Loop quantum gravity is background-independent but has not reproduced low-energy QFT cleanly. Causal dynamical triangulations and asymptotic safety each capture pieces. None is decisive.',
                ],
                ftdAngle: [
                    { tag: 'SELECTION', text: 'Both QM and GR are framed as emergent from the same lattice dynamics: QM as the statistics of manifestation events on short scales, GR as the coarse-grained flux-field geometry on long scales.' },
                    { tag: 'SELECTION', text: 'A Deser-iterative bootstrap from linear spin-2 flux modes to the nonlinear Einstein equations is presented; engine benchmarks (test_einstein_equations.cpp) report a 0.004% time-dilation match. The full derivation chain currently sits at [SELECTION] / [PARTIAL DERIVED] post-2026-04-19 reframe.' },
                    { tag: 'CONJECTURE', text: 'The UV divergences of naive quantum gravity are absent because the lattice is discrete by postulate — there is no continuum to renormalise over.' },
                ],
                stillOpen: [
                    'FTD demonstrates that QM and GR coexist as emergent limits but does not yet quantitatively reproduce all gravitational scattering amplitudes.',
                    'The specific continuum limit that recovers QFT perturbation theory from FTD is a work in progress.',
                ],
                theoryRefs: [
                    'docs/theory/03_derivations/DERIV_CONTINUUM_LIMIT_QED_EQUIVALENCE.md',
                ],
            },
            {
                id: 'black-hole-information',
                question: 'Where does information go when a black hole evaporates?',
                shortQuestion: 'Black-hole information',
                problem: [
                    'Hawking radiation appears thermal, suggesting information about what fell into a black hole is lost. But unitarity of quantum mechanics forbids genuine information loss. One of these two very well-tested principles must yield.',
                ],
                mainstreamStruggle: [
                    'Firewalls, fuzzballs, islands formulas, and ER=EPR are active research directions. No single resolution commands consensus; experimental input from real black holes is unavailable.',
                ],
                ftdAngle: [
                    { tag: 'SELECTION', text: 'A black hole is the dense-manifestation region where flux-field gradients reach the confinement scale. Information is carried by the flux structure outside the horizon and by manifestation events that propagate outward via the latency mechanism.' },
                    { tag: 'CONJECTURE', text: 'Hawking radiation in FTD is the gradual dissipation of flux-field structure — not strictly thermal, and therefore carries information in its correlations.' },
                ],
                stillOpen: [
                    'FTD offers a qualitative route to unitarity-preserving evaporation but no quantitative entanglement-entropy curve.',
                    'No prediction that distinguishes FTD corrections from Page-curve-style unitary evaporation models.',
                ],
                theoryRefs: [
                    'docs/theory/03_derivations/DERIV_STELLAR_LIFECYCLE_LATTICE.md',
                ],
            },
            {
                id: 'bell-nonlocality',
                question: 'What does Bell / EPR nonlocality actually mean?',
                shortQuestion: 'Bell / nonlocality',
                problem: [
                    'Bell inequalities are violated experimentally. Any local hidden-variable theory is ruled out. Yet quantum correlations do not permit faster-than-light signalling. What is the ontological content of quantum nonlocality?',
                ],
                mainstreamStruggle: [
                    'Interpretations split sharply: Copenhagen drops counterfactual definiteness, Bohmian mechanics accepts nonlocal hidden variables at the cost of preferred frame, Many-Worlds retains locality at the cost of branching ontology. The physics is clear; the metaphysics is not.',
                ],
                ftdAngle: [
                    { tag: 'SELECTION', text: 'Bell violation \\(S = 2\\sqrt{2}\\) is framed in FTD as a property of singlet states produced by void events — entangled pairs share a common manifestation history on the lattice.' },
                    { tag: 'SELECTION', text: 'Tsirelson\'s bound is argued to emerge from the lattice emergence of quantum mechanics via a five-lemma chain in DERIV_SINGLET_FROM_VOID_EVENT.md; the substrate-to-aggregate transition (single-event \\(S \\le 2\\) to ensemble \\(S = 2\\sqrt{2}\\)) remains [OPEN].' },
                ],
                stillOpen: [
                    'The claim that void events produce singlet correlations depends on assumptions about the lattice history that are not uniquely fixed.',
                    'FTD does not privilege one metaphysical interpretation of nonlocality over another — it sharpens the question by replacing "instantaneous collapse" with "shared manifestation history".',
                ],
                theoryRefs: [
                    'docs/theory/03_derivations/DERIV_SINGLET_FROM_VOID_EVENT.md',
                ],
            },
            {
                id: 'three-dimensions',
                question: 'Why does space have three dimensions?',
                shortQuestion: 'Why D=3',
                problem: [
                    'Most fundamental theories treat spatial dimensionality as an input. Special arguments (stable orbits, Maxwell propagation, knotted field theory) make 3D look privileged, but none is decisive.',
                ],
                mainstreamStruggle: [
                    'In string theory the dimension count is fixed by internal consistency (10 or 11) with 6 or 7 compactified. This explains the "mathematical" dimension but not why the large-scale physical dimension we observe is 3.',
                ],
                ftdAngle: [
                    { tag: 'SELECTION', text: 'D = 3 is not axiomatic in FTD: it is selected via a consistency argument that D=3 lattices admit the ternary-state symmetry along with the Moore-neighborhood geometry the rest of the framework uses. The argument constrains but does not uniquely force D=3.' },
                    { tag: 'SELECTION', text: 'Alternative dimensions break the master-quadratic structure (verified numerically in scripts/proofs/proof_master_verification.py); this rules out the specific D≠3 cases tested but does not constitute a uniqueness theorem over all alternative ontologies.' },
                ],
                stillOpen: [
                    'The selection argument constrains but does not exclude higher-dimensional ontologies that quotient down to D=3.',
                    'No experimental discriminator between "D=3 is fundamental" and "D=3 is an emergent large-scale approximation".',
                ],
                theoryRefs: [
                    'docs/theory/02_foundations/FOUND_DIMENSIONAL_COUNTING.md',
                ],
            },
        ],
    },
    {
        id: 'foundations',
        title: 'Foundations',
        description: 'Four foundational questions about existence, time, and the shape of reality.',
        entries: [
            {
                id: 'why-anything-exists',
                question: 'Why does anything exist at all?',
                shortQuestion: 'Why something rather than nothing',
                problem: [
                    'Leibniz\'s question: why is there something rather than nothing? Any physical theory that starts from existing objects (fields, particles, spacetime) presupposes the very thing it was supposed to explain.',
                ],
                mainstreamStruggle: [
                    'Modern physics typically brackets this question as outside the domain of empirical science. "Quantum fluctuations produced the universe" still begs for the quantum vacuum itself to exist. No mainstream framework derives existence from a principle more primitive than existence.',
                ],
                ftdAngle: [
                    { tag: 'SELECTION', text: 'FTD starts from three primitive states {-1, 0, +1} with no prior structure — a minimal asymmetry. The ternary choice itself is argued (not proven) to be the smallest self-consistent ontology supporting nontrivial dynamics.' },
                    { tag: 'CONJECTURE', text: 'The "void" state 0 is interpreted not as absence but as balanced potential — the dispositional ground from which manifestation emerges. Existence is then the recurrence of imbalance.' },
                    { tag: 'OPEN', text: 'FTD moves the question one step: why the ternary structure rather than a unary or binary one? The selection argument constrains but does not eliminate the choice.' },
                ],
                stillOpen: [
                    'The ternary postulate is primitive. FTD explains what follows from it, not why it holds.',
                    'No meta-theory distinguishes "FTD is the ontology" from "FTD is one ontology among many possible minimal ones".',
                ],
                theoryRefs: [
                    'docs/theory/02_foundations/FOUND_ONTOLOGICAL_GENESIS.md',
                ],
            },
            {
                id: 'what-is-time',
                question: 'What is time — a parameter, an emergent property, or an illusion?',
                shortQuestion: 'What is time',
                problem: [
                    'General relativity treats time as a coordinate woven into spacetime. Thermodynamics treats it as the axis along which entropy grows. Quantum mechanics treats it as an external evolution parameter. These three usages are not obviously compatible.',
                ],
                mainstreamStruggle: [
                    'Attempts to quantize time (Page-Wootters, Wheeler-DeWitt) remove the external parameter at the cost of making evolution relational and hard to interpret. Block-universe and presentist views both have problems explaining the apparent flow of time.',
                ],
                ftdAngle: [
                    { tag: 'SELECTION', text: 'Time in FTD is the discrete tick index of lattice updates. It is neither an external parameter nor a dimension of a pre-existing manifold — it is the order of manifestation events.' },
                    { tag: 'CONJECTURE', text: 'The continuous time of classical and quantum physics is an emergent coarse-graining of tick counts over many manifestation events.' },
                ],
                stillOpen: [
                    'The relationship between the lattice tick and the proper time of a relativistic observer is sketched but not formally derived in general.',
                    'No operational discriminator between "time is fundamentally discrete" and "time is effectively discrete at some unobservably small scale".',
                ],
                theoryRefs: [
                    'docs/theory/02_foundations/FOUND_LATTICE_PHYSICS_INTUITIONS.md',
                ],
            },
            {
                id: 'singularities',
                question: 'What replaces singularities where GR and QM both break?',
                shortQuestion: 'Singularities',
                problem: [
                    'General relativity predicts its own breakdown: curvature diverges at the center of black holes and at the Big Bang. At those points the theory ceases to describe anything. A physical theory that replaces singularities with well-defined structure has not been established.',
                ],
                mainstreamStruggle: [
                    'Loop quantum cosmology, string-theoretic fuzzballs, and effective-field-theory regularisations each offer candidate replacements. None has experimental support or decisive theoretical consensus.',
                ],
                ftdAngle: [
                    { tag: 'SELECTION', text: 'Because the lattice is discrete by axiom, curvature and density cannot diverge. A "singularity" in the continuum limit corresponds to a voxel whose flux gradient has reached the confinement scale — a finite, well-defined state.' },
                    { tag: 'CONJECTURE', text: 'The Big Bang is proposed as a phase of maximal flux-field structure that gradually manifests outward; it is a boundary in the lattice\'s manifestation history, not a point of infinite density.' },
                ],
                stillOpen: [
                    'FTD replaces "singularity" with "saturated voxel" but has not yet produced quantitative predictions for observable near-horizon or early-universe signatures.',
                    'The claim that no lattice configuration corresponds to an ill-defined limit is structural; no theorem establishes this across all possible dynamics.',
                ],
                theoryRefs: [
                    'docs/theory/03_derivations/DERIV_STELLAR_LIFECYCLE_LATTICE.md',
                ],
            },
            {
                id: 'discreteness',
                question: 'Why does reality appear quantized — why discreteness rather than continuum?',
                shortQuestion: 'Why quantization',
                problem: [
                    'Physics is full of discrete structures: quantized angular momentum, particle species, charges, spectra, energy levels. The underlying mathematical machinery (QFT) is built on a continuum yet repeatedly delivers integer-valued answers. Why?',
                ],
                mainstreamStruggle: [
                    'In mainstream physics discreteness is a consequence of eigenvalue problems on continuous operators. That is a technique, not an explanation: it answers "where do these integers come from in this formalism" but not "why is reality integer-valued in the first place".',
                ],
                ftdAngle: [
                    { tag: 'SELECTION', text: 'Discreteness is primitive in FTD: the lattice and the ternary states are discrete by postulate. Integer-valued observables in the continuum limit are not a mystery — they are the original structure surviving coarse-graining.' },
                    { tag: 'SELECTION', text: 'Specific integer ratios (3 generations, \\(N_c = 3\\) colors, \\(17/27\\) structural dark-state fraction, 3 spatial dimensions) are read off the Moore Layer decomposition. The decomposition itself is structural; whether the framework uniquely selects each ratio (vs admits other decompositions) is a [SELECTION] argument, and the cosmological identification of \\(17/27\\) with \\(\\Omega_\\mathrm{DM}/\\Omega_m\\) is rejected by Planck 2018 (see dark-matter entry).' },
                ],
                stillOpen: [
                    'The postulate of discreteness is primitive; FTD does not argue that a strictly continuous ontology is impossible.',
                    'The experimental lower bound on any putative lattice spacing is well below the Planck scale, meaning discreteness is presently a philosophical rather than empirical commitment.',
                ],
                theoryRefs: [
                    'docs/theory/08_structural/THEOREM_MOORE_LAYER_DECOMPOSITION.md',
                ],
            },
        ],
    },
]);

function _validate(sections) {
    const required = ['problem', 'mainstreamStruggle', 'ftdAngle', 'stillOpen'];
    for (const section of sections) {
        if (!Array.isArray(section.entries) || section.entries.length === 0) {
            throw new Error(`FAQ section '${section.id}' has no entries`);
        }
        for (const entry of section.entries) {
            for (const field of required) {
                const value = entry[field];
                if (!Array.isArray(value) || value.length === 0) {
                    throw new Error(`FAQ entry '${entry.id}' missing non-empty '${field}' array`);
                }
            }
            for (let i = 0; i < entry.ftdAngle.length; i++) {
                const bullet = entry.ftdAngle[i];
                if (!bullet || typeof bullet !== 'object') {
                    throw new Error(`FAQ entry '${entry.id}' ftdAngle[${i}] is not an object`);
                }
                if (!FAQ_TAGS.includes(bullet.tag)) {
                    throw new Error(`FAQ entry '${entry.id}' ftdAngle[${i}] tag '${bullet.tag}' not in ${FAQ_TAGS.join(',')}`);
                }
                if (typeof bullet.text !== 'string' || !bullet.text.trim()) {
                    throw new Error(`FAQ entry '${entry.id}' ftdAngle[${i}] missing text`);
                }
            }
        }
    }
}
_validate(FAQ_SECTIONS);

export function getFaqSections() {
    return FAQ_SECTIONS;
}
