# PRE-REGISTRATION — Rigidity Audit of the Catalog Rational Identifications (v1)

**Tag:** [PRE-REGISTRATION]
**Date:** 2026-06-24
**LEDGER row reservation:** FTD-0320
**Hash-lock target tag:** `preregister-rigidity-catalog-v1`
**Runner (frozen-logic lock):** `scripts/exploration/rigidity_audit_catalog.py`
**Lineage:** extends FTD-0310 (`AUDIT_RIGIDITY_LOADBEARING_v1.md`) — same frozen method + thresholds, broader target set.

---

## §1 — Motivation & Scope

FTD-0311's rigidity-coverage map records that exactly **one** identification (x₊=1/α, FTD-0319) has survived an adversarial uniqueness scan; everything tested since (FTD-0097 monomials, FTD-0262 cluster mass, FTD-0310's three load-bearing ratios) came back NULL or non-rigid; and **~125 `[PARAMETRIC]` claims are tagged-but-unscanned**. This is the framework's deepest open methodological question (the gtca **F10** gap: *a tag labels a claim's status but does not answer whether the framework's integer-combination space is dense enough that some low-complexity expression hits each target by chance*).

This pre-registration extends the FTD-0310 rigidity test to the **directly-testable subset** of the catalog: the simple framework-integer **rational identifications** in `CATALOG_PARAMETRIC_INSERTIONS.md` §7 / §7.1 that still carry the **`[STRUCTURALLY MOTIVATED PARAMETRIC]`** qualifier (i.e. have not yet been rigidity-tested). The catalog itself already notes each has rational competitors within tolerance, so the prior-favoured outcome is **deflationary** (CHANCE_LEVEL / MDL_DOMINATED → demote the "structurally motivated" qualifier). This is a DEBUNKING test in the FTD-0097/0189/0310 tradition, **not** a fishing scan for new matches.

**Out of scope (deferred):** the integer-*combination* families (quark masses, ~90 hadron masses, m_p/m_e, CKM Wolfenstein) are NOT simple rationals; their rigidity test is the FTD-0097-style combinatorial look-elsewhere scan, which FTD-0097 already ran at the monomial level and found **over-rich / NULL**. A full combination-space scan is registered as future work (v2), not run here.

## §2 — Pre-registered targets (LOCKED)

Targets and their framework rationals (experimental values: PDG 2024 neutrino global fit / `constants.py` where present). Each is currently `[STRUCTURALLY MOTIVATED PARAMETRIC]` unless noted:

| name | framework rational | story | experimental target |
|---|---|---|---|
| `sin2_theta_12` | 3/10 | N_c/(N_c+b_3) | 0.307 |
| `sin2_theta_23` | 16/29 | (N_eff+N_c)/(2N_eff+N_c) | 0.546 |
| `dm2_ratio` | 100/3 | (b_3+N_c)²/N_c | 32.8 |
| `sin2_theta_13` | 1/52 | 1/(N_base·N_eff) | 0.0220 (**control** — already plain `[PARAMETRIC]`, 12.6% mis-prediction; included to confirm the method flags it) |

No other targets are added after lock. All four are reported regardless of verdict.

## §3 — Pre-registered method (FROZEN, identical to FTD-0310)

Per identification (T = experimental target, claim = p₀/q₀, e₀ = |p₀/q₀ − T|/T):
1. **MDL / Pareto dominance.** Enumerate reduced rationals p/q with q < q₀ in the bracket [T(1−W), T(1+W)]; a **dominator** is any p/q with q < q₀ AND relerr < e₀ (strictly simpler AND strictly more accurate).
2. **Null-calibrated p-value.** Draw K random targets T′ ~ U[T(1−W), T(1+W)]; p = fraction for which the best rational at q ≤ q₀ achieves relerr ≤ e₀.

## §4 — Pre-registered verdicts & frozen constants

```
MDL_DOMINATED  if a strictly-simpler rational (q<q0) fits strictly better (relerr<e0).
CHANCE_LEVEL   elif p_value >= P_THRESH (the match is routine for its complexity).
RIGID          else (not dominated AND p_value < P_THRESH).
```
**FROZEN CONSTANTS:** `Q_MAX=120, W=0.30, K=200000, P_THRESH=0.05, SEED=20260624`. (Q_MAX/W/K/P_THRESH match FTD-0310 verbatim; only the RNG seed differs, set before execution.) **No threshold is tuned to any per-claim result.**

## §5 — Pre-registered consequences

- **MDL_DOMINATED or CHANCE_LEVEL** → demote the claim from `[STRUCTURALLY MOTIVATED PARAMETRIC]` to `[PARAMETRIC]` (the "structurally motivated" qualifier is not supported — the integer story is not statistically distinguishable from a simple-rational fit). Mirror in LEDGER + `CATALOG_PARAMETRIC_INSERTIONS.md`. For the already-`[PARAMETRIC]` control (sin²θ₁₃), no tag change — it confirms the method.
- **RIGID** (not expected) → leave the tag; record as a second scan-rigid identification alongside x₊=1/α and flag for follow-up. **No promotion** beyond the existing tag without a separate audit.
- Update the FTD-0311 §4 rigidity-coverage map: move the tested claims from bucket (iv) "tagged-but-unscanned" to bucket (iii) "scan-tested".

## §6 — Pre-registered falsifiers / banned moves

- **F-a (no tuning):** the four frozen thresholds are not adjusted post-hoc. Robustness is reported over W ∈ {0.2, 0.3, 0.4, 0.5} (disclosure, like FTD-0310), but the verdict uses W=0.30.
- **F-b (no cherry-picking):** all four targets are reported; none dropped for an inconvenient verdict.
- **F-c (deflationary only):** this scan can only demote or leave-as-is. A RIGID verdict does **not** auto-promote.
- **F-d (no value-planting):** experimental targets are the PDG/constants.py values, fixed before the run.

## §7 — Cross-references

FTD-0310 (the load-bearing-ratio rigidity audit, the method source), FTD-0097 (the monomial-catalog look-elsewhere precedent / the deferred combination layer), FTD-0319 (x₊=1/α, the one scan-rigid exemplar), FTD-0311 (the coverage map this updates), FTD-0318 (the spine audit that spawned this).

## §8 — Hash-lock procedure

After owner review: `git tag preregister-rigidity-catalog-v1 <commit>`; record the runner SHA256 in this section. The runner's verdict logic + thresholds are frozen in its docstring before the run.

**Runner SHA256 (`scripts/exploration/rigidity_audit_catalog.py`, frozen-logic lock):** `5935005107b341bd82741016cb24ba4dc2d68faa9658d48edb7b0e09a4b47efb` (2026-06-24). Result is deterministic (bit-identical SUMMARY on re-run). Run of record + verdicts: `docs/theory/07_assessment/audits/AUDIT_RIGIDITY_CATALOG_v1.md`.
