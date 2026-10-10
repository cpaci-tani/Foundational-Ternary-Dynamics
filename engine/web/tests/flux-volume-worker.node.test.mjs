// Flux-volume off-thread pipeline: the kernel must equal the renderer's own
// synchronous path bit for bit, and the worker glue (snapshot, latest-wins,
// buffer recycling, cancellation, failure fallback) must commit exactly what
// the kernel computes. The worker is replaced by an in-process stand-in that
// runs the real worker script and applies real transfer (detach) semantics.
import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import vm from 'node:vm';
import * as THREE from '../js/vendor/three/build/three.module.js';
import { fluxToColorInto, fluxToColor } from '../js/fields.js';
import { computeFluxActivation, createFluxActivationStepper } from '../js/viewport/flux-activation.js';
import { copyScalarActivation } from '../js/viewport/scalar-activation.js';
import { clampFluxThreshold, DEFAULT_FLUX_THRESHOLD } from '../js/viewport/flux-threshold.js';
import { computeFluxVolumeFrame } from '../js/viewport/flux-volume-kernel.js';
import { FluxVolumeWorkerClient } from '../js/viewport/flux-volume-worker-client.js';

function load(path, expression, extra = {}) {
    const url = new URL('../js/' + path, import.meta.url);
    const src = readFileSync(url, 'utf8')
        .replace(/^import[\s\S]*?from\s+['"][^'"]+['"];\s*/gm, '')
        .replace(/export\s+/g, '')
        .replace(/import\.meta\.url/g, JSON.stringify(url.href));
    return vm.runInNewContext(src + '\n' + expression, { URL, ...extra });
}

// ── In-process stand-in for the module worker ──────────────────────────
const fake = { jobs: [], posted: [], failNext: false, terminated: 0, run: () => false };
function makeFakeWorker() {
    const worker = {
        onmessage: null,
        onerror: null,
        onmessageerror: null,
        postMessage(message, transfer = []) {
            const copy = structuredClone(message, { transfer });
            fake.posted.push({
                id: copy.id,
                shared: !!copy.shared,
                transfers: transfer.length,
                recycled: !!copy.colors,
                stateMask: !!copy.stateMask,
                insideMask: !!copy.insideMask,
            });
            fake.jobs.push(copy);
        },
        terminate() { fake.terminated++; },
    };
    const scope = {
        onmessage: null,
        postMessage(message, transfer = []) {
            worker.onmessage?.({ data: structuredClone(message, { transfer }) });
        },
    };
    load('viewport/flux-volume-worker.js', '0', { self: scope, computeFluxVolumeFrame, performance });
    fake.run = () => {
        const job = fake.jobs.shift();
        if (!job) return false;
        if (fake.failNext) {
            fake.failNext = false;
            worker.onmessage({ data: { type: 'error', id: job.id, message: 'injected failure' } });
        } else {
            scope.onmessage({ data: job });
        }
        return true;
    };
    return worker;
}
class TestClient extends FluxVolumeWorkerClient {
    constructor(options) { super({ ...options, workerFactory: makeFakeWorker }); }
}

const rafQueue = [];
const warnings = [];
const R = load(
    'viewport/flux-renderer.js',
    '({ViewportFluxRenderer, FLUX_LATTICE_MIN_POINT_SIZE, FLUX_LATTICE_INSPECTION_COLOR_FLOOR,'
        + ' FLUX_PEAK_HOLD_DECAY, FLUX_ASYNC_SOURCE_COUNT})',
    {
        THREE,
        attachBackToFrontOrdering: () => () => {},
        fluxToColorInto,
        fluxToColor,
        FLUX_VOL_VERT: 'void main(){}',
        PARTICLE_FRAG: 'void main(){}',
        PARTICLE_SHADER_UNIFORMS: { shapeType: { value: 0 } },
        clampFluxThreshold,
        DEFAULT_FLUX_THRESHOLD,
        computeFluxActivation,
        createFluxActivationStepper,
        copyScalarActivation,
        FluxVolumeWorkerClient: TestClient,
        performance,
        console: { ...console, warn: (...args) => warnings.push(args.join(' ')) },
        requestAnimationFrame: (callback) => rafQueue.push(callback),
        cancelAnimationFrame: () => {},
    },
);

const inSphere = (x, y, z) => x * x + y * y + z * z <= 1;
const isSharedArray = (array) => Object.prototype.toString.call(array.buffer) === '[object SharedArrayBuffer]';

function makeRenderer(N, { shape = 'cube', threshold = 0, shared = true } = {}) {
    const renderer = new R.ViewportFluxRenderer({
        scene: new THREE.Scene(),
        latticeSize: N,
        halfN: N / 2,
        boundaryShape: shape,
        insideBoundary: shape === 'sphere' ? inSphere : () => true,
        applyScenarioScale: () => {},
        buildStreamlineMesh: () => null,
        writeStreamlinesIntoMesh: () => {},
    });
    renderer.setFluxThreshold(threshold);
    renderer.setFluxVolumeSharedMemory(shared);
    return renderer;
}

// Deterministic field: two Gaussian lobes plus low-level noise.
function field(axisCount, seed, scale = 1, Type = Float32Array) {
    const out = new Type(axisCount ** 3);
    let state = (seed * 2654435761) >>> 0;
    const c = axisCount / 2;
    const s2 = (axisCount * 0.14) ** 2;
    let i = 0;
    for (let z = 0; z < axisCount; z++) {
        for (let y = 0; y < axisCount; y++) {
            for (let x = 0; x < axisCount; x++, i++) {
                state = (Math.imul(state, 1664525) + 1013904223) >>> 0;
                const r2 = (x - c) ** 2 + (y - c) ** 2 + (z - c) ** 2;
                const r2b = (x - c * 1.5) ** 2 + (y - c * 0.6) ** 2 + (z - c * 1.1) ** 2;
                out[i] = scale * (Math.exp(-r2 / (2 * s2)) + 0.4 * Math.exp(-r2b / s2)
                    + 0.01 * (state / 4294967296));
            }
        }
    }
    return out;
}

// The strongest energy sits in a corner, outside the inscribed sphere: a
// shaped boundary must hide it AND keep it out of the normalisation.
function hotCorner(density, N, value) {
    for (let z = 0; z < 2; z++) {
        for (let y = 0; y < 2; y++) {
            for (let x = 0; x < 2; x++) density[(z * N + y) * N + x] = value;
        }
    }
}
function assertCornerClipped(expected, thresholdFraction) {
    const cornerMax = expected.activation[0];
    assert.ok(cornerMax > expected.instantMaxActivation, 'corner outshines every drawable site');
    assert.ok((cornerMax / expected.instantMaxActivation) ** 2 >= thresholdFraction);
    assert.equal(expected.visibilities[0], 0, 'corner is clipped, not thresholded');
    assert.ok(expected.visibleCount > 0, 'drawable sites still pass a cutoff taken inside the boundary');
}

const bits = (array) => Buffer.from(array.buffer, array.byteOffset, array.byteLength);
function assertSameBits(actual, expected, label) {
    assert.equal(actual.length, expected.length, `${label}: length`);
    assert.ok(bits(actual).equals(bits(expected)), `${label}: values differ`);
}

function frameMeta(renderer, N, sourceN, { compactSpacing = 1, compactOrigin = 0, compact = false } = {}) {
    return {
        N,
        sourceN,
        sourceCount: sourceN ** 3,
        compact,
        compactSpacing,
        compactOrigin,
        needsClip: renderer._boundaryShape === 'sphere',
        boundaryCenter: N / 2,
        boundaryRadius: N / 2,
    };
}

/** The kernel's answer for one frame, using the renderer's own constants and masks. */
function reference(renderer, density, meta, peakHold, stateMask = null) {
    const count = meta.sourceCount;
    const out = {
        scratchA: new Float64Array(count),
        scratchB: new Float64Array(count),
        activation: new Float64Array(count),
        colors: new Float32Array(count * 3),
        sizes: new Float32Array(count),
        visibilities: new Float32Array(count),
    };
    const summary = computeFluxVolumeFrame({
        density,
        sourceN: meta.sourceN,
        stateMask: stateMask || new Uint8Array(count),
        insideMask: meta.needsClip ? renderer._fluxBoundaryMask(meta) : null,
        thresholdFraction: clampFluxThreshold(renderer._fluxThreshold),
        peakHold,
        peakHoldDecay: R.FLUX_PEAK_HOLD_DECAY,
        pointFloor: R.FLUX_LATTICE_MIN_POINT_SIZE,
        pointCeiling: renderer._fluxPointCeiling(meta),
        colorFloor: R.FLUX_LATTICE_INSPECTION_COLOR_FLOOR,
    }, out);
    return { ...out, ...summary };
}

function assertCommitted(renderer, expected, label) {
    const geometry = renderer._fluxVolume.geometry;
    assertSameBits(geometry.getAttribute('particleColor').array, expected.colors, `${label} colour`);
    assertSameBits(geometry.getAttribute('size').array, expected.sizes, `${label} size`);
    assertSameBits(geometry.getAttribute('particleVisibility').array, expected.visibilities, `${label} visibility`);
    assertSameBits(renderer._fluxActivation, expected.activation, `${label} activation`);
    assert.equal(renderer._fluxVisibleCount, expected.visibleCount, `${label} visible count`);
    assert.equal(renderer._fluxMaxDecay, expected.maxActivation, `${label} normaliser`);
    assert.equal(geometry.drawRange.count, expected.sizes.length, `${label} draw range`);
}

test('kernel equals the synchronous renderer path bit for bit', () => {
    // Dense cube, threshold zero: three frames exercise attack, hold and release.
    const N = 12;
    const cube = makeRenderer(N);
    for (const [seed, scale] of [[1, 1], [2, 0.3], [3, 2]]) {
        const density = field(N, seed, scale);
        const before = cube._fluxMaxDecay;
        cube.updateFluxVolume(density, N);
        assertCommitted(cube, reference(cube, density, frameMeta(cube, N, N), before), `cube seed ${seed}`);
    }

    // Shaped boundary, threshold, manifested sites, non-finite and negative input.
    const shaped = makeRenderer(N, { shape: 'sphere', threshold: 0.25 });
    const particles = {
        positions: new Float32Array([6.2, 6.7, 6.1, 3.5, 8.5, 5.5, 40, 1, 1]),
        colors: new Float32Array([0.1, 0.9, 0.1, 0.9, 0.1, 0.1, 0, 0, 0]),
        locked: new Uint8Array([0, 1, 0]),
        count: 3,
    };
    for (const [seed, scale] of [[4, 1], [5, 0.5]]) {
        const density = field(N, seed, scale, Float64Array);
        density[5] = NaN;
        density[N * N * 6 + N * 6 + 2] = Infinity;
        density[N * N * 5 + N * 6 + 6] = -1;
        hotCorner(density, N, 4 * scale);
        const before = shaped._fluxMaxDecay;
        shaped.updateFluxVolume(density, N, particles);
        const expected = reference(
            shaped, density, frameMeta(shaped, N, N), before, shaped._fluxStateMask.slice(),
        );
        assert.equal(expected.manifestedCount, 2, 'two particles map inside the grid');
        assertCommitted(shaped, expected, `sphere seed ${seed}`);
        assertCornerClipped(expected, 0.25);
    }

    // Compact native descriptor: coarser source grid with a stride.
    const compactN = 17;
    const axisCount = 9;
    const compact = makeRenderer(compactN, { threshold: 0.1 });
    const data = field(axisCount, 6);
    const before = compact._fluxMaxDecay;
    compact.updateFluxVolume({ data, latticeSize: compactN, stride: 2, origin: 0, axisCount }, compactN);
    assertCommitted(
        compact,
        reference(compact, data, frameMeta(compact, compactN, axisCount, { compact: true, compactSpacing: 2 }), before),
        'compact',
    );
});

// Both buffer modes must commit the same frames: shared memory (the normal
// mode) and transferred buffers (pages that are not cross-origin isolated).
for (const shared of [true, false]) test(`worker path, ${shared ? 'shared memory' : 'transferred buffers'}: snapshot, latest-wins, recycling, cancel`, () => {
    const N = 59; // 205,379 sources: above the off-thread threshold.
    assert.ok(N ** 3 > R.FLUX_ASYNC_SOURCE_COUNT);
    const renderer = makeRenderer(N, { threshold: 0.02, shared });
    const meta = frameMeta(renderer, N, N);
    const colourBuffers = new Set();
    const noteColours = () => colourBuffers.add(renderer._fluxVolume.geometry.getAttribute('particleColor').array.buffer);
    fake.posted.length = 0;

    // The input is copied when queued; later producer writes cannot leak in.
    const first = field(N, 11);
    const firstCopy = first.slice();
    renderer.updateFluxVolume(first, N);
    assert.equal(renderer._fluxAsyncJob?.worker, true, 'request in flight');
    assert.equal(renderer._fluxVolume.geometry.drawRange.count, 0, 'nothing drawn before the commit');
    assert.equal(first.length, N ** 3, 'the producer buffer is never transferred');
    first.fill(99);
    let expected = reference(renderer, firstCopy, meta, 0);
    assert.ok(fake.run());
    assert.equal(renderer._fluxAsyncJob, null);
    assert.equal(renderer._fluxPendingFrame, null);
    assertCommitted(renderer, expected, 'first frame');
    assertSameBits(renderer._fluxDensitySnapshot, firstCopy, 'published density snapshot');
    noteColours();
    assert.equal(isSharedArray(renderer._fluxVolume.geometry.getAttribute('particleColor').array), shared);
    assert.equal(isSharedArray(renderer._fluxActivation), shared);
    assert.equal(isSharedArray(renderer._fluxDensitySnapshot), shared);
    const source = renderer._fluxVolume.geometry.getAttribute('sourcePosition').array;
    assert.deepEqual(Array.from(source.slice(0, 6)), [0.5, 0.5, 0.5, 1.5, 0.5, 0.5], 'positions written once');

    // Three updates while the worker is busy: the middle one is never computed.
    const second = field(N, 12, 0.4);
    const third = field(N, 13, 5);
    const fourth = field(N, 14, 0.7);
    renderer.updateFluxVolume(second, N);
    renderer.updateFluxVolume(third, N);
    renderer.updateFluxVolume(fourth, N);
    assert.equal(fake.jobs.length, 1, 'one request in flight');
    assert.ok(renderer._fluxPendingFrame?.worker, 'latest frame waits');
    expected = reference(renderer, second, meta, expected.maxActivation);
    assert.ok(fake.run());
    assertCommitted(renderer, expected, 'second frame');
    noteColours();
    assert.equal(renderer._fluxAsyncJob?.worker, true, 'waiting frame dispatched on commit');
    expected = reference(renderer, fourth, meta, expected.maxActivation);
    assert.ok(fake.run());
    assertCommitted(renderer, expected, 'fourth frame');
    noteColours();
    assert.equal(fake.posted.length, 3, 'first, second and fourth only');
    assert.deepEqual(fake.posted.map((job) => job.shared), [shared, shared, shared]);
    if (shared) {
        // Nothing crosses by transfer, and the main thread supplies every buffer.
        assert.deepEqual(fake.posted.map((job) => job.transfers), [0, 0, 0]);
        assert.deepEqual(fake.posted.map((job) => job.recycled), [true, true, true]);
        assert.equal(colourBuffers.size, 2, 'two output sets alternate between screen and worker');
    } else {
        assert.deepEqual(fake.posted.map((job) => job.transfers), [1, 5, 5], 'density, then density + four outputs');
        assert.deepEqual(fake.posted.map((job) => job.recycled), [false, true, true], 'output arrays ping-pong');
    }
    assert.deepEqual(fake.posted.map((job) => job.stateMask || job.insideMask), [false, false, false]);

    // Hiding the volume while a request is in flight commits nothing.
    const shownColors = renderer._fluxVolume.geometry.getAttribute('particleColor').array;
    renderer.updateFluxVolume(field(N, 15, 9), N);
    renderer.toggleFluxVolume(false);
    assert.equal(renderer._fluxAsyncJob, null);
    assert.ok(fake.run(), 'the stale request still completes in the worker');
    assert.equal(renderer._fluxVolume.geometry.getAttribute('particleColor').array, shownColors);
    assert.equal(renderer._fluxVolume.geometry.drawRange.count, 0);
    assert.equal(renderer._fluxMaxDecay, expected.maxActivation, 'stale result leaves the normaliser alone');
    renderer.toggleFluxVolume(true);
    assert.equal(renderer._fluxVolume.geometry.drawRange.count, N ** 3);

    // The buffers of the stale result are reused, not leaked.
    const fifth = field(N, 16);
    renderer.updateFluxVolume(fifth, N);
    assert.equal(fake.posted.at(-1).recycled, true);
    expected = reference(renderer, fifth, meta, expected.maxActivation);
    assert.ok(fake.run());
    assertCommitted(renderer, expected, 'fifth frame');
    noteColours();
    if (shared) assert.equal(colourBuffers.size, 2, 'no further output set after a stale result');

    renderer.dispose();
    assert.equal(renderer._fluxWorkerClient, null);
});

for (const shared of [true, false]) test(`worker path, ${shared ? 'shared memory' : 'transferred buffers'}: boundary mask and manifested sites`, () => {
    const N = 59;
    const renderer = makeRenderer(N, { shape: 'sphere', threshold: 0.05, shared });
    const meta = frameMeta(renderer, N, N);
    const particles = {
        positions: new Float32Array([29.5, 29.5, 29.5, 20.2, 31.9, 27.1]),
        colors: new Float32Array([0.9, 0.1, 0.1, 0.1, 0.9, 0.1]),
        locked: new Uint8Array([1, 0]),
        count: 2,
    };
    fake.posted.length = 0;
    const density = field(N, 21);
    hotCorner(density, N, 4);
    renderer.updateFluxVolume(density, N, particles);
    assert.deepEqual(
        { stateMask: fake.posted[0].stateMask, insideMask: fake.posted[0].insideMask },
        { stateMask: true, insideMask: true },
    );
    const expected = reference(renderer, density, meta, 0, renderer._fluxStateMask.slice());
    assert.ok(fake.run());
    assertCommitted(renderer, expected, 'sphere with sites');
    assert.equal(expected.manifestedCount, 2);
    const at = (x, y, z) => (z * N + y) * N + x;
    assert.equal(renderer._fluxSiteKind[at(29, 29, 29)], -2, 'locked red site keeps its kind');
    assert.equal(renderer._fluxSiteKind[at(20, 31, 27)], 1, 'green site keeps its kind');
    assertCornerClipped(expected, 0.05);

    // A second frame reuses the masks; in shared mode nothing is transferred.
    const next = field(N, 22, 0.6);
    hotCorner(next, N, 2);
    renderer.updateFluxVolume(next, N, particles);
    const second = reference(renderer, next, meta, expected.maxActivation, renderer._fluxStateMask.slice());
    assert.ok(fake.run());
    assertCommitted(renderer, second, 'second sphere frame');
    assert.deepEqual(fake.posted.map((job) => job.transfers), shared ? [0, 0] : [2, 6]);
    renderer.dispose();
});

test('worker failure degrades in two steps: transferred buffers, then the cooperative path', () => {
    const N = 59;
    const renderer = makeRenderer(N, { threshold: 0.02 });
    const meta = frameMeta(renderer, N, N);
    const density = field(N, 31);
    warnings.length = 0;
    fake.posted.length = 0;

    // Step 1: the shared-memory worker fails. A new worker takes the same
    // frame by transfer; the frame is not lost.
    fake.failNext = true;
    renderer.updateFluxVolume(density, N);
    assert.equal(fake.posted[0].shared, true);
    const firstClient = renderer._fluxWorkerClient;
    assert.ok(fake.run());
    assert.equal(warnings.length, 1, 'one warning');
    assert.match(warnings[0], /retrying with transferred buffers/);
    assert.notEqual(renderer._fluxWorkerClient, firstClient, 'the failed worker was replaced');
    assert.equal(firstClient.available, false);
    assert.equal(renderer._fluxAsyncJob?.worker, true, 'the frame is back in flight');
    assert.deepEqual([fake.posted.length, fake.posted[1].shared, fake.posted[1].transfers], [2, false, 1]);
    let expected = reference(renderer, density, meta, 0);
    assert.ok(fake.run());
    assertCommitted(renderer, expected, 'frame replayed by transfer');
    assert.equal(isSharedArray(renderer._fluxActivation), false);

    // Step 2: the transfer worker fails too. The cooperative path replays the frame.
    const next = field(N, 32, 0.5);
    fake.failNext = true;
    renderer.updateFluxVolume(next, N);
    assert.ok(fake.run());
    assert.equal(warnings.length, 2);
    assert.match(warnings[1], /using the main-thread path/);
    assert.equal(renderer._fluxWorkerClient.available, false);
    assert.ok(renderer._fluxAsyncJob && !renderer._fluxAsyncJob.worker, 'cooperative job replays the frame');
    expected = reference(renderer, next, meta, expected.maxActivation);
    for (let guard = 0; guard < 10000 && renderer._fluxAsyncJob; guard++) rafQueue.shift()?.();
    assert.equal(renderer._fluxAsyncJob, null);
    assertCommitted(renderer, expected, 'frame replayed on the main thread');

    // Later frames never touch a worker again.
    const posted = fake.posted.length;
    const later = field(N, 33, 0.8);
    renderer.updateFluxVolume(later, N);
    assert.equal(fake.posted.length, posted);
    expected = reference(renderer, later, meta, expected.maxActivation);
    for (let guard = 0; guard < 10000 && renderer._fluxAsyncJob; guard++) rafQueue.shift()?.();
    assertCommitted(renderer, expected, 'fallback frame');
    renderer.dispose();
});

test('setFluxVolumeWorkerEnabled(false) keeps a renderer on the cooperative path', () => {
    const N = 59;
    const renderer = makeRenderer(N);
    renderer.setFluxVolumeWorkerEnabled(false);
    fake.posted.length = 0;
    const density = field(N, 41);
    renderer.updateFluxVolume(density, N);
    assert.equal(fake.posted.length, 0);
    assert.equal(renderer._fluxWorkerClient, null, 'no worker is created');
    for (let guard = 0; guard < 10000 && renderer._fluxAsyncJob; guard++) rafQueue.shift()?.();
    assertCommitted(renderer, reference(renderer, density, frameMeta(renderer, N, N), 0), 'cooperative frame');
    renderer.dispose();
});
