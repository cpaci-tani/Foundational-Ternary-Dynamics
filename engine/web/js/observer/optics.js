/** Float64 optical oracle for an adopted Minkowski reference model; c = 1. */
import { dot, cross, normalize, rotationMatrix, inverseRotate, rotate, intersectShape, buildBVH, hitBounds } from './geometry.js';

/** @typedef {{position:number[],velocity:number[],properTime?:number,yaw?:number,pitch?:number,roll?:number}} CameraState */
/** @typedef {{entityId?:string,id?:string,revision:number,start?:number,end?:number|null,originTime?:number,position:number[],velocity:number[],size:number[],rotation:number[],shape:string,color:number[],spectral?:string,emission?:number,clockOffset?:number,alive?:boolean,name?:string,overlay?:unknown,decorative?:boolean,animationRate?:number}} OpticalSegment */
/** @typedef {{time:number,profile:string,historyStart:number,observer:CameraState,entities:OpticalSegment[],segments:OpticalSegment[],environment?:object,environmentHistory?:{start:number,end:number|null,revision:number,environment:object}[],pulses?:{origin:number[],start:number,color:number[]}[],epoch?:number,revision?:number}} OpticalSnapshot */
/** @typedef {{fov?:number,aspect?:number,optical?:boolean,cameraOverride?:Partial<CameraState>,selectedId?:string|null,[key:string]:unknown}} OpticalSettings */
/** @typedef {{entityId:string,id:string,revision:number,distance:number,emissionTime:number,properTime:number,position:number[],sourcePosition:number[],restPosition:number[],normal:number[],historical:boolean,mirrored:boolean,segmentIndex:number,doppler:number}} OpticalHit */
/** @typedef {{segments:OpticalSegment[],bvh:ReturnType<typeof buildBVH>,offset:number[]}} TraceAcceleration */
/** @param {number[]} v */
export function gamma(v) { const b2 = dot(v, v); if (b2 >= 1) throw new RangeError('SR velocities must be strictly below c.'); return 1 / Math.sqrt(1 - b2); }
/** @param {CameraState} observer */
export function cameraBasis(observer) {
    const yaw = observer.yaw || 0, pitch = observer.pitch || 0, roll = observer.roll || 0;
    const forward = [-Math.sin(yaw) * Math.cos(pitch), Math.sin(pitch), -Math.cos(yaw) * Math.cos(pitch)];
    const right = [Math.cos(yaw), 0, -Math.sin(yaw)], up = cross(right, forward);
    return { forward, right: right.map((x, j) => x * Math.cos(roll) + up[j] * Math.sin(roll)), up: up.map((x, j) => x * Math.cos(roll) - right[j] * Math.sin(roll)) };
}
/** Lorentz boost of a displacement from the rest frame to the coordinate frame.
 * @param {number} time @param {number[]} space @param {number[]} velocity
 */
export function boost(time, space, velocity) {
    const g = gamma(velocity), b2 = dot(velocity, velocity), projection = dot(velocity, space);
    const k = b2 > 1e-20 ? (g - 1) * projection / b2 + g * time : 0;
    return { time: g * (time + projection), space: space.map((x, j) => x + k * velocity[j]) };
}
/** @param {OpticalSnapshot} snapshot @param {OpticalSettings} settings @param {number} ndcX @param {number} ndcY */
export function observerRay(snapshot, settings = {}, ndcX = 0, ndcY = 0) {
    const observer = { ...snapshot.observer, ...settings.cameraOverride }, basis = cameraBasis(observer);
    const tangent = Math.tan((settings.fov || 60) * Math.PI / 360), aspect = settings.aspect || 1;
    const restDirection = normalize(basis.forward.map((x, j) => x + ndcX * tangent * aspect * basis.right[j] + ndcY * tangent * basis.up[j]));
    if (snapshot.profile !== 'sr') return { origin: observer.position, direction: restDirection, timeSlope: 0, restDirection };
    const optical = settings.optical !== false, transformed = boost(optical ? -1 : 0, restDirection, observer.velocity);
    const scale = optical ? -transformed.time : 1;
    return { origin: observer.position, direction: transformed.space.map(x => x / scale), timeSlope: optical ? -1 : transformed.time, restDirection };
}
/** @param {OpticalSnapshot} snapshot @param {OpticalSettings} settings @param {OpticalSegment[]} extras */
export function visibleSegments(snapshot, settings = {}, extras = []) {
    const optical = snapshot.profile === 'sr' && settings.optical !== false;
    const source = optical ? snapshot.segments : snapshot.entities.filter(e => e.alive !== false).map(e => ({ ...e, entityId: e.id, originTime: snapshot.time, start: -Infinity, end: null }));
    return [...source.filter(s => (s.start ?? -Infinity) <= snapshot.time && (s.end ?? Infinity) > (optical ? snapshot.historyStart : -Infinity)), ...extras];
}
/** Conservative spatial bounds over the complete retained historical interval. Rotation and Lorentz contraction never enlarge the circumsphere.
 * @param {OpticalSegment} segment @param {number} time @param {number} historyStart @param {number} index
 */
export function segmentBounds(segment, time, historyStart, index) {
    const start = Math.max(segment.start ?? historyStart, historyStart), end = Math.min(segment.end ?? time, time);
    const radius = Math.hypot(...segment.size) * 0.5 + 1e-4, originTime = segment.originTime || 0;
    const a = segment.position.map((x, j) => x + segment.velocity[j] * (start - originTime));
    const b = segment.position.map((x, j) => x + segment.velocity[j] * (end - originTime));
    return { min: a.map((x, j) => Math.min(x, b[j]) - radius), max: a.map((x, j) => Math.max(x, b[j]) + radius), index };
}

/** @param {OpticalSnapshot} snapshot @param {OpticalSettings} settings @param {number} ndcX @param {number} ndcY @param {OpticalSegment[]} extras @param {TraceAcceleration|null} prepared @returns {OpticalHit|null} */
export function traceObserverRay(snapshot, settings = {}, ndcX = 0, ndcY = 0, extras = [], prepared = null) {
    const ray = observerRay(snapshot, settings, ndcX, ndcY), segments = prepared?.segments || visibleSegments(snapshot, settings, extras);
    const optical = snapshot.profile === 'sr' && settings.optical !== false, sr = snapshot.profile === 'sr';
    // Simultaneous-frame intersections can lie at coordinate times later than the observation event.
    // Their analytic inertial continuation is a geometry comparison, not historical optical evidence.
    const bvh = optical ? prepared?.bvh || buildBVH(segments.map((s, i) => segmentBounds(s, snapshot.time, snapshot.historyStart, i))) : null;
    let closest = Infinity;
    /** @type {OpticalHit|null} */
    let result = null;
    /** Reflected rays inspect the original rest geometry, including handed asymmetric meshes.
     * The image is presentation only: identity, revision and rest surface remain the source's.
     * @param {number} index @param {ReturnType<typeof observerRay>} tracingRay @param {boolean} mirrored
     */
    const visit = (index, tracingRay, mirrored) => {
        const s = segments[index], velocity = sr ? s.velocity : [0, 0, 0], originTime = s.originTime || 0;
        const dt = snapshot.time - originTime, displacement = tracingRay.origin.map((x, j) => x - s.position[j]);
        const transformedOrigin = boost(dt, displacement, velocity.map(x => -x));
        const transformedRay = boost(tracingRay.timeSlope, tracingRay.direction, velocity.map(x => -x));
        const rotation = rotationMatrix(s.rotation), localOrigin = inverseRotate(rotation, transformedOrigin.space).map((x, j) => x / s.size[j]), localDirection = inverseRotate(rotation, transformedRay.space).map((x, j) => x / s.size[j]);
        const shape = s.shape === 'pulse' ? 'light-pulse' : s.shape;
        const minimum = optical ? Math.max(0, snapshot.time - (s.end ?? Infinity)) : 0;
        const maximum = optical ? Math.min(closest, snapshot.time - Math.max(s.start ?? -Infinity, snapshot.historyStart) + 1e-6) : closest;
        if (minimum >= maximum) return;
        const hit = intersectShape(shape, localOrigin.map((x, j) => x + minimum * localDirection[j]), localDirection, maximum - minimum);
        if (!hit) return;
        hit.distance += minimum;
        const emissionTime = snapshot.time + hit.distance * ray.timeSlope;
        if (optical && (emissionTime < Math.max(s.start ?? -Infinity, snapshot.historyStart) || emissionTime >= (s.end ?? Infinity))) return;
        closest = hit.distance;
        const id = s.entityId || s.id || `environment-${index}`, live = snapshot.entities.find(e => e.id === id);
        const observer = { ...snapshot.observer, ...settings.cameraOverride };
        // Simultaneous geometry is a coordinate comparison, not received radiation.
        const sourceEnergy = optical ? gamma(velocity) * (1 + dot(velocity, tracingRay.direction)) : 1;
        const receivedEnergy = optical ? gamma(observer.velocity) * (1 + dot(observer.velocity, ray.direction)) : 1;
        result = {
            entityId: id, id, revision: s.revision, distance: hit.distance, emissionTime,
            properTime: (s.clockOffset || 0) + transformedOrigin.time + hit.distance * transformedRay.time,
            position: ray.origin.map((x, j) => x + hit.distance * ray.direction[j]),
            sourcePosition: tracingRay.origin.map((x, j) => x + hit.distance * tracingRay.direction[j]),
            restPosition: localOrigin.map((x, j) => (x + hit.distance * localDirection[j]) * s.size[j]),
            normal: normalize(rotate(rotation, hit.normal.map((x, j) => x / s.size[j]))).map((x, j) => mirrored && j === 1 ? -x : x),
            historical: !live || live.alive === false || live.revision !== s.revision,
            segmentIndex: index, mirrored, doppler: receivedEnergy / sourceEnergy,
        };
    };
    for (let pass = 0; pass < (settings.mirrorWorld === true ? 2 : 1); pass++) {
        const mirrored = pass === 1;
        const tracingRay = mirrored ? { ...ray, origin: ray.origin.map((x, j) => j === 1 ? -x : x), direction: ray.direction.map((x, j) => j === 1 ? -x : x) } : ray;
        const boundsOrigin = tracingRay.origin.map((x, j) => x - (prepared?.offset[j] || 0));
        if (bvh) {
            let cursor = 0;
            while (cursor < bvh.nodes.length) {
                const node = bvh.nodes[cursor];
                if (!hitBounds(boundsOrigin, tracingRay.direction, node.min, node.max, closest)) { cursor = node.escape; continue; }
                for (let j = 0; j < node.count; j++) visit(bvh.order[node.first + j], tracingRay, mirrored);
                cursor++;
            }
        } else for (let j = 0; j < segments.length; j++) visit(j, tracingRay, mirrored);
    }
    const floorDistance = ray.direction[1] < -1e-6 ? -ray.origin[1] / ray.direction[1] : Infinity;
    return settings.mirrorWorld !== true && floorDistance > 0 && floorDistance < closest ? null : result;
}

/** Capture one immutable camera projection for many reference-overlay points.
 * Basis, gamma, aspect and FoV are evaluated once, outside the line loop.
 * This is the same Lorentz projection as projectPoint, without per-point vector
 * arrays or repeated camera setup. Coordinates are normalized, with y downward.
 * @param {OpticalSnapshot} snapshot @param {OpticalSettings} settings
 */
export function createPointProjector(snapshot, settings) {
    const observer = { ...snapshot.observer, ...settings.cameraOverride };
    const [ox, oy, oz] = observer.position, [vx, vy, vz] = observer.velocity;
    const sr = snapshot.profile === 'sr', optical = sr && settings.optical !== false;
    const speedSquared = vx * vx + vy * vy + vz * vz, g = sr ? gamma(observer.velocity) : 1;
    const boostCoefficient = speedSquared > 1e-20 ? (g - 1) / speedSquared : 0;
    const { forward, right, up } = cameraBasis(observer), tangent = Math.tan((settings.fov || 60) * Math.PI / 360);
    const scaleX = 1 / (2 * tangent * (settings.aspect || 1)), scaleY = 1 / (2 * tangent);
    /** @param {number[]} point */
    return point => {
        const dx = point[0] - ox, dy = point[1] - oy, dz = point[2] - oz;
        const distance = Math.hypot(dx, dy, dz), projection = vx * dx + vy * dy + vz * dz;
        const coefficient = sr && speedSquared > 1e-20 ? boostCoefficient * projection - g * (optical ? -distance : projection) : 0;
        const sx = dx + coefficient * vx, sy = dy + coefficient * vy, sz = dz + coefficient * vz;
        const depth = sx * forward[0] + sy * forward[1] + sz * forward[2];
        // Homogeneous numerators remain finite exactly on the camera plane.
        const homogeneousX = 0.5 * depth + (sx * right[0] + sy * right[1] + sz * right[2]) * scaleX;
        const homogeneousY = 0.5 * depth - (sx * up[0] + sy * up[1] + sz * up[2]) * scaleY;
        const x = homogeneousX / depth, y = homogeneousY / depth;
        return { x, y, visible: depth > 1e-5 && x >= 0 && x <= 1 && y >= 0 && y <= 1, distance, depth, homogeneousX, homogeneousY };
    };
}

/** Project one already selected world event. Batch callers use createPointProjector.
 * @param {OpticalSnapshot} snapshot @param {OpticalSettings} settings @param {number[]} point
 */
export function projectPoint(snapshot, settings, point) { return createPointProjector(snapshot, settings)(point); }

/** Centre event on the selected observation surface, used only for labels.
 * Optical uses the past light cone; SR comparison uses the observer's simultaneity
 * hyperplane; Playground uses coordinate-now. Surface targeting uses full geometry.
 * @param {OpticalSnapshot} snapshot @param {OpticalSettings} settings @param {OpticalSegment} s
 */
export function apparentCenter(snapshot, settings, s) {
    const observer = { ...snapshot.observer, ...settings.cameraOverride }, optical = snapshot.profile === 'sr' && settings.optical !== false;
    const position = s.position.map((x, j) => x + s.velocity[j] * (snapshot.time - (s.originTime || 0)));
    if (snapshot.profile !== 'sr') return { position, emissionTime: snapshot.time };
    if (!optical) {
        const separation = position.map((x, j) => x - observer.position[j]);
        const dt = dot(observer.velocity, separation) / (1 - dot(observer.velocity, s.velocity));
        return { position: position.map((x, j) => x + s.velocity[j] * dt), emissionTime: snapshot.time + dt };
    }
    const r = position.map((x, j) => x - observer.position[j]), v = s.velocity, rv = dot(r, v), a = 1 - dot(v, v);
    const delay = (-rv + Math.sqrt(rv * rv + a * dot(r, r))) / a, emissionTime = snapshot.time - delay;
    if (emissionTime < Math.max(snapshot.historyStart, s.start ?? -Infinity) || emissionTime >= (s.end ?? Infinity)) return null;
    return { position: position.map((x, j) => x - v[j] * delay), emissionTime };
}
