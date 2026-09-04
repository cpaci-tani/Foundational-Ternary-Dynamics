// @ts-check
import { test, expect } from '@playwright/test';
import { attachConsoleWatcher, gotoAndReady, realErrors, switchMode } from './_helpers.js';

test.describe('Scale 3 molecule engine', () => {
    test('all canonical scenario contracts load with exact seed topology and finite dynamics', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'molecules');

        const result = await page.evaluate(async () => {
            const registry = await import('/js/scales/scale3/scenario-registry.js');
            const validation = registry.validateScale3ScenarioRegistry();
            const select = /** @type {HTMLSelectElement} */ (document.getElementById('mol-scenario-select'));
            const bridge = window._ftdBridge;
            const failures = [];
            for (const scenario of registry.SCALE3_SCENARIOS) {
                select.value = scenario.id;
                select.dispatchEvent(new Event('change', { bubbles: true }));
                const initial = bridge.aeGetDiagnostics();
                const molecule = bridge.aeGetMoleculeDiagnostics();
                const runtime = bridge.aeGetRuntimeState();
                const expectedBonds = scenario.expected.bondCount ?? scenario.expected.initialBondCount;
                if (initial.atomCount !== scenario.expected.atomCount ||
                    (expectedBonds !== undefined && initial.bondCount !== expectedBonds) ||
                    (scenario.expected.componentCount !== undefined && molecule.componentCount !== scenario.expected.componentCount)) {
                    failures.push(`${scenario.id}: seed ${initial.atomCount}/${initial.bondCount}/${molecule.componentCount}`);
                }
                for (const [key, enabled] of Object.entries(scenario.physics)) {
                    if (runtime.toggles[key] !== enabled) failures.push(`${scenario.id}: toggle ${key}`);
                }
                for (let tick = 0; tick < 40; tick++) bridge.aeTick();
                const after = bridge.aeGetDiagnostics();
                const afterMolecule = bridge.aeGetMoleculeDiagnostics();
                const finite = [after.totalEnergy, after.momentumX, after.momentumY, after.momentumZ,
                    afterMolecule.translationalKE, afterMolecule.rotationalKE,
                    afterMolecule.vibrationalKE, afterMolecule.radiusOfGyration,
                    afterMolecule.bondRmsStrain].every(Number.isFinite);
                if (!finite || after.lastError !== 'ok') failures.push(`${scenario.id}: finite/status`);
            }
            return {
                validation,
                optionCount: select.options.length,
                failures,
                detailsVisible: getComputedStyle(document.getElementById('mol-scenario-desc')).display !== 'none',
                detailsText: document.getElementById('mol-scenario-desc-text')?.textContent || '',
            };
        });

        expect(result.validation).toEqual({ ok: true, errors: [], count: 35 });
        expect(result.optionCount).toBe(35);
        expect(result.failures).toEqual([]);
        expect(result.detailsVisible).toBe(true);
        expect(result.detailsText).toContain('Evidence:');
        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('dissociation and capture change live topology in opposite directions', async ({ page }) => {
        await gotoAndReady(page);
        await switchMode(page, 'molecules');
        const outcomes = await page.evaluate(() => {
            const select = /** @type {HTMLSelectElement} */ (document.getElementById('mol-scenario-select'));
            const bridge = window._ftdBridge;
            const run = (id, ticks) => {
                select.value = id;
                select.dispatchEvent(new Event('change', { bubbles: true }));
                const before = bridge.aeGetMoleculeDiagnostics();
                for (let i = 0; i < ticks; i++) bridge.aeTick();
                return { before, after: bridge.aeGetMoleculeDiagnostics(), status: bridge.aeGetDiagnostics().lastError };
            };
            return {
                dissociation: run('mol-h2-dissociation', 450),
                capture: run('mol-h2-recombination', 550),
            };
        });
        expect(outcomes.dissociation.before.bondCount).toBe(1);
        expect(outcomes.dissociation.after.bondCount).toBe(0);
        expect(outcomes.dissociation.after.brokenBonds).toBe(1);
        expect(outcomes.capture.before.bondCount).toBe(0);
        expect(outcomes.capture.after.bondCount).toBe(1);
        expect(outcomes.capture.after.formedBonds).toBe(1);
        expect(outcomes.dissociation.status).toBe('ok');
        expect(outcomes.capture.status).toBe('ok');
    });

    test('Scale 3 exposes molecular telemetry without nuclear controls', async ({ page }) => {
        await gotoAndReady(page);
        await switchMode(page, 'molecules');
        await page.click('[data-panel="diagnostics"]');
        await page.waitForTimeout(300);
        const ui = await page.evaluate(() => ({
            nuclearCardVisible: [...document.querySelectorAll('.scale2-only')].some((node) =>
                node.textContent?.includes('Nuclear Transport Laboratory') && getComputedStyle(node).display !== 'none'),
            moleculeRows: document.querySelectorAll('.diag-scale3-root [data-row]').length,
            atomRowsVisible: getComputedStyle(document.querySelector('.diag-scale2-root')).display !== 'none',
            moleculeRootVisible: getComputedStyle(document.querySelector('.diag-scale3-root')).display !== 'none',
            overlayTitle: document.querySelector('#ae-viewport-overlay .scale3-only')?.textContent?.trim(),
            inspectorMode: document.getElementById('insp-mode-label')?.textContent?.trim(),
            inspectorPrompt: document.getElementById('insp-selection-summary')?.textContent?.trim(),
        }));
        expect(ui.nuclearCardVisible).toBe(false);
        expect(ui.moleculeRows).toBeGreaterThan(20);
        expect(ui.atomRowsVisible).toBe(false);
        expect(ui.moleculeRootVisible).toBe(true);
        expect(ui.overlayTitle).toBe('Molecule overlays');
        expect(ui.inspectorMode).toBe('Molecules');
        expect(ui.inspectorPrompt).toContain('live molecular component');
    });

    test('a moving molecule remains directly pickable and live in the inspector', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'molecules');
        await page.selectOption('#mol-scenario-select', 'mol-water-rotation');
        await page.waitForTimeout(400);

        const target = await page.evaluate(() => {
            const inspector = window.__ftdCtx.inspector;
            const atomData = window._ftdBridge.aeGetAtomData();
            const rect = inspector.viewport.renderer.domElement.getBoundingClientRect();
            const point = inspector._aePickProjection
                .set(atomData.positions[0], atomData.positions[1], atomData.positions[2])
                .project(inspector.viewport.camera);
            return {
                id: Number(atomData.ids[0]),
                x: rect.left + (point.x + 1) * rect.width / 2,
                y: rect.top + (1 - point.y) * rect.height / 2,
            };
        });

        await page.mouse.click(target.x, target.y);
        await page.click('[data-panel="inspector"]');
        await expect(page.locator('#ae-inspector-content')).toBeVisible();
        await expect(page.locator('#insp-selection-summary')).toContainText(`Selected atom #${target.id}`);
        await expect(page.locator('#ae-insp-component-kind')).toHaveText('Bonded component');
        await expect(page.locator('#ae-insp-component-members')).toContainText(`#${target.id}`);
        // Each live bond renders one partner row and one distance/equilibrium
        // row, so the selected water oxygen exposes four detail cells.
        await expect(page.locator('#ae-insp-bonds dd')).toHaveCount(4);

        const before = await page.locator('#ae-insp-pos').textContent();
        const tickBefore = await page.evaluate(() => window._ftdBridge.aeGetDiagnostics().tick);
        await page.click('#btn-play');
        await expect.poll(
            () => page.evaluate(() => window._ftdBridge.aeGetDiagnostics().tick),
            { timeout: 5_000, message: 'molecule engine advances while an atom is selected' },
        ).toBeGreaterThan(tickBefore);
        await expect.poll(
            () => page.locator('#ae-insp-pos').textContent(),
            { timeout: 5_000, message: 'selected atom position remains live during motion' },
        ).not.toBe(before);

        const selected = await page.evaluate(() => window.__ftdCtx.inspector._selectedAEAtomId);
        expect(selected).toBe(target.id);
        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });
});
