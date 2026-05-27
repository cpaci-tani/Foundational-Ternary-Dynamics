// @ts-check
import { test, expect } from '@playwright/test';

test.beforeEach(async ({ page }) => {
    await page.setViewportSize({ width: 1280, height: 720 });
});

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

test('left mount docks the panel to the left edge with viewport-safe height', async ({ page }) => {
    await page.goto('/');
    await page.waitForTimeout(600);

    const box = await page.evaluate(async () => {
        const { writePanelMount } = await import('/js/ui/shell/panel-mount-state.js');
        writePanelMount('left');
        await new Promise((r) => requestAnimationFrame(r));
        const el = document.getElementById('panel-area');
        const rect = el.getBoundingClientRect();
        writePanelMount('bottom');
        return {
            left: rect.left,
            width: rect.width,
            top: rect.top,
            height: rect.height,
            innerWidth: window.innerWidth,
            innerHeight: window.innerHeight,
        };
    });

    expect(box.left).toBeLessThan(40);
    expect(box.width).toBeGreaterThanOrEqual(320);
    expect(box.width).toBeLessThanOrEqual(box.innerWidth * 0.5);
    expect(box.height).toBeGreaterThan(box.innerHeight * 0.5);
});

test('right mount docks the panel to the right edge', async ({ page }) => {
    await page.goto('/');
    await page.waitForTimeout(600);

    const box = await page.evaluate(async () => {
        const { writePanelMount } = await import('/js/ui/shell/panel-mount-state.js');
        writePanelMount('right');
        await new Promise((r) => requestAnimationFrame(r));
        const el = document.getElementById('panel-area');
        const rect = el.getBoundingClientRect();
        writePanelMount('bottom');
        return {
            right: window.innerWidth - rect.right,
            width: rect.width,
            innerWidth: window.innerWidth,
        };
    });

    expect(box.right).toBeLessThan(40);
    expect(box.width).toBeGreaterThanOrEqual(320);
});

test('viewport stays full-bleed in every mount state', async ({ page }) => {
    await page.goto('/');
    await page.waitForTimeout(600);

    const rects = await page.evaluate(async () => {
        const { writePanelMount } = await import('/js/ui/shell/panel-mount-state.js');
        const out = {};
        for (const mount of ['bottom', 'left', 'right']) {
            writePanelMount(mount);
            await new Promise((r) => requestAnimationFrame(r));
            const r = document.getElementById('viewport').getBoundingClientRect();
            out[mount] = { w: r.width, h: r.height };
        }
        writePanelMount('bottom');
        return out;
    });

    const tol = 4;
    expect(Math.abs(rects.bottom.w - rects.left.w)).toBeLessThan(tol);
    expect(Math.abs(rects.bottom.w - rects.right.w)).toBeLessThan(tol);
    expect(Math.abs(rects.bottom.h - rects.left.h)).toBeLessThan(tol);
});

test('mount toggle renders three buttons with aria-pressed reflecting current mount', async ({ page }) => {
    await page.goto('/');
    await page.waitForTimeout(800);

    const snapshot = await page.evaluate(() => {
        const buttons = Array.from(document.querySelectorAll('[data-panel-mount-toggle] button'));
        return buttons.map((b) => ({
            value: b.dataset.mount,
            pressed: b.getAttribute('aria-pressed'),
        }));
    });

    expect(snapshot.map((b) => b.value)).toEqual(['left', 'bottom', 'right']);
    expect(snapshot.find((b) => b.value === 'bottom').pressed).toBe('true');
    expect(snapshot.find((b) => b.value === 'left').pressed).toBe('false');
    expect(snapshot.find((b) => b.value === 'right').pressed).toBe('false');
});

test('clicking a toggle button switches the mount and persists it', async ({ page }) => {
    await page.goto('/');
    await page.waitForTimeout(800);

    // Click via DOM (bypasses any overlay that may still be fading out)
    await page.evaluate(() =>
        document.querySelector('[data-panel-mount-toggle] button[data-mount="right"]').click()
    );
    const afterClick = await page.evaluate(() => ({
        attr: document.documentElement.dataset.panelMount,
        stored: localStorage.getItem('ftd.panel.mount'),
    }));
    expect(afterClick).toEqual({ attr: 'right', stored: 'right' });

    await page.evaluate(() =>
        document.querySelector('[data-panel-mount-toggle] button[data-mount="bottom"]').click()
    );
    await page.evaluate(() => localStorage.removeItem('ftd.panel.mount'));
});

test('keyboard shortcuts change the mount', async ({ page }) => {
    await page.goto('/');
    await page.waitForTimeout(800);

    await page.keyboard.press('Control+Shift+ArrowLeft');
    expect(await page.evaluate(() => document.documentElement.dataset.panelMount)).toBe('left');

    await page.keyboard.press('Control+Shift+ArrowRight');
    expect(await page.evaluate(() => document.documentElement.dataset.panelMount)).toBe('right');

    await page.keyboard.press('Control+Shift+ArrowDown');
    expect(await page.evaluate(() => document.documentElement.dataset.panelMount)).toBe('bottom');

    await page.evaluate(() => localStorage.removeItem('ftd.panel.mount'));
});

test('side-mount collapse hides panel-area but keeps tab rail visible', async ({ page }) => {
    await page.goto('/');
    await page.waitForTimeout(800);

    const result = await page.evaluate(async () => {
        const { writePanelMount } = await import('/js/ui/shell/panel-mount-state.js');
        writePanelMount('left');
        await new Promise((r) => requestAnimationFrame(r));

        // Collapse via the toggle button
        document.getElementById('btn-panel-toggle')?.click();
        await new Promise((r) => requestAnimationFrame(r));

        const panelDisplay = getComputedStyle(document.getElementById('panel-area')).display;
        const tabDisplay   = getComputedStyle(document.getElementById('tab-bar')).display;

        writePanelMount('bottom');
        return { panelDisplay, tabDisplay };
    });

    expect(result.panelDisplay).toBe('none');
    expect(result.tabDisplay).not.toBe('none');

    await page.evaluate(() => localStorage.removeItem('ftd.panel.mount'));
});

test('bottom mount sets both safe-edge vars to 0px', async ({ page }) => {
    await page.goto('/');
    await page.waitForTimeout(800);

    const edges = await page.evaluate(async () => {
        const { writePanelMount } = await import('/js/ui/shell/panel-mount-state.js');
        writePanelMount('bottom');
        await new Promise((r) => requestAnimationFrame(r));
        const cs = getComputedStyle(document.documentElement);
        return {
            left:  cs.getPropertyValue('--viewport-safe-left').trim(),
            right: cs.getPropertyValue('--viewport-safe-right').trim(),
        };
    });

    expect(edges.left).toBe('0px');
    expect(edges.right).toBe('0px');

    await page.evaluate(() => localStorage.removeItem('ftd.panel.mount'));
});

test('left mount sets a positive viewport-safe-left and zero right', async ({ page }) => {
    await page.goto('/');
    await page.waitForTimeout(800);

    const edges = await page.evaluate(async () => {
        const { writePanelMount } = await import('/js/ui/shell/panel-mount-state.js');
        writePanelMount('left');
        await new Promise((r) => requestAnimationFrame(r));
        const cs = getComputedStyle(document.documentElement);
        return {
            left:  cs.getPropertyValue('--viewport-safe-left').trim(),
            right: cs.getPropertyValue('--viewport-safe-right').trim(),
        };
    });

    expect(parseFloat(edges.left)).toBeGreaterThan(0);
    expect(edges.right).toBe('0px');

    await page.evaluate(() => localStorage.removeItem('ftd.panel.mount'));
});

test('right mount sets a positive viewport-safe-right and zero left', async ({ page }) => {
    await page.goto('/');
    await page.waitForTimeout(800);

    const edges = await page.evaluate(async () => {
        const { writePanelMount } = await import('/js/ui/shell/panel-mount-state.js');
        writePanelMount('right');
        await new Promise((r) => requestAnimationFrame(r));
        const cs = getComputedStyle(document.documentElement);
        return {
            left:  cs.getPropertyValue('--viewport-safe-left').trim(),
            right: cs.getPropertyValue('--viewport-safe-right').trim(),
        };
    });

    expect(edges.left).toBe('0px');
    expect(parseFloat(edges.right)).toBeGreaterThan(0);

    await page.evaluate(() => localStorage.removeItem('ftd.panel.mount'));
});

test('narrow viewport (<900px) snaps side mount to bottom without clearing localStorage', async ({ page }) => {
    await page.setViewportSize({ width: 800, height: 700 });
    await page.goto('/');
    await page.waitForTimeout(800);

    const result = await page.evaluate(async () => {
        const { writePanelMount } = await import('/js/ui/shell/panel-mount-state.js');
        writePanelMount('left');
        await new Promise((r) => requestAnimationFrame(r));
        window.dispatchEvent(new Event('resize'));
        await new Promise((r) => requestAnimationFrame(r));
        return {
            attr:   document.documentElement.dataset.panelMount,
            stored: localStorage.getItem('ftd.panel.mount'),
        };
    });

    // Attribute must snap to bottom (too narrow for side mount)
    expect(result.attr).toBe('bottom');
    // localStorage preference is preserved so it restores when resized back
    expect(result.stored).toBe('left');

    await page.evaluate(() => localStorage.removeItem('ftd.panel.mount'));
});

test('narrow viewport disables side-mount buttons with aria-disabled', async ({ page }) => {
    await page.setViewportSize({ width: 800, height: 700 });
    await page.goto('/');
    await page.waitForTimeout(800);

    const buttons = await page.evaluate(() => {
        const btns = Array.from(document.querySelectorAll('[data-panel-mount-toggle] button'));
        return btns.map((b) => ({
            mount:    b.dataset.mount,
            disabled: b.getAttribute('aria-disabled'),
        }));
    });

    const left  = buttons.find((b) => b.mount === 'left');
    const right = buttons.find((b) => b.mount === 'right');
    const bottom = buttons.find((b) => b.mount === 'bottom');

    expect(left.disabled).toBe('true');
    expect(right.disabled).toBe('true');
    expect(bottom.disabled).toBeNull();
});

test('wide viewport (>=900px) re-enables side-mount buttons', async ({ page }) => {
    await page.setViewportSize({ width: 800, height: 700 });
    await page.goto('/');
    await page.waitForTimeout(800);

    await page.setViewportSize({ width: 1280, height: 800 });
    // trigger resize event so the component re-evaluates
    await page.evaluate(() => window.dispatchEvent(new Event('resize')));
    await page.waitForTimeout(100);

    const buttons = await page.evaluate(() => {
        const btns = Array.from(document.querySelectorAll('[data-panel-mount-toggle] button'));
        return btns.map((b) => ({
            mount:    b.dataset.mount,
            disabled: b.getAttribute('aria-disabled'),
        }));
    });

    for (const btn of buttons) {
        expect(btn.disabled, `${btn.mount} button should not be aria-disabled at 1280px`).toBeNull();
    }
});
