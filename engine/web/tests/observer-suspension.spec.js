/* global window, document, requestAnimationFrame, Event */
import { test, expect } from '@playwright/test';
import { gotoAndReady, openObserverWorkspace } from './_helpers.js';

async function prepare(page, running) {
    await gotoAndReady(page, { path: '/?engine=wasm&lattice=9' });
    await page.waitForFunction(() => {
        const owner = window.__ftdCtx && window.__FTD_DEV__.registry.get('observerHost').deps.readLattice().owner;
        return owner?.isWorker && owner.ready && owner.runningStateSettled;
    });
    await page.evaluate(async running => {
        const host = window.__FTD_DEV__.registry.get('observerHost');
        const owner = host.deps.readLattice().owner;
        window.__suspensionOwner = owner;
        window.__suspensionWorker = owner._worker;
        window.__suspensionSource = owner.lifecycleDebug;
        const { rafCoordinator } = await import('/js/lib/raf-coordinator.js');
        window.__suspensionPanelCalls = 0;
        window.__suspensionProbe = rafCoordinator.subscribe('observer-suspension-probe', {
            hz: 60, cb: () => window.__suspensionPanelCalls++,
        });
        host.deps.setRunning(running);
    }, running);
    await page.waitForFunction(running => window.__suspensionOwner.runningStateSettled
        && window.__suspensionOwner._runningAck === running, running);
}

test('Observer parks worker ticks, sampler messages and panel callbacks, then resumes the retained running owner', async ({ page }) => {
    await prepare(page, true);
    await page.waitForFunction(() => window.__suspensionOwner.currentTick() > 3);
    await openObserverWorkspace(page);
    await page.waitForFunction(() => window.__FTD_DEV__.registry.get('observerHost').active);
    const before = await page.evaluate(() => {
        const owner = window.__suspensionOwner;
        window.__suspensionMessages = [];
        const original = owner._worker.postMessage;
        owner._worker.postMessage = function (message, ...rest) {
            window.__suspensionMessages.push(message.type);
            return original.call(this, message, ...rest);
        };
        window.__restoreWorkerSend = () => { owner._worker.postMessage = original; };
        return { tick: owner.currentTick(), frames: owner.frameCounter, version: owner.dataVersion,
            calls: window.__suspensionPanelCalls, source: owner.lifecycleDebug };
    });
    await page.evaluate(async () => {
        for (let i = 0; i < 45; i++) await new Promise(resolve => requestAnimationFrame(resolve));
    });
    expect(await page.evaluate(() => {
        const owner = window.__suspensionOwner;
        return { tick: owner.currentTick(), frames: owner.frameCounter, version: owner.dataVersion,
            calls: window.__suspensionPanelCalls, source: owner.lifecycleDebug };
    })).toEqual(before);
    expect(await page.evaluate(() => window.__suspensionMessages)).toEqual([]);
    expect(await page.evaluate(() => window.__ftdRAF.suspended && window.__ftdRAF._rafId === null)).toBe(true);
    expect(await page.evaluate(() => {
        const flux = window.__ftdCtx.viewport._fluxRenderer;
        return flux._presentationSuspended && flux._fluxAsyncRaf === 0;
    })).toBe(true);
    await page.getByRole('button', { name: '← Lattice Sim' }).click();
    await page.waitForFunction(tick => window.__suspensionOwner.runningStateSettled
        && window.__suspensionOwner._runningAck && window.__suspensionOwner.currentTick() > tick, before.tick);
    expect(await page.evaluate(() => {
        const owner = window.__FTD_DEV__.registry.get('observerHost').deps.readLattice().owner;
        return owner === window.__suspensionOwner && owner._worker === window.__suspensionWorker
            && !window.__ftdRAF.suspended && window.__suspensionPanelCalls > 0;
    })).toBe(true);
    await page.evaluate(() => { window.__restoreWorkerSend(); window.__suspensionProbe.unsubscribe(); });
});

test('repeated visits retain a paused owner and return focus to the visible simulation button', async ({ page }) => {
    await prepare(page, false);
    const tick = await page.evaluate(() => window.__suspensionOwner.currentTick());
    for (let visit = 0; visit < 3; visit++) {
        await openObserverWorkspace(page);
        await page.waitForFunction(() => window.__FTD_DEV__.registry.get('observerHost').active);
        await page.getByRole('button', { name: '← Lattice Sim' }).click();
        await page.waitForFunction(() => !window.__FTD_DEV__.registry.get('observerHost').suspended);
        await expect(page.locator('#simulation-menu-trigger')).toBeFocused();
        expect(await page.evaluate(() => ({ tick: window.__suspensionOwner.currentTick(),
            running: window.__ftdCtx.running, workerRunning: window.__suspensionOwner._runningAck })))
            .toEqual({ tick, running: false, workerRunning: false });
    }
    await page.evaluate(() => window.__suspensionProbe.unsubscribe());
});

test('a failed suspension restores playback, input and the previous workspace', async ({ page }) => {
    await prepare(page, true);
    const error = await page.evaluate(async () => {
        const host = window.__FTD_DEV__.registry.get('observerHost');
        const suspend = host.deps.suspendDashboard;
        host.deps.suspendDashboard = async () => { throw new Error('Test suspension failure'); };
        try { await host.enter(); return null; }
        catch (error) { return error.message; }
        finally { host.deps.suspendDashboard = suspend; }
    });
    expect(error).toBe('Test suspension failure');
    await page.waitForFunction(() => window.__suspensionOwner.runningStateSettled && window.__suspensionOwner._runningAck);
    expect(await page.evaluate(() => ({ suspended: window.__FTD_DEV__.registry.get('observerHost').suspended,
        inert: document.querySelector('#app').inert, controls: window.__ftdCtx.viewport.controls.enabled,
        running: window.__ftdCtx.running }))).toEqual({ suspended: false, inert: false, controls: true, running: true });
    await page.evaluate(() => window.__suspensionProbe.unsubscribe());
});

test('a scale change waits for pending Observer suspension before destroying an owner', async ({ page }) => {
    await prepare(page, false);
    await page.evaluate(() => {
        const host = window.__FTD_DEV__.registry.get('observerHost');
        const suspend = host.deps.suspendDashboard;
        host.deps.suspendDashboard = async () => {
            const resume = await suspend();
            await new Promise(resolve => { window.__releaseSuspension = resolve; });
            host.deps.suspendDashboard = suspend;
            return resume;
        };
        window.__pendingObserverEntry = host.enter();
    });
    await page.waitForFunction(() => !!window.__releaseSuspension);
    expect(await page.evaluate(() => {
        const select = document.querySelector('#engine-mode');
        select.value = 'particles'; select.dispatchEvent(new Event('change', { bubbles: true }));
        return { mode: window.__ftdCtx.engineMode, disposed: window.__suspensionOwner.disposed,
            inert: document.querySelector('#app').inert };
    })).toEqual({ mode: 'lattice', disposed: false, inert: true });
    await page.evaluate(async () => { window.__releaseSuspension(); await window.__pendingObserverEntry; });
    await page.waitForFunction(() => window.__ftdCtx.engineMode === 'particles');
    expect(await page.evaluate(() => ({ suspended: window.__FTD_DEV__.registry.get('observerHost').suspended,
        active: window.__FTD_DEV__.registry.get('observerHost').active, panelsSuspended: window.__ftdRAF.suspended })))
        .toEqual({ suspended: false, active: false, panelsSuspended: false });
    await page.evaluate(() => window.__suspensionProbe.unsubscribe());
});

test('cancelled initialization discards its disposed workspace and allows a fresh entry', async ({ page }) => {
    await prepare(page, false);
    await page.evaluate(async () => {
        const { ObserverWorkspace } = await import('/js/observer/workspace.js');
        const original = ObserverWorkspace.prototype.initialize;
        ObserverWorkspace.prototype.initialize = async function () {
            await original.call(this);
            await new Promise(resolve => { window.__releaseObserverInitialize = resolve; });
        };
        window.__restoreObserverInitialize = () => { ObserverWorkspace.prototype.initialize = original; };
        const host = window.__FTD_DEV__.registry.get('observerHost');
        window.__pendingObserverEntry = host.enter();
    });
    await page.waitForFunction(() => !!window.__releaseObserverInitialize);
    await page.evaluate(async () => {
        const host = window.__FTD_DEV__.registry.get('observerHost');
        const exit = host.exit();
        window.__releaseObserverInitialize();
        await Promise.all([window.__pendingObserverEntry, exit]);
        window.__restoreObserverInitialize();
    });
    expect(await page.evaluate(() => ({ workspace: window.__FTD_DEV__.registry.get('observerHost').workspace,
        registered: !!window.__FTD_DEV__.registry.get('observerWorkspace'), panelsSuspended: window.__ftdRAF.suspended })))
        .toEqual({ workspace: null, registered: false, panelsSuspended: false });
    await openObserverWorkspace(page);
    await page.waitForFunction(() => window.__FTD_DEV__.registry.get('observerHost').active);
    expect(await page.evaluate(() => window.__FTD_DEV__.registry.get('observerWorkspace').disposed)).toBe(false);
    await page.getByRole('button', { name: '← Lattice Sim' }).click();
    await page.evaluate(() => window.__suspensionProbe.unsubscribe());
});
