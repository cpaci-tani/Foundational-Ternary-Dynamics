/**
 * cosmic/shaders.js — GLSL shader sources + blackbody color helper.
 * Extracted from cosmic-renderer.js. Accretion disk (Doppler beaming +
 * radial heat gradient) and relativistic jet (turbulent plasma noise).
 */

// -- Accretion disk shader (Doppler beaming + radial heat gradient) --
export const DISK_VERT = `
varying vec2 vUv;
varying vec3 vWorldPos;
void main() {
    vUv = uv;
    vWorldPos = (modelMatrix * vec4(position, 1.0)).xyz;
    gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
}`;

export const DISK_FRAG = `
uniform float time;
uniform float innerRadius;
uniform float outerRadius;
uniform vec3 bhPosition;
uniform float opacity;
varying vec2 vUv;
varying vec3 vWorldPos;

void main() {
    vec2 local = vWorldPos.xz - bhPosition.xz;
    float r = length(local);
    float t = clamp((r - innerRadius) / (outerRadius - innerRadius), 0.0, 1.0);

    // Radial heat gradient: inner = white-blue, mid = orange, outer = deep red
    vec3 colInner = vec3(1.0, 0.95, 0.85);
    vec3 colMid   = vec3(1.0, 0.55, 0.15);
    vec3 colOuter = vec3(0.4, 0.08, 0.02);
    vec3 col = t < 0.35
        ? mix(colInner, colMid, t / 0.35)
        : mix(colMid, colOuter, (t - 0.35) / 0.65);

    // Keplerian spiral pattern
    float angle = atan(local.y, local.x);
    float spiral = sin(angle * 3.0 - time * 2.5 / pow(max(r, 0.1), 1.5)) * 0.5 + 0.5;
    col *= 0.75 + 0.25 * spiral;

    // Doppler beaming: approaching side (positive x) is brighter
    float doppler = 0.7 + 0.3 * (local.x / max(r, 0.01));
    col *= doppler;

    // Opacity: strong inner, fading outer with soft edge
    float alpha = (1.0 - t * t) * smoothstep(outerRadius, outerRadius * 0.85, r);
    alpha *= smoothstep(innerRadius * 0.9, innerRadius * 1.2, r);
    alpha *= 0.85;

    gl_FragColor = vec4(col, alpha * opacity);
}`;

export const JET_VERT = `
varying vec2 vUv;
varying vec3 vWorldPos;
void main() {
    vUv = uv;
    vec4 worldPosition = modelMatrix * vec4(position, 1.0);
    vWorldPos = worldPosition.xyz;
    gl_Position = projectionMatrix * viewMatrix * worldPosition;
}`;

export const JET_FRAG = `
uniform float time;
uniform float intensity;
varying vec2 vUv;
varying vec3 vWorldPos;

float hash1( float n ) { return fract(sin(n)*43758.5453); }
float noise( in vec2 x ) {
    vec2 p = floor(x);
    vec2 f = fract(x);
    f = f*f*(3.0-2.0*f);
    float n = p.x + p.y*57.0;
    return mix(mix(hash1(n+0.0), hash1(n+1.0),f.x),
               mix(hash1(n+57.0), hash1(n+58.0),f.x),f.y);
}

void main() {
    float y = vUv.y;
    // Smooth cylinder shape
    float r = sin(vUv.x * 3.14159);
    float alpha = sin(y * 3.14159) * r;

    // Core of the jet is intensely white, outer bounds are brilliant blue
    float core = smoothstep(0.4, 1.0, r);
    vec3 color = mix(vec3(0.05, 0.3, 1.0), vec3(1.0, 1.0, 1.0), core);

    // Organic upward flowing turbulent plasma noise
    // Using scaled time from the physics engine to respect the speed slider
    float turb = noise(vec2(vUv.x * 4.0, vUv.y * 8.0 - time * 6.0)) * 0.6 + 0.4;
    alpha *= turb;

    // Vertical intensity gradient: brighter near the base (BH), tapering heavily at the tip, and soft at the very root
    float rootFade = smoothstep(0.0, 0.1, y); // soft connection to the black hole
    float tipFade = smoothstep(1.0, 0.3, y); // long taper fading out towards the tip
    alpha *= (rootFade * tipFade);

    gl_FragColor = vec4(color, alpha * intensity * 1.2); // slight global brightness bump
}`;


export function blackbodyColor(T) {
    const t = Math.max(0, Math.min(1, (T - 1500) / 30000));
    if (t < 0.15) return [1.0, 0.3, 0.05];
    if (t < 0.3)  return [1.0, 0.55, 0.15];
    if (t < 0.45) return [1.0, 0.85, 0.4];
    if (t < 0.6)  return [1.0, 0.95, 0.8];
    if (t < 0.75) return [0.85, 0.9, 1.0];
    return [0.6, 0.7, 1.0];
}
