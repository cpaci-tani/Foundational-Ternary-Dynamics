// @ts-check
/** Bounded reference instruments for adopted Minkowski physics, c = 1.
 * This module reads snapshots only; it does not infer physics from the lattice.
 */
import { aberration, clamp, dot, gamma, normalize, scale, sub } from './math.js';
import { cameraBasis } from './optics.js';

/** @typedef {import('./optics.js').OpticalSnapshot} OpticalSnapshot */
/** @typedef {import('./optics.js').OpticalHit} OpticalHit */
/** @typedef {import('./optics.js').OpticalSettings & {layers?:Record<string,boolean>}} PhenomenaSettings */
/** @typedef {{entityId:string,revision:number,emissionTime:number,arrivalTime:number,delay:number,distance:number,properTime:number,doppler:number,nullResidual:number,historical:boolean,mirrored:boolean,position:number[],xi:number}} ReceivedEvent */
/** @typedef {{label:string,original:number[],received:number[],angle:number}} CompassDirection */
/** @typedef {{id:string,revision:number,points:[number,number][]}} ProjectedTrail */
/** @typedef {{available:boolean,reason:string,axis:number[],axisLabel:string,beta:number,gamma:number,parallelBeta:number,clockRate:number,time:number,historyStart:number,received:ReceivedEvent|null,compass:CompassDirection[],trails:ProjectedTrail[]}} PhenomenaData */

const TRAIL_SECONDS = 20;
const TRAIL_SEGMENTS = 16;
const TRAIL_SAMPLES = 8;
const HIT_TIME_TOLERANCE = 1e-5;
/** @type {readonly [string,number[]][]} */
const COMPASS_AXES = [
    ['+X', [1, 0, 0]], ['−X', [-1, 0, 0]],
    ['+Y', [0, 1, 0]], ['−Y', [0, -1, 0]],
    ['+Z', [0, 0, 1]], ['−Z', [0, 0, -1]],
];

/** @param {number[]} value */
const finiteVector = value => value.length === 3 && value.every(Number.isFinite);

/** Select actual received events, never a simultaneous-geometry comparison hit.
 * The time fence rejects cached hits from an earlier snapshot. The geometric
 * null residual is computed independently from the surface's actual position.
 * @param {OpticalSnapshot} snapshot @param {PhenomenaSettings} settings
 * @param {OpticalHit|null} hit @param {number[]} observerPosition
 * @returns {ReceivedEvent|null}
 */
function receivedEvent(snapshot, settings, hit, observerPosition) {
    if (settings.optical === false || !hit || !finiteVector(hit.position)) return null;
    const numbers = [hit.emissionTime, hit.distance, hit.properTime, hit.doppler];
    if (!numbers.every(Number.isFinite) || hit.distance < 0 || hit.doppler <= 0) return null;
    if (hit.emissionTime < snapshot.historyStart || hit.emissionTime > snapshot.time
        || Math.abs(hit.emissionTime + hit.distance - snapshot.time) > HIT_TIME_TOLERANCE) return null;
    const position = [...hit.position], displacement = sub(position, observerPosition);
    const distance = Math.hypot(...displacement), delay = snapshot.time - hit.emissionTime;
    // Same-time relocation can leave a temporally plausible cached hit. Require
    // its actual surface displacement to remain null relative to this observer.
    if (Math.abs(distance - delay) > 1e-7 * Math.max(1, distance, delay)) return null;
    return {
        entityId: hit.entityId, revision: hit.revision,
        emissionTime: hit.emissionTime, arrivalTime: snapshot.time,
        delay, distance, properTime: hit.properTime, doppler: hit.doppler,
        nullResidual: delay * delay - dot(displacement, displacement),
        historical: hit.historical, mirrored: hit.mirrored, position, xi: 0,
    };
}

/** Canonical source trajectories: separate arrays preserve revision boundaries.
 * Samples are privileged coordinate events, not received optical positions.
 * @param {OpticalSnapshot} snapshot @param {string|null|undefined} selectedId
 * @param {number[]} observerPosition @param {number[]} axis
 * @returns {ProjectedTrail[]}
 */
function projectedTrails(snapshot, selectedId, observerPosition, axis) {
    if (!selectedId) return [];
    const earliest = Math.max(snapshot.historyStart, snapshot.time - TRAIL_SECONDS);
    /** @type {ProjectedTrail[]} */
    const trails = [];
    for (let index = snapshot.segments.length - 1; index >= 0 && trails.length < TRAIL_SEGMENTS; index--) {
        const segment = snapshot.segments[index];
        if ((segment.entityId || segment.id) !== selectedId
            || !finiteVector(segment.position) || !finiteVector(segment.velocity)) continue;
        const start = Math.max(earliest, segment.start ?? earliest);
        const end = Math.min(snapshot.time, segment.end ?? snapshot.time);
        const origin = segment.originTime ?? 0;
        if (!Number.isFinite(start) || !Number.isFinite(end) || !Number.isFinite(origin) || end <= start) continue;
        /** @type {[number,number][]} */
        const points = [];
        for (let sample = 0; sample < TRAIL_SAMPLES; sample++) {
            const time = start + (end - start) * sample / (TRAIL_SAMPLES - 1);
            const position = segment.position.map((value, component) => value + segment.velocity[component] * (time - origin));
            points.push([dot(sub(position, observerPosition), axis), time - snapshot.time]);
        }
        if (points.every(point => point.every(Number.isFinite))) trails.push({ id: selectedId, revision: segment.revision, points });
    }
    return trails.reverse();
}

/** Build reference readouts without changing snapshots, settings, or hit records.
 * All times and coordinates are the session's normalized coordinate frame.
 * Distant compass directions denote source look directions; aberration accepts
 * the opposite photon propagation vector, hence both sign changes below.
 * @param {OpticalSnapshot} snapshot @param {PhenomenaSettings} [settings]
 * @param {OpticalHit|null} [hit] @returns {PhenomenaData}
 */
export function phenomenaData(snapshot, settings = {}, hit = null) {
    const observer = { ...snapshot.observer, ...settings.cameraOverride };
    /** @type {PhenomenaData} */
    const result = {
        available: false, reason: 'Special-relativity instruments are unavailable in Playground.',
        axis: [...cameraBasis(observer).forward], axisLabel: 'Camera forward',
        beta: 0, gamma: 1, parallelBeta: 0, clockRate: 1,
        time: snapshot.time, historyStart: snapshot.historyStart,
        received: null, compass: [], trails: [],
    };
    if (snapshot.profile !== 'sr') return result;
    if (!finiteVector(observer.position) || !finiteVector(observer.velocity)
        || !Number.isFinite(snapshot.time) || !Number.isFinite(snapshot.historyStart)
        || dot(observer.velocity, observer.velocity) >= 1) {
        result.reason = 'A finite timelike observer event is required.';
        return result;
    }
    result.available = true; result.reason = '';
    result.beta = Math.hypot(...observer.velocity);
    result.gamma = gamma(observer.velocity); result.clockRate = 1 / result.gamma;
    result.received = receivedEvent(snapshot, settings, hit, observer.position);
    const receivedDisplacement = result.received ? sub(result.received.position, observer.position) : null;
    if (settings.layers?.lightPaths && receivedDisplacement && Math.hypot(...receivedDisplacement) > 1e-12) {
        result.axis = normalize(receivedDisplacement); result.axisLabel = 'Received surface direction';
    } else if (result.beta > 1e-12) {
        result.axis = normalize(observer.velocity); result.axisLabel = 'Observer motion';
    }
    result.parallelBeta = dot(observer.velocity, result.axis);
    if (result.received && receivedDisplacement) result.received.xi = dot(receivedDisplacement, result.axis);
    result.compass = COMPASS_AXES.map(([label, direction]) => {
        const original = [...direction], received = scale(aberration(scale(original, -1), observer.velocity), -1);
        return { label, original, received, angle: Math.acos(clamp(dot(original, received), -1, 1)) };
    });
    result.trails = projectedTrails(snapshot, settings.selectedId, observer.position, result.axis);
    return result;
}
