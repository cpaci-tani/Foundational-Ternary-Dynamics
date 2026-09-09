// engine/web/tests/scale3-liquid-labs.node.test.mjs
//
// Regression coverage for Task 2 of the hydro-scenarios-part-b brief: the
// four Scale 3 liquid-transport laboratories (scenarios.js recipes,
// scenario-registry.js entries, the molecular-liquid-transport protocol in
// scale2/experiment-runtime.js, and the telemetry-hub.js hookup).
//
// Runs the real JS AtomEngine (mock-atom-engine.js createAtomEngine) headless
// in Node, using the same minimal bridge-stub contract wasm-bridge.js's
// `_ensureAEFallback()` uses (`_boundaryShape`, `_reflectiveBoundary`,
// `setBoundaryShape`, `setFluxBoundaryMode`, `setReflectiveBoundary`,
// `_reflectIntoBoundary`). scale3/controller.js is intentionally NOT
// imported here — it (transitively, via scale2/controller.js) pulls in
// Three.js viewport and DOM-coupled UI modules that do not load under plain
// Node; the pieces under test (scenarios.js, scenario-registry.js,
// experiment-runtime.js, liquid-transport.js, telemetry-hub.js) are all
// DOM-tolerant on their own and are exercised directly instead.

import assert from 'node:assert/strict';
import test from 'node:test';

import { createAtomEngine } from '../js/bridge/mock-atom-engine.js';
import { reflectIntoBoundary } from '../js/bridge/boundary.js';
import { AE_PHYSICS_SPECS } from '../js/scales/scale2/scenario-registry.js';
import { startAEExperiment, advanceAEExperiment, resetAEExperiment } from '../js/scales/scale2/experiment-runtime.js';
import { LiquidTransportTracker, imposeLiquidFlow } from '../js/scales/scale3/liquid-transport.js';
import {
    SCALE3_SCENARIOS, getScale3ScenarioMeta, validateScale3ScenarioRegistry,
} from '../js/scales/scale3/scenario-registry.js';
import { setupScale3Scenario } from '../js/scales/scale3/scenarios.js';
import { telemetryHub } from '../js/telemetry-hub.js';

const LIQUID_IDS = [
    'mol-liquid-shear-layer',
    'mol-liquid-channel-decay',
    'mol-liquid-droplet-diffusion',
    'mol-liquid-spinning-droplet',
];

// The registry held 35 scenarios before this task added the four liquid
// labs (the value scale3-molecule-engine.spec.js asserted pre-Task-2).
const PRE_TASK2_SCENARIO_COUNT = 35;

/**
 * Minimal state object satisfying createAtomEngine(state)'s STATE CONTRACT
 * docblock in mock-atom-engine.js — the same shape wasm-bridge.js's
 * `_ensureAEFallback()` builds to host the JS AtomEngine without a full
 * MockBridge. The returned engine object exposes every `ae*` method
 * directly, so it doubles as the "bridge" the scenario/protocol/telemetry
 * code expects.
 */
function makeBridge() {
    const stub = {
        _boundaryShape: 'cube',
        _reflectiveBoundary: false,
        setBoundaryShape(s) { this._boundaryShape = s; },
        setFluxBoundaryMode() {},
        setReflectiveBoundary(on) { this._reflectiveBoundary = on; },
        _reflectIntoBoundary(p, cx, cy, cz, R) {
            reflectIntoBoundary(this._boundaryShape, p, cx, cy, cz, R, this._reflectiveBoundary);
        },
    };
    const engine = createAtomEngine(stub);
    engine.initAE();
    return engine;
}

/**
 * Mirrors the physics-toggle and dt/softening/thermostatTemp half of
 * scale2/controller.js's applyAEScenarioPhysics. The DOM-syncing half of
 * that function (slider/checkbox mirrors) is skipped: this harness never
 * imports scale2/controller.js (see the file banner), and none of the
 * assertions below depend on DOM widget state.
 */
function applyPhysics(bridge, scenario) {
    for (const spec of AE_PHYSICS_SPECS) bridge[spec.setter]?.(!!scenario.physics[spec.key]);
    bridge.aeSetDt?.(scenario.parameters.dt);
    bridge.aeSetSoftening?.(scenario.parameters.softening);
    bridge.aeSetThermostatTemp?.(scenario.parameters.thermostatTemp);
}

function setupFresh(id) {
    const scenario = getScale3ScenarioMeta(id);
    assert.ok(scenario, `registry has ${id}`);
    const bridge = makeBridge();
    const { atomIds } = setupScale3Scenario(bridge, scenario);
    return { scenario, bridge, atomIds };
}

/** A lightweight bridge stub for protocol tests: no real physics, just
 * position/velocity buffers plus call recorders for aeSetThermostat and
 * aeSetAtomVelocity so the phase-transition wiring can be asserted directly.
 * `atomicNumsList` defaults every atom to Z=0 (never a wall species) since
 * these tests exercise the flow-imposition formulas, not the wall filter. */
function makeRecordingStub(positionsList, atomicNumsList = null) {
    const count = positionsList.length;
    const positions = new Float32Array(count * 3);
    positionsList.forEach((p, i) => { positions[3 * i] = p[0]; positions[3 * i + 1] = p[1]; positions[3 * i + 2] = p[2]; });
    const velocities = new Float32Array(count * 3);
    const ids = Int32Array.from({ length: count }, (_, i) => i);
    const atomicNums = Int32Array.from({ length: count }, (_, i) => (atomicNumsList ? atomicNumsList[i] : 0));
    const calls = { thermostat: [], setVelocity: [] };
    return {
        aeSetThermostat: (on) => calls.thermostat.push(on),
        aeSetAtomVelocity: (id, vx, vy, vz) => calls.setVelocity.push([id, vx, vy, vz]),
        aeGetAtomData: () => ({ positions, ids, count, atomicNums }),
        aeGetVelocities: () => ({ velocities, count }),
        // Double-precision per-atom read (M6): this stub's velocities never
        // move independently of aeSetAtomVelocity's recorded calls, so the
        // stored Float32 velocities (all zero, never mutated) are exact.
        aeInspectAtom: (id) => ({ vx: velocities[3 * id], vy: velocities[3 * id + 1], vz: velocities[3 * id + 2] }),
        aeSetExperimentState: () => {},
        aeGetRuntimeState: () => ({ experiment: null }),
        calls,
    };
}

test('validateScale3ScenarioRegistry passes and the scenario count grew by exactly 4', () => {
    const result = validateScale3ScenarioRegistry();
    assert.deepEqual(result, { ok: true, errors: [], count: PRE_TASK2_SCENARIO_COUNT + 4 });
    assert.equal(result.count, SCALE3_SCENARIOS.length);
    for (const id of LIQUID_IDS) {
        assert.ok(SCALE3_SCENARIOS.some((s) => s.id === id), `registry contains ${id}`);
    }
});

for (const id of LIQUID_IDS) {
    test(`${id}: seed topology matches the registry's expected atom/bond/component counts`, () => {
        const { scenario, bridge } = setupFresh(id);
        const data = bridge.aeGetAtomData();
        const mol = bridge.aeGetMoleculeDiagnostics();
        assert.equal(data.count, scenario.expected.atomCount, 'atomCount');
        assert.equal(data.bondCount, scenario.expected.bondCount, 'bondCount');
        assert.equal(mol.componentCount, scenario.expected.componentCount, 'componentCount');
    });

    test(`${id}: placeWaters leaves ~zero total momentum before the thermostat acts`, () => {
        const { bridge, atomIds } = setupFresh(id);
        // aeGetAtomData()/aeGetVelocities() truncate to Float32Array for the
        // renderer, which is too lossy for a momentum-near-zero assertion at
        // 1e-9 * n; aeInspectAtom() reads the underlying double-precision
        // per-atom state directly.
        let px = 0, py = 0, pz = 0;
        for (const atomId of atomIds) {
            const a = bridge.aeInspectAtom(atomId);
            px += a.mass * a.vx; py += a.mass * a.vy; pz += a.mass * a.vz;
        }
        const p = Math.hypot(px, py, pz);
        const threshold = 1e-9 * atomIds.length;
        assert.ok(p < threshold, `|P| ${p} should be < 1e-9 * n = ${threshold}`);
    });

    test(`${id}: positions and velocities stay finite over 40 ticks with the thermostat on`, () => {
        resetAEExperiment();
        const { scenario, bridge } = setupFresh(id);
        applyPhysics(bridge, scenario);
        startAEExperiment(scenario, bridge);
        for (let t = 0; t < 40; t++) { bridge.aeTick(); advanceAEExperiment(bridge); }
        const data = bridge.aeGetAtomData();
        const vel = bridge.aeGetVelocities();
        assert.ok(Array.from(data.positions).every(Number.isFinite), 'positions finite');
        assert.ok(Array.from(vel.velocities).every(Number.isFinite), 'velocities finite');
        assert.equal(bridge.aeGetDiagnostics().lastError, 'ok');
        resetAEExperiment();
    });

    test(`${id}: the same seed reproduces identical seed positions (determinism)`, () => {
        const a = setupFresh(id).bridge.aeGetAtomData();
        const b = setupFresh(id).bridge.aeGetAtomData();
        assert.deepEqual(Array.from(a.positions), Array.from(b.positions));
    });
}

test('molecular-liquid-transport protocol threads thermostat on -> impose flow -> thermostat off', () => {
    resetAEExperiment();
    const scenario = getScale3ScenarioMeta('mol-liquid-shear-layer');
    // Two atoms above the shear plane (y >= 0), two below, so imposeLiquidFlow's
    // 'shear-layer' branch exercises both signs of the counter-flow step.
    const bridge = makeRecordingStub([[0, 5, 0], [0, -5, 0], [0, 3, 0], [0, -3, 0]]);
    startAEExperiment(scenario, bridge);
    for (let t = 0; t < 300; t++) advanceAEExperiment(bridge);
    assert.deepEqual(bridge.calls.thermostat, [true, false], 'thermostat toggles on then off');
    assert.ok(bridge.calls.setVelocity.length >= 1, 'imposeLiquidFlow set at least one atom velocity');
    assert.equal(bridge.calls.setVelocity.length, 4, 'imposeLiquidFlow visited every atom');
    resetAEExperiment();
});

test('molecular-liquid-transport protocol threads the scenario for every liquid kind (channel, droplet, spinning-droplet)', () => {
    // mol-liquid-droplet-diffusion imposes no flow (M6): the kind is a
    // free-flight measurement, so imposeLiquidFlow is a declared no-op and
    // visits zero atoms, not every atom.
    const expectedSetVelocityCalls = {
        'mol-liquid-channel-decay': 4,
        'mol-liquid-droplet-diffusion': 0,
        'mol-liquid-spinning-droplet': 4,
    };
    for (const id of Object.keys(expectedSetVelocityCalls)) {
        resetAEExperiment();
        const scenario = getScale3ScenarioMeta(id);
        const bridge = makeRecordingStub([[0, 5, 0], [0, -5, 0], [3, 0, 0], [-3, 0, 0]]);
        startAEExperiment(scenario, bridge);
        for (let t = 0; t < 300; t++) advanceAEExperiment(bridge);
        assert.deepEqual(bridge.calls.thermostat, [true, false], `${id}: thermostat toggles on then off`);
        assert.equal(bridge.calls.setVelocity.length, expectedSetVelocityCalls[id], `${id}: imposeLiquidFlow visited the expected number of atoms`);
        resetAEExperiment();
    }
});

test('imposeLiquidFlow (M6): the channel lab skips locked wall atoms and leaves diagnostics clean', () => {
    const { scenario, bridge } = setupFresh('mol-liquid-channel-decay');
    const data = bridge.aeGetAtomData();
    const wallIds = [];
    const before = new Map();
    for (let i = 0; i < data.count; i++) {
        const a = bridge.aeInspectAtom(data.ids[i]);
        before.set(data.ids[i], [a.vx, a.vy, a.vz]);
        if (data.atomicNums[i] === scenario.liquid.wallZ) wallIds.push(data.ids[i]);
    }
    assert.ok(wallIds.length > 0, 'the channel lab seeds locked argon wall atoms');

    imposeLiquidFlow(bridge, scenario);

    assert.equal(bridge.aeGetDiagnostics().lastError, 'ok', 'no rejected aeSetAtomVelocity call on a wall atom');
    for (const id of wallIds) {
        const a = bridge.aeInspectAtom(id);
        const [vx, vy, vz] = before.get(id);
        assert.equal(a.vx, vx, `wall atom ${id} vx must be untouched`);
        assert.equal(a.vy, vy, `wall atom ${id} vy must be untouched`);
        assert.equal(a.vz, vz, `wall atom ${id} vz must be untouched`);
    }
});

test('imposeLiquidFlow (M6): the droplet lab is a declared no-op (the kind imposes no flow)', () => {
    const { bridge, atomIds } = setupFresh('mol-liquid-droplet-diffusion');
    const before = atomIds.map((id) => { const a = bridge.aeInspectAtom(id); return [a.vx, a.vy, a.vz]; });

    imposeLiquidFlow(bridge, getScale3ScenarioMeta('mol-liquid-droplet-diffusion'));

    atomIds.forEach((id, i) => {
        const a = bridge.aeInspectAtom(id);
        assert.deepEqual([a.vx, a.vy, a.vz], before[i], `atom ${id} velocity must be unchanged`);
    });
});

test('telemetry-hub: attachLiquidTracker + collectScale2 populate s2.liquid across the thermalize/measure boundary', () => {
    resetAEExperiment();
    const scenario = getScale3ScenarioMeta('mol-liquid-shear-layer');
    const bridge = makeBridge();
    setupScale3Scenario(bridge, scenario);
    applyPhysics(bridge, scenario);
    telemetryHub.attachLiquidTracker(new LiquidTransportTracker(scenario.liquid, scenario.parameters.dt));
    startAEExperiment(scenario, bridge);

    for (let t = 0; t < 40; t++) { bridge.aeTick(); advanceAEExperiment(bridge); telemetryHub.collectScale2(bridge); }
    assert.equal(telemetryHub.s2.liquid.status, 'thermalizing', 'still before the tick-300 measurement window');

    for (let t = 40; t < 320; t++) { bridge.aeTick(); advanceAEExperiment(bridge); telemetryHub.collectScale2(bridge); }
    assert.equal(telemetryHub.s2.liquid.status, 'measuring');
    assert.equal(telemetryHub.s2.liquid.quantity, 'nu');
    assert.ok(Number.isFinite(telemetryHub.s2.liquid.value));
    assert.ok(telemetryHub.s2.liquid.samples >= 4);

    telemetryHub.attachLiquidTracker(null);
    resetAEExperiment();
});
