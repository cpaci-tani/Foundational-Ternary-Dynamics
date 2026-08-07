# AUDIT — Native contact active set

**Date:** 2026-07-25  
**Identifier:** `FTD-0525`  
**Status:** `[THEOREM — SITE-STATE CONTACT NONDETERMINATION]` +
`[CONSTRUCTIVE — REMAINDER GAP OBSERVER]` +
`[MEASURED — RAW NO-DISPATCH/DELAY]` +
`[CORRECTED BY FTD-0526 — IDENTICAL CONTACT PHYSICAL QUOTIENT]`  
**Verdict:**
`HARD_CONTACT_REMAINS_SELECTED_PRODUCTION_ACTIVE_SET_IS_LATE`  
**Pre-registration:**
[`PREREG_NATIVE_CONTACT_ACTIVE_SET_v1.md`](../../10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_NATIVE_CONTACT_ACTIVE_SET_v1.md)  
**Run of record:** `engine/results/ftd_0525/windows_msvc_cpu.json`

## 1. Exact state-space discriminator

For adjacent stable anchors `a1` and `a2=a1+d`, set

```text
n=d/|d|,
x_c=(a1+a2)/2,
phi=(x2-x1) dot n.
```

The three configurations

```text
separated: x1=x_c-epsilon n, x2=x_c+epsilon n, phi=+2epsilon,
contact:   x1=x_c,           x2=x_c,           phi=0,
crossed:   x1=x_c+epsilon n, x2=x_c-epsilon n, phi=-2epsilon
```

have the same two occupied site anchors and the same ternary polarity. Site
state alone therefore cannot determine contact or penetration. The existing
continuous remainder does retain the distinction because `x=a+r`; no new
geometric variable is needed merely to evaluate `phi`.

This exact separation matters. It excludes the claim that ternary occupancy
already enforces the FTD-0516 inequality, but it does not exclude an active-set
rule computed from the full existing phase state.

## 2. Frozen production active set

The production movement phase advances the remainder first and dispatches an
occupied-target response only when a component reaches `+/-1` and attempts a
site hop. Midpoint contact instead occurs at chart remainders `d/2` and
`-d/2`. For a mover starting at contact with speed `v` along `n`, production's
later hop time is

```text
t_hop=|d|/(2v),
N_hop=ceil(t_hop),
```

with equality counted on the integer tick. Thus the selected FTD-0516 contact
surface and the frozen movement dispatch are different active sets.

## 3. Actual production campaign

The locked campaign used `L=17`, both polarities, every nonzero Moore
direction, three translations, and speeds `1/8` and `1/4`.

```text
geometry arms                           312
two-body crossing arms                  312
static-target activation arms           312
worst exact-gap residual                 2.6645352591003757e-15
worst stable-chart residual              0
minimum contact hop margin               0.5
minimum crossed hop margin               0.25
minimum measured crossing depth          0.24999999999999867
worst contact raw-state residual         0
worst crossed raw-state residual         0
worst crossed-gap residual               2.6645352591003757e-15
worst two-tick reverse residual           0
worst pretrigger residual                2.2204460492503131e-16
worst activation residual                0
worst activation-tick error              0
minimum/maximum activation delay         2 / 7 ticks
maximum movement-journal events          0
worst field residual                     0
worst translation residual               2.5257573810222311e-15
worst polarity residual                  0
```

In the two-body continuation, tick 1 reaches `phi=0`, tick 2 reaches
`phi<0`, both anchors and velocities remain unchanged, the field and journal
remain empty, and reversing both velocities for two ticks exactly restores the
separated raw state. This is a real production crossing, not an observer-only
counterexample.

In the static-target continuation, no response occurs before the predicted
hop tick. At that tick production performs its documented mover-only axis
flip and remainder reset while the target, field, and journal stay unchanged.
Every observed delay agrees exactly with `ceil(|d|/(2v))` and lies between two
and seven ticks in the registered ensemble.

## 4. Closed and surviving claims

Closed for the frozen raw dispatch:

- ternary site occupancy does not itself define the FTD-0516 contact surface;
- production does not activate collision at `phi=0`;
- the FTD-0516 surface is not an explicit frozen production active set.

FTD-0526 corrects the physical interpretation. For identical same-polarity
carriers, pass-through and hard-contact bounce are the same unlabeled
phase-space and exact-current history. Production respects that quotient
before the hop threshold. At the threshold, commensurate face arms rejoin the
bounce representative exactly; only edge/corner arms become physically
different, by the precise fractional overshoot deleted by remainder reset.

Therefore this audit does not close physical hard contact negative. It closes
only an explicit raw impulse dispatch at `phi=0`. The actual frozen defect is
the later direction-dependent loss of subcell displacement.

Still valid after that correction:

- FTD-0516 derives the restricted Householder impulse conditional on a
  selected unilateral contact action;
- existing remainders are sufficient to evaluate a contact gap locally;
- in the identical class that impulse selects a raw chart representative but
  adds no aggregate physical content;
- this audit does not derive a face-field origin, general collision law, or
  reciprocal production transaction.

The immediate constructive target is narrower than a new contact force:
preserve the residual subcell displacement when an occupied-target chart
rebase occurs. Distinguishable carriers, unequal masses, and nontrivial
scattering still require a genuine collision mechanism.

No production code, default, toggle, scenario, force, collision rule, field,
normalization, ontology, or tolerance changed.

- checks: `6/6 PASS`;
- test SHA256:
  `E220F0918EEA1761043FC9462A27790412C35004A55D85FB96B11E86CA2BE79E`;
- header SHA256:
  `9EDD6C5574385B4D671B057C7CE23D3AEBB19BE0693A3549F1826A6CCADBC090`;
- implementation SHA256:
  `8C893CA4E552208593E240AD436907619A6278E80F31458C64261DDFC846AA14`;
- locked preregistration SHA256:
  `C8976C1C99356998FFE9C23B34CFD0632A761B6EEFB7AB995C9EAA2416464824`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- production state and defaults: unchanged.
