/** Rest-frame geometry and threaded bounding-volume hierarchies. No DOM or GPU dependencies. */
export const SHAPE_NAMES = Object.freeze(['sphere', 'box', 'plane', 'disk', 'capsule', 'cylinder', 'cone', 'torus', 'tetrahedron', 'octahedron', 'icosahedron', 'dodecahedron', 'ellipsoid', 'pyramid', 'prism', 'wedge', 'clock', 'ruler', 'beacon', 'light-pulse']);
/** @param {number[]} a @param {number[]} b */
export const dot = (a, b) => a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
/** @param {number[]} a @param {number[]} b */
export const cross = (a, b) => [a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]];
/** @param {number[]} a */
export const normalize = a => { const n = Math.hypot(...a) || 1; return a.map(x => x / n); };
/** @param {number[]} a @param {number[]} b */
export const subtract = (a, b) => a.map((x, i) => x - b[i]);

/** Euler XYZ matrix, row-major; rotation is a fixed rest-frame orientation.
 * @param {number[]} e
 */
export function rotationMatrix(e = [0, 0, 0]) {
    const [a, b, c] = e.map(Math.cos), [d, f, g] = e.map(Math.sin);
    return [b * c, -b * g, f, a * g + d * f * c, a * c - d * f * g, -d * b, d * g - a * f * c, d * c + a * f * g, a * b];
}
/** @param {number[]} m @param {number[]} v */
export const rotate = (m, v) => [dot(m.slice(0, 3), v), dot(m.slice(3, 6), v), dot(m.slice(6, 9), v)];
/** @param {number[]} m @param {number[]} v */
export const inverseRotate = (m, v) => [m[0] * v[0] + m[3] * v[1] + m[6] * v[2], m[1] * v[0] + m[4] * v[1] + m[7] * v[2], m[2] * v[0] + m[5] * v[1] + m[8] * v[2]];

/** @typedef {{min:number[],max:number[],index:number}} Bounds */
/** @typedef {{min:number[],max:number[],first:number,count:number,escape:number}} BVHNode */
/** Deterministic median BVH, preorder with escape links: no fixed traversal stack.
 * @param {Bounds[]} bounds @param {number} leafSize
 * @returns {{nodes:BVHNode[],order:number[]}}
 */
export function buildBVH(bounds, leafSize = 4) {
    /** @type {BVHNode[]} */ const nodes = [];
    /** @type {number[]} */ const order = [];
    /** @param {Bounds[]} items */
    function visit(items) {
        const min = [Infinity, Infinity, Infinity], max = [-Infinity, -Infinity, -Infinity];
        for (const b of items) for (let j = 0; j < 3; j++) { min[j] = Math.min(min[j], b.min[j]); max[j] = Math.max(max[j], b.max[j]); }
        const node = { min, max, first: order.length, count: 0, escape: 0 };
        nodes.push(node);
        if (items.length <= leafSize) { node.count = items.length; order.push(...items.map(x => x.index)); }
        else {
            const widths = max.map((x, j) => x - min[j]);
            const axis = widths.indexOf(Math.max(...widths));
            items.sort((a, b) => (a.min[axis] + a.max[axis]) - (b.min[axis] + b.max[axis]) || a.index - b.index);
            const middle = items.length >> 1;
            visit(items.slice(0, middle)); visit(items.slice(middle));
        }
        node.escape = nodes.length;
    }
    if (bounds.length) visit([...bounds]);
    return { nodes, order };
}

/** @typedef {{vertices:number[][],triangles:number[][],bvh:{nodes:BVHNode[],order:number[]}}} RestMesh */
/** @type {Map<string, RestMesh>} */ const meshCache = new Map();
/** Triangulate a convex point cloud by its supporting planes, including polygonal faces.
 * @param {number[][]} vertices @returns {number[][]}
 */
function convexTriangles(vertices) {
    /** @type {Map<string, {indices:number[],normal:number[]}>} */ const faces = new Map();
    for (let a = 0; a < vertices.length; a++) for (let b = a + 1; b < vertices.length; b++) for (let c = b + 1; c < vertices.length; c++) {
        let normal = cross(subtract(vertices[b], vertices[a]), subtract(vertices[c], vertices[a]));
        if (Math.hypot(...normal) < 1e-8) continue;
        normal = normalize(normal);
        const distances = vertices.map(v => dot(normal, subtract(v, vertices[a])));
        if (distances.some(x => x > 1e-7) && distances.some(x => x < -1e-7)) continue;
        if (distances.some(x => x > 1e-7)) normal = normal.map(x => -x);
        const indices = distances.flatMap((x, i) => Math.abs(x) < 1e-7 ? [i] : []);
        faces.set(indices.join(','), { indices, normal });
    }
    /** @type {number[][]} */ const result = [];
    for (const { indices, normal } of faces.values()) {
        const center = [0, 1, 2].map(j => indices.reduce((s, i) => s + vertices[i][j], 0) / indices.length);
        const u = normalize(subtract(vertices[indices[0]], center)), v = cross(normal, u);
        indices.sort((a, b) => Math.atan2(dot(subtract(vertices[a], center), v), dot(subtract(vertices[a], center), u)) - Math.atan2(dot(subtract(vertices[b], center), v), dot(subtract(vertices[b], center), u)));
        for (let k = 1; k < indices.length - 1; k++) result.push([indices[0], indices[k], indices[k + 1]]);
    }
    return result;
}

/** Shared canonical shapes have full extents of one. Curved meshes explicitly use finite tessellation.
 * @param {string} shape @returns {RestMesh}
 */
export function restMesh(shape) {
    const cached = meshCache.get(shape); if (cached) return cached;
    /** @type {number[][]} */ let vertices = [], triangles = [];
    const phi = (1 + Math.sqrt(5)) / 2;
    if (shape === 'torus') {
        const major = 32, minor = 16;
        for (let a = 0; a < major; a++) for (let b = 0; b < minor; b++) {
            const u = 2 * Math.PI * a / major, v = 2 * Math.PI * b / minor;
            vertices.push([(0.35 + 0.15 * Math.cos(v)) * Math.cos(u), 0.5 * Math.sin(v), (0.35 + 0.15 * Math.cos(v)) * Math.sin(u)]);
            const i = a * minor + b, j = ((a + 1) % major) * minor + b, k = a * minor + (b + 1) % minor, l = ((a + 1) % major) * minor + (b + 1) % minor;
            triangles.push([i, j, k], [k, j, l]);
        }
    } else if (['capsule', 'cylinder', 'cone'].includes(shape)) {
        const rings = shape === 'capsule' ? 18 : 1, slices = 32;
        for (let r = 0; r <= rings; r++) {
            const theta = Math.PI * r / rings;
            const radius = shape === 'capsule' ? 0.5 * Math.sin(theta) : shape === 'cone' ? r * 0.5 : 0.5;
            const y = shape === 'capsule' ? 0.25 * Math.cos(theta) + (r <= rings / 2 ? 0.25 : -0.25) : 0.5 - r;
            for (let s = 0; s < slices; s++) vertices.push([radius * Math.cos(2 * Math.PI * s / slices), y, radius * Math.sin(2 * Math.PI * s / slices)]);
        }
        for (let r = 0; r < rings; r++) for (let s = 0; s < slices; s++) {
            const a = r * slices + s, b = r * slices + (s + 1) % slices, c = a + slices, d = b + slices;
            triangles.push([a, c, b], [b, c, d]);
        }
        if (shape !== 'capsule') for (const [r, sign] of [[0, 1], [1, -1]]) {
            const center = vertices.length; vertices.push([0, 0.5 - r, 0]);
            for (let s = 0; s < slices; s++) triangles.push(sign > 0 ? [center, r * slices + (s + 1) % slices, r * slices + s] : [center, r * slices + s, r * slices + (s + 1) % slices]);
        }
    } else {
        if (shape === 'tetrahedron') vertices = [[1, 1, 1], [1, -1, -1], [-1, 1, -1], [-1, -1, 1]];
        else if (shape === 'octahedron') vertices = [[1, 0, 0], [-1, 0, 0], [0, 1, 0], [0, -1, 0], [0, 0, 1], [0, 0, -1]];
        else if (shape === 'icosahedron') for (const a of [-1, 1]) for (const b of [-phi, phi]) vertices.push([0, a, b], [a, b, 0], [b, 0, a]);
        else if (shape === 'dodecahedron') {
            for (const x of [-1, 1]) for (const y of [-1, 1]) for (const z of [-1, 1]) vertices.push([x, y, z]);
            for (const a of [-1 / phi, 1 / phi]) for (const b of [-phi, phi]) vertices.push([0, a, b], [a, b, 0], [b, 0, a]);
        } else if (shape === 'pyramid') vertices = [[-1, -1, -1], [-1, -1, 1], [1, -1, -1], [1, -1, 1], [0, 1, 0]];
        else if (shape === 'prism') vertices = [[-1, -1, -1], [1, -1, -1], [0, 1, -1], [-1, -1, 1], [1, -1, 1], [0, 1, 1]];
        else if (shape === 'wedge') vertices = [[-1, -1, -1], [1, -1, -1], [-1, 1, -1], [-1, -1, 1], [1, -1, 1], [-1, 1, 1]];
        else vertices = [[-1, -1, -1], [-1, -1, 1], [-1, 1, -1], [-1, 1, 1], [1, -1, -1], [1, -1, 1], [1, 1, -1], [1, 1, 1]];
        const extents = [0, 1, 2].map(j => Math.max(...vertices.map(v => Math.abs(v[j]))) * 2);
        vertices = vertices.map(v => v.map((x, j) => x / extents[j]));
        triangles = convexTriangles(vertices);
    }
    const bounds = triangles.map((t, index) => ({ min: [0, 1, 2].map(j => Math.min(...t.map(i => vertices[i][j]))), max: [0, 1, 2].map(j => Math.max(...t.map(i => vertices[i][j]))), index }));
    const mesh = { vertices, triangles, bvh: buildBVH(bounds) }; meshCache.set(shape, mesh); return mesh;
}

/** Deterministic breadth-first midpoint subdivision for a measured geometry workload.
 * Each parent triangle is replaced by four non-overlapping child triangles on
 * precisely the same rest surface. No duplicate faces or changed dynamics are used
 * to inflate the workload. The final count is the largest reachable count <= target.
 * @param {RestMesh} source @param {number} targetTriangles @returns {RestMesh}
 */
export function subdivideRestMesh(source, targetTriangles) {
    if (!Number.isInteger(targetTriangles) || targetTriangles < source.triangles.length || targetTriangles > 100000) throw new RangeError('Subdivision target must be an integer between the original face count and 100000.');
    if (targetTriangles === source.triangles.length) return source;
    const vertices = source.vertices.map(vertex => [...vertex]);
    const queue = source.triangles.map(triangle => [...triangle]);
    /** @type {Map<string,number>} */ const midpoints = new Map();
    /** @param {number} a @param {number} b */
    function midpoint(a, b) {
        const key = a < b ? `${a}:${b}` : `${b}:${a}`;
        const cached = midpoints.get(key); if (cached !== undefined) return cached;
        const index = vertices.length;
        vertices.push(vertices[a].map((value, j) => (value + vertices[b][j]) * 0.5));
        midpoints.set(key, index); return index;
    }
    let cursor = 0;
    while (queue.length - cursor + 3 <= targetTriangles) {
        const [a, b, c] = queue[cursor++], ab = midpoint(a, b), bc = midpoint(b, c), ca = midpoint(c, a);
        queue.push([a, ab, ca], [ab, b, bc], [ca, bc, c], [ab, bc, ca]);
    }
    const triangles = queue.slice(cursor);
    const bounds = triangles.map((triangle, index) => ({
        min: [0, 1, 2].map(j => Math.min(...triangle.map(i => vertices[i][j]))),
        max: [0, 1, 2].map(j => Math.max(...triangle.map(i => vertices[i][j]))), index,
    }));
    return { vertices, triangles, bvh: buildBVH(bounds) };
}

/** Slab intersection works for zero direction components, including rays on slab boundaries.
 * @param {number[]} origin @param {number[]} direction @param {number[]} min @param {number[]} max @param {number} limit
 */
export function hitBounds(origin, direction, min, max, limit = Infinity) {
    let near = 0, far = limit;
    for (let j = 0; j < 3; j++) {
        if (Math.abs(direction[j]) < 1e-15) { if (origin[j] < min[j] || origin[j] > max[j]) return false; continue; }
        const a = (min[j] - origin[j]) / direction[j], b = (max[j] - origin[j]) / direction[j];
        near = Math.max(near, Math.min(a, b)); far = Math.min(far, Math.max(a, b));
        if (near > far) return false;
    }
    return far >= 0;
}

/** @param {string} shape @param {number[]} o @param {number[]} d @param {number} limit
 * @returns {{distance:number,normal:number[]}|null}
 */
export function intersectShape(shape, o, d, limit = Infinity) {
    if (['sphere', 'ellipsoid', 'beacon', 'light-pulse'].includes(shape)) {
        const a = dot(d, d), b = dot(o, d), c = dot(o, o) - 0.25, disc = b * b - a * c;
        if (disc < 0) return null;
        const root = Math.sqrt(disc), near = (-b - root) / a, far = (-b + root) / a;
        const t = near > 1e-5 ? near : far;
        return t > 1e-5 && t < limit ? { distance: t, normal: normalize(o.map((x, j) => x + t * d[j])) } : null;
    }
    if (['plane', 'disk'].includes(shape)) {
        if (Math.abs(d[1]) < 1e-14) return null;
        const t = -o[1] / d[1], x = o[0] + t * d[0], z = o[2] + t * d[2];
        return t > 1e-5 && t < limit && (shape === 'disk' ? x * x + z * z <= 0.25 : Math.abs(x) <= 0.5 && Math.abs(z) <= 0.5) ? { distance: t, normal: [0, d[1] > 0 ? -1 : 1, 0] } : null;
    }
    const mesh = restMesh(shape), { nodes, order } = mesh.bvh;
    let best = limit, cursor = 0; /** @type {number[]|null} */ let normal = null;
    while (cursor < nodes.length) {
        const n = nodes[cursor];
        if (!hitBounds(o, d, n.min, n.max, best)) { cursor = n.escape; continue; }
        for (let k = 0; k < n.count; k++) {
            const [a, b, c] = mesh.triangles[order[n.first + k]].map(i => mesh.vertices[i]);
            const e1 = subtract(b, a), e2 = subtract(c, a), p = cross(d, e2), det = dot(e1, p);
            if (Math.abs(det) < 1e-12) continue;
            const s = subtract(o, a), u = dot(s, p) / det; if (u < 0 || u > 1) continue;
            const q = cross(s, e1), v = dot(d, q) / det; if (v < 0 || u + v > 1) continue;
            const t = dot(e2, q) / det;
            if (t > 1e-5 && t < best) { best = t; normal = normalize(cross(e1, e2)); }
        }
        cursor++;
    }
    return normal ? { distance: best, normal } : null;
}
