# Mind's Eye compact-star reference experiment

Enter **Mind's Eye → Experiments → A floating compact star → Enter preparation**.
The small marked sphere floats in front of the observer. Its static source geometry
defines the adopted exterior; there is no pedestal or mechanical support model.
**Release into free fall** follows a timelike geodesic; **Hold at rest** applies
an instantaneous stopping impulse and subsequently supports the observer.
Navigation controls work in guided mode. The HUD reports the observer clock rate,
surface redshift to infinity, motion mode and conversion to SI time.

**[IMPOSED] Adopted GR/SR reference model.** This experiment uses the exterior of
a fixed, spherical, nonrotating star. It is separate from both the Minkowski
reference experiments and native FTD. No G* corrections or lattice gravity
identification enter the calculations. Switching preparations restores the
ordinary Minkowski path; switching profiles declares a fresh preparation.

The default source has nominal mass 1.4 solar masses and areal radius 12 km.
One displayed coordinate unit represents 48 km; one coordinate-time unit
represents 160.1107657 microseconds. The displayed areal radius is 0.25;
the sphere's isotropic coordinate radius is approximately 0.2046660391.
The mass sets GM using the exact nominal solar conversion
GM_sun = 1.3271244e20 m³/s²; c = 299792458 m/s.
These are standard reference inputs, independent of FTD constants.
The [IAU nominal conversion resolution](https://arxiv.org/abs/1510.07674)
defines the solar GM convention.

## Metric and clocks

In isotropic Cartesian coordinates, with c = 1 and rho the coordinate distance
from the source center:

```
rs = 2 GM / c²                 q = rs / (4 rho)
A = (1-q)/(1+q)                B = (1+q)²
ds² = A² dt² - B² dx·dx        r_areal = rho B
dτ/dt = A / gamma(v_local)
```

For the default star, rs is approximately 4.134550107 km and the surface lapse
is approximately 0.80960124. A static surface clock has redshift to infinity
z = 1/A_surface - 1, approximately 0.23517598. At the initial observer location,
the clock rate is approximately 0.98574624; the received surface frequency
factor is approximately 0.82130796. The surface clock, rather than an undefined
clock at the source center, is used for optical diagnostics.

The Hamiltonian for timelike geodesics is H = A sqrt(1 + p²/B²), with
p = B gamma(v_local) v_local. Coordinate-time RK4 evolves position, canonical
momentum and proper time. Free fall conserves Killing energy and angular
momentum within the measured numerical tolerances. Guided movement instead
prescribes local du/dt, u = gamma v; it is an ideal rocket control, not the
Minkowski coordinate-force API or a computed material force.

## Received light

The optical index is n = B/A. Backward null rays evolve Euclidean coordinate
arclength l using:

```
dx/dl = d
dd/dl = grad(log n) - d (d·grad(log n))
d(travel time)/dl = n
```

The camera ray is first boosted from the observer rest frame to the local
static orthonormal frame. The static emitting surface then gives
D = A_emit/A_observer × gamma_observer × (1 + v_observer·d_initial).
The same combined frequency factor enters the three-line reference spectrum
and integrated line-intensity factor D⁴. No extra image magnification multiplier
is applied: image geometry follows the curved rays. The marked surface emission
and background angular grid are prescribed test patterns, not stellar atmosphere
or thermal spectra. Turning off color shifting retains the same spectral
baseline; intensity is a separate display control.

The production WebGL2 material integrates these rays on the GPU. A separate
Float64 implementation supports picking and numerical comparisons. The original
Minkowski shader is preserved. This follows the full-metric geometric-optics
approach described in [Perlick's spacetime lensing review](https://link.springer.com/article/10.12942/lrr-2004-9).

## Bounds and provenance

- No stellar interior, equation of state, rotation, collapse, accretion, finite-size
  tidal response or backreaction are simulated. The source is fixed at the center
  of the adopted Schwarzschild exterior.
- The model requires R > 1.5 rs, outside the photon sphere. The observer remains
  outside the surface and inside isotropic radius 40. Motion pauses at domain
  boundaries or the application's 0.99c local speed limit; a free-fall path is
  never projected onto that limit and called a geodesic.
- Background radiation is prescribed at a finite outer boundary, not at infinity.
  The HUD's surface redshift to infinity is a separate analytic reference value.
- Rays use at most 768 RK4 steps. Surface-distance stepping uses a 1e-7 CPU
  coordinate tolerance and a 1e-5 GPU tolerance. Budget exhaustion or missing
  history produces no fabricated hit. Near-critical rays and arbitrary imported
  mass/radius pairs do not have a universal error guarantee.
- Static segment histories retain the existing 60-unit window and explicit
  prehistory. Emission times preceding the cutoff are unavailable. Star snapshots
  preserve and validate the metric, source identity, single sphere and histories.
  Their integrator is tagged `gr-schwarzschild-exterior-rk4-v1`.
  Obsolete four-object pedestal snapshots are explicitly rejected with instructions
  to re-enter this preparation; imported optical histories are never silently
  rewritten to remove their former support geometry.
- Flat-space rulers, mirrored geometry, wave overlays, history scrubbing and
  scene authoring are unavailable in this locked GR preparation. The other
  reference experiments retain their existing controls.
- Generic author entity mass is not the GR source mass. `spacetime.massSolar`
  sets nominal GM; `observer.velocity` denotes a local static-frame velocity.
  Diagnostic distance is isotropic Euclidean path arclength, distinct from
  light-travel time. Diagnostic exports include these conventions.

## Regression commands

```
npm run test:observer
npm run typecheck:observer
npm run test:assistant
npm run check:assistant-graph
# PowerShell; require a hardware renderer and actual float-target readback
$env:FTD_HARDWARE_WEBGL='1'
npm run test:observer:browser
```

`observer-compact-star.node.test.mjs` checks exact radial Shapiro delay,
independent areal-coordinate quadrature for oblique light paths, the analytic
lensed silhouette, clock and frequency factors, the zero-mass SR limit,
geodesic energy/angular-momentum conservation, surface termination, locked
commands, retained history, portable persistence, the single-sphere preparation
and unobstructed rays through the former pedestal. `observer-compact-star.spec.js`
checks the actual compiled production GPU material and public UI controls.
CPU agreement alone and screenshots are not GPU physics certification.

Floating-sphere verification on 2026-10-03:

| Check | Result |
| --- | --- |
| Compact-star Node mathematics/session/optics/persistence tests | 11 passed |
| GR production GPU, public UI and context-recovery checks | 3 passed |
| Former support rays at NDC y = −0.22, −0.4, −0.7 | CPU and actual GPU misses |
| Obsolete pedestal session and portable snapshot imports | Rejected with explicit recovery message |

Before removing the pedestal, the same focused runs reproduced all 10 original
compact-star Node checks and all 3 browser checks. The original reference feature
also passed 234 Observer Node tests, 230 Assistant Node tests, 61 existing browser
checks and its 3 GR checks. Those broad-suite counts describe the earlier pedestal
version; the focused floating-source results above are newly executed. All focused
tests executed; none were blocked. Native C++/CUDA and theory Python suites were
not rerun for this reference-workspace-only change.

[Recorded GPU evidence](COMPACT_STAR_GPU_EVIDENCE.json) identifies Chromium
147.0.7727.15, NVIDIA RTX 5090 through ANGLE D3D11, 23-bit fragment precision
and actual floating-point readback. The 75 interior/test rays yielded 39 hits
and 36 misses, with no identity disagreements. Maximum absolute residuals:
path arclength 2.79e-5; emission time 4.26e-5; surface proper time 3.46e-5;
combined frequency factor 7.18e-7; coordinate position 2.79e-5. Two additional
silhouette rays bracket the exact impact-parameter limit at ±0.1%; their
separate travel-time tolerance is 0.005 coordinate units. D=1 GPU color-toggle
outputs agree exactly; the D⁴ intensity check has a 1e-6 absolute tolerance.
These are fixed-case numerical checks, not universal accuracy guarantees.
The final clock check also compares actual GPU and Float64 stellar surface clocks
at coordinate time 1e7 (1e-4 absolute tolerance). Static surface clock origins
remain Float64 CPU metadata; the GPU returns their small relative offsets.
