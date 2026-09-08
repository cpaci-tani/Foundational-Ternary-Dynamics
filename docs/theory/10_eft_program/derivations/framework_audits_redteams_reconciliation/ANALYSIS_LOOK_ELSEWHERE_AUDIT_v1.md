# FTD-0791 — The FTD-0319 Look-Elsewhere Audit v1

**Status:** `[AUDIT — INDEPENDENTLY REPLICATED]` +
`[RETAG — FTD-0319 FROM [MEASURED] TO [SELECTION]]` +
`[WITHDRAWAL — "THE FRAMEWORK'S SINGLE SCAN-RIGID IDENTIFICATION"]` +
`[CORRECTION — FTD-0785's RECORDED LEAD]`
**Verdict:** `ALPHA_MATCH_SITS_AT_THE_CHANCE_BASE_RATE`
**Parents:** `FTD-0319`, `FTD-0013`, `FTD-0014`, `FTD-0387`, `FTD-0785`
**Production impact:** none — documentation and tagging only

## 1. Why this existed

FTD-0319's uniqueness scan is the only evidence that `x_+ = 137.0361714582`
is structure rather than coincidence, and it had **never been audited**. This
document is that audit, run refute-by-default and then **independently
replicated by a second implementation** which reproduced every number below
to the printed digit.

## 2. What was scanned

`tools/scan_adversarial_look_elsewhere.py`, preregistration genuine and
intact: runner SHA256 matches the prereg byte-for-byte, tag
`preregister-adversarial-look-elsewhere-v1` on commit `9e5ad8f`
(2026-05-21), and `git log` shows no modification since. **The family was
frozen before the run — this part of the methodology is clean.**

- Family `x^2 - c1*K^a*x + c2*K^b`, `c1,c2 in [1,64]`, `a,b in [0,5]`, 18-constant
  basket: **2,654,208** polynomials (2,171,802 real-rooted). Replicated exactly.
- Gate: `|x_+ - 1/alpha|/(1/alpha) < 2.0e-6` **AND** `|x_- - N_c|/N_c < 1.0e-2`.

## 3. Finding 1 — leg 1 sits exactly at the chance base rate

Counting against `1/alpha` with **no second leg**:

| tolerance | observed | linear null | ratio |
|---|---|---|---|
| `1e-3` | 712 | 712.0 | 1.00 |
| `1e-4` | 59 | 71.2 | 0.83 |
| `1e-5` | 9 | 7.1 | 1.26 |
| **`2e-6` (the gate)** | **1** | **1.42** | **0.70** |
| `1.2572e-6` (its own residual) | 1 | 0.90 | 1.12 |

The null is linear and well calibrated across three decades. Independent
confirmation by local root density: 6,095 roots with `x_+ in [136,138]`, i.e.
3,047.5 per unit `x`; gate window `5.4814e-4` wide; **expected hits 1.67,
observed 1.**

Monte Carlo over 20,000 random targets in `[110,170]` against the same real
family: **mean 1.84 hits per target, P(at least one hit) = 0.791.** A randomly
chosen number near 137 is matched to 2 ppm by this family four times in five.

**The alpha match carries no evidential weight on its own.**

## 4. Finding 2 — at the operative gate the second leg eliminates nothing

| `resid_+` gate | no `x_-` gate | with `x_-` gate | removed |
|---|---|---|---|
| `1e-2` | 8,517 | 32 | 8,485 |
| `1e-3` | 712 | 4 | 708 |
| `1e-4` | 59 | 1 | 58 |
| `1e-5` | 9 | 1 | 8 |
| **`2e-6`** | **1** | **1** | **0** |

"Dual-matcher" and "matcher" are the same predicate at the tolerance actually
used. The word *dual* does rhetorical work only.

## 5. Finding 3 — all remaining surprise belongs to a retired identification

A priori the `x_-` gate *is* selective (2.65M → 7,313; gated density near 137
is 11.5 vs 3,047.5 per unit `x`), and it supplies **100%** of the scan's
improbability. Three routes agree: gated expectation `6.4e-3`, density method
`6.30e-3`, Monte Carlo `P(>=1) = 0.00528` — i.e. **~190:1**, not the retracted
`4e5:1`.

But that 190:1 is *entirely* the `x_- <-> N_c` leg, and
`SPEC_ALGEBRAIC_SPINE.md` §11 with the v1.4 §5 taxonomy **retired that
identification** (`X_MINUS` is annotated in engine source as "artifact; NOT
N_c — retired FTD-0014"). Strip the retired leg and Finding 1 is what remains.

The standing defence — that polynomial uniqueness is independent of which
constant one identifies with `x_-` — is false *as an evidential claim*. The
fact is stated independently of the interpretation, but every bit of its
improbability comes from requiring `x_- ≈ 3`. Retire the reason to care about
3 and the number 3 is an arbitrary post-hoc filter.

## 6. Finding 4 — the preregistered test had no power

Non-`G*` gated density gives `E[non-G* dual-matchers] = 5.8e-3`, so under the
scan's own null:

```text
P(observe 0 non-G*)      = 0.9942     <- the registered Outcome A
P(Outcome C, 1-2 hits)   = 5.78e-3
P(Outcome B, >= 3 hits)  = 3.24e-8
```

Outcome A was a foregone conclusion **whether or not `G*` is special**. "Zero
non-`G*` dual-matchers across 2.65M polynomials" reads as a strong null
result; it is arithmetically indistinguishable from no test having been run.
The prereg has no power section; one would have caught this before hash-lock.

## 7. Finding 5 — tolerances were set by the observation

Both gates are two-sided and relative. `TOL_PLUS = 2.0e-6` against an observed
`1.2572e-6` (**1.59x headroom**); `TOL_MINUS = 1.0e-2` against `7.988e-3`
(**1.25x**). The source says so in as many words: thresholds chosen "so the
master quadratic counts comfortably." Asymmetry between the legs: **5,000x**.

Worse, the *extended* scan (`scripts/proofs/proof_polynomial_look_elsewhere_
extended.py`) targets `X_PLUS_TARGET = 137.0361714582` and
`X_MINUS_TARGET = 3.0239639163` — **the master quadratic's own roots**, so its
residual is identically zero and it passes by construction. That scan cannot
fail and measures nothing. The adversarial scan deserves credit for fixing the
centering; it did not fix the width.

Symmetric-gate sensitivity: at `1e-2` there are 32 dual-matchers (29 non-`G*`,
11 distinct constants); at `1e-3`, exactly 1; at `<=1e-4`, zero. **Uniqueness
exists only in a narrow band of gate choices, and the band was chosen after
the answer was known.**

## 8. Finding 6 — "rank 1 by 130x" is a within-family statement

Replicated: the ratio is **129.4x**. But rank 2 is `G_star` *itself*
(`x_+ = 137.058299`, resid `1.627e-4`) and rank 3 is `sqrt5`. The gap is
between two `G*` polynomials, i.e. a statement about coefficient choice, not
about `G*` versus other constants. `SPEC_ALGEBRAIC_SPINE.md` §11 already says
this; downstream summaries quote the bare "rank 1 by ~130x" without it.

## 9. Correction to FTD-0785's recorded lead

FTD-0785 recorded an unverified lead that the historical criterion "may have
accepted `x_-` within 1% of any small integer 1–10." **That is refuted as
stated:** both runners target `N_c = 3.0` specifically. The lead's *conclusion*
is nonetheless correct and stronger — the counterfactual shows that loosening
leg 2 to "any integer 1–10" changes the count at the operative gate from 1 to
1. The leg is not nearly free; **at the gate used it is exactly free.**

## 10. Tags and withdrawals

- `x_+ ≈ 1/alpha` at 1.26 ppm: **`[NUMERICAL FACT]`** — unchanged; it is arithmetic.
- **FTD-0319's uniqueness claim: `[MEASURED]` → `[SELECTION]`**, with a
  mandatory rider that the null expectation for leg 1 is 1.42–1.67 against 1
  observed, and that both tolerances were set by the observation.
- **"The framework's single scan-rigid identification" is WITHDRAWN.** It
  appears in at least eight canonical files (`SPEC_FTD_COMPLETE_FRAMEWORK.md`,
  `SPEC_UNIFIED_AXIOM_REGISTER.md` U-11, `unified_axiom_register.json`,
  `adoption_pricing.json`, `SPEC_ADOPTION_PRICING_RULES.md`,
  `SPEC_OPEN_MATH_FRONTIERS.md`, `SPEC_DIMENSIONAL_MAP.md`,
  `INDEX_CONSTRUCTION_SPINE.md`). **The scan-rigid count goes from exactly one
  to zero.** Those files are flagged, not yet edited.
- `[COORDINATE COINCIDENCE]` **does not apply** — that precedent marks a
  units/coordinate mismatch, a different failure mode. Borrowing it would
  mislabel this one.

**Downstream:** FTD-0387's D7 FC-W calibration prices the alpha-root as a
high-value gap-class *because* it is scan-rigid; that valuation loses its
basis. `SPEC_ALGEBRAIC_SPINE.md` §11 still asserts the polynomial "is the
unique dual-matcher" — that sentence must state the base rate instead.

## 11. Process failures worth recording

The prereg's §7 required a post-run `ANALYSIS_ADVERSARIAL_LOOK_ELSEWHERE.md` —
**it does not exist**. The campaign was never entered in
`REF_PREREGISTER_MANIFEST.md`. The results directory
`engine/results/adversarial_look_elsewhere_2026-05-21/` is **untracked in
git**, so the run of record has no commit. §2.3's F9 guard recommended
external review of the basket before hash-lock; no record exists.

## 12. The honest one-line replacement

> Among 2.65M polynomials over an 18-constant basket, exactly one matched
> `1/alpha` to 2 ppm — which is what chance predicts (`E = 1.4`–`1.7`). It also
> happened to have its small root near 3, and no other polynomial did: a joint
> event of prior probability ~1/190 whose second leg targets an identification
> the framework has retired.
