# Pre-Registration — Clock-Hypothesis Substrate-Derivation Attempt (v1)

**Tag:** `[PRE-REGISTRATION]` — locks the **design** of the closure attempt against the clock hypothesis used implicitly in `SPEC_FTD_LAGRANGIAN.md` §4.3 [THEOREM]. Contains **no result**. All three pre-blessed outcomes (FOUND / UNDERDETERMINED / CLOSED-NEGATIVE) are admissible; verdict is genuinely open. **Prior-favoured outcome: UNDERDETERMINED** (the clock hypothesis is a standard interpretive step in relativistic-particle theory; substrate-derivation would be a substantive advance, but is plausible via the bandwidth-constraint route).

**Date:** 2026-05-24
**Hash-lock target tag:** `preregister-clock-hypothesis-derivation-v1`
**LEDGER row reservation:** provisional, confirm next-free against `../07_assessment/LEDGER.md` at hash-lock time (current top per audit 2026-05-24 is FTD-0203).
**Supersedes:** none — first pre-registration on the clock hypothesis as an isolable interpretive step.
**Companion docs:**
- [`DERIV_NEWTON_FROM_SUBSTRATE.md`](DERIV_NEWTON_FROM_SUBSTRATE.md) §1.4 — flagged POSTULATE 2 (now reconciled to SPEC §4.3 modulo clock hypothesis)
- [`../01_reference/SPEC_FTD_LAGRANGIAN.md`](../01_reference/SPEC_FTD_LAGRANGIAN.md) §3.7 (bandwidth constraint) + §4.3 (Born-Infeld proper time) + §8 L-1 [THEOREM modulo clock hypothesis]
- [`AUDIT_NEWTON_POSTULATES_RECONCILIATION.md`](AUDIT_NEWTON_POSTULATES_RECONCILIATION.md) §2 (reconciliation that identified the clock hypothesis as the narrowed remaining open piece)
- [`../02_foundations/FOUND_SPACETIME_EMERGENCE_AND_GRAVITY.md`](../02_foundations/FOUND_SPACETIME_EMERGENCE_AND_GRAVITY.md) §14 — tick-rate-variation framing
- Methodological templates: [`../10_eft_program/PREREG_ALPHA_READOUT_OBSERVABLE_SELECTION_v1.md`](../10_eft_program/PREREG_ALPHA_READOUT_OBSERVABLE_SELECTION_v1.md) (9-section format + F-a..F-j ruleset + §8 banned moves)

> **Pre-registration discipline.** Sections §§2–9 are committed before the closure attempt is run. After commit: SHA256 → `../10_eft_program/REF_PREREGISTER_MANIFEST.md`, git tag applied. Any post-hoc edit to §§2–9 invalidates v1; a v2 is required before the closure attempt is run or re-run. The closure attempt's result lands in a separate doc (`REPORT_CLOCK_HYPOTHESIS_DERIVATION.md` or `AUDIT_CLOCK_HYPOTHESIS_CLOSED_NEGATIVE.md`), never as edits to this file.

**Purpose.** Lock, *before* any closure-attempt construction, (a) what would count as a substrate derivation of the clock hypothesis from FTD primitives, (b) what would **falsify** any candidate closure mechanism, and (c) the banned-moves list that catches re-import of GR's empirical clock postulate. This pre-registration is the anti-laundering instrument for an attempt where the target value (`√(f - v²/f)` for proper-time scaling) is a canonical GR result and the temptation to engineer toward it is therefore high (F9 collusion-bias risk: HIGH).

---

## §1 — Context and doctrine

**Origin.** The Arc B P0 reconciliation audit (2026-05-24, [`AUDIT_NEWTON_POSTULATES_RECONCILIATION.md`](AUDIT_NEWTON_POSTULATES_RECONCILIATION.md)) found that `SPEC_FTD_LAGRANGIAN.md` §4.3 [THEOREM] subsumes `DERIV_NEWTON_FROM_SUBSTRATE.md` §1.4's `[POSTULATE 2, flagged]` for the linearized tick-rate response. SPEC §4.3 derives `dτ/dt = √(f - v²/f)` (exact Schwarzschild proper time for all `f ∈ (0,1]`) from the Born-Infeld action `S = -K_B ∫√((f²-v²)/f) dt` plus a single interpretive step: "**By the clock hypothesis**, `dτ ∝ √((f²-v²)/f) dt`."

**The clock hypothesis** is the identification: the Born-Infeld action measure IS proper time, rather than merely a Lagrangian density. A grep across `docs/` (executed 2026-05-24) returns exactly 2 files: SPEC §4.3 and the AUDIT doc. **The clock hypothesis is not formally tagged anywhere in the FTD corpus.** It is treated as a definitional/interpretive step in §4.3 without explicit `[AXIOM]` or `[SELECTION]` flag.

**Doctrine clause this serves.** CLAUDE.md goal-clause 2: "Derive everything we can from a discrete ontology — **and rigorously establish what we cannot.**" A FOUND outcome promotes SPEC §4.3 from `[THEOREM modulo clock hypothesis]` to fully `[THEOREM]` and propagates through `DERIV_NEWTON_FROM_SUBSTRATE.md` §1.4 + LEDGER FTD-0131. A CLOSED-NEGATIVE outcome explicitly tags the clock hypothesis as `[AXIOM]` (or `[SELECTION]`) in SPEC §4.3, removing the informal-tag ambiguity and making the dependency structure honest. An UNDERDETERMINED outcome records what was attempted and queues a v2 with sharpened admissibility.

**Prior-favoured outcome.** UNDERDETERMINED. The clock hypothesis is a standard interpretive step in relativistic-particle theory (the proper-time parameter is what makes the Born-Infeld action reparametrization-invariant). A substrate-physics derivation via bandwidth-constraint + manifestation-rate scaling is **plausible** (SPEC §3.7's "v and ℒ draw from same bandwidth budget" framing is suggestive) but has not been attempted. The likely failure mode is that the candidate derivation requires a finite-trace-mechanics axiom or another §12 doctrine-ledger principle not in the §4 frozen catalog, in which case the verdict is UNDERDETERMINED rather than FOUND.

**F9 collusion-bias note (operational).** Target value `√(f - v²/f)` is a canonical GR proper-time formula known to any physics-trained agent or reviewer. **§7 falsifiers and §8 banned moves are calibrated specifically to catch reverse-engineering toward this target.** Mandatory adversarial-review checkpoint (§9 step 9) BEFORE any numerical comparison to SPEC §4.3.

---

## §2 — The question (LOCKED)

**Q-CH-1.** Does the clock hypothesis — the identification

> "the Born-Infeld action measure `dS = -K_B √((f²-v²)/f) dt` defines proper time as `dτ ∝ √((f²-v²)/f) dt`"

— derive from FTD substrate primitives (specifically: the bandwidth constraint `v < f` of SPEC §3.7 + substrate manifestation-rate scaling on the lattice tick mechanism) **without invoking** standard relativistic-particle theory's empirical clock postulate ("ideal clocks measure proper time along their worldline") or any GR-imported clock-rule input?

The derivation must:
1. Use only primitives from the §4 frozen catalog;
2. Produce `dτ/dt = √(f - v²/f)` as a derived consequence, not as a definitional input;
3. Pass the §7 falsifier ruleset mechanically;
4. Pass the §8 banned-moves checklist mechanically;
5. Reach the numerical comparison (§9 step 10) only after steps 1–9 close cleanly.

All three §6 outcomes are pre-blessed. The verdict is genuinely open.

---

## §3 — Definitions (LOCKED)

- **D1 — Clock hypothesis.** Identification of the action-measure `√((f²-v²)/f) dt` (units of time × dimensionless) with proper time `dτ` (units of time), via the proportionality `dτ ∝ √((f²-v²)/f) dt` with proportionality constant 1 in c=1 units. The hypothesis has **two parts**: (a) the action measure IS a time, not just a Lagrangian, and (b) the time it IS is the proper time experienced along the worldline. Both parts must be derived (not posited) for FOUND.
- **D2 — Substrate primitives.** The §4 frozen catalog, drawn from SPEC §3.7 (bandwidth constraint `v < f`), the substrate manifestation rule (threshold-crossing on `|J|² > K_GENESIS²` and `|J|² < K_EVAP²`), the engine tick mechanism (universal discrete tick `T_U` advancing all sites in lockstep), and the Born-Infeld action measure as a Lagrangian density (NOT yet identified with time). FTD axioms 1–5 from `SPEC_FTD.md` are implicit primitives.
- **D3 — Operational tick.** The engine's discrete universal tick `T_U`; one tick is one application of the phase_read → phase_write → gauss_project → phase_forces → phase_movement cycle per `engine/SPEC_ENGINE.md`. Per FTD-0041, one tick is calibrated to `√3·ℓ_P/c`; the proper-time identification must use this calibration consistently.
- **D4 — Substrate clock.** A counting process on manifested-site transitions: each transition (manifestation, evaporation, or sign-flip) is one substrate-clock event. The clock rate is the rate at which transitions occur per universal tick. The substrate clock is NOT assumed to measure proper time; whether it does is the closure question.
- **D5 — Bandwidth constraint (operational form).** `v < f` per SPEC §3.7 with `v = |Δ_tJ|/K_B` and `f = 1 - ℒ²`. Operationally: if a voxel's `|Δ_tJ|` would exceed `K_B·f` at the next tick, the engine clamps via the genesis/evap thresholds; the bandwidth constraint is the structural reason for this clamping. The constraint is **derived** from the Born-Infeld action (via §3.7 of SPEC); it is not an additional axiom.
- **D6 — Closure (operational).** A chain of derivation steps, each tagged with epistemic status, where the last step shows that `dτ/dt = √(f - v²/f)` follows from §4 catalog primitives without invoking "ideal clocks measure proper time" or any GR clock-rule as input. Each intermediate step must cite which §4 primitive it uses and must pass the §7 falsifier check.

---

## §4 — Admissible search space (LOCKED)

The closure attempt may use ONLY the following primitives (frozen 2026-05-24 at v1 hash-lock):

1. **SPEC §3.7 bandwidth constraint** `v < f`, `v = |Δ_tJ|/K_B`, `f = 1 - ℒ²` — derived from Born-Infeld action.
2. **Born-Infeld action measure as Lagrangian density** `S = -K_B ∫√((f²-v²)/f) dt` per SPEC §3.3, treated as a function of (v, ℒ) not yet identified with time.
3. **Substrate manifestation rate** — rate at which threshold-crossing events (|J|² ↔ K_GENESIS² / K_EVAP²) occur per universal tick T_U.
4. **Engine tick T_U** — universal discrete tick advancing all sites in lockstep; calibrated to `√3·ℓ_P/c` per FTD-0041 / FTD-0096.
5. **D4 substrate clock** — counting process on manifested-site transitions.
6. **FTD axioms 1–5** from `SPEC_FTD.md` (cubic lattice, discrete time, ternary states, 26-Moore locality, determinism).
7. **Algebraic spine theorems** as cited tools (master quadratic, Phase G geometric Coulomb, etc.) — usable only via citation, not as load-bearing derivation primitives for the clock-hypothesis chain itself.

**Explicitly EXCLUDED from §4 catalog (any invocation triggers F-a, F-e, or F-g):**

- GR's empirical clock postulate ("ideal clocks measure proper time along their worldline")
- "The proper-time parameter is what makes the action reparametrization-invariant" (standard relativistic-particle-theory move; this is GR import, not substrate derivation)
- Postulating `dτ ∝ √((f²-v²)/f) dt` directly (this IS the question; cannot be postulated)
- Schwarzschild metric `ds² = f·c²dt² - dr²/f - r²dΩ²` (the target; cannot be input)
- Any continuum-spacetime metric formalism (`g_μν`, `h_μν`) as a derivation primitive — these are imported scaffold per FTD-0189 audit
- Operational definitions from outside FTD's axioms (e.g., "ideal atomic clock", "GPS clock"); reference to these for sanity-check only at §9 step 10

---

## §5 — Benchmark (LOCKED)

**Benchmark form (target value to be compared at §9 step 10):**

`dτ/dt = √(f - v²/f)` for all `f ∈ (0,1]` and `v ∈ [0, f)`, derived from §4 primitives without invoking §4-excluded items.

**Benchmark sanity check (numerical):** the engine's `test_einstein_equations.cpp` EIN-4 (gravitational time dilation `0.004%` match in the Schwarzschild scalar sector) is consistent with the SPEC §4.3 form. A successful derivation does NOT need to re-verify EIN-4; it must produce the form whose linearization gives the EIN-4 prediction. EIN-4's numerical agreement is a verification artifact, not a derivation input.

**Benchmark scope:** the v=0 case `dτ/dt = √f` (gravitational time dilation, SPEC §5.2 [THEOREM]) and the v ≠ 0 case `dτ/dt = √(f - v²/f)` (full Schwarzschild proper time, SPEC §4.3 [THEOREM]). The flat-space limit `f = 1` reduces to special-relativity time dilation `√(1 - v²)`; recovery of this limit is automatic if the full form is derived, and is a sanity check, not an independent target.

---

## §6 — Three pre-registered outcomes (LOCKED)

**Outcome A (FOUND).** A derivation chain exists, each step tagged with epistemic status, using only §4-catalog primitives, that produces `dτ/dt = √(f - v²/f)` as a derived consequence of substrate primitives. All §7 falsifiers must be checked as not firing; all §8 banned moves must be checked as not invoked. Adversarial review (§9 step 9) must agree. **Tag consequences:** SPEC §4.3 promoted from `[THEOREM modulo clock hypothesis]` to fully `[THEOREM]`; SPEC §8 L-1 promoted similarly; `DERIV_NEWTON_FROM_SUBSTRATE.md` §1.4 reconciliation note updated; LEDGER FTD-0131 row updated to remove the "one flagged interpretive step" qualifier; AUDIT_NEWTON_POSTULATES_RECONCILIATION.md §2.4 verdict updated. Plan v2 Arc B P5 marked CLOSED with FOUND verdict.

**Outcome B (UNDERDETERMINED).** A derivation chain reaches `dτ/dt = √(f - v²/f)` but requires an intermediate principle outside the §4 catalog — for example, a finite-trace mechanics axiom, a graph spectral curvature principle, or another doctrine §12 candidate principle — and that principle has not been independently substrate-derived. **Tag consequences:** SPEC §4.3 status unchanged (`[THEOREM modulo clock hypothesis]`); v2 pre-registration required with the additional principle added to §4 catalog and its own derivation pursued as a sub-arc. Plan v2 Arc B P5 marked PARTIAL.

**Outcome C (CLOSED-NEGATIVE).** No derivation chain from §4 primitives produces `dτ/dt = √(f - v²/f)` without F-falsifier firing or banned-move invocation. The clock hypothesis is established as irreducibly interpretive within the §4 catalog; equivalently, the proportionality `dτ ∝ √((f²-v²)/f) dt` requires either (a) an external clock postulate (importing GR), or (b) an ontology extension beyond §4 primitives. **Tag consequences:** SPEC §4.3 + §8 L-1 explicitly tagged `[THEOREM, conditional on clock hypothesis AXIOM]`; the clock hypothesis itself receives an explicit `[AXIOM]` tag in SPEC §4.3 with cross-reference to this pre-registration; LEDGER FTD-0131 updated to "one explicit AXIOM (clock hypothesis)"; AUDIT_NEWTON_POSTULATES_RECONCILIATION.md §2.4 verdict updated to record CLOSED-NEGATIVE. Plan v2 Arc B P5 marked CLOSED with CLOSED-NEGATIVE verdict, which serves project-goal clause 2 honestly.

---

## §7 — Falsifier rules (LOCKED) — F-a..F-j

Mechanical falsifier checklist. Any single firing → at most Outcome B or Outcome C; not Outcome A.

- **F-a.** No insertion of "ideal clocks measure proper time" or equivalent GR clock postulate at any step. If the derivation prose contains the words "ideal clock", "proper-time parameter", "reparametrization-invariant action measure", or equivalent, F-a fires.
- **F-b.** No insertion of `dτ = √((f²-v²)/f) dt` or `dτ/dt = √(f - v²/f)` before the derivation chain produces it. The first appearance of this form must be a derived line, not a definitional line.
- **F-c.** No free parameter introduced. The derivation may use only constants from the algebraic spine + ontic chain; any proportionality constant must be either =1 (in c=1 units) or derived from §4 primitives. If a constant is fitted post-hoc to match the §5 benchmark, F-c fires.
- **F-d.** D5 bandwidth-constraint dependency must be operationally specified. "Voxel v ∈ Λ at tick t with `|Δ_tJ_v(t)| < K_B·f(v,t)`" must appear explicitly; vague invocations like "the bandwidth constraint implies" without site-level operational unpacking fire F-d.
- **F-e.** No appeal to "standard relativistic-particle theory" or "the proper-time parameter is what makes the action reparametrization-invariant" or any "this is well-known from GR" move. F-e fires on the first such appeal.
- **F-f.** D6 closure check: each derivation step must cite which §4-catalog primitive it uses. Any step that does not cite a §4 primitive (or that cites a §4-excluded item) fires F-f.
- **F-g.** No re-invocation of FALSIFIED closure routes — specifically the "Born-Infeld with hidden GR import" pattern. If the derivation invokes the Born-Infeld action AND assumes the proper-time interpretation simultaneously (without deriving the interpretation), F-g fires.
- **F-h.** No comparison to measured Schwarzschild proper time or EIN-4 engine measurement before §9 step 10. Any earlier numerical comparison fires F-h.
- **F-i.** No look-elsewhere across candidate "clock-hypothesis-replacing" mechanisms. This pre-registration is scoped to **one** mechanism: bandwidth-constraint + manifestation-rate scaling. If the attempt switches mechanisms mid-derivation, F-i fires (the alternative requires its own v2 pre-reg).
- **F-j.** The SPEC §4.3 [THEOREM modulo clock hypothesis] derivation as currently written CANNOT be used as scaffold. The clock-hypothesis step must be derived FRESH from §4 primitives. Citing "per SPEC §4.3" as a derivation step fires F-j (citing for context or comparison at §9 step 10 is allowed).

---

## §8 — Banned moves / anti-laundering (LOCKED)

Process-level rules that go beyond §7 falsifiers. Any banned move invocation invalidates the closure attempt.

- **B-1.** No fitting of any proportionality constant after seeing the §5 benchmark form. The derivation prose must precede the numerical comparison; the order is locked in §9 method.
- **B-2.** No re-tagging of SPEC §4.3, SPEC §8 L-1, DERIV_NEWTON_FROM_SUBSTRATE.md §1.4, or LEDGER FTD-0131 before the result document (`REPORT_*` or `AUDIT_*_CLOSED_NEGATIVE.md`) lands. Tag changes are a result-document deliverable, not a pre-derivation move.
- **B-3.** No invocation of "ideal clocks" as a substrate primitive. The substrate has manifestation transitions (D4); it does not have ideal clocks. If the derivation introduces ideal clocks as a substrate object, B-3 fires.
- **B-4.** No appeal to operational definitions from outside FTD's axioms (e.g., "atomic clock frequencies", "GPS time", "cesium-133 hyperfine transitions"). Reference to such definitions for §9 step 10 sanity check only.
- **B-5.** No promotion of DERIV_NEWTON_FROM_SUBSTRATE.md `[POSTULATE 2]` tag (already reconciled to `[DERIVED via SPEC §4.3 modulo clock hypothesis]` per AUDIT 2026-05-24) until the result document lands. The current tag is the floor; promotion requires FOUND verdict.
- **B-6.** No use of "manifestly Lorentz-invariant" or "covariant" or similar dignifying terminology to smuggle in the proper-time identification without derivation. The clock hypothesis is precisely the step that connects substrate counts to relativistic invariants; calling it "manifestly invariant" is not a derivation, it's a re-statement.
- **B-7.** No appeal to "the clock hypothesis is standard" or "this is just how proper time works". The pre-registration's purpose is to test whether the clock hypothesis is substrate-derivable; treating it as standard is the answer the pre-reg is testing.
- **B-8.** No conflation of "substrate clock" (D4) with "proper-time clock". They are distinct concepts; whether they coincide is the closure question. Conflating them at any step fires B-8.

---

## §9 — Method (LOCKED) — 11 steps

1. **State substrate primitives** from §4 catalog explicitly, with site-level operational form for each (no vague invocations; F-d compliance).
2. **Define the substrate clock** (D4) as a counting process on manifested-site transitions; specify the rate at which transitions occur per universal tick T_U as a function of (v, ℒ).
3. **State the bandwidth constraint** `v < f` (D5) operationally; show explicitly how it constrains the per-voxel transition rate as a function of (v, ℒ).
4. **Derive the transition-rate scaling** as a function of (v, ℒ) from steps 1–3, using §4-catalog primitives only.
5. **Map the transition-rate scaling to a tick-rate scaling** (the rate at which substrate clock events occur per universal tick T_U, as a function of (v, ℒ)).
6. **Check that the tick-rate scaling reduces to `√(f - v²/f)` under specified limits.** Specifically: at v=0 it must reduce to `√f`; in flat space (f=1) it must reduce to `√(1-v²)`; in general it must produce `√(f - v²/f)`. The reduction must be a derived consequence, not a target-matching adjustment (F-b compliance).
7. **F-falsifier checklist (mechanical):** walk through F-a through F-j; for each, state explicitly whether it fired or not, with a one-sentence justification. Any single firing → Outcome B or C per §6.
8. **Banned-moves checklist (mechanical):** walk through B-1 through B-8; for each, state explicitly whether it was invoked or not. Any single invocation → invalidate attempt; v2 pre-reg required.
9. **Adversarial review checkpoint.** A separate reviewer (human or independent agent) must verify steps 1–8 without seeing the §5 benchmark or §9 step 10 numerical comparison. The reviewer's verdict is "PASS" or "FAIL"; FAIL invalidates the attempt.
10. **Numerical comparison to §5 benchmark.** Only after steps 1–9 close cleanly: compute the derived `dτ/dt(f, v)` and compare to `√(f - v²/f)` across a grid of (f, v) values (e.g., f ∈ {0.1, 0.5, 0.9}, v ∈ {0, 0.1·f, 0.5·f, 0.9·f}). Agreement must be exact (modulo numerical precision); approximate agreement only triggers Outcome B or C, not Outcome A.
11. **Verdict assignment per §6.** State explicitly which outcome (A, B, or C) the attempt landed, with the §7 + §8 checklists as justification. Result lands in `REPORT_CLOCK_HYPOTHESIS_DERIVATION.md` (FOUND or UNDERDETERMINED) or `AUDIT_CLOCK_HYPOTHESIS_CLOSED_NEGATIVE.md` (CLOSED-NEGATIVE), never as edits to this pre-registration.

---

## Closing note

This pre-registration is `[PRE-REGISTRATION]`-tagged. It contains no derivation, no tag promotion, no closure attempt. Its purpose is to lock the design BEFORE the attempt, so that whatever verdict lands is rigorous and F9-resistant.

**To hash-lock:** commit this file, compute SHA256, record in `../10_eft_program/REF_PREREGISTER_MANIFEST.md`, apply git tag `preregister-clock-hypothesis-derivation-v1`. Per CLAUDE.md commit policy, hash-lock + git tag operations require explicit user direction; this pre-registration is staged for that operation.
