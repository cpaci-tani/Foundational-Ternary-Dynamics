# AUDIT — Contact quotient horizon

**Date:** 2026-07-25  
**Identifier:** `FTD-0526`  
**Status:** `[THEOREM — IDENTICAL CONTACT PERMUTATION QUOTIENT]` +
`[MEASURED — COMMENSURATE LATE REBASE]` +
`[THEOREM + MEASURED — DIAGONAL OVERSHOOT-RESET DEFECT]` +
`[CORRECTION — FTD-0525 RAW-DISPATCH SCOPE]` +
`[OPEN — DISTINGUISHABLE CONTACT/FIELD ORIGIN]`  
**Verdict:**
`CONTACT_IS_GAUGE_LATE_RESET_BREAKS_QUOTIENT_ONLY_BY_OVERSHOOT`  
**Pre-registration:**
[`PREREG_CONTACT_QUOTIENT_HORIZON_v1.md`](../10_eft_program/preregistrations/PREREG_CONTACT_QUOTIENT_HORIZON_v1.md)  
**Run of record:** `engine/results/ftd_0526/windows_msvc_cpu.json`

## 1. The correction

FTD-0525 measured a true raw-state fact: production does not dispatch at the
FTD-0516 surface `phi=0`. That fact does not by itself distinguish physical
pass-through from physical hard contact for identical carriers. FTD-0504 had
already proved that equal-mass same-polarity pass-through and momentum exchange
are the same unlabeled phase-space and exact-current history.

At exact contact, compare two raw representatives:

```text
crossing chart assignment: v1=+v n, v2=-v n,
bounce chart assignment:   v1=-v n, v2=+v n.
```

The first attaches each outgoing worldline to the carrier that approached from
the opposite side. The second exchanges those identical labels at contact.
Their physical position/velocity/polarity multisets are equal even though their
chart-associated remainders and velocities differ.

## 2. Exact pre-horizon quotient

Both representatives were advanced with the actual frozen CPU movement phase.
Before the site-hop horizon

```text
N_hop=ceil(|d|/(2v)),
```

they agree in every registered physical observable:

```text
worst phase-space multiset residual     1.7763568394002505e-15
worst compact-density residual          8.8817841970012523e-16
worst exact face-current residual       4.6629367034256575e-15
minimum raw-label residual              0.14433756729740649
```

The positive raw-label residual alongside zero physical residual proves the
point: production's contact crossing and the hard-contact bounce are distinct
chart representatives of one identical-carrier history. No impulse event is
needed to obtain the physical bounce quotient.

## 3. What the late production response actually does

Define the scalar threshold overshoot

```text
delta=N_hop*v-|d|/2 >= 0.
```

At the horizon, the crossed raw branch attempts to hop into the occupied
adjacent chart. Production flips its velocity and unconditionally sets its
remainder to zero. The already-bounced representative does not hop and retains

```text
r1=-delta*n,
r2=+delta*n.
```

Therefore the two representatives rejoin if and only if `delta=0`. This gives
the exact horizon defect

```text
||Delta x||_infinity=delta ||n||_infinity.
```

The registered campaign separated accordingly:

```text
base direction/polarity/translation/speed arms     312
including default + symmetric movement orders      624
commensurate face arms                              144
positive-overshoot edge/corner arms                 480
worst horizon prediction residual                   7.7368667028565596e-16
minimum positive displacement defect                0.0051814855409233473
maximum positive displacement defect                0.077350269189626175
```

Every commensurate face arm rejoined exactly at the late raw collision and
remained identical for one further tick. Every edge/corner arm first diverged
at the predicted horizon by exactly the deleted overshoot. Default sequential
and fixed-seed symmetric movement orders agreed exactly.

## 4. Why the defect is easy to miss

At the divergence tick, both branches still have:

- identical ternary site occupancy;
- identical total polarity;
- identical total relativistic momentum and matter energy;
- zero field change;
- zero history-journal events.

All corresponding residuals were exactly zero. The defect lives only in the
continuous subcell position. A test based on site state or conserved scalar
totals cannot see it.

This also explains the directional pattern. Face travel at the selected speeds
is commensurate with the integer tick, so reset erases no remainder. Edge and
corner travel involves `sqrt(2)` or `sqrt(3)` and reaches the componentwise hop
threshold after a fractional excess. The reset discards that excess and
creates a cubic-direction-dependent displacement.

## 5. Correct research statement

The correct statement is not “frozen production lacks hard contact.” It is:

> For physically identical carriers, pass-through and hard-contact bounce are
> one permutation quotient, and frozen production respects that quotient until
> its chart-hop response. The response is an exact chart rebase on
> commensurate face arms and a non-injective overshoot deletion on edge/corner
> arms.

Consequences:

1. adding the FTD-0516 impulse at `phi=0` would choose a raw chart section but
   add no aggregate physical content in this identical-carrier class;
2. the immediate repair target is preservation of the residual subcell
   displacement through occupied-target resolution, not force amplification;
   FTD-0527 subsequently constructs that physical repair and proves its raw
   history cost;
3. FTD-0516 may still matter for distinguishable carriers, unequal masses, or
   nontrivial scattering, but none of those mechanisms is derived here;
4. reciprocal face-field mobile matter remains open and is not advanced by
   this contact quotient.

No production code, default, toggle, scenario, force, collision rule, field,
normalization, ontology, or tolerance changed.

- checks: `6/6 PASS`;
- test SHA256:
  `F2E8947A73A096EBF6CAF9AE90BD5EE3FED9EA05D47821A3858A52BB10C73FC8`;
- header SHA256:
  `5ABBE4FB4265D803E2E357992841DF5776B5594DA989A6F3016496C5094BD224`;
- implementation SHA256:
  `FF59601BB9392B2E6B42BFB977DD0B230F3B7739C806FE86F0918D4754DAAB9F`;
- locked preregistration SHA256:
  `28EFC586766D76EBE40D96E3252B9B4A311986FFD4514476C72ED37CD622B4B9`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- production state and defaults: unchanged.
