# AUDIT — Corpus Consolidation Disposition Table

**Tag:** `[AUDIT]` — planning artifact (not a theory claim)
**Date:** 2026-06-02
**Purpose:** Phase 1a of the FTD Construction Monograph + Corpus Consolidation plan
(`docs/superpowers/plans/2026-06-02-ftd-construction-monograph.md`, Task 2).
Enumerates every active `docs/theory/` document (excluding archive subdirs), classifies
each into exactly one disposition, and identifies the minimal navigation-layer changes
required when the owner-approved archive moves are executed.

**Hard constraints:** No files were moved, renamed, deleted, or edited. No epistemic
tags were changed. This is a READ-ONLY audit producing a single new file.

**Owner gate:** The ARCHIVE-proposed set requires owner approval before any `git mv`
in Phase 1b. This document is the input to that gate.

---

## §1 — Inventory count

Active `.md` files under `docs/theory/` **excluding** all `**/archive/**` paths:
**421 documents** across 10 clusters + 3 top-level navigation files.

| Cluster | Count |
|---|---|
| `01_reference/` | 27 |
| `02_foundations/` | 31 |
| `03_derivations/` | 62 (top + 4 sub-clusters) |
| `04_coupling/` | 14 |
| `05_particles/` | 12 |
| `06_reference_frames_and_measurement/` | 13 |
| `07_assessment/` | 58 (incl. `audits/`, `campaigns/`, `core_ledgers/`) |
| `08_structural/` | 18 |
| `09_mathematical/` | 56 (incl. `algebra/`, `fqcr_program/`, `general_math/`, `number_theory/`) |
| `10_eft_program/` | 57 (incl. all sub-clusters) |
| Top-level (`META_INDEX`, `META_STRUCTURE`, `STRATEGY_PAPER_SPLIT`) | 3 |
| **Total** | **421** |

Note: the 421 count excludes the 88 files already in `archive/` subdirs.

---

## §2 — Disposition categories

| Code | Meaning |
|---|---|
| `KEEP-canonical` | A canonical reference, tracker, or spec; the monograph cites it as an authority |
| `KEEP-supporting` | An active derivation, foundation, or audit that the monograph relies on or that provides provenance for claims made in it |
| `ARCHIVE-absorbed` | A session synthesis the monograph explicitly replaces; move-don't-erase |
| `ARCHIVE-superseded` | A draft, retracted doc, or version superseded by a newer canonical doc |
| `ARCHIVE-closed-negative-scratch` | A closed-negative exploration or underdetermined scratch whose result is folded into Part II/III of the monograph |

---

## §3 — Full disposition table

Abbreviations used in **backs-Part** column: `0`=Part 0 Seed, `I`=Part I Constructive Reach, `II`=Part II The Boundary, `III`=Part III Bridge, `Coda`=Coda, `none`=outside monograph scope or infrastructure only.

### 01_reference — Canonical Reference Layer (27 docs)

| path | backs-Part | disposition | one-line reason |
|---|---|---|---|
| `01_reference/INDEX_01_REFERENCE.md` | none | KEEP-canonical | Cluster navigation index; stays live for drilling into 01_reference |
| `01_reference/MAP_ENGINE_ARCHITECTURE.md` | none | KEEP-supporting | C++ engine architecture map; infrastructure reference, not directly cited in monograph |
| `01_reference/MAP_LAGRANGIAN_TO_ENGINE.md` | none | KEEP-supporting | Lagrangian-to-engine cross-reference; infrastructure |
| `01_reference/MATH_ECOSYSTEM_MINDMAP.md` | none | KEEP-supporting | High-density conceptual guide; broad reference, not a monograph citation target |
| `01_reference/MATH_MASTER_QUADRATIC.md` | I | KEEP-canonical | Self-contained pure-math master-quadratic treatment; cited by Part I §I.3 |
| `01_reference/MONOGRAPH_GSTAR_BRIDGE_CONSTANT.md` | I | KEEP-canonical | G* derived from nine branches; cited by Part I §I.2 — direct canonical source |
| `01_reference/REF_REFERENCE_FRAME_VOCABULARY.md` | none | KEEP-supporting | Vocabulary canon; not cited in monograph body but background for consistent terminology |
| `01_reference/SPEC_ALGEBRAIC_SPINE.md` | I | KEEP-canonical | Nine numbered spine results; primary Part I citation target (OT-anchors) |
| `01_reference/SPEC_ALPHA_READOUT_CONTRACT.md` | II | KEEP-canonical | MC-T4.3 closure criteria; defines the readout problem for Part II §II.1 |
| `01_reference/SPEC_CLASS_B_CLUSTER_PERSISTENCE.md` | none | KEEP-supporting | Measurement infrastructure spec; out of monograph scope |
| `01_reference/SPEC_CLASS_C_CLUSTER_INTERACTION.md` | none | KEEP-supporting | Measurement infrastructure spec; out of monograph scope |
| `01_reference/SPEC_DIMENSIONAL_MAP.md` | III | KEEP-canonical | Dimensionless vs dimensional map; cited by Part III §III.5 for calibration-conditional predictions |
| `01_reference/SPEC_DISCRETE_NATIVE_DERIVATION.md` | 0 | KEEP-supporting | Methodological reframe (FTD-0136); background to Part 0 discrete-ontology framing |
| `01_reference/SPEC_DOCTRINE_LEDGER.md` | 0 | KEEP-canonical | Single-page status map (FTD-0145); canonical anchor for Part 0 epistemic contract |
| `01_reference/SPEC_FQCR.md` | I | KEEP-canonical | FQCR capstone; operator-theoretic provenance of G* cited in Part I §I.2 |
| `01_reference/SPEC_FTD_COMPARATIVE_PHYSICS.md` | none | KEEP-supporting | PF Atlas; broad reference, not a primary monograph citation |
| `01_reference/SPEC_FTD_COMPLETE_CHAIN.md` | I | KEEP-supporting | i-to-α complete chain; supporting for Part I narrative but lower priority than SPEC_ALGEBRAIC_SPINE |
| `01_reference/SPEC_FTD_LAGRANGIAN.md` | none | KEEP-supporting | Canonical 3-term action; engine/physics layer, not cited in the pure-math monograph body |
| `01_reference/SPEC_FTD_REFERENCE.md` | none | KEEP-supporting | Framework-wide reference; background, superseded by SPEC_ALGEBRAIC_SPINE + SPEC_DOCTRINE_LEDGER as citation targets |
| `01_reference/SPEC_MATH_FIRST_ONTOLOGY.md` | 0 | KEEP-canonical | Canonical math-first ontology ordering (FTD-0153); cited by Part 0 §0.2 |
| `01_reference/SPEC_NOVEL_PREDICTIONS.md` | none | KEEP-supporting | Predictions catalog; outside monograph scope (physics at [PARAMETRIC]) |
| `01_reference/SPEC_OPEN_MATH_BY_SECTOR.md` | II | KEEP-canonical | MC-T4.3 foundational-obstruction framing; cited by Part II §II.1 |
| `01_reference/SPEC_PHYSICS_BRIDGE.md` | III | KEEP-canonical | Physics-bridge synthesis (FTD-0121); primary Part III §III.1/§III.2 source |
| `01_reference/SPEC_QFT_GRT_BRIDGE_ROADMAP.md` | none | KEEP-supporting | QFT-GRT gap inventory; outside monograph scope |
| `01_reference/SPEC_QUADRATIC_PHYSICS_BRIDGE.md` | I | KEEP-supporting | SP1–SP6 selection principles; supporting context for Part I §I.3 |
| `01_reference/SPEC_SIX_ALGORITHMS.md` | none | KEEP-supporting | Engine-rule reference; infrastructure |
| `01_reference/SPEC_SM_REPLACEMENT_COMPLETE.md` | none | KEEP-supporting | SM replacement capstone; broad reference beyond monograph scope |

### 02_foundations — Ontological Foundations (31 docs)

| path | backs-Part | disposition | one-line reason |
|---|---|---|---|
| `02_foundations/DERIV_D3_FROM_AUTOMORPHISM.md` | I | KEEP-supporting | D=3 uniqueness from |Aut(E_i)|²; supports Part I §I.1 (why ℤ[i]) |
| `02_foundations/FOUND_AXIOM_ZERO.md` | 0 | KEEP-supporting | Single foundational axiom proposal; context for Part 0 §0.1 postulates |
| `02_foundations/FOUND_BASE_INTEGERS_UNCONSTRAINED.md` | I | KEEP-supporting | Ab-initio derivation of {3,4,7,13} from Fibonacci/Lucas; supports Part I framework integers |
| `02_foundations/FOUND_BLIND_DERIVATION_CHAIN.md` | I | KEEP-supporting | 13-step blind derivation from i to α⁻¹; supports Part I narrative chain |
| `02_foundations/FOUND_BORN_RULE_NULL_CONE.md` | none | KEEP-supporting | Born rule from null-cone geometry; outside primary monograph scope |
| `02_foundations/FOUND_DIMENSIONAL_COUNTING.md` | I | KEEP-supporting | D=3 constructive counting argument; supports Part I §I.1 |
| `02_foundations/FOUND_EPISTEMIC_SYMMETRIES_AND_CHIRALITY.md` | none | KEEP-supporting | Postulate Six conjecture; speculative, outside monograph scope |
| `02_foundations/FOUND_EULER_IDENTITY_TERNARY.md` | 0 | KEEP-supporting | Ternary annihilation rule; supports Part 0 §0.1 discrete-ontology context |
| `02_foundations/FOUND_FORCE_STRUCTURE.md` | I | KEEP-supporting | Four-forces genealogy from master quadratic; supports Part I §I.3 |
| `02_foundations/FOUND_FOURCIER_ONTIC_TOOL.md` | none | KEEP-supporting | Fourcier curve as distinction instrument; speculative, outside monograph scope |
| `02_foundations/FOUND_GSTAR_SCALE.md` | none | KEEP-supporting | G* scale conjecture; conjectural context, not a monograph claim |
| `02_foundations/FOUND_LADDER_GENERATING_RULE.md` | I | KEEP-supporting | α-power ladder exponent identity; supports Part I §I.4 spine |
| `02_foundations/FOUND_LADDER_WALK_FROM_OH_STRUCTURE.md` | I | KEEP-supporting | O_h ladder-walk (FTD-0084); supports Part I framework integers |
| `02_foundations/FOUND_LATTICE_PHYSICS_INTUITIONS.md` | none | KEEP-supporting | Reference table of phenomena ↔ lattice definitions; infrastructure |
| `02_foundations/FOUND_LATTICE_SPACING_GAUGE_FREEDOM.md` | none | KEEP-supporting | Lattice spacing as gauge freedom (FTD-0137); methodology, outside monograph scope |
| `02_foundations/FOUND_MASTER_QUADRATIC_BARE_STRUCTURE.md` | I | KEEP-supporting | Master quadratic stripped to bare algebra (FTD-0082); supports Part I §I.3 |
| `02_foundations/FOUND_MASTER_QUADRATIC_UNIFIED_MOTIVATION.md` | I | KEEP-supporting | Two independent routes converge on same polynomial (FTD-0081); supports Part I §I.3 |
| `02_foundations/FOUND_MASTER_QUADRATIC_UNIQUENESS_PROOF.md` | I | KEEP-supporting | SP2 uniqueness promoted to theorem (FTD-0083); supports Part I §I.3 |
| `02_foundations/FOUND_META_PATTERNS.md` | 0 | KEEP-supporting | Five boundary-type taxonomy; supports Part 0 §0.4 two-clause framing |
| `02_foundations/FOUND_MINIMAL_INSTANTIATED_UNIVERSE.md` | 0 | KEEP-supporting | Minimal ingredients for instantiated universe; supports Part 0 §0.1 |
| `02_foundations/FOUND_NONLINEAR_SPACETIME_EMERGENCE.md` | none | KEEP-supporting | GR from tick-rate saturation (2026-05-29 doc); supporting physics derivation, outside monograph math scope |
| `02_foundations/FOUND_ONTIC_MATHEMATICAL_FOUNDATIONS.md` | none | KEEP-supporting | Historical γ→ϖ→M→π→G* constant chain; background reference with honesty caveat noted in index |
| `02_foundations/FOUND_ONTOLOGICAL_GENESIS.md` | 0 | KEEP-supporting | Void-to-physics emergence hierarchy; supports Part 0 §0.1 |
| `02_foundations/FOUND_PHENOMENAL_NOUMENAL_BRIDGE.md` | none | KEEP-supporting | Two-layer ontology vocabulary (FTD-0078); interpretive, outside monograph scope |
| `02_foundations/FOUND_POTENTIAL_CORE_AND_GENERATIVE_INTERIOR.md` | none | KEEP-supporting | Manifestation vocabulary; speculative, outside monograph scope |
| `02_foundations/FOUND_SELF_REFERENTIAL_CLOSURE.md` | none | KEEP-supporting | Self-referential closure argument; interpretive, outside monograph scope |
| `02_foundations/FOUND_SPACETIME_EMERGENCE_AND_GRAVITY.md` | none | KEEP-supporting | Dimensions/time/gravity from relations; physics layer, outside monograph math scope |
| `02_foundations/FOUND_STRUCTURAL_DECOUPLING.md` | II | KEEP-supporting | Algebraic spine decouples from engine action (FTD-0129); supports Part II §II.2 route analysis |
| `02_foundations/FOUND_STRUCTURAL_DYNAMICAL_DISCRIMINATOR.md` | II | KEEP-canonical | Boundary theorem Stage 1 [DEFINITION] (FTD-0186); cited by Part II §II.2 |
| `02_foundations/FOUND_TERNARY_STATE_FROM_I.md` | 0 | KEEP-canonical | Ternary state field from ℤ[i]^× ∪ {0} (FTD-0128); primary source for Part 0 §0.1 |
| `02_foundations/FOUND_THE_COMPLETE_ALGEBRA_OF_i.md` | I | KEEP-supporting | Complete algebra of i (perpendicularity, division algebras); supports Part I §I.1 |
| `02_foundations/FOUND_THE_FIRST_DISTINCTION.md` | I | KEEP-supporting | "i exists" as first binary distinction; supports Part I §I.1 |
| `02_foundations/FOUND_THE_RATIO_AND_THE_PRODUCT.md` | I | KEEP-supporting | Euler reflection (π) vs ratio (G*); supports Part I §I.2 G* framing |
| `02_foundations/INDEX_02_FOUNDATIONS.md` | none | KEEP-canonical | Cluster navigation index |

### 03_derivations — Core Physics Derivations (62 docs across sub-clusters)

| path | backs-Part | disposition | one-line reason |
|---|---|---|---|
| `03_derivations/DERIV_ALPHA_OPERATIONAL_READOUT.md` | II | KEEP-supporting | Scalar fixed-point readout attempt; ARC scratch — folded into Part II closed-route survey |
| `03_derivations/DERIV_BORN_PROPORTIONALITY.md` | none | KEEP-supporting | Born rule proportionality (2026-05-29 doc); outside monograph math scope |
| `03_derivations/DERIV_LAGRANGIAN_FROM_TICK_RULE.md` | none | KEEP-supporting | Bare action construction (FTD-0246 [OPEN]); EFT open item, outside monograph scope |
| `03_derivations/DERIV_LATTICE_PATH_INTEGRAL_JTWIST.md` | II | KEEP-supporting | J-twisted path integral (FTD-0245 [OPEN]); provides context for Part II §II.2 Route 1 |
| `03_derivations/INDEX_03_DERIVATIONS.md` | none | KEEP-canonical | Cluster navigation index |
| `03_derivations/electromagnetism/DERIV_CONTINUUM_LIMIT_QED_EQUIVALENCE.md` | III | KEEP-supporting | FTD U(1)→QED continuum-limit argument [SMC FTD-0013]; Part III background |
| `03_derivations/electromagnetism/DERIV_COULOMB_SCATTERING_AMPLITUDE.md` | none | KEEP-supporting | Tree-level Coulomb amplitude; physics detail, outside monograph scope |
| `03_derivations/electromagnetism/DERIV_EM_REGIMES_UNIFIED.md` | none | KEEP-supporting | EM regimes unification; physics detail |
| `03_derivations/electromagnetism/DERIV_LATTICE_HODGE_DUALITY.md` | none | KEEP-supporting | Bianchi identities on lattice; physics |
| `03_derivations/electromagnetism/DERIV_LATTICE_LIENARD_WIECHERT.md` | none | KEEP-supporting | Lattice boosted Coulomb; physics detail |
| `03_derivations/electromagnetism/DERIV_LATTICE_LW_EXTENSIONS.md` | none | KEEP-supporting | Maxwell-exploit Q5–Q8; physics detail |
| `03_derivations/electromagnetism/DERIV_LATTICE_QED_COMPLETE.md` | none | KEEP-supporting | Consolidated one- and two-loop lattice QED; physics detail |
| `03_derivations/electromagnetism/DERIV_STATE_FLUX_COUPLING_DERIVATION.md` | none | KEEP-supporting | g_c = √α coupling derivation; EFT detail |
| `03_derivations/electromagnetism/THEOREM_HARMONIC_INVARIANT_TOWER.md` | I | KEEP-canonical | Harmonic-invariant tower 1/y₊+1/y₋=1 [THEOREM] (FTD-0111); cited by Part I §I.4 (OT-1.3) |
| `03_derivations/foundational_mechanics/DERIV_18PT_LAPLACIAN_VARIATIONAL.md` | none | KEEP-supporting | 18-point Laplacian from action principle; engine/EFT detail |
| `03_derivations/foundational_mechanics/DERIV_ALPHA_FROM_PHASE_STRUCTURE.md` | III | KEEP-supporting | Phase-structure argument x₊ in Coulomb phase [SELECTION]; Part III §III.3 closed route |
| `03_derivations/foundational_mechanics/DERIV_AP_NO_OVER_COUNT.md` | none | KEEP-supporting | Lattice no-over-count argument; technical detail |
| `03_derivations/foundational_mechanics/DERIV_BOTTOM_UP_PHYSICS.md` | 0 | KEEP-supporting | Entry-point narrative: 0=(−1)+(+1) to SM; supports Part 0 §0.4 |
| `03_derivations/foundational_mechanics/DERIV_BRANCH_HOLONOMY_GAP.md` | none | KEEP-supporting | Branch holonomy gap analysis; technical detail |
| `03_derivations/foundational_mechanics/DERIV_CHARGE_QUARTIC_FROM_GSTAR.md` | I | KEEP-supporting | Charge quartic e²=1/x; supports Part I §I.3 master-quadratic structure |
| `03_derivations/foundational_mechanics/DERIV_COMPTON_INVERSION_RESOLUTION.md` | none | KEEP-supporting | Compton inversion resolution; physics detail |
| `03_derivations/foundational_mechanics/DERIV_CONFINEMENT_FROM_GAP_EQUATION.md` | III | KEEP-supporting | Confinement from area-law Wilson loops at x₋; Part III §III.5 [PARAMETRIC] context |
| `03_derivations/foundational_mechanics/DERIV_DAMPING_RAYLEIGH.md` | none | KEEP-supporting | Rayleigh damping; physics detail |
| `03_derivations/foundational_mechanics/DERIV_FORCE_EMERGENCE.md` | none | KEEP-supporting | Four forces from lattice Green's function; physics detail |
| `03_derivations/foundational_mechanics/DERIV_FTD0110_FREE_ENERGY_LANDSCAPE.md` | none | KEEP-supporting | FTD-0110 free-energy landscape analysis; EFT detail |
| `03_derivations/foundational_mechanics/DERIV_FTD0110_NONLINEAR_BRIDGE.md` | none | KEEP-supporting | Nonlinear bridge (challenged by AUDIT_FTD0110_2026-05-27); status OPEN per latest audit |
| `03_derivations/foundational_mechanics/DERIV_FTD0110_VARIANCE_ENTROPY.md` | none | KEEP-supporting | Variance/entropy analysis for FTD-0110; EFT detail |
| `03_derivations/foundational_mechanics/DERIV_GAP_EQUATION_FORM.md` | I | KEEP-supporting | Self-consistency form of the gap equation; supports Part I §I.3 |
| `03_derivations/foundational_mechanics/DERIV_GSTAR_FINITE_APPROX.md` | I | KEEP-supporting | G* finite-N approximation (FTD-0142 attractor); cited by Part I §I.2 (OT-1.8) |
| `03_derivations/foundational_mechanics/DERIV_GSTAR_QUARTER_CONJUGACY.md` | I | KEEP-canonical | det_ζ quarter-conjugacy bridge (FTD-0141); cited by Part I §I.2 (OT-1.7) |
| `03_derivations/foundational_mechanics/DERIV_HEAT_EQUATION_FROM_RATIO.md` | none | KEEP-supporting | Heat equation from Euler ratio; physics detail |
| `03_derivations/foundational_mechanics/DERIV_INTEGER_PHYSICAL_IDENTIFICATION.md` | I | KEEP-supporting | Framework integers {3,4,7,13} physical roles; supports Part I §I.4 |
| `03_derivations/foundational_mechanics/DERIV_KCOMP_VOLUMETRIC_SHELL.md` | none | KEEP-supporting | Volumetric shell analysis; EFT detail |
| `03_derivations/foundational_mechanics/DERIV_K_FROM_OH_A1G_MULTIPLICITY.md` | I | KEEP-supporting | k=1/N_base from O_h rep theory (FTD-0110 linear level); supports Part I §I.1 |
| `03_derivations/foundational_mechanics/DERIV_MASTER_QUADRATIC_FROM_Z.md` | I | KEEP-supporting | Partition-function route (L→∞ retracted, polynomial preserved); Part I §I.3 provenance |
| `03_derivations/foundational_mechanics/DERIV_MASTER_QUADRATIC_GAP_EQUATION.md` | I | KEEP-canonical | Canonical master-quadratic reference: identity, coefficient routes, discriminant; primary Part I §I.3 citation |
| `03_derivations/foundational_mechanics/DERIV_QFT_GRT_BRIDGE.md` | none | KEEP-supporting | Lattice Green's function = Euclidean propagator; physics |
| `03_derivations/foundational_mechanics/DERIV_QUADRATIC_NECESSITY.md` | I | KEEP-supporting | Two independent arguments that degree-2 is forced; supports Part I §I.3 |
| `03_derivations/foundational_mechanics/DERIV_RADIAL_METRIC_RESOLUTION.md` | none | KEEP-supporting | Radial metric resolution; technical detail |
| `03_derivations/foundational_mechanics/DERIV_RETARDED_GREEN_LATTICE.md` | none | KEEP-supporting | Retarded lattice Green's function; physics detail |
| `03_derivations/foundational_mechanics/DERIV_THREE_RESOLUTIONS.md` | none | KEEP-supporting | Three resolution approaches; technical detail |
| `03_derivations/foundational_mechanics/DERIV_VARIATIONAL_PROOF.md` | none | KEEP-supporting | δS=0 reproduces engine update rules; engine verification |
| `03_derivations/foundational_mechanics/EXPLR_FTD_0110_NONLINEAR_BRIDGE_ANALYSIS.md` | none | KEEP-supporting | Nonlinear bridge analysis; EFT open item detail |
| `03_derivations/foundational_mechanics/MONOGRAPH_EFFECTIVE_EQUATIONS.md` | none | KEEP-supporting | Effective equations monograph; engine/EFT layer |
| `03_derivations/foundational_mechanics/PREREG_CLOCK_HYPOTHESIS_DERIVATION_v1.md` | none | KEEP-supporting | Clock-hypothesis v1 pre-reg (superseded by v3 but retained as provenance) |
| `03_derivations/foundational_mechanics/PREREG_CLOCK_HYPOTHESIS_DERIVATION_v3.md` | none | KEEP-supporting | Clock-hypothesis v3 active pre-reg (FTD-0208; open item) |
| `03_derivations/foundational_mechanics/PROTOCOL_TOWER_LEVEL_FALSIFIER.md` | none | KEEP-supporting | Tower level-scan pre-reg draft; active protocol |
| `03_derivations/gravity_and_cosmology/DERIV_BLACK_HOLE_PHYSICS.md` | none | KEEP-supporting | BH physics; physics detail |
| `03_derivations/gravity_and_cosmology/DERIV_DARK_SECTOR_DYNAMICS.md` | none | KEEP-supporting | Dark sector dynamics; physics detail |
| `03_derivations/gravity_and_cosmology/DERIV_EINSTEIN_FIELD_EQUATIONS.md` | none | KEEP-supporting | Einstein field equations; physics detail |
| `03_derivations/gravity_and_cosmology/DERIV_EINSTEIN_NONLINEAR_FROM_LATTICE.md` | none | KEEP-supporting | Deser bootstrap from lattice; physics |
| `03_derivations/gravity_and_cosmology/DERIV_EMERGENT_DIFFEROMORPHISM_INVARIANCE.md` | none | KEEP-supporting | Diffeomorphism invariance; physics detail |
| `03_derivations/gravity_and_cosmology/DERIV_LATTICE_BLACK_HOLES.md` | none | KEEP-supporting | Lattice BH thermodynamics; physics detail |
| `03_derivations/gravity_and_cosmology/DERIV_NEWTON_FROM_SUBSTRATE.md` | none | KEEP-supporting | Newton from substrate; gravity derivation |
| `03_derivations/gravity_and_cosmology/DERIV_RELATIVITY_DERIVATION.md` | none | KEEP-supporting | Relativity derivation; physics detail |
| `03_derivations/gravity_and_cosmology/DERIV_SCALE_GROWTH_AND_COSMIC_EMERGENCE.md` | none | KEEP-supporting | Scale growth; physics detail |
| `03_derivations/gravity_and_cosmology/DERIV_STELLAR_LIFECYCLE_LATTICE.md` | none | KEEP-supporting | Stellar lifecycle on lattice; physics detail |
| `03_derivations/gravity_and_cosmology/FOUND_STRONG_FIELD_GRAVITY_SIGNATURE.md` | none | KEEP-supporting | Strong-field gravity [OPEN] (FTD-0184 finding); active open item |
| `03_derivations/gravity_and_cosmology/SCOPE_NEWTON_POSTULATES_RECONCILIATION.md` | none | KEEP-supporting | Newton postulates reconciliation scope (audit outcome); active reference |
| `03_derivations/quantum_mechanics/DERIV_BELL_COSINE_FROM_GAUSS.md` | none | KEEP-supporting | Bell S=2√2 from Gauss; QM foundations detail |
| `03_derivations/quantum_mechanics/DERIV_BORN_PROPORTIONALITY_RESOLUTION.md` | none | KEEP-supporting | Born proportionality resolution; QM detail |
| `03_derivations/quantum_mechanics/DERIV_DIRAC_FROM_MASTER_QUADRATIC.md` | I | KEEP-supporting | Dirac equation from master quadratic; Part I §I.4 supporting derivation |
| `03_derivations/quantum_mechanics/DERIV_NONCOMMUTATIVE_EMERGENCE.md` | II | KEEP-supporting | Non-commutativity emergence attempt; supports Part II commutativity-wall context |
| `03_derivations/quantum_mechanics/DERIV_OBSERVER_BELL_MECHANISM.md` | none | KEEP-supporting | Observer Bell mechanism; QM detail |
| `03_derivations/quantum_mechanics/DERIV_PATH_INTEGRAL_CONSTRUCTION.md` | none | KEEP-supporting | Path integral natively on lattice; EFT detail |
| `03_derivations/quantum_mechanics/DERIV_QM_FROM_LATTICE.md` | none | KEEP-supporting | QM as statistics of lattice events; physics background |
| `03_derivations/quantum_mechanics/DERIV_SINGLET_FROM_VOID_EVENT.md` | none | KEEP-supporting | Bell loop closed via 5 lemmas; QM foundations |
| `03_derivations/quantum_mechanics/DERIV_SPIN_STATISTICS_BRIDGE.md` | none | KEEP-supporting | Spin-statistics bridge; QM detail |
| `03_derivations/quantum_mechanics/MEASUREMENT_TOWER_LEVEL_SCAN.md` | I | KEEP-supporting | Hash-locked tower level scan [MEASURED CONFIRMATORY]; supports Part I §I.4 |
| `03_derivations/standard_model/DERIV_FERMI_COUPLING_CONSTANT.md` | none | KEEP-supporting | G_F from Higgs VEV chain; physics |
| `03_derivations/standard_model/DERIV_HIGGS_FROM_MANIFESTATION.md` | none | KEEP-supporting | Higgs mechanism; physics |
| `03_derivations/standard_model/DERIV_LATTICE_CHIRAL_ANOMALY.md` | none | KEEP-supporting | ABJ anomaly from lattice; physics |
| `03_derivations/standard_model/DERIV_LATTICE_SU2_WEAK.md` | none | KEEP-supporting | SU(2) weak from ternary states; physics |
| `03_derivations/standard_model/DERIV_LATTICE_SU3_GAUGE.md` | none | KEEP-supporting | SU(3) color from flux geometry; physics |
| `03_derivations/standard_model/DERIV_MOORE_GAUGE_STRUCTURE.md` | I | KEEP-supporting | SM gauge groups from Moore neighborhood; supports Part I §I.4 |
| `03_derivations/standard_model/DERIV_NC_FROM_TOPOLOGY.md` | I | KEEP-canonical | N_c=3 from four topological routes [THEOREM]; cited by Part I §I.4 (independent N_c source) |
| `03_derivations/standard_model/DERIV_PION_MASS_FROM_GSTAR.md` | none | KEEP-supporting | Pion mass from G*; physics |
| `03_derivations/standard_model/DERIV_SUBSTRATE_YUKAWA_VERTEX.md` | none | KEEP-supporting | Substrate Yukawa vertex; physics |
| `03_derivations/standard_model/DERIV_THREE_GENERATIONS.md` | none | KEEP-supporting | N_gen=3 from cuboctahedral axes; physics |
| `03_derivations/standard_model/DERIV_YUKAWA_FROM_27BLOCK_CHARACTER.md` | none | KEEP-supporting | Yukawa from 27-block characters; physics |
| `03_derivations/standard_model/DERIV_Z3_CENTER_GRAPH_CLOSURE.md` | none | KEEP-supporting | Z_3 center graph; physics |

### 04_coupling — Coupling Constants & Precision (14 docs)

| path | backs-Part | disposition | one-line reason |
|---|---|---|---|
| `04_coupling/DERIV_ALPHA_LATTICE_MECHANISM.md` | I | KEEP-supporting | Five-step chain ℤ³→α (x₊=1/α is [SMC]); supports Part I §I.2/§I.3 narrative |
| `04_coupling/DERIV_ALPHA_PRECISION_FORMULA.md` | III | KEEP-supporting | 7-term precision series [CONJECTURE]; Part III §III.2 evidence context |
| `04_coupling/DERIV_ALPHA_READOUT_RESOLUTION.md` | III | ARCHIVE-superseded | Retracted (banner-marked 2026-06-01): substitution-identity facade; Part III §III.3 closed-negative provenance only |
| `04_coupling/DERIV_COSMOLOGICAL_CONSTANT.md` | none | KEEP-supporting | Λ from lattice vacuum energy [SELECTION]; physics beyond monograph scope |
| `04_coupling/DERIV_DISCRETE_CONTINUOUS_BRIDGE.md` | I | KEEP-supporting | Master quadratic as lattice↔lemniscate connector; supports Part I §I.3 |
| `04_coupling/DERIV_GSTAR_PF_BRIDGE.md` | I | KEEP-supporting | G*=ϖ/√(PF) decomposition; supports Part I §I.2 G* structure |
| `04_coupling/DERIV_LAMBDA_QCD_DERIVATION.md` | none | KEEP-supporting | Λ_QCD via dimensional transmutation [SELECTION]; physics detail |
| `04_coupling/DERIV_LEMNISCATE_HIERARCHY_WHITEPAPER.md` | none | KEEP-supporting | Lemniscate hierarchy white paper; physics context |
| `04_coupling/DERIV_ONE_LOOP_LATTICE_ALPHA.md` | none | KEEP-supporting | One-loop α correction [SELECTION/scheme-conditional]; physics detail |
| `04_coupling/DERIV_PHI3_EXACT_EFT.md` | I | KEEP-supporting | Master cubic EFT [THEOREM algebraic]; supports Part I §I.3 |
| `04_coupling/DERIV_PLANCK_MASS_AND_LAMBDA_QCD.md` | none | KEEP-supporting | M_P scale-setting; physics |
| `04_coupling/DERIV_WATSON_GSTAR_IDENTITY.md` | I | KEEP-canonical | Watson identity W₃=G*²/(2π) [THEOREM] (OT-2.1); primary Part I §I.2 citation |
| `04_coupling/EXPLR_A_OVER_D_AUDIT.md` | none | KEEP-supporting | Lattice spacing a=2/D audit; EFT methodology |
| `04_coupling/INDEX_04_COUPLING.md` | none | KEEP-canonical | Cluster navigation index |

### 05_particles — Particle Physics (12 docs)

| path | backs-Part | disposition | one-line reason |
|---|---|---|---|
| `05_particles/DERIV_COLOR_BINDING_STRUCTURE_AND_ME_STATUS.md` | none | KEEP-supporting | Color binding + m_e status; physics detail |
| `05_particles/DERIV_COMPLETE_PARTICLE_PHYSICS.md` | none | KEEP-supporting | Complete particle physics derivations; physics detail |
| `05_particles/DERIV_ELECTRON_MASS_MOTIVATION.md` | III | KEEP-supporting | m_e=m_P·√(2π)·(16/3)·α¹¹ motivation; Part III §III.5 [PARAMETRIC] context |
| `05_particles/DERIV_EMERGENT_GRAVITON_CENSUS.md` | none | KEEP-supporting | Graviton census; physics detail |
| `05_particles/DERIV_MATERIAL_EMERGENCE_FROM_LATTICE.md` | none | KEEP-supporting | Material emergence; physics detail |
| `05_particles/DERIV_OCTONIONIC_STRUCTURE.md` | none | KEEP-supporting | Octonionic structure; speculative physics |
| `05_particles/EXPLR_FTD_MASS_CHAIN.md` | none | KEEP-supporting | Mass chain exploration; physics detail |
| `05_particles/EXPLR_GENERATION_GRAPH_GAMMA_D.md` | none | KEEP-supporting | Generation graph exploration; physics |
| `05_particles/FOUND_DISCRETE_NATIVE_MASS_GENERATION.md` | none | KEEP-supporting | Discrete-native mass generation; physics |
| `05_particles/INDEX_05_PARTICLES.md` | none | KEEP-canonical | Cluster navigation index |
| `05_particles/PRED_ELECTROWEAK_MASSES.md` | none | KEEP-supporting | Electroweak mass predictions; physics |
| `05_particles/REF_PHYSICS_REFERENCE.md` | none | KEEP-supporting | Physics reference table; broad reference |

### 06_reference_frames_and_measurement — Reference Frames & Measurement (13 docs)

| path | backs-Part | disposition | one-line reason |
|---|---|---|---|
| `06_reference_frames_and_measurement/DERIV_COLLAPSE_MECHANISM.md` | none | KEEP-supporting | Collapse mechanism derivation; QM foundations |
| `06_reference_frames_and_measurement/DERIV_CONNES_LAMBDA_FROM_MODULAR_FLOW.md` | none | KEEP-supporting | Connes λ from modular flow; QM foundations |
| `06_reference_frames_and_measurement/DERIV_CONSCIOUSNESS_QFT_GR_SYNTHESIS.md` | none | KEEP-supporting | QFT/GR synthesis via modular flow; speculative, outside monograph scope |
| `06_reference_frames_and_measurement/DERIV_VON_NEUMANN_CONSTRUCTION.md` | none | KEEP-supporting | von Neumann construction; QM foundations |
| `06_reference_frames_and_measurement/EXPLR_BORN_EQUILIBRIUM_PRESERVATION_NEGATIVE.md` | III | KEEP-supporting | Born equilibrium preservation [CLOSED NEGATIVE]; Part III §III.3 route provenance |
| `06_reference_frames_and_measurement/EXPLR_THRESHOLD_CROSSING_BORN_NEGATIVE.md` | III | KEEP-supporting | Threshold crossing Born [CLOSED NEGATIVE]; Part III §III.3 route provenance |
| `06_reference_frames_and_measurement/FOUND_DOMAIN_PARTITION_AND_CONTEXT_SELECTION.md` | none | KEEP-supporting | Domain partition and context selection; interpretive |
| `06_reference_frames_and_measurement/FOUND_THE_EXISTENCE_FILTER.md` | none | KEEP-supporting | Existence filter; interpretive |
| `06_reference_frames_and_measurement/FOUND_VON_NEUMANN_CHAIN.md` | none | KEEP-supporting | von Neumann chain; QM foundations |
| `06_reference_frames_and_measurement/FOUND_WIGNERS_FRIEND_RESOLUTION.md` | none | KEEP-supporting | Wigner's friend resolution; QM foundations |
| `06_reference_frames_and_measurement/INDEX_06_CONSCIOUSNESS.md` | none | KEEP-canonical | Cluster navigation index |
| `06_reference_frames_and_measurement/PREREG_BORN_EQUILIBRIUM_PRESERVATION_v1.md` | none | KEEP-supporting | Born equilibrium pre-reg v1; provenance for EXPLR_BORN closed-negative |
| `06_reference_frames_and_measurement/PREREG_THRESHOLD_CROSSING_BORN_v1.md` | none | KEEP-supporting | Threshold crossing pre-reg v1; provenance for EXPLR_THRESHOLD closed-negative |

### 07_assessment — Audits, Trackers & Assessment (58 docs)

| path | backs-Part | disposition | one-line reason |
|---|---|---|---|
| `07_assessment/AUDIT_BELL_ANALYSIS.md` | none | KEEP-supporting | Bell analysis; QM foundations detail |
| `07_assessment/AUDIT_DUAL_SUBSTRATE_PROVENANCE.md` | none | KEEP-supporting | Dual-substrate provenance check; hygiene audit |
| `07_assessment/AUDIT_ENGINE_CALLSTACK.md` | none | KEEP-supporting | Engine callstack audit; infrastructure |
| `07_assessment/AUDIT_EPISTEMIC_AUDIT.md` | 0 | KEEP-canonical | Honest "derived vs imported" accounting; cited by Part 0 §0.3 epistemic-contract context |
| `07_assessment/AUDIT_GSTAR_PAPER_MULTI_ROUND.md` | I | KEEP-supporting | Three-round referee report on G* papers; provenance for Part I spine claims |
| `07_assessment/AUDIT_HIDDEN_SELECTIONS.md` | II | KEEP-supporting | Selection-principles catalog; superseded parts banner-marked, but §§1–5 support Part II §II.2 |
| `07_assessment/AUDIT_INFINITY_REFRAME.md` | 0 | KEEP-canonical | Undefined-boundary ontology triage; primary Part 0 §0.1 citation (AUDIT_INFINITY_REFRAME) |
| `07_assessment/AUDIT_KAPPA_PSI_4PI.md` | III | KEEP-supporting | κ_ψ=4π is [DEFINITION]; [CLOSED NEGATIVE] on closure-law; Part III §III.3 |
| `07_assessment/AUDIT_LOOK_ELSEWHERE_RESULTS.md` | III | KEEP-canonical | Look-elsewhere scan results [MEASURED] (FTD-0097); primary Part III §III.2 citation |
| `07_assessment/AUDIT_MASS_CHAIN_REDTEAM.md` | III | KEEP-supporting | Mass chain adversarial critique; Part III §III.5 honest-physics context |
| `07_assessment/AUDIT_MASTER_QUADRATIC.md` | III | KEEP-canonical | Master-quadratic audit verdict [SMC]; cited by Part III §III.1 |
| `07_assessment/AUDIT_PAPER_SYMPATHETIC_2026-04-28.md` | I | KEEP-supporting | Sympathetic audit of PAPER_MASTER_QUADRATIC_AND_BRIDGE; Part I provenance |
| `07_assessment/AUDIT_RATIONAL_FIT_CLAIMS.md` | III | KEEP-supporting | Rational-fit claims demoted; Part III §III.3 demotions provenance |
| `07_assessment/AUDIT_SELF_CONSISTENCY.md` | I | KEEP-supporting | Framework integers {3,4,7,13} self-consistency; supports Part I §I.4 |
| `07_assessment/AUDIT_SESSION_2026_04_24.md` | none | ARCHIVE-absorbed | Session artifact: per-claim numerical verification; absorbed into live canonical trackers |
| `07_assessment/AUDIT_WHAT_IS_GENUINELY_NEW.md` | none | ARCHIVE-superseded | [LEGACY/pre-reframe]; superseded by SPEC_ALGEBRAIC_SPINE + TRACKER_ONTIC_TRUTH; do-not-cite banner in place |
| `07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md` | III | KEEP-canonical | Parametric-insertions enumeration (~162 rows); primary Part III §III.5 citation |
| `07_assessment/CHANGELOG_REFRAME.md` | none | KEEP-canonical | Append-only reframe changelog; live provenance record |
| `07_assessment/INDEX_07_ASSESSMENT.md` | none | KEEP-canonical | Cluster navigation index |
| `07_assessment/PARKING_LOT.md` | none | KEEP-supporting | Deferred reframe items; active working list |
| `07_assessment/PROTOCOL_LOOK_ELSEWHERE_SCAN.md` | III | KEEP-supporting | Pre-registration of look-elsewhere scan; provenance for FTD-0097 |
| `07_assessment/REDTEAM_GSTAR_IVY_LEAGUE_2026-05-19.md` | I | KEEP-supporting | Four-agent parallel red-team; catches L(E,1) BSD error; Part I provenance |
| `07_assessment/REF_CLAIMS_MATRIX.md` | none | KEEP-supporting | Claims matrix with IDs; infrastructure reference |
| `07_assessment/REPORT_DETECTOR_INFORMATION_LOSS.md` | none | KEEP-supporting | Detector information-loss exploration; QM foundations detail |
| `07_assessment/ROUNDTABLE_STATE_OF_FTD_2026-05-22.md` | none | ARCHIVE-absorbed | State-of-theory [SYNTHESIS] (FTD-0183, 2026-05-22); the monograph replaces its construction narrative |
| `07_assessment/STATUS_2026-05-04_post_bughunt.md` | none | ARCHIVE-absorbed | Session artifact: engine status snapshot; absorbed into LEDGER + WHERE_WE_LEFT_OFF |
| `07_assessment/SYNTHESIS_COMMUTATIVITY_BOUNDARY_2026-05-30.md` | none | ARCHIVE-absorbed | Commutativity-wall [SYNTHESIS] (FTD-0238); the monograph absorbs this into Part II §II.6 |
| `07_assessment/SYNTHESIS_GSTAR_BEDROCK_2026-05-19.md` | none | ARCHIVE-absorbed | G*-bedrock polymath [SYNTHESIS]; the monograph absorbs this into Part I §I.2 narrative |
| `07_assessment/audits/AUDIT_ALPHA_EXTRACTION.md` | II | KEEP-supporting | Phase-F α extraction audit (category-error resolved by Phase G); Part II §II.2 provenance |
| `07_assessment/audits/AUDIT_ALPHA_OPERATOR_FORCING_ROUTE_INVARIANCE.md` | II | KEEP-canonical | Route-invariant boundary audit (FTD-0242); primary Part II §II.2/§II.3 citation |
| `07_assessment/audits/AUDIT_ALPHA_READOUT_DET_IDENTITY_UNDERDETERMINED.md` | II | KEEP-supporting | det↔det_ζ identity [UNDERDETERMINED] (FTD-0235); Part II §II.2 Route 1 provenance |
| `07_assessment/audits/AUDIT_ALPHA_READOUT_ODD_PERIOD_UNDERDETERMINED.md` | II | KEEP-supporting | J-twisted det_ζ ratio [UNDERDETERMINED] (FTD-0234); Part II §II.2 provenance |
| `07_assessment/audits/AUDIT_ARC_C1_B2_FOUND_INDEPENDENT_REVIEW.md` | II | KEEP-supporting | ARC-C1/B2 FOUND→UNDERDETERMINED correction (FTD-0232); Part II §II.2 provenance |
| `07_assessment/audits/AUDIT_CHARGE_QUANTIZATION_NO_CHEAT.md` | II | KEEP-supporting | Charge quantization audit; Part II boundary provenance |
| `07_assessment/audits/AUDIT_CLOCK_HYPOTHESIS_v1_UNDERDETERMINED.md` | none | KEEP-supporting | Clock-hypothesis v1 [UNDERDETERMINED] (FTD-0208); active open item provenance |
| `07_assessment/audits/AUDIT_CLOCK_HYPOTHESIS_v2_UNDERDETERMINED.md` | none | KEEP-supporting | Clock-hypothesis v2 [UNDERDETERMINED/INVALIDATED] (FTD-0208); active open item provenance |
| `07_assessment/audits/AUDIT_FTD0110_2026-05-27_RESOLUTION.md` | none | KEEP-supporting | FTD-0110 challenged retag [OPEN]; active provenance for revert recommendation |
| `07_assessment/audits/AUDIT_HEEGNER_TOWER_RIGIDITY.md` | III | KEEP-supporting | Heegner tower rigidity scan; Part III §III.2 null result |
| `07_assessment/audits/AUDIT_LEMNISCATE_ALPHA_RIGIDITY.md` | III | KEEP-supporting | Lemniscate-alpha rigidity scan; Part III §III.2 evidence |
| `07_assessment/audits/AUDIT_NEWTON_POSTULATES_RECONCILIATION.md` | none | KEEP-supporting | Newton postulates reconciliation; gravity detail |
| `07_assessment/audits/AUDIT_RSI_LEG3_CONDITIONAL_THEOREM.md` | II | KEEP-canonical | RSI Leg 3: conditional theorem [THEOREM] + K-BIND [OPEN] (FTD-0243); primary Part II §II.3–§II.5 citation |
| `07_assessment/audits/AUDIT_SCALE0_SUBSTRATE_RESULTS.md` | none | KEEP-supporting | Scale-0 substrate measurement results; engine measurement provenance |
| `07_assessment/audits/AUDIT_SPEKKENS_KNOWLEDGE_BALANCE_PARTIAL.md` | none | KEEP-supporting | Spekkens knowledge-balance [PARTIAL]; QM foundations |
| `07_assessment/campaigns/archive_session_outputs/DEVILS_ADVOCATE_REPORT.md` | none | KEEP-supporting | April 19 devil's-advocate report (3 bugs fixed); historical provenance |
| `07_assessment/campaigns/archive_session_outputs/ENGINE_AUDIT_REFRAME.md` | none | KEEP-supporting | Engine reframe audit; historical provenance |
| `07_assessment/campaigns/archive_session_outputs/FLAGGED_PASSAGES_PAPERS.md` | none | KEEP-supporting | Flagged passages in papers; historical provenance |
| `07_assessment/campaigns/archive_session_outputs/INVENTORY_PORTFOLIO.md` | none | KEEP-supporting | April 19 portfolio inventory; historical provenance |
| `07_assessment/campaigns/archive_session_outputs/README.md` | none | KEEP-supporting | Session-outputs README; navigation |
| `07_assessment/campaigns/archive_session_outputs/REDERIVE_REPORT_YM_NS.md` | none | KEEP-supporting | YM/NS re-derive assessment; historical provenance |
| `07_assessment/campaigns/archive_session_outputs/SESSION_SYNTHESIS_2026-04-30.md` | none | ARCHIVE-absorbed | April 30 session synthesis; superseded by LEDGER entries and live trackers |
| `07_assessment/campaigns/reframe_deployment/CANONICAL_REFRAME.md` | 0 | KEEP-canonical | Authoritative statement of what the reframe means; active reference for Part 0 §0.1 |
| `07_assessment/campaigns/reframe_deployment/DEPLOYMENT_GUIDE.md` | none | KEEP-supporting | 7-phase deployment guide; historical reference |
| `07_assessment/campaigns/reframe_deployment/README.md` | none | KEEP-supporting | Reframe deployment README |
| `07_assessment/campaigns/reframe_deployment/agents/01_inventory.md` | none | KEEP-supporting | Reframe agent prompt (historical) |
| `07_assessment/campaigns/reframe_deployment/agents/02_classifier.md` | none | KEEP-supporting | Reframe agent prompt (historical) |
| `07_assessment/campaigns/reframe_deployment/agents/03_triage.md` | none | KEEP-supporting | Reframe agent prompt (historical) |
| `07_assessment/campaigns/reframe_deployment/agents/04_restatement.md` | none | KEEP-supporting | Reframe agent prompt (historical) |
| `07_assessment/campaigns/reframe_deployment/agents/05_rederivation.md` | none | KEEP-supporting | Reframe agent prompt (historical) |
| `07_assessment/campaigns/reframe_deployment/agents/06_engine_audit.md` | none | KEEP-supporting | Reframe agent prompt (historical) |
| `07_assessment/campaigns/reframe_deployment/agents/07_devils_advocate.md` | none | KEEP-supporting | Reframe agent prompt (historical) |
| `07_assessment/campaigns/reframe_deployment/agents/08_consistency.md` | none | KEEP-supporting | Reframe agent prompt (historical) |
| `07_assessment/campaigns/reframe_deployment/agents/09_ledger.md` | none | KEEP-supporting | Reframe agent prompt (historical) |
| `07_assessment/campaigns/reframe_deployment/checklists/per_paper.md` | none | KEEP-supporting | Reframe per-paper checklist (historical) |
| `07_assessment/campaigns/reframe_deployment/checklists/post_flight.md` | none | KEEP-supporting | Reframe post-flight checklist (historical) |
| `07_assessment/campaigns/reframe_deployment/checklists/pre_flight.md` | none | KEEP-supporting | Reframe pre-flight checklist (historical) |
| `07_assessment/campaigns/reframe_deployment/templates/AUDIT_REPORT_TEMPLATE.md` | none | KEEP-supporting | Reframe template (historical) |
| `07_assessment/campaigns/reframe_deployment/templates/CLAUDE_MD_TEMPLATE.md` | none | KEEP-supporting | Reframe template (historical) |
| `07_assessment/campaigns/reframe_deployment/templates/LEDGER_ENTRY_TEMPLATE.md` | none | KEEP-supporting | Reframe template; still referenced for LEDGER entries |
| `07_assessment/campaigns/reframe_deployment/templates/RESTATEMENT_TEMPLATE.md` | none | KEEP-supporting | Reframe template (historical) |
| `07_assessment/core_ledgers/LEDGER.md` | 0 | KEEP-canonical | Master claim ledger; primary citation authority across all Parts |
| `07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md` | I | KEEP-canonical | Canonical 5-tier bedrock tracker (OT-IDs); primary Part I/II citation authority |
| `07_assessment/core_ledgers/TRACKER_OPEN_ITEMS.md` | none | KEEP-canonical | Central [OPEN] items ledger; updated by Phase 2 red-team |

### 08_structural — Structural & Geometric Principles (18 docs)

| path | backs-Part | disposition | one-line reason |
|---|---|---|---|
| `08_structural/AUDIT_COLOUR_SINGLET_RANK.md` | I | KEEP-supporting | Colour singlet rank audit [COMPLETE]; backs Part I §I.4 N_c structural argument |
| `08_structural/AUDIT_FINITE_NEUTRAL_LOCK.md` | I | KEEP-supporting | Finite neutral lock audit [COMPLETE]; backs Part I §I.4 |
| `08_structural/AUDIT_WEAK_SU2_PROVENANCE.md` | I | KEEP-supporting | SU(2) weak provenance audit; backs Part I §I.4 |
| `08_structural/DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md` | I | KEEP-canonical | BCC structure unifies Watson + SU(3); cited by Part I §I.2/§I.4 (OT-1.5/1.6) |
| `08_structural/DERIV_CUBOCTAHEDRAL_INTEGERS.md` | I | KEEP-canonical | {3,4,7,13} from cuboctahedron [THEOREM]; supports Part I §I.1 |
| `08_structural/DERIV_DUAL_DERIVATION_OF_16.md` | I | KEEP-canonical | Coefficient 16 from two routes [THEOREM]; Part I §I.3 T4 soft-spot source |
| `08_structural/DERIV_EXISTENTIAL_UNIT.md` | I | KEEP-supporting | 3³ minimal lattice N_c=3 selection; supports Part I §I.1 |
| `08_structural/DERIV_STABILIZER_DECOMPOSITION.md` | I | KEEP-supporting | Stab_{O_h}(e₃) ≅ D₄×ℤ/2 bridge to Aut(E_i); supports Part I §I.3 |
| `08_structural/EXPLR_25_VOXEL_CLUSTER_GEOMETRY.md` | none | KEEP-supporting | 25-voxel cluster topology (§3 hypothesis refuted); engine measurement |
| `08_structural/EXPLR_LOOP_GRID_DUALITY.md` | none | KEEP-supporting | Loop/grid duality exploration; structural detail |
| `08_structural/EXPLR_OCTAHEDRAL_BOUND_STATES.md` | none | KEEP-supporting | Octahedral bound state exploration; engine measurement |
| `08_structural/EXPLR_PHASE_LATTICE_MOORE.md` | none | KEEP-supporting | Phase lattice Moore analysis; structural detail |
| `08_structural/EXPLR_TRIT_INFORMATION_THEORY.md` | 0 | KEEP-supporting | Trit information theory lens on G*; supports Part 0 §0.1 ternary-states context |
| `08_structural/INDEX_08_STRUCTURAL.md` | none | KEEP-canonical | Cluster navigation index |
| `08_structural/PREREG_COLOUR_SINGLET_RANK_v1.md` | I | KEEP-supporting | Colour singlet rank v1 pre-reg; provenance for AUDIT_COLOUR_SINGLET |
| `08_structural/PREREG_FINITE_NEUTRAL_LOCK_v1.md` | I | KEEP-supporting | Finite neutral lock v1 pre-reg; provenance for AUDIT_FINITE_NEUTRAL_LOCK |
| `08_structural/PREREG_WEAK_SU2_PROVENANCE_v1.md` | I | KEEP-supporting | SU(2) provenance v1 pre-reg; provenance for AUDIT_WEAK_SU2 |
| `08_structural/THEOREM_MOORE_LAYER_DECOMPOSITION.md` | I | KEEP-canonical | Moore Layer Theorem [THEOREM/SELECTION]: SM gauge groups + particles from Moore neighborhood; cited by Part I §I.4 |

### 09_mathematical — Number Theory & Mathematical Connections (56 docs)

| path | backs-Part | disposition | one-line reason |
|---|---|---|---|
| `09_mathematical/INDEX_09_MATHEMATICAL.md` | none | KEEP-canonical | Cluster navigation index |
| `09_mathematical/NODE_MAP_FTD_MATH.md` | I | KEEP-supporting | Node map overview; navigation for the construction DAG figure (Part I §I.5) |
| `09_mathematical/algebra/DERIV_BIVECTOR_ALGEBRA_PROGRAM_F.md` | none | KEEP-supporting | Cl(3,0) campaign; fermion-emergence detail |
| `09_mathematical/algebra/DERIV_DIRAC_KAHLER_IDENTIFICATION.md` | none | KEEP-supporting | Dirac-Kähler identification; algebra detail |
| `09_mathematical/algebra/DERIV_LINK_BILINEAR_CLIFFORD_PARTIAL.md` | none | KEEP-supporting | Link bilinear program F; algebra detail |
| `09_mathematical/algebra/DERIV_WH_ALGEBRA_VS_CLIFFORD_NOGO.md` | none | KEEP-supporting | Walsh-Hadamard vs Clifford [no-go]; algebra |
| `09_mathematical/algebra/EXPLR_CAYLEY_DICKSON_FOURCIER_ISOMORPHISM.md` | none | KEEP-supporting | Fourcier {1,2,4,8,16} = Cayley-Dickson dims; algebra exploration |
| `09_mathematical/algebra/EXPLR_FOURCIER_KINEMATIC_TOPOLOGY.md` | none | KEEP-supporting | Fourcier curve topology; algebra exploration |
| `09_mathematical/algebra/EXPLR_TERNARY_MATRIX_BCC_SNAP_NEGATIVE.md` | III | KEEP-supporting | Ternary matrix BCC snap [CLOSED NEGATIVE]; Part III §III.3 route provenance |
| `09_mathematical/algebra/EXPLR_WALSH_HADAMARD_B2_ALGEBRA.md` | none | KEEP-supporting | Walsh-Hadamard grading; algebra detail |
| `09_mathematical/algebra/PREREG_TERNARY_MATRIX_BCC_SNAP_v1.md` | III | KEEP-supporting | Pre-reg for ternary matrix BCC snap; provenance for EXPLR closed-negative |
| `09_mathematical/fqcr_program/DERIV_FQCR_EM_CONNECTED_RESPONSE.md` | I | KEEP-supporting | Tree-level FQCR connected response → master quadratic x₊; supports Part I §I.2 FQCR |
| `09_mathematical/fqcr_program/EXPLR_FQCR_OBSERVER_TESTS_SUITE.md` | none | KEEP-supporting | FQCR observer tests vs QED running; EFT detail |
| `09_mathematical/general_math/CONJ_ALPHA_FROM_CM.md` | III | KEEP-canonical | x₊=1/α statement [CONJECTURE]; primary Part III §III.1 citation |
| `09_mathematical/general_math/CONJ_SEVEN_TERM_PRECISION_SERIES.md` | III | KEEP-supporting | 7-term series [CONJECTURE]; Part III §III.2 evidence |
| `09_mathematical/general_math/DERIV_BCC_COMPLEX_STRUCTURE.md` | I | KEEP-canonical | BCC complex structure V_complex≅ℤ[i]² [DERIVED] + ℤ[i]^×→O_h^ab no-go (OT-1.5/1.6); cited by Part I §I.4 |
| `09_mathematical/general_math/DERIV_CONJECTURE_16_5_2_CLOSURE.md` | I | KEEP-supporting | Conj 16.5.2 closure [DERIVED] (FTD-0182); supports Part I §I.4 spine |
| `09_mathematical/general_math/DERIV_JONES_INDEX_THRESHOLD_RATIO.md` | none | KEEP-supporting | Jones index conjecture; speculative |
| `09_mathematical/general_math/DERIV_PYTHAGOREAN_FERMAT_BRIDGE.md` | I | KEEP-supporting | Pythagorean-Fermat bridge to master quadratic; supports Part I §I.3 |
| `09_mathematical/general_math/EXPLR_3X3_MIXING_NEGATIVE.md` | III | KEEP-supporting | 3×3 mixing [NEGATIVE]; Part III §III.3 route provenance |
| `09_mathematical/general_math/EXPLR_ALPHA_OVER_42_MASS_GAP.md` | none | KEEP-supporting | α/42 proton correction exploration; speculative |
| `09_mathematical/general_math/EXPLR_COLLAPSE_GRAVITY_BRIDGE.md` | none | KEEP-supporting | Collapse-gravity bridge; speculative |
| `09_mathematical/general_math/EXPLR_COLOR_EXCESS_CLOSED_FORM.md` | III | KEEP-supporting | Color excess [CLOSED NEGATIVE]; Part III §III.3 |
| `09_mathematical/general_math/EXPLR_CURVE_FAMILY_MATHEMATICAL_ANALYSIS.md` | I | KEEP-supporting | FTD curve family → {3,4,7,13}; supports Part I §I.4 |
| `09_mathematical/general_math/EXPLR_FOURIER_CURVE_LEVEL_4.md` | none | KEEP-supporting | Fourier curve level-4; exploration |
| `09_mathematical/general_math/EXPLR_HALF_MOBIUS_LEMNISCATE.md` | I | KEEP-supporting | Z₄ topology from period lattice; supports Part I §I.1 |
| `09_mathematical/general_math/EXPLR_HIGHER_DIM_WATSON.md` | I | KEEP-canonical | Generalised Watson identity W^(D) [THEOREM] (FTD-0156); cited by Part I §I.2 (OT spine) |
| `09_mathematical/general_math/EXPLR_MASTER_QUADRATIC_STRUCTURAL_READINGS.md` | I | KEEP-supporting | Consolidated structural readings of master quadratic; Part I §I.3 context |
| `09_mathematical/general_math/EXPLR_MODULAR_QUADRATIC.md` | I | KEEP-supporting | Is master quadratic a modular equation? [No]; Part I §I.3 |
| `09_mathematical/general_math/EXPLR_NUMBER_THEORY.md` | I | KEEP-supporting | {3,4,7,13} across modular forms; supports Part I §I.4 |
| `09_mathematical/general_math/EXPLR_PATHS_TO_ALPHA.md` | III | KEEP-canonical | Exhaustive α-derivation survey: no new path [SURVEY]; primary Part III §III.3 citation |
| `09_mathematical/general_math/EXPLR_POLYNOMIAL_LOOK_ELSEWHERE.md` | III | KEEP-canonical | 147k-polynomial scan: master quadratic is unique dual-matcher; Part III §III.2 |
| `09_mathematical/general_math/EXPLR_RELU_TYPE_TRANSITION.md` | none | KEEP-supporting | Softplus β interpolates von Neumann factors; speculative |
| `09_mathematical/general_math/EXPLR_SPECTRAL_ARTIFACT_DISCOVERY.md` | none | KEEP-supporting | 2D-FFT gauge-group artifact [honest negative]; methodology |
| `09_mathematical/general_math/EXPLR_SPECTRAL_CIRCLE_TO_LEMNISCATE.md` | none | KEEP-supporting | Born rule as Joukowski transform; speculative |
| `09_mathematical/general_math/EXPLR_TOPOLOGICAL_DRAG_ALPHA.md` | III | KEEP-supporting | Topological-drag α [CLOSED NEGATIVE tautology]; Part III §III.3 |
| `09_mathematical/general_math/MATH_ANTI_CORRELATION_THEOREM.md` | I | KEEP-supporting | ζ(s)/β(s) alternation theorem [THEOREM]; supports Part I §I.4 spine |
| `09_mathematical/general_math/MATH_FAMILY_OF_RACES.md` | I | KEEP-supporting | R_q=Γ(1/q)/Γ(1−1/q) family; R_4=G* supports Part I §I.2 |
| `09_mathematical/general_math/NODE_MAP_FTD_MATH.md` | I | KEEP-supporting | Node map (duplicate in general_math); Part I §I.5 DAG figure source |
| `09_mathematical/general_math/PREREG_CATALAN_INDEPENDENCE_v1.md` | none | KEEP-supporting | Catalan independence pre-reg; active pre-reg (FTD-0162 related) |
| `09_mathematical/general_math/PREREG_SYM_K_C_INVARIANT_PARITY_V1.md` | I | KEEP-supporting | Sym^k eigenline pre-reg (FTD-0177–0182); provenance for Part I §I.4 |
| `09_mathematical/general_math/PROOF_ALPHA_FROM_SELF_DUALITY.md` | I | KEEP-canonical | α⁻¹ from CM elliptic curve via master quadratic [THEOREM steps 1–6]; primary Part I §I.3 proof |
| `09_mathematical/general_math/PROPOSAL_OBSERVER_OPERATOR_EXTENSION.md` | none | KEEP-supporting | Observer extension proposal; speculative |
| `09_mathematical/general_math/REF_GUILLERA_CORPUS_MAP.md` | I | KEEP-supporting | Guillera Ramanujan-type series corpus; supports Part I §I.2 G* computation routes |
| `09_mathematical/general_math/REF_QCR_TRILOGY_BRIDGE.md` | I | KEEP-supporting | External QCR trilogy mapped to FQCR; Part I FQCR context |
| `09_mathematical/general_math/ROADMAP_IDENTITY_PRIORITIES.md` | none | KEEP-supporting | Identity priorities roadmap; research-queue context |
| `09_mathematical/general_math/THEOREM_BCC_WATSON_REFLECTION_BRIDGE.md` | I | KEEP-supporting | BCC Green's function ↔ reflection ratio [THEOREM]; supports Part I §I.2 |
| `09_mathematical/number_theory/DERIV_INTEGER_4_UNIFICATION.md` | I | KEEP-canonical | Lemniscatic 4s + |μ_K|=|disc(K)| uniqueness [THEOREM] (FTD-0181); Part I §I.4 (OT-1.9) |
| `09_mathematical/number_theory/DERIV_LFUNCTION_GSTAR_CONNECTION.md` | I | KEEP-canonical | G*=8·L(E,1)/√π [THEOREM]; Part I §I.2 core identity |
| `09_mathematical/number_theory/DERIV_MASTER_QUADRATIC_CM_LVALUES.md` | I | KEEP-canonical | Coefficients as Deligne L-values: 16G*²=2⁹·L(Sym²E,1) [THEOREM]; Part I §I.3 (OT-2.5) |
| `09_mathematical/number_theory/DERIV_MASTER_QUADRATIC_FROM_PERIOD_ALGEBRA.md` | I | KEEP-canonical | Master quadratic from motivic symmetric period algebra [THEOREM]; Part I §I.3 |
| `09_mathematical/number_theory/DERIV_MODE_ERASURE_AND_SPIN_ALGEBRA.md` | none | KEEP-supporting | Mode-erasure theorem; physics detail |
| `09_mathematical/number_theory/EXPLR_CHOWLA_SELBERG_HIGHER_H.md` | I | KEEP-canonical | Analytic machinery to upgrade Theorem 3 to h≥2 (OT-2.3 upgrade path); Part I §I.4 |
| `09_mathematical/number_theory/EXPLR_CM_RATIO_TOWER.md` | I | KEEP-supporting | 9-element h=1 tower; supports Part I §I.4 CM uniqueness |
| `09_mathematical/number_theory/EXPLR_EULER_RATIO_RICCI_FLOW.md` | none | KEEP-supporting | Euler reflection ratio and Ricci flow; speculative |
| `09_mathematical/number_theory/EXPLR_GAUSSIAN_EISENSTEIN_DICHOTOMY.md` | II | KEEP-supporting | Gaussian/Eisenstein dichotomy (FTD-0237); MC-T4.3 trace/det odd-term gap reframe; Part II §II.2 |
| `09_mathematical/number_theory/EXPLR_GSTAR_ARITHMETIC_IDENTITIES.md` | I | KEEP-supporting | Consolidated G* arithmetic identities; supports Part I §I.2 |
| `09_mathematical/number_theory/EXPLR_LVALUE_SPAN_CORRECTION_SEARCH.md` | III | KEEP-supporting | L-value span search for CODATA gap [NEGATIVE]; Part III §III.2 |
| `09_mathematical/number_theory/EXPLR_ONTIC_CONSTANT_ATLAS.md` | I | KEEP-supporting | G*=3 fixed-point atlas; Part I §I.2 context |
| `09_mathematical/number_theory/EXPLR_RIEMANN_ZETA_CONNECTION.md` | none | KEEP-supporting | FTD–Riemann-zeta connections [mostly fits]; methodology |
| `09_mathematical/number_theory/EXPLR_SYM_PERIOD_ALGEBRA_CONVENTIONS.md` | I | KEEP-supporting | Sym^k conventions; supports Part I §I.4 period algebra |
| `09_mathematical/number_theory/EXPLR_TOWER_MULTIPLIER_UNIQUENESS.md` | III | KEEP-supporting | (m=2,k=4) uniqueness in (1+i)-tower [STRUCTURAL]; Part III §III.2 evidence |
| `09_mathematical/number_theory/MATH_LOG_GSTAR_IDENTITY.md` | I | KEEP-supporting | log G* expansion [THEOREM]; supports Part I §I.2 |

### 10_eft_program — EFT Recovery Program (57 docs)

| path | backs-Part | disposition | one-line reason |
|---|---|---|---|
| `10_eft_program/INDEX_FTD_NATIVE_EFT.md` | none | KEEP-canonical | Cluster navigation index |
| `10_eft_program/REF_PREREGISTER_MANIFEST.md` | none | KEEP-canonical | Pre-registration manifest; active provenance record |
| `10_eft_program/derivations/DERIV_ALPHA_READOUT_BOUNDARY.md` | II | KEEP-supporting | ARC-A1 v2 [UNDERDETERMINED]: Trace forced, Det unforced; Part II §II.2 Route evidence |
| `10_eft_program/derivations/DERIV_ALPHA_READOUT_C1_QUANTIZATION.md` | II | KEEP-supporting | ARC-C1 [UNDERDETERMINED]: winding-based readout; Part II §II.2 Route evidence |
| `10_eft_program/derivations/DERIV_ALPHA_READOUT_EMPIRICAL.md` | II | KEEP-supporting | ARC-D1 [CLOSED NEGATIVE]: engine-native measurement; Part II §II.4 last-resort route |
| `10_eft_program/derivations/DERIV_BCC_ALGEBRAIC_READOUT.md` | II | KEEP-supporting | ARC-B2 BCC algebraic readout [DERIVED/PARTIAL]; Part II §II.2 Route evidence |
| `10_eft_program/derivations/DERIV_EMERGENT_COULOMB_GEOMETRIC.md` | I | KEEP-canonical | V(r) is geometric Coulomb, zero fine-structure content [THEOREM]; Phase G result, Part I §I.4 |
| `10_eft_program/derivations/DERIV_EMERGENT_U1_FROM_FLUX_PROJECTION.md` | none | KEEP-supporting | U(1) as effective description of projected flux; EFT ontology |
| `10_eft_program/derivations/DERIV_FTD_NATIVE_COMPLETE_HISTORY_ACTION.md` | none | KEEP-supporting | Microscopic history action; EFT detail |
| `10_eft_program/derivations/DERIV_FTD_NATIVE_NONLINEAR_FLOW.md` | none | KEEP-supporting | Native RG flow; EFT detail |
| `10_eft_program/derivations/DERIV_FTD_NATIVE_RESPONSE_AND_BLOCKING.md` | none | KEEP-supporting | Bare linear response + b=2 blocking; EFT detail |
| `10_eft_program/derivations/DERIV_J_BILINEAR_NO_SPIN2_POLE.md` | none | KEEP-supporting | J-bilinear no-spin-2-pole theorem; EFT detail |
| `10_eft_program/derivations/DERIV_PARTITION_FUNCTION_L2.md` | I | KEEP-canonical | Phase J partition function ultralocality [THEOREM at L=2] (OT-3.1); cited by Part I close |
| `10_eft_program/derivations/DERIV_SPIN2_BOUNDARY_THEOREM_FREE_THEORY.md` | II | KEEP-supporting | Spin-2 boundary theorem free theory; Part II §II.2 boundary evidence |
| `10_eft_program/derivations/DERIV_STATE_FLUX_TO_EFT_DICTIONARY.md` | none | KEEP-supporting | State↔flux EFT dictionary; EFT detail |
| `10_eft_program/derivations/FOUND_ALPHA_READOUT_QUANTIZATION_RESOLUTION.md` | II | KEEP-supporting | ARC-C (quantization) FOUND→UNDERDETERMINED (FTD-0231); Part II §II.2 provenance |
| `10_eft_program/derivations/FOUND_BCC_ALGEBRAIC_READOUT_RESOLUTION.md` | II | KEEP-supporting | ARC-B2 FOUND→UNDERDETERMINED (FTD-0230); Part II §II.2 provenance |
| `10_eft_program/derivations/FOUND_COLOR_CONFINEMENT_RESOLUTION.md` | none | KEEP-supporting | Color confinement [FOUND] (FTD-0217); physics |
| `10_eft_program/derivations/FOUND_DM_BARYON_W5_CONFIRMATION.md` | none | KEEP-supporting | DM/baryon W5 [FOUND] (FTD-0219); physics |
| `10_eft_program/derivations/FOUND_LEMNISCATIC_K2_REGULATOR.md` | I | KEEP-supporting | Lemniscatic K₂ regulator [FOUND] (FTD-0222); supports Part I §I.4 spine |
| `10_eft_program/derivations/FOUND_NO_4TH_GENERATION_NO_GO.md` | none | KEEP-supporting | No 4th generation [THEOREM] (FTD-0220); physics detail |
| `10_eft_program/derivations/FOUND_READOUT_STRUCTURE_INDEPENDENCE.md` | II | KEEP-supporting | Readout structure independence [FOUND/SELECTION]; Part II §II.2 boundary provenance |
| `10_eft_program/derivations/FOUND_SPIN2_BOUNDARY_THEOREM.md` | II | KEEP-supporting | Spin-2 boundary theorem [FOUND]; Part II §II.2 commutativity-wall evidence |
| `10_eft_program/derivations/FOUND_STOCHASTIC_EFFECTIVE_ACTION_RESOLUTION.md` | none | KEEP-supporting | Stochastic effective action [FOUND] (FTD-0218); EFT |
| `10_eft_program/derivations/THEOREM_A_PHYS_NO_GO.md` | 0 | KEEP-canonical | No a_phys derivable from Axiom Zero [THEOREM] (FTD-0059); cited by Part 0 §0.1 |
| `10_eft_program/derivations/THEOREM_BLOCKING_DIAGONAL_IDENTITIES.md` | none | KEEP-supporting | M_JJ=16, M_J⁴=256 exact [THEOREM]; EFT infrastructure |
| `10_eft_program/derivations/THEOREM_COMMUTATIVITY_INDEPENDENCE.md` | II | KEEP-canonical | Commutativity independence [THEOREM] (supersedes SYNTHESIS_COMMUTATIVITY); primary Part II §II.2 citation |
| `10_eft_program/derivations/THEOREM_MU_NO_GO_FTD0096.md` | I | KEEP-canonical | Mass-unit μ not derivable [THEOREM/CLOSED NEGATIVE] (FTD-0096); Part I §I.4 closes T4.5 |
| `10_eft_program/general/EXPLR_DM_BARYON_W5_WEIGHTING.md` | none | KEEP-supporting | DM/baryon W5 weighting; physics detail |
| `10_eft_program/preregistrations/PREREG_ADVERSARIAL_LOOK_ELSEWHERE_v1.md` | III | KEEP-canonical | FTD-0189 adversarial look-elsewhere pre-reg; provenance for Part III §III.2 |
| `10_eft_program/preregistrations/PREREG_ALPHA_ARITHMETIC_GENERATIVITY_v1.md` | III | KEEP-supporting | Alpha generativity pre-reg (FTD-0185); Part III §III.4 gap-closer context |
| `10_eft_program/preregistrations/PREREG_ALPHA_READOUT_BCC_BRIDGE_v1.md` | II | KEEP-supporting | ARC-B2 BCC readout pre-reg (FTD-0230); Part II §II.2 provenance |
| `10_eft_program/preregistrations/PREREG_ALPHA_READOUT_BOUNDARY_v2.md` | II | KEEP-supporting | ARC-A1 v2 pre-reg (FTD-0238); Part II §II.2 provenance |
| `10_eft_program/preregistrations/PREREG_ALPHA_READOUT_DETERMINANT_GRADING_v1.md` | II | KEEP-supporting | ARC determinant grading pre-reg (FTD-0233, CLOSED NEGATIVE); Part II §II.2 provenance |
| `10_eft_program/preregistrations/PREREG_ALPHA_READOUT_DET_IDENTITY_v1.md` | II | KEEP-supporting | det↔det_ζ identity pre-reg (FTD-0235); Part II §II.2 provenance |
| `10_eft_program/preregistrations/PREREG_ALPHA_READOUT_OBSERVABLE_SELECTION_v1.md` | II | KEEP-supporting | ARC-B1 observable-selection pre-reg (FTD-0198); Part II §II.2 provenance |
| `10_eft_program/preregistrations/PREREG_ALPHA_READOUT_ODD_PERIOD_v1.md` | II | KEEP-supporting | ARC odd-period pre-reg (FTD-0234); Part II §II.2 provenance |
| `10_eft_program/preregistrations/PREREG_ALPHA_READOUT_QUANTIZATION_v1.md` | II | KEEP-supporting | ARC quantization pre-reg (FTD-0231); Part II §II.2 provenance |
| `10_eft_program/preregistrations/PREREG_COLOR_CONFINEMENT_v1.md` | none | KEEP-supporting | Color confinement pre-reg (FTD-0217); physics provenance |
| `10_eft_program/preregistrations/PREREG_COMMUTATIVITY_DERIVATION_v1.md` | II | KEEP-supporting | Commutativity derivation pre-reg; Part II §II.2 provenance |
| `10_eft_program/preregistrations/PREREG_COMMUTATIVITY_INDEPENDENCE_v1.md` | II | KEEP-supporting | Commutativity independence pre-reg; Part II §II.2 provenance |
| `10_eft_program/preregistrations/PREREG_DM_BARYON_W5_INDEPENDENT_CONFIRMATION_v1.md` | none | KEEP-supporting | DM/baryon W5 confirmation pre-reg; physics provenance |
| `10_eft_program/preregistrations/PREREG_FQCR_QUOTIENT_UNIQUENESS_v1.md` | I | KEEP-supporting | FQCR Model IV uniqueness scan pre-reg (FTD-0143); Part I §I.2 FQCR provenance |
| `10_eft_program/preregistrations/PREREG_FTD_0110_NONLINEAR_BRIDGE_v1.md` | none | KEEP-supporting | Nonlinear bridge pre-reg (FTD-0215); EFT provenance |
| `10_eft_program/preregistrations/PREREG_GRAVITON_SUBSTRATE_MODE_v1.md` | none | KEEP-supporting | Graviton mode v1 pre-reg (superseded by v2; retained for provenance) |
| `10_eft_program/preregistrations/PREREG_GRAVITON_SUBSTRATE_MODE_v2.md` | none | KEEP-supporting | Graviton mode v2 active pre-reg; Frontier 4 |
| `10_eft_program/preregistrations/PREREG_LEMNISCATIC_K2_REGULATOR_v1.md` | I | KEEP-supporting | K₂ regulator pre-reg (FTD-0222); Part I provenance |
| `10_eft_program/preregistrations/PREREG_MANIFESTATION_NONCOMMUTATIVITY_v1.md` | none | KEEP-supporting | Manifestation noncommutativity pre-reg; EFT provenance |
| `10_eft_program/preregistrations/PREREG_MODULAR_TIME_ALGEBRA_TYPE_v1.md` | none | KEEP-supporting | Modular time algebra pre-reg; EFT provenance |
| `10_eft_program/preregistrations/PREREG_NO_4TH_GENERATION_NO_GO_v1.md` | none | KEEP-supporting | No 4th generation pre-reg (FTD-0220); provenance |
| `10_eft_program/preregistrations/PREREG_READOUT_STRUCTURE_INDEPENDENCE_v1.md` | II | KEEP-supporting | Readout structure independence pre-reg; Part II provenance |
| `10_eft_program/preregistrations/PREREG_SCALE0_SUBSTRATE_PROTOCOL_v1.md` | none | KEEP-supporting | Scale-0 substrate v1 (superseded by v2; retained for provenance) |
| `10_eft_program/preregistrations/PREREG_SCALE0_SUBSTRATE_PROTOCOL_v2.md` | none | KEEP-supporting | Scale-0 substrate v2 active pre-reg |
| `10_eft_program/preregistrations/PREREG_SPEKKENS_KNOWLEDGE_BALANCE_v1.md` | none | KEEP-supporting | Spekkens knowledge balance pre-reg; QM provenance |
| `10_eft_program/preregistrations/PREREG_SPIN2_BOUNDARY_THEOREM_v1.md` | II | KEEP-supporting | Spin-2 boundary theorem pre-reg; Part II provenance |
| `10_eft_program/preregistrations/PREREG_STOCHASTIC_EFFECTIVE_ACTION_v1.md` | none | KEEP-supporting | Stochastic effective action pre-reg (FTD-0218); EFT provenance |
| `10_eft_program/preregistrations/PREREG_STRONG_FIELD_GRAVITY_v1.md` | none | KEEP-supporting | Strong-field gravity pre-reg; Frontier 4 |
| `10_eft_program/preregistrations/PREREG_STRUCTURAL_DYNAMICAL_DISCRIMINATOR_v1.md` | II | KEEP-supporting | Discriminator v1 pre-reg (invalidated, retained as provenance); Part II §II.2 |
| `10_eft_program/preregistrations/PREREG_STRUCTURAL_DYNAMICAL_DISCRIMINATOR_v2.md` | II | KEEP-supporting | Discriminator v2 active pre-reg (FTD-0186); Part II §II.2 |
| `10_eft_program/preregistrations/PREREG_SYMPLECTIC_BUDGET_SYMMETRY_v1.md` | none | KEEP-supporting | Symplectic budget symmetry pre-reg; EFT provenance |
| `10_eft_program/preregistrations/PREREG_X_MINUS_PHYSICAL_IDENTIFICATION_v1.md` | III | KEEP-supporting | x₋ physical ID pre-reg (FTD-0210, CLOSED NEGATIVE); Part III §III.3 |
| `10_eft_program/reports_and_audits/ANALYSIS_NONLINEAR_BRIDGE_SWEEPS.md` | none | KEEP-supporting | F-D3 parameter sweep analysis; EFT |
| `10_eft_program/reports_and_audits/REPORT_GRAVITON_SUBSTRATE_MODE.md` | none | KEEP-supporting | Graviton mode measurement report (FTD-0193); Frontier 4 |
| `10_eft_program/reports_and_audits/RETROSPECTIVE_EFT_RECOVERY.md` | none | KEEP-supporting | Narrative roll-up of whole EFT program [SYNTHESIS]; active reference |
| `10_eft_program/reports_and_audits/STATUS_EFT_CHECKLIST.md` | none | KEEP-supporting | EFT checklist; active open-items tracker |
| `10_eft_program/reports_and_audits/STATUS_NONLINEAR_REGIME_2026-04-30.md` | none | KEEP-supporting | Nonlinear regime handoff; EFT context |
| `10_eft_program/scopes_and_specs/OPEN_FTD_NATIVE_ACTION_OR_MEASURE.md` | none | KEEP-supporting | Native action/measure gap (FTD-0059 bridge); EFT open item |
| `10_eft_program/scopes_and_specs/OPEN_FTD_TO_EFT_BRIDGE_STATUS.md` | III | KEEP-canonical | QED-α bridge failure pivot document; primary Part III §III.4 citation (why ARC-D matters) |
| `10_eft_program/scopes_and_specs/OPEN_GC_FROM_FIRST_PRINCIPLES.md` | none | KEEP-supporting | g_c open problem; EFT open item |
| `10_eft_program/scopes_and_specs/SCOPE_ALPHA_READOUT_NEXT_STEPS.md` | II | KEEP-supporting | Next-steps after route closures; Part II §II.4 context |
| `10_eft_program/scopes_and_specs/SCOPE_DERIVE_QM_GAP.md` | none | KEEP-supporting | QM-gap derivation scope; EFT |
| `10_eft_program/scopes_and_specs/SCOPE_DET_IDENTITY_ATTACK_v1.md` | II | KEEP-supporting | det-identity attack scope (FTD-0240); Part II §II.2 provenance |
| `10_eft_program/scopes_and_specs/SCOPE_DISCRETE_NATIVE_COMPARATOR.md` | none | KEEP-supporting | Discrete-native comparator scope; EFT |
| `10_eft_program/scopes_and_specs/SCOPE_FTD_0110_NONLINEAR_BRIDGE.md` | none | KEEP-supporting | FTD-0110 nonlinear bridge scope; EFT open item |
| `10_eft_program/scopes_and_specs/SCOPE_GC_QUANTUM_PATH_INTEGRAL.md` | none | KEEP-supporting | g_c quantum path integral scope; EFT |
| `10_eft_program/scopes_and_specs/SCOPE_ROUTE_B_MODULAR_TIME.md` | none | KEEP-supporting | Route B modular time scope; EFT |
| `10_eft_program/scopes_and_specs/SCOPE_SPIN2_BOUNDARY_THEOREM.md` | none | KEEP-supporting | Spin-2 boundary theorem scope; EFT |
| `10_eft_program/scopes_and_specs/SPEC_EFT_RECOVERY_PROGRAM.md` | none | KEEP-canonical | Original Phase 0–F pre-registration; EFT program reference |
| `10_eft_program/scopes_and_specs/SPEC_FTD_DYNAMICAL_SU3_HADRODYNAMICS.md` | none | KEEP-supporting | SU(3) hadrodynamics spec; EFT detail |
| `10_eft_program/scopes_and_specs/SPEC_FTD_EFT_BRIDGE_CONTRACT.md` | none | KEEP-supporting | 7-gate EFT bridge contract; EFT methodology |
| `10_eft_program/scopes_and_specs/SPEC_FTD_NATIVE_BLOCKING_MAP.md` | none | KEEP-supporting | b=2 blocking map; EFT detail |
| `10_eft_program/scopes_and_specs/SPEC_FTD_NATIVE_ELECTRODYNAMICS.md` | none | KEEP-supporting | Native EM spec post-pivot; EFT |
| `10_eft_program/scopes_and_specs/SPEC_OPERATOR_BASIS_COMPLETE.md` | none | KEEP-supporting | Gate-3 operator basis closure; EFT |
| `10_eft_program/scopes_and_specs/SPEC_WILSON_DIRAC_FTD.md` | none | KEEP-supporting | Wilson-Dirac matter sector; EFT |

### Top-level (3 docs)

| path | backs-Part | disposition | one-line reason |
|---|---|---|---|
| `META_INDEX.md` | none | KEEP-canonical | Master catalog; primary navigation layer; updated by Phase 1b-archive moves |
| `META_STRUCTURE.md` | none | KEEP-canonical | Placement and archive rules; active structural reference |
| `STRATEGY_PAPER_SPLIT_2026-04-30.md` | none | KEEP-supporting | Paper-split strategy [RECOMMENDATION, not yet acted on]; active planning |

---

## §4 — Disposition tally

| Disposition | Count |
|---|---|
| KEEP-canonical | 65 |
| KEEP-supporting | 348 |
| ARCHIVE-absorbed | 6 |
| ARCHIVE-superseded | 2 |
| ARCHIVE-closed-negative-scratch | 0 |
| **Total** | **421** |

Row count equals inventory count: **421 = 421**. No double-counting; no REVIEW rows.

---

## §5 — Absorbed set (explicitly seeded per plan + additional discovered)

These are the docs proposed for `ARCHIVE-absorbed` or `ARCHIVE-superseded`. The owner confirms this set before any `git mv`. Monograph replaces their narrative function; provenance preserved by archiving.

| File (relative to `docs/theory/`) | Current LEDGER/tag | Archive reason |
|---|---|---|
| `07_assessment/SYNTHESIS_GSTAR_BEDROCK_2026-05-19.md` | [SYNTHESIS] (FTD-0176 context) | Polymath synthesis on G* bedrock; the monograph Part I §I.2 absorbs its narrative |
| `07_assessment/SYNTHESIS_COMMUTATIVITY_BOUNDARY_2026-05-30.md` | [SYNTHESIS] (FTD-0238) | Commutativity-wall [SYNTHESIS]; superseded by THEOREM_COMMUTATIVITY_INDEPENDENCE (FTD-0238 promoted to [THEOREM]) — monograph Part II §II.6 absorbs the construction narrative |
| `07_assessment/ROUNDTABLE_STATE_OF_FTD_2026-05-22.md` | [SYNTHESIS] | State-of-theory roundtable; monograph is the canonical construction story that replaces it |
| `07_assessment/AUDIT_SESSION_2026_04_24.md` | session artifact | Per-session numerical verification; absorbed into canonical trackers (LEDGER, TRACKER_ONTIC_TRUTH); no live claim relies on it exclusively |
| `07_assessment/STATUS_2026-05-04_post_bughunt.md` | session artifact | Engine status snapshot; superseded by current LEDGER + WHERE_WE_LEFT_OFF; no live claim relies on it exclusively |
| `07_assessment/campaigns/archive_session_outputs/SESSION_SYNTHESIS_2026-04-30.md` | session synthesis | April 30 session synthesis; content rolled into LEDGER entries and later canonical docs |
| `04_coupling/DERIV_ALPHA_READOUT_RESOLUTION.md` | RETRACTED (banner 2026-06-01) | Explicitly retracted substitution-identity facade; archive under `closed_negative` for provenance |

**Archive-target subdirectory recommendations:**
- `SYNTHESIS_*` + `ROUNDTABLE_*` → `docs/theory/07_assessment/archive/` (or a new `07_assessment/archive/absorbed/`)
- `AUDIT_SESSION_*` + `STATUS_*` + `SESSION_SYNTHESIS_*` → `docs/theory/07_assessment/archive/session_artifacts/`
- `DERIV_ALPHA_READOUT_RESOLUTION.md` → `docs/theory/04_coupling/archive/retracted/`

**Note on SYNTHESIS_COMMUTATIVITY_BOUNDARY:** this doc is still referenced by active files (`SCOPE_DET_IDENTITY_ATTACK_v1.md`, `DERIV_ALPHA_READOUT_EMPIRICAL.md`, `PREREG_READOUT_STRUCTURE_INDEPENDENCE_v1.md`, LEDGER.md). The repoint step in Phase 1b-archive must redirect those references to `THEOREM_COMMUTATIVITY_INDEPENDENCE.md` before the `git mv` is executed.

---

## §6 — Canonical-anchor existence check

All 14 spec-§11 canonical anchors confirmed present on disk:

| Anchor | Status |
|---|---|
| `docs/theory/07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md` | PRESENT |
| `docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md` | PRESENT |
| `docs/theory/07_assessment/core_ledgers/LEDGER.md` | PRESENT |
| `docs/theory/01_reference/SPEC_DOCTRINE_LEDGER.md` | PRESENT |
| `docs/theory/01_reference/SPEC_OPEN_MATH_BY_SECTOR.md` | PRESENT |
| `docs/theory/07_assessment/audits/AUDIT_ALPHA_OPERATOR_FORCING_ROUTE_INVARIANCE.md` | PRESENT |
| `docs/theory/07_assessment/audits/AUDIT_RSI_LEG3_CONDITIONAL_THEOREM.md` | PRESENT |
| `dissemination/papers/PAPER_A_PI_FREE_GENERATOR.tex` | PRESENT |
| `dissemination/papers/PAPER_B_BCC_COMPLEX_STRUCTURE.tex` | PRESENT |
| `dissemination/papers/PAPER_FTD_AS_WILSONIAN_EFT.tex` | PRESENT |
| `dissemination/papers/PAPER_MASTER_QUADRATIC_AND_BRIDGE.tex` | PRESENT |
| `scripts/constants.py` | PRESENT |
| `engine/include/ftd/ontic.h` | PRESENT |
| `engine/web/js/constants.js` | PRESENT |

**No missing anchors.**

---

## §7 — Navigation layers to touch when archive moves execute

These are the files that reference one or more of the ARCHIVE-proposed docs and will need path updates in the same commit:

### References to `SYNTHESIS_GSTAR_BEDROCK_2026-05-19.md`

- `docs/theory/07_assessment/AUDIT_GSTAR_PAPER_MULTI_ROUND.md` (lines 13, 87, 276) — citation references; update to new archive path
- `docs/theory/07_assessment/INDEX_07_ASSESSMENT.md` (table row) — remove from active list or note as archived

### References to `SYNTHESIS_COMMUTATIVITY_BOUNDARY_2026-05-30.md`

- `docs/theory/10_eft_program/scopes_and_specs/SCOPE_DET_IDENTITY_ATTACK_v1.md` — redirect to `THEOREM_COMMUTATIVITY_INDEPENDENCE.md`
- `docs/theory/10_eft_program/derivations/DERIV_ALPHA_READOUT_EMPIRICAL.md` — redirect to `THEOREM_COMMUTATIVITY_INDEPENDENCE.md`
- `docs/theory/10_eft_program/preregistrations/PREREG_READOUT_STRUCTURE_INDEPENDENCE_v1.md` — redirect to `THEOREM_COMMUTATIVITY_INDEPENDENCE.md`
- `docs/theory/07_assessment/core_ledgers/LEDGER.md` (FTD-0238 path entry) — update path
- `docs/theory/10_eft_program/derivations/THEOREM_COMMUTATIVITY_INDEPENDENCE.md` (header "Supersedes status of:" line) — update path to archive
- `docs/theory/10_eft_program/preregistrations/PREREG_COMMUTATIVITY_INDEPENDENCE_v1.md` — update two references

### References to `ROUNDTABLE_STATE_OF_FTD_2026-05-22.md`

- `docs/theory/META_INDEX.md` (catalog section) — update to note archived
- `docs/theory/07_assessment/AUDIT_HIDDEN_SELECTIONS.md` (inline reference) — update to archive path
- `docs/theory/07_assessment/INDEX_07_ASSESSMENT.md` (table row) — remove from active list or note as archived
- `docs/theory/08_structural/PREREG_FINITE_NEUTRAL_LOCK_v1.md` — update reference
- `docs/theory/01_reference/SPEC_FTD_REFERENCE.md` — update reference to monograph or new path

### References to `AUDIT_SESSION_2026_04_24.md`

- No external references found by grep; safe to move without repointing.

### References to `STATUS_2026-05-04_post_bughunt.md`

- No external references found by grep; safe to move without repointing.

### References to `SESSION_SYNTHESIS_2026-04-30.md`

- No external references found by grep; safe to move without repointing.

### References to `DERIV_ALPHA_READOUT_RESOLUTION.md`

- `docs/theory/07_assessment/core_ledgers/LEDGER.md` — if there is a LEDGER row pointing to this path, update to archive path; retraction banner already in-file.
- Grep `docs/theory` for `DERIV_ALPHA_READOUT_RESOLUTION` to catch any additional references before moving.

### Navigation layers that always need touching after any archive move

1. `docs/theory/META_INDEX.md` — master catalog; remove/refile any row pointing to moved docs
2. Relevant local `INDEX_*.md` (especially `docs/theory/07_assessment/INDEX_07_ASSESSMENT.md` and `docs/theory/04_coupling/INDEX_04_COUPLING.md`) — remove from active tables
3. `docs/theory/07_assessment/core_ledgers/LEDGER.md` — path-only updates for any moved file with a LEDGER row
4. `docs/theory/07_assessment/core_ledgers/TRACKER_OPEN_ITEMS.md` — confirm no moved file is an open-item anchor
5. `.gitignore` — add exception if any top-level archived file needs to be tracked

---

## §8 — Link-checker result (post-audit)

Run immediately after writing this file:

```
python scripts/verification/verify_index_links.py
```

Expected: `Broken: 0` (this audit adds no internal links to theory docs).

*(See acceptance-check section below for the pasted tail.)*

---

## §9 — Self-review

- **Row count = inventory count?** Yes: 421 rows (summing all cluster table rows) = 421 active docs.
- **Any REVIEW rows?** None. Every doc resolved to a concrete disposition.
- **Any tags edited?** No. This is a read-only classification.
- **Any files moved?** No.
- **No ARCHIVE-closed-negative-scratch rows?** Correct. The genuinely closed-negative exploration docs in `10_eft_program/archive/` are already archived. The active closed-negative scratch docs in `10_eft_program/derivations/` and `07_assessment/audits/` carry UNDERDETERMINED or CLOSED-NEGATIVE tags but are load-bearing provenance for Part II/III — they stay KEEP-supporting, not proposed for additional archiving. The only properly-closed session artifacts are the six ARCHIVE-* rows above.

---

*End of audit.*
