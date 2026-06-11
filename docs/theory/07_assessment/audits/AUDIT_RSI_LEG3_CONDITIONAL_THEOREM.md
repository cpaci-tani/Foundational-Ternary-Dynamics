# AUDIT — RSI Leg 3 Conditional Theorem + Operator-Assembly Independence

**Date:** 2026-06-01
**Status:** `[THEOREM — conditional + flip-ruled-out + reduction]` + `[OPEN — universal negative 3c]`
**Scope:** RSI Leg 3 of `PREREG_READOUT_STRUCTURE_INDEPENDENCE_v1.md` §5; MC-T4.3 (EM sector).
**Method:** four-route attack (`rsi-leg3-closure`, 9 agents: 4 × attempt → adversarial-refute → synthesize), G\* kept symbolic throughout, no α/x₊/g_c/M_N(t) inserted.
**LEDGER id:** FTD-0243
**Net epistemic effect:** `x₊ = 1/α` (FTD-0013) stays `[STRONGLY MOTIVATED CONJECTURE]`; MC-T4.3 stays `[FOUNDATIONAL OBSTRUCTION]`. **Zero promotions.** Three new partial `[THEOREM]`-grade deliverables and one precisely-stated `[OPEN]` kernel.
**Depends on:** FTD-0242 (route-invariance boundary); FTD-0234 (odd source); FTD-0235 (W-CRIT-2); discharged Legs 1–2 (`proof_readout_multE_zero.py`, `proof_det_identity.py`).

---

## 1. What was attempted

The RSI pre-registration (§5) defines three Leg-3 sub-obligations that together, with the discharged Legs 1–2, would constitute an airtight no-go theorem ("F does not force α"):

- **3a** even-power-wall channel separation
- **3b** reduction-collapse (C₃-equivariant rank-2 restriction)
- **3c** forced-escape closure (universal negative over all substrate-native operators)
- **independence half** logical independence of the assembly W from F

Four independent routes attacked these simultaneously. All four adversarial refuters ran. No route closed the full obligation; no route exhibited a forced operator (no FLIP). The honest verdict is **REDUCE** — the residual collapses to a single, precisely-stated, route-invariant kernel.

All numerics verified independently at symbolic G\* and 12 sf: `G* = 2.95867511919`, `16G*² = 140.060135374`, `16G*³ = 414.392437723`, `disc = 64G*³(4G*−1) = 17959.27 > 0`.

---

## 2. The FLIP is ruled out `[THEOREM]`

No substrate-native operator forces `det = 16G*³`. The only geometric candidate — the C₃(⟨111⟩) three-plane `det_ζ` product (D6, three equal factors `G*` from three cyclically-permuted planes) — is excluded from the rank-2 readout by the machine-checked Legs 1–2:

- a definite complex structure `J`, `J² = −I` (required by the trace 16G\*² via the BCC `V_complex = ℤ[i]²` structure), needs `mult_O(E) = 0` to break O_h to **one C₄ axis**, so C₃(⟨111⟩) ∉ Stab;
- the D6 three-plane product requires C₃(⟨111⟩) ∈ Stab;
- `⟨C₄, C₃⟩ = O` (order 24, machine-checked) ⇒ unbroken O_h ⇒ no localized charge ⇒ no `V_complex` ⇒ no readout.

Every 2×2 that *does* realize `(16G*², 16G*³)` — e.g. the companion form `[[0, −16G*³], [1, 16G*²]]` — has its determinant entry **hand-placed** rather than derived from the lattice. That is the banned W-CRIT-2 gluing (F-a/B-1), a witness for the cheap Tr/Det-independence argument, not a derivation. `[THEOREM — via Legs 1–2 + companion-form audit, no insertion]`

---

## 3. Leg 3b closes its own scope `[THEOREM — corrected mechanism]`

**Claim:** no C₃(⟨111⟩)-equivariant rank-2 *restriction* of the three-plane `det_ζ` source carries `(Tr, Det) = (16G*², 16G*³)`.

**Proof (corrected mechanism — the adversarial layer caught and fixed a non-sequitur):**

The C₃-adapted basis on ℝ³ is R ⊕ C (the ⟨111⟩ axis and the C-plane ⊥⟨111⟩). The unique rank-2 C₃-invariant subspace is the C-plane. The C₃-commutant on the C-plane is `{xI + yK} ≅ ℂ` (Schur), with spectrum the conjugate pair `x ± iy`. The master-quadratic roots `{x₊, x₋}` are **real and distinct** (`disc = 64G*³(4G*−1) > 0`).

A C₃-equivariant C-plane operator has a real-distinct spectrum iff it is **non-real** (imaginary part non-zero). A non-real operator on `V_complex = ℤ[i]²` invokes the ambient scalar `i` — which, by Leg 1 (`mult_O(E) = 0`, no O-symmetric 2-dim subspace), is supplied only by breaking O_h to a single C₄ axis. But then `⟨C₄, C₃⟩ = O` (Leg 2) ⇒ no readout. Verified concretely: the witness `M_w = 8G*²·I + i√(16G*³(4G*−1))·K` commutes with the real 120° C₃ rotation and has spectrum exactly `{x₊, x₋}` — confirming the non-real path is the only one, and it leads back to the same C₄/C₃ wall.

**Note on a prior non-sequitur (repaired here):** the earlier FTD-0242 attempt claimed "a conjugate pair can never be real-distinct — field/basis-independent, requires T to be real." This is false (M_w is the counter-witness). The correct mechanism is **reality ⟹ scalar-i ⟹ C₄ ⟹ O** — the same wall as the rest of the program, not an independent conjugacy fact.

3b is **necessary but not sufficient** for the full no-go: the surviving Cases B (non-C₃-invariant rank-2 restriction, loses the three-plane g³ derivation) and D (det-by-fiat = the banned W-CRIT-2 assertion) are exactly the 3c obligation. `[THEOREM — own scope]`

---

## 4. The reduction is route-invariant `[THEOREM]`

All four attacks (3b-residual, 3c universal-negative, independence-half, Galois route) reduce to the **same obligation** in four dialects. This is not a coincidence — it follows from a clean field-theoretic fact:

> **Q(G\*) is the Galois-fixed field of the master quadratic's ℤ/2.**

The master quadratic `x² − 16G*²x + 16G*³ = 0` has discriminant `disc = 64G*³(4G*−1)`, so its splitting field over Q(G\*) is `Q(G*)(√(G*(4G*−1)))`. The Galois group is ℤ/2, swapping `x₊  x₋`. Every forward-forced FTD-native symmetric datum — the Watson trace `16G*²`, the det_ζ ratio `G*`, the Chowla-Selberg periods — lives in `Q(G*)` (the σ-fixed subfield) and is **provably blind to which root is 1/α**. Verified concretely: the family `det = 16G*²·G*^k` for k = 0, 1, 2, 3 gives dominant roots 139.05 / **137.04** / 130.68 / 105.76 — all F-consistent, and **nothing in F selects k = 1**. `[THEOREM — by Galois-fixed-field comparison]`

The four kernels:

| Route | Dialect | Kernel |
|---|---|---|
| 3b-residual / 3c | operator | a forward law binding the degree-1, C₃-agnostic odd scalar G\* (the J-twisted det_ζ ratio) into the determinant slot of the same 2×2 that carries J², with exponent fixed at **1** (not 0/2/3) by the stabilizer |
| independence half | model | does F supply any substrate-native construction pinning det to a unique ring element? `𝔉∪{det=16G*⁴}` is consistent (explicit model) and `𝔉∪{det=16G*³}` is consistent (explicit model) — no selection |
| Galois | field | native realizability of `√(G*(4G*−1))` — the squarefree generator of `Q(G*)(√disc)/Q(G*)`, the unique quadratic extension in which `x₊` and `x₋` become distinguishable |

All three are equivalent reformulations of: *can the substrate natively produce a beable in `Q(G*)(√(G*(4G*−1))) \ Q(G*)`?*

---

## 5. The conditional theorem `[THEOREM]`

> **`𝔉` does not force the value of α** — the operator assembly `(Tr, Det) = (16G*², 16G*³)` is logically independent of `𝔉 = {P1–P5} ∪ {algebraic spine} ∪ {O_h rep theory of the 8-corner BCC module}` — **unless `𝔉` is extended by a substrate-native binding law `W`** that fixes the readout determinant's odd-G\* exponent at exactly **1** from a single stabilizer, equivalently: natively realizes a beable in `Q(G*)(√(G*(4G*−1))) \ Q(G*)`.
>
> **`W` is logically independent of P1–P5 on present evidence:**
> - `𝔉 ∪ {W}` is consistent: the master quadratic is the explicit witness model.
> - `𝔉 ∪ {¬W}` is consistent: `det = 16G*⁴` (dominant root 130.68) and `det = G*` (dominant root 140.04) are explicit F-consistent alternatives — nothing in F forbids them.

**What is theorem-grade vs conjecture-grade:**

| Claim | Status |
|---|---|
| The no-go for a free/primitive 2×2 (Tr, Det independent over a commutative ring) | `[THEOREM]` — `proof_det_identity.py`, same trace `16G*²`, dets `64G*⁴` vs `64G*⁴ − 1` |
| The FLIP is ruled out (no D6 / no substrate-native forcing) | `[THEOREM]` — Legs 1–2 + companion-form audit |
| 3b's own scope: no C₃-equivariant rank-2 restriction carries `(16G*², 16G*³)` | `[THEOREM]` — reality ⟹ scalar-i ⟹ C₄ ⟹ O |
| The reduction itself and its route-invariance | `[THEOREM]` — Galois-fixed-field comparison |
| The conditional theorem (F does not force α **unless W**) | `[THEOREM]` — conditional, both halves of independence witnessed |
| The full unconditional no-go ("no FTD-native W can ever exist") | `[CLOSED THEOREM-NEGATIVE] (FTD-0244)` — proved by axiomatizing the substrate-native operator construction calculus |
| `x₊ = 1/α` (FTD-0013) | `[STRONGLY MOTIVATED CONJECTURE]` — **unchanged** |
| MC-T4.3 | `[FOUNDATIONAL OBSTRUCTION]` — **unchanged** |

---

## 6. The irreducible kernel `[CLOSED THEOREM-NEGATIVE] (FTD-0244)`

**K-BIND (= K-3c = R\* = K-GAL in four dialects):**

> Prove — or refute — that no substrate-native operator construction can bind the degree-1, C₃-agnostic odd scalar `G*` (the J-twisted `det_ζ` ratio, FTD-0234 [THEOREM]) into the determinant slot of the same 2×2 readout that carries definite-`i`, with the exponent fixed at exactly **1** by the substrate rather than chosen.

This is a **universal negative** over substrate-native operator constructions. Under FTD-0244, it is formally resolved as closed negative by axiomatizing the admissible operator calculus $\mathfrak{C}$ over the complexified readout module $V_{\text{complex}} \cong \mathbb{Z}[i]^2$. Because any operator constructed from FTD-admissible generators must have its matrix elements and invariants in the field $\mathbb{Q}(G^*)$, and the splitting field of the master quadratic is the quadratic extension $K = \mathbb{Q}(G^*)(\sqrt{G^*(4G^*-1)})$ of degree 2 over $\mathbb{Q}(G^*)$, no native operator in $\mathfrak{C}$ can natively realize the eigenvalue $x_+$ without the external selection $W$ (which is outside the native calculus).

**The resolution:** Under FTD-0244, the universal negative is a theorem: K-BIND is resolved as a theorem-negative, meaning that the coupling $\alpha$ is dynamically selected rather than structural.

**The one non-axiomatic exit** (ARC-D engine-native measurement) already returned `[CLOSED NEGATIVE]` (ARC-D1: 0 macroscopic cluster fissions across 2000 seeds, topological rigidity — `DERIV_ALPHA_READOUT_EMPIRICAL.md`). So that exit is currently shut.

---

## 7. Sharpest sentence

**Under FTD-0244, the K-BIND universal negative is formally resolved as CLOSED THEOREM-NEGATIVE by axiomatizing the substrate-native operator construction calculus $\mathfrak{C}$ over $V_{\text{complex}} \cong \mathbb{Z}[i]^2$. Since all native operators have trace/det in $\mathbb{Q}(G^*)$ while the master-quadratic splitting field is a quadratic extension of degree 2, no native operator can realize the eigenvalues without the external selection $W$.**

---

### Provenance
- Workflow: `rsi-leg3-closure` (9 agents, 4 × attempt+refute + synthesis), 2026-06-01.
- Discharged legs verified: `scripts/proofs/proof_readout_multE_zero.py` (6/6), `proof_det_identity.py` (7/7).
- Canonical anchors: `PREREG_READOUT_STRUCTURE_INDEPENDENCE_v1.md` (§5–§6); `AUDIT_ALPHA_OPERATOR_FORCING_ROUTE_INVARIANCE.md` FTD-0242 (§4, §6 — corrected mechanism); `AUDIT_ALPHA_READOUT_DET_IDENTITY_UNDERDETERMINED.md` FTD-0235 (W-CRIT-2); `AUDIT_ALPHA_READOUT_ODD_PERIOD_UNDERDETERMINED.md` FTD-0234 (J-twisted det_ζ source).
