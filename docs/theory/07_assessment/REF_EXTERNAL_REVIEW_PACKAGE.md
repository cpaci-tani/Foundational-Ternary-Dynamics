# REF — External human-review dispatch package (v1)

**Tag:** `[REFERENCE — dispatch package]`. Prepared 2026-07-25 against the standing meta-gap of [`TRACKER_OPEN_ITEMS.md`](core_ledgers/TRACKER_OPEN_ITEMS.md) §0: *zero items in this corpus have been reviewed by a human outside the project; every adversarial pass to date is AI-generated self-critique.* This package is the dispatch-ready close-path. **It does not and cannot close the §0 item** — only a named external human's completed review, booked with date and findings, closes it. Dispatch is an owner action.

---

## 1 · What is being asked, of whom

**Reviewer profile (Package A, the priority):** one mathematician with working knowledge of transcendence theory or CM elliptic curves (arithmetic geometry / analytic number theory). No physics background required, and none should be assumed: Package A contains no physics claims.

**The ask:** verify or refute the seven theorem-grade results of the algebraic spine, at either of two tiers —

- **Tier 1 (spot check, ~2–4 hours):** read the claim statements (§2), run the verification scripts (§4), and check the two proofs the project itself flags as most load-bearing: the Watson identity route through Chowla–Selberg, and the `ℚ(G*)` π-freeness argument's use of Chudnovsky 1976.
- **Tier 2 (full, ~1–2 days):** additionally audit the CM-uniqueness scan domain (the OT-1.9 caveat), the Sym²⊕Sym³ constraint-set correction (OT-2.7, which contains a self-documented retraction — a good probe of whether the corpus's corrections are real), and the coefficient-16 status (OT-4.1, the project's own declared softest spot).

## 2 · The minimal claim set (nothing else is being submitted)

All statements live in [`SPEC_ALGEBRAIC_SPINE.md`](../01_reference/SPEC_ALGEBRAIC_SPINE.md); tier assignments in [`TRACKER_ONTIC_TRUTH.md`](core_ledgers/TRACKER_ONTIC_TRUTH.md). The seven theorem-grade results: the master quadratic's root structure (OT-1.1); `G* = Γ(1/4)/Γ(3/4) = Γ(1/4)²/(π√2)` (OT-1.2); the harmonic-invariant tower `1/y₊ + 1/y₋ = 1` (OT-1.3); the Phase-G geometric Coulomb identity (OT-1.4); CM-uniqueness of `ℚ(i)` under the unit-group/discriminant coincidence (OT-1.9); the Watson identity `W₃ = G*²/(2π)` conditional on Watson 1939 / Glasser–Zucker 1980 (OT-2.1); and `ℚ(G*) ∩ ℚ(π) = ℚ` conditional on Chudnovsky 1976 (OT-2.3).

**The named external dependency to scrutinize:** the whole conditional half rests on Chudnovsky 1976 (algebraic independence of π and Γ(1/4)), consolidated in Waldschmidt's Grundlehren volume. If the reviewer believes this dependency is miscited or overdrawn, that finding alone justifies the review.

## 3 · Mandatory disclosures (send verbatim)

1. Every "red-team," "referee," and "adversarial" document in this corpus is AI self-critique; the project's own failure-mode catalog names the risk this creates (progressive collusion toward defensible readings). You are the first external check. Distrust accordingly.
2. The physics identification `x₊ = 1/α` is tagged `[STRONGLY MOTIVATED CONJECTURE]`, is not part of Package A, and no review of it is requested. No α is derived anywhere in the project, by the project's own statement of record.
3. The project's declared pressure points (TRACKER_ONTIC_TRUTH "Pressure points" section) are part of the package: coefficient-16 (OT-4.1) is a value-level coincidence with a conjectured structural reason; the h ≥ 2 Damerell scan has not been run; pre-registration tag dates are checkable against measurement dates in git and the reviewer is invited to check them.

## 4 · Runnable verification (all paths repo-relative; Python ≥ 3.10, mpmath/numpy/sympy)

| Script | Verifies | Expected |
|---|---|---|
| `scripts/proofs/proof_master_verification.py` | spine + framework consistency battery | 54/54 PASS |
| `scripts/proofs/proof_harmonic_invariant_tower.py` | OT-1.3 at 50-digit precision | 14/14 PASS |
| `scripts/proofs/proof_bcc_complex_structure.py` | OT-1.5/1.6 in exact rationals | 5/5 PASS |
| `scripts/proofs/proof_fqcr_convergence.py` | OT-1.8 finite-N attractor rate | all assertions PASS |
| `scripts/proofs/proof_field_theoretic_qgstar.py` | OT-2.3 π-freeness route | PASS |
| `scripts/proofs/proof_polynomial_look_elsewhere_extended.py` | OT-3.3 scan (numerical fact only) | 0 dual-matchers / 2,871,576 |

A reviewer who prefers independence should recompute OT-1.2 and OT-2.1 in their own CAS; both are one-session computations (the Watson integral to 100 digits in PARI/GP reproduces `G*²/(2π)`).

## 5 · What the project commits to on receipt

Findings are booked verbatim into the LEDGER under a new FTD id with the reviewer named (or recorded anonymous-by-request); any claim the review breaks is demoted the same day, per the standing discipline that demotion is free and promotion requires proof. The §0 tracker item is then updated from "zero external review" to the dated fact of this review, whatever its verdict.

## 6 · Dispatch instructions (owner)

The package = this file + `SPEC_ALGEBRAIC_SPINE.md` + `TRACKER_ONTIC_TRUTH.md` + the six scripts above (or repo access read-only). Suggested channels, in order of signal quality: a direct approach to a transcendence-theory or CM researcher (the Waldschmidt-school orbit is the natural audience for OT-2.2/2.3); a math.NT-adjacent colleague of any collaborator; a carefully-scoped MathOverflow question restricted to one claim (lowest cost, lowest depth). Sending the package publishes its contents; that decision, and any posting, is the owner's alone. Nothing in this file authorizes publication.

## 7 · Status

Package `READY`. TRACKER_OPEN_ITEMS §0 remains `[OPEN]` and must remain `[OPEN]` until a completed external review is booked. This document changes no claim status and reviews nothing itself.
