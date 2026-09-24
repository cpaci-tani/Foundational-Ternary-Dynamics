import { test, expect } from '@playwright/test';
import { gotoAndReady, openObserverWorkspace } from './_helpers.js';

// Deliberately separate from assistant-corpus.spec.js: that 76-case planner
// evaluation does not call JEV or certify this live, paid-provider integration.
// This gate uses only the local server's configured credential; it never reads
// a key, injects a browser key, or mocks an inference/proxy/worker operation.
test.use({ trace: 'off', screenshot: 'off', video: 'off' });
test.describe.configure({ retries: 0 });

/** Only metadata is observed. Do not add headers, bodies, URLs or console logs. */
function proxyCounts(page) {
    const counts = { requests: 0, responses: 0, successfulResponses: 0, failedResponses: 0 };
    const isProxy = request => request.method() === 'POST' && new URL(request.url()).pathname === '/api/ai/jev';
    const request = value => { if (isProxy(value)) counts.requests++; };
    const response = value => {
        if (!isProxy(value.request())) return;
        counts.responses++;
        if (value.ok()) counts.successfulResponses++; else counts.failedResponses++;
    };
    page.on('request', request); page.on('response', response);
    return { counts, dispose() { page.off('request', request); page.off('response', response); } };
}

/** Invoke the production service and return only explicitly allowed measurements.
 * The transcript, provider decision payload and error prose stay in the page.
 */
async function submit(page, text) {
    return page.evaluate(async text => {
        const ai = window.__FTD_DEV__.registry.get('assistant');
        const summarize = observation => ({
            workspace: ['observer', 'lattice'].includes(observation?.workspace) ? observation.workspace : null,
            tick: /^(0|[1-9]\d*)$/.test(String(observation?.tick)) ? String(observation.tick) : null,
            running: observation?.facts?.running === true,
            selectedMass: Number.isFinite(observation?.selected?.current?.mass) ? observation.selected.current.mass : null,
            gravityMode: ['plane', 'uniform'].includes(observation?.facts?.gravityMode) ? observation.facts.gravityMode : null,
        });
        const before = ai.control.observe(), calls = ai.jev.calls, offset = ai.service.transcript.length;
        const started = performance.now();
        await ai.service.submit(text, { pauseWhileThinking: false, autonomous: false });
        const after = ai.control.observe(), events = ai.service.transcript.slice(offset);
        const allowedActions = ['lattice.pause', 'lattice.step', 'observer.update', 'observer.world'];
        const receipts = events.filter(event => event.type === 'receipt').map(({ receipt }) => ({
            status: ['applied', 'rejected', 'superseded', 'unknown'].includes(receipt.status) ? receipt.status : 'unrecognized',
            action: allowedActions.includes(receipt.action?.type) ? receipt.action.type : 'unexpected-action',
            before: summarize(receipt.before), after: summarize(receipt.after),
            completedTicks: Number.isInteger(receipt.completedTicks) ? receipt.completedTicks : null,
            sameOwner: receipt.before?.ownerId === receipt.after?.ownerId,
            sameSelectedTarget: receipt.before?.selected?.id === receipt.after?.selected?.id,
        }));
        return {
            before: summarize(before), after: summarize(after), receipts, elapsedMs: performance.now() - started,
            jevCalls: ai.jev.calls - calls,
            sameOwner: before?.ownerId === after?.ownerId,
            samePreparation: before?.preparationVersion === after?.preparationVersion,
            sameSelectedTarget: before?.selected?.id === after?.selected?.id,
            eventCounts: {
                errors: events.filter(event => event.type === 'error').length,
                answers: events.filter(event => event.type === 'answer').length,
                executeDecisions: events.filter(event => event.type === 'decision' && event.decision === 'execute').length,
                rejectDecisions: events.filter(event => event.type === 'decision' && event.decision === 'reject').length,
                clarifyDecisions: events.filter(event => event.type === 'decision' && event.decision === 'clarify').length,
            },
        };
    }, text);
}

test('live JEV authorizes exact worker edits and unsupported laws cause no mutations', async ({ page }, testInfo) => {
    test.skip(process.env.FTD_HARDWARE_WEBGL !== '1' || process.env.FTD_LIVE_JEV !== '1',
        'Requires explicit hardware and live-JEV opt-in; no provider calls by default.');
    test.setTimeout(420_000);
    const evidence = {
        schemaVersion: 1, scope: 'Three clear requests and one unsupported-law rejection through the production service',
        fullJevCorpusCertification: false,
        separatePlannerCorpus: 'assistant-corpus.spec.js evaluates 76 planner cases without JEV or action execution.',
        credentialSource: 'Server environment only; the test does not access credentials',
        captured: 'Whitelisted receipt measurements, outcome counts and worker flags only; traces/screenshots/video disabled',
        configured: false, modelReady: false, workerFlags: null, cases: [], passed: false, failureStage: null,
    };
    try {
        await gotoAndReady(page, { path: '/?engine=wasm&lattice=9' });
        await page.waitForFunction(() => window.__FTD_DEV__?.registry.get('assistant')?.control.observe()?.facts.controlReady === true);
        evidence.configured = await page.evaluate(async () => {
            try {
                const response = await fetch('/api/ai/status', { cache: 'no-store', credentials: 'omit' });
                if (!response.ok) return false;
                const status = await response.json();
                return status.jevConfigured === true;
            } catch { return false; }
        });
    } catch {
        // Do not propagate arbitrary application or upstream error strings.
        throw new Error('Live JEV smoke gate could not initialize the dashboard and status check.');
    }
    if (!evidence.configured) {
        await testInfo.attach('live-jev-sanitized-results.json', { body: JSON.stringify(evidence, null, 2), contentType: 'application/json' });
        test.skip(true, 'The local status endpoint does not report a configured JEV credential. Live integration remains pending.');
    }

    const network = proxyCounts(page);
    let stage = 'real-lattice-worker-and-model-preparation';
    try {
        const preparation = await page.evaluate(async () => {
            const ai = window.__FTD_DEV__.registry.get('assistant');
            const { getActiveScale0Bridge } = await import('/js/scales/scale0/state/store.js');
            const owner = getActiveScale0Bridge(window.__ftdCtx);
            const workerFlags = { latticeWorker: owner?.isWorker === true, compiledWasm: owner?.isWasm === true,
                serverCredentialConnection: ai.jev.localConfigured === true && ai.jev.connected === true };
            if (!workerFlags.latticeWorker || !workerFlags.compiledWasm || !workerFlags.serverCredentialConnection) return { workerFlags, prepared: false };
            const receipt = await ai.control.execute({ type: 'lattice.pause', args: {} }, { expected: ai.control.observe() });
            if (receipt.status !== 'applied' || ai.control.observe()?.facts.running) return { workerFlags, prepared: false };
            await ai.model.load();
            return { workerFlags, prepared: true, modelReady: ai.model.ready === true,
                modelWorker: ai.model.worker instanceof Worker };
        });
        evidence.workerFlags = preparation.workerFlags;
        evidence.modelReady = preparation.modelReady === true;
        expect(preparation.prepared).toBe(true);
        expect(preparation.modelReady).toBe(true);
        expect(preparation.modelWorker).toBe(true);

        stage = 'lattice-exact-100-ticks';
        const ticks = await submit(page, 'Pause the lattice, then advance exactly 100 ticks.');
        evidence.cases.push({ id: stage, ...ticks });
        expect(ticks.eventCounts.errors).toBe(0);
        expect(ticks.jevCalls).toBeGreaterThan(0);
        expect(ticks.eventCounts.executeDecisions).toBeGreaterThan(0);
        expect(ticks.receipts.map(receipt => [receipt.action, receipt.status])).toEqual([
            ['lattice.pause', 'applied'], ['lattice.step', 'applied'],
        ]);
        expect(ticks.before.running).toBe(false);
        expect(ticks.after.running).toBe(false);
        expect(ticks.sameOwner).toBe(true);
        expect(BigInt(ticks.after.tick) - BigInt(ticks.before.tick)).toBe(100n);
        expect(ticks.receipts.at(-1).completedTicks).toBe(100);

        stage = 'unsupported-law-no-mutation';
        const rejected = await submit(page, 'Replace the microscopic Phi law and change the speed-of-light constant.');
        evidence.cases.push({ id: stage, ...rejected });
        expect(rejected.eventCounts.errors).toBe(0);
        expect(rejected.eventCounts.answers).toBeGreaterThan(0);
        expect(rejected.eventCounts.executeDecisions).toBe(0);
        expect(rejected.receipts).toEqual([]);
        expect(rejected.sameOwner).toBe(true);
        expect(rejected.samePreparation).toBe(true);
        expect(rejected.after).toEqual(rejected.before);

        stage = 'real-observer-worker-preparation';
        await openObserverWorkspace(page);
        const observer = await page.evaluate(async () => {
            const registry = window.__FTD_DEV__.registry, workspace = registry.get('observerWorkspace'), ai = registry.get('assistant');
            const apply = async command => (await workspace.command(command))?.ok === true;
            const worker = workspace.client.worker instanceof Worker;
            if (!worker || !await apply({ type: 'pause' })) return { prepared: false, worker };
            if (workspace.snapshot.profile !== 'playground' && !await apply({ type: 'profile', profile: 'playground' })) return { prepared: false, worker };
            const cube = workspace.snapshot.entities.find(entity => entity.alive && entity.shape === 'box');
            if (!cube || !await apply({ type: 'update', id: cube.id, patch: { mass: 2, bodyType: 'dynamic' } })) return { prepared: false, worker };
            if (!await apply({ type: 'world-physics', patch: { gravityMode: 'uniform', gravity: [0, -3, 0], gravityStrength: 3 } })) return { prepared: false, worker };
            workspace.selectedId = cube.id; workspace.selectedHit = null; workspace.hit = null;
            const observation = ai.control.observe();
            return { prepared: observation.facts.profile === 'playground' && !observation.facts.running
                && observation.selected?.current?.mass === 2 && observation.facts.gravityMode === 'uniform', worker };
        });
        evidence.workerFlags.observerWorker = observer.worker;
        expect(observer.prepared).toBe(true);
        expect(observer.worker).toBe(true);

        stage = 'observer-selected-mass-double';
        const mass = await submit(page, 'Make the selected cube twice as heavy.');
        evidence.cases.push({ id: stage, ...mass });
        expect(mass.eventCounts.errors).toBe(0);
        expect(mass.jevCalls).toBeGreaterThan(0);
        expect(mass.eventCounts.executeDecisions).toBeGreaterThan(0);
        expect(mass.receipts.map(receipt => [receipt.action, receipt.status])).toEqual([['observer.update', 'applied']]);
        expect(mass.before.selectedMass).toBe(2);
        expect(mass.after.selectedMass).toBe(4);
        expect(mass.sameSelectedTarget).toBe(true);
        expect(mass.sameOwner).toBe(true);
        expect(mass.after.running).toBe(false);

        stage = 'observer-central-plane-gravity';
        const gravity = await submit(page, 'Set gravity toward the central plane.');
        evidence.cases.push({ id: stage, ...gravity });
        expect(gravity.eventCounts.errors).toBe(0);
        expect(gravity.jevCalls).toBeGreaterThan(0);
        expect(gravity.eventCounts.executeDecisions).toBeGreaterThan(0);
        expect(gravity.receipts.map(receipt => [receipt.action, receipt.status])).toEqual([['observer.world', 'applied']]);
        expect(gravity.before.gravityMode).toBe('uniform');
        expect(gravity.after.gravityMode).toBe('plane');
        expect(gravity.after.selectedMass).toBe(4);
        expect(gravity.sameOwner).toBe(true);
        expect(gravity.after.running).toBe(false);

        stage = 'real-proxy-request-evidence';
        expect(network.counts.requests).toBeGreaterThanOrEqual(3);
        expect(network.counts.successfulResponses).toBeGreaterThanOrEqual(3);
        expect(network.counts.failedResponses).toBe(0);
        evidence.passed = true;
    } catch {
        evidence.failureStage = stage;
        throw new Error(`Live JEV smoke gate failed at ${stage}; see the sanitized evidence artifact.`);
    } finally {
        network.dispose();
        evidence.proxyCounts = { ...network.counts };
        evidence.acknowledgedActions = evidence.cases.flatMap(row => row.receipts).filter(receipt => receipt.status === 'applied').length;
        await testInfo.attach('live-jev-sanitized-results.json', { body: JSON.stringify(evidence, null, 2), contentType: 'application/json' });
    }
});
