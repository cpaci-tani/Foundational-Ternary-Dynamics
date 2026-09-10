import { test, expect } from '@playwright/test';
import { readFile } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import { gotoAndReady, switchMode } from './_helpers.js';

test('validity details support hover, keyboard, literal errors, and clean remount', async ({ page }) => {
    // Same-origin minimal fixture; no physics, WASM build, or simulation needed.
    await page.route('**/validity-fixture', route => route.fulfill({
        contentType: 'text/html',
        body: '<!doctype html><link rel="stylesheet" href="/css/ui/components/validity-status.css">'
            + '<div id="host" style="position:absolute;right:8px;top:8px"></div>',
    }));
    await page.goto('/validity-fixture');
    await page.evaluate(async () => {
        const { createValidityIndicator } = await import('/js/ui/components/validity-status.js');
        const host = document.getElementById('host');
        window.indicator = createValidityIndicator(host, { id: 'fixture-validity' });
        window.sameIndicator = createValidityIndicator(host, { id: 'fixture-validity' }) === window.indicator;
        window.indicator.set({ label: 'Undefined', severity: 'warning',
            details: 'Tick 12: log amplitude is undefined at zero.\n<img src=x onerror=alert(1)>' });
    });
    const button = page.locator('#fixture-validity');
    const details = page.locator('#fixture-validity-details');
    await expect(button).toHaveText('Undefined');
    await expect(button).toHaveAttribute('aria-label', 'Validity: Undefined');
    await button.hover();
    await expect(details).toBeVisible();
    await expect(details).toContainText('Tick 12');
    await expect(details.locator('img')).toHaveCount(0);
    await details.hover();
    await expect(details).toBeVisible();
    await page.keyboard.press('Escape');
    await expect(details).toBeHidden();
    await page.mouse.move(0, 200);
    await button.focus();
    await expect(details).toBeVisible();
    await page.evaluate(() => window.indicator.set({ label: 'Overflow', severity: 'error', details: 'Tick counter overflow.' }));
    await expect(details).toHaveText('Tick counter overflow.');
    await expect(button).toHaveText('Overflow');
    await page.keyboard.press('Escape');
    await expect(details).toBeHidden();
    await button.click();
    await expect(details).toBeVisible();
    await button.click();
    await expect(details).toBeHidden();
    const result = await page.evaluate(async () => {
        const before = document.querySelectorAll('.validity-status').length;
        let rejected = 0;
        for (const invalid of [
            { severity: 'error', details: 'missing label' },
            { label: 'Overflow', severity: 'error' },
            { label: 'Two words', details: 'not a compact label' },
        ]) {
            try { window.indicator.set(invalid); } catch { rejected++; }
        }
        let mutations = 0;
        const observer = new MutationObserver(records => { mutations += records.length; });
        observer.observe(document.body, { attributes: true, childList: true, subtree: true });
        window.indicator.set({ label: 'Overflow', severity: 'error', details: 'Tick counter overflow.' });
        await Promise.resolve();
        observer.disconnect();
        window.indicator.destroy(); window.indicator.destroy();
        const after = document.querySelectorAll('.validity-status, .validity-status-details').length;
        const { createValidityIndicator } = await import('/js/ui/components/validity-status.js');
        window.indicator = createValidityIndicator(document.getElementById('host'), { id: 'fixture-validity' });
        return { before, mutations, after, same: window.sameIndicator, rejected };
    });
    expect(result).toEqual({ before: 1, mutations: 0, after: 0, same: true, rejected: 3 });
    await expect(button).toHaveText('Unchecked');
    await page.setViewportSize({ width: 360, height: 640 });
    await button.hover();
    const box = await details.boundingBox();
    expect(box.x).toBeGreaterThanOrEqual(0);
    expect(box.x + box.width).toBeLessThanOrEqual(360);
    expect(box.y + box.height).toBeLessThanOrEqual(640);
});

test('dashboard mounts one validity indicator and clears its scope on scale switch', async ({ page }) => {
    await gotoAndReady(page, { path: '/?engine=wasm' });
    await page.waitForFunction(() => !!window.__ftdCtx?.appShell?.topbar);
    const indicator = page.locator('#toolbar .validity-status');
    await expect(indicator).toHaveCount(1);
    await expect(indicator).toBeVisible();
    await expect(indicator).toHaveText('Clear');
    await indicator.hover();
    await expect(page.locator('.validity-status-details')).toBeVisible();
    await page.screenshot({ path: '../test-results/validity-topbar.png' });
    await page.evaluate(() => window.dispatchEvent(new CustomEvent('ftd:engine-error', {
        detail: { error: 'Synthetic validity test: counter overflow' },
    })));
    await expect(indicator).toHaveText('Overflow');
    await expect(page.locator('.validity-status-details')).toContainText('Synthetic validity test: counter overflow');
    await switchMode(page, 'particles');
    await expect(indicator).toHaveText('Unchecked');
    await indicator.hover();
    await expect(page.locator('.validity-status-details')).toContainText(/scale|Scale/);
    await expect(page.locator('.validity-status-details')).toHaveCount(1);
    await switchMode(page, 'lattice');
    await expect(indicator).toHaveText('Clear');
    await page.evaluate(() => {
        const viewport = window.__ftdCtx.viewport;
        const original = viewport.setGlobalClockState;
        viewport.setGlobalClockState = function (...args) {
            this.setGlobalClockState = original;
            throw new Error('Synthetic Scale 0 frame failure');
        };
    });
    await expect(indicator).toHaveText('Runtime');
    await expect(page.locator('.validity-status-details')).toContainText('Synthetic Scale 0 frame failure');
});

test('hydro shows admission failure details and clears the issue when reset starts', async ({ page }) => {
    // Load the actual lab UI with a deliberately unavailable preparation. No
    // collision table, generated preparation, or worker execution is required.
    for (const name of ['index.html', 'hydro-lab.js', 'hydro-preparation.js', 'hydro-validity.js']) {
        await page.route(`**/strict/web/hydro/${name}`, route => route.fulfill({
            path: fileURLToPath(new URL(`../../strict/web/hydro/${name}`, import.meta.url)),
            contentType: name.endsWith('.html') ? 'text/html' : 'text/javascript',
        }));
    }
    for (const [relative, contentType] of [
        ['js/ui/components/validity-status.js', 'text/javascript'],
        ['css/ui/components/validity-status.css', 'text/css'],
    ]) {
        const body = await readFile(new URL(`../${relative}`, import.meta.url), 'utf8');
        await page.route(`**/web/${relative}`, route => route.fulfill({ contentType, body }));
    }
    await page.route('**/build_strict_hydro/lab/manifest.json', route => route.fulfill({ status: 404, body: 'Missing fixture' }));
    await page.goto('/strict/web/hydro/index.html');
    const indicator = page.locator('#hydro-validity-status');
    await expect(indicator).toHaveText('Invalid');
    await indicator.hover();
    await expect(page.locator('#hydro-validity-status-details')).toContainText('Local hydro preparations are missing');
    await expect(page.locator('#step')).toBeDisabled();
    await page.screenshot({ path: '../test-results/validity-hydro.png' });
    // Hold the next preparation fetch long enough to inspect the loading state.
    await page.route('**/build_strict_hydro/lab/manifest.json', () => {});
    await page.locator('#reset').click();
    await expect(indicator).toHaveText('Unchecked');
    await indicator.hover();
    await expect(page.locator('#hydro-validity-status-details')).not.toContainText('Local hydro preparations are missing');
});
