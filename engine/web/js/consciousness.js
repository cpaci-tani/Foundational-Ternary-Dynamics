/**
 * Reflexivity Engine (Scale 11) — holographic figure, sLoop ring, and audio
 * synthesis driven by lattice flux dynamics.
 *
 * VOCABULARY NOTE (2026-05-01): module name "consciousness" is preserved
 * here for backward compatibility (extensive import surface). The conceptual
 * content is the reflexive-projection layer per
 * docs/theory/01_reference/REF_REFLEXIVITY_VOCABULARY.md. UI labels and user-
 * facing strings now use "reflexivity" / "reflexive readout" terminology.
 * A coordinated module-rename is queued for a future refactor.
 *
 * Scale 11 in the FTD DAG hierarchy: the meta-scale where physics meets the
 * reflexive-projection layer (formerly framed as "consciousness").
 *
 * The holographic figure responds to real-time flux data:
 *   - Vertex deformation from flux spatial variance
 *   - Color shifts from flux polarity
 *   - Opacity from central flux density
 *   - Sound from flux energy / wave energy / curl
 *
 * The sLoop torus ring orbits at theta_C = 52.54 degrees — the reflexive
 * phase angle (formerly "consciousness phase angle") from the master
 * quadratic with k = 1/2.
 */

import * as THREE from 'three';
import { FIGURE_REGISTRY } from './consciousness-figure.js';
import {
    COS2_THETA_C, THETA_C_RAD, THETA_C_DEG,
} from './constants.js';
import {
    HOLO_VERT, HOLO_FRAG,
    PARTICLE_VERT, PARTICLE_FRAG,
    SLOOP_FRAG,
} from './consciousness/consciousness-shaders.js';
import {
    LemniscateAlphaCurve,
    buildAudioGraph, teardownAudioGraph, updateAudio,
} from './consciousness/consciousness-audio.js';

// ── ConsciousnessEngine ──────────────────────────────────────────────

export class ConsciousnessEngine {
    constructor(scene) {
        this._scene = scene;
        this._group = new THREE.Group();
        this._scene.add(this._group);

        this._figureMesh = null;
        this._figurePoints = null;
        this._descriptor = null;
        this._sloopRing = null;
        this._sloopPivot = null;
        this._figureType = 'plasmoid';
        this._startTime = performance.now() / 1000;

        // Audio state (scenario-aware synthesis engine)
        this._audioCtx = null;
        this._audioEnabled = false;
        this._audioProfile = null;   // current profile name
        this._masterGain = null;
        this._filter = null;
        this._shaper = null;         // WaveShaperNode for overtone enrichment
        this._convolver = null;      // ConvolverNode for spatial reverb
        this._dryGain = null;
        this._wetGain = null;
        this._oscBank = [];          // [{osc, gain, panner, ratio, freqOffset, basePan}]
        this._lfo = null;
        this._lfoGain = null;

        // Flux data (updated per frame)
        this._fluxEnergy = 0;
        this._waveEnergy = 0;
        this._variance = 0;
        this._curlMag = 0;
        this._centralDensity = 0;
        // Consciousness diagnostics (new — drives audio modulation)
        this._fluxRatio = 0;
        this._effTheta = THETA_C_DEG;
        this._consciousnessI = 0;
        this._mandelbrotZ = 0;

        this._buildFigure('plasmoid');
        this._buildSLoop();
    }

    // ── Figure Management ──

    _buildFigure(type) {
        // Dispose old
        if (this._figureMesh) {
            this._group.remove(this._figureMesh);
            this._figureMesh.geometry.dispose();
            this._figureMesh.material.dispose();
            this._figureMesh = null;
        }
        if (this._figurePoints) {
            this._group.remove(this._figurePoints);
            this._figurePoints.geometry.dispose();
            this._figurePoints.material.dispose();
            this._figurePoints = null;
        }

        // Look up builder
        const builder = FIGURE_REGISTRY.get(type) || FIGURE_REGISTRY.get('humanoid');
        const d = builder();
        this._descriptor = d;
        this._figureType = type;

        const overrides = d.shaderOverrides;

        // Build mesh if descriptor has geometry
        if (d.geometry && (d.type === 'mesh' || d.type === 'hybrid')) {
            const mat = new THREE.ShaderMaterial({
                vertexShader: HOLO_VERT,
                fragmentShader: HOLO_FRAG,
                uniforms: {
                    uTime:              { value: 0 },
                    uFluxEnergy:        { value: 0.3 },
                    uDeformAmt:         { value: 0.03 },
                    uObservableFraction:{ value: COS2_THETA_C },
                    uBaseColor:         { value: d.palette.primary.clone() },
                    uSecondaryColor:    { value: d.palette.secondary.clone() },
                    uOpacity:           { value: 0.7 },
                    uScanLineFreq:      { value: overrides.scanLineFreq },
                    uScanLineSpeed:     { value: overrides.scanLineSpeed },
                    uFresnelPower:      { value: overrides.fresnelPower },
                    uGlitchScale:       { value: overrides.glitchScale },
                    uIridescenceStr:    { value: overrides.iridescenceStr },
                },
                transparent: true,
                side: THREE.DoubleSide,
                depthWrite: false,
                blending: THREE.AdditiveBlending,
            });

            this._figureMesh = new THREE.Mesh(d.geometry, mat);
            this._figureMesh.scale.setScalar(d.scale);
            if (d.yOffset) this._figureMesh.position.y = d.yOffset;
            this._group.add(this._figureMesh);
        }

        // Build particles if descriptor has them
        if (d.particlePositions && d.particleCount > 0 && (d.type === 'points' || d.type === 'hybrid')) {
            const geom = new THREE.BufferGeometry();
            geom.setAttribute('position', new THREE.BufferAttribute(d.particlePositions, 3));
            geom.setAttribute('aSize', new THREE.BufferAttribute(d.particleSizes, 1));

            const pMat = new THREE.ShaderMaterial({
                vertexShader: PARTICLE_VERT,
                fragmentShader: PARTICLE_FRAG,
                uniforms: {
                    uTime:          { value: 0 },
                    uFluxEnergy:    { value: 0.3 },
                    uDeformAmt:     { value: 0.02 },
                    uBaseColor:     { value: d.palette.primary.clone() },
                    uSecondaryColor:{ value: d.palette.secondary.clone() },
                    uOpacity:       { value: 0.8 },
                    uScanLineFreq:  { value: overrides.scanLineFreq },
                    uScanLineSpeed: { value: overrides.scanLineSpeed },
                },
                transparent: true,
                depthWrite: false,
                blending: THREE.AdditiveBlending,
            });

            this._figurePoints = new THREE.Points(geom, pMat);
            this._figurePoints.scale.setScalar(d.scale);
            if (d.yOffset) this._figurePoints.position.y = d.yOffset;
            this._group.add(this._figurePoints);
        }
    }

    setFigureType(type) {
        if (type === this._figureType) return;
        this._buildFigure(type);
    }

    // ── sLoop Ring ──

    _buildSLoop() {
        this._sloopPivot = new THREE.Group();

        const curve = new LemniscateAlphaCurve(0.7);
        const ringGeom = new THREE.TubeGeometry(curve, 300, 0.05, 12, true);

        const ringMat = new THREE.ShaderMaterial({
            vertexShader: HOLO_VERT,
            fragmentShader: SLOOP_FRAG,
            uniforms: {
                uTime:      { value: 0 },
                uDeformAmt: { value: 0.005 },
                uFluxEnergy:{ value: 0.5 },
                uPulse:     { value: 0 },
            },
            transparent: true,
            side: THREE.DoubleSide,
            depthWrite: false,
            blending: THREE.AdditiveBlending,
        });

        this._sloopRing = new THREE.Mesh(ringGeom, ringMat);

        this._sloopPivot.rotation.x = Math.PI / 2 - THETA_C_RAD;
        this._sloopPivot.add(this._sloopRing);
        this._group.add(this._sloopPivot);
    }

    // ── Audio (Scenario-Aware Synthesis) ──

    enableAudio(profileName = 'cs-custom') {
        // If same profile already running, just resume
        if (this._audioCtx && this._audioProfile === profileName) {
            this._audioCtx.resume();
            this._audioEnabled = true;
            return;
        }

        // Create AudioContext if needed
        if (!this._audioCtx) {
            try {
                this._audioCtx = new (window.AudioContext || window.webkitAudioContext)();
            } catch (e) {
                console.warn('[CS] Web Audio not available:', e);
                return;
            }
        }
        this._audioCtx.resume();

        // Tear down old graph, build new one for this scenario
        teardownAudioGraph(this);
        buildAudioGraph(this, profileName);
        this._audioProfile = profileName;
        this._audioEnabled = true;
    }

    disableAudio() {
        teardownAudioGraph(this);
        // CS-H1 audit pass 2: fully close the AudioContext rather than
        // just suspending. Suspended contexts are cheap but they keep
        // the audio device pinned and the WebAudio thread alive; on
        // long sessions or repeated enable/disable cycles, a fresh
        // close-then-recreate is cleaner. enableAudio() is responsible
        // for re-allocating the context if it's null.
        const ctx = this._audioCtx;
        this._audioCtx = null;
        this._audioEnabled = false;
        if (ctx && ctx.state !== 'closed') {
            // Promise; ignore — close() returns void-async, we don't need to await
            ctx.close().catch(() => { /* defensive: ignore close errors */ });
        }
    }

    // ── Per-Frame Particle Animation ──

    _animateParticles(elapsed, dt) {
        if (!this._figurePoints || !this._descriptor) return;
        const d = this._descriptor;
        const posAttr = this._figurePoints.geometry.getAttribute('position');
        const sizeAttr = this._figurePoints.geometry.getAttribute('aSize');
        if (!posAttr) return;
        const pos = posAttr.array;
        const count = d.particleCount;

        switch (d.animationFn) {
            case 'plasmoid': {
                // Swirl along torus path
                const angles = d._torusAngles;
                const R = d._torusR || 2.0;
                const r = d._torusr || 1.0;
                if (!angles) break;
                const speed = 0.3 + this._fluxEnergy * 0.5;
                for (let i = 0; i < count; i++) {
                    angles[i*2] += speed * dt; // advance toroidal angle
                    const u = angles[i*2];
                    const v = angles[i*2+1];
                    const rr = r + Math.sin(u * 3 + elapsed) * 0.15;
                    pos[i*3]   = (R + rr * Math.cos(v)) * Math.cos(u);
                    pos[i*3+1] = rr * Math.sin(v);
                    pos[i*3+2] = (R + rr * Math.cos(v)) * Math.sin(u);
                }
                posAttr.needsUpdate = true;
                break;
            }

            case 'spirit': {
                // Dissolution particles drift downward, respawn at waist
                const driftSpeed = 0.8 + this._fluxEnergy * 0.5;
                for (let i = 0; i < count; i++) {
                    pos[i*3+1] -= driftSpeed * dt;
                    // Horizontal jitter
                    pos[i*3]   += (Math.random() - 0.5) * 0.02;
                    pos[i*3+2] += (Math.random() - 0.5) * 0.02;
                    // Respawn at top
                    if (pos[i*3+1] < -3.5) {
                        const angle = Math.random() * Math.PI * 2;
                        const rr = Math.random() * 0.3;
                        pos[i*3]   = rr * Math.cos(angle);
                        pos[i*3+1] = 0.5;
                        pos[i*3+2] = rr * Math.sin(angle);
                    }
                }
                posAttr.needsUpdate = true;
                break;
            }

            case 'yahweh': {
                // Fire particles drift upward, flicker size
                const upSpeed = 1.2 + this._fluxEnergy * 0.8;
                const sizes = sizeAttr.array;
                for (let i = 0; i < count; i++) {
                    pos[i*3+1] += upSpeed * dt;
                    // Slight horizontal wobble
                    pos[i*3]   += Math.sin(elapsed * 5 + i * 0.1) * 0.01;
                    pos[i*3+2] += Math.cos(elapsed * 4 + i * 0.13) * 0.01;
                    // Respawn at bottom
                    if (pos[i*3+1] > 5.0) {
                        const angle = Math.random() * Math.PI * 2;
                        const rr = Math.random() * 0.8;
                        pos[i*3]   = rr * Math.cos(angle);
                        pos[i*3+1] = -2.0;
                        pos[i*3+2] = rr * Math.sin(angle);
                    }
                    // Flicker size
                    sizes[i] = 0.06 + 0.12 * Math.abs(Math.sin(elapsed * 5 + i * 0.37));
                }
                posAttr.needsUpdate = true;
                sizeAttr.needsUpdate = true;
                break;
            }

            case 'death-cloud': {
                // Vortex rotation + outward drift
                const rotSpeed = 0.5 + this._fluxEnergy * 0.3;
                const expandSpeed = 0.3;
                for (let i = 0; i < count; i++) {
                    const x = pos[i*3];
                    const z = pos[i*3+2];
                    const r = Math.sqrt(x*x + z*z) || 0.01;
                    const angle = Math.atan2(z, x) + rotSpeed * dt;
                    const newR = r + expandSpeed * dt * 0.1;
                    pos[i*3]   = newR * Math.cos(angle);
                    pos[i*3+2] = newR * Math.sin(angle);
                    // Slight downward drift
                    pos[i*3+1] -= 0.1 * dt;
                    // Respawn when too far
                    if (newR > 3.5 || pos[i*3+1] < -4.0) {
                        const a = Math.random() * Math.PI * 2;
                        const rr = Math.random() * 0.3;
                        pos[i*3]   = rr * Math.cos(a);
                        pos[i*3+1] = 2.5 + Math.random() * 0.5;
                        pos[i*3+2] = rr * Math.sin(a);
                    }
                }
                posAttr.needsUpdate = true;
                break;
            }

            case 'mayan-sun': {
                // Particles drift outward along rays, respawn at center
                const driftSpeed = 0.6 + this._fluxEnergy * 0.4;
                const rayAngles = d._rayAngles;
                const rayDists  = d._rayDists;
                if (!rayAngles || !rayDists) break;
                for (let i = 0; i < count; i++) {
                    rayDists[i] += driftSpeed * dt;
                    if (rayDists[i] > 3.2) {
                        rayDists[i] = 1.2;
                    }
                    pos[i*3]   = rayDists[i] * Math.cos(rayAngles[i]);
                    pos[i*3+1] = rayDists[i] * Math.sin(rayAngles[i]);
                    pos[i*3+2] = (Math.random() - 0.5) * 0.1;
                }
                posAttr.needsUpdate = true;
                break;
            }

            case 'demiurge': {
                // Orbiting particles rotate around Y
                const orbSpeed = 0.3 + this._fluxEnergy * 0.2;
                for (let i = 0; i < count; i++) {
                    const x = pos[i*3];
                    const z = pos[i*3+2];
                    const angle = Math.atan2(z, x) + orbSpeed * dt;
                    const r = Math.sqrt(x*x + z*z) || 1.5;
                    pos[i*3]   = r * Math.cos(angle);
                    pos[i*3+2] = r * Math.sin(angle);
                    // Gentle vertical oscillation
                    pos[i*3+1] += Math.sin(elapsed * 2 + i * 0.1) * 0.003;
                }
                posAttr.needsUpdate = true;
                break;
            }
        }
    }

    // ── Per-Frame Update ──

    update(opts = {}) {
        const now = performance.now() / 1000;
        const elapsed = now - this._startTime;
        const dt = Math.min(1/30, now - (this._lastTime || now));
        this._lastTime = now;

        this._fluxEnergy = opts.fluxEnergy ?? 0.3;
        this._waveEnergy = opts.waveEnergy ?? 0.2;
        this._variance = opts.variance ?? 0.1;
        this._curlMag = opts.curlMag ?? 0.05;
        this._centralDensity = opts.centralDensity ?? 0.3;
        // Consciousness diagnostics (drive audio modulation)
        this._fluxRatio = opts.fluxRatio ?? 0;
        this._effTheta = opts.effTheta ?? THETA_C_DEG;
        this._consciousnessI = opts.consciousnessI ?? 0;
        this._mandelbrotZ = opts.mandelbrotZ ?? 0;
        const polarity = opts.polarity ?? 0;

        const d = this._descriptor;

        // ── Update figure mesh shader uniforms ──
        if (this._figureMesh && d) {
            const u = this._figureMesh.material.uniforms;
            u.uTime.value = elapsed;
            u.uFluxEnergy.value = this._fluxEnergy;
            u.uDeformAmt.value = 0.01 + this._variance * 0.12;
            u.uOpacity.value = 0.4 + this._centralDensity * 0.5;

            // Color shifts with polarity, using figure's palette
            const mix = (polarity + 1) / 2;
            u.uBaseColor.value.copy(d.palette.primary).lerp(d.palette.secondary, 1 - mix);
            u.uSecondaryColor.value.copy(d.palette.secondary).lerp(d.palette.primary, 1 - mix);

            // Slow idle rotation
            this._figureMesh.rotation.y = elapsed * 0.15;
        }

        // ── Update figure particles ──
        if (this._figurePoints && d) {
            const pu = this._figurePoints.material.uniforms;
            pu.uTime.value = elapsed;
            pu.uFluxEnergy.value = this._fluxEnergy;
            pu.uDeformAmt.value = 0.005 + this._variance * 0.05;
            pu.uOpacity.value = 0.5 + this._centralDensity * 0.4;

            const mix = (polarity + 1) / 2;
            pu.uBaseColor.value.copy(d.palette.primary).lerp(d.palette.secondary, 1 - mix);
            pu.uSecondaryColor.value.copy(d.palette.secondary).lerp(d.palette.primary, 1 - mix);

            // Rotate particles with mesh (except pure point figures)
            if (d.type === 'hybrid') {
                this._figurePoints.rotation.y = elapsed * 0.15;
            } else {
                this._figurePoints.rotation.y = elapsed * 0.08; // slower for pure clouds
            }
        }

        // ── Animate particles ──
        this._animateParticles(elapsed, dt);

        // ── Update sLoop ring ──
        if (this._sloopPivot) {
            const orbitPeriod = 2 * Math.PI / THETA_C_RAD;
            this._sloopPivot.rotation.y = (elapsed / orbitPeriod) * 2 * Math.PI;

            const ru = this._sloopRing.material.uniforms;
            ru.uTime.value = elapsed;
            ru.uFluxEnergy.value = this._fluxEnergy;
            ru.uPulse.value = 0.5 + 0.5 * Math.sin(elapsed * 2 * Math.PI * this._centralDensity);
        }

        // ── Update audio ──
        updateAudio(this);
    }

    // ── Cleanup ──

    dispose() {
        // Stop audio
        teardownAudioGraph(this);
        if (this._audioCtx) {
            this._audioCtx.close();
            this._audioCtx = null;
        }

        // Remove mesh
        if (this._figureMesh) {
            this._group.remove(this._figureMesh);
            this._figureMesh.geometry.dispose();
            this._figureMesh.material.dispose();
        }
        // Remove particles
        if (this._figurePoints) {
            this._group.remove(this._figurePoints);
            this._figurePoints.geometry.dispose();
            this._figurePoints.material.dispose();
        }
        // Remove sLoop
        if (this._sloopRing) {
            this._sloopPivot.remove(this._sloopRing);
            this._sloopRing.geometry.dispose();
            this._sloopRing.material.dispose();
        }
        if (this._sloopPivot) {
            this._group.remove(this._sloopPivot);
        }
        this._scene.remove(this._group);
    }
}
