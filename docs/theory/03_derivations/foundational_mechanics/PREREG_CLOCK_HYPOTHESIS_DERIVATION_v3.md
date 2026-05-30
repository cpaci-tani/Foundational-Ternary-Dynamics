# Pre-Registration — Clock-Hypothesis Substrate-Derivation Attempt (v3)

**Tag:** `[PRE-REGISTRATION]` — locks the **design** of the v3 closure attempt against the clock hypothesis used implicitly in `SPEC_FTD_LAGRANGIAN.md` §4.3 [THEOREM]. Contains **no result**. All three pre-blessed outcomes (FOUND / UNDERDETERMINED / CLOSED-NEGATIVE) are admissible; verdict is genuinely open. **Prior-favoured outcome: UNDERDETERMINED or CLOSED-NEGATIVE** — see §1 audit-of-priors below.

**Date:** 2026-05-25
**Hash-lock target tag:** `preregister-clock-hypothesis-derivation-v3`
**LEDGER row:** FTD-0208 (Arc B P2 v3 closure verdict)
**Supersedes:** v1 (`PREREG_CLOCK_HYPOTHESIS_DERIVATION_v1.md`, commit `4c15ba1`, tag `preregister-clock-hypothesis-derivation-v1`) closed UNDERDETERMINED per `AUDIT_CLOCK_HYPOTHESIS_v1_UNDERDETERMINED.md`; **v2** (drafted 2026-05-25, never hash-locked, **INVALIDATED** on process + substance axes per `AUDIT_CLOCK_HYPOTHESIS_v2_UNDERDETERMINED.md`; archived to `archive/retracted/`).
**Companion docs:**
- [`DERIV_NEWTON_FROM_SUBSTRATE.md`](../gravity_and_cosmology/DERIV_NEWTON_FROM_SUBSTRATE.md) §1.4 — flagged POSTULATE 2 (reconciled to SPEC §4.3 modulo clock hypothesis 2026-05-24)
- [`../01_reference/SPEC_FTD_LAGRANGIAN.md`](../01_reference/SPEC_FTD_LAGRANGIAN.md) §3.7 (bandwidth constraint) + §4.3 (Born-Infeld proper time) + §8 L-1 [THEOREM modulo clock hypothesis]
- [`AUDIT_NEWTON_POSTULATES_RECONCILIATION.md`](../../07_assessment/audits/AUDIT_NEWTON_POSTULATES_RECONCILIATION.md) §2 (reconciliation that identified the clock hypothesis as the narrowed remaining open piece)
- [`AUDIT_CLOCK_HYPOTHESIS_v1_UNDERDETERMINED.md`](../../07_assessment/audits/AUDIT_CLOCK_HYPOTHESIS_v1_UNDERDETERMINED.md) (v1 verdict + named routes for v2)
- [`AUDIT_CLOCK_HYPOTHESIS_v2_UNDERDETERMINED.md`](../../07_assessment/audits/AUDIT_CLOCK_HYPOTHESIS_v2_UNDERDETERMINED.md) (v2 invalidation; sharpens v3 target)
- [`archive/retracted/PREREG_CLOCK_HYPOTHESIS_DERIVATION_v2.md`](archive/retracted/PREREG_CLOCK_HYPOTHESIS_DERIVATION_v2.md) (archived v2 attempt)
- [`archive/retracted/FOUND_CLOCK_HYPOTHESIS.md`](archive/retracted/FOUND_CLOCK_HYPOTHESIS.md) (archived v2 result)

> **Pre-registration discipline.** Sections §§2–9 are committed **before** the closure attempt is run. After commit: SHA256 → `../10_eft_program/REF_PREREGISTER_MANIFEST.md`, git tag `preregister-clock-hypothesis-derivation-v3` applied. Any post-hoc edit to §§2–9 invalidates v3; a v4 is required before the closure attempt is run or re-run. The closure attempt's result lands in a separate doc (`FOUND_CLOCK_HYPOTHESIS_v3.md` or `AUDIT_CLOCK_HYPOTHESIS_v3_UNDERDETERMINED.md` or `AUDIT_CLOCK_HYPOTHESIS_v3_CLOSED_NEGATIVE.md`), never as edits to this file. **B-9 (NEW per v2 audit): the pre-reg file and the result document MUST NOT share a same-minute mtime; the commit-then-tag step must be visible in `git log` before the result document is authored.** **B-10 (NEW per v2 audit): adversarial review (§9 step 9) must be conducted by a separately dispatched independent agent (Task / Agent tool invocation to a `general-purpose` or `Explore` agent), not by a same-process self-review.**

---

## §1 — Audit of priors (the lesson from v1 + v2)

**v1 prior** ("substrate-physics derivation via the bandwidth-constraint route is plausible") was conditionally validated: the v1 attempt DID reach `dτ/dt = √(f - v²/f)` algebraically — but only by asserting an unjustified intermediate identification (`Ω_clock ~ |ℒ_matter|/S_trans`, an F-d/F-f-firing dimensional placeholder).

**v2 prior** (with FTD-0041 calibration + bandwidth-internal-time routes in the §4 catalog, the derivation should close FOUND) was **falsified by the post-hoc audit**: the v2 derivation closes only because the magic ingredient (quadratic L²-norm budget conservation `(dτ/dt_local)² + v_local² = 1`) was added to the §4 catalog as a primitive. The relation itself is QM/SR-borrowed Pythagorean structure with no derivation from FTD axioms. Per v2's own Outcome B definition this is exactly "an intermediate principle outside the catalog that has not been independently substrate-derived" → UNDERDETERMINED.

**Lesson recorded.** The recursive pattern (v1 hides the gap in a dimensional placeholder; v2 hides it by promoting the gap to a primitive) argues that **for every primitive added to §4, the primitive itself must be derivable from the prior catalog or honestly tagged [AXIOM]**. v3 sharpens the target to test this directly.

**v3 prior-favoured outcome.** **UNDERDETERMINED or CLOSED-NEGATIVE** are more likely than FOUND. The quadratic Pythagorean structure of orthogonal degrees of freedom is the *content* of SR/GR's local Minkowski geometry; deriving it from FTD's discrete substrate (5 axioms + linear bandwidth constraint) would be a substantial advance — substantial enough that the substrate-rate-of-progress prior makes UNDERDETERMINED the modal outcome of a single closure attempt. The honest goal of v3 is **a rigorous boundary statement**, not necessarily a positive derivation. A CLOSED-NEGATIVE v3 verdict (genuinely demonstrated, not declared) would tag the clock hypothesis as `[AXIOM]` honestly and serve CLAUDE.md goal-clause 2 ("rigorously establish what we cannot derive").

---

## §2 — The question (LOCKED)

**Q-CH-3.** Does the **bandwidth budget conservation relation**

> $\left(\frac{d\tau}{dt_{\text{local}}}\right)^2 + v_{\text{local}}^2 = 1$

(used in v2 §4 catalog item 7 as the load-bearing primitive that closes the clock hypothesis derivation) **itself derive from FTD substrate primitives** — specifically:
- Postulates 1–5 of `SPEC_FTD.md` (discrete cubic lattice, discrete time, ternary states `{−1, 0, +1}`, local 26-Moore causality, determinism);
- SPEC §3.7 bandwidth constraint $v < f$ with $v = |\Delta_t \mathbf{J}|/K_B$ and $f = 1 - \mathcal{L}^2$ (linear);
- Engine tick mechanism + FTD-0041 calibration $T_U \equiv \sqrt{3}\ell_P/c$;
- D6 local speed limit $v_{\text{local}} = v/f$ + D7 local time tick $dt_{\text{local}} = \sqrt{f} dt$ (derived from SPEC §4.2 Poisson field equation);
**without invoking** Pythagorean / Minkowskian / L²-norm Hilbert structure as an axiom or as an asserted "L²-norm probability conservation on 26-Moore" without substrate derivation?

The derivation must:
1. Use only primitives from the §4 frozen catalog;
2. Produce the quadratic relation $(d\tau/dt_{\text{local}})^2 + v_{\text{local}}^2 = 1$ as a derived consequence, **not** as a definitional input;
3. Show explicitly why the addition is *quadratic* (not linear, not L¹, not L∞) from substrate considerations;
4. Pass the §7 falsifier ruleset mechanically (including new F-k and F-l);
5. Pass the §8 banned-moves checklist mechanically (including new B-9 and B-10);
6. Reach the §9 step 11 verdict only after steps 1–10 close cleanly.

All three §6 outcomes are pre-blessed. The verdict is genuinely open.

---

## §3 — Definitions (LOCKED)

- **D1 — Bandwidth budget conservation relation (target).** $(d\tau/dt_{\text{local}})^2 + v_{\text{local}}^2 = 1$ with $v_{\text{local}} = v/f \in [0, 1)$ and $d\tau/dt_{\text{local}} \in (0, 1]$. The relation says: per local coordinate time tick, a voxel's "update capacity" is partitioned **quadratically** between spatial field translation (fraction $v_{\text{local}}^2$) and internal clock transitions (fraction $(d\tau/dt_{\text{local}})^2$), with the two fractions summing to 1.
- **D2 — Substrate primitives.** The §4 frozen catalog, drawn from SPEC §3.7 (linear bandwidth constraint), the substrate manifestation rule, the engine tick mechanism, the D4 substrate clock, and D6/D7 local-velocity/local-time-tick (derived from SPEC §4.2). **Excluded as primitives: any Pythagorean / Minkowskian / L²-norm Hilbert structure on the discrete state space**.
- **D3 — Quadratic structure (operational form).** The exponent 2 in the budget-conservation relation. v3 asks whether this exponent is *forced* by FTD primitives (in which case FOUND), or requires an external principle (UNDERDETERMINED), or is incompatible with FTD's discrete substrate (CLOSED-NEGATIVE).
- **D4 — Orthogonality (operational form).** The independence of "spatial field translation" and "internal clock transitions" as separable degrees of freedom in the voxel's per-tick state update. v3 asks whether this orthogonality is *derivable* from the 26-Moore locality + ternary state space + Postulate 5 determinism, or requires an external inner-product / metric structure to define.
- **D5 — Substrate clock (D4 of v1/v2).** Counting process on manifested-site transitions per universal tick $T_U$.
- **D6 — Closure (operational).** A chain of derivation steps, each tagged with epistemic status, where the last step shows that the quadratic budget conservation relation **follows from** §4 primitives without invoking D1's quadratic exponent or D4's orthogonality as definitional inputs. Each intermediate step must cite which §4 primitive it uses.

---

## §4 — Admissible search space (LOCKED)

The closure attempt may use ONLY the following primitives (frozen 2026-05-25 at v3 hash-lock):

1. **SPEC §3.7 bandwidth constraint** $v < f$ (LINEAR, not quadratic) with $v = |\Delta_t \mathbf{J}|/K_B$ and $f = 1 - \mathcal{L}^2$.
2. **Born-Infeld action measure as Lagrangian density** $S = -K_B \int \sqrt{(f^2-v^2)/f}\,dt$ per SPEC §3.3, treated as a function of (v, ℒ) — NOT yet identified with time.
3. **Substrate manifestation rate** — rate at which threshold-crossing events occur per universal tick $T_U$.
4. **Engine tick $T_U$** — universal discrete tick advancing all sites in lockstep; calibrated to $\sqrt{3}\ell_P/c$ per FTD-0041.
5. **D4 substrate clock** — counting process on manifested-site transitions.
6. **D6 local speed limit + D7 local time tick** — derived from SPEC §4.2 Poisson equation.
7. **FTD axioms 1–5** from `SPEC_FTD.md`.
8. **Algebraic spine theorems** as cited tools.

**Explicitly EXCLUDED from §4 catalog** (any invocation triggers F-k, F-l, F-a, F-e, or F-g as appropriate):

- The quadratic budget conservation relation $(d\tau/dt_{\text{local}})^2 + v_{\text{local}}^2 = 1$ itself (this IS the target).
- Pythagorean / Minkowskian addition of "orthogonal degrees of freedom" without per-voxel per-tick derivation from §4 primitives.
- L²-norm / Hilbert-space / probability-amplitude structure on the discrete state space $\{-1, 0, +1\}^\Lambda$ (no such structure is established by Postulates 1–5).
- "Orthogonal degrees of freedom" framing that imports an inner product / metric to define orthogonality.
- GR's empirical clock postulate; standard relativistic-particle theory's reparametrization-invariance argument; Schwarzschild metric form; any continuum-spacetime metric formalism ($g_{\mu\nu}$, $h_{\mu\nu}$) as a derivation primitive.

---

## §5 — Benchmark (LOCKED)

**Benchmark form:** $(d\tau/dt_{\text{local}})^2 + v_{\text{local}}^2 = 1$ for all $f \in (0,1]$ and $v \in [0, f)$, **derived** from §4 primitives without invoking §4-excluded items.

**Secondary benchmark (downstream, conditional on FOUND of primary):** the algebraic consequence $d\tau/dt = \sqrt{f - v^2/f}$ following from the quadratic relation + D6/D7 substitution — verifiable by simple algebra once the quadratic relation is established. This is NOT an independent target; it is automatic given primary closure.

---

## §6 — Three pre-registered outcomes (LOCKED)

**Outcome A (FOUND).** A derivation chain exists, each step tagged with epistemic status, using only §4-catalog primitives, that produces the quadratic budget conservation relation $(d\tau/dt_{\text{local}})^2 + v_{\text{local}}^2 = 1$ as a derived consequence. The exponent 2 is shown to be **forced** by §4 primitives (not by analogy to SR/QM, not by importing Pythagorean structure as definitional). The orthogonality of spatial/internal updates is **derived** from 26-Moore locality + ternary state space + Postulate 5, not posited via an external inner product. All §7 falsifiers must be checked as not firing; all §8 banned moves must be checked as not invoked. Adversarial review (§9 step 9, independent agent per B-10) must agree. **Tag consequences:** the v2 result (claimed FOUND but invalidated) is retroactively legitimized at the conditional-on-budget-conservation-primitive level — but only if v3 itself closes FOUND on the primitive. SPEC §4.3 + §8 L-1 promoted from `[THEOREM modulo clock hypothesis]` to fully `[THEOREM]`; LEDGER FTD-0131 promoted to fully `[DERIVED]`; LEDGER FTD-0208 promoted to `[THEOREM]`. Plan v2 Arc B P5 marked CLOSED with FOUND verdict.

**Outcome B (UNDERDETERMINED).** A derivation chain reaches the quadratic relation but requires an intermediate principle outside the §4 catalog — for example, an axiom of probability-amplitude structure, a finite-trace mechanics axiom, a graph spectral geometry principle, or another doctrine §12 candidate principle — and that principle has not been independently substrate-derived. **Tag consequences:** SPEC §4.3 + §8 L-1 status unchanged (`[THEOREM modulo clock hypothesis]`); LEDGER FTD-0208 stays `[UNDERDETERMINED]` with the new intermediate principle named for v4 attempt; v4 pre-reg required. Plan v2 Arc B P5 marked PARTIAL.

**Outcome C (CLOSED-NEGATIVE).** No derivation chain from §4 primitives produces the quadratic budget conservation relation without F-k / F-l firing or banned-move invocation, AND a structural argument shows that Pythagorean addition is genuinely incompatible with FTD's discrete substrate at Postulates-1–5 level (e.g., the ternary state space lacks the metric structure necessary to define orthogonality; the 26-Moore neighborhood propagation is L¹/L∞-natural, not L²; or a no-go theorem analogous to FTD-0059 / FTD-0096). **Tag consequences:** the clock hypothesis is honestly tagged `[AXIOM]` in SPEC §4.3 + §8 L-1, with explicit cross-reference to this pre-reg. LEDGER FTD-0208 marked `[CLOSED NEGATIVE, AXIOM-LEVEL]` per pre-reg §6 Outcome C. LEDGER FTD-0131 retains `[DERIVED]` with the clock hypothesis explicitly tagged `[AXIOM]` rather than `[OPEN]`. Plan v2 Arc B P5 marked CLOSED with CLOSED-NEGATIVE verdict — this is a positive deliverable under CLAUDE.md goal-clause 2 ("rigorously establish what we cannot derive").

---

## §7 — Falsifier rules (LOCKED) — F-a..F-l

Mechanical falsifier checklist. Any single firing → at most Outcome B or Outcome C; not Outcome A.

- **F-a.** No insertion of "ideal clocks measure proper time" or equivalent GR clock postulate at any step.
- **F-b.** No insertion of the target quadratic relation $(d\tau/dt_{\text{local}})^2 + v_{\text{local}}^2 = 1$ before the derivation chain produces it.
- **F-c.** No free parameter introduced. The exponent 2 must be derived, not fitted.
- **F-d.** Operational-bandwidth-constraint dependency must be specified per voxel per tick. Vague invocations ("the bandwidth constraint implies", $S_{\text{trans}} \sim K_B \cdot \text{constant}$ placeholders) fire F-d. Sharpened from v2 by requiring explicit derivation, not assertion.
- **F-e.** No appeal to "standard relativistic-particle theory" or "the proper-time parameter is what makes the action reparametrization-invariant" or any "this is well-known from GR" move.
- **F-f.** Each derivation step must cite which §4-catalog primitive it uses. Any step that does not cite a §4 primitive (or that cites a §4-excluded item) fires F-f.
- **F-g.** No re-invocation of FALSIFIED or INVALIDATED closure routes — specifically the v1 dimensional-action-quantum placeholder pattern (`Ω_clock ~ |ℒ_matter|/S_trans`) and the v2 budget-conservation-as-primitive smuggling (the relation may be the *target* but cannot be a derivation primitive).
- **F-h.** No comparison to measured Schwarzschild proper time or EIN-4 engine measurement before §9 step 10. Any earlier numerical comparison fires F-h.
- **F-i.** No look-elsewhere across candidate "budget-conservation-replacing" mechanisms. v3 is scoped to one mechanism: derivation of the quadratic relation from §4 primitives. Switching mechanisms mid-derivation fires F-i (alternative requires its own v4 pre-reg).
- **F-j.** The v2 derivation chain CANNOT be used as scaffold. The budget-conservation step must be derived FRESH from §4 primitives, not cited from v2 §4.7 (which has been INVALIDATED).
- **F-k (NEW per v2 audit).** Any assertion of quadratic / Pythagorean / L²-norm addition of orthogonal degrees of freedom without per-voxel per-tick operational derivation from §4 primitives fires F-k. The v2 §4.7 "discrete L²-norm probability conservation on 26-Moore" assertion is the canonical F-k-firing pattern. The 26-Moore neighborhood is not a Hilbert space; "probability conservation" cites a structure FTD does not have.
- **F-l (NEW per v2 audit).** Any invocation of "orthogonal degrees of freedom" without explicit derivation of the orthogonality from FTD primitives fires F-l. Orthogonality in a metric space requires a metric; FTD has no fundamental metric. The metric is precisely what a successful FTD derivation of SR/GR would have to **produce**, not assume.

---

## §8 — Banned moves / anti-laundering (LOCKED) — B-1..B-10

Process-level rules that go beyond §7 falsifiers. Any banned move invocation invalidates the closure attempt.

- **B-1.** No fitting of any proportionality constant after seeing the §5 benchmark form. No post-hoc adjustment of the exponent 2.
- **B-2.** No re-tagging of SPEC §4.3, SPEC §8 L-1, DERIV_NEWTON_FROM_SUBSTRATE.md §1.4, or LEDGER FTD-0131 / FTD-0208 before the result document lands.
- **B-3.** No invocation of "ideal clocks" / "Born rule" / "L²-norm probability conservation" / "Hilbert space inner product" as a substrate primitive.
- **B-4.** No appeal to operational definitions from outside FTD's axioms.
- **B-5.** No promotion of any tag until the result document lands.
- **B-6.** No use of "manifestly Lorentz-invariant" / "covariant" / "Pythagorean" / "Minkowskian" / similar dignifying terminology to smuggle in the quadratic structure without derivation.
- **B-7.** No appeal to "the clock hypothesis is standard" / "Pythagorean structure is fundamental" / "this is just how proper time works".
- **B-8.** No conflation of substrate-level orthogonality (discrete ternary state space) with Hilbert-space orthogonality (Born-rule structure).
- **B-9 (NEW per v2 audit, METHODOLOGICAL).** The pre-reg file and the result document **MUST NOT** share a same-minute mtime. The commit-then-tag step (`git commit PREREG_*_v3.md && git tag preregister-clock-hypothesis-derivation-v3`) MUST be visible in `git log` BEFORE the result document is authored. Any closure attempt with same-minute or earlier mtimes on the result document relative to the pre-reg commit invalidates v3 by this banned-move rule (audit can confirm via `git log --follow PREREG_*_v3.md` and `stat` on the result document).
- **B-10 (NEW per v2 audit, METHODOLOGICAL).** Adversarial review (§9 step 9) MUST be conducted by a **separately dispatched independent agent** (Task or Agent tool invocation to a `general-purpose` or `Explore` subagent, OR a human reviewer). Same-process self-review (e.g., the v2 attempt's "self subagent" review) does NOT satisfy this requirement and invalidates v3. The reviewer must not see the §5 benchmark or §9 step 10 numerical comparison before issuing their verdict.

---

## §9 — Method (LOCKED) — 11 steps

1. **State substrate primitives** from §4 catalog explicitly, with site-level operational form for each (F-d compliance). Include explicit acknowledgment that §4 contains NO quadratic / Pythagorean / L² structure.
2. **Examine the structure of voxel state updates** under the 5 axioms + SPEC §3.7 + engine tick mechanism. Specifically: per universal tick $T_U$, what is the dimension and metric (if any) of the per-voxel update space? Cite §4 primitives explicitly.
3. **Identify "spatial field translation" and "internal clock transitions"** as separable update modes within the per-voxel update budget under §3.7's $v < f$ ceiling. Show explicitly whether these modes are orthogonal under any natural §4-derivable structure (F-l compliance).
4. **Derive the addition law** for the two modes' contributions to the per-tick budget. Show explicitly whether the addition is linear, quadratic, L¹, L∞, or other — from §4 primitives only, without invoking the target relation.
5. **If quadratic addition is derived in step 4**: prove that the exponent 2 is *forced* by §4 primitives, not chosen by analogy or for target-matching. **If non-quadratic addition is derived**: state the actual form and verify whether it can produce $d\tau/dt = \sqrt{f - v^2/f}$ under D6/D7 substitution (it generally cannot; this would trigger Outcome B or C).
6. **Map the derived budget law to $(d\tau/dt_{\text{local}}, v_{\text{local}})$** via D6/D7. Check that the resulting relation is $(d\tau/dt_{\text{local}})^2 + v_{\text{local}}^2 = 1$ as a **derived consequence**, not a posited form (F-b compliance).
7. **F-falsifier checklist (mechanical):** walk through F-a through F-l; for each, state explicitly whether it fired or not, with a one-sentence justification. Any single firing → Outcome B or C per §6. **F-k and F-l are the load-bearing new falsifiers per the v2 audit.**
8. **Banned-moves checklist (mechanical):** walk through B-1 through B-10; for each, state explicitly whether it was invoked or not. Verify B-9 and B-10 procedurally (`git log --follow PREREG_*_v3.md` + Agent-tool invocation record for adversarial review).
9. **Adversarial review checkpoint.** A **separately dispatched independent agent** (per B-10) must verify steps 1–8 without seeing the §5 benchmark or §9 step 10 numerical comparison. The reviewer's verdict is "PASS" or "FAIL"; FAIL invalidates the attempt; PASS-WITH-CAVEATS triggers Outcome B (caveats become named v4 follow-ups).
10. **Numerical comparison to §5 benchmark.** Only after steps 1–9 close cleanly: compute the derived relation at a grid of (f, v) values and confirm $(d\tau/dt_{\text{local}})^2 + v_{\text{local}}^2 = 1$ holds exactly (modulo numerical precision). Confirm the secondary benchmark $d\tau/dt = \sqrt{f - v^2/f}$ follows algebraically.
11. **Verdict assignment per §6.** State explicitly which outcome (A, B, or C) the attempt landed, with the §7 + §8 checklists as justification. Result lands in `FOUND_CLOCK_HYPOTHESIS_v3.md` (FOUND), `AUDIT_CLOCK_HYPOTHESIS_v3_UNDERDETERMINED.md` (UNDERDETERMINED), or `AUDIT_CLOCK_HYPOTHESIS_v3_CLOSED_NEGATIVE.md` (CLOSED-NEGATIVE), **never as edits to this pre-registration**.

---

## §10 — F9 collusion-bias risk register (HIGH; v3-specific framing)

The v2 attempt failed precisely because F9 mitigation was bypassed:
- The pre-reg + result were authored in the same edit session (no git-tag-as-immutability lock).
- The "adversarial review" was a same-process self-review, not an independent agent.
- The v2 §4 catalog was expanded to include the magic ingredient between v1 (UNDERDETERMINED) and v2 (FOUND) — exactly the search-space-expansion laundering pattern F9 is meant to catch.

**v3 mitigations** (additive to v1 + v2 mitigations):
- **B-9** codifies the commit-then-tag-before-result requirement. Audit-checkable via `git log` and `stat`.
- **B-10** codifies the independent-agent adversarial-review requirement. Audit-checkable via the Agent/Task invocation record.
- **F-k and F-l** codify the substantive guards against quadratic-addition-as-primitive and orthogonality-without-metric. Mechanically checkable in any candidate derivation.
- **§1 audit-of-priors** documents v1 and v2 failure modes explicitly, so v3 cannot pretend the magic ingredient is new.

**Prior-favoured outcome (recorded):** UNDERDETERMINED or CLOSED-NEGATIVE > FOUND. A genuine derivation of Pythagorean structure from FTD discrete-substrate primitives would be a substantial advance; if it lands, the discipline that caught v1 and v2 will have produced a real result. A CLOSED-NEGATIVE verdict tagging the clock hypothesis as `[AXIOM]` would be honest and serves CLAUDE.md goal-clause 2.

---

## §11 — Hash-lock procedure (REQUIRED before closure attempt)

1. Commit this file in a coherent commit (e.g., `preregister(arc-b-p3): clock-hypothesis substrate-derivation v3`).
2. Compute SHA256: `sha256sum docs/theory/03_derivations/PREREG_CLOCK_HYPOTHESIS_DERIVATION_v3.md`.
3. Record SHA256 in `docs/theory/10_eft_program/REF_PREREGISTER_MANIFEST.md` v3 entry.
4. Apply git tag `preregister-clock-hypothesis-derivation-v3` to the commit.
5. Verify: `git rev-list -n1 preregister-clock-hypothesis-derivation-v3` resolves to the commit; `sha256sum` matches the manifest entry.
6. ONLY THEN may the v3 closure attempt be authored.

Per CLAUDE.md commit policy and the user's explicit direction on prior pre-regs, the hash-lock + git tag operations require explicit user direction; this pre-registration is staged for that operation.

---

## Closing note

This pre-registration is `[PRE-REGISTRATION]`-tagged. It contains no derivation, no tag promotion, no closure attempt. Its purpose is to lock the design BEFORE the attempt, so that whatever verdict lands is rigorous and F9-resistant. The v2 attempt's collapse of the F9 mitigation is the lesson recorded in §1 + §10; v3 must not repeat it.
