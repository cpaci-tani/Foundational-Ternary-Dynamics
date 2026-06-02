# Pre-Registration — MC-T4.3 det↔det_ζ Structural Identity (v1)

**Tag:** `[PRE-REGISTRATION]` — the decisive attempt against the single remaining MC-T4.3 hinge localized by `AUDIT_ALPHA_READOUT_ODD_PERIOD_UNDERDETERMINED.md` (FTD-0234): is the readout operator's determinant **forced** to be the J-twisted ζ-regularized determinant (→ `16G*³`), or not? **Contains no result.** FOUND / UNDERDETERMINED / CLOSED-NEGATIVE pre-blessed.

**Date:** 2026-05-28
**Hash-lock target tag:** `preregister-alpha-readout-det-identity-v1`
**LEDGER row reservation:** FTD-0235 (confirm next-free identifier against `../07_assessment/core_ledgers/LEDGER.md` at hash-lock).
**Decides:** whether MC-T4.3 (operational α-readout) closes **positive** (FOUND ⇒ ARC-3 eligibility for FTD-0013) or whether the BCC/quantization route closes **negative/underdetermined** definitively.
**Builds on:** FTD-0233 (parity no-go: odd G\* unreachable from even ingredients) and FTD-0234 (the J-twisted det_ζ ratio = G\* is a clean odd source, but `Det = Tr·G*` is asserted).
**Companion docs:** `SPEC_FQCR.md` §2 Prop 1 (Model I), `DERIV_GSTAR_QUARTER_CONJUGACY.md`, `FOUND_ALPHA_READOUT_QUANTIZATION_RESOLUTION.md` (the C₄/winding charge quantization), `AUDIT_ARC_C1_B2_FOUND_INDEPENDENT_REVIEW.md`.

> Discipline: §§1–6 SHA-stamped before the attempt; commit deferred per owner. Result in a separate doc. Defective design → v2.

---

## §1 — The single hinge

FTD-0234 established: a clean, forward, FTD-native **odd** power of G\* exists — the J-twisted det_ζ ratio `det_ζ(D_{3/4})/det_ζ(D_{1/4}) = G*` (Model I, `[THEOREM]`, no prefactor). The **only** remaining obstruction to a genuine FOUND is whether the readout operator's determinant **`Det(T) = 16G*³` is FORCED** by a structural identity — `Det(T)` *is* the J-twisted ζ-regularized determinant of `T`'s own spectrum — rather than **asserted** as `Det = Tr·G*`.

For a 2×2 operator, `Tr` and `Det` are **independent** invariants. The master quadratic imposes the dependence `Det = Tr·G*` (`16G*³ = 16G*² · G*`). The question is whether the J-twisted-det_ζ structure forces exactly that one-extra-power-of-G\* dependence.

The owner's hint suggests a 3-plane reading: `16G*³ = |μ₄|² · ∏_{3 planes}(per-plane det_ζ ratio = G*)`, i.e. the determinant carries **three** J-twisted det_ζ ratios (one per spatial plane / "conjugate fields each 3D"), while the trace carries two.

---

## §2 — The question (LOCKED), three sub-tests

**Q-DI.** Is there an FTD-native readout operator `T` such that **all** of the following hold, each `[THEOREM]`/`[DERIVED]`, no §5 falsifier firing?

- **A (det↔det_ζ).** `Det(T)` *is* the ζ-regularized determinant of `T`'s own J-twisted spectrum (a forced operator-determinant ↔ ζ-regularized-determinant identity), **not** the asserted product `Tr(T)·G*`.
- **B (symmetry-breaking consistency).** The structure delivering `Det(T) = 16G*³` (e.g. a 3-plane / full-O_h det_ζ product) is **consistent with** the symmetry-breaking that the charge quantization requires. *(The FTD-0231 winding-index charge quantization needs a localized charge to break `O_h → C₄` about ONE axis, giving ONE `V_complex` and one det_ζ ratio = G\*, degree 1. A degree-3 determinant needs three axes' det_ζ ratios. Q-DI(B) asks whether one preparation supplies both — or whether the trace and determinant require **incompatible** symmetry-breakings.)*
- **C (joint forcing).** Both invariants `Tr(T) = 16G*²` and `Det(T) = 16G*³` follow from the **same** operator structure (not two independent choices), so the dependence `Det = Tr·G*` is forced.

**The decisive locked criterion:** A FOUND requires A ∧ B ∧ C, all derived. If the determinant's extra G\* is asserted (¬A), or requires a symmetry-breaking incompatible with the charge quantization (¬B), or `Tr` and `Det` are independent choices rather than jointly forced (¬C), the attempt does **not** reach FOUND.

---

## §3 — Admissible ingredients (LOCKED)

The FTD-0234 set (frozen ingredients **+** the J-twisted ζ-regularized determinant / Model I ratio = G\*), plus the standard operator-theoretic relations between a transfer operator and its ζ-regularized determinant (`det_ζ T = exp(−ζ_T'(0))`), and the O_h / C₄ / `V_complex` representation theory (FTD-0122).

**Out of scope:** asserting `Det = Tr·G*` or `Det = 16G*³` without the det↔det_ζ derivation; importing FQCR `M_N`; inserting the master quadratic / roots / Theorem 8; a transcendental prefactor; CODATA.

---

## §4 — Benchmark (LOCKED)

`Tr(T) = 16G*²`, `Det(T) = 16G*³` reproduced as **forced** invariants. Numerical comparison only after §5 passes.

---

## §5 — Falsifiers (LOCKED, mechanical)

- **V1 — assertion (¬A).** `Det(T)` is set to `Tr·G*` or `16G*³` without a derived det↔det_ζ identity (`Det(T)` shown to *be* a ζ-regularized determinant of `T`'s spectrum).
- **V2 — incompatible symmetry-breaking (¬B).** The determinant's `G*³` requires a structure (e.g. 3 axes) incompatible with the single-axis `O_h → C₄` breaking the charge quantization relies on; trace and determinant need different, non-co-realizable preparations.
- **V3 — M_N import.** FQCR Model V matrix imported as scaffold.
- **V4 — prefactor.** A transcendental prefactor selected.
- **V5 — insertion.** Master quadratic / G\* value / Theorem 8 inserted as input.
- **V6 — CODATA.** Any empirical α value enters.
- **V7 — independence (¬C).** `Tr` and `Det` are fixed by two *independent* choices rather than jointly forced by one structure (so `Det = Tr·G*` is coincidental, not compelled).

---

## §6 — Three pre-blessed outcomes (LOCKED)

- **FOUND.** A ∧ B ∧ C all hold, derived, no falsifier. ⇒ The readout operator's determinant *is* the J-twisted ζ-regularized determinant, `Det = 16G*³` forced, jointly with `Tr = 16G*²`, consistent symmetry-breaking. **MC-T4.3 closes POSITIVE**; the ARC-C1/B2 result is rehabilitated to a genuine derivation; FTD-0013 becomes ARC-3-eligible (separate ratification); `AUDIT_ARC_C1_B2_FOUND_INDEPENDENT_REVIEW.md` is **superseded** (its overclaim finding overturned). The owner's "lattice is J²/Aut(E²)/3D" hint is fully vindicated.
- **UNDERDETERMINED.** The det↔det_ζ identity is suggestive and the 3-plane reading assembles `16G*³` numerically, but A or B or C is natural-yet-unforced. ⇒ The independent review's UNDERDETERMINED stands; the precise unmet sub-criterion (A/B/C) is recorded for a v3.
- **CLOSED-NEGATIVE.** A or B or C is provably *unforced or impossible* — e.g. V2 (trace and determinant demonstrably require incompatible symmetry-breakings) or V7 (Tr, Det provably independent under the FTD-native structure). ⇒ The BCC/quantization observable route closes negative **definitively**; MC-T4.3's surviving space is ARC-D (engine-native measurement) or a `[CONJECTURE — new postulate]`.

---

## §7 — Method (LOCKED, ordered)

1. **A — det↔det_ζ:** state precisely what operator `T` would have to be for `Det(T)` to *be* a ζ-regularized determinant; test whether `T`'s actual (finite, eigenvalues `x₊, x₋`) structure admits this, or whether the identity requires `T` infinite (with spectrum `{n+¼}/{n+¾}`) — in which case its det_ζ ratio is `G*` (degree 1), **not** `16G*³`. Determine if V1 fires.
2. **B — symmetry-breaking:** determine the axis-count the determinant's `G*³` needs (1 vs 3) and whether it co-realizes with the single-axis charge quantization. Determine if V2 fires.
3. **C — joint forcing:** determine whether one FTD-native structure fixes both `Tr = 16G*²` and `Det = 16G*³` (forcing `Det = Tr·G*`), or whether they are independent. Determine if V7 fires.
4. Apply V1–V7 mechanically. Numerical check only if all pass. Verdict per §6.

---

## §8 — Hash-lock

`sha256sum` recorded in-session; commit deferred per owner; canonize with B-9/B-10. Defective design → v2.

*Authored 2026-05-28. **No result.** Decisive hinge genuinely open; engineering toward FOUND invalidates the attempt.*
