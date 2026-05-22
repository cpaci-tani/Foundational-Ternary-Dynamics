# INDEX · Core Physics Derivations

**Tag:** [REFERENCE]
**Date:** 2026-05-22
**Status:** [REFERENCE] — local navigation index for `docs/theory/03_derivations/`.
**Purpose:** This is the largest theory cluster — the working derivations that turn the FTD axioms and the algebraic spine into physics: the master quadratic and its consequences, the Standard Model gauge sectors, QED/QFT machinery, gravity, quantum-mechanics foundations, the G\* provenance chain, and the FTD-0110 cluster-mass bridge. Read this cluster when you need the explicit chain behind a physics claim; check each doc's own epistemic tag, because most mix [THEOREM]/[DERIVED] algebra with [SELECTION]/[CONJECTURE] physical identifications.

---

## Read first

Newcomers should read these in order:

1. [DERIV_BOTTOM_UP_PHYSICS.md](DERIV_BOTTOM_UP_PHYSICS.md) — the entry-point narrative: from `0 = (−1) + (+1)` to the Standard Model.
2. [DERIV_MASTER_QUADRATIC_GAP_EQUATION.md](DERIV_MASTER_QUADRATIC_GAP_EQUATION.md) — canonical statement of the master quadratic: algebraic identity + physical match.
3. [DERIV_FORCE_EMERGENCE.md](DERIV_FORCE_EMERGENCE.md) — the four forces as regimes of one lattice Green's function.
4. [DERIV_QFT_GRT_BRIDGE.md](DERIV_QFT_GRT_BRIDGE.md) — lattice propagators + stress-energy tensor; the shared QFT/GR foundation other derivations depend on.
5. [DERIV_QM_FROM_LATTICE.md](DERIV_QM_FROM_LATTICE.md) — quantum mechanics as statistics of lattice events.
6. [DERIV_K_FROM_OH_A1G_MULTIPLICITY.md](DERIV_K_FROM_OH_A1G_MULTIPLICITY.md) — the cluster-mass bridge (FTD-0110): `k = 1/N_base` from O_h representation theory.

---

## Master quadratic and the framework integers

The algebraic core: the polynomial `x² − 16G*²x + 16G*³ = 0`, its provenance, and what its roots mean.

| File | Tag | Purpose |
|---|---|---|
| [DERIV_MASTER_QUADRATIC_GAP_EQUATION.md](DERIV_MASTER_QUADRATIC_GAP_EQUATION.md) | [THEOREM] algebra / [STRONGLY MOTIVATED CONJECTURE] physical ID | Canonical master-quadratic reference: identity, coefficient routes, discriminant trichotomy, dual match. |
| [DERIV_MASTER_QUADRATIC_FROM_Z.md](DERIV_MASTER_QUADRATIC_FROM_Z.md) | [THEOREM] polynomial / route RETRACTED (FTD-0032) | Partition-function route to the gap equation; the L→∞ derivation is retracted, polynomial preserved. |
| [DERIV_GAP_EQUATION_FORM.md](DERIV_GAP_EQUATION_FORM.md) | [THEOREM given self-consistency prescription] | Why the two coefficients are linked: one-loop self-consistency forces `x² = K(x − G*)`. |
| [DERIV_QUADRATIC_NECESSITY.md](DERIV_QUADRATIC_NECESSITY.md) | [DERIVATION] (two proofs) | Two independent arguments that degree 2 is structurally forced, not chosen. |
| [DERIV_CHARGE_QUARTIC_FROM_GSTAR.md](DERIV_CHARGE_QUARTIC_FROM_GSTAR.md) | [THEOREM] (6) + [SELECTION] (1) | The charge quartic: master quadratic viewed in charge space `e² = 1/x`. |
| [DERIV_INTEGER_PHYSICAL_IDENTIFICATION.md](DERIV_INTEGER_PHYSICAL_IDENTIFICATION.md) | [THEOREM] given N_gen = N_c [SELECTION] | Traces each framework integer {3, 4, 7, 13} to its physical role. |
| [DERIV_NC_FROM_TOPOLOGY.md](DERIV_NC_FROM_TOPOLOGY.md) | [THEOREM] for N_c=3 invariant / [SELECTION] for QCD ID | N_c = 3 from four independent topological routes on the Moore neighborhood. |
| [THEOREM_HARMONIC_INVARIANT_TOWER.md](THEOREM_HARMONIC_INVARIANT_TOWER.md) | [THEOREM] + [DERIVED] | The (1+i)-tower of master quadratics; harmonic invariant `1/y₊ + 1/y₋ = 1` (FTD-0111). |
| [MEASUREMENT_TOWER_LEVEL_SCAN.md](MEASUREMENT_TOWER_LEVEL_SCAN.md) | [MEASURED · CONFIRMATORY BLIND] | Hash-locked tower level-scan: no physics content at levels k ≠ 4. |
| [PROTOCOL_TOWER_LEVEL_FALSIFIER.md](PROTOCOL_TOWER_LEVEL_FALSIFIER.md) | [PRE-REGISTRATION DRAFT] | Pre-registration protocol for the tower level-scan falsifier. |

## Standard Model gauge sectors

The U(1) / SU(2) / SU(3) structure, the Higgs sector, generations, and couplings.

| File | Tag | Purpose |
|---|---|---|
| [DERIV_MOORE_GAUGE_STRUCTURE.md](DERIV_MOORE_GAUGE_STRUCTURE.md) | [THEOREM] | SM gauge group U(1)×SU(2)×SU(3) from Moore-neighborhood sublattice decomposition. |
| [DERIV_LATTICE_SU2_WEAK.md](DERIV_LATTICE_SU2_WEAK.md) | [THEOREM] + [SELECTION] | W/Z bosons + Weinberg angle from the ternary state structure. |
| [DERIV_LATTICE_SU3_GAUGE.md](DERIV_LATTICE_SU3_GAUGE.md) | [THEOREM] + [SELECTION] | SU(3) color gauge theory + gluons from flux geometry; QCD beta function. |
| [DERIV_HIGGS_FROM_MANIFESTATION.md](DERIV_HIGGS_FROM_MANIFESTATION.md) | [THEOREM] | Higgs mechanism + Mexican-hat potential from manifestation dynamics. |
| [DERIV_THREE_GENERATIONS.md](DERIV_THREE_GENERATIONS.md) | [THEOREM] counting / [SELECTION] physical ID | N_gen = 3 from cuboctahedral axis types. |
| [DERIV_FERMI_COUPLING_CONSTANT.md](DERIV_FERMI_COUPLING_CONSTANT.md) | [THEOREM] | Fermi coupling G_F from the Higgs VEV chain. |
| [DERIV_STATE_FLUX_COUPLING_DERIVATION.md](DERIV_STATE_FLUX_COUPLING_DERIVATION.md) | conditional derivation / matching selection | The g_c = √α coupling within the selected state-flux dictionary (audit-conditioned). |

## QED, QFT machinery, and scattering

Path integral, Feynman rules, loop corrections, anomalies — the lattice field-theory toolkit.

| File | Tag | Purpose |
|---|---|---|
| [DERIV_QFT_GRT_BRIDGE.md](DERIV_QFT_GRT_BRIDGE.md) | [THEOREM] + [SELECTION] | Lattice Green's function = Euclidean propagator; T_μν via Noether. Shared foundation. |
| [DERIV_PATH_INTEGRAL_CONSTRUCTION.md](DERIV_PATH_INTEGRAL_CONSTRUCTION.md) | [THEOREM] + [SELECTION] + [CONJECTURE] | Partition function, generating functional, effective action built natively on the lattice. |
| [DERIV_VARIATIONAL_PROOF.md](DERIV_VARIATIONAL_PROOF.md) | [THEOREM] | δS = 0 reproduces every engine update rule to machine precision. |
| [DERIV_FORCE_EMERGENCE.md](DERIV_FORCE_EMERGENCE.md) | [THEOREM] + [SELECTION] | Coulomb/Yukawa/Lorentz forces as regimes of one lattice Green's function. |
| [DERIV_COULOMB_SCATTERING_AMPLITUDE.md](DERIV_COULOMB_SCATTERING_AMPLITUDE.md) | [THEOREM] | Tree-level Coulomb scattering amplitude from the FTD Lagrangian. |
| [DERIV_LATTICE_QED_COMPLETE.md](DERIV_LATTICE_QED_COMPLETE.md) | [THEOREM] + [SELECTION] + [CONJECTURE] | Consolidated one- and two-loop lattice QED renormalization. |
| [DERIV_LATTICE_CHIRAL_ANOMALY.md](DERIV_LATTICE_CHIRAL_ANOMALY.md) | [THEOREM] + [SELECTION] | ABJ axial anomaly + π⁰ decay from the lattice triangle diagram. |
| [DERIV_ALPHA_FROM_PHASE_STRUCTURE.md](DERIV_ALPHA_FROM_PHASE_STRUCTURE.md) | [SELECTION] (historical) | Phase-structure argument that x₊ lies in the Coulomb phase; supporting evidence only. |
| [DERIV_CONTINUUM_LIMIT_QED_EQUIVALENCE.md](DERIV_CONTINUUM_LIMIT_QED_EQUIVALENCE.md) | [STRONGLY MOTIVATED CONJECTURE] (FTD-0013) | FTD U(1) → QED continuum-limit argument; theorem framing withdrawn under reframe. |
| [DERIV_18PT_LAPLACIAN_VARIATIONAL.md](DERIV_18PT_LAPLACIAN_VARIATIONAL.md) | [THEOREM] + [SELECTION] | The engine's 18-point isotropic Laplacian stencil from the action principle. |

## Electrodynamics — radiation, retardation, duality

The lattice-ED program: the Maxwell-exploit thread (FTD-0113…0120) and EM-regime unification.

| File | Tag | Purpose |
|---|---|---|
| [DERIV_RETARDED_GREEN_LATTICE.md](DERIV_RETARDED_GREEN_LATTICE.md) | [DERIVED] | Retarded lattice Green's-function identity; radiation-zone extension of Phase G. |
| [DERIV_LATTICE_LIENARD_WIECHERT.md](DERIV_LATTICE_LIENARD_WIECHERT.md) | [DERIVED] | Lattice boosted Coulomb at uniform velocity + lattice Cherenkov pole. |
| [DERIV_LATTICE_LW_EXTENSIONS.md](DERIV_LATTICE_LW_EXTENSIONS.md) | [DERIVED] / [PARTIAL DERIVED] (Q5) | Q5–Q8 closure: Larmor, Cherenkov rate, extended sources, source-half audit. |
| [DERIV_LATTICE_HODGE_DUALITY.md](DERIV_LATTICE_HODGE_DUALITY.md) | [DERIVED] | Lattice Bianchi identities (`d² = 0`) on the vertex-centered stencil. |
| [DERIV_EM_REGIMES_UNIFIED.md](DERIV_EM_REGIMES_UNIFIED.md) | [PARTIAL — DERIVED / SELECTION] | The three engine EM-force modes: Poisson↔Legacy equivalent, Emergent distinct. |
| [DERIV_DAMPING_RAYLEIGH.md](DERIV_DAMPING_RAYLEIGH.md) | [IMPOSED — with motivation] | The Rayleigh dissipation coefficient DAMPING = α; three-roles diagnosis. |
| [DERIV_HEAT_EQUATION_FROM_RATIO.md](DERIV_HEAT_EQUATION_FROM_RATIO.md) | [THEOREM] | Heat equation + arrow of time from the Euler reflection ratio; α as the dissipation parameter. |

## Gravity and general relativity

From the latency field to the full nonlinear Einstein equations and black holes.

| File | Tag | Purpose |
|---|---|---|
| [DERIV_RELATIVITY_DERIVATION.md](DERIV_RELATIVITY_DERIVATION.md) | rigorous derivation, gaps marked | Special + weak-field general relativity from the speed-of-causality axiom. |
| [DERIV_EINSTEIN_FIELD_EQUATIONS.md](DERIV_EINSTEIN_FIELD_EQUATIONS.md) | [THEOREM] chain + [SELECTION] IDs | Full nonlinear Einstein equations via the Lovelock route. |
| [DERIV_EINSTEIN_NONLINEAR_FROM_LATTICE.md](DERIV_EINSTEIN_NONLINEAR_FROM_LATTICE.md) | [THEOREM] chain + [SELECTION] IDs | Constructive Deser iterative-bootstrap route to nonlinear EFE; cross-check on Lovelock. |
| [DERIV_NEWTON_FROM_SUBSTRATE.md](DERIV_NEWTON_FROM_SUBSTRATE.md) | [STRONGLY MOTIVATED CONJECTURE] / [DERIVED] chain | Newton's law from the substrate; α_G(e,e); falsifies the "G_N = 1/100" claim (FTD-0131). |
| [DERIV_BLACK_HOLE_PHYSICS.md](DERIV_BLACK_HOLE_PHYSICS.md) | [THEOREM] + [SELECTION] | Hawking radiation, entropy, information-paradox resolution on the lattice. |
| [DERIV_LATTICE_BLACK_HOLES.md](DERIV_LATTICE_BLACK_HOLES.md) | [THEOREM] / [SELECTION] / [CONJECTURE] | Schwarzschild, Kerr, Reissner-Nordström metrics via the computational-budget principle. |
| [DERIV_STELLAR_LIFECYCLE_LATTICE.md](DERIV_STELLAR_LIFECYCLE_LATTICE.md) | [SELECTION] | Intuitive ground-up account of the stellar lifecycle as flux-budget dynamics. |
| [DERIV_DARK_SECTOR_DYNAMICS.md](DERIV_DARK_SECTOR_DYNAMICS.md) | [SELECTION] | Dark energy + dark matter from coupling injection vs selective damping. |
| [DERIV_QFT_GRT_BRIDGE.md](DERIV_QFT_GRT_BRIDGE.md) | [THEOREM] + [SELECTION] | (Also QFT) — provides T_μν, the gravitational source, via Noether. |

## Quantum mechanics, measurement, and Bell

QM as lattice statistics, the Born rule, the singlet, and Bell-inequality emergence.

| File | Tag | Purpose |
|---|---|---|
| [DERIV_QM_FROM_LATTICE.md](DERIV_QM_FROM_LATTICE.md) | derivation (QM as statistics) | Quantum mechanics as the statistics of definite lattice events. |
| [DERIV_QUANTUM_MECHANICS_RESOLVED.md](DERIV_QUANTUM_MECHANICS_RESOLVED.md) | [AXIOM]→[THEOREM]/[SELECTION]/[CONJECTURE] | The full logical chain from the First Distinction to every QM rule. |
| [DERIV_DIRAC_FROM_MASTER_QUADRATIC.md](DERIV_DIRAC_FROM_MASTER_QUADRATIC.md) | derivation, honest assessment | The Dirac equation as the complex regime of the master quadratic. |
| [DERIV_SPIN_STATISTICS_BRIDGE.md](DERIV_SPIN_STATISTICS_BRIDGE.md) | [THEOREM] + [SELECTION] | Spin-statistics from two-lemniscate geometry; G\* as boson-fermion bridge. |
| [DERIV_OBSERVER_BELL_MECHANISM.md](DERIV_OBSERVER_BELL_MECHANISM.md) | [SELECTION] | Three-level observer hierarchy producing S = 2√2 from a local substrate. |
| [DERIV_BELL_COSINE_FROM_GAUSS.md](DERIV_BELL_COSINE_FROM_GAUSS.md) | [THEOREM] + [SELECTION] | The Gauss constraint produces E(θ) = −cos θ and the Tsirelson bound. |
| [DERIV_SINGLET_FROM_VOID_EVENT.md](DERIV_SINGLET_FROM_VOID_EVENT.md) | [THEOREM] + [SELECTION] | The void event `0 → (+1)+(−1)` produces the spin-1/2 singlet; closes the Bell loop. |
| [DERIV_KCOMP_VOLUMETRIC_SHELL.md](DERIV_KCOMP_VOLUMETRIC_SHELL.md) | [SELECTION] / [THEOREM] / [EMERGENT] | The K_comp shell: dynamical mechanism for non-factorizable joint probabilities. |

## G\* provenance and the FQCR program

Where the bridge constant G\* comes from, and finite-N approximations.

| File | Tag | Purpose |
|---|---|---|
| [DERIV_GSTAR_QUARTER_CONJUGACY.md](DERIV_GSTAR_QUARTER_CONJUGACY.md) | [THEOREM] | G\* as a ζ-regularized determinant ratio of quarter-twisted spectra (FQCR Model I, FTD-0141). |
| [DERIV_GSTAR_FINITE_APPROX.md](DERIV_GSTAR_FINITE_APPROX.md) | [THEOREM] | Finite-N attractor `G*_N → G*` at a controlled rate (FQCR Model II, FTD-0142). |

## FTD-0110 cluster-mass bridge

The linear→nonlinear bridge connecting cluster size to particle mass.

| File | Tag | Purpose |
|---|---|---|
| [DERIV_K_FROM_OH_A1G_MULTIPLICITY.md](DERIV_K_FROM_OH_A1G_MULTIPLICITY.md) | [DERIVED at linear level] / [STRONGLY MOTIVATED CONJECTURE] nonlinear | `k = 1/N_base = 1/4` from O_h representation theory of the 27-block. |
| [DERIV_FTD0110_NONLINEAR_BRIDGE.md](DERIV_FTD0110_NONLINEAR_BRIDGE.md) | [DERIVED] Bridge-I / [FALSIFIED EMPIRICALLY] local reading | Nonlinear-bridge closure: O_h-equivariance + single-block energy budget. |
| [DERIV_FTD0110_FREE_ENERGY_LANDSCAPE.md](DERIV_FTD0110_FREE_ENERGY_LANDSCAPE.md) | [DERIVED] framework / [PARTIAL] parameters | Cluster phenomenology as a multi-basin free-energy landscape. |
| [DERIV_FTD0110_VARIANCE_ENTROPY.md](DERIV_FTD0110_VARIANCE_ENTROPY.md) | [PARTIAL] | Cluster-size variance as boundary entropy; tested against engine data. |
| [EXPLR_FTD_0110_NONLINEAR_BRIDGE_ANALYSIS.md](EXPLR_FTD_0110_NONLINEAR_BRIDGE_ANALYSIS.md) | [PARTIAL] (exploratory) | Sharpens the [OPEN] nonlinear-bridge gap; does not close it. |

## Hadrons, confinement, and Yukawa structure

QCD-sector derivations: confinement, pion mass, the electron Yukawa prefactor.

| File | Tag | Purpose |
|---|---|---|
| [DERIV_CONFINEMENT_FROM_GAP_EQUATION.md](DERIV_CONFINEMENT_FROM_GAP_EQUATION.md) | [THEOREM] confinement at x₋ / [SELECTION] QCD ID | Area-law Wilson loops + linear confinement from the master quadratic's x₋ root. |
| [DERIV_PION_MASS_FROM_GSTAR.md](DERIV_PION_MASS_FROM_GSTAR.md) | 6 [THEOREM] + 3 [SELECTION] + 5 [CONJECTURE] | The 15-step G\* → m_π chain; integer-reduction theorem; 0.048% match. |
| [DERIV_YUKAWA_FROM_27BLOCK_CHARACTER.md](DERIV_YUKAWA_FROM_27BLOCK_CHARACTER.md) | [STRUCTURALLY MOTIVATED PARAMETRIC] | The electron Yukawa prefactor `16√2/3` from O_h character theory (FTD-0134). |

## Foundational chains and resolutions

Bottom-up overview docs and gap-closing arguments.

| File | Tag | Purpose |
|---|---|---|
| [DERIV_BOTTOM_UP_PHYSICS.md](DERIV_BOTTOM_UP_PHYSICS.md) | [AXIOM]→[THEOREM]/[SELECTION]/[CONJECTURE] | Entry-point: from `0 = (−1)+(+1)` to the Standard Model. |
| [DERIV_THREE_RESOLUTIONS.md](DERIV_THREE_RESOLUTIONS.md) | derivation (closes three gaps) | Compact U(1), bare = physical, one-loop exact — answered by the tick. |

---

61 active docs in this cluster (+ 0 archived).
