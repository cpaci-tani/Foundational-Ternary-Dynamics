# PRE-REGISTRATION — Completing the act-count: do the SM masses/angles need a field-act beyond {i, δ}?

**Tag:** `[PRE-REGISTRATION]` (locks method before result; no claim is made or promoted here).
**Date:** 2026-06-25
**LEDGER id:** FTD-0317 (the test; result `FOUND_SM_ACT_COUNT.md` shares this id).
**Git tag (applied at commit of this file):** `preregister-sm-act-count-test-v1`
**Depends on (reuse, not re-derive):** FTD-0314 (`FOUND_ACT_REDUCTION_COUNT.md`, the act-taxonomy + the open mass-sector question); FTD-0244 (`FOUND_OPERATOR_CALCULUS_AXIOMATIZATION.md`, the Galois degree-2 machinery); `CATALOG_PARAMETRIC_INSERTIONS.md` (the complete constant catalog).

---

## 0 · The question

FTD-0314 reduced FTD's irreducible-choice content to **one generative act `i = √(−1)`** plus a small register set and **one independent algebraic selection — α's `δ = √(G*(4G*−1))`** (Galois-independent of `i`, FTD-0244). It left **open** whether the **SM masses and mixing angles** reduce to the same two field-acts `{i, δ}` or introduce a **third**. This test classifies every *exactly-expressible dimensionless* SM quantity by its field-act and returns **CLOSED** (no third act) or **OPEN** (a third act named).

A **field-act** is an *unforced* square root — an irrational of degree >1 whose two branches are both structurally admissible (the FTD-0314 criterion: `i`/`−i`, the two master-quadratic roots `±δ`). A *forced magnitude* (`√2`, `√3`, `√(2π)`: positive lengths/speeds/Gaussian normalizations, one admissible branch) is **not** a field-act.

---

## 1 · The field-act taxonomy (locked; reuse FTD-0314)

Each exactly-expressible dimensionless quantity's closed form → **exactly one**:
1. **rational** — an integer/rational over the framework integers `{N_c, b₃, N_base, N_eff}` (e.g. `m_μ/m_e = 207`, `sin²θ_W = 3/13`); a **selection**, **no field-act**.
2. **uses `i`** (FC-0).
3. **uses `δ`** (the α extension; e.g. `1/α = x₊ = 8G*² + 4G*δ`).
4. **new independent algebraic selection** — adjoins an irrational of **degree >1 over `ℚ(G*)`, not in `ℚ(G*)(i, δ)`, AND unforced** (both branches admissible). **This is a third act.**
5. **positivity-forced magnitude** — a `√` of a positive real with one admissible branch (`√2`, `√3`, `√(2π)`, `π`); **not** an act.
6. **calibration** — a dimensional / unit-fixing quantity (separate `[DECLARED]` register).

## 2 · The inventory (anchored to the catalog for completeness, then fixed)

The inventory is **every dimensionless quantity with an exact closed form in `CATALOG_PARAMETRIC_INSERTIONS.md`** (the ~162-row master catalog), cross-checked against `SPEC_OPEN_MATH_BY_SECTOR.md` §5 and the mass-ratio / loop-coefficient derivations. Representative items (the classification must cover all, not only these):
- mass ratios: `m_μ/m_e`, `m_τ/m_e`, `m_p/m_e`;
- mixing/angles: `sin²θ_W`, `α_s`, PMNS `sin²θ_12/23/13`, `Δm²₃₁/Δm²₂₁`;
- loop coefficients `c1, c2, c3`; the framework integers `N_c, b₃, N_base, N_eff`;
- `G*/α`-carriers: `1/α`, `G_C = √α`, `α_G`.

**Explicitly EXCLUDED and flagged (not counted — the `[PARAMETRIC]` residue):** reverse-engineered / under-determined fits without a clean closed form — quark masses, flavor-depth matrices, CKM Wolfenstein entries, neutrino absolute scale, hadron masses, decay rates, imported QED loops — and all **dimensional** quantities (`m_e` in MeV, `ℓ_P`, VEV, `M_Z`). The count is over the *exactly-expressible dimensionless* set only; the residue is reported, never silently dropped.

## 3 · Mechanical criterion for category 4 (the decisive one)

A quantity is a **third act** iff its exact closed form contains an irrational `r` with:
- minimal polynomial over `ℚ(G*)` of degree >1, **and** `r ∉ ℚ(G*)(i, δ)` (FTD-0244 §2–§4 method — `G*` transcendental, squarefree test; **reused, not re-derived**), **and**
- `r` is *unforced* (both branches structurally admissible — not a positivity-pinned magnitude).

If every inventory item is rational, or routes through `i`/`δ`, or is a forced magnitude, or is a calibration → **no third act**.

## 4 · Admissible verdicts

- **CLOSED** — every inventory item ∈ {rational, uses-`i`, uses-`δ`, forced-magnitude, calibration}; the exactly-expressible dimensionless SM's field-acts are exactly **`{i, δ}`**.
- **OPEN** — ≥1 item is a category-4 third act (name it, with its minimal polynomial).

**Prior-favoured (disclosed, not pre-decided): CLOSED** — Phase-1 inventory found mass ratios are integers, angles rational, `G*/α`-carriers route through `δ`, and the only other roots (`√2`, `√3`, `√(2π)`) are forced magnitudes.

## 5 · Banned moves

- **B-1** Calling a rational integer *formula* (e.g. `m_μ/m_e = 207`) `[DERIVED]`/`[THEOREM]` — these are matched **selections** (the exact 2026-06-18 retraction error); the value's *rationality* is what the field-classification uses, independent of which integer formula is canonical.
- **B-2** Promoting any mass/angle tag (they stay `[SMC]`/`[PARAMETRIC]`/`[SELECTION]`).
- **B-3** Counting a positivity-forced magnitude (`√2`, `√3`, `√(2π)`, `π`) or a calibration as a field-act.
- **B-4** Re-deriving FTD-0244 instead of citing it.
- **B-5** Adding/removing inventory items after this lock to swing the verdict; the residue must be the catalog's `[PARAMETRIC]`/dimensional rows, listed.
- **B-6** Minting an FTD id without `scripts/audit/check_registry.py` (this test uses FTD-0317, confirmed next-free 2026-06-25).

## 6 · What the result delivers

`FOUND_SM_ACT_COUNT.md` (`[SYNTHESIS]`): a classification of every exactly-expressible dimensionless quantity, the CLOSED/OPEN verdict, the completed count ("field-acts = {i, δ}; N rational selections; forced magnitudes; calibration register; flagged `[PARAMETRIC]` residue"), and the tie-back closing FTD-0314's open mass-sector question.
