# PRE-REGISTRATION — Quartic Waveform Nonlinear Edge Signature v1

**Date locked:** 2026-08-02  
**Identifier:** `FTD-0773`  
**Status:** `[PRE-REGISTRATION — LOCKED/NOT YET RUN]`  
**Parents:** `FTD-0659`, `FTD-0770`, `FTD-0771`, `FTD-0772`  
**Campaign type:** exact symbolic audit and epistemic boundary; no native or
selected-model engine execution  
**Production status:** unchanged

## 0. Question and firewall

The registered questions are:

1. Which parts of the proposed chain

   ```text
   temporal occupancy -> speed -> potential -> phase/action
   ```

   are exact consequences of a fixed continuous one-dimensional natural
   coordinate?
2. For a separately selected quadratic coordinate-edge interaction, is

   ```text
   B_4 = [Vbar(pi)-Vbar(0)]/Vbar''(0) = 48 pi/G*^4
   ```

   exact and invariant under constant scale changes?
3. Does that identity complete a **native FTD** derivation or provide an
   independent native coupling measurement now?

The protocol must not call a continuous auxiliary flow the native FTD tick
map. It must not infer a density, derivative, potential, canonical momentum,
action, or phase-response covector from the recurrence-unqualified FTD-0659
corpus. It must not insert a quartic oscillator and a quadratic edge into a
new engine test and present reproduction of their analytic consequence as a
native discovery.

No exponent search, density fit, near-miss scan, physical-constant
substitution, or production change is admissible.

## 1. Locked continuous-flow hypotheses

Let `X(t)` be a continuous autonomous flow with a regular periodic orbit of
period `T(A)`. Fix a signed scalar `q=Q(X)` before waveform inspection and a
true turning amplitude `A>0`. Define `x=q/A`.

For every interior `x`, require exactly two crossings per cycle with equal
speed magnitude. Only under this branch-reversibility hypothesis may the
occupancy density be inverted pointwise:

```text
rho_A(x) = 2/[T(A)|x_dot(x)|],
|q_dot| = 2A/[T(A)rho_A(x)].
```

Without equal branch speeds the density determines only the harmonic mean of
the crossing speeds:

```text
rho_A(x) = [1/v_+(x)+1/v_-(x)]/T(A),
v_harm(x) = 2/[T(A)rho_A(x)].
```

It does not determine either branch separately. A sample maximum is not a
turning amplitude unless a separately locked interpolation/preparation rule
earns that identification.

## 2. Locked inverse-potential statement

Add the independent hypotheses that `q` is a fixed unit-mass natural
coordinate and closes conservatively:

```text
(1/2) q_dot^2 + V(q) = V(A),
V(0)=0,
V(A)>V(q) for |q|<A.
```

Then require the exact inverse formulas

```text
V(A)-V(Ax) = 2A^2/[T(A)^2 rho_A(x)^2],
V(A) = 2A^2/[T(A)^2 rho_A(0)^2],
V(Ax) = [2A^2/T(A)^2]
  [rho_A(0)^(-2)-rho_A(x)^(-2)].
```

Outside the branch-reversible conservative closure, the last expression may
be emitted only as a **pseudo-potential diagnostic**. Cross-amplitude collapse
is necessary evidence for a one-coordinate model, not proof that the native
state supplies a canonical symplectic pair.

## 3. Locked quartic characterization and clock corollaries

For exact amplitude-invariant occupancy

```text
rho_4(x) = C_4/sqrt(1-x^4),
C_4 = 2/[sqrt(pi)G*],
G* = Gamma(1/4)/Gamma(3/4),
```

require

```text
V(Ax)=V(A)x^4,
V(q)=lambda q^4
```

only on the region swept by a nontrivial amplitude interval. `lambda>0`
remains a time/mass normalization; `lambda=1/2` is the selected canonical
normalization, not an occupancy output.

For the unit-mass Hamiltonian

```text
H=p^2/2+lambda q^4,
```

lock

```text
T(A) = sqrt(pi)G*/[A sqrt(2lambda)],
Omega(A) = 2sqrt(pi)A sqrt(2lambda)/G*,
I(A) = A^3 sqrt(2lambda)G*/[3sqrt(pi)],
Omega=dE/dI,
H_0''(I)=2pi/[A^2 G*^2].
```

On the increasing branch from the central crossing,

```text
dtheta/dx = pi rho_4(x).
```

For `0<=x<=1`, require

```text
theta(x) = pi/[2sqrt(pi)G*]
  B_(x^4)(1/4,1/2),
theta(1)=pi/2.
```

For `-1<=x<=0`, the signed extension carries `sign(x)`; an unsigned
`B_(x^4)` formula is not valid on both sides. The remaining branches follow
by reflection and continuous phase lifting.

Deriving a canonical action additionally requires a native symplectic form
and `p=q_dot`; a periodic scalar waveform alone does not supply either type.

## 4. Locked moment and exponent identities

Require, for `r>-1`,

```text
mu_r = <|x|^r>
     = B((r+1)/4,1/2)/B(1/4,1/2),
mu_(r+4) = [(r+1)/(r+3)] mu_r,
mu_1=sqrt(pi)/G*,
mu_2=4/G*^2,
mu_4=1/3.
```

The identities

```text
G_rms = 2/sqrt(mu_2),
G_abs = sqrt(pi)/mu_1,
G_kurt = [48 mu_4/mu_2^2]^(1/4)
```

are correlated functionals of one waveform, not statistically independent
discoveries.

For an exact homogeneous density family only,

```text
m(x) = log[1-(rho(0)/rho(x))^2]/log|x|.
```

This expression is undefined at `x=0, +/-1`, ill-conditioned near those
points, and unavailable for a finite atomic tick measure without a separately
locked density estimator. FTD-0773 will prove the identity but perform no
empirical exponent inference.

## 5. Locked quadratic-edge waveform functional

Select separately—not derive—the weak coordinate-edge energy

```text
V_vw = epsilon(q_v-q_w)^2/2.
```

For equal-amplitude uncoupled waveforms parameterized by uniform phase, define

```text
C_m(phi) = (1/2pi) integral_0^(2pi)
  x_m(theta)x_m(theta+phi) dtheta,
Vbar_m(phi) = epsilon A^2[mu_2,m-C_m(phi)].
```

For the even-power family `H_m=(p^2+|q|^m)/2`, lock

```text
B_m0 = B(1/m,1/2),
mu_2,m = B(3/m,1/2)/B_m0,
D_m = <(dx_m/dtheta)^2>
    = 4 B_m0^2/[m(m+2)pi^2],
Delta_m = Vbar_m(pi)-Vbar_m(0)
        = 2 epsilon A^2 mu_2,m,
K_m = Vbar_m''(0) = epsilon A^2 D_m,
B_m = Delta_m/K_m
    = m(m+2)pi^2 B(3/m,1/2)/[2 B_m0^3].
```

Require the registered controls

```text
B_2 = 2,
B_4 = 48pi/G*^4,
B_6 = 24pi^3/B(1/6,1/2)^3.
```

For `m=4`, also prove

```text
(dx/dtheta)^2 = [G*^2/(4pi)](1-x^4),
D_4=G*^2/(6pi),
Delta_4=8 epsilon A^2/G*^2,
K_4=epsilon A^2G*^2/(6pi),
B_4=48pi/G*^4,
H_0'' K_4=epsilon/3.
```

`B_4` is invariant under constant changes of `A`, `epsilon`, physical time
unit, and `lambda`, once uniform `2pi` phase and the quadratic coordinate edge
are fixed. It is not invariant under a nonlinear change of the observable or
a change in edge functional.

The rearrangement

```text
G_edge = [48pi K_4/Delta_4]^(1/4)
```

is a conditional waveform consistency functional. It is not independent of
the quartic waveform from which `Delta_4` and `K_4` are computed.

## 6. Discrete-native boundary

FTD's primitive evolution is a tick map, not the continuous flow assumed in
§1. A finite `P`-tick orbit has an atomic measure and does not license
`rho(x)`, `x_dot`, the continuous inverse potential, or the continuous adjoint
equation without an explicit suspension, refinement, or equidistribution
theorem.

For a differentiable discrete map, a phase-response covector would obey a
discrete adjoint recurrence involving the transpose inverse of the Jacobian,
not `Z_dot=-DF^T Z`. That discrete construction is outside this exact audit.

FTD-0772 already establishes that the current FTD-0659 fixed-ray candidate is
recurrence-unqualified: all `18` cells fail its locked return/stationarity
gates. The corpus also contains no native paired-edge interaction record from
which `Vbar(phi)` can be measured. Therefore the native nonlinear signature
is blocked before numerical comparison.

No engine execution is admissible in v1. A new engine test would have to
insert both `q^4` and `epsilon(q_v-q_w)^2/2`; it would be a selected-model
implementation check, not a native FTD measurement, while the exact identity
is already decidable symbolically.

## 7. Locked exact certificate

Add one independent SymPy certificate with at least these groups:

1. two-branch occupancy/speed inversion and the unequal-branch harmonic-mean
   boundary;
2. conservative inverse-potential formulas;
3. quartic normalization and fixed-coordinate characterization;
4. period, frequency, action, `dE/dI`, and `H_0''`;
5. signed incomplete-beta phase and quarter-turn value;
6. general moments, recurrence, and all three `G` functionals;
7. exact homogeneous exponent identity and excluded endpoints;
8. general `m` waveform derivative moment, barrier, curvature, and ratio;
9. exact `m={2,4,6}` controls;
10. quartic `48pi/G*^4` and `epsilon/3` cancellation;
11. scale cancellation and nonlinear-coordinate dependence; and
12. finite atomic versus continuous measure distinction.

All checks are exact identities or structural inequalities. The displayed
decimal value of `B_4` may be certified only as a rounding of the exact
expression, never as a searched match.

## 8. Verdict map

- all exact groups pass:
  `QUARTIC_CONTINUOUS_INVERSE_CHAIN_CONDITIONAL_THEOREMS_PASS` plus
  `QUARTIC_NONLINEAR_EDGE_SHAPE_FUNCTIONAL_GSTAR_PRESENT`;
- the ratio retains `A`, `epsilon`, `lambda`, or a constant time scale:
  `NONLINEAR_EDGE_RATIO_NOT_SCALE_FREE`;
- the ratio is claimed independent of the selected waveform/edge functional:
  `INDEPENDENCE_PROMOTION_INVALID`;
- the continuous chain or selected edge is reported as native FTD content:
  `NATIVE_QUARTIC_TIME_DERIVATION_NOT_ESTABLISHED`;
- an FTD-0659/native coupling run is reported despite the recurrence and edge
  prerequisites:
  `NATIVE_NONLINEAR_EDGE_TEST_BLOCKED`;
- any engine, production, calibration, toggle, scenario, or golden state
  changes:
  `SCOPE_VIOLATION_INVALID`.

The final deliverable must include the immutable protocol hash, exact proof,
canonical analysis, and synchronized theory navigation.
