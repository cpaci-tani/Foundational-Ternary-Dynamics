# AUDIT — Constituent-relative collision selector

**Date:** 2026-07-25  
**Identifier:** `FTD-0512`  
**Status:** `[DERIVED — UNIQUE WITHIN SELECTED CENTRAL-ELASTIC CLASS]` +
`[CONSTRUCTIVE — EXISTING PHASE SPACE SUPPORTS REFLECTION]` +
`[THEOREM — AXIAL AGGREGATE FACE-ACTION KERNEL]` +
`[CLOSED NEGATIVE — AGGREGATE FACE ACTION AS COLLISION ORIGIN]`  
**Verdict:** `SELECTED_REFLECTION_EXISTS_FACE_ACTION_CANNOT_DERIVE_IT`  
**Pre-registration:**
[`PREREG_CONSTITUENT_RELATIVE_COLLISION_SELECTOR_v1.md`](../../10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_CONSTITUENT_RELATIVE_COLLISION_SELECTOR_v1.md)  
**Run of record:** `engine/results/ftd_0512/windows_msvc_cpu.json`

## 1. Existing phase space is sufficient to write the restricted map

FTD-0507 leaves two manifested records on distinct stable charts at the same
effective position. Their anchor difference supplies a local oriented normal

```text
n=(a2-a1)/|a2-a1|.
```

No new temporal phase or particle identity is needed to form `n`, total
momentum `P=p1+p2`, and relative momentum `q=(p1-p2)/2`. In the registered
zero-COM-normal sector (`P·n=0`), constrain the impulse to be central and
equal-and-opposite:

```text
p1'=p1-lambda n,
p2'=p2+lambda n.
```

Because `P·n=0`, preserving each equal-mass dispersion energy reduces to

```text
|q-lambda n|^2=|q|^2
lambda(lambda-2 q·n)=0.
```

There are exactly two algebraic solutions. `lambda=0` is free continuation;
the nontrivial outgoing solution is

```text
lambda=2 q·n,
q'=q-2(q·n)n.
```

This is a Householder reflection. It preserves total momentum and both
relativistic matter energies, reverses only the normal relative component,
is an involution, commutes with time reversal, and is covariant under the
signed cubic group. The outgoing condition selects it uniquely *within the
central elastic contact class*.

The italicized restriction is load-bearing. Central contact, elasticity, and
nonpenetration are selected interaction premises; none follows from the five
postulates or from aggregate face coupling.

## 2. Exact observer result

The locked campaign exercised every nonzero Moore direction, both polarities,
three translations, and speeds `1/8` and `1/4`:

```text
26 x 2 x 3 x 2 = 312 arms.
```

All registered map gates passed:

```text
worst conservation residual          2.22e-16
worst collision-solution residual    1.94e-16
worst involution/reversal residual   3.89e-16
worst translation residual           1.78e-15
worst continuity residual            4.16e-17
worst causal residual                0
polarity-mirror residual             0.
```

The result is a valid observer construction. It is not a production law and
does not repair the production same-sign reset branch audited in FTD-0506.

## 3. The aggregate face action cannot derive the axial impulse

For a face-normal collision, compare two histories over the same observation
slab:

```text
H0: two coincident static same-sign carriers,
H1: the same pair separating symmetrically along the face normal.
```

The one-dimensional trilinear hat is affine within its cell. Therefore the
two separating shapes sum exactly to twice the midpoint shape, while their
oriented currents cancel. The complete aggregate input is identical:

```text
(rho_before,rho_after,J_face)_H0
  = (rho_before,rho_after,J_face)_H1.
```

The 72 registered face arms measured:

```text
maximum full H0/H1 signature residual       0 exactly
maximum aggregate H1 current L1             0 exactly
minimum summed constituent-current L1       0.5
minimum H1-H0 matter-energy gap              0.024829530331357486.
```

This is an exact counterexample, not a low-resolution miss. Any deterministic
interaction functional whose matter input is only aggregate trilinear
`(rho,J_face)` receives the same input for H0 and H1. It cannot select one
history over the other or account for their nonzero energy difference.
Consequently the aggregate face action cannot be the origin of the selected
collision impulse.

Edge and corner tensor-product shapes do retain even cross terms: their
aggregate current norm is approximately `0.125`. That does not rescue the
universal action because the exact face-normal kernel already lies inside its
declared domain.

## 4. Consequence for the face-flux mobile-matter plan

The obstruction is no longer missing storage and is not evidence for a hidden
temporal phase. Existing constituent phase space can support a reciprocal
restricted collision map. What is absent is a native interaction functional
that selects the central-elastic premise and carries the constituent-relative
mode before aggregation.

Thus the collision extension of the FTD-0479 common-action requirement is
closed for a functional depending only on aggregate trilinear face
density/current. A future candidate must do one of the following openly:

1. add a selected constituent-level contact term using the already-associated
   chart normal and momenta;
2. enrich the field coupling with a higher moment/internal channel that does
   not quotient out axial counterflow; or
3. exclude same-sign contact events from the ontology by a separately selected
   finite-range rule.

Option 1 adds dynamics but not a hidden state variable. Option 2 changes the
coupling representation. Option 3 changes the interaction range. None is
licensed by this audit, and conservation alone still does not select general
three-dimensional scattering angles, unequal-mass collisions, or reactions.

**Successor FTD-0516 realizes Option 1 at observer level.** A selected
unilateral chart-normal contact manifold and the existing relativistic free
action derive the restricted Householder impulse through exact corner/KKT
conditions. This removes central contact, elasticity, and nonpenetration as
three independent premises, but the hard-contact inequality remains selected
and has no aggregate face-field origin. The axial kernel theorem above is
unchanged.

No production code, default, toggle, scenario, force, collision rule, or
tolerance changed.

- checks: `5/5 PASS`;
- test SHA256:
  `F973E0389807ABD92E8517F5041458AD606C78D9672CE1861ABD0E07984459F5`;
- header SHA256:
  `5CB254B04DA58BC330D049556433C2F78C2E62EC35A665D8908ABD38B313E15A`;
- implementation SHA256:
  `9B97E2F7DB5FF27D3D336B915796650851D1851ECCAC6A3209C1FD0BC549D8B7`;
- locked preregistration-body SHA256:
  `2DA607D150683A082F98730BABECEDC11B4B1F713996007AE6EDA7DFD44179E0`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- production state and defaults: unchanged.
