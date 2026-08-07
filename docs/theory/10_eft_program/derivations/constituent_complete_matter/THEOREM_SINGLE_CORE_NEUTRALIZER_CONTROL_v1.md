# FTD-0610 — Single-core neutralizer control v1

**Status:** `[MEASURED — EXACT COMMON ACTION AND STATE-ONLY INVERSION]` +
`[CLOSED NEGATIVE — EXTRACTED CORE IS NOT AN ISOLATED STATIC STATE]`
**Protocol:**
[`PREREG_SINGLE_CORE_NEUTRALIZER_CONTROL_v1.md`](../../preregistrations/constituent_complete_matter/PREREG_SINGLE_CORE_NEUTRALIZER_CONTROL_v1.md),
prefix SHA-256 `DB4363D2A132BB84BFF10218FCE8B4B20BC4C677F6FE813815F368E38A4EED85`
**Production status:** unchanged

## 1. Registered separation

The phase-15 trimer that carries every FTD-0609 shared-anchor event was
extracted without changing its constituents or shape. Its net charge `+1` was
neutralized in two external control environments:

- a uniform density `-1/L^3`, which removes the periodic zero mode without a
  localized neutralizer gradient;
- the exact opposite trimer coat frozen at its initial position.

Both total densities close to zero within `3.34e-14`. Their independently
initialized minimum-energy fields have Poisson, Gauss, and curl residuals
below `8.24e-16`, and integer-translation covariance is exact at state level.

## 2. Exact dynamical result

All 832 registered forward/reverse ticks converge. Worst common-action
residual is `2.35e-14`, maximum energy drift is `2.34e-15`, and worst
state-only recovery is `2.17e-13`. The trimer's pair distances remain within
`1.41399...1.41446`; the chart fibre reaches multiplicity two without
effective-position coincidence.

Thus current deposition, field update, recoil, binding work, energy exchange,
site crossing, and state-only inversion coexist for this three-constituent
core. The failure below is physical relative to the registered gates, not a
solver-coverage or representation failure.

## 3. Rest and boost result

| control | launch | longitudinal displacement | nominal | result |
|---|---:|---:|---:|---|
| uniform | rest, 16 ticks | `0.06473` | `0` | not static |
| uniform | `v=1/64` | `0.80715` | `2` | slow transport fails |
| uniform | `v=1/32` | `1.89357` | `2` | passes |
| frozen partner | rest, 16 ticks | `0.05928` | `0` | not static |
| frozen partner | `v=1/64` | `-0.22442` | `2` | reverses direction |
| frozen partner | `v=1/32` | `1.81945` | `2` | passes |

The uniform rest arm also changes centre momentum by `0.01220`. Therefore the
extracted phase-15 trimer is not a stationary solution of the single-core
uniform-neutralizer action. The locked verdict is

```text
SINGLE_CORE_STATIC_REFERENCE_NOT_ISOLATED
```

The frozen partner strongly changes the slow trajectory, but the protocol's
verdict hierarchy stops at the earlier uniform-rest failure. It is therefore
not licensed to assign the FTD-0609 slow failure solely to partner dynamics.

## 4. Matter-dynamics consequence

The selected compact core has a robust internal shape and a lossless atomic
transaction, but it has not yet supplied a rest state that can be meaningfully
boosted. Staticity is a property of the complete constituent-plus-field
environment, not of a trimer shape copied out of that environment.

The next discriminator must minimize the *single-core* energy in the uniform
neutralizer environment before applying a boost. If no interior stationary
single-core state exists, compact matter is exposed to an intrinsic lattice
self-force/Peierls landscape and the research mainline must move to a wider
low-momentum carrier. If one exists, its slow and fast boost response becomes
the clean compact-mobility test.

No isolated charged particle, closed momentum channel, production ontology,
scenario, pole, Lorentz recovery, or unitarity claim follows.
