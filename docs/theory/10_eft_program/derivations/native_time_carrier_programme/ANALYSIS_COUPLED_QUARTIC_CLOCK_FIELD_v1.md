# Analysis — Coupled Quartic Clock Field v1

**Identifier:** `FTD-0770`  
**Protocol:**
[`PREREG_COUPLED_QUARTIC_CLOCK_FIELD_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_COUPLED_QUARTIC_CLOCK_FIELD_v1.md),
SHA-256
`384C67CF1D6B96829C46C144414B1B5F43E8AE1FCD4FB4D83AA132EFB6616AB4`  
**Status:** `[IMPOSED / SELECTED CANDIDATE HAMILTONIAN]` +
`[THEOREM — CONDITIONAL ON THAT HAMILTONIAN]` +
`[CLOSED NEGATIVE — NON-RESCALABLE G* SIGNATURE IN THE REGISTERED LINEAR SECTOR]` +
`[OPEN — SUBSTRATE BRIDGE AND DYNAMICAL CONNECTION]`  
**Production status:** unchanged

## 0. Result in one sentence

A positive-action graph of coupled even-power clocks has exact total-action
conservation, graph-Laplacian relative-phase waves, the proposed compliance
rate law, and the standard cycle-holonomy integrability criterion; however,
after comparison with quadratic and sextic controls, every registered
dimensionless linear observable loses the local period normalization, so the
quartic exponent is dynamically distinctive while `G*` remains only a clock
calibration in this v1 model.

The locked verdicts are:

```text
COUPLED_QUARTIC_CLOCK_FIELD_V1_CONDITIONAL_THEOREMS_PASS
GSTAR_LINEAR_SIGNATURE_ABSENT
FIXED_BACKGROUND_HOLONOMY_KINEMATIC_ONLY
```

## 1. Claim altitude and relation to existing canon

This is not a theorem from FTD Postulates 1--5. A continuous canonical pair
`(I_v,theta_v)` at every voxel, a stiffness `kappa`, a compliance value `U_v`,
and a link connection `A_vw` are additional types. This document studies the
consequences after those types and their Hamiltonian are selected.

The distinction matters because:

- FTD-0208 closed derivation of the quadratic clock hypothesis from the bare
  discrete substrate negative;
- FTD-0658 found no intrinsic rest phase in the registered matter candidates;
- FTD-0659 found a coherent excited-matter phase but `0.898691` relative
  drift in its proposed conjugate action, so it did not establish an
  autonomous native action--angle pair; and
- gauge covariance of a fixed background `A` supplies neither a dynamical
  gauge field nor a Gauss constraint.

Accordingly, nothing here promotes a production clock, proper-time law,
physical `U(1)`, charge, gravity, or matter ontology.

## 2. Exact local clock family

For even `m>=2`, define

```text
H_m(q,p) = (p^2+q^m)/2.
```

Writing

```text
C_m = 2 B(1/m,3/2)/(pi m),
a_m = (m+2)/(2m),
nu_m = 2m/(m+2),
```

the action and its inverse are

```text
I_m(E) = C_m (2E)^a_m,
H_0,m(I) = (I/C_m)^nu_m/2.
```

Therefore

```text
Omega_m(I) = nu_m H_0,m(I)/I,
H_0,m''(I) = nu_m(nu_m-1)H_0,m(I)/I^2.
```

The period obeys

```text
T_m(E)(2E)^((m-2)/(2m)) = 4 B(1/m,1/2)/m.       (1)
```

For `m=4`, gamma recurrence gives

```text
C_4 = G*/(3 sqrt(pi)),
T_4(E)(2E)^(1/4) = sqrt(pi)G*.                   (2)
```

### Normalization correction

For the Hamiltonian used here, `sqrt(pi)G*` is the period on the
**amplitude-one shell** `2E=1`, hence `E=1/2`. At `E=1`, the period is
`2^(-1/4)sqrt(pi)G*`. The phrase “unit-energy period” is therefore ambiguous
and is not used in this result.

Numerically,

```text
C_4 = 0.55641789444938228,
T_4(E=1/2) = 5.2441151085842392.
```

## 3. Selected coupled Hamiltonian and its domain

Orient every undirected edge once and set

```text
H = sum_v exp(-U_v)H_0,m(I_v)
    + kappa sum_(v,w) [1-cos(theta_v-theta_w-A_vw)].       (3)
```

Here `theta_v` is stored as a real lift, while the interaction remains compact
modulo `2pi`. Hamilton's equations are

```text
dot(theta_v) = exp(-U_v)Omega_m(I_v),
dot(I_v) = -kappa sum_(w~v) sin(theta_v-theta_w-A_vw),     (4)
```

with the sign of `A` reversed when an edge is traversed backward.

The action chart is `I_v>0`. Equation (3) does not itself prescribe what
happens if coupling drives a site to `I_v=0`. The implementation therefore
rejects and rolls back any split step that crosses this boundary. This makes
the weak-perturbation theorem and verifier well defined but leaves a global
strong-coupling completion open.

## 4. Exact conserved temporal action

Each edge contributes `-kappa sin(Delta_vw)` to one endpoint and the opposite
amount to the other. Hence

```text
d/dt sum_v I_v = 0.                                (5)
```

This is Noether action conjugate to the global phase-shift symmetry of the
selected model. It is not identified with energy, charge, or information.
The Hamiltonian is also conserved by the exact continuous flow because `U`
and `A` are static; the implemented kick--drift--kick map is symplectic and
shows bounded numerical energy error rather than exact endpoint conservation.

## 5. Conditional temporal-wave theorem

Set `U=0`, `A=0`, and perturb the synchronized solution by

```text
I_v=I_0+j_v,
theta_v=Omega_0 t+theta_0+phi_v.
```

To first order,

```text
dot(phi)=H_0,m''(I_0)j,
dot(j)=-kappa L phi,
```

so

```text
ddot(phi)+kappa H_0,m''(I_0)L phi=0.              (6)
```

On a periodic one-dimensional chain,

```text
omega^2(k)=4kappa H_0,m''(I_0)sin^2(k ell/2).     (7)
```

The mode is gapless because (3) is invariant under one global phase shift.
Equation (6) is a theorem of the selected Hamiltonian, not yet an emergent
mode of the FTD substrate.

## 6. Lattice-topology correction

For an inversion-symmetric neighbor displacement shell `R`, the long-wave
Laplacian is controlled by

```text
D_ij = (1/2) sum_(r in R) r_i r_j,
L phi = -ell^2 D_ij partial_i partial_j phi+O(ell^4).     (8)
```

The six axial cubic neighbors give `D=identity`, so the speed displayed in the
proposal is correct for that graph:

```text
c_axial^2 = kappa H_0,m''(I_0)ell^2.              (9)
```

Equal coupling to all 26 Moore neighbors instead gives `D=9 identity`:

```text
c_Moore^2 = 9kappa H_0,m''(I_0)ell^2.             (10)
```

For the quartic law this is

```text
c_axial^2 = (2kappa ell^2/9)
  (3sqrt(pi)/G*)^(4/3) I_0^(-2/3),
c_Moore^2 = 9 c_axial^2.                           (11)
```

Thus a claim about an FTD-voxel clock field must name its edge set and weights;
“neighboring voxels” is not enough to fix the continuum coefficient.

## 7. Decisive control-family theorem

Let `E_0=H_0,m(I_0)` and define the independently meaningful energy ratio
`eta=kappa/E_0`. Using the power-law identities in §2,

```text
kappa H_0,m''(I_0)/Omega_m(I_0)^2
 = eta (nu_m-1)/nu_m
 = eta (m-2)/(2m).                                 (12)
```

Therefore, for the axial graph,

```text
(c/(Omega_0 ell))^2 = 0       for m=2,
                       eta/4  for m=4,
                       eta/3  for m=6.              (13)
```

For a neighbor factor `d_R`, the right side is multiplied by `d_R`. In every
case the normalization `C_m` cancels exactly. In particular, the quartic
`G*` in (2) is absent.

This control establishes two different facts:

1. **The exponent matters.** The quadratic oscillator is isochronous,
   `H_0,2''=0`, so action transfer does not restore a relative-phase
   perturbation. Quartic and sextic clocks do support the wave.
2. **The quartic modulus does not yet matter non-rescalably.** Once the local
   cycle and coupling energy are compared dimensionlessly, `G*` disappears.

The other proposed candidate, “defect energy divided by local clock action,”
is not dimensionless: energy/action is a frequency. The corrected ratios are
`E_defect/E_0` or `E_defect/(Omega_0 I_0)`; for the fixed cosine connection
both again reduce to `eta` times a geometric phase factor and contain no `G*`.

The scoped closed-negative verdict is therefore:

> `GSTAR_LINEAR_SIGNATURE_ABSENT` — within the registered linear coupled-clock
> Hamiltonian, `G*` is a canonical quartic clock calibration, not a
> non-rescalable constant of relational dynamics.

This does not exclude a nonlinear or substrate-derived observable with an
independently fixed second structure.

## 8. Compliance law

For two uncoupled clocks,

```text
dot(theta_1)/dot(theta_2)
 = exp[-(U_1-U_2)] Omega(I_1)/Omega(I_2).          (14)
```

At equal action this reduces exactly to

```text
dot(theta_1)/dot(theta_2)=exp[-(U_1-U_2)].         (15)
```

The verifier obtains `0.57694981038048676` for `U_1=0.2`, `U_2=-0.35`,
matching `exp(-0.55)` inside `1e-13`. This proves the rate law for the imposed
Hamiltonian. It does not derive `U`, its source law, or gravitational time
dilation.

## 9. Connection integrability and the holonomy boundary

Under

```text
theta_v -> theta_v+alpha_v,
A_vw -> A_vw+alpha_v-alpha_w,
```

the edge phase in (3) is invariant. On a finite connected graph, a vertex
gauge can remove `A` modulo `2pi` iff every cycle sum vanishes modulo `2pi`.
This is the exact graph-cohomology integrability criterion.

The verifier checks a flat square, a square with one `0.3`-radian edge flux,
and a nonuniform gauge transformation. The flat square is integrable; the
second retains cycle residual `0.3`; its Hamiltonian and obstruction are gauge
invariant inside `1e-12`.

But `A` is fixed input in (3). There is no conjugate link momentum and no
Hamilton equation for `A`. Its holonomy therefore “persists” only because the
program never updates it. A propagation/decay classification would be a
category error. A successor must add a dynamical connection plus its conjugate
field energy and then preregister defect-energy, transport, and decay gates.

## 10. Locked numerical verification

The C++ verifier uses velocity Verlet for the original `(q,p)` controls and a
second-order canonical split for the graph Hamiltonian. All gates pass:

| Quantity | Measured | Locked gate |
|---|---:|---:|
| maximum period relative error, 9 control arms | `1.5320887892e-9` | `<=2e-6` |
| maximum single-clock relative energy drift | `3.1501714015e-8` | `<=2e-6` |
| quartic period-invariant relative spread | `0` | `<=2e-6` |
| maximum dispersion relative error, 6 chain arms | `2.1603819622e-8` | `<=2e-3` |
| maximum total-action absolute drift | `4.0500935938e-13` | `<=1e-11` |
| maximum chain relative energy drift | `1.6209256159e-14` | `<=5e-7` |
| quadratic phase-mode drift | `7.1341858202e-16` | `<=1e-10` |
| fixed-connection change | `0` | exact |

The symbolic certificate reports `15/15 PASS`. The focused native CTest
reports `1/1 PASS`.

Implementation:

- `engine/include/ftd/eft/coupled_quartic_clock_field.h`;
- `engine/src/eft/coupled_quartic_clock_field.cpp`;
- `engine/tests/test_coupled_quartic_clock_field.cpp`; and
- `scripts/proofs/proof_coupled_quartic_clock_field.py`.

## 11. Scientific verdict and successor gate

The coupled-clock construction successfully crosses from an isolated period
to a mathematically coherent relational Hamiltonian, but it has not crossed
from an imposed auxiliary model to an FTD-native temporal field theory.

The immediate v1 conclusion is:

```text
local even-power clock + selected phase coupling
    -> conserved total action
    -> conditional gapless relative-phase waves
    -> exact compliance and cycle-integrability laws,

but

dimensionless linear comparison
    -> exponent m survives
    -> period modulus C_m cancels
    -> G* remains calibration at this scope.
```

The next admissible research step is not another constant identity. It must do
at least one of the following under a fresh lock:

1. derive or select an autonomous positive-action pair from existing substrate
   state while explaining FTD-0659's action leakage;
2. derive `kappa/E_0` and the neighbor weights from an independently fixed FTD
   interaction rather than inserting them;
3. promote `A` to a fully specified dynamical link sector with conjugate field
   energy and test a genuine defect; or
4. preregister a nonlinear dimensionless observable for which the exact
   cancellation theorem (12) does not already decide the outcome.

The quartic-selection lemma remains conditional on the unearned premise that
the signed distinction has no quadratic term. Until the substrate derives or
explicitly adopts that premise, `V=q^4/2` is a coherent candidate, not a
selected law of physical time.
