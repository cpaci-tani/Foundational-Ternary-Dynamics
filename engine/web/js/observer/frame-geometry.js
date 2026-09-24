// @ts-check
/** Privileged simultaneous-frame reference geometry, using adopted SR with c=1. */
import { dot, gamma } from './math.js';
import { rotationMatrix, rotate } from './geometry.js';

/** @typedef {{position:number[],time:number}} FrameEvent */
/** @typedef {{a:number[],b:number[],startTime:number,endTime:number,revision:number}} FrameEdge */

/** Intersect each inertial rest-box corner worldline with the observer's now plane.
 * Rest offsets first contract along the body's velocity at equal coordinate time.
 * Then dt=v_observer·separation/(1-v_observer·v_body) locates observer simultaneity.
 * The caller must clip these events to the revision lifetime before drawing them.
 * @param {import('./optics.js').CameraState} observer
 * @param {number} time
 * @param {import('./optics.js').OpticalSegment} segment
 * @param {boolean} [mirrored]
 * @returns {FrameEvent[]}
 */
export function simultaneousBoundsCorners(observer, time, segment, mirrored = false) {
    const reflect = (/** @type {number[]} */ point) => point.map((x, j) => mirrored && j === 1 ? -x : x);
    const velocity = reflect(segment.velocity), speedSquared = dot(velocity, velocity), inverseGamma = 1 / gamma(velocity);
    const center = reflect(segment.position).map((x, j) => x + velocity[j] * (time - (segment.originTime ?? 0)));
    const rotation = rotationMatrix(segment.rotation), denominator = 1 - dot(observer.velocity, velocity);
    /** @type {FrameEvent[]} */ const corners = [];
    for (const x of [-0.5, 0.5]) for (const y of [-0.5, 0.5]) for (const z of [-0.5, 0.5]) {
        const rest = reflect(rotate(rotation, [x * segment.size[0], y * segment.size[1], z * segment.size[2]]));
        const coefficient = speedSquared > 1e-20 ? (inverseGamma - 1) * dot(velocity, rest) / speedSquared : 0;
        const coordinate = rest.map((value, j) => value + velocity[j] * coefficient + center[j]);
        const deltaTime = dot(observer.velocity, coordinate.map((value, j) => value - observer.position[j])) / denominator;
        corners.push({ position: coordinate.map((value, j) => value + velocity[j] * deltaTime), time: time + deltaTime });
    }
    return corners;
}

/** Clip each simultaneous edge to the source revision and retained history.
 * Open-ended inertial revisions permit analytic future continuation; closed
 * revisions and missing history never get replaced with the current pose.
 * @param {import('./optics.js').CameraState} observer
 * @param {number} time
 * @param {import('./optics.js').OpticalSegment} segment
 * @param {number} historyStart
 * @param {boolean} [mirrored]
 * @returns {FrameEdge[]}
 */
export function simultaneousBoundsEdges(observer, time, segment, historyStart, mirrored = false) {
    const start = Math.max(historyStart, segment.start ?? -Infinity), end = segment.end ?? Infinity;
    if (start >= end) return [];
    const velocity = segment.velocity.map((x, j) => mirrored && j === 1 ? -x : x);
    const center = segment.position.map((x, j) => (x + segment.velocity[j] * (time - (segment.originTime ?? 0))) * (mirrored && j === 1 ? -1 : 1));
    const denominator = 1 - dot(observer.velocity, velocity);
    const centerTime = time + dot(observer.velocity, center.map((x, j) => x - observer.position[j])) / denominator;
    // Contraction cannot enlarge the rest circumsphere. This conservative time
    // bound cheaply rejects revisions far from the simultaneity surface.
    const timeRadius = Math.hypot(...observer.velocity) * Math.hypot(...segment.size) * 0.5 / denominator;
    if (centerTime + timeRadius < start || centerTime - timeRadius >= end) return [];
    const corners = simultaneousBoundsCorners(observer, time, segment, mirrored);
    /** @type {FrameEdge[]} */ const edges = [];
    for (let index = 0; index < 8; index++) for (const bit of [1, 2, 4]) {
        if (index & bit) continue;
        const a = corners[index], b = corners[index | bit], dt = b.time - a.time;
        let low = 0, high = 1;
        if (Math.abs(dt) < 1e-14) {
            if (a.time < start || a.time >= end) continue;
        } else {
            const p = (start - a.time) / dt, q = (end - a.time) / dt;
            low = Math.max(low, Math.min(p, q)); high = Math.min(high, Math.max(p, q));
            if (high <= low) continue;
        }
        edges.push({
            a: a.position.map((value, j) => value + (b.position[j] - value) * low),
            b: a.position.map((value, j) => value + (b.position[j] - value) * high),
            startTime: a.time + dt * low, endTime: a.time + dt * high, revision: segment.revision,
        });
    }
    return edges;
}
