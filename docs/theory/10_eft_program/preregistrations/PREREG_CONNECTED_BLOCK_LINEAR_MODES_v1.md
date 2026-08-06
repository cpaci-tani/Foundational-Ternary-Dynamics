# FTD-0629 — Connected-block linear modes v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION/RUN]`  
**Parent:** FTD-0628 verdict
`CONNECTED_BLOCK_STATIC_DRESSED_FIXED_POINT_CONSTRUCTIVE`  
**Scope:** mode-selective linearization of the selected connected-block action
about its refined dressed fixed point  
**Date:** 2026-07-27

## 1. Question

Does the FTD-0628 dressed fixed point support bounded, reversible,
approximately decoupled small-amplitude modes whose discrete frequencies are
predicted by the static Hessian and the unchanged production inertia?

This is a classical linear-response campaign. A successful result does not by
itself establish a quantum state, physical clock, particle mass, pole, spin, or
production ontology.

## 2. Frozen parent data

Use `L=17`, `width=2`, `kappa=1`, `dt=1`, `C_SPEED=1/sqrt(3)`, the unchanged
72-edge graph/action, minimum-energy longitudinal Gauss redressing, zero
magnetic half-field, and `allow_shared_anchor_chart=true`.

The fixed coordinates are the FTD-0628 x-arm values

`theta_0=(1.4993153663084844,0.4994670538459639,
          0.50006590532229034,0.50018096647517352)`.

Freeze the recorded x-arm Hessian `H` exactly as stored in the FTD-0628 arm
record. The small-velocity kinetic metric follows from the production
dispersion and the constituent count:

`M = M_INERTIAL diag(8,8,16,16)`, with `M_INERTIAL=0.511`.

Solve `H v_m = omega_m^2 M v_m`, order modes by increasing `omega`, and choose
each sign so its largest-magnitude coordinate is positive. Normalize
`v_m^T M v_n=delta_mn`.

The preregistered generalized eigenvalues, continuous linear frequencies,
and implicit-midpoint phase advances are:

| mode | `omega^2` | `omega` | `Omega=2 atan(omega/2)` | period ticks |
|---:|---:|---:|---:|---:|
| 0 | 4.9772034539 | 2.2309646913 | 1.6798663355 | 3.7402888400 |
| 1 | 22.4545051528 | 4.7386184857 | 2.3428303451 | 2.6818780627 |
| 2 | 61.3002662224 | 7.8294486538 | 2.6413975635 | 2.3787351794 |
| 3 | 67.7919941222 | 8.2335893827 | 2.6650081739 | 2.3576608015 |

The period prediction is the adiabatic matter-shape prediction. Failure may
show that dynamical field inertia hybridizes the mode; it is not automatically
an instability.

## 3. Arms

From `theta_0`, construct zero-momentum, freshly Gauss-redressed displacements
`theta=theta_0+s A v_m`.

- x orientation: all four modes at `s=+1`, `A=1e-4,2e-4`; and `s=-1`,
  `A=1e-4` (12 arms);
- cyclic y orientation: all four modes at `s=+1`, `A=1e-4` (4 arms).

Every arm runs 64 forward and 64 state-only inverse ticks. No other amplitude,
initial momentum, phase, mode mixture, window, or orientation may be added in
v1.

## 4. Observables and estimators

At every tick reconstruct the four symmetry coordinates by averaging the
registered outer/inner axial radii and transverse radii. Define

`q_m(t)=v_m^T M [theta(t)-theta_0]`.

For the launched mode, estimate phase from the fixed second-order recurrence

`cos(Omega_hat)=sum q_t(q_{t+1}+q_{t-1}) / [2 sum q_t^2]`

over ticks `1..62`, clamped only to `[-1,1]`. Do not search a frequency grid or
select a fit window. Mode leakage is the maximum nonlaunched RMS `q_n` divided
by launched RMS `q_m`. Excess energy is measured relative to the FTD-0628
fixed state using the unchanged complete energy.

## 5. Gates

1. Parent/protocol/coverage and all redressed initializations pass.
2. The independently recomputed generalized eigensystem is `M`-orthonormal;
   all `omega^2>0`; the tabulated predictions agree to `1e-9` relative.
3. Every transaction has common-action residual `<=1e-10`, energy drift
   `<=1e-12`, recovery `<=1e-10`, centre displacement `<=1e-8`, chart
   multiplicity `<=2`, and shared-anchor separation `>=0.9`.
4. Every launched-mode recurrence estimate is finite and within 2% of its
   predicted `Omega`.
5. Leakage is `<=0.10` in every arm.
6. For each positive x arm, the `2e-4` and `1e-4` phase estimates agree within
   0.5%; initial excess-energy ratio lies in `[3.90,4.10]`.
7. For each x mode at `A=1e-4`, signed trajectories satisfy
   `max_t |q_+(t)+q_-(t)|/A <=0.05` and their phase estimates agree within 0.5%.
8. The positive `A=1e-4` x/y trajectories satisfy
   `max_t |q_x(t)-q_y(t)|/A <=0.05`; phase estimates agree within 0.5%.

## 6. Locked verdicts

- `CONNECTED_BLOCK_ADIABATIC_LINEAR_MODES_CONSTRUCTIVE` if all gates pass.
- `CONNECTED_BLOCK_BOUNDED_HYBRID_MODES_OPEN` if execution, boundedness,
  energy, and inversion pass but a frequency, purity, amplitude, sign, or
  covariance gate fails.
- `CONNECTED_BLOCK_LINEAR_MODE_STABILITY_CLOSED_NEGATIVE` if any arm becomes
  unbounded or loses action, energy, fibre, or state-only inversion.
- `CONNECTED_BLOCK_LINEAR_MODES_EXECUTION_INVALID` for parent, protocol,
  eigensystem, coverage, instrumentation, or output failure.

## 7. Artifacts

Add a focused observer CTest; versioned JSON plus mode, arm, and tick CSVs; an
independent Python eigensystem/recurrence/gate certificate; analysis and audit;
and synchronized canonical navigation/status records. Production remains
unchanged.
