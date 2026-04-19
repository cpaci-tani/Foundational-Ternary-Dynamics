/**
 * Nebula theme — colorful multi-layer volumetric gas clouds with filaments
 * and a composed starfield backdrop.
 */
import * as THREE from 'three';
import { BG_RADIUS, NEBULA_CLOUDS, NEBULA_PTS, randSphere, hsl, gaussRand } from './_shared.js';
import { buildStarField, animateStarField } from './starfield.js';

function buildNebula() {
    const group = new THREE.Group();
    group.name = 'bg-nebula';

    // Rich palette — each cloud gets a primary + secondary hue
    const palettes = [
        { h: 0.78, s: 0.8, l: 0.35, h2: 0.85, name: 'violet' },
        { h: 0.55, s: 0.7, l: 0.30, h2: 0.50, name: 'teal' },
        { h: 0.90, s: 0.75, l: 0.30, h2: 0.95, name: 'magenta' },
        { h: 0.08, s: 0.8, l: 0.30, h2: 0.05, name: 'ember' },
        { h: 0.62, s: 0.65, l: 0.28, h2: 0.70, name: 'sapphire' },
        { h: 0.0,  s: 0.7, l: 0.28, h2: 0.97, name: 'crimson' },
        { h: 0.45, s: 0.6, l: 0.25, h2: 0.40, name: 'emerald' },
        { h: 0.72, s: 0.7, l: 0.32, h2: 0.80, name: 'lavender' },
        { h: 0.15, s: 0.9, l: 0.28, h2: 0.10, name: 'gold' },
        { h: 0.58, s: 0.5, l: 0.22, h2: 0.65, name: 'steel' },
    ];

    const cloudData = [];

    // Nebula cloud shader — large soft gaussian blobs
    const nebulaVert = `
        attribute float size;
        attribute vec3 particleColor;
        attribute float alpha;
        varying vec3 vColor;
        varying float vAlpha;
        void main() {
            vColor = particleColor;
            vAlpha = alpha;
            vec4 mv = modelViewMatrix * vec4(position, 1.0);
            gl_PointSize = size * (400.0 / -mv.z);
            gl_PointSize = clamp(gl_PointSize, 1.0, 64.0);
            gl_Position = projectionMatrix * mv;
        }
    `;
    const nebulaFrag = `
        varying vec3 vColor;
        varying float vAlpha;
        void main() {
            float d = length(gl_PointCoord - vec2(0.5));
            if (d > 0.5) discard;
            // soft gaussian falloff — gives volumetric cloud look
            float a = exp(-d * d * 4.0) * vAlpha;
            // subtle rim brightening for depth illusion
            float rim = smoothstep(0.3, 0.48, d) * 0.15;
            gl_FragColor = vec4(vColor + rim, a);
        }
    `;

    for (let c = 0; c < NEBULA_CLOUDS; c++) {
        const pal = palettes[c % palettes.length];
        const [cx, cy, cz] = randSphere(BG_RADIUS * 0.75);

        // Each cloud has 3 layers: dense core, mid halo, outer wisp
        const layers = [
            { count: Math.floor(NEBULA_PTS * 0.15), spread: 20 + Math.random() * 15,
              sizeMin: 8, sizeMax: 25, alphaBase: 0.55, lBoost: 0.15 },     // dense core
            { count: Math.floor(NEBULA_PTS * 0.35), spread: 50 + Math.random() * 30,
              sizeMin: 12, sizeMax: 40, alphaBase: 0.30, lBoost: 0.05 },    // mid halo
            { count: NEBULA_PTS - Math.floor(NEBULA_PTS * 0.5), spread: 90 + Math.random() * 60,
              sizeMin: 15, sizeMax: 55, alphaBase: 0.12, lBoost: -0.02 },   // outer wisp
        ];

        const totalPts = layers.reduce((s, l) => s + l.count, 0);
        const positions = new Float32Array(totalPts * 3);
        const colors    = new Float32Array(totalPts * 3);
        const sizes     = new Float32Array(totalPts);
        const alphas    = new Float32Array(totalPts);

        let idx = 0;
        for (const layer of layers) {
            for (let i = 0; i < layer.count; i++) {
                // true gaussian scatter for natural cloud shape
                const gx = gaussRand() * layer.spread;
                const gy = gaussRand() * layer.spread * 0.6; // flatten slightly
                const gz = gaussRand() * layer.spread;
                positions[idx * 3]     = cx + gx;
                positions[idx * 3 + 1] = cy + gy;
                positions[idx * 3 + 2] = cz + gz;

                // blend primary and secondary hue for color variation
                const hueBlend = Math.random();
                const h = hueBlend < 0.7
                    ? pal.h + (Math.random() - 0.5) * 0.06
                    : pal.h2 + (Math.random() - 0.5) * 0.06;
                const col = hsl(
                    h,
                    pal.s + (Math.random() - 0.5) * 0.15,
                    Math.max(0.08, pal.l + layer.lBoost + Math.random() * 0.1)
                );
                colors[idx * 3]     = col.r;
                colors[idx * 3 + 1] = col.g;
                colors[idx * 3 + 2] = col.b;

                sizes[idx] = layer.sizeMin + Math.random() * (layer.sizeMax - layer.sizeMin);
                alphas[idx] = layer.alphaBase * (0.6 + Math.random() * 0.4);
                idx++;
            }
        }

        const geom = new THREE.BufferGeometry();
        geom.setAttribute('position',      new THREE.BufferAttribute(positions, 3));
        geom.setAttribute('particleColor', new THREE.BufferAttribute(colors, 3));
        geom.setAttribute('size',          new THREE.BufferAttribute(sizes, 1));
        geom.setAttribute('alpha',         new THREE.BufferAttribute(alphas, 1));

        const mat = new THREE.ShaderMaterial({
            vertexShader: nebulaVert,
            fragmentShader: nebulaFrag,
            transparent: true,
            depthWrite: false,
            blending: THREE.AdditiveBlending,
        });

        const pts = new THREE.Points(geom, mat);
        group.add(pts);
        cloudData.push({
            geom, basePositions: new Float32Array(positions),
            baseAlphas: new Float32Array(alphas), sizes: new Float32Array(sizes)
        });
    }

    // Filaments — thin bright streaks connecting cloud regions
    const filCount = 2000;
    const filPos = new Float32Array(filCount * 3);
    const filCol = new Float32Array(filCount * 3);
    const filSz  = new Float32Array(filCount);
    const filAl  = new Float32Array(filCount);
    for (let i = 0; i < filCount; i++) {
        const [x, y, z] = randSphere(BG_RADIUS * 0.7);
        // elongate along a random axis for streaky look
        const axis = (i % 3);
        const stretch = 1.0 + Math.random() * 3.0;
        filPos[i * 3]     = x * (axis === 0 ? stretch : 1);
        filPos[i * 3 + 1] = y * (axis === 1 ? stretch : 1);
        filPos[i * 3 + 2] = z * (axis === 2 ? stretch : 1);
        const col = hsl(Math.random(), 0.4, 0.15 + Math.random() * 0.1);
        filCol[i * 3] = col.r; filCol[i * 3 + 1] = col.g; filCol[i * 3 + 2] = col.b;
        filSz[i] = 1 + Math.random() * 3;
        filAl[i] = 0.08 + Math.random() * 0.12;
    }
    const filGeom = new THREE.BufferGeometry();
    filGeom.setAttribute('position',      new THREE.BufferAttribute(filPos, 3));
    filGeom.setAttribute('particleColor', new THREE.BufferAttribute(filCol, 3));
    filGeom.setAttribute('size',          new THREE.BufferAttribute(filSz, 1));
    filGeom.setAttribute('alpha',         new THREE.BufferAttribute(filAl, 1));
    const filMat = new THREE.ShaderMaterial({
        vertexShader: nebulaVert, fragmentShader: nebulaFrag,
        transparent: true, depthWrite: false, blending: THREE.AdditiveBlending,
    });
    group.add(new THREE.Points(filGeom, filMat));

    // star field behind
    const stars = buildStarField();
    stars.name = 'bg-nebula-stars';
    group.add(stars);

    group.userData = { cloudData, stars };
    return group;
}

function animateNebula(group, time) {
    const { cloudData, stars } = group.userData;
    for (let ci = 0; ci < cloudData.length; ci++) {
        const cloud = cloudData[ci];
        const posAttr = cloud.geom.attributes.position;
        const alphaAttr = cloud.geom.attributes.alpha;
        const pos = posAttr.array;
        const alp = alphaAttr.array;
        const base = cloud.basePositions;
        const baseA = cloud.baseAlphas;
        const n = pos.length / 3;
        const drift = 0.1 + ci * 0.02; // each cloud drifts at slightly different speed
        for (let i = 0; i < n; i++) {
            const phase = i * 0.19 + time * drift;
            // slow billowing motion
            pos[i * 3]     = base[i * 3]     + Math.sin(phase) * 4.0;
            pos[i * 3 + 1] = base[i * 3 + 1] + Math.cos(phase * 0.6) * 3.0;
            pos[i * 3 + 2] = base[i * 3 + 2] + Math.sin(phase * 0.9 + 1.0) * 4.0;
            // gentle alpha pulsing
            alp[i] = baseA[i] * (0.75 + 0.25 * Math.sin(time * 0.3 + i * 0.07));
        }
        posAttr.needsUpdate = true;
        alphaAttr.needsUpdate = true;
    }
    animateStarField(stars, time);
}

export const NebulaTheme = {
    label: 'Nebula',
    build: buildNebula,
    animate: animateNebula,
};
