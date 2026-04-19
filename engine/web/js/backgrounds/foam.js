/**
 * Quantum Foam theme — dense flickering micro-points suggesting
 * vacuum fluctuations.
 */
import * as THREE from 'three';
import { BG_RADIUS, FOAM_COUNT, randSphere, hsl } from './_shared.js';

function buildQuantumFoam() {
    const group = new THREE.Group();
    group.name = 'bg-foam';

    const positions = new Float32Array(FOAM_COUNT * 3);
    const colors    = new Float32Array(FOAM_COUNT * 3);
    const sizes     = new Float32Array(FOAM_COUNT);
    const phases    = new Float32Array(FOAM_COUNT);

    for (let i = 0; i < FOAM_COUNT; i++) {
        const [x, y, z] = randSphere(BG_RADIUS * (0.5 + 0.5 * Math.random()));
        positions[i * 3]     = x;
        positions[i * 3 + 1] = y;
        positions[i * 3 + 2] = z;

        // muted blue-violet palette
        const col = hsl(
            0.6 + Math.random() * 0.2,
            0.3 + Math.random() * 0.3,
            0.15 + Math.random() * 0.15
        );
        colors[i * 3]     = col.r;
        colors[i * 3 + 1] = col.g;
        colors[i * 3 + 2] = col.b;

        sizes[i] = 0.3 + Math.random() * 1.0;
        phases[i] = Math.random() * Math.PI * 2;
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
                gl_PointSize = size * (150.0 / -mv.z);
                gl_PointSize = clamp(gl_PointSize, 0.3, 4.0);
                gl_Position = projectionMatrix * mv;
            }
        `,
        fragmentShader: `
            varying vec3 vColor;
            void main() {
                float d = length(gl_PointCoord - vec2(0.5));
                if (d > 0.5) discard;
                float a = 1.0 - smoothstep(0.1, 0.5, d);
                gl_FragColor = vec4(vColor, a * 0.7);
            }
        `,
        transparent: true,
        depthWrite: false,
        blending: THREE.AdditiveBlending,
    });

    const pts = new THREE.Points(geom, mat);
    group.add(pts);

    group.userData = { geom, sizes, phases };
    return group;
}

function animateQuantumFoam(group, time) {
    const { geom, sizes, phases } = group.userData;
    const sizeAttr = geom.attributes.size;
    const arr = sizeAttr.array;
    const n = sizes.length;
    // rapid flickering — each point blinks at its own frequency
    for (let i = 0; i < n; i++) {
        const t = time * (2.0 + (i % 7) * 0.5) + phases[i];
        const flicker = 0.2 + 0.8 * (0.5 + 0.5 * Math.sin(t));
        arr[i] = sizes[i] * flicker;
    }
    sizeAttr.needsUpdate = true;
}

export const FoamTheme = {
    label: 'Quantum Foam',
    build: buildQuantumFoam,
    animate: animateQuantumFoam,
};
