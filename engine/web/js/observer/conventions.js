// @ts-check
/** Numerical provenance for future evolution; retained legacy intervals keep their own version. */
export const INTEGRATOR_VERSION = 'sr-coordinate-force-kdk-v2';
export const LEGACY_INTEGRATOR_VERSION = 'sr-legacy-euler-drift-v1';

/** Normalize the historical misnamed field at a data boundary, never reinterpret it.
 * @template {Record<string, any>} T @param {T} patch @returns {T}
 */
export function normalizeCoordinateForce(patch) {
    const result = { ...patch };
    const old = patch.properAcceleration, current = patch.coordinateForcePerMass;
    for (const value of [old, current]) if (value !== undefined && (!Array.isArray(value) || value.length !== 3 || value.some(x => typeof x !== 'number' || !Number.isFinite(x) || Math.abs(x) > 1e12))) throw new Error('Coordinate force / rest mass must contain three finite numbers within +/-1e12.');
    if (old !== undefined && current !== undefined && old.some((/** @type {number} */ x, /** @type {number} */ i) => x !== current[i])) throw new Error('Conflicting properAcceleration and coordinateForcePerMass fields.');
    if (old !== undefined || current !== undefined) Object.assign(result, { coordinateForcePerMass: [...(current ?? old)] });
    delete result.properAcceleration;
    return result;
}

/** Read-only migration. Positions, velocities, clocks and interval bounds are copied verbatim.
 * @param {import('./types.js').WorldSnapshot} snapshot
 */
export function migrateObserverSnapshot(snapshot) {
    const result = structuredClone(snapshot);
    result.integratorVersion ??= LEGACY_INTEGRATOR_VERSION;
    result.entities = result.entities.map(entity => normalizeCoordinateForce(entity));
    for (const segment of result.segments) segment.integratorVersion ??= LEGACY_INTEGRATOR_VERSION;
    return result;
}
