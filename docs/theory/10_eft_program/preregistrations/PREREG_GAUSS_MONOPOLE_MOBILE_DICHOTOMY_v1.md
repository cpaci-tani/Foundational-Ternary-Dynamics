# Pre-registration — Gauss Monopole / Mobile-Dressing Dichotomy v1

**Record:** FTD-0563
**Status:** [PRE-REGISTRATION — LOCKED/RUN; POSITIVE DICHOTOMY]
**Date locked:** 2026-07-26
**Production changes:** forbidden

## 1. Question

FTD-0560 through FTD-0562 close fixed finite rigid linear co-moving
dressings, while showing that microscopic neutrality raises the order of the
remaining hop source.  Can a microscopically neutral finite carrier nonetheless
possess a true long-range Gauss monopole, or does the framework face a strict
linear dichotomy between charge and radiationless rigid motion?

## 2. Frozen statements

### 2.1 Periodic zero mode

For the selected oriented-face divergence

\[
 D E(x)=\sum_i[E_i(x)-E_i(x-\hat i)],
\]

periodicity gives `sum_x D E(x)=0` by telescoping.  Therefore `D E=z rho` is
solvable on a torus only when `Q=sum rho=0`.  The production projector enforces
this compatibility by using `s-Q/N`, and the matched minimum-energy solver
rejects non-neutral integer source data.

### 2.2 Infinite/open monopole coefficient

For a finite source with transform

\[
 S(\mathbf k)=\sum_x\rho_xe^{i\mathbf k\cdot x},
 \qquad Q=S(0),
\]

the minimum-energy face field is

\[
 E_i(\mathbf k)=z\frac{d_i^*(\mathbf k)}{\lambda(\mathbf k)}S(\mathbf k),
\quad d_i=1-e^{-ik_i},\quad
 \lambda=\sum_i|d_i|^2.
\]

Hence

\[
 \sqrt\lambda\,|E|=z|S|,
\qquad
 \lim_{\kappa\to0}\kappa|E(\kappa\mathbf n)|=z|Q|.
\]

A finite neutral profile has `Q=0` and therefore no monopole coefficient.  If
its first nonzero total moment has order `m>=1`, then `S(k)=P_m(k)+...` and
the infrared estimator falls as `kappa^m`.  Adding a localized divergence-free
field cannot change closed-surface flux or this longitudinal coefficient.

The same conclusion applies to the restricted native FTD-0429 response because
`(div J)_k/S(k) -> 3G_C` is finite and nonzero.

### 2.3 Combined mobile-carrier dichotomy

- `Q=0`: no static Gauss monopole in the linear finite-source sector;
- `Q!=0`: a monopole exists in the open/infinite static realization, but
  FTD-0561 gives universal `T^-2` forcing and FTD-0562 forbids a
  square-summable exactly co-moving fixed finite rigid linear dressing.

The candidate theorem therefore closes a fixed finite rigid *linear* carrier
that is both genuinely monopole-charged and exactly radiationless.  It does not
close nonlinear field self-source, defect charge, boundary charge, or a
self-consistent deforming carrier.

## 3. Frozen profiles and Fourier arms

Primitive site coefficients remain in `{+1,-1}`.

| profile | polynomial | `Q` | first order |
|---|---|---:|---:|
| point | `1` | 1 | 0 |
| dipole | `1-exp(i k_x)` | 0 | 1 |
| quadrupole | `(1-exp(i k_x))(1-exp(i k_y))` | 0 | 2 |
| octupole | `(1-exp(i k_x))(1-exp(i k_y))(1-exp(i k_z))` | 0 | 3 |

Use `L in {32,64,128,256}` and integer momentum directions

```text
(1,0,0), (1,1,0), (1,1,1), (-1,2,3).
```

Run three cyclic profile/direction rotations and both global polarity mirrors.
The locked Fourier arm count is

\[
4\text{ profiles}\times4L\times4\text{ directions}
\times3\text{ axes}\times2\text{ mirrors}=384.
\]

Directions on which the leading polynomial vanishes are retained as exact
subdirection cancellations and excluded only from division by zero.

## 4. Frozen implementation controls

1. Construct a deterministic periodic matched-face field at `L=8` and verify
   its total divergence telescopes to zero.
2. Verify exact integer zero-mode cancellation for the production source
   numerator `N*s-Q` on `L in {8,16}` for every registered profile.
3. Call the existing matched minimum-energy initializer at `L=8`; it must
   reject a single point source as non-neutral and accept a dipole.
4. Construct a nonzero matched curl and verify its divergence and its change
   to every closed-surface charge are zero below tolerance.

## 5. Frozen gates

- 384 Fourier arms and 96 profile/volume/axis/mirror witness groups;
- maximum face-Gauss identity residual
  `|sqrt(lambda)|E|-|S|| <= 1e-12`;
- maximum periodic telescope residual `<=1e-12`;
- maximum scaled production zero-mode numerator exactly `0`;
- non-neutral matched initialization rejected and neutral initialization
  accepted with Gauss residual `<=1e-9`;
- maximum divergence of the registered matched curl `<=1e-12`;
- maximum closed-surface flux change under that curl `<=1e-12`;
- point monopole-estimator error `<=1e-12` in every arm;
- every neutral witness estimator decreases monotonically as `L` doubles;
- at `L=256`, maximum relative error against the first-moment asymptotic
  `<0.02` over nonzero-leading arms;
- at `L=256`, every neutral monopole estimator is `<0.1`;
- polarity-mirror and cyclic-covariance residuals `<=1e-12`;
- the independent Python proof reproduces the Fourier arm count, multipole
  scaling, periodic zero-mode theorem, and verdict without reading C++ JSON.

## 6. Outcome map

- **Positive dichotomy:** all exact derivations and registered gates pass.
  Close fixed finite rigid linear monopole charge plus radiationless co-motion.
- **Counterexample:** a finite neutral profile retains nonzero monopole
  coefficient, a localized solenoidal addition changes closed flux, or a fixed
  finite charged profile evades the cited FTD-0562 hypothesis.  Keep the branch
  open and record the counterexample.
- **Invalid:** an implementation, covariance, solver, or independent
  reproduction gate fails.  No ontological consequence.

No failed gate licenses changing profiles, directions, volumes, tolerances,
Gauss operators, source normalization, or production dynamics.

## 7. Locked paths

- `engine/include/ftd/eft/native_gauss_monopole_dichotomy.h`
- `engine/src/eft/native_gauss_monopole_dichotomy.cpp`
- `engine/tests/test_native_gauss_monopole_dichotomy.cpp`
- `scripts/proofs/proof_native_gauss_monopole_dichotomy.py`
- `engine/results/ftd_0563/windows_msvc_cpu.json`

## 8. Execution record

The pre-execution SHA-256 was
`7629151DEA58E98F44A7FF37271BA591054D67C158C6600C2581523F4E6CFC6C`.
All 384 Fourier arms, 96 witness groups, 54 neutral monotonicity witnesses,
periodic zero-mode controls, matched-solver controls, matched-curl controls,
mirror/covariance gates, and the independent Python proof passed. Verdict:
`GAUSS_MONOPOLE_MOBILE_DRESSING_DICHOTOMY_PROVED`. Production was unchanged.
