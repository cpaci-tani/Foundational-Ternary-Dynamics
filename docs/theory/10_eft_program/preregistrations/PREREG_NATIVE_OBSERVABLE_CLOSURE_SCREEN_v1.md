# PRE-REGISTRATION — Native Observable Closure Screen v1

**Date locked:** 2026-08-03  
**Identifier:** `FTD-0778`  
**Status:** `[PRE-REGISTRATION — LOCKED/NOT YET RUN]`  
**Parents:** `FTD-0659`, `FTD-0770`, `FTD-0772`, `FTD-0776`  
**Campaign type:** `[LOCKED RETROSPECTIVE SCREEN]` of the immutable FTD-0776
corpus; a diagnostic of the *observable*, not of the substrate.  
**Scope:** observer-only analysis of existing artifacts. No engine execution.
No engine state, update rule, Hamiltonian, clock variable, phase variable,
coupling, calibration, tolerance, toggle, scenario, production path, or golden
state may change.

## 0. Question and epistemic firewall

The registered question is:

> Does the preregistered aggregate `q_active`, or either recorded companion
> channel, behave as a **natural coordinate** — acceleration a single-valued
> function of position, with constant effective mass and no dependence on
> unobserved state — over the locked `L=32`, seed-1 FTD-0776 corpus?

Motivation. FTD-0772 and FTD-0776 both tested *occupancy* and *recurrence*
properties. Both presuppose that the observable is a closed one-dimensional
natural system; neither established that premise separately. An exact result
on power-law oscillators (§1) shows that this premise is not decorative: the
occupancy law and every `G*`-bearing moment are defined only in a fixed
natural coordinate, and are destroyed by nonlinear relabeling. A closure
screen is therefore logically prior to any occupancy or recurrence verdict.

The campaign may compute derivatives of recorded channels, fit binned
conditional means, and compare variance-explained fractions. It must not fit a
potential and call it derived, select a channel after inspecting closure,
re-run the engine, adjust the locked FTD-0776 corpus, tune the bin counts or
thresholds after seeing results, or interpret a closure failure as evidence
about the substrate rather than about the observable.

The following remain outside scope regardless of outcome:

- any claim that the substrate does or does not possess a natural coordinate;
- any claim about quarticity, occupancy, `G*`, or minimum `dt`;
- any promotion or retraction of FTD-0772 or FTD-0776, both of which remain
  binding at their registered scopes;
- any statement about untested channels, amplitudes, lattice sizes, or
  configurations.

## 1. Locked exact target

For a one-dimensional natural system `H = p^2/(2 mu) + V(q)` with constant
`mu`, the acceleration is a single-valued function of position alone:

```text
qddot = -V'(q)/mu.
```

Two exact facts make this the prior gate.

**(a) Spectral rigidity.** For homogeneous `V ~ |q|^n / n`, scaling forces
`E = C I^k` with `k = 2n/(n+2)` exactly. The Beta function `B(1/n,1/2)` — the
sole source of `G*` at `n=4` — resides entirely in the dimensionful constant
`C`, which cancels from every dimensionless combination of `{I, E, E', E'', ...}`.
Hence

```text
E/(Omega I)      = (n+2)/(2n),
H0'' E/Omega^2   = (n-2)/(2n),
E^2 E'''/Omega^3 = (2-n)/n^2,
```

all rational in `n`, containing no Gamma value. This reproduces FTD-0770's
`kappa H0''/Omega^2 = (kappa/E0)(m-2)/(2m)` and explains its observation that
"the exponent survives but the quartic `G*` period normalization cancels."

**(b) Position-space survival.** `G*` enters only through coordinate-space
quantities. The normalized moments

```text
<|x|^r> = B((r+1)/n, 1/2) / B(1/n, 1/2),   x = q/A,
```

carry Gamma ratios except when `n | r`, where the recursion collapses them to
rationals — matching the recorded `<x^4>=1/3`, `<x^8>=5/21` against
`<x^2>=4/G*^2`, `<x^6>=12/(5 G*^2)`. Equivalently

```text
G* = (1/A) sqrt(6 pi I / (mu Omega)),
```

so `G*` is the ratio of the length the spectrum can build *given a mass* to the
length the orbit actually has. A mass — the kinetic metric — is therefore
required, and it is exactly what closure supplies. Closure is prior.

## 2. Corpus

`engine/results/gstar_qactive_pilot_20260802/raw/native_L32_A{10,12,14,16}_seed1.csv`,
200,000 ticks each, accepted only under the existing FTD-0776 artifact
manifest. Primary channel `q_active`; companion channel `p_active`. Channels
`q_all` and `q_center` are exploratory and carry no decision weight.
`active_count` is recorded as a diagnostic covariate.

## 3. Locked hypotheses

- **H0 — natural.** `qddot = F(q)`, single-valued.
- **H1 — locally dissipative.** `qddot = F(q, qdot)`, single-valued in the
  phase plane; memory-free but not conservative.
- **H2 — non-autonomous.** Acceleration depends on state outside `(q, qdot)`.

## 4. Locked metrics

Acceleration is computed two independent ways — second difference of
`q_active`, and first difference of `p_active` — and every verdict must hold
under both. A Savitzky–Golay variant (window 21, order 3) is run as a noise
control; conclusions must be stable across raw and smoothed.

- **P0** — regress `p_active` on `dq_active/dt`; report `R^2` and fitted `mu`.
  Prerequisite for treating `(q,p)` as a natural pair.
- **M1** — `R^2` of `qddot = F(q)`, `F` by binned means, 200 bins across the
  0.1–99.9 percentile range, bins with fewer than 20 samples dropped.
- **M2** — `R^2` of `qddot = F(q, qdot)`, 60×60 binned means, same drop rule.
- **M3** — stationarity of `F`: estimate on the first and last thirds
  separately; report the maximum relative discrepancy over overlapping bins.
- **N1** — noise control: fraction of `qddot` power in the upper half-band, and
  lag-1 autocorrelation of `qddot`. A closure failure accompanied by
  upper-half-band fraction near 1 is uninformative and must be reported as
  such rather than as H2.

## 5. Locked decision rule

| condition | reading |
|---|---|
| `M1 > 0.95` | H0 — closure passes; the observable is a natural coordinate |
| `M1 <= 0.95` and `M2 > 0.95` | H1 — memory-free dissipation; `F(q)` extractable |
| `M2 <= 0.95` | H2 — hidden-state dependence; observable not closed |

Threshold `0.95` is fixed here, before inspection. All four arms are reported
separately; the verdict is the majority across arms, and any disagreement is
reported rather than averaged.

## 6. Permitted verdict tokens

- `NATIVE_OBSERVABLE_CLOSURE_PASS`
- `NATIVE_OBSERVABLE_CLOSURE_DISSIPATIVE`
- `NATIVE_OBSERVABLE_CLOSURE_FAILED`
- `NATIVE_OBSERVABLE_CLOSURE_UNINFORMATIVE_NOISE`

## 7. What a negative licenses

Only this: that the tested channels, in this locked profile, are not natural
coordinates, and that occupancy or recurrence statistics computed on them are
statistics of a non-closed observable. It licenses no claim about the
substrate, and it does not supersede FTD-0772 or FTD-0776.

If the screen returns a failure, the constructive consequence is a selection
criterion for future campaigns, not a physics conclusion: **closure should be
screened before a recurrence campaign is scheduled**, since the screen costs
seconds on existing artifacts while a campaign costs hours of wall time.
