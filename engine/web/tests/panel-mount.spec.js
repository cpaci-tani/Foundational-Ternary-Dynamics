// @ts-check
import { test, expect } from '@playwright/test';

test('every panel descriptor exposes a unicode icon glyph', async ({ page }) => {
    await page.goto('/');
    await page.waitForTimeout(800);

    const report = await page.evaluate(async () => {
        const { PANEL_REGISTRY } = await import('/js/ui/scale-registry/panel-registry.js');
        return PANEL_REGISTRY.map((p) => ({ id: p.id, icon: p.icon }));
    });

    for (const entry of report) {
        expect(entry.icon, `panel "${entry.id}" must declare an icon`).toBeTruthy();
        expect(typeof entry.icon).toBe('string');
        expect(entry.icon.length).toBeGreaterThan(0);
    }
});

test('tab bar renders icons alongside labels', async ({ page }) => {
    await page.goto('/');
    await page.waitForTimeout(800);

    const tabs = await page.$$eval('#tab-bar .tab', (els) => els.map((el) => ({
        panel: el.dataset.panel,
        icon: el.querySelector('.tab-icon')?.textContent || '',
        label: el.querySelector('.tab-label')?.textContent || '',
    })));

    expect(tabs.length).toBeGreaterThan(0);
    for (const tab of tabs) {
        expect(tab.icon, `tab "${tab.panel}" must render an icon node`).not.toBe('');
        expect(tab.label, `tab "${tab.panel}" must render a label node`).not.toBe('');
    }
});

test('html[data-panel-mount] is set before first paint and defaults to bottom', async ({ page }) => {
    await page.goto('/');
    const mount = await page.evaluate(() => document.documentElement.dataset.panelMount);
    expect(mount).toBe('bottom');
});

test('panel-mount state module exposes read/write helpers', async ({ page }) => {
    await page.goto('/');
    const api = await page.evaluate(async () => {
        const mod = await import('/js/ui/shell/panel-mount-state.js');
        return {
            hasRead: typeof mod.readPanelMount === 'function',
            hasWrite: typeof mod.writePanelMount === 'function',
            hasValid: typeof mod.isValidMount === 'function',
            valid: mod.isValidMount('left') && mod.isValidMount('bottom') && mod.isValidMount('right'),
            invalid: !mod.isValidMount('top') && !mod.isValidMount(null),
        };
    });
    expect(api).toEqual({ hasRead: true, hasWrite: true, hasValid: true, valid: true, invalid: true });
});

test('writePanelMount persists to localStorage and updates attribute', async ({ page }) => {
    await page.goto('/');
    const after = await page.evaluate(async () => {
        const { writePanelMount } = await import('/js/ui/shell/panel-mount-state.js');
        writePanelMount('left');
        return {
            attr: document.documentElement.dataset.panelMount,
            stored: localStorage.getItem('ftd.panel.mount'),
        };
    });
    expect(after.attr).toBe('left');
    expect(after.stored).toBe('left');

    await page.evaluate(() => localStorage.removeItem('ftd.panel.mount'));
});

test('bottom-mount panel-area keeps absolute centering layout', async ({ page }) => {
    await page.goto('/');
    await page.evaluate(() => { document.documentElement.dataset.panelMount = 'bottom'; });

    const styles = await page.evaluate(() => {
        const el = document.getElementById('panel-area');
        if (!el) return null;
        const cs = window.getComputedStyle(el);
        return { position: cs.position, transform: cs.transform };
    });

    expect(styles).not.toBeNull();
    expect(styles.position).toBe('absolute');
    // translateX(-50%) resolves to a matrix; confirm it is NOT the identity
    expect(styles.transform).not.toBe('none');
    expect(styles.transform).not.toBe('matrix(1, 0, 0, 1, 0, 0)');
});

test('left-mount panel-area loses the centering transform', async ({ page }) => {
    await page.goto('/');
    await page.evaluate(() => { document.documentElement.dataset.panelMount = 'left'; });
    await page.waitForTimeout(50);

    const styles = await page.evaluate(() => {
        const el = document.getElementById('panel-area');
        if (!el) return null;
        const cs = window.getComputedStyle(el);
        return { position: cs.position, transform: cs.transform };
    });

    expect(styles).not.toBeNull();
    // Without the left-mount CSS the position falls through to the agnostic #panel-area block
    // which has no centering transform — so transform must be identity or 'none'
    const isIdentity = styles.transform === 'none' || styles.transform === 'matrix(1, 0, 0, 1, 0, 0)';
    expect(isIdentity).toBe(true);

    await page.evaluate(() => { document.documentElement.dataset.panelMount = 'bottom'; });
});
