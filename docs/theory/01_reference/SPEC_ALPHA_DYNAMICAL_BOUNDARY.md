# SPEC — α is dynamical, not structural (the MC-T4.3 boundary, canonical verdict)

**Tag:** `[SYNTHESIS / FOUNDATIONAL OBSTRUCTION — ACCEPTED BOUNDARY]` — consolidation of established results; introduces NO new claim and promotes nothing.
**Date:** 2026-06-22
**Scope:** the single canonical statement of the α-derivation question. The α-readout program is large (20+ docs under `10_eft_program/` + `07_assessment/audits/`); this is the one-page final verdict + the map over it. Precedence: **LEDGER > this doc > other prose.**

---

## 0 · The verdict

The master quadratic `x² − 16G*²x + 16G*³ = 0` is a `[THEOREM]` (FTD-0001); its physical root `x₊ = 137.0362…` matches CODATA `1/α` to **1.26 ppm**. But the **identification `x₊ = 1/α` is `[STRONGLY MOTIVATED CONJECTURE]`** (FTD-0013), **not a derivation**. After the exhaustion of every FTD-native route, the honest verdict is:

> **α is DYNAMICAL, not STRUCTURAL.** The discrete ontology (P1–P5) does not, by any examined route, fix the electromagnetic coupling. Closing the gap requires an *external selection* (a 6th-postulate-class commitment) — admitting which would make `x₊ = 1/α` `[SELECTED / CONDITIONAL THEOREM]`, never `[DERIVED]` from the bare substrate.

This is a **Number-One-Goal boundary result** (clause 2): a rigorously-mapped limit of what discreteness determines. **MC-T4.3 stays `[FOUNDATIONAL OBSTRUCTION]`.**

---

## 1 · Why it is route-invariant (the three theorem-grade closures)

| Result | Statement | Tag | Source |
|---|---|---|---|
| **FTD-0242** (route-invariance) | **0/4** FTD-native routes (J-twisted ζ-determinant, BCC body-diagonal transfer, lemniscatic-CM arithmetic, variational/valuation/Hodge) force the master-quadratic operator assembly `(Tr, Det) = (16G*², 16G*³)`. The **trace `16G*²` is forward-forced** (clean odd source from the determinant ratio); the **determinant assembly is NOT forced** (Tr and Det are independent invariants for a 2×2 readout). α is dynamical, not structural. | `[STRONGLY MOTIVATED CONJECTURE no-go / BOUNDARY]` | `07_assessment/audits/AUDIT_ALPHA_OPERATOR_FORCING_ROUTE_INVARIANCE.md` |
| **FTD-0243** (RSI Leg-3 conditional theorem) | The forcing functional `𝔉` **does not force α unless** an external W natively realizes `√(G*(4G*−1))`. The reduction is **route-invariant**: `Q(G*)` is the Galois-fixed field; forward-forced symmetric data is blind to which root is `1/α`. Choosing W is a **6th-postulate-class input**, not a derivation. | `[THEOREM]` (conditional) | `07_assessment/audits/AUDIT_RSI_LEG3_CONDITIONAL_THEOREM.md` |
| **FTD-0244** (K-BIND) | The substrate-native operator-construction calculus `𝔉` is axiomatized; **trace and determinant of any operator in `𝔉` lie in `Q(G*)`**, and the master quadratic's splitting field is a degree-2 extension ⇒ **no native operator forces the assembly without the external selection W**. K-BIND closed **theorem-negative**. | `[CLOSED THEOREM-NEGATIVE]` | `10_eft_program/derivations/FOUND_OPERATOR_CALCULUS_AXIOMATIZATION.md` + `scripts/proofs/proof_k_bind_axiomatization.py` |

**Sharpest narrowing (FTD-0284, the D=3 forced-escape):** the factor `16 = |ℤ[i]^×|²` can enter the 2D readout two ways — **(A) as a complex structure J: `[CLOSED NEGATIVE]`** by a general elliptic/hyperbolic incompatibility (the master quadratic is hyperbolic, Det < Tr²/4; complex structures are elliptic — they cannot meet on one real 2D readout); **(B) as a bare integer coefficient: `[UNDERDETERMINED]`** (W-CRIT-2 — with no module structure the determinant is a free invariant). The entire surviving escape is provably pinned to branch (B) + one un-covered infinite-descended transfer/monodromy sub-branch.

---

## 2 · The closed-negative route ledger

All of the following are `[CLOSED NEGATIVE]` (preserved for provenance to prevent zombie re-attempts): R1/R2/R3/R4, Z-factor (FTD-0116), RG-running, algebraic combinations, 1/√d, Langevin-equipartition, **ARC-A** boundary readout (FTD-0214), **ARC-B1** observable-selection (FTD-0205), **ARC-B2/C1** BCC-bridge/quantization (FTD-0224, FOUND→UNDERDETERMINED corrected), **K-BIND** (FTD-0244), **ARC-D1** engine-native fission-rate (FTD-0224), the deterministic oscillatory-cloud Floquet readout (2026-06-15). FTD-0050 (master quadratic as RG-step characteristic polynomial) closed-negative (engine stencil is (SC+FCC)/2, BCC-orthogonal).

---

## 3 · The two surviving exits (neither derives α from P1–P5)

1. **A 6th W-postulate** that natively realizes `√(G*(4G*−1))` and forces the determinant's odd-exponent grading `16G*³`. Admitting it upgrades `x₊ = 1/α` to `[SELECTED / CONDITIONAL THEOREM]` — and **concedes that α is not derivable from the bare substrate**. This is the W-CRIT-2 residual of FTD-0284 branch (B).
2. **A fresh ARC-D engine-native measurement** — a cluster-interaction / lifetime / spectrum observable measured L-independently and compared to lab α. **ARC-D1 already `[CLOSED NEGATIVE]`**; remaining candidates unmeasured. (Note: the engine's own α-readout is itself engine-emergent — `SPEC_EFT_RECOVERY_PROGRAM.md` Phase-G; FTD-0309 is the analogous "no scalar reduction" result for the cluster-mass calibration.)

**Neither is in scope for the consolidation program** — they are registered, not chased.

### 3.1 · The BCC-stencil sub-route, closed (FTD-0313 — the geometric anatomy of the obstruction)

A 2026-06-22 adversarial deep dive (4-agent workflow, all grounded claims survived refutation) sharpened *why* exit (1) is a selection, via the lattice **Green's function**:
- **G\* IS a pure body-diagonal (BCC-corner) return value** `[THEOREM]`: `G* = √(2π·G_BCC(0))`, `G_BCC(0) = Γ(1/4)⁴/(4π³) = 1.3932` is G_BCC(0) — the BCC return Green's function at the origin for the triple-product operator `D = 1 − cx·cy·cz` (independently Richardson-extrapolated to 1.393204). (Note: this 1.3932 is NOT the standard SC Watson self-energy, which is a different number ≈0.5054.) The master quadratic is then **pure-BCC-sublattice algebra**: G\* (from the BCC return Green's function at the origin) + `16 = |ℤ[i]^×|²` (the BCC corners' Z₄ automorphism count).
- **But EM does not propagate there.** The engine's dynamical wave operator is the **18-pt (SC+FCC) Laplacian**, **variationally derived** with the 8 BCC corners at weight **zero** (forced by leading-order isotropy; `DERIV_18PT_LAPLACIAN_VARIATIONAL.md`). Only the *pure* body-diagonal symbol yields a lemniscatic constant — SC+FCC and the full 26-Moore do not. So G\*'s sublattice is **orthogonal to the EM-propagation sublattice** (FTD-0050 RG-step road; FTD-0079 no exact (SC+FCC)/2↔BCC identity).
- **Verdict `[SELECTION + THEOREM-NEGATIVE]`:** routing the physical EM kinetic operator onto the pure-BCC sublattice is a **6th-postulate-class selection, not forced** — three independent grounds: (i) the wave operator is already variationally fixed as SC+FCC; (ii) P1–P5 do not pick a sub-stencil; (iii) **even granting the BCC selection, the assembly is still unforced** — the roots require `√(G*(4G*−1))` with `4t²−t` squarefree, so the surd lies in a genuine **degree-2 extension of Q(G\*)** that no native operator supplies (FTD-0244 K-BIND; a symmetric-2×2 natural-element scan returned 0 hits — only the hand-built companion form realizes it). This is the **third road** (after FTD-0050's RG-step road) into the same W-CRIT-2 / branch-(B) boundary; it dies on the same algebraic wall, route-invariantly. No computation (including a σ_BCC spectrum measurement) can flip it — only the W-postulate of exit (1) can, and that is a new axiom, not a derivation.

### 3.2 · Exit (1) precisely pinned, then ADOPTED — the carrier-narrowing theorem (FTD-0314) + FC-W (FTD-0315)

A 2026-06-23 pre-registered adversarial attack (7-agent workflow; `scripts/proofs/proof_w_carrier_narrowing.py` 11/11 PASS at dps=150) asked whether exit (1)'s W can be **earned** from native structure rather than postulated. Verdict: **W cannot be earned natively (~85% CLOSED)** — and the failure is a new `[THEOREM]` ([`AUDIT_W_CARRIER_NARROWING.md`](../07_assessment/audits/AUDIT_W_CARRIER_NARROWING.md)):

- **The narrowing theorem `[THEOREM]` (conditional on Chudnovsky):** the distinguishing surd `√(G*(4G*−1))` is **transcendental over ℚ** (G\* transcendental ⇒ `Q(G*)∩Q^ab=Q`; the surd is degree-2 over `Q(G*)`). So **every** carrier with algebraic invariants is excluded — chirality, the ±1 ternary sign, the binary-octahedral 2O double cover, permutation parity, and every native operator (Tr/Det ∈ `Q(G*)`, FTD-0244). This **extends K-BIND from operators to the whole finite-symmetry class** and explains it geometrically (the transcendence gap between `Q(G*)` and the surd's degree-2 extension). The only door: a forced ℤ/2 **twist on a G\*-bearing analytic carrier**.
- **The three natural analytic carriers close `[THEOREM]`:** the BCC-Watson twist **degenerates** (`G_odd = G_even` exactly — odd-n angular integrals vanish); a second Watson integral is moot (`4G*−1 ∈ Q(G*)`, PSLQ `[1,−4,1]`); the CM period/L-value route stays inside `F = Q̄(π, Γ(1/4))` (the surd's square is degree-1 in π ⇒ outside F). One loophole — a *new* forward-derived transcendental period — survives `[OPEN]` but leans CLOSED (pressured by the surd's motivic **weight-inhomogeneity**) and cannot be opened without the banned W-CRIT-2 planting.
- **Consequence — FC-W (FTD-0315), the disciplined "6th postulate":** because the narrowing theorem *pins exactly* what an external W must be and *proves no cheaper object suffices*, the constitution now **declares W** as the `[AXIOM]`-class Framework Commitment FC-W ([`SPEC_FTD_FRAMEWORK_V1.md`](SPEC_FTD_FRAMEWORK_V1.md) §3.5). Under FC-W, `x₊ = 1/α` is a `[CONDITIONAL THEOREM given W]`, explicitly **not** `[DERIVED]`; FC-W is the framework's first *adopted* import and does no work beyond the α-root unless its carrier forces independent content `[OPEN]`. **MC-T4.3 stays a `[FOUNDATIONAL OBSTRUCTION]`** — FC-W is an external axiom, not a closure.

---

## 4 · What the conjecture *does* rest on (evidence, not derivation)

`x₊ = 1/α` carries the strongest structural evidence the framework holds — but it is **evidence, not a chain**:
- **FTD-0319** (formerly cited as FTD-0189) adversarial look-elsewhere scan: the master quadratic is the **unique dual-matcher** — 0 non-G\* dual-matchers across **2.65M degree-2 polynomials** over an 18-constant basket FTD did not design (rank 1 by ~130×).
- The **ℤ[i] complex-structure unification** (FTD-0122/OT-1.5): the CM automorphism count (4) and the tower level k=4 unify through `ℤ[i]²`.
- CM-curve uniqueness of the lemniscatic d=−4 at class number 1.

These make `[STRONGLY MOTIVATED CONJECTURE]` the *honest, evidence-upgraded* tag — and explicitly **not** `[DERIVED]`.

---

## 5 · Non-promotion

**Nothing promoted.** `x₊ = 1/α` (FTD-0013) stays `[STRONGLY MOTIVATED CONJECTURE]`; MC-T4.3 stays `[FOUNDATIONAL OBSTRUCTION]`; no α derived anywhere. The algebraic spine (the master quadratic + G\* + the 16-coefficient) is **untouched pure mathematics** — its theorem-grade status is independent of the physical identification this doc declares undecidable from the substrate.
