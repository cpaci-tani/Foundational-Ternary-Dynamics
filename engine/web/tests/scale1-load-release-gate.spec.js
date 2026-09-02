// @ts-check
/**
 * Opt-in Scale-1 browser load gate.
 *
 * Run on hardware WebGL from engine/web/tests:
 *   FTD_HARDWARE_WEBGL=1 FTD_SCALE1_LOAD_GATE=1 npx playwright test scale1-load-release-gate.spec.js --workers=1
 */
import { test, expect } from '@playwright/test';
import { attachConsoleWatcher, gotoAndReady, realErrors } from './_helpers.js';

test.use({ trace: 'off' });

const LOADS = [
    { count: 32, minTickHz: 55 },
    { count: 64, minTickHz: 45 },
    { count: 128, minTickHz: 20 },
    { count: 256, minTickHz: 5 },
];
const PANEL_STATES = ['closed', 'diagnostics'];

test.describe('Scale 1 sustained-load release gate', () => {
    test.skip(process.env.FTD_SCALE1_LOAD_GATE !== '1',
        'Set FTD_SCALE1_LOAD_GATE=1 to run the sustained multi-load probe.');

    for (const load of LOADS) for (const panelState of PANEL_STATES) {
        test(`${load.count} particles with ${panelState} panels remains responsive`,
        async ({ page }, testInfo) => {
            testInfo.setTimeout(120_000);
            const consoleErrors = attachConsoleWatcher(page);
            await gotoAndReady(page, { path: '/?engine=wasm', timeout: 90_000 });
            await page.selectOption('#engine-mode', 'particles');
            await page.selectOption('#pe-scenario-select', 's1-empty-zoo');

            const webglRenderer = await page.evaluate(() => {
                const canvas = document.querySelector('#viewport canvas');
                const gl = canvas?.getContext?.('webgl2') || canvas?.getContext?.('webgl');
                const rendererInfo = gl?.getExtension?.('WEBGL_debug_renderer_info') || null;
                return rendererInfo
                    ? String(gl.getParameter(rendererInfo.UNMASKED_RENDERER_WEBGL) || '')
                    : String(gl?.getParameter?.(gl.RENDERER) || '');
            });
            if (process.env.FTD_HARDWARE_WEBGL === '1') {
                expect(webglRenderer, 'hardware release gate requires renderer provenance')
                    .not.toBe('');
                expect(webglRenderer, 'release gate does not certify software WebGL')
                    .not.toMatch(/swiftshader|software|llvmpipe/i);
            }

            if (panelState === 'diagnostics') {
                await page.locator('[data-panel="diagnostics"], #panel-tab-diagnostics')
                    .first().click().catch(() => {});
            } else {
                await page.evaluate(() => {
                    document.querySelectorAll('.side-panel:not([hidden]) [data-action="close-panel"]')
                        .forEach(button => button.click());
                });
            }

            const setup = await page.evaluate((particleCount) => {
                const bridge = window.__ftdCtx?.bridge;
                if (!bridge) throw new Error('Scale 1 bridge unavailable');
                for (const spec of Array.from(bridge.peGetPhysicsRegistry()?.physics || [])) {
                    if (spec.available && spec.toggle) bridge.peSetToggle(spec.toggle, true);
                }
                const side = 6;
                for (let i = 0; i < particleCount; i++) {
                    const x = (i % side) * 1.4 - 3.5;
                    const y = (Math.floor(i / side) % side) * 1.4 - 3.5;
                    const z = Math.floor(i / (side * side)) * 1.4 - 2.1;
                    bridge.peAddParticle(
                        'electron', 1, x, y, z,
                        0, 0, 0, 0.511, 0.08,
                    );
                }
                return {
                    count: bridge.peParticleCount(),
                    tick: bridge.peGetTick(),
                };
            }, load.count);
            expect(setup.count).toBe(load.count);

            const paused = await page.evaluate(async () => {
                const probe = await import('/tests/scale0-ui-audit-probe.js');
                const bridge = window.__ftdCtx.bridge;
                const startTick = bridge.peGetTick();
                probe.startScale0UiAuditProbe({ rootSelector: '#viewport' });
                await new Promise(resolve => setTimeout(resolve, 3000));
                return {
                    ...await probe.stopScale0UiAuditProbe(),
                    startTick,
                    endTick: bridge.peGetTick(),
                };
            });
            expect(paused.endTick).toBe(paused.startTick);
            expect(paused.frames.effectiveFps).toBeGreaterThanOrEqual(59.5);
            expect(paused.longTasks).toEqual([]);

            const running = await page.evaluate(async () => {
                const probe = await import('/tests/scale0-ui-audit-probe.js');
                const bridge = window.__ftdCtx.bridge;
                document.getElementById('btn-play')?.click();
                const startTick = bridge.peGetTick();
                const started = performance.now();
                probe.startScale0UiAuditProbe({ rootSelector: '#viewport' });
                await new Promise(resolve => setTimeout(resolve, 6000));
                const elapsedSeconds = (performance.now() - started) / 1000;
                const report = await probe.stopScale0UiAuditProbe();
                const endTick = bridge.peGetTick();
                document.getElementById('btn-play')?.click();
                return {
                    ...report,
                    startTick,
                    endTick,
                    tickHz: (endTick - startTick) / elapsedSeconds,
                };
            });
            await testInfo.attach(`scale1-${load.count}-${panelState}-performance.json`, {
                body: Buffer.from(JSON.stringify({
                    load, panelState, webglRenderer, paused, running,
                }, null, 2)),
                contentType: 'application/json',
            });

            expect(running.frames.effectiveFps).toBeGreaterThanOrEqual(59.5);
            expect(running.tickHz).toBeGreaterThanOrEqual(load.minTickHz);
            expect(running.longTasks).toEqual([]);
            expect(realErrors(consoleErrors)).toEqual([]);
        });
    }
});
