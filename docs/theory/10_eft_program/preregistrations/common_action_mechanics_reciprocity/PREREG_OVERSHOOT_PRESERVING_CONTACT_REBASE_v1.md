# PRE-REGISTRATION — Overshoot-preserving contact rebase

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0527`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Scope:** observer-only construction and injectivity audit of the minimal
FTD-0526 diagonal-overshoot repair. No production state, default, toggle,
scenario, force, collision rule, field, or ontology change.

## 1. Candidate paired transaction

Use the identical, equal-mass, zero-COM-normal FTD-0526 pair. Let

```text
N=ceil(|d|/(2v)),
delta=Nv-|d|/2,
n=d/|d|.
```

One tick before the chart horizon, the crossing and already-bounced raw
representatives are physically permutation-equivalent but chart-distinct.

For the crossing representative, atomically exchange the two complete carrier
records across the occupied target charts and retain the normalized
remainders. Ordered by the two original anchors, the proposed output is

```text
a1: r=-delta*n, v=-v*n,
a2: r=+delta*n, v=+v*n.
```

For the already-bounced representative, ordinary free movement gives the same
output. No contact force or remainder reset is applied.

## 2. Registered theorem gates

For both polarities, three translations, every nonzero Moore direction, and
speeds `1/8` and `1/4` (`312` arms), require at `1e-12`:

1. the two preimages have positive raw chart-associated residual;
2. their unlabeled position/velocity/polarity multisets agree exactly;
3. their exact compact densities and face currents over the final tick agree;
4. paired record exchange preserves the full FTD-0526 overshoot and lands on
   the same raw output as free evolution of the bounced representative;
5. output continuity, total polarity, relativistic momentum, matter energy,
   causality, translation, polarity mirror, and cubic covariance close;
6. time reversal recovers the physical preimage quotient exactly;
7. without a branch record, the raw map has at least two preimages for one
   output and therefore no exact raw inverse;
8. one explicit branch bit distinguishes and reconstructs both registered
   preimages for one event;
9. omitting the bit or a physical identity label makes the merger unavoidable;
10. invalid inputs fail closed.

## 3. Locked verdicts

- If every constructive and two-preimage gate passes:
  `OVERSHOOT_REPAIR_CLOSES_PHYSICS_RAW_INVERSE_NEEDS_BRANCH_RECORD`.
- If overshoot cannot be preserved with exact current/continuity:
  `OVERSHOOT_PRESERVING_REBASE_CLOSED_NEGATIVE`.
- If the raw output remains injective without a record:
  `CONTACT_REBASE_IS_RAW_INJECTIVE`.
- If transformed arms disagree:
  `CONTACT_REBASE_COVARIANCE_UNRESOLVED`.

The pass verdict would not introduce a new no-go beyond FTD-0499. It would
instantiate that finite-fiber obstruction on the exact production contact
defect: physical quotient repair is constructive, but exact raw reversal must
either retain a branch/history datum or refactor all downstream production to
ignore the chart representative.

## 4. Execution record

Executed 2026-07-25 with pinned MSVC `14.44.35207`, Release, CPU observer.
The locked preregistration SHA256 before execution/status annotation was
`F04A9832F68E57D3389A85C69F2133E769E281EF6C0DF51FFFAD6AD89183FD4D`.

All `7/7` checks passed over 312 registered arms. The paired rebase preserved
every commensurate and diagonal overshoot, exact compact density, exact face
current, continuity, relativistic invariants, causality, physical reversal,
translation, polarity mirror, and cubic covariance. Two raw preimages merge
to one physical output; one branch bit reconstructs either registered history
for this event. The locked pass verdict applies:

```text
OVERSHOOT_REPAIR_CLOSES_PHYSICS_RAW_INVERSE_NEEDS_BRANCH_RECORD
```

Canonical result:
[`AUDIT_OVERSHOOT_PRESERVING_CONTACT_REBASE.md`](../../../07_assessment/common_action_mechanics_reciprocity/AUDIT_OVERSHOOT_PRESERVING_CONTACT_REBASE.md).
