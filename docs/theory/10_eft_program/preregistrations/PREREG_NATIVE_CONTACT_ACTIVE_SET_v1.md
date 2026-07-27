# PRE-REGISTRATION — Native contact active set

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0525`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Reservation note:** `FTD-0525` is intentionally reserved for this engine
audit while a parallel derivation sequence consumes `FTD-0519` onward.  
**Scope:** observer-only comparison of the FTD-0516 selected hard-contact
surface with the frozen ternary/chart representation and actual production
movement dispatch. No production state, default, toggle, scenario, force,
collision rule, field, or ontology change.

## 1. Exact configurations

For two adjacent stable charts with anchors `a1`, `a2=a1+d`, define

```text
n=d/|d|,
x_c=(a1+a2)/2,
phi=(x2-x1) dot n.
```

For offset `epsilon>0`, compare

```text
separated: x1=x_c-epsilon n, x2=x_c+epsilon n, phi=+2epsilon;
contact:   x1=x_c,           x2=x_c,           phi=0;
crossed:   x1=x_c+epsilon n, x2=x_c-epsilon n, phi=-2epsilon.
```

All three use the same two occupied ternary anchors and polarity. Their
remainders are determined by `r_i=x_i-a_i`. The registered structural question
is whether site-valued ternary manifestation itself excludes `phi<0`, or
whether only the continuous remainders retain that distinction.

## 2. Frozen production active set

Production movement first performs

```text
r <- r + v dt
```

and dispatches a target-site collision only if at least one remainder component
reaches `+/-1`, generating a Moore hop. The FTD-0516 contact surface for the
midpoint charts instead has nonzero components `r1=d/2`, `r2=-d/2`.

Starting at contact with mover velocity `v n`, the exact continuous time to
the production hop threshold is

```text
t_hop=|d|/(2v),
N_hop=ceil(t_hop)
```

ticks, with exact-integer cases counted at equality. A positive `N_hop` is an
active-set delay, not automatically a defect; the decisive test is whether
production permits a reversible state with `phi<0` before dispatch.

## 3. Registered fixtures

Use `L=17`, both polarities, three translations, all 26 nonzero Moore
directions, and speeds `1/8` and `1/4`:

```text
26 x 2 polarities x 3 translations x 2 speeds = 312 arms.
```

All algebraic and production residuals use `1e-12`.

## 4. Structural gates

For every arm require:

1. the two stable anchors exist and store both same-sign carriers with zero
   chart-capacity defect;
2. separated, contact, and crossed configurations have the same site-valued
   ternary occupancy and remain inside the same stable remainder charts;
3. their remainder-derived gaps are exactly `+2epsilon`, `0`, and
   `-2epsilon`;
4. site state alone therefore cannot determine the sign of `phi`, while the
   full `(site,remainder)` phase state can;
5. the contact and first crossed configurations are below the production
   `+/-1` hop threshold;
6. the predicted hop-delay tick is positive and follows the registered formula.

## 5. Actual production gates

Two actual CPU continuations are registered for every arm.

**Two-body crossing:** initialize at the separated configuration with velocities
`v1=+v n`, `v2=-v n`. Require:

1. tick 1 reaches `phi=0` and tick 2 reaches `phi=-2v`;
2. both anchors and velocities remain unchanged through the crossed state;
3. no movement journal event, field change, remainder reset, or collision
   dispatch occurs;
4. reversing both velocities for two ticks recovers the separated raw state
   below `1e-12`.

**Static-target activation:** initialize both carriers at contact, keep carrier
2 static, and move carrier 1 along `+n`. Require:

1. no collision response before the predicted `N_hop`;
2. at `N_hop`, the mover undergoes the documented production axis flip and
   remainder reset while the target stays unchanged;
3. the measured delay equals the formula exactly;
4. the field and history journal remain unchanged.

## 6. Locked verdicts

- If production rejects `phi<0` at contact and the active sets coincide:
  `HARD_CONTACT_IS_FROZEN_NATIVE_ACTIVE_SET`.
- If the crossed state is valid and production dispatches only later at the
  chart-hop threshold:
  `HARD_CONTACT_REMAINS_SELECTED_PRODUCTION_ACTIVE_SET_IS_LATE`.
- If site state excludes crossing but remainders disagree:
  `TERNARY_EXCLUSION_EXISTS_WITH_CHART_REPRESENTATION_CONFLICT`.
- If transformed arms disagree or actual dispatch is seed/order dependent:
  `NATIVE_CONTACT_ACTIVE_SET_UNRESOLVED`.

The expected negative verdict would prove only that the FTD-0516 inequality is
not the frozen production active set. It would not prove that hard contact is
physically wrong, forbid a new selected production rule, or close all possible
native exclusion mechanisms outside the frozen tick.

## 7. Execution record

Executed 2026-07-25 with pinned MSVC `14.44.35207`, Release, CPU observer.
The locked preregistration SHA256 before execution/status annotation was
`C8976C1C99356998FFE9C23B34CFD0632A761B6EEFB7AB995C9EAA2416464824`.

All `6/6` checks passed over 312 exact-geometry arms, 312 actual two-body
crossing continuations, and 312 actual static-target activation continuations.
Production reached and crossed `phi=0` with no event, field change, or state
reset, reversed the crossing exactly, and activated only at the later site-hop
threshold after two to seven ticks. The locked negative verdict applies:

```text
HARD_CONTACT_REMAINS_SELECTED_PRODUCTION_ACTIVE_SET_IS_LATE
```

Canonical result:
[`AUDIT_NATIVE_CONTACT_ACTIVE_SET.md`](../../07_assessment/AUDIT_NATIVE_CONTACT_ACTIVE_SET.md).

**Successor correction:** FTD-0526 preserves this locked verdict only at its
raw-dispatch scope. For identical carriers, pass-through and bounce are the
same physical quotient before the hop; commensurate face arms rejoin exactly.
Only edge/corner overshoot deletion at the later reset creates a physical
difference. See
[`AUDIT_CONTACT_QUOTIENT_HORIZON.md`](../../07_assessment/AUDIT_CONTACT_QUOTIENT_HORIZON.md).
