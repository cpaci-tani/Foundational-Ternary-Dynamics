/**
 * Spin-Arrow Manager — Three.js primitive that follows tracked
 * particles and visualizes their spin orientation + precession rate.
 *
 * Architecture per Auditor #3 spec:
 *   - quaternion.slerp toward target orientation each frame (no Euler-angle pop)
 *   - depthTest: false, renderOrder: 999 — always-on-top against bright flux
 *   - two-tone material (#FFD24A shaft, #FF4A4A head)
 *   - reference axis cylinder (gray) + θ=0 phase tick so even sub-degree
 *     rotation registers visually
 *   - parent each arrow to a THREE.Group that follows particle position
 *     via position.lerp (decouples render-rate jitter from physics ticks)
 *
 * Honest disclosure: the engine does not yet implement spin-precession
 * physics — particle.spin is randomly initialized at manifestation and
 * does not evolve under applied B-field. This visualization rotates the
 * arrow at the *Schwinger-predicted* rate (passed in via track()) so
 * users can see what the prediction LOOKS like; the slot is reserved
 * for true measurement once engine spin dynamics are added.
 */

import * as THREE from 'three';

const SHAFT_COLOR = 0xFFD24A;
const HEAD_COLOR  = 0xFF4A4A;
const REF_COLOR   = 0x88AABB;

/**
 * Build the arrow Group: cylinder + cone + reference axis + phase tick.
 * Local +Z is the spin axis; arrow tip points at +Z.
 */
function buildArrowGroup() {
    const group = new THREE.Group();

    // Shaft
    const shaftLen = 1.6;
    const shaftGeom = new THREE.CylinderGeometry(0.08, 0.08, shaftLen, 12);
    shaftGeom.translate(0, shaftLen / 2, 0);                 // base at origin
    const shaftMat = new THREE.MeshBasicMaterial({
        color: SHAFT_COLOR,
        depthTest: false,
        transparent: true,
    });
    const shaft = new THREE.Mesh(shaftGeom, shaftMat);
    shaft.renderOrder = 999;
    group.add(shaft);

    // Head
    const headLen = 0.55;
    const headGeom = new THREE.ConeGeometry(0.22, headLen, 14);
    headGeom.translate(0, shaftLen + headLen / 2, 0);
    const headMat = new THREE.MeshBasicMaterial({
        color: HEAD_COLOR,
        depthTest: false,
        transparent: true,
    });
    const head = new THREE.Mesh(headGeom, headMat);
    head.renderOrder = 999;
    group.add(head);

    // Reference axis: thin gray cylinder along local +z so even tiny rotation
    // is visible relative to a fixed reference. Slightly longer than the arrow
    // and at lower opacity.
    const refLen = 2.6;
    const refGeom = new THREE.CylinderGeometry(0.015, 0.015, refLen, 8);
    refGeom.translate(0, refLen / 2 - 0.4, 0);
    const refMat = new THREE.MeshBasicMaterial({
        color: REF_COLOR,
        depthTest: false,
        transparent: true,
        opacity: 0.4,
    });
    const ref = new THREE.Mesh(refGeom, refMat);
    ref.renderOrder = 998;
    group.add(ref);

    // Phase tick — short radial segment at the head plane indicating θ=0.
    // Rendered as two small spheres for a "+" cue rather than a line.
    const tickGeom = new THREE.SphereGeometry(0.06, 8, 8);
    const tickMat = new THREE.MeshBasicMaterial({
        color: 0xFFFFFF,
        depthTest: false,
        transparent: true,
        opacity: 0.85,
    });
    const tick = new THREE.Mesh(tickGeom, tickMat);
    tick.position.set(0.30, shaftLen + headLen / 2, 0);  // offset on +x at the head height
    tick.renderOrder = 999;
    group.add(tick);

    // Three.js cylinders/cones default to +Y; rotate so local axis is +Z.
    // After this rotation, +Z (object-local) is the arrow direction.
    group.rotation.x = Math.PI / 2;
    return group;
}

export class SpinArrowManager {
    constructor(scene) {
        this._scene = scene;
        // Map: particleId → {
        //   group, targetQuat, currentQuat, theta, omega,
        //   lastPos, getSpinFn, lastUpdate
        // }
        this._tracked = new Map();

        // Reusable objects for update loop to prevent GC thrashing
        this._tempVec = new THREE.Vector3();
        this._tempTarget = new THREE.Vector3();
        this._tempQuat = new THREE.Quaternion();
        this._axisLocal = new THREE.Vector3(0, 0, 1);
        this._axialQ = new THREE.Quaternion();
    }

    /**
     * Begin tracking a particle. The optional `getSpin` callback is
     * invoked each frame to read live spin/omega data; if not provided,
     * the arrow uses the initial orientation and a fixed omega.
     *
     * @param {number} particleId          - identifier (used for un-track)
     * @param {object} opts
     * @param {() => {x,y,z}} opts.getPosition - particle position lookup
     * @param {() => {sx,sy,sz,omega_z}} [opts.getSpin] - spin lookup (sx,sy,sz unit, omega_z rad/sec)
     * @param {number} [opts.omegaDefault] - fallback omega rad/sec when getSpin missing
     */
    track(particleId, opts = {}) {
        if (!this._scene) return;
        if (this._tracked.has(particleId)) {
            this.untrack(particleId);
        }
        const group = buildArrowGroup();
        this._scene.add(group);

        const initialPos = (opts.getPosition && opts.getPosition()) || { x: 0, y: 0, z: 0 };
        const containerGroup = new THREE.Group();
        containerGroup.position.set(initialPos.x, initialPos.y, initialPos.z);
        containerGroup.add(group);
        this._scene.remove(group);
        this._scene.add(containerGroup);

        const initialQuat = new THREE.Quaternion();        // identity
        this._tracked.set(particleId, {
            group: containerGroup,
            inner: group,
            targetQuat: initialQuat.clone(),
            currentQuat: initialQuat.clone(),
            theta: 0,                                      // accumulated rotation about spin axis
            omega: opts.omegaDefault || 0,                 // rad/sec
            getPosition: opts.getPosition,
            getSpin: opts.getSpin,
            lastUpdate: performance.now(),
        });
    }

    /** Stop tracking a particle and dispose its meshes. */
    untrack(particleId) {
        const t = this._tracked.get(particleId);
        if (!t) return;
        this._scene.remove(t.group);
        // Dispose all geometries / materials in the inner group + container
        const disposeAll = (obj) => {
            obj.traverse((child) => {
                if (child.geometry) child.geometry.dispose();
                if (child.material) {
                    if (Array.isArray(child.material)) child.material.forEach((m) => m.dispose());
                    else child.material.dispose();
                }
            });
        };
        disposeAll(t.group);
        this._tracked.delete(particleId);
    }

    /** Tear down all tracked arrows. */
    dispose() {
        for (const id of Array.from(this._tracked.keys())) this.untrack(id);
    }

    /**
     * Per-frame update — call from Viewport.render() after the scene has
     * been built. dtMs is the elapsed time since last update, in
     * milliseconds. If null, computed from performance.now() per arrow.
     */
    update(dtMs = null) {
        const now = performance.now();
        for (const t of this._tracked.values()) {
            const dt = dtMs ?? (now - t.lastUpdate);
            const dtSec = Math.max(0, dt / 1000);
            t.lastUpdate = now;

            // 1) Position follow with damped lerp
            if (t.getPosition) {
                const p = t.getPosition();
                if (p) {
                    this._tempVec.set(p.x, p.y, p.z);
                    t.group.position.lerp(this._tempVec, 0.3);
                }
            }

            // 2) Read spin orientation if available, else keep last target
            if (t.getSpin) {
                const s = t.getSpin();
                if (s && Number.isFinite(s.sx) && Number.isFinite(s.sy) && Number.isFinite(s.sz)) {
                    const len = Math.sqrt(s.sx * s.sx + s.sy * s.sy + s.sz * s.sz);
                    if (len > 1e-6) {
                        this._tempTarget.set(s.sx / len, s.sy / len, s.sz / len);
                        this._tempQuat.setFromUnitVectors(this._axisLocal, this._tempTarget);
                        t.targetQuat.copy(this._tempQuat);
                    }
                    if (Number.isFinite(s.omega_z)) {
                        t.omega = s.omega_z;
                    }
                }
            }

            // 3) Slerp current → target (per-frame factor 0.25 per Auditor #3)
            t.currentQuat.slerp(t.targetQuat, 0.25);

            // 4) Rotate around the spin axis at omega rad/sec
            t.theta += t.omega * dtSec;
            if (t.theta > Math.PI * 2) t.theta -= Math.PI * 2;
            if (t.theta < 0) t.theta += Math.PI * 2;

            // Compose: orientation quaternion × axial-spin quaternion
            this._axialQ.setFromAxisAngle(this._axisLocal, t.theta);
            t.inner.quaternion.copy(t.currentQuat).multiply(this._axialQ);
        }
    }

    /** Diagnostic: which particles are currently tracked. */
    trackedIds() {
        return Array.from(this._tracked.keys());
    }
}
