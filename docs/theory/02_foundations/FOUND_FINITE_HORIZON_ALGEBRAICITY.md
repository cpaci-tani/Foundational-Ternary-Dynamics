# FOUND — Lemma 0: finite-horizon algebraicity of the default substrate (δ-independence program S1)

**Tag:** [DERIVED — schema-level, with the completeness flag of the charter's S1 gate stated in §3] for the lemma; [coherent-interpretation] for §4's readings.
**LEDGER id:** FTD-0368 (Stage S1 deliverable; no new id — program-internal, per the charter).
**Verification:** `scripts/proofs/proof_lemma0_finite_horizon.py` (schema-level, exact arithmetic throughout — zero floats).
**Audience:** project owner + agents executing FTD-0368 stages S2/S3, or anyone asking "can the finite dynamics produce a transcendental?"

---

## §1 — Statement

**Parameter field.** Let k₀ be the field generated over ℚ by the spec's declared calibration symbols treated as **independent indeterminates** — g_c (state-flux coupling), α (force coupling), G_N, K_GENESIS, K_B, dt — together with the forced algebraic constants (c² = 1/3 exactly; C_SPEED = 1/√3, algebraic of degree 2). Treating the couplings as indeterminates is not pedantry; it is the engine/theory separation made algebraic: *the ontology does not natively assign these values* (that assignment is what an import is), so the native closure is computed over the generic fiber.

> **Lemma 0 (finite-horizon algebraicity).** Fix a lattice size L and a finite tick horizon T. Every rule of the default Scale-0 substrate — the six core rules of `engine/SPEC_ENGINE.md` §1 and the three default-ON promoted toggles (`dual_substrate`, `selective_damping`, `weak_transmutation`) — is a piecewise-polynomial (semi-algebraic) map with coefficients in k₀. Hence the T-tick evolution map is semi-algebraic over k₀, and **every finite-horizon native observable is algebraic over k₀(initial data)**. With algebraic parameter assignments and algebraic initial data, every finite-horizon native constant is an algebraic number.

**Corollary 1 (transcendence inertness).** No finite-horizon native computation produces any transcendental — not G\*, not π, not δ. All transcendental content in the corpus's native outputs is **limit-borne**. (The corpus's own finite-L data concurs: the finite-L Watson-type values are exact rationals, e.g. W₂^BCC = 1/4; the L→∞ limits are where I₁ = G\*²/(2π) lives.)

**Corollary 2 (the wall factorizes).** The δ-IND question decomposes into exactly two policies, both now cleanly separated from the dynamics:
- the **admissible-limit policy** — which transcendence classes the ontology's own ε-L limits generate (the FTD-0353/0360 valuation theorem's territory: G\*-class yes, δ-class no, relative to the documented inventory); and
- the **parameter-assignment policy** — coupling symbols are generic natively; assigning one a value *is* an import (MC-T4.3's content restated algebraically).

The finite dynamics contributes nothing to either. The α-wall is not a property of what the substrate *does*; it is a property of what the substrate is allowed to *converge to* and to *be handed*.

## §2 — Proof

Rule by rule, at the spec's own level of description (`SPEC_ENGINE.md` §1, quoted forms):

| # | rule (spec form) | map class | coefficients | verified |
|---|---|---|---|---|
| 1 | flux wave: dJ/dt = c²∇²J | linear (discrete Laplacian) | ℚ (c² = 1/3) | L1 |
| 2 | state-flux coupling: g_c·grad(s) + g_c·curl(s·v) | linear + bilinear | ℚ(g_c) | L1 |
| 3 | Gauss projection: enforce div J = s | exact linear solve + linear subtraction; the torus projector is a rational-linear map (compatibility handled by the zero-mode convention; the residue is the constant mean) | ℚ | L2 |
| 4 | manifestation/evaporation: \|J\| > K_GENESIS; nbhd energy < K_B²·10⁻⁶ | semi-algebraic case-split — both thresholds are polynomial inequalities after squaring (\|J\| > K ⟺ J·J > K² on the positive branch); branch maps are alphabet updates | ℚ(K_GENESIS, K_B) | L3 |
| 5 | forces: F = −α·s·grad(φ_C) + G_N·grad(ρ) + α·s·(v×B), B = curl J | linear solves (φ_C) + gradients/curls (linear) + cross product (bilinear) | ℚ(α, G_N) | L4 |
| 6 | movement + collision: remainder accumulation, speed limit C_SPEED = 1/√3, annihilation on contact | rational bookkeeping + integer carry; the clamp v ↦ v·C_SPEED/\|v\| introduces **quadratic surds** — outputs leave ℚ but remain algebraic (explicit minimal polynomial exhibited); annihilation is an alphabet rule | ℚ(√3)(inputs) | L5 |
| 7b′ | `dual_substrate` (default ON) | second linear wave copy | ℚ(g_c) | L6 |
| — | `selective_damping` (default ON) | condition-gated linear scaling (1 − λ·dt) | ℚ(λ, dt) | L6 |
| — | `weak_transmutation` (default ON) | polynomial-threshold-gated sign flip s ↦ ±s | ℚ | L6 |

Composition: a finite composition of piecewise-polynomial maps with k₀-coefficients is semi-algebraic over k₀ (finitely many branch cells per tick, polynomial on each; T ticks give finitely many cells). Algebraicity of outputs over k₀(inputs) follows because semi-algebraic functions are algebraic on each cell. The one place outputs genuinely leave the rational field — the rule-6 clamp — is exhibited exactly: |v_clamped|² = 1/3 verified symbolically, with the clamped component's minimal polynomial over ℚ computed (degree 2). A composed toy tick (rules 1–4) on the 3³ torus with rational data runs in exact arithmetic end-to-end: all flux components and potentials come out exact rationals, all states ternary (L7). ∎ [DERIVED — schema-level]

Two toy-model honesty notes, preserved because both are instructive and both reproduce corpus-documented phenomena in a 27-site toy:

1. **L = 2 degeneracy.** The first draft ran the composed tick at L = 2, where the residue check *failed*: at L = 2 the ±1 neighbors coincide and central differences vanish identically. L = 3 is the minimal non-degenerate torus. Finite-lattice degeneracies are exactly what the ε-L discipline exists to keep visible.
2. **The matched-stencil requirement.** The second draft solved the *compact* (−6/+1) Laplacian and subtracted a central-difference gradient — and the residue check failed again, because central-difference div∘grad is **not** the compact Laplacian (it is the quarter-weighted stride-2 operator; at L = 3 it equals lap/4). The projector must be built from the same div/grad it is asked to cancel. This is precisely the stencil-mismatch phenomenon the corpus already documents at engine scale — the Phase-F matched-stencil CG Poisson lesson (Ward floor 1% → 1e-8) and the FTD-0363 E2 postmortem (a central-difference divergence measured against a compact-stencil projector disagreeing at a fixed O(10⁻¹) floor). The verifier now uses the matched operator (`matched_poisson_matrix()`), and the residue cancels exactly. That the toy *forced* the same lesson the engine learned empirically is itself small evidence that the schema implementation is faithful in form.

## §2.1 — A0-audit addendum (2026-07-05): enumeration extended

The Clause-2/3 program's adversarial audit (finding m-1) identified default-ON machinery outside the original nine-row table: the core **`damping`** toggle (`term_toggles.h` default true; `phase_write` applies flux *= (1 − DAMPING) — a linear scaling, same map class; its constant and λ_d now belong to k₀), and the genesis rule's **per-(site,tick) deterministic draw** (field-independent constants — algebraicity unaffected; enumerated for completeness). Finding m-2's partially-vacuous weak_transmutation check was replaced by a real per-branch verification. The lemma's conclusion is unchanged; the enumeration and k₀ are now: k₀ = ℚ(g_c, α, G_N, K_GENESIS, K_B, dt, DAMPING, λ_d) plus the forced algebraic constants. The verifier reflects both repairs.

## §3 — The completeness flag (charter S1 gate, stated not hidden)

The enumeration above is complete **relative to `SPEC_ENGINE.md` §1's core-rule list plus its named default-ON toggle set**, at the spec's level of description. Two scoping consequences: (i) toggle-gated extensions (default OFF) are outside the lemma's scope — each is, on inspection, the same map classes (thresholds, linear/bilinear terms), but no completeness is claimed for them; (ii) if the spec's rule list is later amended, the lemma inherits the amendment as a proof obligation, not silently. This flag is the lemma's analog of the valuation theorem's inventory-[SELECTION] — narrower (a spec section, not a corpus survey), but of the same honest kind.

## §4 — Readings `[coherent-interpretation]`

- **The infinity-reframe becomes load-bearing.** `AUDIT_INFINITY_REFRAME.md` demanded ε-L restatements as hygiene; Lemma 0 makes the limit policy the *entire habitat* of the framework's hardest question. Which limits the ontology owns is no longer a stylistic discipline — it is where δ-IND will be decided (S2's definition fight).
- **The two doors, named.** MC-T4.3's obstruction now has an algebraic shape: nothing the substrate *does* (finite dynamics) reaches transcendence; only what it *converges to* (limits) and what it *is handed* (parameter assignments) can. The carrier closures (FTD-0244/0314/0326/0327/0341) all live at door one or door two; none needed to inspect the dynamics — Lemma 0 explains why that was never an omission.
- **Gödel-echo, one line and no more:** the finite-horizon stratum is the "computation-checkable" fragment where everything is decided internally; independence phenomena can only begin where the infinite enters. The analogy remains motivation, per the charter's guard 1.
- **Guard 6 inherited verbatim:** Lemma 0 classifies *where* transcendental content can sit — it does not explain *why* the limit policy admits G\* but (conjecturally) not δ. That "why" is the program's remaining question, untouched here.

## §5 — Cross-references

- `SCOPE_DELTA_INDEPENDENCE_PROGRAM.md` (FTD-0368) — the charter this discharges Stage S1 of; next stage S2 (the definition fight: N1 = N0 + effective ε-L limits, pre-registered before δ is tested).
- `engine/SPEC_ENGINE.md` §1 — the canonical rule list the enumeration is relative to.
- `docs/theory/07_assessment/framework_postulates_constitution/AUDIT_INFINITY_REFRAME.md` — the limit-policy vocabulary now load-bearing.
- `docs/theory/09_mathematical/number_theory/THEOREM_VALUATION_4GSTAR_MINUS_1.md` (FTD-0353/0360) — door one's current best result.
- `docs/theory/01_reference/SPEC_OPEN_MATH_BY_SECTOR.md` MC-T4.3 — door two's canonical statement.
- `scripts/proofs/proof_lemma0_finite_horizon.py` — the schema verifier (exact arithmetic, zero floats).

---

*Lemma 0 moves no tag: MC-T4.3 [FOUNDATIONAL OBSTRUCTION], x₊ = 1/α [SMC], FC-W [AXIOM] all stand. What changed is the shape of the battlefield: the finite dynamics is out of the fight.*
