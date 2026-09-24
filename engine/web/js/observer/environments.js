/** Instance-owned Observer environments. Physical shells and distant radiance have separate contracts. */
import * as THREE from 'three';
import { RGBELoader } from 'three/addons/loaders/RGBELoader.js';
import { HDRI_ENVIRONMENTS } from '../backgrounds/hdri-loader.js';
import { restMesh } from './geometry.js';
import { FRACTAL_PRESETS } from './fractal-presets.js';

const geometricNames = ['cubic-grid', 'wireframe-chamber', 'concentric-spheres', 'nested-cubes', 'polyhedral-cathedral', 'octahedral-halls', 'cuboctahedral-lattice', 'stella-octangula', 'radial-spokes', 'hexagonal-tunnel', 'crystalline-corridor', 'layered-planes'];
export const OBSERVER_ENVIRONMENTS = Object.freeze([
    { id: 'void', kind: 'radiance' }, ...['stars', 'nebula', 'foam', 'beyond', 'storm'].map(id => ({ id, kind: 'radiance' })),
    ...Object.keys(HDRI_ENVIRONMENTS).map(id => ({ id, kind: 'panorama' })),
    ...geometricNames.map(id => ({ id, kind: 'geometry' })),
    ...FRACTAL_PRESETS.map(({ id }) => ({ id, kind: 'radiance' })),
]);
export const MAX_ENVIRONMENT_INSTANCES = 2048;
const LOCAL_PANORAMA_BASE = new URL('../../assets/observer/environments/', import.meta.url);
/** @param {number} seed */
export function seededRandom(seed) { let state = seed >>> 0; return () => { state += 0x6D2B79F5; let x = state; x = Math.imul(x ^ x >>> 15, x | 1); x ^= x + Math.imul(x ^ x >>> 7, x | 61); return ((x ^ x >>> 14) >>> 0) / 4294967296; }; }

/** @typedef {{preset?:string,seed?:number,radius?:number,density?:number,spacing?:number,orientation?:number,opacity?:number,color?:number[],animationRate?:number,anchor?:string}} EnvironmentSettings */
/** @param {EnvironmentSettings} environment @returns {import('./optics.js').OpticalSegment[]} */
export function buildEnvironmentGeometry(environment) {
    const preset = environment.preset || 'void';
    if (!geometricNames.includes(preset) || environment.anchor === 'camera' || environment.opacity === 0 || environment.density === 0) return [];
    const radius = Math.max(2, Math.min(500, environment.radius || 40)), density = Math.max(0.1, Math.min(4, environment.density || 1)), spacing = Math.max(0.25, environment.spacing || 4);
    const color = environment.color || [0.2, 0.7, 1], orientation = environment.orientation || 0, thickness = Math.max(0.018, radius * 0.0006);
    const random = seededRandom(environment.seed || 1);
    /** @type {import('./optics.js').OpticalSegment[]} */ const entities = [];
    const turn = (/** @type {number[]} */ p) => [p[0] * Math.cos(orientation) - p[2] * Math.sin(orientation), p[1], p[0] * Math.sin(orientation) + p[2] * Math.cos(orientation)];
    /** @param {number[]} a @param {number[]} b @param {number} brightness */
    function edge(a, b, brightness = 1) {
        if (entities.length >= MAX_ENVIRONMENT_INSTANCES) throw new RangeError(`Environment exceeds ${MAX_ENVIRONMENT_INSTANCES} physical instances; lower density or increase spacing.`);
        a = turn(a); b = turn(b);
        const direction = new THREE.Vector3(...b).sub(new THREE.Vector3(...a));
        const rotation = new THREE.Euler().setFromQuaternion(new THREE.Quaternion().setFromUnitVectors(new THREE.Vector3(0, 1, 0), direction.clone().normalize()));
        entities.push({ entityId: `environment:${preset}:${entities.length}`, revision: 0, start: -Infinity, end: null, originTime: 0, position: a.map((x, j) => (x + b[j]) * 0.5), velocity: [0, 0, 0], size: [thickness, direction.length(), thickness], rotation: [rotation.x, rotation.y, rotation.z], shape: 'box', color, emission: brightness * (environment.opacity ?? 1), clockOffset: 0, decorative: false, animationRate: environment.animationRate || 0 });
    }
    /** @param {number[][]} vertices @param {number[][]} edges @param {number[]} origin @param {number} scale */
    function wire(vertices, edges, origin, scale) { for (const [a, b] of edges) edge(vertices[a].map((x, j) => origin[j] + x * scale), vertices[b].map((x, j) => origin[j] + x * scale)); }
    /** @param {string} shape @param {number[]} origin @param {number} scale */
    function poly(shape, origin, scale) {
        const { vertices, triangles } = restMesh(shape); const unique = new Set();
        for (const t of triangles) for (const [a, b] of [[t[0], t[1]], [t[1], t[2]], [t[2], t[0]]]) {
            const key = a < b ? `${a}:${b}` : `${b}:${a}`; if (unique.has(key)) continue; unique.add(key); edge(vertices[a].map((x, j) => origin[j] + x * scale), vertices[b].map((x, j) => origin[j] + x * scale));
        }
    }
    /** @param {number} r @param {number} y @param {number} sides */
    function ring(r, y, sides = 64) { for (let j = 0; j < sides; j++) { const a = j * 2 * Math.PI / sides, b = (j + 1) * 2 * Math.PI / sides; edge([r * Math.cos(a), y, r * Math.sin(a)], [r * Math.cos(b), y, r * Math.sin(b)]); } }
    if (preset === 'cubic-grid' || preset === 'wireframe-chamber' || preset === 'layered-planes') {
        const cells = Math.min(10, Math.max(2, Math.round(radius / spacing * density)));
        for (let a = -cells; a <= cells; a++) for (let b = preset === 'layered-planes' ? 0 : -cells; b <= (preset === 'layered-planes' ? 0 : cells); b++) {
            const x = a * radius / cells, z = b * radius / cells;
            if (preset !== 'wireframe-chamber' || Math.abs(a) === cells || Math.abs(b) === cells) {
                edge([-radius, x, z], [radius, x, z]); edge([x, -radius, z], [x, radius, z]); edge([x, z, -radius], [x, z, radius]);
            }
        }
    } else if (preset === 'concentric-spheres') {
        for (let r = 1; r <= Math.min(6, Math.ceil(density * 3)); r++) {
            const size = radius * r / Math.ceil(density * 3);
            for (let latitude = -2; latitude <= 2; latitude++) ring(size * Math.cos(latitude * Math.PI / 6), size * Math.sin(latitude * Math.PI / 6), 48);
        }
    } else if (preset === 'nested-cubes') {
        for (let j = 1; j <= Math.ceil(density * 5); j++) poly('box', [0, 0, 0], radius * 2 * j / Math.ceil(density * 5));
    } else if (preset === 'polyhedral-cathedral' || preset === 'octahedral-halls' || preset === 'crystalline-corridor') {
        const count = Math.min(18, Math.ceil(6 * density));
        for (let j = 0; j < count; j++) for (const side of [-1, 1]) poly(preset === 'polyhedral-cathedral' ? 'dodecahedron' : preset === 'octahedral-halls' ? 'octahedron' : 'icosahedron', [side * spacing * 2.5, spacing * 1.4, -j * spacing * 2], spacing * (1.5 + random()));
    } else if (preset === 'cuboctahedral-lattice') {
        /** @type {number[][]} */ const vertices = []; for (let axis = 0; axis < 3; axis++) for (const a of [-1, 1]) for (const b of [-1, 1]) { const p = [a, b]; p.splice(axis, 0, 0); vertices.push(p); }
        const edges = []; for (let a = 0; a < vertices.length; a++) for (let b = a + 1; b < vertices.length; b++) if (vertices[a].reduce((sum, x, j) => sum + (x - vertices[b][j]) ** 2, 0) === 2) edges.push([a, b]);
        const cells = Math.min(2, Math.ceil(density));
        for (let x = -cells; x <= cells; x++) for (let z = -cells; z <= cells; z++) wire(vertices, edges, [x * spacing * 4, spacing * 2, z * spacing * 4], spacing);
    } else if (preset === 'stella-octangula') {
        const vertices = restMesh('tetrahedron').vertices, edges = [[0, 1], [0, 2], [0, 3], [1, 2], [1, 3], [2, 3]];
        for (let j = 1; j <= Math.ceil(density * 4); j++) { wire(vertices, edges, [0, radius / 3, 0], radius * j / Math.ceil(density * 4)); wire(vertices.map(v => v.map(x => -x)), edges, [0, radius / 3, 0], radius * j / Math.ceil(density * 4)); }
    } else if (preset === 'radial-spokes') {
        for (let j = 0; j < Math.ceil(density * 72); j++) { const a = random() * Math.PI * 2, y = random() * 2 - 1, r = Math.sqrt(1 - y * y); const v = [r * Math.cos(a), y, r * Math.sin(a)]; edge(v.map(x => x * radius * 0.3), v.map(x => x * radius)); }
    } else if (preset === 'hexagonal-tunnel') {
        const count = Math.min(100, Math.ceil(radius / spacing * density));
        for (let j = 0; j < count; j++) for (let k = 0; k < 6; k++) {
            const a = k * Math.PI / 3, b = (k + 1) * Math.PI / 3;
            const p = [spacing * 2 * Math.cos(a), spacing * 2 + spacing * 2 * Math.sin(a), -j * spacing];
            edge(p, [spacing * 2 * Math.cos(b), spacing * 2 + spacing * 2 * Math.sin(b), -j * spacing]);
            if (j < count - 1) edge(p, [p[0], p[1], p[2] - spacing]);
        }
    }
    return entities;
}

export class ObserverEnvironment {
    constructor() {
        this.key = ''; this.preset = 'void'; this.status = 'ready'; this.message = '';
        /** @type {THREE.DataTexture|null} */ this.texture = null;
        /** @type {AbortController|null} */ this.abort = null;
        /** @type {import('./optics.js').OpticalSegment[]} */ this.segments = [];
        /** @type {Map<string,import('./optics.js').OpticalSegment[]>} */ this.geometryCache = new Map();
        this.disposed = false; this.generation = 0;
    }
    /** @param {EnvironmentSettings} environment @param {{start:number,end:number|null,revision:number,environment:EnvironmentSettings}[]|undefined} history */
    update(environment = {}, history) {
        const key = JSON.stringify([environment, history]); if (key === this.key || this.disposed) return;
        this.key = key;
        try {
            const retained = new Set();
            this.segments = (history || [{ environment, revision: 0, start: -Infinity, end: null }]).flatMap(entry => {
                const configKey = JSON.stringify(entry.environment); retained.add(configKey);
                if (!this.geometryCache.has(configKey)) this.geometryCache.set(configKey, buildEnvironmentGeometry(entry.environment));
                return (this.geometryCache.get(configKey) || []).map(segment => ({ ...segment, revision: entry.revision, start: entry.start, end: entry.end }));
            });
            for (const cached of this.geometryCache.keys()) if (!retained.has(cached)) this.geometryCache.delete(cached);
            this.message = '';
        }
        catch (error) { this.message = error instanceof Error ? error.message : String(error); throw error; }
        const preset = environment.preset || 'void'; if (preset === this.preset) return;
        this.preset = preset; this.generation++; this.abort?.abort(); this.texture?.dispose(); this.texture = null;
        this.status = 'ready';
        const descriptor = HDRI_ENVIRONMENTS[/** @type {keyof typeof HDRI_ENVIRONMENTS} */ (preset)];
        if (descriptor) void this.loadPanorama(descriptor.file, this.generation);
    }
    /** @param {string} file @param {number} generation */
    async loadPanorama(file, generation) {
        const controller = new AbortController(); this.abort = controller; this.status = 'loading';
        try {
            const response = await fetch(new URL(file, LOCAL_PANORAMA_BASE), { signal: controller.signal });
            if (!response.ok) throw new Error(`Panorama request failed (${response.status}).`);
            const parsed = new RGBELoader().parse(await response.arrayBuffer());
            if (this.disposed || generation !== this.generation || controller.signal.aborted) return;
            const texture = new THREE.DataTexture(/** @type {BufferSource} */ (parsed.data), parsed.width, parsed.height, THREE.RGBAFormat, parsed.type);
            texture.minFilter = THREE.LinearFilter; texture.magFilter = THREE.LinearFilter; texture.wrapS = THREE.RepeatWrapping; texture.needsUpdate = true;
            this.texture = texture; this.status = 'ready';
        } catch (error) {
            if (controller.signal.aborted || this.disposed || generation !== this.generation) return;
            this.status = 'fallback'; this.message = `Panorama unavailable. Procedural sky active. ${error instanceof Error ? error.message : error}`;
        }
    }
    dispose() { this.disposed = true; this.generation++; this.abort?.abort(); this.texture?.dispose(); this.texture = null; this.segments = []; this.geometryCache.clear(); }
}
