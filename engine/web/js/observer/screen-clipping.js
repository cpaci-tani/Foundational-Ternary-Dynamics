// @ts-check
/** Homogeneous clipping for projected reference-overlay chords. No DOM or GPU. */
/** @typedef {{depth:number,homogeneousX:number,homogeneousY:number}} ProjectedPoint */
/** @typedef {{x:number,y:number}} ScreenPoint */

/** Clip against a positive near depth and the four normalized viewport planes
 * before dividing by depth. A chord crossing the view survives even when both
 * source endpoints lie offscreen, including one on/behind the camera plane.
 * This clips the presentation chord, not an additional physical photon path.
 * @param {ProjectedPoint} a @param {ProjectedPoint} b @param {number} [near]
 * @returns {{a:ScreenPoint,b:ScreenPoint}|null}
 */
export function clipProjectedSegment(a, b, near = 1e-5) {
    if (![a.depth, a.homogeneousX, a.homogeneousY, b.depth, b.homogeneousX, b.homogeneousY, near].every(Number.isFinite) || near <= 0) return null;
    let low = 0, high = 1;
    const planeA = [a.depth - near, a.homogeneousX, a.depth - a.homogeneousX, a.homogeneousY, a.depth - a.homogeneousY];
    const planeB = [b.depth - near, b.homogeneousX, b.depth - b.homogeneousX, b.homogeneousY, b.depth - b.homogeneousY];
    for (let i = 0; i < planeA.length; i++) {
        const p = planeA[i], q = planeB[i];
        if (p < 0 && q < 0) return null;
        if (p < 0) low = Math.max(low, p / (p - q));
        else if (q < 0) high = Math.min(high, p / (p - q));
        if (low >= high) return null;
    }
    /** @param {number} fraction */
    const screen = fraction => {
        const depth = a.depth * (1 - fraction) + b.depth * fraction;
        const x = (a.homogeneousX * (1 - fraction) + b.homogeneousX * fraction) / depth;
        const y = (a.homogeneousY * (1 - fraction) + b.homogeneousY * fraction) / depth;
        return { x: Math.max(0, Math.min(1, x)), y: Math.max(0, Math.min(1, y)) };
    };
    const first = screen(low), last = screen(high);
    if (![first.x, first.y, last.x, last.y].every(Number.isFinite)) return null;
    return { a: first, b: last };
}
