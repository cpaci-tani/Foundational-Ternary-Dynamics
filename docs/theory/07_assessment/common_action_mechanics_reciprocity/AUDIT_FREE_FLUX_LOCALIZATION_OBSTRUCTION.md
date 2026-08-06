# FTD-0557 Audit — Free-Flux Localization Obstruction

**Status:** [AUDIT — FREE-FLUX LOCALIZED CARRIER CLOSED NEGATIVE; NONLINEAR CARRIER OPEN]  
**Date:** 2026-07-26  
**Run of record:** Windows MSVC 14.44 Release, explicitly forced CPU

## Audit conclusion

The frozen isolated native `(flux,wave_vel)` operator transports extended
Bloch packets but has no nonzero square-summable stationary state and no
nonzero square-summable finite-time rigid translate on the infinite lattice.
The locked finite CPU realization agrees with the exact Bloch solution and
shows positive packet broadening without producing any ternary manifestation.

The result closes only the proposal that a free native flux packet is already
a localized particle.  It does not close nonlinear, defect-bound,
topological, or manifested composite carriers.

## Claims independently checked

1. **Frozen multiplier — passed by source inspection.**  The `FULL`-stencil
   `phase_read` acceleration and default `phase_write` kick-drift yield the
   FTD-0556 matrix `U(k)`.
2. **Nonconstant analytic band — passed analytically.**  The multiplier is a
   finite trigonometric polynomial, `M(0)=0`, and `M(k,0,0)>0` near but away
   from zero.
3. **No `l2` point spectrum — passed analytically.**  A putative eigenstate is
   supported on the zero set of a nonzero real-analytic determinant, which has
   measure zero.
4. **No finite-time rigid `l2` translate — passed analytically.**  The same
   zero-set argument applies to `U(k)^T-exp(i phi-i k dot d)I`; an orthogonal
   momentum line proves the determinant is not identically zero.
5. **Second-moment law — passed analytically.**  Fourier differentiation gives
   `X(t)=X(0)+t grad(theta)` and the exact covariance formula.
6. **Production replay — passed.**  Three principal-direction packets replay
   for 16 ticks with maximum `(J,W)` residual `1.6013e-16`.
7. **Broadening — passed.**  Every registered packet has positive group-speed
   variance; the maximum observed variance increase is `1.9157`.
8. **No hidden manifestation — passed exactly.**  Zero nonzero `s` sites were
   observed in all 48 arm-ticks.

## Frozen phase audit

The exact production locations are:

- `engine/src/render_bridge_phases/phase_read.cpp`, `phase_read_main_loop`:
  linear `C_WAVE^2 laplacian(J)` plus optional one-way state source;
- `engine/src/render_bridge_phases/phase_write.cpp`, lines 174--273:
  kick-drift integration plus separately gated damping/noise;
- `engine/src/render_bridge_phases/phase_movement.cpp`, lines 104--375:
  stored-velocity remainder accumulation, threshold hop, collision, and
  bounded self-field carry;
- `engine/src/render_bridge_phases/phase_forces.cpp`, lines 71--255:
  independently selected/imposed force branches and momentum update;
- `engine/src/scenarios/flux.cpp`, lines 85--93: `flux-soliton` is explicitly a
  dispersion diagnostic whose frozen wave sector has no soliton nonlinearity.

This call graph contains no reaction-free common-action term that binds a
manifested core to its complete moving field dressing.

## Finite-volume qualification

The no-point-spectrum theorem is an infinite-lattice statement.  A finite
periodic lattice has a discrete set of extended Bloch eigenvectors.  Their
normalizability in finite volume is not localization: their participation
volume grows with the box and they do not supply a finite-energy localized
thermodynamic-limit carrier.

The locked `L=65` replay is therefore not used to infer absence from a finite
sample.  It verifies that the production engine realizes the operator used in
the theorem and that the registered localized packets broaden as predicted.

## Epistemic disposition

| Statement | Status after FTD-0557 |
|---|---|
| Infinite-lattice free operator has no nonzero `l2` eigenstate | [THEOREM] |
| It has no nonzero finite-time rigid `l2` translate | [THEOREM] |
| Unchirped one-band width obeys the ballistic variance law | [THEOREM] |
| Frozen CPU engine realizes the registered dispersive packets | [NUMERICAL FACT] |
| `flux-soliton` is a particle/soliton | [CLOSED NEGATIVE — ISOLATED FREE CONFIGURATION] |
| Frozen nonlinear `(s,J,W)` map has no stable carrier | [OPEN — NOT PROVED HERE] |
| Native matter pole or common cone exists | [OPEN — unchanged] |

## Next gate

FTD-0560 subsequently closes the simplest manifested candidate: a single
periodically hopping point polarity cannot carry a square-summable linear
native dressing because every finite hop period has nonzero on-shell forcing.
The admissible next test is therefore an *extended or nonlinear* stable carrier
campaign only after a candidate is specified without an imposed mass clock or
legacy force law.
It must track manifested core, co-moving dressing, radiation leakage, energy,
translation covariance, finite-volume scaling, and survival across seeds.
The historical Phase-B amplitude islands do not satisfy that entry condition:
the canonical record retracts their finite-size and multi-cluster
interpretations, and no nontrivial `L`-invariant bound cluster survived the
registered campaigns.
