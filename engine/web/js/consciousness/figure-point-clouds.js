/**
 * Shared point-cloud helpers for figure builders.
 *
 * Extracted from consciousness-figure.js (ticket CF-1).
 * Pure geometry sampling — no Three.js material state.
 */

export function gaussRandom() {
    // Box-Muller
    const u1 = Math.random();
    const u2 = Math.random();
    return Math.sqrt(-2 * Math.log(u1 || 1e-10)) * Math.cos(2 * Math.PI * u2);
}

/** Generate random points in a sphere shell */
export function spherePoints(count, rMin, rMax) {
    const pos = new Float32Array(count * 3);
    const sizes = new Float32Array(count);
    for (let i = 0; i < count; i++) {
        const u = Math.random();
        const r = rMin + (rMax - rMin) * Math.cbrt(u);
        const theta = Math.acos(2 * Math.random() - 1);
        const phi = Math.random() * Math.PI * 2;
        pos[i*3]   = r * Math.sin(theta) * Math.cos(phi);
        pos[i*3+1] = r * Math.sin(theta) * Math.sin(phi);
        pos[i*3+2] = r * Math.cos(theta);
        sizes[i] = 0.4 + Math.random() * 0.8;
    }
    return { pos, sizes };
}

/** Generate points along a vertical cylinder */
export function cylinderPoints(count, radius, yMin, yMax) {
    const pos = new Float32Array(count * 3);
    const sizes = new Float32Array(count);
    for (let i = 0; i < count; i++) {
        const angle = Math.random() * Math.PI * 2;
        const r = radius * Math.sqrt(Math.random());
        pos[i*3]   = r * Math.cos(angle);
        pos[i*3+1] = yMin + Math.random() * (yMax - yMin);
        pos[i*3+2] = r * Math.sin(angle);
        sizes[i] = 0.4 + Math.random() * 1.0;
    }
    return { pos, sizes };
}

/** Generate points on a torus */
export function torusPoints(count, R, r, noiseSigma = 0) {
    const pos = new Float32Array(count * 3);
    const sizes = new Float32Array(count);
    // Store toroidal/poloidal angles for animation
    const angles = new Float32Array(count * 2);
    for (let i = 0; i < count; i++) {
        const u = Math.random() * Math.PI * 2; // toroidal
        const v = Math.random() * Math.PI * 2; // poloidal
        const rr = r + (noiseSigma > 0 ? gaussRandom() * noiseSigma : 0);
        pos[i*3]   = (R + rr * Math.cos(v)) * Math.cos(u);
        pos[i*3+1] = rr * Math.sin(v);
        pos[i*3+2] = (R + rr * Math.cos(v)) * Math.sin(u);
        sizes[i] = 0.3 + Math.random() * 0.9;
        angles[i*2] = u;
        angles[i*2+1] = v;
    }
    return { pos, sizes, angles };
}

/** Generate points in a downward-expanding cone */
export function conePoints(count, yTop, yBot, rTop, rBot) {
    const pos = new Float32Array(count * 3);
    const sizes = new Float32Array(count);
    for (let i = 0; i < count; i++) {
        const t = Math.random(); // 0=top, 1=bottom
        const y = yTop + t * (yBot - yTop);
        const maxR = rTop + t * (rBot - rTop);
        const r = maxR * Math.sqrt(Math.random());
        const angle = Math.random() * Math.PI * 2;
        pos[i*3]   = r * Math.cos(angle);
        pos[i*3+1] = y;
        pos[i*3+2] = r * Math.sin(angle);
        // Smaller at top, larger at bottom (dissolution)
        sizes[i] = 0.3 + t * 0.5 + Math.random() * 0.2;
    }
    return { pos, sizes };
}

/** Generate points along radial rays */
export function rayPoints(count, numRays, rMin, rMax) {
    const pos = new Float32Array(count * 3);
    const sizes = new Float32Array(count);
    const rayAngles = new Float32Array(count); // which ray
    const rayDists  = new Float32Array(count); // distance along ray
    for (let i = 0; i < count; i++) {
        const rayIdx = Math.floor(Math.random() * numRays);
        const angle = (rayIdx / numRays) * Math.PI * 2;
        const dist = rMin + Math.random() * (rMax - rMin);
        pos[i*3]   = dist * Math.cos(angle);
        pos[i*3+1] = dist * Math.sin(angle);
        pos[i*3+2] = (Math.random() - 0.5) * 0.3; // slight depth
        sizes[i] = 0.4 + Math.random() * 0.6;
        rayAngles[i] = angle;
        rayDists[i] = dist;
    }
    return { pos, sizes, rayAngles, rayDists };
}
