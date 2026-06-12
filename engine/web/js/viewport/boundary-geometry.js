/**
 * viewport/boundary-geometry.js — Three.js boundary wireframe builders
 *
 * Extracted from viewport.js as refactoring-analyst ticket RF-4 of the
 * post-modularization cleanup (see engine/web/docs/INDEX.md).
 *
 * Pure geometry — no state beyond the returned Three.js Group. No access to
 * `this` on the viewport, no scene reference, no material ownership. Callers
 * add the returned group to their scene and own its lifecycle.
 *
 * Companion to bridge/boundary.js which handles the physics side (particle
 * reflection / containment). This module handles ONLY the visual wireframe.
 *
 * Public API:
 *   buildBoundary(shape, mode, ctx, mat) → Group   — builds the boundary mesh
 *   insideBoundary(shape, nx, ny, nz)    → bool    — pure predicate (no state)
 *
 * Where:
 *   shape: 'cube' | 'sphere' | 'dodecahedron' | 'icosahedron' | 'octahedron' |
 *          'cylinder' | 'torus' | 'none'
 *   mode:  'lattice' | 'origin'
 *   ctx:   { latticeSize: number }   — minimum state needed for cube subdiv
 *   mat:   THREE.LineBasicMaterial   — base material (cloned for ring styles)
 *
 * Returned group is UNSCALED and at ORIGIN. Caller is responsible for
 * positioning and scaling (viewport.js::_buildBoundary handles this).
 */

import * as THREE from 'three';

/**
 * Dispatch on shape; returns a THREE.Group containing the wireframe mesh.
 * @param {string} shape
 * @param {string} mode
 * @param {{latticeSize: number}} ctx
 * @param {THREE.LineBasicMaterial} mat
 * @returns {THREE.Group}
 */
export function buildBoundary(shape, mode, ctx, mat) {
    switch (shape) {
        case 'cube':         return buildCubeBoundary(mat, mode, ctx);
        case 'sphere':       return buildSphereBoundary(mat);
        case 'dodecahedron': return buildPlatonicBoundary('dodecahedron', mat);
        case 'icosahedron':  return buildPlatonicBoundary('icosahedron', mat);
        case 'octahedron':   return buildPlatonicBoundary('octahedron', mat);
        case 'cylinder':     return buildCylinderBoundary(mat);
        case 'torus':        return buildTorusBoundary(mat);
        default:             return buildCubeBoundary(mat, mode, ctx);
    }
}

function buildCubeBoundary(mat, mode, ctx) {
    const vertices = [];
    const s = (mode === 'lattice') ? ctx.latticeSize : 1;

    // 12 edges of bounding cube
    const h = s / 2;
    const corners = (mode === 'lattice')
        ? [[0, 0, 0], [s, 0, 0], [s, s, 0], [0, s, 0], [0, 0, s], [s, 0, s], [s, s, s], [0, s, s]]
        : [[-h, -h, -h], [h, -h, -h], [h, h, -h], [-h, h, -h], [-h, -h, h], [h, -h, h], [h, h, h], [-h, h, h]];
    const edges = [
        [0, 1], [1, 2], [2, 3], [3, 0],
        [4, 5], [5, 6], [6, 7], [7, 4],
        [0, 4], [1, 5], [2, 6], [3, 7]
    ];
    for (const [a, b] of edges) {
        vertices.push(...corners[a], ...corners[b]);
    }

    // Subdivision lines only in lattice mode (sparse — just midpoint cross).
    //
    // Crosshair alignment: voxel k is rendered at world centre k+0.5 (the
    // universal Scale-0 convention). For EVEN N, N/2 is a voxel CORNER, so
    // a subdivision line at integer world `i = N/2` passes between voxels
    // rather than through any voxel's centre. EVERY scenario that anchors
    // at `mc = Math.round((N-1)/2)` (the default for Moore, proton, atom,
    // photon, flux-pulse, and basically all centred scenarios) then places
    // its centroid at world (mc+0.5) — exactly 0.5 off from the crosshair.
    //
    // Shift subdivision by +0.5 so the crosshair lands on voxel-centre world
    // coords for ALL N (even OR odd). Outer cube stays at [0, N] — only the
    // interior cross marker moves. Matches the physics-layer voxel-centre
    // convention and aligns EVERY centred scenario with the wireframe cross.
    if (mode === 'lattice') {
        const step = Math.max(8, Math.floor(s / 2));
        for (let raw = step; raw < s; raw += step) {
            const i = raw + 0.5;
            vertices.push(i, 0, 0, i, s, 0);
            vertices.push(i, 0, s, i, s, s);
            vertices.push(0, i, 0, s, i, 0);
            vertices.push(0, i, s, s, i, s);
            vertices.push(0, 0, i, s, 0, i);
            vertices.push(0, s, i, s, s, i);
        }
    }

    const geo = new THREE.BufferGeometry();
    geo.setAttribute('position', new THREE.Float32BufferAttribute(vertices, 3));
    const group = new THREE.Group();
    group.add(new THREE.LineSegments(geo, mat));
    return group;
}

function buildSphereBoundary(mat) {
    const group = new THREE.Group();

    // Wireframe sphere
    const sphereGeo = new THREE.SphereGeometry(1, 24, 16);
    const edgesGeo = new THREE.EdgesGeometry(sphereGeo);
    group.add(new THREE.LineSegments(edgesGeo, mat));
    sphereGeo.dispose();

    // 3 great-circle rings for structure
    const ringMat = mat.clone();
    ringMat.opacity = 0.5;
    const segments = 64;
    for (let axis = 0; axis < 3; axis++) {
        const pts = [];
        for (let i = 0; i <= segments; i++) {
            const t = (i / segments) * Math.PI * 2;
            const c = Math.cos(t), sn = Math.sin(t);
            if (axis === 0) pts.push(new THREE.Vector3(0, c, sn));
            else if (axis === 1) pts.push(new THREE.Vector3(c, 0, sn));
            else pts.push(new THREE.Vector3(c, sn, 0));
        }
        const ringGeo = new THREE.BufferGeometry().setFromPoints(pts);
        group.add(new THREE.Line(ringGeo, ringMat));
    }

    return group;
}

function buildPlatonicBoundary(shape, mat) {
    const group = new THREE.Group();
    let solidGeo;
    const detail = 0;
    switch (shape) {
        case 'dodecahedron': solidGeo = new THREE.DodecahedronGeometry(1, detail); break;
        case 'icosahedron':  solidGeo = new THREE.IcosahedronGeometry(1, detail); break;
        case 'octahedron':   solidGeo = new THREE.OctahedronGeometry(1, detail); break;
    }
    const edgesGeo = new THREE.EdgesGeometry(solidGeo);
    group.add(new THREE.LineSegments(edgesGeo, mat));
    solidGeo.dispose();
    return group;
}

function buildCylinderBoundary(mat) {
    const group = new THREE.Group();

    // Cylinder wireframe
    const cylGeo = new THREE.CylinderGeometry(1, 1, 2, 24, 1, true);
    const edgesGeo = new THREE.EdgesGeometry(cylGeo);
    group.add(new THREE.LineSegments(edgesGeo, mat));
    cylGeo.dispose();

    // Top and bottom cap circles
    const capMat = mat.clone();
    capMat.opacity = 0.4;
    const segments = 48;
    for (const y of [-1, 1]) {
        const pts = [];
        for (let i = 0; i <= segments; i++) {
            const t = (i / segments) * Math.PI * 2;
            pts.push(new THREE.Vector3(Math.cos(t), y, Math.sin(t)));
        }
        const capGeo = new THREE.BufferGeometry().setFromPoints(pts);
        group.add(new THREE.Line(capGeo, capMat));
    }

    return group;
}

function buildTorusBoundary(mat) {
    const group = new THREE.Group();
    const torusGeo = new THREE.TorusGeometry(0.7, 0.3, 12, 36);
    const edgesGeo = new THREE.EdgesGeometry(torusGeo);
    const mesh = new THREE.LineSegments(edgesGeo, mat);
    // Three.js TorusGeometry lies in XY plane (hole along Z) by default.
    // Rotate so major circle lies in XZ plane (hole along Y) to match
    // insideBoundary clipping and the PE grid orientation.
    mesh.rotation.x = -Math.PI / 2;
    group.add(mesh);
    torusGeo.dispose();
    return group;
}

/**
 * Pure predicate: is (nx, ny, nz) inside the given boundary shape?
 * Coordinates are normalized −1..1 from center (matches flux-volume clip).
 * Used to clip flux rendering against non-cube boundaries.
 *
 * @param {string} shape
 * @param {number} nx
 * @param {number} ny
 * @param {number} nz
 * @returns {boolean}
 */
export function insideBoundary(shape, nx, ny, nz) {
    switch (shape) {
        case 'none':
        case 'cube':
            return true; // cube = full lattice, no clipping
        case 'sphere':
            return (nx * nx + ny * ny + nz * nz) <= 1.0;
        case 'octahedron':
            return (Math.abs(nx) + Math.abs(ny) + Math.abs(nz)) <= 1.0;
        case 'dodecahedron': {
            // Dodecahedron defined by 6 pairs of face normals
            // Inradius of unit dodecahedron ≈ 0.7946
            const phi = 1.618033988749895;
            const ir = 0.7946; // inradius / circumradius
            const normals = [
                [0, 1, phi], [0, -1, phi], [0, 1, -phi], [0, -1, -phi],
                [1, phi, 0], [-1, phi, 0], [1, -phi, 0], [-1, -phi, 0],
                [phi, 0, 1], [-phi, 0, 1], [phi, 0, -1], [-phi, 0, -1],
            ];
            for (const n of normals) {
                const len = Math.sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
                const d = (nx * n[0] + ny * n[1] + nz * n[2]) / len;
                if (d > ir) return false;
            }
            return true;
        }
        case 'icosahedron': {
            // Icosahedron defined by 10 pairs of face normals
            // Inradius of unit icosahedron ≈ 0.7558
            const phi = 1.618033988749895;
            const ir = 0.7558;
            const normals = [
                [1, 1, 1], [1, 1, -1], [1, -1, 1], [1, -1, -1],
                [-1, 1, 1], [-1, 1, -1], [-1, -1, 1], [-1, -1, -1],
                [0, phi, 1 / phi], [0, phi, -1 / phi], [0, -phi, 1 / phi], [0, -phi, -1 / phi],
                [1 / phi, 0, phi], [-1 / phi, 0, phi], [1 / phi, 0, -phi], [-1 / phi, 0, -phi],
                [phi, 1 / phi, 0], [phi, -1 / phi, 0], [-phi, 1 / phi, 0], [-phi, -1 / phi, 0],
            ];
            for (const n of normals) {
                const len = Math.sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
                const d = (nx * n[0] + ny * n[1] + nz * n[2]) / len;
                if (d > ir) return false;
            }
            return true;
        }
        case 'cylinder':
            return (nx * nx + nz * nz) <= 1.0 && Math.abs(ny) <= 1.0;
        case 'torus': {
            // Torus: major R=0.7, minor r=0.3 (matches buildTorusBoundary)
            const dist_xz = Math.sqrt(nx * nx + nz * nz);
            const dx = dist_xz - 0.7;
            return (dx * dx + ny * ny) <= (0.3 * 0.3);
        }
        default:
            return true;
    }
}
