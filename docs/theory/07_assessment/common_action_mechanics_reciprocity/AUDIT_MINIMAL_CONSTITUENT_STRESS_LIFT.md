# AUDIT — Minimal constituent-stress lift

**Date:** 2026-07-25  
**Identifier:** `FTD-0513`  
**Status:** `[THEOREM — MINIMAL ADDITIVE POLYNOMIAL MOMENT, SCOPED]` +
`[CONSTRUCTIVE — TWO-STREAM STRESS INVERSION]` +
`[THEOREM — RANK-2 MULTISTREAM NONINJECTIVITY]` +
`[CLOSED BY FTD-0514 — LOCAL MOMENTUM BALANCE]` +
`[OPEN — CONJUGATE FIELD/CONTACT ACTION]`  
**Verdict:** `RANK2_STRESS_IS_MINIMAL_FOR_TWO_STREAM_KERNEL_NOT_COMPLETE`  
**Pre-registration:**
[`PREREG_MINIMAL_CONSTITUENT_STRESS_LIFT_v1.md`](../10_eft_program/preregistrations/PREREG_MINIMAL_CONSTITUENT_STRESS_LIFT_v1.md)  
**Run of record:** `engine/results/ftd_0513/windows_msvc_cpu.json`

## 1. Why density and vector current had to fail

The FTD-0512 counterflow state is the momentum multiset

```text
{+p n,-p n}.
```

It is invariant under momentum inversion. Carrier count is a degree-zero
scalar and contains no direction. Every additive degree-one momentum moment
is proportional to total momentum and vanishes. More generally, an exactly
cubic-covariant polar vector is inversion-odd, so it must vanish on an
inversion-even pair state. A scalar even moment can retain the speed but not
the unoriented axis `{+n,-n}`.

The first additive polynomial object that can retain both is the symmetric
degree-two moment. Under the full cubic group,

```text
Sym^2(T1u) = A1g + Eg + T2g.
```

The `A1g` trace carries magnitude; the `Eg + T2g` quadrupole carries the
unoriented axis. This proves minimality only in the preregistered class of
additive polynomial moments with exact `O_h` covariance. It does not exclude
constituent lists, non-polynomial encodings, or higher moments.

## 2. Derived kinetic-stress observer

From existing carrier momentum and the production dispersion,

```text
E_i=sqrt(m^2+c^2|p_i|^2),
v_i=c^2 p_i/E_i,
Sigma=sum_i p_i tensor v_i
     =c^2 sum_i (p_i tensor p_i)/E_i.
```

`Sigma` is symmetric, positive semidefinite, polarity-even, and transforms as
`Sigma -> R Sigma R^T`. It is the spatial kinetic-stress moment of the
constituents. It is not electromagnetic stress and is not a new primitive
field in this audit.

For an equal opposite pair,

```text
Sigma=tau n tensor n,
tau=2 c^2 p^2/E=2(E-m^2/E).
```

The tensor is rank one. Its normalized form recovers the axis projector:

```text
Sigma/tr(Sigma)=n tensor n.
```

Let `t=tr(Sigma)/2`. The positive dispersion branch gives

```text
E=(t+sqrt(t^2+4m^2))/2,
p=sqrt(E^2-m^2)/c.
```

Thus the same six-component tensor recovers the unoriented collision axis,
momentum magnitude, and pair kinetic energy in this restricted class.

## 3. Exact two-stream result

Across the 312 direction/polarity/translation/speed arms and 144 explicit
signed-cubic covariance arms:

```text
worst PSD residual                    1.08e-19
worst rank-one residual               6.94e-18
worst axis-projector residual         1.11e-16
worst energy recovery residual        1.11e-16
worst momentum recovery residual      2.22e-16
worst kinetic recovery residual       2.22e-16
worst translation residual            0
worst polarity residual               0
worst cubic covariance residual       0
minimum nonzero stress trace          0.04907013423428238.
```

This closes observability of the FTD-0512 rank-one counterflow mode. It does
not select the central-elastic collision premise and does not yet provide a
local momentum-balance update.

## 4. Rank-2 stress is not a complete matter state

Consider four equal-magnitude streams at one point:

```text
A={+p ex,-p ex,+p ey,-p ey},
B={+p d1,-p d1,+p d2,-p d2},
d1=(ex+ey)/sqrt(2), d2=(ex-ey)/sqrt(2).
```

Both configurations have the same carrier count, zero total momentum, equal
total energy, and exactly the same rank-2 stress:

```text
Sigma_A=Sigma_B=2 c^2 p^2/E (ex tensor ex + ey tensor ey).
```

They are different momentum multisets. Their fourth moments differ; for
example, the axial set has zero `sum p_x^2 p_y^2`, while the diagonal set does
not. Three registered magnitudes gave:

```text
minimum momentum-multiset separation  0.07653668647301795
worst conserved-quantity residual      0
worst rank-2 stress residual            5.55e-17
minimum fourth-moment difference        0.0001.
```

Therefore no finite claim that `(rho,J,Sigma)` is the complete general matter
state is admissible. It is sufficient for a single rank-one two-stream mode,
not for arbitrary multistream structure. Higher even moments form a hierarchy;
an explicit constituent list remains the lossless representation.

## 5. Ontological and dynamical consequence

FTD now has a clean separation of roles:

1. oriented face current is the correct additive charge-transport observable;
2. constituent kinetic stress is the first scoped observable of equal-and-
   opposite internal momentum flow;
3. neither observable by itself supplies a contact interaction law.

Stress cannot be appended to the electromagnetic face action as another
vector source without additional structure. A symmetric stress tensor is
conjugate to a symmetric strain/metric-like variable or enters a constitutive
matter contact functional. Either choice is a new dynamical commitment and
must be named. This audit does not identify stress with gravity, a metric, or
an elastic medium.

The next legitimate gate was local balance: deposit constituent momentum and
stress on the lattice and test a discrete equation of the form

```text
Delta g_i + div Sigma_i = f_i
```

for free worldlines and selected contact events. **Successor FTD-0514 closes
that gate:** exact face continuity lifts componentwise to momentum balance and
its integrated tensor flux equals `Delta t Sigma`; the selected restricted
contact composes with cancelling internal impulse sources. This is balance
after selection, not a derivation of the collision impulse or a conjugate
field. The multistream
counterexample prevents treating rank-2 closure as exact without a declared
closure approximation or retained constituent histories.

No production code, default, toggle, scenario, force, collision rule, field,
or tolerance changed.

- checks: `5/5 PASS`;
- test SHA256:
  `2BBD1815A4E777EED1B0A50BFD1E4CE84012019F77AF17A378AAC7789838B159`;
- header SHA256:
  `9858353D918D4780B1DD05DFD1907E79FE548DBE1F5D09EE8763EB5F3E6605D3`;
- implementation SHA256:
  `1318BED3CB44B718CE5B4707BB0ACC94342D95EBDBF0A283200B09727CE8C226`;
- locked preregistration-body SHA256:
  `F57B4BBDF7DED9B656E159142BE32D3D79657D6896FDF064342DAC5FEEE56379`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- production state and defaults: unchanged.
