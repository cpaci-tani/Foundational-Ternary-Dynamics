import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import vm from 'node:vm';
import { SpectrumAnalysisClient } from '../js/scales/scale0/analysis/spectrum-analysis-client.js';

const source = readFileSync(new URL('../js/scales/scale0/ui/overlays/spectrum-panel.js', import.meta.url), 'utf8');
const mountedSource = source.slice(source.indexOf('export function mountSpectrumPanel('), source.indexOf('export function initSpectrumPanel(')).replace(/^export /, '');

// Execute the production mount and its real registered handlers. Only browser,
// coordinator, sampling and rendering dependencies are replaced with test doubles.
function fixture() {
    const elements = new Map();
    function element() {
        const listeners = new Map();
        return {
            hidden: false, disabled: false, innerHTML: '', textContent: '', dataset: {},
            classList: { add() {}, toggle() {} }, setAttribute() {}, remove() {},
            addEventListener(type, cb) { listeners.set(type, cb); },
            removeEventListener(type, cb) { if (listeners.get(type) === cb) listeners.delete(type); },
            click() { if (!this.disabled) listeners.get('click')?.({ currentTarget: this }); },
            dispatch(type) { listeners.get(type)?.({ currentTarget: this }); },
        };
    }
    const panel = element();
    panel.querySelector = key => {
        if (!elements.has(key)) elements.set(key, element());
        return elements.get(key);
    };
    const host = { live: true, appendChild() {} };
    const state = { currentScenarioId: 'wave', qualificationAnchor: { loadGeneration: 1 }, ready: true };
    const owner = () => ({ capabilities: { scale0: { latticeSize: 97 } }, wants: [], calls: [],
        replaceSamplerWants(id, keys) { this.wants = [...keys]; this.calls.push([id, [...keys]]); } });
    let bridge = owner(), now = 0, nextId = 1, qualification;
    const timers = new Map(), rafs = new Map(), callbacks = new Set();
    const counts = { deep: 0, live: 0, spatial: 0 };
    let available = false;
    const workers = [];
    const visibilityListeners = new Map();
    const env = {
        document: { getElementById: () => null }, window: {
            addEventListener: (name, cb) => visibilityListeners.set(name, cb),
            removeEventListener: name => visibilityListeners.delete(name),
        }, buildPanel: () => panel, PANEL_VISIBILITY_CHANGE_EVENT: 'visibility',
        PANEL_ID: 'test-spectrum', EMPTY_SCENARIO_ID: 'empty', HZ: 2, M_DEEP: 64,
        SCENARIO_SYNC_MAX_FRAMES: 120, METRIC_KINDS: [{ kind: 'curlJ' }],
        liveStride: () => 4, liveGridSize: () => 16,
        getScale0State: () => state, isScale0AuthoritativeGenerationReady: s => s.ready,
        isPanelLive: h => h.live, readQualifiedTelemetry: () => ({}),
        subscribeScale0Qualification: cb => { qualification = cb; return () => { qualification = null; }; },
        rafCoordinator: { subscribe(id, { cb }) { callbacks.add(cb); return { unsubscribe() { callbacks.delete(cb); } }; } },
        requestAnimationFrame: cb => { const id = nextId++; rafs.set(id, cb); return id; },
        cancelAnimationFrame: id => rafs.delete(id),
        setTimeout: (cb, delay) => { const id = nextId++; timers.set(id, { cb, due: now + delay }); return id; },
        clearTimeout: id => timers.delete(id), performance: { now: () => now }, console,
        SpectrumAnalysisClient: class extends SpectrumAnalysisClient {
            constructor() {
                super({ now: () => now,
                    schedule: (cb, delay) => { const id = nextId++; timers.set(id, { cb, due: now + delay }); return id; },
                    unschedule: id => timers.delete(id), workerFactory: () => {
                        const worker = { message: null, terminated: false,
                            postMessage(message) { this.message = message; },
                            terminate() { this.terminated = true; } };
                        workers.push(worker); return worker;
                    } });
            }
        },
        captureSpectrumObservation(caps, { mode }) {
            if (mode === 'deep') counts.deep++;
            else if (mode === 'live') counts.live++;
            else counts.spatial++;
            return { mode, flux: mode === 'deep' && !available ? null : {}, metrics: [], provenance: {} };
        },
        renderSpectrum() {}, renderTopology() {}, renderMetrics() {}, renderEnergy() {},
    };
    const mount = vm.runInNewContext(`${mountedSource}\nmountSpectrumPanel`, env);
    const api = mount(host, () => bridge);
    return {
        api, host, state, counts, timers, callbacks, owner,
        get bridge() { return bridge; }, set bridge(v) { bridge = v; },
        set available(v) { available = v; },
        button: name => elements.get(`#test-spectrum-${name}`),
        firstTimer: () => [...timers.values()][0]?.cb,
        workers,
        visibility() { visibilityListeners.get('visibility')?.(); },
        complete(worker = workers.at(-1)) {
            const message = worker.message;
            worker.onmessage({ data: { ...message, result: { provenance: message.observation.provenance,
                spectrum: { kind: message.observation.mode === 'deep' ? 'deep' : 'live' }, topology: {}, metrics: [] } } });
        },
        notify() { qualification?.({ scenarioId: state.currentScenarioId,
            authoritativeLoad: state.ready ? null : { status: 'pending', loadGeneration: state.qualificationAnchor.loadGeneration },
            anchor: state.qualificationAnchor }); },
        advance(ms) {
            const end = now + ms;
            for (;;) {
                const entry = [...timers.entries()].sort((a, b) => a[1].due - b[1].due)[0];
                if (!entry || entry[1].due > end) break;
                now = entry[1].due; timers.delete(entry[0]); entry[1].cb();
            }
            now = end;
        },
        deliverLate(ms) { now += ms; const entry = [...timers.entries()][0]; if (entry) { timers.delete(entry[0]); entry[1].cb(); } },
    };
}
const dense = f => f.bridge.wants.includes('fluxVector@1');

test('Deep waits for completed sampling, freezes once, and releases dense demand', () => {
    const f = fixture(); f.api.deepMeasure(); assert.equal(dense(f), true);
    f.advance(30); assert.equal(f.api.mode, 'deep'); assert.equal(f.api.lastSpec, null);
    f.available = true; f.advance(50); f.complete();
    assert.equal(f.api.lastSpec.kind, 'deep'); assert.equal(dense(f), false);
    f.complete();
    assert.equal(f.timers.size, 0);
    const prior = { ...f.counts }; f.api.update();
    assert.equal(f.counts.deep, prior.deep); assert.equal(f.counts.live, prior.live);
    assert.equal(f.counts.spatial, prior.spatial + 1);
});

test('nonempty reload clears pending state before the new owner registers live demand', () => {
    const f = fixture(); const old = f.bridge; f.api.deepMeasure(); const stale = f.firstTimer();
    f.state.ready = false; f.notify(); assert.deepEqual(old.wants, []); assert.equal(f.timers.size, 0);
    f.bridge = f.owner(); f.state.qualificationAnchor.loadGeneration++; f.state.ready = true; f.notify();
    assert.equal(f.api.mode, 'live'); assert.equal(dense(f), false);
    const before = f.counts.deep; stale(); assert.equal(f.counts.deep, before);
});

for (const throughTimer of [false, true]) {
    test(`hidden panel cancels pending demand through ${throughTimer ? 'timer' : 'update'}`, () => {
        const f = fixture(); f.api.deepMeasure(); f.host.live = false;
        if (throughTimer) f.advance(30); else f.api.update();
        assert.equal(f.api.mode, 'live'); assert.deepEqual(f.bridge.wants, []); assert.equal(f.timers.size, 0);
        f.host.live = true; f.api.update(); assert.equal(dense(f), false);
    });
}

test('owner swap cancels against the old owner without touching the new owner demand', () => {
    const f = fixture(); const old = f.bridge; f.api.deepMeasure(); f.bridge = f.owner();
    f.advance(30); assert.deepEqual(old.wants, []); assert.equal(f.bridge.calls.length, 0);
    f.api.update(); assert.equal(dense(f), false);
});

test('unready generation and a changed generation cancel before any deep computation', () => {
    for (const mutate of [f => { f.state.ready = false; }, f => { f.state.qualificationAnchor.loadGeneration++; }]) {
        const f = fixture(); f.api.deepMeasure(); mutate(f); f.advance(30);
        assert.equal(f.counts.deep, 0); assert.equal(f.api.mode, 'live'); assert.deepEqual(f.bridge.wants, []);
    }
});

test('entry is inert when hidden or generation is unready, including synthetic clicks', () => {
    for (const mutate of [f => { f.state.ready = false; }, f => { f.host.live = false; }]) {
        const f = fixture(); mutate(f); const before = f.bridge.calls.length, timersBefore = f.timers.size;
        f.button('deep').dispatch('click');
        assert.equal(f.timers.size, timersBefore); assert.equal(f.bridge.calls.length, before);
    }
});

test('Live cancels immediately and a delivered old callback cannot terminate the next Deep', () => {
    const f = fixture(); f.api.deepMeasure(); const stale = f.firstTimer();
    f.button('live').dispatch('click'); assert.equal(dense(f), false); f.complete(); assert.equal(f.timers.size, 0);
    f.api.deepMeasure(); const current = f.firstTimer(); stale();
    assert.equal(f.firstTimer(), current); assert.equal(dense(f), true); assert.equal(f.api.mode, 'deep');
    f.available = true; f.advance(30); f.complete(); assert.equal(f.api.lastSpec.kind, 'deep');
});

test('empty transition removes pending work and sampler demand', () => {
    const f = fixture(); f.api.deepMeasure(); f.state.currentScenarioId = 'empty'; f.notify();
    assert.equal(f.timers.size, 0); assert.equal(f.api.coordinatorActive, false);
    assert.equal(f.api.applicability, 'inapplicable-empty'); assert.deepEqual(f.bridge.wants, []);
});

test('disposal makes retained public APIs and detached handlers inert', () => {
    const f = fixture(); f.api.deepMeasure(); const stale = f.firstTimer(); f.api.dispose();
    const before = f.bridge.calls.length, counts = { ...f.counts };
    f.api.update(); f.api.deepMeasure(); f.api.rebindScenarioApplicability();
    f.button('deep').dispatch('click'); f.button('live').dispatch('click'); stale(); f.api.dispose();
    assert.equal(f.bridge.calls.length, before); assert.deepEqual(f.counts, counts);
    assert.equal(f.timers.size, 0); assert.equal(f.callbacks.size, 0); assert.deepEqual(f.bridge.wants, []);
});

test('a throttled timer does not sample or accept data arriving after the original deadline', () => {
    const f = fixture(); f.api.deepMeasure(); f.available = true; f.deliverLate(5001);
    assert.equal(f.counts.deep, 0); assert.equal(f.api.mode, 'live'); assert.equal(dense(f), false);
    assert.equal(f.button('mode').textContent, 'deep sample unavailable');
});

test('retries have one bounded deadline and cannot leave dense demand behind', () => {
    const f = fixture(); f.api.deepMeasure(); f.advance(5100);
    assert.ok(f.counts.deep > 1 && f.counts.deep <= 100);
    assert.equal(dense(f), false); assert.equal(f.api.mode, 'live');
    f.complete(); assert.equal(f.timers.size, 0);
});

for (const boundary of ['visibility', 'owner', 'generation', 'empty', 'dispose']) {
    test(`late live worker publication is rejected after ${boundary}`, () => {
        const f = fixture(); const worker = f.workers.at(-1);
        if (boundary === 'visibility') { f.host.live = false; f.visibility(); }
        else if (boundary === 'owner') f.bridge = f.owner();
        else if (boundary === 'generation') f.state.qualificationAnchor.loadGeneration++;
        else if (boundary === 'empty') { f.state.currentScenarioId = 'empty'; f.notify(); }
        else f.api.dispose();
        f.complete(worker); assert.equal(f.api.lastSpec, null);
        f.api.dispose();
    });
}

test('a computing Deep is terminated on hide and its reply cannot overwrite a later Deep', () => {
    const f = fixture(); f.available = true; f.api.deepMeasure(); f.advance(30);
    const oldWorker = f.workers.at(-1);
    f.host.live = false; f.visibility(); assert.equal(oldWorker.terminated, true);
    f.host.live = true; f.api.update(); f.api.deepMeasure(); f.advance(30);
    f.complete(oldWorker); assert.equal(f.api.lastSpec, null);
    f.complete(); assert.equal(f.api.lastSpec.kind, 'deep'); f.api.dispose();
});

test('Deep computation has the same absolute five-second budget as sample availability', () => {
    const f = fixture(); f.api.deepMeasure(); f.advance(4900);
    f.available = true; f.advance(50); const worker = f.workers.at(-1);
    f.advance(51); assert.equal(worker.terminated, true);
    assert.equal(f.api.mode, 'live'); assert.equal(dense(f), false);
    f.complete(worker); assert.equal(f.api.lastSpec, null); f.api.dispose();
});

test('worker completion only updates a bounded mailbox until the registered live update', () => {
    const f = fixture(); f.complete(); assert.equal(f.api.lastSpec, null);
    f.api.update(); assert.equal(f.api.lastSpec.kind, 'live'); f.api.dispose();
});

test('a completed Deep stays frozen across hide while live topology work is cancelled', () => {
    const f = fixture(); f.available = true; f.api.deepMeasure(); f.advance(30); f.complete();
    const frozen = f.api.lastSpec, worker = f.workers.at(-1);
    f.host.live = false; f.visibility(); assert.equal(worker.terminated, true);
    assert.equal(f.api.lastSpec, frozen); assert.equal(f.api.mode, 'deep');
    f.host.live = true; f.api.update(); f.complete(); f.api.update();
    assert.equal(f.api.lastSpec, frozen); assert.equal(f.api.mode, 'deep'); f.api.dispose();
});

test('unrelated visibility notifications neither sample nor publish outside the 2 Hz callback', () => {
    const f = fixture(); f.complete(); const counts = { ...f.counts };
    for (let i = 0; i < 20; i++) f.visibility();
    assert.deepEqual(f.counts, counts); assert.equal(f.api.lastSpec, null);
    f.host.live = false; f.visibility(); assert.deepEqual(f.bridge.wants, []);
    f.host.live = true; f.visibility(); assert.equal(f.counts.live, counts.live + 1);
    f.complete(); f.api.update(); assert.equal(f.api.lastSpec.kind, 'live'); f.api.dispose();
});
