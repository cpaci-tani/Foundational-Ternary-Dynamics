// @ts-check
/** Reflection in the world plane y=0. Positions, velocities and forces are polar. */
/** @param {number[]} vector @returns {number[]} */
export function reflectPolar(vector) { return [vector[0], -vector[1], vector[2]]; }

/** Angular velocity and torque are axial: det(S)S, where S=diag(1,-1,1). */
/** @param {number[]} vector @returns {number[]} */
export function reflectAxial(vector) { return [-vector[0], vector[1], -vector[2]]; }

/** Conjugating an XYZ rotation by S gives Rx(-x) Ry(y) Rz(-z). */
/** @param {number[]} rotation @returns {number[]} */
export function reflectEulerXYZ(rotation) { return reflectAxial(rotation); }

/** @param {{x:number,y:number,z:number,w:number}} rotation */
export function reflectQuaternion(rotation) {
    return { x: -rotation.x, y: rotation.y, z: -rotation.z, w: rotation.w };
}

/**
 * Convert between linked source and reflected-image author frames. Reflection
 * is its own inverse; sizes, materials, identities and scalar physics stay fixed.
 * @template {Record<string, unknown>} T
 * @param {T} patch @param {boolean} [mirrored]
 * @returns {T}
 */
export function reflectAuthorPatch(patch, mirrored = true) {
    if (!mirrored) return patch;
    /** @type {Record<string, unknown>} */ const result = { ...patch };
    for (const key of ['position', 'velocity', 'properAcceleration', 'impulse', 'force']) {
        if (Array.isArray(patch[key])) result[key] = reflectPolar(patch[key]);
    }
    for (const key of ['rotation', 'angularVelocity', 'torque']) {
        if (Array.isArray(patch[key])) result[key] = reflectAxial(patch[key]);
    }
    return /** @type {T} */ (result);
}
