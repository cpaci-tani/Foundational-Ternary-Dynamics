import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { barLogoOptions, createLatticeLogo } from '../js/ui/components/lattice-logo.js';
import { getLoadingOverlayTemplate } from '../js/ui/components/loading-overlay/template.js';

const read = (path) => readFileSync(new URL(`../${path}`, import.meta.url), 'utf8');
const squash = (text) => text.replace(/\s+/g, '');

/** A canvas that records what is drawn on it. */
function recordingCanvas(size) {
    const calls = { lines: [], dots: [], glows: 0, cleared: 0, widths: new Set() };
    let pen = null;
    const context = {
        clearRect: () => { calls.cleared += 1; },
        beginPath: () => { pen = null; },
        moveTo: (x, y) => { pen = [x, y]; },
        lineTo: (x, y) => { calls.lines.push([pen[0], pen[1], x, y]); },
        stroke: () => { calls.widths.add(context.lineWidth); },
        arc: (x, y, r) => { calls.dots.push([x, y, r]); },
        fill: () => {},
        fillRect: () => {},
        createRadialGradient: () => { calls.glows += 1; return { addColorStop: () => {} }; },
        strokeStyle: '', fillStyle: '', lineWidth: 1, globalAlpha: 1,
    };
    return { canvas: { width: size, height: size, getContext: () => context }, calls };
}

const seeded = (seed = 7) => () => { seed = (seed * 16807) % 2147483647; return seed / 2147483647; };

test('the splash cube is unchanged: eight sites an edge, hairlines, haloes on live sites', () => {
    const { canvas, calls } = recordingCanvas(512);
    createLatticeLogo(canvas, { random: seeded() }).step();
    assert.equal(calls.dots.length, 8 ** 3);
    // Wires run along the six faces only: 3 · 2 · N(N−1) face lines, less the shared cube edges counted twice.
    assert.equal(calls.lines.length, 3 * (2 * 8 * 7 * 2 - 4 * 7));
    assert.deepEqual([...calls.widths], [0.5]);
    assert.ok(calls.glows > 0);
    // Centred on the canvas, well inside it.
    for (const [x, y] of calls.dots) assert.ok(x > 120 && x < 392 && y > 120 && y < 392);
    const component = read('js/ui/components/loading-overlay/component.js');
    assert.match(component, /createLatticeLogo\(cv\)/, 'the splash draws with the defaults');
});

test('the bar cube is the same drawing at four sites an edge and never leaves its canvas', () => {
    for (const size of [44, 88, 132]) {
        const { canvas, calls } = recordingCanvas(size);
        const logo = createLatticeLogo(canvas, { ...barLogoOptions(size), random: seeded() });
        // A whole turn, sampled.
        for (let turn = 0; turn < 80; turn++) logo.step(10);
        assert.equal(calls.dots.length, 80 * 4 ** 3);
        assert.equal(calls.glows, 0, 'no halo at this size');
        for (const [x, y, r] of calls.dots) {
            assert.ok(x - r >= 0 && x + r <= size && y - r >= 0 && y + r <= size, `a site left the ${size}px canvas`);
        }
        // The tilt makes the cube taller than it is wide, so its height is what fills the canvas.
        const span = (axis) => Math.max(...calls.dots.map((dot) => dot[axis])) - Math.min(...calls.dots.map((dot) => dot[axis]));
        assert.ok(span(1) > size * 0.8, 'and it uses the canvas');
        assert.ok(span(0) > size * 0.7);
    }
});

test('sites walk through 0, +1, 0, −1 and the cube turns at the splash pace', () => {
    const { canvas, calls } = recordingCanvas(88);
    const logo = createLatticeLogo(canvas, { ...barLogoOptions(88), random: () => 0 });
    logo.step(0);
    const start = calls.dots.slice();
    calls.dots.length = 0;
    logo.step(125);      // 125 frames · 0.008 rad = one radian
    const later = calls.dots;
    assert.notDeepEqual(later.map(([x]) => x.toFixed(2)), start.map(([x]) => x.toFixed(2)));
    // All sites share a phase here: two time units at 0, then +1.
    const source = read('js/ui/components/lattice-logo.js');
    assert.match(source, /phase < 2 \? 0 : phase < 3 \? 1 : phase < 5 \? 0 : -1/);
    assert.match(source, /const TURN_PER_FRAME = 0\.008;/);
});

test('the top bar holds the cube, is still named FTD Engine, and respects reduced motion', () => {
    const html = read('index.html');
    const brand = html.match(/<h1 class="brand"[^>]*>[\s\S]*?<\/h1>/)?.[0] || '';
    assert.match(brand, /<h1 class="brand" aria-label="FTD Engine">/);
    assert.match(brand, /<canvas class="brand-lattice" width="84" height="84" aria-hidden="true"><\/canvas>/);
    assert.doesNotMatch(brand.replace(/<[^>]+>/g, ''), /\S/, 'no text beside the cube');
    const topbar = read('js/ui/components/topbar/component.js');
    assert.match(topbar, /mountBarLatticeLogo\(this\.toolbar\.querySelector\('\.brand-lattice'\)\)/);
    const source = read('js/ui/components/lattice-logo.js');
    assert.match(source, /prefers-reduced-motion: reduce/);
    assert.match(source, /const BAR_LOGO_FPS = 30;/);
});

test('the inlined splash and its script template are the same markup', () => {
    const html = read('index.html');
    const inlined = html.match(/<div id="loading-overlay"[^>]*>([\s\S]*?)<\/div>\s*<!-- GPU Acceleration/)?.[1] || '';
    assert.equal(squash(inlined), squash(getLoadingOverlayTemplate()));
});
