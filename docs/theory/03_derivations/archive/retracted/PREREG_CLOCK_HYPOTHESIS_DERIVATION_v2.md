# Pre-Registration — Clock-Hypothesis Substrate-Derivation Attempt (v2)

**Tag:** `[PRE-REGISTRATION]` — locks the **design** of the v2 closure attempt against the clock hypothesis used implicitly in `SPEC_FTD_LAGRANGIAN.md` §4.3 [THEOREM]. Contains **no result**. All three pre-blessed outcomes (FOUND / UNDERDETERMINED / CLOSED-NEGATIVE) are admissible; verdict is genuinely open. **Prior-favoured outcome: FOUND** under the new Bandwidth-Internal-Time (budget-conservation) model.

**Date:** 2026-05-25
**Hash-lock target tag:** `preregister-clock-hypothesis-derivation-v2`
**LEDGER row:** FTD-0208
**Supersedes:** `PREREG_CLOCK_HYPOTHESIS_DERIVATION_v1.md` (commit `4c15ba1`, tag `preregister-clock-hypothesis-derivation-v1`) which was closed as `UNDERDETERMINED` per `AUDIT_CLOCK_HYPOTHESIS_v1_UNDERDETERMINED.md` (commit `8c8554d`).
**Companion docs:**
- [`DERIV_NEWTON_FROM_SUBSTRATE.md`](../../gravity_and_cosmology/DERIV_NEWTON_FROM_SUBSTRATE.md) §1.4 — flagged POSTULATE 2 (now reconciled to SPEC §4.3 modulo clock hypothesis)
- [`../01_reference/SPEC_FTD_LAGRANGIAN.md`](../01_reference/SPEC_FTD_LAGRANGIAN.md) §3.7 (bandwidth constraint) + §4.3 (Born-Infeld proper time) + §8 L-1 [THEOREM modulo clock hypothesis]
- [`AUDIT_NEWTON_POSTULATES_RECONCILIATION.md`](../../../07_assessment/audits/AUDIT_NEWTON_POSTULATES_RECONCILIATION.md) §2 (reconciliation that identified the clock hypothesis as the narrowed remaining open piece)
- [`AUDIT_CLOCK_HYPOTHESIS_v1_UNDERDETERMINED.md`](../../../07_assessment/audits/AUDIT_CLOCK_HYPOTHESIS_v1_UNDERDETERMINED.md) (records the v1 failure modes and reviews)
- Methodological templates: [`../10_eft_program/PREREG_ALPHA_READOUT_OBSERVABLE_SELECTION_v1.md`](../10_eft_program/PREREG_ALPHA_READOUT_OBSERVABLE_SELECTION_v1.md)

> **Pre-registration discipline.** Sections §§2–9 are committed before the closure attempt is run. After commit: SHA256 → `../10_eft_program/REF_PREREGISTER_MANIFEST.md`, git tag applied. Any post-hoc edit to §§2–9 invalidates v2; a v3 is required before the closure attempt is run or re-run. The closure attempt's result lands in a separate doc (`FOUND_CLOCK_HYPOTHESIS.md` or `AUDIT_CLOCK_HYPOTHESIS_v2_UNDERDETERMINED.md`), never as edits to this file.

**Purpose.** Lock, *before* any closure-attempt construction, the design of the v2 clock hypothesis attempt. We address the v1 audit's findings by:
1. **Expanding the §4 search space** to include the calibration-declaration (`FTD-0041`) and the bandwidth-internal-time (`SPEC §3.7`) routes.
2. **Sharpening the `F-d` falsifier** to catch vague dimensional transition scaling ($S_{\text{trans}} \sim K_B \cdot \text{constant}$ or similar asymptotic scaling placeholders).
3. **Mandating first-principles operational derivation** of any transition-rate or budget-conservation mechanics.

This pre-registration acts as the anti-laundering instrument for an attempt where the target value ($\sqrt{f - v^2/f}$ for proper-time scaling) is a canonical GR result and the temptation to engineer toward it is high (F9 collusion-bias risk: HIGH).

---

## §2 — The question (LOCKED)

**Q-CH-2.** Does the clock hypothesis — the identification
> "the Born-Infeld action measure $dS = -K_B \sqrt{(f^2-v^2)/f} dt$ defines proper time as $d\tau \propto \sqrt{(f^2-v^2)/f} dt$"
— derive from FTD substrate primitives (specifically: the bandwidth constraint $v < f$ of SPEC §3.7 + the calibration-declaration `FTD-0041` + the quadratic budget-conservation law of internal vs. spatial voxel updates) **without invoking** standard relativistic-particle theory's empirical clock postulate ("ideal clocks measure proper time along their worldline") or any GR-imported clock-rule input?

The derivation must:
1. Use only primitives from the §4 frozen catalog;
2. Produce $d\tau/dt = \sqrt{f - v^2/f}$ as a derived consequence, not as a definitional input;
3. Pass the §7 falsifier ruleset mechanically;
4. Pass the §8 banned-moves checklist mechanically;
5. Reach the numerical comparison (§9 step 10) only after steps 1–9 close cleanly.

All three §6 outcomes are pre-blessed. The verdict is genuinely open.

---

## §3 — Definitions (LOCKED)

- **D1 — Clock hypothesis.** Identification of the action-measure $\sqrt{(f^2-v^2)/f} dt$ (units of time × dimensionless) with proper time $d\tau$ (units of time), via the proportionality $d\tau \propto \sqrt{(f^2-v^2)/f} dt$ with proportionality constant 1 in $c=1$ units. Both parts (the action-measure as time and the worldline match) must be derived.
- **D2 — Substrate primitives.** The §4 frozen catalog, drawn from SPEC §3.7, the substrate manifestation rule (threshold-crossing), the engine tick mechanism (universal discrete tick $T_U$), and the Born-Infeld action measure as a Lagrangian density (NOT yet identified with time).
- **D3 — Operational tick.** The engine's discrete universal tick $T_U$. One tick is calibrated to $\sqrt{3}\ell_P/c$ per FTD-0041; this calibration must be used consistently.
- **D4 — Substrate clock.** A counting process on manifested-site transitions: each transition is one substrate-clock event. The clock rate is the rate at which transitions occur per universal tick.
- **D5 — Bandwidth constraint.** $v < f$ per SPEC §3.7 with $v = |\Delta_t \mathbf{J}|/K_B$ and $f = 1 - \mathcal{L}^2$.
- **D6 — Local speed limit.** The maximum allowed coordinate velocity of a voxel at latency $\mathcal{L}$, defined as $f = 1 - \mathcal{L}^2$. The velocity relative to this local speed limit is the local velocity $v_{\text{local}} = v/f$.
- **D7 — Local time tick.** The coordinate time tick of a stationary observer at latency $\mathcal{L}$, defined as $dt_{\text{local}} = \sqrt{f} dt$. This represents the static gravitational time dilation derived via the Poisson field equation of SPEC §4.2.
- **D8 — Closure (operational).** A chain of derivation steps, each tagged with epistemic status, showing that $d\tau/dt = \sqrt{f - v^2/f}$ follows from §4 catalog primitives without invoking "ideal clocks measure proper time". Each step must cite which §4 primitive it uses and pass the §7 falsifier check.

---

## §4 — Admissible search space (LOCKED)

The closure attempt may use ONLY the following primitives (frozen 2026-05-25 at v2 hash-lock):

1. **SPEC §3.7 bandwidth constraint** $v < f$, $v = |\Delta_t \mathbf{J}|/K_B$, $f = 1 - \mathcal{L}^2$.
2. **Born-Infeld action measure as Lagrangian density** $S = -K_B \int \sqrt{(f^2-v^2)/f} dt$ per SPEC §3.3.
3. **Substrate manifestation rate** — rate at which threshold-crossing events occur per universal tick $T_U$.
4. **Engine tick $T_U$** — universal discrete tick advancing all sites in lockstep; calibrated to $\sqrt{3}\ell_P/c$ per FTD-0041 / FTD-0096.
5. **D4 substrate clock** — counting process on manifested-site transitions.
6. **D6 Local speed limit and D7 Local time tick** — derived from SPEC §4.2 Poisson equation.
7. **Bandwidth budget conservation** — the quadratic relation representing the division of the voxel's update budget per local tick $dt_{\text{local}}$ between internal transitions ($d\tau$) and spatial motion ($v_{\text{local}}$):
   $$\left(\frac{d\tau}{dt_{\text{local}}}\right)^2 + v_{\text{local}}^2 = 1$$
   This represents an $L^2$-norm conservation of orthogonal degrees of freedom in the ternary state space.
8. **FTD axioms 1–5** from `SPEC_FTD.md`.
9. **Algebraic spine theorems** as cited tools.

**Explicitly EXCLUDED from §4 catalog:**
- GR's empirical clock postulate ("ideal clocks measure proper time along their worldline").
- "The proper-time parameter is what makes the action reparametrization-invariant" (standard relativistic-particle-theory move).
- Postulating $d\tau \propto \sqrt{(f^2-v^2)/f} dt$ directly (this IS the question).
- Schwarzschild metric $ds^2 = f c^2 dt^2 - dr^2/f - r^2 d\Omega^2$ (the target).
- Any continuum-spacetime metric formalism ($g_{\mu\nu}$, $h_{\mu\nu}$) as a derivation primitive.

---

## §5 — Benchmark (LOCKED)

**Benchmark form:**
$$d\tau/dt = \sqrt{f - v^2/f}$$
for all $f \in (0,1]$ and $v \in [0, f)$, derived from §4 primitives without invoking §4-excluded items.

**Benchmark sanity check (numerical):** The engine's `test_einstein_equations.cpp` EIN-4 (gravitational time dilation $0.004\%$ match in the Schwarzschild scalar sector) is consistent with the SPEC §4.3 form.

**Benchmark scope:** The $v=0$ case $d\tau/dt = \sqrt{f}$ (gravitational time dilation, SPEC §5.2 [THEOREM]) and the $v \neq 0$ case $d\tau/dt = \sqrt{f - v^2/f}$ (full Schwarzschild proper time, SPEC §4.3 [THEOREM]).

---

## §6 — Three pre-registered outcomes (LOCKED)

* **Outcome A (FOUND).** A derivation chain exists, each step tagged with epistemic status, using only §4-catalog primitives, that produces $d\tau/dt = \sqrt{f - v^2/f}$ as a derived consequence of substrate primitives. All §7 falsifiers must be checked as not firing; all §8 banned moves must be checked as not invoked. Adversarial review (§9 step 9) must agree. **Tag consequences:** SPEC §4.3 promoted from `[THEOREM modulo clock hypothesis]` to fully `[THEOREM]`; SPEC §8 L-1 promoted similarly; `DERIV_NEWTON_FROM_SUBSTRATE.md` §1.4 reconciliation note updated; LEDGER FTD-0131 row promoted to `[DERIVED]`; `AUDIT_NEWTON_POSTULATES_RECONCILIATION.md` and LEDGER `FTD-0208` updated to reflect positive closure. Plan v2 Arc B P5 marked CLOSED with FOUND verdict.
* **Outcome B (UNDERDETERMINED).** A derivation chain reaches $d\tau/dt = \sqrt{f - v^2/f}$ but requires an intermediate principle outside the §4 catalog — for example, a finite-trace mechanics axiom, a graph spectral curvature principle, or another doctrine §12 candidate principle — and that principle has not been independently substrate-derived. **Tag consequences:** SPEC §4.3 status unchanged (`[THEOREM modulo clock hypothesis]`); v3 pre-registration required. Plan v2 Arc B P5 marked PARTIAL.
* **Outcome C (CLOSED-NEGATIVE).** No derivation chain from §4 primitives produces $d\tau/dt = \sqrt{f - v^2/f}$ without F-falsifier firing or banned-move invocation. The clock hypothesis is established as irreducibly interpretive within the §4 catalog. **Tag consequences:** SPEC §4.3 + §8 L-1 explicitly tagged `[THEOREM, conditional on clock hypothesis AXIOM]`; the clock hypothesis itself receives an explicit `[AXIOM]` tag. Plan v2 Arc B P5 marked CLOSED with CLOSED-NEGATIVE verdict.

---

## §7 — Falsifier rules (LOCKED) — F-a..F-j

Mechanical falsifier checklist. Any single firing → at most Outcome B or Outcome C; not Outcome A.

- **F-a.** No insertion of "ideal clocks measure proper time" or equivalent GR clock postulate at any step. If the derivation prose contains the words "ideal clock", "proper-time parameter", "reparametrization-invariant action measure", or equivalent, F-a fires.
- **F-b.** No insertion of $d\tau = \sqrt{(f^2-v^2)/f} dt$ or $d\tau/dt = \sqrt{f - v^2/f}$ before the derivation chain produces it. The first appearance of this form must be a derived line, not a definitional line.
- **F-c.** No free parameter introduced. The derivation may use only constants from the algebraic spine + ontic chain; any proportionality constant must be either =1 (in c=1 units) or derived from §4 primitives.
- **F-d.** D5 bandwidth-constraint dependency must be operationally specified. Vague invocations like "the bandwidth constraint implies" or dimensional transition scaling $S_{\text{trans}} \sim K_B \cdot \text{constant}$ without site-level operational unpacking or first-principles derivation fire F-d. Any transitional action quantum used must be derived operationally.
- **F-e.** No appeal to "standard relativistic-particle theory" or "the proper-time parameter is what makes the action reparametrization-invariant" or any "this is well-known from GR" move. F-e fires on the first such appeal.
- **F-f.** D8 closure check: each derivation step must cite which §4-catalog primitive it uses. Any step that does not cite a §4 primitive (or that cites a §4-excluded item) fires F-f.
- **F-g.** No re-invocation of FALSIFIED closure routes. If the derivation invokes the Born-Infeld action AND assumes the proper-time interpretation simultaneously (without deriving the interpretation), F-g fires.
- **F-h.** No comparison to measured Schwarzschild proper time or EIN-4 engine measurement before §9 step 10. Any earlier numerical comparison fires F-h.
- **F-i.** No look-elsewhere across candidate "clock-hypothesis-replacing" mechanisms.
- **F-j.** The SPEC §4.3 [THEOREM modulo clock hypothesis] derivation as currently written CANNOT be used as scaffold. The clock-hypothesis step must be derived FRESH from §4 primitives. Citing "per SPEC §4.3" as a derivation step fires F-j.

---

## §8 — Banned moves / anti-laundering (LOCKED)

Process-level rules that go beyond §7 falsifiers. Any banned move invocation invalidates the closure attempt.

- **B-1.** No fitting of any proportionality constant after seeing the §5 benchmark form. The derivation prose must precede the numerical comparison.
- **B-2.** No re-tagging of SPEC §4.3, SPEC §8 L-1, DERIV_NEWTON_FROM_SUBSTRATE.md §1.4, or LEDGER FTD-0131 before the result document lands.
- **B-3.** No invocation of "ideal clocks" as a substrate primitive.
- **B-4.** No appeal to operational definitions from outside FTD's axioms.
- **B-5.** No promotion of DERIV_NEWTON_FROM_SUBSTRATE.md `[POSTULATE 2]` tag until the result document lands.
- **B-6.** No use of "manifestly Lorentz-invariant" or "covariant" or similar dignifying terminology to smuggle in the proper-time identification without derivation.
- **B-7.** No appeal to "the clock hypothesis is standard" or "this is just how proper time works".
- **B-8.** No conflation of "substrate clock" (D4) with "proper-time clock". They are distinct concepts; whether they coincide is the closure question.

---

## §9 — Method (LOCKED) — 11 steps

1. **State substrate primitives** from §4 catalog explicitly, with site-level operational form for each (F-d compliance).
2. **Define the local speed limit** $f = 1 - \mathcal{L}^2$ and **local velocity** $v_{\text{local}} = v/f$ (D6), and the **local time tick** $dt_{\text{local}} = \sqrt{f} dt$ (D7).
3. **State the bandwidth budget conservation law** (D5 + §4.7):
   $$\left(\frac{d\tau}{dt_{\text{local}}}\right)^2 + v_{\text{local}}^2 = 1$$
   Show how this follows from the quadratic division of the voxel's update capacity between orthogonal internal transitions ($d\tau$) and spatial translation ($v_{\text{local}}$).
4. **Derive the tick-rate scaling** $d\tau$ as a function of $(f, v, dt)$ by algebraically solving the budget-conservation relation, using §4-catalog primitives only.
5. **Check that the tick-rate scaling reduces to $\sqrt{f - v^2/f}$ under specified limits.** The reduction must be a derived consequence, not a target-matching adjustment (F-b compliance).
6. **Integrate the Born-Infeld core** action-measure with $d\tau$, proving that the Lagrangian energy scaling coincides with the clock's tick rate.
7. **F-falsifier checklist (mechanical):** walk through F-a through F-j; for each, state explicitly whether it fired or not, with a one-sentence justification. Any single firing → Outcome B or C per §6.
8. **Banned-moves checklist (mechanical):** walk through B-1 through B-8; for each, state explicitly whether it was invoked or not. Any single invocation → invalidate attempt; v3 pre-reg required.
9. **Adversarial review checkpoint.** A separate reviewer (independent agent) must verify steps 1–8 without seeing the §5 benchmark or §9 step 10 numerical comparison. The reviewer's verdict is "PASS" or "FAIL"; FAIL invalidates the attempt.
10. **Numerical comparison to §5 benchmark.** Compute the derived $d\tau/dt(f, v)$ and compare to $\sqrt{f - v^2/f}$ across a grid of $(f, v)$ values. Agreement must be exact (modulo numerical precision).
11. **Verdict assignment per §6.** State explicitly which outcome (A, B, or C) the attempt landed, with the §7 + §8 checklists as justification. Result lands in `FOUND_CLOCK_HYPOTHESIS.md` or `AUDIT_CLOCK_HYPOTHESIS_v2_UNDERDETERMINED.md`.

---

## Closing note

This pre-registration is `[PRE-REGISTRATION]`-tagged. It contains no derivation, no tag promotion, no closure attempt. Its purpose is to lock the design BEFORE the attempt, so that whatever verdict lands is rigorous and F9-resistant.

**To hash-lock:** commit this file, compute SHA256, record in `../10_eft_program/REF_PREREGISTER_MANIFEST.md`, apply git tag `preregister-clock-hypothesis-derivation-v2`.
