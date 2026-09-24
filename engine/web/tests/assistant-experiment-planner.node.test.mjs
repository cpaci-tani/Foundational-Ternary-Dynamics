import test from 'node:test';
import assert from 'node:assert/strict';
import { proposeExperiment, compactExperimentContext, compactExperimentObservation } from '../js/assistant/experiment-planner.js';
import { BrowserModel } from '../js/assistant/model.js';
import { OBSERVER_ACTION_DESCRIPTORS } from '../js/assistant/observer-control.js';

const signal = () => new AbortController().signal;
function observation() {
    return { workspace: 'lattice', ownerId: 'owner-one', preparationVersion: 'same-preparation', tick: '100',
        facts: { running: false, available: true, sampleTick: '100', manifested: '12', completedTick: '100' },
        capabilities: ['lattice.step'], actions: [{ type: 'lattice.step', description: 'Advance a bounded number of ticks',
            args: { type: 'object', properties: { count: { type: 'integer', minimum: 1, maximum: 4096 } }, required: ['count'], additionalProperties: false } }] };
}
const selection = (phase, overrides = {}) => ({ phase, command: phase === 'act' ? 'Advance 10 ticks.' : '',
    waitMs: phase === 'observe' ? 250 : 0, reason: 'Inspect the next completed sample.', ...overrides });
const step = count => ({ kind: 'actions', message: '', actions: [{ type: 'lattice.step', args: { count } }] });

test('experiment planning receives the fresh sample, baseline, acknowledged history and budget separately from commands', async () => {
    const current = observation(), context = { baseline: { tick: '90', manifested: '10' },
        recent: [{ action: 'lattice.step', status: 'applied', completedTicks: 10 }], remainingActions: 49 };
    let payload, translated;
    const model = { generate: async messages => {
        payload = JSON.parse(messages.at(-1).content);
        return JSON.stringify(selection('act'));
    }, plan: async (command, received) => { translated = command; assert.equal(received, current); return step(10); } };
    const result = await proposeExperiment(model, 'Measure how the count changes over another ten ticks.', current, context, signal());
    assert.equal(payload.observation.tick, '100');
    assert.equal(payload.observation.facts.manifested, '12');
    assert.deepEqual(payload.decisionFacts,{baselineTick:'90',currentTick:'100',baselineSampleTick:'90',currentSampleTick:'100',hasNewEvidence:true,canWait:false,lastAcknowledgedAction:null,exactTickInterval:null});
    assert.deepEqual(payload.experimentContext, { ...context, completedActions: [], earlierSamplesOmitted: 0 });
    assert.equal(translated, 'Advance 10 ticks.');
    assert.deepEqual(result.actions, step(10).actions);
    assert.equal(result.experiment.phase, 'act');
});

test('the model can complete from acknowledged evidence without invoking the command parser', async () => {
    const model = { generate: async () => JSON.stringify(selection('complete', { reason: 'The requested ten ticks were acknowledged.' })),
        plan: async () => { throw new Error('Completion cannot translate a command'); } };
    const result = await proposeExperiment(model, 'Advance ten ticks and inspect the count.', observation(),
        { baseline:{tick:'90'},recent: [{ status: 'applied', completedTicks: 10 }] }, signal());
    assert.equal(result.kind, 'answer'); assert.equal(result.experiment.phase, 'complete'); assert.deepEqual(result.actions, []);
});

test('read-only observe phase preserves a pending diagnostic sample and uses a bounded wait', async () => {
    const current = observation(); current.facts.sampleTick = '90';
    const model = { generate: async messages => {
        const received = JSON.parse(messages.at(-1).content).observation;
        assert.equal(received.tick, '100'); assert.equal(received.facts.sampleTick, '90');
        const facts=JSON.parse(messages.at(-1).content).decisionFacts;
        assert.equal(facts.hasNewEvidence,false);assert.equal(facts.canWait,true);assert.equal(facts.currentSampleTick,'90');
        return JSON.stringify(selection('observe', { waitMs: 1000 }));
    }, plan: async () => { throw new Error('Waiting cannot write'); } };
    const result = await proposeExperiment(model, 'Observe the completed step.', current, {}, signal());
    assert.deepEqual(result.actions, []); assert.equal(result.experiment.waitMs, 1000);
});

test('malformed, mixed and out-of-budget phase output cannot become executable actions', async () => {
    const cases = ['not JSON', { ...selection('act'), extra: 'execute code' }, selection('act', { command: '' }),
        selection('complete', { command: 'Delete the selected object.' }), selection('observe', { waitMs: 0 }),
        selection('observe', { waitMs: 6000 }), selection('act', { waitMs: 250 })];
    for (const candidate of cases) {
        const model = { generate: async () => typeof candidate === 'string' ? candidate : JSON.stringify(candidate),
            plan: async () => { throw new Error('Invalid selection reached command translation'); } };
        await assert.rejects(proposeExperiment(model, 'Observe.', observation(), {}, signal()),
            error => !error.message.includes('reached command translation'));
    }
});

test('multiple commands, continuation, unknown actions and invalid quantities are rejected', async () => {
    const candidates = [{ ...step(1), actions: [...step(1).actions, ...step(2).actions] },
        { ...step(1), continuation: 'reset' }, step(5000), { ...step(1), actions: [{ type: 'eval', args: {} }] }];
    for (const plan of candidates) await assert.rejects(proposeExperiment({
        generate: async () => JSON.stringify(selection('act')), plan: async () => plan,
    }, 'Measure evolution.', observation(), {}, signal()));
});

test('grounding clarification ends the proposed experiment without an action', async () => {
    const result = await proposeExperiment({ generate: async () => JSON.stringify(selection('act')),
        plan: async () => ({ kind: 'clarify', message: 'Select a current object first.', actions: [] }),
    }, 'Inspect the object.', observation(), {}, signal());
    assert.equal(result.experiment.phase, 'clarify'); assert.deepEqual(result.actions, []);
});

test('cancellation after phase generation prevents command translation', async () => {
    const controller = new AbortController();
    await assert.rejects(proposeExperiment({ generate: async () => { controller.abort(); return JSON.stringify(selection('act')); },
        plan: async () => { throw new Error('Cancelled proposal translated'); },
    }, 'Advance.', observation(), {}, controller.signal), { name: 'AbortError' });
});

test('paused coherent worlds cannot wait and unchanged initial measurements cannot conclude',async()=>{
    const current=observation(),context={baseline:structuredClone(current),completedActions:[]};
    for(const phase of ['observe','complete'])await assert.rejects(proposeExperiment({generate:async()=>JSON.stringify(selection(phase)),
        plan:async()=>{throw new Error('No command should be translated');}},'Advance ten ticks then compare.',current,context,signal()),/Invalid experiment/);
});

test('the existing command translator grounds the model-selected next step without parsing the goal as a command', async () => {
    const model = new BrowserModel({ modelBase: './' }); let calls = 0;
    model.generate = async () => JSON.stringify(++calls === 1 ? selection('act') : step(10));
    const result = await proposeExperiment(model, 'Investigate whether the manifested count changes.', observation(), {}, signal());
    assert.equal(calls, 2); assert.deepEqual(result.actions, step(10).actions);
});

test('typical Observer evidence fits the small model while preserving baseline, clocks and light/source distinction', async () => {
    const entity = { id: 'cube-1', name: 'Clock cube', shape: 'cube', alive: true, mass: 2,
        position: [1, 2, 3], velocity: [0.1, 0, 0], bodyType: 'dynamic', gravity: true, collisions: true,
        size: [1, 1, 1], rotation: [0, 0, 0], color: [1, 0, 0], properAcceleration: [0, 0, 0],
        angularVelocity: [0, 0, 0], emission: 1, overlay: true, friction: 0.2, damping: 0.05, restitution: 0.5 };
    const current = { ...observation(), workspace: 'observer', tick: 100,
        actions: OBSERVER_ACTION_DESCRIPTORS, capabilities: OBSERVER_ACTION_DESCRIPTORS.map(action => action.type),
        selected: { id: entity.id, current: entity, author: entity,
            observed: { entityId: entity.id, historical: true, emissionTime: 0.8, observedPosition: [0.8, 2, 3], sourcePosition: [1, 2, 3], distance: 3 } },
        facts: { running: false, experiment: 'clock-baseline', time: 1, profile: 'sr', units: 'c = 1',
            observer: { position: [0, 0, 0], velocity: [0, 0, 0], properTime: 1 }, entities: [entity, { ...entity, id: 'cube-2' }],
            interpretation: 'Current source states and arriving-light observations are distinct.', rendering: { large: 'x'.repeat(20000) } } };
    const baseline = { ...current, tick: 90 }, context = { baseline,
        recent: [90, 95, 99, 100].map(tick => ({ sample: { ...current, tick }, action: { type: 'observer.step', args: { count: 1 } } })),
        actionsCompleted: 4, remainingActions: 46, remainingMs: 200000, round: 4 };
    current.facts.experimentRun = context;
    const compact = compactExperimentContext(context), view = compactExperimentObservation(current);
    assert.equal(compact.baseline.tick, 90); assert.equal(compact.recent.length, 2);
    assert.equal(compact.earlierSamplesOmitted, 2); assert.equal(compact.recent[1].sample.tick, 100);
    assert.equal(view.facts.experiment, 'clock-baseline'); assert.equal(view.facts.experimentRun, undefined);
    assert.equal(view.facts.observer.properTime, 1);
    assert.deepEqual(view.selected.current.position, [1, 2, 3]);
    assert.deepEqual(view.selected.observed.observedPosition, [0.8, 2, 3]);
    const model = { generate: async messages => {
    const payload = JSON.parse(messages.at(-1).content);
        assert.ok(JSON.stringify(payload).length < 8000);
        return JSON.stringify(selection('complete'));
    }, plan: async () => { throw new Error('Completion cannot translate commands'); } };
    await proposeExperiment(model, 'Compare clock positions after the acknowledged steps.', current, context, signal());
});

test('six 4000-character catalog results fit the planner and retain usable identities with explicit omission markers', async () => {
    const catalog = Array.from({ length: 8 }, (_, index) => ({ id: `scenario-${index}`, name: `Prepared scenario ${index}`,
        backend: 'finite-record', status: 'available', description: 'Preparation details '.repeat(10) }));
    const initialLength = JSON.stringify(catalog).length;
    catalog[7].description += 'x'.repeat(4000 - initialLength);
    assert.equal(JSON.stringify(catalog).length, 4000);
    const current = observation();
    current.facts.scientificBoundary = 'Finite records under a staged candidate law. Counts do not identify physical energy.';
    current.facts.scenarios = catalog.slice(0, 8).map(({ id, name, backend }) => ({ id, name, backend }));
    current.actions.push({ type: 'lattice.catalog', description: 'Find supported scenarios and their identifiers',
        args: { type: 'object', properties: { query: { type: 'string', maxLength: 120 }, limit: { type: 'integer', minimum: 1, maximum: 20 } }, required: [], additionalProperties: false } });
    current.capabilities.push('lattice.catalog');
    const context = { baseline: current, recent: Array.from({ length: 4 }, () => ({ sample: current })),
        completedActions: Array.from({ length: 6 }, (_, index) => ({ action: { type: 'lattice.catalog', args: { query: `batch ${index}` } }, result: catalog })),
        actionsCompleted: 6, remainingActions: 44, remainingMs: 150000, round: 7 };
    const compact = compactExperimentContext(context);
    assert.equal(compact.completedActions.length, 6);
    for (const row of compact.completedActions) {
        assert.equal(row.result.items[0].id, 'scenario-0');
        assert.equal(row.result.items[0].name, 'Prepared scenario 0');
        assert.equal(row.result.items[0].status, 'available');
        assert.equal(row.result.originalItemCount, 8);
        assert.equal(row.result.omitted, true); assert.equal(row.result.fullReceiptRequired, true);
        assert.ok(JSON.stringify(row.result).length <= 700);
    }
    const model = { generate: async messages => {
        assert.ok(messages.reduce((sum, message) => sum + message.content.length, 0) < 13500);
        return JSON.stringify(selection('complete'));
    }, plan: async () => { throw new Error('Completion cannot translate a command'); } };
    await proposeExperiment(model, 'Compare these catalog batches and report the available preparations.', current, context, signal());
    assert.equal(JSON.stringify(catalog).length, 4000, 'Original receipts remain intact');
});

test('large preview summaries retain exact preview identity and useful scalar outcome values', () => {
    const previewId = 'native-preview-6a0bb222', result = { previewId, status: 'ready', valid: true, siteCount: 729,
        summary: { lawId: 'phi-v2-staged-candidate-1', fieldTokens: '9007199254740997', detail: 'x'.repeat(3500) } };
    const compact = compactExperimentContext({ completedActions: [{ action: { type: 'lattice.seed.preview', args: {} }, result }] });
    const retained = compact.completedActions[0].result;
    assert.equal(retained.previewId, previewId); assert.equal(retained.status, 'ready');
    assert.equal(retained.valid, true); assert.equal(retained.siteCount, 729);
    assert.equal(retained.summary.fieldTokens, '9007199254740997');
    assert.equal(retained.fullReceiptRequired, true); assert.equal(retained.omitted, true);
    assert.ok(JSON.stringify(retained).length <= 700);
});
