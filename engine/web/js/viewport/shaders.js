/**
 * Centralized Shaders for FTD Web Frontend
 * ────────────────────────────────────────────────────────────────────
 *
 * Houses shared GLSL shader strings to ensure DRY compliance and
 * enable global shader optimizations.
 */

// Custom particle vertex shader. Linear 1/z point-size scaling — the
// standard size convention shared by the particle Points mesh and every
// field-overlay Points/LineSegments material (E/B/Poynting/divergence/
// force volumes/dark matter/genesis/chirality/light…). Centralized here
// (D-1) so all sites import a single byte-identical copy. Byte-identical
// across the former viewport.js + field-renderer.js + particle-renderer.js
// copies (419 chars) verified before merge.
export const PARTICLE_VERT = `
    attribute float size;
    attribute vec3 particleColor;
    varying vec3 vColor;
    varying float vSize;

    void main() {
        vColor = particleColor;
        vSize = size;
        vec4 mvPosition = modelViewMatrix * vec4(position, 1.0);
        gl_PointSize = size * (150.0 / -mvPosition.z);
        gl_PointSize = clamp(gl_PointSize, 1.0, 512.0);
        gl_Position = projectionMatrix * mvPosition;
    }
`;

// Flux-volume variant: sqrt depth scaling instead of linear 1/z.
// For N=8 the camera is only ~9-19 units away, so linear 1/z gives a
// 2× size ratio between near and far faces, making the sphere look
// wildly asymmetric.  sqrt(60/z) compresses that to ~1.4× so both
// hemispheres stay visually balanced regardless of lattice size.
// Centralized here (D-1); byte-identical across the former viewport.js +
// flux-renderer.js copies (461 chars) verified before merge.
export const FLUX_VOL_VERT = `
    attribute float size;
    attribute vec3 particleColor;
    varying vec3 vColor;
    varying float vSize;

    void main() {
        vColor = particleColor;
        vSize = size;
        vec4 mvPosition = modelViewMatrix * vec4(position, 1.0);
        float depth = max(-mvPosition.z, 0.1);
        gl_PointSize = size * sqrt(60.0 / depth);
        gl_PointSize = clamp(gl_PointSize, 1.0, 512.0);
        gl_Position = projectionMatrix * mvPosition;
    }
`;

export const PARTICLE_FRAG = `
    uniform int shapeType;
    uniform float uOpacity;
    uniform float uGlow;
    varying vec3 vColor;
    varying float vSize;

    void main() {
        vec2 c = gl_PointCoord - vec2(0.5);
        float dist;

        if (shapeType == 1) {
            // Square
            dist = max(abs(c.x), abs(c.y));
            if (dist > 0.48) discard;
        } else if (shapeType == 2) {
            // Diamond
            dist = abs(c.x) + abs(c.y);
            if (dist > 0.5) discard;
        } else if (shapeType == 3) {
            // Star (5-pointed)
            float angle = atan(c.y, c.x);
            float r = length(c);
            float star = cos(5.0 * angle) * 0.15 + 0.35;
            if (r > star) discard;
            dist = r / star * 0.5;
        } else if (shapeType == 4) {
            // Triangle
            float x = c.x, y = c.y + 0.15;
            if (y > 0.35 || y < -0.35 + 0.7 * abs(x) / 0.4) discard;
            dist = length(c);
        } else if (shapeType == 5) {
            // Hexagon
            vec2 a = abs(c);
            dist = max(a.x * 0.866 + a.y * 0.5, a.y);
            if (dist > 0.45) discard;
            dist /= 0.45;
        } else if (shapeType == 6) {
            // Ring
            float r = length(c);
            if (r > 0.5 || r < 0.3) discard;
            dist = abs(r - 0.4) / 0.1;
        } else if (shapeType == 7) {
            // Cross
            float ax = abs(c.x), ay = abs(c.y);
            if (ax > 0.15 && ay > 0.15) discard;
            dist = max(ax, ay);
        } else {
            // Circle (default, shapeType == 0)
            dist = length(c);
            if (dist > 0.5) discard;
        }

        float alpha = 1.0 - smoothstep(0.15, 0.5, dist);
        float glow = exp(-dist * dist * 4.0) * uGlow;
        gl_FragColor = vec4(vColor + glow, alpha * alpha * uOpacity);
    }
`;
