// @ts-check
/** Read-only measurements of the adopted reference models. No native FTD recovery.
 * Values retain their source units; missing calibration or observations stay null.
 */
import { LIGHT_SPEED, NOMINAL_SOLAR_GM, starMetric } from './compact-star.js';
import { dot, length, sub, MAX_BETA } from './math.js';

/** CODATA 2022, used only for the nominal GM/G mass estimate and its density proxy.
 * The metric and its dynamics continue to use the independently adopted nominal GM.
 * https://physics.nist.gov/cuu/Constants/Table/allascii.txt
 */
const MASS_CONVERSION_G = 6.67430e-11;
/** Presentation is fixed by row identity, including when its measurement is unavailable.
 * Counters retain exact decimal strings without converting them to floating-point numbers.
 * @typedef {'number'|'text'|'counter'} TelemetryPresentation
 */
/** @typedef {{id:string,label:string,value:number|string|null,unit:string,note:string,presentation:TelemetryPresentation}} TelemetryRow */
/** @typedef {{id:string,title:string,description:string,rows:TelemetryRow[]}} TelemetrySection */
/** @typedef {{hit?:import('./optics.js').OpticalHit|null,cameraOverride?:Partial<import('./optics.js').CameraState>,selectedId?:string|null,settings?:{optical?:boolean,doppler?:boolean,beaming?:boolean,fov?:number},rendering?:Record<string,any>,lattice?:Record<string,any>}} TelemetryView */
/** @param {unknown} value @returns {number|null} */
const finite = value => typeof value === 'number' && Number.isFinite(value) ? value : null;
/** @param {string} id @param {string} label @param {unknown} value @param {string} [unit] @param {string} [note] @param {TelemetryPresentation} [presentation] @returns {TelemetryRow} */
const row = (id, label, value, unit = '', note = '', presentation = 'number') => ({ id, label, value: typeof value === 'string' ? value : finite(value), unit, note, presentation });
/** @param {string} id @param {string} label @param {unknown} value @param {string} [unit] @param {string} [note] @returns {TelemetryRow} */
const textRow = (id, label, value, unit = '', note = '') => row(id, label, value, unit, note, 'text');
/** @param {string} id @param {string} label @param {unknown} value @param {string} [unit] @param {string} [note] @returns {TelemetryRow} */
const counterRow = (id, label, value, unit = '', note = '') => row(id, label, value, unit, note, 'counter');
/** @param {string} id @param {string} title @param {string} description @param {TelemetryRow[]} rows @returns {TelemetrySection} */
const section = (id, title, description, rows) => ({ id, title, description, rows });

/** Proper radial distance on a Schwarzschild constant-time slice, in input units.
 * This integrates dr/sqrt(1-rs/r); it is neither isotropic arclength nor r-R.
 * Rationalized differences preserve accuracy close to the surface.
 * @param {number} radius @param {number} surface @param {number} rs
 * @returns {number|null}
 */
export function properRadialDistance(radius, surface, rs) {
    if (![radius, surface, rs].every(Number.isFinite) || rs < 0 || surface <= rs || radius < surface) return null;
    if (rs === 0) return radius - surface;
    const d = radius - surface;
    const rootR = Math.sqrt(radius * (radius - rs)), rootS = Math.sqrt(surface * (surface - rs));
    const rootDifference = d * (radius + surface - rs) / (rootR + rootS);
    const sumS = Math.sqrt(surface) + Math.sqrt(surface - rs);
    const sumDifference = d / (Math.sqrt(radius) + Math.sqrt(surface)) + d / (Math.sqrt(radius - rs) + Math.sqrt(surface - rs));
    return rootDifference + rs * Math.log1p(sumDifference / sumS);
}

/** Static-slice proper radial distance from a Schwarzschild horizon. The
 * integrable lower-limit singularity is evaluated analytically; no hovering
 * observer or emitting surface at the horizon is assumed.
 * @param {number} radius @param {number} rs @returns {number|null}
 */
export function properHorizonDistance(radius, rs) {
    if (!Number.isFinite(radius) || !Number.isFinite(rs) || rs <= 0 || radius < rs) return null;
    const x = (radius - rs) / rs;
    return rs * (Math.sqrt(x * (1 + x)) + Math.asinh(Math.sqrt(x)));
}

/** The caller supplies only a sight-ray hit belonging to this snapshot/view.
 * Snapshot and camera comparison previews are read without advancing simulation.
 * @param {import('./types.js').WorldSnapshot} snapshot @param {TelemetryView} [view]
 * @returns {TelemetrySection[]}
 */
export function observationTelemetry(snapshot, view = {}) {
    const star = snapshot.spacetime, sr = snapshot.profile === 'sr';
    const blackHole = star?.kind === 'schwarzschild-black-hole';
    const observer = { ...snapshot.observer, ...view.cameraOverride };
    const velocity = observer.velocity, speed = length(velocity), beta = sr ? speed : null;
    const lorentz = sr && speed < 1 ? 1 / Math.sqrt(1 - dot(velocity, velocity)) : null;
    const metric = star ? starMetric(star, observer.position) : null;
    const lapse = metric?.lapse ?? 1, spatial = metric?.spatial ?? 1;
    const unitLength = star?.lengthUnitMeters ?? null, unitTime = unitLength === null ? null : unitLength / LIGHT_SPEED;
    const coordinateSpeed = sr ? speed * lapse / spatial : speed;
    const start = finite(snapshot.observer.worldlineStart);
    const preview = view.cameraOverride?.position !== undefined || view.cameraOverride?.velocity !== undefined || view.cameraOverride?.properTime !== undefined;
    const clockPair = snapshot.scrubTime === null && !preview && start !== null && start <= snapshot.time;
    const elapsed = clockPair ? snapshot.time - /** @type {number} */ (start) : null;
    const slip = elapsed === null ? null : elapsed - snapshot.observer.properTime;
    const rate = sr ? lorentz === null ? null : lapse / lorentz : 1;
    const selected = snapshot.entities.find(entity => entity.id === view.selectedId && entity.alive);
    const coordinateUnit = star ? 'isotropic units' : sr ? 'coordinate units' : 'simulation length units';
    const timeUnit = sr ? 'coordinate time units' : 'simulation time units';
    const sections = [
        section('model', 'Reference and session', blackHole ? 'Schwarzschild vacuum black-hole exterior with local SR. The horizon emits no light; a finite static sky shell supplies the background. The observer remains outside the horizon.' : star ? 'Adopted Schwarzschild vacuum exterior with local SR. Fixed emitting surface; no stellar interior or native FTD identification.' : sr ? 'Adopted Minkowski special relativity, c=1. SI length and time scales are unspecified.' : 'Classical Playground. Its simulation units and body mechanics do not define SR clocks or GR geometry.', [
            textRow('reference', 'Reference model', star ? 'Schwarzschild exterior + local SR' : sr ? 'Minkowski SR' : 'Classical Playground'),
            textRow('engine', 'Physics engine', snapshot.physicsEngine), textRow('integrator', 'Integrator', snapshot.integratorVersion ?? 'Unspecified'),
            textRow('session', 'Session', snapshot.sessionId), row('epoch', 'Epoch', snapshot.epoch), counterRow('tick', 'Tick', snapshot.tick),
            textRow('playing', 'Transport', snapshot.playing ? 'Playing' : 'Paused'), row('playback', 'Playback rate', snapshot.playbackSpeed, '×', 'Presentation rate per wall-clock second; no change to physical clock conventions.'),
            textRow('camera-preview', 'Camera comparison', preview ? 'Preview' : 'Session observer'),
            textRow('units', 'Source units', snapshot.units), row('body-count', 'Live bodies', snapshot.entities.filter(entity => entity.alive).length),
            row('warning-count', 'Session warnings', snapshot.warnings.length),
        ]),
        section('time', 'Time and retained history', 'Worldline clock differences use its recorded coordinate origin. Relocation starts a new clock; scrubbing and camera previews do not define a paired clock comparison.', [
            row('coordinate-time', 'Coordinate time t', snapshot.time, timeUnit),
            row('proper-time', sr ? snapshot.scrubTime !== null ? 'Retained observer proper time τ' : 'Observer proper time τ' : 'Observer elapsed clock', snapshot.observer.properTime, sr ? 'proper time units' : timeUnit, snapshot.scrubTime !== null ? 'Source-history scrubbing rewinds coordinate time and source entities; it does not rewind the retained observer worldline clock.' : preview ? 'Accumulated session observer clock; a camera comparison does not integrate a new clock.' : 'Accumulated on the recorded observer worldline.'),
            row('worldline', 'Observer worldline', snapshot.observer.worldline), textRow('worldline-reason', 'Worldline origin', snapshot.observer.worldlineReason ?? 'Unspecified'),
            row('worldline-start', 'Coordinate clock origin', start, timeUnit), row('coordinate-elapsed', 'Coordinate elapsed on worldline', elapsed, timeUnit),
            row('clock-slip', 'Coordinate elapsed − observer clock', slip, timeUnit, clockPair ? 'Compared from the same worldline origin.' : 'Unavailable without a matching clock origin or during scrub/preview.'),
            row('clock-rate', sr ? 'Instantaneous dτ/dt' : 'Classical clock rate', rate, '', (snapshot.scrubTime !== null ? 'Uses the current observer velocity; source-history scrubbing does not rewind observer motion. ' : '') + (star ? 'A/γ; Schwarzschild coordinate time normalized to infinity.' : sr ? '1/γ in the Minkowski coordinate frame.' : 'Classical elapsed clock follows simulation time.')),
            row('coordinate-time-si', 'Coordinate time t', unitTime === null ? null : snapshot.time * unitTime, 's', 'SI conversion uses the declared Schwarzschild reference length scale.'),
            row('proper-time-si', 'Observer clock τ', unitTime === null ? null : snapshot.observer.properTime * unitTime, 's'),
            row('clock-slip-si', 'Paired clock difference', unitTime === null || slip === null ? null : slip * unitTime, 's'),
            row('history-start', 'Optical history begins', snapshot.historyStart, timeUnit), row('history-end', 'Viewed history ends', snapshot.time, timeUnit),
            row('history-span', 'Retained interval span', snapshot.time - snapshot.historyStart, timeUnit, 'Includes deliberately prepared source prehistory when present.'),
            row('history-policy', 'History policy window', snapshot.historyWindow, timeUnit), row('history-segments', 'Retained worldline segments', snapshot.segments.length),
            row('backlog', 'Queued integration time', snapshot.backlogSeconds, timeUnit),
            row('time-scale', 'One coordinate time unit', unitTime, 's', 'One length unit divided by c, from the declared reference scale.'),
        ]),
        section('motion', sr ? 'Local SR motion and energy' : 'Classical motion', star ? 'Velocity is measured by the local static orthonormal frame. Coordinate drift differs by A/B. Energy and momentum are per rest mass; no observer mass is assumed.' : sr ? 'Velocity refers to the Minkowski coordinate frame. Energy and momentum are per rest mass; no observer mass is assumed.' : 'Coordinate motion from the classical session. Relativistic quantities are unavailable.', [
            row('speed', sr ? 'Local speed β' : 'Coordinate speed', beta ?? speed, sr ? 'c' : 'simulation length/time'),
            ...velocity.map((v, i) => row(`velocity-${i}`, `${sr ? 'Local' : 'Coordinate'} velocity ${['X', 'Y', 'Z'][i]}`, v, sr ? 'c' : 'simulation length/time')),
            row('speed-si', 'Local speed', star ? speed * LIGHT_SPEED : null, 'm/s'),
            row('coordinate-speed', 'Coordinate speed |dx/dt|', coordinateSpeed, star ? 'isotropic units/time unit' : sr ? 'c' : 'simulation length/time'),
            row('coordinate-speed-si', 'Coordinate speed', star ? coordinateSpeed * LIGHT_SPEED : null, 'm/s', 'Coordinate rate, not a local speed measured by an observer.'),
            row('gamma', 'Lorentz factor γ', lorentz), row('rapidity', 'Rapidity magnitude', lorentz === null ? null : Math.atanh(speed)),
            row('sr-clock-rate', 'Local SR dτ/dt_static', lorentz === null ? null : 1 / lorentz),
            row('energy-rest', 'Local total energy / mc²', lorentz), row('kinetic-rest', 'Local kinetic energy / mc²', lorentz === null ? null : lorentz - 1),
            row('momentum-rest', 'Local momentum magnitude / mc', lorentz === null ? null : lorentz * speed),
            row('killing-energy', 'Energy at infinity / mc²', star && lorentz !== null ? lapse * lorentz : null, '', 'Aγ is conserved for freefall in this static metric. Guided controls can change it.'),
            textRow('motion-mode', 'Motion mode', star?.observerMode ?? (sr ? 'Minkowski controls' : 'Classical controls')),
            row('speed-cap', 'Session SR speed limit', sr ? MAX_BETA : null, 'c', 'Implementation boundary for this reference experiment.'),
            textRow('cap-applied', 'Speed cap applied', observer.capApplied === true ? 'Yes' : 'No'),
        ]),
        section('distance', 'Position and distance', star ? 'Isotropic coordinates, areal radius and proper radial distance are distinct. Proper distance is on a constant Schwarzschild-time slice.' : 'Positions and center distances belong to the displayed coordinate frame. No SI length scale is assigned.', [
            ...observer.position.map((v, i) => row(`position-${i}`, `Position ${['X', 'Y', 'Z'][i]}`, v, coordinateUnit)),
            row('selected-distance', 'Selected body center distance', selected ? length(sub(observer.position, selected.position)) : null, coordinateUnit, 'Coordinate center-to-center distance; not optical travel distance.'),
            row('isotropic-radius', 'Source-centered isotropic radius ρ', metric?.rho ?? null, coordinateUnit),
            row('isotropic-radius-si', 'Isotropic radius ρ', metric && unitLength !== null ? metric.rho * unitLength : null, 'm'),
            row('areal-radius', 'Areal radius r', metric && unitLength !== null ? metric.areal * unitLength : null, 'm', 'Sphere circumference / 2π.'),
            row('areal-height', blackHole ? 'Areal radius − horizon radius' : 'Areal radius − stellar R', metric && star && unitLength !== null ? (metric.areal - (blackHole ? star.rs : star.radiusKm * 1000 / unitLength)) * unitLength : null, 'm', 'Radius difference; not proper radial distance.'),
            row('proper-radial-distance', blackHole ? 'Static-slice proper distance from horizon' : 'Proper radial distance above surface', metric && star && unitLength !== null ? blackHole ? properHorizonDistance(metric.areal * unitLength, star.rs * unitLength) : properRadialDistance(metric.areal * unitLength, star.radiusKm * 1000, star.rs * unitLength) : null, 'm', 'Integral of dr/√(1−rs/r) on a static time slice; not a falling observer’s travel distance.'),
            row('radius-rs', 'Areal radius / Schwarzschild radius', metric && star && star.rs > 0 ? metric.areal / star.rs : null),
            row('length-scale', 'One displayed length unit', unitLength, 'm'),
            row('outer-boundary', 'Outer isotropic optical boundary', star?.escapeRadius ?? null, 'isotropic units', 'Finite tracer/domain boundary; not a cosmological horizon.'),
        ]),
        section('mass', blackHole ? 'Black-hole mass and horizon' : star ? 'Compact-star mass and surface' : 'Mass conventions', star ? 'Nominal solar GM calibrates the vacuum metric. Mass in kilograms is a GM/G estimate. No interior density profile is simulated.' : 'Authored body masses are simulation parameters. A rest mass or SI mass calibration has not been assigned to the observer.', [
            row('observer-mass', 'Observer rest mass', null, 'kg', 'Observer is a test worldline without an assigned rest mass.'),
            row('selected-mass', 'Selected body mass parameter', selected?.mass ?? null, 'simulation mass units', star ? 'Surface marker mass does not set the stellar metric.' : 'Source parameter; no kilogram calibration.'),
            row('star-mass-solar', 'Source nominal solar-GM multiples', star?.massSolar ?? null, 'GM☉ᴺ', 'Mass convention from IAU nominal solar mass parameter.'),
            row('star-gm', 'Source GM', star ? star.massSolar * NOMINAL_SOLAR_GM : null, 'm³/s²'),
            row('star-mass-kg', 'Nominal GM / CODATA G', star ? star.massSolar * NOMINAL_SOLAR_GM / MASS_CONVERSION_G : null, 'kg', 'Estimate inherits G uncertainty (~2.2×10⁻⁵ relative). Not an exact nominal solar mass.'),
            row('stellar-radius', 'Stellar areal radius R', star && !blackHole ? star.radiusKm * 1000 : null, 'm', blackHole ? 'A black hole has no stellar emitting surface.' : ''),
            row('schwarzschild-radius', 'Schwarzschild radius rs', star && unitLength !== null ? star.rs * unitLength : null, 'm'),
            row('mean-density-proxy', 'Mass / Euclidean areal-volume proxy', star && !blackHole ? star.massSolar * NOMINAL_SOLAR_GM / MASS_CONVERSION_G / (4 * Math.PI * (star.radiusKm * 1000) ** 3 / 3) : null, 'kg/m³', blackHole ? 'No material density or Euclidean interior volume is assigned to this vacuum black-hole solution.' : 'M/(4πR³/3); this is not a GR proper-volume density or an interior EOS.'),
        ]),
    ];
    const rsMeters = star && unitLength !== null ? star.rs * unitLength : null;
    const rMeters = metric && unitLength !== null ? metric.areal * unitLength : null;
    const GM = star ? star.massSolar * NOMINAL_SOLAR_GM : null;
    const surfaceA = star && !blackHole ? starMetric(star, [star.center[0] + star.radius, star.center[1], star.center[2]]).lapse : null;
    sections.push(section('gravity', 'GR geometry and reference fields', star ? 'Schwarzschild vacuum exterior quantities. Hover acceleration and tidal coefficients are local reference values; free fall follows the timelike geodesic equations.' : 'GR telemetry is available in the compact-star and black-hole experiments.', [
        row('lapse', 'Local lapse A', metric?.lapse ?? null, '', 'Static proper time / coordinate time normalized at infinity.'),
        row('spatial-factor', 'Isotropic spatial factor B', metric?.spatial ?? null, '', 'Static proper length = B × isotropic coordinate length locally.'),
        row('optical-index', 'Coordinate optical index n', metric?.index ?? null, '', 'B/A; coordinate light speed magnitude = 1/n. Local measured light speed remains c.'),
        row('coordinate-light-speed', 'Coordinate light speed', metric ? 1 / metric.index : null, 'c'),
        row('local-compactness', 'Local rs/r', metric && star ? star.rs / metric.areal : null),
        row('surface-compactness', 'Surface rs/R', star && !blackHole && rsMeters !== null ? rsMeters / (star.radiusKm * 1000) : null),
        row('surface-lapse', 'Surface static clock lapse', surfaceA),
        row('surface-redshift', 'Surface redshift to infinity', surfaceA === null ? null : 1 / surfaceA - 1, '', 'Static source and static receiver at infinity; the current received-light shift differs.'),
        row('observer-redshift', 'Local static redshift to infinity', metric ? 1 / metric.lapse - 1 : null),
        row('hover-acceleration', 'Static hover proper acceleration', GM !== null && rMeters !== null && metric ? GM / (rMeters ** 2 * metric.lapse) : null, 'm/s²', 'Acceleration required to hold a static worldline at this radius; not the guided observer thrust.'),
        row('surface-hover-acceleration', 'Surface static hover acceleration', GM !== null && star && surfaceA !== null ? GM / ((star.radiusKm * 1000) ** 2 * surfaceA) : null, 'm/s²'),
        row('radial-tidal', 'Radial tidal stretching coefficient', GM !== null && rMeters !== null ? 2 * GM / rMeters ** 3 : null, 's⁻²', 'Radial geodesic-deviation acceleration per local separation for radial freely falling orthonormal observers.'),
        row('transverse-tidal', 'Transverse tidal compression coefficient', GM !== null && rMeters !== null ? -GM / rMeters ** 3 : null, 's⁻²'),
        row('kretschmann', 'Kretschmann curvature invariant', rsMeters !== null && rMeters !== null ? 12 * rsMeters ** 2 / rMeters ** 6 : null, 'm⁻⁴', 'R_abcd R^abcd = 12 rs²/r⁶ in the Schwarzschild vacuum exterior.'),
        row('escape-speed', 'Local escape-to-infinity speed', metric && star ? Math.sqrt(star.rs / metric.areal) : null, 'c', 'Ideal radial test-particle energy threshold Aγ=1, extending the reference metric to infinity beyond the finite optical boundary.'),
        row('photon-sphere', 'Vacuum photon-sphere radius', rsMeters === null ? null : 1.5 * rsMeters, 'm', blackHole ? 'Unstable circular null geodesics at r = 3rs/2. This is outside the horizon and distinct from its observed shadow.' : 'Formal vacuum radius. This preparation places it inside the opaque surface; no interior photon orbit is simulated.'),
        row('isco-radius', 'Vacuum circular-orbit ISCO radius', rsMeters === null ? null : 3 * rsMeters, 'm', 'Innermost stable circular timelike test-particle orbit, r = 3rs; applicable in the vacuum exterior.'),
    ]));
    if (blackHole && star && metric && unitLength !== null && rsMeters !== null) {
        const impact = 1.5 * Math.sqrt(3) * star.rs;
        const shadowSine = Math.min(1, impact * metric.lapse / metric.areal);
        const shadow = metric.areal >= 1.5 * star.rs ? Math.asin(shadowSine) : Math.PI - Math.asin(shadowSine);
        sections.push(section('horizon', 'Horizon and black-hole shadow', 'The event horizon, photon sphere and apparent shadow have different radii. Angular sizes here belong to a local static observer; motion aberrates the displayed image.', [
            row('horizon-radius', 'Event-horizon areal radius', rsMeters, 'm', 'rs = 2GM/c² for this nonrotating, uncharged vacuum black hole.'),
            row('horizon-isotropic-radius', 'Event-horizon isotropic radius', star.rs / 4, 'isotropic units', 'The exterior isotropic chart ends at ρ = rs/4.'),
            row('horizon-area', 'Event-horizon area', 4 * Math.PI * rsMeters ** 2, 'm²', 'Area is 4πrs²; this is not an emitting surface.'),
            row('critical-impact', 'Critical null impact parameter', impact * unitLength, 'm', 'bc = 3√3 GM/c² = (3√3/2)rs. Conserved angular momentum divided by energy at infinity.'),
            row('static-shadow-angle', 'Static-observer shadow half-angle', shadow * 180 / Math.PI, '°', 'sin α = bc A/r; select α ≤ π/2 outside the photon sphere and α ≥ π/2 inside it. This is not the moving camera’s aberrated angle.'),
            row('static-shadow-solid-angle', 'Static-observer shadow solid angle', 2 * Math.PI * (1 - Math.cos(shadow)), 'sr', 'Solid angle of the capture cone on the local static observer’s sky.'),
            row('horizon-curvature', 'Kretschmann invariant at horizon', 12 / rsMeters ** 4, 'm⁻⁴', 'Finite vacuum curvature at r = rs. The horizon is a coordinate singularity in this exterior chart.'),
            row('sky-shell-radius', 'Static sky-shell areal radius', starMetric(star, [star.center[0] + star.escapeRadius, star.center[1], star.center[2]]).areal * unitLength, 'm', 'Prescribed background at a finite radius, rather than an emitter at infinity.'),
            row('sky-shell-lapse', 'Static sky-shell clock lapse', starMetric(star, [star.center[0] + star.escapeRadius, star.center[1], star.center[2]]).lapse),
        ]));
    }
    const optical = !!star || sr && view.settings?.optical !== false;
    const candidate = optical && !blackHole ? view.hit : null;
    const hit = candidate && Number.isFinite(candidate.emissionTime) && candidate.emissionTime <= snapshot.time && candidate.emissionTime >= snapshot.historyStart ? candidate : null;
    const reportedRay = blackHole ? view.rendering?.blackHoleRay : null;
    const ray = reportedRay && reportedRay.source === 'Float64 CPU geodesic' && reportedRay.observationTime === snapshot.time
        && reportedRay.ndcX === 0 && reportedRay.ndcY === 0
        && JSON.stringify(reportedRay.observerPosition) === JSON.stringify(observer.position)
        && JSON.stringify(reportedRay.observerVelocity) === JSON.stringify(observer.velocity)
        && reportedRay.observerYaw === (observer.yaw ?? 0) && reportedRay.observerPitch === (observer.pitch ?? 0)
        && reportedRay.observerRoll === (observer.roll ?? 0)
        && (view.settings?.fov === undefined || reportedRay.cameraFov === view.settings.fov) ? reportedRay : null;
    const skyReceived = ray?.status === 'escaped';
    const D = hit && hit.doppler > 0 ? finite(hit.doppler) : skyReceived && ray.doppler > 0 ? finite(ray.doppler) : null;
    const delay = hit ? snapshot.time - hit.emissionTime : skyReceived ? finite(ray.delay) : null;
    const sourceA = hit && star ? hit.entityId === star.sourceId ? surfaceA : starMetric(star, hit.sourcePosition).lapse : skyReceived ? finite(ray.emissionLapse) : null;
    const pathDistance = hit?.distance ?? (skyReceived ? finite(ray.distance) : null);
    sections.push(section('optics', 'Received light', blackHole ? 'Current Float64 sky-shell ray. The prescribed static background exists at all reference times without a retained source-entity clock. Captured and unresolved rays have no received emission. Color and intensity switches affect presentation.' : optical ? 'Current sight-ray witness only. Frequency shift combines emission and reception conditions; source and observer clocks can have different origins. Color and intensity switches affect presentation.' : 'A simultaneous geometry view does not supply received-light measurements. Enable the SR optical view to inspect light travel and frequency shift.', [
        textRow('optical-mode', 'Observation mode', optical ? star ? 'Curved null ray + local SR' : 'Retarded Minkowski null ray' : 'Simultaneous geometry'),
        textRow('optical-status', 'Current sight ray', blackHole ? ray?.status ?? 'No current ray measurement' : hit ? 'Source hit' : 'No current source hit'),
        textRow('source', 'Received source identity', hit?.entityId ?? null), row('source-revision', 'Received source revision', hit?.revision ?? null),
        row('frequency-ratio', 'Frequency ratio D = f_received/f_emitted', D), row('redshift', 'Received redshift z = 1/D − 1', D === null ? null : 1 / D - 1),
        row('wavelength-ratio', 'Wavelength ratio λ_received/λ_emitted', D === null ? null : 1 / D),
        row('bolometric-factor', 'Reference bolometric intensity ratio D⁴', D === null ? null : D ** 4, '', 'Reference ray-intensity factor; not a total luminosity or flux at the observer.'),
        row('emission-time', 'Coordinate emission time', hit?.emissionTime ?? (skyReceived && delay !== null ? snapshot.time - delay : null), timeUnit),
        row('source-clock', 'Emitting source proper clock', hit?.properTime ?? null, 'proper time units'),
        row('optical-delay', 'Coordinate light travel time', delay, timeUnit),
        row('optical-delay-si', 'Coordinate light travel time', unitTime === null || delay === null ? null : delay * unitTime, 's'),
        row('optical-path', star ? 'Isotropic ray arclength' : 'Coordinate null-ray distance', pathDistance, coordinateUnit, star ? 'Euclidean isotropic-coordinate arclength along the bent ray; not proper path length or coordinate light travel time.' : 'c=1 coordinate ray distance in the retarded optical view.'),
        row('optical-path-si', 'Isotropic ray arclength', star && unitLength !== null && pathDistance !== null ? pathDistance * unitLength : null, 'm'),
        row('source-lapse', 'Static emission lapse', sourceA), row('receiver-lapse', 'Static reception lapse', metric?.lapse ?? null),
        row('gr-frequency-ratio', 'Static gravitational frequency ratio', sourceA !== null && metric ? sourceA / metric.lapse : null),
        row('local-doppler-ratio', 'Local receiver SR Doppler ratio', D !== null && sourceA !== null && metric ? D * metric.lapse / sourceA : null, '', 'Factor after removing the static emission/reception lapse ratio.'),
        textRow('color-display', 'Doppler color presentation', view.settings?.doppler === false ? 'Off' : 'On'),
        textRow('intensity-display', 'D⁴ intensity presentation', view.settings?.beaming === true ? 'On' : 'Off'),
    ]));
    if (blackHole) {
        const horizonSection = sections.find(group => group.id === 'horizon');
        horizonSection?.rows.push(
            textRow('ray-measurement-source', 'Current ray measurement source', ray?.source ?? null, '', 'Live readings use the Float64 optical witness; production GPU accuracy is tested separately through floating-point framebuffer readback.'),
            row('ray-impact-parameter', 'Sight-ray null impact parameter', ray && unitLength !== null ? finite(ray.impactParameter) === null ? null : ray.impactParameter * unitLength : null, 'm'),
            row('observer-exterior-guard', 'Observer numerical boundary ρ', star?.observerBoundary ?? null, 'isotropic units', 'Exterior integration stops here or at the speed limit. This is a numerical guard, not a solid horizon, impact or physical stopping event.'),
        );
    }
    const rendering = view.rendering;
    sections.push(section('rendering', 'Rendering and observation budget', 'Renderer diagnostics describe the current presentation. Hardware identity or frame timing alone does not certify optical accuracy.', [
        textRow('resolution', 'Internal image resolution', Array.isArray(rendering?.internalResolution) ? rendering.internalResolution.join(' × ') : null, 'px'),
        row('render-scale', 'Internal render scale', rendering?.internalScale), row('requested-scale', 'Requested render scale', rendering?.requestedScale),
        textRow('adaptive-quality', 'Adaptive quality', rendering ? rendering.adaptiveQuality === false ? 'Off' : 'On' : null),
        row('render-time', 'Last measured GPU render duration', rendering && rendering.gpuTimeMs > 0 ? rendering.gpuTimeMs : null, 'ms', 'GPU timer-query measurement; unavailable until a completed query result is published. This is not the owning-frame interval or a physics step duration.'),
        row('gpu-budget', 'Adaptive timing budget', rendering?.adaptiveGpuBudgetMs, 'ms'),
        row('traced-instances', 'Traced instances', rendering?.tracedInstances), row('moving-instances', 'Moving instances', rendering?.movingInstances),
        textRow('gpu-vendor', 'GPU vendor', rendering?.gpu?.vendor), textRow('gpu-renderer', 'GPU renderer', rendering?.gpu?.renderer),
        textRow('float-readback', 'Float readback capability', rendering?.gpu ? rendering.gpu.floatReadback ? 'Available' : 'Unavailable' : null),
        textRow('environment-status', 'Environment status', rendering?.environmentStatus),
    ]));
    if (view.lattice) {
        const lattice = view.lattice, current = lattice.available === true && lattice.stale !== true;
        sections.push(section('lattice', 'Read-only lattice link', 'Completed lattice publications have their own owner and tick. No lattice-to-SR matter, mass, clock or GR metric identification is made.', [
            textRow('lattice-status', 'Publication status', lattice.status), textRow('lattice-backend', 'Backend', lattice.backend),
            counterRow('lattice-tick', 'Lattice sample tick', current ? lattice.sampleTick : null), textRow('lattice-source', 'Lattice source identity', current ? lattice.sourceId : null),
            row('lattice-size', 'Lattice size', current ? lattice.latticeSize : null), row('lattice-manifested', 'Manifested count', current ? lattice.manifested : null),
            textRow('lattice-law', 'Selected law', current ? lattice.lawId : null),
        ]));
    }
    return sections;
}
