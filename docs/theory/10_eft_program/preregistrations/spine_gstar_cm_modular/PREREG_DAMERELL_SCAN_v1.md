# PRE-REGISTRATION — CM d=−4 Uniqueness via the Full Damerell Ideal-Class Scan (v1)

**Tag:** [PRE-REGISTRATION] — design lock only; **run deferred**.
**Date:** 2026-06-24
**LEDGER row reservation:** FTD-0321
**Hash-lock target tag:** `preregister-damerell-scan-v1`
**Lineage:** upgrades FTD-0123 (`proof_chowla_selberg_higher_h_scan.py`, single-number Γ-product, [NUMERICAL FACT]) and respects the FTD-0124 criterion-bifurcation lesson.

---

## §1 — Motivation & Scope

SPEC_ALGEBRAIC_SPINE.md Theorem 3 (CM-curve uniqueness) is `[NUMERICAL FACT]`: among 63 fundamental imaginary-quadratic discriminants (|d|≤907, class numbers 1–4), only d=−4 reproduces the master-quadratic dual-match. The 2026-06-24 audit demoted the "structural privilege of d=−4 is mathematically proven" claim to `[NUMERICAL FACT]` precisely because (i) it is a finite scan and (ii) it flips under criterion choice (FTD-0124: rational-multiplier admits (d=−3,q=3) at +0.9077 ppm).

**The known incompleteness (`EXPLR_CHOWLA_SELBERG_HIGHER_H.md`):** at class number h≥2 the FTD-0123 scan uses a **single** Γ-product `G*_d = ∏ Γ(a/|d|)^{χ_d(a)}` per discriminant — it projects away the **ideal-class structure**. The arithmetically complete object is the **per-ideal-class Damerell period vector** `(ω_{d,1}, …, ω_{d,h})` (Damerell 1973; Chowla–Selberg 1967). The open question: does the *full* Damerell scan still yield d=−4 as the unique dual-matcher, or do the h−1 additional components at h≥2 unlock new matches?

**Scope:** upgrade Theorem 3 from a single-number finite-scan `[NUMERICAL FACT]` toward a per-ideal-class statement. This is **(b)** in `SPEC_OPEN_MATH_FRONTIERS.md` — closeable with the right tool/effort, NOT a deep external problem. **Pure math; golden-neutral.**

## §2 — Pre-registered targets (LOCKED)

The 54 fundamental imaginary-quadratic discriminants with **h ≥ 2** and |d| ≤ 907 (the complement, within the FTD-0123 range, of the 9 Heegner h=1 discriminants already covered). The h=1 set (d ∈ {−3,−4,−7,−8,−11,−19,−43,−67,−163}) is re-included as a consistency control (d=−4 must remain the matcher there).

## §3 — Pre-registered method (to be frozen in the runner before the run)

Per discriminant d (field K=ℚ(√d), class number h):
1. **Enumerate the ideal classes** of O_K (reduced binary quadratic forms of discriminant d).
2. **Compute the per-class Damerell period** `ω_{d,a}` via the Chowla–Selberg / Damerell formula (Γ-products weighted by the Kronecker character, per ideal class), at dps≥40.
3. **Form the per-class master quadratic** `P_{d,a}(x) = x² − 16 ω_{d,a}² x + 16 ω_{d,a}³` (the d=−4, h=1 instance must reproduce the canonical G\* polynomial exactly — a hard correctness gate).
4. **Dual-match test** against the dimensionless target(s), under the **trivial-multiplier criterion** (q=1), at master-quadratic precision.

## §4 — Pre-registered outcomes

- **UNIQUE-CONFIRMED:** d=−4 remains the only dual-matcher across all h-tuples → Theorem 3 strengthens from "single-number scan" to "per-ideal-class scan" (still `[NUMERICAL FACT]`, but the deepest finite obstruction removed; structural proof remains external).
- **COUNTEREXAMPLE:** some (d, ideal class a) with h≥2 produces a dual-match → Theorem 3's d=−4 uniqueness is **falsified at the per-class level**; record and re-tag. (Also goal-clause-2 progress.)
- **INDETERMINATE:** the Damerell-period computation cannot be validated against the d=−4, h=1 control to required precision → the scan is not trustworthy; do not draw a verdict.

## §5 — Banned moves (the FTD-0124 lesson is load-bearing)

- **No criterion-switching to manufacture uniqueness.** The trivial-multiplier criterion is fixed in §3 *before* the run; the result is reported under it. If the rational-multiplier criterion is also run, BOTH are reported (per FTD-0124), and neither is selected post-hoc to make d=−4 look unique.
- **No tolerance tuning** to include/exclude a specific discriminant.
- **Correctness gate first:** the runner must reproduce the canonical G\* master quadratic at (d=−4, h=1) to machine precision before any h≥2 verdict is credited.
- **No promotion:** UNIQUE-CONFIRMED keeps `[NUMERICAL FACT]` (a finite, per-class scan is not a theorem over *all* CM curves — that needs the analytic machinery listed in `EXPLR_CHOWLA_SELBERG_HIGHER_H.md`).

## §6 — Why the run is DEFERRED (honest)

The per-ideal-class Damerell period is intricate number theory; an incorrect ideal-class enumeration or Gauss-sum normalization would produce a **wrong canonical result** — the exact failure mode this whole audit corrected. Rather than rush it, the methodology is locked here and the implementation (ideal-class enumeration + Damerell periods, ~200 lines, optionally Sage/PARI) is scheduled as a focused follow-up. The §3 correctness gate (reproduce d=−4 h=1) is the first acceptance test of that implementation.

## §7 — Cross-references

FTD-0123 (single-number h≥2 scan), FTD-0124 (criterion bifurcation — the banned-move source), Theorem 3 (`SPEC_ALGEBRAIC_SPINE.md` §3, now `[NUMERICAL FACT]`), `EXPLR_CHOWLA_SELBERG_HIGHER_H.md` (the analytic-machinery list), FTD-0318 (the spine audit that demoted Theorem 3's "mathematically proven"), `SPEC_OPEN_MATH_FRONTIERS.md` F4.

## §8 — Hash-lock procedure

On run: freeze the runner's method + tolerance in its docstring, compute its SHA256, record here, and tag `git tag preregister-damerell-scan-v1 <commit>`. Until then this is a **design lock only** (no run, no verdict).
