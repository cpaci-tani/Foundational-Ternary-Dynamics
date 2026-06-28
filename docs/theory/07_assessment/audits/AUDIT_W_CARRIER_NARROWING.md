# AUDIT — Can the α-binding axiom W be EARNED natively? The carrier-narrowing theorem (FTD-0314)

**Tag:** `[THEOREM]` (the narrowing theorem + the three carrier closures) + `[OPEN]` (one surviving loophole) + `[CONJECTURE]` (the weight-inhomogeneity closure of that loophole). **Conditional on Chudnovsky 1976** (algebraic independence of π and Γ(1/4)), exactly as the spine's Theorem 9 / FTD-0112.
**Lock (transparent — not a blind pre-reg):** the FOUND / CLOSED / UNDERDETERMINED verdict criteria were fixed before computing. Verification artifact: `scripts/proofs/proof_w_carrier_narrowing.py` SHA256 `7e1e90def51e…`, 11/11 PASS at dps=150. Read-only / pure mathematics — **golden gate untouched** (`0xb604d81a3d79366e`).
**Precedence:** LEDGER > `SPEC_FTD_FRAMEWORK_V1.md` (constitution) > this doc.

---

## 0 · Verdict

> **W cannot, on the evidence assembled, be earned from native substrate structure: `UNDERDETERMINED, strongly leaning CLOSED` (~85%).** Every examined carrier closes; the residue is one un-exhibited loophole that cannot be opened without the banned W-CRIT-2 value-planting.

The deliverable is **not** a derivation of α — it is a new `[THEOREM]`-grade **boundary** (Number-One-Goal clause 2): the **Narrowing Theorem**, which proves *what kind of object W must be* and *that no cheaper native object can be it*, geometrically explaining the K-BIND wall (FTD-0244). This doc is the load-bearing input that turns the "declare W as a 6th postulate" option (FC-W / FTD-0315) from an ad-hoc assertion into a **precisely-pinned** one.

**Nothing promoted.** `x₊ = 1/α` (FTD-0013) stays `[STRONGLY MOTIVATED CONJECTURE]`; MC-T4.3 stays a `[FOUNDATIONAL OBSTRUCTION]`; FTD-0244 is **extended** (operators → analytic-period carriers), not weakened; no α is derived anywhere.

---

## 1 · The obligation W must discharge (recap of FTD-0243)

The master quadratic `x² − 16G*²x + 16G*³ = 0` has roots `x± = 8G*² ± 4G*·√(G*(4G*−1))`. The surd `√(G*(4G*−1)) = 5.66183351260…` is the **sole irrational distinguishing the two roots** (verified: `surd = (x₊−x₋)/(8G*)`). FTD-0243 (`[THEOREM]`, conditional) proved `𝔉` does not force α unless extended by a binding law **W** that, equivalently: (1) pins the readout determinant's G\*-exponent to exactly 1 from a stabilizer; (2) natively realizes a beable in `Q(G*)(√(G*(4G*−1))) \ Q(G*)`; (3) breaks the ℤ/2 Galois symmetry swapping `x₊ ↔ x₋`. FTD-0244 (K-BIND, `[CLOSED THEOREM-NEGATIVE]`) closed this for the **operator** class — every native operator's trace/det lies in `Q(G*)`. This audit asks whether *any* native object — not just operators — supplies W.

`Q(G*)(√(G*(4G*−1)))/Q(G*)` is a **genuine degree-2 extension**: `4t²−t` is squarefree over `Q(t)` (factor_list `(t,1)(4t−1,1)`), so the surd is not in `Q(G*)` (PSLQ: no relation vs `{1,G*,…,G*⁵}` at dps=150). `[THEOREM]`

---

## 2 · The Narrowing Theorem `[THEOREM]` (conditional on Chudnovsky)

> **G\* is transcendental over ℚ** (`G* = Γ(1/4)²/(π√2)`; π and Γ(1/4) are algebraically independent by Chudnovsky 1976 — spine Theorem 9 / FTD-0112), so `Q(G*) ∩ Q^ab = Q`: G\* lies in **no** cyclotomic field. The surd `√(G*(4G*−1))` is therefore **transcendental over ℚ** (degree 2 over the transcendental `Q(G*)`; transcendence degree 1).
>
> **Consequence.** Any carrier whose complete invariant set is **algebraic over ℚ** (transcendence degree 0) can *never* equal the surd. This **excludes every finite-symmetry carrier outright:**
> - chirality / handedness ℤ/2 (a sign, value in ℚ);
> - the ±1 ternary state sign (value in ℚ);
> - the binary-octahedral / spin double cover 2O and any finite subgroup of SU(2)/SO(3) (character-table invariants lie in a cyclotomic field ⊂ `Q^ab`);
> - the 27-block permutation parity / `S_n` sign character (value in ℚ);
> - **every native operator trace/determinant** (lies in `Q(G*)` — FTD-0244, here PSLQ-corroborated).
>
> **The only surviving door:** a forced order-2 (ℤ/2) **twist acting on a native G\*-bearing *analytic* object** (a lattice period / theta / det_ζ ratio), whose two Galois branches differ by the surd.

This **extends FTD-0244 from the operator class to the entire finite-symmetry class**, and it *geometrically explains* the K-BIND wall: the wall is not a peculiarity of 2×2 operators — it is the transcendence gap between `Q(G*)` (where all native algebraic invariants live) and the degree-2 extension the surd inhabits. The search space collapses from "any ℤ/2" to "an analytic-period twist."

---

## 3 · The three natural analytic carriers all close `[CLOSED NEGATIVE]`

The narrowing leaves exactly three natural G\*-bearing analytic carriers. Each closes — by an *exact* mechanism, no value planted.

**C1 — the ℤ/2-twisted BCC body-diagonal Green's function (the lead).** G\* = `√(2π·W_BCC)`, `W_BCC = Γ(1/4)⁴/(4π³)` = the Watson self-energy of the body-diagonal operator (OT-2.1). The only order-2 maps *forced by a lattice symmetry* are the per-axis antiperiodic boundary conditions `cos kᵤ → −cos kᵤ`. Under them the eigenvalue `1 − cx·cy·cz` only flips the sign of the cosine product — and the angular integral **annihilates exactly the odd-n terms the sign distinguishes** (`∫cosⁿ = 0` for odd n, so `(−1)ⁿ → +1` on the survivors). Hence `G_odd = G_even` **exactly** (verified: `|G_odd − G_even| = 0`). The forced involution **degenerates to the identity on the period** — its branch-difference is *zero*, not the surd. The existing J = ℤ[i] twist (FTD-0234) lands on G\* (in `Q(G*)`). `[THEOREM]`

**C2 — a forced second Watson integral.** PSLQ settles it at the root: `4G*−1 = 4·G* − 1` lies **entirely in `Q(G*)`** (relation `[1,−4,1]`). No second transcendental period is even *needed* to express the inner factor — its only transcendental ingredient is G\* itself, the *first* Watson integral. And a genuine second Watson self-energy (`W_FCC → Γ(1/3)/Q(√−3)`, `W_SC → Γ(1/24)/Q(√−6)`) is a transcendental in a **different CM field**, algebraically independent of Γ(1/4); the engine's own `(SC+FCC)/2` stencil is provably **BCC-orthogonal** (FTD-0050/0079). No forced lattice-symmetry pairing reaches the surd. `[THEOREM]`

**C3 — the deep CM-period / L-value route.** Every period and special L-value structurally forced from the lemniscatic curve `E: y²=x³−x` (CM by ℤ[i], d=−4) under its order-2 maps — the `[−1]` automorphism, the 2-torsion `E[2]`, the quadratic twists `E_D` — lies in the CM-period field `F = Q̄(π, Γ(1/4))` (`Ω = Γ(1/4)²/√(2π)`, `Ω/G* = √π` exactly, `L(E,1) = G*√π/8`; twists inject only `√(integer)`). The surd lies provably **outside F**: `surd² = (4·Γ(1/4)⁴ − √2·π·Γ(1/4)²)/(2π²)`, whose numerator is **degree-1 in π** (squarefree), so it is not a square in `F`. The order-2 maps inject only rational or `√(integer)` surds, never the degree-1-transcendence surd. `[THEOREM]` (conditional on Chudnovsky)

---

## 4 · The weight-inhomogeneity corollary, and the one surviving loophole

**Strengthening corollary `[THEOREM]` (algebra) + `[CONJECTURE]` (the closure it implies).** `surd² = G*(4G*−1)` is a sum of **two monomials of different (π, Γ(1/4)) total-degree** — `4Γ(1/4)⁴` (degree 4) and `√2·π·Γ(1/4)²` (degree 3) — i.e. it is **motivically weight-inhomogeneous** (verified: monomial total-degrees `{3, 4}`). A period of a single **pure / graded** motive carries homogeneous weight; the square root of a weight-inhomogeneous element is therefore the period of **no pure graded motive**. The algebra is exact `[THEOREM]`; the leap "⇒ no native graded period has square = G\*(4G\*−1)" requires the motivic-weight framework and is `[CONJECTURE]`-grade *pressure*, not a proof.

**The surviving loophole `[OPEN]`.** A **new, not-yet-exhibited** forward-derived transcendental period (e.g. a zeta-regularized determinant of an operator *other* than the companion form, or an un-examined theta/period ratio) that is provably squarefree-equal to the surd **and** carries a *forced* (not hand-placed) order-2 stabilizer. Untouched by C1/C2/C3; pressured by the weight corollary; **leaning CLOSED**. The honest reason it is not declared CLOSED: no general theorem "no native graded period has this square" is in hand. The honest reason it is not a live route: exhibiting such a period requires supplying *both* the value *and* a forced ℤ/2 — which, co-fitted, **is** the banned W-CRIT-2 hand-placement.

---

## 5 · Consequence — the disciplined "6th postulate" move (FC-W) + non-promotion

Because W cannot be earned natively, the honest way to *adopt* it is to **declare it** as an external `[AXIOM]`-class Framework Commitment — **FC-W** (the constitution's FC-4, LEDGER FTD-0315). The Narrowing Theorem is what makes that declaration **disciplined rather than ad hoc**: it states *exactly* what is being imported (a forced order-2 twist on a G\*-bearing analytic structure realizing the surd) and *proves no cheaper commitment — no finite group, no operator, no second period, no CM period — can substitute*. Under FC-W, `x₊ = 1/α` becomes a `[CONDITIONAL THEOREM given W]` / `[CONDITIONAL — DERIVED-GIVEN-IMPOSED]`, **explicitly not `[DERIVED]`**.

**The honest cost, stated not buried (F10 / GTCA).** Unlike FC-1/FC-2, which *decline* imports and thereby *buy* the falsifiable deviation spine, **FC-W is an *adoption* of an import**, and it does **no work beyond the α-root** unless its carrier also forces independent content — which is `[OPEN]`. So FC-W earns full commitment status only conditionally; declaring it for α alone is a precisely-pinned declaration, not a derivation.

**Non-promotion.** `x₊ = 1/α` stays `[SMC]` (unconditional); MC-T4.3 stays `[FOUNDATIONAL OBSTRUCTION]`; FTD-0244 is extended, not altered; the algebraic spine and the linear `k=¼` O_h theorem are untouched; no α derived. The three `[CLOSED NEGATIVE]` carrier verdicts and the Narrowing Theorem **harden** the boundary; they promote nothing.
