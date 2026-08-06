# PRE-REGISTRATION — Multi-cell worldline variation

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0533`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0484`, `FTD-0485`, `FTD-0487`, `FTD-0490`, `FTD-0532`  
**Scope:** observer-only differentiation of the already-defined exact
FTD-0484 deposited worldline action across internal cell breaks. No production
state, default, toggle, scenario, force, collision law, phase order, field
ontology, normalization, or tolerance change.

## 1. Mathematical object

For arbitrary causal straight segments, use the existing FTD-0484 partition
at every integer-plane crossing and its exact cellwise cubical Whitney line
integrals. For two joined time slabs define

```text
S_int(x_*)=S_int^-(x_-,x_*)+S_int^+(x_*,x_+).
```

Evaluate the shared-point gradient from the complete deposited action, not by
choosing one incident cell. Use fourth-order centered differences at
`h={2^-10,2^-11,2^-12,2^-13}` and independent second-order one-sided and
directional probes. No fitted smoothing or averaging of incident-cell forces
is permitted.

An internal face/edge/corner crossing is not an endpoint knot. Its crossing
parameter is allowed to move under variation, and the exact FTD-0484 segment
partition must be recomputed on every probe.

## 2. Registered controls and gates

1. On a strict one-cell path, the smallest-step gradient must agree with the
   analytic FTD-0485 automatic-differentiation result below `1e-8`.
2. On face, edge, and corner paths with strictly interior endpoints and one
   internal simultaneous crossing, the last two centered gradients must agree
   below `1e-8`.
3. Forward/backward one-sided gaps must decrease by at least a factor `3` on
   each halving over the final two refinements, consistent with the expected
   second-order truncation rather than a finite cusp.
4. Directional derivatives along all signed axial directions and the
   normalized edge/corner diagonals must agree with the recovered gradient
   contraction below `1e-8`.
5. A connected two-slab gauge transformation with common intermediate gauge
   value must leave the shared gradient invariant below `1e-8`; a pure-gauge
   pair must give gradient below `1e-8`.
6. Both polarities, three translated copies, proper signed-cubic rotations,
   and the 240 FTD-0532 diagonal geometries on a zero connection must remain
   valid. Zero-connection gradients must stay below `1e-10`.
7. The FTD-0485/0487 endpoint-threshold counterexample must retain a nonzero
   side-limit gap above `1e-4`. Internal-knot recovery is not allowed to erase
   endpoint nonuniqueness.
8. Invalid charges, unjoined slabs, noncausal segments, and nonfinite step
   sizes must fail closed.

All action values must agree with direct FTD-0484 deposited-current
contractions by construction. Report convergence and side-limit data; do not
promote finite-difference agreement to an exact differentiability theorem.

## 3. Locked verdicts

- If the internal face/edge/corner gradients converge and all covariance,
  gauge, and threshold controls pass:
  `GLOBAL_DEPOSITED_ACTION_HAS_UNIQUE_INTERNAL_KNOT_VARIATION`.
- If a face, edge, or corner internal crossing retains a nonzero directional
  or one-sided gap:
  `SIMULTANEOUS_INTERNAL_KNOT_ACTION_NONDIFFERENTIABLE`.
- If the compact-domain rejection is removed but the numerical convergence
  discriminator is inconclusive:
  `MULTICELL_WORLDLINE_VARIATION_UNRESOLVED`.

A constructive verdict would extend the interaction-force evaluator through
the FTD-0532 internal hop geometry. It would not prove that the FTD-0531 scalar
energy endpoint is stationary under the full vector action, because that
requires a separate reconstruction of the dynamical connection/field action.
It would not repair a particle endpoint placed exactly on a charged lattice
threshold.

## 4. Execution record

Executed 2026-07-25 without changing the locked discriminator. All `9/9`
checks pass. The exact deposited action reproduces the compact analytic
interior force, converges through internal face/edge/corner knots, is linear on
all signed Moore directions, and is invariant under connected gauge changes.
All 240 FTD-0532 geometries enter the global domain. The charged endpoint side
gap remains nonzero. Locked verdict:

```text
GLOBAL_DEPOSITED_ACTION_HAS_UNIQUE_INTERNAL_KNOT_VARIATION
```

Canonical audit:
[`AUDIT_MULTICELL_WORLDLINE_VARIATION.md`](../../07_assessment/AUDIT_MULTICELL_WORLDLINE_VARIATION.md).
The SHA256 of this preregistration before this execution annotation was
`3A9AB2FBE62921DB3D847843C12F9A731B9A5B6EE05DD28EBA63FBA915E8CF3F`.
