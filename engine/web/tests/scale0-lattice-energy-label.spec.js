// @ts-check
/**
 * The joules figure above the Scale 0 bracket.
 *
 * It names the energy of whatever the bracket spans: one voxel when zoomed
 * in, the whole lattice when the bracket spans the lattice, and nothing once
 * the bracket has moved on to the quasi-domain. The lattice figure is the
 * engine's total energy, checked here against the energy audit of the same
 * tick.
 */
import { test, expect } from '@playwright/test';
import { attachConsoleWatcher, gotoAndReady, openDockPanel, realErrors } from './_helpers.js';

/** The label, and the engine values of the tick it was drawn from. */
async function readEnergy(page) {
    return page.evaluate(async () => {
        const { telemetryHub } = await import('/js/telemetry-hub.js');
        const { formatEnergy, latticeEnergyEv } = await import('/js/ui/components/live-rulers/measure.js');
        const { E_REST } = await import('/js/constants.js');
        const viewport = window.__ftdCtx.viewport;
        const string = document.querySelector('.live-ruler-lattice .live-ruler-string');
        const diag = telemetryHub.s0?.diag;
        const audit = telemetryHub.s0?.audit;
        const diagMeta = telemetryHub.getScale0TelemetryMeta('diagnostics');
        const auditMeta = telemetryHub.getScale0TelemetryMeta('audit');
        const ledgerTotal = diag && Number.isFinite(diag.dynamicEnergy) && Number.isFinite(diag.manifested)
            ? diag.dynamicEnergy + diag.manifested * E_REST : null;
        const auditCurrent = !!audit && !!auditMeta && auditMeta.stale !== true && auditMeta.tick === diagMeta?.tick;
        return {
            bracket: document.querySelector('.live-ruler-lattice .live-ruler-label')?.textContent || '',
            shown: !!string && string.getClientRects().length > 0,
            hiddenAttr: !!string?.hidden,
            text: string?.querySelector('.live-ruler-string-value')?.textContent || '',
            tooltip: string?.dataset.uiTooltip || '',
            tick: diagMeta?.tick ?? null,
            manifested: diag?.manifested ?? null,
            dynamic: diag?.dynamicEnergy ?? null,
            ledgerTotal,
            ledgerText: ledgerTotal === null ? null : formatEnergy(latticeEnergyEv(ledgerTotal)),
            auditTotal: auditCurrent ? audit.totalEnergy : null,
            auditText: auditCurrent ? formatEnergy(latticeEnergyEv(audit.totalEnergy)) : null,
            voxelText: viewport._voxelEnergyTrace ? formatEnergy(viewport._voxelEnergyTrace.last) : null,
        };
    });
}

const frame = (page, id) => page.evaluate((view) => window.__ftdCtx.viewport.setFramedView(view), id);

test.describe('Scale 0 joules figure', () => {
    test.beforeEach(async ({ page }) => {
        await gotoAndReady(page, { path: '/?engine=wasm', timeout: 90_000 });
        await page.waitForFunction(() => document.getElementById('app')?.dataset.shellReady === 'true');
        await page.evaluate(() => document.getElementById('gpu-server-card')?.remove());
    });

    test('at the lattice bracket it is the total energy of the lattice', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        await frame(page, 'lattice');
        // Run, then stop, so the figure and the engine values are one tick.
        await page.locator('#btn-play').click();
        await page.waitForFunction(() => (window.__ftdCtx.fluxMock?.currentTick?.() ?? 0) > 20, undefined, { timeout: 30_000 });
        await page.locator('#btn-play').click();
        await expect.poll(async () => {
            const now = await readEnergy(page);
            return now.shown && now.text === now.ledgerText;
        }, { timeout: 10_000 }).toBe(true);

        const ledger = await readEnergy(page);
        expect(ledger.bracket).toMatch(/^lattice · /);
        expect(ledger.text).toMatch(/^\d\.\d{3}×10[⁻⁰¹²³⁴⁵⁶⁷⁸⁹]+ J$/);
        expect(ledger.ledgerTotal).toBeGreaterThan(0);
        expect(ledger.ledgerTotal, 'rest energy is part of the total').toBeGreaterThanOrEqual(ledger.dynamic);
        expect(ledger.tooltip).toContain('Total energy of the lattice');

        // In the default view the clock face stands over the bracket; the figure is clear of it.
        const clearance = await page.evaluate(() => {
            const viewport = window.__ftdCtx.viewport;
            const view = viewport.container.getBoundingClientRect();
            const disc = viewport._sceneCore.clockDisc(view.width, view.height);
            const range = document.createRange();
            range.selectNodeContents(document.querySelector('.live-ruler-lattice .live-ruler-string-value'));
            const ink = range.getBoundingClientRect();
            const line = document.querySelector('.live-ruler-lattice .live-ruler-string svg').getBoundingClientRect();
            const cx = view.left + disc.x;
            const cy = view.top + disc.y;
            const dx = Math.max(line.left - cx, 0, cx - ink.right);
            const dy = Math.max(ink.top - cy, 0, cy - ink.bottom);
            return { gap: Math.hypot(dx, dy) - disc.r, radius: disc.r, left: line.left - view.left };
        });
        expect(clearance.radius).toBeGreaterThan(10);
        expect(clearance.gap, 'the figure does not cross the clock face').toBeGreaterThan(0);
        expect(clearance.left, 'and it stays inside the view').toBeGreaterThanOrEqual(0);

        // The engine's energy audit of the same tick gives the same total.
        await openDockPanel(page, 'diagnostics');
        await expect.poll(async () => (await readEnergy(page)).auditTotal !== null, { timeout: 15_000 }).toBe(true);
        const audited = await readEnergy(page);
        expect(audited.tick).toBe(ledger.tick);
        expect(Math.abs(audited.auditTotal - audited.ledgerTotal) / audited.auditTotal).toBeLessThan(1e-9);
        await expect.poll(async () => (await readEnergy(page)).text, { timeout: 5_000 }).toBe(audited.auditText);
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('it follows what the bracket spans and never keeps a figure from another stage', async ({ page }) => {
        await frame(page, 'lattice');
        await expect.poll(async () => (await readEnergy(page)).shown, { timeout: 10_000 }).toBe(true);
        const lattice = await readEnergy(page);
        expect(lattice.text).toBe(lattice.ledgerText);

        // Zoomed in, the bracket is one voxel and the figure is that voxel's.
        await frame(page, 'moore');
        await expect.poll(async () => (await readEnergy(page)).bracket, { timeout: 10_000 })
            .toMatch(/^(voxel|positive|negative|locked positive|locked negative) · /);
        await expect.poll(async () => {
            const now = await readEnergy(page);
            return now.shown && now.text === now.voxelText;
        }, { timeout: 10_000 }).toBe(true);
        const voxel = await readEnergy(page);
        expect(voxel.tooltip).toContain('Display energy of this voxel');
        expect(voxel.text, 'one voxel is not the lattice').not.toBe(voxel.ledgerText);

        // Back out, the lattice total returns in place of the voxel figure.
        await frame(page, 'lattice');
        await expect.poll(async () => {
            const now = await readEnergy(page);
            return now.bracket.startsWith('lattice · ') && now.shown && now.text === now.ledgerText;
        }, { timeout: 10_000 }).toBe(true);

        // Past the lattice the bracket spans the quasi-domain, which has no energy reading.
        // The hand-off happens once the whole quasi-domain fits the open width.
        await frame(page, 'quasi');
        await page.evaluate(() => {
            const viewport = window.__ftdCtx.viewport;
            const target = viewport.controls.target;
            viewport.camera.position.sub(target).multiplyScalar(2.5).add(target);
            viewport.controls.update();
        });
        await expect.poll(async () => (await readEnergy(page)).bracket, { timeout: 10_000 }).toMatch(/^quasi-domain · /);
        const quasi = await readEnergy(page);
        expect(quasi.hiddenAttr).toBe(true);
        expect(quasi.shown, 'a hidden figure is not drawn').toBe(false);

        // The Live Measure switch removes it and brings it back.
        await frame(page, 'lattice');
        await expect.poll(async () => (await readEnergy(page)).shown, { timeout: 10_000 }).toBe(true);
        await page.evaluate(() => window.__ftdCtx.viewport.setLiveMeasure({ energyString: false }));
        await expect.poll(async () => (await readEnergy(page)).shown, { timeout: 5_000 }).toBe(false);
        await page.evaluate(() => window.__ftdCtx.viewport.setLiveMeasure({ energyString: true }));
        await expect.poll(async () => (await readEnergy(page)).shown, { timeout: 5_000 }).toBe(true);
    });
});
