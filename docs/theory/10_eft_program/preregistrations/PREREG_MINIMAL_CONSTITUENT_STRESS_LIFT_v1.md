# PRE-REGISTRATION — Minimal constituent-stress lift

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0513`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Scope:** observer-only test of the lowest cubic-covariant constituent moment
that retains the FTD-0512 axial counterflow mode. No production state,
default, toggle, scenario, force, collision law, field normalization, or
ontology change.

## 1. Question

FTD-0512 gives an exact face-normal kernel: two opposite constituent currents
cancel in aggregate even though the pair carries kinetic energy and a collision
axis. This campaign asks:

1. what is the lowest polynomial moment that retains both magnitude and axis;
2. whether it can be computed from already-associated constituent phase space;
3. whether it is complete for the restricted two-stream collision class; and
4. whether it is complete for general multistream matter.

## 2. Frozen stress moment

For equal rest energy `m`, production cone speed `c`, momentum `p_i`, and

```text
E_i=sqrt(m^2+c^2|p_i|^2),
v_i=c^2 p_i/E_i,
```

define the symmetric spatial kinetic-stress moment

```text
Sigma = sum_i p_i tensor v_i
      = c^2 sum_i (p_i tensor p_i)/E_i.
```

This is an observer derived from existing carrier momenta. It is not a new
primitive field and is not the electromagnetic stress tensor.

For the registered zero-COM two-stream pair `p_1=+p n`, `p_2=-p n`,

```text
Sigma=tau n tensor n,
tau=2 c^2 p^2/E=2(E-m^2/E).
```

The registered inverse is

```text
t=tau/2,
E=(t+sqrt(t^2+4m^2))/2,
p=sqrt(E^2-m^2)/c.
```

Thus a rank-one positive tensor recovers the unoriented collision axis
`{+n,-n}`, momentum magnitude, and pair kinetic energy.

## 3. Minimality statement under test

The claim is deliberately scoped to additive polynomial moments and exact
`O_h` covariance:

- degree 0 records carrier count only;
- every covariant polar-vector moment is inversion-odd and vanishes for the
  inversion-even multiset `{+p,-p}`;
- every additive degree-1 momentum moment is proportional to total momentum
  and also vanishes;
- a scalar even moment can recover speed but not the collision axis;
- the symmetric degree-2 tensor is the first moment carrying both the `A1g`
  trace and the `Eg + T2g` quadrupole content.

No claim is made against non-polynomial encodings, constituent lists, or
arbitrarily high moments. The test must not promote this scoped result to a
universal information-minimality theorem.

## 4. Registered fixtures and gates

Use the FTD-0512 fixtures: `L=17`, rest energy `0.511`,
`c=1/sqrt(3)`, every nonzero Moore direction, both polarities, three integer
translations, and speeds `1/8` and `1/4`:

```text
26 x 2 x 3 x 2 = 312 arms.
```

For every arm require below `1e-12`:

1. symmetry and positive-semidefiniteness of `Sigma`;
2. exact covariance `Sigma(Rp)=R Sigma(p) R^T` for signed cubic maps;
3. translation and polarity independence;
4. rank-one spectrum for the opposite pair;
5. recovery of the unoriented chart axis, momentum magnitude, single-carrier
   energy, and pair kinetic-energy gap;
6. zero total momentum/vector current but strictly positive stress trace.

The axis comparison uses projectors `n tensor n`, so the physical `n <-> -n`
ambiguity is not falsely counted as a defect.

## 5. General multistream incompleteness gate

At one effective point compare two distinct four-stream momentum multisets:

```text
A={+p ex,-p ex,+p ey,-p ey},
B={+p d1,-p d1,+p d2,-p d2},
d1=(ex+ey)/sqrt(2), d2=(ex-ey)/sqrt(2).
```

Require exact equality of carrier count, total momentum, total energy, and
`Sigma`, while the momentum multisets remain separated and their fourth
moments differ. This proves that rank-2 stress is sufficient for the
registered rank-one two-stream mode but is not a complete general matter
state.

## 6. Locked verdicts

- If all two-stream gates and the multistream counterexample pass:
  `RANK2_STRESS_IS_MINIMAL_FOR_TWO_STREAM_KERNEL_NOT_COMPLETE`.
- If stress retains the mode but the registered inverse is non-unique:
  `STRESS_OBSERVES_COUNTERFLOW_BUT_DOES_NOT_RECONSTRUCT_IT`.
- If stress also fails to distinguish the two-stream mode:
  `RANK2_STRESS_LIFT_CLOSED_NEGATIVE`.

No verdict licenses adding six stress components to production. A dynamical
extension would still need a separately preregistered local conservation law,
field energy, conjugate variable, and common interaction functional.

## 7. Execution record

The locked body above had SHA256
`F57B4BBDF7DED9B656E159142BE32D3D79657D6896FDF064342DAC5FEEE56379`
before this execution section and status transition were appended. No gate,
fixture, tolerance, or verdict cell changed after the lock.

- test SHA256:
  `2BBD1815A4E777EED1B0A50BFD1E4CE84012019F77AF17A378AAC7789838B159`;
- header SHA256:
  `9858353D918D4780B1DD05DFD1907E79FE548DBE1F5D09EE8763EB5F3E6605D3`;
- implementation SHA256:
  `1318BED3CB44B718CE5B4707BB0ACC94342D95EBDBF0A283200B09727CE8C226`;
- result: `5/5 PASS`;
- verdict:
  `RANK2_STRESS_IS_MINIMAL_FOR_TWO_STREAM_KERNEL_NOT_COMPLETE`.
