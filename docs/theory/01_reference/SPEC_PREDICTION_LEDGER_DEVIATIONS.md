# SPEC — Prediction Ledger: Structural Deviations from the QM/SR Formalism

**Tag:** `[SYNTHESIS]` — a **registry**. Each row carries its source's existing LEDGER tag; **no row is promoted by inclusion here**. The ledger adds discriminating protocols, scope caveats, and kill conditions — not new claims.
**Date:** 2026-06-09
**LEDGER:** FTD-0258.
**Parent:** [`SPEC_FTD_FRAMEWORK_V1.md`](SPEC_FTD_FRAMEWORK_V1.md) §6.1 (the constitution; FC-1/FC-2 are declared there).
**Division of labor:** [`SPEC_NOVEL_PREDICTIONS.md`](SPEC_NOVEL_PREDICTIONS.md) catalogs *numeric values* against Standard-Model parameters (mostly post-dictions, flagged as such there). **This ledger catalogs *structural deviations from the QM/SR formalism itself*** — places where FTD's native posture (commutative substrate, FC-1; sector-scoped metric, FC-2) predicts experiment will disagree with the textbook *structure*, not merely with a parameter value.

---

## 0 · The standard caveat (applies to every row)

> Engine-native `[MEASURED]` results are statements about the FTD substrate at the stated lattice size, stencil, and protocol. Their **physical** readings are conditional on: **(i)** the calibration register (`a_phys ≡ ℓ_P`, `M_REST = m_e`, `t_phys = √3·ℓ_P/c` — constitution §3.3; gauge choices per FTD-0137); **(ii)** the manifestation  detection mapping at its LEDGER tag (`[CONJECTURE]`-grade where used); **(iii)** FC-1/FC-2 as **declared commitments** (constitution §2.4/§2.6/§5.2 — declarations, not derivations). Dimensionless deviation *structure* (which law, which exponent) is calibration-invariant; absolute physical scales are not.

**Physical-scale honesty for PL-4/PL-5:** under `a_phys ≡ ℓ_P` the UV corrections are Planck-suppressed; laboratory reach is bounded by existing Lorentz-violation searches. The engine-native versions are testable now; the physical versions are long-horizon. Both readings are stated per row.

---

## PL-1 · Detection statistics: Rice upcrossing, not Born

| Field | Content |
|---|---|
| **FTD result** | Threshold-manifestation event frequency follows **Rice's Gaussian-upcrossing law** `log freq = log B − k·(K_B − μ)²/σ²`: fit **R² = 0.9923** (k = 0.0971). The Born form `freq ∝ \|J\|^n` fits at **R² = 0.7137** (n = 2.1858, 95 % CI [1.26, 3.99]) — Rice wins by 0.2786, far above the pre-registered 0.05 margin. 5.3 M events, 7,973 in-mask voxels, 100 trials × 80 ticks. `[CLOSED NEGATIVE for Born]` + `[NUMERICAL FACT]` — FTD-0200 ([`EXPLR_THRESHOLD_CROSSING_BORN_NEGATIVE.md`](../06_reference_frames_and_measurement/EXPLR_THRESHOLD_CROSSING_BORN_NEGATIVE.md)); companion equilibrium negative FTD-0199. |
| **QM expectation** | Born rule: detection probability ∝ `\|ψ\|²` exactly. |
| **Discriminating protocol** | Near-threshold detector statistics vs incident intensity: bin event rates by mean local intensity μ², fit Rice vs power-law on the same binned data (the FTD-0200 protocol, pre-registered: tag `preregister-threshold-crossing-born-v1`, runner SHA `2781b3ce…`). The signature is **saturation of the rate envelope** at high intensity (the Rice tail) where Born predicts continued power-law growth. |
| **Scope + caveats** | Measured in the **6-neighbour** substrate at L = 24, ReLU manifestation at `K_B = 0.5`, damping 0.001 — the stated construction only. Generalization to the canonical 26-neighbour engine with all toggles is **`[CONJECTURE — testable in v2]`** (FTD-0200 §3: nonlinear back-reaction could drive the field non-Gaussian). The binding-without-sharpness account (FTD-0227 `[PARTIAL]`) is **post-hoc**. Physical reading additionally rides the manifestation = detection identification. |
| **Falsification (of the FTD side)** | A P1–P5 construction whose threshold statistics fit Born (power n = 2) better than Rice under the same pre-registered protocol — in particular, the v2 canonical-engine run returning Born. That outcome would *also* kill the PL-1 deviation claim and revive the Born-emergence program (and strengthen the case against FC-1, constitution §6.2). |

## PL-2 · Bell/CHSH: the substrate bound — **highest-risk row, read the honesty block**

| Field | Content |
|---|---|
| **FTD result** | Any correlation experiment fully implementable inside the commutative substrate (no measurement-map M) is bounded **S ≤ 2** — structural: the substrate satisfies realism + locality + statistical independence, so Bell's theorem applies (`[THEOREM]`-grade via FTD-0243 + Bell). Engine CHSH measurements: **S ≈ 1.95–2.00** across all tested configurations (vector flux, ternary states, wave propagation, sLoop coupling). The ternary state space produces an apparent S ≈ 3.6 at ~49 % detection efficiency — the **detection loophole**, a known artifact, not a violation. ([`AUDIT_BELL_ANALYSIS.md`](../07_assessment/AUDIT_BELL_ANALYSIS.md); [`THEOREM_COMMUTATIVITY_INDEPENDENCE.md`](../10_eft_program/derivations/THEOREM_COMMUTATIVITY_INDEPENDENCE.md) §4.) |
| **QM expectation** | S = 2√2 ≈ 2.828 at optimal settings (Tsirelson). |
| **Discriminating protocol** | Engine-native: CHSH on raw substrate flux correlations, no complexification step — available now, and any S > 2 (outside the detection loophole) kills FC-1 (constitution §6.2). Physical: identify a regime whose readout chain is demonstrably M-free; FTD stakes S ≤ 2 there. |
| ** Honesty block (mandatory)** | **Loophole-free laboratory Bell experiments already measure S > 2.** This row is therefore *not* a prediction that lab Bell tests will read 2.000. It is (a) a **scoped substrate bound**, plus (b) an **accepted open burden**: FTD must produce the observed S > 2 at its observer layer — the existing aggregate-correlation mechanism (CLAIM.8, three-level hierarchy) is `[SELECTION]`-grade and the complexification it uses is *exactly* an instance of the M that FC-1 declines, so FTD's account of laboratory Bell violations is **`[OPEN]`**. This is the deviation spine's highest-risk entry and the most likely place the framework breaks. |
| **Falsification (of the FTD side)** | Substrate-native S > 2 beyond numerical error (kills FC-1 directly); or a proof that *no* observer-layer mechanism consistent with FC-1 can yield the laboratory statistics (kills the program's account of established experiments — the burden becoming a verdict). |

## PL-3 · Quadrature compatibility: the commutative clock

| Field | Content |
|---|---|
| **FTD result** | All quadratures of a mode are **co-measurable**: the symplectic pair has `{q,p} ≠ 0` (Poisson — the winding is real) yet observable commutator `[q,p] = 0` (`[THEOREM]`, FTD-0243 §3). Measured (FTD-0251, `[MEASURED]`, 10/10): quadrature phase winds at `ω(k) = 2·C_WAVE·\|sin(k/2)\|` (multi-tick winding matches the single-tick eigenvalue to 0.10–0.98 %, modes n = 1, 2, 4 at L = 32); transverse orientation frozen at machine zero (leakage ≈ 1.6×10⁻¹⁶); L/R dual channels an exact mirror (`\|J_L − J_R\| = 0`). The substrate supplies the homodyne *angle*; it provably does not supply the *incompatibility*. ([`EXPLR_SUBSTRATE_NATIVE_ANGLE.md`](../06_reference_frames_and_measurement/EXPLR_SUBSTRATE_NATIVE_ANGLE.md).) |
| **QM expectation** | Conjugate quadratures are incompatible (`[q̂,p̂] = iħ`): joint sharp values do not exist; measuring one disturbs the other. |
| **Discriminating protocol** | Simultaneous sharp readout of both quadratures of one mode without back-action. Engine-native: trivially available (every tick carries joint definite `(q,p)`). Physical: any system whose readout is demonstrably M-free should show no complementarity floor — FC-1-conditional. |
| **Scope + caveats** | Engine-native facts are `[MEASURED]`; the identification of the quadrature angle with "the measurement angle" is `[SELECTION]` (FTD-0251 §4). The physical assertion (real detectors could in principle co-measure) is **FC-1-conditional** and carries the same observer-layer burden as PL-2 — established quantum-optics complementarity must be reproduced at the observer layer or the commitment fails. |
| **Falsification (of the FTD side)** | A substrate-native derivation of quadrature incompatibility from P1–P5 (kills FTD-0243's premise and FC-1); or — on the physics side — demonstration that observer-layer epistemics consistent with FC-1 *cannot* reproduce measured homodyne complementarity. |

## PL-4 · Moving-clock rate: γ is IR-emergent, with a calculable UV bend

| Field | Content |
|---|---|
| **FTD result** | A moving lattice wave-clock's dilation departs from exact `γ = 1/√(1−v²)` by a **calculable lattice UV correction that vanishes as `R = \|D − √(1−v²)\| ∝ L⁻¹·⁹⁸ ≈ L⁻²` (∝ k²)** on the ⟨100⟩ axis at v ≲ 0.85 — residuals shrink 34–94× from L = 33 to L = 193 (e.g. v ≈ 0.29: 0.0024 → 0.00003; v ≈ 0.67: 0.0218 → 0.00065); 9/9 matched groups monotone; reviewer-reproduced. At fixed L the clock **over-dilates** — D bends *below* γ as v grows (v = 0.639 at L = 129: D = 0.71701 vs γ-value 0.76550); low-v fidelity < 0.06 %. **Scoped `[MEASURED]`** (⟨100⟩, moderate v); ultra-relativistic diagonals (v > 0.9, ⟨110⟩/⟨111⟩) unconverged at L ≤ 193 — `[OBSERVATION/OPEN]`. FTD-0252 ([`ANALYSIS_DYNAMICAL_TIME_DILATION.md`](../03_derivations/foundational_mechanics/ANALYSIS_DYNAMICAL_TIME_DILATION.md), v2 pre-registered, adversarially reviewed). |
| **SR expectation** | Exact γ at every scale, every axis, every velocity. |
| **Discriminating protocol** | **Lead with the law, not with "γ emerges":** the v1 reviewer's load-bearing catch is that `√(1−v²)` is an *algebraic identity* of the co-moving-frequency construction on any sum-of-squares dispersion — the empirical content is **(i) the `L⁻²` convergence law and (ii) the UV bend below γ**. Engine-native: the FTD-0252 v2 protocol (fix `n⊥`, grow L). Physical: dispersion-level departures from exact relativistic kinematics at high k — Planck-suppressed under the calibration (§0 honesty note); current bounds from Lorentz-violation searches are the relevant comparison class. |
| **Scope + caveats** | The clock hypothesis stays `[AXIOM]` at coordinate level (FTD-0208 `[CLOSED NEGATIVE]` for substrate derivation) — annotated with measured IR-emergent support, nothing stronger. FTD-0208's linear single-event budget is a *different observable*; not in contest. |
| **Falsification (of the FTD side)** | The `L⁻²` law breaking on ⟨100⟩ at larger L (the emergence stalling), or exact γ holding at finite L where the lattice requires a UV correction (no bend where one is predicted) — either kills FC-2's metric half (constitution §6.2). The open diagonal regime resolving to a *fundamental* (not finite-L) non-convergence would force a scoped retraction of the IR-emergence claim. |

## PL-5 · Isotropy: native UV anisotropy, dying as k⁴

| Field | Content |
|---|---|
| **FTD result** | The substrate is anisotropic at UV and isotropizes in the IR with a **measured power law**: rotation-breaking residual exponent **p = 4.0008 ± 0.0006 (R² = 1.000000)** — `δ ∝ k⁴` in the phase speed, the rotation-breaking operator entering at dimension (D+4) = 7, **strongly irrelevant** under Wilsonian counting. `[MEASURED]` ([`AUDIT_LORENTZ_ANISOTROPY.md`](../10_eft_program/archive/campaign_complete/AUDIT_LORENTZ_ANISOTROPY.md); fit exact to four decimals across a factor-8 range in L, factor-16 in k). |
| **SR expectation** | Exact rotational invariance at every scale. |
| **Discriminating protocol** | Engine-native: the closed-form anisotropy measurement (already canonical). Physical: direction-dependent dispersion at high energy — `k⁴`-suppressed, hence Planck-suppressed under the calibration; cubic-anisotropy signatures (an O_h fingerprint, cf. the mild ⟨111⟩ excess in FTD-0252 T3) distinguish a lattice from rotationally-invariant Lorentz violation. Cross-reference: [`SPEC_NOVEL_PREDICTIONS.md`](SPEC_NOVEL_PREDICTIONS.md) lattice-specific section for the numeric-value side. |
| **Scope + caveats** | The measured exponent is the substrate's; physical accessibility per §0 honesty note. |
| **Falsification (of the FTD side)** | The k⁴ decay reversing or plateauing at larger L (isotropization stalling — kills FC-2's "emergent" reading); physically, *confirmed* isotropic-exact dispersion at scales where the calibrated lattice predicts measurable cubic anisotropy. |

## PL-6 · Structural nulls `[THEOREM]`

| Field | Content |
|---|---|
| **FTD result** | The ontology *forbids* (verbatim from [`CATALOG_PARAMETRIC_INSERTIONS.md`](../07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md) §16, all `[THEOREM]`): **τ_proton = ∞** (charge conservation exact on the lattice — Gauss constraint; `proof_complete_sm.py:467`); **N_monopole = 0** (`div(B) ≡ div(curl J) = 0` identity; `:495`); **N_SUSY = 0** (the ternary state space carries no fermionic grading; `:500`); **extra dimensions = 0** (`\|Aut(E)\|² = 2^D·(D−1)!` forces D = 3; `:503`). |
| **Competitor expectation** | GUTs: proton decay; many unification schemes: monopoles; string theory: SUSY partners + extra dimensions (characteristic imports — constitution §4.2 contrast 3). |
| **Discriminating protocol** | The world's existing experimental program *is* the protocol: Super-K/Hyper-K proton-decay limits, monopole searches, collider SUSY searches, short-range-gravity and collider extra-dimension searches. Every continued null is evidence for the FTD ontology against frameworks whose structure requires the positives. |
| **Scope + caveats** | Null-predictions are absence claims — they accumulate support but a single confirmed positive kills the corresponding theorem's physical reading outright. |
| **Falsification (of the FTD side)** | An observed proton decay, magnetic monopole, SUSY partner, or extra dimension — each falsifies the corresponding `[THEOREM]`'s physical identification immediately and unconditionally. |

---

## Maintenance

Rows are added/updated only with: source doc + LEDGER tag + engine artifact + (for new measurements) pre-registration reference. Each row inherits its source's tag; **inclusion here never promotes**. When a kill condition fires, the row is retagged in the LEDGER first (per the constitution's §7 governance), then annotated here.
