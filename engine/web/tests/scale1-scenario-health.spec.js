// @ts-check
/**
 * Scale-1 admitted-scenario mechanical health matrix.
 *
 * This is deliberately not a physics-validation oracle.  Native registry
 * evidence owns each scenario's scientific qualification.  The matrix proves
 * that every admitted row can be selected through the production dashboard,
 * seeds its declared state, advances when dynamic, publishes finite complete
 * records, applies its exact registered physics mask, and remains free of
 * browser errors.
 */
import { test, expect } from '@playwright/test';
import { attachConsoleWatcher, gotoAndReady, realErrors } from './_helpers.js';

const NON_ADVANCING_BEHAVIORS = new Set([
    'read_only_replay',
    'static_field',
    'static_reference',
    'awaiting_input',
]);

async function selectScenario(page, id) {
    await page.selectOption('#pe-scenario-select', id);
    await expect.poll(() => page.evaluate((scenarioId) => {
        const state = window.__ftdCtx?.scale1State;
        const selected = document.getElementById('pe-scenario-select')?.value;
        const snap = window.__ftdCtx?.bridge?.peGetSnapshot?.(scenarioId);
        return selected === scenarioId && snap?.core?.scenario === scenarioId;
    }, id), { timeout: 15_000, message: `Scale 1 scenario ${id} did not settle` }).toBe(true);
}

test.describe('Scale 1 admitted scenario health', () => {
    test('all registered scenarios remain mechanically healthy', async ({ page }, testInfo) => {
        testInfo.setTimeout(240_000);
        const consoleErrors = attachConsoleWatcher(page);
        await gotoAndReady(page, { path: '/?engine=wasm', timeout: 90_000 });
        await page.selectOption('#engine-mode', 'particles');

        await expect.poll(() => page.evaluate(() => {
            const registry = window.__ftdCtx?.bridge?.peGetPhysicsRegistry?.();
            return Array.from(registry?.scenarios || []).length;
        }), {
            timeout: 30_000,
            message: 'Scale 1 native scenario registry did not finish hydrating',
        }).toBe(36);
        await expect.poll(
            () => page.locator('#pe-scenario-select option').count(),
            { timeout: 30_000, message: 'Scale 1 scenario selector did not finish hydrating' }
        ).toBe(36);

        const manifest = await page.evaluate(() => {
            const bridge = window.__ftdCtx?.bridge;
            const registry = bridge?.peGetPhysicsRegistry?.();
            return {
                scenarios: Array.from(registry?.scenarios || []).map(row => ({
                    id: row.id,
                    mode: row.mode,
                    behavior: row.behavior,
                    available: row.available,
                    physicsMask: row.physicsMask,
                    validationState: row.validationState,
                })),
                physics: Array.from(registry?.physics || [])
                    .map(row => ({ toggle: row.toggle, available: row.available })),
                visible: Array.from(document.querySelectorAll('#pe-scenario-select option'))
                    .map(option => option.value).filter(Boolean),
            };
        });

        expect(manifest.scenarios).toHaveLength(36);
        expect(manifest.scenarios.every(row => row.available)).toBe(true);
        expect(new Set(manifest.visible)).toEqual(new Set(manifest.scenarios.map(row => row.id)));

        const rows = [];
        for (const scenario of manifest.scenarios) {
            const errorStart = consoleErrors.length;
            await selectScenario(page, scenario.id);

            const row = await page.evaluate(({ scenario, physics }) => {
                const bridge = window.__ftdCtx.bridge;
                const beforeTick = Number(bridge.peGetTick?.() || 0);
                if (!['read_only_replay', 'static_field', 'static_reference', 'awaiting_input']
                    .includes(scenario.behavior)) {
                    for (let i = 0; i < 12; i++) bridge.peTick();
                }
                const afterTick = Number(bridge.peGetTick?.() || 0);
                const data = bridge.peGetParticleData?.() || { count: 0 };
                const diagnostics = bridge.peGetDiagnostics?.() || {};
                const snapshot = bridge.peGetSnapshot?.(scenario.id) || {};
                const finite = values => Array.from(values || []).every(Number.isFinite);
                const ids = Array.from(data.ids || []);
                const expectedToggles = physics.map((spec, index) => ({
                    name: spec.toggle,
                    expected: !!(scenario.physicsMask & (1 << index)) && !!spec.available,
                    actual: !!bridge.peGetToggle?.(spec.toggle),
                }));
                return {
                    id: scenario.id,
                    behavior: scenario.behavior,
                    beforeTick,
                    afterTick,
                    count: Number(data.count || 0),
                    idsUnique: new Set(ids).size === ids.length,
                    finiteState: finite(data.positions) && finite(data.velocities)
                        && finite(data.masses) && finite(data.charges),
                    finiteDiagnostics: Number.isFinite(Number(diagnostics.totalEnergy))
                        && Number.isFinite(Number(diagnostics.totalKE))
                        && Number.isFinite(Number(diagnostics.totalPE)),
                    snapshotScenario: snapshot?.core?.scenario || '',
                    snapshotStatus: snapshot?.core?.validationState || '',
                    objectCount: Array.from(snapshot?.objects || []).length,
                    // The native-matter replay does not execute ParticleEngine
                    // terms; its hidden engine instance deliberately rejects
                    // mutation while the immutable artifact is selected.
                    toggleMismatches: scenario.mode === 'native_matter' ? []
                        : expectedToggles.filter(toggle => toggle.actual !== toggle.expected),
                };
            }, { scenario, physics: manifest.physics });
            row.errors = realErrors(consoleErrors.slice(errorStart));
            rows.push(row);
        }

        await testInfo.attach('scale1-scenario-health.json', {
            body: Buffer.from(JSON.stringify(rows, null, 2)),
            contentType: 'application/json',
        });

        const failures = rows.filter(row =>
            !row.idsUnique || !row.finiteState || !row.finiteDiagnostics
            || row.snapshotScenario !== row.id
            || row.toggleMismatches.length > 0
            || row.errors.length > 0
            || (row.behavior !== 'awaiting_input'
                && Math.max(row.count, row.objectCount) === 0)
            || (!NON_ADVANCING_BEHAVIORS.has(row.behavior)
                && row.afterTick <= row.beforeTick));

        expect(failures, JSON.stringify(failures, null, 2)).toEqual([]);
        expect(realErrors(consoleErrors)).toEqual([]);
    });
});
