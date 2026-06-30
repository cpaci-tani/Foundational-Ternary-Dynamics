# AUDIT — The four named analytic-orientation carriers for `δ` all close negative; the magnitude/phase theorem (FTD-0341)

**Tag:** `[DERIVED]` (the four carrier closures + the magnitude/phase theorem, conditional on Chudnovsky 1976) + `[SYNTHESIS]` (boundary consolidation). **Sibling / direct follow-up of `AUDIT_W_CARRIER_NARROWING.md` (FTD-0314):** it attacks that document's §4 *surviving loophole* `[OPEN]` with the four named prime-suspect carriers and closes them.
**LEDGER id:** FTD-0341
**Lock (transparent — not a blind pre-reg):** the Gate-1/2/3/4 verdict criteria (below) were fixed before computing. Verification artifacts — `scripts/proofs/proof_orientation_carriers_closed.py` (22/22 PASS, dps=80) and `scripts/proofs/proof_delta_weight_zero.py` (17/17 PASS, dps=100). Read-only / pure mathematics — **golden gate untouched** (`0xb604d81a3d79366e`).
**Precedence:** LEDGER > `SPEC_FTD_FRAMEWORK_V1.md` (constitution) > this doc.

---

## 0 · Verdict

> **`LOOPHOLE_CLOSED_NEGATIVE` on the four natural carriers.** FTD-0314 §4 left one un-exhibited loophole: a forward-derived transcendental period, provably square-free-equal to the surd `δ = √(G*(4G*−1))`, carrying a *forced* (not hand-placed) order-2 stabilizer. The four named prime suspects for that period — the **eta-invariant**, **theta-with-characteristics**, the **half-derivative**, and the **AGM orientation** — each close negative under a fixed gate criterion. The boundary is **hardened**, not opened.

**Nothing promoted.** `x₊ = 1/α` (FTD-0013) stays `[STRONGLY MOTIVATED CONJECTURE]`; MC-T4.3 stays a `[FOUNDATIONAL OBSTRUCTION]`; FTD-0244/0314 are **extended** (operator + finite-symmetry + three analytic carriers → four *named* analytic-orientation carriers), not weakened; FC-W (FTD-0315) stands as the disciplined import; **no α is derived anywhere.** This is a Number-One-Goal clause-2 boundary deliverable.

This audit also records a **correction** to FTD-0314's reasoning (not its conclusion) — see §5, flagged for owner sign-off.

---

## 1 · The obligation, and the four named carriers (recap of FTD-0314 §4)

FTD-0314's Narrowing Theorem collapsed the search for `W` to "a forced order-2 twist on a native `G*`-bearing **analytic** object, whose two Galois branches differ by `δ`." Its §3 closed three *structural* analytic carriers (BCC-Watson twist, second Watson integral, CM-period/L-value); its §4 left `[OPEN]` one un-exhibited transcendental-period loophole. This audit names the four most natural concrete realizations of that loophole and tests each against a fixed four-gate criterion:

- **Gate 1 (forward):** the carrier is forward-derived from substrate structure (the de-Broglie/proper-time clock operator, the lemniscatic point `τ=i`, the arrow's half-derivative, the AGM of the spine), not reverse-engineered from `δ`.
- **Gate 2 (no-smuggle):** `δ` is not planted; the carrier's value is computed independently.
- **Gate 3 (generate the diagonal):** does the carrier generate **exactly** the diagonal quadratic line `Q(G*)(δ)`? — the decision gate.
- **Gate 4 (forced):** the order-2 twist is forced by a symmetry, not chosen by hand.

A carrier that **passes 1/2/4 but fails 3** is a *load-bearing* negative — its failure is a structural fact, not the artifact of a bad choice.

---

## 2 · The four carriers `[DERIVED]` (conditional on Chudnovsky)

| carrier | forward source | extension generated | square class | Gate 3 | verdict |
|---|---|---|---|---|---|
| **eta-invariant** | clock operator `D_a`, spectrum `{n+a}` | `Q` (rational) | `[1]` | **FAIL** | `CLOSED_NEGATIVE` |
| **theta-with-characteristics** | theta-nulls at `τ=i` | `Q(G*)(√G*)` | `[G*]` | **FAIL** | `CLOSED_NEGATIVE` |
| **half-derivative** | arrow operator `∂_t^{±1/2}` | `Q(G*)` | `[G*]` | **FAIL** | `CLOSED_NEGATIVE` |
| **AGM orientation** | AGM(1,√2) branch ℤ/2 | `Q(√2)·G* (+ i·same)` | fixed | **FAIL** | `CLOSED_NEGATIVE` |

All four pass Gates 1/2/4. (Carriers 2–4 are *forced-but-degenerate* at the self-dual lemniscatic anchor `τ=i`, `k=k'`, `z=1/4`.)

**C1 — eta-invariant / spectral asymmetry.** The substrate clock operator `D_a = −i d/dθ + a` (FTD-0234, even route) has spectrum `{n+a}`. Its APS spectral asymmetry is `η(D_a, 0) = ζ_H(0,a) − ζ_H(0,1−a) = (½−a) − (a−½) = 1 − 2a` — a **Bernoulli value, rational**. So `η(D_{1/4}) = +½`, `η(D_{3/4}) = −½`, and every ratio/combination lands in `Q`. The eta-invariant — the most natural "odd" analog of the determinant that sources `G*` — carries **no** Gamma-content at all. `[DERIVED]`

**C2 — theta-with-characteristics.** At the self-dual point `τ=i` (nome `q=e^{−π}`), the Jacobi theta-nulls are `θ₃(0,i)=π^{1/4}/Γ(3/4)`, `θ₂=θ₄` (self-dual), and `θ₃/√G* = 2^{−1/4}/π^{1/4}`. Every null is `(algebraic) · √G*/π^{1/4}` — square class `[G*]`, with **no `−1` shift anywhere**. The characteristics generate the `√G*` line (additive theta-transform structure), never the `√(4G*−1)` line. `[DERIVED]`

**C3 — half-derivative.** The arrow's Riemann–Liouville operator (FTD-0323) has eigenvalue `Γ(β+1)/Γ(β−α+1)`: forward (`z=1/4`) `= Γ(1/4)/Γ(3/4) = G*`; reversed-orientation (`z=3/4`) `= 1/G*`. The forward·reversed product is `1` (reversible ℤ/2); the difference `G* − 1/G*` lies in `Q(G*)`. The orientation ℤ/2 of the arrow stays inside the fixed field. `[DERIVED]`

**C4 — AGM orientation.** The spine's `G* = 2√π/AGM(1,√2)` (FTD-0327) carries an order-2 branch ambiguity (the sign of `√(ab)` per AGM step). The single-flip orientation vector is purely imaginary with `|Im|/G* = 4√2`, i.e. it lands in `Q(√2)·G* (+ i·Q(√2)·G*)`. The **robust, gate-deciding** fact: the entire orientation output sits in `Q(√2)·G*`, and `√(4G*−1)` is **not** in that field (PSLQ: no relation vs `{1, G*}`). *Caveat (audit-found):* the exact magnitude/axis (`i·4√2·G*` vs `i·4G*`; purely imaginary vs mixed) is **convention-dependent** across reproductions; the square-class conclusion does not depend on it. `[DERIVED]`

---

## 3 · The magnitude/phase theorem — why the odd route cannot carry the surd `[DERIVED]`

The four closures are not four coincidences; they share one structural cause.

> The substrate's `G*` is sourced by the **zeta-regularized determinant** of the clock operator, governed by `ζ'_H(0,a)` (the *derivative*): Lerch gives `det_ζ(D_a) = exp(−ζ'_H(0,a)) = √(2π)/Γ(a)`, so `det_ζ(D_{3/4})/det_ζ(D_{1/4}) = Γ(1/4)/Γ(3/4) = G*` (FTD-0234 — **even sector, carries the Gamma-content**). The **eta-invariant / orientation** is governed by `ζ_H(0,a) = ½ − a` (the *value*) — a Bernoulli polynomial, **rational**.

`det_ζ` and `η` are the **magnitude and phase of one complex zeta-determinant**: `det = |det|·exp(−iπ(η + ζ(0))/2)`. The Gamma-transcendence that builds `G*` lives entirely in the **magnitude** (`ζ'_H`, even); the orientation supplies only a **phase** (`ζ_H`, odd, rational/unimodular). A phase cannot be a real magnitude-bearing surd. Since `δ = √(G*(4G*−1))` is a **real magnitude**, no native orientation — `i`, the arrow, chirality, `η`, the AGM branch, all **phases** — can equal it. *The substrate can choose a direction (a phase) but not the size of the choice (a magnitude).* This is the structural reason behind the FTD-0340 reading and the FTD-0336 modulus/argument frontier, made concrete on the zeta-determinant. `[DERIVED]`

---

## 4 · The sharpened target

The corrected geometry (§5) pins `W` precisely. `δ` is the **diagonal** ℤ/2 of the biquadratic `(ℤ/2)²` compositum `Q(G*)(√G*, √(4G*−1))`: `δ = √G* · √(4G*−1)`. The substrate **nearly reaches** the first factor — `det_ζ → G*` gives the `[G*]` line, i.e. `√G*` (carriers C2/C3 land here). The **missing piece is the second factor `√(4G*−1)`**: a real surd whose `−1` shift sits in no native period (the `−1` is exactly what no Gamma/π monomial supplies). `W` must supply this one factor on the diagonal line — and that, co-fitted with a forced ℤ/2, **is** the banned hand-placement, which is why `W` is adopted (FC-W) rather than earned.

---

## 5 · Correction pending owner reconciliation — `δ` is weight-0, the obstruction is square-class (NOT applied to FTD-0314)

> **This subsection flags a correction to `AUDIT_W_CARRIER_NARROWING.md` (FTD-0314) for owner-reviewed reconciliation. It does NOT edit that `[THEOREM]`-tagged document.** The flagged reasoning's *conclusion is unchanged*; only its stated *reason* is wrong.

`AUDIT_W_CARRIER_NARROWING.md` §4 (its lines 56–58) argues the loophole "leans closed" via **motivic weight-inhomogeneity**: it reads `surd² = G*(4G*−1) = 4Γ(1/4)⁴/(2π²) − Γ(1/4)²/(π√2)` as a sum of monomials of *different (π, Γ(1/4)) total-degree* `{3, 4}`, hence "the period of no pure graded motive." (The same "degree-1 in π" phrasing also appears in §3 C3.)

**This is mislocated.** That "degree 3 vs 4" is **generator-monomial-degree** in `{π, Γ(1/4)}`, which is **not a motivic invariant**. In the correct grading `w(Γ(1/4)) = 1`, `w(π) = 2` (the Tate motive `Q(−1)` has period `2πi`):

- `G* = Γ(1/4)²/(π√2)` is a **ratio of two periods of the same CM motive** `h¹(E_i)`, `E_i : y² = x³ − x` — verified `Ω = Γ(1/4)²/√(2π)` (the holomorphic weight-1 period), so `w(Γ(1/4)²) = 2` and `w(G*) = 2 − 2 = 0`.
- `δ² = 4G*² − G*` is then **weight-homogeneous (weight 0)**: both terms are weight 0. Equivalently `π²·δ² = 2Γ(1/4)⁴ − πΓ(1/4)²/√2` is weight-4 *homogeneous* (`w(Γ(1/4)⁴)=4 = w(πΓ(1/4)²)=4`), though generator-degree-*inhomogeneous* (`4 ≠ 3`).

So `δ` is **weight 0**, not weight-inhomogeneous; weight does **not** close the door. The door is closed (to within the §6 residue) by a **square-class** obstruction over `Q(G*) ≅ Q(t)` (conditional on Chudnovsky 1976): `δ² ∼ t(4t−1) = 4t²−t` is **square-free** over `Q(t)`, so `Q(G*)(δ)/Q(G*)` is a genuine degree-2 extension — and this is *already* stated correctly in FTD-0314 §1/§2 (lines 23, 50). The conclusion was over-determined: a correct reason (square-class) and a mislocated one (weight). **Recommended owner edit:** in FTD-0314 §4 and §3-C3, replace the weight-inhomogeneity argument with the square-class argument, and demote the §4 `[CONJECTURE]` weight-closure to a corrected `[DERIVED]` square-class statement. Verified: `scripts/proofs/proof_delta_weight_zero.py` (17/17). **Owner sign-off pending.**

---

## 6 · The residue, and non-promotion

**What remains `[OPEN]` (~10%).** A genuinely *new* transcendental period — a forced `η` evaluated at a **non-self-dual** point (off `τ=i`), carrying a real curvature the flat self-dual substrate degenerates away — is neither exhibited nor excluded by standard results. Every named carrier above degenerates *because* it is anchored at the self-dual lemniscatic point, where the orientation ℤ/2 collapses to a pure phase. A carrier living off that point is the last structurally-distinct candidate; it maps to the locked **genesis-cokernel pre-registration** (`PREREG_GENESIS_COKERNEL_GRADING_v1.md`) as the remaining attempt. Per FTD-0340, a cokernel is what a 2-to-1 fold forgets, so the reading predicts that attempt lands **closed** — but it is not yet run, so the residue stays honestly `[OPEN]`.

**Non-promotion.** `x₊ = 1/α` stays `[SMC]`; MC-T4.3 stays `[FOUNDATIONAL OBSTRUCTION]`; FTD-0242/0244/0314 are extended, not altered; FC-W stands; the algebraic spine is untouched; no α derived. The four `CLOSED_NEGATIVE` carriers and the magnitude/phase theorem **harden** the boundary; they promote nothing.
