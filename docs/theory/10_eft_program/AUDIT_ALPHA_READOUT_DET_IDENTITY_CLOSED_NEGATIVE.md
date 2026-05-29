# AUDIT — det↔det_ζ Structural Identity: CLOSED-NEGATIVE (consolidated MC-T4.3 closure)

**Tag:** `[CLOSED NEGATIVE]` — pre-reg §6 CLOSED-NEGATIVE (sub-tests A and C provably fail). **No spine claim moved.**
**Date:** 2026-05-28
**Result of:** `PREREG_ALPHA_READOUT_DET_IDENTITY_v1.md` (FTD-0219 provisional), SHA256 `03b967c760fa38fffa8c7d08d5a75c34392dcd2c4c546f24a9c58b4d97a78122` (recorded in-session before the analysis; commit deferred by owner).
**Verifier:** [`scripts/proofs/proof_det_identity.py`](../../../scripts/proofs/proof_det_identity.py) — 7/7 verified.
**Consolidates:** the four-pass MC-T4.3 audit arc (see §3).

---

## 0 · Executive summary

This was the **decisive** test: is the readout operator's determinant `16G*³` a **forced** J-twisted ζ-regularized-determinant identity, or an **asserted** factorization? FOUND required A (det *is* a det_ζ, derived) ∧ B (consistent symmetry-breaking) ∧ C (Tr, Det jointly forced). **A and C provably fail → CLOSED-NEGATIVE.** The BCC/quantization observable-readout route to MC-T4.3 is **exhausted**.

- **A fails (V1).** The master-quadratic determinant `16G*³ = x₊·x₋` is an **ordinary finite product** of the two roots (the constant term of a degree-2 polynomial). It is **not** a ζ-regularized determinant of anything. An infinite operator carrying the J-twisted spectrum `{n+¼}/{n+¾}` has det_ζ ratio `= G*` (degree 1), **not** `16G*³` (degree 3). So **no single operator's ζ-regularized determinant is `16G*³`** — the det↔det_ζ identity is not realized.
- **C fails (V7).** For a 2×2 operator, **trace and determinant are independent invariants**: fixing `Tr = 16G*²` leaves `Det` entirely free (verified: same trace, different determinants `64G*⁴` vs `64G*⁴−1`). So `Det = 16G*³` is the master-quadratic **target** (Vieta of FTD-0001), **inserted, not compelled**.
- The 3-plane assembly `16G*³ = |μ₄|² · (det_ζ ratio)³` holds *numerically* but is a product of **three separate** det_ζ ratios × the unit count — **not one operator's det_ζ** — with a trace/det G\*-degree asymmetry (2 vs 3) a symmetric 3-plane tensor product would not produce. It assembles the *value*; it does not realize the *identity*.

**Conclusion: the determinant grading `16G*³` is definitively an unforced assertion of the master-quadratic target, not a forward derivation.** Every rescue route is now closed (§3). The ARC-C1/B2 "FOUND" **overclaims**; honest status **UNDERDETERMINED** (the audit's finding, now complete).

---

## 1 · The two failures (verified)

**A — det↔det_ζ (V1).** A ζ-regularized determinant regularizes an **infinite** spectral product. The master quadratic is **degree 2** — its determinant `16G*³ = x₊x₋` is an ordinary product of **two finite** roots (`≈ 137.036 × 3.024 = 414.39`). The J-twisted ζ-regularized object (Model I) is the *ratio* `det_ζ(D_{3/4})/det_ζ(D_{1/4}) = G* ≈ 2.96` — categorically a different number and a different kind of object. There is no operator whose ζ-regularized determinant *is* `16G*³`. The identity asserted by the resolution docs (and probed via FTD-0218) is not realized.

**C — joint forcing (V7).** Symbolically, a general 2×2 `[[p, q],[r, 16G²−p]]` has trace `16G²` for *all* `p,q,r` but determinant `16G²p − p² − qr` — a **free** function of the entries. Two such operators with identical trace `16G*²` have different determinants. So the master quadratic's specific `Det = 16G*³` (equivalently the relation `Det = Tr·G*`) is **not** forced by the 2×2 structure; it is the imposed Vieta data of FTD-0001. The "extra G\*" in the determinant is the inserted target.

(B was therefore moot — with A and C failing, no consistent symmetry-breaking can rescue a FOUND. For completeness: the single-axis `O_h → C₄` charge localization gives one det_ζ ratio = G\*, consistent with the *trace's* G\* content, but supplies no forced route to the determinant's extra power.)

---

## 2 · The genuine kernel still stands

The closure does **not** retract the real results that emerged across the arc; they are preserved at grade:

- `V_complex ≅ Z[i]²` (BCC C₄ decomposition) — `[THEOREM]` (FTD-0122).
- Charge quantization to `{−1,0,+1}` via the winding index — `[THEOREM]` (FTD-0216 Thm 1).
- `16 = |μ₄|² = |Aut(E)|²` — `[DERIVED]` (FTD-0212).
- Watson self-energy `G_BCC(0) = G*²/(2π)` — `[THEOREM]`.
- The J-twisted det_ζ ratio `= G*` (clean, no prefactor) — `[THEOREM]` (FTD-0141 / Model I).
- Finite-block CLOSED-NEGATIVE (FTD-0215 Thm 2).

These reach **ARC-1** (genuine mathematical structures) + a derived coefficient 16 + a clean odd G\* source. What is **not** reached is **ARC-2/ARC-3**: a forced operator-determinant returning `1/x₊` from first principles.

---

## 3 · The four-pass arc — consolidated

| # | Doc / LEDGER | Route | Verdict |
|---|---|---|---|
| 1 | `AUDIT_ARC_C1_B2_FOUND_INDEPENDENT_REVIEW` | the committed "FOUND" claims | overclaim; honest = **UNDERDETERMINED**; gap = the determinant's odd G\* |
| 2 | `..._DETERMINANT_GRADING_CLOSED_NEGATIVE` (FTD-0217) | frozen ingredients (even-degree) | **CLOSED-NEGATIVE** — odd G\* unreachable; only `√Watson` (F4) |
| 3 | `..._ODD_PERIOD_UNDERDETERMINED` (FTD-0218) | + J-twisted det_ζ ratio | **UNDERDETERMINED** — clean odd source *exists* (G\*), but `Det = Tr·G*` asserted |
| 4 | `..._DET_IDENTITY_CLOSED_NEGATIVE` (FTD-0219, this) | forced det↔det_ζ identity | **CLOSED-NEGATIVE** — A fails (det = ordinary product, not det_ζ) + C fails (Tr, Det independent) |

**Net, definitive:** the determinant grading `16G*³` cannot be forward-forced by any BCC/quantization observable construction — via `√Watson` (closed), via the det_ζ *value* (link unforced), or via a det↔det_ζ *identity* (not realized; Tr/Det independent). The ARC-C1/B2 FOUND **overclaims**; the honest status of the entire BCC/quantization observable readout is **UNDERDETERMINED→exhausted**. **MC-T4.3 remains a `[FOUNDATIONAL OBSTRUCTION]`.** Surviving search space: **ARC-D** (engine-native measurement) or a `[CONJECTURE — new postulate]`. (ARC-A and ARC-B1 were already closed-negative.)

Spine untouched: `x₊ = 1/α` (FTD-0013) `[STRONGLY MOTIVATED CONJECTURE]`; `G*`, master quadratic, coefficient 16 — unchanged (contract §7 forbids tag moves before ARC-3 regardless).

---

## 4 · Recommended record correction (now fully justified; not executed)

With every rescue route closed, the audit's recommendation stands without caveat:

1. **Downgrade `FOUND → UNDERDETERMINED`** in `FOUND_ALPHA_READOUT_QUANTIZATION_RESOLUTION.md` (FTD-0216) and `FOUND_BCC_ALGEBRAIC_READOUT_RESOLUTION.md` (FTD-0215): retitle; correct the §5/§6 "F-j PASS"/"from first principles" to the honest reading (the determinant grading is an asserted Vieta target, not forward-derived); **preserve** §1–§4 genuine content + the finite-block CLOSED-NEGATIVE. Keep FTD-0212 `[DERIVED]/[PARTIAL]` (already correct).
2. **Align LEDGER** FTD-0215/0216 → `[UNDERDETERMINED]`; add FTD-0217/0218/0219 rows.
3. **Reconcile canon** (CLAUDE.md header; `WHERE_WE_LEFT_OFF.md`; `SPEC_OPEN_MATH_BY_SECTOR.md §10.1`; `SPEC_DOCTRINE_LEDGER.md §14`; `SCOPE_ALPHA_READOUT_NEXT_STEPS.md`): MC-T4.3 **open**; ARC-A/B1/B2/C1 closed-negative-or-underdetermined; surviving space ARC-D / new postulate.
4. **No spine tag moves.**

---

## 5 · Provenance & discipline

- Deferred commit (owner-authorized); pre-reg SHA `03b967c7…` recorded in-session before the analysis; design frozen pre-attempt. Canonize with B-9/B-10.
- Compute, not recall: `proof_det_identity.py` (7/7), cross-checked vs `constants.py` `G_STAR`.
- **GTCA F9 guarded.** Across four passes the owner's hints were tested, not rubber-stamped; the genuine partial advances (clean odd source, derived 16, charge quantization) are credited at grade, and the FOUND is **not** manufactured. The verdicts are the structural findings, not the prior.

*The cleanest possible negative: not "we couldn't find it," but "the determinant grading is provably an inserted target, not a forward identity — the BCC/quantization observable route to α is exhausted; the boundary is mapped, and the surviving routes (ARC-D / new postulate) are named."*
