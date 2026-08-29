// @ts-check
import { test, expect } from '@playwright/test';
import {
    attachConsoleWatcher,
    gotoAndReady,
    realErrors,
    selectScale0Scenario,
} from './_helpers.js';

test.describe('Scale 0 contextual Standard Model overlay', () => {
    test.beforeEach(async ({ page }, testInfo) => {
        testInfo.setTimeout(120_000);
        page.setDefaultTimeout(60_000);
        await gotoAndReady(page);
        await page.waitForFunction(
            () => document.getElementById('viewport-overlay')?.dataset.scenarioId === 'flux-pulse',
        );
    });

    test('covers every elementary-particle scenario with catalog reference data', async ({ page }) => {
        const result = await page.evaluate(async () => {
            const {
                SCALE0_SM_PARTICLE_SCENARIOS,
                getScale0StandardModelContext,
            } = await import('/js/scales/scale0/ui/overlays/standard-model.js?v=2');
            const entries = Object.entries(SCALE0_SM_PARTICLE_SCENARIOS);
            return {
                count: entries.length,
                missing: entries
                    .filter(([scenarioId]) => !getScale0StandardModelContext(scenarioId))
                    .map(([scenarioId]) => scenarioId),
                groups: {
                    quarks: entries.filter(([id]) => id.includes('quark')).length,
                    neutrinos: entries.filter(([id]) => id.includes('neutrino')).length,
                    bosons: entries.filter(([id]) => id.includes('boson') || id.includes('photon') || id.includes('gluon') || id.includes('higgs')).length,
                },
            };
        });
        expect(result).toEqual({
            count: 30,
            missing: [],
            groups: { quarks: 12, neutrinos: 6, bosons: 6 },
        });
    });

    test('is absent from generic fields and exposes an honest reference HUD on particle templates', async ({ page }) => {
        const errors = attachConsoleWatcher(page);
        const generic = await page.evaluate(() => {
            const col = document.querySelector('[data-col="standard-model"]');
            return {
                display: getComputedStyle(col).display,
                domain: document.getElementById('viewport-overlay')?.dataset.overlayDomains || '',
                hudHidden: document.getElementById('s0-sm-reference-hud')?.hidden,
            };
        });
        expect(generic.display).toBe('none');
        expect(generic.domain.split(/\s+/)).not.toContain('standardModel');
        expect(generic.hudHidden).toBe(true);

        await selectScale0Scenario(page, 's0-vacuum-electron');
        await page.waitForFunction(
            () => document.getElementById('viewport-overlay')?.dataset.scenarioId === 's0-vacuum-electron',
        );
        const electron = await page.evaluate(() => {
            const col = document.querySelector('[data-col="standard-model"]');
            const button = document.getElementById('toggle-sm-reference');
            const card = document.getElementById('s0-sm-context-card');
            return {
                display: getComputedStyle(col).display,
                applicable: !button.classList.contains('is-inapplicable'),
                name: card.querySelector('[data-sm-field="name"]').textContent,
                spin: card.querySelector('[data-sm-field="spin"]').textContent,
                charge: card.querySelector('[data-sm-field="charge"]').textContent,
                chirality: card.querySelector('[data-sm-field="chirality"]').textContent,
                statusDetail: card.title,
            };
        });
        expect(electron.display).not.toBe('none');
        expect(electron.applicable).toBe(true);
        expect(electron.name).toBe('Electron');
        expect(electron.spin).toBe('½');
        expect(electron.charge).toBe('-1');
        expect(electron.chirality).toBe('L / R fields');
        expect(electron.statusDetail).toContain('[CLOSED NEGATIVE]');

        await page.locator('[data-col="standard-model"] .s0-overlay-col-head').click();
        await page.locator('#toggle-sm-reference').click();
        const hud = page.locator('#s0-sm-reference-hud');
        await expect(hud).toBeVisible();
        await expect(hud.locator('[data-sm-field="name"]')).toHaveText('Electron');
        await expect(hud.locator('.s0-sm-hud-caveat')).toContainText('not Scale 0 measurements');

        await selectScale0Scenario(page, 'flux-pulse');
        await expect(hud).toBeHidden();
        expect(realErrors(errors)).toEqual([]);
    });

    test('keeps chirality honest across fermions, neutrinos, and bosons', async ({ page }) => {
        const values = await page.evaluate(async () => {
            const { getScale0StandardModelContext } = await import('/js/scales/scale0/ui/overlays/standard-model.js?v=2');
            const get = (id) => getScale0StandardModelContext(id);
            return {
                electron: get('s0-vacuum-electron').chirality,
                neutrino: get('s0-vacuum-electron-neutrino').chirality,
                antineutrino: get('s0-vacuum-electron-antineutrino').chirality,
                photon: get('s0-vacuum-photon').chirality,
                higgs: get('s0-vacuum-higgs').chirality,
            };
        });
        expect(values).toEqual({
            electron: 'L / R fields',
            neutrino: 'L weak sector',
            antineutrino: 'R weak sector',
            photon: 'N/A · helicity ±1',
            higgs: 'N/A · scalar',
        });
    });
});
