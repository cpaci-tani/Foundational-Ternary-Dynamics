/**
 * @file engine/web/js/viewport/mesh-factory.js
 * @purpose Utility factory functions to build Three.js buffer geometries and line meshes.
 */

import * as THREE from 'three';

/**
 * Builds a LineSegments mesh configured for streamline visualization.
 * @param {THREE.Scene} scene - The active scene to add the mesh to.
 * @param {number} maxVerts - The maximum number of vertices for the streamline.
 * @param {number} [opacity=0.7] - The material opacity.
 * @returns {THREE.LineSegments} The constructed streamline mesh.
 */
export function buildStreamlineMesh(scene, maxVerts, opacity = 0.7) {
    const positions = new Float32Array(maxVerts * 3);
    const colors = new Float32Array(maxVerts * 3);
    const geo = new THREE.BufferGeometry();
    geo.setAttribute('position', new THREE.Float32BufferAttribute(positions, 3));
    geo.setAttribute('color', new THREE.Float32BufferAttribute(colors, 3));
    geo.setDrawRange(0, 0);
    const mat = new THREE.LineBasicMaterial({
        vertexColors: true,
        transparent: true,
        opacity,
        blending: THREE.AdditiveBlending,
        depthWrite: false,
    });
    const mesh = new THREE.LineSegments(geo, mat);
    mesh.visible = false;
    mesh.frustumCulled = false;
    scene.add(mesh);
    return mesh;
}

/**
 * Builds a LineSegments mesh configured for arrow vector field visualization.
 * @param {THREE.Scene} scene - The active scene to add the mesh to.
 * @param {number} maxArrows - The maximum number of arrows to support.
 * @param {number} [opacity=0.7] - The material opacity.
 * @returns {THREE.LineSegments} The constructed arrow field mesh.
 */
export function buildArrowFieldMesh(scene, maxArrows, opacity = 0.7) {
    const positions = new Float32Array(maxArrows * 2 * 3);
    const colors = new Float32Array(maxArrows * 2 * 3);
    const geo = new THREE.BufferGeometry();
    geo.setAttribute('position', new THREE.Float32BufferAttribute(positions, 3));
    geo.setAttribute('color', new THREE.Float32BufferAttribute(colors, 3));
    geo.setDrawRange(0, 0);
    const mat = new THREE.LineBasicMaterial({
        vertexColors: true,
        transparent: true,
        opacity,
        depthWrite: false,
    });
    const mesh = new THREE.LineSegments(geo, mat);
    mesh.visible = false;
    mesh.frustumCulled = false;
    scene.add(mesh);
    return mesh;
}
