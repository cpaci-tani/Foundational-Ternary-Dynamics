/**
 * Consciousness Figure Builders — procedural geometry for holographic rendering.
 *
 * 12 archetypal forms rendered as "boundaries of flux":
 *   Ethereal:   Aetheric Body, Spirit in the Sky, Plasmoid Cloud
 *   Divine:     God Figure, Demiurge, Yahweh (Pillar of Fire)
 *   Historical: Hotep, Jesus, Mayan Sun God
 *   Dark:       Death Cloud
 *   Classic:    Humanoid, Alien
 *
 * Each builder returns a FigureDescriptor:
 *   { type, geometry, particlePositions, particleCount, particleSizes,
 *     palette, shaderOverrides, scale, yOffset, animationFn }
 */

import * as THREE from 'three';
import { mergeGeometries as _mergeRaw } from 'three/addons/utils/BufferGeometryUtils.js';

/** Safe merge: normalize all geometries to non-indexed with matching attribs before merge */
function mergeGeometries(geos, useGroups) {
    // Ensure all geometries are non-indexed and share the same attribute set
    const attrNames = new Set();
    for (const g of geos) {
        for (const name of Object.keys(g.attributes)) attrNames.add(name);
    }
    const normalized = geos.map(g => {
        let ni = g.index ? g.toNonIndexed() : g;
        for (const name of attrNames) {
            if (!ni.getAttribute(name)) {
                const itemSize = name === 'uv' ? 2 : 3;
                const count = ni.getAttribute('position').count;
                ni.setAttribute(name, new THREE.BufferAttribute(new Float32Array(count * itemSize), itemSize));
            }
        }
        return ni;
    });
    return _mergeRaw(normalized, useGroups);
}

// ── Defaults ─────────────────────────────────────────────────────────

const DEFAULT_PALETTE = {
    primary:   new THREE.Color(0x00e5ff),
    secondary: new THREE.Color(0x7c4dff),
    accent:    new THREE.Color(0xffd740),
};

const DEFAULT_OVERRIDES = {
    scanLineFreq:      40.0,
    scanLineSpeed:     3.0,
    fresnelPower:      2.0,
    glitchScale:       1.0,
    iridescenceStr:    0.15,
};

function desc(partial) {
    return {
        type:              partial.type              || 'mesh',
        geometry:          partial.geometry           || null,
        particlePositions: partial.particlePositions  || null,
        particleCount:     partial.particleCount      || 0,
        particleSizes:     partial.particleSizes      || null,
        palette:           { ...DEFAULT_PALETTE, ...partial.palette },
        shaderOverrides:   { ...DEFAULT_OVERRIDES, ...partial.shaderOverrides },
        scale:             partial.scale              ?? 1.2,
        yOffset:           partial.yOffset            ?? 0,
        animationFn:       partial.animationFn        || null,
    };
}

// ── Helpers ──────────────────────────────────────────────────────────

function cyl(rTop, rBot, h, seg = 8) {
    return new THREE.CylinderGeometry(rTop, rBot, h, seg);
}

function sph(r, wSeg = 12, hSeg = 8) {
    return new THREE.SphereGeometry(r, wSeg, hSeg);
}

/** Generate random points in a sphere shell */
function spherePoints(count, rMin, rMax) {
    const pos = new Float32Array(count * 3);
    const sizes = new Float32Array(count);
    for (let i = 0; i < count; i++) {
        const u = Math.random();
        const r = rMin + (rMax - rMin) * Math.cbrt(u);
        const theta = Math.acos(2 * Math.random() - 1);
        const phi = Math.random() * Math.PI * 2;
        pos[i*3]   = r * Math.sin(theta) * Math.cos(phi);
        pos[i*3+1] = r * Math.sin(theta) * Math.sin(phi);
        pos[i*3+2] = r * Math.cos(theta);
        sizes[i] = 0.4 + Math.random() * 0.8;
    }
    return { pos, sizes };
}

/** Generate points along a vertical cylinder */
function cylinderPoints(count, radius, yMin, yMax) {
    const pos = new Float32Array(count * 3);
    const sizes = new Float32Array(count);
    for (let i = 0; i < count; i++) {
        const angle = Math.random() * Math.PI * 2;
        const r = radius * Math.sqrt(Math.random());
        pos[i*3]   = r * Math.cos(angle);
        pos[i*3+1] = yMin + Math.random() * (yMax - yMin);
        pos[i*3+2] = r * Math.sin(angle);
        sizes[i] = 0.4 + Math.random() * 1.0;
    }
    return { pos, sizes };
}

/** Generate points on a torus */
function torusPoints(count, R, r, noiseSigma = 0) {
    const pos = new Float32Array(count * 3);
    const sizes = new Float32Array(count);
    // Store toroidal/poloidal angles for animation
    const angles = new Float32Array(count * 2);
    for (let i = 0; i < count; i++) {
        const u = Math.random() * Math.PI * 2; // toroidal
        const v = Math.random() * Math.PI * 2; // poloidal
        const rr = r + (noiseSigma > 0 ? gaussRandom() * noiseSigma : 0);
        pos[i*3]   = (R + rr * Math.cos(v)) * Math.cos(u);
        pos[i*3+1] = rr * Math.sin(v);
        pos[i*3+2] = (R + rr * Math.cos(v)) * Math.sin(u);
        sizes[i] = 0.3 + Math.random() * 0.9;
        angles[i*2] = u;
        angles[i*2+1] = v;
    }
    return { pos, sizes, angles };
}

/** Generate points in a downward-expanding cone */
function conePoints(count, yTop, yBot, rTop, rBot) {
    const pos = new Float32Array(count * 3);
    const sizes = new Float32Array(count);
    for (let i = 0; i < count; i++) {
        const t = Math.random(); // 0=top, 1=bottom
        const y = yTop + t * (yBot - yTop);
        const maxR = rTop + t * (rBot - rTop);
        const r = maxR * Math.sqrt(Math.random());
        const angle = Math.random() * Math.PI * 2;
        pos[i*3]   = r * Math.cos(angle);
        pos[i*3+1] = y;
        pos[i*3+2] = r * Math.sin(angle);
        // Smaller at top, larger at bottom (dissolution)
        sizes[i] = 0.3 + t * 0.5 + Math.random() * 0.2;
    }
    return { pos, sizes };
}

/** Generate points along radial rays */
function rayPoints(count, numRays, rMin, rMax) {
    const pos = new Float32Array(count * 3);
    const sizes = new Float32Array(count);
    const rayAngles = new Float32Array(count); // which ray
    const rayDists  = new Float32Array(count); // distance along ray
    for (let i = 0; i < count; i++) {
        const rayIdx = Math.floor(Math.random() * numRays);
        const angle = (rayIdx / numRays) * Math.PI * 2;
        const dist = rMin + Math.random() * (rMax - rMin);
        pos[i*3]   = dist * Math.cos(angle);
        pos[i*3+1] = dist * Math.sin(angle);
        pos[i*3+2] = (Math.random() - 0.5) * 0.3; // slight depth
        sizes[i] = 0.4 + Math.random() * 0.6;
        rayAngles[i] = angle;
        rayDists[i] = dist;
    }
    return { pos, sizes, rayAngles, rayDists };
}

function gaussRandom() {
    // Box-Muller
    const u1 = Math.random();
    const u2 = Math.random();
    return Math.sqrt(-2 * Math.log(u1 || 1e-10)) * Math.cos(2 * Math.PI * u2);
}

// ── 1. Humanoid (Classic Cortana) ────────────────────────────────────

export function buildHumanoid() {
    const parts = [];

    // Head
    const head = sph(0.55, 20, 14);
    head.translate(0, 2.85, 0);
    parts.push(head);

    // Neck
    const neck = cyl(0.18, 0.22, 0.3, 10);
    neck.translate(0, 2.15, 0);
    parts.push(neck);

    // Torso
    const torso = cyl(0.55, 0.4, 1.6, 14);
    torso.translate(0, 1.2, 0);
    parts.push(torso);

    // Hips
    const hips = sph(0.42, 12, 8);
    hips.translate(0, 0.35, 0);
    parts.push(hips);

    // Arms
    const upperArmL = cyl(0.12, 0.1, 1.0);
    upperArmL.rotateZ(0.15);
    upperArmL.translate(-0.72, 1.65, 0);
    parts.push(upperArmL);
    const upperArmR = upperArmL.clone();
    upperArmR.translate(1.44, 0, 0);
    parts.push(upperArmR);

    const forearmL = cyl(0.09, 0.08, 0.9);
    forearmL.rotateZ(0.2);
    forearmL.translate(-0.82, 0.85, 0);
    parts.push(forearmL);
    const forearmR = forearmL.clone();
    forearmR.translate(1.64, 0, 0);
    parts.push(forearmR);

    const handL = sph(0.1, 8, 6);
    handL.translate(-0.9, 0.35, 0);
    parts.push(handL);
    const handR = handL.clone();
    handR.translate(1.8, 0, 0);
    parts.push(handR);

    // Legs
    const upperLegL = cyl(0.16, 0.13, 1.2);
    upperLegL.translate(-0.22, -0.55, 0);
    parts.push(upperLegL);
    const upperLegR = upperLegL.clone();
    upperLegR.translate(0.44, 0, 0);
    parts.push(upperLegR);

    const lowerLegL = cyl(0.12, 0.09, 1.1);
    lowerLegL.translate(-0.22, -1.7, 0);
    parts.push(lowerLegL);
    const lowerLegR = lowerLegL.clone();
    lowerLegR.translate(0.44, 0, 0);
    parts.push(lowerLegR);

    const footL = sph(0.12, 8, 6);
    footL.scale(1, 0.5, 1.6);
    footL.translate(-0.22, -2.3, 0.06);
    parts.push(footL);
    const footR = footL.clone();
    footR.translate(0.44, 0, 0);
    parts.push(footR);

    const merged = mergeGeometries(parts, false);
    merged.computeVertexNormals();

    return desc({
        type: 'mesh',
        geometry: merged,
        palette: {
            primary:   new THREE.Color(0x00e5ff),
            secondary: new THREE.Color(0x7c4dff),
            accent:    new THREE.Color(0xffd740),
        },
    });
}

// ── 2. Alien ─────────────────────────────────────────────────────────

export function buildAlien() {
    const parts = [];

    // Elongated head
    const head = sph(0.6, 20, 14);
    head.scale(0.9, 1.5, 0.85);
    head.translate(0, 3.5, 0);
    parts.push(head);

    // Eyes
    const eyeL = sph(0.18, 10, 8);
    eyeL.translate(-0.25, 3.55, 0.45);
    parts.push(eyeL);
    const eyeR = eyeL.clone();
    eyeR.translate(0.5, 0, 0);
    parts.push(eyeR);

    // Neck
    const neck = cyl(0.14, 0.18, 0.4, 10);
    neck.translate(0, 2.4, 0);
    parts.push(neck);

    // Torso
    const torso = cyl(0.4, 0.25, 1.8, 12);
    torso.translate(0, 1.3, 0);
    parts.push(torso);

    // 4 arms
    const armUL1 = cyl(0.08, 0.06, 1.2);
    armUL1.rotateZ(0.3);
    armUL1.translate(-0.55, 1.8, 0);
    parts.push(armUL1);
    const armUR1 = armUL1.clone();
    armUR1.translate(1.1, 0, 0);
    parts.push(armUR1);

    const armUL2 = cyl(0.07, 0.05, 1.0);
    armUL2.rotateZ(0.4);
    armUL2.rotateY(0.3);
    armUL2.translate(-0.45, 1.2, 0.15);
    parts.push(armUL2);
    const armUR2 = armUL2.clone();
    armUR2.translate(0.9, 0, 0);
    parts.push(armUR2);

    // Forearms
    const foreL1 = cyl(0.05, 0.04, 0.8, 6);
    foreL1.rotateZ(0.35);
    foreL1.translate(-0.85, 1.05, 0);
    parts.push(foreL1);
    const foreR1 = foreL1.clone();
    foreR1.translate(1.7, 0, 0);
    parts.push(foreR1);

    const foreL2 = cyl(0.04, 0.03, 0.7, 6);
    foreL2.rotateZ(0.45);
    foreL2.rotateY(0.3);
    foreL2.translate(-0.7, 0.55, 0.2);
    parts.push(foreL2);
    const foreR2 = foreL2.clone();
    foreR2.translate(1.4, 0, 0);
    parts.push(foreR2);

    // Tentacle legs
    const legL1 = cyl(0.13, 0.1, 1.0);
    legL1.rotateZ(0.05);
    legL1.translate(-0.18, -0.2, 0);
    parts.push(legL1);
    const legL2 = cyl(0.1, 0.07, 0.9);
    legL2.rotateZ(0.1); legL2.rotateX(-0.15);
    legL2.translate(-0.22, -1.15, 0.08);
    parts.push(legL2);
    const legL3 = cyl(0.07, 0.04, 0.8);
    legL3.rotateZ(0.15); legL3.rotateX(-0.25);
    legL3.translate(-0.28, -2.0, 0.2);
    parts.push(legL3);

    const legR1 = cyl(0.13, 0.1, 1.0);
    legR1.rotateZ(-0.05);
    legR1.translate(0.18, -0.2, 0);
    parts.push(legR1);
    const legR2 = cyl(0.1, 0.07, 0.9);
    legR2.rotateZ(-0.1); legR2.rotateX(-0.15);
    legR2.translate(0.22, -1.15, 0.08);
    parts.push(legR2);
    const legR3 = cyl(0.07, 0.04, 0.8);
    legR3.rotateZ(-0.15); legR3.rotateX(-0.25);
    legR3.translate(0.28, -2.0, 0.2);
    parts.push(legR3);

    const merged = mergeGeometries(parts, false);
    merged.computeVertexNormals();

    return desc({
        type: 'mesh',
        geometry: merged,
        palette: {
            primary:   new THREE.Color(0x39ff14),
            secondary: new THREE.Color(0x6a0dad),
            accent:    new THREE.Color(0x00ff88),
        },
    });
}

// ── 3. Hotep (Egyptian Pharaoh) ──────────────────────────────────────

export function buildHotep() {
    const parts = [];

    // Head
    const head = sph(0.5, 16, 12);
    head.translate(0, 2.85, 0);
    parts.push(head);

    // Nemes headdress — two trapezoidal panels
    const nemesL = new THREE.BoxGeometry(0.5, 1.4, 0.08);
    nemesL.translate(-0.45, 2.3, 0);
    nemesL.rotateZ(0.15);
    parts.push(nemesL);
    const nemesR = new THREE.BoxGeometry(0.5, 1.4, 0.08);
    nemesR.translate(0.45, 2.3, 0);
    nemesR.rotateZ(-0.15);
    parts.push(nemesR);

    // Nemes top ridge
    const nemesTop = new THREE.BoxGeometry(0.9, 0.12, 0.5);
    nemesTop.translate(0, 3.25, -0.05);
    parts.push(nemesTop);

    // Uraeus (cobra on forehead)
    const uraeus = cyl(0.05, 0.03, 0.35, 6);
    uraeus.translate(0, 3.35, 0.35);
    uraeus.rotateX(-0.3);
    parts.push(uraeus);

    // Broad collar (ring around neck/shoulders)
    const collar = new THREE.TorusGeometry(0.55, 0.08, 8, 20);
    collar.rotateX(Math.PI / 2);
    collar.translate(0, 2.0, 0);
    parts.push(collar);

    // Neck
    const neck = cyl(0.18, 0.22, 0.25, 10);
    neck.translate(0, 2.15, 0);
    parts.push(neck);

    // Torso — slightly broader, regal
    const torso = cyl(0.5, 0.35, 1.6, 12);
    torso.translate(0, 1.2, 0);
    parts.push(torso);

    // Shendyt (kilt) — wider cylinder
    const kilt = cyl(0.4, 0.5, 0.8, 12);
    kilt.translate(0, 0.0, 0);
    parts.push(kilt);

    // Right arm — holds ankh
    const armR = cyl(0.1, 0.08, 1.0);
    armR.rotateZ(-0.3);
    armR.translate(0.65, 1.55, 0);
    parts.push(armR);
    const forearmR = cyl(0.08, 0.06, 0.8);
    forearmR.rotateZ(-0.1);
    forearmR.translate(0.8, 0.75, 0.15);
    parts.push(forearmR);

    // Ankh in right hand
    const ankhLoop = new THREE.TorusGeometry(0.12, 0.025, 8, 16);
    ankhLoop.translate(0.85, 0.25, 0.15);
    parts.push(ankhLoop);
    const ankhStem = cyl(0.025, 0.025, 0.35, 6);
    ankhStem.translate(0.85, -0.05, 0.15);
    parts.push(ankhStem);
    const ankhCross = new THREE.BoxGeometry(0.2, 0.04, 0.04);
    ankhCross.translate(0.85, 0.08, 0.15);
    parts.push(ankhCross);

    // Left arm — holds staff
    const armL = cyl(0.1, 0.08, 1.0);
    armL.rotateZ(0.15);
    armL.translate(-0.65, 1.55, 0);
    parts.push(armL);
    const forearmL = cyl(0.08, 0.06, 0.9);
    forearmL.rotateZ(0.05);
    forearmL.translate(-0.7, 0.75, 0);
    parts.push(forearmL);

    // Was-scepter (staff) in left hand
    const staff = cyl(0.03, 0.03, 4.0, 6);
    staff.translate(-0.75, 1.0, 0);
    parts.push(staff);
    const staffHead = sph(0.08, 8, 6);
    staffHead.translate(-0.75, 3.0, 0);
    parts.push(staffHead);

    // Legs
    const legL = cyl(0.13, 0.1, 1.1);
    legL.translate(-0.18, -0.9, 0);
    parts.push(legL);
    const legR = cyl(0.13, 0.1, 1.1);
    legR.translate(0.18, -0.9, 0);
    parts.push(legR);

    // Feet
    const footL = sph(0.1, 8, 6);
    footL.scale(1, 0.4, 1.5);
    footL.translate(-0.18, -1.5, 0.05);
    parts.push(footL);
    const footR = footL.clone();
    footR.translate(0.36, 0, 0);
    parts.push(footR);

    const merged = mergeGeometries(parts, false);
    merged.computeVertexNormals();

    return desc({
        type: 'mesh',
        geometry: merged,
        palette: {
            primary:   new THREE.Color(0xffd700),
            secondary: new THREE.Color(0x00b4d8),
            accent:    new THREE.Color(0x1a237e),
        },
        shaderOverrides: {
            glitchScale: 0.15,
            scanLineFreq: 30,
        },
    });
}

// ── 4. Aetheric Body ─────────────────────────────────────────────────

export function buildAethericBody() {
    const parts = [];

    // Simplified humanoid silhouette (lighter than full humanoid)
    const head = sph(0.45, 14, 10);
    head.translate(0, 2.7, 0);
    parts.push(head);

    const neck = cyl(0.12, 0.15, 0.25);
    neck.translate(0, 2.15, 0);
    parts.push(neck);

    const torso = cyl(0.4, 0.3, 1.5, 10);
    torso.translate(0, 1.2, 0);
    parts.push(torso);

    const hips = sph(0.3, 10, 6);
    hips.translate(0, 0.35, 0);
    parts.push(hips);

    // Arms (simple)
    const armL = cyl(0.08, 0.06, 1.6, 6);
    armL.rotateZ(0.2);
    armL.translate(-0.6, 1.3, 0);
    parts.push(armL);
    const armR = armL.clone();
    armR.translate(1.2, 0, 0);
    parts.push(armR);

    // Legs (simple)
    const legL = cyl(0.1, 0.07, 1.8, 6);
    legL.translate(-0.18, -0.8, 0);
    parts.push(legL);
    const legR = legL.clone();
    legR.translate(0.36, 0, 0);
    parts.push(legR);

    // 7 Chakra spheres along spine
    const chakraPositions = [
        3.1,   // Crown
        2.7,   // Third Eye
        2.15,  // Throat
        1.5,   // Heart
        1.0,   // Solar Plexus
        0.35,  // Sacral
        -0.3,  // Root
    ];
    for (const y of chakraPositions) {
        const chakra = sph(0.1, 10, 8);
        chakra.translate(0, y, 0.15); // slightly forward
        parts.push(chakra);
    }

    const merged = mergeGeometries(parts, false);
    merged.computeVertexNormals();

    // Meridian particles — curves connecting chakras
    const meridianCount = 100;
    const pos = new Float32Array(meridianCount * 3);
    const sizes = new Float32Array(meridianCount);
    let idx = 0;
    for (let i = 0; i < chakraPositions.length - 1 && idx < meridianCount; i++) {
        const y0 = chakraPositions[i];
        const y1 = chakraPositions[i + 1];
        const pointsPerSegment = Math.floor(meridianCount / (chakraPositions.length - 1));
        for (let j = 0; j < pointsPerSegment && idx < meridianCount; j++) {
            const t = j / pointsPerSegment;
            const y = y0 + t * (y1 - y0);
            // Spiral path around body
            const angle = t * Math.PI * 2 + i * 0.9;
            const r = 0.25 + 0.1 * Math.sin(t * Math.PI);
            pos[idx*3]   = r * Math.cos(angle);
            pos[idx*3+1] = y;
            pos[idx*3+2] = r * Math.sin(angle) + 0.15;
            sizes[idx] = 0.3 + Math.random() * 0.5;
            idx++;
        }
    }

    return desc({
        type: 'hybrid',
        geometry: merged,
        particlePositions: pos,
        particleCount: meridianCount,
        particleSizes: sizes,
        palette: {
            primary:   new THREE.Color(0x1a237e),
            secondary: new THREE.Color(0xffd740),
            accent:    new THREE.Color(0xaa00ff),
        },
        shaderOverrides: {
            iridescenceStr: 0.35,
            glitchScale: 0.3,
        },
    });
}

// ── 5. God Figure ────────────────────────────────────────────────────

export function buildGodFigure() {
    const parts = [];

    // Head
    const head = sph(0.55, 18, 14);
    head.translate(0, 3.8, 0);
    parts.push(head);

    // Crown — torus
    const crown = new THREE.TorusGeometry(0.4, 0.06, 8, 20);
    crown.rotateX(Math.PI / 2);
    crown.translate(0, 4.2, 0);
    parts.push(crown);

    // Halo ring behind head
    const halo = new THREE.RingGeometry(0.6, 0.85, 32);
    halo.translate(0, 3.9, -0.3);
    parts.push(halo);

    // Neck
    const neck = cyl(0.2, 0.25, 0.3, 10);
    neck.translate(0, 3.15, 0);
    parts.push(neck);

    // Torso — wide robe
    const torso = cyl(0.5, 0.85, 2.8, 14);
    torso.translate(0, 1.6, 0);
    parts.push(torso);

    // Robe skirt — expanding downward
    const skirt = cyl(0.85, 1.2, 1.8, 14);
    skirt.translate(0, -0.7, 0);
    parts.push(skirt);

    // Arms raised skyward (~45 degrees)
    const armL = cyl(0.12, 0.08, 1.4);
    armL.rotateZ(0.7); // raised
    armL.translate(-0.9, 3.2, 0);
    parts.push(armL);
    const armR = armL.clone();
    armR.translate(1.8, 0, 0);
    parts.push(armR);

    // Forearms reaching up
    const foreL = cyl(0.08, 0.06, 1.0);
    foreL.rotateZ(1.0);
    foreL.translate(-1.5, 3.8, 0);
    parts.push(foreL);
    const foreR = foreL.clone();
    foreR.translate(3.0, 0, 0);
    parts.push(foreR);

    // Hands — open palms
    const handL = sph(0.09, 8, 6);
    handL.translate(-1.9, 4.3, 0);
    parts.push(handL);
    const handR = handL.clone();
    handR.translate(3.8, 0, 0);
    parts.push(handR);

    const merged = mergeGeometries(parts, false);
    merged.computeVertexNormals();

    // Aura particles
    const { pos, sizes } = spherePoints(200, 1.5, 4.0);
    // Shift up to center on figure
    for (let i = 0; i < 200; i++) pos[i*3+1] += 1.5;

    return desc({
        type: 'hybrid',
        geometry: merged,
        particlePositions: pos,
        particleCount: 200,
        particleSizes: sizes,
        palette: {
            primary:   new THREE.Color(0xffd700),
            secondary: new THREE.Color(0xff8f00),
            accent:    new THREE.Color(0xffffff),
        },
        shaderOverrides: {
            scanLineFreq: 20,
            glitchScale: 0.2,
            iridescenceStr: 0.3,
        },
        scale: 1.0, // already large geometry
        yOffset: -0.5,
    });
}

// ── 6. Demiurge ──────────────────────────────────────────────────────

export function buildDemiurge() {
    const parts = [];

    // Head — slightly larger, mysterious
    const head = sph(0.5, 16, 12);
    head.translate(0, 3.0, 0);
    parts.push(head);

    // Third eye
    const thirdEye = sph(0.08, 8, 6);
    thirdEye.translate(0, 3.15, 0.42);
    parts.push(thirdEye);

    // Neck
    const neck = cyl(0.15, 0.2, 0.25);
    neck.translate(0, 2.5, 0);
    parts.push(neck);

    // Torso — broad shoulders
    const torso = cyl(0.5, 0.35, 1.4, 12);
    torso.translate(0, 1.6, 0);
    parts.push(torso);

    // Cross-legged base (lotus position)
    const lotusBase = cyl(0.6, 0.7, 0.3, 12);
    lotusBase.translate(0, 0.5, 0);
    parts.push(lotusBase);

    // Crossed legs — two bent cylinders
    const legCrossL = cyl(0.12, 0.09, 1.2);
    legCrossL.rotateZ(1.2);
    legCrossL.translate(-0.3, 0.3, 0.1);
    parts.push(legCrossL);
    const legCrossR = cyl(0.12, 0.09, 1.2);
    legCrossR.rotateZ(-1.2);
    legCrossR.translate(0.3, 0.3, 0.1);
    parts.push(legCrossR);

    // 6 Arms — 3 pairs at different heights
    const armConfigs = [
        { y: 2.2, rot: 0.5,  len: 1.2, r: 0.1 },   // top pair — up
        { y: 1.7, rot: 1.2,  len: 1.0, r: 0.09 },   // mid pair — wide
        { y: 1.3, rot: 0.8,  len: 0.9, r: 0.08 },   // low pair — angled
    ];
    for (const cfg of armConfigs) {
        const aL = cyl(cfg.r, cfg.r * 0.7, cfg.len);
        aL.rotateZ(cfg.rot);
        aL.translate(-0.55, cfg.y, 0);
        parts.push(aL);
        const aR = cyl(cfg.r, cfg.r * 0.7, cfg.len);
        aR.rotateZ(-cfg.rot);
        aR.translate(0.55, cfg.y, 0);
        parts.push(aR);
    }

    // Sacred geometry behind — hexagram from 6 thin cylinders
    for (let i = 0; i < 6; i++) {
        const angle = (i / 6) * Math.PI * 2;
        const nextAngle = ((i + 2) / 6) * Math.PI * 2;
        const x1 = Math.cos(angle) * 1.8;
        const y1 = Math.sin(angle) * 1.8 + 1.8;
        const x2 = Math.cos(nextAngle) * 1.8;
        const y2 = Math.sin(nextAngle) * 1.8 + 1.8;
        const dx = x2 - x1, dy = y2 - y1;
        const len = Math.sqrt(dx*dx + dy*dy);
        const bar = cyl(0.02, 0.02, len, 4);
        bar.rotateZ(Math.atan2(dy, dx) + Math.PI/2);
        bar.translate((x1+x2)/2, (y1+y2)/2, -0.5);
        parts.push(bar);
    }

    const merged = mergeGeometries(parts, false);
    merged.computeVertexNormals();

    // Orbiting particles in concentric rings
    const ringCount = 150;
    const pos = new Float32Array(ringCount * 3);
    const sizes = new Float32Array(ringCount);
    for (let i = 0; i < ringCount; i++) {
        const ring = Math.floor(Math.random() * 3);
        const r = [1.5, 2.5, 3.5][ring];
        const angle = Math.random() * Math.PI * 2;
        pos[i*3]   = r * Math.cos(angle);
        pos[i*3+1] = 1.8 + (Math.random() - 0.5) * 0.3;
        pos[i*3+2] = r * Math.sin(angle);
        sizes[i] = 0.3 + Math.random() * 0.6;
    }

    return desc({
        type: 'hybrid',
        geometry: merged,
        particlePositions: pos,
        particleCount: ringCount,
        particleSizes: sizes,
        palette: {
            primary:   new THREE.Color(0x4a148c),
            secondary: new THREE.Color(0x0d47a1),
            accent:    new THREE.Color(0xffd700),
        },
        shaderOverrides: {
            iridescenceStr: 0.25,
            glitchScale: 0.5,
        },
        animationFn: 'demiurge',
    });
}

// ── 7. Jesus ─────────────────────────────────────────────────────────

export function buildJesus() {
    const parts = [];

    // Head
    const head = sph(0.5, 16, 12);
    head.translate(0, 3.0, 0);
    parts.push(head);

    // Hair / beard suggestion — slightly larger behind
    const hair = sph(0.35, 10, 8);
    hair.scale(1, 1.2, 0.8);
    hair.translate(0, 2.75, -0.15);
    parts.push(hair);

    // Halo — ring behind head
    const halo = new THREE.RingGeometry(0.55, 0.75, 32);
    halo.translate(0, 3.1, -0.35);
    parts.push(halo);

    // Neck
    const neck = cyl(0.15, 0.18, 0.25);
    neck.translate(0, 2.45, 0);
    parts.push(neck);

    // Torso — robed
    const torso = cyl(0.45, 0.55, 1.6, 12);
    torso.translate(0, 1.4, 0);
    parts.push(torso);

    // Robe / skirt
    const robe = cyl(0.55, 0.75, 2.0, 12);
    robe.translate(0, -0.4, 0);
    parts.push(robe);

    // Arms outstretched — cruciform pose (horizontal)
    const armL = cyl(0.1, 0.07, 1.8);
    armL.rotateZ(Math.PI / 2); // horizontal
    armL.translate(-1.3, 2.0, 0);
    parts.push(armL);
    const armR = cyl(0.1, 0.07, 1.8);
    armR.rotateZ(-Math.PI / 2);
    armR.translate(1.3, 2.0, 0);
    parts.push(armR);

    // Hands — open palms
    const handL = sph(0.09, 8, 6);
    handL.scale(1, 0.6, 1.2);
    handL.translate(-2.2, 2.0, 0);
    parts.push(handL);
    const handR = handL.clone();
    handR.translate(4.4, 0, 0);
    parts.push(handR);

    const merged = mergeGeometries(parts, false);
    merged.computeVertexNormals();

    // Nimbus / aureole particles behind upper body
    const nimbusCount = 100;
    const pos = new Float32Array(nimbusCount * 3);
    const sizes = new Float32Array(nimbusCount);
    for (let i = 0; i < nimbusCount; i++) {
        const angle = Math.random() * Math.PI * 2;
        const r = 0.3 + Math.random() * 1.2;
        pos[i*3]   = r * Math.cos(angle) * 0.6;
        pos[i*3+1] = 2.8 + r * Math.sin(angle) * 0.8;
        pos[i*3+2] = -0.3 - Math.random() * 0.4;
        sizes[i] = 0.4 + Math.random() * 0.6;
    }

    return desc({
        type: 'hybrid',
        geometry: merged,
        particlePositions: pos,
        particleCount: nimbusCount,
        particleSizes: sizes,
        palette: {
            primary:   new THREE.Color(0xfff8e1),
            secondary: new THREE.Color(0xe8b4b4),
            accent:    new THREE.Color(0x90caf9),
        },
        shaderOverrides: {
            glitchScale: 0.05,
            fresnelPower: 2.5,
            scanLineFreq: 15,
            iridescenceStr: 0.2,
        },
    });
}

// ── 8. Yahweh (Pillar of Fire) ───────────────────────────────────────

export function buildYahweh() {
    const parts = [];

    // Central pillar — tall thin column of light
    const pillar = cyl(0.15, 0.2, 6.0, 8);
    pillar.translate(0, 1.0, 0);
    parts.push(pillar);

    // Nested octahedral wireframes — built from thin cylinders (edge struts)
    // to avoid mergeGeometries attribute mismatch with polyhedra
    function strutBetween(ax, ay, az, bx, by, bz, r) {
        const dx = bx - ax, dy = by - ay, dz = bz - az;
        const len = Math.sqrt(dx*dx + dy*dy + dz*dz);
        if (len < 1e-6) return null;
        const strut = cyl(r, r, len, 4);
        // CylinderGeometry is along Y — rotate to align with (dx,dy,dz)
        const dir = new THREE.Vector3(dx, dy, dz).normalize();
        const quat = new THREE.Quaternion().setFromUnitVectors(
            new THREE.Vector3(0, 1, 0), dir
        );
        strut.applyQuaternion(quat);
        strut.translate((ax+bx)/2, (ay+by)/2, (az+bz)/2);
        return strut;
    }

    function octaEdges(radius, yCenter) {
        const struts = [];
        const v = [
            [0, radius + yCenter, 0], [0, -radius + yCenter, 0],
            [radius, yCenter, 0], [-radius, yCenter, 0],
            [0, yCenter, radius], [0, yCenter, -radius],
        ];
        const edges = [
            [0,2],[0,3],[0,4],[0,5], [1,2],[1,3],[1,4],[1,5],
            [2,4],[4,3],[3,5],[5,2],
        ];
        for (const [a, b] of edges) {
            const s = strutBetween(
                v[a][0], v[a][1], v[a][2],
                v[b][0], v[b][1], v[b][2], 0.02
            );
            if (s) struts.push(s);
        }
        return struts;
    }

    parts.push(...octaEdges(0.8, 1.5));

    // Second octahedron, rotated
    const oct2Parts = octaEdges(1.5, 1.5);
    for (const g of oct2Parts) g.rotateY(Math.PI / 4);
    parts.push(...oct2Parts);

    // Third octahedron, rotated differently
    const oct3Parts = octaEdges(2.5, 1.5);
    for (const g of oct3Parts) {
        g.rotateY(Math.PI / 8);
        g.rotateX(Math.PI / 6);
    }
    parts.push(...oct3Parts);

    // Inner sacred core — sphere (compatible with cylinder merge)
    const core = sph(0.5, 12, 8);
    core.translate(0, 1.5, 0);
    parts.push(core);

    const merged = mergeGeometries(parts, false);
    merged.computeVertexNormals();

    // Fire particles — vertical column
    const { pos, sizes } = cylinderPoints(400, 0.8, -2.0, 5.0);

    return desc({
        type: 'hybrid',
        geometry: merged,
        particlePositions: pos,
        particleCount: 400,
        particleSizes: sizes,
        palette: {
            primary:   new THREE.Color(0xffffff),
            secondary: new THREE.Color(0xff6d00),
            accent:    new THREE.Color(0xffee58),
        },
        shaderOverrides: {
            scanLineFreq: 100,
            scanLineSpeed: 8,
            fresnelPower: 1.0,
            glitchScale: 2.0,
            iridescenceStr: 0.08,
        },
        animationFn: 'yahweh',
    });
}

// ── 9. Mayan Sun God ─────────────────────────────────────────────────

export function buildMayanSunGod() {
    const parts = [];

    const NUM_RAYS = 16;

    // Central face disc
    const face = new THREE.CircleGeometry(1.0, 32);
    face.translate(0, 0, 0);
    parts.push(face);

    // Inner ring
    const innerRing = new THREE.TorusGeometry(1.1, 0.06, 8, 32);
    innerRing.translate(0, 0, 0);
    parts.push(innerRing);

    // Eyes (two small spheres on face)
    const eyeL = sph(0.12, 8, 6);
    eyeL.translate(-0.3, 0.15, 0.1);
    parts.push(eyeL);
    const eyeR = eyeL.clone();
    eyeR.translate(0.6, 0, 0);
    parts.push(eyeR);

    // Mouth
    const mouth = new THREE.TorusGeometry(0.15, 0.04, 6, 12, Math.PI);
    mouth.rotateZ(Math.PI);
    mouth.translate(0, -0.2, 0.1);
    parts.push(mouth);

    // Radiating rays — wedge shapes
    for (let i = 0; i < NUM_RAYS; i++) {
        const angle = (i / NUM_RAYS) * Math.PI * 2;
        const ray = new THREE.BoxGeometry(0.12, 2.0, 0.04);
        ray.translate(0, 2.1, 0); // offset from center
        ray.rotateZ(angle);
        parts.push(ray);

        // Stepped decoration at ray tips
        if (i % 2 === 0) {
            const tip = new THREE.BoxGeometry(0.25, 0.25, 0.04);
            tip.translate(0, 3.1, 0);
            tip.rotateZ(angle);
            parts.push(tip);
        }
    }

    // Outer ring
    const outerRing = new THREE.TorusGeometry(3.3, 0.08, 8, 48);
    outerRing.translate(0, 0, 0);
    parts.push(outerRing);

    const merged = mergeGeometries(parts, false);
    merged.computeVertexNormals();

    // Ray particles drifting outward
    const { pos, sizes, rayAngles, rayDists } = rayPoints(200, NUM_RAYS, 1.2, 3.2);

    return desc({
        type: 'hybrid',
        geometry: merged,
        particlePositions: pos,
        particleCount: 200,
        particleSizes: sizes,
        palette: {
            primary:   new THREE.Color(0xff6d00),
            secondary: new THREE.Color(0xc62828),
            accent:    new THREE.Color(0xffeb3b),
        },
        shaderOverrides: {
            glitchScale: 0.8,
            fresnelPower: 1.5,
            scanLineFreq: 25,
        },
        animationFn: 'mayan-sun',
        _rayAngles: rayAngles,
        _rayDists: rayDists,
        _numRays: NUM_RAYS,
    });
}

// ── 10. Death Cloud ──────────────────────────────────────────────────

export function buildDeathCloud() {
    const parts = [];

    // Hood — cone + sphere creating reaper silhouette
    const hood = new THREE.ConeGeometry(0.6, 1.0, 12);
    hood.translate(0, 3.5, 0);
    parts.push(hood);

    const face = sph(0.4, 12, 10);
    face.translate(0, 3.0, 0.1);
    parts.push(face);

    // Eye sockets — two small indentations (dark spheres)
    const eyeL = sph(0.1, 8, 6);
    eyeL.translate(-0.15, 3.1, 0.35);
    parts.push(eyeL);
    const eyeR = eyeL.clone();
    eyeR.translate(0.3, 0, 0);
    parts.push(eyeR);

    // Scythe handle
    const handle = cyl(0.03, 0.03, 3.5, 6);
    handle.rotateZ(-0.3);
    handle.translate(0.8, 1.8, 0);
    parts.push(handle);

    // Scythe blade — curved box
    const blade = new THREE.BoxGeometry(1.2, 0.04, 0.15);
    blade.rotateZ(0.6);
    blade.translate(0.3, 3.6, 0);
    parts.push(blade);
    const bladeTip = new THREE.BoxGeometry(0.6, 0.03, 0.12);
    bladeTip.rotateZ(1.0);
    bladeTip.translate(-0.2, 3.3, 0);
    parts.push(bladeTip);

    const merged = mergeGeometries(parts, false);
    merged.computeVertexNormals();

    // Dissolution particles — dense near hood, chaotic below
    const { pos, sizes } = conePoints(700, 2.5, -3.0, 0.5, 3.0);

    return desc({
        type: 'hybrid',
        geometry: merged,
        particlePositions: pos,
        particleCount: 700,
        particleSizes: sizes,
        palette: {
            primary:   new THREE.Color(0x1a0033),
            secondary: new THREE.Color(0x76ff03),
            accent:    new THREE.Color(0x8b0000),
        },
        shaderOverrides: {
            glitchScale: 3.0,
            scanLineFreq: 60,
            iridescenceStr: 0.05,
        },
        animationFn: 'death-cloud',
    });
}

// ── 11. Plasmoid Cloud ───────────────────────────────────────────────

export function buildPlasmoidCloud() {
    const { pos, sizes, angles } = torusPoints(800, 2.0, 1.0, 0.3);

    return desc({
        type: 'points',
        particlePositions: pos,
        particleCount: 800,
        particleSizes: sizes,
        palette: {
            primary:   new THREE.Color(0xff6d00),
            secondary: new THREE.Color(0x2979ff),
            accent:    new THREE.Color(0xffffff),
        },
        shaderOverrides: {
            scanLineFreq: 80,
            scanLineSpeed: 6,
            fresnelPower: 1.2,
            glitchScale: 0.6,
        },
        animationFn: 'plasmoid',
        _torusAngles: angles,
        _torusR: 2.0,
        _torusr: 1.0,
    });
}

// ── 12. Spirit in the Sky ────────────────────────────────────────────

export function buildSpirit() {
    const parts = [];

    // Upper body only — head, neck, torso, upper arms
    const head = sph(0.5, 16, 12);
    head.translate(0, 2.85, 0);
    parts.push(head);

    const neck = cyl(0.15, 0.18, 0.25);
    neck.translate(0, 2.2, 0);
    parts.push(neck);

    const torso = cyl(0.45, 0.35, 1.4, 12);
    torso.translate(0, 1.3, 0);
    parts.push(torso);

    // Arms spread outward gently
    const armL = cyl(0.1, 0.06, 1.4);
    armL.rotateZ(0.6);
    armL.translate(-0.8, 1.8, 0);
    parts.push(armL);
    const armR = armL.clone();
    armR.translate(1.6, 0, 0);
    parts.push(armR);

    const merged = mergeGeometries(parts, false);
    merged.computeVertexNormals();

    // Dissolution particles below waist
    const { pos, sizes } = conePoints(500, 0.5, -3.5, 0.3, 2.5);

    return desc({
        type: 'hybrid',
        geometry: merged,
        particlePositions: pos,
        particleCount: 500,
        particleSizes: sizes,
        palette: {
            primary:   new THREE.Color(0xe3f2fd),
            secondary: new THREE.Color(0xb0bec5),
            accent:    new THREE.Color(0xffd54f),
        },
        shaderOverrides: {
            glitchScale: 0.1,
            iridescenceStr: 0.4,
            fresnelPower: 3.0,
            scanLineFreq: 25,
        },
        animationFn: 'spirit',
    });
}

// ── FIGURE REGISTRY ──────────────────────────────────────────────────

export const FIGURE_REGISTRY = new Map([
    ['humanoid',    buildHumanoid],
    ['alien',       buildAlien],
    ['hotep',       buildHotep],
    ['aetheric',    buildAethericBody],
    ['god',         buildGodFigure],
    ['demiurge',    buildDemiurge],
    ['jesus',       buildJesus],
    ['yahweh',      buildYahweh],
    ['mayan-sun',   buildMayanSunGod],
    ['death-cloud', buildDeathCloud],
    ['plasmoid',    buildPlasmoidCloud],
    ['spirit',      buildSpirit],
]);
