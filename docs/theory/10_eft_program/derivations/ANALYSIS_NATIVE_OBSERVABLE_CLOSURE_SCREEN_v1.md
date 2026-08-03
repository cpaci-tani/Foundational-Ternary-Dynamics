# FTD-0778 — Native Observable Closure Screen v1

> ## ⚠ AMENDED 2026-08-03 AFTER ADVERSARIAL AUDIT — VERDICT CHANGED
>
> The original verdict `NATIVE_OBSERVABLE_CLOSURE_FAILED` is **RETRACTED** and replaced
> by `NATIVE_OBSERVABLE_CLOSURE_UNINFORMATIVE`. The screening metric was found to be
> **structurally incapable of returning a pass on a drifting record**, so the original
> reading measured the tool rather than the data. Six further claims are retracted in
> §4. The exclusion of `q_active` as a clock candidate **stands**, on different and
> simpler grounds (§5).

**Status:** `[THEOREM — EXACT, SPECTRAL RIGIDITY]` +
`[ENGINE FACT — MEASURED, CORPUS-SCOPED]` +
`[METHOD DEFECT — SCREENING METRIC RETIRED]` +
`[OPEN — A CLOSING NATIVE OBSERVABLE]`
**Verdict:** `NATIVE_OBSERVABLE_CLOSURE_UNINFORMATIVE`
(supersedes `NATIVE_OBSERVABLE_CLOSURE_FAILED`)
**Preregistration:**
[`PREREG_NATIVE_OBSERVABLE_CLOSURE_SCREEN_v1.md`](../preregistrations/PREREG_NATIVE_OBSERVABLE_CLOSURE_SCREEN_v1.md)
**Production impact:** none; read-only, no engine execution, no artifact modified

## 1. Result in one sentence

`q_active` is excluded as a clock candidate because **after the lattice saturates it is
identically the whole-lattice sum, whose motion is free centre-of-mass drift at the
conserved total momentum** — a straight line carrying no dynamics — and *not* because it
failed a closure test, which it never validly received.

## 2. Exact result: where `G*` can and cannot live

Theorems A and B are unaffected by the method defect and are carried in corrected form.

**Theorem A (spectral rigidity).** Scaling forces `E = C I^k` with `k = 2n/(n+2)`. The
Beta factor — the sole Gamma content of `E(I)` — sits in the dimensionful constant `C`.
For any **monomial** in the single-orbit spectral data, the dimensional constraints
force `sum_m a_m = 0`, so `C` cancels identically:

```text
E/(Omega I)      = 1/k              = (n+2)/(2n)
H0'' E/Omega^2   = (k-1)/k          = (n-2)/(2n)
E^2 E'''/Omega^3 = (k-1)(k-2)/k^2   = (2-n)/n^2
```

Scope correction: the theorem covers the **monomial group of a single orbit**, not
*every* dimensionless combination — `E(2I)/E(I) = 2^k` is dimensionless, single-orbit
and not rational in `n`. No such combination introduces a Gamma value, which is the
theorem's real content. Also `C` carries `B(1/n, 3/2)`, not `B(1/n, 1/2)`.

**Theorem B (position-space survival) — dichotomy corrected.** The moments
`<|x|^r> = B((r+1)/n,1/2)/B(1/n,1/2)` are **not rational in `n`** generically, but
collapse in at least two infinite families:

- `r = 0 (mod n)` — rational (virial identities);
- `n` even and `r = (n-4)/2 (mod n)` — equal to `(rational) * tan(pi/n)`, an algebraic
  irrational: `n=6, r=1 -> sqrt(3)/3`; `n=8, r=2 -> sqrt(2)-1`; `n=12, r=4 -> 2-sqrt(3)`

plus sporadic collapses at `(12,2)`, `(20,2)`, `(20,6)`, `(24,4)`, `(24,6)`. **At `n=4`
the second family degenerates into the first** (`r_0 = 0`, `tan(pi/4) = 1`), which is
why the original two-way dichotomy looked exact — an artifact of testing only the
quartic. The correct invariant claim is "not a rational function of `n`", not "carries a
Gamma ratio".

**The length identity** `G* = (1/A) sqrt(6 pi I/(mu Omega))` is exact, proved
symbolically and verified to 19+ digits across 8 orders of magnitude in `lambda`. It is
`n=4`-specific. Correction: the mass **alone** does not close the dimensional gap —
adding `mu` to `{I,E,Omega}` raises the rank but yields no new dimensionless group. What
closes it is `mu` **and** `lambda` jointly (equivalently `mu` and `A`).

## 3. Screen executed on the FTD-0776 corpus — and why it was uninformative

All metrics were computed as preregistered and the numbers reproduce exactly
(`M1_fromq` A10 = `0.00366`). **The metrics themselves are invalid on this data.**

### 3.1 The screening metric is degenerate on a drifting record [METHOD DEFECT]

Ground truth, `a = -omega^2 q` **exactly** (perfect closure), with drift added:

| drift slope | M1 |
|---|---|
| `0` | `1.0000` |
| `0.01` | `0.0000` |
| `1000` | `0.0000` |

Any drift collapses it. With 200 bins across the full `q` range each bin spans ~1000
ticks ~ 20 oscillation periods, so every bin mean averages to ~0. **The metric measures
oscillation-periods-per-bin, not closure**, and the decision rule could never return
`PASS` on a drifting record. The original H2 reading was predetermined by the tool.

Two further defects in the same estimator:

- **Positive-bias floor.** `np.gradient(np.gradient(q))` expands to
  `(q[k+2] - 2q[k] + q[k-2])/4`, carrying an explicit `-q[k]/2` self-term. On **pure
  white noise** it returns `R^2 = 0.6617` (analytic `0.25/0.375 = 0.6667`). The metric
  fails in both directions.
- **Broken noise guard.** `np.gradient^2` has transfer function `|H|^2 = sin^4(2 pi f)`,
  which **vanishes at Nyquist**. The preregistered N1 null of "band fraction near `1.0`"
  is unreachable — white noise gives `0.5`, the pure-roundoff tail gives `0.426`. The
  one control designed to catch "this failure is just noise" could not fire, and that is
  exactly the failure that occurred.

The estimator is correct on drift-free data: quartic `M1 = 1.0000`, damped `0.9955`,
incommensurate two-mode `0.9427`.

### 3.2 What the corpus actually contains

- `active_count` reaches `32768 = 32^3` — the **entire lattice** — at ticks
  `2857/3342/2564/2797`, then never changes. After that tick **`q_active` and
  `p_active` are exactly identical to `q_all` and `p_all`** (max absolute difference
  `0.0`).
- The tail is thus the whole-lattice sum, whose motion is **free centre-of-mass drift at
  the conserved total momentum**: slope `286.6460096` against `p_all = 286.6459969`
  (A=10). The straight line is expected physics, not pathology.
- Tail second-difference autocorrelation is lag-1 `-0.6667`, lag-2 `+0.1662`, lag-3
  `+0.001` — the exact analytic values `(-2/3, +1/6, 0)` for the second difference of
  **white noise**, at ~100 ULPs of `|q|`. `R^2 ~ 0.003` is the double-precision noise
  floor, reproduced from synthetic `1e7 + 282t` plus `1e-14` relative noise.
- Aggressive detrending (moving-average high-pass `w = 3..20001`, polynomials to degree
  15) leaves residual `~5e-10` of `|q|` with no spectral line above `P/median ~ 20`.
  **There is no buried oscillation in the saturated tail.**

## 4. Retractions

1. ~~`NATIVE_OBSERVABLE_CLOSURE_FAILED` / H2 hidden-state.~~ **RETRACTED** (§3.1);
   verdict is `UNINFORMATIVE`.
2. ~~"No oscillatory content anywhere in the record at any amplitude."~~ **RETRACTED** —
   true of the saturated tail only. The pre-saturation transient (1.4% of ticks) carries
   **100.0000%** of the acceleration power.
3. ~~"`p_active` **is** the conjugate momentum, `R^2 = 0.9998`."~~ **RETRACTED as
   evidence — the test is circular.** `p_active[k] = q_active[k] - q_active[k-1]` to a
   median relative difference of `3.5e-10`; it is the integrator's own backward
   difference. Regressing `p` on `np.gradient(q) = (p[k]+p[k+1])/2` regresses `p`
   against a linear function of itself. A pure-identity null reproduces the committed
   values to 5–6 digits (`0.999797` vs `0.999800` at A=10). It restates the update rule
   and says nothing about a kinetic metric.
4. ~~"Post-saturation `q_active` is perfectly monotone."~~ **AMENDED** — `0.74%` of steps
   go backwards, all inside the transient; the saturated tail is monotone.
5. ~~"Neither companion channel closes; three channels fail."~~ **RETRACTED** — `q_all`
   is **bit-identical** to `q_active` post-saturation, not an independent test.
   `q_center` was never validly screened (§5).
6. ~~"The failure is a property of the observable *class*."~~ **RETRACTED** — rested on
   three independent channels; there are at most two, one untested. No class claim is
   supported.
7. ~~N1 "excludes `UNINFORMATIVE_NOISE`."~~ **RETRACTED** (§3.1); the preregistered null
   of `1.0` should be `0.5`.

## 5. What survives, and one live lead

**Survives.** `q_active` is excluded as a clock candidate: post-saturation it is the
whole-lattice sum in free drift, carrying no dynamics. FTD-0776's crossing counts
`1,1,0,0` measured the sign of that drift. Re-running it at other amplitudes or lattice
sizes cannot inform. The arms are additionally **not a clean amplitude scan** — ramp
slopes `+286.6, +503.7, -1113.9, -568.0`, sign-flipping, reflecting random initial total
lattice momentum.

**Live lead — `q_center`, never screened as a primary channel.** After degree-5
detrending it has residual std `0.36–0.47` and sharp lines. The dominant line is at
`f = 0.0180225` (period `55.486` ticks), **identical to 7 digits across all four arms** —
amplitude-independent, hence a harmonic lattice normal mode, not a quartic clock. It does
**not** close as a one-dimensional system: `640–660` spectral lines carry 80% of the
residual variance. It is a superposition of hundreds of normal modes.

## 6. Method requirements for any successor screen

1. **Detrend before binning**, or use an estimator drift does not destroy.
2. **Use the true second difference** `q[k+1] - 2q[k] + q[k-1]`, not `np.gradient^2`
   (self-term, spacing-2 stencil blind to tick-scale content, annihilates Nyquist).
3. **Calibrate every metric against a white-noise null before preregistering it.** The
   correct band-fraction null is `0.5`.
4. **Screen a channel whose support does not saturate to the whole lattice.**
5. Control for set-membership discontinuities: the only ticks where the `p`/`dq`
   identity breaks beyond `1e-9` are exactly the `557/720/635/652` `active_count` change
   ticks, with jumps to 90 units.

## 7. Preregistration integrity [DISCLOSED]

`P0`, `M1`, `M2`, `M3` and the `0.95` threshold were locked before inspection. **`N1`
was added in a second protocol revision** and is not covered by the original lock. The
saturation, monotonicity and detrend diagnostics that produced the surviving §5 result
were preregistered in **neither** protocol and computed by **neither** committed script;
they are post-hoc and are reported as such.

## 8. Scope

An `[ENGINE FACT]` about one observable in one locked profile, plus a `[METHOD DEFECT]`
about the screening tool. It licenses **no** claim that the substrate lacks a natural
coordinate, a recurrence, or a clock. FTD-0772 and FTD-0776 remain binding and
unsuperseded. The corpus was not re-run and no production state was touched.
