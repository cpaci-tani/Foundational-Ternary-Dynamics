# FTD-0702 — Matched face-current spectrum observer v1

**Status:** `[PRE-REGISTRATION — OBSERVER QUALIFICATION]`  
**Production status:** unchanged  
**Campaign class:** deterministic algebraic/numerical observer qualification

## 1. Purpose

FTD-0701 uses the constituent point structure factor only. The reciprocal
matter action instead deposits an oriented quadratic-coat face current. This
protocol qualifies a carrier-aware observer for that actual current before any
connected-matter form-factor measurement is run.

## 2. Frozen definition

For an arbitrary nonzero wavevector `k` in the first Brillouin zone, define

\[
\widehat K_a(\mathbf k)
=\frac1N\sum_f K^a_f
 e^{-i\mathbf k\cdot\mathbf r^a_f},
\]

where `r^x_f=(x+1/2,y,z)` and cyclically for the other oriented faces. `N>0`
is an explicit caller-supplied normalization; there is no implicit `L^-3`
factor. The lattice projector uses `khat_a=2 sin(k_a/2)`:

\[
K_L=\widehat k\,{\widehat k\cdot K\over|\widehat k|^2},
\qquad K_T=K-K_L.
\]

The observer reports complex coefficients, transverse/longitudinal/total
power, transverse fraction, input `L1` norm, and projection residual. Sparse
entries and dense face arrays are two representations of the same definition.

## 3. Locked fixtures

Use `L=17` and wavevectors strictly inside `[-pi,pi]^3`.

1. one oriented face with an analytically known carrier phase;
2. one quadratic-coat straight segment and its dense/sparse representations;
3. charge mirrors `q=+1,-1`;
4. segment displacements `delta` and `2 delta`;
5. three independent integer translations with no boundary crossing;
6. two cyclic cubic rotations of segment and wavevector;
7. an explicit duplicate-entry superposition;
8. zero wavevector, zero normalization, invalid axis, and nonfinite-value
   rejection.

## 4. Acceptance gates

- one-face complex coefficient residual `<=1e-14`;
- dense/sparse complex coefficient residual `<=1e-14`;
- charge mirror coefficient residual `<=1e-14` and power residual `<=1e-14`;
- doubled-displacement coefficient linearity is diagnostic only because the
  moving spline support changes; both segments must still pass exact
  continuity and current-moment gates;
- integer translation changes only the registered Fourier phase within
  `2e-13`;
- cyclic rotation permutes coefficients and preserves all powers within
  `2e-13`;
- `|K|^2=|K_T|^2+|K_L|^2` within relative `2e-13`;
- projection residual `<=1e-14`;
- every invalid fixture fails closed.

## 5. Consequence

Passing qualifies only the numerical current observer. It does not establish
a connected-object form factor, radiation, wake, lifetime, photon, particle
pole, or production behavior. A separate locked campaign must use the
qualified observer on the selected connected object.
