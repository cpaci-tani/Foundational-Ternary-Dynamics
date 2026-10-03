# Mind's Eye telemetry and interface

The Telemetry dropdown presents live readings from the authoritative Observer
snapshot and its matching received-light witness. Opening it releases mouse capture
and keeps playback running. Author drawers retain the existing inspection pause;
switching to Telemetry releases only that inspection-owned pause. A manually
paused experiment stays paused when Telemetry closes.
Click the scene to recapture the mouse and navigate while Telemetry remains
visible. Category summaries retain keyboard activation without applying movement.

The centered landscape panel spans up to 1200 pixels on desktop, with four
category columns, two on midsize screens and one on mobile. Readings use dark
tiles, subdued labels, bright tabular values and separate unit styling. The
header shows playback state, reading and category counts, and the reference
model. Categories begin expanded, scroll inside the dropdown and can collapse
independently. Other authoring controls retain their narrow side drawers.

The small **UI** button hides the interface, reticle, spatial labels, tether and
gizmo. It closes active controls and the JEV console so hidden controls cannot
retain input ownership. The same button restores the interface; shortcuts that
open controls also restore it. JEV starts hidden and can be enabled with
**Controls → Show JEV button**. That preference belongs to portable world settings;
interface hiding is temporary presentation state.

## Implemented measurement plan

| Category | Measurements and interpretation |
| --- | --- |
| Model | Active reference model, integrator, timeline and source identity. Minkowski SR, Schwarzschild exterior and classical Playground keep separate meanings. |
| Time | Coordinate and proper clocks, worldline clock origin, elapsed time, clock rate and retained optical history. Clock differences use a common origin. |
| Motion | Velocity components, speed, Lorentz factor, rapidity, momentum and energy per rest mass. GR local static-frame motion stays separate from coordinate motion. |
| Distance | Observer position and received path; in GR, isotropic radius, areal radius and proper radial separation on a static spatial slice. These are distinct distance measures. |
| Mass | Adopted source nominal GM, solar-mass parameter, SI mass estimate and radii. The density proxy applies only to the star. Sandbox author mass does not set the reference spacetime. |
| Gravity | Lapse, spatial scale, optical index, compactness, gravitational redshift, static support acceleration and exterior curvature/tidal reference quantities. |
| Black-hole horizon | Horizon area and radius, photon-sphere and ISCO scales, critical impact parameter, static shadow angle and solid angle, finite sky-shell lapse, ray provenance and the exterior numerical guard. |
| Arriving light | Entity identity and proper clock require a source hit. Escaping black-hole sky rays provide emission coordinate time, delay, path and frequency without an entity clock. Captured or unresolved rays leave emission measurements unavailable. |
| Rendering | Current resolution, quality scale, actual GPU timing and renderer capabilities where reported. Missing measurements remain unavailable. |
| Lattice link | Retained native owner/source/tick provenance, separate from the adopted relativity reference models. |

Categories expand independently and their expansion state survives live updates.
Numeric outputs refresh at the existing 10 Hz interface cadence. This panel is
read-only; it does not accumulate a second physical history, change integration,
or infer measurements from screenshots.
There are 123 readings in nine categories when the lattice link is present,
or 135 readings in ten categories for the black-hole preparation. Unavailable
measurements remain explicitly blank. The black-hole horizon has no emitting
surface or assigned source clock; stellar-surface measurements stay unavailable.

Settings, actions, categories, telemetry and other information have hover and
keyboard-focus explanations. Equations render as mathematical notation using
KaTeX when available or native MathML. Escape dismisses an explanation, pointer
movement can enter its visible bounds, and scrolling repositions or dismisses it
as appropriate. Interface hiding and disposal clear it.
The explanations name the local or coordinate measuring frame and retain the
unit and clock conventions of each preparation.

## Units and limits

The Minkowski sandbox uses c = 1 without an SI length calibration. Its world
distances and times retain normalized units. The compact-star preparation has an
explicit scale: by default one coordinate unit represents 48 km and one time
unit is 48 km/c. The default ten-solar-GM black hole instead uses approximately
118.13 km per isotropic coordinate unit and 118.13 km/c per time unit.
SI conversions apply only where the preparation declares its calibration.

The star is a floating emitting sphere at the fixed metric center. Exterior GR
and local SR are adopted reference physics, not recovered FTD dynamics. No
stellar interior, rotation, material equilibrium or backreaction is simulated.
Local tidal and static-support readings are exterior reference quantities; they
do not simulate an extended body or a propulsion system.

Nominal stellar GM comes from the existing IAU nominal solar mass parameter. The
displayed mass estimate GM/G and density proxy use CODATA 2022 G = 6.67430e-11
m³ kg⁻¹ s⁻² (relative uncertainty about 2.2e-5), only for conversion. They do not
alter the metric or its integrator. Sources: [IAU nominal constants](https://arxiv.org/abs/1510.07674)
and [NIST CODATA 2022 constants](https://physics.nist.gov/cuu/Constants/Table/allascii.txt).

See [the compact-star model and GPU verification](REF_MINDS_EYE_COMPACT_STAR.md)
for the null-ray solver, timelike integration, bounded exterior domain and test
evidence, and [the black-hole model](REF_MINDS_EYE_BLACK_HOLE.md) for its horizon,
shadow, sky-shell and precision limits. Node telemetry regressions test units and frame distinctions; browser
regressions test live updates, playback ownership, interface hiding and responsive
controls. Actual production WebGL readback remains the optical GPU verification.

## Recorded verification

The landscape dropdown, tooltip sweep and black-hole extension have a separate
[verification record](MINDS_EYE_HORIZON_EVIDENCE.json), including source hashes,
responsive-layout checks, coverage across all eleven drawers in both profiles,
and actual production GPU readback. The earlier record below describes the
original floating-star and telemetry implementation.

[The original telemetry evidence record](MINDS_EYE_TELEMETRY_EVIDENCE.json) records source
hashes and the individual runs: 245 Observer and 230 assistant Node tests passed;
73 unique Observer browser regressions and 11 JEV browser tests passed across the
broad and focused runs. The final nine telemetry browser tests all passed,
including captured navigation, native category keyboard controls, page visibility,
JEV input release and desktop/mobile access. One intermediate regression found
JEV covering the UI button; its placement was repaired and verified with a normal
browser click. Typechecks, scoped lint, dependency checks and whitespace checks
passed. No targeted tests were blocked.

The original production GR shader's actual RTX 5090 float readback has 39 hits and 36 misses
across 75 rays, with maximum Doppler residual about 7.18e-7 against the independent
Float64 ray oracle. The three rays formerly hitting the pedestal now miss. Late
clocks, the D=1 color baseline, D⁴ intensity and real graphics-context recovery
also passed. Native C++/CUDA and theory Python suites were outside this web-only
change and were not rerun.
