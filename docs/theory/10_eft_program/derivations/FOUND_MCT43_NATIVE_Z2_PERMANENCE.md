# FOUND — No FTD-native ℤ/2 supplies δ: the MC-T4.3 boundary is robust against every native orientation symmetry (PERMANENT-EXTENDED)

**Tag:** `[DERIVED]` (the field-extension enumeration, extending FTD-0244 `[THEOREM]`) + `[SYNTHESIS]` (the act-lens interpretation). Introduces no theorem of its own, **derives no α, promotes no tag.**
**Date:** 2026-06-25
**LEDGER id:** FTD-0318
**Pre-registration:** `PREREG_MCT43_NATIVE_Z2_PERMANENCE_TEST_v1.md`, git tag `preregister-mct43-native-z2-permanence-test-v1`, SHA256 `9ece1ba2c0d95e6e11b66a976ce8907f09924f6a2bd86a660b32f5503dee46f9` (committed before this result).
**Reuse (not re-derived):** FTD-0244 (`FOUND_OPERATOR_CALCULUS_AXIOMATIZATION.md`); FTD-0242 (`AUDIT_ALPHA_OPERATOR_FORCING_ROUTE_INVARIANCE.md`); FTD-0314/0315/0317.

---

## 0 · Verdict: **PERMANENT-EXTENDED** (pre-registered outcome 1 of 2)

**Every** FTD-native ℤ/2 symmetry is **BLIND** to `δ = √(G*(4G*−1))`: adjoining any of them to the native operator calculus 𝔠 keeps every invariant in `ℚ(G*)`, so none can select the master-quadratic root `x₊`. The FTD-0244 no-go — proved for a fixed 5-generator list — is therefore **robust against adjoining any native orientation symmetry**: the obstruction is not an artifact of which generators FTD-0244 happened to list.

> The δ-selection that fixes α is a **Galois orbit**, not a substrate symmetry. FTD's native ℤ/2's (conjugation, wave-orientation, charge, parity, time-reversal) all act by `ℚ`-entry matrices and fix `ℚ(G*)` pointwise; the one ℤ/2 that moves `δ` — `Gal(ℚ(G*)(δ)/ℚ(G*))` — is realized by **no** native operator. That is the precise, field-theoretic content of "δ is an *act*, not a *structure*" (the FTD-0315 lens): an act is exactly a ℤ/2 the substrate carries no operator to perform.

This **strengthens** FTD-0242's boundary (a route-invariant `[SMC no-go]`) along one rigorous axis and **extends** FTD-0317's single `i ⊥ δ` independence to all five native ℤ/2's. It does **not** derive α, does **not** remove MC-T4.3, and leaves FTD-0013 at `[SMC]`.

---

## 1 · The enumeration (the `[DERIVED]` core)

**Setup (reused, FTD-0244 §1–§2).** The readout structure is `V_complex ≅ ℤ[i]²`. FTD-0244 **Lemma 1**: every operator whose matrix entries lie in `ℚ(G*)` has `Tr, Det ∈ ℚ(G*)`; the native calculus 𝔠 (identity, complex structure `J`, Watson scaling `G*²/2π`, J-twisted det_ζ ratio `=G*`, rational stencils) has all entries in `ℚ(G*)`. FTD-0244 **Lemma 2**: `δ` is degree 2 over `ℚ(G*)` (Chudnovsky: `G*` transcendental ⇒ `G*(4G*−1) = 4G*²−G*` is a non-square in `ℚ(G*)`; min. poly `y² − G*(4G*−1)` irreducible). Verified independently here with `sympy` (irreducible over `ℚ(t)`; `Tr = x₊+x₋ = 16G*²`, `Det = x₊x₋ = 16G*³`, both in `ℚ(G*)`).

**The test.** For each native ℤ/2 `g`, exhibit its representation on `V_complex`, confirm it is a genuine involution (`g² = I`), record its entry-field, and apply the §3 criterion: `ℚ(G*)`-entry ⇒ all invariants of `⟨𝔠, g⟩` stay in `ℚ(G*)` ⇒ **BLIND**.

| # | native ℤ/2 | representative action on `V_complex` | `g²=I` | Tr | Det | entry-field | invariants of `⟨𝔠,g⟩` | verdict |
|---|---|---|---|---|---|---|---|---|
| 1 | **i-conjugation** (FC-0) | complex conjugation `a+bi ↦ a−bi` (real basis `diag(1,−1)`) | ✓ | 0 | −1 | `ℚ` ⊂ `ℚ(G*)` | `⊆ ℚ(G*)` | **BLIND** |
| 2 | **±ω wave orientation** (FTD-0316) | mode sign-flip `e^{±iωt}`: `−I` | ✓ | −2 | 1 | `ℚ` | `⊆ ℚ(G*)` | **BLIND** |
| 3 | **matter/antimatter** charge | ternary `s ↦ −s` (`+1 ↔ −1`): `−I` on state sector | ✓ | −2 | 1 | `ℚ` | `⊆ ℚ(G*)` | **BLIND** |
| 4 | **lattice parity / inversion** (`O_h` center) | spatial inversion `x ↦ −x`: signed involution | ✓ | −2 | 1 | `ℚ` | `⊆ ℚ(G*)` | **BLIND** |
| 5 | **time-reversal T** (linear part) | sign on momentum / `∂_t`: `diag(1,−1)` involution | ✓ | 0 | −1 | `ℚ` | `⊆ ℚ(G*)` | **BLIND** |

**The closure argument (why the table is exhaustive of `⟨𝔠,g⟩`, not just of `g`).** Each `g` has a representation with entries in `ℚ` (a fortiori `ℚ(G*)`). The algebra `⟨𝔠, g⟩` is generated under sum, product, and `ℚ`-scaling from matrices whose entries all lie in the **field** `ℚ(G*)`; those operations keep entries in `ℚ(G*)`. Hence **every** operator in `⟨𝔠, g⟩` — not merely `g` itself — has invariants in `ℚ(G*)` (Lemma 1). But `δ ∉ ℚ(G*)` (Lemma 2). Therefore no invariant of `⟨𝔠, g⟩` equals `δ` up to `ℚ(G*)`-scale: **no native ℤ/2 reaches δ.** ∎(relative to the inventory)

**Category REACHES-δ is empty.** The only ℤ/2 acting non-trivially on `δ` is the Galois automorphism `σ ∈ Gal(ℚ(G*)(δ)/ℚ(G*))` — and `σ` is **not** induced by any `ℚ(G*)`-entry operator (it fixes `ℚ(G*)` pointwise while moving `δ`), so it is not realizable by any element of `⟨𝔠, g⟩` for any native `g`. It is the act under test, not a native symmetry (per the locked exclusions). (Note: i-conjugation is *antilinear*, but this does not help — `G*` is real, so conjugation fixes `ℚ(G*)` pointwise and sends `δ = √(positive real)` to itself, not `−δ`; it is BLIND regardless.)

### 1.1 Robustness beyond the five (adversarial hardening)

The closure argument is generator-agnostic: it depends only on `ℚ(G*)`-entry-ness, so any further native ℤ/2 of the same kind is BLIND by the *same* one line. Concretely, every native ℤ/2 outside the locked five that one might propose reduces to the closure:

- **composites** — `CP` = conjugation∘parity, and any product of the five, are `ℚ(G*)`-entry (a product of `ℚ(G*)`-entry matrices) ⇒ BLIND.
- **permutation/sublattice involutions** — `SC↔FCC` exchange, the Phase-J ℤ/2, the `O_h^{ab}` involutions: `{0,1}`/`ℚ`-entry permutation matrices ⇒ BLIND.
- **the `(1+i)`-tower ℤ/2 (Theorem 8, FTD-0111)** — levels `k = 3,5,6,7` adjoin *independent* irreducible surds (`√2`, and the roots of `8G*²−1`, `16G*³−1`, `32G*⁴−1`), each a fresh degree-2 extension orthogonal to `δ` ⇒ BLIND. Level **`k = 4` reproduces the master quadratic itself** — its discriminant is `δ²` up to a perfect square — so `k=4` *is* the α-act under test, **not a new native symmetry** (it is the very object whose forcing is in question, excluded by the locked taxonomy).
- **the θ-null `√G*` near-miss (FTD-0242 §9)** — the most dangerous candidate: the substrate's weight-½ object `θ₃(0,i) = √G*/(2π)^{1/4}` is a genuine native `√G*` (up to measure-dressing). Even granting a hypothetical clean `√G*`-native operator, it is **still BLIND to δ**: `δ = √G*·√(4G*−1)`, and `√(4G*−1)` is independent of `√G*` — `ℚ(G*, √G*, δ)` has degree **4** over `ℚ(G*)` (`4G*−1` remains a non-square after adjoining `√G*`). The θ-null supplies one of δ's two square-root factors and **provably not the second**. This matches FTD-0242 §9's own "shortfall is one measure factor" reading and the Theorem-8 closed form `α_tree = 1/(2G*) − √(4G*−1)/(4G*^{3/2})`, where `√(4G*−1)` is the irreducible second ingredient.

None of these reaches `δ`; the verdict is robust against every native ℤ/2 we can name, not only the five. The honest residual stays as in §3 (completeness of the *kind* "native ℤ/2 = `ℚ(G*)`-entry involution").

---

## 2 · The act-lens reading (the `[SYNTHESIS]` layer)

FTD-0315 (`[SYNTHESIS]`) framed a square root as "an act of intent — an unforced ℤ/2-break." The enumeration gives that lens its exact field-theoretic content:

- A **structure** ℤ/2 is one the substrate carries an operator to perform — and every such operator has `ℚ(G*)`-entries, hence fixes `ℚ(G*)` (the five rows above). These are the geometric/sign involutions of the ontology.
- An **act** ℤ/2 is `Gal(ℚ(G*)(δ)/ℚ(G*))`: it moves `δ` while fixing `ℚ(G*)`, so **no substrate operator performs it.** Selecting which root is `x₊` is choosing a Galois branch — an operation categorically outside the substrate's operator algebra.

So "the universe is chosen twice — `i` builds the spine, `δ` is the one selection the substrate cannot make for itself" (FTD-0317 §2) is not metaphor: it is the statement that `δ`'s ℤ/2 is Galois, while `i`'s and the other natives' ℤ/2's are operator-realized. This is **why** α is dynamical (FTD-0242 §5) at the level of field theory, not just route-counting.

---

## 3 · What this does and does not establish (the honest scope)

**Establishes (`[DERIVED]`):** the FTD-0244 no-go is **robust** — adjoining *any* of the five native orientation ℤ/2's to 𝔠 cannot supply `δ`. The boundary FTD-0242 mapped as "route-invariant `[SMC]`" is, along this axis, a clean field-extension fact. This is a **wider scoped no-go**, the Number-One Goal's clause-two deliverable sharpened.

**Does NOT establish:** an *unconditional* impossibility. The result is scoped to the **inventory of native ℤ/2's** (§1 of the pre-reg) exactly as FTD-0244 is scoped to its "admissible construction set." A genuinely new substrate ingredient — one whose representation *forces* an entry outside `ℚ(G*)` (e.g. a hypothetical operator natively valued in `ℚ(G*)(δ)`) — is not covered, and would be precisely the surviving exit-(i) "6th-postulate-class input." Such an ingredient would not be a *native ℤ/2*; it would be a **declaration** that the substrate realizes `δ` — i.e. exactly the proposed FC-4 (`DRAFT_FC4_DELTA_ACT_DECLARATION.md`, un-minted). The act-lens shows why no amount of *native-symmetry* engineering reaches it: you cannot perform a Galois automorphism with an operator that fixes the base field.

**The two honest ways forward (unchanged in kind, sharpened here):**
1. **Accept the boundary** — MC-T4.3 stays an obstruction; α stays `[SMC]`; this result records that no native orientation symmetry closes it.
2. **Declare the act** — adopt the proposed FC-4 (declare the `δ`-branch, parallel to FC-0 declaring `i`); then α is *forced-given-FC-4* — a declaration, never a derivation. Left to owner sign-off.

---

## 4 · Tie-backs

- **FTD-0242** (route-invariance, `[SMC no-go]`): this result upgrades one axis of that boundary from "no route examined forces it" to "no native orientation ℤ/2, adjoined to 𝔠, *can* force it" — a field-extension fact, not a search outcome. The 14 closed-negative routes are untouched and un-rerun.
- **FTD-0244** (`[THEOREM]`): reused verbatim (Lemmas 1–2, the method); its scope is *extended* from the fixed generator list to that list ∪ any native ℤ/2.
- **FTD-0317** (`i ⊥ δ`): generalized — not just `i`, but all five native ℤ/2's are Galois-independent of `δ`.

---

## 5 · Status line

**Nothing is promoted.** Result tag `[DERIVED]` (enumeration) + `[SYNTHESIS]` (act-lens), **never `[THEOREM]`** (the inventory-completeness scope is the explicit residual, mirroring FTD-0244's "admissible construction set"). FTD-0244 stays `[THEOREM]` (unchanged, cited not re-derived); FTD-0242's boundary stays a no-go (now strengthened); MC-T4.3 stays a `[FOUNDATIONAL OBSTRUCTION]`; FTD-0013 stays `[STRONGLY MOTIVATED CONJECTURE]`; no α derived; P1–P5 and the FC register untouched; the proposed FC-4 is **drafted, not minted**. The verdict (PERMANENT-EXTENDED) is a pre-registered admissible outcome; the pre-registration SHA + git tag predate this commit. Adversarially red-teamed **SOUND** (no missed NEW-EXIT; the θ-null `√G*` attack fails by an independent degree-4 check — see §1.1; no covert promotion; pre-reg SHA recomputed to match).
