// @ts-check
import { test, expect } from '@playwright/test';
import { gotoAndReady, switchMode, attachConsoleWatcher, realErrors } from './_helpers.js';

async function selectAEScenario(page, id) {
    await page.evaluate((scenarioId) => {
        const sel = document.getElementById('ae-scenario-select');
        if (!sel) throw new Error('ae-scenario-select not found');
        sel.value = scenarioId;
        sel.dispatchEvent(new Event('change', { bubbles: true }));
    }, id);
}

test.describe('Scale 2 atom scenarios and overlays', () => {
    test.beforeEach(async ({ page }) => {
        page.setDefaultTimeout(20_000);
    });

    test('ionic preset lights force arrows and the decomposition works on the default bridge', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        // Deliberately the DEFAULT page (no ?dev=1): the production bridge is
        // WasmBridge, whose AE surface forwards to the JS fallback. Before
        // 2026-06-10 it never forwarded aeGetForceDecomposition, so every
        // force-arrow toggle threw a TypeError each compute frame (B10).
        await gotoAndReady(page);
        await switchMode(page, 'atoms');
        await selectAEScenario(page, 'ae-nacl-form');

        await expect.poll(
            () => page.evaluate(() => window._ftdBridge?.aeGetAtomData?.()?.count || 0),
            { timeout: 10_000, message: 'ae-nacl-form did not seed atoms' },
        ).toBe(2);

        const state = await page.evaluate(async () => {
            const presets = await import('./js/scales/scale2/scenarios.js');
            const ctrl = await import('./js/scales/scale2/controller.js');
            const b = window._ftdBridge;
            const decomp = b.aeGetForceDecomposition({ ionic: true });
            let ionicMag = 0;
            for (let i = 0; i < decomp.count * 3; i++) ionicMag += Math.abs(decomp.ionic[i]);
            const rt = b.aeGetRuntimeState();
            return {
                bridgeType: b.constructor?.name,
                preset: presets.getAEScenarioPreset('ae-nacl-form').visuals,
                visState: ctrl.getAEVisualState(),
                buttons: {
                    ionic: document.getElementById('ae-force-ionic')?.classList.contains('active') || false,
                    vdw: document.getElementById('ae-force-vdw')?.classList.contains('active') || false,
                    field: document.getElementById('toggle-ae-field')?.classList.contains('active') || false,
                },
                decompCount: decomp.count,
                ionicLen: decomp.ionic.length,
                ionicNonzero: ionicMag > 0,
                runtimeToggles: rt?.toggles ?? null,
            };
        });

        // B10 regression: the production bridge must be WasmBridge AND the
        // decomposition must come back through its AE forwarding.
        expect(state.bridgeType).toBe('WasmBridge');
        expect(state.decompCount).toBe(2);
        expect(state.ionicLen).toBe(6);
        expect(state.ionicNonzero).toBe(true);

        // Preset → flags → DOM buttons, end to end.
        expect(state.preset.forceIonic).toBe(true);
        expect(state.preset.field).toBe(true);
        expect(state.visState.showAEForceIonic).toBe(true);
        expect(state.visState.showAEField).toBe(true);
        expect(state.buttons).toMatchObject({ ionic: true, vdw: false, field: true });

        // Engine truth from the runtime snapshot: NaCl disables auto-bonding.
        expect(state.runtimeToggles).not.toBeNull();
        expect(state.runtimeToggles.bonding).toBe(false);
        expect(state.runtimeToggles.ionic).toBe(true);

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('periodic table preset disables clouds; every visual control clicks cleanly', async ({ page }) => {
        // Heavy spec: the click audit exercises every per-frame overlay path.
        // 60s per the project's heavy-spec pattern (timeout-not-assert +
        // healthy page = load, not regression).
        test.setTimeout(60_000);
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'atoms');
        await selectAEScenario(page, 'ae-periodic');

        await expect.poll(
            () => page.evaluate(() => window._ftdBridge?.aeGetAtomData?.()?.count || 0),
            { timeout: 10_000, message: 'ae-periodic did not seed 118 atoms' },
        ).toBe(118);

        const preset = await page.evaluate(async () => {
            const ctrl = await import('./js/scales/scale2/controller.js');
            return {
                cloudsCheckbox: document.getElementById('ae-show-clouds')?.checked ?? null,
                visClouds: ctrl.getAEVisualState().showOrbitalClouds,
                visShells: ctrl.getAEVisualState().showNucleusShells,
            };
        });
        expect(preset.cloudsCheckbox).toBe(false);
        expect(preset.visClouds).toBe(false);
        expect(preset.visShells).toBe(false);

        // The click audit runs on the 6-atom water dimer, NOT the 118-atom
        // periodic table: with all four force channels on, the O(N²)
        // decomposition every 2nd frame saturates the main thread at N=118
        // and page.evaluate round-trips blow the test budget. Wiring
        // correctness (button → flag → viewport) is atom-count-independent.
        await selectAEScenario(page, 'ae-water-dimer');
        await expect.poll(
            () => page.evaluate(() => window._ftdBridge?.aeGetAtomData?.()?.count || 0),
            { timeout: 10_000, message: 'ae-water-dimer did not seed atoms' },
        ).toBe(6);

        // Play so per-frame update paths execute while we click.
        await page.evaluate(() => {
            const btn = document.getElementById('btn-play');
            if (btn && btn.getAttribute('data-paused') === 'true') btn.click();
        });

        // The end-to-end button→flag→viewport audit: click EVERY Scale 2
        // visual control on and off; any wiring break throws to the console.
        const clickIds = [
            'ae-show-shells', 'ae-show-shell-bounds', 'ae-show-lobes',
            'ae-force-ionic', 'ae-force-vdw', 'ae-force-bond', 'ae-force-net',
            'toggle-ae-field', 'toggle-ae-velocities', 'toggle-ae-dipoles',
            'toggle-ae-hbonds', 'ae-show-clouds',
        ];
        for (const id of clickIds) {
            await page.evaluate((elId) => {
                const el = document.getElementById(elId);
                if (!el) throw new Error(`missing control: ${elId}`);
                el.click();
            }, id);
            await page.waitForTimeout(150);
        }
        // Cycle the bond style select through all options.
        for (const styleValue of ['lines', 'off', 'cylinders']) {
            await page.evaluate((v) => {
                const sel = document.getElementById('bond-style-select');
                if (!sel) throw new Error('missing bond-style-select');
                sel.value = v;
                sel.dispatchEvent(new Event('change', { bubbles: true }));
            }, styleValue);
            await page.waitForTimeout(150);
        }
        // Toggle everything back off.
        for (const id of clickIds) {
            await page.evaluate((elId) => document.getElementById(elId)?.click(), id);
            await page.waitForTimeout(100);
        }
        await page.waitForTimeout(500);

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });

    test('water dimer preset arms dashed H-bond lines with real pairs', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'atoms');
        await selectAEScenario(page, 'ae-water-dimer');

        await expect.poll(
            () => page.evaluate(() => window._ftdBridge?.aeGetAtomData?.()?.count || 0),
            { timeout: 10_000, message: 'ae-water-dimer did not seed atoms' },
        ).toBe(6);

        const state = await page.evaluate(async () => {
            const ctrl = await import('./js/scales/scale2/controller.js');
            const b = window._ftdBridge;
            const hb = b.aeGetHBondPairs();
            const vel = b.aeGetVelocities();
            const dip = b.aeGetDipoles();
            return {
                btnActive: document.getElementById('toggle-ae-hbonds')?.classList.contains('active') || false,
                flagOn: ctrl.getAEVisualState().showAEHBondLines,
                pairs: hb.count,
                segmentsLen: hb.segments.length,
                velCount: vel.count,
                dipCount: dip.count,
            };
        });

        expect(state.btnActive).toBe(true);
        expect(state.flagOn).toBe(true);
        // Symmetric dimer: each donor-H sees the other molecule's O.
        expect(state.pairs).toBeGreaterThanOrEqual(1);
        expect(state.segmentsLen).toBe(state.pairs * 6);
        // The sibling getters expose full per-atom arrays.
        expect(state.velCount).toBe(6);
        expect(state.dipCount).toBe(6);

        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });
});
