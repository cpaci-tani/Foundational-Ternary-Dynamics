# Site-ontic atomic reciprocal-hop closure (FTD-0599)

**Status:** `[COMPUTER-ASSISTED THEOREM — CLOSED NEGATIVE FOR THE LOCKED R0 MAP]`  
**Verdict:** `SITE_ONTIC_NATIVE_RECOIL_MAP_FAILS_ATOMIC_COMPATIBILITY`  
**Date:** 2026-07-26  
**Protocol:** [`PREREG_SITE_ONTIC_ATOMIC_RECIPROCAL_HOP_v1.md`](../../preregistrations/common_action_mechanics_reciprocity/PREREG_SITE_ONTIC_ATOMIC_RECIPROCAL_HOP_v1.md)  
**Protocol SHA-256:** `DDD146E19C06E488C584AFBAB4092FB802E72F4DFC13F12407A5A914704E8886`

## Result

The locked no-new-persistent-variable R0 transaction fails its first
nontrivial ballistic arm. The half-open site chart, exact deposited current,
native source update, three-component recoil equation, and causal kinematic
closure coexist and select a unique body-diagonal hop. The independently
declared native energy and matter-work equations do not coexist with that
same transaction.

The decisive arm is the second arm in the locked order:

```text
L                      17
polarity               -1
initial remainder      (0.05, 0.05, 0.05)
initial speed          0.15 along (-1,-1,-1)/sqrt(3)
selected site shift    (-1,-1,-1)
```

The preceding stationary dressed control passes. No fixture was changed
after the protocol lock.

## Certified root

The C++ observer finds one admitted root. All 28 registered starts converge:

\[
p_1=(-0.04581326207056\ldots)^3,
\qquad \|F(p_1)\|_\infty=8.18\times10^{-14},
\]

with numerical Jacobian condition number `1.0000309442`.

The independent Python/Arb reconstruction does not assume the Newton root is
unique. It first uses

\[
\|I_i\|\le G_C(\sqrt3+1)\|D_iJ_0\|_2
\]

to enclose every possible root in
`p_0 + [-0.006040267,0.006040267]^3`. That box lies entirely in the
registered one-plane-crossing CIC chart. Directed Arb differentiation of all
six possible simultaneous-plane orderings gives

\[
\|DT\|_\infty\le 0.001573504586<1.
\]

Consequently the fixed point is unique on the necessary box and there is no
admitted root outside it. Residual divided by `1-||DT||` encloses the root in
an infinity-radius `5.01e-14` ball. The spectral static-dressing residual is
`1.79e-17`, with solution-error bound `3.98e-16`.

## Exact gates that pass

For the C++ transaction:

| Gate | Residual |
|---|---:|
| coated continuity | `7.81e-18` |
| kinematic closure | `0` |
| matter plus field recoil | `8.18e-14` |
| root residual | `8.18e-14` |

The deposited current reconstruction differs from the independently existing
FTD-0577 coat by `1.73e-18`. Frozen production and inherited-observer hashes
match the preregistration exactly.

## Decisive incompatibility

Using the independently frozen state energies,

\[
E_0=0.1759811457231967,
\qquad
E_1=0.1759747953114613.
\]

Therefore

\[
\epsilon_E=6.3504117354\times10^{-6},
\qquad
\epsilon_W=6.3504117354\times10^{-6}.
\]

The registered tolerance is `1e-12`. An independent reconstruction agrees at
the `2.2e-14` level and, after a deliberately conservative `1e-10` numerical
enclosure, proves both residuals exceed `6.35031e-6`. The candidate misses
both energy gates by at least `6.35e6` times the tolerance.

The mismatch is not a failure to deposit current or conserve the selected
translation momentum. It is a compatibility failure between the impulse
defined by exact native field recoil and the work required by the exact native
energy functional. Changing either equation, adding a counterterm, subtracting
self-field energy, changing source timing, or fitting a multiplier would be a
new candidate forbidden by this lock.

## Scope and consequence

This closes the registered R0 map, not every conceivable law on the same
variables. Because one-event acceptance is conjunctive, the inverse and the
256-event campaign are not run after this certified counterexample. Their
non-execution is required by the protocol and is not evidence that they fail.

The result establishes:

```text
the locked site-ontic native-recoil map is not an energy-closing atomic hop.
```

It does not establish:

```text
all site-ontic dynamics are impossible;
a new primitive is logically mandatory;
the quadratic-coat position experiment is a physical particle;
constituent phase space or a gauge connection has already been adopted.
```

Under FTD-0598, the next escalation boundary is explicit
constituent-complete phase space for matter. A microscopic connection/holonomy
and conjugate electric field remain the later electromagnetic ontology floor
if the constituent branch cannot close reciprocal local coupling.

## Artifacts

- `engine/include/ftd/eft/site_ontic_atomic_reciprocal_hop.h`
- `engine/src/eft/site_ontic_atomic_reciprocal_hop.cpp`
- `engine/tests/test_site_ontic_atomic_reciprocal_hop.cpp`
- `scripts/proofs/proof_site_ontic_atomic_reciprocal_hop.py`
- `engine/results/ftd_0599/ftd_0599_one_event_v1.json`
- `engine/results/ftd_0599/ftd_0599_one_event_v1.csv`

Production state, defaults, RNG, toggles, scenarios, CUDA, and WASM are
unchanged.

## Verification boundary

The focused `site_ontic_atomic_reciprocal_hop` CTest passes, and the
independent Arb certificate exits successfully. The repository-wide
`engine\\build_native.bat golden` command presently passes 3 of 7 registered
goldens and fails `render_bridge_golden`, `default`, `boundary`, and `L9` in
the pre-existing dirty production/backend worktree. FTD-0599 does not alter
those targets or their frozen inputs, and every production-file hash named in
the preregistration matches exactly. The broader golden battery is therefore
not claimed green by this result.
