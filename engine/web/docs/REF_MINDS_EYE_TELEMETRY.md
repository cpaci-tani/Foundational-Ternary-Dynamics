# Mind's Eye telemetry and interface

The Telemetry drawer presents live readings from the authoritative Observer
snapshot and its matching received-light hit. Opening it releases mouse capture
and keeps playback running. Author drawers retain the existing inspection pause;
switching to Telemetry releases only that inspection-owned pause. A manually
paused experiment stays paused when Telemetry closes.
Click the scene to recapture the mouse and navigate while Telemetry remains
visible. Category summaries retain keyboard activation without applying movement.

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
| Mass | Adopted stellar nominal GM, solar-mass parameter, SI mass estimate, radii and mean-density proxy. Sandbox author mass does not become the stellar source mass. |
| Gravity | Lapse, spatial scale, optical index, compactness, gravitational redshift, static support acceleration and exterior curvature/tidal reference quantities. |
| Arriving light | Hit identity, emission time, source proper clock, travel delay, path length, received frequency factor, redshift and intensity factor. Missing hits remain unavailable. |
| Rendering | Current resolution, quality scale, actual GPU timing and renderer capabilities where reported. Missing measurements remain unavailable. |
| Lattice link | Retained native owner/source/tick provenance, separate from the adopted relativity reference models. |

Categories expand independently and their expansion state survives live updates.
Numeric outputs refresh at the existing 10 Hz interface cadence. This panel is
read-only; it does not accumulate a second physical history, change integration,
or infer measurements from screenshots.
There are 123 readings in nine categories when the lattice link is present;
unavailable measurements remain explicitly blank.

## Units and limits

The Minkowski sandbox uses c = 1 without an SI length calibration. Its world
distances and times retain normalized units. The compact-star preparation has an
explicit scale: by default one coordinate unit represents 48 km and one time
unit is 48 km/c. SI conversions apply only where that calibration exists.

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
evidence. Node telemetry regressions test units and frame distinctions; browser
regressions test live updates, playback ownership, interface hiding and responsive
controls. Actual production WebGL readback remains the optical GPU verification.

## Recorded verification

[The telemetry evidence record](MINDS_EYE_TELEMETRY_EVIDENCE.json) records source
hashes and the individual runs: 245 Observer and 230 assistant Node tests passed;
73 unique Observer browser regressions and 11 JEV browser tests passed across the
broad and focused runs. The final nine telemetry browser tests all passed,
including captured navigation, native category keyboard controls, page visibility,
JEV input release and desktop/mobile access. One intermediate regression found
JEV covering the UI button; its placement was repaired and verified with a normal
browser click. Typechecks, scoped lint, dependency checks and whitespace checks
passed. No targeted tests were blocked.

The production GR shader's actual RTX 5090 float readback has 39 hits and 36 misses
across 75 rays, with maximum Doppler residual about 7.18e-7 against the independent
Float64 ray oracle. The three rays formerly hitting the pedestal now miss. Late
clocks, the D=1 color baseline, D⁴ intensity and real graphics-context recovery
also passed. Native C++/CUDA and theory Python suites were outside this web-only
change and were not rerun.
