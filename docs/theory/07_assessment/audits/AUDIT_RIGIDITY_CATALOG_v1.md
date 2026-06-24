# AUDIT — Rigidity of the Catalog Rational Identifications (v1)

**Tag:** [MEASURED — rigidity audit]
**Date:** 2026-06-24
**LEDGER row:** FTD-0320
**Pre-registration:** `PREREG_RIGIDITY_CATALOG_v1.md` (tag `preregister-rigidity-catalog-v1`)
**Runner:** `scripts/exploration/rigidity_audit_catalog.py` (SHA256 `5935005107b341bd82741016cb24ba4dc2d68faa9658d48edb7b0e09a4b47efb`; deterministic)
**Lineage:** extends FTD-0310 (same frozen MDL + null-calibration method + thresholds Q_MAX=120, W=0.30, K=200000, P_THRESH=0.05; seed 20260624).
**Outcome:** deflationary — **0 RIGID; all three tested `[STRUCTURALLY MOTIVATED PARAMETRIC]` claims demote to `[PARAMETRIC]`.** Nothing promoted.

---

## 1. Question

The gtca **F10** gap: a `[STRUCTURALLY MOTIVATED PARAMETRIC]` tag asserts that a framework-integer rational identification is more than a chance fit, but does not test it. This audit applies the FTD-0310 rigidity test to the simple-rational identifications in `CATALOG_PARAMETRIC_INSERTIONS.md` §7/§7.1 that still carried that qualifier and had not been scanned. (Integer-*combination* families are out of scope — deferred to a v2 combinatorial scan; FTD-0097 already found the monomial catalog over-rich/NULL.)

## 2. Results (run of record, frozen thresholds)

| claim | framework story | rel. err | MDL dominator (simpler AND better) | null p-value | robustness (W=0.2–0.5) | **verdict** |
|---|---|---|---|---|---|---|
| **sin²θ₁₂ = 3/10** | N_c/(N_c+b_3) | 2.28% | none | **0.481** | 0.44–0.49 | **CHANCE_LEVEL** |
| **sin²θ₂₃ = 16/29** | (N_eff+N_c)/(2N_eff+N_c) | 1.05% | **6/11** (0.100%) | 0.960 | 0.94–0.96 | **MDL_DOMINATED** |
| **Δm²₃₁/Δm²₂₁ = 100/3** | (b_3+N_c)²/N_c | 1.63% | **33/1** (0.610%) | 1.000 | 1.0 | **MDL_DOMINATED** |
| sin²θ₁₃ = 1/52 *(control, already [PARAMETRIC])* | 1/(N_base·N_eff) | 12.59% | **1/41** (10.87%) | 0.872 | 0.72–1.0 | **MDL_DOMINATED** |

**RIGID count: 0.** Result is deterministic (bit-identical SUMMARY on re-run).

## 3. Reading

- **sin²θ₂₃ = 16/29 is dominated by 6/11** — a strictly simpler rational (denom 11 < 29) that is **~10× more accurate** (0.100% vs 1.05%). The (N_eff+N_c)/(2N_eff+N_c) integer story does no work; the framework chose a more complex fraction than the data prefers.
- **Δm²₃₁/Δm²₂₁ = 100/3 is dominated by 33/1** (the catalog already flagged "33/1 fits within 1%"); null p = 1.000 — hitting this target at q≤3 is certain.
- **sin²θ₁₂ = 3/10 is chance-level** (null p = 0.48): not dominated, but hitting it this well at its complexity is routine — not statistically special.
- The **control sin²θ₁₃ = 1/52** is correctly flagged MDL_DOMINATED (1/41 is simpler and better; it is a known 12.6% mis-prediction), confirming the method discriminates.

**None of the three is statistically distinguishable from a simple-rational fit.** The "structurally motivated" qualifier is unsupported.

## 4. Consequences applied (demotion-only, per pre-reg §5)

- `sin²θ₁₂ = 3/10`, `sin²θ₂₃ = 16/29`, `Δm²₃₁/Δm²₂₁ = 100/3`: **`[STRUCTURALLY MOTIVATED PARAMETRIC]` → `[PARAMETRIC]`** in `CATALOG_PARAMETRIC_INSERTIONS.md` §7.1 and the LEDGER.
- `sin²θ₁₃ = 1/52`: no tag change (already `[PARAMETRIC]`); recorded as the method's positive control.
- FTD-0311 §4 rigidity-coverage map: these four move from bucket (iv) "tagged-but-unscanned" to bucket (iii) "scan-tested → not rigid". The scan-rigid count stays **exactly one** (x₊=1/α, FTD-0319).

## 5. Scope / what this does NOT do

- It does **not** test the integer-*combination* families (quark/hadron masses, m_p/m_e, CKM Wolfenstein) — those are not simple rationals; their rigidity is the FTD-0097-style combinatorial look-elsewhere scan (registered as deferred v2 work). The bucket-(iv) "tagged-but-unscanned" population therefore remains large (~120); this audit retires only the simple-rational subset.
- A RIGID verdict, had one occurred, would **not** have auto-promoted anything (pre-reg F-c).

## 6. Bottom line

Every framework-integer *rational* identification tested to date — FTD-0310's three load-bearing ratios and now these four — comes back NULL, chance-level, or MDL-dominated. The one identification that survives an adversarial uniqueness scan remains **x₊ = 1/α** alone (and that at `[SMC]`, supported-not-derived). This is goal-clause-2 progress: a sharper, honestly-bounded map of what the integer catalog does and does not buy.
