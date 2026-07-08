# SPEC — FTD Complete Framework: the unified honest map (FTD-0311)

**Tag:** `[SYNTHESIS]` — integrates the constitution, algebraic spine, boundary docs, prediction spine, and accounting; introduces **no new mathematics** and **promotes nothing**. Every claim carries its canonical tag.
**Supersedes:** `SPEC_FTD_COMPLETE_CHAIN.md` (the "i→α proof chain"; archived). This doc keeps that "every link tagged" spirit and adds the framework commitments, the boundary map, the rigidity-coverage map, the prediction spine, and the external-validation status.
**Precedence:** **LEDGER > constitution (`SPEC_FTD_FRAMEWORK_V1`) > this doc > other prose.** If a number or tag here disagrees with the LEDGER, the LEDGER wins.

> **What this document is.** The one page a peer reads to know — exactly and honestly — what FTD *derives*, what it *cannot* derive (and why, rigorously), what it *predicts*, what is *externally validated*, and — per claim family — whether a numerical match is **scan-rigid, scan-NULL, or merely tagged**. It is a *map*, not a textbook: it restates no proofs and no narrative (those live in `SPEC_ALGEBRAIC_SPINE` and `MONOGRAPH_FTD_CONSTRUCTION`); it cites.
>
> **What "complete" means here.** Not "derives the Standard Model" — the mapped boundaries (§3) show it cannot, by any examined route (α route-invariant; QM's M and reversibility declined as `[AXIOM]`-class commitments; atomic dynamics structurally wrong-dispersion). Complete = the derive-vs-cannot map is **exhaustive and honestly tagged**. FTD is a **philosophy-of-mathematics project with a rigorous algebraic core and suggestive — not derived — physics connections**; its north star is rigorous algebra + honestly-mapped boundaries.

---

## §1 · The three registers (→ constitution `SPEC_FTD_FRAMEWORK_V1`)

| Register | Content | Status |
|---|---|---|
| **Postulates P1–P5** | discrete 3D lattice (undefined boundary) · discrete time · ternary states {−1,0,+1} · 26-neighbour local causality · determinism | `[AXIOM]` (frozen) |
| **Framework Commitments** | **FC-0** ℤ[i] reading of the order-4 symmetry · **FC-1** declines the measurement-map M (the commutative observable algebra is complete) · **FC-2** native arrow + emergent IR-only Lorentzian metric · **FC-3** scale-ratio-covariance (only internal ratios physical) · **FC-4 (FC-W)** adopts the external α-binding axiom W, pinned by FTD-0314 | `[AXIOM]`-class **declarations, not derivations** — each sits on a fork a theorem proved *open* (§3); FC-W is the framework's first *adopted* (vs declined) import |
| **Calibrations** | `a_phys ≡ ℓ_P` · `M_REST = m_e` · `t_phys = ℓ_P/(√3·c)` | `[IMPOSED]` (dimensionless predictions are calibration-independent; dimensional ones ride these) |

---

## §2 · What FTD derives (the load-bearing core) → `SPEC_ALGEBRAIC_SPINE`

**The algebraic spine is the part that stands on its own as mathematics, independent of any physics interpretation.** Seven theorem-grade + two honestly tiered below:

| # | Result | Tag |
|---|---|---|
| OT-1.1 | Master quadratic `x²−16G*²x+16G*³=0` + roots (pure algebra) | `[THEOREM]` |
| OT-1.2 | `G* = Γ(1/4)/Γ(3/4) = 2.95868…` (Euler reflection ratio) | `[THEOREM]` |
| OT-1.3 | Harmonic-invariant tower `1/y₊+1/y₋=1`, anomaly transcendence k≥4 | `[THEOREM]` |
| OT-1.4 | Phase-G geometric Coulomb = lattice Poisson Green's fn (R²=1.0000) | `[THEOREM]` |
| OT-1.5/1.6 | BCC complex structure ≅ ℤ[i]²; ℤ[i]^× → O_h^ab no-go | `[THEOREM]` |
| OT-2.1 | Watson identity `W₃ = G*²/(2π)` (cond. Watson 1939) | `[THEOREM]` |
| OT-2.x | Field-theoretic Q(G*), π-free (cond. Chudnovsky 1976) | `[THEOREM]` |
| OT-4.1 | coefficient `16 = |Aut(E)|²` — value true, structural *necessity* | `[CONJECTURE]` (T4) |
| OT-3.4 | Phase-J ultralocality | `[THEOREM at L=2]` (disconfirmed general L) |

Plus, theorem-grade and **independent of the master-quadratic identification**: **N_c = 3** (4 topological routes, `DERIV_NC_FROM_TOPOLOGY`) `[THEOREM]`; **D = 3** uniquely selected `[SELECTION]`; **C_SPEED = 1/√3** `[THEOREM]`; the **structural nulls** (no monopoles, no SUSY grading, no extra dimensions) `[THEOREM]`. Proof scripts under `scripts/proofs/`.

---

## §3 · The boundary map — what FTD provably cannot derive without extra input

Each row is a *result* (Number-One-Goal clause 2): a rigorously-mapped limit, not unfinished work.

| Boundary | Verdict | Canonical doc |
|---|---|---|
| **α (the EM coupling)** | **DYNAMICAL, not structural** — route-invariant (FTD-0242); K-BIND closed *theorem-negative* (FTD-0244); **FTD-0314 extends this to *all* finite-symmetry carriers** (the distinguishing surd `√(G*(4G*−1))` is transcendental over ℚ) ⇒ W is provably *external*. The constitution adopts it as **FC-W (FTD-0315)**, under which `x₊=1/α` is a `[CONDITIONAL THEOREM given W]` — still **not** `[DERIVED]`, unconditionally `[SMC]`. | `SPEC_ALPHA_DYNAMICAL_BOUNDARY` |
| **Cluster-mass law N(A)** | **`[BOUNDARY HARDENED]`** on 3 axes (exit-i FTD-0276, exit-ii convention FTD-0307, reduction FTD-0309): shape derived, calibration engine-emergent, no scalar reduction. | `SPEC_FTD0110_BRIDGE_BOUNDARY` |
| **Confinement (area-law)** | **`[OPEN STRUCTURAL OBSTRUCTION]`** — no deterministic-substrate analog; YM proof retracted (FTD-0042), per-voxel mass gap (FTD-0044) survives. | `SPEC_OPEN_MATH_BY_SECTOR` §4 |
| **Cosmology** | ΛCDM apparatus + FTD numerology; no cosmological observable derived. | `SPEC_COSMOLOGY_FRAMEWORK_BOUNDARY` |
| **Reversibility / Lorentzian metric** | not forced by P1–P5 (FTD-0253); FC-2 commits to native-arrow + emergent-IR metric. | constitution §2.6, §6.2 |
| **Measurement map M / non-commutativity** | logically independent of P1–P5 (FTD-0243 `[THEOREM]`); FC-1 declines it. | constitution §2.4 |
| **Atomic spectra / QM dynamics** | wrong dispersion (ω∝k vs Schrödinger ω∝k²); ~0% substrate-derivable (FTD-0270); the engine-native FFT is also instability-limited (FTD-0308). | `AUDIT_ATOMIC_DYNAMICS_STATUS` |
| **Born rule** | binding `[THEOREM]` + sharpness `[OPEN]` (the missing ℤ/3); detection is Rice, not Born (PL-1). | `AUDIT_SPEKKENS_KNOWLEDGE_BALANCE_PARTIAL` (FTD-0227) |

---

## §4 · The rigidity-scan coverage map — KEYSTONE (the F10 "tagging ≠ resolution" defense)

A LEDGER tag *labels* a claim; it does not answer *is the numerical match statistically surprising?* That requires a look-elsewhere / uniqueness scan. **Most of the framework's [PARAMETRIC] periphery has never been scanned** — and where it has, the news is sobering:

| Bucket | Claim family | Evidence |
|---|---|---|
| **(i) Scan-rigid** | **x₊ = 1/α** | FTD-0319 (the scan's dedicated row, formerly mis-cited "FTD-0189"): 0 non-G\* dual-matchers / 2.65M polynomials — the framework's *one* rigid identification (a `[NUMERICAL FACT]` under the registered gate; the "~4×10⁵:1 Bayes" is unsupported — runner yields ~19× scan-size; uniqueness is tolerance-conditioned). |
| **(ii) Scan-tested → NULL** | FTD integer/monomial catalog (general); cluster-mass SM identification | FTD-0097: catalog over-rich >5× background; FTD-0262: SM ratios p=2.05 (chance-level). |
| **(iii) Scan-tested → NOT rigid** | **sin²θ_W=3/13, α_s=7/59, m_e prefactor 16/3** (FTD-0310); **sin²θ₁₂=3/10, sin²θ₂₃=16/29, Δm²₃₁/Δm²₂₁=100/3** (FTD-0320) | **FTD-0310 + FTD-0320: none rigid.** α_s MDL-dominated by 2/17; sin²θ_W p≈0.05; prefactor p≈0.08; sin²θ₂₃ MDL-dominated by 6/11 (~10× better); Δm²-ratio by 33/1; sin²θ₁₂ chance-level (p=0.48). All six **demoted** to `[PARAMETRIC]`. |
| **(iv) Tagged, NOT scanned** | ~122 remaining `[PARAMETRIC]` — now essentially the integer-**combination** families (quark masses, ~90 hadron spectroscopy, decay rates, precision-QED imports, CKM Wolfenstein, Koide) | **No rigidity scan run** for the combination families (the simple-rational subset is now scanned, bucket iii). Their test is the FTD-0097-style combinatorial look-elsewhere scan (deferred v2; FTD-0097 already NULL at the monomial level). *Still the deepest open methodological question.* |

**The honest one-liner:** exactly **one** numerical identification (x₊=1/α) has survived an adversarial uniqueness scan; everything tested since has come back NULL or non-rigid; the large remainder is tagged-but-unscanned. The framework's physics matches should be read as *suggestive*, with the rigid algebraic spine (§2) carrying the weight.

---

## §5 · The falsifiable prediction spine → `SPEC_PREDICTION_LEDGER_DEVIATIONS` + `SPEC_PREDICTIONS_FORWARD_2026`

| Row | Prediction | Status |
|---|---|---|
| PL-1 | detection = Rice upcrossing, not Born (R² 0.9923 vs 0.7137) | `[NUMERICAL FACT]`, pre-reg confirmed |
| **PL-2** | substrate Bell **S ≤ 2** | `[THEOREM]` substrate; **observer-layer account of lab S>2 is `[SELECTION]`+`[OPEN]` — the framework's highest-risk burden** |
| PL-3 | co-measurable quadratures `[q,p]=0` | `[THEOREM]` substrate, FC-1-conditional physical |
| PL-4 | γ IR-emergent ∝ L⁻² (⟨100⟩) | `[MEASURED]`; **EP-1 blind-confirmed** L=257; diagonals `[OPEN]` |
| PL-5 | UV anisotropy ∝ k⁴ (p=4.0008) | `[MEASURED]`; **EP-3** verified to L=768 |
| PL-6 | structural nulls (monopole/SUSY/extra-dim) | `[THEOREM]` |
| FP-1..4 | first-order EWPT GW; Higgs λ=3/23 (FCC); LV nulls dim 5/6; structural nulls | registered forward, kill-conditions stated; external-horizon |

---

## §6 · The honest accounting → `CATALOG_PARAMETRIC_INSERTIONS` + `TRACKER_ONTIC_TRUTH`

- **~23 `[DERIVED]`/`[THEOREM]`** · **~129 `[PARAMETRIC]`** · **~10 `[IMPOSED]`/`[SELECTION]`** (~162 total claims). No drift across CATALOG / TRACKER_ONTIC_TRUTH / spine.
- **Truth tiers:** T1 rock-solid `[THEOREM]` (OT-1.x) · T2 conditional-on-classical-theorems · T3 numerical-fact · T4 the coefficient-16 `[CONJECTURE]` · T5 the central conjecture x₊=1/α `[SMC]`.
- **Rigidity overlay (the §4 news, quantified):** of the ~129 `[PARAMETRIC]`, **1** is scan-rigid (x₊, also the T5 conjecture; FTD-0319), **~6** are scan-tested-and-not-rigid (FTD-0310 + FTD-0320, all now `[PARAMETRIC]`), the bulk catalog is scan-NULL-at-the-monomial-level (FTD-0097), and **~122 are unscanned** — now essentially the integer-combination families (the simple-rational subset is scanned).

---

## §7 · External validation status

- **Defensible-now (pure mathematics, no physics input):** the **G\* paper** (`PAPER_GSTAR_INTRODUCTION.tex`, 36pp, math.NT, submission-ready) and **Papers A/B** (π-free generator, BCC complex structure). These stand on the §2 spine alone.
- **In progress:** the **EFT measurement pilot** (β-function via blocking; `SPEC_EFT_RECOVERY_PROGRAM`) — decides whether Paper C is a measurement paper or an honest null.
- **Not externally validated:** every physics *identification* (α, masses, gauge ratios) — these are internal, at their §4/§6 tags.

---

## §8 · Framework-level falsification (→ constitution §6.2)

- **FC-0** killed by: a non-ℤ[i] reading generating the same spine (uniqueness failure), or any spine theorem failing.
- **FC-1** killed by: a substrate-native `[A,B]≠0` from P1–P5; a P1–P5 derivation of Born; or substrate-native S>2 (M-free).
- **FC-2** killed by: exact γ at finite L off the IR limit; a P1–P5 derivation of global reversibility; or the L⁻²/k⁴ emergence laws breaking at larger L.

These are sharp and checkable. FC-2 has *measured support* (EP-1/EP-3 blind confirmations); FC-1 carries the highest risk (PL-2).

---

## §9 · What FTD has NOT delivered (the honesty row)

α as a derivation · the Born rule from P1–P5 · an FC-1-consistent (M-free) account of lab Bell S>2 · QFT scattering amplitudes / interference at the observer layer · atomic spectra · any cosmological observable · confinement · a derived mass spectrum (genesis or Gaussian sector). **And the F10 gap:** ~125 of the ~129 `[PARAMETRIC]` matches are *tagged but never rigidity-scanned* — the single most important methodological caveat. *Any framing that presents these as derivations is overclaim.*

---

## §10 · Forward (→ `SPEC_OPEN_MATH_BY_SECTOR`)

The remaining program is **mapping and validation, not α attacks**: mature the feasible predictions (PL-4 diagonals), run the EFT pilot, submit the defensible math, and — the highest-leverage integrity move — **extend the FTD-0310 rigidity audit across the unscanned `[PARAMETRIC]` periphery** (bucket iv), so the §4 map becomes complete rather than mostly-unscanned. Closing the central boundaries (α, the FTD-0110 reduction) each require a new 6th-postulate-class input, not the current program.

---

*Nothing in this document promotes any claim. FTD-0013 `[SMC]`, MC-T4.3 `[FOUNDATIONAL OBSTRUCTION]`, FC-0/1/2/3/4 `[AXIOM]`-class, the algebraic spine — all at their canonical tags. Golden gate untouched. LEDGER FTD-0311 `[SYNTHESIS]`; FC-W rows FTD-0314/0315.*
