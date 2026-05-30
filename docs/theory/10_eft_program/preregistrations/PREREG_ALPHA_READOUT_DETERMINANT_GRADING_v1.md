# Pre-Registration — MC-T4.3 Determinant-Grading Forward Derivation (v1)

**Tag:** `[PRE-REGISTRATION]` — locks the *design* of an attempt to close the single open gap localized by `AUDIT_ARC_C1_B2_FOUND_INDEPENDENT_REVIEW.md` in the ARC-B2/ARC-C1 α-readout verdicts. **Contains no result.** All three outcomes — FOUND / UNDERDETERMINED / CLOSED-NEGATIVE — are pre-blessed; the verdict is genuinely open.

**Date:** 2026-05-28
**Hash-lock target tag:** `preregister-alpha-readout-determinant-grading-v1`
**LEDGER row reservation:** FTD-0233 (confirm next-free identifier against `../07_assessment/LEDGER.md` at hash-lock).
**Governs / refines:** the ARC-B2 (FTD-0230) and ARC-C1 (FTD-0231) attempts, on the *one* step the independent review found unjustified. It does not re-open their genuine `[THEOREM]` content.
**Companion docs:** `SPEC_ALPHA_READOUT_CONTRACT.md` (FTD-0152), `AUDIT_ARC_C1_B2_FOUND_INDEPENDENT_REVIEW.md` (the review that localized this gap), `DERIV_BCC_COMPLEX_STRUCTURE.md` (FTD-0122, `V_complex ≅ Z[i]²`), `DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md` (Watson identity), `AUDIT_CHARGE_QUANTIZATION_NO_CHEAT.md` (FTD-0231, Gate 4).

> **Pre-registration discipline.** §§1–6 are committed and git-tagged **before** the attempt is run. After commit: SHA256 → `REF_PREREGISTER_MANIFEST.md`, tag applied. Any post-hoc edit to §§1–6 invalidates v1; a v2 is required. The result lands in a separate doc (`FOUND_*` / `AUDIT_*_CLOSED_NEGATIVE.md` / `AUDIT_*_UNDERDETERMINED.md`), never as edits here. B-9 (no same-minute mtime pre-reg↔result) and B-10 (separately-dispatched independent review) apply.

---

## §1 — Context and the localized gap

The ARC-B2 (FTD-0230) and ARC-C1 (FTD-0231) resolutions claim **FOUND-at-ARC-2** for the operational α-readout. The independent review `AUDIT_ARC_C1_B2_FOUND_INDEPENDENT_REVIEW.md` found the "FOUND" label is an overclaim and localized the defect to **one step**: the leap from the genuinely-derived ingredients to the **full** master quadratic `x² − 16G*²x + 16G*³`.

The master quadratic's two coefficients are `16G*²` (the sum / trace, by Vieta) and `16G*³` (the product / determinant). They differ by exactly **one factor of G\***: `Det/Tr = G*`. The genuinely-derived ingredients supply `G*²` (Watson) and the integer `16` (`|μ₄|²`) — but the determinant requires a **third, odd power of G\***, and the resolution docs supply it by an admitted `[SELECTION]` (FTD-0231 §3 "Selection 1"; FTD-0230 §4 asserts `Det = 16G*³` without derivation). This pre-registration locks an attempt to determine whether that third power can be obtained **forward**, or whether it is a structural obstruction.

---

## §2 — The question (LOCKED)

**Q-DG.** Using **only** the frozen ingredients of §3, does there exist a forward-derived readout operator `T` on `V_complex` (or on a Z[i]-module built from it) such that its characteristic polynomial is the master quadratic — equivalently, such that its two invariants are `Tr(T) = 16G*²` **and** `Det(T) = 16G*³` (the `(G*², G*³)` grading, `Det/Tr = G*`) — where **every step is `[THEOREM]`/`[DERIVED]` from §3** and **no §5 falsifier fires**?

The trace `Tr = 16G*²` is *granted as plausibly reachable* (it is `|μ₄|² · 2π · G_BCC(0)`, degree 2 in G\*). **The decisive sub-question is the determinant's third (odd) power of G\*:** can `Det = 16G*³` arise forward from §3 without insertion or a G\*-prefactor selection?

The verdict is genuinely open. All three §6 outcomes are pre-blessed.

---

## §3 — Frozen admissible ingredients (LOCKED)

The attempt may use **only** these, each already `[THEOREM]`/`[DERIVED]` independent of any α target:

1. **`V_complex ≅ Z[i]²`** — the 2D complex (rank-2 free Z[i]-module) subspace of the BCC corner representation, with `J²=−I` acting as `i`. Source: FTD-0122 / `DERIV_BCC_COMPLEX_STRUCTURE.md` (OT bedrock). G\*-degree: **0**.
2. **`μ₄ = Z[i]^× = {1,i,−1,−i}`**, `|μ₄| = 4`, `|μ₄|² = 16`. The Z[i]-module operations (addition, Z[i]-scalar multiplication, the Hermitian norm `|z|² = z z̄`, the unit-group action). G\*-degree: **0** (counts/integers).
3. **The BCC Watson self-energy** `G_BCC(0) = W₃ = G*²/(2π)`, and its defining lattice Fourier integral. Source: `DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md` §1, Spine Thm 5. G\*-degree: **2**.
4. **The winding / topological index** `Ind(γ) ∈ Z` on `V_complex` (charge quantization to `{−1,0,+1}`). Source: FTD-0231 Thm 1. G\*-degree: **0**.
5. Standard lattice-field operations on the above (finite differences, projections, transfer-matrix construction, eigenvalue/trace/determinant) — provided they introduce **no** new G\*-bearing constant.

**Out of scope (NOT admissible as input):** the master quadratic `x²−16G*²x+16G*³` (FTD-0001) and its roots/coefficients; the harmonic invariant Theorem 8 `1/y₊+1/y₋=1` (FTD-0111); the FQCR Model V operator `M_N` (FQCR Prop 5); any prefactor, normalization, or constant `∝ G*^k` (k odd) whose value is not forced by ingredients 1–4; any CODATA/α value.

---

## §4 — Benchmark (LOCKED)

The target invariant to be reproduced **forward** is `Det(T) = 16G*³` (with `Tr(T) = 16G*²`). The numerical value of the determinant or of `x₊ = 1/α` is **benchmark-side only** and is compared (if at all) only after §5/§6 pass — never used in the construction.

---

## §5 — Falsifiers (LOCKED, mechanical)

The attempt is falsified (→ UNDERDETERMINED or CLOSED-NEGATIVE) if any fires:

- **F1 — target insertion.** The master quadratic, its coefficients (`16G*²`, `16G*³`), or its roots (`x₊`, `x₋`) appear as an *input* anywhere in the construction.
- **F2 — FQCR import.** The Model V operator `M_N` (FQCR Prop 5) is imported as scaffold and its eigenvalue equation read off.
- **F3 — root-property circularity.** Theorem 8 (`1/y₊+1/y₋=1`) or any property *of* the master quadratic's roots is used to obtain the grading (the property presupposes the polynomial).
- **F4 — G\*-prefactor selection.** A prefactor / normalization / constant `∝ G*` (or any odd power of G\*) is introduced whose value is not forced by §3 ingredients 1–4 — including a `√(G_BCC(0)) ∝ G*` factor used solely to supply the odd power. (This is the "Selection 1" move the review flagged.)
- **F5 — CODATA.** Any empirical α / QED value enters the construction.
- **F6 — look-elsewhere.** The third G\* power enters via an unforced choice among equally-admissible constructions, with no a-priori reason for the chosen one.

The §-method step "apply the falsifier checklist" must state, per F-rule, whether it fires and why, before any numerical comparison.

---

## §6 — The three pre-blessed outcomes (LOCKED)

- **FOUND.** A readout operator `T` is exhibited with `Det(T) = 16G*³` and `Tr(T) = 16G*²`, every step `[THEOREM]`/`[DERIVED]` from §3, **no** F-rule firing. ⇒ The third G\* power is forward-derivable; the ARC-B2/C1 **FOUND is rescued** (eligible for re-confirmation), and the independent review's overclaim finding is itself overturned. Consequence: ARC-3 ratification of FTD-0013 becomes separately considerable (not automatic).
- **UNDERDETERMINED.** A partial forward relation is reached (e.g. `Tr = 16G*²` derivable) but the determinant's third G\* power requires a choice not forced by §3 (an admissible construction exists but the grading is unforced). ⇒ Confirms the review: honest status of ARC-B2/C1 is UNDERDETERMINED.
- **CLOSED-NEGATIVE.** It is shown that no operator built forward from §3 can have `Det = 16G*³` without a falsifier firing — e.g. a structural argument that the §3 ingredients generate only a restricted set of G\*-degrees that excludes the determinant's. ⇒ Strengthens the review from "UNDERDETERMINED" to a **structural no-go**: the ARC-B2/C1 FOUND cannot be rescued within the frozen ingredients, and the surviving α-readout space must look outside them (a `[CONJECTURE — new postulate]` would be required). A recognized deliverable (CLAUDE.md goal-clause 2).

---

## §7 — Method (LOCKED, ordered)

1. **G\*-degree accounting.** Assign each §3 ingredient its G\*-degree (1: 0; 2: 0; 3: 2; 4: 0). Determine the set of G\*-degrees reachable by the admissible operations (§3.5) applied to ingredients 1–4 — i.e. the closure of `{0, 2}` under the operations the construction uses (sum, product, Hermitian norm, eigenvalue extraction, etc.).
2. **Trace check.** Determine whether `Tr = 16G*²` (degree 2) is forward-reachable (expected plausible: `|μ₄|² · 2π · G_BCC(0)`), stating each step's source.
3. **Determinant attempt.** Attempt to construct `Det = 16G*³` (degree 3) forward from §3. Test every admissible route to an odd power of G\* (in particular: is `√(G_BCC(0))` admissible, or does it fire F4? is there a second independent G\*-bearing lattice invariant of odd degree?).
4. **Falsifier checklist.** Apply F1–F6 mechanically to the construction as derived.
5. **Numerical comparison** (only if 1–4 pass): compare `Det` to `16G*³` and the dominant root to `1/x₊`. Benchmark-side values per §4.
6. **Verdict** per §6, with the full trace (FOUND), the unforced-choice identification (UNDERDETERMINED), or the structural obstruction (CLOSED-NEGATIVE).

---

## §8 — Hash-lock protocol

1. Finalise §§1–7. `sha256sum` this file; record in `REF_PREREGISTER_MANIFEST.md`; add a `LEDGER.md` row (FTD-0233 or next-free) tagged `[PRE-REGISTRATION]`.
2. `git commit`; `git tag preregister-alpha-readout-determinant-grading-v1`.
3. The attempt (executing §7) runs only against the tagged commit; its result lands in a separate doc.
4. If a §3 ingredient or §5 falsifier proves defective once the attempt starts, the response is a **v2 pre-registration**, not an edit to v1 (FTD-0186 v1→v2 precedent).

*Authored 2026-05-28. **No result.** The attempt is the next step and runs only after hash-lock. Engineering toward any verdict invalidates the attempt.*
