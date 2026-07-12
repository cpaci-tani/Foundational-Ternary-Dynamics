# SPEC — FTD Complete Framework: the unified honest map (FTD-0311)

**Tag:** `[SYNTHESIS]` — integrates the constitution, algebraic spine, boundary docs, priced-import ledger, prediction spine, and accounting; introduces **no new mathematics** and **promotes nothing**. Every claim carries its canonical tag.
**Version:** **v2 (2026-07-12)** — v1→v2: claims register advanced through FTD-0382 + the finishing arc (FTD-0143 verdict, IMP-S4 pricing, FTD-0348/0350/0355 reconciliations); the priced-import ledger folded in as the boundary's quantitative face (§3); the July-2026 arc register added (§3.5); §8 replaced by the **consolidated falsifier table** (previously scattered across four document families); accounting refreshed to the FTD-0348 correction of record. FTD-0311 id retained.
**Supersedes:** `SPEC_FTD_COMPLETE_CHAIN.md` (the "i→α proof chain"; archived). This doc keeps that "every link tagged" spirit and adds the framework commitments, the boundary map (now priced), the rigidity-coverage map, the prediction spine, the falsifier table, and the external-validation status.
**Precedence:** **LEDGER > constitution (`SPEC_FTD_FRAMEWORK_V1`) > this doc > other prose.** If a number or tag here disagrees with the LEDGER, the LEDGER wins.

> **What this document is.** The one page a peer reads to know — exactly and honestly — what FTD *derives*, what it *cannot* derive (and why, rigorously, and at what priced cost), what it *predicts*, what would *kill* it (§8), what is *externally validated*, and — per claim family — whether a numerical match is **scan-rigid, scan-NULL, or merely tagged**. It is a *map*, not a textbook: it restates no proofs and no narrative (those live in `SPEC_ALGEBRAIC_SPINE` and `MONOGRAPH_FTD_CONSTRUCTION`); it cites.
>
> **What "complete" means here.** Not "derives the Standard Model" — the mapped boundaries (§3) show it cannot, by any examined route (α route-invariant; QM's M and reversibility declined as `[AXIOM]`-class commitments; atomic dynamics structurally wrong-dispersion). Complete = the derive-vs-cannot map is **exhaustive, honestly tagged, and priced** (every import counted in a common currency with a falsifier, §3/§8). FTD is a **philosophy-of-mathematics project with a rigorous algebraic core and suggestive — not derived — physics connections**; its north star is rigorous algebra + honestly-marked-and-priced boundaries.

---

## §1 · The three registers (→ constitution `SPEC_FTD_FRAMEWORK_V1`)

| Register | Content | Status |
|---|---|---|
| **Postulates P1–P5** | discrete 3D lattice (undefined boundary) · discrete time · ternary states {−1,0,+1} · 26-neighbour local causality · determinism | `[AXIOM]` (frozen) |
| **Framework Commitments** | **FC-0** ℤ[i] reading of the order-4 symmetry · **FC-1** declines the measurement-map M (the commutative observable algebra is complete) · **FC-2** native arrow + emergent IR-only Lorentzian metric · **FC-3** scale-ratio-covariance (only internal ratios physical) · **FC-4 (FC-W)** adopts the external α-binding axiom W, pinned by FTD-0314 | `[AXIOM]`-class **declarations, not derivations** — each sits on a fork a theorem proved *open* (§3); FC-W is the framework's first *adopted* (vs declined) import |
| **Calibrations** | `a_phys ≡ ℓ_P` · `M_REST = m_e` · `t_phys = ℓ_P/(√3·c)` — **default gauge is electron-primary since 2026-07-08** (FTD-0137 §4.5, `FOUND_ELECTRON_PRIMARY_GAUGE.md`): import `{ℏ, c, m_e}`, under which `a_phys = ℓ_P` and `t_phys` become derived-at-`[SMC]` and only the m_e anchor is a genuine import (IMP-K3); the legacy Planck-primary declaration remains a valid alternative gauge | `[IMPOSED]` (dimensionless predictions are calibration-independent; dimensional ones ride these) |

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
| OT-4.1 | coefficient `16 = |Aut(E)|²` — value true, structural *necessity* | `[CONJECTURE]` (T4; forcing `[SELECTION — declared]`, FTD-0355) |
| OT-3.4 | Phase-J ultralocality | `[THEOREM at all L ≥ 2]` per FTD-0350 — matched-stencil / Gauss-realizable scope, conditional on the exact-constraint AXIOM + stencil-consistency SELECTION; kept in the honestly-tiered bucket pending the owner's bucket-move decision |

Plus, theorem-grade and **independent of the master-quadratic identification**: **N_c = 3** (4 topological routes, `DERIV_NC_FROM_TOPOLOGY`) `[THEOREM]`; **D = 3** `[SELECTION — declared]` (FTD-0355 — the `2^D(D−1)! = 16` arithmetic uniqueness is `[THEOREM]`, the dimension-forcing is not forced, circularity named); **C_SPEED = 1/√3** `[THEOREM]`; the **structural nulls** (no monopoles, no SUSY grading, no extra dimensions) `[THEOREM]`. Proof scripts under `scripts/proofs/`.

---

## §3 · The boundary map — what FTD provably cannot derive without extra input

Each row is a *result* (Number-One-Goal clause 2): a rigorously-mapped limit, not unfinished work. **Each boundary is now also *priced*:** `SPEC_IMPORT_LEDGER.md` (FTD-0371, rev v1.1 2026-07-12) counts every import in a common currency with a falsifier per line — **1 adopted bit** (FC-W/δ), **4 selected types** (D=3, singlet, ℭ generator-set, A_μ=𝒫_T J_μ), **4 named results** (Chudnovsky proven; CM-h=1/E1/E\*E\*\* open), **3 calibrations**, the **empirical bridges** (x₊=1/α `[SMC]` + ~131 `[PARAMETRIC]` + ~50 external), **2 declined bets** (M, reversibility). ⚠ Reading guard: the "1 adopted bit" is the α-sector branch choice only — never the total physics import.

| Boundary | Verdict | Canonical doc |
|---|---|---|
| **α (the EM coupling)** | **DYNAMICAL, not structural** — route-invariant (FTD-0242); K-BIND closed *theorem-negative* (FTD-0244); **FTD-0314 extends this to *all* finite-symmetry carriers** (the distinguishing surd `δ = √(G*(4G*−1))` is transcendental over ℚ) ⇒ W is provably *external*. The constitution adopts it as **FC-W (FTD-0315)**, under which `x₊=1/α` is a `[CONDITIONAL THEOREM given W]` — still **not** `[DERIVED]`, unconditionally `[SMC]`. **July-2026 sharpening:** δ ∉ N for the *defined* native closure `[THEOREM — conditional on E0–E2]` (FTD-0369, as A0-amended); the exclusion is a coordinate-ramification law, Ram_t = {0,∞} (FTD-0370); the conditionality is maximally named — E1 = multi-curve Chudnovsky, E2 behind the (e,π)/exponential-period-conjecture wall, **floors closed, walls named** (FTD-0376/0377/0378); the whole boundary restated in period/GPC vocabulary (FTD-0375). | `SPEC_ALPHA_DYNAMICAL_BOUNDARY` + `SCOPE_DELTA_INDEPENDENCE_PROGRAM` |
| **Native fermion emergence (Branch-A)** | **`[CLOSED NEGATIVE at every protocol tested]`** — DK evolution (FTD-0379, incl. the v1.1 true-DK-operator re-run, fitted speed a\*≈0) and su(2) closure noise-recoverability (FTD-0380) both closed negative under pre-registered locks; Branch-B (imported Wilson–Dirac matter, imposed coupling = IMP-E1∘IMP-E3, gauge connection A_μ=𝒫_T J_μ priced IMP-S4) is the current accounting. | `SCOPE_VERTEX_PROGRAM` |
| **Native rest-mass gap** | **`[MEASURED — CLOSED NEGATIVE]`** (FTD-0362) — no native mass gap in the probed regime. | LEDGER FTD-0362 |
| **Cluster-mass law N(A)** | **`[BOUNDARY HARDENED]`** on 3 axes (exit-i FTD-0276, exit-ii convention FTD-0307, reduction FTD-0309): shape derived, calibration engine-emergent, no scalar reduction. | `SPEC_FTD0110_BRIDGE_BOUNDARY` |
| **Confinement (area-law)** | **`[OPEN STRUCTURAL OBSTRUCTION]`** — no deterministic-substrate analog; YM proof retracted (FTD-0042), per-voxel mass gap (FTD-0044) survives. | `SPEC_OPEN_MATH_BY_SECTOR` §4 |
| **Cosmology** | ΛCDM apparatus + FTD numerology; no cosmological observable derived. | `SPEC_COSMOLOGY_FRAMEWORK_BOUNDARY` |
| **Reversibility / Lorentzian metric** | not forced by P1–P5 (FTD-0253); FC-2 commits to native-arrow + emergent-IR metric. | constitution §2.6, §6.2 |
| **Measurement map M / non-commutativity** | logically independent of P1–P5 (FTD-0243 `[THEOREM]`); FC-1 declines it. | constitution §2.4 |
| **Atomic spectra / QM dynamics** | wrong dispersion (ω∝k vs Schrödinger ω∝k²); ~0% substrate-derivable (FTD-0270); the engine-native FFT is also instability-limited (FTD-0308). | `AUDIT_ATOMIC_DYNAMICS_STATUS` |
| **Born rule** | binding `[THEOREM]` + sharpness `[OPEN]` (the missing ℤ/3); detection is Rice, not Born (PL-1). | `AUDIT_SPEKKENS_KNOWLEDGE_BALANCE_PARTIAL` (FTD-0227) |

### §3.5 · The 2026-07 arc register (FTD-0366..0382 + the finishing arc) — tags verbatim from the LEDGER

| id | Content (one line) | Tag |
|---|---|---|
| FTD-0366 | G\* as the irreducible transcendental of the CHPS quartic matrix model; ℤ₄ sectors realize the χ₋₄ parity twist | `[SYNTHESIS]` + `[STRUCTURAL OBSERVATION]` |
| FTD-0367 | Reflection flow parity — product/ratio branches as first-order flows; c_R hypertranscendental | `[THEOREM — classical identities assembled]` + `[coherent-interpretation]` |
| FTD-0368 | δ-independence program chartered (N0–N3 ladder, stages S0–S4, guards) | `[SCOPE / PROGRAM CHARTER]` |
| FTD-0369 | δ-IND v1 verdict: δ ∉ N | `[THEOREM — conditional on E0–E2]` + `[THEOREM — conditional on Chudnovsky only]` (BCC sector, m=1-restricted, retirement suspended) |
| FTD-0370 | Ramification locus Ram_t(hull) = {0,∞}; δ de-specialized as the c=1/4 instance | R1 `[THEOREM — conditional on Chudnovsky only]` + R2 `[THEOREM — conditional on E0 + E**]` |
| FTD-0371 | The priced-import ledger (rev v1.1 2026-07-12: IMP-S4 minted) | `[SYNTHESIS]` |
| FTD-0372 | 18-point stencil LGF: order-4/degree-12 ODE, irreducible; W₁₈ a genuinely new period | `[NUMERICAL FACT — reconstructed operator]` |
| FTD-0373 | W₁₈ is not self-dual; rigid-CY/weight-4-modular branch closed | `[THEOREM — exact local-exponent obstruction]` |
| FTD-0374 | Two-loop BCC sunset stays lemniscatic; B to 230 certified digits; no classical Γ(1/4)-quotient closed form | `[NUMERICAL FACT]` (exponents/operator) + `[SMC]` (CM-type reading) |
| FTD-0375 | Period-conjecture framing of the import boundary; GPC a theorem for h¹(E_lemn) | `[SYNTHESIS]` (⚠ A4 needs external review before outward citation) |
| FTD-0376 | E1/E2 transcendence SOTA; δ∉N priced = Chudnovsky (proven) + Rohrlich–Lang (open) + exp-period conjecture (open) | `[SYNTHESIS]` |
| FTD-0377 | {π, W_SC} algebraically independent — disc −24 reduction; E1 wall re-priced to multi-curve Chudnovsky | `[THEOREM — external, assembled]` (⚠ external review owed) |
| FTD-0378 | Exponential lattice periods transcendental (SC + BCC); E2-full behind the (e,π) wall | `[THEOREM — external, assembled]` (Thm A) + `[THEOREM — assembled]` (Thm B) + `[SYNTHESIS]` (⚠ SO₄ hazard: never cite full functional independence for the BCC ₂F₃ block) |
| FTD-0379 | Vertex M1: engine evolution does not satisfy the DK equation (true operator, fitted speed a\*≈0) | `[CLOSED NEGATIVE — DK at tested protocols]` + `[MEASURED — KG-better-described]` (⚠ never cite m\*≈0.21 as a mass) |
| FTD-0380 | Vertex M2: su(2) closure failure is not noise-recoverable; Branch-A closed, Branch-B chartered | `[CLOSED NEGATIVE — closure-robust under prescribed controls]` |
| FTD-0381 | Parity twist as superdeterminant on the CHPS χ₋₄-graded moment module (N≡0 mod 4; Ber\|₄ = G\*⁴/48; orientation forced) | `[STRUCTURAL OBSERVATION]` (redteam-corrected scope) |
| FTD-0382 | Bilateral-symmetry orientation-carrier criterion C_s = Stab_{O(3)}(v,g); exactly-C_s ⟺ D=3 (consonance, not derivation); δ/magnitude probe closed-negative | `[SYNTHESIS]` + `[STRUCTURAL OBSERVATION]` + `[coherent-interpretation]` |
| FTD-0143 | *(finishing arc)* FQCR Model-IV quadruple uniqueness scan executed per the 2026-05-06 lock: **uniqueness rejected** — 2401/2401 quadruples match α⁻¹ at 1e−5; readout quadruple-insensitive at t=1 | `[CLOSED NEGATIVE]` (Model IV stays `[SELECTION]`; FTD-0013 unaffected) |

*(The finishing arc also reconciled the tracker layer to canon — FTD-0348 m_H propagation, FTD-0350 Theorem-7 scope, FTD-0355 wording, MC-T4.1 closure — with zero promotions; see the LEDGER maintenance log, 2026-07-12.)*

---

## §4 · The rigidity-scan coverage map — KEYSTONE (the F10 "tagging ≠ resolution" defense)

A LEDGER tag *labels* a claim; it does not answer *is the numerical match statistically surprising?* That requires a look-elsewhere / uniqueness scan. **Most of the framework's [PARAMETRIC] periphery has never been scanned** — and where it has, the news is sobering (the 2026-07 arc entries above extend this record; cross-ref §3.5):

| Bucket | Claim family | Evidence |
|---|---|---|
| **(i) Scan-rigid** | **x₊ = 1/α** | FTD-0319 (the scan's dedicated row, formerly mis-cited "FTD-0189"): 0 non-G\* dual-matchers / 2.65M polynomials — the framework's *one* rigid identification (a `[NUMERICAL FACT]` under the registered gate; the "~4×10⁵:1 Bayes" is unsupported — runner yields ~19× scan-size; uniqueness is tolerance-conditioned). |
| **(ii) Scan-tested → NULL** | FTD integer/monomial catalog (general); cluster-mass SM identification; **FQCR Model-IV quadruple (4,6;3,2)** | FTD-0097: catalog over-rich >5× background; FTD-0262: SM ratios p=2.05 (chance-level); **FTD-0143 (2026-07-12): all 2401 quadruples reproduce α⁻¹ at 1e−5 — the ansatz choice is generic, uniqueness rejected**. |
| **(iii) Scan-tested → NOT rigid** | **sin²θ_W=3/13, α_s=7/59, m_e prefactor 16/3** (FTD-0310); **sin²θ₁₂=3/10, sin²θ₂₃=16/29, Δm²₃₁/Δm²₂₁=100/3** (FTD-0320) | **FTD-0310 + FTD-0320: none rigid.** α_s MDL-dominated by 2/17; sin²θ_W p≈0.05; prefactor p≈0.08; sin²θ₂₃ MDL-dominated by 6/11 (~10× better); Δm²-ratio by 33/1; sin²θ₁₂ chance-level (p=0.48). All six **demoted** to `[PARAMETRIC]`. |
| **(iv) Tagged, NOT scanned** | ~122 remaining `[PARAMETRIC]` — now essentially the integer-**combination** families (quark masses, ~90 hadron spectroscopy, decay rates, precision-QED imports, CKM Wolfenstein, Koide) | **No rigidity scan run** for the combination families (the simple-rational subset is now scanned, bucket iii). Their test is the FTD-0097-style combinatorial look-elsewhere scan (deferred v2; FTD-0097 already NULL at the monomial level). *Still the deepest open methodological question.* |

**The honest one-liner:** exactly **one** numerical identification (x₊=1/α) has survived an adversarial uniqueness scan; everything tested since — including the 2026-07 pre-registered executions — has come back NULL, non-rigid, or generic; the large remainder is tagged-but-unscanned. The framework's physics matches should be read as *suggestive*, with the rigid algebraic spine (§2) carrying the weight.

---

## §5 · The falsifiable prediction spine → `SPEC_PREDICTION_LEDGER_DEVIATIONS` + `SPEC_PREDICTIONS_FORWARD_2026`

| Row | Prediction | Status |
|---|---|---|
| PL-1 | detection = Rice upcrossing, not Born (R² 0.9923 vs 0.7137) | `[NUMERICAL FACT]`, pre-reg confirmed; **deviation now in closed form with the single parameter β = K_B/σ_n and an internal kill switch (FTD-0359)** |
| **PL-2** | substrate Bell **S ≤ 2** | `[THEOREM]` substrate; **observer-layer account of lab S>2 is `[SELECTION]`+`[OPEN]` — the framework's highest-risk burden** |
| PL-3 | co-measurable quadratures `[q,p]=0` | `[THEOREM]` substrate, FC-1-conditional physical |
| PL-4 | γ IR-emergent ∝ L⁻² (⟨100⟩) | `[MEASURED]`; **EP-1 blind-confirmed** L=257; diagonals `[OPEN]` (ultra-relativistic unconverged at L ≤ 193) |
| PL-5 | UV anisotropy ∝ k⁴ (p=4.0008) | `[MEASURED]`; **EP-3** verified to L=768 |
| PL-6 | structural nulls (monopole/SUSY/extra-dim) | `[THEOREM]` |
| FP-1..4 | first-order EWPT GW; Higgs λ=3/23 (FCC); LV nulls dim 5/6; structural nulls | registered forward, kill-conditions stated; external-horizon |

---

## §6 · The honest accounting → `CATALOG_PARAMETRIC_INSERTIONS` + `TRACKER_ONTIC_TRUTH`

- **~21 `[DERIVED]`/`[THEOREM]`** · **~131 `[PARAMETRIC]`** · **~10 `[IMPOSED]`/`[SELECTION]`** (~162 total claims; figures per the FTD-0348 correction of record). No drift across CATALOG / TRACKER_ONTIC_TRUTH / spine.
- **Truth tiers:** T1 rock-solid `[THEOREM]` (OT-1.x) · T2 conditional-on-classical-theorems · T3 numerical-fact · T4 the coefficient-16 `[CONJECTURE]` · T5 the central conjecture x₊=1/α `[SMC]`.
- **Rigidity overlay (the §4 news, quantified):** of the ~131 `[PARAMETRIC]`, **1** is scan-rigid (x₊, also the T5 conjecture; FTD-0319), **~6** are scan-tested-and-not-rigid (FTD-0310 + FTD-0320, all now `[PARAMETRIC]`), the bulk catalog is scan-NULL-at-the-monomial-level (FTD-0097), the FQCR quadruple is scan-generic (FTD-0143), and **~122 are unscanned** — now essentially the integer-combination families.
- **The import bill (the §3 pricing, one line):** 1 adopted bit + 4 selected types + 4 named results (3 open) + 3 calibrations + the empirical bridges + 2 declined bets — stratified, deliberately **no single headline number** (`SPEC_IMPORT_LEDGER.md` §5's reading guard).

---

## §7 · External validation status

- **Defensible-now (pure mathematics, no physics input):** the **G\* paper** (`PAPER_GSTAR_INTRODUCTION.tex`, 36pp, math.NT, submission-ready) and **Papers A/B** (π-free generator, BCC complex structure). These stand on the §2 spine alone.
- **Built for external circulation (2026-07-09):** `REF_EXPORTED_RESULTS_SPINE.md` (FTD-free, mathematician-facing statement of the closed theorem-grade number theory around G\*) and `REF_EXPORTED_PROBLEMS_E1_E2.md` (the open problems E1/E2 + the 18-point LGF classification, exported without FTD vocabulary).
- **In progress:** the **EFT measurement pilot** (β-function via blocking; `SPEC_EFT_RECOVERY_PROGRAM`) — decides whether Paper C is a measurement paper or an honest null.
- **Not externally validated:** every physics *identification* (α, masses, gauge ratios) — these are internal, at their §4/§6 tags.
- **The standing item (TRACKER §0):** **zero items in the corpus have been reviewed by a human outside the project** — every red-team/referee pass to date, including the 2026-07 adversarial reviews, is AI-generated self-critique. This is the program's remaining honest dependency and is not resolvable by any internal pass.

---

## §8 · The consolidated falsifier table

Every way the framework can be killed, upgraded, or refuted — merged from four previously-scattered families: the constitution's commitment falsifiers (§6.2), the prediction spine (§6.1 + `SPEC_PREDICTIONS_FORWARD_2026`), the per-import falsifiers (`import_ledger.json`, verbatim), and the closed-negative provenance (falsifiers that already fired or ran). **Status legend:** *standing* = live, checkable, not yet fired · *run* = executed, result recorded · *not-yet-runnable* = precondition or external horizon named.

### 8.1 Commitment falsifiers (constitution §6.2) — all standing

| ID | Falsifier — what fires it | What it kills | Status |
|---|---|---|---|
| FC-0·a | an inequivalent (non-ℤ[i]) lattice-symmetry reading producing the same spine | FC-0 (uniqueness of the reading) | standing |
| FC-0·b | any spine theorem failing under FC-0 | FC-0 | standing |
| FC-1·a | a substrate-native `[A,B] ≠ 0` derived from P1–P5 | FC-1 (the declined M becomes derivable) | standing |
| FC-1·b | a P1–P5 derivation of Born statistics | FC-1 | standing |
| FC-1·c | a substrate-native (M-free) S > 2 measured on the engine | FC-1 | standing |
| FC-2·a | exact γ / exact Lorentz invariance at finite L (off the IR limit) | FC-2 (native arrow) | standing — measured support so far: EP-1/EP-3 blind confirmations |
| FC-2·b | a P1–P5 derivation of global reversibility | FC-2 | standing |
| FC-2·c | the measured IR approach failing (the L⁻² law breaking, or k⁴ anisotropy reversing, at larger L) | FC-2 | standing |
| FC-W·a | **a forward-derived substrate object realizing `√(G*(4G*−1))` with a forced ℤ/2** — converts IMP-B1 from adopted bit to derivation, retires the ledger's largest line, upgrades x₊=1/α toward [SELECTED/DERIVED]. "The one refutation FTD would welcome." Named loophole: FTD-0314 §4's last door (a new forward-derived period) | FC-W (by *upgrade*) | standing (the K-BIND route to it is closed theorem-negative, FTD-0244; the FTD-0382 reflection-reduction probe also closed negative) |
| FC-W·b | the carrier-narrowing theorem failing / G\* shown algebraic | FC-W (and much of the spine's transcendence layer) | standing |
| FC-W·c | α measured to disagree with x₊ beyond tree-level tolerance | FC-W's motivation + IMP-E1 | standing (retrospective; α known to 0.08 ppb) |

### 8.2 Prediction-spine falsifiers (constitution §6.1)

| ID | Falsifier | What it kills | Status |
|---|---|---|---|
| PL-1 | detection statistics fit Born better than Rice | the detection-layer account | **run — Rice confirmed** (R² 0.9923 vs 0.7137; FTD-0359 closed form + internal kill switch) |
| PL-2 | an FC-1-consistent account of lab S > 2 failing to exist / substrate S > 2 | FC-1 (via 8.1) / the observer-layer `[SELECTION]` | **standing — highest-risk**: substrate bound S ≤ 2 is `[THEOREM]`, lab experiments measure S > 2, the reconciliation burden is accepted and open |
| PL-3 | non-co-measurable quadratures on the substrate | the commutative observable algebra | run (substrate, ~10⁻¹⁶) / standing (physical, FC-1-conditional) |
| PL-4 | moving-clock γ failing L⁻² IR emergence | FC-2's emergent-metric account | **run ⟨100⟩ — blind-confirmed** (EP-1, L=257); **standing** for diagonals (unconverged at L ≤ 193, `[OPEN]`) |
| PL-5 | UV anisotropy exponent ≠ 4 | the lattice UV signature | **run — confirmed** (p = 4.0008, EP-3, to L=768) |
| PL-6 | a magnetic monopole / SUSY partner / extra dimension observed | the structural nulls `[THEOREM]` | standing (external experiments) |
| FP-1..4 | first-order EWPT GW background absent at reachable sensitivity; Higgs λ ≠ 3/23 at FCC precision; LV at dim 5/6 observed; structural-null violations | the respective forward registrations | not-yet-runnable (external horizon; kill-conditions pre-stated) |

### 8.3 Per-import falsifiers (`import_ledger.json` rev v1.1 — 15 imports + 2 declined)

| Ref | Import | Falsifier | Status |
|---|---|---|---|
| IMP-B1 | the δ branch bit (FC-W) | = FC-W·a above | standing |
| IMP-S1 | D = 3 | a forcing proof (→ self-set); or an equally-consistent alternate D (confirms free choice) | standing |
| IMP-S2 | the singlet (J→ψ) | a native forced singlet; or a native S > 2 on the engine (an FC-1 falsifier) | standing |
| IMP-S3 | the ℭ generator-set (N_calc) | a canonically forced generating set (retires the FTD-0347 flag); or an outside generator changing N_calc | standing |
| IMP-S4 | A_μ = 𝒫_T J_μ (gauge connection) | an alternative flux-to-connection map with inequivalent vertex phenomenology at matched protocol | standing (minted 2026-07-12; until tested, vertex results carry it as an explicit conditional) |
| IMP-C1 | Chudnovsky 1976 | n/a — proven external; the *import* is the spine's total dependence on it | n/a |
| IMP-C2 | CM h=1 uniqueness | an h≥2 curve reproducing the identity; or a structural all-h proof | not-yet-runnable (open math; MC-T2.3 machinery note exists) |
| IMP-C3 | E1 (Watson-constant joint independence) | a proof either way re-adjudicates FTD-0369 via the frozen map | not-yet-runnable (open math = multi-curve Chudnovsky; exported as P1) |
| IMP-C4 | E\*/E\*\* (exponential-period leg) | a proof either way = progress | not-yet-runnable (behind the (e,π) wall; exported as P2) |
| IMP-K1 | a_phys ≡ ℓ_P | a substrate derivation of a_phys | **run — Mechanism-γ closed negative** (currently genuinely imported; derived-at-[SMC] under electron-primary) |
| IMP-K2 | t_phys | a substrate derivation of the tick | standing (derived-at-[SMC] under electron-primary gauge) |
| IMP-K3 | K_B = m_e anchor | disentangling the FTD-0130 role-conflation so the anchor is forced | standing |
| IMP-E1 | x₊ = 1/α `[SMC]` | α measured beyond tree-level tolerance (= FC-W·c) | standing |
| IMP-E2 | ~131 `[PARAMETRIC]` insertions | per-row: the borrowed standard formula is the falsifiable object (FTD supplies only the number); the m_H row already sits at **−4.1σ** (FTD-0348) — an *exclusion as exact relation*, honestly recorded | standing, per-row |
| IMP-E3 | ~50+ adopted external physics | each is the established physics's own test | standing (external) |
| DEC-1 | the measurement map M — **declined** (FC-1) | FTD predicts the substrate where M differs; the wager fires via FC-1·a/b/c | standing (a bet, not a debt) |
| DEC-2 | global reversibility — **declined** (FC-2) | the wager fires via FC-2·a/b/c | standing (a bet, not a debt) |

### 8.4 Closed-negative provenance — falsifiers that already ran (the record that keeps zombies dead)

| Ran | Result |
|---|---|
| FTD-0097 monomial look-elsewhere | catalog **over-rich** >5× background — monomial matches carry no evidential weight |
| FTD-0319 x₊ dual-match scan | **survived** — the one scan-rigid identification (tolerance-conditioned `[NUMERICAL FACT]`) |
| FTD-0262 SM cluster-mass scan | **NULL** (p = 2.05, chance-level) |
| FTD-0310 + FTD-0320 rigidity audits | six simple-rational identifications **not rigid** — all demoted `[PARAMETRIC]` |
| FTD-0143 FQCR quadruple scan (2026-07-12) | **uniqueness rejected** — 2401/2401 α-matchers; ansatz generic at t=1 |
| FTD-0244 K-BIND | **theorem-negative** — the native-carrier route to FC-W·a is closed |
| FTD-0208 clock hypothesis v3 | **closed negative, axiom-level** — the L² budget is an independent import |
| FTD-0131 "G_N = 1/100" | **falsified** under every natural reading (10²⁰–10⁴³ off) |
| FTD-0025 confinement routes | all three classical substrate routes **closed negative** (structural obstruction stands) |
| FTD-0362 native mass gap | **closed negative** in the probed regime |
| FTD-0379/0380 vertex Branch-A | **closed negative** at every protocol tested (DK + su(2) closure) |
| FTD-0193 spin-2 substrate mode | **closed negative in the probed regime** (Frontier 4 stays open; the Einstein-chain graviton is imported, FTD-0189) |

*Complete set: `SPEC_OPEN_MATH_BY_SECTOR.md` §13 + the LEDGER's closed-negative rows.*

---

## §9 · What FTD has NOT delivered (the honesty row)

α as a derivation · the Born rule from P1–P5 · an FC-1-consistent (M-free) account of lab Bell S>2 · QFT scattering amplitudes / interference at the observer layer · atomic spectra · any cosmological observable · confinement · a derived mass spectrum (genesis or Gaussian sector) · **native fermions (Branch-A closed negative, FTD-0379/0380)** · **a native rest-mass gap (FTD-0362)** · **a substrate spin-2 mode in the probed regime (FTD-0193; the GR chain's graviton is imported)**. **And the F10 gap:** ~122 of the ~131 `[PARAMETRIC]` matches are *tagged but never rigidity-scanned* — the single most important methodological caveat. *Any framing that presents these as derivations is overclaim.*

---

## §10 · Forward (→ `SPEC_OPEN_MATH_BY_SECTOR` + doctrine ledger §14)

The remaining program is **mapping, validation, and external exposure — not α attacks** (closing the central boundaries would require a new 6th-postulate-class input, and the doctrine ledger's own assessment is that the maximal self-claim `T_FTD` "may not be achievable without ontology extension beyond the 5 axioms"):

1. **External human review** — the standing TRACKER §0 item; circulate `REF_EXPORTED_RESULTS_SPINE.md` + `REF_EXPORTED_PROBLEMS_E1_E2.md` (the items flagged "needs external eyes": FTD-0375 A4, FTD-0377, FTD-0378's no-rank-1 lemma). Nothing internal can substitute.
2. **Rigidity-audit extension to bucket (iv)** — the FTD-0097-style combinatorial scan over the ~122 unscanned `[PARAMETRIC]` combination families; the §4 map's completion.
3. **Doctrine-ledger Phase-2 hardening priorities** (all `[OPEN]`): MC-T3.1/FTD-0110-NL nonlinear bridge (with the FTD-0277-v1-negative constraint), MC-T3.6 β-coefficients, χ_H, flavor depth matrices, confinement (structurally obstructed), substrate strong-field gravity (FTD-0184).
4. **Named engine/theory residues:** PL-4 diagonals; the nonlinear-rung v2 pre-registration (δ-IND); M1-v2 vertex follow-up + the Program-F effective-toggle audit; the E1/E2 walls (progress in either direction re-adjudicates FTD-0369 via the frozen map).
5. **The queued dissemination refresh** — the public layer (whitepaper, manuscript_v2) still carries the retired x₋↔N_c identification and pre-FTD-0348 Higgs figures; queued as one coherent arc (TRACKER §10), not piecemeal patches.

---

*Nothing in this document promotes any claim. FTD-0013 `[SMC]`, MC-T4.3 `[FOUNDATIONAL OBSTRUCTION]`, FC-0/1/2/3/W `[AXIOM]`-class, D=3 `[SELECTION — declared]`, the algebraic spine — all at their canonical tags. Golden gate untouched (docs+scripts arc). LEDGER FTD-0311 `[SYNTHESIS]` v2 (id retained); FC-W rows FTD-0314/0315; import ledger FTD-0371 rev v1.1.*
