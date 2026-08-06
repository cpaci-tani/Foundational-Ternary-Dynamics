# FTD-0556 Audit — Integer Translation and Bloch Transport

**Status:** [AUDIT — FREE-FLUX POSITIVE; MOBILE MATTER UNCHANGED OPEN]  
**Date:** 2026-07-26  
**Run of record:** Windows MSVC 14.44 Release, explicitly forced CPU

## Audit conclusion

The frozen isolated free-flux tick supports a nontrivial local Bloch band and a
continuously moving extended-packet centroid while retaining exact integer
translation covariance and a one-Moore-shell causal support bound.

This closes one false dilemma: discrete microscopic sites do not require an
observable extended wave centroid to jump by integer cells.  It does not close
the mobile-matter problem.  The tested object is the native `(flux,wave_vel)`
pair with `s=0`, not a stable manifested configuration.

## Claims independently checked

1. **Scalar type boundary — passed analytically.**  A scalar finite Laurent
   symbol with unit modulus is a phase times a monomial.  A dispersive local
   stable band therefore needs internal type, temporal memory, nonlocality, or
   nonunitarity.
2. **Production transfer map — passed.**  Direct inspection of
   `phase_read.cpp` and `phase_write.cpp` gives the kick-drift matrix
   `[[1-a,1],[-a,1]]` with `a=C_WAVE^2 M(k)`.
3. **Positive invariant — passed analytically and numerically.**  The map
   preserves `|W|^2+a|J|^2-a Re(J*W)` for `0<a<4`.
4. **Exact pole — passed.**  All nine registered modes obey
   `theta=2 asin(C_WAVE sqrt(M)/2)` and replay for 64 production ticks.
5. **Locality — passed exactly.**  The compact impulse remained bit-zero
   outside Chebyshev radius `t` for all six registered ticks.
6. **Integer translation covariance — passed.**  The translated compact
   histories agreed to `2.776e-17`.
7. **Continuous centroid — passed.**  The exact complex Bloch packet and CPU
   engine agreed to `9.326e-15` in centroid displacement; the one-tick shift
   was `0.565426363216128` cells.
8. **Cubic structure — passed analytically.**  The fourth-order symbol term is
   isotropic; anisotropy starts at sixth order in `M`.

## Protocol defect and disposition

The first executable invocation inherited the CUDA backend selected by the
`RenderBridge` constructor.  Because the preregistration froze CPU, that
invocation is `PROTOCOL_IMPLEMENTATION_INVALID` and has no evidential status.
The fixture was changed only to call `force_cpu()` and assert the backend.  The
corrected run recorded `13/13` CPU assertions and passed the original formulas,
samples, thresholds, and verdict rule.

## Epistemic disposition

| Statement | Status after FTD-0556 |
|---|---|
| Scalar finite-range unitary symbol is monomial | [THEOREM] |
| Isolated native `(J,W)` map has the stated pole and positive invariant | [THEOREM] |
| Frozen CPU tick realizes that map | [NUMERICAL FACT — exact replay] |
| Integer updates permit continuous extended-wave centroid motion | [THEOREM + CONSTRUCTIVE NUMERICAL FACT] |
| The native band is a photon | [SELECTION — unchanged] |
| The native band carries stable manifested matter | [OPEN — unchanged] |
| Matter and field share an infrared cone | [OPEN — unchanged] |

## Damage-prevention statement

FTD-0556 cannot be cited as evidence that a particle moved, that charge was
transported, that a mass pole exists, or that Lorentz symmetry recovered.  The
result removes a kinematic objection to discreteness; it does not supply the
missing nonlinear binding mechanism.  A free wavepacket generically disperses,
and a plane wave is not localized.

## Next gate

Search the frozen nonlinear production map for a localized finite-energy
spectral carrier whose manifested core, dressing, and conserved energy move
together.  The carrier must be identified without an imposed de Broglie clock,
legacy force branch, or post-hoc envelope.  Failure leaves Bloch transport as a
field-only result.

