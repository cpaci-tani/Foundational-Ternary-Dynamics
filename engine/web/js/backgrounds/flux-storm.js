/**
 * Flux Storm theme — swirling tilted bands of colored particles
 * suggesting active flux dynamics, with a composed starfield backdrop.
 */
import * as THREE from 'three';
import { BG_RADIUS, hsl } from './_shared.js';
import { buildStarField, animateStarField } from './starfield.js';

function buildFluxStorm() {
    const group = new THREE.Group();
    group.name = 'bg-storm';

    const bandCount = 4;
    const ptsPerBand = 3000;
    const bandData = [];

    const bandPalettes = [
        { h: 0.55, s: 0.7, l: 0.3 },  // electric blue
        { h: 0.8,  s: 0.6, l: 0.25 },  // violet
        { h: 0.1,  s: 0.7, l: 0.25 },  // orange
        { h: 0.45, s: 0.5, l: 0.2 },   // teal
    ];

    for (let b = 0; b < bandCount; b++) {
        const pal = bandPalettes[b];
        const positions = new Float32Array(ptsPerBand * 3);
        const colors    = new Float32Array(ptsPerBand * 3);
        const sizes     = new Float32Array(ptsPerBand);
        const angles    = new Float32Array(ptsPerBand);

        // spiral band on a tilted plane
        const tilt = (b / bandCount) * Math.PI;
        const radius = BG_RADIUS * (0.6 + 0.3 * Math.random());

        for (let i = 0; i < ptsPerBand; i++) {
            const angle = (i / ptsPerBand) * Math.PI * 2 + b * 0.5;
            const r = radius + (Math.random() - 0.5) * 80;
            const spread = (Math.random() - 0.5) * 50;

            let x = r * Math.cos(angle);
            let y = spread;
            let z = r * Math.sin(angle);

            // tilt
            const cosT = Math.cos(tilt), sinT = Math.sin(tilt);
            const ny = y * cosT - z * sinT;
            const nz = y * sinT + z * cosT;

            positions[i * 3]     = x;
            positions[i * 3 + 1] = ny;
            positions[i * 3 + 2] = nz;

            const col = hsl(
                pal.h + (Math.random() - 0.5) * 0.08,
                pal.s,
                pal.l + Math.random() * 0.1
            );
            colors[i * 3]     = col.r;
            colors[i * 3 + 1] = col.g;
            colors[i * 3 + 2] = col.b;

            sizes[i] = 3 + Math.random() * 10;
            angles[i] = angle;
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
                    gl_PointSize = size * (350.0 / -mv.z);
                    gl_PointSize = clamp(gl_PointSize, 0.5, 40.0);
                    gl_Position = projectionMatrix * mv;
                }
            `,
            fragmentShader: `
                varying vec3 vColor;
                void main() {
                    float d = length(gl_PointCoord - vec2(0.5));
                    if (d > 0.5) discard;
                    float a = exp(-d * d * 4.0) * 0.5;
                    gl_FragColor = vec4(vColor, a);
                }
            `,
            transparent: true,
            depthWrite: false,
            blending: THREE.AdditiveBlending,
        });

        const pts = new THREE.Points(geom, mat);
        group.add(pts);

        bandData.push({ geom, basePositions: new Float32Array(positions), tilt, radius, angles });
    }

    // background stars
    const stars = buildStarField();
    group.add(stars);

    group.userData = { bandData, stars };
    return group;
}

function animateFluxStorm(group, time) {
    const { bandData, stars } = group.userData;
    for (let b = 0; b < bandData.length; b++) {
        const band = bandData[b];
        const posAttr = band.geom.attributes.position;
        const arr = posAttr.array;
        const base = band.basePositions;
        const n = arr.length / 3;
        const rotSpeed = 0.03 * (1 + b * 0.3);
        const cosR = Math.cos(time * rotSpeed);
        const sinR = Math.sin(time * rotSpeed);
        for (let i = 0; i < n; i++) {
            // rotate around Y axis
            const bx = base[i * 3], bz = base[i * 3 + 2];
            arr[i * 3]     = bx * cosR - bz * sinR;
            arr[i * 3 + 1] = base[i * 3 + 1] + Math.sin(time * 0.8 + i * 0.01) * 3;
            arr[i * 3 + 2] = bx * sinR + bz * cosR;
        }
        posAttr.needsUpdate = true;
    }
    animateStarField(stars, time);
}

export const FluxStormTheme = {
    label: 'Flux Storm',
    build: buildFluxStorm,
    animate: animateFluxStorm,
};
