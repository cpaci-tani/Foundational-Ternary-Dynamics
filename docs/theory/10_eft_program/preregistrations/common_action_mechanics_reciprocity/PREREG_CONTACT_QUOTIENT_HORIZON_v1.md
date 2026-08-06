# PRE-REGISTRATION — Contact quotient horizon

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0526`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Scope:** observer-only comparison of two actual frozen-production
representatives of one FTD-0504 identical-carrier contact quotient. No
production state, default, toggle, scenario, force, collision rule, field, or
ontology change.

## 1. Registered correction question

FTD-0504 already proves that equal-mass, same-polarity pass-through and
momentum exchange at an interior contact are the same unlabeled phase-space
and exact-current history. FTD-0525 proves that frozen raw production crosses
the FTD-0516 contact surface and responds only later at a chart-hop threshold.

The registered question is whether FTD-0525's raw no-dispatch result also
distinguishes physical contact dynamics, or whether pass-through and bounce
remain quotient-equivalent until the later chart rule makes their futures
different.

## 2. Actual production branches

For the FTD-0525 adjacent charts at exact contact, initialize two CPU bridges
with identical site state and remainders.

```text
crossing representative: v1=+v n, v2=-v n;
bounce representative:   v1=-v n, v2=+v n.
```

At contact these are related by permutation of two physically identical
carriers. Both branches use actual production movement. No manually applied
impulse or post-tick state edit is allowed.

The registered raw chart horizon is

```text
N_hop=ceil(|d|/(2v)),
```

with the same equality convention as FTD-0525.

## 3. Fixtures

Use `L=17`, both polarities, three translations, every nonzero Moore
direction, and speeds `1/8` and `1/4`:

```text
26 x 2 polarities x 3 translations x 2 speeds = 312 arms.
```

Repeat the horizon comparison under default sequential movement and under the
existing symmetric movement order with fixed seed `13`. All residual gates use
`1e-12`.

## 4. Locked gates

For every arm require:

1. at contact and after every tick `t<N_hop`, the two branches have the same
   unlabeled multiset of effective position, velocity, and polarity;
2. their compact aggregate polarity densities agree at every such snapshot;
3. their exact aggregate face currents agree on every pre-horizon tick;
4. their raw chart-associated remainders/velocities differ after the first
   tick, proving that quotient equality is not raw-state equality;
5. no field or history-journal event occurs before the horizon;
6. at `N_hop`, the crossing representative triggers the documented raw
   occupied-target response while the bounce representative continues;
7. define the exact scalar overshoot
   `delta=N_hop*v-|d|/2`; the crossing branch resets its remainders to zero,
   while the bounce branch retains remainders `-/+ delta*n`;
8. when `delta=0`, both raw and physical representatives rejoin exactly at the
   horizon and remain equal for one additional tick;
9. when `delta>0`, the first physical phase-space divergence occurs exactly at
   `N_hop` and equals the registered overshoot geometry;
10. ternary site occupancy, total polarity, total momentum, and total matter
    energy still agree at every horizon arm;
11. default and symmetric movement orders give the same horizon and result;
12. translated and polarity-mirrored copies preserve all scalar gates.

## 5. Locked verdicts

- If equality persists before the horizon, commensurate arms rejoin exactly,
  and only positive-overshoot arms diverge by the predicted reset defect:
  `CONTACT_IS_GAUGE_LATE_RESET_BREAKS_QUOTIENT_ONLY_BY_OVERSHOOT`.
- If all arms fail physically at the horizon:
  `CONTACT_IS_PERMUTATION_GAUGE_PRODUCTION_BREAKS_QUOTIENT_AT_HOP`.
- If the branches differ physically before the horizon:
  `RAW_CONTACT_CROSSING_IS_PHYSICALLY_DISTINCT`.
- If all arms remain quotient-equivalent through the horizon:
  `LATE_PRODUCTION_RESPONSE_IS_ALSO_QUOTIENT_GAUGE`.
- If order or transformed copies disagree:
  `CONTACT_QUOTIENT_HORIZON_UNRESOLVED`.

A pass under the first verdict corrects FTD-0525's scope. It means the absence
of a raw impulse at `phi=0` does not itself refute physical hard-contact
behavior for identical carriers; any later physical defect comes specifically
from deletion of a noncommensurate subcell overshoot, not from pass-through as
such. It does not solve distinguishable collisions or license a production
repair.

## 6. Execution record

Executed 2026-07-25 with pinned MSVC `14.44.35207`, Release, CPU observer.
The locked preregistration SHA256 before execution/status annotation was
`28EFC586766D76EBE40D96E3252B9B4A311986FFD4514476C72ED37CD622B4B9`.

All `6/6` checks passed over 312 base arms and both registered production
orders (`624` order arms). The two representatives agree physically before
the hop horizon. All 144 commensurate face arms rejoin exactly. All 480
edge/corner arms first diverge at the horizon by the exact remainder overshoot
deleted by production reset. The locked mixed verdict applies:

```text
CONTACT_IS_GAUGE_LATE_RESET_BREAKS_QUOTIENT_ONLY_BY_OVERSHOOT
```

Canonical result:
[`AUDIT_CONTACT_QUOTIENT_HORIZON.md`](../../07_assessment/AUDIT_CONTACT_QUOTIENT_HORIZON.md).
