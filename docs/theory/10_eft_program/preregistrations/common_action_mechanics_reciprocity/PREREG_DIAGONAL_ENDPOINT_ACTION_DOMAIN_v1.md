# PRE-REGISTRATION — Diagonal endpoint action-domain audit

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0532`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0484`, `FTD-0485`, `FTD-0487`, `FTD-0527`, `FTD-0531`  
**Scope:** observer-only composition test between the constructive FTD-0531
edge/corner endpoint and the frozen one-cell FTD-0485 two-slab action domain.
No production state, default, toggle, scenario, force, collision law, basis,
phase order, field ontology, normalization, or tolerance change.

## 1. Registered discriminator

Reconstruct each FTD-0531 next endpoint from the already-bounced FTD-0527
representative and its solved discrete-gradient displacement. For every active
component, register whether the open segment crosses an integer coordinate
plane. Record the crossing parameter

```text
tau_a=(m_a-x_start,a)/(x_end,a-x_start,a).
```

The symmetric Moore-normal path predicts simultaneous crossings: two planes
for a face diagonal and three for a body diagonal.

The FTD-0485 evaluator accepts a slab only if both endpoints lie in one closed
unit cell. Call it on a zero connection using a previous free point, the shared
start, and the FTD-0531 next endpoint. Zero connection removes all field-value
issues; rejection then tests only the declared action domain.

## 2. Registered arms and gates

Use the 240 FTD-0531 arms. Require:

1. every FTD-0531 root remains valid;
2. each edge arm crosses exactly two coordinate planes and each corner arm
   exactly three;
3. all active crossing parameters lie strictly in `(0,1)` and agree within
   `1e-12` on each arm;
4. the endpoint overshoot beyond every crossed plane is positive and the
   minimum exceeds `1e-8`;
5. the FTD-0531 field-dependent endpoint shift does not remove any crossing;
6. the frozen FTD-0485 one-cell evaluator rejects every arm even on the zero
   connection;
7. exact piecewise current/continuity, Gauss, energy, causality, and inverse
   from FTD-0531 remain below their locked tolerances;
8. translation, polarity mirror, and signed-cubic crossing data agree below
   `1e-12`;
9. invalid axial inputs fail closed.

## 3. Locked verdicts

- If every diagonal coupled path crosses the registered simultaneous planes
  and the compact evaluator rejects it:
  `ENERGY_ENDPOINT_CONSTRUCTIVE_COMPACT_COMMON_ACTION_OUT_OF_DOMAIN_AT_HOP`.
- If the coupled endpoint stays inside one cell and the evaluator accepts:
  `DIAGONAL_COUPLED_ENDPOINT_ENTERS_INTERIOR_ACTION_DOMAIN`.
- If crossings or rejection are not explained by the frozen domain rule:
  `DIAGONAL_ENDPOINT_ACTION_DOMAIN_UNRESOLVED`.

The first verdict would not invalidate the FTD-0531 energy transaction. It
would prove that FTD-0485 cannot supply its force without a registered
multi-cell variational extension. Because FTD-0485/0487 already establish
one-sided threshold nonuniqueness for generic charged connections, merely
splitting the path at the simultaneous planes would not by itself derive a
unique impulse.

## 4. Execution record

Executed 2026-07-25 without changing the locked protocol. All `240` arms pass
all eight classification gates. Edge paths cross two planes and corner paths
three at a common parameter; the coupled endpoint remains beyond the planes;
all `480` nonzero previous-segment controls are accepted; all `480` coupled
next-segment calls are rejected on the zero connection. Locked verdict:

```text
ENERGY_ENDPOINT_CONSTRUCTIVE_COMPACT_COMMON_ACTION_OUT_OF_DOMAIN_AT_HOP
```

Canonical audit:
[`AUDIT_DIAGONAL_ENDPOINT_ACTION_DOMAIN.md`](../../../07_assessment/common_action_mechanics_reciprocity/AUDIT_DIAGONAL_ENDPOINT_ACTION_DOMAIN.md).
The SHA256 of this preregistration before this execution annotation was
`CE71957C571052FD5BE4A6A29ADB283D8F0E71D75275E8D7604DB3CBDC83B3F7`.
