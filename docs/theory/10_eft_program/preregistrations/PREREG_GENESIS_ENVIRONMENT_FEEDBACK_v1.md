# PRE-REGISTRATION — Genesis environment-feedback necessity v1

**Identifier:** FTD-0571
**Date locked:** 2026-07-26
**Status:** `[PRE-REGISTERED — EXACT MATRIX THEOREM / SOURCE AUDIT]`
**Parents:** FTD-0567, FTD-0569, FTD-0570
**Production effect:** none.

## 1. Question

FTD-0570 constructs a branchwise common action only after adding environmental
and conjugate variables. Can those variables be reinterpreted as already
existing `Voxel` spectators while leaving the projected production genesis
assignment exactly unchanged and independent of their microstate?

This registration tests a local derivative theorem on one accepted canonical
single-genesis branch. It does not assume global Lorentz symmetry, quantum
unitarity, or a continuum Hamiltonian between ticks.

## 2. Locked block theorem

Let `x` be a `2n`-dimensional canonical system and `e` a `2m`-dimensional
canonical environment. At one differentiable branch, write the derivative of
an enlarged map as

```text
S = [ M  B ]
    [ C  D ].
```

The direct-sum symplectic form is `Omega=diag(Omega_x,Omega_e)`. If the
projected system output is independent of the incoming environment, then
`B=0`. The lower-right symplectic equation gives

```text
D^T Omega_e D = Omega_e,
```

so `D` is invertible. The upper-right equation then gives

```text
C^T Omega_e D = 0  =>  C=0.
```

The upper-left equation reduces to

```text
M^T Omega_x M = Omega_x.
```

Therefore an environment-independent projected map has a symplectic
extension only if the projected derivative is already symplectic. Equivalently,
every symplectic dilation of a non-symplectic system map requires `B!=0`: the
system output must depend on the incoming environmental microstate away from
any specially prepared submanifold.

The independent proof must verify these block implications symbolically for
generic nonsingular `Omega_e` and `D`.

## 3. Frozen genesis defect and rank

For accepted single genesis, use the FTD-0570 canonical interpretation
`x=(J,W)` and the radial/tangential basis:

```text
M = diag(A,aI),
A = diag(1,t,t),
t = x/(x+kg),
a = 1-d.
```

With

```text
Omega_x = [ 0  I]
          [-I  0],
```

the symplectic defect is

```text
Delta = Omega_x-M^T Omega_x M
      = [0, K; -K, 0],
K = I-aA = diag(d,1-at,1-at).
```

Hence

```text
rank Delta = 4  for d=0,
rank Delta = 6  for 0<d<1,
```

because `0<t<1`. The raw volume Jacobian remains `t^2 a^3<1`.

Run 90 arms: ten directions, excesses `{0.125,0.5,1.25}`, and drains
`{0,0.5,0.9}`. Require 30 rank-four and 60 rank-six witnesses, analytic/matrix
defect residual below `1e-12`, and every determinant strictly below one.

## 4. Existing-variable source audit

Within the accepted single-genesis event submap, the only continuous writes
are

```text
J -> (1-kg/|J|)J,
W -> (1-d)W.
```

`manifest_at` writes only discrete `state`, `particle_id`, `spin`, and `color`.
The following 34 continuous `Voxel` components are spectators during that
event:

```text
flux_L/R and wave_vel_L/R                      12
velocity and remainder                         6
latency, tau, phase, accel_mag                  4
flux_strong, wave_vel_strong                    6
flux_weak, wave_vel_weak                        6
```

The observer must report this inventory and the proof must source-lock
`phase_write.cpp` and `voxel.h`. Reads used to select polarity/spin do not
alter the continuous `(J,W)` output inside a fixed accepted/label branch.

The stateless `voxel_uniform(seed,site,tick,salt)` schedule is not an incoming
dynamical environment coordinate and supplies no nonzero `B` block.

## 5. Prepared-bath loophole

A symplectic enlarged map may reproduce the production assignment on a
special incoming bath state `e=e0` while retaining `B!=0` off that submanifold.
That is not excluded. It has a precise price:

- the raw production rule is no longer environment independent as an enlarged
  law;
- the bath must be prepared at `e0`, returned to it, or replaced before each
  event;
- otherwise later projected updates depend on retained bath history.

Combined with FTD-0499/0569/0570, repeated exact production behavior therefore
requires reset/export, an infinite-information exact state, or a modified
projected transition. This is an open driven-environment construction, not a
closed native common action.

## 6. Frozen source provenance

- `phase_write.cpp`:
  `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4`
- `voxel.h`:
  `8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3`
- `genesis_natural_extension.h`:
  `07FE4D2FDA22DB221BB1F22683F402FD7E8AAA8E6B075472C9DA1CE6179D21F1`
- `genesis_natural_extension.cpp`:
  `9572106322C83383AD087DCBD7EA5EFBBE5F5E3B10A5B49923A89BEDDEFA24BD`
- FTD-0570 theorem:
  `2611A6DE2D2318DFC4EC97FDF148D91D952BE3775421BE4DDAC441EA2F534076`

## 7. Verdicts

- `ENVIRONMENT_FEEDBACK_OR_RESET_REQUIRED` if the block theorem, all 90 rank
  arms, the 34-component source audit, and source provenance pass.
- `EXISTING_SPECTATORS_CLOSE_NATIVE_ACTION` only if the projected output can
  remain independent of the environment (`B=0`) while `M` is non-symplectic;
  the block theorem predicts this verdict is impossible.
- `RAW_GENESIS_SYMPLECTIC` only if every registered defect vanishes.

No result licenses a bath toggle, event reset, new RNG state, particle,
unitarity, equilibrium, or production change.

## 8. Planned artifacts

- `engine/include/ftd/eft/genesis_environment_feedback.h`
- `engine/src/eft/genesis_environment_feedback.cpp`
- `engine/tests/test_genesis_environment_feedback.cpp`
- `scripts/proofs/proof_genesis_environment_feedback.py`
- `engine/results/ftd_0571/windows_msvc_cpu.json`
- theorem and audit records under the native-EFT and assessment directories.
