import { test, expect } from '@playwright/test';
import { gotoAndReady, openObserverWorkspace } from './_helpers.js';

async function ready(page) {
    await page.addInitScript(() => localStorage.setItem('ftd-gpu-card-dismissed', '1'));
    await gotoAndReady(page, { path: '/?engine=wasm&lattice=9' });
    await page.waitForFunction(() => window.__FTD_DEV__?.registry.get('assistant'));
}

test('keyboard opens JEV and floating close returns it to the dock', async ({ page }) => {
    await ready(page);
    await page.getByRole('tab', { name: 'Controls', exact: true }).focus();
    await page.keyboard.press('ArrowDown');
    await expect(page.getByRole('tab', { name: 'JEV', exact: true })).toBeFocused();
    await page.keyboard.press('Enter');
    await expect(page.getByRole('tab', { name: 'JEV', exact: true })).toHaveAttribute('aria-selected', 'true');
    await expect(page.locator('#panel-jev .jev-console')).toBeVisible();

    await page.evaluate(() => window.__ftdCtx.appShell.panelDock.floatPanel('jev', 450, 250));
    await expect(page.locator('.floating-window[data-panel-id="jev"] .jev-console')).toBeVisible();
    await page.getByRole('button', { name: 'Close JEV console' }).click();
    await expect(page.locator('.floating-window[data-panel-id="jev"]')).toHaveCount(0);
    await expect(page.getByRole('tab', { name: 'Controls', exact: true })).toHaveAttribute('aria-selected', 'true');
    await expect(page.locator('.jev-console')).toBeHidden();
});

test('floating JEV closes on Escape and cannot remain open after a mobile resize', async ({ page }) => {
    await ready(page);
    await page.getByRole('tab', { name: 'JEV', exact: true }).click();
    await page.evaluate(() => window.__ftdCtx.appShell.panelDock.floatPanel('jev', 450, 250));
    await expect(page.locator('.floating-window[data-panel-id="jev"] .jev-console')).toBeVisible();
    await page.locator('#jev-input').focus();
    await page.keyboard.press('Escape');
    await expect(page.locator('.floating-window[data-panel-id="jev"]')).toHaveCount(0);

    await page.getByRole('tab', { name: 'JEV', exact: true }).click();
    await page.evaluate(() => window.__ftdCtx.appShell.panelDock.floatPanel('jev', 450, 250));
    await page.setViewportSize({ width: 390, height: 844 });
    await expect(page.locator('.floating-window[data-panel-id="jev"]')).toHaveCount(0);
    await expect(page.locator('.jev-console')).toBeHidden();
});

test('floating an inactive JEV tab opens a visible console', async ({ page }) => {
    await ready(page);
    await expect(page.getByRole('tab', { name: 'Controls', exact: true })).toHaveAttribute('aria-selected', 'true');
    await page.evaluate(() => window.__ftdCtx.appShell.panelDock.floatPanel('jev', 450, 250));
    await expect(page.locator('.floating-window[data-panel-id="jev"] .jev-console')).toBeVisible();
    await page.getByRole('button', { name: 'Close JEV console' }).click();
    await expect(page.locator('.floating-window[data-panel-id="jev"]')).toHaveCount(0);
    await expect(page.getByRole('tab', { name: 'Controls', exact: true })).toHaveAttribute('aria-selected', 'true');
});

test('open JEV survives an Observer round trip without an empty selected tab', async ({ page }) => {
    await ready(page);
    await page.getByRole('tab', { name: 'JEV', exact: true }).click();
    await openObserverWorkspace(page);
    await expect(page.locator('#observer-workspace .jev-console')).toBeVisible();
    await page.getByRole('button', { name: /Lattice Sim/ }).click();
    await expect(page.locator('#panel-jev.active .jev-console')).toBeVisible();
});

test('closing JEV in Observer restores focus within the active workspace', async ({ page }) => {
    await ready(page);
    await page.getByRole('tab', { name: 'JEV', exact: true }).click();
    await openObserverWorkspace(page);
    await page.getByRole('button', { name: 'Close JEV console' }).click();
    expect(await page.evaluate(() => {
        const active = document.activeElement;
        return active instanceof HTMLElement && active.closest('#observer-workspace') !== null
            && !active.hidden && !active.closest('[inert]');
    })).toBe(true);
    await page.getByRole('button', { name: /Lattice Sim/ }).click();
    const state = await page.evaluate(() => ({
        selected: document.querySelector('#tab-bar .tab.active')?.dataset.panel,
        visible: document.querySelector('#panel-jev .jev-console')?.hidden === false,
    }));
    expect(state.selected !== 'jev' || state.visible).toBe(true);
});

test('Observer input remains available when the assistant listener is absent', async ({ page }) => {
    await ready(page);
    await openObserverWorkspace(page);
    const state = await page.evaluate(async () => {
        const registry = window.__FTD_DEV__.registry;
        const workspace = registry.get('observerWorkspace');
        registry.get('assistant').dispose();
        await workspace.action('assistant');
        return { assistantOpen: workspace.assistantOpen, inputBlocked: workspace.input.panelOpen };
    });
    expect(state).toEqual({ assistantOpen: false, inputBlocked: false });
});

test('BFCache pagehide retains a reconnectable MCP bridge', async ({ page }) => {
    await ready(page);
    const disposed = await page.evaluate(() => {
        const mcp = window.__FTD_DEV__.registry.get('assistant').mcp;
        window.dispatchEvent(new PageTransitionEvent('pagehide', { persisted: true }));
        window.dispatchEvent(new PageTransitionEvent('pageshow', { persisted: true }));
        return mcp.disposed;
    });
    expect(disposed).toBe(false);
});

test('cancelled Observer entry cannot publish a late active event', async ({ page }) => {
    await ready(page);
    await openObserverWorkspace(page);
    const state = await page.evaluate(async () => {
        const registry = window.__FTD_DEV__.registry;
        const host = registry.get('observerHost');
        const workspace = registry.get('observerWorkspace');
        await host.exit();
        const originalEnter = workspace.enter;
        let releaseEnter;
        const events = [];
        const onChange = event => events.push(event.detail?.workspace);
        document.addEventListener('ftd:workspace-change', onChange);
        workspace.enter = async function () {
            await new Promise(resolve => { releaseEnter = resolve; });
            return originalEnter.call(this);
        };
        try {
            const entering = host.enter();
            while (!releaseEnter) await new Promise(resolve => setTimeout(resolve, 0));
            const exiting = host.exit();
            releaseEnter();
            await Promise.all([entering, exiting]);
            return { active: host.active, workspaceActive: workspace.active, appInert: document.getElementById('app').inert, events };
        } finally {
            workspace.enter = originalEnter;
            document.removeEventListener('ftd:workspace-change', onChange);
        }
    });
    expect(state.active).toBe(false);
    expect(state.workspaceActive).toBe(false);
    expect(state.appInert).toBe(false);
    expect(state.events).not.toContain('observer');
});

test('mobile has no JEV entry and a hidden sheet can be reopened', async ({ page }) => {
    await page.setViewportSize({ width: 390, height: 844 });
    await ready(page);
    await expect(page.getByRole('tab', { name: 'JEV', exact: true, includeHidden: true })).toBeHidden();
    expect(await page.evaluate(() => {
        const option = document.querySelector('#tab-select-mobile option[value="jev"]');
        return option === null || option.disabled;
    })).toBe(true);
    await page.evaluate(() => window.__ftdCtx.appShell.activatePanel('jev'));
    await expect(page.locator('#panel-jev')).not.toHaveClass(/active/);
    await page.locator('#btn-panel-hide-mobile').click();
    await page.getByRole('button', { name: 'Show simulation panels' }).click();
    await expect(page.locator('#panel-area')).toBeInViewport();

    await page.evaluate(() => document.dispatchEvent(new CustomEvent('ftd:assistant-toggle')));
    await expect(page.locator('.jev-console')).toBeHidden();
    await openObserverWorkspace(page);
    await expect(page.locator('[data-observer-assistant]')).toBeHidden();
});
