// engine/web/js/scales/scale0/runtime/knot-line-attribution.js
// Attribute field-line streamline segments to the nearest knot centroid.
// `streamlines` is the POOLED result from computeStreamlines: do NOT retain it.
// `centroids` is a flat Float32Array [x,y,z, ...] of length knotCount*3.
// Returns Map<knotIndex, {segments, length, legSet:Set<knotIndex>}>.
export function attributeSegmentsToKnots(streamlines, centroids, knotCount) {
    const out = new Map();
    for (let k = 0; k < knotCount; k++) out.set(k, { segments: 0, length: 0, legSet: new Set() });
    if (!streamlines || !streamlines.count || knotCount === 0) return out;
    const { count, buffer, offsets, lengths } = streamlines;
    const nearest = (x, y, z) => {
        let best = -1, bestD = Infinity;
        for (let k = 0; k < knotCount; k++) {
            const dx = x - centroids[k*3], dy = y - centroids[k*3+1], dz = z - centroids[k*3+2];
            const d = dx*dx + dy*dy + dz*dz;
            if (d < bestD) { bestD = d; best = k; }
        }
        return best;
    };
    for (let i = 0; i < count; i++) {
        const start = offsets[i], len = lengths[i];
        // line endpoints define its legs (which knots it connects).
        const aK = nearest(buffer[start], buffer[start+1], buffer[start+2]);
        const bK = nearest(buffer[start+len-3], buffer[start+len-2], buffer[start+len-1]);
        // walk vertex pairs; attribute each segment to its midpoint's nearest knot.
        for (let p = start; p + 5 < start + len; p += 3) {
            const mx = (buffer[p]+buffer[p+3])*0.5, my=(buffer[p+1]+buffer[p+4])*0.5, mz=(buffer[p+2]+buffer[p+5])*0.5;
            const k = nearest(mx, my, mz);
            const dx=buffer[p+3]-buffer[p], dy=buffer[p+4]-buffer[p+1], dz=buffer[p+5]-buffer[p+2];
            const rec = out.get(k);
            rec.segments += 1;
            rec.length += Math.sqrt(dx*dx+dy*dy+dz*dz);
        }
        if (aK >= 0 && bK >= 0 && aK !== bK) { out.get(aK).legSet.add(bK); out.get(bK).legSet.add(aK); }
    }
    return out;
}
