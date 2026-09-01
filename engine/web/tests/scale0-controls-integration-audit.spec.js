// @ts-check
import { test, expect } from '@playwright/test';
import { attachConsoleWatcher, gotoAndReady, realErrors } from './_helpers.js';

test.describe('Scale 0 Controls sidepanel integration gate', () => {
    test('six-card composition is idempotent and floated layouts stay one or two columns', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        const result = await page.evaluate(async () => {
            const [{ Scale0ControlsComponent }, { floatingWindowManager }] = await Promise.all([
                import('/js/scales/scale0/ui/controls/component.js'),
                import('/js/ui/components/floating-window/component.js'),
            ]);
            const panel = document.getElementById('panel-controls');
            const grid = document.getElementById('panel-controls-grid');
            const before = [...grid.querySelectorAll('[data-scale0-control-card]')];
            for (let i = 0; i < 10; i++) new Scale0ControlsComponent(panel).init();
            const after = [...grid.querySelectorAll('[data-scale0-control-card]')];
            const duplicateIds = [...grid.querySelectorAll('[id]')]
                .map((element) => element.id)
                .filter((id, index, ids) => ids.indexOf(id) !== index);

            const dock = window.__ftdCtx?.appShell?.panelDock;
            floatingWindowManager.getWindow('controls')?.dock();
            const win = dock.floatPanel('controls', 220, 90);
            const nextLayout = () => new Promise((resolve) => (
                requestAnimationFrame(() => requestAnimationFrame(resolve))
            ));
            const columnsAt = async (width) => {
                win.el.style.width = `${width}px`;
                await nextLayout();
                const tracks = getComputedStyle(grid).gridTemplateColumns.trim();
                return tracks && tracks !== 'none' ? tracks.split(/\s+/).length : 0;
            };
            const columns = {
                narrow: await columnsAt(420),
                wide: await columnsAt(780),
                ultra: await columnsAt(1100),
            };
            const containerName = getComputedStyle(win.body).containerName;
            win.dock();

            return {
                keys: after.map((card) => card.dataset.scale0ControlCard),
                count: after.length,
                retainedIdentity: after.every((card, index) => card === before[index]),
                duplicateIds: [...new Set(duplicateIds)],
                columns,
                containerName,
            };
        });

        expect(result.keys).toEqual([
            'physics', 'substrate', 'flux-volume', 'flow-lines',
            'particle-display', 'selection',
        ]);
        expect(result.count).toBe(6);
        expect(result.retainedIdentity).toBe(true);
        expect(result.duplicateIds).toEqual([]);
        expect(result.columns).toEqual({ narrow: 1, wide: 2, ultra: 2 });
        expect(result.containerName).toBe('floating-sidepanel');
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('combined six-card input burst sustains the formal hardware frame budget', async ({ page }, testInfo) => {
        testInfo.setTimeout(120_000);
        const consoleErrors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await page.waitForTimeout(3_000);
        const report = await page.evaluate(async () => {
            const probe = await import('/tests/scale0-ui-audit-probe.js');
            const gl = window.__ftdCtx?.viewport?.renderer?.getContext?.() || null;
            const rendererInfo = gl?.getExtension?.('WEBGL_debug_renderer_info') || null;
            const webglRenderer = rendererInfo
                ? String(gl.getParameter(rendererInfo.UNMASKED_RENDERER_WEBGL) || '')
                : '';
            const sliders = [
                document.getElementById('flux-opacity'),
                document.getElementById('particle-opacity'),
                document.getElementById('flow-line-density'),
                document.getElementById('sel-radius'),
            ];
            const injX = document.getElementById('inj-x');
            const physics = document.getElementById('t-evaporation');
            probe.startScale0UiAuditProbe({ rootSelector: '#panel-controls-grid' });
            for (let i = 0; i < 40; i++) {
                await probe.measureScale0UiActionToPaint(`combined controls ${i}`, () => {
                    sliders[0].value = String(0.3 + (i % 20) * 0.02);
                    sliders[1].value = String(0.5 + (i % 20) * 0.02);
                    sliders[2].value = String(25 + (i % 4) * 25);
                    sliders[3].value = String(1 + (i % 10));
                    for (const slider of sliders) slider.dispatchEvent(new Event('input', { bubbles: true }));
                    injX.value = String(i % 33);
                    injX.dispatchEvent(new Event('change', { bubbles: true }));
                    physics.click();
                });
            }
            await new Promise((resolve) => setTimeout(resolve, 12_000));
            return { ...await probe.stopScale0UiAuditProbe(), webglRenderer };
        });
        await testInfo.attach('scale0-controls-integration-performance-report.json', {
            body: Buffer.from(JSON.stringify(report, null, 2)),
            contentType: 'application/json',
        });
        console.log('scale0 controls integration performance', JSON.stringify(report));

        if (process.env.FTD_HARDWARE_WEBGL === '1') {
            expect(report.webglRenderer, 'release gate exposes a WebGL renderer').not.toBe('');
            expect(report.webglRenderer, 'release gate does not certify SwiftShader/software WebGL')
                .not.toMatch(/swiftshader|software/i);
        }
        expect(report.frames.count).toBeGreaterThanOrEqual(600);
        expect(report.frames.effectiveFps).toBeGreaterThanOrEqual(59.5);
        expect(report.frames.p95Ms).toBeLessThanOrEqual(17);
        expect(report.frames.p99Ms).toBeLessThanOrEqual(20);
        expect(report.frames.intervalsOver33_4ms).toBe(0);
        expect(report.longTasks).toEqual([]);
        expect(report.actions.p95Ms).toBeLessThanOrEqual(50);
        expect(report.resourceDelta.rafSubscribers).toBe(0);
        expect(report.resourceDelta.domNodes).toBe(0);
        expect(report.resourceDelta.canvases).toBe(0);
        expect(report.errors).toEqual([]);
        expect(realErrors(consoleErrors)).toEqual([]);
    });
});
