# AUDIT — Dressing-fiber ledger

**Date:** 2026-07-25  
**Identifier:** `FTD-0495`  
**Status:** `[CONSTRUCTIVE — MINIMAL SCALAR HISTORY LEDGER]` +
`[THEOREM — NONZERO FIBER HOLONOMY]` +
`[CLOSED NEGATIVE — CENTERED ORDINARY COMMON ACTION]`  
**Verdict:** `SCALAR_LEDGER_CLOSES_BOOKKEEPING_NOT_COMMON_ACTION`  
**Pre-registration:**
[`PREREG_DRESSING_FIBER_LEDGER_v1.md`](../../10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_DRESSING_FIBER_LEDGER_v1.md)  
**Run of record:** `engine/results/ftd_0495/windows_msvc_cpu.json`

## 1. One real history coordinate closes the scalar energy ledger

Let `D` be carried by the manifested history and update it by the exact
FTD-0493 omission:

```text
D_1-D_0=W_cusp=W_field-W_center.
```

Then

```text
Delta E_field  =-W_field,
Delta E_matter =+W_center,
Delta D        =+W_cusp,
```

and their sum vanishes identically.  The registered source-free arm has

```text
W_field=-0.0087600000000000022,
W_center=0,
Delta D=-0.0087600000000000022,
```

with zero energy residual.  Subtracting the same work on the reversed event
recovers `D_0` exactly.  Polarity/field mirroring and translated histories
close to `3.47e-18`.

FTD-0494 proves that zero additional state variables cannot encode this work
as a local state function.  This construction shows that one real scalar is
sufficient for bookkeeping.  It is therefore minimal in scalar memory
dimension for that limited purpose.

## 2. The coordinate is a fiber, not a hidden local energy

Transporting `D` along the two FTD-0494 paths gives

```text
D_xy-D_0=+0.219,
D_yx-D_0=-0.219,
D_xy-D_yx=0.438.
```

Following `xy` and then reverse-`yx` returns the particle to its starting
site but changes the fiber by `0.438`.  Reversing that entire oriented loop
recovers the initial fiber exactly.  Adding an arbitrary constant to `D_0`
does not change work or holonomy.

Thus `D` is a coordinate on a history fiber with a non-flat connection.  It
cannot be eliminated in favor of `(site,remainder,E,q)`.  The new coordinate
also carries a freely selected additive energy zero.

## 3. An ordinary action requires a multiplier

The standard discrete constraint is

```text
S_c=lambda[W_cusp-(D_1-D_0)].
```

For two adjacent steps, variation of their shared `D_k` gives

```text
partial S/partial D_k=lambda_k-lambda_(k-1)=0.
```

The multiplier is therefore a conserved conjugate quantity.  Its value is not
fixed by the constraint; reproducing the desired coefficient requires the
additional selection `lambda=1`.

At that value, position variation contributes

```text
partial S_c/partial r=grad W_cusp.
```

The registered multiplier and analytic gradient residuals are zero; an
independent finite difference of position, `D_0`, and `D_1` closes to
`1.39e-17`.  By FTD-0494 this
gradient exactly restores the ordinary one-sided field trace, and hence the
FTD-0491 eightfold branch ambiguity.  The scalar ledger does not allow an
ordinary action to retain centered motion.

## 4. Precise status of the extension

There are now two sharply separated statements:

- **Bookkeeping:** one real `D` is a local-along-history, reversible, cubic-
  covariant accumulator that closes the scalar energy identity.
- **Dynamics:** keeping centered motion while updating `D` is a nonholonomic
  procedural rule, not the Euler--Lagrange map of the same ordinary action.
  Enforcing the update variationally introduces a conserved multiplier and
  restores the rejected branch force.

This does not complete mobile matter.  It prices a possible ontology change:
at least one real history coordinate, and for a standard variational theory a
conjugate multiplier whose normalization is selected.  Neither variable is
present in the frozen ontology, and neither selects an outgoing branch.

## 5. Reproducibility

- checks: `12/12 PASS`;
- test SHA256:
  `8FD6A19353F0820ADB426B935D8F216C1BBE17E0D66F283702AA80F3F747F282`;
- header SHA256:
  `AA8AB2F1195259A58893EBA8C0799F83105E433F5B6C659024E5C0B8FB886867`;
- implementation SHA256:
  `91D55FC7268860F15923E886F1C5736644B87529D14F1DD41A7C08DE60779612`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- production state and defaults: unchanged.
