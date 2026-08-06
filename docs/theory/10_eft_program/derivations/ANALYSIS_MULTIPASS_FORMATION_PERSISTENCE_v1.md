# FTD-0731 — Multi-pass formation persistence v1

**Status:** `[SELECTED DYNAMICS + MEASURED — FINITE-HORIZON TWO-VOLUME RADIATIVE-CAPTURE WITNESS]`  
**Verdict:** `MULTIPASS_RADIATIVE_CAPTURE_VOLUME_STABLE`  
**Production status:** unchanged

## Result

All 48 locked `L=33/65`, 192-forward/192-reverse histories pass the
common-action, energy, recoil, state-only inverse, lower-energy parent, and
pre-bound control gates. Every initially unbound `p=0.0120` arm satisfies the
pre-registered durable multi-pass capture classifier.

```text
histories executed / identity / inverse / recoil       48 / 48 / 48 / 48
p=0.0120 durable captures on L=33 / L=65                 6 / 6
lower-energy persistent parents on L=33 / L=65          12 / 12
pre-bound controls on L=33 / L=65                         6 / 6
transition-count / timing / final-class mismatches        0 / 0 / 0
maximum matched transition-tick difference                    0
maximum common-action residual                       9.772e-14
maximum recoil defect                                3.834e-14
maximum state-only inverse recovery                  5.087e-11
maximum pair-plus-field energy defect                1.129e-14
```

The `p=0.0120` transition sequences remain exactly cubic and exactly matched
across volumes and polarity orders:

| ray class | direction | entry | exit | re-entry | transitions through 192 |
|---|---|---:|---:|---:|---:|
| face | `0_0_1` | 7 | 26 | 63 | 3 |
| edge | `0_1_-1` | 7 | 26 | 79 | 3 |
| body diagonal | `1_1_1` | 7 | 26 | 96 | 3 |

No fourth transition occurs. For every `p=0.0120` arm, graph membership is
inside and pair internal energy is negative at every tick `129--192`. Across
all arms that tail lies in

```text
-1.8752474097e-3 <= E_pair <= -2.5953616030e-4.
```

The field gains `1.4756895e-3--2.0094281e-3` while exact pair-plus-field
balance is retained. The dynamic field is nonzero, magnetically active, and
extended. Its doubled median radius at ticks `48,96,128,160,192` is:

| ray class | radius history |
|---|---|
| face | `5, 3, 5, 7, 10` |
| edge | `5, 6, 4, 6, 8` |
| body diagonal | `5, 12, 4, 5, 7` |

Thus the receiver is not an absent or permanently compact field. It evolves
while the negative relational core persists.

## Interpretation

The FTD-0722 single-pass capture classifier correctly remained negative: the
pair first enters and exits. FTD-0731 qualifies a different mechanism under a
freshly locked classifier. Energy exported to the matched face/edge field is
followed by a direction-timed re-entry into the selected negative-energy
graph sector, after which no release is observed through tick 192.

The correct statement is therefore:

> the selected two-constituent common-action dynamics admits a reversible,
> energy-balanced, two-volume-stable, finite-horizon radiative-capture
> witness after a multi-pass encounter.

This is stronger than energetic trapping and stronger than re-entry alone.
It is not a theorem of the five FTD postulates because the compact `C1` pair
well and this common-action branch remain selected dynamics.

## Ontological consequence

The minimal matter candidate is now a process, not a voxel or rigid pair:

1. two opposite ternary manifestations carry constituent phase space;
2. their causal movement deposits oriented face current;
3. the matched face/edge field receives and stores encounter energy;
4. a derived relational graph identifies the negative core sector;
5. the complete core-field state determines the later re-entry and state-only
   inverse.

No new primitive is required by this witness. The field is constitutive of
the captured object but is not a literal strand, photon, pilot wave, wake, or
aura. Those interpretations require separate morphology and propagation
discriminators.

## Limits and next gate

The result does **not** establish an open basin of attraction, asymptotic or
infinite/open-volume stability, a translating composite, particle poles,
mass, charge, spin, statistics, quantum unitarity, or production adoption.
The periodic volumes and finite 192-tick horizon remain material limitations.

The next priority is a preregistered local stability/basin test around the
qualified captured state. It must perturb relative position, radial and
transverse momenta, and divergence-free dynamic-field amplitude without
retuning the action. A finite cross of successful arms is evidence for local
robustness; only a certified open neighborhood can support a stable-matter
claim.

## Verification anchors

- protocol `F319B4CA…C01EE`;
- runner `CE40EFAC…5266`;
- JSON `0D4F8519…F03D`;
- CSV `BC060706…F163`, including all 193 scalar states per history;
- independent certificate `2894C516…17E1`, `583/583 PASS`;
- focused CTest `1/1 PASS` in `2008.31 s`.

