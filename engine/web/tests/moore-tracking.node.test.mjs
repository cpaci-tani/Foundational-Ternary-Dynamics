// Moore-neighbourhood tracking: the index arithmetic behind the on-canvas
// readouts, the write-only-on-change contract of the readout components, and
// the bounded panel history. Each test pins a defect measured on 2026-10-10
// (see engine/web/docs/audits/AUDIT_WEB_ENGINE_PERFORMANCE_2026-10-09.md §7).
import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { siteIndex } from '../js/link-geometry.js';
import {
    fluxCellIndex, fluxPointIndex, mooreCellIndices, nearestVisiblePoint,
} from '../js/viewport/flux-point-grid.js';
import { makeMooreTrace } from '../js/scales/scale0/ui/overlays/moore-trace.js';

// The flux renderer's own layout: x fastest, cell centres at c + 0.5.
function grid(L) {
    const source = new Float32Array(L ** 3 * 3);
    let i = 0;
    for (let z = 0; z < L; z++) {
        for (let y = 0; y < L; y++) {
            for (let x = 0; x < L; x++, i += 3) {
                source[i] = x + 0.5;
                source[i + 1] = y + 0.5;
                source[i + 2] = z + 0.5;
            }
        }
    }
    return source;
}

// Organic mode: every dot moved by up to half a cell, clamped to the lattice.
function jittered(source, L, seed = 7) {
    const points = new Float32Array(source.length);
    let state = seed >>> 0;
    const next = () => { state = (Math.imul(state, 1664525) + 1013904223) >>> 0; return state / 4294967296; };
    for (let i = 0; i < source.length; i++) {
        points[i] = Math.max(0.5, Math.min(L - 0.5, source[i] + (next() - 0.5)));
    }
    return points;
}

test('the point cloud is x-fastest; the link sample order is a different order', () => {
    const L = 7;
    const source = grid(L);
    const count = L ** 3;
    for (const [x, y, z] of [[0, 0, 0], [3, 5, 6], [6, 0, 2], [1, 1, 1], [2, 4, 0]]) {
        const index = fluxCellIndex(source, count, L, null, x, y, z);
        assert.equal(index, fluxPointIndex(L, x, y, z));
        assert.deepEqual([source[index * 3], source[index * 3 + 1], source[index * 3 + 2]], [x + 0.5, y + 0.5, z + 0.5]);
    }
    // The defect: indexing the cloud with the link order lands on cell (z, y, x).
    const wrong = siteIndex(L, 3, 5, 6);
    assert.deepEqual(
        [source[wrong * 3], source[wrong * 3 + 1], source[wrong * 3 + 2]],
        [6.5, 5.5, 3.5],
    );
    assert.equal(fluxCellIndex(source, count, L, null, -1, 0, 0), -1, 'outside the lattice');
    assert.equal(fluxCellIndex(source, count - 1, L, null, 1, 1, 1), -2, 'not the dense grid');
    // Organic positions are not cell centres, so they cannot identify a cell.
    const organic = jittered(source, L);
    let moved = -1;
    for (let i = 0; i < count && moved < 0; i++) {
        if (Math.abs(organic[i * 3] - source[i * 3]) > 0.2) moved = i;
    }
    assert.ok(moved >= 0);
    const cell = [0, 1, 2].map((axis) => Math.round(source[moved * 3 + axis] - 0.5));
    assert.equal(fluxCellIndex(organic, count, L, null, ...cell), -2);
    assert.equal(fluxCellIndex(source, count, L, null, ...cell), moved);
});

test('Moore lookup finds every neighbour without a scan, for any focus cell', () => {
    const L = 9;
    const source = grid(L);
    const count = L ** 3;
    // An interior cell with x !== z: the case the link order got wrong.
    const interior = mooreCellIndices(source, count, L, null, 2.5, 4.5, 6.5, false);
    assert.equal(interior.length, 26);
    const cells = interior.map(({ index }) => [source[index * 3], source[index * 3 + 1], source[index * 3 + 2]].join());
    assert.equal(new Set(cells).size, 26);
    for (const { index, d } of interior) {
        const dx = source[index * 3] - 2.5;
        const dy = source[index * 3 + 1] - 4.5;
        const dz = source[index * 3 + 2] - 6.5;
        assert.ok(Math.max(Math.abs(dx), Math.abs(dy), Math.abs(dz)) === 1);
        assert.equal(d, dx * dx + dy * dy + dz * dz);
    }
    assert.equal(mooreCellIndices(source, count, L, null, 2.5, 4.5, 6.5, true).length, 27);
    assert.equal(mooreCellIndices(source, count, L, null, 0.5, 0.5, 0.5, false).length, 7, 'corner');
    assert.equal(mooreCellIndices(source, count, L, null, 0.5, 4.5, 8.5, false).length, 11, 'edge');

    // Hidden cells are skipped, the focus itself may be hidden.
    const visibility = new Float32Array(count).fill(1);
    visibility[fluxPointIndex(L, 3, 4, 6)] = 0;
    visibility[fluxPointIndex(L, 2, 4, 6)] = 0;
    assert.equal(mooreCellIndices(source, count, L, visibility, 2.5, 4.5, 6.5, false).length, 25);

    // A buffer that is not the lattice grid asks the caller to scan.
    assert.equal(mooreCellIndices(source.subarray(0, 300), 100, L, null, 2.5, 4.5, 6.5, false), null);
});

test('nearest visible dot equals a scan of every dot, regular and Organic', () => {
    const L = 9;
    const source = grid(L);
    const count = L ** 3;
    const scan = (points, visibility, lx, ly, lz) => {
        let best = -1;
        let bestD = Infinity;
        for (let i = 0; i < count; i++) {
            if (visibility && visibility[i] < 0.5) continue;
            const d = (points[i * 3] - lx) ** 2 + (points[i * 3 + 1] - ly) ** 2 + (points[i * 3 + 2] - lz) ** 2;
            if (d < bestD) { bestD = d; best = i; }
        }
        return best;
    };
    let state = 12345;
    const next = () => { state = (Math.imul(state, 1103515245) + 12345) >>> 0; return state / 4294967296; };
    let trials = 0;
    for (const points of [source, jittered(source, L, 3), jittered(source, L, 99)]) {
        for (const density of [1, 0.6, 0.15, 0.02, 0.003, 0]) {
            for (let k = 0; k < 40; k++) {
                const visibility = new Float32Array(count);
                for (let i = 0; i < count; i++) visibility[i] = next() < density ? 1 : 0;
                // Targets inside the lattice, on cell boundaries, and beyond its edges.
                const lx = k % 4 === 0 ? Math.floor(next() * L) : (next() * 1.4 - 0.2) * L;
                const ly = k % 5 === 0 ? Math.floor(next() * L) + 0.5 : (next() * 1.4 - 0.2) * L;
                const lz = (next() * 1.4 - 0.2) * L;
                assert.equal(
                    nearestVisiblePoint(points, source, count, L, visibility, lx, ly, lz),
                    scan(points, visibility, lx, ly, lz),
                    `density ${density}, target ${lx},${ly},${lz}`,
                );
                trials++;
            }
        }
    }
    assert.equal(trials, 720);
    assert.equal(nearestVisiblePoint(source, source, count, L, null, 4.4, 4.6, 4.5), fluxPointIndex(L, 4, 4, 4));
    // Not the dense grid (only the first 100 points): the scan answers.
    // Cell (1, 1, 0) is point 10, and it is among them.
    assert.equal(nearestVisiblePoint(source, source, 100, L, null, 1.5, 1.5, 0.5), fluxPointIndex(L, 1, 1, 0));
});

// ── Readout components: a second render of the same frame writes nothing ──
// The fake element mimics the browser behaviour that defeated the original
// guards: reading a style back does not return the string that was written.
function fakeDocument(log) {
    const element = (tag) => {
        const style = new Proxy({}, {
            set(target, key, value) { log.push(`style.${String(key)}`); target[key] = value; return true; },
            get(target, key) {
                if (key === 'setProperty') return (name, value) => { log.push(`var ${name}`); target[name] = value; };
                const value = target[key];
                return typeof value === 'string' ? value.replace(', 0)', ', 0px)').replace('hsl(', 'rgb(') : value;
            },
        });
        const attributes = new Map();
        let text = '';
        let hidden = false;
        return {
            tag, style, children: [], className: '',
            append(...nodes) { this.children.push(...nodes); },
            appendChild(node) { this.children.push(node); return node; },
            setAttribute(name, value) { log.push(`attr.${name}`); attributes.set(name, String(value)); },
            getAttribute(name) { return attributes.get(name) ?? null; },
            get textContent() { return text; },
            set textContent(value) { log.push('text'); text = String(value); },
            get hidden() { return hidden; },
            set hidden(value) { log.push('hidden'); hidden = !!value; },
        };
    };
    return { createElement: element, createElementNS: (_ns, tag) => element(tag) };
}

async function withFakeDom(run) {
    const log = [];
    const previous = globalThis.document;
    globalThis.document = fakeDocument(log);
    try {
        await run(log);
    } finally {
        if (previous === undefined) delete globalThis.document;
        else globalThis.document = previous;
    }
}

test('Moore readouts and clocks write the DOM only when a value changes', async () => {
    await withFakeDom(async (log) => {
        const { createMooreNeighborhood } = await import('../js/ui/components/live-rulers/neighborhood.js');
        const { createVoxelClocks } = await import('../js/ui/components/live-rulers/voxel-clocks.js');
        const sites = Array.from({ length: 26 }, (_, i) => ({
            x: 100 + i * 7.25, y: 200 + i * 3.5, px: 12 + i, fill: `hsl(${(i * 9).toFixed(1)} 78% 58%)`, opacity: 0.9,
            bar: true, showJoule: true, joule: 1e-9 * (i + 1),
            wave: { path: `M0 ${i}L100 8`, value: i, min: 0, max: 30, samples: [0, i] },
        }));
        const energy = { x: 400, y: 120, width: 96, value: 2.5e-8, min: 0, max: 1, samples: [0, 1], area: 'M0 14L100 2L100 16L0 16Z' };
        const neighborhood = createMooreNeighborhood();
        neighborhood.render(sites, energy);
        assert.ok(log.length > 26 * 5, 'first render writes every readout');

        log.length = 0;
        neighborhood.render(sites, energy);
        neighborhood.render(sites.map((site) => ({ ...site })), { ...energy });
        assert.deepEqual(log, [], 'an unchanged frame writes nothing');

        // Camera moved: only the transforms change.
        log.length = 0;
        neighborhood.render(sites.map((site) => ({ ...site, x: site.x + 1 })), { ...energy, x: energy.x + 1 });
        assert.deepEqual([...new Set(log)], ['style.transform']);
        assert.equal(log.length, 27);

        // One value changed: one text write.
        log.length = 0;
        const moved = sites.map((site) => ({ ...site, x: site.x + 1 }));
        moved[4] = { ...moved[4], joule: 5e-7 };
        neighborhood.render(moved, { ...energy, x: energy.x + 1 });
        assert.deepEqual(log, ['text']);

        // Fewer sites: the surplus readouts are hidden once.
        log.length = 0;
        neighborhood.render(moved.slice(0, 20), null);
        assert.equal(log.filter((entry) => entry === 'hidden').length, 7);
        log.length = 0;
        neighborhood.render(moved.slice(0, 20), null);
        assert.deepEqual(log, []);

        const clocks = createVoxelClocks();
        const faces = Array.from({ length: 27 }, (_, i) => ({ x: 50 + i * 4.5, y: 60 + i, opacity: 0.45 + i * 0.02, phase: i / 27 }));
        clocks.render(faces);
        log.length = 0;
        clocks.render(faces.map((face) => ({ ...face })));
        assert.deepEqual(log, [], 'unchanged clock faces write nothing');
        clocks.render(faces.map((face) => ({ ...face, phase: face.phase + 0.25 })));
        assert.deepEqual([...new Set(log)], ['style.transform']);
        assert.equal(log.length, 27, 'only the hands turn');
        // A phase change below the displayed tenth of a degree is not written.
        log.length = 0;
        clocks.render(faces.map((face) => ({ ...face, phase: face.phase + 0.25 + 1e-7 })));
        assert.deepEqual(log, []);
    });
});

test('the view ruler does no string work for an unchanged scale', async () => {
    await withFakeDom(async (log) => {
        const { createViewportRuler } = await import('../js/ui/components/live-rulers/viewport-ruler.js');
        const { viewportScale } = await import('../js/ui/components/live-rulers/measure.js');
        globalThis.document.createElement = ((make) => (tag) => {
            const el = make(tag);
            el.replaceChildren = () => { el.children.length = 0; };
            return el;
        })(globalThis.document.createElement);
        const ruler = createViewportRuler();
        let formatted = 0;
        const format = (units) => { formatted++; return `${units} m`; };
        ruler.render(viewportScale(40), format, 1.6e-35);
        assert.ok(formatted > 1);
        formatted = 0;
        log.length = 0;
        ruler.render(viewportScale(40), format, 1.6e-35);
        assert.equal(formatted, 0);
        assert.deepEqual(log, []);
        ruler.render(viewportScale(40), format, 3.2e-35);
        assert.ok(formatted > 0, 'a new length unit relabels the ruler');
        formatted = 0;
        ruler.render(viewportScale(41), format, 3.2e-35);
        assert.ok(formatted > 0, 'a new span relabels the ruler');
    });
});

test('Moore panel history is bounded and stays ordered through a trim', () => {
    const trace = makeMooreTrace(1000, 100);
    for (let tick = 0; tick < 5000; tick++) trace.push(tick * 0.5, tick);
    assert.ok(trace.count >= 1000 && trace.count < 1100, `kept ${trace.count}`);
    assert.equal(trace.total, 5000);
    assert.equal(trace.last(), 4999 * 0.5);
    assert.equal(trace.getTick(trace.count - 1), 4999);
    for (let i = 1; i < trace.count; i++) assert.equal(trace.getTick(i) - trace.getTick(i - 1), 1);
    assert.equal(trace.get(0), trace.getTick(0) * 0.5, 'values and ticks are trimmed together');

    // Without engine ticks the stamp is the running total, which a trim cannot rewind.
    const unstamped = makeMooreTrace(50, 10);
    for (let i = 0; i < 200; i++) unstamped.push(i);
    for (let i = 1; i < unstamped.count; i++) assert.ok(unstamped.getTick(i) > unstamped.getTick(i - 1));
    assert.equal(unstamped.getTick(unstamped.count - 1), 199);

    unstamped.push(NaN);
    assert.equal(unstamped.total, 200, 'non-finite samples are ignored');
    const generation = unstamped.generation;
    unstamped.clear();
    assert.deepEqual([unstamped.count, unstamped.total, unstamped.generation], [0, 0, generation + 1]);
});

test('source contract: the fixes are wired where the defects were measured', () => {
    const read = (path) => readFileSync(new URL('../js/' + path, import.meta.url), 'utf8');
    const viewport = read('viewport.js');
    assert.match(viewport, /import \{ fluxCellIndex, mooreCellIndices, nearestVisiblePoint \} from '\.\/viewport\/flux-point-grid\.js'/);
    const cellIndex = viewport.slice(viewport.indexOf('    _fluxCellIndex('), viewport.indexOf('    _neighborhoodIndices('));
    assert.ok(!cellIndex.includes('siteIndex('), 'the point cloud is not indexed with the link order');
    assert.match(viewport, /this\._updateLiveRulers\(false\);/, 'the per-frame call may return early');
    const lattice = read('scales/scale0/ui/overlays/moore-lattice.js');
    const draw = lattice.slice(lattice.indexOf('const draw = () => {'), lattice.indexOf('const scalarKey'));
    assert.ok(!draw.includes('getBoundingClientRect'), 'draw() reads no layout');
    const layout = lattice.slice(lattice.indexOf('const layout = () => {'), lattice.indexOf('const drawAxes'));
    assert.match(layout, /if \(!measured\)/, 'layout is read once, then kept by the ResizeObserver');
    for (const file of ['ui/components/live-rulers/neighborhood.js', 'ui/components/live-rulers/voxel-clocks.js']) {
        assert.ok(!/style\.\w+ !== /.test(read(file)), `${file} does not compare against a style getter`);
    }
});
