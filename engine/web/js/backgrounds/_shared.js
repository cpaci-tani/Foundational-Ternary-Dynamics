/**
 * Shared utilities for background theme modules.
 *
 * Extracted from backgrounds.js during BG-1 refactor.
 * Themes that build procedural particle systems share these helpers:
 *   - Constants: BG_RADIUS (sphere radius), palette-independent sizing
 *   - randSphere(): uniform point on shell with slight radial jitter
 *   - hsl(): THREE.Color factory from HSL
 *   - gaussRand(): Box-Muller gaussian for natural cloud scatter
 *
 * Note: StarfieldTheme.build() is also effectively shared — both
 * NebulaTheme and FluxStormTheme compose a star field behind their
 * primary effect. They import StarfieldTheme directly rather than
 * re-exporting here to keep the dependency visible.
 */
import * as THREE from 'three';

export const BG_RADIUS     = 500;   // sphere radius for star/nebula placement
export const STAR_COUNT    = 3000;  // reduced from 6000 to prevent grid-like patterns at rotation
export const NEBULA_CLOUDS = 10;
export const NEBULA_PTS    = 3000;  // per cloud layer
export const FOAM_COUNT    = 12000;
export const GRID_EXTENT   = 300;
export const GRID_STEP     = 8;

export function randSphere(radius) {
    const u = Math.random(), v = Math.random();
    const theta = 2 * Math.PI * u;
    const phi = Math.acos(2 * v - 1);
    const r = radius * (0.85 + 0.15 * Math.random());
    return [
        r * Math.sin(phi) * Math.cos(theta),
        r * Math.sin(phi) * Math.sin(theta),
        r * Math.cos(phi)
    ];
}

export function hsl(h, s, l) {
    const c = new THREE.Color();
    c.setHSL(h, s, l);
    return c;
}

/** Gaussian random with Box-Muller transform */
export function gaussRand() {
    let u, v;
    do { u = Math.random(); } while (u === 0);
    v = Math.random();
    return Math.sqrt(-2.0 * Math.log(u)) * Math.cos(2.0 * Math.PI * v);
}
