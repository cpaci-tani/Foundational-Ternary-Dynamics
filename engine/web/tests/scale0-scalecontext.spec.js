// @ts-check
/** Scale Context integration and responsive-layout contract (FTD-0306). */
import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

test.describe('Scale-0 Scale Context', () => {
    test.beforeEach(async ({ page }) => {
        test.setTimeout(60_000);
        await gotoAndReady(page);
        await expect.poll(() => page.evaluate(() => !!window.__ftdScaleContextPanel),
            { timeout: 20_000 }).toBe(true);
        await page.evaluate(() => {
            document.querySelector('#tab-bar .tab[data-panel="scale-context"]')?.click();
        });
    });

    test('renders acknowledged geometry, calibration context, and honest tags', async ({ page }) => {
        const result = await page.evaluate(async () => {
            const {
                ALPHA, K_GENESIS, M_E_PHYS, HBAR_C_MEV_M, C_MS,
                FTD_ELECTRON_MASS_LADDER_K, FTD_ELECTRON_PLANCK_RATIO,
                FTD_ELECTRON_PRIMARY_PLANCK_LENGTH_M,
                FTD_ELECTRON_PRIMARY_PLANCK_TIME_S, FTD_TICK_S,
            } = await import('/js/constants.js');
            const bridgeL = Number(window._ftdBridge?.latticeSize);
            const panel = document.getElementById('scale-context-panel');
            const text = panel?.textContent || '';
            return {
                bridgeL,
                panelL: Number(panel?.dataset.latticeSize),
                text,
                hasRuler: !!panel?.querySelector('svg.sc-ruler'),
                cardCount: panel?.querySelectorAll('.sc-card').length ?? 0,
                rowCount: panel?.querySelectorAll('.sc-row').length ?? 0,
                activeMark: panel?.querySelector('.sc-size-mark.is-active')?.dataset.size,
                expectedSites: new Intl.NumberFormat('en-US').format(bridgeL ** 3),
                expectedGenesis: K_GENESIS.toFixed(3),
                expectedPair: (2 * M_E_PHYS).toFixed(3),
                lengthIdentityError: Math.abs(FTD_ELECTRON_PRIMARY_PLANCK_LENGTH_M
                    - (HBAR_C_MEV_M / M_E_PHYS) * FTD_ELECTRON_MASS_LADDER_K * Math.pow(ALPHA, 11)),
                ratioIdentityError: Math.abs(FTD_ELECTRON_PLANCK_RATIO
                    - FTD_ELECTRON_MASS_LADDER_K * Math.pow(ALPHA, 11)),
                timeIdentityError: Math.abs(FTD_ELECTRON_PRIMARY_PLANCK_TIME_S
                    - FTD_ELECTRON_PRIMARY_PLANCK_LENGTH_M / C_MS),
                tickIdentityError: Math.abs(FTD_TICK_S
                    - FTD_ELECTRON_PRIMARY_PLANCK_TIME_S / Math.sqrt(3)),
            };
        });

        expect(result.panelL).toBe(result.bridgeL);
        expect(result.activeMark).toBe(String(result.bridgeL));
        expect(result.text).toContain(result.expectedSites);
        expect(result.hasRuler).toBe(true);
        expect(result.cardCount).toBe(3);
        expect(result.rowCount).toBe(15);
        expect(result.text).toContain('[SMC · ELECTRON-PRIMARY]');
        expect(result.text).toContain('CODATA ℓP reference');
        expect(result.text).toContain('CODATA tP reference');
        expect(result.text).toContain('-0.192%');
        expect(result.text).toContain('IDENT-NULL');
        expect(result.text).toContain(result.expectedGenesis);
        expect(result.text).toContain(result.expectedPair);
        expect(result.text).toMatch(/LHC element \/ lattice[\s\S]*× longer/);
        expect(result.lengthIdentityError).toBeLessThan(1e-49);
        expect(result.ratioIdentityError).toBeLessThan(1e-36);
        expect(result.timeIdentityError).toBeLessThan(1e-58);
        expect(result.tickIdentityError).toBeLessThan(1e-58);
    });

    test('tracks a real engine-acknowledged resize without static DOM churn', async ({ page }) => {
        await page.selectOption('#lattice-size', '25');
        const readResizeState = () => page.evaluate(async () => {
            const store = await import('/js/scales/scale0/state/store.js');
            const activeBridge = store.resolveActiveScale0BridgeFromWindow();
            return {
                status: store.getScale0QualificationState().status,
                active: Number(activeBridge?.latticeSize),
                panel: document.getElementById('scale-context-panel')?.dataset.latticeSize,
            };
        });
        const accepted = { status: 'within-contract', active: 25, panel: '25' };
        await expect.poll(readResizeState, { timeout: 30_000 }).toEqual(accepted);
        await page.waitForTimeout(650);
        await expect.poll(readResizeState, { timeout: 5_000 }).toEqual(accepted);

        const result = await page.evaluate(async () => {
            const panel = document.getElementById('scale-context-panel');
            const rail = panel?.querySelector('[data-sc-ref="size-track"]');
            const ruler = panel?.querySelector('[data-sc-ref="ruler"]');
            if (!panel || !rail || !ruler) throw new Error('Scale Context structure missing');
            const immediate = {
                panelL: panel.dataset.latticeSize,
                dimension: panel.querySelector('[data-sc-ref="dimension"]')?.textContent,
                sites: panel.querySelector('[data-sc-ref="sites"]')?.textContent,
                activeMark: panel.querySelector('.sc-size-mark.is-active')?.dataset.size,
            };

            // The global tooltip observer promotes the newly-created marks'
            // native title attributes asynchronously. Let that one-time
            // accessibility annotation settle before measuring steady state.
            await new Promise((resolve) => setTimeout(resolve, 50));
            let staticMutations = 0;
            const observer = new MutationObserver((entries) => { staticMutations += entries.length; });
            observer.observe(rail, { childList: true, subtree: true, attributes: true });
            observer.observe(ruler, { childList: true, subtree: true, attributes: true });
            await new Promise((resolve) => setTimeout(resolve, 1_150));
            observer.disconnect();
            return { immediate, staticMutations };
        });

        expect(result.immediate.panelL).toBe('25');
        expect(result.immediate.dimension).toBe('25 × 25 × 25');
        expect(result.immediate.sites).toContain('15,625 sites');
        expect(result.immediate.activeMark).toBe('25');
        expect(result.staticMutations).toBe(0);
    });

    test('stays single-column when narrow and never exceeds two columns when wide', async ({ page }) => {
        const columns = await page.evaluate(async () => {
            const host = document.getElementById('panel-scale-context');
            const grid = host?.querySelector('.sc-card-grid');
            if (!host || !grid) throw new Error('Scale Context grid missing');
            host.style.width = '440px';
            await new Promise((resolve) => requestAnimationFrame(() => requestAnimationFrame(resolve)));
            const narrow = getComputedStyle(grid).gridTemplateColumns.split(' ').length;
            host.style.width = '760px';
            await new Promise((resolve) => requestAnimationFrame(() => requestAnimationFrame(resolve)));
            const wide = getComputedStyle(grid).gridTemplateColumns.split(' ').length;
            return { narrow, wide, scrollWidth: host.scrollWidth, clientWidth: host.clientWidth };
        });

        expect(columns.narrow).toBe(1);
        expect(columns.wide).toBe(2);
        expect(columns.wide).toBeLessThanOrEqual(2);
        expect(columns.scrollWidth).toBeLessThanOrEqual(columns.clientWidth + 1);
    });
});
