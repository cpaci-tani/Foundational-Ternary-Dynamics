# AUDIT — Common-action kick reciprocity

**Identifier:** `FTD-0468`  
**Date executed:** 2026-07-24  
**Status:** `[THEOREM — EXACT COMMON-ACTION KICK MOMENTUM IDENTITY]` +
`[MEASURED — EVOLVING NATIVE HISTORIES]` +
`[OPEN — FULL-TICK ENERGY AND HOP]`  
**Run of record:** `engine/results/ftd_0468/windows_msvc_cpu.csv`

## Result

The existing `J/W` field already contains an exact local recoil channel for
the matter force derived from the written interaction. The locked verdict is

`COMMON_ACTION_KICK_MOMENTUM_RECIPROCITY_EXACT`.

Across 12 nontrivial static controls and 384 snapshots from evolving native
wave/coupling histories, the worst matter-plus-field momentum residual is
`4.08e-15`. The source kick is exactly face-local and reverses below
`6.94e-18`.

## Exact proof

Let the periodic central derivative in direction `i` be `D_i`, and use the
registered field momentum

`P_i^field = -sum_x W_a(x) D_i J_a(x)`.

The production stationary-electric source component is

`Delta W_a = -G_C D_a s`.

At fixed `J`, its field-momentum change is therefore

`Delta P_i^field
 = -sum_x Delta W_a D_i J_a
 = +G_C sum_x (D_a s)(D_i J_a)`.

Periodic central differences are skew-adjoint and commute, so

`Delta P_i^field
 = -G_C sum_x s D_a D_i J_a
 = -G_C sum_x s D_i div(J)`.

The last expression is exactly the negative of the common-action matter
impulse

`I_i^matter = +G_C sum_x s D_i div(J)`.

Thus

`Delta P^field + I^matter = 0`

on every finite periodic lattice, without a continuum limit, fitted recoil,
or new degree of freedom.

## Measurements

The static fixtures deliberately avoid zero-impulse symmetry:

| Fixture | Matter impulse magnitude | Support | Worst closure |
|---|---:|---:|---:|
| one polarity, quadratic `J` | `1.70849e-4` | 6 | `5.42e-19` |
| opposite pair, cubic `J` | `3.07528e-3` | 12 | `1.04e-17` |

All three axes and both polarity orientations give exact axial covariance and
oddness. A deterministic nonzero pre-kick `W` does not affect closure.

For six evolving `L=33` pair arms, each with 64 snapshots and a longitudinal
travelling mode:

- impulse RMS is `3.40968e-4` in every arm;
- minimum sampled impulse is `6.26456e-7`, so no arm closes only through a
  trivial zero;
- worst momentum residual is `4.08e-15`;
- worst direct formula-replay residual is `4.08e-15`;
- worst inverse residual is `6.94e-18`;
- support remains exactly 12 face-neighbor sites with zero update elsewhere.

The larger evolving residual is floating accumulation over a full `33^3`
momentum sum and remains more than two orders below the registered `1e-12`
gate.

## Reconciliation with earlier negatives

FTD-0438 remains correct for the force it measured: production
`emergent_forces` uses `G_C s grad|J|`, while the field source uses
`-G_C grad(s)`. Those two laws are not adjoint partners, so the selected force
produces no corresponding central-field recoil.

FTD-0465/0466 also remain correct: translating or permuting a field coat is not
the same operation as applying the interaction source kick. Their momentum
failures reject those event maps, not the ability of `J/W` to carry recoil.

FTD-0453 through FTD-0458 solve an assigned finite-hop recoil after the fact.
FTD-0468 identifies a more primitive route: derive both impulses from the same
interaction before selecting an arbitrary recoil distribution.

## Ontological consequence

No additional 13-channel flux ontology is currently required for electric
momentum conservation. The three-vector field plus its conjugate `W` has enough
structure because the local polarity gradient deposits exactly the momentum
removed by the common-action matter force.

The cleanest current story is therefore:

1. polarity `s` sources the conjugate field momentum on its six face links;
2. the resulting longitudinal field acts back on polarity through
   `+G_C s grad(div J)`;
3. these two operations form an exact action-reaction pair;
4. the current production matter force breaks that pair by reading a different
   functional.

This establishes a native electric recoil mechanism, not yet a complete
particle or photon ontology.

## Next gate

The remaining obstruction is energy and time-centering. The next campaign must
apply the common-action matter impulse and source kick as one shadow
half-tick transaction, then measure:

- particle kinetic-energy change;
- exact native field-energy change;
- interaction-energy change;
- total momentum and energy through forward and reverse steps;
- whether one symmetric kick-drift-kick rule closes without a fitted
  counterterm.

Only after that should the transaction be combined with an integer site hop.
Continuous remainder motion is still invisible to `s` and therefore remains a
separate source-motion problem.

## Reproducibility

- campaign SHA-256:
  `5FAF98C9D0C1183C0DA6D9482DF39F6A5A0B77F886664B501467221A85AE502C`
- run-record SHA-256:
  `FF9AE8486A0812AB25AFF2D9C18A45BA0AD9808A901A2128F33752E1868268ED`
- post-execution preregistration SHA-256:
  `7C14E8AC2E53F7986253F6900D33F8DF42E3FCF4F340452DE22FE1EFFFBCD269`
- compiler: pinned MSVC `14.44.35207`, Release
- focused CTest: `1/1` pass
- production tick: unchanged
