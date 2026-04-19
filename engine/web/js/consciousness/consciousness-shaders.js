/**
 * Consciousness shader strings — pure GLSL, zero state.
 *
 * Extracted from consciousness.js (ticket CE-2).
 */

// ── Holographic Mesh Shader ──────────────────────────────────────────

export const HOLO_VERT = `
    uniform float uTime;
    uniform float uDeformAmt;

    varying vec3 vNormal;
    varying vec3 vPosition;
    varying vec3 vWorldPosition;
    varying vec2 vUv;

    void main() {
        vNormal = normalize(normalMatrix * normal);
        vUv = uv;

        // Flux-driven vertex displacement (breathing + ripple)
        float displacement = sin(position.y * 3.0 + uTime * 2.0) * uDeformAmt;
        displacement += sin(position.x * 5.0 + uTime * 1.5) * uDeformAmt * 0.5;
        displacement += cos(position.z * 4.0 + uTime * 1.8) * uDeformAmt * 0.3;
        vec3 pos = position + normal * displacement;

        vec4 worldPos = modelMatrix * vec4(pos, 1.0);
        vWorldPosition = worldPos.xyz;
        vPosition = pos;
        gl_Position = projectionMatrix * viewMatrix * worldPos;
    }
`;

export const HOLO_FRAG = `
    uniform float uTime;
    uniform float uFluxEnergy;
    uniform float uObservableFraction;
    uniform vec3  uBaseColor;
    uniform vec3  uSecondaryColor;
    uniform float uOpacity;
    uniform float uScanLineFreq;
    uniform float uScanLineSpeed;
    uniform float uFresnelPower;
    uniform float uGlitchScale;
    uniform float uIridescenceStr;

    varying vec3 vNormal;
    varying vec3 vPosition;
    varying vec3 vWorldPosition;
    varying vec2 vUv;

    void main() {
        // Fresnel rim glow
        vec3 viewDir = normalize(cameraPosition - vWorldPosition);
        float fresnel = 1.0 - abs(dot(viewDir, vNormal));
        fresnel = pow(fresnel, uFresnelPower);

        // Scan lines (scrolling upward)
        float scanLine = sin(vPosition.y * uScanLineFreq - uTime * uScanLineSpeed) * 0.5 + 0.5;
        scanLine = smoothstep(0.3, 0.7, scanLine);

        // Glitch noise (inversely proportional to flux stability)
        float glitch = fract(sin(dot(vUv * 100.0 + uTime * 0.5, vec2(12.9898, 78.233))) * 43758.5453);
        float glitchStrength = (0.05 * uGlitchScale) / (1.0 + uFluxEnergy * 2.0);

        // Iridescence (rainbow shift based on view angle)
        float iridescence = fract(fresnel * 2.0 + uTime * 0.1);
        vec3 iriColor = vec3(
            sin(iridescence * 6.2832) * 0.5 + 0.5,
            sin(iridescence * 6.2832 + 2.094) * 0.5 + 0.5,
            sin(iridescence * 6.2832 + 4.189) * 0.5 + 0.5
        );

        // Compose final color
        vec3 base = mix(uBaseColor, uSecondaryColor, fresnel);
        base = mix(base, iriColor, uIridescenceStr * uObservableFraction);
        base *= (0.7 + 0.3 * scanLine);
        base += glitch * glitchStrength;

        // Opacity: solid core, transparent edges with fresnel glow
        float alpha = uOpacity * (0.3 + 0.7 * (1.0 - fresnel));
        alpha += fresnel * 0.6;
        alpha *= (0.85 + 0.15 * scanLine);

        gl_FragColor = vec4(base, clamp(alpha, 0.0, 1.0));
    }
`;

// ── Particle Holographic Shader ──────────────────────────────────────

export const PARTICLE_VERT = `
    attribute float aSize;
    uniform float uTime;
    uniform float uDeformAmt;

    varying vec3 vWorldPosition;
    varying float vSize;

    void main() {
        vSize = aSize;
        vec3 pos = position;
        // Gentle breathing on particles
        pos.y += sin(pos.x * 3.0 + uTime * 1.5) * uDeformAmt * 0.5;
        pos.x += cos(pos.y * 2.0 + uTime * 1.2) * uDeformAmt * 0.3;

        vec4 worldPos = modelMatrix * vec4(pos, 1.0);
        vWorldPosition = worldPos.xyz;

        vec4 mvPosition = modelViewMatrix * vec4(pos, 1.0);
        gl_PointSize = aSize * (800.0 / max(-mvPosition.z, 0.1));
        gl_PointSize = clamp(gl_PointSize, 2.0, 180.0);
        gl_Position = projectionMatrix * mvPosition;
    }
`;

export const PARTICLE_FRAG = `
    uniform float uTime;
    uniform float uFluxEnergy;
    uniform vec3  uBaseColor;
    uniform vec3  uSecondaryColor;
    uniform float uOpacity;
    uniform float uScanLineFreq;
    uniform float uScanLineSpeed;

    varying vec3 vWorldPosition;
    varying float vSize;

    void main() {
        vec2 center = gl_PointCoord - vec2(0.5);
        float dist = length(center);

        // Soft Gaussian falloff — no hard discard, smooth gas cloud
        float gauss = exp(-dist * dist * 8.0);

        // Very subtle scan lines (reduced from mesh intensity)
        float scanLine = sin(vWorldPosition.y * uScanLineFreq * 0.3 - uTime * uScanLineSpeed * 0.5) * 0.5 + 0.5;
        scanLine = mix(0.85, 1.0, smoothstep(0.3, 0.7, scanLine));

        // Color: gentle shift from center to edge
        float edgeMix = smoothstep(0.0, 0.45, dist);
        vec3 base = mix(uBaseColor, uSecondaryColor, edgeMix * 0.5);
        base *= scanLine;

        // Inner glow (brighter core)
        float glow = exp(-dist * dist * 20.0) * 0.4;
        base += glow * uBaseColor;

        float alpha = gauss * uOpacity * (0.15 + 0.35 * uFluxEnergy);
        gl_FragColor = vec4(base, clamp(alpha, 0.0, 0.7));
    }
`;

// ── sLoop ring shader (golden) ───────────────────────────────────────

export const SLOOP_FRAG = `
    uniform float uTime;
    uniform float uFluxEnergy;
    uniform float uPulse;

    varying vec3 vNormal;
    varying vec3 vPosition;
    varying vec3 vWorldPosition;
    varying vec2 vUv;

    void main() {
        vec3 viewDir = normalize(cameraPosition - vWorldPosition);
        float fresnel = 1.0 - abs(dot(viewDir, vNormal));
        fresnel = pow(fresnel, 1.5);

        float scanLine = sin(vPosition.x * 30.0 + vPosition.z * 30.0 - uTime * 4.0) * 0.5 + 0.5;

        vec3 gold = vec3(1.0, 0.84, 0.0);
        vec3 white = vec3(1.0, 0.95, 0.8);
        vec3 base = mix(gold, white, fresnel * 0.5);
        base *= (0.6 + 0.4 * scanLine);

        float alpha = (0.15 + 0.25 * fresnel + 0.15 * uPulse) * (0.7 + 0.3 * scanLine);
        alpha *= (0.5 + 0.5 * uFluxEnergy);

        gl_FragColor = vec4(base, clamp(alpha, 0.0, 0.85));
    }
`;
