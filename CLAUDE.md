# Foundational Ternary Dynamics (FTD) — Project Instructions

**Version:** 5.40 (post-2026-05-08 F6 — FTD/FQCR Doctrine Ledger v1.2 shipped: FTD-0145 [SYNTHESIS] roll-up of LEDGER + TRACKER_ONTIC_TRUTH + SPEC_ALGEBRAIC_SPINE + SPEC_FQCR + CHECKLIST_MATH_COMPLETE into single-page status map; §7 bivector/Dirac bridges [OPEN] per FTD-0073; §8 sin²θ_W at two scales (GUT 3/8 [SELECTION] / IR 3/13 [PARAMETRIC FTD-0018]); §10 flavor depth matrices [PARAMETRIC scaffold]; §12 cites FTD-0131 partial gravity closure α_G(e,e)≈0.38%; **NO tag promotions, NO new claims, NO derivations**; baseline 5.39 prior content preserved below)
**5.39 baseline** (post-2026-05-04 night Phase B cluster-persistence arc + trim-the-fat round 4 — 4 retractions in F1/F9 hygiene pattern + (a)+(b)+(c) closure under FTD-0136; **toggle interactions are non-linear under full physics** ("sum greater than parts" operationally confirmed at L=32); **two stability islands at A∈{9.0–9.5} and A=13.0** amid flooding regimes at L=64 full physics ([OBSERVATION], pre-registered falsification queued); **L=256 full-physics 3-axis spot check** via WSL2/CUDA (linear axis→color binding x→R y→G z→B sizes {1,2,3}, sub-saturation caveat); cross-L set-property holds: every (axis, L) under full physics returns a framework integer; SPEC §5.6.21–§5.6.27 documents full arc; LEDGER FTD-0136 carries provenance; **trim-the-fat round 4** removed 30 superseded Phase B exploratory tests (-5,397 LOC; commit `08c517e`); 9 load-bearing Phase B keepers (cluster_tracker + 4 persistence sanity tests + 4 dump_full_physics* runners) build clean via WSL2/CUDA ninja)
**Full specification:** [`docs/SPEC_FTD.md`](docs/SPEC_FTD.md)
**🔑 Start here if resuming:** [`docs/WHERE_WE_LEFT_OFF.md`](docs/WHERE_WE_LEFT_OFF.md) — single-doc context recovery.
**🧭 Architecture navigation:** [`META_PROJECT_ATLAS.md`](META_PROJECT_ATLAS.md) — task→file table + directory tree + subsystem dependency graph + post-refactor sweep history (§10).
**📜 Cross-module contracts:** [`CONTRACTS.md`](CONTRACTS.md) — 12 contract sections (bridge state, capability factories, scale ctx, scenarios, toggles, energy convention, constants chain, telemetry, refactor companion, cascade callback, mesh-factory callback, golden-tick gate).
**🏛 Architectural decisions:** [`docs/adr/INDEX.md`](docs/adr/INDEX.md) — 13 ADRs governing patterns (4 new from refactor sweep: 0010–0013).
**🔍 Audit ledgers:** [`docs/audits/INDEX.md`](docs/audits/INDEX.md) — historical sweep ledgers, including [the 2026-04 refactor sweep](docs/audits/AUDIT_2026-04_refactor-sweep.md).

---

## Number-One Goal

> **Derive everything we can from a discrete ontology — and rigorously establish what we cannot.**

This is the project's single north star. The algebraic spine, the engine, the pre-registered tests, the LEDGER, the manuscripts — each one serves this goal or is subordinate to it.

Read precisely:

- **"Discrete ontology"** — FTD's five postulates: a finite, undefined-boundary lattice (no completed infinity, no primitive continuum); discrete time; ternary states {−1, 0, +1}; local Moore-neighbour causality; determinism. The ternary cubic lattice is the current concrete model; discreteness and finiteness are the non-negotiable commitment.
- **"Derive"** — a strict, explicit chain from those postulates to the result: `[THEOREM]` or `[DERIVED]`. A `[PARAMETRIC]` insertion (a standard physics formula filled with FTD numbers) is **not** a derivation; a `[STRONGLY MOTIVATED CONJECTURE]` is a *match*, not a derivation. A claim's LEDGER tag is the measure of whether it serves the goal.
- **"...and establish what we cannot"** — the boundary is itself a deliverable. Rigorously showing that the discrete ontology does **not** determine something (e.g. the value of a dynamical coupling) is as much a project result as a derivation. Closed-negatives are not failures; they map how far discreteness reaches. The project succeeds by drawing that map honestly in both directions.

**Operational test** for any claim, paper, or experiment: does it *derive* something from the discrete ontology, does it *mark a boundary* of what discreteness determines, or is it a match/import still awaiting one of those two verdicts? The Epistemic Discipline rules and tag system (below) are *how* the goal is pursued with rigour; this goal is *what* is pursued.

---

## Current epistemic state (2026-05-21 — post physics-panel review, boundary-theorem correction, FTD-0189 Outcome A)

The 2026-05-21 session subjected the framework to an adversarial physics-panel review and acted on the verdict. Net effect: the epistemic record got **more honest** (two claims corrected, none promoted), and the one decisive pre-registered test ran and returned a favourable but tightly-scoped result.

- **Physics-panel review.** A five-physicist adversarial panel (Pauli, Feynman, Dirac, Noether, Einstein) stress-tested the ontic chain. Verdict: the algebraic spine is genuine theorem-grade mathematics; everything that turns it toward *physics* is weaker than prior framing implied. A forward plan (priorities P1–P6) was adopted.

- **Boundary theorem (FTD-0186) — honesty-corrected.** The structural/dynamical discriminator's v1 pre-registered falsifier **fired** (type-ii closed-negatives violate criterion A1 as locked). Honest status: `[DEFINITION]` (the discriminator stands) + `[OPEN]` (the classification — a fresh v2 pre-registration + re-run is required). It is **not** a "theorem." See `FOUND_STRUCTURAL_DYNAMICAL_DISCRIMINATOR.md` §5.

- **Algebraic spine reconciled to its honest count.** "Nine theorems" → **6 theorem-grade + 3 honestly-tiered** (CM-uniqueness = `[NUMERICAL FACT]`; coefficient-16 = a value-level identity whose structural necessity is `[CONJECTURE]`, T4; Phase-J = `[THEOREM at L=2]`). Canonical: `SPEC_ALGEBRAIC_SPINE.md` §0.

- **FTD-0189 — adversarial look-elsewhere scan, Outcome A.** Pre-registered (tag `preregister-adversarial-look-elsewhere-v1`, commit `9e5ad8f`) and run: the master-quadratic template over an 18-constant basket FTD did **not** design. The master quadratic is the **unique** dual-matcher — 0 non-G\* dual-matchers across 2.65M degree-2 polynomials; rank 1 by ~130×. The family-conditioning objection (Pauli/Dirac) is answered. **`x₊ = 1/α` (FTD-0013) retains `[STRONGLY MOTIVATED CONJECTURE]`** — evidential basis upgraded, **no tag promotion**. Analysis doc + LEDGER FTD-0189 row pending. (Renumbered from FTD-0187 on 2026-05-21 — that ID is held by the Born-rule consolidation LEDGER row; FTD-0188 is the κ_ψ=4π audit.)

- **FTD-0184** (FQCR red-team — the exponential-metric gravity route is the Yilmaz metric, `[CLOSED NEGATIVE]`); **FTD-0185** (alpha-arithmetic-generativity pre-registration / desk-audit gate).

- **G\* computation routes.** Two verified fast routes to G\* — the Landen log-derivative form and Guillera's quartic self-replication (arXiv:1702.05378) — in `scripts/proofs/` + `docs/theory/09_mathematical/REF_GUILLERA_CORPUS_MAP.md`. They strengthen the *computation* of G\* (spine link ②); they bear on no physics claim.

- **The honest headline.** The central conjecture `x₊ = 1/α` stays `[STRONGLY MOTIVATED CONJECTURE]`. No claim was promoted this session; two were honesty-corrected. The physics mechanism (MC-T4.3) and a real-theorem Stage 2 of the boundary theorem remain `[OPEN]`. `docs/papers/PAPER_GSTAR_INTRODUCTION.tex` (pure number theory) is submission-ready and not yet on arXiv.

- **Corpus consolidation — merge pass complete (2026-05-22).** The reconcile + archive + restructure consolidation ran to completion: canonical-layer drift reconciled, dead content archived, the navigation layer restructured, and a merge pass consolidated 28 overlapping documents into 13 (every tagged claim, theorem, numeric result, and `FTD-NNNN` cross-reference preserved; husk references repointed corpus-wide; LEDGER edits path-only). Every cluster now carries a local `INDEX_*` navigation file. The active theory corpus is **~349 documents** across 10 clusters. Plan: `.claude/plans/take-the-role-of-fancy-kahn.md`.

Read **`docs/theory/07_assessment/TRACKER_ONTIC_TRUTH.md`** (canonical bedrock) and **`docs/theory/07_assessment/LEDGER.md`** (per-claim status) before defending any FTD math claim.

---

## Previous epistemic state (2026-05-03 — post-publication-trio + tracker consolidation)

After 2026-05-02 + 2026-05-03 (19 commits across two days), the project has its most externally-defensible state to date and its most consolidated documentation footprint. Key updates:

- **Publication trio complete**, all build clean, all anti-target audited:
  - **Paper A v2** (`PAPER_A_PI_FREE_GENERATOR.tex`, 8pp, LMP-target): π-free generator + extended polynomial scan (2.87M polynomials, ~4×10⁵:1 Bayes, 0 Eisenstein-family dual-matchers). Pre-reg tag `preregister-polynomial-scan-extended-v1`.
  - **Paper B v1** (`PAPER_B_BCC_COMPLEX_STRUCTURE.tex`, 7pp, LMP-target): BCC complex-structure theorem + dual-4 partial unification (Roles 1+3 [DERIVED]) + honest no-go (Roles 2+4 cannot unify with Z[i]^×).
  - **Paper C revision** (`PAPER_FTD_AS_WILSONIAN_EFT.tex`, 15pp): Branch-A native EFT measurements + Phase-G reframe + structural-decoupling diagnosis. Old "160× QED β" framing wrapped in `\sout` with retraction note.

- **3 new LEDGER entries** (FTD-0122 through FTD-0124):
  - **FTD-0122 [DERIVED for Roles 1+3] + [NO-GO for Roles 2+4]**: BCC complex-structure theorem. Z[BCC] ⊗ Q decomposes as `V_triv² ⊕ V_sign² ⊕ V_complex²` with V_complex carrying natural Z[i]-module structure ≅ Z[i]². Unifies CM Aut count + tower level k=4 via Z[i]; no-go for O_h^ab (Klein four, not Z/4) and 27-block orbit count (sizes (1,6,12,8) cannot be permuted).
  - **FTD-0123 [NUMERICAL FACT]**: Chowla-Selberg h≥2 scan. 63 fundamental discriminants spanning class numbers 1-4 (|d| ≤ 907). ZERO h ≥ 2 dual-matchers via Γ-product analogue. Theorem 3 numerical net 7× larger.
  - **FTD-0124 [NUMERICAL FACT + METHODOLOGICAL]**: 9-Heegner rigidity scan + criterion-bifurcation. 5814-quadruple grid. Trivial-multiplier criterion: 1/5814 match (canonical d=−4). Rational-multiplier criterion: 21/5814 matches. Load-bearing methodological finding: framework currently applies BOTH criteria in different places without flagging.

- **Theorem 3 honestly restated** (SPEC_ALGEBRAIC_SPINE.md §3): now `[NUMERICAL FACT, exhaustive across class numbers 1-4 with |d| ≤ 907; under the trivial-multiplier criterion declared in §3]`. Criterion declaration is load-bearing per FTD-0124.

- **Canonical bedrock tracker shipped**: [`TRACKER_ONTIC_TRUTH.md`](docs/theory/07_assessment/TRACKER_ONTIC_TRUTH.md). 5 truth tiers; 6 T1 ★★★★★ rock-solid + 3 T2 ★★★★ conditional + 4 T3 ★★★ numerical + 1 T4 ★★ (coefficient 16 = |Aut(E)|²) + 2 T5 ★ (the central α / N_c conjectures). Each row has unique `OT-N.M` ID and points at a verification artifact. **Read this FIRST before defending any FTD math claim.**

- **87-paper inventory database**: [`dissemination/papers/INVENTORY.json`](dissemination/papers/INVENTORY.json) + [`INVENTORY.md`](dissemination/papers/INVENTORY.md). Auto-generated by `scripts/build_paper_inventory.py`. Anti-target audit + heuristic tier + verdict (KEEP/REVISE/RETIRE/ARCHIVED) per row. Initial: 12 KEEP, 2 REVISE, 43 RETIRE, 30 ARCHIVED.

- **6 live trackers, 0 stale** (post 2026-05-03 consolidation):
  - `TRACKER_ONTIC_TRUTH.md` (canonical bedrock)
  - `TRACKER_OPEN_ITEMS.md` (every [OPEN] item)
  - `LEDGER.md` (per-claim provenance)
  - `SPEC_OPEN_MATH_BY_SECTOR.md` (sector-organised research queue; replaces archived `CHECKLIST_MATH_COMPLETE.md`)
  - `INVENTORY.{json,md}` (papers)
  - `AUDIT_WEAKNESSES_MASTER.md` (cross-cutting weaknesses)

  4 stale trackers deleted (TRACKER_DOCUMENT_STATUS, ISSUE_TRACKER, TRACKER_PDF_ONLY_PAPERS, TRACKER_REFRAME_FLAGS — total 1499 lines; git history preserves them).

- **MC-T4.1 reframed** (`cc93c2d`): not a Severity-1 ontological gap. SPEC_FTD §1.1 graded-monism table establishes J-primary; Postulate 3 made explicit. Severity demoted to docs-alignment.

- **Overclaim cleanup** (`be045b3`): README, manuscript v2 prefaces (src + vol1), FAQ data.js. 6 FAQ THEOREM tags downgraded. Browser preview verified.

- **MC-checklist current state** (Tier breakdown):
  - **Tier I: 5/5 closed**
  - **Tier II: 3/3 closed** (T2.3 §4 item 3 also closed today via FTD-0123; structural theorem item 4 remains [OPEN])
  - **Tier III: 1/5 closed (T3.2 m_e exponent n=11 [DERIVED])**, 3/5 investigated, 1/5 blocked
  - **Tier IV: T4.5 Roles 1+3 [DERIVED] + Roles 2+4 [NO-GO]** (FTD-0122 closure); T4.1 reframed (docs-alignment); T4.2, T4.3 (foundational obstruction), T4.4 unchanged

- **α-derivation routes status**: R1/R2/R3/R4, Z-factor, RG-running, algebraic combinations, 1/√d, Langevin-equipart all closed-negative (carried over). The IDENTIFICATION x_+ = 1/α stays [STRONGLY MOTIVATED CONJECTURE]; structural evidence is the strongest the framework has held — Bayes ~4×10⁵:1, Eisenstein-family null, h≥2 null, BCC complex-structure unification of CM Aut count with tower level k=4.

Read **`docs/WHERE_WE_LEFT_OFF.md` §0.6** for the comprehensive
2026-05-02 evening + 2026-05-03 session summary, **`TRACKER_ONTIC_TRUTH.md`** for the canonical bedrock, and **`SPEC_PHYSICS_BRIDGE.md`** for the physics-bridge synthesis.

The publication trio (Papers A, B, C) is ready for external pre-submission review by a number-theory / representation-theory colleague unfamiliar with FTD. Suggested review focus: tightness of Paper A §6 / Paper B Theorem 3.1 proofs + honesty of Paper B §6 no-go + Paper C Phase-G reframe consistency.

---

### Earlier today (2026-05-01 morning — post Maxwell-exploit thread closure)

After the 2026-04-30 / 2026-05-01 two-day session, the project gained
**9 new LEDGER entries** (FTD-0112 through FTD-0120) and the algebraic
spine is now at **9 theorems** (Theorem 8 = (1+i)-tower harmonic invariant,
Theorem 9 = field-theoretic Q(G\*) characterization). The
**Maxwell-exploit thread is COMPLETE** with all 8 sub-questions Q1-Q8
addressed. A canonical-reference G\* typo bug (FTD-0117) was caught and
fixed across 5 docs. The FTD-0110 nonlinear-bridge gap was analyzed and
sharpened (FTD-0119) but not closed.

Read **`docs/WHERE_WE_LEFT_OFF.md` §0** for the latest-session summary
and §0.3 for the priority queue. The previous-session foundations
(2026-04-27 engine refactor + 2026-04-28 FTD-0110 linear-level closure)
are preserved below as historical context.

Do **not** claim results stronger than what's listed in
`docs/WHERE_WE_LEFT_OFF.md` §0 / §0.1 without re-auditing.

---

### Previous epistemic state (2026-04-27 evening — post engine refactor sweep)

After the 2026-04-27 engine-as-instrument portfolio + look-elsewhere scan,
the project is in a structurally narrowed but defensible state. The
engine codebase was then decomposed across 8 phases (17 commits 2db67ca…87158ae)
with bit-exact physics preservation; physics-bearing claims are unchanged
by the refactor. Do **not** claim results stronger than what's listed in
`docs/WHERE_WE_LEFT_OFF.md` §4 without re-auditing. The bird's-eye
assessment lives in `WHERE_WE_LEFT_OFF.md` §10 — read that for
"what's missing" diagnosis.

**Engine architecture (post-refactor):** the 5 hottest files were
decomposed into focused modules following 4 newly-codified patterns:
cascade callback (ADR-0010), mesh-factory callback (ADR-0011), golden-tick
regression gate (ADR-0012), and TOGGLE_SPECS[] table-driven toggles
(ADR-0013). `viewport.js` 3953→1256 LOC; `wasm-bridge-dag.js` 2395→42 LOC;
`render_bridge.cpp` 1231→545 LOC; `kernels_stencil.cu` 1530→deleted-and-split-into-3-TUs.
See [META_PROJECT_ATLAS.md §10](META_PROJECT_ATLAS.md#10--refactor-sweep-history-2026-04-27-completed)
for the full commit ledger and [docs/audits/AUDIT_2026-04_refactor-sweep.md](docs/audits/AUDIT_2026-04_refactor-sweep.md)
for the audit. **WSL2 GPU parity verified 2026-04-28** (golden hash `0xcd957b601d47868a` bit-exact at L=16 on CUDA backend; `gpu_parity_complete` 70/0 across all 20 physics domains at L=32; `sim_parity` PASS at 100 + 500 ticks). The refactor sweep is fully verified end-to-end across both CPU and CUDA backends.

**Firm theorems (nine numbered: six theorem-grade + three honestly-tiered below theorem grade — canonical reference: `docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md` §0):**
G\* algebraic identity (= Γ(1/4)/Γ(3/4) ≈ 2.9587 — note: NOT the
Bernoulli/Gauss lemniscate constant ϖ ≈ 2.622; per FTD-0117 fix), master
quadratic polynomial + roots, CM curve uniqueness among class-number-1
fields (operationally tabulated in `EXPLR_CM_RATIO_TOWER.md`),
coefficient 16 = |Aut(E)|² for E: y² = x³ − x, Watson identity
W₃ = G\*²/(2π), Phase G geometric Coulomb = lattice Poisson Green's
function at every finite L, Phase J partition-function ultralocality,
**(1+i)-tower harmonic invariant** (Theorem 8, FTD-0111, 2026-04-29:
1/y₊ + 1/y₋ = 1 with anomaly transcendence A_k ∉ Q for k ≥ 4),
**field-theoretic Q(G\*)** (Theorem 9, FTD-0112, 2026-04-30: maximal
π-free subfield of Q(π, Γ(1/4)) conditional on Chudnovsky 1976).
**All nine numbered results stand — six theorem-grade, three honestly tiered below theorem grade (see `SPEC_ALGEBRAIC_SPINE.md` §0).**

**Maxwell-exploit thread COMPLETE (2026-04-30 / 2026-05-01):** 9 LEDGER
entries FTD-0112 through FTD-0120. Lattice ED framework now covers
every classical EM phenomenon — static Coulomb (Phase G FTD-0004),
retarded radiation (FTD-0113), Bianchi identities (FTD-0114), boosted
Coulomb + lattice Cherenkov pole (FTD-0115), extended sources + Cherenkov
rate (FTD-0120 Q6/Q7), Larmor (FTD-0120 Q5), source-half consistency
(FTD-0120 Q8). The Z-factor reading FTD-0116 was floated as
[HYPOTHESIS] and falsified via Q4a numerical test (CLOSED NEGATIVE).
What remains for full Maxwell-on-FTD: dynamical source coupling
(g_s ↔ α relationship, EFT recovery program territory). 5 new proof
scripts; all PASS at machine precision (where applicable).

**[STRONGLY MOTIVATED CONJECTURE]:** x+ = 1/α (1.26 ppm); x− = N_c
(0.80%); the master quadratic dual-prediction property (both roots
simultaneously matching unrelated physical sectors) is the strongest
structural evidence — explicitly distinguished from monomial-level fits
that the FTD-0097 scan ruled as chance-level on 2026-04-27.

**[PARTIAL] — engine-as-instrument findings (2026-04-27):**
- **FTD-0107: deterministic cluster counts L-invariant at L ∈ {32, 64}**
  (1 from point injection, 2 from collision; 5/5 seeds at both L; cluster
  sizes absolute, ~25 voxels for ic1, ~3-5 voxels for ic3). The most
  novel positive structural finding of the engine-as-instrument program.
  See `ANALYSIS_EMERGENT_SPECTRUM_G1.md`.
- **FTD-0103 continuum-limit**: cond(S) monotone improving across L;
  Wilson eigenvalue positivity non-monotonic.
- **FTD-0104 topology atlas**: clean grid match across Wilson loop, flux
  tube, monopole, vacuum instanton at L=32.
- **FTD-0105 lemniscatic 2-sphere test**: PASS-NONE strict, secondary
  closed-negative — lattice horizon is sphere-symmetric.

**[MEASURED] — methodological-hygiene scans (2026-04-27):**
- **FTD-0097 look-elsewhere scan**: NULL REJECTED upward at ε = 10⁻⁴
  (62 raw / 11 dedup hits vs Poisson null λ=4); χ²(df=19) = 470 raw / 38
  dedup; per-target uniformity rejected at 99.9%+ raw / 99% dedup. Catalog
  is over-rich at the monomial level. The L2 identity 8·G\*²·α appears
  in the scan as a chance-level fit at exactly its reported 68.77 ppm
  precision. **Confirms FTD-0094 [PARAMETRIC] from methodological side.**
  See `AUDIT_LOOK_ELSEWHERE_RESULTS.md`.

**[CLOSED NEGATIVE]:**
- **FTD-0050** (master quadratic as characteristic polynomial of RG step;
  2026-04-20). Engine stencil orthogonal to BCC. Does NOT demote
  FTD-0001/0013/0014 — algebraic spine unchanged.
- **FTD-0093 Mechanism C** (g_c as bridge-operator eigenvalue on σ_BCC;
  closed 2026-04-27 at L ∈ {24, 32, 48} with non-monotonic ratio trend
  rejecting predicted 45.31). Combined with prior closures of Mechanisms
  A and B, **all three first-principles routes for g_c are now closed
  negative; g_c remains [PARAMETRIC]**.

**[PARAMETRIC] (terminal demotion 2026-04-27):**
- **FTD-0094** (L2 candidate identity 2·m_e/α = 16G\*²; demoted per
  pre-registered criterion: FTD-0093 closed AND FTD-0096 [OPEN]). Confirmed
  from methodological side by FTD-0097's m_e-cluster of chance-level fits.
- sin²θ_W (3.5%), sin²θ_13 (12.6%), α_s = 7/59, PMNS angles — already
  demoted April 19.

**[OPEN] (the real research program):**
- ~~**WHY 25 voxels for ic1 cluster?**~~ — **ANSWERED 2026-04-28 (FTD-0110 [DERIVED at linear level])**:
  cluster size scales as `N(A) ≈ ¼·(A/K_GENESIS)²` with `k = 1/N_base = ¼`.
  ¼ coefficient now **DERIVED** from O_h representation theory: mult(A_{1g}) = 4
  in the 27-block by character-table formula [THEOREM]; δ_center is A_{1g}-pure
  (O_h-fixed point); the 18-point Laplacian preserves the 4-dim A_{1g} subspace;
  δ_center projects onto 4 A_{1g} eigenmodes with energies {3/8, 1/8, 3/8, 1/8},
  mean = 1/N_base = ¼; cluster harvests the mean. Direction-invariance follows
  automatically (per-component scalar evolution). See `DERIV_K_FROM_OH_A1G_MULTIPLICITY.md`.
  25 voxels at A=10 is the canonical-amplitude steady state. SM-particle masses
  identified with cluster size N at amplitude A=2√(m/m_e): e/μ/π/K/p/τ match within
  0/1.1/2.0/10/15/18% naively, ~5% with empirical k(A) drift correction.
  Open sub-question: rigorously prove the linear→nonlinear bridge (engine
  reproduces linear-mode equipartition under genesis + Langevin + projection).
- **FTD-0096 μ-from-ℓ_P missing arrow** — mass-unit calibration; either
  closes or terminally demotes L2.
- **FTD-0106 G\*/π asymmetry** per-domain engine measurements
  (Domain A Langevin dissipation; Domain B Coulomb phase; Domain C BH evap)
  — pre-registered, theory-only catalog committed, engine measurements
  deferred.
- **L=128 G2 follow-up to FTD-0107** — locks L-invariance further.
- ~~**The structural bridge between algebraic spine and engine
  phenomenology**~~ — **CANDIDATE BRIDGE IDENTIFIED 2026-04-27 late evening**:
  the framework integer N_base = 4 connects O_h-cubic-point-group structure
  (algebraic) to cluster-efficiency coefficient ¼ (engine). Verified via two
  code paths: GPU campaign (T5b/T6/T7/T8, 5/5 amplitudes + 5 SM particles)
  and WASM dashboard (Poynting-vector asymmetry visual cross-check). See
  `FOUND_LATTICE_SPACING_GAUGE_FREEDOM.md` and `WHERE_WE_LEFT_OFF.md §10.6`.

**[NEW INFRASTRUCTURE 2026-04-27]:**
- Pre-registration discipline operationalized via SHA256 hash + git tags
  applied BEFORE measurement. Today's tags: `preregister-lemniscatic-v1`,
  `preregister-gstar-asymmetry-v1`, `preregister-emergent-spectrum-g1`,
  `preregister-look-elsewhere-scan-v1`. All measurements held the gate.
- `tools/scan_look_elsewhere.py` — deterministic look-elsewhere runner
  (FTD-0097, hash-locked).
- Engine extension: `--lemniscatic-mode` in `benchmark_black_hole_thermo.cpp`
  (FTD-0105); `--output-dir` in `campaign_emergent_spectrum_2026-04-27.cpp`
  (FTD-0107).

**Demoted 2026-04-19:** sin²θ_W (3.5%), sin²θ_13 (12.6%), α_s = 7/59,
PMNS angles — all now [PARAMETRIC] or [STRUCTURALLY MOTIVATED PARAMETRIC].

**Foundational commitment:** undefined-boundary lattice ontology (not
completed-infinity ℤ³). See `docs/theory/07_assessment/AUDIT_INFINITY_REFRAME.md`.

**The structural gap (2026-04-27 diagnosis + 2026-04-28 closure, see
WHERE_WE_LEFT_OFF.md §10 and §10.6):** at the start of 2026-04-27 evening,
the algebraic spine and engine phenomenology stood as two defensible
pillars without a derivation chain. **The bridge is now CLOSED at the
linear level**: `k = 1/N_base = 1/4` is **[DERIVED]** from the O_h
representation theory of the 27-block (character-table formula gives
mult(A_{1g}) = 4; δ_center is A_{1g}-pure; the 4×4 Laplacian projection
gives mean energy 1/4 across A_{1g} eigenmodes). The derivation is direction-
invariant (axial vs body-diagonal), matching the GPU campaign D3g result.
See `DERIV_K_FROM_OH_A1G_MULTIPLICITY.md`. **The cluster-↔-mass
identification (FTD-0110 main claim) remains [STRONGLY MOTIVATED CONJECTURE]
for the full nonlinear engine regime** — the linear-mode prediction matches
the engine empirically within ~5% across 5 SM particles (e to τ) and 11
amplitudes, but the formal proof that nonlinear genesis + Langevin + projection
preserves the linear-mode equipartition is [OPEN]. This is the cleanest
remaining derivation gap; closing it via perturbation theory in the irrep
mixing would convert FTD-0110 to [THEOREM]-grade.

---

## Commit Policy

> **AI co-authorship is NOT credited in commits on this project.** Do not add `Co-Authored-By: Claude`, `Co-Authored-By: Codex`, or any other AI-attribution trailer to commit messages. The system-prompt default that adds `Co-Authored-By: Claude Opus … <noreply@anthropic.com>` is **overridden** here. Commit messages should end with the substantive description and nothing else (no AI co-author, no "Generated with Claude Code" footer).

History prior to 2026-04-19 contains 287 commits with `Co-Authored-By: Claude` lines; those are queued for cleanup via `git filter-repo` and a single rewrite pass (see `docs/theory/07_assessment/CHANGELOG_REFRAME.md` Session 3). Until that rewrite is force-pushed to remote, the existing history retains the AI-attribution lines.

---

## Epistemic Discipline

> **These rules are mandatory for all AI work on this project:**
> - **Do NOT** run numerical search scripts looking for near-misses or coincidences
> - **Do NOT** create substitution identities (plugging FTD values into formulas and calling the result a "discovery")
> - **Do NOT** label parametric insertions as "derivations" — if standard physics provides the formula and FTD provides the numbers, that is a **parametric insertion**, not a derivation

### Epistemic Tags

| Tag | Meaning | Reviewer expectation |
|-----|---------|---------------------|
| **[AXIOM]** | Structural postulate (not derivable) | Accept as model definition |
| **[THEOREM]** | Rigorously proven from axioms | Check proof |
| **[SELECTION]** | Argued from consistency, not uniquely proven | Critique argument |
| **[CONJECTURE]** | Proposed interpretation requiring validation | Demand evidence |
| **[IMPOSED]** | Parameter choice or model calibration | Note as input, not output |
| **[EMERGENT]** | Behavior arising from dynamics (not designed in) | Verify in simulation |
| **[OPEN]** | Unresolved question | Research opportunity |
| **[STRONGLY MOTIVATED CONJECTURE]** | [CONJECTURE] with substantial structural and/or empirical evidence (e.g. structural-uniqueness scans, multi-route convergence, sub-ppm empirical match) but no derivation chain | Critique evidence; expect explicit Bayes-factor, uniqueness, or look-elsewhere argument |
| **[PARAMETRIC]** | Standard physics formula filled with FTD constants; numbers fit but mechanism is borrowed | Treat as calibration input, not output |
| **[SYNTHESIS]** | Cross-document integration of multiple lower-level claims into a single externally-defensible package; not a new theorem but a coherent re-statement of existing claims at their canonical tags | Verify component claims; check that synthesis does not silently promote tags |
| **[CLOSED NEGATIVE]** | Hypothesis was tested and falsified; preserved for provenance to prevent re-attempt | Confirm closure evidence; cite to prevent zombie re-emergence |
| **[DERIVED]** | Established from axioms or prior theorems by an explicit chain that the doc itself reproduces; weaker than [THEOREM] when the chain has non-trivial assumptions | Check the chain; flag any smuggled axioms |

---

## Documentation Cleanup Discipline

> **These rules are mandatory for AI cleanup work.** The goal is persistent consolidation, not one-off tidying that creates future drift.

- **Preserve provenance; move, do not erase.** Superseded, retracted, resolved, and closed-negative theory documents should be archived with `git mv`, not deleted, unless the user explicitly asks for deletion.
- **Keep active directories active.** Documents whose live status is `[CLOSED NEGATIVE]`, `[RETRACTED]`, or `[CLOSED -- RESOLVED]` should live under `docs/theory/archive/` or a local archive such as `docs/theory/10_eft_program/archive/{closed_negative,resolved,retracted}/`.
- **Track cleanup provenance deliberately.** Theory archives used as canonical cleanup provenance must be tracked in git. The top-level `archive/` directory remains ignored. Local cleanup archives such as `docs/theory/10_eft_program/archive/**` are tracked wholesale; the broad `docs/theory/archive/` directory uses explicit `.gitignore` exceptions, so add a matching exception whenever a new canonical top-level archived file is introduced.
- **Update all navigation layers in the same cleanup.** If a file is moved or status-changed, update the relevant index/tracker/spec references in the same commit: `docs/theory/META_INDEX.md`, local sub-indexes such as `docs/theory/10_eft_program/INDEX_FTD_NATIVE_EFT.md`, `docs/theory/07_assessment/LEDGER.md`, `TRACKER_OPEN_ITEMS.md`, and any project-level maps that link to the file.
- **Open trackers must contain open work.** Do not leave closed, retracted, or resolved items counted as `[OPEN]`. Either remove them from `TRACKER_OPEN_ITEMS.md`, move them to a resolved/provenance tracker, or clearly mark them as "not counted as open" until a resolved tracker exists.
- **Do not promote claims during cleanup.** Cleanup may clarify status, archive provenance, and align links; it must not upgrade epistemic tags or introduce new derivations without a separate audit.
- **Verify before committing.** At minimum run `git diff --check` and `rg` for old active paths after any move. Use documentation/link checks only; do not run numerical near-miss or coincidence searches as part of cleanup.
- **Commit cleanup in small coherent batches.** Prefer one commit per cleanup theme (archive tracking, tracker split, index reconciliation, sector consolidation) so future agents can audit the history.

---

## What FTD Is

A discrete computational framework for simulating physical systems from explicit postulates. The model postulates a 3D cubic lattice where each site ("voxel") occupies one of three states: void (0), positive (+1), or negative (−1). Dynamics proceed via local update rules within a 26-connected Moore neighborhood, with information propagating at maximum one lattice unit per discrete time step.

**Two-layer ontology:**
- **Flux field** J ∈ ℝ³ — continuous vector field encoding potential energy density (dispositional)
- **State field** s ∈ {−1, 0, +1} — discrete ternary states representing manifestation (actual)

**Five postulates:** Discrete space (3D cubic lattice, no defined boundary — at every specified position, axis-adjacent sites exist; **not** a completed-infinity ℤ³ totality, per `AUDIT_INFINITY_REFRAME.md`), discrete time (ticks), ternary states, local causality (26-neighbor Moore), determinism.

**Foundational commitment (2026-04-19):** FTD uses **undefined-boundary** lattice ontology, not completed-infinity. Arbitrarily large finite computations are permitted; claims of the form "in the L → ∞ limit" are not well-posed without explicit ε-L restatement. See `docs/theory/07_assessment/AUDIT_INFINITY_REFRAME.md` for the full triage of which claims survive, which need restatement, and which need re-derivation.

**Key results** (within framework assumptions):
- Fine structure constant: master quadratic `x² − 16G*²x + 16G*³ = 0` has x₊ = 137.036 matching 1/α to **1.26 ppm** at tree level (pure algebra [THEOREM]; physical identification [STRONGLY MOTIVATED CONJECTURE] per `AUDIT_MASTER_QUADRATIC.md`). Same polynomial gives x₋ = 3.024 ≈ N_c = 3 — the dual-prediction property is the strongest structural evidence. The 7-term series matching CODATA to 24 digits is a post-hoc fit [CONJECTURE] beyond experimental precision (CODATA 2022 has ~11 digits), not a "< 0.001 ppt derivation"
- Loop coefficients c1–c3 derived from lattice Feynman diagrams: c1 = 9/47 (0.8%), c2 = 5/64 via gauge factor 13/9 (0.07%), c3 = 4/141 via gauge factor 11/6 (0.33%)
- Electron mass m_e = m_P √(2π) (16/3) α¹¹ (0.19% error)
- Higgs mass m_H = (N_eff/α²)·m_e = 124.8 GeV (0.24% error), λ_H = m_H²/(2v²)
- Proton mass m_p/m_e = N_eff/α + N_base·N_eff + N_c = 1836.47 (174 ppm)
- Electron g-2: a_e = α/(2π) to 5-loop = 2.55 ppb
- Lamb shift: 1055.4 MHz (0.23% from experiment)
- Color charge number N_c = 3 from RG flow + topological quantization
- **Moore Layer Theorem**: gauge groups U(1)×SU(2)×SU(3), 3 generations of 4 fermions, matter-antimatter symmetry, 17 dark states — all from Moore neighborhood polyhedral decomposition (octahedron + cuboctahedron + stella octangula)
- BCC multiplicative structure: Watson identity W₃ = G*²/(2π) and SU(3) gauge group both arise from the BCC eigenvalue's triple cosine product (docs/theory/08_structural/DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md)
- Confinement from area-law Wilson loops at x₋ (σ = 0.209)
- Bell violation S = 2√2 [SELECTION] resolved as emergent from QM lattice emergence (Tsirelson's bound; April 2026)
- Full nonlinear Einstein equations via Deser iterative bootstrap — **[2026-05-21 Step-0 correction, FTD-0189]** the bootstrap *completes* a posited massless spin-2 field, it does not derive one; its linearized-EFE input is conditional on Conjecture 10.1 (h_μν posited, not substrate-constructed; spin-2 spatial part is Gap 10.1). Whether the substrate carries an emergent spin-2 mode is [OPEN] — Frontier 4
- D = 3 uniquely selected (no longer axiomatic)
- Cyclotomic structure: Hamiltonian parameters are Phi_4, Phi_1·Phi_2, Phi_6 evaluated at sqrt(pi)
- The Ratio and the Arrow: Euler reflection product (commutative, gives pi, time-symmetric) vs ratio (non-commutative, gives G*, time-asymmetric)
- 50 physics predictions tested across three tiers: `scripts/exploration/test_all_physics.py`
- Complete Standard Model computation: `scripts/proofs/proof_complete_sm.py`

**Honest accounting:** ~50 predictions tested (20 structural theorems, 20 G*-derived, 10 novel cube predictions), ~50 parametric insertions (FTD values in standard QFT formulas), ~50+ external physics adopted. Manuscript v2: 83 chapters (26 new + 57 editorial pass). April 11 audit: 267/267 Python tests pass, 54/54 master verification pass, 3 META_INDEX overclaims fixed. See [EPISTEMIC_AUDIT.md](docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md).

**Engine-theory bridge (April 13, 2026):** 20-benchmark suite connects engine output to theory. Coulomb 1/r^2 converges (B+), hydrogen 1/n^2 < 0.001% (A+), color forces correct (A+), Higgs threshold exact (A+), Bell S=2.000 (A+), Born lattice bias 10x (A-). EFT reconstruction: alpha = G_C^2 (derived, not input). Added Wilson loops (12/17, flux tube detected), gluon dynamics (7/11, linear E(r)), budget equation (0.2% at r=6). LATENCY FIX unlocked GR: time dilation 0.004% match, BH gravitational wells L_peak=0.62. Three theorem papers: continuum limit -> QED, singlet from void event, N_c from topology. WASM rebuilt and deployed. 148/166 CTest passing. Scientific status: C+ -> B+.

---

## Project Structure

```
ftd/                                     # Project root
├── docs/
│   ├── SPEC_FTD.md              # Framework overview (defers to the canonical hierarchy in META_STRUCTURE.md)
│   ├── theory/                   # ~349 active theory documents (10 categories, each with a local INDEX_ file) + local archive subdirs
│   │   ├── META_INDEX.md         # Complete catalog
│   │   ├── 01_reference/         # Master references and proofs
│   │   ├── 02_foundations/       # Ontological emergence
│   │   ├── 03_derivations/       # Core physics derivations
│   │   ├── 04_coupling/          # Coupling constants
│   │   ├── 05_particles/         # Particle physics
│   │   ├── 06_consciousness/     # Reflexive-projection layer + measurement (formerly framed as "consciousness"; directory name preserved for backward compatibility, see REF_REFLEXIVITY_VOCABULARY.md)
│   │   ├── 07_assessment/        # Epistemic audits
│   │   ├── 08_structural/        # Geometry and information theory
│   │   ├── 09_mathematical/      # Number theory and connections
│   │   └── archive/              # Superseded/historical documents
│   ├── reference/                # REF_EPISTEMIC_LABELS, REF_SYMBOL_GLOSSARY, etc.
│   ├── papers/                   # Published/submitted PDFs and TeX sources
│   └── internal/                 # Session summaries, exploration scripts
├── engine/                       # C++ simulation engine (v2.14)
│   ├── SPEC_ENGINE.md            # Engine reference document
│   ├── include/ftd/              # 29 headers (ontic.h, voxel.h, lattice.h, scenarios.h, etc.)
│   ├── src/                      # 14 source files
│   ├── tests/                    # 169+ test files (120 unit + 49 campaign + 4 GPU)
│   ├── cuda/                     # GPU acceleration
│   ├── wasm/                     # Emscripten bindings
│   └── web/                      # Browser dashboard (Three.js, modular JS)
├── scripts/                      # ALL Python scripts (~149 scripts)
│   ├── constants.py              # Canonical shared constants (single source of truth)
│   ├── verification/             # Formal derivation verification (40 scripts)
│   ├── proofs/                   # Formal mathematical proofs with error bounds (57 scripts)
│   ├── experiments/              # Bell tests, CERN analysis, physics sims (17 scripts)
│   ├── exploration/              # Focused research investigations (25+ scripts)
│   ├── tests/                    # Python test suites — pytest (11 scripts)
│   │   └── comprehensive/        # 7-tier verification framework
│   ├── visualization/            # Publication figure generation (11 scripts)
│   └── runners/                  # Test protocol runners (2 scripts)
├── evaluation/                   # Multi-domain assessment & certification
├── dissemination/                # All publication/outreach content
│   ├── manuscript/               # 96-chapter Quarto book (src/ + media/images/)
│   ├── manuscript_v2/            # 83-chapter physicist-targeted rewrite
│   ├── whitepaper/               # LaTeX whitepaper + figures
│   ├── book/                     # "The Golden Thread" narrative (53 .qmd files)
│   ├── notebooks/                # 12 Jupyter pedagogy notebooks
│   └── interactive/              # 8+ standalone HTML simulations (forces, photon, Hamiltonian bridge, ternary cube)
├── models/                       # Physics derivation package (gitignored)
├── archive/                      # Curated historical record (gitignored; see docs/theory/archive/ for archived theory docs)
├── META_DOCUMENTATION_MAP.md     # Master catalog / card catalog
└── META_PROJECT_ATLAS.md         # AI agent navigation guide
```

---

## C++ Engine

**Build**: `cmake -S engine -B engine/build && cmake --build engine/build --config Release`
**Test**: `cd engine/build && ctest --output-on-failure -C Release`
**WASM**: `engine\build_wasm.bat` (Windows wrapper; runs emcmake/emmake + deploys to `engine/web/wasm/`). Manual: `emcmake cmake -S engine -B engine/build_wasm -DCMAKE_BUILD_TYPE=Release && emmake cmake --build engine/build_wasm --target ftd_wasm`
**Web UI**: `python engine/web/serve.py 8080` (no-cache dev server — emits `Cache-Control: no-store` on every response so JS edits hit the browser without manual hard-refresh). Plain fallback: `python -m http.server 8080 -d engine/web` (caches aggressively; expect to bounce + hard-refresh after edits).

### Key Constants (all derived from D=3 + varpi via `ontic.h`)

| Constant | Value | Origin |
|----------|-------|--------|
| G* (lemniscatic) | 2.95868... | Γ(1/4)/Γ(3/4) |
| α (fine structure) | 1/137.036 | Master quadratic x₊ |
| N_c (colors) | 3 | Master quadratic x₋ |
| K_B (manifestation) | 0.511 | m_e = m_P·√(2π)·(16/3)·α¹¹ (current calibration: K_B = m_e mass anchor; role-conflated with engine manifestation threshold — see FTD-0130) |
| C_SPEED | 1/√3 | CFL stability on cubic lattice |
| G_N (gravity) | 0.01 | 1/(b₃+N_c)² — **falsified as identification with physical G_N** (FTD-0131); substrate derivation gives instead the gravitational fine-structure ratio for one electron: α_G(e,e) = (m_e/m_P)² = (√(2π)·(16/3)·α¹¹)² ≈ 1.745×10⁻⁴⁵ (predicted, 0.38% match to measured 1.752×10⁻⁴⁵) — derived via Phase G + FTD-0015 + 2 flagged postulates; see `docs/theory/03_derivations/DERIV_NEWTON_FROM_SUBSTRATE.md`. The "1/100" numerical coincidence has no substrate justification under any natural reading. |

### Engine Philosophy

Logic-first: only 6 rules derived from axioms. All phenomenological features are toggle-gated extensions (default OFF).

**Tick cycle:** phase_read → phase_write → gauss_project → phase_forces → phase_movement → tick++

---

## Key Navigation Documents

- **Full FTD spec**: `docs/SPEC_FTD.md`
- **External comparison constants (canonical edition standard, 2026-05-19)**: `docs/reference/REF_EXTERNAL_CONSTANTS.md` — the single source for which CODATA/PDG edition every externally-measured comparison value uses (current standard: **CODATA 2022 / PDG 2024**). All future references to α⁻¹, G, ℓ_P, particle masses, etc. cite this; machine-readable mirror is `scripts/constants.py` `Experimental`. Pre-registered/hash-locked artifacts legitimately retain registration-time values (provenance, not drift).
- **🧭 Doctrine ledger v1.2 (single-page status map, 2026-05-08, FTD-0145 [SYNTHESIS])**: `docs/theory/01_reference/SPEC_DOCTRINE_LEDGER.md` — 14 sections + non-circularity audit + compressed roadmap. Roll-up across LEDGER + TRACKER_ONTIC_TRUTH + SPEC_ALGEBRAIC_SPINE + SPEC_FQCR + CHECKLIST_MATH_COMPLETE. **Introduces no new theorems**; every claim points at a canonical source. v1.2-to-canonical tag map in §0.2. Read BEFORE planning hardening arcs or answering "what is the status of claim X" questions.
- **Ontic-truth tracker (read this FIRST before defending any FTD math claim, 2026-05-02)**: `docs/theory/07_assessment/TRACKER_ONTIC_TRUTH.md` — the single canonical bedrock reference. 5 truth tiers (T1 rock-solid → T5 conjecture); each entry has unique ID `OT-N.M` and points at a verification artifact. The 10 entries that matter are listed at the bottom under "Quick reference". If LEDGER and this tracker disagree on a tier, this tracker is correct on tier-assignment. Anything below T5 (parametric insertions, selection arguments, engine measurements) is NOT in this tracker — it lives in LEDGER and CATALOG_PARAMETRIC_INSERTIONS.
- **Algebraic spine (canonical theorems-only reference, 2026-04-27)**: `docs/theory/01_reference/SPEC_ALGEBRAIC_SPINE.md` — citation target for paper drafts; states nine numbered results (G* identity, master quadratic, CM uniqueness h=1, coefficient 16, Watson identity, Phase G geometric Coulomb, Phase J ultralocality at L=2, harmonic invariant tower, Q(G*) field-theoretic) of which six are theorem-grade and three are honestly tiered below theorem grade per §0, independent of any physics interpretation. Theorem 7 honest status: [THEOREM at L=2] + [CONJECTURE for general L] (audit 2026-05-01). Read this before claiming anything load-bearing about FTD's algebraic content.
- **Open math by physics sector (2026-05-08, FTD-0146 [SYNTHESIS])**: `docs/theory/01_reference/SPEC_OPEN_MATH_BY_SECTOR.md` — sector-aligned canonical research-questions queue, 10 SM-sectors (pure-math / EM-α / EW-Higgs / QCD / flavor / gravity / QM-foundations / cosmology / engine-bridge / cross-cutting). Replaces tier-aligned `CHECKLIST_MATH_COMPLETE.md` (now archived to `docs/theory/archive/ARCH_CHECKLIST_MATH_COMPLETE.md` for provenance). Preserves effort codes (D/W/M/RP/FO) + dependency graph + foundational-obstruction framing. Tier I + II closed (8/8); Tier III 1/5 closed + 3/5 investigated; MC-T4.3 (non-action α-injection mechanism) is the central foundational obstruction.
- **Reflexivity vocabulary** (2026-05-01; sweep applied 2026-05-02): `docs/theory/01_reference/REF_REFLEXIVITY_VOCABULARY.md` — canonical replacement for "consciousness" terminology. Two-term core (reflexivity = structural; agency = dynamical). Drops qualia commitments without losing conceptual content. P1-P4 sweep applied across theory + manuscript + whitepaper. Cite this before applying vocabulary in new docs.
- **Chowla–Selberg h≥2 theory note** (2026-05-02): `docs/theory/09_mathematical/EXPLR_CHOWLA_SELBERG_HIGHER_H.md` — analytic-machinery list for upgrading Theorem 3 from [NUMERICAL FACT, h=1 only] to a structural theorem covering all CM curves. Closes MC-T2.3.
- **Tier-I/II/III closure proof scripts** (2026-05-02; under `scripts/proofs/`):
  - `proof_field_theoretic_qgstar.py` — FTD-0112 / Theorem 9 (T1.3)
  - `proof_per_voxel_mass_gap.py` — FTD-0044 / per-voxel mass gap (T1.4)
  - `proof_phase_j_general_L.py` — Theorem 7 investigation (T1.1)
  - `proof_m_e_exponent_n11.py` — m_e exponent n=11 derivation (T3.2)
  - `proof_scfcc_bcc_bridge.py` — (SC+FCC)/2 ↔ BCC investigation (T3.3)
  - `proof_ftd0110_mechanism_gamma.py` — Mechanism γ investigation (T3.1)
  - `proof_bridge_functional_arithmetic_mean.py` — four-mean investigation (T3.4)
  - `proof_polynomial_look_elsewhere_extended.py` — extended scan with pre-registration (T2.1+T2.2)
  - `proof_a1g_dual4_via_zi_units.py` — Z[i]^× structural argument (T1.5+T4.5; superseded 2026-05-02 afternoon by `proof_bcc_complex_structure.py`)
  - `proof_bcc_complex_structure.py` — BCC complex-structure theorem (FTD-0122; T4.5 Roles 1+3 [DERIVED], Roles 2+4 [NO-GO])
- **Dimensionless ↔ Dimensional Map** (2026-04-29): `docs/theory/01_reference/SPEC_DIMENSIONAL_MAP.md` (rendered) + `docs/theory/01_reference/dimensional_map.json` (canonical data, 15 entries). Single citation target for "is this prediction dimensionless or calibration-conditional?". Walks the bridge from the 7 algebraic-spine theorems through the 4 dimensionless physical predictions (α, N_c, m_μ/m_e, m_τ/m_e) through the 3 calibration declarations theorem-enforced by FTD-0059 + FTD-0096 (`a_phys ≡ ℓ_P`, `t_phys`, `K_B = m_e`) to one worked dimensional application (m_e in MeV). Renderer: `scripts/proofs/build_dimensional_map.py`. Tests (12 assertions): `scripts/tests/test_dimensional_map.py`.
- **Engine spec**: `engine/SPEC_ENGINE.md`
- **Theory catalog**: `docs/theory/META_INDEX.md`
- **Documentation map**: `META_DOCUMENTATION_MAP.md`
- **Epistemic audit**: `docs/theory/07_assessment/AUDIT_EPISTEMIC_AUDIT.md`
- **Parametric insertions catalog** (April 19, 2026): `docs/theory/07_assessment/CATALOG_PARAMETRIC_INSERTIONS.md` (~162 rows enumerated: ~23 [DERIVED]/[THEOREM], ~129 [PARAMETRIC], ~10 [IMPOSED]/[SELECTION])
- **EFT Recovery Program** (April 19, 2026, COMPLETE Phase 0 → F + Phase G reframe): `docs/theory/10_eft_program/SPEC_EFT_RECOVERY_PROGRAM.md` — pre-registered 7-phase campaign. **Phase-F measurement:** a lattice-α plateau at ~1.8× α_ref (classical convention; 3.6× under engine-internal energy convention) across L ∈ {64, 128, 256, 384} GPU scan. **Phase-G reframe (2026-04-19):** the plateau is the zero-free-parameter periodic lattice Poisson Green's function `α_r(r, L) = 2 · r · G_L(r)`; R² = 1.0000 at L=384, median 0.07% residual in the Coulomb tail. **This is not a QED deviation** — it is lattice geometry with zero fine-structure content. See `AUDIT_ALPHA_EXTRACTION.md` and `DERIV_EMERGENT_COULOMB_GEOMETRIC.md`. Day-2 interim "1.23×" claim **RETRACTED** (ticks=100 under-equilibrated). Day-2 shipped: matched-stencil CG Poisson (Ward floor 1% → 1e-8), EWSB sharp first-order transition at amp ∈ (0.6, 0.7), condensate m ≈ 0.18 (flux/charge channels agree 3%), Rutherford α = 0.042 ± 0.005 independent cross-check. WSL2 + CUDA 13 path unblocks RTX 5090 (30× speedup). Pipeline<Backend> architecture with CPU/GPU parity. Paper: `dissemination/papers/PAPER_FTD_AS_WILSONIAN_EFT.tex`. Day-2 doc: `docs/theory/10_eft_program/archive/phase_0_f_campaign/DERIV_DAY2_CAMPAIGN.md`. Plan: `C:\Users\cpaci\.claude\plans\vivid-marinating-pudding.md`
- **Engine callstack audit**: `docs/theory/07_assessment/AUDIT_ENGINE_CALLSTACK.md` (CPU/GPU parity, toggle gaps, 10 findings)
- **Open items tracker**: `docs/theory/07_assessment/TRACKER_OPEN_ITEMS.md` (every `[OPEN]` across code + theory, one place)
- **Infinity reframe** (April 19, 2026): `docs/theory/07_assessment/AUDIT_INFINITY_REFRAME.md` — undefined-boundary ontology triage; foundational replacement for completed-infinity ℤ³ framing
- **a_phys open problem** (April 19, 2026): `docs/theory/10_eft_program/archive/resolved/OPEN_A_PHYS_DERIVATION.md` — load-bearing problem the reframe creates: derive `a_phys` (lattice→physical length) from Axiom-Zero invariants or declare it empirical. Three derivation candidates analysed
- **Mechanism γ attempt** (April 19, 2026): `docs/theory/10_eft_program/archive/closed_negative/DERIV_A_PHYS_MECHANISM_GAMMA_ATTEMPT.md` — gravitational `a_phys` derivation attempted and **closed as candidate** (negative result; recommendation: declare `a_phys ≡ ℓ_P` in `SPEC_FTD.md`)
- **Master quadratic (rewritten)** (April 19, 2026): `docs/theory/03_derivations/DERIV_MASTER_QUADRATIC_GAP_EQUATION.md` — full rewrite as algebraic identity + physical match (CM-curve uniqueness + dual match); gap-equation/thermodynamic-limit narrative withdrawn
- **Reframe deployment package** (April 19, 2026): `docs/theory/07_assessment/reframe_deployment/` — `CANONICAL_REFRAME.md` (single source of truth for what the reframe means; agent-facing) + `DEPLOYMENT_GUIDE.md` (7-phase plan) + `agents/` (9 agent prompts) + `templates/` + `checklists/`. Read CANONICAL_REFRAME.md before any reframe-related work
- **Master claim ledger** (April 19, 2026; extended April 20): `docs/theory/07_assessment/LEDGER.md` — 52 load-bearing claims with tag history, dependencies, reframe status. **Single source of truth for claim status** — papers cite tags from here; if they disagree, the ledger wins. Rows FTD-0050/0051/0052 (Link 8 closure + Langevin infrastructure + deferred s-Metropolis) added 2026-04-20.
- **Link 8 closure audit** (April 20, 2026): `docs/theory/10_eft_program/archive/closed_negative/AUDIT_LINK8_CLOSURE.md` — full closure report on "master quadratic as RG-step characteristic polynomial" hypothesis. Three independent tests (Kadanoff blocking, Watson-integral analytical, thermalized |J|² correlator) all NEGATIVE for structurally consistent reasons (engine stencil is (SC+FCC)/2, BCC-orthogonal; master quadratic lives on BCC Watson integral). FTD-0001/0013/0014 UNAFFECTED.
- **Langevin thermostat** (April 20, 2026): `engine/src/render_bridge.cpp` + `engine/include/ftd/term_toggles.h` — CPU single-substrate OU update on wave_vel, toggle-gated. Validated by `engine/tests/test_langevin_equipartition.cpp` (equipartition to 4%). Unblocks non-zero-T matched-stencil β, condensate ensemble measurements, fluctuation-dissipation tests.
- **Reframe changelog** (April 19, 2026, append-only): `docs/theory/07_assessment/CHANGELOG_REFRAME.md` — every decision and change made under the reframe deployment
- **Devil's advocate report** (April 19, 2026): `docs/theory/07_assessment/archive_session_outputs/DEVILS_ADVOCATE_REPORT.md` — falsification pass on 6 substantive rewrites; 3 blocking bugs found and fixed same-day, 5 PASS-WITH-NOTES queued
- **Engine reframe audit** (April 19, 2026): `docs/theory/07_assessment/archive_session_outputs/ENGINE_AUDIT_REFRAME.md` — C++/CUDA/JS sweep for completed-infinity + hidden α; 3 HIGH (2 fixed same-day, 1 deferred for owner: `α_inf` rename across CSV/Python/TeX), 6 MEDIUM, 9 LOW. Parameter-free claim status: CONDITIONAL
- **Portfolio inventory** (April 19, 2026): `docs/theory/07_assessment/archive_session_outputs/INVENTORY_PORTFOLIO.md` — 280 artifacts cataloged outside `docs/theory/`; 267 editable, 13 PDF-only; manuscript_v1↔v2 share ~57 chapters that must be propagated together
- **Paper classification** (April 19, 2026): `docs/theory/07_assessment/archive_session_outputs/FLAGGED_PASSAGES_PAPERS.md` — 34 TeX/MD papers in `docs/papers/` classified; 10 clean, ~37 proscribed passages in 7 files. Top-7 priority list inside
- **YM/NS RE-DERIVE assessment** (April 19, 2026): `docs/theory/07_assessment/archive_session_outputs/REDERIVE_REPORT_YM_NS.md` — both speculative Clay-aimed papers lose post-reframe; YM has 1 surviving theorem (per-voxel mass gap), NS has none; SPLIT/DEMOTE/RETRACT options laid out per paper
- **Manuscript propagation rule** (April 19, 2026): `dissemination/manuscript_v2/PROPAGATION_RULE.md` — authoritative rule for v1↔v2↔vol1↔vol2 chapter editing. **Mandatory before any chapter edit** (vol1/vol2 are NOT symlinks; already diverged)
- **a_phys ≡ ℓ_P calibration** (April 19, 2026, declared in SPEC_FTD.md): one voxel ≡ Planck length; one tick ≡ √3·ℓ_P/c ≈ 9.34×10⁻⁴⁴ s; mass-unit ≡ m_e/K_B = 1 MeV/c². **Every dimensional FTD prediction is conditional on this calibration; dimensionless predictions (α, mass ratios, mixing angles) are calibration-independent and constitute the falsifiable spine.**
- **Changelog (engine + project)**: `CHANGELOG.md`
- **Complete SM**: `scripts/proofs/proof_complete_sm.py`
- **Motivic proof**: `scripts/proofs/proof_motivic_master_quadratic.py`
- **Moore Layer Theorem**: `docs/theory/08_structural/THEOREM_MOORE_LAYER_DECOMPOSITION.md`
- **Phase lattice**: `docs/theory/08_structural/EXPLR_PHASE_LATTICE_MOORE.md`
- **50-test battery**: `scripts/exploration/test_all_physics.py`
- **Loop derivations**: `scripts/exploration/compute_c2.py`, `derive_all_loops.py`, `gauge_loops.py`
- **Arrow paper**: `docs/papers/PAPER_RATIO_AND_THE_ARROW.tex`
- **Engine coupling test**: `engine/tests/test_intervoxel_coupling.cpp`
- **Complete Chain** (April 2026): `docs/theory/01_reference/SPEC_FTD_COMPLETE_CHAIN.md`
- **QM as Statistics** (April 2026): `docs/theory/03_derivations/DERIV_QM_FROM_LATTICE.md`
- **Lattice Physics Reference** (April 2026): `docs/theory/02_foundations/FOUND_LATTICE_PHYSICS_INTUITIONS.md`
- **Stellar Lifecycle** (April 2026): `docs/theory/03_derivations/DERIV_STELLAR_LIFECYCLE_LATTICE.md`
- **Master Verification** (April 2026): `scripts/proofs/proof_master_verification.py` (54/54 checks)
- **BCC Unification** (April 2026): `docs/theory/08_structural/DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md`
- **Observer Formalism** (April 2026): `docs/theory/02_foundations/FOUND_POTENTIAL_CORE_AND_GENERATIVE_INTERIOR.md` (Part II: 3³ lattice grounding)
- **Manuscript v2**: `dissemination/manuscript_v2/CHECKLIST.md` (83 chapters, physicist-targeted)
- **Engine-Theory Bridge** (April 13, 2026): `engine/tests/benchmark_engine_theory.cpp` (20 benchmarks)
- **Emergent Alpha** (April 13, 2026): `engine/tests/benchmark_emergent_alpha.cpp` (6 EFT experiments)
- **Benchmark Harness**: `scripts/benchmarks/benchmark_engine_vs_theory.py` (Python analysis)
- **Convergence Analysis**: `scripts/benchmarks/analyze_convergence.py` (20-benchmark report + plots)
- **Benchmark Results**: `scripts/benchmarks/results/` (reports, plots, CSV)
- **Wilson Loops** (April 13, 2026): `engine/tests/benchmark_wilson_loops.cpp` (12/17 pass, flux tube detected)
- **Gluon Dynamics** (April 13, 2026): `engine/tests/campaign_gluon_dynamics.cpp` (7/11 pass, linear E(r))
- **Einstein Equations** (April 13, 2026): `engine/tests/test_einstein_equations.cpp` (time dilation 0.004% match after latency fix)
- **BH Thermodynamics** (April 13, 2026): `engine/tests/benchmark_black_hole_thermo.cpp` (L_peak 0.62, proper time dilation)
- **Budget Equation** (April 13, 2026): `engine/tests/benchmark_budget_equation.cpp` (x/K+G*/x=1 to 0.2%)
- **Continuum Limit -> QED** (April 13, 2026): `docs/theory/03_derivations/DERIV_CONTINUUM_LIMIT_QED_EQUIVALENCE.md` (x+ = 1/alpha conditional [THEOREM])
- **Singlet from Void** (April 13, 2026): `docs/theory/03_derivations/DERIV_SINGLET_FROM_VOID_EVENT.md` (Bell loop closed via 5 lemmas)
- **N_c from Topology** (April 13, 2026): `docs/theory/03_derivations/DERIV_NC_FROM_TOPOLOGY.md` (N_c = 3 from 4 independent routes)
- **Web refactor spec** (April 18-19, 2026): `engine/web/docs/SPEC_REFACTOR_LARGE_FILES.md` (Waves 0-3 split of viewport/wasm-bridge-dag/app_dag + RF-1/3/4/5/6/7/8/10 post-audit cleanup; Ticket 14 + RF-9-full deferred). Final: viewport 5325→3900, wasm-bridge-dag 5736→2132, app_dag 1898→1723 (−5204 LOC, −40% across three files)
- **Whole-project extraction** (April 19, 2026, v2.15.0): 16-agent parallel refactor split every file ≥500 LOC into discrete-responsibility modules. C++: render_bridge.cpp 2139→1097, constructors.cpp 1245→0 (deleted, 5 split files), scenarios.cpp 1241→79, ftd_wasm.cpp 1224→607, cosmic_engine 1193→500, atom_engine 1029→325, main.cpp 938→74, ontic.h 806→45 (+6 theme-headers), ws_server 831→496. JS: mock-scale5 1903→313, consciousness triad −1711, scale controllers −1248, backgrounds 846→178, field-overlays 976→455. Python: 3 common helpers extracted. Total: ~13800 LOC redistributed across ~97 new files, every module nameable in ≤5 words
- **Scenario library (C++)** (April 18, 2026): `engine/include/ftd/scenarios.h` + `engine/src/scenarios/{flux,light,quantum,s0_seed,s0_field}.cpp` (all 83 Scale-0 scenarios ported from JS MockBridge; 84/84 Playwright coverage, 5/5 parity CI guard)
- **Web power-user guide** (April 18-19, 2026): `engine/web/docs/USER_GUIDE.md` (15-section reference for dashboard + console workflows)
- **Scenario parity CI guard** (April 19, 2026): `engine/web/tests/scenario-parity.spec.js` (5 assertions covering JS↔C++ scenario name drift; runs in <1s)
- **Viewport extracted modules** (April 19, 2026): `engine/web/js/viewport/{color-ramps,molecular-renderer,boundary-geometry,topology-sheet-renderer}.js` (own their Three.js concerns; viewport.js keeps thin delegators)

---

## EFT Reconstruction (April 13, 2026)

Alpha is now a DERIVED quantity in the engine:
- `ALPHA_EFT = G_C * G_C` defined in `constants.h` with compile-time `static_assert`
- G_C (wave equation coupling) is the fundamental lattice parameter
- All force computations use `ALPHA_EFT` (= G_C²), not hardcoded `ALPHA`
- New toggle `emergent_forces` computes force from flux gradient without Poisson solver
- 20-benchmark suite validates: Coulomb convergence (B+), hydrogen spectrum (A+), color forces (A+), Higgs threshold (A+), Bell S=2 (A+), Born lattice bias (A-), 139/179 CTest passing

---

## Naming Conventions

- Markdown files: `UPPER_SNAKE_CASE` with semantic prefix
- Prefixes: `SPEC_` (specifications), `DERIV_` (derivations), `FOUND_` (foundations), `AUDIT_` (assessment), `EXPLR_` (exploratory), `REF_` (reference), `ARCH_` (archived), `META_` (meta-documentation)
- Engine: C++17, snake_case functions, CamelCase types

---

## Environment Notes

- Platform: Windows 11. No `rsync` — use `cp -r` for directory copies
- **GPU execution MUST go through WSL2 Ubuntu-22.04, not Windows-native CUDA.** RTX 5090 speedup (~30×) is only available via the WSL2 build at `engine/build_wsl`. Windows-native CUDA builds from `engine/build/` technically run but are pathologically slow (observed 19 minutes wall for a single L=64 density=0.1 seed). Invocation pattern:
  ```
  wsl.exe -d Ubuntu-22.04 -- bash -c "cd /mnt/c/Users/cpaci/Desktop/ftd && \
      engine/build_wsl/benchmark_foo --args"
  ```
  Windows-native CUDA is acceptable for compile-time checks and single-tick correctness tests only. Any measurement campaign, sweep, or multi-seed run goes through WSL2.
- Python tests: `scripts/tests/` (pytest). C++ tests: `engine/tests/` (CTest). No overlap between them
- `scripts/constants.py` is the canonical shared constants module imported by 20+ scripts
- Build `.bat` files live in `engine/` — use `vswhere.exe` for portable VS detection
- `dissemination/media/`, `models/`, and `archive/` are gitignored — they exist on disk but not in git
- `docs/internal/` is gitignored — session summaries and explorations are local-only
