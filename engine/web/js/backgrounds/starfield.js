/**
 * Starfield theme — deep-space star field with twinkling.
 *
 * Also composed by NebulaTheme and FluxStormTheme as a background layer,
 * so the build/animate functions are exported alongside the theme object.
 */
import * as THREE from 'three';
import { BG_RADIUS, STAR_COUNT, randSphere, hsl } from './_shared.js';

export function buildStarField() {
    const group = new THREE.Group();
    group.name = 'bg-starfield';

    const positions = new Float32Array(STAR_COUNT * 3);
    const colors    = new Float32Array(STAR_COUNT * 3);
    const sizes     = new Float32Array(STAR_COUNT);
    const baseAlpha = new Float32Array(STAR_COUNT);  // stored for twinkle

    for (let i = 0; i < STAR_COUNT; i++) {
        const [x, y, z] = randSphere(BG_RADIUS);
        positions[i * 3]     = x;
        positions[i * 3 + 1] = y;
        positions[i * 3 + 2] = z;

        // color: mostly white, some blue-white, some warm
        const temp = Math.random();
        let col;
        if (temp < 0.6)      col = hsl(0, 0, 0.7 + 0.3 * Math.random());       // white
        else if (temp < 0.8) col = hsl(0.6, 0.4, 0.6 + 0.3 * Math.random());   // blue-ish
        else if (temp < 0.9) col = hsl(0.08, 0.5, 0.6 + 0.3 * Math.random());  // warm
        else                 col = hsl(0.55, 0.6, 0.5 + 0.4 * Math.random());   // cyan

        colors[i * 3]     = col.r;
        colors[i * 3 + 1] = col.g;
        colors[i * 3 + 2] = col.b;

        sizes[i] = 0.5 + Math.random() * 1.5;
        baseAlpha[i] = 0.4 + Math.random() * 0.6;
    }

    const geom = new THREE.BufferGeometry();
    geom.setAttribute('position',      new THREE.BufferAttribute(positions, 3));
    geom.setAttribute('particleColor', new THREE.BufferAttribute(colors, 3));
    geom.setAttribute('size',          new THREE.BufferAttribute(sizes, 1));

    const mat = new THREE.ShaderMaterial({
        vertexShader: `
            attribute float size;
            attribute vec3 particleColor;
            varying vec3 vColor;
            void main() {
                vColor = particleColor;
                vec4 mv = modelViewMatrix * vec4(position, 1.0);
                gl_PointSize = size * (200.0 / -mv.z);
                gl_PointSize = clamp(gl_PointSize, 0.3, 3.0);
                gl_Position = projectionMatrix * mv;
            }
        `,
        fragmentShader: `
            varying vec3 vColor;
            void main() {
                float d = length(gl_PointCoord - vec2(0.5));
                if (d > 0.5) discard;
                float a = 1.0 - smoothstep(0.0, 0.5, d);
                gl_FragColor = vec4(vColor, a * 0.45);
            }
        `,
        transparent: true,
        depthWrite: false,
        blending: THREE.AdditiveBlending,
    });

    const pts = new THREE.Points(geom, mat);
    pts.renderOrder = -10; // always behind the simulation
    group.add(pts);

    group.userData = { sizes, baseAlpha, geom };
    return group;
}

export function animateStarField(group, time) {
    const { sizes, baseAlpha, geom } = group.userData;
    const sizeAttr = geom.attributes.size;
    const arr = sizeAttr.array;
    // twinkle ~10% of stars per frame for efficiency
    const n = sizes.length;
    for (let k = 0; k < n * 0.08; k++) {
        const i = (Math.random() * n) | 0;
        const flicker = baseAlpha[i] * (0.6 + 0.4 * Math.sin(time * 3.0 + i * 7.13));
        arr[i] = sizes[i] * flicker;
    }
    sizeAttr.needsUpdate = true;
}

export const StarfieldTheme = {
    label: 'Star Field',
    build: buildStarField,
    animate: animateStarField,
};
