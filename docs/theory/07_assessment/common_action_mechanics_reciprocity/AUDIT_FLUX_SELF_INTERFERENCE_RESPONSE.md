# FTD-0435 — Flux/self-field interference response

**Date:** 2026-07-24  
**Status:** `[MEASURED — SELECTED FLUX-GRADIENT FORCE]`  
**Verdict:** `MIXED_POLARITY_RESPONSE`  
**Electric-force identification:** `[CLOSED NEGATIVE — THIS FORCE LAW/PROTOCOL]`

## 1. Question and scope

FTD-0288 found deterministic recoil when the selected production
`emergent_forces` extension was enabled. FTD-0435 asked whether that response
has the polarity symmetry required of an electric force, and whether its
direction and scaling support a simple external flux-magnitude-gradient
interpretation.

The tested force remains exactly

$$
F_s=G_Cs\,\nabla |J|_{r=2}.
$$

This is a selected EFT extension in the production engine, not a consequence
of the five frozen postulates. The campaign added no dynamics.

## 2. Locked execution

The hash-locked v1 campaign ran a periodic CPU lattice at `L=33` for 200 ticks.
It compared both source polarities under `linear_y`, `linear_z`, and
equal-energy constant-magnitude `circular_yz` travelling waves at amplitudes
`0.025, 0.05, 0.10`. Source-only and wave-only counterfactuals were subtracted.
The baseline combined arms were repeated exactly.

All finite, source-survival, forbidden-toggle, forced-CPU, detection, and
repeat gates passed. Both complete executions returned zero repeat residual.
The campaign and the two predecessor regressions pass `3/3` under CTest.

## 3. Primary polarity result

At the baseline `linear_y, A=0.05`, the paired displacements were

$$
d_+=(-0.0315481960,-0.1667443769,-0.0002034136),
$$

$$
d_-=(+0.0315481960,-0.1667443769,+0.0002034136).
$$

Their exact even/odd decomposition is therefore

$$
d_{\rm even}=\frac{d_++d_-}{2}
\simeq(0,-0.1667443769,0),
$$

$$
d_{\rm odd}=\frac{d_+-d_-}{2}
\simeq(-0.0315481960,0,-0.0002034136).
$$

The same split occurs in the integrated force: the dominant polarization-axis
component is polarity-even, while the smaller propagation-axis and residual
orthogonal components are polarity-odd. Consequently neither preregistered
pure symmetry outcome passes:

| estimator | result | pure-outcome gate |
|---|---:|---:|
| force odd residual | `1.3771045871` | `<=0.10` |
| force even residual | `0.3218430613` | `<=0.10` |
| displacement odd residual | `1.3895601819` | `<=0.20` |
| displacement even residual | `0.2629115838` | `<=0.20` |

The locked verdict is `MIXED_POLARITY_RESPONSE`.

## 4. Mechanism inference

For `J=J_self+J_wave`, the weak-wave expansion is

$$
|J|=|J_{\rm self}|+
\widehat J_{\rm self}\cdot J_{\rm wave}+O(J_{\rm wave}^2).
$$

Because the native source term makes `J_self` reverse with primitive polarity,
the outer factor `s` in the force cancels that reversal in the leading
interference term. A polarity-even transverse component is therefore expected.
The measured decomposition realizes that prediction.

This inference receives two independent internal checks:

- `98.2567%` of each baseline displacement direction lies on the incident
  polarization axis, not the propagation axis;
- the constant-`|J_wave|` circular arm retains `0.996758` of the equal-energy
  linear arm's RMS force for either polarity.

The circular result rules out the description “the particle merely follows an
external `grad |J_wave|` hill.” Its external magnitude is spatially constant.
The response instead depends on the total local field containing the source
halo and the incident wave.

## 5. Secondary outcomes

### Cubic rotation

Rotating the linear polarization from `y` to `z` rotates the full force history
with residuals `6.59e-15` and `8.09e-15`. `CUBIC_ROTATION_PASS` establishes
implementation-level cubic covariance for this protocol.

### Amplitude scaling

The RMS-force exponents are `0.1077094` for both polarities, outside the locked
linear and quadratic windows. Displacement is also nonmonotonic across the
three amplitudes. The 200-tick mobile response is therefore a nonlinear,
trajectory-fed response rather than a simple `A` or `A^2` law. This does not
contradict the instantaneous algebraic linearity of `grad |J|`; the measured
object and its self-field move during the campaign.

### Accounted energy

The maximum normalized inclusion-exclusion residual is `5.3914e-5`, failing
the `1e-6` diagnostic gate. The verdict is `ACCOUNTED_ENERGY_OPEN`. The current
`dynamic_energy` audit explicitly omits interaction energies, so this is not a
proof of fundamental nonconservation. It does establish that the existing
audit cannot yet close the selected force/wave/source exchange.

## 6. Correct statement

> The selected `G_C s grad|J|` production extension produces a deterministic,
> cubically rotating light/matter response mediated by the total source-plus-
> wave field. Its dominant transverse component is polarity-even, while a
> smaller longitudinal component is polarity-odd. It is therefore neither a
> Bohmian guidance law nor ordinary electric `qE` coupling. The current data
> support the narrower description **self-field-mediated flux interference**.

This closes only the ordinary-electric identification of this force law in the
locked protocol. It does not close other native force constructions and does
not establish a photon, scattering cross-section, radiation law, gauge
symmetry, or empirical coupling normalization.

## 7. Artifacts

- preregistration:
  `docs/theory/10_eft_program/preregistrations/PREREG_FLUX_SELF_INTERFERENCE_RESPONSE_v1.md`
- campaign: `engine/tests/campaign_flux_self_interference_response.cpp`
- run record: `engine/results/ftd_0435/windows_msvc_cpu_L33.csv`
- manifest: `engine/results/ftd_0435/manifest.json`
- source SHA256:
  `246cf58c01008cce02420a7938990d874624890095051b11d8601a7918dec67d`

