# FTD-0803 — Why the G\* window is crowded: an elliptic fixed point forces a van Hove singularity

**Status:** `[MEASURED — VERIFIED OVER 3 DECADES]` +
`[EXPLANATION — SURVIVED ONE ADVERSARIAL PASS, NOT TWO]` +
`[SCOPE CORRECTION — APPLIES TO THE CM SCAN ONLY, NOT TO FTD-0319 / OT-3.3]`
**Verdict:** `GSTAR_SITS_AT_A_FORCED_CRITICAL_VALUE_SO_ITS_WINDOW_IS_STRUCTURALLY_CROWDED`
**Parent:** FTD-0321 (which measured the crowding; this explains it)
**Production impact:** none.

---

## 1. The question

FTD-0321 measured that the dual-match criterion has no discriminating power at
scale: over `|d| ≤ 200,000` a random target near `G*` is matched by ~204 ideal
classes with `P(≥1) = 1.0000`. That is an observation. **Why** is that window
crowded?

## 2. The claim

The per-class invariant used by FTD-0321,

```
G*(tau) := sqrt( 8 pi * |eta(tau)|^4 * Im(tau) )
```

is `SL_2(Z)`-invariant. `tau = i` is the order-2 elliptic fixed point of
`SL_2(Z)` (fixed by `S: tau -> -1/tau`), and an invariant function must have
vanishing gradient at an elliptic fixed point: the differential of `S` acts as
`-1` on the tangent space, so `grad G = -grad G`, hence `0`. Canonical
`G* = G*(i)` is therefore, **by construction, a critical value**.

Near a critical point the map `tau -> G*` is degenerate: a two-dimensional
neighbourhood is compressed into a vanishing range of values. CM points
equidistribute in the fundamental domain, so the density of `G*` values is the
pushforward of a smooth measure through that degenerate map, and it diverges at
the critical value. The Hessian at `tau = i` has signature `(+, -)`, i.e. a
**saddle**, and in two dimensions a saddle gives a **logarithmic** divergence —
the van Hove singularity familiar from 2D densities of states.

**Consequence:** the FTD-0321 scan was structurally guaranteed to be
uninformative at scale, independent of anything about `alpha`.

## 3. Evidence

| Test | Result |
|---|---|
| `SL_2(Z)`-invariance (6 matrices × 3 points) | worst deviation `8.7e-30` |
| `dG/dy` at `x=0`, `y = 0.90 / 0.95 / **1.00** / 1.05 / 1.10` | `0.225 / 0.104 / **3.7e-40** / -0.089 / -0.167` |
| `dG/dy` at `tau = rho` (order-3 point) | `1.8e-32` |
| Hessian at `tau = i` | `d2G/dx2 = +0.441`, `d2G/dy2 = -1.920` → saddle |
| Global max of the distribution | `2.990158 = G*(rho)`; `G*(i) = 2.958675` is the saddle below it |

Density of `G*` values (exact `tau = i` forms **excluded** throughout — see §4):

| half-width | \|d\| ≤ 25,000 | \|d\| ≤ 100,000 | \|d\| ≤ 400,000 |
|---:|---:|---:|---:|
| 1e-2 | 3,079,750 | 24,651,800 | 197,230,450 |
| 1e-3 | 4,617,000 | 37,749,500 | 302,102,500 |
| 1e-4 | 5,545,000 | 50,035,000 | 406,035,000 |
| 1e-5 | 5,300,000 | 52,900,000 | 507,700,000 |
| 1e-6 | 2,000,000 | 49,000,000 | 452,000,000 |

Log-slope over the clean decades `1e-2 .. 1e-4`, at `G*(i)` versus at a generic
value `2.90`:

| range | at `G*(i)` | at 2.90 | ratio |
|---|---:|---:|---:|
| \|d\| ≤ 25,000 | 535,322 / e-fold | 7,872 | **68×** |
| \|d\| ≤ 100,000 | 5,511,892 / e-fold | 15,743 | **350×** |
| \|d\| ≤ 400,000 | 45,341,332 / e-fold | ≈ 0 (flat) | unbounded |

**The saturation is a sampling cutoff, not a ceiling:** the plateau moves from
half-width `1e-4` to `1e-5` as the range grows, exactly as a finite discriminant
bound predicts. The divergence is genuine.

## 4. Adversarial pass — three of the author's own statements were wrong

This section is the audit, recorded because the corrections are the load-bearing
part.

**(a) A cited derivative was vacuous.** The first write-up cited `dG/dx = 0` at
`tau = i` as evidence of the elliptic fixed point. It is not: `x = 0` is a
reflection axis (`tau -> -conj(tau)`), so `dG/dx` vanishes there for **every**
`y` — verified at `y = 1.1, 1.6, 2.4`. Only `dG/dy` isolates `tau = i`. The
claim survives on the corrected evidence; the original presentation padded it.

**(b) The sharpest density figures were self-contamination.** 223 forms
`(a,0,a)` have `tau = i` *exactly* (`d = -4f^2`, the conductor family) and pile a
**delta spike** onto `G*(i)`. That is the same point counted repeatedly, not a
van Hove singularity. Removing them drops the reported density at half-width
`1e-6` from `276,500,000` to `165,000,000`, and at `1e-7` from `1,185,000,000`
to noise. The divergence is real over `1e-2 .. 1e-5`; the author's most dramatic
numbers were reading his own artifact. **All figures in §3 exclude these forms.**

**(c) The punchline overreached.** The first statement was that *any*
look-elsewhere test targeting `G*` is asking at the most crowded value. That
holds for scans over **CM points / modular values** (FTD-0321). It does **not**
transfer to FTD-0319 (2.65M polynomials over an 18-constant basket) or OT-3.3
(2.87M `n·G*^p`-coefficient polynomials): those scan a different space with a
different measure. Their high base rates are real and independently measured
(FTD-0791, FTD-0802) but have their **own, separate causes**. Three scans
failing for related-looking reasons is not one mechanism explaining all three.

## 5. Honest limitations

- The density argument is **numerical, not proved**. Steps 2's symmetry claims
  are checks of standard theorems; the pushforward divergence is measured.
- Equidistribution of CM points (Duke) is **cited, not tested here**.
- **One adversarial pass, not two.** The pass above was run by the same author
  who wrote the claim. Per `feedback-ftd-audit-before-constructing`, that is
  weaker than an independent refute-by-default review, and this row is tagged
  accordingly. A second, independent pass is the natural next step before any
  promotion.
- Nothing here promotes a tag. `x_+ = 1/alpha` remains `[SMC]`; OT-1.9's
  Tier-1 arithmetic is untouched; FTD-0321's registered `UNIQUE-CONFIRMED`
  stands at `[NUMERICAL FACT]`.

## 6. Cross-references

FTD-0321 (the scan this explains), FTD-0791 / FTD-0802 (the base-rate method,
whose *results* this does **not** explain — see §4c),
`ANALYSIS_DAMERELL_IDEAL_CLASS_SCAN_v1.md`, `TRACKER_ONTIC_TRUTH.md` OT-1.9.
