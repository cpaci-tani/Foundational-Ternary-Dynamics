import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

test('floating charts perform no resize work while collapsed, hidden or disposed', async ({ page }) => {
    await gotoAndReady(page, { path: '/?engine=wasm' });
    const result = await page.evaluate(async () => {
        const { FloatingWindow } = await import('/js/ui/components/floating-window/component.js');
        const panel = document.createElement('div');
        panel.className = 'panel active';
        const chart = document.createElement('div');
        let resizes = 0;
        chart._ftdResize = () => resizes++;
        panel.appendChild(chart);
        const win = new FloatingWindow('audit-fixture', { panelEl: panel }).init();
        win.triggerChartResize();
        const mounted = resizes;
        win.toggleCollapse();
        for (let i = 0; i < 30; i++) win.triggerChartResize();
        const collapsed = resizes;
        win.toggleCollapse();
        const restored = resizes;
        document.documentElement.classList.add('ui-hidden');
        try { win.triggerChartResize(); } finally {
            document.documentElement.classList.remove('ui-hidden');
        }
        const hidden = resizes;
        win.destroy();
        win.triggerChartResize();
        return { mounted, collapsed, restored, hidden, disposed: resizes };
    });
    expect(result).toEqual({ mounted: 1, collapsed: 1, restored: 2, hidden: 2, disposed: 2 });
});

test('unchanged clock observations do not rewrite DOM and hidden clocks do not animate', async ({ page }) => {
    await gotoAndReady(page, { path: '/?engine=wasm' });
    const result = await page.evaluate(() => {
        window.__ftdCtx.pauseSimulation();
        const core = window.__ftdCtx.viewport._sceneCore;
        const readout = document.getElementById('global-clock-readout');
        const state = { tick: 123, running: false, maxCausalBudget: 0.25, causalProjectionEvents: 0 };
        core.setGlobalClockState(state);
        const color = core._globalClockRateColor;
        const observer = new MutationObserver(() => {});
        observer.observe(readout, { attributes: true, childList: true, subtree: true });
        for (let i = 0; i < 100; i++) core.setGlobalClockState(state);
        const mutations = observer.takeRecords().length;
        observer.disconnect();
        const visible = core.globalClock.visible;
        core.globalClock.visible = false;
        core._globalClockPhaseCursor.rotation.z = 0.4321;
        core._animateGlobalClock(performance.now() + 1000);
        const rotation = core._globalClockPhaseCursor.rotation.z;
        core.globalClock.visible = visible;
        return { mutations, reusedColor: color === core._globalClockRateColor, rotation, text: readout.textContent };
    });
    expect(result.mutations).toBe(0);
    expect(result.reusedColor).toBe(true);
    expect(result.rotation).toBe(0.4321);
    expect(result.text).toBe('tick 123 · τ′min 0.866');
});

test('particle uniforms belong to one renderer and inactive-scale panels are not live', async ({ page }) => {
    await gotoAndReady(page, { path: '/?engine=wasm' });
    const result = await page.evaluate(async () => {
        const { PARTICLE_SHADER_UNIFORMS } = await import('/js/viewport/shaders.js');
        const { isPanelLive } = await import('/js/ui/panels/panel-visibility.js?v=2');
        const ctx = window.__ftdCtx;
        const uniforms = ctx.viewport.particles.material.uniforms;
        const aliases = Object.keys(PARTICLE_SHADER_UNIFORMS).filter(key => uniforms[key] === PARTICLE_SHADER_UNIFORMS[key]);
        const panel = document.createElement('div');
        panel.id = 'panel-audit-visibility';
        panel.className = 'panel active';
        panel.dataset.panelScales = '1';
        document.getElementById('app').appendChild(panel);
        const incompatible = [isPanelLive(panel), ctx.isPanelVisible('audit-visibility')];
        panel.dataset.panelScales = '0';
        const compatible = [isPanelLive(panel), ctx.isPanelVisible('audit-visibility')];
        panel.remove();
        const detached = isPanelLive(panel);
        return { aliases, incompatible, compatible, detached };
    });
    expect(result).toEqual({ aliases: [], incompatible: [false, false], compatible: [true, true], detached: false });
});
