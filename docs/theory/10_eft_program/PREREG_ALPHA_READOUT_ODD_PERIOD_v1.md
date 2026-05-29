# Pre-Registration — MC-T4.3 Odd-Period via J-Twisted Regularized Determinant (v1)

**Tag:** `[PRE-REGISTRATION]` — locks the design of an attempt to supply the odd power of G\* localized by `AUDIT_ALPHA_READOUT_DETERMINANT_GRADING_CLOSED_NEGATIVE.md` (FTD-0217) through the **J-twisted ζ-regularized determinant** (FQCR Model I), the route suggested by the owner's hint *"the lattice is J²."* **Contains no result.** FOUND / UNDERDETERMINED / CLOSED-NEGATIVE all pre-blessed.

**Date:** 2026-05-28
**Hash-lock target tag:** `preregister-alpha-readout-odd-period-v1`
**LEDGER row reservation:** FTD-0218 (provisional; confirm next-free at lock).
**Builds on:** FTD-0217 (determinant-grading no-go: the odd G\* power is unreachable from `{V_complex, |μ₄|², Watson, winding}` because all are *even* G\*-degree). This pre-reg **broadens** the admissible set by exactly one candidate odd-degree source — the J-twisted ζ-regularized determinant — and asks whether it closes the gap *structurally*.
**Companion docs:** `SPEC_FQCR.md` §2 Prop 1 (Model I, the det_ζ ratio = G\*), `DERIV_GSTAR_QUARTER_CONJUGACY.md` (FTD-0141, full Model I derivation), `AUDIT_ARC_C1_B2_FOUND_INDEPENDENT_REVIEW.md`, `AUDIT_ALPHA_READOUT_DETERMINANT_GRADING_CLOSED_NEGATIVE.md`.

> Discipline: §§1–6 hash-stamped (SHA256 recorded) before the attempt; commit deferred per owner instruction (provenance via in-session SHA + script, canonized later with B-9/B-10). Result lands in a separate doc.

---

## §1 — The localized gap and the candidate source

FTD-0217 proved: the master-quadratic determinant `16G*³` is **odd** G\*-degree (3); the frozen ingredients are **even**-degree (0, 2); forward operations keep the determinant even-degree; the only even→odd route within the frozen set is `√Watson` = the field amplitude, which is a forbidden prefactor selection (F4). The missing piece is a **forward, FTD-native, clean odd power of G\***.

**Candidate (owner's hint "the lattice is J²"):** FQCR Model I (`SPEC_FQCR.md` §2 Prop 1, `[THEOREM]`):

$$\frac{\det{}_\zeta D_{3/4}}{\det{}_\zeta D_{1/4}} = \frac{\Gamma(1/4)}{\Gamma(3/4)} = G^* \qquad(\text{degree 1, ODD; clean — the }\sqrt{2\pi}\text{ cancels in the ratio}),$$

where `D_{1/4}, D_{3/4}` are the quarter-twisted spectra of the conjugacy operator `J` (`J² = −I`) on `V_complex`. This is a **degree-1 G\* with no √π prefactor** — exactly the odd-degree object the no-go showed is needed, and it lives on the **same `J²=−I` structure** as the readout's `V_complex`.

---

## §2 — The question (LOCKED)

**Q-OP.** Is there a forward FTD-native readout operator `T` on `V_complex` such that:

1. `Tr(T) = 16G*²` arises from `|μ₄|² · 2π · G_BCC(0)` (granted plausible per FTD-0217 step 2);
2. `Det(T) = 16G*³`, where the **extra degree-1 factor `G*` is the J-twisted ζ-regularized-determinant ratio** `det_ζ(D_{3/4})/det_ζ(D_{1/4}) = G*` (Model I), arising **STRUCTURALLY** — *because the operator determinant `Det(T)` IS a ζ-regularized determinant of the J-twisted spectrum* — **not** by asserting `Det = Tr·G*`, **not** by importing the FQCR `M_N`, **not** via a selected prefactor;
3. and the construction is admissible under §5?

**The single decisive hinge (LOCKED):** whether the identity **`Det(T) ↔ (J-twisted det_ζ ratio) × (energy content)`** is a *forced structural property* of the readout operator, or an unforced selection. Everything turns on this. The √(2π)-cancellation in Model I removes the prefactor objection; the *only* remaining question is whether the determinant is **compelled** to carry the J-twisted det_ζ ratio.

---

## §3 — Admissible ingredients (LOCKED)

The FTD-0217 frozen set `{V_complex ≅ Z[i]² ; |μ₄|² = 16 ; Watson G_BCC(0) = G*²/(2π) ; winding}` **plus**:

- **The J-twisted ζ-regularized determinant** `det_ζ D_{a}` and the Model I ratio `det_ζ(D_{3/4})/det_ζ(D_{1/4}) = G*` (`SPEC_FQCR.md` §2 Prop 1, `DERIV_GSTAR_QUARTER_CONJUGACY.md`), on the `J²=−I` structure of `V_complex`. Admissible **only as the regularized determinant of the readout operator's own J-twisted spectrum** — i.e. the construction must *exhibit* `Det(T)` as this object, not cite the number `G*`.

**Out of scope:** the master quadratic / its coefficients / roots (FTD-0001); the FQCR Model V operator `M_N` (Prop 5); Theorem 8 root properties (FTD-0111); `√Watson` as the odd source (closed by FTD-0217); any selected transcendental prefactor; CODATA/α.

---

## §4 — Benchmark (LOCKED)

Target: `Det(T) = 16G*³` reproduced forward. Numerical comparison (to `16G*³` / `1/x₊`) only after §5 passes; benchmark-side, never an input.

---

## §5 — Falsifiers (LOCKED, mechanical)

- **OP1 — assertion.** `Det(T) = 16G*³` (or `Det = Tr·G*`) is *asserted/posited* rather than derived from the operator's structure. (This is the defect FTD-0215/0216 had.)
- **OP2 — M_N import.** The FQCR Model V matrix is imported as scaffold.
- **OP3 — unforced det↔det_ζ link.** No structural reason is given that the readout operator's determinant *is* the J-twisted ζ-regularized determinant; the identification is a choice. (The decisive falsifier.)
- **OP4 — prefactor.** A transcendental prefactor (√π, etc.) is selected to make the value land.
- **OP5 — insertion.** The master quadratic / `G*` value / Theorem 8 inserted.
- **OP6 — CODATA.** Any empirical α value enters.
- **OP7 — look-elsewhere.** The odd factor is chosen among several candidate periods without an a-priori forced reason.

---

## §6 — Three pre-blessed outcomes (LOCKED)

- **FOUND.** `Det(T)` is shown, by a `[THEOREM]`/`[DERIVED]` structural argument, to **be** the J-twisted ζ-regularized determinant of the readout operator, equal to `16G*³`; no OP fires. ⇒ The FTD-0217 parity no-go is *escaped* (the J-twisted det_ζ is a genuine forward odd-degree source); the ARC-C1/B2 **FOUND is rescued**, and `AUDIT_ARC_C1_B2_FOUND_INDEPENDENT_REVIEW.md` is **revised** (its overclaim finding overturned for this route). The owner's "lattice is J²" hint is vindicated.
- **UNDERDETERMINED.** The J-twisted det_ζ ratio is real and FTD-native, but its identification with the *operator determinant* is natural-yet-unforced (OP3 borderline) — a structural lead, not a closure. ⇒ The independent review's UNDERDETERMINED stands; the lead is documented for a future v2.
- **CLOSED-NEGATIVE.** No structural compulsion links the operator determinant to the J-twisted det_ζ ratio; supplying the odd G\* still requires assertion (OP1) or selection (OP3/OP4). ⇒ Hardens the no-go: even with the J-twisted det_ζ admitted, the determinant grading is not forward-forced.

---

## §7 — Method (LOCKED, ordered)

1. Read Model I in full (`DERIV_GSTAR_QUARTER_CONJUGACY.md`): the precise objects `D_{1/4}, D_{3/4}`, the det_ζ definition, and the `J²=−I` provenance.
2. Construct the readout operator `T` on `V_complex` forward; identify its J-twisted spectrum.
3. **Test the hinge:** is `Det(T)` *structurally* the ζ-regularized determinant of that spectrum (a forced identity, e.g. via the transfer-operator/partition-function ↔ det_ζ correspondence), or must it be asserted? State the argument and which way OP1/OP3 fall.
4. If forced: assemble `Det = 16 · G*² · (det_ζ ratio) = 16G*³`; verify no OP4 prefactor; confirm the √(2π) cancellation.
5. Apply OP1–OP7 mechanically.
6. Numerical check (only if 1–5 pass).
7. Verdict per §6.

---

## §8 — Hash-lock

`sha256sum` this file; record in-session and (at canonization) in `REF_PREREGISTER_MANIFEST.md` + LEDGER (FTD-0218). Commit deferred per owner; if canonized, commit pre-reg first then verdict separately (B-9) with an independent review (B-10). Defective design → v2, not an edit.

*Authored 2026-05-28. **No result.** The hinge (is the operator determinant *forced* to be the J-twisted det_ζ ratio?) is genuinely open; engineering toward FOUND invalidates the attempt.*
