# FOUND — The AGM place-bridge: why the substrate's √-machinery lands on G\*, never on δ (three faces of one fact)

**Tag:** `[SYNTHESIS]` — assembles existing canonical results (FTD-0318, the prime-duality §5.1, FTD-0317, Theorem 9, the AGM identities) under one reading. **Introduces no theorem, derives nothing, promotes no tag.**
**Date:** 2026-06-25
**LEDGER id:** FTD-0319
**Reuse (not re-derived):** FTD-0318 (`FOUND_MCT43_NATIVE_Z2_PERMANENCE.md`); `EXPLR_GAUSSIAN_EISENSTEIN_DICHOTOMY.md` §5.1 (`[SYNTHESIS]`/`[STRONGLY MOTIVATED]`); FTD-0317 (`FOUND_SM_ACT_COUNT.md`); Theorem 9 (`SPEC_ALGEBRAIC_SPINE.md` §9, FTD-0112); the AGM identities (`MONOGRAPH_GSTAR_BRIDGE_CONSTANT.md`; `PAPER_GSTAR_INTRODUCTION.tex`, Gauss 1799).

---

## 0 · The one-line statement

> `G* = 2√π / AGM(1,√2)`. The AGM is the substrate's only native, convergent, **all-square-roots** machine for crossing the place-seam between the **ramified prime 2** (its input `√2 = |1+i|`) and the **archimedean place** (its output `π/ϖ`). But every AGM step is a *geometric-mean* `√(ab)` — a **positivity-forced magnitude, not a chiral branch-selecting act** — so the whole tower lands on `G*` (the π-free residue, degree 1 over `ℚ`) and **never on `δ = √(G*(4G*−1))`** (degree 2 over `ℚ(G*)`). The AGM is the concrete *witness* of why no substrate operation reaches the α-selecting root. This is the **arithmetic mechanism** behind FTD-0318's operator-side no-go.

---

## 1 · Three faces of one fact

The δ-unreachability that fixes α as *dynamical* (FTD-0242 §5) appears in three already-canonical places. They are the **same fact** seen from three sides:

| face | statement | source (at its existing tag) |
|---|---|---|
| **operator / Galois** | every native ℤ/2 acts by `ℚ(G*)`-entry matrices and fixes `ℚ(G*)`; the one ℤ/2 that moves `δ` is `Gal(ℚ(G*)(δ)/ℚ(G*))`, realized by **no** operator | FTD-0318 (`[DERIVED]`+`[SYNTHESIS]`) |
| **arithmetic / local–global** | the forced menu (trace `16G*²`, units `16`, period `G*²=2π·G_BCC(0)`) is integer-degree, product-formula-clean (**global**); a √ is an **intrinsically local act** carrying one place's valuation — so every native degree-½ `G*`-object is dressed once (archimedean `(2π)^{1/4}` or ramified-prime `2^{1/4}`), never unit-clean | `EXPLR_GAUSSIAN_EISENSTEIN_DICHOTOMY.md` §5.1 (`[SYNTHESIS]`/`[STRONGLY MOTIVATED]`) |
| **act / forced-magnitude** | an *act* is an unforced ℤ/2-break (`i`, `δ`); a *forced magnitude* (`√2`, `√3`, `√(2π)`) has one admissible branch and is not an act | FTD-0317 (`[SYNTHESIS]`) |

The dictionary that aligns them: **operator-realized ⟺ `ℚ(G*)`-entry ⟺ global/product-formula-clean ⟺ a forced magnitude (no branch to choose)**; **act ⟺ moves outside `ℚ(G*)` ⟺ local/valuation-carrying ⟺ a chiral branch-selection**. FTD-0318's "act vs structure" *is* §5.1.3's "epistemic vs ontic seam" *is* FTD-0317's "act vs forced magnitude."

---

## 2 · The AGM keystone (the new connective tissue)

`G* = 2√π / AGM(1,√2)`, equivalently `ϖ/π = 1/AGM(1,√2)` (Gauss's diary, 30 May 1799: `AGM(1,√2)·ϖ/π = 1`). Read it as a *map between places*:

- **input** `√2 = |1+i|` — the **ramified prime** of `ℚ(i)` (`2 = −i(1+i)²`, `N(1+i)=2`; the non-archimedean datum);
- **output** `π/ϖ` — pure **archimedean** (a period ratio);
- **residue** `G*` — the **π-free** part (Theorem 9: `ℚ(G*) ∩ ℚ(π) = ℚ`); the half that survives the crossing as algebraic-over-the-spine content.

So the AGM is literally the substrate's bridge across the prime-duality seam §5.1 names. And here is the decisive structural point that ties the bridge to the no-go:

> **Each AGM iterate is `(a,b) ↦ ((a+b)/2, √(ab))` — and `√(ab)` is a *geometric mean*, a √ of a positive real with one admissible branch.** By the FTD-0317 criterion it is a **forced magnitude**, not an act. The AGM is therefore an *infinite tower of forced-magnitude square roots*. A tower of forced (non-chiral) √'s can only ever produce more global/clean content — it lands on `G*` and its `ℚ(G*)`-rational functions, and it **cannot** perform the one chiral branch-selecting √ that `δ` requires.

This is the witness FTD-0318 asserts abstractly, made concrete: the substrate *does* have a powerful native √-machine (the AGM, which even reaches the transcendental `π/ϖ`), yet it is **δ-blind by construction** — because its √'s are means, not branch-choices. The "act the substrate cannot take for itself" (FTD-0317 §2) is exactly the chiral √ that the AGM's geometric means are not.

---

## 3 · Why δ is *doubly* out of reach

`δ = √(G*) · √(4G*−1)` needs **two** independent half-degree ingredients, and the substrate supplies **neither** as a clean act:

1. **`√(G*)` is never unit-clean.** Every native weight-½ `G*`-object is place-dressed: `θ₃(0,i) = √G*/(2π)^{1/4}` (archimedean) or `det_ζ(D_{3/4}) = 2^{1/4}√G*` (ramified-prime). In `ℚ(i)` you can keep the clean units `16 = |μ₄|²` (→ clean trace `16G*²`) **or** a clean `√`-half, not both — the prime you'd strip (2) is exactly the prime the discriminant ramifies at (§5.1.1, VERIFIED 16 dp; re-confirmed here, §6).
2. **`√(4G*−1)` is a *second, independent* surd.** `ℚ(G*, √G*, δ)` has degree **4** over `ℚ(G*)` (`4G*−1` stays a non-square even after adjoining `√G*`; FTD-0318 §1.1, red-team CAS-verified; re-confirmed §6). The θ-null / AGM machinery supplies (a dressed) `√G*` but **provably not** the second factor.

So even granting the substrate a hypothetical clean `√G*`, `δ` still needs an independent chiral √ the native machinery does not contain. The boundary is not one missing object; it is the absence of *any* native operation that performs branch-selection at all.

**Where the second factor "lives" is `[OPEN]`.** It is tempting to assign `√(4G*−1)` to the Eisenstein / prime-3 place (the determinant's "3-plane product"), but §5/§3 of the dichotomy doc show the Eisenstein forcing *cannot* supply the odd term and the equianharmonic master quadratic *has no canonical form* (`3^6 ≠ 6^2`). The only claim made here is the rigorous one: `√(4G*−1)` is **independent of `√G*`**; its arithmetic home is not established.

---

## 4 · Honest scope — what this is and is not

- **`[SYNTHESIS]`/`[SELECTION]`-grade interpretive bridge**, mirroring §5.1's own tagging. The kernel is `[THEOREM]`-grade and verified: the ramification `2 = −i(1+i)²`, the AGM identities, Theorem 9, FTD-0318's Galois fact, the degree-4 independence, the §5.1.1 dressings. The *identification* of "AGM/forced-magnitude/global/clean" with the ontic layer and "chiral √/act/local" with the epistemic layer is a structural reading (`[SELECTION]`), not a forced consequence of P1–P5.
- It **deepens the explanation** of the FTD-0318 / MC-T4.3 boundary (it says *why* δ is unreachable and exhibits the witness); it does **not strengthen the proof**, **does not close MC-T4.3**, **does not derive α**, and **does not promote** `x₊ = 1/α` (FTD-0013, `[STRONGLY MOTIVATED CONJECTURE]`).
- The `√(4G*−1)`-place reading is explicitly `[OPEN]` (§3).

## 5 · Status line

**Nothing is promoted.** Tag `[SYNTHESIS]`. FTD-0318 stays `[DERIVED]`+`[SYNTHESIS]`; §5.1 stays `[SYNTHESIS]`/`[STRONGLY MOTIVATED]`; FTD-0317 stays `[SYNTHESIS]`; Theorem 9 stays `[THEOREM]`; MC-T4.3 stays a `[FOUNDATIONAL OBSTRUCTION]`; FTD-0013 stays `[SMC]`; no α derived; P1–P5 and the FC register untouched. This note is exposition unifying existing claims at their existing tags, with the AGM identity as the connective witness.

## 6 · Verification (mpmath, ≥40 dp, computed in-session — identities, no near-miss search)

- `G* = Γ(1/4)/Γ(3/4) = 2.95867511918863889231…`; `ϖ = 2.62205755429211981046…`; `AGM(1,√2) = 1.19814023473559220744…`.
- `G* = 2√π/AGM(1,√2)` — diff `0.0` (exact to working precision); `AGM(1,√2)·ϖ/π = 1.000…` (`<10⁻³⁵`); `ϖ/π = 1/AGM` (`<10⁻⁴⁰`); `G* = 2ϖ/√π` (`<10⁻⁴⁰`).
- `θ₃(0,i) = √G*/(2π)^{1/4} = 1.08643481121…` ✓; `det_ζ(D_{3/4}) = 2^{1/4}√G* = 2.04553134422…`; clean `√G* = 1.72007997464…`; needed trace `4√G* = 6.88031989859…`.
- `δ = √(G*(4G*−1)) = 5.66183351260…`; `4G*−1 = 10.8347… > 0`; sympy: `4t−1` and `t(4t−1)` are **not** perfect squares in `ℚ[t]` ⇒ `√G*`, `√(4G*−1)` independent ⇒ `ℚ(t,√t,δ)/ℚ(t)` degree 4 (δ needs both factors).
