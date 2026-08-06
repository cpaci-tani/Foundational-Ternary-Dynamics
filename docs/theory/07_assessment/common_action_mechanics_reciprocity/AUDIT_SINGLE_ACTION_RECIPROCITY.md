# AUDIT — Single-action reciprocity

**Identifier:** `FTD-0467`  
**Date executed:** 2026-07-24  
**Status:** `[THEOREM — DISCRETE FIELD-SOURCE ADJOINT]` +
`[MEASURED — FORCE-BRANCH MISMATCH]` +
`[CLOSED NEGATIVE — CURRENT SINGLE-ACTION RECIPROCITY]`  
**Run of record:** `engine/results/ftd_0467/windows_msvc_cpu.csv`

## Result

The production electric field source is the exact field-side variation of the
written state-flux interaction, but no current production electric-force
branch is its matter-side partner. The locked verdict is

`NO_PRODUCTION_FORCE_BRANCH_IS_NATIVE_SINGLE_ACTION_PARTNER`.

This supplies a direct explanation for the matter-field momentum failures in
FTD-0438 and FTD-0465/0466: the current source and force laws are not generated
as the two variations of one interaction.

## Exact discrete identity

For the registered stationary electric interaction

`L_int = +G_C sum_x s(x) div(J)(x)`,

periodicity and the central-difference stencil give the exact adjoint identity

`sum_x J(x) dot[-G_C grad(s)(x)]
 = G_C sum_x s(x) div(J)(x)`.

The campaign measures a one-tick coupling-only production source residual of
exactly zero and an independent periodic summation-by-parts residual of
`2.17e-19`. Thus `-G_C grad(s)` is not the defect.

At fixed field, the same written interaction gives the registered central
point-probe force

`F_action = +G_C s grad(div J)`.

The three production branches instead implement

- legacy direct: `F_legacy = -alpha s grad(div J)`;
- emergent-density: `F_emergent = +G_C s grad|J|_r=2`;
- Poisson: `F_Poisson = -alpha s grad(phi_C)`, with `phi_C` solved from `s`
  rather than varied from the prescribed `J`.

The standalone `coupling_force` helper implements
`+alpha s grad(div J)`. It has the action operator and sign, but not the
coefficient of the written interaction, because `alpha=G_C^2` rather than
`G_C`. It is not called by the production force loop.

## Decisive fixtures

For `J_i=a r_i^2`, `a=1e-3`, at the probe center,

`grad(div J)=2a e_i`, while `grad|J|_r=2=0`.

For a positive probe, every axis gives

| Quantity | Axial value |
|---|---:|
| common-action force | `+1.708490862057e-4` |
| standalone helper | `+1.459470512866e-5` |
| production legacy | `-1.459470512866e-5` |
| production emergent-density | `0` |

The legacy law therefore has the opposite sign and magnitude ratio
`|F_legacy/F_action|=G_C`. The emergent-density law misses an action force
that is nonzero.

For `J_i=J0+a r_i`, `J0=0.1`, the relationship reverses:

`grad(div J)=0`, while `grad|J|_r=2=a e_i`.

The common action and legacy branch give zero, while the emergent-density
branch gives `+8.542454310285e-5 e_i` for positive polarity. Thus
`grad|J|` is not a discretization of `grad(div J)`; the two operators are
independent even on elementary local fields.

Changing between the quadratic and affine `J` fixtures changes the common-
action force but changes the production Poisson force by exactly zero. Its
action residual is `1.58e-4` to `1.61e-4` on the quadratic fixture and
`9.68e-6` to `1.24e-5` on the affine fixture. The Poisson branch is therefore
a separate auxiliary-potential mechanism at this gate.

All direct/emergent production formula replays, polarity-oddness tests, and
axis-covariance tests close exactly. The result is an operator-level mismatch,
not numerical noise or a preferred-axis artifact in those branches.

## Consequence for the ontology

The FTD-0466 failure does not yet require hidden 13-channel flux variables.
The simpler obstruction comes first: the present `J/W` field is asked to
receive `-G_C grad(s)`, while manifested matter responds through another
functional. No discrete Noether or action-reaction argument can close total
momentum across that split.

The current engine therefore contains three distinct electric models:

1. a native state-to-`J` source from the written interaction;
2. a selected `grad|J|` probe rule called emergent;
3. a selected Poisson auxiliary-potential force.

They may be studied separately, but they cannot be combined and described as
one variational matter-field dynamics.

## Next gate

Before adding channel variables or another recoil optimizer, construct an
observer-only common-action candidate using the existing `s,J,W` variables:

1. retain the exact source `-G_C grad(s)`;
2. use the same interaction's central force `+G_C s grad(div J)`;
3. derive the paired field impulse from the interaction rather than assigning
   a recoil afterward;
4. test isolated-pair total momentum, finite-hop work, reversibility, and
   stability with no production modification.

If that candidate closes, the immediate defect is the force implementation,
not the ontology. If it fails even as an observer construction, the
13-channel/half-tick transaction extension becomes the next justified model.

## Reproducibility

- campaign SHA-256:
  `2EB51724924C07631FBDBE2C218E1D3B02144DB0398B1E5BFA5FB251AE94A6AF`
- run-record SHA-256:
  `105C99892EACDEB6693B793354DFF51688D7B2E05773E915FECD0D793F3FF32B`
- post-execution preregistration SHA-256:
  `DADDF946C13CF36C37D4CADDB1ABFCF0ED68FF2ADFFB7671A3D97013CDAC3D0D`
- compiler: pinned MSVC `14.44.35207`, Release
- focused CTest: `1/1` pass
- production tick: unchanged
