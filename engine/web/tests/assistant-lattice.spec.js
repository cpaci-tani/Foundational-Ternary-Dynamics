import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

async function ready(page) {
    await gotoAndReady(page, { path: '/?engine=wasm&lattice=9' });
    await page.waitForFunction(() => {
        const control = window.__FTD_DEV__?.registry.get('assistant')?.control;
        return control?.observe()?.capabilities.includes('lattice.step');
    });
    await page.evaluate(async () => {
        const ai = window.__FTD_DEV__.registry.get('assistant');
        const { getActiveScale0Bridge } = await import('/js/scales/scale0/state/store.js');
        const owner = getActiveScale0Bridge(window.__ftdCtx);
        if (!owner?.isWorker || !owner?.isWasm) throw new Error('This gate requires the real compiled WASM worker');
        window.__assistantLatticeOwner = owner;
        ai.knowledge.search = async () => [];
        ai.jev.evaluate = async () => ({ decision: 'execute', confidence: 1, model: 'test-fixture' });
    });
}

// Stub only language interpretation and remote approval. The production service,
// adapter, controller, worker and compiled lattice perform every state change.
test('lattice commands acknowledge exactly 100 worker ticks and reset can resume', async ({ page }) => {
    await ready(page);
    const stepped = await page.evaluate(async () => {
        const ai = window.__FTD_DEV__.registry.get('assistant');
        const initial = ai.control.observe();
        ai.model.plan = async () => ({ kind: 'actions', message: 'Pause and advance 100 ticks', actions: [
            { type: 'lattice.pause', args: {} }, { type: 'lattice.step', args: { count: 100 } },
        ] });
        await ai.service.submit('Pause the lattice and advance exactly 100 ticks');
        const receipts = ai.service.transcript.filter(row => row.type === 'receipt').map(row => row.receipt);
        const { getActiveScale0Bridge } = await import('/js/scales/scale0/state/store.js');
        return { initial, after: ai.control.observe(), receipts,
            errors: ai.service.transcript.filter(row => row.type === 'error'),
            sameOwner: getActiveScale0Bridge(window.__ftdCtx) === window.__assistantLatticeOwner,
            actualTick: getActiveScale0Bridge(window.__ftdCtx).currentTick() };
    });
    expect(stepped.errors).toEqual([]);
    expect(stepped.receipts.map(row => [row.action.type, row.status])).toEqual([
        ['lattice.pause', 'applied'], ['lattice.step', 'applied'],
    ]);
    const [paused, advanced] = stepped.receipts;
    expect(advanced.completedTicks).toBe(100);
    expect(BigInt(advanced.after.tick) - BigInt(paused.after.tick)).toBe(100n);
    expect(BigInt(stepped.actualTick)).toBe(BigInt(advanced.after.tick));
    expect(advanced.after.preparationVersion).toBe(paused.after.preparationVersion);
    expect(stepped.after.ownerId).toBe(stepped.initial.ownerId);
    expect(stepped.sameOwner).toBe(true);
    expect(stepped.after.facts.running).toBe(false);

    const reset = await page.evaluate(async () => {
        const ai = window.__FTD_DEV__.registry.get('assistant');
        ai.service.clear();
        const before = ai.control.observe();
        ai.model.plan = async () => ({ kind: 'actions', message: 'Reset and resume', actions: [
            { type: 'lattice.reset', args: {} }, { type: 'lattice.resume', args: {} },
        ] });
        await ai.service.submit('Reset the current lattice scenario, then resume playback');
        return { before, after: ai.control.observe(),
            receipts: ai.service.transcript.filter(row => row.type === 'receipt').map(row => row.receipt),
            errors: ai.service.transcript.filter(row => row.type === 'error') };
    });
    expect(reset.errors).toEqual([]);
    expect(reset.receipts.map(row => [row.action.type, row.status])).toEqual([
        ['lattice.reset', 'applied'], ['lattice.resume', 'applied'],
    ]);
    expect(reset.after.facts.scenarioId).toBe(reset.before.facts.scenarioId);
    expect(reset.after.preparationVersion).not.toBe(reset.before.preparationVersion);
    expect(reset.after.facts.running).toBe(true);
    await page.waitForFunction(tick => {
        const now = window.__FTD_DEV__.registry.get('assistant').control.observe();
        return now.tick !== null && BigInt(now.tick) > BigInt(tick);
    }, reset.after.tick);
});

test('ordinary worker ticks during inference preserve the preparation fence', async ({ page }) => {
    await ready(page);
    const result = await page.evaluate(async () => {
        const ai = window.__FTD_DEV__.registry.get('assistant');
        await ai.control.execute({ type: 'lattice.resume', args: {} }, {
            expected: ai.control.observe(), signal: new AbortController().signal, assertActive() {},
        });
        const before = ai.control.observe();
        let interpreted = null;
        ai.model.plan = async (_text, observation) => {
            const deadline = performance.now() + 10000;
            do {
                await new Promise(resolve => setTimeout(resolve, 16));
                interpreted = ai.control.observe();
                if (interpreted.tick !== null && BigInt(interpreted.tick) > BigInt(observation.tick)) break;
                if (performance.now() > deadline) throw new Error('The actual lattice worker did not advance during interpretation');
            } while (true);
            return { kind: 'actions', message: 'Pause the same preparation', actions: [{ type: 'lattice.pause', args: {} }] };
        };
        await ai.service.submit('Pause the currently running lattice');
        return { before, interpreted, after: ai.control.observe(),
            receipts: ai.service.transcript.filter(row => row.type === 'receipt').map(row => row.receipt),
            errors: ai.service.transcript.filter(row => row.type === 'error') };
    });
    expect(result.errors).toEqual([]);
    expect(BigInt(result.interpreted.tick)).toBeGreaterThan(BigInt(result.before.tick));
    expect(result.interpreted.preparationVersion).toBe(result.before.preparationVersion);
    expect(result.receipts.at(-1).status).toBe('applied');
    expect(result.after.ownerId).toBe(result.before.ownerId);
    expect(result.after.facts.running).toBe(false);
});
