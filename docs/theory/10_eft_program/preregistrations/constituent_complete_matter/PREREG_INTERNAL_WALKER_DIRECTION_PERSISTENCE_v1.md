# FTD-0616 — Internal-walker direction and persistence v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION/EXECUTION]`
**Scope:** observer-only long-time discriminator for the constructive FTD-0615 arms
**Production change:** forbidden

## 1. Frozen parent and candidate modes

Require the FTD-0615 run-of-record SHA-256
`8B7DD5809DE70B5EEA3C398C4A58AE2B0F64EFD6FD0BF653FBA4F4F0569ABA2C`
and reproduce the exact FTD-0612 rest state, uniform neutralizer, common-action
solver, and complete six-mode basis.  Test only the two modes selected by the
locked parent result: rotation modes `0` and `1` at excitation
`4 Delta_ref`.  This is a registered successor selection, not a new search.

## 2. Arms and histories

For each selected mode and both momentum signs, run the complete state under
zero initial centre momentum for 512 forward ticks and 512 state-only inverse
ticks.  Repeat each arm under zero, one, and two cyclic proper-cubic rotations
`R(x,y,z)=(y,z,x)`, rotating constituents, momenta, face/edge fields, and the
internal tangent together.  The uniform neutralizer is invariant.  This gives
12 arms and 12,288 atomic transactions.

Record at every forward tick:

- the full centre and displacement vectors;
- centre momentum;
- the internal tangent coordinates
  `q_u=sum_a[(r_a-c)-(r_a0-c0)] dot u_a` and
  `p_u=sum_a p_a dot u_a`;
- anchor changes, pair-distance range, common-action residual, total-energy
  drift, and field-plus-matter pseudomomentum defect.

No displacement direction, phase origin, fit window, or mode may be selected
after execution.

## 3. Algebraic and covariance gates

Every arm must complete all 1,024 transactions, retain pair distances in
`[0.5,2.0]`, keep maximum anchor multiplicity at two, common-action residual
at most `1e-12`, energy drift at most `1e-10`, and state-only recovery at most
`1e-8`.

For every mode/sign, the one- and two-turn histories must equal the cyclic
rotation of the zero-turn history.  Test the centre at every recorded tick and
the complete final state.  The maximum covariance residual must be at most
`1e-8`.

## 4. Direction-control gate

For each mode at zero rotation, let `d_+` and `d_-` be the 512-tick signed
displacement vectors.  Internal sign controls direction only if

```text
|d_+| >= 2,  |d_-| >= 2,
cos(d_+,d_-) <= -0.99,
||d_+|/|d_-| - 1| <= 0.05.
```

The test is on vectors, not displacement magnitudes.  Same-direction or
orthogonal sign pairs fail this gate.

## 5. Persistence gate

Split each forward history into four fixed 128-tick windows with displacement
vectors `w_j`.  An arm is persistent only if every `|w_j| >= 0.5`, every
successive direction cosine is at least `0.95`, and the coefficient of
variation of the four window speeds is at most `0.25`.  All 12 arms must pass.

The recorded `(q_u,p_u)` history is diagnostic only in v1.  No recurrence or
phase-lock verdict is inferred from it until an independently fixed phase
coordinate and return criterion are registered.

## 6. Verdicts

- `INTERNAL_WALKER_DIRECTION_CONTROLLED_PERSISTENT`: algebraic/covariance,
  direction-control, and persistence gates all pass;
- `INTERNAL_WALKER_TRANSIENT_OR_UNCONTROLLED`: every algebraic/covariance arm
  completes, but direction control or persistence fails;
- `INTERNAL_WALKER_DIRECTION_PERSISTENCE_NUMERICALLY_UNRESOLVED`: any parent,
  rest, arm, action, energy, inverse, record, or covariance coverage fails.

Even the constructive verdict is selected, externally neutralized lattice
walking.  It does not establish isolated self-propulsion, conserved total
momentum, a gapless pole, a physical particle, or microscopic electromagnetism.

**Protocol lock:** `protocol_sha256=E55D5CFA92EB719569B2A8F6D4F19EDB9C90DE49BA4C2B1721AC06F0B0AA730B`

**Hash-record correction (2026-07-27):** the first displayed value
`548C8035...05C79` was computed by a PowerShell command in which the Markdown
backtick delimiter was consumed as a shell escape. The actual hash of the
unchanged prefix above is `E55D5CFA...A730B`. No protocol text, arm, gate,
tolerance, or verdict rule changed. The campaign was rerun after correcting
the executable metadata.
