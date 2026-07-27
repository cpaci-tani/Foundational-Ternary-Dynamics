# PRE-REGISTRATION — Production same-sign bounce reciprocity v1

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0506`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0449`, `FTD-0478`, `FTD-0497`, `FTD-0504`, `FTD-0505`

## Question

Does production `phase_movement` implement its documented same-sign "elastic
bounce" as a reciprocal finite-range exclusion event when subcell position is
`site+remainder`, or is it a fixed-target velocity reflection plus a lossy
remainder reset?

## Locked engine fixture

Use the actual CPU `RenderBridge` at `L=17`, periodic boundaries, `dt=1`, and
movement as the only enabled term. Enable the read-only history journal.

For each of all 26 nonzero Moore directions `d in {-1,0,+1}^3`, both
polarities, and three integer translations:

```text
source anchor       x
target anchor       x+d
source remainder    0.80 d
source velocity     0.25 d
target remainder    0
target velocity     0
source state        target state = polarity.
```

The maximum registered source speed is `sqrt(3)*0.25<C_SPEED`; the proposed
remainder is `1.05 d`, so every nonzero direction component attempts the
registered hop.

## Locked production-output checks

After one actual tick, require source and target manifestation to remain. Read
the literal output:

- source velocity components on `d` are sign-flipped;
- source remainder is reset to zero;
- target velocity/remainder/attributes remain unchanged;
- no same-sign collision event appears in the history journal.

Do not infer these from source comments; measure all 156 arms.

## Locked specular-exclusion comparator

For a finite equal-time reflection at the occupied target position, the
proposed component `r*=1.05 sign(d)` has specular endpoint

```text
r_spec = 2 sign(d)-r* = 0.95 sign(d).
```

The exact path is `x+0.80d -> x+d -> x+0.95d`. Compare it to the production
effective endpoint `x+0`. Require:

- nonzero remainder/position mismatch;
- production effective displacement speed above `C_SPEED` if the reset is
  interpreted as physical motion;
- distinct exact piecewise face-current signatures;
- exact continuity for each registered interpretation.

## Locked energy–momentum comparator

Use the production flat dispersion and momentum map. Require matter energy to
be unchanged by the source sign flip, but test total pair momentum directly.
The stationary target is not a wall primitive: if it remains unchanged while
the mover reverses, the matter momentum defect is

```text
Delta P_matter = -2 p_source.
```

All fields start at zero. Require no field momentum/recoil record and no event
current capable of balancing this defect.

## Locked inverse-tick check

From the one-tick production output, execute one more unchanged tick. A true
inverse would recover source remainder `0.80d` and velocity `0.25d`. Record the
actual maximum raw phase-space residual. No manual velocity flip, stored
branch, or altered update order is allowed.

## Frozen verdicts

- `PRODUCTION_BOUNCE_IS_FIXED_TARGET_RESET_NOT_RECIPROCAL_COLLISION` if energy
  is preserved but target recoil, subcell specularity, current accounting, and
  inverse recovery fail.
- `PRODUCTION_BOUNCE_QUALIFIES_FINITE_RANGE_EXCLUSION` only if the measured
  output matches the specular path and closes pair+field momentum/current and
  inverse gates below `1e-12`.
- `PRODUCTION_BOUNCE_SOURCE_COMMENTS_STALE` if the measured output differs from
  the documented axis-flip/reset rule.

## Scope ceiling

This is a read-only audit of the frozen production rule. It does not authorize
changing collisions, adding recoil, retaining remainders, changing movement
order, adding a toggle, or publishing a scenario.

## Run-of-record hashes

- test SHA256:
  `1BE727708850748B91D9699A3C967F8462B1D1C3584F92746575DF1F6E1AAE95`;
- header SHA256:
  `D6C36B535298FDD2ADC0AE49FA44230FE9E51109A35FB02106BD5D61AD522845`;
- observer implementation SHA256:
  `2D1211069DE48E40846135F813CDF162618796E270A73C84DF60FB942C271E3E`;
- audited production phase SHA256:
  `6149B37C5A28B8EE9B8544CAEC24006D0964D1C8F344CA63C68DC6536A47E8FB`.
