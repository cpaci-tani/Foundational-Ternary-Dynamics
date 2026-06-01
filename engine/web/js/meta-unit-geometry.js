// ── meta-unit-geometry.js ── Pure geometry helpers for MetaUnit ──
// Extracted from meta-unit.js: sphere/wireframe/axis/mirror factories
// and edge-finding utilities. No physics state; no DOM.

import * as THREE from 'three';

const SCALE = 1.5;

// Sphere tessellation — keep identical to the legacy per-site SphereGeometry
// so the InstancedMesh silhouette is pixel-for-pixel the same.
const SPHERE_WIDTH_SEGMENTS = 24;
const SPHERE_HEIGHT_SEGMENTS = 16;

// Material constants shared by both the legacy single-sphere factory and the
// instanced-shell factory, so site appearance is unchanged.
const SITE_EMISSIVE_INTENSITY = 0.3;
const SITE_METALNESS = 0.2;
const SITE_ROUGHNESS = 0.5;

export function makeSphere(radius, color) {
    const geo = new THREE.SphereGeometry(radius * SCALE, SPHERE_WIDTH_SEGMENTS, SPHERE_HEIGHT_SEGMENTS);
    const mat = new THREE.MeshStandardMaterial({
        color,
        emissive: color,
        emissiveIntensity: SITE_EMISSIVE_INTENSITY,
        metalness: SITE_METALNESS,
        roughness: SITE_ROUGHNESS,
    });
    return new THREE.Mesh(geo, mat);
}

// ── Instanced shell factory (F-11) ──────────────────────────────────
// Builds ONE InstancedMesh that draws `count` site spheres of identical
// radius in a single draw call, replacing `count` individual Meshes.
//
// Per-instance colour is carried by `instanceColor`. The legacy per-site
// material set BOTH `color` and `emissive` to the same hex with
// emissiveIntensity 0.3; that invariant (color === emissive) holds in
// every recolour path (default / parity / inversion). We reproduce it
// exactly by:
//   • setting the base material color + emissive to WHITE,
//   • leaving emissiveIntensity at 0.3,
//   • injecting one shader line so the emissive term is multiplied by the
//     per-instance vertex colour (which the renderer sets from
//     instanceColor when USE_INSTANCING_COLOR is defined).
// Result per instance: diffuse = white·instanceColor = instanceColor;
// emissive = white·0.3·instanceColor = instanceColor·0.3 — identical to
// `MeshStandardMaterial({ color: c, emissive: c, emissiveIntensity: 0.3 })`.
export function makeInstancedShell(radius, count) {
    const geo = new THREE.SphereGeometry(radius * SCALE, SPHERE_WIDTH_SEGMENTS, SPHERE_HEIGHT_SEGMENTS);
    const mat = new THREE.MeshStandardMaterial({
        color: 0xFFFFFF,
        emissive: 0xFFFFFF,
        emissiveIntensity: SITE_EMISSIVE_INTENSITY,
        metalness: SITE_METALNESS,
        roughness: SITE_ROUGHNESS,
    });
    // Per-instance emissive: fold the instance colour into the emissive term.
    // `vColor` is white (1,1,1) for un-instanced verts and equals the
    // instance colour once USE_INSTANCING_COLOR is active, so this is a no-op
    // for any non-instanced use and exact per-instance emissive when instanced.
    mat.onBeforeCompile = (shader) => {
        shader.fragmentShader = shader.fragmentShader.replace(
            '#include <emissivemap_fragment>',
            '#include <emissivemap_fragment>\n\ttotalEmissiveRadiance *= vColor;'
        );
    };
    const inst = new THREE.InstancedMesh(geo, mat, count);
    // Allocate the per-instance colour buffer up front so setColorAt works.
    inst.instanceColor = new THREE.InstancedBufferAttribute(new Float32Array(count * 3), 3);
    return inst;
}

export function makeWireframe(vertices, edges, color) {
    const positions = [];
    for (const [a, b] of edges) {
        positions.push(
            vertices[a][0] * SCALE, vertices[a][1] * SCALE, vertices[a][2] * SCALE,
            vertices[b][0] * SCALE, vertices[b][1] * SCALE, vertices[b][2] * SCALE,
        );
    }
    const geo = new THREE.BufferGeometry();
    geo.setAttribute('position', new THREE.Float32BufferAttribute(positions, 3));
    const mat = new THREE.LineBasicMaterial({ color, linewidth: 1, transparent: true, opacity: 0.7 });
    return new THREE.LineSegments(geo, mat);
}

export function makeAxisLine(from, to, color) {
    const positions = [
        from[0] * SCALE, from[1] * SCALE, from[2] * SCALE,
        to[0] * SCALE, to[1] * SCALE, to[2] * SCALE,
    ];
    const geo = new THREE.BufferGeometry();
    geo.setAttribute('position', new THREE.Float32BufferAttribute(positions, 3));
    const mat = new THREE.LineBasicMaterial({ color, linewidth: 2 });
    return new THREE.LineSegments(geo, mat);
}

export function makeMirrorPlane(normal, color) {
    const geo = new THREE.PlaneGeometry(3.5 * SCALE, 3.5 * SCALE);
    const mat = new THREE.MeshBasicMaterial({
        color,
        transparent: true,
        opacity: 0.1,
        side: THREE.DoubleSide,
        depthWrite: false,
    });
    const mesh = new THREE.Mesh(geo, mat);
    const up = new THREE.Vector3(0, 0, 1);
    const n = new THREE.Vector3(normal[0], normal[1], normal[2]).normalize();
    const quat = new THREE.Quaternion().setFromUnitVectors(up, n);
    mesh.quaternion.copy(quat);
    return mesh;
}

export function findAllEdges(vertices) {
    const edges = [];
    for (let i = 0; i < vertices.length; i++) {
        for (let j = i + 1; j < vertices.length; j++) {
            const dx = vertices[i][0] - vertices[j][0];
            const dy = vertices[i][1] - vertices[j][1];
            const dz = vertices[i][2] - vertices[j][2];
            const d2 = dx * dx + dy * dy + dz * dz;
            edges.push({ i, j, d2 });
        }
    }
    return edges;
}

export function edgesAtDistance(vertices, targetD2) {
    return findAllEdges(vertices)
        .filter(e => Math.abs(e.d2 - targetD2) < 0.01)
        .map(e => [e.i, e.j]);
}
