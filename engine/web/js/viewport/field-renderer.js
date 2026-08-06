/**
 * ViewportFieldRenderer — Scale-0 field overlay façade.
 * Families: field-em / field-force / field-topology / field-quantum / field-renderer-core.
 * Shared: field-renderer-shared.js (VOXEL_CENTER_OFFSET = 0.0 — do not change).
 */
import { setVoxelCenterOffset } from './field-renderer-shared.js';
import { fieldCoreMethods } from './field-renderer-core.js';
import { fieldEmMethods } from './field-em-renderer.js';
import { fieldForceMethods } from './field-force-renderer.js';
import { fieldTopologyMethods } from './field-topology-renderer.js';
import { fieldQuantumMethods } from './field-quantum-renderer.js';
export { VOXEL_CENTER_OFFSET, getVoxelCenterOffset } from './field-renderer-shared.js';

export class ViewportFieldRenderer {
    constructor({
        scene,
        camera,
        latticeSize,
        halfN,
        boundaryShape,
        insideBoundary,
        getBoundaryMode,
    }) {
        this._scene = scene;
        this._camera = camera;
        this._latticeSize = latticeSize;
        this._halfN = halfN;
        this._center = latticeSize / 2;
        this._radius = latticeSize / 2;
        this._boundaryShape = boundaryShape;
        this._insideBoundary = insideBoundary;
        this._getBoundaryMode = getBoundaryMode || (() => 'lattice');

        // State owned by FieldRenderer (every mesh starts null and is built lazily).
        this._fieldHeatmap = null;
        // Dedicated flux-slice mesh — kept separate from _fieldHeatmap so the
        // Flux Volume appearance controls (opacity/shape/point-size/threshold)
        // can drive the slice without disturbing the potential-heatmap overlay,
        // and so the buffer can size for all three mid-planes (3·N²) at any L.
        // Per-axis enable map keyed by axis index (0=yz, 1=xz, 2=xy); frame-sync
        // reads the enabled set each upload tick to pick which planes to gather.
        this._fluxSliceMesh = null;
        this._fluxSliceMeshSize = 0;
        this._fluxSliceAxes = { 0: true, 1: true, 2: true };
        this._fluxSlicePointScale = 1.0;
        this._fluxSliceThreshold = 0.005;
        this._fieldVectors = null;
        this._peStreamlines = null;
        this._gravityVectors = null;
        this._eFieldLines = null;
        this._bFieldLines = null;
        this._poyntingVectors = null;
        this._divField = null;
        this._forceVolume = null;
        this._gravityField = null;
        this._strongForce = null;
        this._weakField = null;
        this._forceHeatmap = null;
        this._forceStreamlinePool = null;
        this._forceStreamlineMats = null;
        this._forceStreamlineCount = 0;
        this._forceGlyphMeshes = null;
        this._darkMatterHalo = null;
        this._eventHorizonSphere = null;
        this._eventHorizonRing = null;
        this._dampingZones = null;
        this._knotZones = null;
        this._genesisIsosurface = null;
        this._confinementStrings = null;
        this._dualFluxVolume = null;
        this._chiralityField = null;
        this._quantumField = null;
        this._quantumFieldKind = null;
        this._softDiscTex = null;
        this._phaseNeedles = null;
        this._horizonField = null;

        // Visibility state flags (mirrors viewport.js originals).
        this.showHeatmap = false;
        this._psi2Visible = false;
        this._phaseVisible = false;
        this._lagrangianVisible = false;
        this._entropyVisible = false;
        this._psi2Data = null;
        this._phaseData = null;
        this._lagrangianData = null;
        this._entropyData = null;
        this._entropyJitterSeed = 0;
        this._animationClock = 0;

        // Per-overlay magnitude scratch caches.
        this._magCache = null;
        this._strongMagCache = null;
        this._heatMagCache = null;
        this._magCacheDual = null;

        // PERF (F-16): reusable active-index scratch for the arrow/point writers
        // (_writeArrowFieldIntoMesh, updatePoyntingVectors, updateDivergenceField,
        // updateGravityField). Pre-fix each of those allocated a fresh JS Array
        // and grew it via `.push` every update over up to `count` (~4k at L=32,
        // up to 32k for the long-range EM/gravity force fields at fine stride) —
        // a per-update boxed-array allocation that contradicts the F-2 buffer-
        // reuse philosophy. One persistent Int32Array, grown on demand, replaces
        // all four; callers track the live length explicitly. Output-exact: the
        // gather writes the SAME indices in the SAME ascending order, only into
        // a reused typed array instead of a fresh boxed Array.
        this._activeIdx = null;
    }

    // Grow-on-demand accessor for the shared active-index scratch (F-16).
    onLatticeSizeChanged(size, halfN) {
        this._latticeSize = size;
        this._halfN = halfN;
        this._center = size / 2;
        this._radius = size / 2;

        const isOrigin = this._getBoundaryMode && this._getBoundaryMode() === 'origin';
        const cx = isOrigin ? 0.0 : this._center;
        const cy = isOrigin ? 0.0 : this._center;
        const cz = isOrigin ? 0.0 : this._center;

        if (this._eventHorizonSphere) {
            this._eventHorizonSphere.position.set(cx, cy, cz);
        }
        if (this._eventHorizonRing) {
            this._eventHorizonRing.position.set(cx, cy, cz);
        }

        // Rebuild field heatmap for new lattice capacity (it sizes from MAX_FIELD_GRID
        // so capacity is fine, but ensure stale data is cleared).
        if (this._fieldHeatmap) {
            this._scene.remove(this._fieldHeatmap);
            this._fieldHeatmap.geometry.dispose();
            this._fieldHeatmap.material.dispose();
            this._fieldHeatmap = null;
        }

        // Flux-slice mesh sizes from 3·N² — drop it so the next updateFluxSlices()
        // rebuilds at the new capacity (preserving the current showHeatmap state).
        if (this._fluxSliceMesh) {
            this._scene.remove(this._fluxSliceMesh);
            this._fluxSliceMesh.geometry.dispose();
            this._fluxSliceMesh.material.dispose();
            this._fluxSliceMesh = null;
            this._fluxSliceMeshSize = 0;
        }

        // Clear draw ranges on every dynamic field mesh so stale L-data doesn't persist.
        const dynamicMeshes = [
            this._fieldVectors, this._peStreamlines, this._gravityVectors,
            this._eFieldLines, this._bFieldLines, this._poyntingVectors,
            this._divField, this._forceVolume, this._gravityField,
            this._strongForce, this._weakField, this._forceHeatmap,
            this._darkMatterHalo, this._dampingZones, this._genesisIsosurface,
            this._confinementStrings, this._dualFluxVolume,
            this._chiralityField, this._phaseNeedles,
            this._quantumField,
        ];
        for (const m of dynamicMeshes) {
            if (m && m.geometry) m.geometry.setDrawRange(0, 0);
        }
    }
    setBoundaryShape(shape) {
        this._boundaryShape = shape;
        // Most field meshes don't rebuild on shape change — clipping is checked
        // dynamically via the insideBoundary callback per-frame.
    }

    // Animation clock pass-through used by _animateQuantumField. The orchestrator
    // sets this each frame from its own advanceAnimationClock accumulator.
    setAnimationClock(ms) {
        this._animationClock = ms;
    }

    // ── Field Heatmap (potential colored grid dots on XZ plane) ───────
    setEventHorizon(active, radius) {
        if (!this._eventHorizonSphere) this._buildEventHorizon();
        if (active && radius > 0) {
            const isOrigin = this._getBoundaryMode && this._getBoundaryMode() === 'origin';
            const cx = isOrigin ? 0.0 : this._center;
            const cy = isOrigin ? 0.0 : this._center;
            const cz = isOrigin ? 0.0 : this._center;
            this._eventHorizonSphere.position.set(cx, cy, cz);
            this._eventHorizonRing.position.set(cx, cy, cz);
            this._eventHorizonSphere.scale.setScalar(radius);
            this._eventHorizonSphere.visible = true;
            this._eventHorizonRing.scale.setScalar(radius * 3.0);
            this._eventHorizonRing.visible = true;
        } else {
            this._eventHorizonSphere.visible = false;
            this._eventHorizonRing.visible = false;
        }
    }

    // ── Selective Damping Zones (wireframe cubes around damped voxels) ─
    dispose() {
        const disposeMesh = (obj) => {
            if (!obj) return;
            this._scene.remove(obj);
            if (obj.geometry) obj.geometry.dispose();
            if (obj.material) {
                if (obj.material.map) obj.material.map.dispose();
                obj.material.dispose();
            }
        };

        // Simple geometry+material pairs (Points / LineSegments / Mesh).
        const simpleMeshFields = [
            '_fieldHeatmap', '_fluxSliceMesh', '_fieldVectors', '_peStreamlines', '_gravityVectors',
            '_eFieldLines', '_bFieldLines', '_poyntingVectors', '_divField',
            '_forceVolume', '_gravityField', '_strongForce', '_weakField',
            '_forceHeatmap',
            '_darkMatterHalo', '_dampingZones', '_knotZones', '_genesisIsosurface',
            '_confinementStrings',
            '_dualFluxVolume', '_chiralityField',
            '_quantumField', '_phaseNeedles',
            '_eventHorizonSphere', '_eventHorizonRing',
        ];
        for (const name of simpleMeshFields) {
            disposeMesh(this[name]);
            this[name] = null;
        }

        // Per-force glyph meshes (one InstancedMesh per force type).
        if (this._forceGlyphMeshes) {
            for (const m of Object.values(this._forceGlyphMeshes)) disposeMesh(m);
            this._forceGlyphMeshes = null;
        }

        // Force streamline pool (array of Line objects).
        if (this._forceStreamlinePool) {
            for (const line of this._forceStreamlinePool) disposeMesh(line);
            this._forceStreamlinePool = null;
            this._forceStreamlineMats = null;
        }

        // Horizon field (wraps a Points object plus metadata).
        if (this._horizonField) {
            disposeMesh(this._horizonField.points);
            this._horizonField = null;
        }

        // Quantum scaffolding texture (instance-owned, not the static
        // _softSpriteTexture used by weak-field).
        if (this._softDiscTex) {
            this._softDiscTex.dispose();
            this._softDiscTex = null;
        }
    }
    destroy(ctx) {
        this.dispose();
    }
}

Object.assign(
    ViewportFieldRenderer.prototype,
    fieldCoreMethods,
    fieldEmMethods,
    fieldForceMethods,
    fieldTopologyMethods,
    fieldQuantumMethods,
);
