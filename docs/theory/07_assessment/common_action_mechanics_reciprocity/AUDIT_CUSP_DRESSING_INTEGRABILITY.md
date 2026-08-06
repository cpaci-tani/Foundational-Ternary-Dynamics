# AUDIT — Cusp-dressing integrability

**Date:** 2026-07-25  
**Identifier:** `FTD-0494`  
**Status:** `[THEOREM — UNIQUE CELLWISE CUSP PRIMITIVE]` +
`[THEOREM — GLOBAL GLUING OBSTRUCTION]` +
`[CLOSED NEGATIVE — FROZEN LOCAL DRESSING ENERGY]`  
**Verdict:** `CELLWISE_PRIMITIVE_GLOBAL_MEMORY_OBSTRUCTION`  
**Pre-registration:**
[`PREREG_CUSP_DRESSING_INTEGRABILITY_v1.md`](../10_eft_program/preregistrations/PREREG_CUSP_DRESSING_INTEGRABILITY_v1.md)  
**Run of record:** `engine/results/ftd_0494/windows_msvc_cpu.json`

## 1. The omitted work has a unique cellwise primitive

FTD-0493 gives, for every segment from a lattice knot into one incident cell,

```text
W_cusp=(gq/2) sum_a J_a(n)|d_a|.
```

Choosing zero energy at the knot therefore fixes the value at every admitted
subcell point.  The only cellwise state function reproducing all those work
values is

```text
U_n(r)=(gq/2) sum_a J_a(n)|r_a|.
```

This is not a fitted energy.  It is forced by the exact work identity.  The
implemented primitive agrees with independent Whitney-current work to
`5.21e-18`, reverses exactly, is local, and is covariant under all 48 signed
cubic maps.  Mirroring both polarity and field leaves it invariant.

## 2. Production hop equivalence forces a lattice one-form

Production represents the same physical threshold point in two ways:

```text
(n,e_a) ~ (n+e_a,0),
```

because the integer anchor advances and `remainder` is reduced by one.  Bare
cellwise energies differ by

```text
U_n(e_a)-U_(n+e_a)(0)=(gq/2)J_a(n).
```

For the locked fixture the largest one-axis mismatch is `0.1095`.  Adding an
arbitrary site offset `C(n)` can glue the representations only if

```text
C(n+e_a)-C(n)=omega_a(n),
omega_a(n)=(gq/2)J_a(n).
```

Thus the requested dressing energy exists globally only when the lattice
one-form `omega` is exact.  Every elementary plaquette must satisfy

```text
H_ab(n)=omega_a(n)+omega_b(n+e_a)
        -omega_b(n)-omega_a(n+e_b)=0.
```

This condition follows from representation invariance alone.  It is
independent of whether `C` is written as a linear or nonlinear local function
of the neighboring field.

## 3. Exact Gauss-free counterexample

On an even periodic lattice define

```text
h(x,y)=j(-1)^(x+y),
E_x=h/2, E_y=-h/2, E_z=0.
```

Then

```text
J_x=h, J_y=-h, J_z=0,
div E=J_x+J_y+J_z=0
```

at every site.  It is therefore a source-free allowed matched-face snapshot,
not a charged-self-field fixture.  Nevertheless the two offset paths around
one plaquette give

```text
C_xy-C_0=+0.219,
C_yx-C_0=-0.219,
H_xy=0.438.
```

The measured divergence and holonomy-formula residual are both exactly zero
at printed precision.  Since the obstruction occurs on one contractible
plaquette, neither an uncontained domain nor removal of periodic boundaries
repairs it.  A global scalar offset does not exist for the allowed frozen
field space.

## 4. A variational repair restores the old branch problem

Away from a coordinate cusp,

```text
partial U_n/partial r_a=(gq/2)J_a sign(r_a).
```

Adding this gradient to the centered interaction derivative gives

```text
gq E_center,a + partial U_n/partial r_a
=gq[E_center,a+sign(r_a)J_a/2],
```

which is exactly the ordinary one-sided branch derivative.  The locked
residual is zero.  Varying the cusp energy honestly therefore returns to the
eight exact branches of FTD-0491; it does not select one of them.

The field variation is also nonzero:

```text
||partial U/partial E||_2
=|gq| sqrt(sum_a |r_a|^2/2).
```

It is `0.21771875206329844` in the registered arm and agrees with an
independent six-face finite difference exactly at printed precision.  The
variation remains nonzero for a moving subcell particle even at `J=0`.
Appending `U` as an energy while omitting these position and field variations
is not a common action.

## 5. Exact trilemma

The centered-trace repair has only three outcomes under the frozen analysis:

1. **Vary `U` as part of the action:** the ordinary one-sided branch and its
   FTD-0491 eightfold nonuniqueness return.
2. **Book `U` as energy but do not vary it:** energy bookkeeping is detached
   from the equations of motion and the common-action requirement fails.
3. **Accumulate `W_cusp` along the realized history:** energy can be closed,
   but the accumulated value is a new history variable and still does not
   select the outgoing branch.

Therefore no local single-valued dressing energy of `(site,remainder,E,q)`
completes reciprocal mobile matter.  The additional-primitive-variable clause
in the original FTD-0479 plan has fired.  FTD-0481--0483 remain closed for the
frozen ontology.

## 6. Reproducibility

- checks: `12/12 PASS`;
- test SHA256:
  `8D46BF13B48B7DE86595DA3A530FD653B9415322B30B929A82C1AFF8F0462BBE`;
- header SHA256:
  `A70EC9EB4308266E32F1E3C4BDB3BE79523811CC2E6A8FFCB4CCF15261C1ABF9`;
- implementation SHA256:
  `DA1402C5952B8CFCAB02C54A4AD9142A98373352C53CA94F0E92253BC99B0562`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- production state and defaults: unchanged.
