// ── meta-unit-geometry.js ── Pure geometry helpers for MetaUnit ──
// Extracted from meta-unit.js: sphere/wireframe/axis/mirror factories
// and edge-finding utilities. No physics state; no DOM.

import * as THREE from 'three';

const SCALE = 1.5;

export function makeSphere(radius, color) {
    const geo = new THREE.SphereGeometry(radius * SCALE, 24, 16);
    const mat = new THREE.MeshStandardMaterial({
        color,
        emissive: color,
        emissiveIntensity: 0.3,
        metalness: 0.2,
        roughness: 0.5,
    });
    return new THREE.Mesh(geo, mat);
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
