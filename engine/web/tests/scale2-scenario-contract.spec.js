// @ts-check
import { test, expect } from '@playwright/test';
import { gotoAndReady, switchMode, attachConsoleWatcher, realErrors } from './_helpers.js';

test.describe('Scale 2 canonical scenario contract', () => {
    test('every physics checkbox writes through the canonical registry binding', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'atoms');

        const audit = await page.evaluate(async () => {
            const registry = await import('./js/scales/scale2/scenario-registry.js');
            const bridge = window._ftdBridge;
            const failures = [];

            for (const spec of registry.AE_PHYSICS_SPECS) {
                const checkbox = /** @type {HTMLInputElement | null} */ (
                    document.getElementById(spec.elementId));
                const setter = bridge[spec.setter];
                if (!checkbox) { failures.push(`${spec.key}:missing checkbox`); continue; }
                if (typeof setter !== 'function') { failures.push(`${spec.key}:missing ${spec.setter}`); continue; }

                const before = bridge.aeGetRuntimeState().toggles[spec.key];
                checkbox.checked = !before;
                checkbox.dispatchEvent(new Event('change', { bubbles: true }));
                const after = bridge.aeGetRuntimeState().toggles[spec.key];
                if (after !== !before) failures.push(`${spec.key}:UI change did not reach engine`);

                checkbox.checked = before;
                checkbox.dispatchEvent(new Event('change', { bubbles: true }));
                const restored = bridge.aeGetRuntimeState().toggles[spec.key];
                if (restored !== before) failures.push(`${spec.key}:could not restore prior state`);
            }

            return { count: registry.AE_PHYSICS_SPECS.length, failures };
        });

        expect(audit.count).toBe(11);
        expect(audit.failures, audit.failures.join('\n')).toEqual([]);
        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('all 150 selector entries load, match their physics profile, and remain finite', async ({ page }) => {
        test.setTimeout(180_000);
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'atoms');

        const audit = await page.evaluate(async () => {
            const registry = await import('./js/scales/scale2/scenario-registry.js');
            const select = /** @type {HTMLSelectElement} */ (document.getElementById('ae-scenario-select'));
            const bridge = window._ftdBridge;
            const ids = Array.from(select.options, option => option.value);
            const failures = [];
            const load = (id) => {
                select.value = id;
                select.dispatchEvent(new Event('change', { bubbles: true }));
            };

            for (const id of ids) {
                const meta = registry.getAEScenarioMeta(id);
                if (!meta) { failures.push(`${id}:missing metadata`); continue; }
                load(id);
                const data = bridge.aeGetAtomData();
                const diag = bridge.aeGetDiagnostics();
                const runtime = bridge.aeGetRuntimeState();
                if (data.count !== meta.expected.atomCount)
                    failures.push(`${id}:atoms ${data.count} != ${meta.expected.atomCount}`);
                if (Number.isFinite(meta.expected.bondCount) && diag.bondCount !== meta.expected.bondCount)
                    failures.push(`${id}:bonds ${diag.bondCount} != ${meta.expected.bondCount}`);
                for (const spec of registry.AE_PHYSICS_SPECS) {
                    if (runtime.toggles[spec.key] !== meta.physics[spec.key])
                        failures.push(`${id}:${spec.key}=${runtime.toggles[spec.key]} expected ${meta.physics[spec.key]}`);
                }
                if (meta.expected.dynamic) {
                    for (let tick = 0; tick < 8; tick++) {
                        if (bridge.aeTick() === false) break;
                    }
                    const after = bridge.aeGetAtomData();
                    const afterDiag = bridge.aeGetDiagnostics();
                    const finite = [...after.positions, ...after.charges,
                        afterDiag.totalKE, afterDiag.totalEnergy,
                        afterDiag.momentumX, afterDiag.momentumY, afterDiag.momentumZ]
                        .every(Number.isFinite);
                    if (!finite) failures.push(`${id}:non-finite runtime state`);
                    if (afterDiag.lastError !== 'ok') failures.push(`${id}:${afterDiag.lastError}`);
                }
            }

            load('ae-thermal-gas');
            const first = Array.from(bridge.aeGetAtomData().positions);
            load('ae-thermal-gas');
            const second = Array.from(bridge.aeGetAtomData().positions);
            const deterministic = first.length === second.length && first.every((v, i) => v === second[i]);
            return {
                count: ids.length,
                curated: registry.validateAEScenarioRegistry(),
                failures,
                deterministic,
            };
        });

        expect(audit.count).toBe(150);
        expect(audit.curated.ok, audit.curated.errors.join('\n')).toBe(true);
        expect(audit.curated.count).toBe(33);
        expect(audit.failures, audit.failures.join('\n')).toEqual([]);
        expect(audit.deterministic).toBe(true);
        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('largest and most coupled applicable profiles remain finite over extended runs', async ({ page }) => {
        test.setTimeout(120_000);
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'atoms');
        const audit = await page.evaluate(() => {
            const select = /** @type {HTMLSelectElement} */ (document.getElementById('ae-scenario-select'));
            const bridge = window._ftdBridge;
            const profiles = [
                ['ae-water-cluster', 1200],
                ['ae-polar-dimer', 1200],
                ['ae-thermal-gas', 1800],
                ['ae-damped-relaxation', 1200],
            ];
            const results = [];
            for (const [id, ticks] of profiles) {
                select.value = id;
                select.dispatchEvent(new Event('change', { bubbles: true }));
                let tickAccepted = true;
                for (let tick = 0; tick < ticks; tick++) {
                    if (bridge.aeTick() === false) { tickAccepted = false; break; }
                }
                const data = bridge.aeGetAtomData();
                const diag = bridge.aeGetDiagnostics();
                const runtime = bridge.aeGetRuntimeState();
                results.push({
                    id,
                    tickAccepted,
                    tick: diag.tick,
                    finite: [...data.positions, ...data.charges,
                        diag.totalKE, diag.totalEnergy, diag.forceClampScale,
                        diag.momentumX, diag.momentumY, diag.momentumZ].every(Number.isFinite),
                    lastError: diag.lastError,
                    forceClampEvents: runtime.forceClampEvents,
                });
            }
            return results;
        });

        for (const result of audit) {
            expect(result.tickAccepted, `${result.id} rejected a tick`).toBe(true);
            expect(result.tick, `${result.id} did not complete its run`).toBeGreaterThanOrEqual(1200);
            expect(result.finite, `${result.id} produced non-finite telemetry`).toBe(true);
            expect(result.lastError, `${result.id} engine error`).toBe('ok');
            expect(Number.isInteger(result.forceClampEvents)).toBe(true);
            expect(result.forceClampEvents).toBeGreaterThanOrEqual(0);
        }
        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });
});
