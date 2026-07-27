# Pre-registration — Full-Surface Finite-Source Obstruction v1

**Record:** FTD-0562
**Status:** [PRE-REGISTRATION — LOCKED/RUN; POSITIVE FULL-SURFACE OBSTRUCTION]
**Date locked:** 2026-07-26
**Production changes:** forbidden

## 1. Question

FTD-0560 rules out an exactly co-moving square-summable linear dressing for a
periodically hopping point polarity.  FTD-0561 proves that finite rigid width
does not cure the obstruction when the source has nonzero net polarity, while
successive neutral axial multipoles suppress its axial witness.  The remaining
linear escape is a finite neutral form factor that cancels the complete
three-dimensional resonant surface.

This campaign tests the theorem-level claim that no fixed, nonzero,
finite-support rigid source can cancel every slow-hop `l=1` resonance of the
production `FULL` field operator.  It does not test nonlinear, deforming,
period-dependent, or self-consistent matter-field carriers.

## 2. Frozen analytic target

For a hop along coordinate axis `a`, period `T`, unit direction `n`, and radial
variable `r`, define

\[
 \mathbf k=\frac{r\mathbf n}{T},\qquad
 \Omega=\frac{2\pi+k_a}{T},
\]

\[
 D_T(r,\mathbf n)=C_{\rm WAVE}^2M(\mathbf k)
 -4\sin^2(\Omega/2).
\]

The production symbol obeys

\[
 M(\mathbf k)=|\mathbf k|^2-|\mathbf k|^4/12+O(|\mathbf k|^6).
\]

With `C_WAVE^2=1/3`, the registered slow branch is

\[
 r_T=r_0+\frac{6\pi n_a}{T}+O(T^{-2}),
 \qquad r_0=2\pi\sqrt3.
\]

For a finite profile `rho_x`, let

\[
 S(\mathbf k)=\sum_{\mathbf x}\rho_{\mathbf x}e^{i\mathbf k\cdot\mathbf x}.
\]

If `m` is the lowest total degree with nonzero homogeneous Taylor polynomial

\[
 P_m(\mathbf n)=\frac{i^m}{m!}
 \sum_{\mathbf x}\rho_{\mathbf x}(\mathbf n\cdot\mathbf x)^m,
\]

then on any registered direction with `n_a P_m(n) != 0`, the frozen asymptotic
source magnitude is

\[
 |f_T|=
 G_C\sqrt3\,r_0^{m+1}|n_aP_m(\mathbf n)|T^{-(m+2)}
 +O(T^{-(m+3)}).
\]

The theorem proof is analytic: if a nonzero finite source cancelled every
slow-branch direction for all sufficiently large `T`, multiplying its form
factor by `T^m` and taking `T -> infinity` would force `P_m` to vanish on an
open subset of the unit sphere.  Homogeneity then forces `P_m` to be the zero
polynomial, contradicting the definition of `m`.

## 3. Frozen profiles

All site coefficients are primitive ternary polarities `+1` or `-1`.

| profile | support polynomial | first order |
|---|---|---:|
| point | `1` | 0 |
| axial dipole | `1-exp(i k_x)` | 1 |
| planar quadrupole | `(1-exp(i k_x))(1-exp(i k_y))` | 2 |
| cubic octupole | `(1-exp(i k_x))(1-exp(i k_y))(1-exp(i k_z))` | 3 |

For hop axes `y` and `z`, profiles and directions are carried by the same
cyclic coordinate permutation.  Both global polarity mirrors are run.

## 4. Frozen directions and periods

Normalize the following integer direction vectors:

```text
(-1,0,0), (1,1,0), (1,0,1), (1,1,1),
(-1,1,1), (2,-1,3), (-2,3,1), (3,2,-1)
```

Use `T in {64,128,256,512}`.  Every radial root is solved by bisection in
`r in [0, 4*pi*sqrt(3)]`; no root polishing, fit-window selection, or
post-run direction replacement is permitted.

The locked arm count is

\[
 4\text{ profiles}\times4T\times8\text{ directions}
 \times3\text{ axes}\times2\text{ mirrors}=768.
\]

Directions with `n_a P_m(n)=0` are retained as exact subdirection
cancellations but excluded from division by the asymptotic coefficient.

## 5. Frozen diagnostics and gates

- every arm brackets and solves a radial resonance;
- maximum absolute denominator residual `<= 1e-12`;
- minimum scaled radial regularity `T^2 |dD_T/dr| > 1`;
- maximum polarity-mirror residual `<= 1e-12`;
- maximum cyclic-covariance residual `<= 1e-12`;
- for every profile, period, axis, and mirror, at least one registered direction
  has nonzero exact forcing and nonzero leading polynomial;
- at `T=512`,
  `|T(r_T-r_0)-6*pi*n_a| < 0.25` for every direction;
- at `T=512`, the maximum relative error of the scaled forcing against the
  frozen leading coefficient is `< 0.20` over nonzero-leading arms;
- the independent Python proof must reproduce the arm count, all algebraic
  gates, and the verdict without reading the C++ result JSON.

## 6. Outcome map

- **Positive theorem:** all analytic steps hold and every registered gate
  passes.  Close fixed finite rigid linear sources as a universal slow-hop
  dressing cure.
- **Counterexample:** a nonzero finite profile cancels every registered
  full-surface witness or the analytic limiting argument fails.  Keep the
  finite neutral branch open and record the counterexample exactly.
- **Invalid:** implementation, root bracketing, covariance, or independent
  reproduction fails.  No physical conclusion.

No failed gate authorizes changing the directions, periods, profiles,
tolerances, field symbol, source numerator, or production dynamics.

## 7. Locked implementation paths

- `engine/include/ftd/eft/full_surface_source_obstruction.h`
- `engine/src/eft/full_surface_source_obstruction.cpp`
- `engine/tests/test_full_surface_source_obstruction.cpp`
- `scripts/proofs/proof_full_surface_source_obstruction.py`
- `engine/results/ftd_0562/windows_msvc_cpu.json`

## 8. Execution record

The pre-execution document was locked before observer implementation at
SHA-256
`D9F9B23232AB1A67A1829090C216207BAF58873E3EC9CE75CC809E395E0531D5`.

The registered campaign passed all gates: 768 arms, 96/96 witness groups,
maximum pole residual `2.855246421240576e-16`, minimum scaled regularity
`7.224481330653771`, maximum mirror residual `0`, maximum cyclic covariance
residual `3.481659405224491e-13`, `T=512` radius-correction residual
`0.13254411344298234`, and maximum forcing-asymptotic error
`0.010924707775298748`.  The independent Python verifier reproduced the
verdict.  Verdict:
`FINITE_RIGID_FULL_SURFACE_CANCELLATION_OBSTRUCTED`.
