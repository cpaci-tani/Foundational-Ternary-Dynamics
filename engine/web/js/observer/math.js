// @ts-check
/** Standard special relativity reference mathematics, c = 1. No FTD recovery claim. */
/** @typedef {number[]} Vec3 */
/** @typedef {number[]} FourVector */
export const MAX_BETA = 0.99;
export const FIXED_DT = 1 / 120;
/** @param {Vec3} a @param {Vec3} b */
export const dot = (a, b) => a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
/** @param {Vec3} a */
export const length = (a) => Math.sqrt(dot(a, a));
/** @param {Vec3} a @param {Vec3} b */
export const add = (a, b) => a.map((x, i) => x + b[i]);
/** @param {Vec3} a @param {Vec3} b */
export const sub = (a, b) => a.map((x, i) => x - b[i]);
/** @param {Vec3} a @param {number} s */
export const scale = (a, s) => a.map(x => x * s);
/** @param {Vec3} a */
export const normalize = (a) => length(a) > 1e-15 ? scale(a, 1 / length(a)) : [0, 0, 0];
/** @param {Vec3} a @param {Vec3} b */
export const cross = (a, b) => [a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]];
/** @param {number} x @param {number} low @param {number} high */
export const clamp = (x, low, high) => Math.max(low, Math.min(high, x));
/** @param {number | Vec3} beta */
export function gamma(beta) {
    const b2 = typeof beta === 'number' ? beta * beta : dot(beta, beta);
    if (!Number.isFinite(b2) || b2 >= 1) throw new RangeError('A timelike velocity must have |beta| < 1.');
    return 1 / Math.sqrt(1 - b2);
}
/** Coordinates in a frame moving at velocity v relative to the input frame. @param {FourVector} event @param {Vec3} v */
export function boostEvent(event, v) {
    const [t, ...r] = event;
    const b2 = dot(v, v);
    if (b2 < 1e-30) return [...event];
    const g = gamma(v), vr = dot(v, r);
    return [g * (t - vr), ...add(r, scale(v, (g - 1) * vr / b2 - g * t))];
}
export const boostFourVector = boostEvent;
/** @param {FourVector} event @param {Vec3} velocity */
export const inverseBoost = (event, velocity) => boostEvent(event, scale(velocity, -1));
/** Signature +---. @param {FourVector} event */
export const intervalSquared = (event) => event[0] ** 2 - event.slice(1).reduce((s, x) => s + x * x, 0);
/** Transform a velocity into a frame moving at frameVelocity. @param {Vec3} velocity @param {Vec3} frameVelocity */
export function transformVelocity(velocity, frameVelocity) {
    const four = boostEvent([1, ...velocity], frameVelocity);
    return scale(four.slice(1), 1 / four[0]);
}
/** @param {Vec3} velocity @param {Vec3} frameVelocity */
export const velocityAddition = (velocity, frameVelocity) => transformVelocity(velocity, scale(frameVelocity, -1));
/** Photon direction is direction of propagation, not look direction. @param {Vec3} propagation @param {Vec3} observerVelocity */
export function aberration(propagation, observerVelocity) {
    return normalize(boostEvent([1, ...normalize(propagation)], observerVelocity).slice(1));
}
/** Emission-to-observation frequency ratio for a shared world photon propagation. @param {Vec3} propagation @param {Vec3} emitterVelocity @param {Vec3} observerVelocity */
export function dopplerFactor(propagation, emitterVelocity, observerVelocity) {
    const n = normalize(propagation);
    return gamma(observerVelocity) * (1 - dot(observerVelocity, n)) / (gamma(emitterVelocity) * (1 - dot(emitterVelocity, n)));
}
/** Convert observer rest-frame look direction to a backward null ray in world space. @param {Vec3} look @param {Vec3} velocity */
export function observerRayToWorld(look, velocity) {
    const ray = inverseBoost([-1, ...normalize(look)], velocity);
    return { direction: scale(ray.slice(1), -1 / ray[0]), timeRate: -1 };
}
/** @param {number} yaw @param {number} pitch @param {number} [roll] */
export function cameraBasis(yaw, pitch, roll = 0) {
    const forward = [-Math.sin(yaw) * Math.cos(pitch), Math.sin(pitch), -Math.cos(yaw) * Math.cos(pitch)];
    const right0 = [Math.cos(yaw), 0, -Math.sin(yaw)];
    const up0 = cross(right0, forward);
    return { forward, right: add(scale(right0, Math.cos(roll)), scale(up0, Math.sin(roll))), up: add(scale(up0, Math.cos(roll)), scale(right0, -Math.sin(roll))) };
}
/** Constant proper acceleration from rest, analytic worldline. @param {number} properTime @param {number} acceleration */
export function constantProperAcceleration(properTime, acceleration) {
    if (Math.abs(acceleration) < 1e-15) return { time: properTime, position: 0, velocity: 0 };
    const rapidity = acceleration * properTime;
    return { time: Math.sinh(rapidity) / acceleration, position: (Math.cosh(rapidity) - 1) / acceleration, velocity: Math.tanh(rapidity) };
}
/** Integrate constant coordinate force per rest mass, u=gamma*v. @param {Vec3} velocity @param {Vec3} properForce @param {number} dt @param {number} [maxBeta] */
export function integrateFourVelocity(velocity, properForce, dt, maxBeta = MAX_BETA) {
    const u0 = scale(velocity, gamma(velocity));
    let u1 = add(u0, scale(properForce, dt));
    const maxU = maxBeta / Math.sqrt(1 - maxBeta * maxBeta);
    if (length(u1) > maxU) u1 = scale(normalize(u1), maxU);
    const v1 = scale(u1, 1 / Math.sqrt(1 + dot(u1, u1)));
    return { velocity: v1, displacement: scale(add(velocity, v1), dt / 2), properElapsed: dt * (1 / gamma(velocity) + 1 / gamma(v1)) / 2 };
}
/** Retarded center event for an inertial segment; stable positive quadratic root. @param {Vec3} observer @param {number} time @param {Vec3} position @param {Vec3} velocity @param {number} [originTime] */
export function retardedTime(observer, time, position, velocity, originTime = 0) {
    const separation = sub(add(position, scale(velocity, time - originTime)), observer);
    const a = 1 - dot(velocity, velocity), b = dot(separation, velocity), c = dot(separation, separation);
    if (a <= 0) throw new RangeError('Retarded center requires a timelike trajectory.');
    const root = Math.sqrt(b * b + a * c);
    const delay = b >= 0 ? c / (root + b || 1) : (root - b) / a;
    return time - delay;
}
/** @param {number} mass @param {Vec3} velocity */
export const fourMomentum = (mass, velocity) => scale([1, ...velocity], mass * gamma(velocity));
/** Elastic 1D two-point collision; transform to center-of-momentum, reflect, transform back. @param {number} massA @param {number} velocityA @param {number} massB @param {number} velocityB */
export function elasticCollision1D(massA, velocityA, massB, velocityB) {
    if (!(massA > 0 && massB > 0)) throw new RangeError('Collision masses must be positive.');
    const a = fourMomentum(massA, [velocityA, 0, 0]), b = fourMomentum(massB, [velocityB, 0, 0]);
    const com = (a[1] + b[1]) / (a[0] + b[0]);
    const ac = (velocityA - com) / (1 - velocityA * com), bc = (velocityB - com) / (1 - velocityB * com);
    return { velocityA: (-ac + com) / (1 - ac * com), velocityB: (-bc + com) / (1 - bc * com), momentumBefore: a.map((n, i) => n + b[i]) };
}
