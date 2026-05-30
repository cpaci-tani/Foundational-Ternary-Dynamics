# AUDIT — Independent adversarial review of the ARC-C1 / ARC-B2 "FOUND" α-readout verdicts

**Tag:** `[AUDIT]` — independent adversarial review (GTCA P4 mode). **Recommends an epistemic correction to committed work; executes no tag moves.** No spine claim is affected either way (FTD-0013 stays `[STRONGLY MOTIVATED CONJECTURE]` under every reading below).
**Date:** 2026-05-28
**LEDGER row:** FTD-0232
**Reviewer scope:** the two "FOUND-at-ARC-2" verdicts for MC-T4.3 (the operational α-readout obstruction):
- `FOUND_ALPHA_READOUT_QUANTIZATION_RESOLUTION.md` (ARC-C1, LEDGER **FTD-0231**)
- `FOUND_BCC_ALGEBRAIC_READOUT_RESOLUTION.md` (ARC-B2, LEDGER **FTD-0230**)
**Audited against:** the locked pre-regs `PREREG_ALPHA_READOUT_QUANTIZATION_v1.md` (FTD-0231) §4–§5 and `PREREG_ALPHA_READOUT_BCC_BRIDGE_v1.md` (FTD-0230); the project's own no-cheat audit `AUDIT_CHARGE_QUANTIZATION_NO_CHEAT.md` (FTD-0231); the honest companion `DERIV_BCC_ALGEBRAIC_READOUT.md` (FTD-0212); the contract `SPEC_ALPHA_READOUT_CONTRACT.md` §7.
**Verdict of this review:** the **"FOUND" label is an overclaim**. By the verdicts' *own locked pre-registration criteria*, the honest status of both ARC-C1 and ARC-B2 (infinite-aperture) is **UNDERDETERMINED**. The genuine, defensible advance is real and should be preserved at its honest grade.

---

## 0 · Executive summary

Both "FOUND" docs derive a genuine partial result and then take one unjustified step:

**Genuinely established (`[THEOREM]`-grade, preserve):**
1. A localized charge breaks `O_h → C₄`; the 8 BCC corners give `V_complex ≅ Z[i]²` (FTD-0122 bedrock, OT-1.x).
2. Charge quantizes to `{−1, 0, +1}` as the topological winding index on `V_complex` — a clean, novel match to the ternary alphabet (FTD-0231 Thm 1).
3. `16 = |μ₄|² = |Z[i]^×|² = |Aut(E)|²` — the master-quadratic coefficient **16** derived as the squared Gaussian-unit-group order (FTD-0212, honestly tagged `[DERIVED]`).
4. The BCC self-energy is the Watson integral `G_BCC(0) = G*²/(2π)` (Spine Theorem; FTD-0230 Thm 3).
5. On any **finite block**, the readout **CLOSED-NEGATIVE** — a finite transfer operator has algebraic eigenvalues, not the transcendental `G*` (FTD-0230 Thm 2, correct and honest).

**The unjustified step (the overclaim):** the leap from items 3–4 to the **full** master quadratic `x² − 16G*²x + 16G*³ = 0`, hence to `x₊ = 1/α`. This requires a transfer operator with **trace = 16G\*²** *and* **determinant = 16G\*³**. The Watson limit supplies only the **scalar** `G*²/(2π)` (quadratic in G\*); the determinant's **extra, third power of G\*** is *not derived* — it is **asserted/selected to match the target polynomial.**

This is confirmed five independent ways (§2). The honest verdict per the locked pre-reg FTD-0231 §4 is **UNDERDETERMINED** ("a partial topological map is achieved, but the relation … is not fully derived"). **MC-T4.3 is therefore NOT resolved**; it remains a `[FOUNDATIONAL OBSTRUCTION]`.

---

## 1 · What the verdicts genuinely establish (and must keep)

This review is adversarial but fair. The following are real and should be preserved at the grades shown — the correction below does **not** retract them:

| Result | Grade | Source |
|---|---|---|
| `V_complex ≅ Z[i]²` from the BCC `C₄` decomposition | `[THEOREM]` | FTD-0122 / OT-1.x bedrock |
| Charge quantization to `{−1,0,+1}` via winding index on `V_complex` | `[THEOREM]` | FTD-0231 Thm 1 |
| `16 = |μ₄|² = |Aut(E)|²` (coefficient 16, derived) | `[DERIVED]` | FTD-0212 §3 (honest) |
| BCC self-energy `G_BCC(0) = G*²/(2π)` (Watson) | `[THEOREM]` | FTD-0230 Thm 3 / Spine Thm 5 |
| Finite-block readout closes negative | `[CLOSED NEGATIVE]` | FTD-0230 Thm 2 |

Together these reach **ARC-1** (mathematical structures genuinely contacted) **plus** a structural derivation of the coefficient 16. That is a real contribution. It is *not* ARC-2 (an operational readout returning `1/x₊` from first principles).

---

## 2 · The defect — the determinant's third power of G\* is selected, not derived

The master quadratic has, by Vieta, **sum = 16G\*²** and **product = 16G\*³**; the two coefficients differ by exactly one factor of **G\***. The Watson self-energy provides `G*²` (via `2π·G_BCC(0) = G*²`). The **product/determinant requires one more power of G\***, and nothing in the construction derives it. Five independent confirmations:

**(a) FTD-0231 admits it (self-contradiction).** §3 "Selection 1" states verbatim that `Tr(T_BCC) = 16G*²`, `Det(T_BCC) = 16G*³`, and the prefactor `R = 8√(2π)G*` are *"**epistemic selections designed to couple** the lattice Green's function evaluation **to the algebraic master quadratic** … rather than unique, first-principles derivations."* Yet §5 asserts *"F-j (No reverse-engineering): **PASS** … emerge directly … rather than being pasted as scaffold,"* and §6 claims *\"deriving … **from first principles**.\"* §3 and §5/§6 cannot both be true. A coefficient "designed to couple to the master quadratic" **is** the target pasted as scaffold.

**(b) FTD-0230 obscures the same gap.** §4 Thm 3 derives only the scalar `O_EM = G_BCC(0) = G*²/(2π)`, then **simply asserts** `Det(T_BCC) = 16G*³` and `Tr(T_BCC) = 16G*²` with no derivation that the operator has these invariants. The cubic-in-G\* determinant does not follow from the quadratic-in-G\* self-energy.

**(c) Against the locked pre-reg FTD-0231 §4.** FOUND requires *"a **rigorous derivation trace** … yielding the master quadratic roots … relative error … 0."* UNDERDETERMINED is defined as *"a **partial topological map** is achieved, but the relation between the winding number and QED charge normalization is **not fully derived**."* The construction's bridge is, by its own admission (a), not derived — this is the **UNDERDETERMINED criterion verbatim**, and it trips the pre-registered **F-j** ("Reverse-engineers … by inserting the target master quadratic as a scaffold").

**(d) Against the project's own no-cheat audit FTD-0231.** Gate 4 requires *"an explicit, mathematically **proven** projection map from the multi-block lattice operators to the lemniscatic CM curve invariants."* Selection 1 admits there is only a *selected* map. **Gate 4 fails by the doc's own admission** (and Gate 2, "no scheme chosen *because* it yields the value," is strained for the same reason).

**(e) Cross-document inconsistency.** The honest companion FTD-0212 (`DERIV_BCC_ALGEBRAIC_READOUT`) tags itself `[DERIVED]/[PARTIAL]` and claims **only** the coefficient `16 = |μ₄|²` plus a `[PARTIAL]` operational protocol — it does **not** claim the full master quadratic. FTD-0230/0231 then build a **stronger** ("FOUND") verdict on top of that `[PARTIAL]` foundation plus a `[SELECTION]`. A verdict cannot be stronger than its weakest load-bearing step.

> **Note on what is *not* wrong.** Items 1–5 of §1 are sound; the finite-block CLOSED-NEGATIVE (FTD-0230 Thm 2) is correct; the spine is untouched; and the docs deserve credit for tagging the prefactor `[SELECTION]` *somewhere* (FTD-0231 §3) — the failure is that the **verdict label and §5/§6 conclusions ignore that tag** (GTCA failure mode F10: treating a `[SELECTION]` as if it resolved what it only labels).

---

## 3 · Honest verdict

| Doc | Claimed | Honest (per its own locked pre-reg) |
|---|---|---|
| ARC-C1 — FTD-0231 | **FOUND (ARC-2)** | **UNDERDETERMINED** — genuine `[THEOREM]` topological-quantization + coefficient-16 map; the bridge to the full master quadratic / `x₊` is an admitted `[SELECTION]` (F-j fires per §3 admission) |
| ARC-B2 — FTD-0230 | **FOUND (ARC-2)** in `L→∞`; CLOSED-NEGATIVE finite | finite-block CLOSED-NEGATIVE **stands**; `L→∞` "FOUND" → **UNDERDETERMINED** — Watson self-energy `G*²/(2π)` is genuine, but `Det = 16G*³` (the third G\* power) is asserted, not derived |
| ARC-B2 deriv — FTD-0212 | `[DERIVED]/[PARTIAL]` | **correct as written** — the model for honest tagging here |

The contract `SPEC_ALPHA_READOUT_CONTRACT.md` §7 already guarantees *"no tag changes occur before ARC-3,"* and ARC-3 requires returning `1/x₊` **without target input** — which neither doc achieves. So **FTD-0013 (`x₊ = 1/α`) is `[STRONGLY MOTIVATED CONJECTURE]` under every reading**; nothing here promotes or demotes the spine.

---

## 4 · Consequence for MC-T4.3

MC-T4.3 (the operational α-readout mechanism) is **NOT resolved.** Honest program status after this review:

- **ARC-A** (boundary-condition): `[CLOSED NEGATIVE]` (stands).
- **ARC-B1** (observable-selection, items 4/6/7): `[CLOSED NEGATIVE]` (stands; FTD-0204/0205).
- **ARC-B2 / ARC-C1** (BCC bridge / quantization): **`[UNDERDETERMINED]`** — a genuine partial result (topological charge quantization on `V_complex ≅ Z[i]²` + coefficient `16` + Watson self-energy + finite-block no-go), with the bridge to the master quadratic / `x₊` an open `[SELECTION]`.
- **ARC-D** (discrete-native measurement): unattempted.

The open mathematical gap is now **sharply localized**: *derive the determinant's third power of G\** — i.e., show forward, from `V_complex`/Watson structure and **without inserting the master quadratic**, that the readout operator's determinant is `16G*³` (equivalently, that the coefficient grading is `(G*², G*³)` rather than `(G*², G*²)`). This is the precise object a future ARC-B2-v2 / ARC-C-v2 must produce. The `DERIV_BCC_COMPLEX_STRUCTURE.md` Z[i]-module (FTD-0122) is the natural place to look, but operationalizing it without firing F-j is exactly what remains open (as `SCOPE_ALPHA_READOUT_NEXT_STEPS.md` §1 Route 3 itself flags: "the open gap is designing an *operational* measurement protocol … without violating … F-j").

---

## 5 · Recommended corrections (NOT executed — pending owner sign-off)

This review changes committed verdicts and the canonical record, so it executes nothing. Recommended, in order:

1. **Downgrade the two verdict docs `FOUND → UNDERDETERMINED`** (FTD-0230, FTD-0231): retitle, fix §6 ("from first principles" → the honest partial statement), and correct the §5 "F-j PASS" to "F-j fires on the trace/determinant selection; verdict UNDERDETERMINED per §4." Preserve §1–§4 genuine content and the finite-block CLOSED-NEGATIVE.
2. **Align LEDGER** rows FTD-0230, FTD-0231 to `[UNDERDETERMINED]`, citing this review. Keep FTD-0212 at `[DERIVED]/[PARTIAL]` (already correct) and FTD-0231 (no-cheat audit) unchanged.
3. **Reconcile the canon** (separately stale anyway): CLAUDE.md header, `WHERE_WE_LEFT_OFF.md`, `SPEC_OPEN_MATH_BY_SECTOR.md §10.1`, `SPEC_DOCTRINE_LEDGER.md §14`, and `SCOPE_ALPHA_READOUT_NEXT_STEPS.md` should state: ARC-A/B1 closed-negative; ARC-B2/C1 **UNDERDETERMINED** (not FOUND); MC-T4.3 **open**, gap localized to the G\*³ grading.
4. **No spine tag moves** (FTD-0013/0001/0006 untouched) — required either way by contract §7.

If, on the contrary, a reviewer can exhibit the forward derivation of `Det = 16G*³` from `V_complex`/Watson without inserting the master quadratic, then this review is wrong and the FOUND stands — that is the single falsifiable hinge.

---

## 6 · Provenance

Independent adversarial review conducted 2026-05-28, desk analysis only, against the committed corpus and locked pre-regs. Method: GTCA P4 (aesthetic-inverted, monitor-hot), refute-by-default. The review's own load-bearing claim — that the determinant's third G\* power is selected, not derived — is itself falsifiable (§5 hinge) and should be independently checked before the §5 corrections are executed.
