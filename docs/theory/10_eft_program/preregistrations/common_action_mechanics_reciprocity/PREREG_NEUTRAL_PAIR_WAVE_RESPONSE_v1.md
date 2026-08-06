# PRE-REGISTRATION — Neutral-pair wave response v1

**Date locked:** 2026-07-24  
**Identifier:** `FTD-0436`  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Parent:** `FTD-0435` flux/self-field interference response  
**Engine artifact:** `engine/tests/campaign_neutral_pair_wave_response.cpp`  
**Artifact SHA256:** `a546e5ec38fd4cc0df9f13b0f367e83f35c278ed0d1f8f569b9b8d65fdf1654a`

## 1. Question

FTD-0435 found that the selected production force

$$
F_s=G_Cs\,\nabla|J|
$$

drives the dominant transverse response in the same direction for isolated
positive and negative manifested sites. FTD-0436 tests the operational
consequence for a neutral pair:

> Does an incident transverse wave translate the pair in common mode, or
> polarize it by moving the two signs oppositely?

Ordinary electric `qE` response requires a polarity-odd transverse force and
therefore pair polarization. A polarity-even self-field/interference response
predicts common translation. This campaign does not add a binding force,
change the production tick, or claim that the pair is an atom.

## 2. Frozen protocol

| Quantity | Value |
|---|---:|
| lattice | periodic `L=33` |
| ticks | `200` |
| incident wave | `+x` travelling, `y` polarized |
| harmonic | `n=4` |
| amplitude | `A=0.05` |
| pair | one `+1` and one `-1` manifested site |
| initial separation | `8` lattice sites |
| pair axes | `y` and `z` |
| RNG seed | `4360` |
| repeat gate | `1e-12` |
| response gate | `1e-8` |

Both pair orientations have the same `x` coordinate, so the two signs sample
the same incident-wave phase. Enabled terms are exactly `wave_propagation`,
`coupling`, `forces`, `movement`, and `emergent_forces` with
`strict_validation=true`. All Gauss, Poisson, Lorentz, gravity, damping,
reaction, colour, dual-substrate, and other Boolean extensions are disabled.

For each pair orientation the arms are:

1. `pair_only`;
2. `pair_plus_wave`;
3. an exact repeat of `pair_plus_wave`.

A single `wave_only` arm supplies the energy counterfactual. Particle IDs track
both signs through production movement. Opposite-sign collision/annihilation
is not disabled because it is part of the frozen movement rule.

## 3. Estimators

At each tick the production `f_coulomb` diagnostic is recorded at each
particle's pre-movement site. Pair-only histories are subtracted sign by sign:

$$
\Delta F_\pm(t)=F_{\pm,\,pair+wave}(t)-F_{\pm,\,pair}(t).
$$

The common and polarizing channels are

$$
F_C(t)=\frac{\Delta F_+(t)+\Delta F_-(t)}{2},\qquad
F_P(t)=\frac{\Delta F_+(t)-\Delta F_-(t)}{2}.
$$

The primary transverse common fraction is

$$
f_C^y=\frac{\mathrm{RMS}(F_C^y)}
{\mathrm{RMS}(F_C^y)+\mathrm{RMS}(F_P^y)}.
$$

The full-vector common fraction replaces component RMS by vector RMS. Paired
center-of-mass displacement and half the relative displacement are reported as
trajectory-level versions of the same decomposition. The minimum pair
separation and survival of both particle IDs are recorded.

The accounted-energy diagnostic is the normalized inclusion-exclusion drift

$$
[\Delta E_{pair+wave}-\Delta E_{pair}-\Delta E_{wave}]
/(|E_{pair}(0)|+|E_{wave}(0)|).
$$

As in FTD-0435, this uses the incomplete `dynamic_energy` audit and cannot by
itself establish fundamental conservation or nonconservation.

## 4. Locked outcomes

All symmetry outcomes require finite values, both particle IDs surviving,
RMS transverse response above `1e-8`, forbidden toggles off, and full repeat
disagreement at or below `1e-12`.

### Outcome C — `COMMON_MODE_NEUTRAL_TRANSLATION`

`f_C^y>=0.80` for both pair axes. The selected force acts predominantly as a
common wave/medium force on the neutral pair, not as an electric polarizer.

### Outcome E — `ELECTRIC_LIKE_PAIR_POLARIZATION`

`f_C^y<=0.20` for both pair axes. The selected force has the transverse
sign-odd behavior required of an electric-force candidate in this protocol.
No gauge, normalization, radiation, or atomic claim follows.

### Outcome M — `MIXED_ORIENTATION_DEPENDENT_PAIR_RESPONSE`

The response is resolved but neither Outcome C nor E passes for both axes.
No single common-mode or electric-polarization identification is licensed.

### Outcome A — `PAIR_ANNIHILATION_OR_LOSS`

One or both tracked particle IDs disappear in any registered pair arm. This is
a valid movement-sector outcome but the force-symmetry estimator is
inadmissible.

### Outcome N — `NO_RESOLVED_PAIR_RESPONSE`

The transverse response remains at or below `1e-8` for either orientation.

### Outcome X — `INVALID_PROTOCOL`

Any nonfinite value, forbidden toggle/backend violation, incomplete history, or
repeat residual above `1e-12` invalidates the campaign.

## 5. Secondary labels

- `GLOBAL_COMMON_DOMINANT` iff the full-vector common fraction is at least
  `0.80` for both orientations.
- `COM_TRANSLATION_DOMINANT` iff the common displacement magnitude exceeds
  half the relative-displacement magnitude for both orientations.
- `ACCOUNTED_ENERGY_CLOSED` iff both normalized energy residuals are at most
  `1e-6`; otherwise `ACCOUNTED_ENERGY_OPEN`.

## 6. Banned moves

- No separation, orientation, amplitude, duration, gate, or wave-phase change
  after first execution.
- No binding, Poisson, Lorentz, diagnostic `qE`, Gauss, damping, or reaction
  toggle may be added under this identifier.
- No reinterpretation of common translation as electric polarization.
- No atomic, photon, Thomson, pilot-wave, or empirical-charge claim.
