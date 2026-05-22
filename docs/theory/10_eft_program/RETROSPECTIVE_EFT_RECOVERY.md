# RETROSPECTIVE — The EFT Recovery Program (2026-04 .. 2026-05)

**Tag:** [SYNTHESIS]
**Date:** 2026-05-22
**Status:** Narrative roll-up of a largely-completed program. Records what happened; introduces no new result, theorem, or claim, and changes no tag.
**Scope:** the `docs/theory/10_eft_program/` cluster — the multi-month attempt (April–May 2026) to recover a defensible effective field theory from the FTD lattice engine.

---

## 1 · Purpose & scope

The EFT Recovery Program generated ~89 top-level documents. Most are process scaffolding — protocols, pre-registrations, analyses, audits, and measurement triplets around campaigns that have since run and recorded their result. In the 2026-05-22 cluster consolidation, ~48 of those scaffolding documents were archived (`git mv` into `archive/closed_negative/` or `archive/campaign_complete/`), with every epistemic tag, closed-negative finding, and `FTD-NNNN` cross-reference preserved in the moved file and in `LEDGER.md`.

This document exists so the *narrative thread* survives in one readable place once the scaffolding is no longer top-level. It is a `[SYNTHESIS]`: it integrates existing claims at their canonical tags. Where it states a result, that result keeps the tag its canonical source assigns it; nothing here is promoted, re-derived, or re-graded.

The canonical survivors this retrospective points back to — read these for the load-bearing detail:

- `SPEC_EFT_RECOVERY_PROGRAM.md` — `[REFERENCE]`, the original pre-registered Phase 0–F program.
- `SPEC_FTD_EFT_BRIDGE_CONTRACT.md` — `[SELECTION]`, the 7-gate epistemic guardrail.
- `SPEC_FTD_NATIVE_BLOCKING_MAP.md`, `SPEC_FTD_NATIVE_ELECTRODYNAMICS.md` — the post-pivot native specs.
- `OPEN_FTD_TO_EFT_BRIDGE_STATUS.md` — the program's keystone honesty document.
- `DERIV_EMERGENT_COULOMB_GEOMETRIC.md` — `[THEOREM]`, the Phase-G geometric-Coulomb resolution.
- `THEOREM_A_PHYS_NO_GO.md`, `THEOREM_MU_NO_GO_FTD0096.md` — the two no-go theorems.
- `DERIV_PARTITION_FUNCTION_L2.md` — `[THEOREM at L=2]`, the ultralocality finding.
- `STATUS_EFT_CHECKLIST.md`, `STATUS_NONLINEAR_REGIME_2026-04-30.md` — live program-state trackers.

---

## 2 · The original program (Phase 0–F)

The program was pre-registered. `SPEC_EFT_RECOVERY_PROGRAM.md` (tag `[REFERENCE]`, 2026-04-19) committed to the repository — *before any measurement ran* — a five-pillar checklist for FTD to qualify as a Wilsonian effective field theory: a measured β-function, Ward-identity closure for composite operators, Lorentz covariance after rescaling by `c = 1/√3`, an operator expansion classifying lattice operators as relevant/marginal/irrelevant, and continuum matching with controlled `O(1/L^p)` error. The spec's §11 explicitly forbids editing a pre-registered expectation to match a measurement; §10 lists known limitations up-front (no lattice fermions, cubic-lattice rotation breaking at the `a`-scale, three blocking stages as the minimum for β extraction).

The pre-registered falsifiable target (§7.3) was sharp: the continuum-extrapolated `α_eff(∞)` should land within **1% of CODATA**.

**Outcome: NULL on the QED-α target.** The post-audit headline recorded in the spec itself is an `α_∞` plateau at **1.8–3.6× α_ref** across `L ∈ {64, 128, 256, 384}`, the range spanning the engine-internal versus classical energy-accumulator convention. The pre-registered 1%-of-CODATA target was not met under any convention. Per the program's own discipline, the spec was not edited to match — the miss was reported honestly and reframed downstream (§3 below). An interim Day-2 "1.23×" claim was retracted as under-equilibrated (ticks=100). Several pillars produced real positive content even as the α target failed — notably Lorentz anisotropy (§8).

---

## 3 · The QED-α projection route and why it closed

The first instinct — extract physical `α` by *projecting* the FTD flux/state sector onto a U(1) gauge theory and matching to QED — was pursued through a sequence of individually pre-registered or audited sub-attempts. All closed negative, and for structurally consistent reasons. `OPEN_FTD_TO_EFT_BRIDGE_STATUS.md` enumerates roughly eight such closed sub-attempts; among them:

- **Projected stiffness** — `K_T,0 = x₊`: the projected transverse stiffness is canonically `1`, not `x₊`. `[CLOSED NEGATIVE]` (`archive/closed_negative/`).
- **Projected response-eigenvalue** — `x₊` as a normal-mode eigenvalue of a coupled two-sector matrix: the projected action is block-diagonal; no such matrix is forced. `[CLOSED NEGATIVE]`.
- **Source-current normalization** — `e₀² = 1/x₊`: ternary source transport fixes *integer* charge and current conservation, not the physical coupling magnitude. `[CLOSED NEGATIVE]`.
- **Structure-2 Ward-valid scalar gauge completion** — `AUDIT_STRUCTURE2_WARD_VALIDATION.md`: a Ward-valid two-U(1) BCC scalar-loop calculation across five matter cases reproduces none of the Structure-1 ppb α correction (residuals +1257 to +6185 ppb against a ±30 ppb threshold). `[CLOSED NEGATIVE]`.
- **Projected-EFT matter coupling** — `DERIV_PROJECTED_EFT_MATTER_COUPLING.md`: the native current/coupling dictionary survives, but charge normalization `e² = α` is closed-negative under the projected action.
- **BCC tadpole regulator** — the unrenormalized BCC one-loop tadpole residual has no continuum limit (FTD-0056).

Two audits closed the *interpretation* of the Phase-F headline:

- `AUDIT_ALPHA_EXTRACTION.md` (`[AUDIT]`, LOAD-BEARING) audited the "3.6× α_ref plateau" line by line, confirmed the three V(r) codepaths are bit-consistent, identified a factor-2 energy-convention artifact, and retracted the Day-2 "1.23×" claim.
- `DERIV_EMERGENT_COULOMB_GEOMETRIC.md` (`[THEOREM]`, Phase-G resolution) supplied the closed form: the engine's emergent-forces `V(r)` is `V = −2·G_L(r)`, `α_r = 2·r·G_L(r)`, where `G_L` is the periodic lattice Poisson Green's function — a zero-free-parameter quantity verified against Phase-F data at `R² = 1.0000`, 0.07% median residual at L=384. The "plateau" was a category error: it is pure lattice geometry with **no fine-structure content**. The same doc records Phase H — inserting an explicit Gauss coupling `g_c` scales the measurement exactly as `g_c²·2rG_L(r)`, confirmed to 0.0000%.

The QED-α projection route was therefore not abandoned out of fatigue; it was closed by a chain of pre-registered falsifications and a theorem showing the headline observable carries no `α`.

---

## 4 · The 2026-04-22 methodological pivot

`OPEN_FTD_TO_EFT_BRIDGE_STATUS.md` (tag `[CURRENT-ACTION CLOSED NEGATIVE for QED alpha]`, 2026-04-22) is the program's keystone honesty document. It states plainly that the FTD→QED-α bridge is closed negative under the current projected action: the matching chain (FTD dynamics → unique continuum fields → unique matter content → unique operator/regulator → unique renormalized observable → physical α) has several steps that are *selected*, not *forced*. As long as matter content, kinetic operator, regulator, and α-observable are chosen rather than derived, any number that lands near CODATA is calibration or fitting — explicitly disallowed by the project's anti-near-miss rule.

The pivot redefined the target. Instead of "derive QED α," the program would **measure FTD's own native source/flux response coefficients in the lattice's own units**, and demote the QED comparison to a diagnostic. The bridge endpoint for the central conjecture was stated honestly: `x₊` remains arithmetic-only under the current projected action (route R4). This is the document the rest of the cluster is downstream of.

---

## 5 · The FTD-native blocking EFT (the post-pivot program)

The post-pivot program is governed by two specs. `SPEC_FTD_EFT_BRIDGE_CONTRACT.md` (`[SELECTION]`) freezes a seven-gate contract — field dictionary, blocking map, Gauss/continuity preservation, RG flow, operator basis, matter, matching/observables — and imposes a hard prohibition: no calling a QED formula filled with FTD numbers a "derivation." `SPEC_FTD_NATIVE_BLOCKING_MAP.md` (`[SELECTION]`) defines the finite-volume coarse-graining map `B_b` (the `b=2` transformation used program-wide), with Gauss-preservation and reaction-continuity preservation under the native map tagged `[THEOREM]`.

The constructive result of this program is real but modest, and the native-family docs are scrupulous in saying so. The bare linear FTD source/flux sector is a **free Gaussian theory with canonical normalization**: the native response tuple `(C_L, K_T, Z_j, g_sJ) = (1,1,1,1)`, derived for the linear G18 generator by constrained minimisation of flux energy under the Gauss constraint, and *preserved* (unit coefficients) under native `b=2` blocking. Per-quantity tags are mixed and honest: `C_L`/`c_FTD` `[THEOREM]`, `K_T`/`g_sJ` `[DEFINITION]`, `Z_j`/`W_18` `[MEASURED]`. A multiscale measurement at `b ∈ {1,2,4,8}` finds all three β-function estimates consistent with zero within 1σ — a stable Gaussian IR attractor (`[MEASURED]`, FTD-0070). A Langevin-thermostatted tick cycle has a unique stationary, equipartitioned ensemble (`[THEOREM]`, FTD-0069; standard Ornstein–Uhlenbeck theory).

This content is, in plain terms, "the bare linear lattice is a free theory with canonical normalization" — true, useful as a boundary marker, but carrying **no fine-structure content**. The native-family docs state exactly this. In the 2026-05-22 consolidation the eleven thin `DERIV_FTD_NATIVE_*` docs were merged into three consolidated docs (`..._RESPONSE_AND_BLOCKING`, `..._NONLINEAR_FLOW`, `..._HISTORY_ACTION`), every tag carried verbatim; the closure doc `DERIV_FTD_NATIVE_SOURCE_FLUX_COUPLING_CLOSURE.md` (`g_sJ = √α_QED` `[CLOSED NEGATIVE]`; native `g_sJ = 1` `[DEFINITION]`) was archived with the other closed routes.

The interesting physics, if it exists, must therefore live in the **nonlinear** sector. The blocked nonlinear effective action `S_eff` is `[OPEN]` — this is the cluster's central live deliverable (see §10).

---

## 6 · g_c from first principles — the three mechanisms

For the engine's geometric Coulomb to reproduce QED Coulomb, the Gauss coupling needs the specific value `g_c = √(2π·α_ref) ≈ 0.2141` (engine convention). `OPEN_GC_FROM_FIRST_PRINCIPLES.md` (`[OPEN]`, LOAD-BEARING) scopes whether that value is derivable, and addresses all three candidate mechanisms:

- **Mechanism A — topological / Dirac quantisation.** RULED OUT (2026-04-19, `test_wilson_topology.cpp`). FTD's flux `J ∈ ℝ³` is real-valued and non-compact; measured plaquette circulations are numerically zero with a continuous distribution. There is no discrete structure to quantise `g_c`. Making A viable would require compactifying `J` or adding magnetic-monopole sources — neither is in the theory.
- **Mechanism B — lattice-to-continuum matching.** `[OPEN]`. A standard lattice-gauge matching would relate a bare `g_c` to a continuum renormalised coupling via a one-loop coefficient — but FTD's engine is classical, with no explicit β-function to match against. This requires promoting the engine to a quantum path integral with an explicit UV regulator. It is a separate program; it remains genuinely open. (The companion `archive/closed_negative/DERIV_MECHANISM_B_GC_DERIVATION.md` records a closed attempt within this route.)
- **Mechanism C — self-consistent / gap-equation fixed point.** `[CLOSED NEGATIVE]` (FTD-0093, closed 2026-04-27 at `L ∈ {24,32,48}` with a non-monotonic ratio rejecting the predicted `λ₊/λ₋ ≈ 45.31`). `DERIV_PARTITION_FUNCTION_L2.md` supplies the structural reason: on the 2×2×2 torus, under the Gauss constraint, the FTD action `S_E` depends only on the *count* of manifested voxels, not their placement — the Lagrangian is **ultralocal** in the state field. Two dipoles at different separations give identical `S_E`. Therefore no classical variational principle can fix `g_c`.

Net: with A ruled out, C closed, and B open-but-not-classical, `g_c` remains `[OPEN]`. FTD's `α` is consequently an **algebraic prediction whose physical identification is `[SELECTION]`/`[STRONGLY MOTIVATED CONJECTURE]`**, not a dynamical derivation. The master quadratic's dual match (`x₊ ↔ 1/α`, `x₋ ↔ N_c`) lives in the motivic/algebraic structure — the Watson identity, CM curve periods, Moore-neighbourhood integers — not in the dynamical action.

---

## 7 · The no-go theorems (what the program proved it cannot do)

The program's two genuine `[THEOREM]`-grade physics-facing results are *negative* — and they are positive structural results in the sense of CLAUDE.md's goal clause 2: they map the boundary honestly.

- **`THEOREM_A_PHYS_NO_GO.md`** (`[THEOREM]`, FTD-0059). No quantity with SI length dimension is derivable from Axiom Zero alone; the lattice-to-physical conversion `a_phys` must be an external calibration. Corollary 3.1 extends this to mass, time, energy, temperature, and charge. The proof is a ring-algebra argument: every Axiom-Zero invariant lies in a dimensionless ring `R`, and no function of dimensionless quantities yields a dimensioned one (under the dimensional-analysis axioms of physics — a convention note the doc flags explicitly). Consequence: FTD has exactly two theorem-enforced calibrations (`a_phys ≡ ℓ_P`, `K_B = m_e`), and its dimensionless predictions are the falsifiable spine.
- **`THEOREM_MU_NO_GO_FTD0096.md`** (`[THEOREM]` / `[CLOSED NEGATIVE for FTD-0096]`). Extends the no-go to mass specifically, closing the FTD-0096 hypothesis that the dynamical threshold parameters (`K_GENESIS`, `K_EVAP`, `K_drain`, `K_LANGEVIN_T`) might smuggle in a mass-dimensional generator. They do not — they are dimensionless reals in the abstract update rules. Consequence: the L₂ identity `2m_e/α = 16G*²` (FTD-0094) is terminally `[PARAMETRIC]` under three independent closures (methodological FTD-0097, structural FTD-0093, dimensional FTD-0096).

These theorems convert two open problems into structural features without overclaiming: they say precisely what FTD *cannot* do and why.

---

## 8 · Engine-as-instrument campaigns (the measurement record)

In parallel with the bridge program, the engine was run as an instrument — generic initial conditions, pre-registered outcome grids, no Standard-Model comparison. The campaigns and their recorded results:

- **Emergent particle spectrum, G / G1 / G2** (FTD-0102, FTD-0107). The L=32 campaign found a three-regime phase structure (stable vacuum / deterministic bound-state regime / runaway crystallization). The L=64 (G1) and L=128 (G2) reruns confirmed that the *deterministic cluster counts* are L-invariant across the 64× volume range `{32, 64, 128}` — point injection gives exactly 1 cluster (~25 voxels), collision gives exactly 2. All `[PARTIAL]`; the L-invariant cluster count is the most novel positive structural finding of the engine-as-instrument portfolio.
- **Operator-mixing matrix + L-scan** (FTD-0098/0099, FTD-0140/0141/0142). The native `M_ab(b=2)` matrix was measured; the L-scan met 3 of 5 pre-registered criteria. The RG-semigroup property `M(b=4) = M(b=2)²` honestly FAILS — a robust feature across all L. `[PARTIAL]` for the matrix; `[CLOSED NEGATIVE]` for the RG-semigroup hypothesis.
- **Topological observables** (FTD-0104). A four-sub-experiment atlas (Wilson loop, flux tube, monopole, vacuum instanton) at L=32; each landed on a unique pre-registered outcome cell. Engine-native phenomenology, no SM quantization recovered. `[PARTIAL]`.
- **Lorentz anisotropy** (EFT-Recovery Pillar 3). The closed-form lattice dispersion gives anisotropy exponent `p = 4.0008` (`R² = 1.000000`); the rotation-breaking operator is dimension-6 in D=3, hence strongly irrelevant under Wilsonian RG. It passes the pre-registered pillar by 6–8 orders of magnitude — the strongest clean positive result among the engine campaigns. `[MEASURED]` / `[DERIVED, closed-form]`.
- **Ward identity** (FTD-0090). A reconciliation audit: the "1% Ward residual" is a known SOR-projection feature (18-point/6-point stencil mismatch), not a physics gap; the matched-stencil CG projector reaches `≤1e-8`.
- **Blocking-diagonal identities** (`THEOREM_BLOCKING_DIAGONAL_IDENTITIES.md`). `M_JJ = b⁴` and `M_J4 = b⁸` exactly for constant flux; `[THEOREM]`, confirmed L-independent and invariant under action perturbation (Gate-D T-perturbation campaign). These are convention-level identities — infrastructure-grade, certifying the measurement pipeline; the doc is explicit that they "do not contribute to physics RG flow."

The slice's value is boundary-mapping: structural positives (L-invariant cluster counts, sector decoupling, irrelevant `k⁴` anisotropy) and disciplined negatives (the RG semigroup does not close). No engine campaign promoted a physics claim to `[THEOREM]`.

---

## 9 · Wilson-Dirac and the graviton census (the latest negatives)

Two of the program's most recent campaigns continue the pattern — FTD's algebraic spine does not flow into the engine's dynamical/matter-sector observables.

- **Wilson-Dirac matter** (`SPEC_WILSON_DIRAC_FTD.md`, FTD-0126). Native FTD fermion emergence is closed-negative (FTD-0073/0076), so a standard Wilson-Dirac matter sector was *inserted* as an explicit Branch-B step, with the coupling fixed to `g_FTD = √(1/x₊)`, to measure the electron anomalous moment `a_e` against Schwinger's `α/(2π)`. The Phase-II infrastructure stages closed at machine precision, but the g−2 measurement returned **Outcome C**: measured `a_e_lattice ≈ 0.80` versus Schwinger `≈ 0.00116`, relative error **684**. The doc is scrupulous — it does not falsify the algebraic spine, but it nulls the conjecture that Wilson-Dirac with `α = 1/x₊` reproduces Schwinger, diagnosed as a Wilson-`r` artifact plus the absence of loop physics.
- **Frontier-4 graviton census** (`REPORT_GRAVITON_SUBSTRATE_MODE.md`, FTD-0189). A pre-registered engine campaign (v2 registration, hash-locked) testing whether the FTD substrate carries an emergent propagating massless spin-2 (graviton) mode. A 3-vector flux field cannot carry a fundamental spin-2 representation, so any graviton must be emergent. Test 4a-i (linear vacuum census) is COMPLETE — exactly 3 gapless branches (1 spin-0 + 2 spin-1), no fundamental spin-2 DOF. The transverse-traceless two-point measurement at L=32 and L=64 both indicate **Outcome B (no pole)**: spin-2 power 7–9 orders below the validated spin-1 control. The verdict is formally **PENDING the L=128 run**; if Outcome B holds, Frontier 4 is `[CLOSED NEGATIVE]` and FTD gravity is at most scalar+vector — the Einstein-chain graviton imported, not derived.

---

## 10 · What closed negative / what remains live

**Closed negative (provenance preserved; do not re-attempt):**

- The QED-α projection route in full — projected stiffness (R1), response-eigenvalue (R3), source-current normalization (R2), Structure-2 Ward-valid scalar completion, BCC tadpole regulator.
- `g_sJ = √α_QED` — the native source-flux coupling is `[DEFINITION]` `g_sJ = 1`.
- g_c Mechanisms A (topological quantisation) and C (gap-equation fixed point, FTD-0093).
- The `s·div J` gauge route — `s·div J` is not gauge-invariant under `J → J + grad χ`, so microscopic `J` is not a U(1) potential (`DERIV_STATE_FLUX_TO_EFT_DICTIONARY.md`).
- The `a → 0` continuum limit of the bare scale flow.
- The lemniscatic-replacement hypothesis for the black-hole horizon (FTD-0105, PASS-NONE — the lattice horizon is sphere-symmetric).
- Phase-I native-coupling engine cross-check (FTD-0125, falsified on the engine at all 8 fixtures — gauss-projection overwrites the longitudinal `G_C` contribution every tick).

**Live (genuinely open):**

- The nonlinear blocked effective action `S_eff` — the cluster's central live deliverable (`OPEN_FTD_NATIVE_ACTION_OR_MEASURE.md`; live entry point `STATUS_NONLINEAR_REGIME_2026-04-30.md`).
- g_c Mechanism B (lattice-to-continuum matching) — open, but not a classical computation.
- The μ-from-ℓ_P missing arrow as a *derivation path* — FTD-0096 is closed theorem-negative, but `OPEN_MU_FROM_LP_MISSING_ARROW.md` remains the live scoping doc for what an independent mass-quantum characterization would need.
- The Frontier-4 L=128 verdict (`REPORT_GRAVITON_SUBSTRATE_MODE.md`).
- Pre-registered, not-yet-run scans: `PREREG_ALPHA_ARITHMETIC_GENERATIVITY_v1` (FTD-0185, blocked behind MC-T4.3), `PREREG_FQCR_QUOTIENT_UNIQUENESS_v1` (FTD-0143, scan-runner not yet written), `PREREG_STRUCTURAL_DYNAMICAL_DISCRIMINATOR_v1` (FTD-0186; per CLAUDE.md the boundary classification is `[OPEN]` and needs a v2 re-pre-registration).

---

## 11 · Honest headline

The EFT Recovery Program ran its pre-registered five-pillar campaign and its post-pivot native program to a disciplined conclusion. What it produced, stated without inflation:

- A body of **disciplined closed-negatives** — roughly a dozen pre-registered or audited routes to physical `α` and to `g_c`, each closed for a structurally consistent reason, each preserved as provenance.
- **Two genuine no-go theorems** — `THEOREM_A_PHYS_NO_GO` and `THEOREM_MU_NO_GO_FTD0096` — establishing that no SI-dimensioned quantity is derivable from Axiom Zero, so FTD's dimensional interface is exactly two calibrations wide.
- A **Phase-G theorem** showing the engine's emergent `V(r)` is pure lattice geometry with no fine-structure content, which demolished the earlier "3.6× α plateau" overclaim.
- A modest constructive core — the bare linear sector is a free Gaussian theory with canonical normalization `(1,1,1,1)`, scale-invariant — which is canonical-normalisation bookkeeping, not predictive physics.

The program **did not derive α dynamically.** It did not produce a nonlinear effective action. The central conjecture `x₊ = 1/α` stays `[STRONGLY MOTIVATED CONJECTURE]` — its evidential basis is the master quadratic's dual match, CM-curve uniqueness, and the FTD-0189 look-elsewhere null, but it has no derivation chain, and the physics mechanism (MC-T4.3) remains `[OPEN]`.

The honest deliverable is the boundary itself. The program mapped, with pre-registration discipline and theorem-grade rigour, exactly how far the discrete ontology reaches into effective field theory and exactly where it stops. Per CLAUDE.md's number-one goal, rigorously establishing what the ontology *cannot* determine is as much a project result as a derivation — and that map, drawn honestly in both directions, is what the EFT Recovery Program produced.

---

## 12 · Pointer index

Every archived scaffolding doc, its archive subdirectory, and the one-line result it recorded. Subdirectory mapping per the consolidation ledger bucket B.

| Archived doc | Subdir | Result it recorded |
|---|---|---|
| `DERIV_FTD_NATIVE_SOURCE_FLUX_COUPLING_CLOSURE.md` | `closed_negative/` | `g_sJ = √α_QED` not derivable; native coupling is `[DEFINITION]` `g_sJ = 1`. |
| `DERIV_PROJECTED_EFT_MATTER_COUPLING.md` | `closed_negative/` | Projected-U(1) matter branch; charge normalization `e² = α` closed-negative under the projected action. |
| `AUDIT_STRUCTURE2_WARD_VALIDATION.md` | `closed_negative/` | Ward-valid Structure-2 scalar loop reproduces no Structure-1 ppb α correction. |
| `ANALYSIS_LEMNISCATIC_REPLACEMENT.md` | `closed_negative/` | Pre-reg PASS-NONE; lemniscatic-replacement closed negative for the horizon-area observable (FTD-0105). |
| `PREREG_PHASE_I_NATIVE_COUPLING.md` | `closed_negative/` | FTD-0125 pre-registered and falsified on the engine (Outcome C) — V(r) carries no `G_C²` prefactor. |
| `PROTOCOL_BCC_SUBLATTICE_SPECTRUM.md` | `closed_negative/` | Mechanism-C BCC-spectrum falsifier protocol; Mechanism C closed negative (FTD-0093). |
| `PROTOCOL_EMERGENT_PARTICLE_SPECTRUM.md` | `campaign_complete/` | FTD-0102 emergent-spectrum protocol; campaign ran. |
| `ANALYSIS_EMERGENT_SPECTRUM.md` | `campaign_complete/` | FTD-0102 result — L=32 three-regime phase structure. |
| `PROTOCOL_EMERGENT_SPECTRUM_G1.md` | `campaign_complete/` | FTD-0107 L=64 protocol; campaign completed 2026-04-27. |
| `ANALYSIS_EMERGENT_SPECTRUM_G1.md` | `campaign_complete/` | FTD-0107 L=64 result — deterministic cluster counts L-invariant. |
| `PROTOCOL_EMERGENT_SPECTRUM_G2.md` | `campaign_complete/` | L=128 protocol; G2 campaign completed 2026-04-28. |
| `ANALYSIS_EMERGENT_SPECTRUM_G2.md` | `campaign_complete/` | L=128 result — L-invariance locked across `{32,64,128}`. |
| `PROTOCOL_TOPOLOGICAL_OBSERVABLES.md` | `campaign_complete/` | FTD-0104 topology-atlas protocol; campaign ran. |
| `ANALYSIS_TOPOLOGICAL_OBSERVABLES.md` | `campaign_complete/` | FTD-0104 result `[PARTIAL]` — four observables on unique outcome cells. |
| `PROTOCOL_LEMNISCATIC_REPLACEMENT.md` | `campaign_complete/` | FTD-0105 horizon-area protocol; campaign ran (closed PASS-NONE). |
| `AUDIT_LEMNISCATIC_SPHERE_REPLACEMENT.md` | `campaign_complete/` | FTD-0105 pre-investigation catalog; investigation completed negative. |
| `AUDIT_FTD0105_MATH_CHECK.md` | `campaign_complete/` | Arithmetic audit of FTD-0105; verdict HOLDS with two corrigenda. |
| `PROTOCOL_GSTAR_ASYMMETRY_SCAN.md` | `campaign_complete/` | FTD-0106 G*/π asymmetry pre-registration; theory-catalog phase complete (0/7 match). |
| `AUDIT_GSTAR_ASYMMETRY_SCAN.md` | `campaign_complete/` | FTD-0106 catalog + verdict NEGATIVE for direct π→G* substitution. |
| `PROTOCOL_OPERATOR_MIXING_MATRIX.md` | `campaign_complete/` | FTD-0098/0099 operator-mixing protocol; measurement landed `[PARTIAL]`. |
| `PREREG_OPERATOR_MIXING_L_SCAN_v1.md` | `campaign_complete/` | FTD-0140/0141/0142 L-scan pre-registration; the L-scan ran. |
| `ANALYSIS_OPERATOR_MIXING_L_SCAN.md` | `campaign_complete/` | R3a L-scan — 3/5 criteria met; RG-semigroup closed-negative. |
| `AUDIT_CONTINUUM_LIMIT.md` | `campaign_complete/` | Continuum-limit convergence audit of `M_ab(b=2)`; `[PARTIAL]`. |
| `AUDIT_OPERATOR_SPECTRUM.md` | `campaign_complete/` | EFT Phase-3 operator-spectrum audit; `[PARTIAL]`. |
| `ANALYSIS_GATE_C_VS_L.md` | `campaign_complete/` | Cross-L Gate-C analysis; theorem-2 19σ deviation at L=128 recorded. |
| `ANALYSIS_OFFDIAGONAL_ASYMMETRY.md` | `campaign_complete/` | Off-diagonal J⁴ structural observation — three near-isolated sub-blocks. |
| `PROTOCOL_S_EFF_NONLINEAR_CAMPAIGN.md` | `campaign_complete/` | s_eff v1 campaign design; v1 ran and produced a partial measurement. |
| `PROTOCOL_S_EFF_NONLINEAR_v2_DESIGN.md` | `campaign_complete/` | s_eff v2 design draft; superseded by the live `STATUS_NONLINEAR_REGIME` handoff. |
| `MEASUREMENT_S_EFF_NONLINEAR_v1_partial.md` | `campaign_complete/` | s_eff v1 measurement `[PARTIAL]` — Gates B & C pass, Gate A subthreshold. |
| `AUDIT_S_EFF_SMOKE_VALIDATION.md` | `campaign_complete/` | End-to-end smoke validation of the s_eff campaign; infrastructure check. |
| `AUDIT_GAUSSIANITY_v1_LARGE.md` | `campaign_complete/` | Gaussianity audit — non-Gaussian operator distributions (skew 1.1–26). |
| `MEASUREMENT_GATE_D_T_PERTURBATION.md` | `campaign_complete/` | Gate-D T-perturbation — theorem-grade diagonals invariant under perturbation. |
| `GAUSSIAN_EXPANSION_DATA_INVENTORY.md` | `campaign_complete/` | Inventory of 21 GPU ctest binaries for the Gaussian sector. |
| `PROTOCOL_BETA_MEASUREMENT.md` | `campaign_complete/` | β-function protocol; design + smoke test landed, β≈0 expectation recorded. |
| `AUDIT_ALPHA_SCALING_L256.md` | `campaign_complete/` | L=256 α_eff scaling; certifies the geometric baseline only. |
| `AUDIT_WARD_IDENTITY.md` | `campaign_complete/` | Ward-identity reconciliation (FTD-0090); 1% residual is an SOR feature. |
| `AUDIT_LORENTZ_ANISOTROPY.md` | `campaign_complete/` | Lorentz anisotropy `p = 4.0008`; dimension-6 operator, strongly irrelevant. |
| `AUDIT_GPU_PLAN_PRIORITIES_1_3_5_6.md` | `campaign_complete/` | External GPU-plan execution audit; one-loop PT confirmed within 0.2σ. |
| `AUDIT_EFT_BCC_ORTHOGONALITY.md` | `campaign_complete/` | Link-8 BCC-orthogonality guardrail; verdict — no caveat needed on existing claims. |
| `ANALYSIS_MASTER_QUADRATIC_EFT_OPEN_ITEMS.md` | `campaign_complete/` | Master-quadratic ↔ EFT-checklist correlation pass; superseded by `STATUS_EFT_CHECKLIST`. |
| `PREREG_HEEGNER_TOWER_RIGIDITY.md` | `campaign_complete/` | Heegner-tower rigidity pre-registration; the scan ran (result in the KEEP audit). |
| `PREREG_LEMNISCATE_ALPHA_RIGIDITY.md` | `campaign_complete/` | Lemniscate-α rigidity pre-registration; the scan ran (result in the KEEP audit). |
| `PREREG_PHASE_II_WILSON_DIRAC_G2.md` | `campaign_complete/` | FTD-0126 Phase-II pre-registration; campaign ran to completion (Outcome C). |
| `REPORT_CHI_MINUS_4_ENGINE_2026-05-19.md` | `campaign_complete/` | chi_{−4} engine campaign `[OBSERVATION]`; sub-significant at n=10. |
| `SPEC_OPERATOR_BASIS.md` | `campaign_complete/` | The 6-operator Phase-3 basis; superseded by `SPEC_OPERATOR_BASIS_COMPLETE.md`. |
| `EXPLR_SELF_DUAL_HALF_SHELL.md` | `campaign_complete/` | Self-dual half-shell sketch; central identification `[SELECTION]`/`[CONJECTURE]`, not picked up. |
| `DECISION_FIELD_BASIS.md` | `campaign_complete/` | Project decision — collocated `(s, J)` as the canonical EFT field basis. |
| `DECISION_GAUSS_REPRESENTATION.md` | `campaign_complete/` | Project decision — GPU cuFFT collocated single-substrate as the canonical Gauss representation. |

> Note: the `DERIV_FTD_NATIVE_*` family was consolidated by merge (into `..._RESPONSE_AND_BLOCKING`, `..._NONLINEAR_FLOW`, `..._HISTORY_ACTION`), not archived — every tag carried verbatim. The sole exception, `DERIV_FTD_NATIVE_SOURCE_FLUX_COUPLING_CLOSURE.md`, is archived (first row above) because it is a closure, not a flow-derivation.
