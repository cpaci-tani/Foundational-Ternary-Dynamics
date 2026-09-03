/**
 * Centralized Shaders for FTD Web Frontend
 * ────────────────────────────────────────────────────────────────────
 *
 * Houses shared GLSL shader strings to ensure DRY compliance and
 * enable global shader optimizations.
 */

/** Shared uniforms for any material using PARTICLE_FRAG (manifest off by default). */
export const PARTICLE_SHADER_UNIFORMS = {
    shapeType: { value: 0 },
    uOpacity: { value: 0.9 },
    uGlow: { value: 0.15 },
    uManifestTime: { value: 0 },
    uManifestEnabled: { value: 0 },
    uManifestThresh: { value: 0.587 },
};

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
    attribute float manifestPhase;
    attribute float manifestRate;
    varying vec3 vColor;
    varying float vSize;
    varying float vManifestPhase;
    varying float vManifestRate;
    varying float vVisibility;

    void main() {
        vColor = particleColor;
        vSize = size;
        vManifestPhase = manifestPhase;
        vManifestRate = manifestRate;
        vVisibility = 1.0;
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
    attribute float manifestPhase;
    attribute float manifestRate;
    attribute float particleVisibility;
    varying vec3 vColor;
    varying float vSize;
    varying float vManifestPhase;
    varying float vManifestRate;
    varying float vVisibility;

    void main() {
        vColor = particleColor;
        vSize = size;
        vManifestPhase = manifestPhase;
        vManifestRate = manifestRate;
        vVisibility = particleVisibility;
        if (particleVisibility < 0.5) {
            gl_PointSize = 0.0;
            gl_Position = vec4(2.0, 2.0, 2.0, 1.0);
            return;
        }
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
    uniform float uManifestTime;
    uniform float uManifestEnabled;
    uniform float uManifestThresh;
    varying vec3 vColor;
    varying float vSize;
    varying float vManifestPhase;
    varying float vManifestRate;
    varying float vVisibility;

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

        float bright = 1.0;
        if (uManifestEnabled > 0.5) {
            float wave = sin(uManifestTime * vManifestRate + vManifestPhase);
            float on = smoothstep(uManifestThresh - 0.10, uManifestThresh + 0.10, wave);
            bright = mix(0.10, 1.0, on);
        }

        vec3 rgb = (vColor + glow) * bright;
        gl_FragColor = vec4(rgb, alpha * alpha * uOpacity * bright * vVisibility);
    }
`;

// Scale-0/1 particle mesh variant.  Scale 1 supplies `appearanceRole` and
// `focusWeight` so its effective records can remain legible without turning
// their presentation cloud into a literal solid-particle claim.  Scale 0
// leaves both attributes at zero and therefore keeps the legacy particle
// rendering path byte-for-byte equivalent in behavior.
export const RECORD_PARTICLE_VERT = `
    attribute float size;
    attribute vec3 particleColor;
    attribute float manifestPhase;
    attribute float manifestRate;
    attribute float appearanceRole;
    attribute float focusWeight;
    varying vec3 vColor;
    varying float vManifestPhase;
    varying float vManifestRate;
    varying float vAppearanceRole;
    varying float vFocusWeight;

    void main() {
        vColor = particleColor;
        vManifestPhase = manifestPhase;
        vManifestRate = manifestRate;
        vAppearanceRole = appearanceRole;
        vFocusWeight = focusWeight;
        vec4 mvPosition = modelViewMatrix * vec4(position, 1.0);
        float focusScale = appearanceRole > 0.5 && appearanceRole < 1.5
            ? mix(1.0, 1.28, focusWeight)
            : 1.0;
        gl_PointSize = size * focusScale * (150.0 / -mvPosition.z);
        gl_PointSize = clamp(gl_PointSize, 1.0, 512.0);
        gl_Position = projectionMatrix * mvPosition;
    }
`;

export const RECORD_PARTICLE_FRAG = `
    uniform int shapeType;
    uniform float uOpacity;
    uniform float uGlow;
    uniform float uManifestTime;
    uniform float uManifestEnabled;
    uniform float uManifestThresh;
    varying vec3 vColor;
    varying float vManifestPhase;
    varying float vManifestRate;
    varying float vAppearanceRole;
    varying float vFocusWeight;

    void main() {
        vec2 c = gl_PointCoord - vec2(0.5);
        float dist;

        if (shapeType == 1) {
            dist = max(abs(c.x), abs(c.y));
            if (dist > 0.48) discard;
        } else if (shapeType == 2) {
            dist = abs(c.x) + abs(c.y);
            if (dist > 0.5) discard;
        } else if (shapeType == 3) {
            float angle = atan(c.y, c.x);
            float r = length(c);
            float star = cos(5.0 * angle) * 0.15 + 0.35;
            if (r > star) discard;
            dist = r / star * 0.5;
        } else if (shapeType == 4) {
            float x = c.x, y = c.y + 0.15;
            if (y > 0.35 || y < -0.35 + 0.7 * abs(x) / 0.4) discard;
            dist = length(c);
        } else if (shapeType == 5) {
            vec2 a = abs(c);
            dist = max(a.x * 0.866 + a.y * 0.5, a.y);
            if (dist > 0.45) discard;
            dist /= 0.45;
        } else if (shapeType == 6) {
            float r = length(c);
            if (r > 0.5 || r < 0.3) discard;
            dist = abs(r - 0.4) / 0.1;
        } else if (shapeType == 7) {
            float ax = abs(c.x), ay = abs(c.y);
            if (ax > 0.15 && ay > 0.15) discard;
            dist = max(ax, ay);
        } else {
            dist = length(c);
            if (dist > 0.5) discard;
        }

        float isCore = step(0.5, vAppearanceRole) * (1.0 - step(1.5, vAppearanceRole));
        float isRim = step(1.5, vAppearanceRole);
        float alpha = 1.0 - smoothstep(0.15, 0.5, dist);
        float glow = exp(-dist * dist * 4.0) * uGlow;

        // Effective-record cores and support rims never blink away.  The
        // localization cloud still carries the manifestation duty cycle, but
        // a raised floor keeps its geometry readable between active phases.
        float bright = 1.0;
        if (uManifestEnabled > 0.5) {
            float wave = sin(uManifestTime * vManifestRate + vManifestPhase);
            float on = smoothstep(uManifestThresh - 0.10, uManifestThresh + 0.10, wave);
            float floorValue = mix(0.24, 0.68, max(isCore, isRim * 0.72));
            bright = mix(floorValue, 1.0, on);
        }

        float hotCenter = isCore * (1.0 - smoothstep(0.0, 0.34, dist));
        vec3 rgb = mix(vColor + glow, vec3(1.0), hotCenter * 0.48) * bright;

        // Inspection is presentation-only: the selected record receives a
        // cyan rim inside its core sprite, with no mutation of engine state.
        float selectionRing = vFocusWeight * isCore
            * smoothstep(0.31, 0.37, dist)
            * (1.0 - smoothstep(0.43, 0.49, dist));
        rgb = mix(rgb, vec3(0.34, 0.94, 1.0), selectionRing);
        alpha = max(alpha, selectionRing * 0.96);

        gl_FragColor = vec4(rgb, alpha * alpha * uOpacity * bright);
    }
`;
