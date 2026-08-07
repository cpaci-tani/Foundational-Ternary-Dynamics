# PRE-REGISTRATION — Hard-contact corner action

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0516`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Scope:** observer-only test of whether the FTD-0512 restricted collision
impulse is the variational corner condition of one selected relativistic
hard-contact matter action. No production state, default, toggle, scenario,
force branch, field, collision rule, or ontology change.

## 1. Selected action and contact manifold

For each equal-mass carrier adopt the existing dispersion's free Lagrangian

```text
L(v) = -m sqrt(1-|v|^2/c^2),
H(p) = sqrt(m^2+c^2|p|^2),
v(p) = c^2 p/H(p).
```

At one already detected chart-boundary contact, select the unilateral relative
gap

```text
phi(x1,x2) = (x2-x1) dot n >= 0,
```

where `n` is the existing FTD-0507 chart normal from carrier 1 toward carrier
2. `phi` is a selected hard-contact geometry, not a native consequence of the
electromagnetic face field.

The piecewise free action is varied with its corner constrained to `phi=0`.
The registered Weierstrass-Erdmann/KKT corner conditions are

```text
p1+ - p1- = -lambda n,
p2+ - p2- = +lambda n,
H1+ + H2+ = H1- + H2-,
lambda >= 0,
lambda phi = 0.
```

Because the constraint depends only on relative position, common translation
gives total-momentum continuity. Tangential corner variation gives zero
tangential impulse. Collision-time variation gives total-energy continuity.
No persistent multiplier is introduced: `lambda` is an algebraically eliminated
event variable.

## 2. Registered restricted inverse

Use the same equal-mass, axial-relative, zero-COM fixture as FTD-0512. Let

```text
q = (p1- - p2-)/2,
q_n = q dot n > 0.
```

Substitution of the normal impulse into energy continuity must reduce to the
two algebraic branches

```text
lambda(lambda-2q_n)=0.
```

The zero branch leaves the carriers incoming. The unilateral outgoing gate

```text
u- = (v2- - v1-) dot n < 0,
u+ = (v2+ - v1+) dot n > 0
```

must select uniquely

```text
lambda = 2q_n,
q+ = q - 2(q dot n)n.
```

This is exactly the FTD-0512 Householder reflection if the derivation closes.

## 3. Registered fixtures and gates

Use `L=17`, rest energy `0.511`, `c=1/sqrt(3)`, both polarities, three
translations, every nonzero Moore direction, and speeds `1/8` and `1/4`:

```text
26 x 2 polarities x 3 translations x 2 speeds = 312 arms.
```

For every arm require at `1e-12`:

1. exact equality with the FTD-0512 incoming/outgoing momenta and multiplier;
2. equal-and-opposite normal corner impulses and zero tangential jump;
3. total momentum and total relativistic energy continuity;
4. active-contact KKT signs, exact complementarity, incoming approach, and
   outgoing separation;
5. analytic collision-point and collision-time corner derivatives equal the
   momentum-jump and energy-jump conditions;
6. exact composition with the FTD-0514 local face momentum balance;
7. time reversal maps the outgoing corner back to the incoming corner with the
   same nonnegative event multiplier;
8. translated, polarity-mirrored, and signed-cubic copies satisfy the same
   scalar/vector/tensor relations;
9. a positive-gap inactive control has exactly zero multiplier and impulse;
10. invalid inputs fail closed.

## 4. Locked verdicts

- If every gate passes:
  `SELECTED_HARD_CONTACT_ACTION_DERIVES_RESTRICTED_IMPULSE_NO_FIELD_ORIGIN`.
- If conservation holds but corner stationarity does not:
  `CONSERVATIVE_REFLECTION_NOT_A_CORNER_ACTION`.
- If the multiplier cannot be eliminated without retained state:
  `HARD_CONTACT_REQUIRES_ADDITIONAL_EVENT_STATE`.
- If the result disagrees with FTD-0512 or FTD-0514:
  `HARD_CONTACT_CORNER_ACTION_CLOSED_NEGATIVE`.

A pass converts the independent FTD-0512 central-contact/elasticity premises
into consequences of one explicitly selected matter-contact action. It does
not make the contact constraint native, couple the action to face E/B, explain
electromagnetic force, handle general scattering, or license a production
toggle.

## 5. Execution record

Executed 2026-07-25 with pinned MSVC `14.44.35207`, Release, CPU observer.
The locked preregistration SHA256 was
`9A25729DA28971BCA6E6A7A87C2EA8236E96ED93EF0AEFFAF5B49F5E86E28725`.

All `6/6` checks passed over 312 collision arms and 144 explicit signed-cubic
transforms. The largest residual was `6.67e-16`; the positive-gap inactive
control returned exactly zero and a penetrating control failed closed. The
locked pass verdict applies:

```text
SELECTED_HARD_CONTACT_ACTION_DERIVES_RESTRICTED_IMPULSE_NO_FIELD_ORIGIN
```

Canonical result:
[`AUDIT_HARD_CONTACT_CORNER_ACTION.md`](../../../07_assessment/framework_boundary_imports_consumption/AUDIT_HARD_CONTACT_CORNER_ACTION.md).
