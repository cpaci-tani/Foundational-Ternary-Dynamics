# FTD-0732 — Captured-state perturbation survival v1

**Status:** `[EXECUTION UNRESOLVED — LOCKED CROSS CONTAINS INADMISSIBLE COMPRESSION PROBE]`  
**Verdict:** `CAPTURE_PERTURBATION_TRANSACTION_UNRESOLVED`  
**Production status:** unchanged

## Locked result

The full 84-history campaign cannot pass its registered execution gate. All
six `L=33` `separation_minus` variants satisfy Gauss, total-momentum, graph,
and causal constraints but start with **positive** pair internal energy. The
protocol requires every perturbed initial state to remain in the negative
captured sector and states that an invalid variant fails the campaign rather
than being replaced.

| direction | compressed separation | compressed pair energy |
|---|---:|---:|
| face `0_0_1` | `0.9032932101` | `+0.0029027563` |
| edge `0_1_-1` | `0.9158135603` | `+0.0025363780` |
| body `1_1_1` | `0.8960388562` | `+0.0040179626` |

Both polarity orders are identical. The largest initial Gauss residual among
these rejected arms is `1.629e-13`; momentum preservation is exact and maximum
constituent speed is `0.12835 < 1/sqrt(3)`. The defect is therefore the locked
energy-sector admissibility condition, not field inconsistency or acausality.

## Subordinate observations

The unresolved aggregate verdict must not erase the behavior of the
admissible records:

```text
admissible histories executed and survived                 78 / 78
L=33 valid perturbation survivors                           60 / 60
held-out L=65 hostile confirmations                         18 / 18
unperturbed centers through parent tick 384                 12 / 12
polarity / volume class mismatches                            0 / 0
maximum common-action residual                           9.772e-14
maximum recoil defect                                    3.248e-14
maximum state-only inverse recovery                      3.778e-11
maximum pair-plus-field energy defect                    1.458e-14
```

All 78 admissible histories remain graph-inside and below `-1e-6` for every
one of their 257 stored continuation states, have no graph transition, and
recover the perturbed initial state through a 256-step state-only inverse.

The predeclared hostile selector independently chooses
`radial_impulse_plus` as the least energy-margin arm and
`dynamic_field_minus` as the distinct least graph-margin arm for every
direction and polarity. Both reproduce on `L=65`. The smallest observed valid
energy margin is `0.0043452` in units of the `0.01` well depth; the smallest
graph margin is `0.0248627` lattice units.

These are qualified subordinate measurements. They do not change the locked
verdict and do not constitute a passed finite cross.

## Interpretation

The fixed 5% coordinate cross was not wholly inside the captured energy
sector. It therefore mixed two questions:

1. whether an already admissible captured state survives a perturbation; and
2. where the selected compact pair model's inner energy boundary lies.

The first question is positive for every admissible arm tested. The second is
also exposed qualitatively: a 5% inward separation change crosses the inner
zero-energy boundary before evolution begins. Under the locked map, however,
this is `CAPTURE_PERTURBATION_TRANSACTION_UNRESOLVED`, not
`CAPTURE_FINITE_PERTURBATION_BOUNDARY_WITNESSED`, because the latter requires
valid initialized variants that fail later.

## Ontological consequence

The candidate core is not arbitrarily compressible. Its captured sector has
an inner energetic boundary in relative configuration space. This supports a
finite-extent phase-space object picture, but the boundary currently comes
from the **selected** compact potential and cannot be called a derived
physical size.

No new primitive is indicated. The admissible momentum and dynamic-field
perturbations remain uniquely executable, energy balanced, volume matched,
polarity mirrored, and state-only invertible. The next question is not hidden
state but correct parameterization of the admissible neighborhood.

## Next gate

Derive the connected negative-energy radial interval directly from the fixed
potential

```text
V(d) = -16 D (d-3/2)^2 (d-3/4),  d=r^2<3/2,
```

and each frozen tick-128 kinetic energy. Preregister interior fractions of
that interval before testing mixed momentum/field/radial corners. Do not tune
a smaller percentage to recover a desired pass.

## Verification anchors

- protocol `1A93899A…0903`;
- runner `4D706C2A…5A4E`;
- JSON `508EAB61…2B09`;
- CSV `15926F9E…E2AD`, including all valid 257-state histories and rejected
  initial records;
- independent certificate `5AED00E0…04F8`, `1469/1469 PASS`;
- focused CTest `0/1` by design of the locked invalid-variant gate,
  `2610.96 s`.

