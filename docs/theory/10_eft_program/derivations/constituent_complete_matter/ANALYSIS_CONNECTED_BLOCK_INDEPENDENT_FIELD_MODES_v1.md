# Connected-block independent field modes

**Campaign:** FTD-0641  
**Status:** `[THEOREM — FINITE-L SOURCE-FREE DISPERSION] + [MEASURED —
INDEPENDENT TRANSVERSE FIELD MODES]`  
**Production impact:** none

## Discrete field spectrum

The matched source-free tick is

\[
B_{t+1/2}=B_{t-1/2}-\lambda C^TE_t,
\qquad
E_{t+1}=E_t+\lambda CB_{t+1/2}.
\]

Eliminating `B` gives

\[
E_{t+1}-2E_t+E_{t-1}=-\lambda^2CC^TE_t.
\]

For a transverse periodic Fourier mode,

\[
\sigma^2=4\sum_a\sin^2(\pi n_a/L),
\qquad
\Omega=2\arcsin(\lambda\sigma/2).
\]

At `lambda=C_SPEED=1/sqrt(3)`, the full cubic Brillouin zone obeys
`lambda sigma<=2`; the body corner saturates the stability bound. This is the
precise role of the engine's fixed-cone coefficient in the matched field
sector.

## Result

FTD-0641 constructs every perturbation as `delta E=C A`, so its divergence
vanishes algebraically. It evolves the FTD-0638 dressed background and the
background-plus-mode state separately, subtracting the control before phase
estimation.

All 42 primary arms (`n=1,2,3`; `<100>`, `<110>`, `<111>` cubic copies; two
polarizations) and 12 amplitude/sign controls pass 256 forward and 256 reverse
ticks. The verdict is
`CONNECTED_BLOCK_INDEPENDENT_FIELD_MODES_CONSTRUCTIVE`.

| diagnostic | worst value | gate |
|---|---:|---:|
| primary phase error | `1.430e-13` | `1e-8` |
| recurrence residual | `1.587e-12` | `1e-8` |
| divergence | `3.432e-15` | `1e-12` |
| modified-energy drift | `9.170e-14` | `1e-12` |
| inverse recovery | `6.661e-16` | `1e-11` |
| amplitude phase residual | `1.733e-13` | `1e-8` |
| sign trajectory residual | `8.995e-12` | `1e-8` |
| polarization mismatch | `1.238e-13` | `1e-10` |
| cubic mismatch | `9.734e-14` | `1e-10` |

## Ontological consequence

The selected model now has two separately qualified tangent sectors around one
finite dressed object:

- 48 matter-coordinate modes, including six soft lattice-dressed collective
  motions and 42 internal deformations;
- independent divergence-free transverse face/edge field modes with a fixed
  lattice dispersion.

This makes the field more than a static aura or visualization. It carries
reversible propagating degrees of freedom even while matter is held fixed.
It still does not show that a localized field packet is a photon, that an
excited object emits radiation, or that the coupled system has a common pole.

## Next boundary

The next campaign must release the matter coordinates while applying the same
registered transverse perturbations. It must measure field survival, induced
matter amplitudes, energy transfer, coupled eigenfrequencies, and state-only
inversion without calling either bare sector the interacting spectrum.
