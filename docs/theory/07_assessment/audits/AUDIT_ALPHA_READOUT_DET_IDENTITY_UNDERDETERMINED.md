# AUDIT — detdet_ζ Structural Identity: UNDERDETERMINED

**Tag:** `[UNDERDETERMINED]` (pre-reg §6 UNDERDETERMINED). **No spine claim moved.**
**Date:** 2026-05-28
**Result of:** `PREREG_ALPHA_READOUT_DET_IDENTITY_v1.md` (FTD-0235 provisional), SHA256 `03b967c760fa38fffa8c7d08d5a75c34392dcd2c4c546f24a9c58b4d97a78122` (commit deferred by owner).
**Verifier:** [`scripts/proofs/proof_det_identity.py`](../../../scripts/proofs/proof_det_identity.py).

> **Correction (2026-05-28, post owner review).** This verdict was first written
> as **CLOSED-NEGATIVE**, resting partly on an argument that the operator
> determinant is "a finite product, categorically different from an *infinite*
> ζ-regularized determinant." **That argument was wrong.** As the owner noted,
> `G_BCC(0) = G*²/(2π)` (the Watson Green's function at the origin), the J-twisted
> det_ζ ratio `= G*`, and the finite-N approximant `G_N*` are all **scalars** —
> plain, forward-derived numbers. There is no "infinite-vs-finite kind mismatch."
> Consequently the determinant grading is **unforced, not impossible**, and the
> honest verdict is **UNDERDETERMINED** (this also softens the FTD-0233 parity
> framing — see §1). The net audit conclusion is unchanged: the ARC-C1/B2 "FOUND"
> still overclaims; MC-T4.3 stays open.

---

## 0 · Executive summary

**Verdict: UNDERDETERMINED.** The readout operator's determinant `16G*³` is *not
forward-forced*, but it is also *not impossible* — an admissible construction
exists; it is just unforced.

The decisive correction: **every master-quadratic coefficient is a product of
forward-derived FTD-native scalars**:
- `16 = |μ₄|²` (Z[i] unit group) — `[DERIVED]`,
- `G*² = 2π · G_BCC(0)` (BCC Watson Green's function, a scalar) — `[THEOREM]`,
- `G* = det_ζ(D_{3/4})/det_ζ(D_{1/4})` (FQCR Model I, a scalar) — `[THEOREM]`.

So `Tr = 16G*² = |μ₄|²·2π·G_BCC(0)` and `Det = 16G*³ = Tr·G*` are **both
forward-computable** (script confirms). The owner's "lattice is J²" hint is
vindicated for the *odd source*: `G*` is available forward and clean.

**Why it is still not FOUND.** Having the scalars in hand does **not derive *why*
the EM readout operator has trace `16G*²` *and* determinant `16G*³`**. A 2×2
operator's trace and determinant are **independent** invariants (verified: same
trace, different determinants `64G*⁴` vs `64G*⁴−1`). You can *assemble* a 2×2 with
exactly `(Tr, Det) = (16G*², 16G*³)` from the forward scalars — but you do so by
*choosing* the entries to match the master quadratic. That specific operator
structure is the **imposed master-quadratic target**, i.e. the long-standing
**W-CRIT-2** ("master quadratic imposed not derived", `SPEC_OPEN_MATH_BY_SECTOR.md`).
Unforced admissible construction ⇒ **UNDERDETERMINED**.

---

## 1 · What is forward-derivable, and what is not

**Forward-derivable (scalars):** the coefficients `16`, `G*²`, `G*`, and hence
`16G*²` and `16G*³`, are all forward-derived FTD-native scalar products (§0). The
**FTD-0233 parity "no-go" is therefore scoped, not fundamental**: it showed only
that parity blocks odd G*-degree if one excludes det_ζ ratio; once det_ζ
is admitted, that parity block is lifted. A clean forward **odd** scalar
(det_ζ ratio `= Γ(1/4)/Γ(3/4) = G*`, a clean forward **odd** scalar; FTD-0234) supplies the missing odd degree
directly. So parity is **not** the operative obstruction.

**Not forward-derivable (the operator structure):** there is no derived reason the
readout operator's two invariants are `(16G*², 16G*³)` rather than any other pair
assembled from the same scalars. The dependence `Det = Tr·G*` (one extra factor of
the det_ζ scalar in the determinant but not the trace) is exactly the
master-quadratic Vieta structure (FTD-0001) — the **target**, imposed. This is the
operative obstruction (W-CRIT-2 / W-CRIT-1), the same one the independent review
identified; it is an *unforced assembly*, not a parity or kind-mismatch no-go.

---

## 2 · Verdict and consolidated arc (corrected)

**UNDERDETERMINED.** The determinant grading is unforced. The FOUND is not rescued
(the operator structure is the imposed target); equally, it is not a hard
structural impossibility (the coefficients are forward-computable scalars).

| Pass | LEDGER | Verdict (corrected) |
|---|---|---|
| Independent review of the "FOUND" | FTD-0232 | overclaim → **UNDERDETERMINED** |
| Determinant grading (parity) | FTD-0233 | **CLOSED-NEGATIVE — scoped** (frozen set *excluding* det_ζ; superseded as operative obstruction) |
| Odd period via J-twisted det_ζ | FTD-0234 | **UNDERDETERMINED** (clean odd scalar `G*` exists; `Det=Tr·G*` unforced) |
| detdet_ζ identity (this) | FTD-0235 | **UNDERDETERMINED** (assemblable from forward scalars but unforced) |

**Consolidated honest status:** the ARC-C1/B2 BCC/quantization observable readout is
**UNDERDETERMINED** — the EM-coupling coefficients `16G*²`, `16G*³` are forward-computable
FTD-native scalars, but *why the EM readout operator has that specific structure* is
the imposed master quadratic (W-CRIT-2). **MC-T4.3 stays a `[FOUNDATIONAL OBSTRUCTION]`**;
the spine is untouched (`x₊=1/α` FTD-0013 stays `[STRONGLY MOTIVATED CONJECTURE]`).
Surviving routes: ARC-D (engine-native measurement) or a new postulate that *forces*
the operator structure. The path that would make this FOUND: a derived reason the
readout operator's determinant carries exactly one extra factor of the det_ζ scalar
relative to its trace (a genuine, forced detdet_ζ correspondence).

---

## 3 · Provenance & discipline

- Deferred commit (owner-authorized); pre-reg SHA `03b967c7…` recorded in-session
  before the analysis. Verified facts in `proof_det_identity.py` (the script's facts
  — `Det = x₊x₋` is an ordinary product; `Tr`/`Det` independent for a 2×2 — are correct
  and support **UNDERDETERMINED**; the script's verdict line is updated accordingly).
- **GTCA discipline note (both directions).** The first version of this verdict
  *over-hardened* the negative (a CLOSED-NEGATIVE leaning on a kind-mismatch that the
  scalar nature of `G_BCC(0)`/det_ζ dissolves). Corrected here to UNDERDETERMINED on
  owner review — guarding against over-claiming a no-go just as much as over-claiming
  a FOUND. The substantive conclusion (FOUND overclaims; MC-T4.3 open) is unchanged.
