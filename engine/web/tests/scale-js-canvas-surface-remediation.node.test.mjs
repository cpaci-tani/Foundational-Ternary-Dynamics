import test from 'node:test';
import assert from 'node:assert/strict';

import {
    CanvasSurface,
    resizeCanvasSurface,
} from '../js/ui/utils/canvas-surface.js';
import { drawGasProfileBars } from '../js/scales/scale5/ui/gas-profile-canvas.js';

function makeCanvas(width = 120, height = 60) {
    const calls = [];
    const ctx = new Proxy({
        setTransform: (...args) => calls.push(['setTransform', ...args]),
    }, {
        get(target, key) {
            if (key in target) return target[key];
            return (...args) => calls.push([key, ...args]);
        },
        set(target, key, value) { target[key] = value; return true; },
    });
    return {
        width: 1,
        height: 1,
        clientWidth: width,
        clientHeight: height,
        rect: { width, height },
        getBoundingClientRect() { return this.rect; },
        getContext() { return ctx; },
        ctx,
        calls,
    };
}

test('resizeCanvasSurface keeps logical CSS pixels and DPR backing pixels separate', () => {
    const priorWindow = globalThis.window;
    globalThis.window = { devicePixelRatio: 3 };
    try {
        const canvas = makeCanvas(125.5, 62.25);
        const result = resizeCanvasSurface(canvas, { maxDpr: 2 });
        assert.equal(result.width, 125.5);
        assert.equal(result.height, 62.25);
        assert.equal(result.dpr, 2);
        assert.equal(canvas.width, 251);
        assert.equal(canvas.height, 125);
        assert.deepEqual(canvas.calls[0], ['setTransform', 2, 0, 0, 2, 0, 0]);
    } finally {
        globalThis.window = priorWindow;
    }
});

test('CanvasSurface redraws after observed size changes and disconnects once', () => {
    const priorWindow = globalThis.window;
    const priorObserver = globalThis.ResizeObserver;
    const priorRaf = globalThis.requestAnimationFrame;
    const priorCancel = globalThis.cancelAnimationFrame;
    let observed = null;
    let observeCount = 0;
    let disconnected = 0;
    let callback = null;
    globalThis.window = { devicePixelRatio: 2 };
    globalThis.ResizeObserver = class {
        constructor(cb) { callback = cb; }
        observe(value) { observed = value; observeCount++; }
        disconnect() { disconnected++; }
    };
    const frames = new Map();
    let nextFrame = 0;
    globalThis.requestAnimationFrame = (cb) => { frames.set(++nextFrame, cb); return nextFrame; };
    globalThis.cancelAnimationFrame = (id) => frames.delete(id);
    const flush = () => {
        const pending = [...frames.values()];
        frames.clear();
        pending.forEach((cb) => cb(0));
    };

    try {
        const canvas = makeCanvas(100, 50);
        const sizes = [];
        const surface = new CanvasSurface(canvas, ({ width, height }) => sizes.push([width, height])).mount();
        surface.mount();
        assert.equal(observed, canvas);
        assert.equal(observeCount, 1);
        flush();
        canvas.rect = { width: 180, height: 90 };
        callback();
        flush();
        assert.deepEqual(sizes, [[100, 50], [180, 90]]);
        assert.equal(canvas.width, 360);
        assert.equal(canvas.height, 180);
        surface.dispose();
        surface.dispose();
        assert.equal(disconnected, 1);
    } finally {
        globalThis.window = priorWindow;
        globalThis.ResizeObserver = priorObserver;
        globalThis.requestAnimationFrame = priorRaf;
        globalThis.cancelAnimationFrame = priorCancel;
    }
});

test('Scale 5 gas profiles use the shared DPR sizing contract', () => {
    const priorWindow = globalThis.window;
    globalThis.window = { devicePixelRatio: 2 };
    try {
        const canvas = makeCanvas(220, 70);
        drawGasProfileBars(canvas, [1, 2, 3], '#fff', 'nonneg', [0, 1]);
        assert.equal(canvas.width, 440);
        assert.equal(canvas.height, 140);
        assert.ok(canvas.calls.some(([name]) => name === 'fillRect'));
        assert.ok(canvas.calls.some(([name]) => name === 'fillText'));
    } finally {
        globalThis.window = priorWindow;
    }
});

test('hidden DOM canvases do not multiply backing pixels before becoming visible', () => {
    const priorWindow = globalThis.window;
    const priorObserver = globalThis.ResizeObserver;
    const priorRaf = globalThis.requestAnimationFrame;
    const priorCancel = globalThis.cancelAnimationFrame;
    let observedCallback = null;
    const frames = [];
    globalThis.window = { devicePixelRatio: 2 };
    globalThis.ResizeObserver = class {
        constructor(cb) { observedCallback = cb; }
        observe() {}
        disconnect() {}
    };
    globalThis.requestAnimationFrame = (cb) => { frames.push(cb); return frames.length; };
    globalThis.cancelAnimationFrame = () => {};
    const flush = () => { while (frames.length) frames.shift()(0); };
    try {
        const canvas = makeCanvas(0, 0);
        canvas.ownerDocument = {};
        canvas.width = 276;
        canvas.height = 140;
        const draws = [];
        const surface = new CanvasSurface(canvas, ({ width, height }) => draws.push([width, height])).mount();
        flush();
        assert.deepEqual([canvas.width, canvas.height], [276, 140]);
        assert.deepEqual(draws, []);
        canvas.rect = { width: 312, height: 158 };
        observedCallback();
        flush();
        assert.deepEqual([canvas.width, canvas.height], [624, 316]);
        assert.deepEqual(draws, [[312, 158]]);
        surface.dispose();
    } finally {
        globalThis.window = priorWindow;
        globalThis.ResizeObserver = priorObserver;
        globalThis.requestAnimationFrame = priorRaf;
        globalThis.cancelAnimationFrame = priorCancel;
    }
});
