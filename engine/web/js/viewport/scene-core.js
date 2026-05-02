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

        // Inspector highlight overlays
        this._voxelHighlight = null;
        this._symHighlights = null;

        // Post-processing (lazy init; was used by the now-deleted Scale 11
        // consciousness mode; retained as a public-API hook in case other
        // modes ever opt into bloom).
        this._composer = null;
        this._bloomPass = null;
        this._usePostProcessing = false;

        // Initial scene decoration
        this._buildBoundary(this._boundaryShape, this._boundaryMode);
        this._buildAxes();
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
                group.scale.setScalar(s / 2);
                group.position.set(s / 2, s / 2, s / 2);
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
        this._scene.add(this.axes);
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

        // Recenter camera for lattice mode
        if (this._boundaryMode === 'lattice') {
            const center = size / 2;
            const dist = size * 1.6;
            this._controls.target.set(center, center, center);
            this._camera.position.set(center + dist * 0.25, center + dist * 0.15, center + dist);
            this._controls.update();
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
    //   'iso'   — default isometric (matches boot / resize position)
    //   'moore' — zoomed-in iso that frames a 3×3×3 Moore neighbourhood
    //             around the lattice centre (useful for seed scenarios)
    setCameraPreset(which) {
        if (this._boundaryMode !== 'lattice') return false;
        const N = this._latticeSize || 32;
        const c = N / 2;
        let dist, pos;
        switch (which) {
            case 'front': dist = N * 1.6; pos = [c, c, c + dist]; break;
            case 'side':  dist = N * 1.6; pos = [c + dist, c, c]; break;
            case 'top':   dist = N * 1.6; pos = [c, c + dist, c + 0.001]; break;  // tiny Z offset so OrbitControls can roll freely
            case 'iso':   dist = N * 1.6; pos = [c + dist * 0.25, c + dist * 0.15, c + dist]; break;
            case 'moore': dist = Math.max(6, N * 0.35); pos = [c + dist * 0.6, c + dist * 0.4, c + dist]; break;
            default: return false;
        }
        this._controls.target.set(c, c, c);
        this._camera.position.set(pos[0], pos[1], pos[2]);
        this._controls.update();
        return true;
    }

    setWireframeBrightness(val) {
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
        if (mode === 'cosmic' || mode === 'meta') return;
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
            this._voxelHighlight.position.set(x + 0.5, y + 0.5, z + 0.5);
            this._voxelHighlight.visible = true;
        } else {
            this._voxelHighlight.visible = false;
        }
    }

    setSymmetryHighlights(x, y, z, u1, su2, su3) {
        if (!this._symHighlights) {
            const geo = new THREE.BoxGeometry(1.0, 1.0, 1.0);
            const edges = new THREE.EdgesGeometry(geo);
            geo.dispose();   // EdgesGeometry copied what it needs; source is orphan
            const mat = new THREE.LineBasicMaterial({ color: 0x4ade80, linewidth: 1, transparent: true, opacity: 0.6 });
            this._symHighlights = new THREE.InstancedMesh(edges, mat, 26);
            this._symHighlights.instanceMatrix.setUsage(THREE.DynamicDrawUsage);
            this._scene.add(this._symHighlights);
        }

        let count = 0;
        const dummy = new THREE.Object3D();

        if (u1 || su2 || su3) {
            for (let dx = -1; dx <= 1; dx++) {
                for (let dy = -1; dy <= 1; dy++) {
                    for (let dz = -1; dz <= 1; dz++) {
                        if (dx === 0 && dy === 0 && dz === 0) continue;

                        const norm = Math.abs(dx) + Math.abs(dy) + Math.abs(dz);
                        let include = false;
                        if (u1 && norm === 1) include = true;   // Face
                        if (su2 && norm === 2) include = true;  // Edge
                        if (su3 && norm === 3) include = true;  // Corner

                        if (include) {
                            // Same voxel-centre convention as setVoxelHighlight:
                            // neighbour voxel (x+dx, y+dy, z+dz) is rendered at
                            // world centre (x+dx+0.5, y+dy+0.5, z+dz+0.5).
                            dummy.position.set(x + dx + 0.5, y + dy + 0.5, z + dz + 0.5);
                            dummy.updateMatrix();
                            this._symHighlights.setMatrixAt(count++, dummy.matrix);
                        }
                    }
                }
            }
        }

        this._symHighlights.count = count;
        this._symHighlights.instanceMatrix.needsUpdate = true;
        this._symHighlights.visible = count > 0;
    }

    toggleGrid(on) {
        this._showGrid = on;
        const mode = this._engineMode || 'lattice';
        if (mode === 'cosmic' || mode === 'meta') return;
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

    // ── Post-Processing (Consciousness Mode) ──────────────────────

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
        disposeMesh(this._symHighlights);  this._symHighlights = null;

        // Coordinate helpers
        disposeMesh(this.axes);    this.axes = null;
        disposeMesh(this.peAxes);  this.peAxes = null;
        disposeMesh(this.peGrid);  this.peGrid = null;
    }
}
