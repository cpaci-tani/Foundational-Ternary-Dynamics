// @ts-check
/**
 * Contract locks for the 2026-08 web/native boundary fixes.
 * These import the helper modules directly (no Chromium / WASM required).
 */
import { test, expect } from '@playwright/test';
import fs from 'node:fs';
import path from 'node:path';
import vm from 'node:vm';
import { fileURLToPath } from 'node:url';

import { visualSampleGrid } from '../js/lib/visual-sample-grid.js';
import { parseFtv2Frame, FTV2_MAGIC } from '../js/lib/ftv2.js';
import { wsOriginAllowed, parseNativeWsPort } from '../js/lib/origin-policy.js';
import { createSamplerWantSet } from '../js/bridge/sampler-want-set.js';
import { SCALE0_SAMPLER_METHODS } from '../js/bridge/bridge-contract.js';
import {
    collectScale0OnDemand,
    getScale0TelemetryDemand,
} from '../js/telemetry/demand.js';

const webRoot = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');

function loadClassicSamplerCadence() {
    const src = fs.readFileSync(
        path.join(webRoot, 'js', 'bridge', 'sampler-cadence.classic.js'),
        'utf8',
    );
    const context = {};
    context.self = context;
    context.globalThis = context;
    vm.createContext(context);
    vm.runInContext(src, context);
    return context.FTD_SAMPLER_CADENCE;
}

test('worker gravity sampler names the embind export, not the JS alias', () => {
    const src = fs.readFileSync(
        path.join(webRoot, 'js', 'bridge', 'sampler-registry.classic.js'),
        'utf8',
    );
    expect(src).toMatch(/gravity:\s*\[['"]getGravityFieldSampled['"]/);
    expect(src).not.toMatch(/gravity:\s*\[['"]getGravityForceField['"]/);
});

test('worker command dispatch is allowlisted', () => {
    const src = fs.readFileSync(
        path.join(webRoot, 'js', 'bridge', 'wasm-bridge.worker.js'),
        'utf8',
    );
    expect(src).toMatch(/WORKER_COMMAND_ALLOWLIST/);
    expect(src).toMatch(/if\s*\(\s*!WORKER_COMMAND_ALLOWLIST\.has\(method\)/);
});

test('centre-anchored visual grid matches C++ for stride-3 L=33', () => {
    expect(visualSampleGrid(33, 3, false)).toEqual({ stride: 3, origin: 1, count: 11 });
});

test('FTV2 parser reads origin from 20-byte headers and infers it on 16-byte frames', () => {
    const axis = 3;
    const count = axis ** 3;
    const v20 = new ArrayBuffer(20 + count * 4);
    const h20 = new DataView(v20);
    h20.setUint32(0, FTV2_MAGIC, true);
    h20.setUint32(4, 33, true);
    h20.setUint32(8, 3, true);
    h20.setUint32(12, 1, true);
    h20.setUint32(16, axis, true);
    new Float32Array(v20, 20, count)[0] = 1.5;

    const v16 = new ArrayBuffer(16 + count * 4);
    const h16 = new DataView(v16);
    h16.setUint32(0, FTV2_MAGIC, true);
    h16.setUint32(4, 33, true);
    h16.setUint32(8, 3, true);
    h16.setUint32(12, axis, true);
    new Float32Array(v16, 16, count)[2] = 2.25;

    const modern = parseFtv2Frame(v20);
    const legacy = parseFtv2Frame(v16);
    expect(modern.origin).toBe(1);
    expect(modern.axisCount).toBe(3);
    expect(modern.data[0]).toBeCloseTo(1.5);
    expect(legacy.origin).toBe(1);
    expect(legacy.data[2]).toBeCloseTo(2.25);
});

test('Origin allowlist accepts loopback and rejects foreign sites', () => {
    expect(wsOriginAllowed('')).toBe(true);
    expect(wsOriginAllowed('', false)).toBe(false);
    expect(wsOriginAllowed('null')).toBe(true);
    expect(wsOriginAllowed('null', false)).toBe(false);
    expect(wsOriginAllowed('file:///C:/ftd/engine/web/index.html', false)).toBe(false);
    expect(wsOriginAllowed('http://localhost:8080')).toBe(true);
    expect(wsOriginAllowed('http://127.0.0.1:9100')).toBe(true);
    expect(wsOriginAllowed('http://[::1]:8080')).toBe(true);
    expect(wsOriginAllowed('https://evil.example')).toBe(false);
    expect(parseNativeWsPort('?wsPort=5555', 'http://127.0.0.1:8080')).toBe(5555);
    expect(parseNativeWsPort('?wsPort=5555', 'https://evil.example')).toBe(9100);
});

test('toolbar Reset/Step and keyboard handlers cover planetary/cosmic/meta', () => {
    const app = fs.readFileSync(path.join(webRoot, 'js', 'app.js'), 'utf8');
    const resetStart = app.indexOf("getElementById('btn-reset')");
    const resetEnd = app.indexOf("getElementById('ticks-per-frame')", resetStart);
    const reset = app.slice(resetStart, resetEnd);
    expect(reset).toMatch(/engineMode === 'planetary'/);
    expect(reset).toMatch(/engineMode === 'cosmic'/);
    expect(reset).toMatch(/engineMode === 'meta'/);

    const stepKb = app.slice(app.indexOf('stepScenario:'), app.indexOf('reloadScenario:'));
    expect(stepKb).toMatch(/planetary/);
    expect(stepKb).toMatch(/cosmic/);
    expect(stepKb).toMatch(/meta/);

    const reloadKb = app.slice(app.indexOf('reloadScenario:'), app.indexOf('Scale0Controller,'));
    expect(reloadKb).toMatch(/planetary/);
    expect(reloadKb).toMatch(/cosmic/);
    expect(reloadKb).toMatch(/meta/);
});

test('sampler want-set unions owners and unwants only dropped keys', () => {
    const ops = [];
    const set = createSamplerWantSet((op, kind, stride) => ops.push(`${op}:${kind}@${stride}`));
    set.replace('overlays', ['e@2', 'b@2']);
    set.replace('gravity-panel', ['latency@2']);
    expect([...set.wanted()].sort()).toEqual(['b@2', 'e@2', 'latency@2']);
    set.replace('overlays', ['e@2']);
    expect(ops.filter((x) => x.startsWith('unwant:'))).toEqual(['unwant:b@2']);
    set.replace('gravity-panel', []);
    expect(set.wanted().has('latency@2')).toBe(false);
    expect(set.wanted().has('e@2')).toBe(true);
});

test('sampler want-set can publish one atomic multi-key replacement', () => {
    const batches = [];
    const set = createSamplerWantSet(
        () => { throw new Error('individual sampler fan-out must not run'); },
        (changes) => batches.push(changes),
    );
    set.replace('gravity-panel', [
        'latency@6', 'kretschmann@6', 'gravity@6', 'gravityMetricAgg@0',
    ], { cadenceClass: 'bounded-instrument' });
    expect(batches).toHaveLength(1);
    expect(batches[0]).toHaveLength(4);
    expect(batches[0].map(({ op, kind, stride }) => `${op}:${kind}@${stride}`).sort())
        .toEqual([
            'want:gravity@6',
            'want:gravityMetricAgg@0',
            'want:kretschmann@6',
            'want:latency@6',
        ]);
    expect(batches[0].every(({ cadenceClass }) => cadenceClass === 'bounded-instrument'))
        .toBe(true);
});

test('worker Gravity samplers share one bounded 4 Hz batch and paused hydration', () => {
    const {
        createBoundedSamplerCadence,
        visitScheduledSamplers,
    } = loadClassicSamplerCadence();
    const wants = new Map([
        ['latency@6', { kind: 'latency', stride: 6, cadenceClass: 'bounded-instrument' }],
        ['kretschmann@6', { kind: 'kretschmann', stride: 6, cadenceClass: 'bounded-instrument' }],
        ['gravity@6', { kind: 'gravity', stride: 6, cadenceClass: 'bounded-instrument' }],
        ['gravityMetricAgg@0', { kind: 'gravityMetricAgg', stride: 0, cadenceClass: 'bounded-instrument' }],
    ]);
    const counts = Object.fromEntries([...wants.keys()].map((key) => [key, 0]));
    const cadence = createBoundedSamplerCadence();

    for (let frame = 0; frame < 120; frame += 1) {
        visitScheduledSamplers(wants, {
            wantGravity: true,
            cadence,
            nowMs: frame * (1000 / 60),
        }, (key) => { counts[key] += 1; });
    }
    expect(counts).toEqual({
        'latency@6': 8,
        'kretschmann@6': 8,
        'gravity@6': 8,
        'gravityMetricAgg@0': 8,
    });

    const pausedCounts = Object.fromEntries([...wants.keys()].map((key) => [key, 0]));
    const pausedCadence = createBoundedSamplerCadence();
    visitScheduledSamplers(wants, {
        wantGravity: true,
        cadence: pausedCadence,
        nowMs: 10,
        forceGravityBatch: true,
    }, (key) => { pausedCounts[key] += 1; });
    // A second postFrame at the same paused instant is not another sample.
    visitScheduledSamplers(wants, {
        wantGravity: true,
        cadence: pausedCadence,
        nowMs: 10,
    }, (key) => { pausedCounts[key] += 1; });
    expect(pausedCounts).toEqual({
        'latency@6': 1,
        'kretschmann@6': 1,
        'gravity@6': 1,
        'gravityMetricAgg@0': 1,
    });
});

test('Time-only demand schedules one bounded aggregate owner; hidden demand schedules none', () => {
    const {
        createBoundedSamplerCadence,
        visitScheduledSamplers,
    } = loadClassicSamplerCadence();
    const timeWants = new Map([
        ['latency@2', { kind: 'latency', stride: 2, cadenceClass: 'bounded-instrument' }],
    ]);
    const timeCounts = { 'latency@2': 0, 'gravityMetricAgg@0': 0 };
    const timeCadence = createBoundedSamplerCadence();
    for (let frame = 0; frame < 120; frame += 1) {
        visitScheduledSamplers(timeWants, {
            wantGravity: true,
            cadence: timeCadence,
            nowMs: frame * (1000 / 60),
        }, (key) => { timeCounts[key] += 1; });
    }
    expect(timeCounts).toEqual({ 'latency@2': 8, 'gravityMetricAgg@0': 8 });

    // Even a briefly stale direct aggregate want cannot outlive telemetry
    // demand after both panels are hidden (or Empty makes gravity inapplicable).
    const staleHiddenWants = new Map([
        ['gravityMetricAgg@0', { kind: 'gravityMetricAgg', stride: 0, cadenceClass: 'bounded-instrument' }],
    ]);
    let hiddenAggregateCalls = 0;
    const hiddenCadence = createBoundedSamplerCadence();
    for (let frame = 0; frame < 120; frame += 1) {
        visitScheduledSamplers(staleHiddenWants, {
            wantGravity: false,
            cadence: hiddenCadence,
            nowMs: frame * (1000 / 60),
        }, () => { hiddenAggregateCalls += 1; });
    }
    expect(hiddenAggregateCalls).toBe(0);
});

test('ordinary overlays stay realtime and realtime wins a shared instrument key', () => {
    const {
        createBoundedSamplerCadence,
        visitScheduledSamplers,
    } = loadClassicSamplerCadence();
    const countOver120 = (wants, wantGravity = true) => {
        const counts = {};
        const cadence = createBoundedSamplerCadence();
        for (let frame = 0; frame < 120; frame += 1) {
            visitScheduledSamplers(wants, {
                wantGravity,
                cadence,
                nowMs: frame * (1000 / 60),
            }, (key) => { counts[key] = (counts[key] || 0) + 1; });
        }
        return counts;
    };

    expect(countOver120(new Map([
        ['gravity@2', { kind: 'gravity', stride: 2, cadenceClass: 'realtime' }],
    ]), false)).toEqual({ 'gravity@2': 120 });

    const batches = [];
    const set = createSamplerWantSet(
        () => {},
        (changes) => batches.push(changes),
    );
    set.replace('time-panel', ['latency@2'], { cadenceClass: 'bounded-instrument' });
    set.replace('viewport-overlay', ['latency@2'], { cadenceClass: 'realtime' });
    const effective = batches.flat().filter(({ op, kind, stride }) => (
        op === 'want' && kind === 'latency' && stride === 2
    )).at(-1);
    expect(effective?.cadenceClass).toBe('realtime');
    expect(countOver120(new Map([
        ['latency@2', { kind: 'latency', stride: 2, cadenceClass: effective.cadenceClass }],
    ]), false)).toEqual({ 'latency@2': 120 });
});

test('worker audit demand gate is zero-work while hidden and hydrates once when reopened paused', () => {
    const { advanceDemandFrameCadence } = loadClassicSamplerCadence();
    let counter = 0;
    let hasSample = false;
    let calls = 0;

    for (let frame = 0; frame < 120; frame += 1) {
        const gate = advanceDemandFrameCadence(false, counter, hasSample, 4);
        counter = gate.nextCounter;
        if (gate.sample) { calls += 1; hasSample = true; }
    }
    expect(calls).toBe(0);

    // false -> true while paused: one current sample, not one per publication.
    let gate = advanceDemandFrameCadence(true, counter, hasSample, 4);
    counter = gate.nextCounter;
    if (gate.sample) { calls += 1; hasSample = true; }
    gate = advanceDemandFrameCadence(true, counter, hasSample, 4);
    if (gate.sample) calls += 1;
    expect(calls).toBe(1);

    const worker = fs.readFileSync(
        path.join(webRoot, 'js', 'bridge', 'wasm-bridge.worker.js'),
        'utf8',
    );
    expect(worker).toContain('advanceDemandFrameCadence(');
    expect(worker).toContain('mod.getEnergyLedger(bridge)');
    expect(worker).toMatch(/if \(auditGate\.sample\)[\s\S]+mod\.getEnergyAudit\(bridge\)/);
    expect(worker).toContain("status: 'inactive'");
    expect(worker).toMatch(/auditChanged[\s\S]+postFrame\(false, gravityBecameWanted\)/);
});

test('Scale-0 Time/Gravity demand reaches the worker mask and Empty suppresses it', () => {
    const readyState = {
        currentScenarioId: 's0-seed-gravitational-wave',
        authoritativeLoad: null,
        qualificationAnchor: {
            scenarioId: 's0-seed-gravitational-wave',
            loadGeneration: 7,
        },
    };
    const ctxFor = (visibleId) => ({
        bridge: { latticeSize: 65 },
        isPanelVisible: (id) => id === visibleId,
    });
    expect(getScale0TelemetryDemand(ctxFor('time'), readyState).wantGravity).toBe(true);
    expect(getScale0TelemetryDemand(ctxFor('controls'), readyState).wantGravity).toBe(false);
    expect(getScale0TelemetryDemand(ctxFor('time'), {
        ...readyState,
        currentScenarioId: 'empty',
        qualificationAnchor: { scenarioId: 'empty', loadGeneration: 8 },
    }).wantGravity).toBe(false);

    const masks = [];
    collectScale0OnDemand({
        _lastAuditVersion: 0,
        _prevWantAudit: false,
        _prevWantLag: false,
    }, {}, {
        useFluxMock: true,
        fluxMock: { setTelemetryMask: (...args) => masks.push(args) },
        fieldDataVersion: 0,
    }, {
        wantAudit: false,
        wantLag: false,
        wantGravity: true,
    });
    expect(masks).toEqual([[false, false, true]]);

    const timePanel = fs.readFileSync(
        path.join(webRoot, 'js', 'scales', 'scale0', 'ui', 'overlays', 'time-panel.js'),
        'utf8',
    );
    expect(timePanel).toContain("replaceSamplerWants?.('time-panel', [`latency@${sampleStride}`])");
    expect(timePanel).toContain('getScale0GravityMetricAgg?.()');
    expect(timePanel).not.toMatch(/replaceSamplerWants[^\n]+gravityMetricAgg/);
});

test('worker sampler batch protocol has a matching cache-busted client and handler', () => {
    const proxy = fs.readFileSync(
        path.join(webRoot, 'js', 'bridge', 'wasm-bridge-proxy.js'),
        'utf8',
    );
    const worker = fs.readFileSync(
        path.join(webRoot, 'js', 'bridge', 'wasm-bridge.worker.js'),
        'utf8',
    );
    expect(proxy).toContain("wasm-bridge.worker.js?v=9");
    expect(proxy).toContain("type: 'replaceSamplerWants'");
    expect(proxy).toContain('wantGravity: g');
    expect(proxy).toContain("owner === 'gravity-panel' || owner === 'time-panel'");
    expect(proxy).toContain("cadenceClass: boundedInstrument ? 'bounded-instrument' : 'realtime'");
    expect(worker).toContain("case 'replaceSamplerWants'");
    expect(worker).toContain('sampler-cadence.classic.js?v=2');
    expect(worker).toContain('visitScheduledSamplers(wantedSamplers');
});

test('classic sampler registry covers every SCALE0_SAMPLER_METHODS key', () => {
    const classic = fs.readFileSync(
        path.join(webRoot, 'js', 'bridge', 'sampler-registry.classic.js'),
        'utf8',
    );
    for (const kind of Object.keys(SCALE0_SAMPLER_METHODS)) {
        expect(classic).toMatch(new RegExp(`\\b${kind}\\s*:`));
    }
    const worker = fs.readFileSync(
        path.join(webRoot, 'js', 'bridge', 'wasm-bridge.worker.js'),
        'utf8',
    );
    expect(worker).toMatch(/sampler-registry\.classic\.js/);
    expect(worker).toMatch(/PTHREAD_POOL_SIZE=8/);
});

test('dev server COEP is credentialless, sends CSP, and loopback-gates GPU download', () => {
    const serve = fs.readFileSync(path.join(webRoot, 'serve.py'), 'utf8');
    expect(serve).toMatch(/Cross-Origin-Embedder-Policy".*credentialless/);
    expect(serve).toMatch(/Content-Security-Policy/);
    expect(serve).toMatch(/unsafe-eval/);
    expect(serve).toMatch(/cdn\.jsdelivr\.net/);
    const download = serve.slice(serve.indexOf('def _gpu_download'), serve.indexOf('def main'));
    expect(download).toMatch(/_client_is_local/);
    const status = serve.slice(serve.indexOf('def _gpu_status'), serve.indexOf('def _gpu_start'));
    expect(status).toMatch(/_client_is_local/);
});

test('index hydrates theme, loads Outfit with CORS, and drops layout.css', () => {
    const html = fs.readFileSync(path.join(webRoot, 'index.html'), 'utf8');
    expect(html).toMatch(/family=Outfit/);
    expect(html).toMatch(/FTD_DEFER_CSS/);
    expect(html).toMatch(/crossOrigin: 'anonymous'/);
    expect(html).toMatch(/localStorage\.getItem\('ftd-theme'\)/);
    expect(html).not.toMatch(/href="css\/layout\.css"/);
});

test('unknown WS binaries are dropped, not parsed as particles', () => {
    const src = fs.readFileSync(path.join(webRoot, 'js', 'ws-bridge.js'), 'utf8');
    expect(src).toMatch(/Ignoring unknown binary frame magic/);
    expect(src).not.toMatch(/Format: \[uint32 count\]\[float32 pos/);
});

test('in-thread energy-audit view maps manifested/strong/weak at 28-30', () => {
    const wasm = fs.readFileSync(path.join(webRoot, '..', 'wasm', 'ftd_wasm.cpp'), 'utf8');
    expect(wasm).toMatch(/s_audit_cache\(31\)/);
    expect(wasm).toMatch(/typed_memory_view\(31/);
    const bridge = fs.readFileSync(path.join(webRoot, 'js', 'bridge', 'wasm-bridge.js'), 'utf8');
    expect(bridge).toMatch(/manifested:\s*arr\[28\]/);
    expect(bridge).toMatch(/strongEnergy:\s*arr\[29\]/);
    expect(bridge).toMatch(/weakEnergy:\s*arr\[30\]/);
    expect(bridge).toMatch(/get _aeHasWasm[\s\S]*return false/);
});

test('wasm32 worker is not used above the Memory64 lattice cap', () => {
    const src = fs.readFileSync(
        path.join(webRoot, 'js', 'scales', 'scale0', 'runtime', 'scenario-loader.js'),
        'utf8',
    );
    expect(src).toMatch(/WASM32_LATTICE_CAP\s*=\s*117/);
    expect(src).toMatch(/N > WASM32_LATTICE_CAP/);
});

test('reduced-motion includes the loading overlay; GPU card uses tokens', () => {
    const tokens = fs.readFileSync(path.join(webRoot, 'css', 'tokens.css'), 'utf8');
    expect(tokens).not.toMatch(/#loading-overlay \*/);
    const gpu = fs.readFileSync(path.join(webRoot, 'css', 'ui', 'components', 'gpu-server-card.css'), 'utf8');
    expect(gpu).toMatch(/var\(--text-primary/);
    expect(gpu).not.toMatch(/color:\s*#e6ecf5/);
});

test('FAQ uses reference-frame-structure vocabulary; Scale 1 uses PROTON_RATIO', () => {
    const faq = fs.readFileSync(path.join(webRoot, 'js', 'ui', 'components', 'faq', 'data.js'), 'utf8');
    expect(faq).toMatch(/hard-problem-reference-frame-structure/);
    expect(faq).not.toMatch(/hard-problem-reference frame context/);
    const s1 = fs.readFileSync(path.join(webRoot, 'js', 'scales', 'scale1', 'scenario-registry.js'), 'utf8');
    expect(s1).toMatch(/PROTON_RATIO \* K_B/);
    expect(s1).not.toMatch(/1836 \* K_B/);
});

test('theme swatches are keyboard radios', () => {
    const tmpl = fs.readFileSync(
        path.join(webRoot, 'js', 'ui', 'components', 'settings-modal', 'template.js'),
        'utf8',
    );
    expect(tmpl).toMatch(/role="radio"/);
    expect(tmpl).toMatch(/role="radiogroup"/);
    const app = fs.readFileSync(path.join(webRoot, 'js', 'app.js'), 'utf8');
    expect(app).toMatch(/ArrowRight/);
    expect(app).toMatch(/sw\.tabIndex = on \? 0 : -1/);
});

test('non-critical CSS is injected after window.load', () => {
    const html = fs.readFileSync(path.join(webRoot, 'index.html'), 'utf8');
    expect(html).toMatch(/window\.FTD_DEFER_CSS/);
    expect(html).toMatch(/addEventListener\('load', inject\)/);
    expect(html).toMatch(/css\/ui\/panels\/zoo-panel\.css/);
    expect(html).not.toMatch(/<link rel="stylesheet" href="css\/ui\/panels\/zoo-panel\.css"/);
    const serve = fs.readFileSync(path.join(webRoot, 'serve.py'), 'utf8');
    expect(serve).toMatch(/script-src.*blob:/);
});

test('flux publish SAB header is 8-byte aligned for Float64Array', () => {
    const n = 8;
    const buf = new ArrayBuffer(8 + 2 * n * 8);
    expect(() => new Float64Array(buf, 8, n)).not.toThrow();
    expect(() => new Float64Array(buf, 4, n)).toThrow();
    const worker = fs.readFileSync(
        path.join(webRoot, 'js', 'bridge', 'wasm-bridge.worker.js'),
        'utf8',
    );
    const proxy = fs.readFileSync(
        path.join(webRoot, 'js', 'bridge', 'wasm-bridge-proxy.js'),
        'utf8',
    );
    expect(worker).toMatch(/const header = 8/);
    expect(proxy).toMatch(/8 \+ slot \* n \* 8/);
});

test('latency samplers preserve request provenance and TOGGLE_REQUIRES is imported', () => {
    const wasm = fs.readFileSync(path.join(webRoot, 'js', 'bridge', 'wasm-bridge.js'), 'utf8');
    const proxy = fs.readFileSync(path.join(webRoot, 'js', 'bridge', 'wasm-bridge-proxy.js'), 'utf8');
    const contract = fs.readFileSync(path.join(webRoot, 'js', 'bridge', 'bridge-contract.js'), 'utf8');
    const bindings = fs.readFileSync(
        path.resolve(webRoot, '..', 'wasm', 'bindings_render_bridge.cpp'),
        'utf8',
    );
    const gravityPanel = fs.readFileSync(
        path.join(webRoot, 'js', 'scales', 'scale0', 'ui', 'overlays', 'gravity-panel.js'),
        'utf8',
    );
    expect(contract).toMatch(/export const TOGGLE_REQUIRES/);
    expect(wasm).toMatch(/TOGGLE_REQUIRES/);
    expect(wasm).toMatch(/getPoissonLatencySampled/);
    expect(proxy).toMatch(/getPoissonLatencySampled/);
    expect(bindings).toMatch(/r\.set\("requested",\s+a\.requested\)/);
    expect(wasm).toMatch(/requested: null/);
    expect(proxy).toMatch(/requested: null/);
    expect(gravityPanel).toContain('requested — no nonzero latency cells in this engine observation');
    expect(gravityPanel).toContain('inactive — Poisson-latency operator not requested');
});
