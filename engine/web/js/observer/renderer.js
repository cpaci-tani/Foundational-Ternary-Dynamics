/** First-person Observer ray renderer. Its owner supplies the only presentation frame loop. */
import * as THREE from 'three';
import { LifetimeScope } from '../ui/utils/lifetime-scope.js';
import { SHAPE_NAMES, restMesh, subdivideRestMesh, buildBVH, rotationMatrix, rotate, dot } from './geometry.js';
import { traceObserverRay, visibleSegments, segmentBounds, cameraBasis, projectPoint, createPointProjector, apparentCenter, gamma } from './optics.js';
import { ObserverEnvironment, OBSERVER_ENVIRONMENTS, seededRandom } from './environments.js';
import { observerVertexShader, observerFragmentShader } from './shaders.js';
import { feedbackConfiguration } from './feedback.js';
import { fractalStyle, boundedFractalDetail } from './fractal-presets.js';
import { LAYERS } from './catalog.js';
import { simultaneousBoundsEdges } from './frame-geometry.js';
import { drawWavePhenomena } from './wave-phenomena.js';
import { clipProjectedSegment } from './screen-clipping.js';

// 65,536 authored revisions + 16 environment revisions of 2,048 instances + 4,096 benchmark instances.
export const MAX_OPTICAL_SEGMENTS = 102400;
export const LAYER_KEYS = Object.freeze(LAYERS.map(layer => layer.id));
const TEXTURE_WIDTH = 512;
/** @param {number} texels */
function dataTexture(texels) {
    const width = Math.min(TEXTURE_WIDTH, Math.max(1, texels)), height = Math.max(1, Math.ceil(texels / width));
    const texture = new THREE.DataTexture(new Float32Array(width * height * 4), width, height, THREE.RGBAFormat, THREE.FloatType);
    texture.minFilter = THREE.NearestFilter; texture.magFilter = THREE.NearestFilter; texture.needsUpdate = true; return texture;
}
/** @param {number[]} values */
function filledTexture(values) { const texture = dataTexture(Math.ceil(values.length / 4)); texture.image.data.set(values); texture.needsUpdate = true; return texture; }
/** Immutable shared BLAS atlas. A benchmark may explicitly rebuild it at a measured
 * triangle budget; normal rendering never rebuilds geometry per frame.
 * @param {number} triangleBudget
 */
function geometryAtlas(triangleBudget = 0) {
    /** @type {number[]} */ const nodes = [];
    /** @type {number[]} */ const triangles = [];
    /** @type {number[]} */ const meshes = [];
    let uniqueTriangles = 0;
    const geometry = SHAPE_NAMES.map(shape => ['sphere', 'ellipsoid', 'beacon', 'light-pulse', 'box', 'clock', 'ruler', 'plane', 'disk'].includes(shape) ? null : restMesh(shape));
    const baseTriangles = geometry.reduce((sum, mesh) => sum + (mesh?.triangles.length || 0), 0);
    if (triangleBudget !== 0 && (!Number.isInteger(triangleBudget) || triangleBudget < baseTriangles || triangleBudget > 100000)) throw new RangeError(`Triangle budget must be 0 (normal geometry) or an integer in [${baseTriangles},100000].`);
    const splits = triangleBudget ? Math.floor((triangleBudget - baseTriangles) / 3) : 0;
    const allocations = geometry.map(mesh => mesh ? Math.floor(splits * mesh.triangles.length / baseTriangles) : 0);
    let remaining = splits - allocations.reduce((sum, value) => sum + value, 0);
    for (let i = 0; remaining > 0; i = (i + 1) % geometry.length) if (geometry[i]) { allocations[i]++; remaining--; }
    for (let i = 0; i < geometry.length; i++) {
        const source = geometry[i]; if (!source) { meshes.push(0, 0, 0, 0); continue; }
        const mesh = subdivideRestMesh(source, source.triangles.length + allocations[i] * 3), base = nodes.length / 12, triBase = triangles.length / 12;
        meshes.push(base, mesh.bvh.nodes.length, 0, 0);
        for (const node of mesh.bvh.nodes) nodes.push(...node.min, 0, ...node.max, 0, triBase + node.first, node.count, base + node.escape, 0);
        for (const index of mesh.bvh.order) for (const vertex of mesh.triangles[index]) triangles.push(...mesh.vertices[vertex], 0);
        uniqueTriangles += mesh.triangles.length;
    }
    return { nodes: filledTexture(nodes), triangles: filledTexture(triangles), meshes: filledTexture(meshes), uniqueTriangles, meshBvhNodes: nodes.length / 12, triangleBudget };
}

/** @typedef {import('./optics.js').OpticalSnapshot} OpticalSnapshot */
/** @typedef {{segment:import('./optics.js').OpticalSegment,center:NonNullable<ReturnType<typeof apparentCenter>>,mirrored:boolean}} ApparentSegment */
/** @typedef {import('./optics.js').OpticalSettings & {renderScale?:number,fractalDetail?:number,autoQuality?:boolean,doppler?:boolean,beaming?:boolean,artisticShading?:boolean,mirrorWorld?:boolean,feedbackEnabled?:boolean,feedbackLayers?:number,feedbackDepth?:number,feedbackStrength?:number,feedbackScale?:number,layers?:Record<string,boolean>,overlayFilter?:string,authorPreview?:import('./optics.js').OpticalSegment}} RendererSettings */
export class ObserverRenderer {
    /** @param {{container:HTMLElement}} options */
    constructor({ container }) {
        this.scope = new LifetimeScope(); this.container = container; this.disposed = false; this.contextLost = false;
        this.width = 1; this.height = 1; this.renderScale = 1;
        this.renderer = new THREE.WebGLRenderer({ antialias: false, alpha: false, powerPreference: 'high-performance', preserveDrawingBuffer: false });
        this.renderer.setPixelRatio(1); this.renderer.autoClear = true;
        this.canvas = this.renderer.domElement; this.canvas.className = 'observer-canvas'; this.canvas.setAttribute('aria-label', 'Mind’s Eye relativistic viewport'); this.canvas.tabIndex = 0;
        this.canvas.style.cssText = 'position:absolute;inset:0;width:100%;height:100%;display:block;outline:none';
        container.appendChild(this.canvas);
        this.overlayCanvas = document.createElement('canvas'); this.overlayCanvas.className = 'observer-geometry-overlay'; this.overlayCanvas.style.cssText = 'position:absolute;inset:0;width:100%;height:100%;pointer-events:none';
        this.overlayCanvas.setAttribute('aria-hidden', 'true'); container.appendChild(this.overlayCanvas); this.overlayContext = this.overlayCanvas.getContext('2d');
        this.environment = new ObserverEnvironment(); this.atlas = geometryAtlas();
        this.capacity = 256;
        this.segmentTexture = dataTexture(this.capacity * 8); this.nodeTexture = dataTexture(this.capacity * 6); this.orderTexture = dataTexture(this.capacity); this.emptyTexture = dataTexture(1);
        this.scene = new THREE.Scene(); this.camera = new THREE.Camera();
        const v3 = () => new THREE.Vector3();
        this.uniforms = {
            uFractalStyle: { value: -1 }, uFractalDetail: { value: 0.65 },
            uSegments: { value: this.segmentTexture }, uNodes: { value: this.nodeTexture }, uOrder: { value: this.orderTexture }, uMeshNodes: { value: this.atlas.nodes }, uTriangles: { value: this.atlas.triangles }, uMeshes: { value: this.atlas.meshes }, uPanorama: { value: this.emptyTexture }, uLines: { value: this.emptyTexture },
            uNodeCount: { value: 0 }, uSegmentCount: { value: 0 }, uLineCount: { value: 0 }, uPreset: { value: 0 }, uLayers: { value: 1 }, uDebugMode: { value: 0 }, uSelected: { value: -1 },
            uSR: { value: true }, uOptical: { value: true }, uDoppler: { value: true }, uBeaming: { value: true }, uShading: { value: true }, uHasPanorama: { value: false }, uCameraShell: { value: false }, uMirrorWorld: { value: false },
            uFeedback: { value: /** @type {THREE.Texture} */ (this.emptyTexture) }, uHasFeedback: { value: false }, uFeedbackLayers: { value: 2 }, uFeedbackStrength: { value: 0.45 }, uFeedbackFade: { value: 1 }, uSeed: { value: 1 },
            uResolution: { value: new THREE.Vector2(1, 1) }, uPickNdc: { value: new THREE.Vector2(0, 0) }, uCamera: { value: v3() }, uVelocity: { value: v3() }, uForward: { value: v3() }, uRight: { value: v3() }, uUp: { value: v3() }, uEnvironmentColor: { value: v3() },
            uFov: { value: 60 }, uAspect: { value: 1 }, uTime: { value: 0 }, uHistoryStart: { value: -60 }, uRadius: { value: 40 }, uDensity: { value: 1 }, uSpacing: { value: 4 }, uOrientation: { value: 0 }, uOpacity: { value: 1 }, uAnimationRate: { value: 0 },
        };
        this.material = new THREE.RawShaderMaterial({ uniforms: this.uniforms, vertexShader: observerVertexShader, fragmentShader: observerFragmentShader, glslVersion: THREE.GLSL3, depthTest: false, depthWrite: false });
        this.geometry = new THREE.PlaneGeometry(2, 2); this.scene.add(new THREE.Mesh(this.geometry, this.material));
        this.pickTarget = new THREE.WebGLRenderTarget(1, 1, { type: THREE.FloatType, format: THREE.RGBAFormat, depthBuffer: false, stencilBuffer: false });
        // Exactly two targets. The first pass each frame has feedback disabled,
        // preventing temporal accumulation and read/write texture feedback hazards.
        this.feedbackTargets = [0, 1].map(() => new THREE.WebGLRenderTarget(1, 1, { type: THREE.UnsignedByteType, format: THREE.RGBAFormat, depthBuffer: false, stencilBuffer: false, minFilter: THREE.LinearFilter, magFilter: THREE.LinearFilter }));
        /** @type {import('./optics.js').OpticalSegment[]} */ this.benchmarkSegments = [];
        /** @type {import('./optics.js').OpticalSegment[]} */ this.segments = [];
        /** @type {Map<string,import('./optics.js').OpticalSegment[]>} */ this.historiesById = new Map();
        /** @type {Map<string,{direct?:ApparentSegment[],mirrored?:ApparentSegment[]}>} */ this.apparentSegmentsById = new Map();
        /** @type {OpticalSnapshot|null} */ this.observationCacheSnapshot = null;
        this.observationCacheTime = 0; this.observationCacheProfile = ''; this.observationCacheOptical = false; this.observationCacheHistoryStart = 0;
        this.observationCachePosition = [0, 0, 0]; this.observationCacheVelocity = [0, 0, 0];
        /** @type {import('./optics.js').OpticalHit|null} */ this.hit = null;
        /** @type {import('./optics.js').TraceAcceleration|null} */ this.traceAcceleration = null;
        /** @type {OpticalSnapshot|null} */ this.preparedSnapshot = null;
        this.preparedViewKey = '';
        this.diagnostics = { fractalStyle: -1, fractalDetail: 0.65, tracedInstances: 0, staticInstances: 0, movingInstances: 0, uniqueTriangles: this.atlas.uniqueTriangles, meshBvhNodes: this.atlas.meshBvhNodes, requestedTriangleBudget: this.atlas.triangleBudget, internalResolution: [1, 1], requestedScale: 1, internalScale: 1, gpuTimeMs: 0, adaptiveQuality: true, adaptiveGpuBudgetMs: 11, feedbackQualityFactor: 1, layerCount: 0, environmentStatus: 'ready', environmentMessage: '', mirrorWorld: false, feedbackEnabled: false, feedbackLayers: 0, feedbackDepth: 0, feedbackPasses: 0, feedbackResolution: [0, 0], feedbackTargetCount: 2, gpu: this.gpuInfo() };
        this.requestedScale = 1;
        // Software devices start at a declared half-resolution image; physical/history accuracy is unchanged.
        this.qualityFactor = /swiftshader|llvmpipe|software/i.test(String(this.diagnostics.gpu.renderer)) ? 0.5 : 1;
        this.feedbackQualityFactor = 1;
        /** @type {number[]} */ this.qualitySamples = [];
        /** @type {number[]} */ this.qualityGpuSamples = [];
        this.recoveryFrames = 0; this.lastGpuTimeMs = 0;
        /** @type {{TIME_ELAPSED_EXT:number,GPU_DISJOINT_EXT:number}|null} */
        this.timerExtension = this.renderer.getContext().getExtension('EXT_disjoint_timer_query_webgl2');
        /** @type {WebGLQuery[]} */ this.pendingQueries = [];
        this.scope.on(this.canvas, 'webglcontextlost', (/** @type {Event} */ event) => {
            event.preventDefault(); this.contextLost = true;
            // Release Three's old-context target listeners while GL is lost (deletes
            // are harmless no-ops). Deleting their stale handles after restoration
            // instead raises INVALID_OPERATION in Chromium/ANGLE.
            for (const target of this.feedbackTargets) target.dispose();
            this.pickTarget.dispose();
            this.uniforms.uHasFeedback.value = false; this.uniforms.uFeedback.value = this.emptyTexture;
            // Query objects and enabled extensions belong to the lost context.
            this.pendingQueries.length = 0; this.timerExtension = null; this.lastGpuTimeMs = 0; this.qualitySamples.length = 0; this.qualityGpuSamples.length = 0;
        });
        this.scope.on(this.canvas, 'webglcontextrestored', () => {
            // Three's earlier listener restores its own GL state. Explicitly reacquire
            // our extensions before any timer enum or float readback is used again.
            this.timerExtension = this.renderer.getContext().getExtension('EXT_disjoint_timer_query_webgl2');
            this.diagnostics.gpu = this.gpuInfo();
            for (const texture of this.textures()) texture.needsUpdate = true;
            this.uniforms.uHasFeedback.value = false; this.uniforms.uFeedback.value = this.emptyTexture;
            if (this.environment.texture) this.environment.texture.needsUpdate = true;
            this.material.needsUpdate = true; this.contextLost = false;
        });
        this.resize(container.clientWidth || 960, container.clientHeight || 640);
    }
    textures() { return [this.segmentTexture, this.nodeTexture, this.orderTexture, this.emptyTexture, this.atlas.nodes, this.atlas.triangles, this.atlas.meshes]; }
    /** Grow reusable GPU buffers on demand; ordinary scenes do not upload the maximum history budget every frame.
     * @param {number} count
     */
    ensureCapacity(count) {
        if (count <= this.capacity) return;
        this.capacity = Math.min(MAX_OPTICAL_SEGMENTS, 2 ** Math.ceil(Math.log2(count)));
        this.segmentTexture.dispose(); this.nodeTexture.dispose(); this.orderTexture.dispose();
        this.segmentTexture = dataTexture(this.capacity * 8); this.nodeTexture = dataTexture(this.capacity * 6); this.orderTexture = dataTexture(this.capacity);
        this.uniforms.uSegments.value = this.segmentTexture; this.uniforms.uNodes.value = this.nodeTexture; this.uniforms.uOrder.value = this.orderTexture;
    }
    gpuInfo() {
        const gl = this.renderer.getContext(), extension = gl.getExtension('WEBGL_debug_renderer_info');
        return { vendor: extension ? gl.getParameter(extension.UNMASKED_VENDOR_WEBGL) : gl.getParameter(gl.VENDOR), renderer: extension ? gl.getParameter(extension.UNMASKED_RENDERER_WEBGL) : gl.getParameter(gl.RENDERER), floatReadback: !!gl.getExtension('EXT_color_buffer_float') };
    }
    /** Reserve GPU headroom for the separately owned inference worker.
     * The existing automatic-quality switch still governs all image changes.
     * @param {boolean} busy
     */
    setExternalGpuLoad(busy) {
        this.externalGpuLoad = busy;
        this.diagnostics.adaptiveGpuBudgetMs = busy ? 7 : 11;
    }
    /** Measure GPU work when available, otherwise use the owning frame loop's measured interval.
     * Resolution is the only automatic adjustment: no rays, records, history or targets are discarded.
     * @param {RendererSettings} settings @param {number} dt
     */
    updateQuality(settings, dt) {
        this.requestedScale = Math.max(0.25, Math.min(1.5, settings.renderScale ?? 1));
        const gl = /** @type {WebGL2RenderingContext} */ (this.renderer.getContext()), extension = this.timerExtension;
        while (extension && this.pendingQueries.length && gl.getQueryParameter(this.pendingQueries[0], gl.QUERY_RESULT_AVAILABLE)) {
            const query = /** @type {WebGLQuery} */ (this.pendingQueries.shift());
            if (!gl.getParameter(extension.GPU_DISJOINT_EXT)) this.lastGpuTimeMs = Number(gl.getQueryParameter(query, gl.QUERY_RESULT)) / 1e6;
            gl.deleteQuery(query);
        }
        if (settings.autoQuality === false) { this.qualitySamples.length = 0; this.qualityGpuSamples.length = 0; this.recoveryFrames = 0; return; }
        if (this.externalGpuLoad) {
            this.qualityFactor = Math.min(this.qualityFactor, 0.8);
            this.feedbackQualityFactor = Math.min(this.feedbackQualityFactor, 0.35);
        }
        const elapsed = dt > 0 && dt < 10 ? dt * 1000 : this.lastGpuTimeMs;
        if (elapsed <= 0) return;
        this.qualitySamples.push(elapsed);
        if (this.lastGpuTimeMs > 0) this.qualityGpuSamples.push(this.lastGpuTimeMs);
        // 60 Hz needs headroom for submission, labels and browser composition. A
        // 20 ms average tolerates missed refreshes; reserve an 11 ms GPU budget.
        if (elapsed <= 17 && this.lastGpuTimeMs > 0 && this.lastGpuTimeMs < 7) this.recoveryFrames++; else this.recoveryFrames = 0;
        if (this.qualitySamples.length >= 12) {
            const average = this.qualitySamples.reduce((sum, value) => sum + value, 0) / this.qualitySamples.length;
            const gpuAverage = this.qualityGpuSamples.length ? this.qualityGpuSamples.reduce((sum, value) => sum + value, 0) / this.qualityGpuSamples.length : 0;
            this.qualitySamples.length = 0; this.qualityGpuSamples.length = 0;
            // Resolution cannot fix isolated CPU/OS stalls once GPU work is
            // already comfortably below budget; avoid shrinking for those.
            const frameOverloaded = average > 17.25 && (!extension || gpuAverage > 8);
            if (frameOverloaded || gpuAverage > this.diagnostics.adaptiveGpuBudgetMs) {
                // The billboard is presentation-only, so spend its pixels before
                // lowering the primary optical image. Never drop recursion passes.
                if (settings.feedbackEnabled && this.feedbackQualityFactor > 0.35) this.feedbackQualityFactor = Math.max(0.35, this.feedbackQualityFactor * 0.8);
                else this.qualityFactor = Math.max(0.25 / this.requestedScale, this.qualityFactor * 0.9);
                this.recoveryFrames = 0;
            }
        }
        // Ten seconds of substantial measured headroom avoids the previous
        // down/up oscillation near the refresh deadline.
        if (this.recoveryFrames >= 600) {
            if (this.qualityFactor < 1) this.qualityFactor = Math.min(1, this.qualityFactor + 0.05);
            else this.feedbackQualityFactor = Math.min(1, this.feedbackQualityFactor + 0.05);
            this.recoveryFrames = 0;
        }
    }
    /** @param {number} width @param {number} height */
    resize(width, height) {
        this.width = Math.max(1, Math.floor(width)); this.height = Math.max(1, Math.floor(height));
        this.renderer.setSize(Math.max(1, Math.floor(this.width * this.renderScale)), Math.max(1, Math.floor(this.height * this.renderScale)), false);
        this.overlayCanvas.width = this.width; this.overlayCanvas.height = this.height;
        this.uniforms.uResolution.value.set(this.canvas.width, this.canvas.height); this.uniforms.uAspect.value = this.width / this.height;
        this.diagnostics.internalResolution = [this.canvas.width, this.canvas.height];
    }
    /** @param {OpticalSnapshot} snapshot @param {RendererSettings} settings */
    prepare(snapshot, settings) {
        const requested = Math.max(0.25, Math.min(1.5, settings.renderScale ?? 1));
        const scale = settings.autoQuality === false ? requested : Math.max(0.25, requested * this.qualityFactor);
        if (scale !== this.renderScale) { this.renderScale = scale; this.resize(this.width, this.height); }
        this.diagnostics.requestedScale = requested; this.diagnostics.internalScale = scale; this.diagnostics.gpuTimeMs = this.lastGpuTimeMs; this.diagnostics.adaptiveQuality = settings.autoQuality !== false;
        const environment = /** @type {import('./environments.js').EnvironmentSettings} */ (snapshot.environment || {});
        this.environment.update(environment, snapshot.profile === 'sr' && settings.optical !== false ? snapshot.environmentHistory : undefined);
        const extras = [...this.environment.segments, ...this.benchmarkSegments];
        this.segments = visibleSegments(snapshot, settings, extras);
        this.historiesById.clear(); this.apparentSegmentsById.clear();
        for (const entity of snapshot.entities) {
            const id = entity.id || entity.entityId; if (id) this.historiesById.set(id, []);
        }
        if (this.segments.length > MAX_OPTICAL_SEGMENTS) throw new RangeError(`Optical history has ${this.segments.length} segments; renderer budget is ${MAX_OPTICAL_SEGMENTS}. No geometry was silently dropped.`);
        this.ensureCapacity(this.segments.length);
        const observer = { ...snapshot.observer, ...settings.cameraOverride }, sr = snapshot.profile === 'sr', optical = sr && settings.optical !== false;
        const position = observer.position, segmentData = this.segmentTexture.image.data;
        const bounds = this.segments.map((segment, index) => {
            this.historiesById.get(segment.entityId || segment.id || '')?.push(segment);
            const rotation = rotationMatrix(segment.rotation), velocity = segment.velocity || [0, 0, 0], originTime = segment.originTime || 0;
            const center = segment.position.map((x, j) => x + velocity[j] * (snapshot.time - originTime) - position[j]);
            const shape = segment.shape === 'pulse' ? 'light-pulse' : segment.shape;
            const shapeId = SHAPE_NAMES.indexOf(shape); if (shapeId < 0) throw new TypeError(`Unsupported geometry: ${segment.shape}`);
            // Declared three-line reference spectra, not a reconstruction of an arbitrary RGB material's spectrum.
            const color = segment.spectral === 'red-line' ? [1, 0, 0] : segment.spectral === 'green-line' ? [0, 1, 0] : segment.spectral === 'blue-line' ? [0, 0, 1] : segment.color;
            segmentData.set([...center, shapeId, ...velocity, sr ? gamma(velocity) : 1, ...segment.size, Math.max(-1e20, (segment.start ?? -Infinity) - snapshot.time), ...rotation.slice(0, 3), Math.min(1e20, (segment.end ?? Infinity) - snapshot.time), ...rotation.slice(3, 6), segment.revision, ...rotation.slice(6, 9), index, ...color, segment.emission ?? 1, segment.clockOffset || 0, originTime - snapshot.time, (segment.entityId || '').startsWith('environment:') ? 2 : segment.decorative ? 1 : 0, segment.animationRate || 0], index * 32);
            const box = segmentBounds(segment, snapshot.time, snapshot.historyStart, index);
            return { ...box, min: box.min.map((x, j) => x - position[j]), max: box.max.map((x, j) => x - position[j]) };
        });
        const bvh = buildBVH(bounds), nodeData = this.nodeTexture.image.data, orderData = this.orderTexture.image.data;
        this.traceAcceleration = { segments: this.segments, bvh, offset: position };
        this.preparedSnapshot = snapshot; this.preparedViewKey = JSON.stringify([settings.optical, settings.cameraOverride]);
        this.rememberObservationContext(snapshot, settings);
        bvh.nodes.forEach((node, index) => nodeData.set([...node.min, 0, ...node.max, 0, node.first, node.count, node.escape, 0], index * 12));
        bvh.order.forEach((value, index) => { orderData[index * 4] = value; });
        this.segmentTexture.needsUpdate = true; this.nodeTexture.needsUpdate = true; this.orderTexture.needsUpdate = true;
        const u = this.uniforms, basis = cameraBasis(observer);
        u.uNodeCount.value = bvh.nodes.length; u.uSegmentCount.value = this.segments.length; u.uSR.value = sr; u.uOptical.value = optical;
        u.uCamera.value.fromArray(position); u.uVelocity.value.fromArray(observer.velocity); u.uForward.value.fromArray(basis.forward); u.uRight.value.fromArray(basis.right); u.uUp.value.fromArray(basis.up);
        u.uTime.value = snapshot.time; u.uHistoryStart.value = snapshot.historyStart; u.uFov.value = settings.fov || 60; u.uDoppler.value = settings.doppler !== false; u.uBeaming.value = settings.beaming !== false; u.uShading.value = settings.artisticShading !== false;
        u.uMirrorWorld.value = settings.mirrorWorld === true; this.diagnostics.mirrorWorld = u.uMirrorWorld.value;
        u.uSelected.value = this.segments.findIndex(s => (s.entityId || s.id) === settings.selectedId);
        u.uLayers.value = LAYER_KEYS.reduce((bits, key, i) => bits | ((settings.layers?.[key] ?? key === 'grid') ? 1 << i : 0), 0);
        u.uPreset.value = Math.max(0, OBSERVER_ENVIRONMENTS.findIndex(p => p.id === (environment.preset || 'void'))); u.uCameraShell.value = environment.anchor === 'camera' && u.uPreset.value >= 12;
        u.uEnvironmentColor.value.fromArray(environment.color || [0.2, 0.7, 1]); u.uRadius.value = environment.radius || 40; u.uDensity.value = environment.density ?? 1; u.uSpacing.value = environment.spacing || 4; u.uOrientation.value = environment.orientation || 0; u.uOpacity.value = environment.opacity ?? 1; u.uAnimationRate.value = environment.animationRate || 0;
        u.uSeed.value = environment.seed ?? 1;
        u.uFractalStyle.value = fractalStyle(environment.preset || 'void');
        u.uFractalDetail.value = boundedFractalDetail(settings.fractalDetail);
        this.diagnostics.fractalStyle = u.uFractalStyle.value; this.diagnostics.fractalDetail = u.uFractalDetail.value;
        u.uHasPanorama.value = !!this.environment.texture; u.uPanorama.value = this.environment.texture || this.emptyTexture;
        this.diagnostics.tracedInstances = this.segments.length; this.diagnostics.staticInstances = this.segments.filter(s => dot(s.velocity, s.velocity) === 0).length; this.diagnostics.movingInstances = this.segments.length - this.diagnostics.staticInstances;
        this.diagnostics.layerCount = LAYER_KEYS.filter(key => settings.layers?.[key]).length; this.diagnostics.environmentStatus = this.environment.status; this.diagnostics.environmentMessage = this.environment.message;
    }
    /** @param {OpticalSnapshot} snapshot @param {RendererSettings} settings @param {number} dt */
    render(snapshot, settings = {}, dt = 0) {
        if (this.disposed || this.contextLost) return null;
        this.updateQuality(settings, dt);
        this.prepare(snapshot, settings); this.uniforms.uDebugMode.value = 0; this.renderer.setRenderTarget(null);
        const gl = /** @type {WebGL2RenderingContext} */ (this.renderer.getContext());
        const query = this.timerExtension && this.pendingQueries.length < 4 ? gl.createQuery() : null;
        if (query && this.timerExtension) gl.beginQuery(this.timerExtension.TIME_ELAPSED_EXT, query);
        try { this.renderFeedback(snapshot, settings); this.renderer.render(this.scene, this.camera); }
        finally { if (query && this.timerExtension) { gl.endQuery(this.timerExtension.TIME_ELAPSED_EXT); this.pendingQueries.push(query); } }
        this.drawOverlays(snapshot, settings);
        this.hit = this.pick(snapshot, settings);
        return this.hit;
    }
    /** Render bounded nested camera billboards from this immutable observation event.
     * Alternate view rotations share the physical observer position and velocity.
     * Their compositor is decorative and cannot replace the primary ray hit.
     * @param {OpticalSnapshot} snapshot @param {RendererSettings} settings
     */
    renderFeedback(snapshot, settings) {
        const configuration = feedbackConfiguration(settings), u = this.uniforms;
        u.uHasFeedback.value = false; u.uFeedback.value = this.emptyTexture;
        this.diagnostics.feedbackEnabled = configuration.enabled;
        this.diagnostics.feedbackPasses = 0; this.diagnostics.feedbackLayers = configuration.enabled ? configuration.layers : 0; this.diagnostics.feedbackDepth = configuration.enabled ? configuration.depth : 0;
        if (!configuration.enabled || configuration.strength === 0) { this.diagnostics.feedbackResolution = [0, 0]; return; }
        // Quality adaptation lowers only billboard pixels. Iteration and layer budgets stay explicit.
        const scale = configuration.scale * (settings.autoQuality === false ? 1 : this.feedbackQualityFactor * Math.sqrt(3 / configuration.depth));
        this.diagnostics.feedbackQualityFactor = settings.autoQuality === false ? 1 : this.feedbackQualityFactor;
        const width = Math.max(1, Math.floor(this.canvas.width * scale)), height = Math.max(1, Math.floor(this.canvas.height * scale));
        for (const target of this.feedbackTargets) if (target.width !== width || target.height !== height) target.setSize(width, height);
        this.diagnostics.feedbackResolution = [width, height];
        const observer = { ...snapshot.observer, ...settings.cameraOverride }, primary = cameraBasis(observer);
        u.uFeedbackLayers.value = configuration.layers; u.uFeedbackStrength.value = configuration.strength;
        try {
            for (let pass = 0; pass < configuration.depth; pass++) {
                const target = this.feedbackTargets[pass % 2], camera = cameraBasis({ ...observer, yaw: (observer.yaw || 0) + (pass % 2 ? -0.065 : 0.065), pitch: (observer.pitch || 0) + (pass % 2 ? -0.025 : 0.025) });
                u.uForward.value.fromArray(camera.forward); u.uRight.value.fromArray(camera.right); u.uUp.value.fromArray(camera.up);
                u.uResolution.value.set(width, height);
                // Each deeper image is attenuated. The last nested image has no child image.
                u.uFeedbackFade.value = pass / configuration.depth;
                this.renderer.setRenderTarget(target); this.renderer.render(this.scene, this.camera);
                u.uFeedback.value = target.texture; u.uHasFeedback.value = true;
                this.diagnostics.feedbackPasses++;
            }
        } finally {
            this.renderer.setRenderTarget(null); u.uResolution.value.set(this.canvas.width, this.canvas.height);
            u.uForward.value.fromArray(primary.forward); u.uRight.value.fromArray(primary.right); u.uUp.value.fromArray(primary.up); u.uFeedbackFade.value = 1;
        }
    }
    /** @param {OpticalSnapshot} snapshot @param {RendererSettings} settings @param {number} ndcX @param {number} ndcY */
    pick(snapshot, settings = {}, ndcX = 0, ndcY = 0) {
        const prepared = this.preparedSnapshot === snapshot && this.preparedViewKey === JSON.stringify([settings.optical, settings.cameraOverride]) ? this.traceAcceleration : null;
        return traceObserverRay(snapshot, { ...settings, aspect: this.width / this.height }, ndcX, ndcY, prepared ? [] : [...this.environment.segments, ...this.benchmarkSegments], prepared);
    }
    /** Exact GPU first-hit output from the same material used to paint the scene. Float render targets are required.
     * @param {OpticalSnapshot} snapshot @param {RendererSettings} settings @param {number} ndcX @param {number} ndcY
     */
    readPixelHit(snapshot, settings = {}, ndcX = 0, ndcY = 0) {
        if (this.disposed || this.contextLost) throw new Error('GPU optical readback requires a live graphics context.');
        if (!this.diagnostics.gpu.floatReadback) throw new Error('EXT_color_buffer_float is required for GPU optical parity checks.');
        this.prepare(snapshot, settings); this.uniforms.uDebugMode.value = 1; this.uniforms.uPickNdc.value.set(ndcX, ndcY);
        const pixels = new Float32Array(4);
        try { this.renderer.setRenderTarget(this.pickTarget); this.renderer.render(this.scene, this.camera); this.renderer.readRenderTargetPixels(this.pickTarget, 0, 0, 1, 1, pixels); }
        finally { this.renderer.setRenderTarget(null); this.uniforms.uDebugMode.value = 0; }
        const index = Math.round(Math.abs(pixels[0])) - 1, segment = this.segments[index];
        return segment ? { entityId: segment.entityId || segment.id, id: segment.entityId || segment.id, revision: segment.revision, distance: pixels[1], emissionTime: pixels[2], properTime: pixels[3], segmentIndex: index, mirrored: pixels[0] < 0 } : null;
    }
    /** @param {OpticalSnapshot} snapshot @param {RendererSettings} settings @param {number[]} point */
    projectPoint(snapshot, settings, point) { return projectPoint(snapshot, { ...settings, aspect: this.width / this.height }, point); }
    /** @param {OpticalSnapshot} snapshot @param {RendererSettings} settings */
    rememberObservationContext(snapshot, settings) {
        const observer = { ...snapshot.observer, ...settings.cameraOverride };
        this.observationCacheSnapshot = snapshot; this.observationCacheTime = snapshot.time; this.observationCacheProfile = snapshot.profile;
        this.observationCacheOptical = snapshot.profile === 'sr' && settings.optical !== false; this.observationCacheHistoryStart = snapshot.historyStart;
        for (let i = 0; i < 3; i++) { this.observationCachePosition[i] = observer.position[i]; this.observationCacheVelocity[i] = observer.velocity[i]; }
    }
    /** Standalone label/overlay calls may use a different observation than the last
     * GPU preparation. Refresh only their immutable authored records and centres.
     * @param {OpticalSnapshot} snapshot @param {RendererSettings} settings
     */
    synchronizeObservationCache(snapshot, settings) {
        const observer = { ...snapshot.observer, ...settings.cameraOverride };
        const sourceChanged = this.observationCacheSnapshot !== snapshot || this.observationCacheTime !== snapshot.time || this.observationCacheProfile !== snapshot.profile
            || this.observationCacheOptical !== (snapshot.profile === 'sr' && settings.optical !== false) || this.observationCacheHistoryStart !== snapshot.historyStart;
        const observerChanged = observer.position.some((x, i) => x !== this.observationCachePosition[i]) || observer.velocity.some((x, i) => x !== this.observationCacheVelocity[i]);
        if (!sourceChanged && !observerChanged) return;
        this.apparentSegmentsById.clear();
        if (sourceChanged) {
            this.historiesById.clear();
            for (const entity of snapshot.entities) { const id = entity.id || entity.entityId; if (id) this.historiesById.set(id, []); }
            for (const segment of visibleSegments(snapshot, settings)) this.historiesById.get(segment.entityId || segment.id || '')?.push(segment);
        }
        this.rememberObservationContext(snapshot, settings);
    }
    /** Compute each retained centre event once per prepared snapshot and mirror side.
     * Labels and geometric overlays share this cache; source records are immutable.
     * @param {OpticalSnapshot} snapshot @param {RendererSettings} settings @param {string} id @param {boolean} mirrored
     * @returns {ApparentSegment[]}
     */
    apparentSegments(snapshot, settings, id, mirrored) {
        let cached = this.apparentSegmentsById.get(id);
        if (!cached) { cached = {}; this.apparentSegmentsById.set(id, cached); }
        const key = mirrored ? 'mirrored' : 'direct';
        if (cached[key]) return /** @type {ApparentSegment[]} */ (cached[key]);
        /** @type {ApparentSegment[]} */ const result = [];
        for (const source of this.historiesById.get(id) || []) {
            const segment = mirrored ? { ...source, position: [source.position[0], -source.position[1], source.position[2]], velocity: [source.velocity[0], -source.velocity[1], source.velocity[2]] } : source;
            const center = apparentCenter(snapshot, settings, segment);
            if (center) result.push({ segment, center, mirrored });
        }
        cached[key] = result; return result;
    }
    /** @param {OpticalSnapshot} snapshot @param {RendererSettings} settings */
    getOverlayAnchors(snapshot, settings = {}) {
        if (settings.overlayFilter === 'none') return [];
        this.synchronizeObservationCache(snapshot, settings);
        const candidates = [];
        for (const entity of snapshot.entities) {
            const selected = entity.id === settings.selectedId;
            if (entity.overlay === false || (!selected && settings.overlayFilter !== 'all')) continue;
            for (let pass = 0; pass < (settings.mirrorWorld ? 2 : 1); pass++) for (const { segment, center } of this.apparentSegments(snapshot, settings, entity.id || entity.entityId || '', pass === 1)) {
                const mirrored = pass === 1;
                const point = [...center.position]; point[1] += segment.size[1] * 0.6;
                const screen = this.projectPoint(snapshot, settings, point); if (!screen.visible) continue;
                candidates.push({ id: entity.id, entityId: entity.id, entity, segment, mirrored, revision: segment.revision, emissionTime: center.emissionTime, historical: entity.alive === false || segment.revision !== entity.revision, ...screen, selected }); break;
            }
        }
        candidates.sort((a, b) => Number(b.selected) - Number(a.selected) || a.distance - b.distance);
        const visible = [], observer = { ...snapshot.observer, ...settings.cameraOverride };
        for (const candidate of candidates) {
            const occluder = this.pick(snapshot, settings, candidate.x * 2 - 1, 1 - candidate.y * 2);
            const occluderDistance = occluder ? Math.hypot(...occluder.position.map((value, j) => value - observer.position[j])) : Infinity;
            if (occluder && occluder.entityId !== candidate.entityId && occluderDistance < candidate.distance - 0.001) continue;
            visible.push(candidate); if (visible.length === 32) break;
        }
        return visible;
    }
    /** @param {OpticalSnapshot} snapshot @param {RendererSettings} settings */
    drawOverlays(snapshot, settings) {
        const context = this.overlayContext; if (!context) return;
        this.synchronizeObservationCache(snapshot, settings);
        context.clearRect(0, 0, this.width, this.height);
        const layers = settings.layers || {}, observer = { ...snapshot.observer, ...settings.cameraOverride };
        const projectionSettings = { ...settings, aspect: this.width / this.height };
        /** Reused mesh vertex arrays have immutable values during this draw.
         * The cache is frame-local, so no camera/time revisions can go stale.
         * @param {ReturnType<typeof createPointProjector>} projector */
        const cachedProjector = projector => {
            /** @type {WeakMap<number[],ReturnType<typeof projector>>} */ const points = new WeakMap();
            return (/** @type {number[]} */ point) => {
                let result = points.get(point);
                if (!result) { result = projector(point); points.set(point, result); }
                return result;
            };
        };
        const project = cachedProjector(createPointProjector(snapshot, projectionSettings));
        const projectSimultaneous = settings.optical === false ? project : cachedProjector(createPointProjector(snapshot, { ...projectionSettings, optical: false }));
        // Thousands of reference segments share only a few visual styles. One
        // Canvas stroke per style avoids per-edge rasterization/submission while
        // retaining every generated segment and the same observation time.
        /** @type {Map<string,{path:Path2D,color:string,width:number}>} */ const paths = new Map();
        /** @param {string} color @param {number} width */
        const pathFor = (color, width) => {
            const key = `${color}:${width}`;
            let batch = paths.get(key);
            if (!batch) { batch = { path: new Path2D(), color, width }; paths.set(key, batch); }
            return batch.path;
        };
        /** @param {ReturnType<typeof project>} p @param {ReturnType<typeof project>} q @param {string} color @param {number} width */
        const screenLine = (p, q, color, width) => {
            let px = p.x, py = p.y, qx = q.x, qy = q.y;
            if (!p.visible || !q.visible) {
                const clipped = clipProjectedSegment(p, q); if (!clipped) return;
                px = clipped.a.x; py = clipped.a.y; qx = clipped.b.x; qy = clipped.b.y;
            }
            const path = pathFor(color, width);
            path.moveTo(px * this.width, py * this.height); path.lineTo(qx * this.width, qy * this.height);
        };
        /** @param {number[]} a @param {number[]} b @param {string} color @param {number} width @param {ReturnType<typeof createPointProjector>} projection */
        const projectedLine = (a, b, color, width, projection) => {
            screenLine(projection(a), projection(b), color, width);
        };
        /** @param {number[]} a @param {number[]} b @param {string} color @param {number} [width] */
        const line = (a, b, color, width = 1) => projectedLine(a, b, color, width, project);
        /** @param {number[]} a @param {number[]} b */
        const velocityArrow = (a, b) => {
            const p = project(a), q = project(b);
            screenLine(p, q, '#f7ba69', 2);
            // Arrowheads belong at the actual endpoint, never at a clipped edge.
            if (!q.visible) return;
            const start = p.visible ? p : clipProjectedSegment(p, q)?.a; if (!start) return;
            const dx = (q.x - start.x) * this.width, dy = (q.y - start.y) * this.height, length = Math.hypot(dx, dy);
            if (length < 2) return;
            const size = Math.min(7, length * 0.35), x = q.x * this.width, y = q.y * this.height;
            const path = pathFor('#f7ba69', 2);
            for (const side of [-1, 1]) { path.moveTo(x, y); path.lineTo(x - size * (dx + side * dy * 0.6) / length, y - size * (dy - side * dx * 0.6) / length); }
        };
        /** @param {import('./optics.js').OpticalSegment} entity @param {number[]} center @param {string} color @param {boolean} mirrored */
        const bounds = (entity, center, color, mirrored = false) => {
            const rotation = rotationMatrix(entity.rotation), corners = [];
            for (const x of [-0.5, 0.5]) for (const y of [-0.5, 0.5]) for (const z of [-0.5, 0.5]) corners.push(rotate(rotation, [x * entity.size[0], y * entity.size[1], z * entity.size[2]]).map((v, j) => (mirrored && j === 1 ? -v : v) + center[j]));
            for (let a = 0; a < 8; a++) for (const bit of [1, 2, 4]) if (!(a & bit)) line(corners[a], corners[a | bit], color);
        };
        const entities = snapshot.entities.filter(e => e.alive !== false || layers.trajectories || layers.ghosts).sort((a, b) => Number(b.id === settings.selectedId) - Number(a.id === settings.selectedId)).slice(0, 64);
        // Reference trails and frame ghosts must use retained revisions even while
        // the image is in instantaneous comparison mode. They never synthesize a
        // missing past pose from the entity's current state.
        /** @type {Map<string,import('./optics.js').OpticalSegment[]>} */ const retained = new Map();
        if (layers.trajectories || (layers.ghosts && snapshot.profile === 'sr')) {
            for (const entity of entities) retained.set(entity.id || entity.entityId || '', []);
            for (const source of snapshot.segments) retained.get(source.entityId || source.id || '')?.push(source);
        }
        let trailSegments = 0, ghostSegments = 0;
        for (const entity of entities) for (let pass = 0; pass < (settings.mirrorWorld ? 2 : 1); pass++) {
            const mirrored = pass === 1;
            const history = retained.get(entity.id || entity.entityId || '') || [];
            for (let index = history.length - 1; index >= 0; index--) {
                const source = history[index];
                if (layers.trajectories && trailSegments < 1024) {
                    const start = Math.max(snapshot.historyStart, source.start ?? snapshot.historyStart), stop = Math.min(snapshot.time, source.end ?? snapshot.time);
                    if (stop > start && Math.hypot(...source.velocity) > 1e-12) {
                        const position = (/** @type {number} */ time) => source.position.map((x, j) => (x + source.velocity[j] * (time - (source.originTime ?? 0))) * (mirrored && j === 1 ? -1 : 1));
                        line(position(start), position(stop), '#679cbd66'); trailSegments++;
                    }
                }
                if (layers.ghosts && snapshot.profile === 'sr' && ghostSegments < 1536) {
                    for (const edge of simultaneousBoundsEdges(observer, snapshot.time, source, snapshot.historyStart, mirrored)) {
                        if (ghostSegments >= 1536) break;
                        projectedLine(edge.a, edge.b, '#ca9bfd99', 1, projectSimultaneous); ghostSegments++;
                    }
                }
            }
            const apparent = this.apparentSegments(snapshot, settings, entity.id || entity.entityId || '', mirrored)[0]; if (!apparent) continue;
            const { segment, center } = apparent;
            if (layers.bounds || entity.id === settings.selectedId) bounds(segment, center.position, entity.id === settings.selectedId ? '#9efbed' : '#6088a880', mirrored);
            if (layers.vectors) velocityArrow(center.position, center.position.map((x, j) => x + segment.velocity[j] * 4));
            if (layers.clocks) {
                const p = project(center.position);
                if (p.visible) {
                    const path = pathFor('#dceaff', 1), x = p.x * this.width, y = p.y * this.height;
                    path.moveTo(x + 8, y); path.arc(x, y, 8, 0, 2 * Math.PI);
                    const clock = (segment.clockOffset || 0) + (center.emissionTime - (segment.originTime || 0)) / (snapshot.profile === 'sr' ? gamma(segment.velocity) : 1);
                    path.moveTo(x, y); path.lineTo(x + Math.sin(clock * 2 * Math.PI) * 6, y - Math.cos(clock * 2 * Math.PI) * 6);
                }
            }
        }
        // Bounded nearest-neighbour reference-plane grid, independent of authored
        // matter. It is decorative reference geometry, not native lattice records.
        if (layers.bonds) {
            const spacing = Math.max(0.1, this.uniforms.uSpacing.value), x = Math.round(observer.position[0] / spacing) * spacing, z = Math.round(observer.position[2] / spacing) * spacing;
            for (let i = -8; i <= 8; i++) for (let j = -8; j <= 8; j++) {
                const p = [x + i * spacing, 0.015, z + j * spacing];
                if (i < 8) line(p, [p[0] + spacing, p[1], p[2]], '#4ba8c066');
                if (j < 8) line(p, [p[0], p[1], p[2] + spacing], '#4ba8c066');
            }
        }
        // Pulse fronts are privileged geometric instruments, not scattered photons in a vacuum.
        // Their radius is exactly c*(coordinateTime-emissionTime), with c=1.
        // The newest 16 recorded emission events bound presentation cost.
        if (layers.pulses) for (const pulse of (snapshot.pulses || []).slice(-16)) {
            const radius = snapshot.time - pulse.start; if (radius <= 0 || radius > 200) continue;
            const color = `rgba(${pulse.color.map(x => Math.round(x * 255)).join(',')},0.45)`;
            for (let axis = 0; axis < 3; axis++) for (let j = 0; j < 48; j++) {
                const a = j * Math.PI / 24, b = (j + 1) * Math.PI / 24;
                const p = [...pulse.origin], q = [...pulse.origin], u = axis, v = (axis + 1) % 3;
                p[u] += radius * Math.cos(a); p[v] += radius * Math.sin(a); q[u] += radius * Math.cos(b); q[v] += radius * Math.sin(b); line(p, q, color);
            }
        }
        if (settings.authorPreview) bounds(settings.authorPreview, settings.authorPreview.position, '#ffdb92');
        drawWavePhenomena(snapshot, settings, line);
        for (const { path, color, width } of paths.values()) {
            context.strokeStyle = color; context.lineWidth = width; context.stroke(path);
        }
    }
    /** Add genuinely traced seeded static geometry for the declared performance scene. Does not affect the world owner.
     * @param {{staticInstances?:number,uniqueTriangles?:number}} options
     */
    setBenchmarkScene({ staticInstances = 0, uniqueTriangles = 0 } = {}) {
        if (this.disposed) throw new Error('Cannot configure a disposed Observer renderer.');
        if (!Number.isInteger(staticInstances) || staticInstances < 0 || staticInstances > 4096) throw new RangeError('Benchmark static instances must be an integer in [0,4096].');
        if (uniqueTriangles !== this.atlas.triangleBudget) {
            // Construct and validate the replacement before releasing the active GPU atlas.
            const atlas = geometryAtlas(uniqueTriangles), previous = this.atlas;
            const maxTextureSize = this.renderer.capabilities.maxTextureSize;
            for (const texture of [atlas.nodes, atlas.triangles, atlas.meshes]) {
                if (texture.image.width > maxTextureSize || texture.image.height > maxTextureSize) {
                    atlas.nodes.dispose(); atlas.triangles.dispose(); atlas.meshes.dispose();
                    throw new RangeError(`Geometry atlas exceeds this GPU's ${maxTextureSize}-pixel texture size.`);
                }
            }
            this.atlas = atlas;
            this.uniforms.uMeshNodes.value = atlas.nodes; this.uniforms.uTriangles.value = atlas.triangles; this.uniforms.uMeshes.value = atlas.meshes;
            this.diagnostics.uniqueTriangles = atlas.uniqueTriangles; this.diagnostics.meshBvhNodes = atlas.meshBvhNodes; this.diagnostics.requestedTriangleBudget = atlas.triangleBudget;
            previous.nodes.dispose(); previous.triangles.dispose(); previous.meshes.dispose();
        }
        const random = seededRandom(120966);
        this.benchmarkSegments = Array.from({ length: staticInstances }, (_, i) => ({ entityId: `benchmark:${i}`, revision: 0, start: -Infinity, end: null, originTime: 0, position: [(random() - 0.5) * 50, 0.5 + random() * 14, -3 - random() * 45], velocity: [0, 0, 0], size: [0.3, 0.3, 0.3], rotation: [random(), random(), random()], shape: SHAPE_NAMES[i % 16], color: [0.15 + random() * 0.3, 0.4 + random() * 0.4, 0.6 + random() * 0.4], emission: 0.8, clockOffset: 0, decorative: true }));
        this.traceAcceleration = null; this.preparedSnapshot = null;
    }
    getViewControlCapabilities() { return { reset: false, fit: false, orbit: false, pan: false, zoom: false, perspective: true }; }
    getViewControlState() { return { workspace: 'observer', projection: 'perspective', fov: this.uniforms.uFov.value, rayTracing: true, qualityFactor: this.qualityFactor, ...this.diagnostics }; }
    dispose() {
        if (this.disposed) return; this.disposed = true; this.scope.dispose(); this.environment.dispose();
        const gl = /** @type {WebGL2RenderingContext} */ (this.renderer.getContext()); for (const query of this.pendingQueries) gl.deleteQuery(query); this.pendingQueries = [];
        this.material.dispose(); this.geometry.dispose(); this.pickTarget.dispose(); for (const texture of this.textures()) texture.dispose();
        for (const target of this.feedbackTargets) target.dispose();
        this.renderer.dispose(); this.canvas.remove(); this.overlayCanvas.remove(); this.benchmarkSegments = []; this.segments = [];
        this.historiesById.clear(); this.apparentSegmentsById.clear();
    }
}
