# INDEX · Core Physics Derivations

**Tag:** [REFERENCE]
**Status:** [REFERENCE] — local navigation index for `docs/theory/03_derivations/`.
**Purpose:** This is the largest theory cluster — the working derivations that turn the FTD axioms and the algebraic spine into physics: the master quadratic and its consequences, the Standard Model gauge sectors, QED/QFT machinery, gravity, quantum-mechanics foundations, the G\* provenance chain, and the FTD-0110 cluster-mass bridge. Read this cluster when you need the explicit chain behind a physics claim; check each doc's own epistemic tag, because most mix [THEOREM]/[DERIVED] algebra with [SELECTION]/[CONJECTURE] physical identifications.

---

## Read first

Newcomers should read these in order:

1. [DERIV_BOTTOM_UP_PHYSICS.md](foundational_mechanics/DERIV_BOTTOM_UP_PHYSICS.md) — the entry-point narrative: from `0 = (−1) + (+1)` to the Standard Model.
2. [DERIV_MASTER_QUADRATIC_GAP_EQUATION.md](foundational_mechanics/DERIV_MASTER_QUADRATIC_GAP_EQUATION.md) — canonical statement of the master quadratic: algebraic identity + physical match.
3. [DERIV_FORCE_EMERGENCE.md](foundational_mechanics/DERIV_FORCE_EMERGENCE.md) — the four forces as regimes of one lattice Green's function.
4. [DERIV_QFT_GRT_BRIDGE.md](foundational_mechanics/DERIV_QFT_GRT_BRIDGE.md) — lattice propagators + stress-energy tensor; the shared QFT/GR foundation other derivations depend on.
5. [DERIV_QM_FROM_LATTICE.md](quantum_mechanics/DERIV_QM_FROM_LATTICE.md) — quantum mechanics as statistics of lattice events.
6. [DERIV_K_FROM_OH_A1G_MULTIPLICITY.md](foundational_mechanics/DERIV_K_FROM_OH_A1G_MULTIPLICITY.md) — the cluster-mass bridge (FTD-0110): `k = 1/N_base` from O_h representation theory.

---

## Master quadratic and the framework integers

The algebraic core: the polynomial `x² − 16G*²x + 16G*³ = 0`, its provenance, and what its roots mean.

| File | Tag | Purpose |
|---|---|---|
| [DERIV_MASTER_QUADRATIC_GAP_EQUATION.md](foundational_mechanics/DERIV_MASTER_QUADRATIC_GAP_EQUATION.md) | [THEOREM] algebra / [STRONGLY MOTIVATED CONJECTURE] physical ID | Canonical master-quadratic reference: identity, coefficient routes, discriminant trichotomy, dual match. |
| [DERIV_QUADRATURE_COVARIANCE.md](DERIV_QUADRATURE_COVARIANCE.md) | [THEOREM] | Formal derivation of the intrinsic $\mathbb{Z}[i]$ quadrature field via edge-covariant transverse projection. |
| [DERIV_FLUCTUATION_KERNEL_GREEN.md](DERIV_FLUCTUATION_KERNEL_GREEN.md) | [THEOREM] | Explicit definition of $\mathcal{K}_{A_J}$ as the covariant lattice Green's operator, linking its trace to the Watson integral $W_3$. |
| [DERIV_OSCILLATORY_CLOUD_DYNAMICS.md](foundational_mechanics/DERIV_OSCILLATORY_CLOUD_DYNAMICS.md) | [OPEN PROGRAM] | Formalizes the dynamic resonance of $\mathcal{B}_{\Omega}(t)$, its return map $M_{\rm cloud}$, and the resonant-mode crossover explaining the FTD-0110 mass bridge. |
| [DERIV_FTD0110_NONLINEAR_CLOSURE.md](foundational_mechanics/DERIV_FTD0110_NONLINEAR_CLOSURE.md) | [OPEN PROGRAM] | Analyzes the perturbation theory of the Oscillatory Cloud, establishing the 18-point Laplacian's spectral gap $\Delta\lambda \approx 1.138$ as the boundary for the mass bridge thermal knee. |
| [DERIV_MASTER_QUADRATIC_FROM_Z.md](foundational_mechanics/DERIV_MASTER_QUADRATIC_FROM_Z.md) | [THEOREM] polynomial / route RETRACTED (FTD-0032) | Partition-function route to the gap equation; the L→∞ derivation is retracted, polynomial preserved. |
| [ANALYSIS_TOPOLOGICAL_CHARGE_TRANSPORT_v1.md](foundational_mechanics/ANALYSIS_TOPOLOGICAL_CHARGE_TRANSPORT_v1.md) | [MEASURED — UNDERDETERMINED] | FTD-0398 terminal scaled-octahedron transport test. Transient charges decide neither persistent transport nor the registered destruction event; no mass evidence and no further shell redesign. |
| [ANALYSIS_FLUX_EOS_v1.md](foundational_mechanics/ANALYSIS_FLUX_EOS_v1.md) | [MEASURED — CLOSED NEGATIVE] | Tests whether x₋ = 3.024 is the dimensionless pressure of the flux field; the EoS configuration that reaches 3.024 is geometric (mode-spectrum-set), not α-locked (FTD-0312). |
| [DERIV_GAP_EQUATION_FORM.md](foundational_mechanics/DERIV_GAP_EQUATION_FORM.md) | [THEOREM given self-consistency prescription] | Why the two coefficients are linked: one-loop self-consistency forces `x² = K(x − G*)`. |
| [DERIV_QUADRATIC_NECESSITY.md](foundational_mechanics/DERIV_QUADRATIC_NECESSITY.md) | [DERIVATION] (two proofs) | Two independent arguments that degree 2 is structurally forced, not chosen. |
| [DERIV_CHARGE_QUARTIC_FROM_GSTAR.md](foundational_mechanics/DERIV_CHARGE_QUARTIC_FROM_GSTAR.md) | [THEOREM] (6) + [SELECTION] (1) | The charge quartic: master quadratic viewed in charge space `e² = 1/x`. |
| [DERIV_INTEGER_PHYSICAL_IDENTIFICATION.md](foundational_mechanics/DERIV_INTEGER_PHYSICAL_IDENTIFICATION.md) | [THEOREM] given N_gen = N_c [SELECTION] | Traces each framework integer {3, 4, 7, 13} to its physical role. |
| [DERIV_NC_FROM_TOPOLOGY.md](standard_model/DERIV_NC_FROM_TOPOLOGY.md) | [THEOREM] for N_c=3 invariant / [SELECTION] for QCD ID | N_c = 3 from four independent topological routes on the Moore neighborhood. |
| [THEOREM_HARMONIC_INVARIANT_TOWER.md](electromagnetism/THEOREM_HARMONIC_INVARIANT_TOWER.md) | [THEOREM] + [DERIVED] | The (1+i)-tower of master quadratics; harmonic invariant `1/y₊ + 1/y₋ = 1` (FTD-0111). |
| [MEASUREMENT_TOWER_LEVEL_SCAN.md](quantum_mechanics/MEASUREMENT_TOWER_LEVEL_SCAN.md) | [MEASURED · CONFIRMATORY BLIND] | Hash-locked tower level-scan: no physics content at levels k ≠ 4. |
| [PROTOCOL_TOWER_LEVEL_FALSIFIER.md](foundational_mechanics/PROTOCOL_TOWER_LEVEL_FALSIFIER.md) | [PRE-REGISTRATION DRAFT] | Pre-registration protocol for the tower level-scan falsifier. |

## Standard Model gauge sectors

The U(1) / SU(2) / SU(3) structure, the Higgs sector, generations, and couplings.

| File | Tag | Purpose |
|---|---|---|
| [DERIV_MOORE_GAUGE_STRUCTURE.md](standard_model/DERIV_MOORE_GAUGE_STRUCTURE.md) | [THEOREM] | SM gauge group U(1)×SU(2)×SU(3) from Moore-neighborhood sublattice decomposition. |
| [DERIV_LATTICE_SU2_WEAK.md](standard_model/DERIV_LATTICE_SU2_WEAK.md) | [THEOREM] + [SELECTION] | W/Z bosons + Weinberg angle from the ternary state structure. |
| [DERIV_LATTICE_SU3_GAUGE.md](standard_model/DERIV_LATTICE_SU3_GAUGE.md) | [THEOREM] + [SELECTION] | SU(3) color gauge theory + gluons from flux geometry; QCD beta function. |
| [DERIV_HIGGS_FROM_MANIFESTATION.md](standard_model/DERIV_HIGGS_FROM_MANIFESTATION.md) | [THEOREM] | Higgs mechanism + Mexican-hat potential from manifestation dynamics. |
| [DERIV_THREE_GENERATIONS.md](standard_model/DERIV_THREE_GENERATIONS.md) | [THEOREM] counting / [SELECTION] physical ID | N_gen = 3 from cuboctahedral axis types. |
| [DERIV_FERMI_COUPLING_CONSTANT.md](standard_model/DERIV_FERMI_COUPLING_CONSTANT.md) | [THEOREM] | Fermi coupling G_F from the Higgs VEV chain. |
| [DERIV_STATE_FLUX_COUPLING_DERIVATION.md](electromagnetism/DERIV_STATE_FLUX_COUPLING_DERIVATION.md) | conditional derivation / matching selection | The g_c = √α coupling within the selected state-flux dictionary (audit-conditioned). |

## Geometric mass-ratio readings (2026-06-18 batch, consolidated 2026-08-06)

Four sibling documents from the same 2026-06-18 batch applied one reused technique (Moore-neighborhood boundary counting by O_h symmetry class) to four different targets. Three carried a `[THEOREM]` upgrade later retracted as a substitution identity; the fourth (atomic spectrum) was never promoted but shared the same unhedged rhetoric. Merged into one document 2026-08-06, each target keeping its own tag and provenance as a section; originals archived.

| File | Tag | Purpose |
|---|---|---|
| [DERIV_GEOMETRIC_MASS_RATIO_READINGS.md](standard_model/DERIV_GEOMETRIC_MASS_RATIO_READINGS.md) | [STRUCTURALLY MOTIVATED PARAMETRIC] throughout, per-section exceptions noted | §1 lepton mass ratios (muon 207, tau 3477); §2 sin²θ_W=3/13 (FTD-0018) + α_s(M_Z)=7/59 (FTD-0020); §3 proton [SMC] + six quark masses [PARAMETRIC]; §4 atomic spectrum/shell structure [SMC, unaudited]. Consolidates `DERIV_LEPTON_MASS_GEOMETRY.md`, `DERIV_WEINBERG_STRONG_GEOMETRY.md`, `DERIV_BARYON_AND_QUARK_GEOMETRY.md`, `DERIV_DISCRETE_ATOMIC_SPECTRUM.md` (all archived, see `archive/retracted/` and `archive/resolved/`). |

## QED, QFT machinery, and scattering

Path integral, Feynman rules, loop corrections, anomalies — the lattice field-theory toolkit.

| File | Tag | Purpose |
|---|---|---|
| [DERIV_QFT_GRT_BRIDGE.md](foundational_mechanics/DERIV_QFT_GRT_BRIDGE.md) | [THEOREM] + [SELECTION] | Lattice Green's function = Euclidean propagator; T_μν via Noether. Shared foundation. |
| [DERIV_PATH_INTEGRAL_CONSTRUCTION.md](quantum_mechanics/DERIV_PATH_INTEGRAL_CONSTRUCTION.md) | [THEOREM] + [SELECTION] + [CONJECTURE] | Partition function, generating functional, effective action built natively on the lattice. |
| [DERIV_VARIATIONAL_PROOF.md](foundational_mechanics/DERIV_VARIATIONAL_PROOF.md) | [PARTIAL THEOREM] + [RETRACTED ALL-UPDATE CLAIM] | Scoped free-field/stationary-source variation plus selected-rule replay; FTD-0467/0565 close the common-action reading for current production forces and genesis/evaporation. |
| [DERIV_FORCE_EMERGENCE.md](foundational_mechanics/DERIV_FORCE_EMERGENCE.md) | [THEOREM] + [SELECTION] | Coulomb/Yukawa/Lorentz forces as regimes of one lattice Green's function. |
| [DERIV_COULOMB_SCATTERING_AMPLITUDE.md](electromagnetism/DERIV_COULOMB_SCATTERING_AMPLITUDE.md) | [THEOREM] | Tree-level Coulomb scattering amplitude from the FTD Lagrangian. |
| [DERIV_LATTICE_QED_COMPLETE.md](electromagnetism/DERIV_LATTICE_QED_COMPLETE.md) | [THEOREM] + [SELECTION] + [CONJECTURE] | Consolidated one- and two-loop lattice QED renormalization. |
| [DERIV_LATTICE_CHIRAL_ANOMALY.md](standard_model/DERIV_LATTICE_CHIRAL_ANOMALY.md) | [THEOREM] + [SELECTION] | ABJ axial anomaly + π⁰ decay from the lattice triangle diagram. |
| [DERIV_ALPHA_FROM_PHASE_STRUCTURE.md](foundational_mechanics/DERIV_ALPHA_FROM_PHASE_STRUCTURE.md) | [SELECTION] (historical) | Phase-structure argument that x₊ lies in the Coulomb phase; supporting evidence only. |
| [DERIV_CONTINUUM_LIMIT_QED_EQUIVALENCE.md](electromagnetism/DERIV_CONTINUUM_LIMIT_QED_EQUIVALENCE.md) | [STRONGLY MOTIVATED CONJECTURE] (FTD-0013) | FTD U(1) → QED continuum-limit argument; theorem framing withdrawn under reframe. |
| [DERIV_18PT_LAPLACIAN_VARIATIONAL.md](foundational_mechanics/DERIV_18PT_LAPLACIAN_VARIATIONAL.md) | [THEOREM] + [SELECTION] | The engine's 18-point isotropic Laplacian stencil from the action principle. |
| [DERIV_LAGRANGIAN_FROM_TICK_RULE.md](DERIV_LAGRANGIAN_FROM_TICK_RULE.md) | [DERIVATION] / [OPEN] | Bare action from time-discretization of the tick rule (Mechanism B bare-action construction, FTD-0246). |
| [DERIV_LATTICE_PATH_INTEGRAL_JTWIST.md](DERIV_LATTICE_PATH_INTEGRAL_JTWIST.md) | [DERIVATION] / [OPEN] | Lattice path integral + J-twisted screening self-energy (Mechanism B lattice→continuum matching, FTD-0245). |
| [DERIV_ALPHA_OPERATIONAL_READOUT.md](DERIV_ALPHA_OPERATIONAL_READOUT.md) | [DERIVATION] | Operational alpha-readout mechanism (remediation). |
| [DERIV_VACUUM_ENERGY_CUTOFF.md](DERIV_VACUUM_ENERGY_CUTOFF.md) | [NUMERICAL FACT] (reconciled) | Finite O(1) vacuum-energy integral over the discrete Brillouin zone; the 10^120-Λ bridge is [OPEN]/[CONJECTURE], not derived (LEDGER FTD-0303). |

## Electrodynamics — radiation, retardation, duality

The lattice-ED program: the Maxwell-exploit thread (FTD-0113…0120) and EM-regime unification.

| File | Tag | Purpose |
|---|---|---|
| [DERIV_RETARDED_GREEN_LATTICE.md](foundational_mechanics/DERIV_RETARDED_GREEN_LATTICE.md) | [THEOREM — AUXILIARY 7-POINT] + [THEOREM — NATIVE STATIC RESOLVENT] | FTD-0558 separates the historical continuous-time identity from the production discrete-time `FULL` resolvent. |
| [DERIV_LATTICE_LIENARD_WIECHERT.md](electromagnetism/DERIV_LATTICE_LIENARD_WIECHERT.md) | [DERIVED — SELECTED DRIVE] + [THEOREM — SPEED FLOOR/EXTERNAL WORK/HOP/GAUSS/TOPOLOGY-MAGNITUDE OBSTRUCTIONS] | Corrected pole and field energy; FTD-0560–0563 close fixed finite rigid linear dressing and finite-neutral monopoles; FTD-0564 proves orientation degree alone cannot quantize Gauss magnitude. Protected defect plus nonlinear common action/recoil/power remain open. |
| [DERIV_LATTICE_LW_EXTENSIONS.md](electromagnetism/DERIV_LATTICE_LW_EXTENSIONS.md) | [THEOREM — EXTERNAL-DRIVE ENERGY/MULTIPOLE/FULL-SURFACE OBSTRUCTIONS] + [OPEN — PHYSICAL POWER] | FTD-0558–0562 replace the aliased physical-power claim with exact field-side response and the finite-rigid-source no-go hierarchy. |
| [DERIV_LATTICE_HODGE_DUALITY.md](electromagnetism/DERIV_LATTICE_HODGE_DUALITY.md) | [DERIVED] | Lattice Bianchi identities (`d² = 0`) on the vertex-centered stencil. |
| [DERIV_EM_REGIMES_UNIFIED.md](electromagnetism/DERIV_EM_REGIMES_UNIFIED.md) | [PARTIAL — DERIVED / SELECTION] | The three engine EM-force modes: PoissonLegacy equivalent, Emergent distinct. |
| [ANALYSIS_LATTICE_WAVE_SECTORS_v1.md](foundational_mechanics/ANALYSIS_LATTICE_WAVE_SECTORS_v1.md) | [SYNTHESIS] + [BOUNDARY] | Sound/light/radio on one dispersion `ω(k)`; light = radio (one flux-wave sector); the no-acoustic-sector boundary (FTD-0298). |
| [ANALYSIS_WAVE_SECTORS_v1.md](foundational_mechanics/ANALYSIS_WAVE_SECTORS_v1.md) | [MEASURED] | Run of record (FTD-0299): light dispersion atlas LIGHT-CONFIRMED (ω matches the 18-pt stencil; isotropic c=1/√3) + condensate-compression probe NULL (no acoustic branch — FTD-0298 boundary engine-confirmed). |
| [DERIV_DAMPING_RAYLEIGH.md](foundational_mechanics/DERIV_DAMPING_RAYLEIGH.md) | [IMPOSED — with motivation] | The Rayleigh dissipation coefficient DAMPING = α; three-roles diagnosis. |
| [DERIV_HEAT_EQUATION_FROM_RATIO.md](foundational_mechanics/DERIV_HEAT_EQUATION_FROM_RATIO.md) | [THEOREM] | Heat equation + arrow of time from the Euler reflection ratio; α as the dissipation parameter. |

## Gravity and general relativity

From the latency field to the full nonlinear Einstein equations and black holes.

| File | Tag | Purpose |
|---|---|---|
| [DERIV_RELATIVITY_DERIVATION.md](gravity_and_cosmology/DERIV_RELATIVITY_DERIVATION.md) | rigorous derivation, gaps marked | Special + weak-field general relativity from the speed-of-causality axiom. |
| [DERIV_EINSTEIN_FIELD_EQUATIONS.md](gravity_and_cosmology/DERIV_EINSTEIN_FIELD_EQUATIONS.md) | [THEOREM] chain + [SELECTION] IDs | Full nonlinear Einstein equations via the Lovelock route. |
| [DERIV_EINSTEIN_NONLINEAR_FROM_LATTICE.md](gravity_and_cosmology/DERIV_EINSTEIN_NONLINEAR_FROM_LATTICE.md) | [THEOREM] chain + [SELECTION] IDs | Constructive Deser iterative-bootstrap route to nonlinear EFE; cross-check on Lovelock. |
| [DERIV_NEWTON_FROM_SUBSTRATE.md](gravity_and_cosmology/DERIV_NEWTON_FROM_SUBSTRATE.md) | [STRONGLY MOTIVATED CONJECTURE] / [DERIVED] chain | Newton's law from the substrate; α_G(e,e); falsifies the "G_N = 1/100" claim (FTD-0131). |
| [DERIV_BLACK_HOLE_PHYSICS.md](gravity_and_cosmology/DERIV_BLACK_HOLE_PHYSICS.md) | [THEOREM] + [SELECTION] | Hawking radiation, entropy, information-paradox resolution on the lattice. |
| [DERIV_LATTICE_BLACK_HOLES.md](gravity_and_cosmology/DERIV_LATTICE_BLACK_HOLES.md) | [THEOREM] / [SELECTION] / [VERIFIED] | Schwarzschild, Kerr, Reissner-Nordström, and Kerr-Newman metrics via the computational-budget principle; all outstanding gaps resolved. |
| [DERIV_STELLAR_LIFECYCLE_LATTICE.md](gravity_and_cosmology/DERIV_STELLAR_LIFECYCLE_LATTICE.md) | [SELECTION] | Intuitive ground-up account of the stellar lifecycle as flux-budget dynamics. |
| [DERIV_DARK_SECTOR_DYNAMICS.md](gravity_and_cosmology/DERIV_DARK_SECTOR_DYNAMICS.md) | [SELECTION] / dark-energy source [OPEN] (FTD-0331) | Dark-matter halo + dark-energy heuristics; the coupling-injection "Λ source" is superseded — it carries no L-dependence and is not a viable Λ source, FTD predicts Λ=0 (FTD-0331). |
| [DERIV_SCALE_GROWTH_AND_COSMIC_EMERGENCE.md](gravity_and_cosmology/DERIV_SCALE_GROWTH_AND_COSMIC_EMERGENCE.md) | [THEOREM] (discrete-BZ limits) / [PARAMETRIC] (dark energy) / [OPEN]·mapped-negative (dark-matter halo) | Discrete-BZ limits [THEOREM]; the comoving stretch + emergent Friedmann are an imported-metric ansatz [CONJECTURE]; the α¹⁶ "dark-energy leak" is a [PARAMETRIC] value-match superseded-in-rationale (FTD predicts Λ=0, FTD-0331); the r⁻⁰·⁶⁹ halo is FALSIFIED (FTD-0300). |
| [DERIV_EMERGENT_DIFFEROMORPHISM_INVARIANCE.md](gravity_and_cosmology/DERIV_EMERGENT_DIFFEROMORPHISM_INVARIANCE.md) | [SELECTION] | Emergent diffeomorphism invariance Diff(M) from discrete point-group symmetry Oh via local spatial averaging. |
| [DERIV_LAMBDA_SCALE_COVARIANT.md](gravity_and_cosmology/DERIV_LAMBDA_SCALE_COVARIANT.md) | [SELECTION] + [BOUNDARY] | The QFT vacuum-energy catastrophe is dissolved (the classical FTD vacuum is exactly zero-energy); FC-3 forces any nonzero Λ to be a scale-ratio, not a constant; source stays [OPEN], value stays [BOUNDARY] (FTD-0331). |
| [ANALYSIS_LAMBDA_LSCAN_FEASIBILITY_v1.md](gravity_and_cosmology/ANALYSIS_LAMBDA_LSCAN_FEASIBILITY_v1.md) | [SYNTHESIS] + [BOUNDARY] | The FTD-0331 source gap resists engine L-scan closure on three independent obstructions (circularity, ill-posed steady state, Green's-function trap); the gap stays [OPEN] (FTD-0364). |
| [SCOPE_NEWTON_POSTULATES_RECONCILIATION.md](gravity_and_cosmology/SCOPE_NEWTON_POSTULATES_RECONCILIATION.md) | [SCOPING MEMO] | Records the tag-reconciliation question between DERIV_NEWTON_FROM_SUBSTRATE.md's flagged postulates and SPEC_FTD_LAGRANGIAN.md's theorem-tagged §4.2/§4.3; superseded by FTD-0400–0402. |
| [SCOPE_GW_AREA_HOLONOMY_v1.md](../10_eft_program/scopes_and_specs/SCOPE_GW_AREA_HOLONOMY_v1.md) | [SCOPE / ADOPTION CANDIDATE] | P6C-G type refinement: gravity as dynamical transport geometry, not a force carrier. Q0 CLOSED-NEGATIVE (FTD-1015). |
| [ANALYSIS_GW_AREA_HOLONOMY_Q0_v1.md](gravity_and_cosmology/ANALYSIS_GW_AREA_HOLONOMY_Q0_v1.md) | [CLOSED NEGATIVE] | Kinematic residual of spatial \(\Omega\): \(H=\{-2,-1,0,0,+1,+2\}\), leftover 4. Does not isolate LIGO shears. |
| [SCOPE_UNIVERSAL_FREEFALL_v1.md](../10_eft_program/scopes_and_specs/SCOPE_UNIVERSAL_FREEFALL_v1.md) | [SCOPE / OPEN PROGRAM] | Weak EP / universal free fall. Q0 FOUND (FTD-1013); default alignment CLOSED-NEGATIVE (FTD-1014); selected geometric integrator FOUND (FTD-1016); \(m_i=m_g\) still imposed. |
| [DERIV_UNIVERSAL_FREEFALL_Q0_v1.md](gravity_and_cosmology/DERIV_UNIVERSAL_FREEFALL_Q0_v1.md) | [THEOREM given FC-2] + [SELECTION] class C | Test-body UFF is the EL of \(S=-\alpha\int d\tau\) in locked class C; weak \(\Phi_N=-(C^2/2)\mathcal{L}^2\). Verifier 15/15. Does not move FTD-0250/0349/0402. |
| [ANALYSIS_UNIVERSAL_FREEFALL_ENGINE_ALIGN_v1.md](gravity_and_cosmology/ANALYSIS_UNIVERSAL_FREEFALL_ENGINE_ALIGN_v1.md) | [MEASURED — CLOSED NEGATIVE] | Live \(F=G_N\nabla|J|\) vs Q0 \(g_{\rm ext}\): Path G \(r_G=1\), Path F \(r_F=0\). CTest protocol pass / A1 fail. Default operator unmoved. |
| [ANALYSIS_GEOMETRIC_FREEFALL_INTEGRATOR_v1.md](gravity_and_cosmology/ANALYSIS_GEOMETRIC_FREEFALL_INTEGRATOR_v1.md) | [MEASURED — SELECTED INTEGRATOR, FOUND] | Default-off \(F=M C^2\,\mathcal{L}\,\nabla\mathcal{L}\): \(r_{F{\rm-on}}\approx 0.9979\). Golden-neutral. Does not derive \(m_i=m_g\). |
| [CATALOG_EMPIRICAL_GRAVITY_ENGINE_v1.md](gravity_and_cosmology/CATALOG_EMPIRICAL_GRAVITY_ENGINE_v1.md) | [SYNTHESIS] | Measured gravity phenomena vs engine (UFF IN, redshift ALREADY+FTD-1019 MEASURED, sourced Newton FTD-1017 freeze + FTD-1021/1022 live, GPU parity FTD-1018 FOUND, lensing OUT FTD-1020 class 0, GWs OUT). |
| [ANALYSIS_SOURCED_GEOMETRIC_FREEFALL_v1.md](gravity_and_cosmology/ANALYSIS_SOURCED_GEOMETRIC_FREEFALL_v1.md) | [MEASURED — SOURCED WIRING, FOUND] | Poisson-written \(\mathcal{L}\) + FTD-1016 operator: \(r_{\rm on}=0.999542\). Not \(1/r^2\), not strong EP. |
| [ANALYSIS_GPU_GEOMETRIC_GRAVITY_PARITY_v1.md](gravity_and_cosmology/ANALYSIS_GPU_GEOMETRIC_GRAVITY_PARITY_v1.md) | [MEASURED — GPU PARITY, FOUND] | Native CUDA reproduces FTD-1016; \(\Delta F=\Delta v=0\). Golden-neutral. |
| [ANALYSIS_ONE_WELL_REDSHIFT_FALLING_v1.md](gravity_and_cosmology/ANALYSIS_ONE_WELL_REDSHIFT_FALLING_v1.md) | [MEASURED — ONE-WELL CLOCKS+FALLING, FOUND] | Frozen Poisson \(\mathcal{L}\) drives FC-2 rest clocks (\(\rho_\tau=1\)) and FTD-1016 falling (\(r_{\rm on}=0.999542\)). Not Pound–Rebka. |
| [ANALYSIS_FROZEN_WELL_CHARACTERISTIC_DEFLECTION_v1.md](gravity_and_cosmology/ANALYSIS_FROZEN_WELL_CHARACTERISTIC_DEFLECTION_v1.md) | [MEASURED STRUCTURAL NULL — FOUND class 0] | Frozen vacuum Poisson \(\mathcal{L}\) unread by live `wave_propagation` (\(\theta_{\rm diff}=0\)). Lensing stays OUT. Not Eddington. |
| [ANALYSIS_LIVE_SOURCED_NEWTON_v1.md](gravity_and_cosmology/ANALYSIS_LIVE_SOURCED_NEWTON_v1.md) | [MEASURED — LIVE SOURCED NEWTON, PROBE SOURCES] | Live Poisson: \(r_L=0.9857\). Freeze not test-body at \(1/125\) (\(\delta_a=3.71\)). Self-force null. |
| [ANALYSIS_SLOW_ENVELOPE_LIVE_NEWTON_v1.md](gravity_and_cosmology/ANALYSIS_SLOW_ENVELOPE_LIVE_NEWTON_v1.md) | [MEASURED — SLOW-ENVELOPE STILL SOURCES] | Locked 3³ live Poisson: \(r_L=0.9546\). Freeze not recovered at \(27/125\) (\(\delta_a=3.18\)). v1 UNDERDETERMINED on P6. |
| [PREREG_CLOCK_HYPOTHESIS_DERIVATION_v3.md](foundational_mechanics/PREREG_CLOCK_HYPOTHESIS_DERIVATION_v3.md) | [PRE-REGISTRATION] | Locks the design of the v3 attempt to substrate-derive the clock hypothesis used implicitly in SPEC_FTD_LAGRANGIAN.md §4.3 (Arc B P2); the run closed negative (archived). |
| [DERIV_QFT_GRT_BRIDGE.md](foundational_mechanics/DERIV_QFT_GRT_BRIDGE.md) | [THEOREM] + [SELECTION] | (Also QFT) — provides T_μν, the gravitational source, via Noether. |


## Quantum mechanics, measurement, and Bell

QM as lattice statistics, the Born rule, the singlet, and Bell-inequality emergence.

| File | Tag | Purpose |
|---|---|---|
| [DERIV_QM_FROM_LATTICE.md](quantum_mechanics/DERIV_QM_FROM_LATTICE.md) | derivation (QM as statistics) | Quantum mechanics as the statistics of definite lattice events. |
| [DERIV_QUANTUM_MECHANICS_RESOLVED.md](archive/DERIV_QUANTUM_MECHANICS_RESOLVED.md) | [AXIOM]→[THEOREM]/[SELECTION]/[CONJECTURE] | The full logical chain from the First Distinction to every QM rule. |
| [DERIV_DIRAC_FROM_MASTER_QUADRATIC.md](quantum_mechanics/DERIV_DIRAC_FROM_MASTER_QUADRATIC.md) | derivation, honest assessment | The Dirac equation as the complex regime of the master quadratic. |
| [DERIV_SPIN_STATISTICS_BRIDGE.md](quantum_mechanics/DERIV_SPIN_STATISTICS_BRIDGE.md) | [THEOREM] + [SELECTION] | Spin-statistics from two-lemniscate geometry; G\* as boson-fermion bridge. |
| [DERIV_OBSERVER_BELL_MECHANISM.md](quantum_mechanics/DERIV_OBSERVER_BELL_MECHANISM.md) | [SELECTION] | Three-level observer hierarchy producing S = 2√2 from a local substrate. |
| [DERIV_BELL_COSINE_FROM_GAUSS.md](quantum_mechanics/DERIV_BELL_COSINE_FROM_GAUSS.md) | [THEOREM] + [SELECTION] | The Gauss constraint produces E(θ) = −cos θ and the Tsirelson bound. |
| [DERIV_SINGLET_FROM_VOID_EVENT.md](quantum_mechanics/DERIV_SINGLET_FROM_VOID_EVENT.md) | [THEOREM] + [SELECTION] | The void event `0 → (+1)+(−1)` produces the spin-1/2 singlet; closes the Bell loop. |
| [DERIV_KCOMP_VOLUMETRIC_SHELL.md](foundational_mechanics/DERIV_KCOMP_VOLUMETRIC_SHELL.md) | [SELECTION] / [THEOREM] / [EMERGENT] | The K_comp shell: dynamical mechanism for non-factorizable joint probabilities. |
| [DERIV_BORN_PROPORTIONALITY.md](DERIV_BORN_PROPORTIONALITY.md) | [DERIVED — conditional candidate mechanism] *(demoted 2026-07-02, FTD-0356; was [THEOREM])* | Born-rule excess-upcrossing-rate mechanism under an `[IMPOSED]` Langevin noise ensemble; T1c (probability = energy density) stays `[OPEN]` per FTD-0187. |
| [ANALYSIS_PL1_QUANTIFIED_DEVIATION.md](quantum_mechanics/ANALYSIS_PL1_QUANTIFIED_DEVIATION.md) | [DERIVED — conditional on the [IMPOSED] Langevin model] | Hermite deviation tower of Rice statistics vs Born, pushed to higher order within the DERIV_BORN_PROPORTIONALITY model; zero promotions (FTD-0187/FTD-0200 unchanged). |

## Atomic dynamics, pilot-wave, and time-dilation campaigns

The FTD-0270/0271/0278/0279/0252/0268 arc: given an imposed rest-mass clock, FTD becomes a conditional pilot-wave theory; the native (clock-free) lattice quantizes but with the wrong dispersion for atomic spectra; and a moving lattice clock's dilation law is measured against √(1−v²). All entries here are `[CONDITIONAL]`/`[MEASURED]`/`[BOUNDARY]` — no unconditional QM derivation is claimed.

| File | Tag | Purpose |
|---|---|---|
| [DERIV_GUIDANCE_ABSENCE_NOGO.md](foundational_mechanics/DERIV_GUIDANCE_ABSENCE_NOGO.md) | [THEOREM at engine level] + [MEASURED] | By-inspection no-go: the complete FTD force law contains no phase-gradient guidance term; formalizes the FTD-0271 dynamic-null result. |
| [PREREG_DE_BROGLIE_CLOCK_v1.md](foundational_mechanics/PREREG_DE_BROGLIE_CLOCK_v1.md) | [PRE-REGISTRATION — design locked before the run of record] | Locks the design for testing whether an imposed Klein-Gordon rest-mass clock turns FTD into a single-particle pilot-wave theory (FTD-0271). |
| [ANALYSIS_DE_BROGLIE_CLOCK_v1.md](foundational_mechanics/ANALYSIS_DE_BROGLIE_CLOCK_v1.md) | [CONDITIONAL — DERIVED-GIVEN-IMPOSED-INPUT] | Run of record: given the imposed clock, a moving cluster carries a de Broglie matter wave and its non-relativistic envelope obeys Schrödinger (FTD-0271). |
| [PREREG_QUANTIZATION_LATTICE_MODES_v1.md](foundational_mechanics/PREREG_QUANTIZATION_LATTICE_MODES_v1.md) | [PRE-REGISTRATION — design locked before the run of record] | Locks the design for testing lattice mode quantization without importing ℏ (FTD-0270). |
| [ANALYSIS_QUANTIZATION_LATTICE_MODES_v1.md](foundational_mechanics/ANALYSIS_QUANTIZATION_LATTICE_MODES_v1.md) | [MEASURED — BOUNDARY] | The lattice does quantize (discrete standing-wave eigenmodes) but carries linear/cavity dispersion, structurally the wrong dispersion for atomic spectra (FTD-0270). |
| [PREREG_QDYN_MASS_GAP_v1.md](foundational_mechanics/PREREG_QDYN_MASS_GAP_v1.md) | [PROTOCOL — to be hash-locked before the run] | Locks the design for testing whether the nonlinear genesis↔Gauss loop generates a native k=0 rest-mass gap (FTD-0270 closure swing, P2). |
| [ANALYSIS_QDYN_MASS_GAP_v1.md](foundational_mechanics/ANALYSIS_QDYN_MASS_GAP_v1.md) | [MEASURED — INVALID per pre-reg] | The v1 sweep trips the frozen instability gate on every run; a strong no-gap hint is recorded but not claimable from compromised runs (FTD-0333). |
| [ANALYSIS_QDYN_MASS_GAP_v2.md](foundational_mechanics/ANALYSIS_QDYN_MASS_GAP_v2.md) | [MEASURED — CLOSED-NEGATIVE] | Stable-integrator v2 re-run: the nonlinear loop freezes the flux rather than gapping it; FTD-0270's dispersion boundary is hardened (FTD-0362, supersedes FTD-0333). |
| [ANALYSIS_ATOMIC_SPECTROSCOPY_ENGINE_v1.md](foundational_mechanics/ANALYSIS_ATOMIC_SPECTROSCOPY_ENGINE_v1.md) | [CONDITIONAL — DERIVED-GIVEN-IMPOSED-INPUT] + [MEASURED — engine↔operator consistency, sparse-regime] + [BOUNDARY — engine FFT readout] | Engine-native FFT spectroscopy of hydrogen: operator↔engine consistency confirmed sparse-regime; the excited hydrogen ladder is finite-size-resolvable but the engine's native FFT readout cannot extract it (FTD-0281/0308). |
| [ANALYSIS_HYDROGEN_LATTICE_SPECTRUM_v1.md](foundational_mechanics/ANALYSIS_HYDROGEN_LATTICE_SPECTRUM_v1.md) | [CONDITIONAL — DERIVED-GIVEN-IMPOSED-INPUT] (narrowed — see correction below) | Hydrogen-like spectrum run of record; the 1s ground state is genuinely bound and Coulombic (FTD-0278 Leg 1). |
| [CORRECTION_FTD0278_HYDROGEN_MULTIPLET.md](foundational_mechanics/CORRECTION_FTD0278_HYDROGEN_MULTIPLET.md) | [CORRECTION] | The "n=2 multiplet / Rydberg ladder" claim is overclaimed — torus continuum-mode degeneracy, not bound 2s/2p orbitals; narrows the verdict to HYDROGEN-1s-CONFIRMED. |
| [ANALYSIS_HELIUM_LATTICE_SCF_v1.md](foundational_mechanics/ANALYSIS_HELIUM_LATTICE_SCF_v1.md) | [CONDITIONAL — DERIVED-GIVEN-IMPOSED-INPUT] | Given three motivated imports (clock, scalar coupling, mode-occupancy), the engine's own Gauss-law Green's function produces a mean-field helium atom with correct screening and ionization physics (FTD-0279). |
| [PREREG_DYNAMICAL_TIME_DILATION_v1.md](foundational_mechanics/PREREG_DYNAMICAL_TIME_DILATION_v1.md) | [PRE-REGISTRATION] | Locks design + analysis for a moving-lattice-clock dilation-law campaign (Campaign 2, FTD-0252). |
| [PREREG_DYNAMICAL_TIME_DILATION_v2.md](foundational_mechanics/PREREG_DYNAMICAL_TIME_DILATION_v2.md) | [PRE-REGISTRATION] | Supersedes v1's T2 sub-question to probe the un-probed IR limit of the dilation law. |
| [ANALYSIS_DYNAMICAL_TIME_DILATION.md](foundational_mechanics/ANALYSIS_DYNAMICAL_TIME_DILATION.md) | [OBSERVATION] | D(v)=√(1−v²) is an algebraic identity of the wave construction, realized to <0.06% at low velocity; UV lattice corrections bend D below γ at higher velocity (FTD-0252). |
| [PREREG_TIME_DILATION_L257_BLIND_v1.md](foundational_mechanics/PREREG_TIME_DILATION_L257_BLIND_v1.md) | [PRE-REGISTRATION] | Locks a blind L=257 extrapolation test of the FTD-0252 residual law before any L=257 data exists. |
| [ANALYSIS_TIME_DILATION_L257_BLIND_v1.md](foundational_mechanics/ANALYSIS_TIME_DILATION_L257_BLIND_v1.md) | [MEASURED — blind extension] | Blind L=257 run: PREDICTION_CONFIRMED (7/9 groups inside their locked intervals); nothing promoted (FTD-0268). |

## G\* provenance and the FQCR program

Where the bridge constant G\* comes from, and finite-N approximations.

| File | Tag | Purpose |
|---|---|---|
| [DERIV_GSTAR_QUARTER_CONJUGACY.md](foundational_mechanics/DERIV_GSTAR_QUARTER_CONJUGACY.md) | [THEOREM] | G\* as a ζ-regularized determinant ratio of quarter-twisted spectra (FQCR Model I, FTD-0141). |
| [DERIV_GSTAR_FINITE_APPROX.md](foundational_mechanics/DERIV_GSTAR_FINITE_APPROX.md) | [THEOREM] | Finite-N attractor `G*_N → G*` at a controlled rate (FQCR Model II, FTD-0142). |

## FTD-0110 cluster-mass bridge

The linear→nonlinear bridge connecting cluster size to particle mass.

| File | Tag | Purpose |
|---|---|---|
| [DERIV_K_FROM_OH_A1G_MULTIPLICITY.md](foundational_mechanics/DERIV_K_FROM_OH_A1G_MULTIPLICITY.md) | [DERIVED at linear level] (nonlinear coefficient origin [OPEN]/[SMC] per the FTD-0110 audit) | `k = 1/N_base = 1/4` from O_h representation theory of the 27-block. |
| [DERIV_FTD0110_NONLINEAR_BRIDGE.md](foundational_mechanics/DERIV_FTD0110_NONLINEAR_BRIDGE.md) | Bridge-I [DERIVED]; **§6 closure REVERTED** by [AUDIT_FTD0110_2026-05-27_RESOLUTION.md](../07_assessment/audits/AUDIT_FTD0110_2026-05-27_RESOLUTION.md) — bridge [OPEN] | Global O_h-equivariance (solid) + the disputed §6 theorems (carry the audit's CAUTION banner). |
| [EXPLR_FTD0110_MECHANISM_ALPHA_LEAKAGE_CLOSED.md](foundational_mechanics/EXPLR_FTD0110_MECHANISM_ALPHA_LEAKAGE_CLOSED.md) | [VERIFIED lemma] + [CLOSED NEGATIVE — Mechanism α leakage family] (FTD-0259) | λ(r) → (2/3)/r² verified; parameter-free drift model falsified; genesis is basis-free so irrep re-projection ≠ harvest loss; queued α projection calculation retired; Langevin knee A* = 12.8 elevates Mechanism γ; decisive next: thermostat-OFF re-sweep. |
| [PREREG_THERMOSTAT_OFF_AMPLITUDE_SWEEP_v1.md](foundational_mechanics/PREREG_THERMOSTAT_OFF_AMPLITUDE_SWEEP_v1.md) | [PRE-REGISTRATION — locked `4fa056c2`, tag applied] (FTD-0260) | The Mechanism-γ discriminator design: arms C/X/G/T, mechanical gates V-1/V-2, outcome map, hash locks. |
| [PREREG_SUBKNEE_BLOCK_HYPOTHESIS_v1.md](foundational_mechanics/PREREG_SUBKNEE_BLOCK_HYPOTHESIS_v1.md) | [PRE-REGISTRATION — locked `5e26ac7b`, tag applied] (FTD-0263) | The 27-block onset hypothesis with its aesthetic-capture guard: three frozen invariance kill-tests; staircase descriptive-only. |
| [ANALYSIS_SUBKNEE_BLOCK_HYPOTHESIS_v1.md](foundational_mechanics/ANALYSIS_SUBKNEE_BLOCK_HYPOTHESIS_v1.md) | [MEASURED — GEOM-PARTIAL] (FTD-0263) | **C1 killed the sharp block reading (elbow N = 14.6, outside [19,33]); C2/C3 passed — the onset is local + smooth.** Six-point constraint profile for future mechanisms; bonus: bulk-branch L-invariance exact (N(30)=45.0 at L=24/32/48). |
| [ANALYSIS_GENESIS_SURVIVAL_TELEMETRY_v1.md](foundational_mechanics/ANALYSIS_GENESIS_SURVIVAL_TELEMETRY_v1.md) | [MEASURED — SURVIVAL-NULL] + [CLOSED NEGATIVE — β post-genesis-survival reading] (**FTD-0267**) | **First direct engine measurement of genesis/evaporation EVENTS** (observation-only counters, golden-neutral). At A=10 the engine fires ~5 (not β's 23); cum_genesis==peak_manifested always ⇒ one-shot burst; evap≈0 ⇒ high survival. **β premise FALSIFIED — suppression is genesis-stage nonlinear throttling, cluster ≈ genesis count.** Converges with the concurrent β v2 (FTD-0263). Surfaced a pre-existing golden regression (c2a8f606 left GOLDEN_HASH stale). |
| [DERIV_BETA_KINETICS_ATTEMPT_v1.md](archive/closed_negative/DERIV_BETA_KINETICS_ATTEMPT_v1.md) | [COMPUTED — DWELL-FAIL] + [CLOSED NEGATIVE — all pre-genesis-kinetics β variants] (**FTD-0266**) | Dwell-time Boltzmann correction negligible (N_c=3 makes rate step-function; P_fire≥0.82 for 1 tick); E[N(10)]=23–24 vs 4.0. (NB: superseded by FTD-0267's direct measurement — the engine fires ~5, not 23; the "post-genesis survival" framing here is the reading FTD-0267 closes.) Engine-free route exhausted. |
| [DERIV_BETA_ENVELOPE_ATTEMPT_v1.md](archive/closed_negative/DERIV_BETA_ENVELOPE_ATTEMPT_v1.md) | [COMPUTED — BETA-PARTIAL] + [CLOSED NEGATIVE — envelope-only β] (**FTD-0265**) | The verdict-of-record write-up: shape FAIL both variants (over-counts 3–6×); T1 pass recorded as hollow (knee_A 5.8 vs 13.5); sharpened [OPEN] = sustained-kinetics β (dwell time + survival); seventh constraint (~3–6× crossing suppression). |
| [ANALYSIS_BETA_ENVELOPE_MODEL_v1.md](archive/closed_negative/ANALYSIS_BETA_ENVELOPE_MODEL_v1.md) | [MEASURED — BETA-PARTIAL] (companion analysis; LEDGER row **FTD-0265** — the earlier 0264 label collided with the blocked-effective-action row) | Companion write-up of the same runner: dispersion check corrected (exact symplectic-Euler arcsin formula), elbow knee_N = 13.1 in target range, shape RMS = 0.749 FAIL; kinetics/back-reaction load-bearing. |
| [PREREG_SM_MASS_IDENT_CURRENT_STACK_v1.md](foundational_mechanics/PREREG_SM_MASS_IDENT_CURRENT_STACK_v1.md) | [PRE-REGISTRATION — locked `2adf80b1`, tag applied] (FTD-0262) | SM identification re-assessment design: anchor / circular-flagged consistency / specialness probe; anti-target windows frozen. |
| [ANALYSIS_SM_MASS_IDENT_CURRENT_STACK_v1.md](foundational_mechanics/ANALYSIS_SM_MASS_IDENT_CURRENT_STACK_v1.md) | [MEASURED — IDENT-NULL] (FTD-0262) | **Anchor PASS (20/20 exact 1-voxel electron); law extrapolates μ/π off-grid at 3–4 %; specialness SMOOTH (p_local = 2.052) — no attractor at R_μ.** FTD-0110 [SMC] support inventory: historical stack-pinned + anchor + nothing else. |
| [PREREG_NA_LAW_CURRENT_STACK_v1.md](foundational_mechanics/PREREG_NA_LAW_CURRENT_STACK_v1.md) | [PRE-REGISTRATION — locked `be63223e`, tag applied] (FTD-0261) | Current-stack N(A) characterization + thermostat discriminator v2 design: coupling-ON protocol, knee-densified grid, frozen flooding rule + mechanical verdicts. |
| [ANALYSIS_NA_LAW_CURRENT_STACK_v1.md](foundational_mechanics/ANALYSIS_NA_LAW_CURRENT_STACK_v1.md) | [MEASURED — current-stack baseline] + [Outcome A] + [CLOSED NEGATIVE — thermal-knee] (FTD-0261) | **The current-stack law: broken power, knee A≈16 (p 3.69→1.86, k_eff≈0.05)**; thermostat shapes it (median ratio 1.61) via **pure friction** (γ-monotone, T-flat) — FTD-0259's thermal-crossover reading closed. V-1 5/5; first valid discriminator run. |
| [ANALYSIS_THERMOSTAT_OFF_SWEEP_v1_INVALID.md](archive/invalid/ANALYSIS_THERMOSTAT_OFF_SWEEP_v1_INVALID.md) | [INVALID RUN — V-1 0/11] + [OBSERVATION — corrected §4: environment-class break] (FTD-0260) | Run of record + same-day double correction: tracked code EXCLUDED (April source rebuilt today reproduces the broken values — bisect withdrawn); backend EXCLUDED (canonical test fails 0/3 on CPU **and** CUDA). Historical FTD-0110 baseline reproduces on **no** available combination — environment forensics [OPEN]; `gpc_03` existence-only parity hole standing; v2 blocked on both backends. |
| [DERIV_FTD0110_FREE_ENERGY_LANDSCAPE.md](foundational_mechanics/DERIV_FTD0110_FREE_ENERGY_LANDSCAPE.md) | [DERIVED] framework / [PARTIAL] parameters | Cluster phenomenology as a multi-basin free-energy landscape. |
| [DERIV_FTD0110_VARIANCE_ENTROPY.md](foundational_mechanics/DERIV_FTD0110_VARIANCE_ENTROPY.md) | [PARTIAL] | Cluster-size variance as boundary entropy; tested against engine data. |
| [EXPLR_FTD_0110_NONLINEAR_BRIDGE_ANALYSIS.md](foundational_mechanics/EXPLR_FTD_0110_NONLINEAR_BRIDGE_ANALYSIS.md) | [PARTIAL] (exploratory) | Sharpens the [OPEN] nonlinear-bridge gap; does not close it. |
| [DERIV_AP_NO_OVER_COUNT.md](foundational_mechanics/DERIV_AP_NO_OVER_COUNT.md) | [DERIVED] / canonical | Active-block partitioning aggregation rule fixing the 27-block over-counting bug; recovers k=1/4 exactly and predicts the empirical k(A) drift to 5.5%. |
| [DERIV_KINETIC_DRAIN_FROM_QUADRATURE.md](foundational_mechanics/DERIV_KINETIC_DRAIN_FROM_QUADRATURE.md) | [CLOSED NEGATIVE] (drain² origin) + [MEASURED] (k_eff(drain), γ-map) | Tests whether the kinetic drain and its ¼ coefficient are derivable; drain² scaling and quadrature-equipartition routes both fail (FTD-0276). |
| [PREREG_FTD0110_NA_LAW_v1.md](foundational_mechanics/PREREG_FTD0110_NA_LAW_v1.md) | [PRE-REGISTRATION — design locked before any run-of-record] | Locks the test of whether a substrate-parameter forward model (coupling source + Gauss boost) reproduces the full N(A) law. |
| [ANALYSIS_FTD0110_NA_LAW.md](foundational_mechanics/ANALYSIS_FTD0110_NA_LAW.md) | [MEASURED — BOUNDARY] | The extended forward model reproduces N(A)'s geometric shape, but its absolute calibration rests on non-framework engine-tuning constants (FTD-0269). |
| [PREREG_FTD0110_CONVENTION_AUDIT_v1.md](foundational_mechanics/PREREG_FTD0110_CONVENTION_AUDIT_v1.md) | [PRE-REGISTRATION — design locked before any run-of-record] | Locks the test of whether the N(A) calibration is pure gauge convention or physical content. |
| [ANALYSIS_FTD0110_CONVENTION_AUDIT_v1.md](foundational_mechanics/ANALYSIS_FTD0110_CONVENTION_AUDIT_v1.md) | [MEASURED — BOUNDARY: exit (ii) CLOSED NEGATIVE] | The N(A) calibration is PHYSICAL on both engine knobs (drain, γ) — no "only the dimensionless shape is physical" escape (FTD-0307). |
| [SCOPE_GENESIS_COUNTING_MODEL.md](foundational_mechanics/SCOPE_GENESIS_COUNTING_MODEL.md) | [SCOPE — design document] | Defines the target and frozen falsifier classes for a conditional derivation of the N(A) law (Arc 3, FTD-0277); the v1 slosh-pass route closed negative. |
| [ANALYSIS_GENESIS_COUNTING_V2.md](foundational_mechanics/ANALYSIS_GENESIS_COUNTING_V2.md) | [MEASURED — BOUNDARY: collective-coordinate reduction obstructed] + [DERIVED — super-knee exponent, given imposed register] | The v2 genesis-counting model: super-knee exponent p_hi=2 derives, sub-knee mechanism derives, the calibration coefficient stays engine-emergent (FTD-0309). |
| [ANALYSIS_GENESIS_AMPLITUDE_CEILING_v1.md](foundational_mechanics/ANALYSIS_GENESIS_AMPLITUDE_CEILING_v1.md) | [EMERGENT / MEASURED — DERIVED-FROM-RULE] (numeric threshold [IMPOSED]-conditional; exact-ceiling reading [RETRACTED — FTD-0567]) | A sharp, wavelength-invariant genesis ignition threshold at \|J\|=K_GENESIS, with measured crest regulation above it (FTD-0316, corrected by FTD-0567). |
| [ANALYSIS_INFO_CREATION_v1.md](foundational_mechanics/ANALYSIS_INFO_CREATION_v1.md) | [MEASURED / EMERGENT] | At fixed energy and \|J\| histogram, coherent flux produces far more organized manifested matter than a scrambled control — information is fuel for organized form, not for quantity of matter (FTD-0317). |
| [DERIV_CLUSTER_COLLECTIVE_COORDINATE_v1.md](foundational_mechanics/DERIV_CLUSTER_COLLECTIVE_COORDINATE_v1.md) | [PARTIAL — obstruction named] ([DERIVED conditional on GNC]) | Rigid cluster translation costs N·M_REST·v iff the dressed flux profile satisfies the Gradient-Normalization Condition; neither profile the framework pins down satisfies it (FTD-0349). |
| [LEMMA_GNC_RIGIDITY.md](foundational_mechanics/LEMMA_GNC_RIGIDITY.md) | [THEOREM] (rigidity lemma, divergence identity, affine classification) + [CONJECTURE] (two-walls-one-shape unification) | GNC-at-a-site is exactly a K_B-scaled local isometry of the flux Jacobian; classifies the affine GNC solution strata (FTD-0354). |
| [PREREG_GNC_QIJ_v1.md](foundational_mechanics/PREREG_GNC_QIJ_v1.md) | [PROTOCOL — to be hash-locked before the run] | Locks the Q_ij discriminator design for testing whether locked, Gauss-dressed engine clusters realize GNC-w (FTD-0349 §9). |
| [ANALYSIS_GNC_QIJ_v1.md](foundational_mechanics/ANALYSIS_GNC_QIJ_v1.md) | [MEASURED — INVALID / re-scope] | All 18 rows fail the frozen Gauss-residual gate by 4–5 orders of magnitude; re-scope to a stencil-matched v2, no tag moves (FTD-0363). |
| [PREREG_BETA_ENVELOPE_MODEL_v2.md](foundational_mechanics/PREREG_BETA_ENVELOPE_MODEL_v2.md) | [PRE-REGISTRATION] | Refines the sub-knee envelope model with genesis back-reaction (kinetic drain, flux drain, Gauss projection); supersedes v1's linear Variant A/B formulation. |
| [SPEC_FTD0110_BRIDGE_BOUNDARY.md](foundational_mechanics/SPEC_FTD0110_BRIDGE_BOUNDARY.md) | [SYNTHESIS / BOUNDARY HARDENED] | Canonical status map of the FTD-0110 bridge: linear k=1/4 [DERIVED]; nonlinear N(A) [OPEN], boundary hardened on three independent axes. |

## Rest-mass and calibration: attempts and boundaries

Attempts to derive a substrate-native rest-mass scale or fix the dimensionless→dimensionful calibration gate; the energy-based and topological-charge routes both close without a mass identification.

| File | Tag | Purpose |
|---|---|---|
| [DERIV_DIMENSIONAL_GATE.md](foundational_mechanics/DERIV_DIMENSIONAL_GATE.md) | [DERIVED — schema-level] + [CORRECTION] (t_phys value) | The dimensionless→dimensionful gate: exactly 3 calibration constants are required (one per base dimension, ℤ³-graded); corrects a factor-3 arithmetic error in t_phys propagated from FTD-0041. |
| [DERIV_ELECTRON_MASS_ANCHOR.md](foundational_mechanics/DERIV_ELECTRON_MASS_ANCHOR.md) | [STRONGLY MOTIVATED CONJECTURE] | The 16/3 prefactor of the electron-mass anchor `m_e = m_P·√(2π)·(16/3)·α¹¹`; the earlier [THEOREM] "dimensional equipartition" promotion is RETRACTED as a substitution identity. |
| [DERIV_REST_MASS_FROM_CONSTRAINT_ENERGY.md](foundational_mechanics/DERIV_REST_MASS_FROM_CONSTRAINT_ENERGY.md) | [CLOSED — §5 identification REFUTED] | Tests M_REST = W_SC (Gauss-constrained self-energy); the genesis-born locked energy is circumstance-dependent (9.2× spread across birth energies, monotonic) — no single mass excess exists for this route. |
| [DERIV_REST_MASS_FROM_TOPOLOGICAL_CHARGE.md](foundational_mechanics/DERIV_REST_MASS_FROM_TOPOLOGICAL_CHARGE.md) | [TERMINAL UNDERDETERMINATION] | Tests the hedgehog charge of the flux direction map as a topological rest-mass invariant; the charge is robustly ZERO at freeze — no mass evidence, no further shell redesign licensed (FTD-0398). |
| [EXPLR_COAT_PEIERLS_CLOCK_TOY.md](foundational_mechanics/EXPLR_COAT_PEIERLS_CLOCK_TOY.md) | [EXPLORATORY — Python toy] + [SPECULATION — every physical reading] | Toy probe of a dressed coat's Peierls self-potential; finds a local internal-clock frequency ω₀, but the predicted 2D landscape separability is refuted (10% nonseparability residual, FTD-0565). |
| [EXPLR_VOXEL_NEIGHBORHOOD_DYNAMICS.md](foundational_mechanics/EXPLR_VOXEL_NEIGHBORHOOD_DYNAMICS.md) | [SYNTHESIS] + [DERIVED — lattice correctness] | Complete microdynamical reference for one voxel and its 26-site Moore neighborhood in a single tick: state vector, free/sourced sectors, genesis/evaporation kinetics, and the K_B/K_GENESIS stability window. |

## Hadrons, confinement, and Yukawa structure

QCD-sector derivations: confinement, pion mass, the electron Yukawa prefactor.

| File | Tag | Purpose |
|---|---|---|
| [DERIV_CONFINEMENT_FROM_GAP_EQUATION.md](foundational_mechanics/DERIV_CONFINEMENT_FROM_GAP_EQUATION.md) | [THEOREM] confinement at x₋ / [SELECTION] QCD ID | Area-law Wilson loops + linear confinement from the master quadratic's x₋ root. |
| [DERIV_PION_MASS_FROM_GSTAR.md](standard_model/DERIV_PION_MASS_FROM_GSTAR.md) | 6 [THEOREM] + 3 [SELECTION] + 5 [CONJECTURE] | The 15-step G\* → m_π chain; integer-reduction theorem; 0.048% match. |
| [DERIV_YUKAWA_FROM_27BLOCK_CHARACTER.md](standard_model/DERIV_YUKAWA_FROM_27BLOCK_CHARACTER.md) | [STRUCTURALLY MOTIVATED PARAMETRIC] | The electron Yukawa prefactor `16√2/3` from O_h character theory (FTD-0134). |
| [DERIV_YANG_MILLS_CONFINEMENT.md](DERIV_YANG_MILLS_CONFINEMENT.md) | [MEASURED at an inserted coupling [SELECTION]] (reconciled) | Engine-measured Wilson-loop area-law confinement signature at β = x₋; the full Yang-Mills proof is RETRACTED (FTD-0042) — only the per-voxel mass gap (FTD-0044) survives as [THEOREM] (LEDGER FTD-0303). |
| [DERIV_SUBSTRATE_YUKAWA_VERTEX_RETRACTED.md](archive/closed_negative/DERIV_SUBSTRATE_YUKAWA_VERTEX_RETRACTED.md) | ~~[DERIVED]~~ **[CLOSED NEGATIVE]** (archived 2026-08-06) | Restated FTD-0135's rejected vertex-derivation sketch as `[DERIVED]`, 2 months after its closure, with no LEDGER id. Content merged into `DERIV_YUKAWA_FROM_27BLOCK_CHARACTER.md` §4.5. |

## Foundational chains and resolutions

Bottom-up overview docs and gap-closing arguments.

| File | Tag | Purpose |
|---|---|---|
| [DERIV_BOTTOM_UP_PHYSICS.md](foundational_mechanics/DERIV_BOTTOM_UP_PHYSICS.md) | [AXIOM]→[THEOREM]/[SELECTION]/[CONJECTURE] | Entry-point: from `0 = (−1)+(+1)` to the Standard Model. |
| [MONOGRAPH_EFFECTIVE_EQUATIONS.md](foundational_mechanics/MONOGRAPH_EFFECTIVE_EQUATIONS.md) | [AXIOM]→[THEOREM]/[SELECTION]/[PARAMETRIC] | Comprehensive monograph of effective continuum equations. *(Corrected 2026-07-02, FTD-0356: §§5.1–5.3 Dirac/Schrödinger/Born rows retagged to canon — Dirac/Schrödinger are `[PARAMETRIC]` imported QM conditional on the imposed clock (FTD-0271); Born is `[SELECTION]` (form) + `[OPEN]` (T1c) per FTD-0187/0200; the §5.3/§7 path-integral Born proof it cited is RETRACTED.)* *(Corrected 2026-07-02, FTD-0361: §4.4 $g_{rr}$ row RETRACTED — the cited proof contradicts its own premise, $g_{rr}$ stays [GAP]; §5.4 Compton row demoted `[PARAMETRIC — conditional]`. All four §7.2 claimed gap-resolutions now adjudicated.)* |
| [DERIV_COMPTON_INVERSION_RESOLUTION.md](foundational_mechanics/DERIV_COMPTON_INVERSION_RESOLUTION.md) | **[PARAMETRIC — conditional]** *(demoted 2026-07-02, FTD-0361; was [THEOREM])* | The $\lambda_C = 3a^3/(4\pi K_B R^3)$ algebra is sound but substitutes between two imposed inputs — the massive envelope equation (`[IMPOSED]`, FTD-0271 A0) and the pole-mass = $K_B N$ identification (FTD-0250 reduction `[OPEN]`). The paradox dissolution survives at [SELECTION] grade; the "duality theorem" does not. |
| [DERIV_NONCOMMUTATIVE_EMERGENCE_RETRACTED.md](archive/retracted/DERIV_NONCOMMUTATIVE_EMERGENCE_RETRACTED.md) | **[RETRACTED 2026-07-01]** | The "Boundary Partition Commutator Theorem" proof is invalid (non-sequitur at step 7; unsupported Type III₁ claim contradicting FTD-0225). GAP-S2 is CLOSED DECLINED under FC-1. Archived for provenance (FTD-0348). |
| [DERIV_RADIAL_METRIC_RESOLUTION_RETRACTED.md](archive/retracted/DERIV_RADIAL_METRIC_RESOLUTION_RETRACTED.md) | **[RETRACTED 2026-07-02]** *(was [THEOREM])* | The "Discrete Radial Metric Projection Theorem" proof is invalid — its concluded metric's radial null speed $C_0(1-r_s/r)$ contradicts its premised wave speed $C_0\sqrt{1-r_s/r}$; consistent accounting of its own premises forces $g_{rr}=1$. $g_{rr}$ remains [GAP] (Gap 10.1/11.1; FTD-0189). Archived for provenance (FTD-0360/FTD-0361, independent convergent passes). |
| [DERIV_BORN_PROPORTIONALITY_RESOLUTION_RETRACTED.md](archive/retracted/DERIV_BORN_PROPORTIONALITY_RESOLUTION_RETRACTED.md) | **[RETRACTED 2026-07-02]** *(was [THEOREM])* | The "Path-Integral Born Proportionality Theorem" proof is invalid — its own exact result is affine in $|\psi|^2$ with **negative** slope (anti-Born); the unexpanded form is Rice statistics (FTD-0200 [CLOSED NEGATIVE]). FTD-0187 remains [SELECTION] (form) / [OPEN] (T1c). Archived for provenance (FTD-0356). |
| [DERIV_THREE_RESOLUTIONS.md](foundational_mechanics/DERIV_THREE_RESOLUTIONS.md) | derivation (closes three gaps) | Compact U(1), bare = physical, one-loop exact — answered by the tick. |

---

## Engine-native overlay primitives

Pure header-only theory overlays on the existing lattice engine. No `RenderBridge` touch; the golden-tick hash is preserved. Each module is a primitive header plus a `[unit]` ctest target.

| File | Tag | Purpose |
|---|---|---|
| [DERIV_BRANCH_HOLONOMY_GAP.md](foundational_mechanics/DERIV_BRANCH_HOLONOMY_GAP.md) | [THEOREM] | `λ_min = 4 sin²(π/(2N))` for the Z₂-twisted ring Laplacian (signed line-bundle gap on a periodic torus). FTD-0194. |
| [DERIV_Z3_CENTER_GRAPH_CLOSURE.md](standard_model/DERIV_Z3_CENTER_GRAPH_CLOSURE.md) | [THEOREM] (closure + projector) + [CANDIDATE PRINCIPLE] (open-flux penalty) | Z₃ center-closure characterisation `∑c_i ≡ 0 (mod 3)` + center projector `P₀ = (1/3)(I+Z+Z²)`. Open-flux penalty is NOT asserted. FTD-0195. |

---

151 active docs in this cluster (+ 16 archived; +1 UFF Q0, +1 UFF engine-align, +1 GW-holonomy Q0, +1 geometric integrator, +1 empirical-gravity catalog, +1 sourced geometric free-fall, +1 GPU geometric-gravity parity, +1 one-well redshift+falling, +1 frozen-well characteristic deflection, +1 live sourced Newton, +1 slow-envelope live Newton on 2026-08-19). This local index curates the load-bearing subset across the foundational_mechanics / electromagnetism / standard_model / quantum_mechanics / gravity_and_cosmology subdirectories; the master catalog is [`../META_INDEX.md`](../META_INDEX.md).
