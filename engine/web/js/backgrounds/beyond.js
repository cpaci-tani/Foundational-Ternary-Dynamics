/**
 * "The Beyond" theme — fading grid extending outward, suggesting
 * a lattice with no defined boundary, with sparse flickering void
 * points between lines.
 */
import * as THREE from 'three';
import { BG_RADIUS, GRID_EXTENT, GRID_STEP, randSphere, hsl } from './_shared.js';

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

export const BeyondTheme = {
    label: 'The Beyond',
    build: buildTheBeyond,
    animate: animateTheBeyond,
};
