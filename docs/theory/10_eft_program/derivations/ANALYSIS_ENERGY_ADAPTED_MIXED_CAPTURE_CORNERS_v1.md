# FTD-0734 — Energy-adapted mixed capture corners v1

**Status:** `[SELECTED DYNAMICS + NUMERICAL FACT — FINITE MIXED-CORNER ROBUSTNESS]`  
**Verdict:** `CAPTURE_ENERGY_ADAPTED_MIXED_CORNERS_SURVIVE`  
**Production status:** unchanged

## Locked result

All 216 registered histories initialize, execute, remain captured through the
256-tick continuation from parent tick 128 to tick 384, and recover their
initial complete state under the 256-step state-only inverse.

```text
L=33 simultaneous mixed histories                         198 / 198
held-out L=65 hostile confirmations                        18 / 18
unperturbed center controls                                12 / 12
polarity / volume class mismatches                           0 / 0
maximum common-action residual                          9.772e-14
maximum recoil defect                                   3.332e-14
maximum state-only inverse recovery                     4.157e-11
maximum pair-plus-field energy defect                   1.430e-14
minimum exact shell margin in d                         0.00323068
minimum observed energy margin in units of D            0.00535958
minimum observed graph margin                           0.0250551
```

Every mixed arm changes all of the following at once:

1. radial relative momentum;
2. two transverse relative-momentum components;
3. relative separation, placed at one half of the nearest exact
   kinetic-dependent energy-shell margin; and
4. the divergence-free dynamic electric and magnetic residual, scaled by
   `0.95` or `1.05`.

For each direction and polarity the complete Stage-A set contains all 32
registered sign/radial/field corners plus the center. No corner was selected
from output. The independent certificate reconstructs every energy root,
half-margin radial point, history class, hostile selector, polarity pairing,
volume comparison, and aggregate gate.

## What changed relative to FTD-0732

FTD-0732 used a Cartesian percentage box. Its inward 5% position arm crossed
the selected potential's inner zero-energy surface before evolution, making
the aggregate campaign unresolved. FTD-0733 proved that the admissible radial
domain depends on kinetic energy. FTD-0734 therefore uses the exact local
phase-space geometry rather than shrinking an arbitrary percentage until it
passes.

The positive result shows that the prior failure was a domain-coordinate
defect, not evidence that the selected captured core is fragile under every
inward perturbation. Position, momentum, and field amplitude cannot be varied
as independent rectangular coordinates near a finite-depth well.

## Ontological consequence

Within the selected common-action branch, the captured object behaves as a
finite region of **relational phase space**, not as one occupied site and not
as a purely spatial shape. Its identity survives simultaneous changes to
relative position, all relative-momentum axes, and dynamic field amplitude
provided the initial complete state remains inside the exact energy shell.

No additional primitive is required for this measured robustness. Existing
constituent position/momentum, polarity, relational interaction, and matched
face/edge field state determine every tested continuation and inverse.

This does not prove that the selected compact potential is postulate-native,
that its constituents are fundamental, or that a physical particle has been
derived. It does support the narrower matter picture:

> a localized matter candidate is a persistent complete-state orbit occupying
> a bounded region of relational matter--field phase space; its instantaneous
> ternary-site pattern is one representation of that orbit.

## Strict scope

The verdict is finite-direction, finite-amplitude, finite-volume, and
finite-horizon evidence. It does **not** establish:

- an invariant open basin or asymptotic attractor;
- irreversible stability in a finite reversible periodic system;
- generic formation from arbitrary incoming states;
- two-object separability, scattering, or reaction-complete charge;
- a pole, mass, spin, statistics, species identity, or infrared Lorentz cone;
- the selected pair well as a consequence of the five postulates.

The compatible long-run ontology is bounded recurrence in a closed finite
sector or persistent localization with excess field energy exported into an
effectively uncontained environment. A dissipative attractor cannot be
inferred from exact finite-sector reversibility.

## Next admissible gate

The next decisive question is no longer another coordinate corner. It is
whether the captured family has a **state-only finite-time neighborhood** and
remains localized when outgoing field energy is allowed to leave a growing
causal buffer without periodic return. The registered finite corner set may
seed such a validated-neighborhood/outgoing-environment protocol; it may not
be renamed an open basin by sampling density alone.

## Verification anchors

- protocol `E2F4F928…D2251C3`;
- runner `3F29678D…FF936F4`;
- JSON `41E0FB2E…6998889`;
- CSV `FCB930BE…8BEF947`;
- independent certificate `3F46ADE2…5AC2C07`, `6299/6299 PASS`;
- production tick, defaults, toggles, and scenarios unchanged.

