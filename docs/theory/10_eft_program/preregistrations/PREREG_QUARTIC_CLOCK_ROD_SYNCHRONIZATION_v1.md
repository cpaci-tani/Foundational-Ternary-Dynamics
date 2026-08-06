# PRE-REGISTRATION — Quartic Clock--Rod Synchronization v1

**Date locked:** 2026-08-02  
**Identifier:** `FTD-0771`  
**Status:** `[PRE-REGISTRATION — LOCKED/NOT YET RUN]`  
**Parents:** `FTD-0059`, `FTD-0137`, `FTD-0208`, `FTD-0407`,
`FTD-0658`, `FTD-0659`, `FTD-0770`  
**Scope:** exact symbolic analysis of a proposed synchronization between the
selected quartic clock and one primitive lattice interval. No production
state, update rule, clock, proper-time law, calibration, toggle, scenario,
engine source, or golden state may change.

## 0. Question and epistemic firewall

The registered question is:

> Does combining the minimum nonzero substrate interval with the normalized
> quartic period derive a unique dimensionless clock step containing `G*`?

The campaign must distinguish:

1. the abstract axial edge `ell` of the cubic coordinate lattice;
2. the primitive tick `tau`;
3. the topological support speed `C_Moore=ell/tau` in the registered
   `L-infinity` convention;
4. the selected transport speed `C_SPEED=ell/(sqrt(3) tau)`; and
5. the continuous-time parameter of the imposed quartic Hamiltonian.

The campaign may prove exact consequences conditional on a shared time
parameter. It must not assume that the Hamiltonian time is synchronized to
the primitive tick, that the amplitude-one shell is dynamically selected, or
that the physical SI length of one voxel is derived. It must not identify a
solver integration step with a physical tick. No fitting, constant search,
near-miss scan, or substitution identity is permitted.

## 1. Locked oscillator family

Let

```text
h_m(q,p) = (p^2+q^m)/2,                 even m >= 2,
H_(m,rho) = (rho/tau) h_m,              rho > 0,
E = h_m(q,p) > 0.
```

Here `rho` is the dimensionless rate matching between the mathematical
oscillator flow and one substrate tick. It is not set to one unless a
separate argument earns that value.

Lock

```text
K_m = 4 B(1/m,1/2)/m,
T_(m,rho)(E)/tau = K_m/[rho (2E)^((m-2)/(2m))].
```

For `m=4`, require

```text
K_4 = sqrt(pi) G*,
T_(4,rho)(E)/tau = sqrt(pi)G*/[rho (2E)^(1/4)].
```

The amplitude-one shell is `2E=1`. The protocol must retain both `rho` and
`E` until their epistemic status is decided.

## 2. Locked clock--rod observable

Define the causal duration of one registered lattice interval by

```text
tau_rod = ell/C_sub.
```

The dimensionless cycle fraction and phase advance are

```text
d_m = tau_rod/T_(m,rho)(E),
chi_m = 2pi d_m = Omega_(m,rho)(E) tau_rod.
```

For the topological support cone `C_sub=C_Moore=ell/tau`, require

```text
d_m = rho (2E)^((m-2)/(2m))/K_m,
d_4 = rho (2E)^(1/4)/(sqrt(pi)G*),
chi_4 = 2sqrt(pi) rho (2E)^(1/4)/G*.
```

At the jointly selected values `rho=1` and `2E=1`, the candidate reduces to

```text
d_4 = 1/(sqrt(pi)G*),
chi_4 = 2sqrt(pi)/G*.
```

Passing this identity licenses only
`CLOCK_ROD_RATIO_CONDITIONAL_GSTAR_PRESENT`. It does not by itself license a
substrate derivation of `rho=1`, `E=1/2`, or the quartic clock.

## 3. Locked invariance and calibration checks

Prove exactly that:

1. `ell` cancels when `C_Moore=ell/tau`; therefore the result is independent
   of the SI gauge assigned to one voxel;
2. under a common change of time coordinate, `rho` and `tau` combine so that
   `chi_m` is coordinate-invariant;
3. replacing the topological cone by the selected wave transport value
   `C_SPEED=ell/(sqrt(3)tau)` multiplies `d_m` by `sqrt(3)` and is not an
   axiomatic uniqueness result; and
4. `d_4` depends nontrivially on both `rho` and `E`.

The result must call `d_m` the minimum nonzero **substrate interval expressed
in clock cycles**, not a minimum numerical integration step.

## 4. Locked underdetermination countermodel

For every `rho>0`, construct the extension that applies the exact time-`tau`
flow of `H_(m,rho)` independently at each site once per substrate tick. The
extension is deterministic and on-site, so changing `rho` neither enlarges
the Moore dependency cone nor changes Postulates 1--5.

Exhibit two positive rates `rho_1 != rho_2` on the same nonzero energy shell.
If both models satisfy the registered postulates while

```text
d_m(rho_1,E) != d_m(rho_2,E),
```

then assign `P1_P5_SYNCHRONIZATION_UNDERDETERMINED`. This is a scoped no-go for
a unique clock--rod value from the current postulates, not a no-go for an
extended substrate rule that dynamically fixes `rho` and `E`.

## 5. Locked common-cone control

The FTD-0770 selected coupled clock gives, for graph continuum factor `d_R`,

```text
(c_clock/(Omega_m ell))^2
  = d_R eta (m-2)/(2m),             eta=kappa/E.
```

Impose only as a control `c_clock=C_sub`. Require

```text
Omega_m ell/C_sub
  = sqrt[2m/(d_R eta (m-2))],        m>2,
```

and for `m=4`,

```text
Omega_4 ell/C_sub = 2/sqrt(d_R eta).
```

The period modulus `K_m`, including `G*` for the quartic case, must cancel.
Passing this gate yields `COMMON_CONE_GSTAR_CANCELLATION`: speed matching
inside the selected linear clock field does not derive the proposed `G*`
synchronization.

## 6. Locked exact certificate

Add one deterministic symbolic proof script. It must check at least:

1. the general period quadrature;
2. `K_4=sqrt(pi)G*`;
3. rate scaling by `rho`;
4. the exact `d_m` and `chi_m` formulas;
5. the amplitude-one reduction;
6. cancellation of `ell` for `C_Moore`;
7. the `sqrt(3)` selected-transport alternative;
8. nonzero `rho` dependence;
9. nonzero quartic shell dependence;
10. the two-rate countermodel discriminator;
11. the general common-cone cancellation; and
12. its quartic specialization.

All checks are exact symbolic identities or inequalities. No empirical
constant comparison or numerical coincidence calculation is admissible.

## 7. Verdict map

- conditional ratio passes, countermodel passes, and common-cone cancellation
  passes: `CLOCK_ROD_RATIO_CONDITIONAL_GSTAR_PRESENT` plus
  `P1_P5_SYNCHRONIZATION_UNDERDETERMINED` plus
  `COMMON_CONE_GSTAR_CANCELLATION`;
- the postulates uniquely fix `rho` and `E` without a new premise:
  `CLOCK_ROD_SYNCHRONIZATION_DERIVED`;
- the proposed ratio is not dimensionless or fails common coordinate
  rescaling: `CLOCK_ROD_RATIO_INVALID`;
- `G*` survives the common-cone control after exact simplification:
  `COMMON_CONE_GSTAR_PRESENT`;
- any production or calibration state changes: `SCOPE_VIOLATION_INVALID`.

The final deliverable must include the locked protocol hash, exact proof,
canonical result, and synchronized theory navigation.
