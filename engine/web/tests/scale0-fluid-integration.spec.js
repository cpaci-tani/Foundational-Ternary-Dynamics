import { test, expect } from '@playwright/test';

test('ordinary lattice scenarios share their owner with volume overlays and passive field observations', async ({ page }) => {
    page.setDefaultTimeout(15000);
    const errors = [], workerUrls = [];
    page.on('pageerror', error => { errors.push(error.message); console.log('PAGE ERROR', error.message); });
    page.on('worker', worker => workerUrls.push(worker.url()));
    await page.setViewportSize({ width: 1440, height: 1000 });
    await page.goto('/?engine=wasm', { waitUntil: 'domcontentloaded' });
    await page.waitForFunction(() => window.__ftdCtx?.bridge && window.__ftdFluidPanel);
    await page.locator('#scenario-select').selectOption('flux-thermalization');
    await page.waitForFunction(() => {
        const v = window.__ftdCtx.viewport._scalarVolumes?.get('emEnergy'); return v?.group.visible && v.uploadCount > 0;
    }).catch(async error => {
        console.log('VOLUME', await page.evaluate(() => {
            const c = window.__ftdCtx, o = c.useFluxMock ? c.fluxMock : c.bridge;
            return { volume: c.viewport._scalarVolumes?.get('emEnergy')?.group.userData,
                owner: o.constructor.name, samples: Object.fromEntries(Object.entries(o._samplerCache || {}).map(([key, v]) => [key,
                    { count: v.count, sampleTick: v.sampleTick, source: v.source, sourceEpoch: v.sourceEpoch, epoch: v.epoch, stride: v.effectiveStride, origin: v.origin }])) };
        })); throw error;
    });
    await expect(page.locator('#scalar-render-row [data-scalar-mode="volume"]')).toHaveAttribute('aria-pressed', 'true');
    await page.locator('#btn-fluid').click();
    await expect(page.locator('#fluid-load')).toHaveCount(0);
    await expect(page.locator('#fluid-size')).toHaveCount(0);
    await expect(page.locator('.fluid-panel')).toHaveAttribute('data-state', 'observed');
    await page.evaluate(() => {
        const ctx = window.__ftdCtx;
        window._fluidTestOwner = ctx.useFluxMock ? ctx.fluxMock : ctx.bridge;
        window._fluidTestTick = window._fluidTestOwner.getDiagnostics().tick;
    });
    // Changing representations never loads another engine or advances physics.
    await page.locator('#scalar-render-row [data-scalar-mode="default"]').click();
    await page.locator('#scalar-render-row [data-scalar-mode="volume"]').click();
    await page.locator('#scalar-volume-opacity').fill('0.6');
    await page.locator('#scalar-volume-opacity').dispatchEvent('input');
    await page.waitForFunction(() => window.__ftdCtx.viewport._scalarVolumes.get('emEnergy').group.visible);
    expect(await page.evaluate(() => {
        const c = window.__ftdCtx, o = c.useFluxMock ? c.fluxMock : c.bridge;
        return { same: o === window._fluidTestOwner, tick: o.getDiagnostics().tick === window._fluidTestTick,
            strict: !!o.isStrictFluid, scalar: c.viewport._scalarVolumes.get('emEnergy').group.children.length };
    })).toEqual({ same: true, tick: true, strict: false, scalar: 1 });
    await page.locator('#btn-step').click();
    await page.waitForFunction(() => window._fluidTestOwner.getDiagnostics().tick > window._fluidTestTick);
    await page.screenshot({ path: '../test-results/scale0-native-fluid-volume.png' });
    await page.locator('#scenario-select').selectOption('s0-field-shear-layer');
    await page.waitForFunction(() => window.__ftdFluidPanel.observation?.scenarioId === 's0-field-shear-layer');
    await page.locator('#btn-reset').click();
    await page.waitForFunction(() => window.__ftdFluidPanel.observation?.scenarioId === 's0-field-shear-layer');
    await page.locator('#engine-mode').selectOption('particles');
    expect(await page.evaluate(() => [...window.__ftdCtx.viewport._scalarVolumes.values()].every(v => !v.group.visible))).toBe(true);
    await page.locator('#engine-mode').selectOption('lattice');
    await page.waitForFunction(() => !!window.__ftdFluidPanel);
    expect(workerUrls.some(url => url.includes('hydro-worker'))).toBe(false);
    expect(errors).toEqual([]);
});

test('physics controls change the active engine and field-energy layers belong to Visualization', async ({ page }) => {
    page.setDefaultTimeout(15000);
    const errors = [];
    page.on('pageerror', error => errors.push(error.message));
    await page.setViewportSize({ width: 1440, height: 1000 });
    await page.goto('/?engine=wasm', { waitUntil: 'domcontentloaded' });
    await page.waitForFunction(() => window.__ftdCtx?.bridge && window.__ftdFluidPanel);
    await page.locator('#scenario-select').selectOption('flux-thermalization');
    await page.waitForFunction(() => window.__ftdCtx.viewport._scalarVolumes?.get('emEnergy')?.group.visible);
    await page.locator('.tab[data-panel="controls"]').click();
    // Read authoritative worker publications rather than its optimistic setter cache.
    for (const [id, key, value] of [
        ['t-wave', 'wave_propagation', false],
        ['t-wave', 'wave_propagation', true],
        ['t-damping', 'damping', true],
        ['t-selective', 'selective_damping', true],
        ['t-selective', 'selective_damping', false],
        ['t-damping', 'damping', false],
        ['t-langevin', 'langevin', true],
        ['t-langevin', 'langevin', false],
    ]) {
        await page.locator(`#${id}`).setChecked(value);
        await page.waitForFunction(({ key, value }) => {
            const c = window.__ftdCtx, owner = c.useFluxMock ? c.fluxMock : c.bridge;
            return (owner.isWorker ? owner.getEngineTruthToggle(key) : owner.getToggle(key)) === value;
        }, { key, value });
    }
    await expect(page.locator('#physics-profile-warning')).toBeVisible();
    await page.locator('#btn-reset-physics-toggles').click();
    await page.waitForFunction(() => {
        const c = window.__ftdCtx, owner = c.useFluxMock ? c.fluxMock : c.bridge;
        const read = key => owner.isWorker ? owner.getEngineTruthToggle(key) : owner.getToggle(key);
        return read('wave_propagation') === true && read('damping') === false && read('langevin') === false;
    });
    const group = page.locator('.s0-overlay-col[data-col="stress-energy"]');
    await expect(group.locator('.s0-overlay-col-label')).toHaveText('Field energy & flow');
    const layers = ['toggle-em-energy', 'toggle-poynting', 'toggle-vorticity', 'toggle-e-pressure', 'toggle-b-pressure'];
    for (const id of layers) {
        await expect(page.locator(`#${id}`)).toHaveCount(1);
        await expect(group.locator(`#${id}`)).toHaveCount(1);
    }
    await page.locator('#s0-overlay-search').fill('fluid');
    for (const id of layers) await expect(group.locator(`#${id}`)).toBeVisible();
    await page.locator('#s0-overlay-search').fill('vorticity');
    await expect(group.locator('#toggle-vorticity')).toBeVisible();
    await page.locator('#s0-overlay-search').fill('fluid');
    await page.evaluate(() => {
        const c = window.__ftdCtx;
        window._fluidTestOwner = c.useFluxMock ? c.fluxMock : c.bridge;
        window._fluidTestTick = window._fluidTestOwner.getDiagnostics().tick;
    });
    for (const id of layers) {
        const button = group.locator(`#${id}`);
        if (await button.getAttribute('aria-pressed') !== 'true') await button.click();
    }
    await expect(group.locator('[data-count-for="stress-energy"]')).toHaveText('5');
    await page.waitForFunction(() => ['emEnergy', 'vorticity', 'ePressure', 'bPressure'].every(key => {
        const volume = window.__ftdCtx.viewport._scalarVolumes.get(key);
        return volume?.group.visible && volume.uploadCount > 0;
    }));
    await group.locator('[data-clear-col="stress-energy"]').click();
    await expect(group.locator('[data-count-for="stress-energy"]')).toHaveText('0');
    for (const id of layers) await expect(group.locator(`#${id}`)).toHaveAttribute('aria-pressed', 'false');
    expect(await page.evaluate(() => {
        const c = window.__ftdCtx, owner = c.useFluxMock ? c.fluxMock : c.bridge;
        return owner === window._fluidTestOwner && owner.getDiagnostics().tick === window._fluidTestTick
            && [...c.viewport._scalarVolumes.values()].every(volume => !volume.group.visible);
    })).toBe(true);
    expect(errors).toEqual([]);
});

test('one warmed energy volume and the passive Fluid pane meet the bounded hardware frame gate', async ({ page }) => {
    test.skip(process.env.FTD_HARDWARE_WEBGL !== '1', 'Hardware provenance requires the hardware launch.');
    page.setDefaultTimeout(15000);
    await page.setViewportSize({ width: 1440, height: 1000 });
    await page.goto('/?engine=wasm', { waitUntil: 'domcontentloaded' });
    await page.waitForFunction(() => window.__ftdCtx?.bridge && window.__ftdFluidPanel);
    await page.locator('#scenario-select').selectOption('flux-thermalization');
    await page.locator('#btn-fluid').click();
    await page.waitForFunction(() => window.__ftdCtx.viewport._scalarVolumes?.get('emEnergy')?.group.visible);
    await page.locator('#btn-play').click();
    await page.waitForFunction(() => {
        const ctx = window.__ftdCtx;
        return (ctx.useFluxMock ? ctx.fluxMock : ctx.bridge).getDiagnostics().tick >= 12;
    });
    const result = await page.evaluate(() => new Promise(resolve => {
        const c = window.__ftdCtx, gl = c.viewport.renderer.getContext(), ext = gl.getExtension('WEBGL_debug_renderer_info');
        const owner = c.useFluxMock ? c.fluxMock : c.bridge;
        const from = owner.getDiagnostics().tick, intervals = []; let previous = null;
        const frame = now => {
            if (previous !== null) intervals.push(now - previous); previous = now;
            if (intervals.length < 600) { requestAnimationFrame(frame); return; }
            const sorted = [...intervals].sort((a, b) => a - b);
            resolve({ renderer: ext ? gl.getParameter(ext.UNMASKED_RENDERER_WEBGL) : '',
                fps: 600000 / intervals.reduce((a, b) => a + b, 0), p95: sorted[569], p99: sorted[593],
                over33ms: intervals.filter(x => x > 33.4).length, ticks: [from, owner.getDiagnostics().tick],
                L: owner.latticeSize, volumes: [...c.viewport._scalarVolumes.values()].filter(v => v.group.visible).length });
        }; requestAnimationFrame(frame);
    }));
    console.log('NATIVE_FIELD_VOLUME_HARDWARE', JSON.stringify(result));
    expect(result.renderer).toMatch(/NVIDIA|AMD|Intel/);
    expect(result.renderer).not.toMatch(/SwiftShader|software/i);
    expect(result.volumes).toBe(1); expect(result.ticks[1]).toBeGreaterThan(result.ticks[0]);
    expect(result.fps).toBeGreaterThanOrEqual(59.5);
    expect(result.p95).toBeLessThanOrEqual(17); expect(result.p99).toBeLessThanOrEqual(20);
    expect(result.over33ms).toBe(0);
    await page.locator('#btn-play').click();
    await page.mouse.move(700, 300);
    await page.screenshot({ path: '../test-results/scale0-native-fluid-volume-evolved.png' });
});
