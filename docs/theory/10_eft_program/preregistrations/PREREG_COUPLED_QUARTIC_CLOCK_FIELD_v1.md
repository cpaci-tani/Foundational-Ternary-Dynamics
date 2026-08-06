# PRE-REGISTRATION — Coupled Quartic Clock Field v1

**Date locked:** 2026-08-02  
**Identifier:** `FTD-0770`  
**Status:** `[PRE-REGISTRATION — LOCKED/NOT YET RUN]`  
**Parents:** `FTD-0002`, `FTD-0208`, `FTD-0411`, `FTD-0543`,
`FTD-0658`, `FTD-0659`  
**Scope:** exact analysis plus a deterministic CPU-only selected-extension
verifier. This protocol changes no production tick, voxel layout, toggle,
scenario, calibration, matter claim, proper-time law, or golden state.

## 0. Epistemic firewall

The proposed variables `(I_v, theta_v)`, stiffness `kappa`, compliance `U_v`,
and edge connection `A_vw` are **not** consequences of FTD Postulates 1--5.
They define an `[IMPOSED / SELECTED CANDIDATE HAMILTONIAN]`. Theorems proved
below are conditional on that Hamiltonian and its positive-action domain.

The campaign must not claim that:

1. the model derives physical time, proper time, gravity, `U(1)`, charge, or a
   production engine clock;
2. a fixed background connection is a dynamical gauge field;
3. the quartic potential is selected by FTD merely because its normalized
   period contains `G*`;
4. a dimensionful occurrence of `G*` is a non-rescalable prediction; or
5. the result reopens the closed-negative FTD-0208 clock-hypothesis campaign
   or promotes the mixed FTD-0659 excited-matter clock.

No parameter fitting, constant search, or near-miss scan is permitted.

## 1. Locked local family

Use the even-power controls

```text
H_m(q,p) = (p^2 + q^m)/2,       m in {2,4,6}.
```

For general even `m >= 2`, lock

```text
C_m = 2 B(1/m,3/2)/(pi m),
a_m = (m+2)/(2m),
nu_m = 1/a_m = 2m/(m+2),
I_m(E) = C_m (2E)^a_m,
H_0,m(I) = (I/C_m)^nu_m / 2.
```

The normalized-shell period is

```text
T_m(E) (2E)^((m-2)/(2m)) = 4 B(1/m,1/2)/m.
```

For `m=4`, require the exact reductions

```text
C_4 = G*/(3 sqrt(pi)),
T_4(E) (2E)^(1/4) = sqrt(pi) G*.
```

The phrase “unit clock” in the result must mean the amplitude-one shell
`2E=1`, hence `E=1/2`; it must not call `E=1` the shell on which the period is
`sqrt(pi) G*`.

## 2. Locked coupled model

On a finite undirected graph, orient each edge once and define

```text
H = sum_v exp(-U_v) H_0,m(I_v)
    + kappa sum_(v,w) [1-cos(theta_v-theta_w-A_vw)].
```

The phase is stored as a real lift, while the interaction remains compact.
The admissible chart is `I_v > 0`. The implementation must reject and roll
back a step that crosses the action boundary.

For static `U` and `A`, require

```text
d theta_v/dt = exp(-U_v) Omega_m(I_v),
d I_v/dt = -kappa sum_(w~v) sin(theta_v-theta_w-A_vw),
sum_v I_v = constant.
```

Use a second-order kick--drift--kick symplectic split. This is an isolated EFT
probe, not a `RenderBridge` phase.

## 3. Locked linear theorems

For `U=0`, `A=0`, and the synchronized background `(I_0, Omega_0 t)`, lock

```text
ddot(phi) + kappa H_0,m''(I_0) L phi = 0,
omega^2(k) = 4 kappa H_0,m''(I_0) sin^2(k ell/2)
```

for the periodic one-dimensional chain.

For an inversion-symmetric displacement shell `R`, the long-wave graph factor
is

```text
D_ij = (1/2) sum_(r in R) r_i r_j.
```

Require `D_ij=delta_ij` for the six axial neighbors and
`D_ij=9 delta_ij` for the equal-weight 26-neighbor Moore shell. Therefore the
attachment's displayed continuum speed is the axial value; the equal-weight
Moore value has `c^2` larger by a factor of nine.

## 4. Locked decisive ratio and controls

At background energy `E_0=H_0,m(I_0)`, define `eta=kappa/E_0`. Prove before
interpreting any numerical result that

```text
R_m^2 := kappa H_0,m''(I_0)/Omega_m(I_0)^2
       = eta (m-2)/(2m).
```

Hence the period-normalized axial wave-to-cycle ratios are locked as

```text
m=2: R_2^2 = 0,
m=4: R_4^2 = eta/4,
m=6: R_6^2 = eta/3.
```

The result must explicitly record that `C_m`, and therefore `G*` at `m=4`,
cancels. Passing this gate yields the scoped verdict
`GSTAR_LINEAR_SIGNATURE_ABSENT`: the quartic exponent is distinguishable in
the linear relational dynamics, but the quartic period modulus is only a
clock calibration in every registered dimensionless linear observable.
This is not a no-go for nonlinear, dynamical-connection, or substrate-bridged
observables not tested here.

## 5. Locked compliance and connection tests

For two uncoupled equal-action clocks, require

```text
dot(theta_1)/dot(theta_2) = exp[-(U_1-U_2)]
```

to relative error `<= 1e-13`. The result must call this an exact property of
the imposed compliance Hamiltonian, not a derivation of `U` or its source law.

For a finite connected graph, reconstruct vertex gauge offsets along a
spanning tree. Require that the connection is removable modulo `2pi` iff every
edge residual is zero modulo `2pi`. Test:

- a zero-holonomy square;
- a square with one registered `0.3`-radian edge flux;
- invariance under a nonuniform vertex gauge transformation; and
- Hamiltonian invariance under the same transformation.

All exact connection and energy residuals must be `<= 1e-12`.

Because `A_vw` is fixed input, its holonomy is constant by construction. The
campaign must return `FIXED_BACKGROUND_HOLONOMY_KINEMATIC_ONLY`, not classify
the connection as a persistent, propagating, or decaying dynamical defect.
Such a test requires a dynamical link variable and a conjugate field term in a
successor protocol.

## 6. Locked numerical gates

### Single clocks

For `m in {2,4,6}` and `2E in {1/16,1,16}`, integrate the original `(q,p)`
dynamics from the positive turning point with velocity Verlet and at least
`32768` steps per predicted period. Measure the first positive-to-negative
zero crossing and infer the period. Require:

- relative period error `<= 2e-6`;
- relative energy drift `<= 2e-6`; and
- quartic scaling-invariant spread `<= 2e-6`.

### Coupled chains

Use `N=64`, `2E_0=1`, `kappa/E_0=0.4`, phase amplitude `1e-4`, and periodic
modes `n in {1,3,8}`. Infer the first modal zero crossing. Require:

- quartic measured-dispersion relative error `<= 2e-3` for every mode;
- sextic measured-dispersion relative error `<= 2e-3` for every mode;
- quadratic phase-mode drift `<= 1e-10` over the registered window;
- total-action absolute drift `<= 1e-11`; and
- relative Hamiltonian drift `<= 5e-7`.

The time step must be no larger than `0.002` and no gate may be retuned after
execution.

## 7. Verdict map

- all exact and numerical gates pass:
  `COUPLED_QUARTIC_CLOCK_FIELD_V1_CONDITIONAL_THEOREMS_PASS` plus
  `GSTAR_LINEAR_SIGNATURE_ABSENT` and
  `FIXED_BACKGROUND_HOLONOMY_KINEMATIC_ONLY`;
- exact algebra passes but a numerical tolerance fails:
  `COUPLED_CLOCK_NUMERICAL_VERIFIER_UNRESOLVED`;
- the decisive ratio retains `C_4` or `G*` after simplification:
  `GSTAR_LINEAR_SIGNATURE_PRESENT`;
- the graph factor is reported without distinguishing axial from Moore:
  `TOPOLOGY_NORMALIZATION_INVALID`;
- any production engine state or default changes:
  `SCOPE_VIOLATION_INVALID`.

The final deliverable must include the exact proof, selected-extension C++
module, focused CTest, result analysis, and synchronized canonical navigation.
