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
/** @typedef {{id:string,label:string,value:number|string|null,unit:string,note:string}} TelemetryRow */
/** @typedef {{id:string,title:string,description:string,rows:TelemetryRow[]}} TelemetrySection */
/** @typedef {{hit?:import('./optics.js').OpticalHit|null,cameraOverride?:Partial<import('./optics.js').CameraState>,selectedId?:string|null,settings?:{optical?:boolean,doppler?:boolean,beaming?:boolean},rendering?:Record<string,any>,lattice?:Record<string,any>}} TelemetryView */
/** @param {unknown} value @returns {number|null} */
const finite = value => typeof value === 'number' && Number.isFinite(value) ? value : null;
/** @param {string} id @param {string} label @param {unknown} value @param {string} [unit] @param {string} [note] @returns {TelemetryRow} */
const row = (id, label, value, unit = '', note = '') => ({ id, label, value: typeof value === 'string' ? value : finite(value), unit, note });
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

/** The caller supplies only a sight-ray hit belonging to this snapshot/view.
 * Snapshot and camera comparison previews are read without advancing simulation.
 * @param {import('./types.js').WorldSnapshot} snapshot @param {TelemetryView} [view]
 * @returns {TelemetrySection[]}
 */
export function observationTelemetry(snapshot, view = {}) {
    const star = snapshot.spacetime, sr = snapshot.profile === 'sr';
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
        section('model', 'Reference and session', star ? 'Adopted Schwarzschild vacuum exterior with local SR. Fixed emitting surface; no stellar interior or native FTD identification.' : sr ? 'Adopted Minkowski special relativity, c=1. SI length and time scales are unspecified.' : 'Classical Playground. Its simulation units and body mechanics do not define SR clocks or GR geometry.', [
            row('reference', 'Reference model', star ? 'Schwarzschild exterior + local SR' : sr ? 'Minkowski SR' : 'Classical Playground'),
            row('engine', 'Physics engine', snapshot.physicsEngine), row('integrator', 'Integrator', snapshot.integratorVersion ?? 'Unspecified'),
            row('session', 'Session', snapshot.sessionId), row('epoch', 'Epoch', snapshot.epoch), row('tick', 'Tick', snapshot.tick),
            row('playing', 'Transport', snapshot.playing ? 'Playing' : 'Paused'), row('playback', 'Playback rate', snapshot.playbackSpeed, '×', 'Presentation rate per wall-clock second; no change to physical clock conventions.'),
            row('camera-preview', 'Camera comparison', preview ? 'Preview' : 'Session observer'),
            row('units', 'Source units', snapshot.units), row('body-count', 'Live bodies', snapshot.entities.filter(entity => entity.alive).length),
            row('warning-count', 'Session warnings', snapshot.warnings.length),
        ]),
        section('time', 'Time and retained history', 'Worldline clock differences use its recorded coordinate origin. Relocation starts a new clock; scrubbing and camera previews do not define a paired clock comparison.', [
            row('coordinate-time', 'Coordinate time t', snapshot.time, timeUnit),
            row('proper-time', sr ? snapshot.scrubTime !== null ? 'Retained observer proper time τ' : 'Observer proper time τ' : 'Observer elapsed clock', snapshot.observer.properTime, sr ? 'proper time units' : timeUnit, snapshot.scrubTime !== null ? 'Source-history scrubbing rewinds coordinate time and source entities; it does not rewind the retained observer worldline clock.' : preview ? 'Accumulated session observer clock; a camera comparison does not integrate a new clock.' : 'Accumulated on the recorded observer worldline.'),
            row('worldline', 'Observer worldline', snapshot.observer.worldline), row('worldline-reason', 'Worldline origin', snapshot.observer.worldlineReason ?? 'Unspecified'),
            row('worldline-start', 'Coordinate clock origin', start, timeUnit), row('coordinate-elapsed', 'Coordinate elapsed on worldline', elapsed, timeUnit),
            row('clock-slip', 'Coordinate elapsed − observer clock', slip, timeUnit, clockPair ? 'Compared from the same worldline origin.' : 'Unavailable without a matching clock origin or during scrub/preview.'),
            row('clock-rate', sr ? 'Instantaneous dτ/dt' : 'Classical clock rate', rate, '', (snapshot.scrubTime !== null ? 'Uses the current observer velocity; source-history scrubbing does not rewind observer motion. ' : '') + (star ? 'A/γ; Schwarzschild coordinate time normalized to infinity.' : sr ? '1/γ in the Minkowski coordinate frame.' : 'Classical elapsed clock follows simulation time.')),
            row('coordinate-time-si', 'Coordinate time t', unitTime === null ? null : snapshot.time * unitTime, 's', 'SI conversion exists only for the calibrated compact-star reference.'),
            row('proper-time-si', 'Observer clock τ', unitTime === null ? null : snapshot.observer.properTime * unitTime, 's'),
            row('clock-slip-si', 'Paired clock difference', unitTime === null || slip === null ? null : slip * unitTime, 's'),
            row('history-start', 'Optical history begins', snapshot.historyStart, timeUnit), row('history-end', 'Viewed history ends', snapshot.time, timeUnit),
            row('history-span', 'Retained interval span', snapshot.time - snapshot.historyStart, timeUnit, 'Includes deliberately prepared source prehistory when present.'),
            row('history-policy', 'History policy window', snapshot.historyWindow, timeUnit), row('history-segments', 'Retained worldline segments', snapshot.segments.length),
            row('backlog', 'Queued integration time', snapshot.backlogSeconds, timeUnit),
            row('time-scale', 'One coordinate time unit', unitTime, 's', 'One length unit / c, from the compact-star scale.'),
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
            row('motion-mode', 'Motion mode', star?.observerMode ?? (sr ? 'Minkowski controls' : 'Classical controls')),
            row('speed-cap', 'Session SR speed limit', sr ? MAX_BETA : null, 'c', 'Implementation boundary for this reference experiment.'),
            row('cap-applied', 'Speed cap applied', observer.capApplied === true ? 'Yes' : 'No'),
        ]),
        section('distance', 'Position and distance', star ? 'Isotropic coordinates, areal radius and proper radial distance are distinct. Proper distance is on a constant Schwarzschild-time slice.' : 'Positions and center distances belong to the displayed coordinate frame. No SI length scale is assigned.', [
            ...observer.position.map((v, i) => row(`position-${i}`, `Position ${['X', 'Y', 'Z'][i]}`, v, coordinateUnit)),
            row('selected-distance', 'Selected body center distance', selected ? length(sub(observer.position, selected.position)) : null, coordinateUnit, 'Coordinate center-to-center distance; not optical travel distance.'),
            row('isotropic-radius', 'Star-centered isotropic radius ρ', metric?.rho ?? null, coordinateUnit),
            row('isotropic-radius-si', 'Isotropic radius ρ', metric && unitLength !== null ? metric.rho * unitLength : null, 'm'),
            row('areal-radius', 'Areal radius r', metric && unitLength !== null ? metric.areal * unitLength : null, 'm', 'Sphere circumference / 2π.'),
            row('areal-height', 'Areal radius − stellar R', metric && star && unitLength !== null ? (metric.areal - star.radiusKm * 1000 / unitLength) * unitLength : null, 'm', 'Radius difference; not proper radial distance.'),
            row('proper-radial-distance', 'Proper radial distance above surface', metric && star && unitLength !== null ? properRadialDistance(metric.areal * unitLength, star.radiusKm * 1000, star.rs * unitLength) : null, 'm', 'Integral of dr/√(1−rs/r) on a static time slice.'),
            row('radius-rs', 'Areal radius / Schwarzschild radius', metric && star && star.rs > 0 ? metric.areal / star.rs : null),
            row('length-scale', 'One displayed length unit', unitLength, 'm'),
            row('outer-boundary', 'Outer isotropic optical boundary', star?.escapeRadius ?? null, 'isotropic units', 'Finite tracer/domain boundary; not a cosmological horizon.'),
        ]),
        section('mass', star ? 'Compact-star mass and surface' : 'Mass conventions', star ? 'Nominal solar GM calibrates the vacuum metric. Mass in kilograms is a GM/G estimate. The interior and its density profile are not simulated.' : 'Authored body masses are simulation parameters. A rest mass or SI mass calibration has not been assigned to the observer.', [
            row('observer-mass', 'Observer rest mass', null, 'kg', 'Observer is a test worldline without an assigned rest mass.'),
            row('selected-mass', 'Selected body mass parameter', selected?.mass ?? null, 'simulation mass units', star ? 'Surface marker mass does not set the stellar metric.' : 'Source parameter; no kilogram calibration.'),
            row('star-mass-solar', 'Star nominal solar-GM multiples', star?.massSolar ?? null, 'GM☉ᴺ', 'Mass convention from IAU nominal solar mass parameter.'),
            row('star-gm', 'Stellar GM', star ? star.massSolar * NOMINAL_SOLAR_GM : null, 'm³/s²'),
            row('star-mass-kg', 'Nominal GM / CODATA G', star ? star.massSolar * NOMINAL_SOLAR_GM / MASS_CONVERSION_G : null, 'kg', 'Estimate inherits G uncertainty (~2.2×10⁻⁵ relative). Not an exact nominal solar mass.'),
            row('stellar-radius', 'Stellar areal radius R', star ? star.radiusKm * 1000 : null, 'm'),
            row('schwarzschild-radius', 'Schwarzschild radius rs', star && unitLength !== null ? star.rs * unitLength : null, 'm'),
            row('mean-density-proxy', 'Mass / Euclidean areal-volume proxy', star ? star.massSolar * NOMINAL_SOLAR_GM / MASS_CONVERSION_G / (4 * Math.PI * (star.radiusKm * 1000) ** 3 / 3) : null, 'kg/m³', 'M/(4πR³/3); this is not a GR proper-volume density or an interior EOS.'),
        ]),
    ];
    const rsMeters = star && unitLength !== null ? star.rs * unitLength : null;
    const rMeters = metric && unitLength !== null ? metric.areal * unitLength : null;
    const GM = star ? star.massSolar * NOMINAL_SOLAR_GM : null;
    const surfaceA = star ? starMetric(star, [star.center[0] + star.radius, star.center[1], star.center[2]]).lapse : null;
    sections.push(section('gravity', 'GR geometry and reference fields', star ? 'Schwarzschild vacuum reference quantities outside the emitting surface. Hover acceleration and tidal coefficients are local reference values, not extended-body or stellar-interior responses.' : 'GR telemetry is available in the compact-star reference experiment.', [
        row('lapse', 'Local lapse A', metric?.lapse ?? null, '', 'Static proper time / coordinate time normalized at infinity.'),
        row('spatial-factor', 'Isotropic spatial factor B', metric?.spatial ?? null, '', 'Static proper length = B × isotropic coordinate length locally.'),
        row('optical-index', 'Coordinate optical index n', metric?.index ?? null, '', 'B/A; coordinate light speed magnitude = 1/n. Local measured light speed remains c.'),
        row('coordinate-light-speed', 'Coordinate light speed', metric ? 1 / metric.index : null, 'c'),
        row('local-compactness', 'Local rs/r', metric && star ? star.rs / metric.areal : null),
        row('surface-compactness', 'Surface rs/R', star && rsMeters !== null ? rsMeters / (star.radiusKm * 1000) : null),
        row('surface-lapse', 'Surface static clock lapse', surfaceA),
        row('surface-redshift', 'Surface redshift to infinity', surfaceA === null ? null : 1 / surfaceA - 1, '', 'Static source and static receiver at infinity; the current received-light shift differs.'),
        row('observer-redshift', 'Local static redshift to infinity', metric ? 1 / metric.lapse - 1 : null),
        row('hover-acceleration', 'Static hover proper acceleration', GM !== null && rMeters !== null && metric ? GM / (rMeters ** 2 * metric.lapse) : null, 'm/s²', 'Acceleration required to hold a static worldline at this radius; not the guided observer thrust.'),
        row('surface-hover-acceleration', 'Surface static hover acceleration', GM !== null && star && surfaceA !== null ? GM / ((star.radiusKm * 1000) ** 2 * surfaceA) : null, 'm/s²'),
        row('radial-tidal', 'Radial tidal stretching coefficient', GM !== null && rMeters !== null ? 2 * GM / rMeters ** 3 : null, 's⁻²', 'Radial geodesic-deviation acceleration per local separation for radial freely falling orthonormal observers.'),
        row('transverse-tidal', 'Transverse tidal compression coefficient', GM !== null && rMeters !== null ? -GM / rMeters ** 3 : null, 's⁻²'),
        row('kretschmann', 'Kretschmann curvature invariant', rsMeters !== null && rMeters !== null ? 12 * rsMeters ** 2 / rMeters ** 6 : null, 'm⁻⁴', 'R_abcd R^abcd = 12 rs²/r⁶ in the Schwarzschild vacuum exterior.'),
        row('escape-speed', 'Local escape-to-infinity speed', metric && star ? Math.sqrt(star.rs / metric.areal) : null, 'c', 'Ideal radial test-particle energy threshold Aγ=1, extending the reference metric to infinity beyond the finite optical boundary.'),
        row('photon-sphere', 'Vacuum photon-sphere radius', rsMeters === null ? null : 1.5 * rsMeters, 'm', 'Formal vacuum radius. This preparation places it inside the opaque surface; no interior photon orbit is simulated.'),
        row('isco-radius', 'Vacuum circular-orbit ISCO radius', rsMeters === null ? null : 3 * rsMeters, 'm', 'Timelike test-particle circular-orbit stability reference; applicable only outside the stellar surface.'),
    ]));
    const optical = !!star || sr && view.settings?.optical !== false;
    const candidate = optical ? view.hit : null;
    const hit = candidate && Number.isFinite(candidate.emissionTime) && candidate.emissionTime <= snapshot.time && candidate.emissionTime >= snapshot.historyStart ? candidate : null;
    const D = hit && hit.doppler > 0 ? finite(hit.doppler) : null;
    const delay = hit ? snapshot.time - hit.emissionTime : null;
    const sourceA = hit && star ? hit.entityId === star.sourceId ? surfaceA : starMetric(star, hit.sourcePosition).lapse : null;
    sections.push(section('optics', 'Received light', optical ? 'Current sight-ray witness only. Frequency shift combines emission and reception conditions; source and observer clocks can have different origins. Color and intensity switches affect presentation.' : 'A simultaneous geometry view does not supply received-light measurements. Enable the SR optical view to inspect light travel and frequency shift.', [
        row('optical-mode', 'Observation mode', optical ? star ? 'Curved null ray + local SR' : 'Retarded Minkowski null ray' : 'Simultaneous geometry'),
        row('optical-status', 'Current sight ray', hit ? 'Source hit' : 'No current source hit'),
        row('source', 'Received source identity', hit?.entityId ?? null), row('source-revision', 'Received source revision', hit?.revision ?? null),
        row('frequency-ratio', 'Frequency ratio D = f_received/f_emitted', D), row('redshift', 'Received redshift z = 1/D − 1', D === null ? null : 1 / D - 1),
        row('wavelength-ratio', 'Wavelength ratio λ_received/λ_emitted', D === null ? null : 1 / D),
        row('bolometric-factor', 'Reference bolometric intensity ratio D⁴', D === null ? null : D ** 4, '', 'Reference ray-intensity factor; not a total luminosity or flux at the observer.'),
        row('emission-time', 'Coordinate emission time', hit?.emissionTime ?? null, timeUnit),
        row('source-clock', 'Emitting source proper clock', hit?.properTime ?? null, 'proper time units'),
        row('optical-delay', 'Coordinate light travel time', delay, timeUnit),
        row('optical-delay-si', 'Coordinate light travel time', unitTime === null || delay === null ? null : delay * unitTime, 's'),
        row('optical-path', star ? 'Isotropic ray arclength' : 'Coordinate null-ray distance', hit?.distance ?? null, coordinateUnit, star ? 'Euclidean isotropic-coordinate arclength along the bent ray; not proper path length or coordinate light travel time.' : 'c=1 coordinate ray distance in the retarded optical view.'),
        row('optical-path-si', 'Isotropic ray arclength', star && unitLength !== null && hit ? hit.distance * unitLength : null, 'm'),
        row('source-lapse', 'Static emission lapse', sourceA), row('receiver-lapse', 'Static reception lapse', metric?.lapse ?? null),
        row('gr-frequency-ratio', 'Static gravitational frequency ratio', sourceA !== null && metric ? sourceA / metric.lapse : null),
        row('local-doppler-ratio', 'Local receiver SR Doppler ratio', D !== null && sourceA !== null && metric ? D * metric.lapse / sourceA : null, '', 'Factor after removing static emission/reception lapse ratio; compact-star source is static.'),
        row('color-display', 'Doppler color presentation', view.settings?.doppler === false ? 'Off' : 'On'),
        row('intensity-display', 'D⁴ intensity presentation', view.settings?.beaming === true ? 'On' : 'Off'),
    ]));
    const rendering = view.rendering;
    sections.push(section('rendering', 'Rendering and observation budget', 'Renderer diagnostics describe the current presentation. Hardware identity or frame timing alone does not certify optical accuracy.', [
        row('resolution', 'Internal image resolution', Array.isArray(rendering?.internalResolution) ? rendering.internalResolution.join(' × ') : null, 'px'),
        row('render-scale', 'Internal render scale', rendering?.internalScale), row('requested-scale', 'Requested render scale', rendering?.requestedScale),
        row('adaptive-quality', 'Adaptive quality', rendering ? rendering.adaptiveQuality === false ? 'Off' : 'On' : null),
        row('render-time', 'Last measured GPU render duration', rendering && rendering.gpuTimeMs > 0 ? rendering.gpuTimeMs : null, 'ms', 'GPU timer-query measurement; unavailable until a completed query result is published. This is not the owning-frame interval or a physics step duration.'),
        row('gpu-budget', 'Adaptive timing budget', rendering?.adaptiveGpuBudgetMs, 'ms'),
        row('traced-instances', 'Traced instances', rendering?.tracedInstances), row('moving-instances', 'Moving instances', rendering?.movingInstances),
        row('gpu-vendor', 'GPU vendor', rendering?.gpu?.vendor), row('gpu-renderer', 'GPU renderer', rendering?.gpu?.renderer),
        row('float-readback', 'Float readback capability', rendering?.gpu ? rendering.gpu.floatReadback ? 'Available' : 'Unavailable' : null),
        row('environment-status', 'Environment status', rendering?.environmentStatus),
    ]));
    if (view.lattice) {
        const lattice = view.lattice, current = lattice.available === true && lattice.stale !== true;
        sections.push(section('lattice', 'Read-only lattice link', 'Completed lattice publications have their own owner and tick. No lattice-to-SR matter, mass, clock or GR metric identification is made.', [
            row('lattice-status', 'Publication status', lattice.status), row('lattice-backend', 'Backend', lattice.backend),
            row('lattice-tick', 'Lattice sample tick', current ? lattice.sampleTick : null), row('lattice-source', 'Lattice source identity', current ? lattice.sourceId : null),
            row('lattice-size', 'Lattice size', current ? lattice.latticeSize : null), row('lattice-manifested', 'Manifested count', current ? lattice.manifested : null),
            row('lattice-law', 'Selected law', current ? lattice.lawId : null),
        ]));
    }
    return sections;
}
