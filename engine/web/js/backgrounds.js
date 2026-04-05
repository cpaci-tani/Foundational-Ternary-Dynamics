/**
 * FTD Environment Backgrounds
 *
 * Pseudo-environmental effects rendered behind the simulation lattice.
 * Each background is a Three.js group of Points/Lines added to the scene
 * at large distance so it doesn't interfere with simulation geometry.
 */
import * as THREE from 'three';
import { RGBELoader } from 'three/addons/loaders/RGBELoader.js';

// ── Constants ────────────────────────────────────────────────────────
const BG_RADIUS     = 500;   // sphere radius for star/nebula placement (pushed further out)
const STAR_COUNT    = 3000;  // reduced from 6000 to prevent grid-like patterns at rotation
const NEBULA_CLOUDS = 10;
const NEBULA_PTS    = 3000;  // per cloud layer
const FOAM_COUNT    = 12000;
const GRID_EXTENT   = 300;
const GRID_STEP     = 8;

// ── Utility ──────────────────────────────────────────────────────────
function randSphere(radius) {
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

function hsl(h, s, l) {
    const c = new THREE.Color();
    c.setHSL(h, s, l);
    return c;
}

// ── Background Builders ──────────────────────────────────────────────

/** Deep-space star field with twinkling */
function buildStarField() {
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

function animateStarField(group, time) {
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


/** Gaussian random with Box-Muller transform */
function gaussRand() {
    let u, v;
    do { u = Math.random(); } while (u === 0);
    v = Math.random();
    return Math.sqrt(-2.0 * Math.log(u)) * Math.cos(2.0 * Math.PI * v);
}

/** Colorful nebula gas clouds — multi-layer volumetric look */
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


/** Quantum foam — dense flickering micro-points suggesting vacuum fluctuations */
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


/** "The Beyond" — fading grid extending outward, suggesting infinite lattice */
function buildTheBeyond() {
    const group = new THREE.Group();
    group.name = 'bg-beyond';

    const linePositions = [];
    const lineColors    = [];

    const fadeStart = 40;
    const fadeEnd   = GRID_EXTENT;

    // generate grid lines on all 3 planes, fading with distance
    for (let axis = 0; axis < 3; axis++) {
        for (let a = -GRID_EXTENT; a <= GRID_EXTENT; a += GRID_STEP) {
            for (let b = -GRID_EXTENT; b <= GRID_EXTENT; b += GRID_STEP) {
                // line along the third axis
                const len = GRID_EXTENT;
                const steps = 8;
                for (let s = 0; s < steps; s++) {
                    const t0 = -len + (2 * len * s / steps);
                    const t1 = -len + (2 * len * (s + 1) / steps);

                    const p0 = [0, 0, 0], p1 = [0, 0, 0];
                    const dims = [0, 1, 2].filter(d => d !== axis);
                    p0[dims[0]] = a; p0[dims[1]] = b; p0[axis] = t0;
                    p1[dims[0]] = a; p1[dims[1]] = b; p1[axis] = t1;

                    // alpha based on distance from origin
                    const d0 = Math.sqrt(p0[0] ** 2 + p0[1] ** 2 + p0[2] ** 2);
                    const d1 = Math.sqrt(p1[0] ** 2 + p1[1] ** 2 + p1[2] ** 2);
                    const a0 = Math.max(0, 1 - (d0 - fadeStart) / (fadeEnd - fadeStart));
                    const a1 = Math.max(0, 1 - (d1 - fadeStart) / (fadeEnd - fadeStart));

                    if (a0 < 0.01 && a1 < 0.01) continue; // skip invisible

                    linePositions.push(...p0, ...p1);
                    // color: dim cyan-blue fading out
                    lineColors.push(0.15 * a0, 0.25 * a0, 0.4 * a0);
                    lineColors.push(0.15 * a1, 0.25 * a1, 0.4 * a1);
                }
            }
        }
    }

    const geom = new THREE.BufferGeometry();
    geom.setAttribute('position', new THREE.Float32BufferAttribute(linePositions, 3));
    geom.setAttribute('color',    new THREE.Float32BufferAttribute(lineColors, 3));

    const mat = new THREE.LineBasicMaterial({
        vertexColors: true,
        transparent: true,
        opacity: 0.4,
        depthWrite: false,
    });

    const lines = new THREE.LineSegments(geom, mat);
    group.add(lines);

    // add sparse flickering void points between grid lines
    const voidCount = 1500;
    const voidPos   = new Float32Array(voidCount * 3);
    const voidCol   = new Float32Array(voidCount * 3);
    const voidSz    = new Float32Array(voidCount);
    const voidPh    = new Float32Array(voidCount);

    for (let i = 0; i < voidCount; i++) {
        const [x, y, z] = randSphere(BG_RADIUS * 0.6);
        voidPos[i * 3]     = x;
        voidPos[i * 3 + 1] = y;
        voidPos[i * 3 + 2] = z;
        const col = hsl(0.58, 0.3, 0.1 + Math.random() * 0.1);
        voidCol[i * 3]     = col.r;
        voidCol[i * 3 + 1] = col.g;
        voidCol[i * 3 + 2] = col.b;
        voidSz[i] = 0.3 + Math.random() * 0.8;
        voidPh[i] = Math.random() * Math.PI * 2;
    }

    const vGeom = new THREE.BufferGeometry();
    vGeom.setAttribute('position',      new THREE.BufferAttribute(voidPos, 3));
    vGeom.setAttribute('particleColor', new THREE.BufferAttribute(voidCol, 3));
    vGeom.setAttribute('size',          new THREE.BufferAttribute(voidSz, 1));

    const vMat = new THREE.ShaderMaterial({
        vertexShader: `
            attribute float size;
            attribute vec3 particleColor;
            varying vec3 vColor;
            void main() {
                vColor = particleColor;
                vec4 mv = modelViewMatrix * vec4(position, 1.0);
                gl_PointSize = size * (150.0 / -mv.z);
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
                gl_FragColor = vec4(vColor, a * 0.5);
            }
        `,
        transparent: true,
        depthWrite: false,
        blending: THREE.AdditiveBlending,
    });

    const vPts = new THREE.Points(vGeom, vMat);
    group.add(vPts);

    group.userData = { vGeom, voidSz, voidPh, lineMat: mat };
    return group;
}

function animateTheBeyond(group, time) {
    const { vGeom, voidSz, voidPh, lineMat } = group.userData;
    // subtle grid pulse
    lineMat.opacity = 0.3 + 0.1 * Math.sin(time * 0.5);

    // flickering void points
    const sizeAttr = vGeom.attributes.size;
    const arr = sizeAttr.array;
    const n = voidSz.length;
    for (let i = 0; i < n; i++) {
        const t = time * 1.5 + voidPh[i];
        arr[i] = voidSz[i] * (0.3 + 0.7 * Math.max(0, Math.sin(t)));
    }
    sizeAttr.needsUpdate = true;
}


/** Flux Storm — swirling colored bands suggesting active flux dynamics */
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


// ══════════════════════════════════════════════════════════════════════
// ── 360° HDRI Environment Backgrounds ───────────────────────────────
// Equirectangular HDRIs from Poly Haven (CC0) loaded via RGBELoader.
// Set as scene.background + scene.environment for realistic lighting.
// ══════════════════════════════════════════════════════════════════════

const HDRI_BASE = 'https://dl.polyhaven.org/file/ph-assets/HDRIs/hdr/1k/';

const HDRI_ENVIRONMENTS = {
    studio:    { label: 'Studio',       file: 'studio_small_09_1k.hdr' },
    workshop:  { label: 'Workshop',     file: 'machine_shop_02_1k.hdr' },
    sunset:    { label: 'Sunset',       file: 'kloofendal_48d_partly_cloudy_puresky_1k.hdr' },
    night:     { label: 'Night Sky',    file: 'moonless_golf_1k.hdr' },
    forest:    { label: 'Forest',       file: 'syferfontein_18d_clear_puresky_1k.hdr' },
    urban:     { label: 'Urban',        file: 'potsdamer_platz_1k.hdr' },
};


// ── Background Registry ──────────────────────────────────────────────
const BACKGROUNDS = {
    // Cosmic (procedural particle effects)
    none:      { label: 'None',          build: null,              animate: null },
    stars:     { label: 'Star Field',    build: buildStarField,    animate: animateStarField },
    nebula:    { label: 'Nebula',        build: buildNebula,       animate: animateNebula },
    foam:      { label: 'Quantum Foam',  build: buildQuantumFoam,  animate: animateQuantumFoam },
    beyond:    { label: 'The Beyond',    build: buildTheBeyond,    animate: animateTheBeyond },
    storm:     { label: 'Flux Storm',    build: buildFluxStorm,    animate: animateFluxStorm },
};

// Add HDRI environments to registry (marker: hdri = true)
for (const [key, env] of Object.entries(HDRI_ENVIRONMENTS)) {
    BACKGROUNDS[key] = { label: env.label, hdri: env.file, build: null, animate: null };
}


// ── BackgroundManager ────────────────────────────────────────────────
export class BackgroundManager {
    constructor(scene) {
        this._scene = scene;
        this._current = null;       // name string
        this._group = null;         // THREE.Group (for particle backgrounds)
        this._animateFn = null;     // per-frame callback
        this._time = 0;
        this._defaultBg = new THREE.Color(0x0f1729);
        this._hdriCache = {};       // cache loaded HDRI textures by key
        this._hdriTexture = null;   // currently active HDRI texture
        this._loader = new RGBELoader();
        this._pmremGenerator = null;
    }

    /** Available background names for populating UI */
    static get options() {
        return Object.entries(BACKGROUNDS).map(([key, val]) => ({
            value: key,
            label: val.label
        }));
    }

    /** Set the active background by name */
    set(name, renderer) {
        // tear down previous particle background
        if (this._group) {
            this._scene.remove(this._group);
            this._disposeGroup(this._group);
            this._group = null;
            this._animateFn = null;
        }
        // clear previous HDRI state (textures stay in cache)
        this._hdriTexture = null;
        this._scene.environment = null;
        this._scene.fog = null;
        this._scene.backgroundIntensity = 1.0;
        this._scene.backgroundBlurriness = 0.0;

        this._current = name;
        const entry = BACKGROUNDS[name];
        if (!entry) {
            this._scene.background = this._defaultBg;
            return;
        }

        // ── HDRI environment ──
        if (entry.hdri) {
            this._scene.background = new THREE.Color(0x111111); // temp while loading
            this._loadHDRI(name, entry.hdri, renderer);
            return;
        }

        // ── 'none' ──
        if (!entry.build) {
            this._scene.background = this._defaultBg;
            return;
        }

        // ── Particle background ──
        this._scene.background = new THREE.Color(0x060a14);
        this._group = entry.build();
        this._animateFn = entry.animate;
        this._scene.add(this._group);
    }

    /** Load HDRI from cache or Poly Haven CDN */
    _loadHDRI(name, file, renderer) {
        // Use cached texture if available
        if (this._hdriCache[name]) {
            this._applyHDRI(this._hdriCache[name], renderer);
            return;
        }

        const url = HDRI_BASE + file;
        this._loader.load(url,
            (texture) => {
                // Only apply if still the current selection (user may have switched)
                this._hdriCache[name] = texture;
                if (this._current === name) {
                    this._applyHDRI(texture, renderer);
                }
            },
            undefined, // progress
            (err) => {
                console.warn(`Failed to load HDRI "${name}" from ${url}:`, err);
                // Fall back to default dark background
                if (this._current === name) {
                    this._scene.background = this._defaultBg;
                }
            }
        );
    }

    /** Apply loaded HDRI texture as scene background + environment */
    _applyHDRI(texture, renderer) {
        texture.mapping = THREE.EquirectangularReflectionMapping;
        this._hdriTexture = texture;
        this._scene.background = texture;
        // Also set as environment for IBL (image-based lighting) on any meshes
        this._scene.environment = texture;
        this._scene.backgroundIntensity = 0.8;
        this._scene.backgroundBlurriness = 0.0;
    }

    /** Call each frame from the render loop */
    update(dt) {
        if (!this._animateFn || !this._group) return;
        this._time += dt;
        this._animateFn(this._group, this._time);
    }

    /** Current background name */
    get current() { return this._current; }

    /** Clean up everything */
    dispose() {
        if (this._group) {
            this._scene.remove(this._group);
            this._disposeGroup(this._group);
        }
        // Dispose cached HDRI textures
        for (const tex of Object.values(this._hdriCache)) {
            tex.dispose();
        }
        this._hdriCache = {};
        this._scene.environment = null;
    }

    _disposeGroup(obj) {
        obj.traverse(child => {
            if (child.geometry) child.geometry.dispose();
            if (child.material) {
                if (Array.isArray(child.material)) child.material.forEach(m => m.dispose());
                else child.material.dispose();
            }
        });
    }
}
