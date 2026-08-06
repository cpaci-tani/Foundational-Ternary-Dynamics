# PRE-REGISTRATION — Open-worldline hop selection v1

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0489`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0484`, `FTD-0487`, `FTD-0488`

## Question

Does the exact straight-worldline coupling of FTD-0484 define a
gauge-independent deterministic rule selecting one manifested Moore endpoint,
or does it only assign an action/current to a history whose endpoint has
already been selected?

This is an observer-only algebraic test. It does not alter production movement,
add a force toggle, or reopen FTD-0481--0483.

## Locked identities

For an open charged worldline from a common initial distribution `rho_0` to
candidate final distribution `rho_d`, FTD-0484 gives

```text
S_d[A+G chi, Phi-Delta_t chi]
  = S_d[A,Phi] + g(<rho_d,chi_1>-<rho_0,chi_0>).
```

For two candidates `d` and `e` with the same start,

```text
(S_d-S_e)' = (S_d-S_e)
            + g<rho_d-rho_e,chi_1>.
```

If the endpoints differ, `chi_1` is arbitrary and the last term can have
arbitrary sign and magnitude. Therefore the ordering of open interaction
actions is not gauge invariant. Adding any gauge-invariant kinetic cost cannot
repair this, because the arbitrary endpoint term remains.

A second exact statement concerns a fully cubic-symmetric input. If a
deterministic endpoint selector is `O_h`-equivariant and its input is fixed by
all of `O_h`, its output must be a vector fixed by all coordinate reflections.
The only such vector is zero. A nonzero face, edge, or corner hop therefore
cannot be uniquely selected from a rest/zero-field symmetric state without a
symmetry-breaking input or branch rule.

## Locked tests

Use `L=17`, `temporal_scale=C_SPEED`, integer-site start/end distributions,
and tolerance `1e-12`.

1. Compare `+x` and `+y` unit hops from the same start with equal
   gauge-invariant matter cost.
2. Apply two endpoint gauges of opposite sign while leaving `E` and `B`
   invariant. Require the exact endpoint-shift identity and opposite action
   orderings.
3. Repeat with unequal finite matter costs and a gauge amplitude large enough
   to reverse their ordering. This is a theorem fixture, not a fitted scan.
4. Check charge-sign reversal and integer translation.
5. Enumerate the 26 nonzero Moore displacements. Require exact cubic orbit
   counts `6/12/8` by squared length and zero nonzero vectors fixed by the
   three coordinate reflections.
6. Confirm that the production-style component threshold map is cubic
   covariant when a velocity/remainder already breaks the symmetry. This is a
   control showing that endpoint selection can be supplied by prior kinematics,
   but is then not derived by comparing open interaction actions.

## Frozen verdicts

- `OPEN_ACTION_SELECTS_HOP` only if candidate ordering is invariant under both
  endpoint gauges. This would contradict the locked gauge-endpoint identity and
  is expected to fail.
- `OPEN_WORLDLINE_ACTION_NOT_A_HOP_SELECTOR` if physical fields remain
  unchanged while candidate ordering reverses, all exact identities pass, and
  the cubic no-selector fixture closes.
- `IMPLEMENTATION_INVALID` if any endpoint, field-invariance, translation, or
  orbit-count identity misses `1e-12`.

## Claim boundary

The negative verdict does not invalidate the fixed-history worldline action,
its current, or its gauge covariance. It proves only that an open-path action
value is not an observable cost that may be minimized across different charged
endpoints. A completed classical update would need gauge-covariant endpoint
momentum data and a declared nonsmooth variational rule; a quantum completion
would need endpoint matter phases/amplitudes. The existing physical velocity
and `remainder` may select an endpoint kinematically, but then the common action
has not generated that selection.

No production dynamics are authorized by either outcome.

Run-of-record test-source SHA256:
`15A91695BE3785E7DD8B85BE06442C0CDBEC5DCE3F4C340230410F2C3EF5C53A`.
