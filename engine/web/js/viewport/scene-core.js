/**
 * @file engine/web/js/viewport/scene-core.js
 * @purpose Owns scene-level rendering infrastructure for the Scale-0
 *          dashboard: boundary wireframe, axis indicators, post-processing
 *          pipeline (bloom), camera presets, render-loop dispatch, resize
 *          handling. One of 4 sub-renderers extracted from the
 *          monolithic Viewport class in Phase 3 of the refactor sweep.
 *          Note: scene/camera/renderer/controls THEMSELVES are owned by
 *          the orchestrator (Viewport) so every sub-renderer can access
 *          them; SceneCore owns the SCENE-DECORATION objects (wireframe,
 *          axes) and the rendering pipeline.
 * @consumers engine/web/js/viewport.js (composes this via constructor)
 * @contract CONTRACTS.md §2 (Capability Factory Contract)
 * @related ./flux-renderer.js (3b, settled),
 *          ./particle-renderer.js (3d, settled),
 *          ./field-renderer.js (3c, future), ./REFACTOR_MAP.md
 *
 * Phase 3a of the refactor sweep. setLatticeSize REMAINS on Viewport
 * orchestrator — it dispatches to every sub-renderer's
 * onLatticeSizeChanged. SceneCore's onLatticeSizeChanged rebuilds
 * the boundary wireframe + axes for the new lattice size.
 */

import * as THREE from 'three';
import { EffectComposer } from 'three/addons/postprocessing/EffectComposer.js';
import { RenderPass } from 'three/addons/postprocessing/RenderPass.js';
import { UnrealBloomPass } from 'three/addons/postprocessing/UnrealBloomPass.js';
import { buildBoundary } from './boundary-geometry.js';
import {
    GLOBAL_CLOCK_PHASES,
    GlobalClockHoverController,
    tagClockHover,
} from './clock-hover.js?v=1';

const CLOCK_COLOR_FREE = new THREE.Color(0x38bdf8);
const CLOCK_COLOR_LOADED = new THREE.Color(0xfbbf24);
const CLOCK_COLOR_LIMIT = new THREE.Color(0xfb7185);
const DEFAULT_FRONT_DISTANCE = 2.2;

const clamp01 = value => Math.max(0, Math.min(1, Number(value) || 0));
const clockNow = () => globalThis.performance?.now?.() ?? Date.now();

function mappedClockColor(rate) {
    const r = clamp01(rate);
    if (r >= 0.5) {
        return CLOCK_COLOR_LOADED.clone().lerp(CLOCK_COLOR_FREE, (r - 0.5) * 2);
    }
    return CLOCK_COLOR_LIMIT.clone().lerp(CLOCK_COLOR_LOADED, r * 2);
}

export class ViewportSceneCore {
    constructor({
        scene,
        camera,
        renderer,
        controls,
        container,
        latticeSize,
        halfN,
        boundaryShape = 'cube',
        boundaryMode = 'lattice',
        engineMode = 'lattice',
        insideBoundary,
    }) {
        this._scene = scene;
        this._camera = camera;
        this._renderer = renderer;
        this._controls = controls;
        this._container = container;
        this._latticeSize = latticeSize;
        this._halfN = halfN;
        this._boundaryShape = boundaryShape;
        this._boundaryMode = boundaryMode;
        this._engineMode = engineMode;
        this._insideBoundary = insideBoundary;

        // Wireframe / boundary state
        this.wireframe = null;
        this.showWireframe = true;
        this._wireframeBrightness = 0.18;

        // Axis state
        this.axes = null;
        this.peAxes = null;
        this.peGrid = null;
        this._showAxes = true;
        this._showGrid = true;
        this._boundaryDynamicsMode = 2;
        this._periodicAxis = 2;
        this._showBoundaryOrientation = true;
        this._showGlobalClock = true;
        this.boundaryOrientation = null;
        this._orientationArrows = [];
        this.globalClock = null;
        this._globalClockHand = null;
        this._globalClockHub = null;
        this._globalClockRateRing = null;
        this._globalClockPhaseCursor = null;
        this._globalClockForwardArrow = null;
        this._globalClockPhaseSegments = [];
        this._globalTick = 0;
        this._globalClockRunning = false;
        this._globalClockHasCausalBudget = false;
        this._globalClockCausalBudget = 0;
        this._globalClockRate = 1;
        this._globalClockProjectionEvents = 0;
        this._globalClockLastTickAt = null;
        this._globalClockPulseStartedAt = Number.NEGATIVE_INFINITY;
        this._globalClockProjectionStartedAt = Number.NEGATIVE_INFINITY;
        this._globalClockPulseDurationMs = 360;
        this._globalClockRateColor = CLOCK_COLOR_FREE.clone();
        this._globalClockActiveReplayPhase = -1;
        this._globalClockHover = null;

        // Inspector highlight overlays
        this._voxelHighlight = null;
        this._areaHighlight = null;
        this._areaHighlightRadius = null;

        // Post-processing (lazy init; public-API hook for any mode that opts into bloom).
        this._composer = null;
        this._bloomPass = null;
        this._usePostProcessing = false;

        // Initial scene decoration
        this._buildBoundary(this._boundaryShape, this._boundaryMode);
        this._buildAxes();
        this._buildBoundaryOrientation();
        this._buildGlobalClock();
        this.setCameraPreset('front');
        this._globalClockHover = new GlobalClockHoverController({
            renderer: this._renderer,
            camera: this._camera,
            container: this._container,
            getClock: () => this.globalClock,
            getState: () => ({
                tick: this._globalTick,
                running: this._globalClockRunning,
                hasCausalBudget: this._globalClockHasCausalBudget,
                causalBudget: this._globalClockCausalBudget,
                rate: this._globalClockRate,
                projectionEvents: this._globalClockProjectionEvents,
                activeReplayPhase: this._globalClockActiveReplayPhase,
            }),
        }).init();
    }

    // ── Boundary system ────────────────────────────────────────────────

    _disposeBoundary() {
        if (this.wireframe) {
            this._scene.remove(this.wireframe);
            this.wireframe.traverse(child => {
                if (child.geometry) child.geometry.dispose();
                if (child.material) child.material.dispose();
            });
            this.wireframe = null;
        }
    }

    _buildBoundary(shape, mode) {
        this._disposeBoundary();
        this._boundaryShape = shape;
        this._boundaryMode = mode;

        if (shape === 'none') return;

        const mat = new THREE.LineBasicMaterial({
            color: 0x1e2d44, transparent: true, opacity: this._wireframeBrightness,
            depthWrite: false,
        });

        const group = buildBoundary(shape, mode, { latticeSize: this._latticeSize }, mat);

        // Scale and position based on mode
        // Non-cube shapes are inscribed within the lattice cube (radius = s/2)
        // so the flux volume clips to the shape boundary
        if (mode === 'lattice') {
            const s = this._latticeSize;
            if (shape === 'cube') {
                // Cube is already built at lattice coords — no transform needed
            } else {
                const center = s / 2;
                group.scale.setScalar(s / 2);
                group.position.set(center, center, center);
            }
        } else {
            // origin mode (PE/AE/molecules)
            const radius = 35;
            if (shape === 'cube') {
                group.scale.setScalar(radius / (this._latticeSize / 2));
                group.position.set(0, 0, 0);
            } else {
                group.scale.setScalar(radius);
                group.position.set(0, 0, 0);
            }
        }

        this.wireframe = group;
        this.wireframe.visible = this.showWireframe;
        this._scene.add(this.wireframe);
    }

    setBoundaryShape(shape) {
        this._buildBoundary(shape, this._boundaryMode);
    }

    setBoundaryMode(mode) {
        this._buildBoundary(this._boundaryShape, mode);
    }

    setEngineMode(mode) {
        this._engineMode = mode;
        const latticeVisible = mode === 'lattice';
        if (!latticeVisible) this._globalClockHover?.hide();
        if (this.boundaryOrientation) {
            this.boundaryOrientation.visible = latticeVisible && this._showBoundaryOrientation;
        }
        if (this.globalClock) {
            this.globalClock.visible = latticeVisible && this._showGlobalClock;
        }
    }

    _buildAxes() {
        // Axis indicator at origin — length scales with lattice size
        const axisLen = Math.max(3, this._latticeSize * 0.1);
        const axisGeo = new THREE.BufferGeometry();
        axisGeo.setAttribute('position', new THREE.Float32BufferAttribute([
            0, 0, 0, axisLen, 0, 0,  // X
            0, 0, 0, 0, axisLen, 0,  // Y
            0, 0, 0, 0, 0, axisLen,  // Z
        ], 3));
        axisGeo.setAttribute('color', new THREE.Float32BufferAttribute([
            0.9, 0.3, 0.3, 0.9, 0.3, 0.3,  // X = red
            0.3, 0.9, 0.3, 0.3, 0.9, 0.3,  // Y = green
            0.3, 0.3, 0.9, 0.3, 0.3, 0.9,  // Z = blue
        ], 3));
        const axisMat = new THREE.LineBasicMaterial({ vertexColors: true, transparent: true, opacity: 0.5 });
        this.axes = new THREE.LineSegments(axisGeo, axisMat);
        this.axes.visible = this._showAxes;
        this._scene.add(this.axes);
    }

    _disposeDecorationGroup(group) {
        if (!group) return;
        this._scene.remove(group);
        group.traverse(child => {
            if (child.geometry) child.geometry.dispose();
            if (child.material) child.material.dispose();
        });
    }

    _buildBoundaryOrientation() {
        this._disposeDecorationGroup(this.boundaryOrientation);
        this._orientationArrows = [];
        const N = this._latticeSize;
        const center = N / 2;
        const gap = Math.max(1.4, N * 0.055);
        const length = Math.max(2.5, N * 0.14);
        const headLength = Math.max(0.7, length * 0.28);
        const headWidth = Math.max(0.38, length * 0.15);
        const group = new THREE.Group();
        group.name = 'scale0-boundary-orientation';
        const axes = [
            { axis: 0, color: 0xff5a67, dir: new THREE.Vector3(1, 0, 0) },
            { axis: 1, color: 0x54d675, dir: new THREE.Vector3(0, 1, 0) },
            { axis: 2, color: 0x5d8dff, dir: new THREE.Vector3(0, 0, 1) },
        ];
        for (const spec of axes) {
            for (const sign of [-1, 1]) {
                const dir = spec.dir.clone().multiplyScalar(sign);
                const origin = new THREE.Vector3(center, center, center);
                origin.setComponent(spec.axis, sign < 0 ? -gap : N + gap);
                const arrow = new THREE.ArrowHelper(
                    dir, origin, length, spec.color, headLength, headWidth,
                );
                arrow.userData.boundaryAxis = spec.axis;
                arrow.userData.baseColor = spec.color;
                group.add(arrow);
                this._orientationArrows.push(arrow);
            }
        }
        group.visible = this._engineMode === 'lattice' && this._showBoundaryOrientation;
        this.boundaryOrientation = group;
        this._scene.add(group);
        this._refreshBoundaryOrientation();
    }

    _refreshBoundaryOrientation() {
        const selected = this._periodicAxis;
        for (const arrow of this._orientationArrows) {
            const active = selected === 3 || selected === arrow.userData.boundaryAxis;
            arrow.scale.setScalar(active ? 1.22 : 0.82);
            arrow.setColor(new THREE.Color(active ? 0xffd166 : arrow.userData.baseColor));
            arrow.line.material.transparent = true;
            arrow.cone.material.transparent = true;
            arrow.line.material.opacity = active ? 1 : 0.48;
            arrow.cone.material.opacity = active ? 1 : 0.48;
        }
    }

    setBoundaryDynamics(mode, periodicAxis = this._periodicAxis) {
        this._boundaryDynamicsMode = Math.max(0, Math.min(2, Math.trunc(Number(mode) || 0)));
        this._periodicAxis = Math.max(0, Math.min(3, Math.trunc(Number(periodicAxis) || 0)));
        this._refreshBoundaryOrientation();
    }

    toggleBoundaryOrientation(on) {
        this._showBoundaryOrientation = Boolean(on);
        if (this.boundaryOrientation) {
            this.boundaryOrientation.visible = this._engineMode === 'lattice'
                && this._showBoundaryOrientation;
        }
    }

    _buildGlobalClock() {
        this._disposeDecorationGroup(this.globalClock);
        this._globalClockPhaseSegments = [];
        this._globalClockHub = null;
        this._globalClockRateRing = null;
        this._globalClockPhaseCursor = null;
        this._globalClockForwardArrow = null;
        const N = this._latticeSize;
        const radius = Math.max(1.7, N * 0.075);
        const group = new THREE.Group();
        group.name = 'scale0-global-ordinal-clock';
        tagClockHover(group, 'clock');
        group.position.set(N * 0.82, N + Math.max(3.2, N * 0.16), N * 0.82);
        group.userData.clockModel = 'global-ordinal-plus-selected-causal-budget';
        group.userData.phaseOrder = GLOBAL_CLOCK_PHASES.map(phase => phase.name);
        group.userData.c4Reference = {
            productionTelemetry: false,
            status: 'conditional-open',
        };

        const backdrop = new THREE.Mesh(
            new THREE.CircleGeometry(radius * 0.79, 48),
            new THREE.MeshBasicMaterial({
                color: 0x061321, transparent: true, opacity: 0.58,
                side: THREE.DoubleSide, depthWrite: false,
            }),
        );
        backdrop.name = 'scale0-clock-backdrop';
        backdrop.position.z = -0.03;
        tagClockHover(backdrop, 'clock');
        group.add(backdrop);

        const phaseStep = Math.PI * 2 / GLOBAL_CLOCK_PHASES.length;
        const phaseGap = phaseStep * 0.11;
        GLOBAL_CLOCK_PHASES.forEach((phase, index) => {
            // RingGeometry winds counter-clockwise. Starting each wedge one
            // step below twelve o'clock lays the indexed phase order clockwise.
            const thetaStart = Math.PI / 2 - (index + 1) * phaseStep + phaseGap / 2;
            const segment = new THREE.Mesh(
                new THREE.RingGeometry(
                    radius * 0.82, radius, 10, 1,
                    thetaStart, phaseStep - phaseGap,
                ),
                new THREE.MeshBasicMaterial({
                    color: phase.color,
                    transparent: true,
                    opacity: 0.34,
                    side: THREE.DoubleSide,
                    depthWrite: false,
                }),
            );
            segment.name = `scale0-clock-phase-${index}-${phase.name.replace('/', '-')}`;
            segment.userData.phaseIndex = index;
            segment.userData.phaseName = phase.name;
            tagClockHover(segment, `phase-${index}`);
            group.add(segment);
            this._globalClockPhaseSegments.push(segment);
        });

        const rateRing = new THREE.Mesh(
            new THREE.RingGeometry(radius * 0.69, radius * 0.76, 48),
            new THREE.MeshBasicMaterial({
                color: CLOCK_COLOR_FREE,
                transparent: true,
                opacity: 0.82,
                side: THREE.DoubleSide,
                depthWrite: false,
            }),
        );
        rateRing.name = 'scale0-clock-causal-rate-ring';
        tagClockHover(rateRing, 'rate');
        group.add(rateRing);
        this._globalClockRateRing = rateRing;

        const tickGeometry = new THREE.BufferGeometry();
        const tickVertices = [];
        for (let i = 0; i < 10; i++) {
            const a = (i / 10) * Math.PI * 2;
            const r0 = radius * 0.66;
            const r1 = radius * 0.79;
            tickVertices.push(
                Math.sin(a) * r0, Math.cos(a) * r0, 0.02,
                Math.sin(a) * r1, Math.cos(a) * r1, 0.02,
            );
        }
        tickGeometry.setAttribute('position', new THREE.Float32BufferAttribute(tickVertices, 3));
        const tickMarks = new THREE.LineSegments(
            tickGeometry,
            new THREE.LineBasicMaterial({ color: 0xe0f2fe, transparent: true, opacity: 0.8 }),
        );
        tickMarks.name = 'scale0-clock-transaction-dial';
        tagClockHover(tickMarks, 'dial');
        group.add(tickMarks);

        const handGeometry = new THREE.BufferGeometry();
        handGeometry.setAttribute('position', new THREE.Float32BufferAttribute([
            0, 0, 0.04, 0, radius * 0.68, 0.04,
        ], 3));
        const hand = new THREE.Line(
            handGeometry,
            new THREE.LineBasicMaterial({
                color: 0xfbbf24, transparent: true, opacity: 0.72,
            }),
        );
        hand.name = 'scale0-clock-ordinal-hand';
        tagClockHover(hand, 'hand');
        group.add(hand);
        this._globalClockHand = hand;

        const hub = new THREE.Mesh(
            new THREE.CircleGeometry(radius * 0.09, 20),
            new THREE.MeshBasicMaterial({
                color: CLOCK_COLOR_FREE,
                transparent: true,
                opacity: 0.95,
                side: THREE.DoubleSide,
                depthWrite: false,
            }),
        );
        hub.name = 'scale0-clock-causal-rate-hub';
        tagClockHover(hub, 'rate');
        hub.position.z = 0.06;
        group.add(hub);
        this._globalClockHub = hub;

        // This small cursor makes one clockwise circuit as the renderer replays
        // the ten ordered stages of the just-completed transaction.
        const cursorPivot = new THREE.Group();
        cursorPivot.name = 'scale0-clock-transaction-cursor';
        const cursor = new THREE.Mesh(
            new THREE.CircleGeometry(radius * 0.055, 16),
            new THREE.MeshBasicMaterial({
                color: 0xffffff,
                transparent: true,
                opacity: 0,
                side: THREE.DoubleSide,
                depthWrite: false,
            }),
        );
        tagClockHover(cursor, 'cursor');
        cursor.position.set(0, radius * 0.91, 0.08);
        cursorPivot.add(cursor);
        group.add(cursorPivot);
        this._globalClockPhaseCursor = cursorPivot;

        // Clockwise is the adopted update direction.  This is justified by the
        // selected non-injective expiry sector, not by the reversible wave map.
        const arrowGeometry = new THREE.BufferGeometry();
        arrowGeometry.setAttribute('position', new THREE.Float32BufferAttribute([
            radius * 0.18, radius * 1.06, 0.07,
            -radius * 0.08, radius * 0.94, 0.07,
            -radius * 0.08, radius * 1.18, 0.07,
        ], 3));
        const forwardArrow = new THREE.Mesh(
            arrowGeometry,
            new THREE.MeshBasicMaterial({
                color: 0xfbbf24,
                transparent: true,
                opacity: 0.78,
                side: THREE.DoubleSide,
                depthWrite: false,
            }),
        );
        forwardArrow.name = 'scale0-clock-forward-update-arrow';
        tagClockHover(forwardArrow, 'arrow');
        group.add(forwardArrow);
        this._globalClockForwardArrow = forwardArrow;

        // A muted C4 marker records the conditional quartic-carrier programme.
        // It is intentionally static and explicitly tagged as non-production:
        // no current Scale-0 telemetry establishes a native G* clock.
        const c4 = new THREE.Group();
        c4.name = 'scale0-clock-c4-theory-reference';
        tagClockHover(c4, 'c4');
        c4.userData.productionTelemetry = false;
        c4.userData.status = 'conditional-open';
        const c4Radius = radius * 0.29;
        for (let index = 0; index < 4; index++) {
            const angle = index * Math.PI / 2;
            const node = new THREE.Mesh(
                new THREE.CircleGeometry(radius * 0.035, 12),
                new THREE.MeshBasicMaterial({
                    color: 0xc084fc,
                    transparent: true,
                    opacity: 0.28,
                    side: THREE.DoubleSide,
                    depthWrite: false,
                }),
            );
            node.position.set(Math.sin(angle) * c4Radius, Math.cos(angle) * c4Radius, 0.03);
            c4.add(node);
        }
        group.add(c4);

        // This is a scene instrument, not a lattice occupant.  Keep its face
        // readable when a dense flux volume happens to cross the same screen
        // pixels, while retaining world-space positioning and camera orbit.
        group.traverse(child => {
            if (!child.material) return;
            child.material.depthTest = false;
            child.renderOrder = 30;
        });

        group.visible = this._engineMode === 'lattice' && this._showGlobalClock;
        this.globalClock = group;
        this._scene.add(group);
        this._applyGlobalClockState();
    }

    toggleGlobalClock(on) {
        this._showGlobalClock = Boolean(on);
        if (!this._showGlobalClock) this._globalClockHover?.hide();
        if (this.globalClock) {
            this.globalClock.visible = this._engineMode === 'lattice' && this._showGlobalClock;
        }
    }

    setGlobalClockTick(tick) {
        this.setGlobalClockState({ tick });
    }

    setGlobalClockState({
        tick = this._globalTick,
        running = this._globalClockRunning,
        maxCausalBudget = this._globalClockHasCausalBudget
            ? this._globalClockCausalBudget : undefined,
        causalProjectionEvents = this._globalClockProjectionEvents,
    } = {}) {
        const next = Number.isFinite(Number(tick))
            ? Math.max(0, Math.trunc(Number(tick))) : 0;
        const previousTick = this._globalTick;
        const previousProjectionEvents = this._globalClockProjectionEvents;
        const now = clockNow();

        if (next !== previousTick) {
            if (this._globalClockLastTickAt !== null) {
                const ticksElapsed = Math.max(1, Math.abs(next - previousTick));
                const observedMs = (now - this._globalClockLastTickAt) / ticksElapsed;
                this._globalClockPulseDurationMs = Math.max(140, Math.min(900, observedMs * 0.92));
            }
            this._globalClockLastTickAt = now;
            this._globalClockPulseStartedAt = now;
        }

        this._globalTick = next;
        this._globalClockRunning = Boolean(running);
        this._globalClockHasCausalBudget = maxCausalBudget !== null
            && maxCausalBudget !== undefined
            && Number.isFinite(Number(maxCausalBudget));
        this._globalClockCausalBudget = this._globalClockHasCausalBudget
            ? Math.max(0, Number(maxCausalBudget)) : 0;
        this._globalClockRate = this._globalClockHasCausalBudget
            ? Math.sqrt(Math.max(0, 1 - this._globalClockCausalBudget)) : 1;
        this._globalClockRateColor = mappedClockColor(this._globalClockRate);
        this._globalClockProjectionEvents = Number.isFinite(Number(causalProjectionEvents))
            ? Math.max(0, Math.trunc(Number(causalProjectionEvents))) : 0;
        if (this._globalClockProjectionEvents > 0
            && (next !== previousTick
                || this._globalClockProjectionEvents !== previousProjectionEvents)) {
            this._globalClockProjectionStartedAt = now;
        }

        this._applyGlobalClockState();
    }

    _applyGlobalClockState() {
        if (this._globalClockHand) {
            // The hand is a base-ten ordinal odometer. The separate white cursor
            // replays phase order; neither is a live sub-phase probe.
            this._globalClockHand.rotation.z = -(this._globalTick % 10) * Math.PI * 2 / 10;
            this._globalClockHand.material.opacity = this._globalClockRunning ? 0.95 : 0.64;
        }
        if (this._globalClockRateRing) {
            this._globalClockRateRing.material.color.copy(this._globalClockRateColor);
            this._globalClockRateRing.material.opacity = this._globalClockHasCausalBudget ? 0.88 : 0.48;
        }
        if (this._globalClockHub) {
            this._globalClockHub.material.color.copy(this._globalClockRateColor);
        }
        if (this.globalClock) {
            this.globalClock.userData.current = {
                tick: this._globalTick,
                running: this._globalClockRunning,
                maxCausalBudget: this._globalClockHasCausalBudget
                    ? this._globalClockCausalBudget : null,
                mappedMinClockRate: this._globalClockHasCausalBudget
                    ? this._globalClockRate : null,
                causalProjectionEvents: this._globalClockProjectionEvents,
            };
        }

        const readout = typeof document !== 'undefined'
            ? document.getElementById('global-clock-readout') : null;
        if (readout) {
            const rateText = this._globalClockHasCausalBudget
                ? ` · τ′min ${this._globalClockRate.toFixed(3)}` : '';
            readout.textContent = `tick ${this._globalTick}${rateText}`;
            readout.dataset.clockState = this._globalClockRunning ? 'running' : 'idle';
            readout.dataset.causalBudget = this._globalClockHasCausalBudget
                ? this._globalClockCausalBudget.toFixed(6) : 'unavailable';
            readout.dataset.clockRate = this._globalClockHasCausalBudget
                ? this._globalClockRate.toFixed(6) : 'unavailable';
            readout.dataset.causalProjection = this._globalClockProjectionEvents > 0 ? 'true' : 'false';
            readout.style.setProperty('--clock-rate-color', `#${this._globalClockRateColor.getHexString()}`);
            readout.title = 'Global ordinal tick [AXIOM]. The colored local-rate band is '
                + 'τ′min=√max(0,1−Bmax) from the engine’s selected/imposed causal budget, '
                + 'not recovered spacetime. The clockwise color pulse replays the ten stages '
                + 'after a settled tick; rose marks a causal projection. The muted C4 motif is '
                + 'a conditional theory reference, not production G* clock telemetry.';
        }
    }

    _animateGlobalClock(now = clockNow()) {
        if (!this.globalClock || !this._globalClockPhaseCursor) return;
        const elapsed = now - this._globalClockPulseStartedAt;
        const duration = this._globalClockPulseDurationMs;
        const pulseLive = Number.isFinite(elapsed) && elapsed >= 0 && elapsed < duration;
        const progress = pulseLive ? clamp01(elapsed / duration) : 1;
        const activePhase = pulseLive
            ? Math.min(GLOBAL_CLOCK_PHASES.length - 1,
                Math.floor(progress * GLOBAL_CLOCK_PHASES.length)) : -1;
        this._globalClockActiveReplayPhase = activePhase;

        this._globalClockPhaseSegments.forEach((segment, index) => {
            const behind = activePhase - index;
            segment.material.opacity = index === activePhase
                ? 1 : (behind === 1 ? 0.62 : 0.34);
        });

        const cursor = this._globalClockPhaseCursor.children[0];
        this._globalClockPhaseCursor.rotation.z = -progress * Math.PI * 2;
        if (cursor?.material) {
            cursor.material.opacity = pulseLive ? 0.35 + 0.65 * Math.sin(Math.PI * progress) : 0;
            cursor.material.color.copy(this._globalClockRateColor);
        }

        const pulse = pulseLive ? Math.sin(Math.PI * progress) : 0;
        const scale = 1 + pulse * 0.055;
        this.globalClock.scale.setScalar(scale);
        if (this._globalClockForwardArrow) {
            this._globalClockForwardArrow.material.opacity = 0.66 + pulse * 0.34;
        }
        if (this._globalClockHub) {
            this._globalClockHub.scale.setScalar(1 + pulse * 0.38);
        }

        const projectionAge = now - this._globalClockProjectionStartedAt;
        const projectionMix = projectionAge >= 0 && projectionAge < 620
            ? 1 - projectionAge / 620 : 0;
        if (this._globalClockRateRing) {
            this._globalClockRateRing.material.color
                .copy(this._globalClockRateColor)
                .lerp(CLOCK_COLOR_LIMIT, projectionMix);
        }
        if (this._globalClockHub) {
            this._globalClockHub.material.color
                .copy(this._globalClockRateColor)
                .lerp(CLOCK_COLOR_LIMIT, projectionMix);
        }
    }

    onLatticeSizeChanged(size, halfN) {
        this._latticeSize = size;
        this._halfN = halfN;

        // Rebuild boundary wireframe for new size (preserves shape + mode)
        this._buildBoundary(this._boundaryShape, this._boundaryMode);

        // Rebuild axes so length scales with lattice
        if (this.axes) {
            this._scene.remove(this.axes);
            this.axes.geometry.dispose();
            this.axes.material.dispose();
        }
        this._buildAxes();
        this._buildBoundaryOrientation();
        this._buildGlobalClock();

        // Every lattice boot/resize returns to the canonical face-on default.
        // Manual orbiting and the explicit side/top/corner presets remain
        // available after this reset boundary.
        if (this._boundaryMode === 'lattice') {
            this.setCameraPreset('front');
        }
    }

    toggleWireframe(on) {
        this.showWireframe = on;
        if (this.wireframe) this.wireframe.visible = on;
    }

    // ── Camera presets ────────────────────────────────────────────────
    // Snap the orbit camera to a named viewpoint. All positions are
    // computed from the current lattice size so the preset reads the
    // same at N=32 and N=128. The target is always the voxel-center
    // midpoint (N/2) — matches where every physics overlay centers.
    //
    // `which` values:
    //   'front' — looking along -Z (standard "face-on" view)
    //   'side'  — looking along -X
    //   'top'   — looking along -Y (birds-eye)
    // The front preset is also the boot/resize default. Its slightly wider
    // framing keeps the original above-boundary clock position in view.
    setCameraPreset(which) {
        if (this._boundaryMode !== 'lattice') return false;
        const N = this._latticeSize || 32;
        const c = N / 2;
        let dist, pos;
        switch (which) {
            case 'front': dist = N * DEFAULT_FRONT_DISTANCE; pos = [c, c, c + dist]; break;
            case 'side':  dist = N * 1.6; pos = [c + dist, c, c]; break;
            case 'top':   dist = N * 1.6; pos = [c, c + dist, c + 0.001]; break;  // tiny Z offset so OrbitControls can roll freely
            case 'corner':
                dist = N * 1.6;
                const d = dist / Math.sqrt(3);
                pos = [c + d, c + d, c + d];
                break;
            default: return false;
        }
        this._controls.target.set(c, c, c);
        this._camera.position.set(pos[0], pos[1], pos[2]);
        this._controls.update();
        return true;
    }

    setWireframeBrightness(val) {
        if (this._wireframeBrightness === val) return;
        this._wireframeBrightness = val;
        if (!this.wireframe) return;
        this.wireframe.traverse(child => {
            if (child.material && 'opacity' in child.material) {
                child.material.opacity = val;
            }
        });
    }

    toggleAxes(on) {
        this._showAxes = on;
        const mode = this._engineMode || 'lattice';
        if (mode === 'cosmic') return;
        if (mode === 'lattice') {
            if (this.axes) this.axes.visible = on;
        } else {
            if (this.peAxes) this.peAxes.visible = on;
        }
    }

    setVoxelHighlight(x, y, z, active) {
        if (!this._voxelHighlight) {
            const geo = new THREE.BoxGeometry(1.2, 1.2, 1.2);
            const edges = new THREE.EdgesGeometry(geo);
            geo.dispose();   // EdgesGeometry copied what it needs; source is orphan
            const mat = new THREE.LineBasicMaterial({ color: 0xffff00, linewidth: 2 });
            this._voxelHighlight = new THREE.LineSegments(edges, mat);
            this._scene.add(this._voxelHighlight);
        }
        if (active) {
            // Voxel k's rendered centre is at world (k+0.5). Previously this
            // snapped the highlight box to integer world coords, so the box
            // sat on the voxel's lower-left corner instead of its centre —
            // half-voxel shift visible when overlaid on particles/flux.
            const px = x + 0.5;
            const py = y + 0.5;
            const pz = z + 0.5;
            if (this._voxelHighlight.position.x !== px
                || this._voxelHighlight.position.y !== py
                || this._voxelHighlight.position.z !== pz) {
                this._voxelHighlight.position.set(px, py, pz);
            }
            if (!this._voxelHighlight.visible) this._voxelHighlight.visible = true;
        } else if (this._voxelHighlight.visible) {
            this._voxelHighlight.visible = false;
        }
    }

    setAreaHighlight(cx, cy, cz, radius, active) {
        if (!active) {
            if (this._areaHighlight?.visible) this._areaHighlight.visible = false;
            return;
        }
        const r = Math.max(1, Math.round(radius));
        const size = r * 2 + 1;
        if (!this._areaHighlight) {
            // Keep one unit box for the SceneCore lifetime. Radius dragging now
            // changes its transform instead of allocating/discarding geometry
            // and GPU resources on every input frame.
            const geo = new THREE.BoxGeometry(1, 1, 1);
            const edges = new THREE.EdgesGeometry(geo);
            geo.dispose();
            const mat = new THREE.LineBasicMaterial({ color: 0x38bdf8, linewidth: 2, transparent: true, opacity: 0.8 });
            this._areaHighlight = new THREE.LineSegments(edges, mat);
            this._areaHighlight.frustumCulled = false;
            this._scene.add(this._areaHighlight);
        }
        if (this._areaHighlightRadius !== r) {
            this._areaHighlight.scale.setScalar(size);
            this._areaHighlightRadius = r;
        }
        const px = cx + 0.5;
        const py = cy + 0.5;
        const pz = cz + 0.5;
        if (this._areaHighlight.position.x !== px
            || this._areaHighlight.position.y !== py
            || this._areaHighlight.position.z !== pz) {
            this._areaHighlight.position.set(px, py, pz);
        }
        if (!this._areaHighlight.visible) this._areaHighlight.visible = true;
    }

    toggleGrid(on) {
        this._showGrid = on;
        const mode = this._engineMode || 'lattice';
        if (mode === 'cosmic') return;
        if (mode === 'lattice') {
            // Scale 0: the wireframe cube serves as the grid reference
            if (this.wireframe) this.wireframe.visible = on;
            this.showWireframe = on;
        } else {
            // Scale 1+: separate XZ plane grid
            if (this.peGrid) this.peGrid.visible = on;
        }
    }

    _buildPEAxes() {
        // Idempotent rebuild guard (Three-M1 audit, 2026-04-27): if a
        // prior build exists, dispose its geometry+material before
        // overwriting the field reference. Prevents the rare leak path
        // where _buildPEAxes is called twice across a lattice resize.
        const tearDown = (obj) => {
            if (!obj) return;
            this._scene.remove(obj);
            if (obj.geometry) obj.geometry.dispose();
            if (obj.material) obj.material.dispose();
        };
        tearDown(this.peAxes); this.peAxes = null;
        tearDown(this.peGrid); this.peGrid = null;

        const len = 30;

        // ── Axes (RGB lines through origin) ──
        const axVerts = [];
        const axColors = [];
        // X axis (red)
        axVerts.push(-len, 0, 0, len, 0, 0);
        axColors.push(0.5, 0.2, 0.2, 0.9, 0.3, 0.3);
        // Y axis (green)
        axVerts.push(0, -len, 0, 0, len, 0);
        axColors.push(0.2, 0.5, 0.2, 0.3, 0.9, 0.3);
        // Z axis (blue)
        axVerts.push(0, 0, -len, 0, 0, len);
        axColors.push(0.2, 0.2, 0.5, 0.3, 0.3, 0.9);

        const axGeo = new THREE.BufferGeometry();
        axGeo.setAttribute('position', new THREE.Float32BufferAttribute(axVerts, 3));
        axGeo.setAttribute('color', new THREE.Float32BufferAttribute(axColors, 3));
        const axMat = new THREE.LineBasicMaterial({ vertexColors: true, transparent: true, opacity: 0.6 });
        this.peAxes = new THREE.LineSegments(axGeo, axMat);
        this.peAxes.visible = false;
        this._scene.add(this.peAxes);

        // ── Grid (XZ plane lines, separate object for independent toggle) ──
        const grVerts = [];
        const grColors = [];
        for (let i = -len; i <= len; i += 5) {
            if (i === 0) continue;
            grVerts.push(i, 0, -len, i, 0, len);
            grColors.push(0.15, 0.18, 0.25, 0.15, 0.18, 0.25);
            grVerts.push(-len, 0, i, len, 0, i);
            grColors.push(0.15, 0.18, 0.25, 0.15, 0.18, 0.25);
        }
        const grGeo = new THREE.BufferGeometry();
        grGeo.setAttribute('position', new THREE.Float32BufferAttribute(grVerts, 3));
        grGeo.setAttribute('color', new THREE.Float32BufferAttribute(grColors, 3));
        const grMat = new THREE.LineBasicMaterial({ vertexColors: true, transparent: true, opacity: 0.6 });
        this.peGrid = new THREE.LineSegments(grGeo, grMat);
        this.peGrid.visible = false;
        this._scene.add(this.peGrid);
    }

    // ── Post-Processing (Reference frame context Mode) ──────────────────────

    enablePostProcessing() {
        if (this._composer) {
            this._usePostProcessing = true;
            return;
        }
        const rect = this._container.getBoundingClientRect();
        const w = rect.width || 800;
        const h = rect.height || 600;

        this._composer = new EffectComposer(this._renderer);
        this._composer.addPass(new RenderPass(this._scene, this._camera));

        this._bloomPass = new UnrealBloomPass(
            new THREE.Vector2(w, h),
            1.5,  // strength
            0.4,  // radius
            0.2   // threshold
        );
        this._composer.addPass(this._bloomPass);
        this._usePostProcessing = true;
    }

    disablePostProcessing() {
        this._usePostProcessing = false;
    }

    /** Accessor for the bloom pass. Null when post-processing has never
     *  been enabled (first call to enablePostProcessing constructs it).
     *  The Scene panel's adapter uses this to read current values
     *  without importing Three.js or touching the _composer directly. */
    getBloomPass() {
        return this._bloomPass;
    }

    /** Write bloom parameters without reaching into _bloomPass from
     *  outside. Unknown keys are ignored. No-op when the pass has not
     *  been created yet (toggle bloom on first to make it effective). */
    setBloomParams({ strength, radius, threshold } = {}) {
        const pass = this._bloomPass;
        if (!pass) return;
        if (typeof strength === 'number' && Number.isFinite(strength)) pass.strength = strength;
        if (typeof radius === 'number' && Number.isFinite(radius)) pass.radius = radius;
        if (typeof threshold === 'number' && Number.isFinite(threshold)) pass.threshold = threshold;
    }

    /**
     * Render dispatch — uses post-processing composer when enabled,
     * else the plain renderer. Animation hooks (animateQuantumField,
     * spinArrowManager.update) are run by the orchestrator BEFORE
     * calling here so this method is a pure paint step.
     */
    render(scene, camera) {
        this._animateGlobalClock();
        if (this.globalClock) this.globalClock.quaternion.copy(camera.quaternion);
        if (this._usePostProcessing && this._composer) {
            this._composer.render();
        } else {
            this._renderer.render(scene, camera);
        }
    }

    onResize(width, height) {
        if (!Number.isFinite(width) || !Number.isFinite(height) || width <= 0 || height <= 0) return;
        if (this._composer) {
            this._composer.setSize(width, height);
        }
    }

    dispose() {
        this._globalClockHover?.dispose();
        this._globalClockHover = null;
        // Helper: dispose geometry+material for any Three.js Object3D
        const disposeMesh = (obj) => {
            if (!obj) return;
            this._scene.remove(obj);
            if (obj.geometry) obj.geometry.dispose();
            if (obj.material) {
                if (obj.material.map) obj.material.map.dispose();
                obj.material.dispose();
            }
        };

        // Helper: dispose a Group by traversing all children
        const disposeGroup = (group) => {
            if (!group) return;
            this._scene.remove(group);
            group.traverse(child => {
                if (child.geometry) child.geometry.dispose();
                if (child.material) {
                    if (child.material.map) child.material.map.dispose();
                    child.material.dispose();
                }
            });
        };

        // Wireframe is a Group containing LineSegments — traverse children
        disposeGroup(this.wireframe);
        this.wireframe = null;

        // Post-processing composer render targets
        if (this._composer) {
            this._composer.renderTarget1.dispose();
            this._composer.renderTarget2.dispose();
            this._composer = null;
            this._bloomPass = null;
        }

        // Inspector helpers
        disposeMesh(this._voxelHighlight); this._voxelHighlight = null;
        disposeMesh(this._areaHighlight);  this._areaHighlight = null;
        this._areaHighlightRadius = null;

        // Coordinate helpers
        disposeMesh(this.axes);    this.axes = null;
        disposeMesh(this.peAxes);  this.peAxes = null;
        disposeMesh(this.peGrid);  this.peGrid = null;
        disposeGroup(this.boundaryOrientation); this.boundaryOrientation = null;
        disposeGroup(this.globalClock); this.globalClock = null;
        this._orientationArrows = [];
        this._globalClockHand = null;
        this._globalClockHub = null;
        this._globalClockRateRing = null;
        this._globalClockPhaseCursor = null;
        this._globalClockForwardArrow = null;
        this._globalClockPhaseSegments = [];
    }
}
