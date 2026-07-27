# AUDIT — Injective local permutation event

**Identifier:** `FTD-0466`  
**Date executed:** 2026-07-24  
**Status:** `[CONSTRUCTIVE CONTROL — EXACTLY INVERTIBLE LOCAL MAP]` +
`[MEASURED — ENERGY/KINEMATICS PASS]` +
`[CLOSED NEGATIVE — MOMENTUM CLOSURE]`  
**Run of record:** `engine/results/ftd_0466/windows_msvc_cpu.csv`

## Result

Replacing FTD-0464's additive field merge with an exact local permutation
removes the information-loss defect but does not close momentum. The locked
verdict is

`INJECTIVE_LOCAL_PERMUTATION_MOMENTUM_MISMATCH`.

The 36-site cyclic map and its inverse agree exactly on the deterministic
fixture and all 84 actual events. No field value outside the support changes.
All 84 particle updates are real and total event energy closes exactly.
Momentum closes in `0/84`.

## Registered measurements

| Dressing | Kinematic | Momentum | Work RMS | Momentum-residual RMS | Minimum residual | Maximum residual |
|---|---:|---:|---:|---:|---:|---:|
| off | 42/42 | 0/42 | `4.98124e-4` | `0.00353528` | `0.00252360` | `0.00603825` |
| on | 42/42 | 0/42 | `4.98783e-4` | `0.00353452` | `0.00251989` | `0.00615513` |

The smallest mismatch remains more than nine orders of magnitude above the
`1e-12` gate. Initial dressing has negligible effect on the RMS mismatch.

## What the control establishes

The old/new Moore-cube union can carry a complete information-preserving field
transaction. The cyclic permutation acts on the engine's actual `J/W` values,
requires no provenance label, has exact finite support, and has an exact
inverse. Therefore FTD-0465's noninjectivity is not an unavoidable consequence
of locality or the 36-site support.

The permutation nevertheless supplies whatever field momentum follows from
reordering its inputs; it does not enforce the recoil selected by the
particle's energy change. Exact reversibility and energy balance do not imply
matter-field momentum reciprocity.

## Ontological consequence

A moving material coat cannot be just a transported field pattern. Its update
must be a coupled canonical transaction in which the manifestation's momentum
change and the field transformation are solved together. Geometry can choose
where an event acts, but it does not determine the exchange magnitude.

The nearly dressing-independent failure also removes the initial dressing as
the controlling defect. The missing ingredient is a reciprocal matter-field
vertex, not more background flux.

## Next gate

Formulate one simultaneous local constrained transaction on the same 36-site
support. Start from the injective permutation and solve a preregistered
minimum-norm paired `J/W` correction together with the outgoing particle
momentum, imposing total energy and central-generator momentum in the same
system. The construction must provide an inverse from native post-state data;
an observer-stored correction is inadmissible. No production change follows
unless that joint map exists, is unique under a stated principle, and composes
sequentially.
