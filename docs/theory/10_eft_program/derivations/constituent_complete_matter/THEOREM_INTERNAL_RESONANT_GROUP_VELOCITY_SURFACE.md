# FTD-0695 — Internal-resonant group-velocity surface

**Status:** `[THEOREM — SOURCE-FREE FIELD KINEMATICS]` +
`[POST-HOC CONSISTENCY — NOT A SPECTRAL IDENTIFICATION]`  
**Production status:** unchanged

## 1. Premises

The selected matched face/edge field has the exact source-free dispersion

\[
\Omega(\mathbf k)=2\arcsin\sqrt{\frac{S(\mathbf k)}{3}},
\qquad
S(\mathbf k)=\sum_{a=1}^3\sin^2\frac{k_a}{2}.
\]

The first connected-matter internal doublet has phase

\[
\phi_{\rm int}=1.0911648733663635.
\]

FTD-0663 proved that this phase is embedded in the field band. The present
result derives the complete group-velocity consequence on the corresponding
constant-frequency surface.

## 2. Exact group velocity

For `0 < S < 3`, differentiation gives

\[
v_{g,a}(\mathbf k)
=\frac{\partial\Omega}{\partial k_a}
=\frac{\sin k_a}{2\sqrt{S(3-S)}}.
\]

The internal-resonant surface is therefore

\[
S(\mathbf k)=S_*
=3\sin^2\frac{\phi_{\rm int}}2
=0.8078216321246361.
\]

For a symmetry direction with `d` equal nonzero components, `d=1,2,3`,

\[
q_d=2\arcsin\sqrt{\frac{S_*}{d}},
\qquad
|\mathbf v_g|_d
=\frac{\sqrt d\,\sin q_d}{2\sqrt{S_*(3-S_*)}}.
\]

This yields

| direction | resonant component `q_d` | `q_d/pi` | `|v_g|` |
|---|---:|---:|---:|
| `<100>` | 2.233998332573721 | 0.7111037549763194 | 0.2960835685214112 |
| `<110>` | 1.377414916810145 | 0.4384447853976927 | 0.5214560095416422 |
| `<111>` | 1.091164873366363 | 0.3473285666489975 | 0.5773502691896256 |

On the body-diagonal branch, `Omega(q,q,q)=q` for `0 <= q <= pi`, so the
last speed is exactly the selected microscopic cone speed `1/sqrt(3)`.

## 3. Consequence for emitted disturbances

A localized oscillating source at `phi_int` cannot emit a single spherical
shell speed on this cubic field. If it couples to a finite portion of the
resonant surface, an outgoing packet is generically directionally dispersed:
different wavevectors with the same temporal frequency have different group
velocities. Threshold fronts, norm quantiles, and mean radii therefore need
not agree with one another or with any one entry in the table.

The positive-profile slopes measured after FTD-0694 lie in the broad interval
spanned by the symmetry-direction velocities, while its threshold-shell fit is
also below the microscopic cone. This is a post-hoc compatibility fact only.
The radial profile is not a Fourier spectrum, its fitted slopes are not direct
group-velocity estimators, and the comparison does not identify the emitted
field with the internal-resonant surface.

## 4. Required falsifier

The resonance interpretation requires a fresh observer that records the
spatiotemporal transverse field spectrum and tests all of the following
without using radial-threshold tuning:

1. spectral weight is concentrated near `Omega(k)=phi_int` during the
   pre-contact interval;
2. the measured temporal phase at occupied wavevectors follows the exact
   matched-field dispersion;
3. directional packet transport follows `grad_k Omega`, including cubic
   anisotropy;
4. the result is stable under sign, amplitude, volume, and observation-window
   changes.

Failure of those tests would reject resonant mode emission while leaving the
FTD-0694 facts of reversible field redistribution intact.

## 5. Ontological scope

This theorem says how native field disturbances *can* transport energy at the
measured internal frequency. It does not show that matter emits that frequency,
that the disturbance is a photon, that the radial front is a particle, or that
the infrared theory has a universal Lorentz cone.
