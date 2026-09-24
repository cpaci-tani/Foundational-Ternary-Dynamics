import { test, expect } from '@playwright/test';
import { gotoAndReady, openObserverWorkspace } from './_helpers.js';

async function retainActiveWorker(page) {
    await page.evaluate(async () => {
        const { getActiveScale0Bridge } = await import('/js/scales/scale0/state/store.js');
        window.__observerGetActiveOwner = () => getActiveScale0Bridge(window.__ftdCtx);
    });
    await page.waitForFunction(() => {
        const owner = window.__observerGetActiveOwner();
        const lifecycle = owner?.lifecycleDebug;
        const meta = owner?.getScale0TelemetryGroupMeta?.('diagnostics');
        return owner?.isWorker === true && owner.isFiniteRecord !== true && owner.ready
            && lifecycle?.workerRuntimeId && lifecycle.configurationToken === lifecycle.appliedConfigurationToken
            && meta?.status === 'available' && meta.stale === false && Number.isSafeInteger(meta.sampleTick);
    });
    await page.evaluate(() => {
        window.__observerOriginalOwner = window.__observerGetActiveOwner();
        window.__observerOriginalWorker = window.__observerOriginalOwner._worker;
    });
}

async function waitForRetainedPause(page) {
    await page.waitForFunction(() => {
        const owner = window.__observerOriginalOwner;
        return window.__observerGetActiveOwner() === owner && window.__ftdCtx.running === false
            && owner.runningStateSettled && owner._runningAck === false;
    });
}

// Fresh one-shot engine hashing is read-only. The cached build digest alone
// cannot establish that later controls left the complete microscopic state intact.
async function captureRetainedState(page) {
    const state = await page.evaluate(async () => {
        const owner = window.__observerOriginalOwner;
        const digest = await owner.captureDynamicalStateDigest();
        const meta = owner.getScale0TelemetryGroupMeta('diagnostics');
        return { sameOwner: window.__observerGetActiveOwner() === owner,
            sameWorker: owner._worker === window.__observerOriginalWorker,
            lifecycle: owner.lifecycleDebug, digest, dataVersion: owner.dataVersion,
            tick: owner.currentTick(), sourceEpoch: meta?.sourceEpoch, sampleTick: meta?.sampleTick,
            running: window.__ftdCtx.running, settled: owner.runningStateSettled,
            workerRunning: owner._runningAck };
    });
    expect(state.sameOwner).toBe(true);
    expect(state.sameWorker).toBe(true);
    expect(state.lifecycle.workerRuntimeId).toEqual(expect.any(String));
    expect(state.digest, 'fresh engine digest is required; absence must fail the gate').not.toBeNull();
    expect(state.digest.hashLo).toMatch(/^[0-9a-f]{16}$/);
    expect(state.digest.hashHi).toMatch(/^[0-9a-f]{16}$/);
    expect(Number.isSafeInteger(state.tick)).toBe(true);
    expect(Number.isSafeInteger(state.dataVersion)).toBe(true);
    expect(state.digest.tick).toBe(state.tick);
    expect(state.sampleTick).toBe(state.tick);
    expect(state.sourceEpoch).toBe(state.lifecycle.appliedConfigurationToken);
    expect(state.running).toBe(false);
    expect(state.workerRunning).toBe(false);
    expect(state.settled).toBe(true);
    return state;
}

async function startWorkerTrace(page) {
    await page.evaluate(() => {
        const worker = window.__observerOriginalWorker;
        const original = worker.postMessage;
        const calls = [];
        worker.postMessage = function (message, ...rest) {
            calls.push({ type: message.type, method: message.method ?? null,
                value: message.value ?? null, configurationToken: message.configurationToken ?? null });
            return original.call(this, message, ...rest);
        };
        window.__observerWorkerTrace = calls;
        window.__observerRestoreWorkerTrace = () => { worker.postMessage = original; };
    });
}

async function finishWorkerTrace(page) {
    return page.evaluate(() => {
        window.__observerRestoreWorkerTrace();
        return window.__observerWorkerTrace;
    });
}

function expectNoWorkerEdits(trace) {
    expect(trace.filter(message => ['command', 'batchCommand', 'create', 'dispose'].includes(message.type))).toEqual([]);
}

async function openObserver(page) {
    await gotoAndReady(page, { path: '/?engine=wasm&lattice=9' });
    await retainActiveWorker(page);
    await openObserverWorkspace(page);
    await expect(page.locator('#observer-workspace')).toBeVisible();
    await page.waitForFunction(() => !!window.__FTD_DEV__.registry.get('observerWorkspace')?.snapshot);
    await waitForRetainedPause(page);
}

test('Observer is an independent workspace and retains the actual lattice worker across visits', async ({ page }) => {
    const errors = [];
    page.on('pageerror', error => errors.push(error.message));
    await gotoAndReady(page, { path: '/?engine=wasm&lattice=9' });
    await retainActiveWorker(page);
    await page.evaluate(() => {
        window.__observerOriginalCamera = window.__FTD_DEV__.viewport.camera.position.toArray();
    });
    for (let visit = 0; visit < 3; visit++) {
        await openObserverWorkspace(page);
        await expect(page.locator('#observer-workspace')).toBeVisible();
        expect(await page.evaluate(() => document.querySelector('#engine-mode').value)).toBe('lattice');
        expect(await page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.profile)).toBe('sr');
        await page.evaluate(() => window.__FTD_DEV__.registry.get('observerHost').exit());
        await expect(page.locator('#observer-workspace')).toBeHidden();
        expect(await page.evaluate(() => {
            const owner = window.__observerGetActiveOwner();
            return owner === window.__observerOriginalOwner && owner._worker === window.__observerOriginalWorker;
        })).toBe(true);
        expect(await page.evaluate(() => window.__FTD_DEV__.viewport.camera.position.toArray())).toEqual(
            await page.evaluate(() => window.__observerOriginalCamera));
    }
    expect(errors).toEqual([]);
});

test('reticle, inspector, author edits and view controls preserve complete retained lattice state', async ({ page }) => {
    await openObserver(page);
    await page.evaluate(async () => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        await w.command({ type: 'pause' });
    });
    const before = await captureRetainedState(page);
    await startWorkerTrace(page);
    await page.locator('.observer-canvas').focus();
    await page.keyboard.press('KeyH');
    await expect(page.locator('.observer-reticle')).toBeHidden();
    await page.keyboard.press('KeyH');
    await expect(page.locator('.observer-reticle')).toBeVisible();
    await page.keyboard.press('Tab');
    await page.waitForFunction(() => window.__FTD_DEV__.registry.get('observerWorkspace').panelOpen);
    expect(await page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.playing)).toBe(false);
    await page.keyboard.press('Escape');
    await page.waitForFunction(() => !window.__FTD_DEV__.registry.get('observerWorkspace').panelOpen);
    const zoomStart = await page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.observer.position[2]);
    await page.locator('.observer-canvas').hover({ position: { x: 600, y: 320 } });
    await page.mouse.wheel(0, -800);
    await expect.poll(() => page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').snapshot.observer.position[2])).toBeCloseTo(zoomStart - 8, 8);
    const lens = await page.evaluate(async () => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        w.setSetting('fov', 85); w.setSetting('reticle', false);
        const entity = w.snapshot.entities.find(item => item.alive);
        const ack = await w.command({ type: 'update', id: entity.id, patch: { color: [1, 0, 0], mass: 2 } });
        if (!ack.ok) throw new Error('Author edit was not applied');
        return w.lattice.snapshot();
    });
    expect(lens.available).toBe(true);
    expect(lens.tick).toBe(String(before.tick));
    expect(lens.sourceEpoch).toBe(String(before.sourceEpoch));
    expect(await captureRetainedState(page)).toEqual(before);
    const trace = await finishWorkerTrace(page);
    expectNoWorkerEdits(trace);
    expect(trace.filter(message => message.type === 'setRunning')).toEqual([]);
});

test('legacy live-link settings cannot resume a suspended lattice owner', async ({ page }) => {
    await openObserver(page);
    await page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').command({ type: 'pause' }));
    const before = await captureRetainedState(page);
    await startWorkerTrace(page);
    await page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').setSetting('liveLink', true));
    await waitForRetainedPause(page);
    await page.evaluate(async () => {
        // Continue rendering the independent sandbox without driving native ticks.
        for (let frame = 0; frame < 8; frame++) await new Promise(resolve => requestAnimationFrame(resolve));
    });
    expect(await captureRetainedState(page)).toEqual(before);
    const trace = await finishWorkerTrace(page);
    expectNoWorkerEdits(trace);
    expect(trace.filter(message => message.type === 'setRunning')).toEqual([]);
    expect(await page.evaluate(() => window.__FTD_DEV__.registry.get('observerHost').liveLink)).toBe(false);
});

test('author transactions preserve old emitted light and preview is nonmutating', async ({ page }) => {
    await openObserver(page);
    const result = await page.evaluate(async () => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        await w.command({ type: 'pause' });
        const object = w.snapshot.entities.find(entity => entity.alive);
        const epoch = w.snapshot.epoch;
        const before = w.snapshot.segments.length;
        await w.action('preview', { id: object.id, patch: { color: [1, 0, 0] } });
        const previewDidNotWrite = w.snapshot.segments.length === before;
        const ack = await w.command({ type: 'update', id: object.id, patch: { color: [1, 0, 0] } });
        const segments = w.snapshot.segments.filter(segment => segment.entityId === object.id);
        return { previewDidNotWrite, ok: ack.ok, epochPreserved: epoch === w.snapshot.epoch,
            revisions: [...new Set(segments.map(segment => segment.revision))].length,
            oldClosed: segments.some(segment => segment.end !== null && Number.isFinite(segment.end)) };
    });
    expect(result).toEqual({ previewDidNotWrite: true, ok: true, epochPreserved: true, revisions: 2, oldClosed: true });
});

test('all environment presets load and a Playground round trip stays isolated', async ({ page }) => {
    test.setTimeout(120_000);
    const errors = [];
    page.on('pageerror', error => errors.push(error.message));
    await openObserver(page);
    await page.evaluate(() => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        w.setSetting('renderScale', 0.25); w.setSetting('autoQuality', false);
        return w.command({ type: 'pause' });
    });
    const ids = await page.evaluate(async () => (await import('/js/observer/catalog.js')).ENVIRONMENT_PRESETS.map(item => item.id));
    const rendererIds = await page.evaluate(async () => (await import('/js/observer/environments.js')).OBSERVER_ENVIRONMENTS.map(item => item.id));
    expect(new Set(ids).size).toBe(ids.length);
    expect([...ids].sort()).toEqual([...rendererIds].sort());
    for (const preset of ids) {
        const ok = await page.evaluate(async preset => {
            const w = window.__FTD_DEV__.registry.get('observerWorkspace');
            const ack = await w.command({ type: 'environment', patch: { preset } });
            await new Promise(resolve => requestAnimationFrame(() => requestAnimationFrame(resolve)));
            return ack.ok;
        }, preset);
        expect(ok, preset).toBe(true);
    }
    const state = await page.evaluate(async () => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        const first = w.snapshot.epoch;
        const playground = await w.command({ type: 'profile', profile: 'playground' });
        const engine = w.snapshot.physicsEngine;
        const sr = await w.command({ type: 'profile', profile: 'sr' });
        return { ok: playground.ok && sr.ok, engine, epoch: w.snapshot.epoch, first,
            mode: document.querySelector('#engine-mode').value };
    });
    expect(state.ok).toBe(true);
    expect(state.engine).toMatch(/rapier/i);
    expect(state.epoch).toBeGreaterThan(state.first);
    expect(state.mode).toBe('lattice');
    expect(errors).toEqual([]);
});

test('default world persistence is off and explicit export/import restores a new epoch', async ({ page }) => {
    await openObserver(page);
    const result = await page.evaluate(async () => {
        const w = window.__FTD_DEV__.registry.get('observerWorkspace');
        await w.command({ type: 'pause' });
        const before = w.snapshot.epoch;
        const text = await w.storage.exportWorld(w.snapshot, w.settings);
        const document = await w.storage.importWorld(text);
        await w.restore(document);
        return { autosave: w.storageState.autosave, restored: w.snapshot.epoch > before,
            entityCount: w.snapshot.entities.length, settings: w.settings.fov };
    });
    expect(result.autosave).toBe(false);
    expect(result.restored).toBe(true);
    expect(result.entityCount).toBeGreaterThan(0);
    expect(result.settings).toBe(60);
});
