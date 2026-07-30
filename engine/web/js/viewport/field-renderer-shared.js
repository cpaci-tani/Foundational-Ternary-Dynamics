/**
 * Shared helpers/constants for FieldRenderer mixins.
 * VOXEL_CENTER_OFFSET must stay 0.0 (do not "fix" to 0.5).
 */

import * as THREE from 'three';
import { PARTICLE_VERT, PARTICLE_FRAG, PARTICLE_SHADER_UNIFORMS } from './shaders.js';

// Confinement-string visual: color-pair proximity glyph cutoff. [IMPOSED]
// √120 ≈ 10.95 voxels — preserved verbatim from prior hardcode.
export const CONFINEMENT_PAIR_DIST2 = 120.0;

// Lattice voxel index k is rendered at world centre k+0.5 natively in WASM
// samplers; this JS offset must remain 0.0.
export let VOXEL_CENTER_OFFSET = 0.0;

/** Points material using PARTICLE_VERT + PARTICLE_FRAG (manifest disabled). */
export function _makeParticleFragMaterial(overrides = {}, extra = {}) {
    return new THREE.ShaderMaterial({
        uniforms: { ...PARTICLE_SHADER_UNIFORMS, ...overrides },
        vertexShader: PARTICLE_VERT,
        fragmentShader: PARTICLE_FRAG,
        transparent: true,
        depthWrite: false,
        blending: THREE.NormalBlending,
        ...extra,
    });
}

export function _ensureManifestAttrs(geometry, capacity) {
    if (!geometry.getAttribute('manifestPhase')) {
        geometry.setAttribute('manifestPhase',
            new THREE.Float32BufferAttribute(new Float32Array(capacity), 1));
    }
    if (!geometry.getAttribute('manifestRate')) {
        geometry.setAttribute('manifestRate',
            new THREE.Float32BufferAttribute(new Float32Array(capacity), 1));
    }
}

// Lazy-built static texture for soft-disc sprite (weak-field / quantum overlays).
let __softSpriteTex = null;
export function _softSpriteTexture() {
    if (__softSpriteTex) return __softSpriteTex;
    const s = 64;
    const canvas = document.createElement('canvas');
    canvas.width = s; canvas.height = s;
    const ctx = canvas.getContext('2d');
    const g = ctx.createRadialGradient(s/2, s/2, 0, s/2, s/2, s/2);
    g.addColorStop(0.00, 'rgba(255, 220, 255, 1.00)');
    g.addColorStop(0.20, 'rgba(255, 255, 255, 0.85)');
    g.addColorStop(0.55, 'rgba(255, 255, 255, 0.35)');
    g.addColorStop(1.00, 'rgba(255, 255, 255, 0.00)');
    ctx.fillStyle = g;
    ctx.fillRect(0, 0, s, s);
    const tex = new THREE.CanvasTexture(canvas);
    tex.needsUpdate = true;
    __softSpriteTex = tex;
    return tex;
}

export function setVoxelCenterOffset(v) { VOXEL_CENTER_OFFSET = v; }
export function getVoxelCenterOffset() { return VOXEL_CENTER_OFFSET; }
