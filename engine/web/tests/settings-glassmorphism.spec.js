// @ts-check
import { test, expect } from '@playwright/test';
import { readdirSync, readFileSync, statSync } from 'node:fs';
import { join } from 'node:path';
import { fileURLToPath } from 'node:url';

const cssRoot = fileURLToPath(new URL('../css/', import.meta.url));

async function waitForShell(page) {
    await expect.poll(
        () => page.evaluate(() => document.getElementById('app')?.dataset.shellReady === 'true'),
        { timeout: 20_000 },
    ).toBe(true);
}

function walkCss(directory) {
    return readdirSync(directory).flatMap((name) => {
        const fullPath = join(directory, name);
        return statSync(fullPath).isDirectory()
            ? walkCss(fullPath)
            : (name.endsWith('.css') ? [fullPath] : []);
    });
}

test('glassmorphism is off by default and does not allocate backdrop blur', async ({ page }) => {
    await page.goto('/index.html');
    await waitForShell(page);

    const state = await page.evaluate(() => {
        const root = document.documentElement;
        const toggle = document.getElementById('settings-glass-enabled');
        const slider = document.getElementById('settings-glass-thickness');
        const panel = document.getElementById('panel-area');
        const panelBackground = panel ? getComputedStyle(panel).backgroundColor : '';
        return {
            mode: root.dataset.glass,
            checked: toggle instanceof HTMLInputElement ? toggle.checked : null,
            ariaChecked: toggle?.getAttribute('aria-checked'),
            sliderDisabled: slider instanceof HTMLInputElement ? slider.disabled : null,
            thickness: slider instanceof HTMLInputElement ? slider.value : null,
            output: document.getElementById('settings-glass-thickness-val')?.textContent?.trim(),
            highFilterToken: getComputedStyle(root).getPropertyValue('--glass-filter-high').trim(),
            panelFilter: panel ? getComputedStyle(panel).backdropFilter : null,
            panelBackground,
            panelOpaque: !panelBackground.startsWith('rgba('),
        };
    });

    expect(state).toEqual({
        mode: 'off',
        checked: false,
        ariaChecked: 'false',
        sliderDisabled: true,
        thickness: '16',
        output: '16 px',
        highFilterToken: 'none',
        panelFilter: 'none',
        panelBackground: expect.any(String),
        panelOpaque: true,
    });
});

test('glass thickness is frame-coalesced, persistent, and resettable', async ({ page }) => {
    await page.goto('/index.html');
    await waitForShell(page);
    await page.click('#btn-settings');
    await page.locator('#settings-glass-enabled').check();

    const burst = await page.evaluate(async () => {
        const slider = document.getElementById('settings-glass-thickness');
        if (!(slider instanceof HTMLInputElement)) throw new Error('glass thickness slider missing');

        let storageWrites = 0;
        const originalSetItem = Storage.prototype.setItem;
        Storage.prototype.setItem = function (key, value) {
            if (key === 'ftd-glass-thickness') storageWrites += 1;
            return originalSetItem.call(this, key, value);
        };

        try {
            for (let i = 0; i < 200; i += 1) {
                slider.value = String(4 + (i % 29));
                slider.dispatchEvent(new Event('input', { bubbles: true }));
            }
            slider.value = '24';
            slider.dispatchEvent(new Event('input', { bubbles: true }));
            await new Promise((resolve) => requestAnimationFrame(() => requestAnimationFrame(resolve)));

            const root = document.documentElement;
            const panel = document.getElementById('panel-area');
            const panelBackground = panel ? getComputedStyle(panel).backgroundColor : '';
            return {
                storageWrites,
                mode: root.dataset.glass,
                low: root.style.getPropertyValue('--glass-blur-low'),
                mid: root.style.getPropertyValue('--glass-blur-mid'),
                high: root.style.getPropertyValue('--glass-blur-high'),
                output: document.getElementById('settings-glass-thickness-val')?.textContent?.trim(),
                panelFilter: panel ? getComputedStyle(panel).backdropFilter : '',
                panelBackground,
                panelOpaque: !panelBackground.startsWith('rgba('),
            };
        } finally {
            Storage.prototype.setItem = originalSetItem;
        }
    });

    expect(burst.storageWrites).toBe(1);
    expect(burst.mode).toBe('on');
    expect(burst.low).toBe('12px');
    expect(burst.mid).toBe('24px');
    expect(burst.high).toBe('36px');
    expect(burst.output).toBe('24 px');
    expect(burst.panelFilter).toContain('blur(36px)');
    expect(burst.panelBackground).toContain('rgba(');
    expect(burst.panelOpaque).toBe(false);

    const immediateCommit = await page.evaluate(() => {
        const slider = document.getElementById('settings-glass-thickness');
        if (!(slider instanceof HTMLInputElement)) throw new Error('glass thickness slider missing');
        slider.value = '26';
        slider.dispatchEvent(new Event('input', { bubbles: true }));
        slider.dispatchEvent(new Event('change', { bubbles: true }));
        return localStorage.getItem('ftd-glass-thickness');
    });
    expect(immediateCommit).toBe('26');

    await page.reload();
    await waitForShell(page);
    await expect(page.locator('#settings-glass-enabled')).toBeChecked();
    await expect(page.locator('#settings-glass-thickness')).toBeEnabled();
    await expect(page.locator('#settings-glass-thickness')).toHaveValue('26');
    await expect(page.locator('#settings-glass-thickness-val')).toHaveText('26 px');
    await expect.poll(() => page.evaluate(() => document.documentElement.dataset.glass)).toBe('on');

    await page.click('#btn-settings');
    await page.click('#settings-reset');
    await expect(page.locator('#settings-glass-enabled')).not.toBeChecked();
    await expect(page.locator('#settings-glass-thickness')).toBeDisabled();
    await expect(page.locator('#settings-glass-thickness')).toHaveValue('16');
    await expect(page.locator('#settings-glass-thickness-val')).toHaveText('16 px');

    const reset = await page.evaluate(() => ({
        mode: document.documentElement.dataset.glass,
        glass: localStorage.getItem('ftd-glassmorphism'),
        thickness: localStorage.getItem('ftd-glass-thickness'),
        panelFilter: getComputedStyle(document.getElementById('panel-area')).backdropFilter,
        panelBackground: getComputedStyle(document.getElementById('panel-area')).backgroundColor,
    }));
    expect(reset).toEqual({
        mode: 'off',
        glass: 'off',
        thickness: '16',
        panelFilter: 'none',
        panelBackground: expect.any(String),
    });
    expect(reset.panelBackground).not.toContain('rgba(');
});

test('every authored backdrop blur is governed by the shared glass tokens', () => {
    const offenders = walkCss(cssRoot).filter((file) => {
        const css = readFileSync(file, 'utf8');
        return /(?:-webkit-)?backdrop-filter\s*:\s*blur\(/i.test(css);
    });

    expect(offenders, 'Hardcoded backdrop blur bypasses the glass setting').toEqual([]);
});
