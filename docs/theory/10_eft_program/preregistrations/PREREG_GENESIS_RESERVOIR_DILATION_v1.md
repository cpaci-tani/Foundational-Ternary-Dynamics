# PRE-REGISTRATION — Genesis reservoir dilation v1

**Identifier:** FTD-0569
**Date locked:** 2026-07-26
**Status:** `[PRE-REGISTERED — EXACT OBSERVER / FROZEN EVENT KERNEL]`
**Parents:** FTD-0425, FTD-0499, FTD-0567
**Production effect:** none.

## 1. Question

Can the existing genesis/evaporation event kernel be understood as the
projection of a local reversible matter--environment transaction without
changing its raw production update? If not, which part fails: accepted
genesis, stochastic acceptance, energy closure, evaporation, or indefinite
record capacity?

This registration tests an observer-only mathematical dilation. It does not
add an engine reservoir, modify a random stream, change event ordering, or
promote a new ontology.

## 2. Frozen production maps

For one accepted canonical single-substrate genesis event, write

```text
r = |J| = kg + x,  x > 0,
J' = (1 - kg/r) J,
W' = (1-d) W,
s: 0 -> sigma in {-1,+1}.
```

The production acceptance probability is

```text
p(x) = 1 - exp(-x/km).
```

Evaporation changes

```text
(s, particle_id, spin, color) -> (0,-1,0,0)
```

and leaves `J`, `W`, velocity, remainder, and latency unchanged.

The dual genesis path assigns the manifested labels without applying the
single-path `J/W` drain.

## 3. Locked theorem candidates

### G1 — Conditional inverse of accepted genesis

For `0 <= d < 1` and canonical void labels, the accepted single-path map has
the candidate inverse

```text
J = (1 + kg/|J'|) J',
W = W'/(1-d),
s_before = 0.
```

The event output state distinguishes the accepted branch from the rejected
identity branch. Test both polarities, ten directions (six axes and four body
diagonals), excesses `{0.125,0.5,1.25}`, three wave velocities, and drains
`{0,0.5,0.9}`: 540 accepted single-path arms. Require maximum state/field
inverse residual below `1e-12`.

At `d=1`, two distinct pre-event wave velocities must map to the same `W'=0`.
This is a registered collision requiring an environmental wave record.

### G2 — Exact one-step Bernoulli dilation

For `0<p<1` and a uniform reservoir phase `u in [0,1)`, define

```text
b = 1, u' = u/p                    if u < p,
b = 0, u' = (u-p)/(1-p)            if u >= p.
```

The inverse is

```text
u = p u'                           if b=1,
u = p + (1-p)u'                    if b=0.
```

Test four probabilities (the three production `p(x)` values plus `p=1/2`)
and four interior phases per probability, covering both branches. Require
inverse residual below `1e-15`, exact branch intervals, and `u' in [0,1)`.

This construction is a deterministic dilation of one Bernoulli draw. It is
not credited as an engine mechanism unless a production variable implements
the phase update and retains the branch history.

### G3 — Repeated-trial information cost

If the branch output is later erased, the map `u -> u'` has two preimages for
every interior `u'`. After `N` erased binary trials it has `2^N` preimages and
requires `N` branch bits for exact reversal. Check this exactly for
`N=1,...,20`, and forward/reverse one 20-step variable-probability sequence
with its stored branch word.

This is the FTD-0499 lower bound specialized to the production acceptance
law. A stateless counter-based RNG may be recomputed from seed/site/tick, but
it is an external time schedule, not a dynamically updated reservoir carrying
past branches.

### G4 — Genesis and evaporation are not inverse event classes

Let `G` be accepted single genesis and `E` production evaporation applied to
its output. The composition must satisfy

```text
E(G(0,J,W)) = (0, J-kg*n, (1-d)W),
n = J/|J|,
```

so its flux distance from the input is exactly `kg`, and its wave distance is
`d|W|`. Test all 540 G1 arms. The exact reverse transition from the manifested
output to the original void input is absent from the frozen event kernel.

Consequently, for any genesis pair `a -> b` with positive transition
probability, the reverse production event probability `P(b -> a)` is zero.
No positive stationary weights can obey detailed balance
`pi(a)P(a,b)=pi(b)P(b,a)` on that pair.

### G5 — Reservoir energy is continuous and branch dependent

With field energy `(|J|^2+|W|^2)/2`, the single-path withdrawal is

```text
D(x,W,d) = kg*x + kg^2/2 + (d-d^2/2)|W|^2.
```

For fixed `kg>0`, `dD/dx=kg`; therefore no finite discrete reservoir and no
single fixed manifested-state quantum can conserve extended energy for every
continuous overshoot. The dual path has `D=0` for the same ternary state
assignment. Verify the formula and slope on every registered arm.

## 4. Frozen source provenance

- `phase_write.cpp`:
  `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4`
- `voxel_rng.h`:
  `15EA4843331471E0B75488BAB9D87072E1CD7FD41FBC485A2BDD81EBC8841093`
- `finite_memory_reversible_lift.h`:
  `D593C991597A69DEF1BE389CB69DEE3168F44B1B774FBBBE7D6B30C59D92B092`
- `finite_memory_reversible_lift.cpp`:
  `13E2C4E8F4777C38C9AA01260E44A0D823DC89E89E92DA58C3BC5704ED9E5265`
- FTD-0567 theorem:
  `877ACAA8C859DFE065120543B8FBC7862BD619AFCB57A4B7CD6D214A6CA18055`

## 5. Verdicts

- `FINITE_LOCAL_REVERSIBLE_PRODUCTION_DILATION` only if one fixed finite local
  reservoir reverses every registered event and repeated cycle while leaving
  the projected production map unchanged.
- `ONE_EVENT_DILATION_OPEN_SYSTEM_ONLY` if accepted genesis and the one-step
  Bernoulli dilation are constructive, but evaporation, detailed balance,
  energy capacity, or repeated history fails.
- `ACCEPTED_GENESIS_NONINVERTIBLE` if the candidate inverse fails for any
  canonical arm with `d<1`.

No tolerance, event rule, drain, energy definition, or failure consequence may
be changed after this lock. A positive one-event dilation does not license a
toggle, scenario, particle, charge, unitarity, or equilibrium claim.

## 6. Planned artifacts

- `engine/include/ftd/eft/genesis_reservoir_dilation.h`
- `engine/src/eft/genesis_reservoir_dilation.cpp`
- `engine/tests/test_genesis_reservoir_dilation.cpp`
- `scripts/proofs/proof_genesis_reservoir_dilation.py`
- `engine/results/ftd_0569/windows_msvc_cpu.json`
- theorem and audit records under the native-EFT and assessment directories.
