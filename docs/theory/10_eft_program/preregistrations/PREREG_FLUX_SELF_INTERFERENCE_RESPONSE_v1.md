# PRE-REGISTRATION — Flux/self-field interference response v1

**Date locked:** 2026-07-24  
**Identifier:** `FTD-0435`  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Parent:** `FTD-0288` unlocked flux-gradient recoil  
**Engine artifact:** `engine/tests/campaign_flux_self_interference_response.cpp`  
**Artifact SHA256:** `246cf58c01008cce02420a7938990d874624890095051b11d8601a7918dec67d`  

## 1. Question

The production `emergent_forces` extension applies

$$
F_s=G_C s\,\nabla |J|,
\qquad J=J_{\rm self}+J_{\rm wave}.
$$

FTD-0288 measured a deterministic mobile-source response whose displacement
was predominantly parallel to the incident wave polarization rather than its
propagation direction. This campaign tests the narrower mechanism claim:

> Is the response charge-odd as required for an electric `qE` interpretation,
> or polarity-even as predicted by the leading self-field/interference term?

The campaign also measures amplitude scaling, cubic polarization rotation,
constant-magnitude circular-wave response, force direction, and the closure of
the currently accounted dynamic-energy audit. It does not modify the tick,
derive electromagnetism, establish a photon, or fit a physical constant.

## 2. Frozen production sector

All arms use the unmodified production `RenderBridge` CPU tick with:

| Setting | Value |
|---|---:|
| lattice | periodic `L=33` |
| ticks | `200` |
| wave direction | `+x` |
| harmonic | `n=4` |
| amplitudes | `0.025, 0.05, 0.10` |
| manifested source | one mobile site at the lattice center |
| source polarity | `s=+1` and `s=-1` |
| RNG seed | `4350` |
| repeat gate | `1e-12` |
| response gate | `1e-8` |

Enabled terms are exactly `wave_propagation`, `coupling`, `forces`, `movement`,
and `emergent_forces`. Damping, Gauss projection, matched Gauss dynamics,
Poisson Coulomb, Lorentz force, gravity, dual substrate, genesis, evaporation,
pair production, weak transmutation, colour forces, and every other Boolean
extension are disabled. `strict_validation=true`.

The three incident-wave families are:

1. `linear_y`: `J_y=A sin(kx)`;
2. `linear_z`: `J_z=A sin(kx)`;
3. `circular_yz`: `J_y=(A/sqrt(2)) sin(kx)` and
   `J_z=(A/sqrt(2)) cos(kx)`.

Each component receives the exact discrete travelling-wave velocity used by
FTD-0288. Linear and circular arms therefore have equal spatial mean
`|J_wave|^2=A^2/2`; the circular arm has spatially constant `|J_wave|`.

## 3. Counterfactual arms and estimators

For each polarity, a `source_only` arm is run once. For each wave family and
amplitude, a `wave_only` arm is run once. Every polarity/wave/amplitude
combination then runs a `source_plus_wave` arm. The two baseline
`linear_y, A=0.05` combined arms are repeated exactly.

At every tick the campaign records the production `f_coulomb` diagnostic at
the particle's pre-movement site. The wave-induced force history is the paired
difference

$$
\Delta F_s(t)=F_{s+\mathrm{wave}}(t)-F_{s\mathrm{only}}(t).
$$

It also records paired displacement, final velocity, RMS and integrated force,
maximum speed, accounted dynamic energy, and source survival. No fitted or
smoothed trajectory is used.

For the baseline linear wave define

$$
r_{\rm odd}^F=
\sqrt{\frac{\sum_t|\Delta F_+(t)+\Delta F_-(t)|^2}
{\sum_t(|\Delta F_+(t)|^2+|\Delta F_-(t)|^2)}},
$$

and define `r_even^F` by replacing the sum in the numerator with a difference.
The displacement residuals use the analogous Euclidean-vector normalization.

Amplitude exponents are ordinary least-squares slopes of
`log(rms_force)` against `log(A)` over the three locked amplitudes. Rotation
residuals compare `linear_z` with the exact cubic rotation
`R_x(+pi/2) linear_y`. Energy closure is reported as the inclusion-exclusion
drift

$$
\delta E_{\rm int}=
[\Delta E_{s+w}-\Delta E_s-\Delta E_w]
$$

using the existing `dynamic_energy` audit. Because that audit explicitly omits
some interaction energies, this is a diagnostic and not a validity gate.

## 4. Locked outcomes

All physics outcomes require finite values, source survival, a baseline value
of `max(rms_force, |paired displacement|)>1e-8` for both polarities, and repeat
disagreement at or below `1e-12`.

### Outcome O — `CHARGE_ODD_RESPONSE`

- `r_odd^F <= 0.10` and displacement odd residual `<=0.20`;
- the corresponding even residuals do not also pass.

Correct statement: the selected flux-gradient response has the polarity
symmetry required of an electric-force candidate in this protocol. Its
normalization, energy closure, gauge structure, and empirical adequacy remain
unestablished.

### Outcome E — `POLARITY_EVEN_SELF_INTERFERENCE`

- `r_even^F <= 0.10` and displacement even residual `<=0.20`;
- the corresponding odd residuals do not also pass.

Correct statement: the incident-wave response is even under source-polarity
reversal and therefore cannot be identified with ordinary `qE` coupling.
Self-field/interference is the supported mechanism-level interpretation.

### Outcome M — `MIXED_POLARITY_RESPONSE`

Neither Outcome O nor E passes. The response contains material even and odd
components; no electric-force identification is licensed.

### Outcome N — `NO_RESOLVED_RESPONSE`

Either baseline polarity response is at or below `1e-8`. The protocol has not
resolved the symmetry and no mechanism conclusion follows.

### Outcome X — `INVALID_PROTOCOL`

Any nonfinite value, lost source, forbidden toggle, backend mismatch, or repeat
residual above `1e-12` invalidates the campaign.

## 5. Secondary classifications

- `LINEAR_AMPLITUDE` iff both baseline-polarity exponents lie in `[0.8,1.2]`.
- `QUADRATIC_AMPLITUDE` iff both lie in `[1.8,2.2]`.
- otherwise `NONPOWER_OR_MIXED_AMPLITUDE`.
- `CUBIC_ROTATION_PASS` iff both polarity force-history rotation residuals are
  `<=0.10`; otherwise `CUBIC_ROTATION_FAIL`.
- `TRANSVERSE_POLARIZATION_DOMINANT` iff the baseline displacement satisfies
  `|d_y|/|d|>=0.90` for both polarities.
- `ACCOUNTED_ENERGY_CLOSED` iff every normalized inclusion-exclusion drift is
  `<=1e-6`; otherwise `ACCOUNTED_ENERGY_OPEN`. This label applies only to the
  current audit channels.

Circular-wave response is reported as the ratio of its RMS force to the
equal-energy linear-wave RMS force. No pass threshold is assigned.

## 6. Banned moves

- No amplitude, phase, duration, lattice size, source position, or threshold
  may change after the first execution.
- No Poisson, Lorentz, Gauss, diagnostic `qE`, damping, or reaction term may be
  enabled in v1.
- No physical charge, photon, Thomson, pilot-wave, or quantum claim may be made
  from a positive response.
- No failed polarity symmetry may be repaired by adding a force term under this
  identifier.
- No numerical target or measured physical constant enters the protocol.
