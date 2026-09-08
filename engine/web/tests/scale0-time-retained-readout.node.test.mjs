import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import vm from 'node:vm';
import * as clock from '../js/scales/scale0/analysis/time-analysis.js';
import * as cards from '../js/scales/scale0/ui/overlays/_card-helpers.js';
import * as reference from '../js/scales/scale0/data/ftd0252-reference.js';
import { C_SPEED } from '../js/constants.js';
import { TickHistoryControl } from '../js/ui/charts/history-window.js';

const source = readFileSync(new URL('../js/scales/scale0/ui/overlays/time-panel.js', import.meta.url), 'utf8');
const executable = source.replace(/^import[\s\S]*?from\s+['"][^'"]+['"];\s*/gm, '').replace(/export\s+/g, '');

// The renderer's real-DOM reconciliation is tested by its owner. Here the sink
// records the exact markup emitted by actual Time renderers and counts writes.
class Element {
    constructor() { this.markup = ''; this.writes = 0; this.children = new Map(); this.listeners = new Map(); }
    set innerHTML(value) { this.markup = value; this.writes++; }
    get innerHTML() { return this.markup; }
    appendChild() {}
    querySelector(selector) {
        if (!this.children.has(selector)) this.children.set(selector, new Element());
        return this.children.get(selector);
    }
    addEventListener(name, handler) { this.listeners.set(name, handler); }
    remove() { this.removed = true; }
}

function fixture(panelSource = source) {
    const executable = panelSource.replace(/^import[\s\S]*?from\s+['"][^'"]+['"];\s*/gm, '').replace(/export\s+/g, '');
    const state = { live: true, reset: 0, epoch: 1, tick: 10, time: 5, valid: true };
    const samples = [], publications = [], demands = [], retained = [], subs = new Map();
    let historyControl;
    const bridge = {
        getToggle: () => false, getOmega0: () => 2,
        inspectVoxel: () => ({ phase: -Math.PI / 2, speed: 0, latency: 0.6 }),
        replaceSamplerWants: (key, wants) => demands.push([key, Array.from(wants)]),
        getDiagnostics() { throw Error('unexpected direct diagnostics'); },
        getEnergyAudit() { throw Error('unexpected direct audit'); },
        capabilities: { scale0: {
            latticeSize: 3,
            getScale0GravityMetricAgg: () => ({ active: true, fMin: 0.64, dilationMaxPct: 20, gammaMax: 1.25 }),
            getScale0FieldSamples: ({ kind, stride }) => {
                samples.push([kind, stride]);
                if (kind === 'latency') return { positions: [1,1,1, 2,2,2], values: [0.6,0], count: 2 };
                const values = kind === 'tau' ? [2,4] : kind === 'lapse' ? [0.8,1] : [0, Math.PI];
                return { values, count: 2, effectiveStride: 2 };
            },
        } },
    };
    const hub = {
        s0: { get diag() { return { tick: state.tick, physicalTime: state.time, dt: 0.5 }; },
            meta: { get expectedSourceEpoch() { return state.epoch; }, expectedSource: 'fixture' } },
        getResetVersion: () => state.reset,
        getScale0TelemetryMeta: () => state.valid ? { tick: state.tick, stale: false } : null,
        publishScale0ProperTimeMetrics: (value, tick) => publications.push({ value, tick }),
    };
    const context = {
        ...clock, ...cards, ...reference, C_SPEED, telemetryHub: hub,
        document: { createElement: () => new Element(), getElementById: () => null },
        isPanelLive: () => state.live,
        updateRetainedReadout(container, markup) {
            retained.push(container);
            if (container.innerHTML !== markup) container.innerHTML = markup;
        },
        beginTimeReadout(container) {
            const values = [];
            return { slot(value) { values.push(String(value)); return `__FTD_TIME_SLOT_${values.length - 1}__`; },
                commit(markup) {
                    const html = markup.replace(/__FTD_TIME_SLOT_(\d+)__/g, (_, index) => values[Number(index)]);
                    assert.doesNotMatch(html, /__FTD_TIME_SLOT_/, 'slots cannot be nested inside slot values');
                    context.updateRetainedReadout(container, html);
                } };
        },
        updateTimeReadout(container, markup) { context.updateRetainedReadout(container, markup); },
        TickHistoryControl: class {
            constructor(_, opts) { this.isAll = false; this.ticks = opts.defaultTicks; this.change = opts.onChange; historyControl = this; }
            destroy() { this.destroyed = true; }
        },
        rafCoordinator: { subscribe(id, options) {
            const entry = { ...options, unsubscribe() { subs.delete(id); } };
            subs.set(id, entry); return entry;
        } },
    };
    const functions = vm.runInNewContext(executable + '\n({mountTimePanel,renderCardA,renderCardB,renderCardC,renderCardD,renderCardE})', context);
    const api = functions.mountTimePanel(new Element(), () => bridge);
    const card = letter => api.element.querySelector(`#time-panel-card-${letter}`);
    return { state, samples, publications, demands, retained, subs, api, card, functions, bridge, hub,
        get historyControl() { return historyControl; } };
}

test('actual Time update preserves sample kinds, strides, reductions and qualified publication', () => {
    const f = fixture(); f.api.update();
    assert.deepEqual(f.samples, [['latency',2], ['tau',1], ['lapse',1], ['dbPhase',1]]);
    assert.equal(f.publications.length, 1); assert.equal(f.publications[0].tick, 10);
    const metrics = f.publications[0].value;
    assert.equal(metrics.properTimeMean, 3); assert.equal(metrics.properTimeMin, 2);
    assert.equal(metrics.properTimeMax, 4); assert.equal(metrics.lapseMean, 0.9);
    assert.ok(Number.isNaN(metrics.dbPhaseMean)); assert.ok(metrics.dbPhaseCircVar > 0.999999);
    assert.equal(metrics.phaseStride, 2);
    assert.match(f.card('a').innerHTML, /t =  5\.0/);
    assert.match(f.card('a').innerHTML, /0\.80000/);
    assert.match(f.card('b').innerHTML, /reference √\(1−\(L·r₀\/r\)²\)/);
    assert.match(f.card('b').innerHTML, /<path d=/);
    assert.match(f.card('e').innerHTML, /φ =  4\.712 rad/);
    assert.match(f.card('e').innerHTML, /1\.60000/);
    assert.match(f.card('f').innerHTML, /3\.00e\+0/);
    assert.match(f.card('f').innerHTML, /effective stride 2/);
    for (const id of ['a','b','c','e','f']) assert.ok(f.retained.includes(f.card(id)));
    assert.ok(!f.retained.includes(f.card('d')), 'form controls never enter retained renderer');
});

test('same-tick readouts remain current without duplicate quadrature or reduced sampling cadence', () => {
    const f = fixture(); f.api.update();
    f.state.time = 5.25; f.api.update();
    assert.match(f.card('a').innerHTML, /t =  5\.3/);
    assert.equal(f.api.historyLength, 0); assert.equal(f.publications.length, 2);
    assert.equal(f.samples.length, 8);
    f.state.tick = 11; f.state.time = 6; f.api.update();
    assert.equal(f.api.twin.tauDeep, 0.8); assert.equal(f.api.twin.tauFar, 1);
    assert.equal(f.api.historyLength, 1);
    f.api.update(); assert.equal(f.api.historyLength, 1);
    assert.equal(f.samples.length, 16); assert.equal(f.publications.length, 4);
});

test('unavailable readouts recover and Card D rebuilds only on actual imposed-value change', () => {
    const f = fixture(); const d = f.card('d');
    assert.equal(d.writes, 1);
    f.api.update(); f.state.valid = false; f.api.update(); f.api.update();
    for (const id of ['a','b','c']) assert.match(f.card(id).innerHTML, /unavailable/);
    assert.equal(f.samples.length, 4); assert.equal(f.publications.length, 1);
    assert.equal(d.writes, 1);
    f.api.setImposedV(0.3); assert.equal(d.writes, 1);
    f.api.setImposedV(0.7); assert.equal(d.writes, 2);
    assert.match(d.innerHTML, /value="0\.7"/);
    f.api.setImposedV(0.7); f.api.update(); assert.equal(d.writes, 2);
    d.listeners.get('input')({ target: { closest: () => ({ value: '0.8' }) } });
    assert.equal(d.writes, 3); assert.match(d.innerHTML, /value="0\.8"/);
    f.state.valid = true; f.api.update();
    assert.doesNotMatch(f.card('a').innerHTML, /telemetry is unavailable/);
});

test('hidden time clears demand; source reset discards quadrature; disposal releases resources', () => {
    const f = fixture(); f.api.update(); f.state.tick++; f.state.time++; f.api.update();
    assert.equal(f.api.historyLength, 1);
    f.state.live = false; f.api.update();
    assert.equal(f.samples.length, 8); assert.equal(f.publications.length, 2);
    assert.deepEqual(f.demands.at(-1), ['time-panel', []]);
    f.state.epoch++; f.state.tick = 0; f.state.time = 0; f.state.live = true; f.api.update();
    assert.equal(f.api.historyLength, 0); assert.equal(f.api.twin.tauDeep, 0);
    assert.equal(f.subs.get('time-panel-arm').hz, 2);
    f.api.dispose(); assert.equal(f.subs.size, 0);
    assert.equal(f.historyControl.destroyed, true); assert.equal(f.api.element.removed, true);
    assert.deepEqual(f.demands.at(-1), ['time-panel', []]);
});

function visible(twin, control) {
    const start = source.indexOf('    function visibleTwin()');
    const end = source.indexOf('    // Card D', start);
    return vm.runInNewContext(source.slice(start, end) + '\nvisibleTwin()', { twin, historyControl: control });
}
function priorWindow(twin, control) {
    const entries = twin.history.map((value, i) => ({ value, tick: twin.historyTicks[i] ?? i }));
    return TickHistoryControl.prototype.slice.call({ ...control, state: { ticks: control.ticks } }, entries, entry => entry.tick).map(entry => entry.value);
}

test('display window matches existing inclusive tick-window oracle including fallback clocks', () => {
    for (const ticks of [[0,10,20,30], [0,10,20,20], [0,undefined,null,3], [0,NaN,20,30], [0,1,2,Infinity], []]) {
        for (const isAll of [false, true]) {
            const twin = { history: ticks.map((_, i) => 0.25 * i), historyTicks: ticks, tauDeep: 9 };
            const before = structuredClone(twin), control = { isAll, ticks: 10 };
            const got = visible(twin, control);
            assert.deepEqual(Array.from(got.history), priorWindow(twin, control));
            assert.equal(got.tauDeep, 9); assert.deepEqual(twin, before);
        }
    }
    const twin = { history: [1,2,3,4], historyTicks: [0,10,20,30] };
    assert.deepEqual(Array.from(visible(twin, { isAll: false, ticks: 10 }).history), [3,4]);
});

test('rolling display does not read or copy hidden history values; All remains complete', () => {
    const data = Array.from({ length: 1000 }, (_, i) => i / 10), reads = [];
    const history = new Proxy(data, { get(target, key, receiver) {
        if (/^\d+$/.test(String(key))) reads.push(Number(key));
        return Reflect.get(target, key, receiver);
    } });
    const twin = { history, historyTicks: data.map((_, i) => i) };
    const result = visible(twin, { isAll: false, ticks: 10 });
    assert.equal(result.history.length, 11);
    assert.deepEqual(reads, Array.from({ length: 11 }, (_, i) => 989 + i));
    reads.length = 0;
    assert.equal(visible(twin, { isAll: true, ticks: 10 }).history.length, 1000);
    assert.equal(reads.length, 1000); assert.equal(data.length, 1000);
});

// The full-source golden comparison imports this fixture, not extracted or
// reimplemented renderer bodies. The readout sink tests exact authored markup;
// compiled DOM bindings and tooltips have a separate actual-browser suite.
export { fixture, Element };

test('count-driven zero samples and Float32 circular cancellation remain observable', () => {
    const f = fixture();
    const data = {
        tau: { values: new Float32Array([0, 0, 0]), count: 3, effectiveStride: 1 },
        lapse: { values: new Float32Array([0, 0, 0]), count: 3, effectiveStride: 1 },
        dbPhase: { values: new Float32Array([0, Math.PI]), count: 2, effectiveStride: 1 },
    };
    const original = f.bridge.capabilities.scale0.getScale0FieldSamples;
    f.bridge.capabilities.scale0.getScale0FieldSamples = options =>
        options.kind === 'latency' ? original(options) : data[options.kind];
    f.api.update();
    const m = f.publications.at(-1).value;
    assert.equal(m.hasField, true); assert.equal(m.properTimeMean, 0);
    assert.equal(m.properTimeMin, 0); assert.equal(m.properTimeMax, 0);
    assert.equal(m.lapseMean, 0); assert.equal(m.tauCount, 3);
    assert.ok(Number.isNaN(m.dbPhaseMean));
    assert.ok(m.dbPhaseCircVar >= 0 && m.dbPhaseCircVar <= 1);
    assert.match(f.card('f').innerHTML, /3 sampled manifested voxels/);
    data.tau = { values: new Float32Array(0), count: 0 };
    data.dbPhase = { values: new Float32Array([-0.01, 0.01]), count: 2 };
    f.api.update();
    const n = f.publications.at(-1).value;
    assert.ok(Number.isNaN(n.properTimeMean)); assert.equal(n.lapseMean, 0);
    assert.equal(n.dbPhaseMean, 0); assert.ok(n.dbPhaseCircVar < 0.001);
    assert.match(f.card('f').innerHTML, /effective stride unavailable/);
});

test('missed ticks integrate observed physical-time intervals and unavailable gaps only relatch', () => {
    const f = fixture(); f.api.update();
    f.state.tick = 30; f.state.time = 8.75; f.api.update();
    assert.equal(f.api.twin.tauDeep, 0.8 * 3.75);
    assert.equal(f.api.twin.tauFar, 3.75); assert.equal(f.api.historyLength, 1);
    f.state.valid = false; f.api.update();
    f.state.valid = true; f.state.tick = 80; f.state.time = 90; f.api.update();
    assert.equal(f.api.historyLength, 1); assert.equal(f.api.twin.tauFar, 3.75);
    f.state.tick = 81; f.state.time = 90.5; f.api.update();
    assert.equal(f.api.historyLength, 2); assert.equal(f.api.twin.tauFar, 4.25);
});
