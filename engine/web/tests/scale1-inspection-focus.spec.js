// @ts-check
import { test, expect } from '@playwright/test';
import { gotoAndReady, switchMode, attachConsoleWatcher, realErrors } from './_helpers.js';

async function selectScenario(page, id) {
    await page.evaluate((scenarioId) => {
        const select = document.getElementById('pe-scenario-select');
        if (!select) throw new Error('Scale 1 scenario selector missing');
        select.value = scenarioId;
        select.dispatchEvent(new Event('change', { bubbles: true }));
    }, id);
}

async function openHierarchy(page) {
    await page.evaluate(() => document
        .querySelector('#tab-bar .tab[data-panel="interaction-hierarchy"]')?.click());
    await expect(page.locator('#panel-interaction-hierarchy')).toHaveClass(/active/);
    await expect(page.locator('[data-inspect-particle]').first()).toBeVisible();
}

async function enableOverlay(page, id) {
    const button = page.locator(`#${id}`);
    if (await button.getAttribute('aria-pressed') !== 'true') await button.click();
    await expect(button).toHaveAttribute('aria-pressed', 'true');
}

test.describe('Scale 1 focused inspection', () => {
    test('particle and cluster selections isolate overlays without changing overlay settings', async ({ page }) => {
        page.setDefaultTimeout(20_000);
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page);
        await switchMode(page, 'particles');
        await selectScenario(page, 's1-mass-ladder');
        await expect.poll(() => page.evaluate(() =>
            window._ftdBridge?.peGetParticleData?.()?.count || 0)).toBeGreaterThan(2);

        for (const id of [
            'toggle-velocities',
            'toggle-trails',
            'toggle-pe-potential',
            'toggle-pe-efield',
            'toggle-pe-gravity-field',
            'toggle-pe-force-net',
            'toggle-pe-system',
        ]) await enableOverlay(page, id);

        await openHierarchy(page);
        const particleButton = page.locator('[data-inspect-particle]').first();
        const particleId = Number(await particleButton.getAttribute('data-inspect-particle'));
        await particleButton.click();

        await expect(page.locator('#insp-selection-summary')).toContainText(`Focused particle #${particleId}`);
        await expect(page.locator('#pe-insp-focus-kind')).toHaveText('Particle focus');
        await expect(page.locator(`.particle-log-node[data-particle-id="${particleId}"]`)).toHaveClass(/is-inspected/);

        await expect.poll(() => page.evaluate(() => {
            const viewport = window.__ftdCtx?.viewport;
            const renderer = viewport?._particleRenderer;
            return {
                kind: viewport?.getPEInspectionFocus?.()?.kind,
                focusCount: renderer?._peInspectionIds?.size || 0,
                velocityVertices: renderer?.velocityVectors?.geometry?.drawRange?.count || 0,
                forceVertices: renderer?._peForceNet?.geometry?.drawRange?.count || 0,
            };
        })).toMatchObject({
            kind: 'particle',
            focusCount: 1,
            velocityVertices: 2,
            forceVertices: 2,
        });

        for (const id of [
            'toggle-velocities', 'toggle-trails', 'toggle-pe-potential',
            'toggle-pe-efield', 'toggle-pe-gravity-field', 'toggle-pe-force-net',
            'toggle-pe-system',
        ]) await expect(page.locator(`#${id}`)).toHaveAttribute('aria-pressed', 'true');

        await page.locator('#interaction-hierarchy-clear-focus').click();
        await expect(page.locator('#insp-selection-summary')).toContainText('Select a particle');
        await expect.poll(() => page.evaluate(() => ({
            focus: window.__ftdCtx?.viewport?.getPEInspectionFocus?.() || null,
            velocityVertices: window.__ftdCtx?.viewport?._particleRenderer
                ?.velocityVectors?.geometry?.drawRange?.count || 0,
        }))).toMatchObject({ focus: null });
        await expect.poll(() => page.evaluate(() => window.__ftdCtx?.viewport?._particleRenderer
            ?.velocityVectors?.geometry?.drawRange?.count || 0)).toBeGreaterThan(2);

        const clusterSummary = page.locator('summary[data-inspect-cluster]').first();
        const clusterKey = await clusterSummary.getAttribute('data-inspect-cluster');
        await clusterSummary.click();
        await expect(page.locator('#pe-insp-focus-kind')).toHaveText('Dynamic cluster focus');
        await page.evaluate(() => document
            .querySelector('#tab-bar .tab[data-panel="inspector"]')?.click());
        await expect(page.locator('#panel-inspector')).toHaveClass(/active/);
        await expect(page.locator('#pe-insp-cluster-fields')).toBeVisible();
        await expect(page.locator(
            '#panel-inspector :is(#inspector-empty, #ae-inspector-empty, #planetary-inspector-empty, #cosmic-inspector-empty):visible',
        )).toHaveCount(0);
        await expect(page.locator(`.particle-log-cluster[data-cluster-key="${clusterKey}"]`)).toHaveClass(/is-inspected/);
        const clusterFocus = await page.evaluate(() => {
            const focus = window.__ftdCtx?.viewport?.getPEInspectionFocus?.();
            return focus ? { kind: focus.kind, members: focus.particleIds.length } : null;
        });
        expect(clusterFocus?.kind).toBe('cluster');
        expect(clusterFocus?.members).toBeGreaterThan(0);

        await selectScenario(page, 's1-cluster-pair');
        await expect.poll(() => page.evaluate(() =>
            window.__ftdCtx?.viewport?.getPEInspectionFocus?.() || null)).toBeNull();
        await expect.poll(() => page.evaluate(() => ({
            empty: document.getElementById('pe-inspector-empty')?.style.display,
            content: document.getElementById('pe-inspector-content')?.style.display,
        }))).toEqual({ empty: 'block', content: 'none' });
        expect(realErrors(errors), `console errors:\n${realErrors(errors).join('\n')}`).toHaveLength(0);
    });
});
