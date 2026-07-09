# FOUND — The modulus/argument frontier: what a discrete deterministic substrate can and cannot ground

**Tag:** `[SYNTHESIS]` (consolidation of established FTD results — promotes nothing) **+ `[CONJECTURE]`/`[SPECULATION]`** (the substrate-independent meta-claim of §3, a precise statement to attempt, **not** a theorem).
**LEDGER id:** FTD-0336 · **Deepens:** `EXPLR_GENESIS_COKERNEL_GRADED_SQRT.md`
**Period-conjecture frame:** [`MATH_PERIOD_IMPORT_FRONTIER.md`](../09_mathematical/number_theory/MATH_PERIOD_IMPORT_FRONTIER.md) (FTD-0375) restates this frontier period-conjecture-relative and records the sharpening that this doc's "exact, not analogical" even/odd (Euler-reflection parity) split is a **different** object from a claimed strict-period-ring decomposition of period integrals — the latter refuted externally (2026-07-09), the parity split unaffected.
**Purpose:** the canonical internal statement of FTD's boundary — *and* the skeleton of a clean external structural-negative paper (Number-One-Goal clause 2 at maximum generality). It replaces the slogan "FTD is missing a second `i`" (a category error, §4) with a precise, falsifiable frontier.

---

## 0 · Thesis in one line

> A finite, discrete, deterministic, forward-only substrate **owns exactly the MODULUS** (even / self-adjoint / tracial / forced-magnitude) half of mathematical physics, and **provably cannot self-supply the ARGUMENT** (odd / asymmetric / non-tracial / branch-selecting) half — *the chosen adjoint of its own lossy forward map.* FTD's open boundaries are **five instances of this one fact.**

This is **not** the claim that FTD lacks a second imaginary unit. The substrate *has* `i` (it is a native generator, FTD-0244) and still cannot reach the surd that distinguishes the master-quadratic roots. The missing structure is one rung deeper and entirely standard: the **chosen adjoint** (equivalently: a section, a disintegrating measure, a non-tracial state) of a many-to-one map — multivalued exactly to the degree the forward map destroyed information.

---

## 1 · The substrate class `S` (definitions)

Let `S` be the class of ontologies with: **(i)** a discrete state space; **(ii)** a deterministic, local update rule; **(iii)** finiteness / undefined boundary (no completed infinity, no primitive continuum); **(iv)** a **forward, possibly many-to-one (information-destroying) dynamics with no native backward pairing.** FTD's five postulates instantiate `S`; the cellular-automaton-class ontologies are its natural members.

The load-bearing feature is **(iv)**: such a substrate has a *forward map* and a *magnitude* (its own metric/modulus), but no native operation that inverts the forward map *across its own kernel*.

---

## 2 · The modulus / argument split (in standard mathematics, not metaphor)

| | **MODULUS half** (owned) | **ARGUMENT half** (not self-supplyable) |
|---|---|---|
| algebraic face | even; `Γ(z)Γ(1−z)=π` (the reflection **product**) — its first-order flow law is self-closing: coefficient `−π·cot(πz)` satisfies its own algebraic ODE `c′ = π² + c²` (FTD-0367) | odd; `Γ(z)/Γ(1−z)=G*` (the reflection **ratio**) — its flow coefficient `ψ(z)+ψ(1−z)` is hypertranscendental (Hölder 1887): the argument-half's flow law is unwritable in any differentially-algebraic world (FTD-0367) |
| ensemble face `[grounded]` (FTD-0366) | contour-independent data: the loop equations (Ward identities); the χ₋₄-symmetric conjugate combination — amplitude **product** `= √2·π` (N=1) — and, separately, the even sector's `ℚ·π^{N/2}` class (a=2 lies outside χ₋₄'s support) | the **contour choice** `C_{r,a}`, left underdetermined by the model's own Ward identities ((N+1)(N+2)/2 independent quartic contours); its χ₋₄-antisymmetric invariant — conjugate-sector **amplitude ratio** `= G*` at N=1 (the a=3 partition function itself vanishes there by the support rule), full-Z ratios `ℚ·G*^N` where both phases are supported (CHPS strongly-coupled matrix models; `09_mathematical/general_math/EXPLR_GSTAR_MATRIX_MODELS.md` §4) |
| operator face | self-adjoint Laplacian `Δ = (operator)∘(adjoint)` | the **chosen adjoint** `Q*` that turns a nilpotent `Q` into `H={Q,Q*}` (SUSY-QM) |
| graded-√ face | Dirac–Kähler `K=d−δ`, `K²=−Δ_Hodge`; `∂_t^{1/2}` with a **cyclotomic** branch sign | a graded √ whose square is the **one-to-many inverse** of a lossy collapse (does not exist over the modulus field) |
| probabilistic face | the trace `τ(ab)=τ(ba)`; the pushforward / marginalization | the **non-tracial state** / modular flow (Tomita–Takesaki, nonzero *iff* `ω(ab)≠ω(ba)`); the disintegrating **section** (Rokhlin), multivalued off-support |
| complex-analytic face | `\|z\|` (modulus) | `arg(z)` (argument / phase) |

The split is the even/odd decomposition under `z ↔ 1−z`; equivalently **forced** (positivity-/magnitude-determined, unique) vs **chosen** (branch-selecting, multivalued). Five mature, independent bodies of mathematics — **Tomita–Takesaki modular theory, Rokhlin disintegration, MERA inverse-RG, free probability, Petz recovery** — each describe the argument half as the *chosen adjoint/section/state of a lossy map*, and each certify that the choice is **imported, never a functor of the forward map** (the inverse is multivalued by construction; that non-uniqueness *is* the choice).

---

## 3 · The meta-conjecture `[CONJECTURE]`

> **Conjecture (modulus/argument frontier).** For every `S` in the substrate class, the natively-reachable structure is exactly the **modulus half** — the closure of the substrate's forced operations (its forward map plus its own magnitude). The **argument half is not self-supplyable**: it is the chosen adjoint / section / non-tracial state of the substrate's own many-to-one forward map, which is multivalued precisely to the degree that map destroyed information. The structure that collapses the multivalued inverse to single-valued (an adjoint, a metric, a prior, a non-tracial state) is **imported**, never derived from the forward dynamics.

This is a **precise statement to attempt**, tagged `[CONJECTURE]`/`[SPECULATION]` — **not** a theorem and **not** a derivation. Its honest status and the work needed to firm it are in §6–§7. FTD is its **worked proof-by-example for one substrate** (§4).

---

## 4 · FTD as the worked instance — five boundaries, one frontier

Every open boundary FTD has mapped is the *same* missing argument-structure, at its canonical (unpromoted) tag. Per `AUDIT_BOUNDARY_MAP.md` §0.5 (2026-07-01 red-team remediation), each row below is classified **PROVEN** (a structural no-go theorem, valid within stated assumptions) or **ATTEMPTED** (an empirical/computational trial that came back negative — raises a prior, proves nothing beyond it) — and each carries a **concrete falsifier**: a specific construction whose success would overturn that row.

| Boundary | The missing **argument**-structure | Canonical FTD status | Class | Falsifier |
|---|---|---|---|---|
| **α via δ** | the degree-4 Galois branch `√(G*(4G*−1))` **outside ℚ(G\*)** — not a second `i` (the substrate has `i`); it is a chosen branch of a degree-2 extension | `x₊=1/α` `[SMC]` (FTD-0013); routes closed FTD-0244/0314/0326/0327; **MC-T4.3 `[FOUNDATIONAL OBSTRUCTION]`** | **PROVEN**, conditional on Chudnovsky (1976) | A native operator or finite-symmetry carrier reaching `√(G*(4G*−1))`; equivalently a disproof of Chudnovsky's algebraic-independence theorem, or an un-exhibited forward-derived weight-mixing period surviving the FTD-0314 pressure |
| **QM non-commutativity** | the measurement map `M` — a non-commutative / non-tracial observable pairing | **FC-1** declines `M` `[AXIOM]`; licensed by FTD-0243 (commutativity independence) `[THEOREM]` | **PROVEN** (structural: commutative substrate ⇒ no quantum core) | A demonstration that the substrate's own flux/state algebra is non-commutative under an operational reading not yet considered |
| **Reversibility / Lorentzian metric** | the backward pairing across the many-to-one manifestation map (its inverse is one-to-many = a choice) | **FC-2** native arrow `[AXIOM]`; gap mapped by FTD-0253 | **PROVEN** (P5 is determinism, not reversibility) | A forcing argument deriving reversibility from P1–P5 as stated |
| **L²-not-L¹ budget** | the transverse phase / orthogonal pairing of the Pythagorean norm (a continuous SO(3) choice the discrete O_h cannot make) | FTD-0208 `[stands]` | **PROVEN** (v3, structural incompatibility with Scale-0 primitives) | A native Scale-0 construction carrying an L²-norm/Pythagorean structure without importing it |
| **Cavity-not-Schrödinger dispersion** | the rest-mass gap / quadratic dispersion a native restoring (non-tracial) term would supply | **FTD-0270** `[MEASURED — BOUNDARY]` | **ATTEMPTED** — a measurement, not a proof that quadratic dispersion is impossible in this substrate class | A measurement at a different scale/coupling/nonlinear regime showing ω∝k² natively; the P2 mass-gap attempt (FTD-0333) tried and returned `[INVALID per pre-registration]`, not a clean negative |

And the **modulus half FTD provably owns**: the algebraic spine (`G* = Γ(1/4)/Γ(3/4)`, master quadratic), classical electromagnetism (Phase-G geometric Coulomb = lattice Green's function), the Dirac–Kähler graded square root (FTD-0089), and the native arrow `∂_t^{1/2}` whose branch sign `(1+i)/√2` is a **root of unity inside ℚ(G\*)** (FTD-0323). *That cyclotomic sign is the structural reason the native √ cannot carry δ:* the substrate's square-root-of-time lands in the cyclotomic field; δ is a transcendental surd in a different arithmetic place.

---

## 5 · The evidence (why the frontier is the honest reading)

1. **One structural fact, five vocabularies — not five independent lines of support** (corrected 2026-07-01; the prior "five-fold independent convergence" framing over-counted this, per `AUDIT_REDTEAM_DISSECTION_2026-07-01.md` §2). Tomita–Takesaki modular theory, Rokhlin disintegration, MERA inverse-RG, free probability, and Petz recovery are five well-established mathematical frameworks that each formalize the *same* underlying fact: a lossy (non-injective) forward map has no canonical single-valued inverse, and recovering one requires an imported choice (an adjoint, a section, a prior, a non-tracial state). This is not five independent discoveries converging on FTD's boundary — it is one elementary fact about lossy maps, restated in five established vocabularies. Citing all five checks that the framing is not idiosyncratic to one sub-field; it does not multiply the evidence for the meta-conjecture of §3.
2. **The corpus no-gos exhaust the modulus side.** Closed: the operator class (FTD-0244 K-BIND — native operators ⊂ ℚ(G\*)); the entire finite-symmetry class and all five native ℤ/2's (FTD-0326, Galois-blind to δ); the substrate's whole √-machine / AGM (FTD-0327, δ-blind); the carrier-narrowing (FTD-0314, ~85% closed); route-invariance (FTD-0242/0243). ~52 routes closed-negative; the surd is transcendental over ℚ(G\*) with no graded-period home.
3. **The split is exact, not analogical.** `Γ(z)Γ(1−z)=π` (even, modulus, reversible) vs `Γ(z)/Γ(1−z)=G*` (odd, argument, the arrow) is the literal even/odd decomposition FTD already holds (FTD-0323); the half-derivative `∂_t^{1/2}∘∂_t^{1/2}=∂_t` with `G*` its eigenvalue is the operator-level form.

---

## 6 · The honest ceiling (mandatory)

- **The meta-claim of §3 is `[CONJECTURE]`/`[SPECULATION]` — a slogan made precise, not a proof.** FTD is proof-by-example for *one* substrate. Whether the frontier firms to `[THEOREM]` for any substrate sub-class is the **open research question** (§7), requiring a category-theoretic formalization of "self-supply" and "no backward pairing across the kernel."
- **Promotes nothing.** `x₊=1/α` stays `[STRONGLY MOTIVATED CONJECTURE]`; **MC-T4.3 stays a `[FOUNDATIONAL OBSTRUCTION]`**; no α is derived. The frontier's explanatory content is logically independent of whether `x₊=1/α` is true — a fair, narrow claim about *what the frontier explains*, not a reason to rate it more defensible than any other claim in the corpus. **Independence from a separate conjecture is not evidence for the independent claim itself**, and treating it that way was flagged as a red-team finding (`AUDIT_REDTEAM_DISSECTION_2026-07-01.md` §2, corrected here 2026-07-01). The honest standing of the frontier is exactly what §4's per-row table states: four rows are genuine structural proofs conditional on stated assumptions (**PROVEN**), one is an empirical measurement rather than a proof of impossibility (**ATTEMPTED**), and each carries its own concrete falsifier. It is that per-instance falsifiability — not invariance to a separate, unrelated conjecture — that makes the frontier a rigorous catalog rather than an unfalsifiable narrative.
- **No new-mathematics overclaim.** The "new math" the boundary points to (graded supersquare / inverse-integral / statistics-of-events) is **assembly-of-existing + one genuinely-new ingredient** (the Galois-position test over ℚ(G\*)), per the novelty audit — not new mathematics.
- This is **clause 2 of the Number-One Goal at maximum generality**: the rigorous map of what determinism + discreteness can and cannot ground.

---

## 7 · What this poses (the research questions it frames)

- **The four-walls-are-one forcing theorem** (T2): are FC-1, FC-2, FC-W, and the L²-not-L¹ budget the *same* import — does adopting any one force the others? A win collapses 3+ axioms to one structural deficiency and firms the frontier for FTD. (High ceiling, currently no proof sketch.)
- **The category-theoretic formalization** of "an `S`-substrate cannot self-supply the section/non-tracial pairing of its own kernel," for a defined substrate sub-class.
- **The genesis-cokernel test** (frozen pre-registration `PREREG_GENESIS_COKERNEL_GRADING_v1`, tag `preregister-genesis-cokernel-grading-v1`) as the empirical probe of whether the lossy step carries δ — the one structurally-distinct carrier then unexamined. **Executed 2026-07-02, registered as FTD-0365: verdict UNDERDETERMINED (re-scope)** — the fiber's section-invariant content is exact-rational + the K_B import (no G\*, a fortiori no δ); formally NOT Outcome B (null / section-dependent grading per the frozen gates), so the wall is **not** hardened by the run. See `ANALYSIS_GENESIS_COKERNEL_GRADING_v1.md`.

### 7.1 · Wins and walls share one root `[CONJECTURE]`

An adversarial round table on the QM/GR incompatibility (assessing whether this frontier explains it) returned mostly `[relabeling]` of known quantum-gravity problems with no forward-distinguishable content — **that reading is explicitly not adopted here.** One narrower observation from the same round table survives its own skeptic and *is* recorded: the **same** substrate renunciation — importing no continuum, no zero-point energy, no chosen adjoint — does three things at once. It (a) dissolves the UV divergence (the compact Brillouin zone makes every mode sum finite, `DERIV_VACUUM_ENERGY_CUTOFF.md`); it (b) dissolves the cosmological-constant catastrophe (`Λ = 0` by construction — the classical void is zero-energy under FC-1, `DERIV_LAMBDA_SCALE_COVARIANT.md` §1); and it (c) is *exactly* what this document's §3 conjecture says forbids the argument-half. Wins (a)+(b) and the wall (c) are not three separate facts — they are one renunciation, read three ways. Mainstream quantum-gravity programs typically treat UV-finiteness as a prize to *engineer*; this reframes it as the very thing that *costs* a substrate its argument-half: **"no UV divergence" was never the real obstruction.**

**What this is not.** This is **not** a claim that FTD diagnoses or resolves the QM/GR incompatibility — the round table's own mainstream-physics skeptic judged that broader reading mostly a relabeling of the problem-of-time / measurement-problem / non-renormalizability literature, with zero forward-distinguishable prediction, and that verdict is accepted, not contested. This is a narrower, self-contained claim about **FTD's own renunciation** — that its wins and its walls share one structural root — independent of whether the QM/GR framing has any further merit. `[CONJECTURE]`; promotes nothing; `Λ`'s value stays `[BOUNDARY]` (FTD-0059) and its source stays `[OPEN]` (FTD-0331).

---

## 8 · Status line

Tag `[SYNTHESIS]` + `[CONJECTURE]`. **Nothing promoted**; standing invariants held (FTD-0013 `[SMC]`; MC-T4.3 `[FOUNDATIONAL OBSTRUCTION]`; no α derived). This document is the canonical internal boundary statement and the skeleton of an external structural-negative paper; it consolidates `EXPLR_GENESIS_COKERNEL_GRADED_SQRT.md`, `AUDIT_BOUNDARY_MAP.md` (FTD-0335), FTD-0244/0314/0323/0326/0327, and FC-1/FC-2/FC-W into one frontier, and replaces the "second `i`" framing corpus-wide with **the chosen adjoint of the lossy map**.
