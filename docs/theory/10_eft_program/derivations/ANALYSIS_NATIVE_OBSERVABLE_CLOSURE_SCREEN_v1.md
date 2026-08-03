# FTD-0778 — Native Observable Closure Screen v1

**Status:** `[THEOREM — EXACT, SPECTRAL RIGIDITY]` +
`[ENGINE FACT — MEASURED, OBSERVABLE- AND CONFIGURATION-SCOPED NEGATIVE]` +
`[OPEN — A CLOSING NATIVE OBSERVABLE]`  
**Verdict:** `NATIVE_OBSERVABLE_CLOSURE_FAILED`  
**Preregistration:**
[`PREREG_NATIVE_OBSERVABLE_CLOSURE_SCREEN_v1.md`](../preregistrations/PREREG_NATIVE_OBSERVABLE_CLOSURE_SCREEN_v1.md),
locked before any inspection of the corpus  
**Production impact:** none; no engine execution, no artifact modified

## 1. Result in one sentence

The preregistered aggregate `q_active` is not a natural coordinate in the
locked `L=32` seed-1 profile — after the lattice saturates at `1.4%` of the
run it is a **perfectly monotone ramp** with no oscillatory content at any
amplitude — so FTD-0776's zero-crossing gate was measuring the sign of a ramp
rather than the absence of recurrence, and neither companion channel closes
either.

## 2. Exact result: where `G*` can and cannot live

Two facts, proved before the screen and independent of any engine data.

### Theorem A (spectral rigidity) `[THEOREM]`

For `H = p^2/(2 mu) + lambda |q|^n / n`, every dimensionless combination of the
action-angle data `{I, E(I), E'(I), E''(I), ...}` is a rational function of `n`
alone; no Gamma or Beta value survives.

*Proof.* Scaling forces `E = C I^k` with `k = 2n/(n+2)` exactly. The constant
`C` carries the dimensions and carries `B(1/n,1/2)`, the sole source of `G*` at
`n=4`. Any dimensionless combination is homogeneous of degree zero in `C`, so
`C` cancels, leaving polynomials in `k`:

```text
E/(Omega I)      = 1/k                 = (n+2)/(2n),
H0'' E/Omega^2   = (k-1)/k             = (n-2)/(2n),
E^2 E'''/Omega^3 = (k-1)(k-2)/k^2      = (2-n)/n^2,
```

and in general `E^(m-1) E^(m) / Omega^m = prod_{j<m} (k-j) / k^m`. QED.

Verified symbolically at orders `m = 1..6` and numerically at
`n = 2,3,4,5,6,8` across two unrelated parameter sets; values are
parameter-independent pure numbers, as required.

**Corollary A.** Any construction extracting `G*` from spectral data alone is
impossible a priori.

**Mechanism.** `[I] = M L^2 T^-1`, `[E] = M L^2 T^-2`, `[Omega] = T^-1`, subject
to `I Omega / E = 1/k`. These span only two independent dimensionful
combinations across three dimensions: **no length is constructible from a
spectrum**, while `G*` compares a period to an amplitude, which is a length.
Supplying the mass closes the gap, giving

```text
G* = (1/A) sqrt(6 pi I / (mu Omega)),
```

verified exactly to 19 significant figures at three parameter sets. `G*` is the
ratio between the length the spectrum can build once given a mass, and the
length the orbit actually has.

### Theorem B (position-space survival) `[THEOREM]`

The normalized moments `<|x|^r> = B((r+1)/n,1/2) / B(1/n,1/2)` carry Gamma
ratios except when `n | r`, where the Gamma recursion collapses them to
rationals. At `n=4`: `<x^4>=1/3`, `<x^8>=5/21`, `<x^12>=15/77` are rational,
while `<|x|>=sqrt(pi)/G*`, `<x^2>=4/G*^2`, `<x^6>=12/(5 G*^2)` carry `G*`,
matching every moment recorded in FTD-0772.

**Corollary B (dichotomy).** `G*` is invisible to symplectic invariants — the
action-angle data alone — and visible to the pair (symplectic structure +
kinetic metric). Action-angle variables are the maximally invariant description
of an integrable system and therefore discard orbit shape; `G*` is shape
information. Requiring constant mass selects the coordinate uniquely up to the
affine group `q -> alpha q + beta`, `t -> gamma t`; a nonlinear point
transformation makes the mass position-dependent and destroys homogeneity.

### Reconciliation of four prior cancellations

| result | quantity | class | `G*` present | explained by |
|---|---|---|---|---|
| FTD-0770 | `kappa H0''/Omega^2` | spectral | no | Theorem A |
| FTD-0772 | coordinate-free orbit modulus | spectral | no | Theorem A + Cor. B |
| FTD-0773 | `B_4 = 48pi/G*^4` | position-space | **yes** | Theorem B |
| — | `H0'' = dOmega/dI` generally | spectral | no | Theorem A |

FTD-0773's *retention* of `G*` is as much a confirmation as the cancellations:
its ratio is built from a coordinate-space edge coupling, and its own caveat
that the ratio "changes under nonlinear coordinate or edge-functional changes"
is precisely Corollary B.

## 3. Screen executed on the FTD-0776 corpus

Read-only over the four hash-locked 200,000-tick arms. All metrics, hypotheses,
and the `0.95` threshold were fixed in the preregistration before inspection.

| metric | result (all four arms concordant) | reading |
|---|---|---|
| P0 — `p_active` vs `dq_active/dt` | `R^2 = 0.9998`, `mu = 0.9998` | `p` **is** the conjugate momentum, unit mass |
| M1 — `qddot = F(q)` | `R^2 ~ 0.003` | no positional closure |
| M2 — `qddot = F(q, qdot)` | `R^2 ~ 0.003 .. 0.045` | no phase-plane closure |
| N1 — upper half-band power | `1.5% .. 2.0%` | **not** noise-dominated |
| N1 — lag-1 autocorr of `qddot` | `0.56 .. 0.73` | smooth dynamics |

Verdict by the locked rule: **H2**, `NATIVE_OBSERVABLE_CLOSURE_FAILED`. The N1
control excludes `NATIVE_OBSERVABLE_CLOSURE_UNINFORMATIVE_NOISE`.

### Why it fails — the diagnostic

- `active_count` runs from `3` to `32768 = 32^3`, **saturating to the entire
  lattice within `1.4%` of the run** (tick ~2800 of 200,000). The observable is
  an extensive sum over an index set that grows to include everything.
- Post-saturation, `q_active` is **perfectly monotone**: the fraction of ticks
  with `dq > 0` is exactly `1.0000` for `A=10,12` and exactly `0.0000` for
  `A=14,16`, across ~196,000 consecutive ticks.
- It is a smooth ramp to numerical precision: after a degree-9 polynomial
  detrend the residual standard deviation is `0.0000%` of the ramp range
  (e.g. `0.008` against `5.6e7`). **There is no oscillatory content anywhere in
  the record at any amplitude.**
- Neither companion channel closes: `q_all` gives `R^2 ~ 0.001..0.010`,
  `q_center` gives `R^2 ~ 0.0003`.

## 4. Consequences

1. **FTD-0776's crossing counts `1,1,0,0` were measuring the sign of a ramp,
   not the absence of recurrence.** A monotone function crosses zero at most
   once; the ones and zeros record only where each ramp began relative to the
   origin. FTD-0776's verdict stands and is not superseded, but the stronger
   and more useful statement is that **`q_active` is not a clock candidate at
   all** — it is a drift- or current-like extensive quantity — so re-running it
   at other amplitudes or lattice sizes cannot be informative.
2. **The engine does supply a constant-mass conjugate pair.** `P0` returns
   `R^2 = 0.9998` with `mu ~ 1`. The absence of a kinetic metric was never the
   obstruction; the choice of observable was.
3. **The failing channels share a structural property:** all three are *global
   extensive aggregates*. Summing over many degrees of freedom washes out
   phase, and an extensive quantity tracks population size rather than
   oscillation. The failure is a property of the observable *class*, not an
   accident of one channel.
4. **Closure screening is cheap.** The screen runs in seconds on existing
   artifacts and requires no engine execution; the FTD-0776 campaign cost four
   runs of ~26 minutes each. **A closure screen should gate any future
   recurrence campaign.**

## 5. What is and is not learned

### Established

1. Theorems A and B, exact, with the `G*` length-ratio identity.
2. All four prior `G*` cancellations share one mechanism.
3. `q_active`, `q_all`, and `q_center` fail closure in the locked profile, and
   the failure is not attributable to noise.
4. `p_active` is the conjugate momentum of `q_active` to within `0.02%`.

### Not established

1. That the substrate lacks a natural coordinate, a recurrence, or a clock.
2. Anything about untested channels, amplitudes, lattice sizes, or seeds.
3. Any quarticity, occupancy, `G*`, or minimum-`dt` claim.
4. Any supersession of FTD-0772 or FTD-0776, both binding at their scopes.

## 6. Next falsifier

The gate order implied by Theorems A and B, with the first stage not yet run on
any candidate:

- **Gate A — closure.** Exhibit a channel with `qddot = F(q)` single-valued, no
  branch/velocity/history dependence, constant effective mass. Prefer **local
  or normal-mode-projected** observables over global aggregates; body-frame
  observables from the Phase 3 body-tracking layer are the natural candidates.
- **Gate B — quarticity.** Only downstream of A: is `F(q) ~ q^3`, equivalently
  is the amplitude-normalized occupancy `rho_4` across an amplitude family?
- **Gate C — `G*` as an output.** With `mu` from the Gate A closure fit and
  `I, Omega, A` measured from the orbit, evaluate
  `G*_meas = (1/A) sqrt(6 pi I / (mu Omega))` against `2.958675...`. Every
  input independently measured; `G*` an output rather than an assumption.

Under Theorem A no amount of spectral precision can produce `G*`; under
Theorem B the position-space route requires the kinetic metric, which requires
closure. **Gate C is meaningful only downstream of Gate A.** FTD-0772's own
next-falsifier list already requested a natural-coordinate closure test
(its item 4); Theorem A supplies the reason it is not optional.

A closure screen requires per-channel telemetry that the current dump does not
provide for local or mode-projected observables; specifying that dump is the
immediate next work item.

## 7. Artifacts

| tracked artifact | SHA256 | result |
|---|---|---|
| `PREREG_NATIVE_OBSERVABLE_CLOSURE_SCREEN_v1.md` | `731F31DD...3268695A` | locked before inspection |
| `scripts/experiments/screen_native_observable_closure.py` | `5E854EC7...F898FD54` | P0/M1/M2/M3 executed, four arms |
| `scripts/experiments/diag_native_observable_closure.py` | `67FD495D...0DA9014B` | N1 noise control + saturation diagnostic |

Per-arm results, `M1` / `M2` taken as the max over the two independent
acceleration estimators:

| arm | M1 | M2 | verdict |
|---|---|---|---|
| `A=10` | `0.0037` | `0.0452` | `NATIVE_OBSERVABLE_CLOSURE_FAILED` |
| `A=12` | `0.0007` | `0.0225` | `NATIVE_OBSERVABLE_CLOSURE_FAILED` |
| `A=14` | `0.0005` | `0.0145` | `NATIVE_OBSERVABLE_CLOSURE_FAILED` |
| `A=16` | `0.0007` | `0.0170` | `NATIVE_OBSERVABLE_CLOSURE_FAILED` |

`M3` returns `n/a (insufficient overlapping bins)` on every arm: the first- and
last-third `q` ranges barely overlap, which is itself a monotone-drift
signature rather than a defect of the metric.
