import { test, expect } from '@playwright/test';
import { gotoAndReady } from './_helpers.js';

test('floating panels keep their header and dock control within the visual viewport', async ({ page }) => {
    await gotoAndReady(page, { path: '/?engine=wasm' });
    const result = await page.evaluate(async () => {
        const { FloatingWindow } = await import('/js/ui/components/floating-window/component.js');
        const panel = document.createElement('div');
        panel.className = 'panel active';
        panel.textContent = 'clamp fixture';
        const win = new FloatingWindow('clamp-fixture', {
            panelEl: panel,
            initialPos: { x: -300, y: -300 },
        }).init();
        win._setClampedPosition(-900, -900);
        const rect = win.el.getBoundingClientRect();
        const header = win.header.getBoundingClientRect();
        const close = win.el.querySelector('.btn-close').getBoundingClientRect();
        const viewport = window.visualViewport || { offsetLeft: 0, offsetTop: 0, width: innerWidth, height: innerHeight };
        const inset = Number.parseFloat(getComputedStyle(document.documentElement)
            .getPropertyValue('--floating-window-inset')) || 12;
        win.destroy();
        return {
            rect,
            header,
            close,
            left: viewport.offsetLeft + inset,
            top: viewport.offsetTop + inset,
            right: viewport.offsetLeft + viewport.width - inset,
            bottom: viewport.offsetTop + viewport.height - inset,
        };
    });
    expect(result.rect.left).toBeGreaterThanOrEqual(result.left - 1);
    expect(result.header.top).toBeGreaterThanOrEqual(result.top - 1);
    expect(result.close.right).toBeLessThanOrEqual(result.right + 1);
    expect(result.close.bottom).toBeLessThanOrEqual(result.bottom + 1);
});

test('dashboard shortcuts ignore editable targets and dispose cleanly', async ({ page }) => {
    await gotoAndReady(page, { path: '/?engine=wasm' });
    const result = await page.evaluate(async () => {
        const { wireKeyboard } = await import('/js/app-wire/keyboard.js');
        const draft = document.createElement('textarea');
        const button = document.createElement('button');
        const modal = document.createElement('div');
        modal.setAttribute('aria-modal', 'true');
        document.body.appendChild(draft);
        document.body.appendChild(button);
        let toggles = 0;
        let pauses = 0;
        let steps = 0;
        const dispose = wireKeyboard({
            getEngineMode: () => 'particles',
            pauseSimulation: () => { pauses++; },
            togglePlay: () => { toggles++; },
            stepScenario: () => { steps++; },
            reloadScenario: () => {},
            Scale0Controller: { handleShortcutKey: () => false },
        });
        draft.dispatchEvent(new KeyboardEvent('keydown', { key: ' ', bubbles: true, cancelable: true }));
        button.dispatchEvent(new KeyboardEvent('keydown', { key: ' ', bubbles: true, cancelable: true }));
        document.body.appendChild(modal);
        document.body.dispatchEvent(new KeyboardEvent('keydown', { key: 's', bubbles: true, cancelable: true }));
        modal.remove();
        document.body.dispatchEvent(new KeyboardEvent('keydown', { key: 's', bubbles: true, cancelable: true }));
        dispose();
        document.body.dispatchEvent(new KeyboardEvent('keydown', { key: 's', bubbles: true, cancelable: true }));
        draft.remove();
        button.remove();
        return { toggles, pauses, steps };
    });
    expect(result).toEqual({ toggles: 0, pauses: 1, steps: 1 });
});

test('keyboard help focuses its close control and restores the opener', async ({ page }) => {
    await gotoAndReady(page, { path: '/?engine=wasm' });
    const result = await page.evaluate(async () => {
        const { KeyboardHelpComponent } = await import('/js/ui/components/keyboard-help/component.js');
        const opener = document.createElement('button');
        opener.textContent = 'Open help';
        document.body.appendChild(opener);
        opener.focus();
        const help = new KeyboardHelpComponent().init();
        help.show();
        const focusedClose = document.activeElement === document.querySelector('.kbd-help-close');
        help.hide();
        const restored = document.activeElement === opener;
        help.destroy();
        opener.remove();
        return { focusedClose, restored };
    });
    expect(result).toEqual({ focusedClose: true, restored: true });
});

test('AppShell completes every teardown when individual cleanup callbacks fail', async ({ page }) => {
    await gotoAndReady(page, { path: '/?engine=wasm' });
    const result = await page.evaluate(async () => {
        const [{ AppShell }, { LifetimeScope }] = await Promise.all([
            import('/js/ui/shell/app-shell.js'),
            import('/js/ui/utils/lifetime-scope.js'),
        ]);
        const calls = [];
        const shell = new AppShell({ app: document.getElementById('app'), onDestroy: () => {
            calls.push('app');
            throw new Error('app cleanup');
        } });
        shell._scope = new LifetimeScope();
        shell._scope.defer(() => {
            calls.push('scope');
            throw new Error('scope cleanup');
        });
        for (const name of ['mobilePanel', 'panelDock', 'keyboardHelp', 'tooltips',
            'viewportOverlays', 'workspaceTabs', 'panelDockView', 'viewportFrame', 'topbar', 'loadingOverlay']) {
            shell[name] = { destroy() { calls.push(name); } };
        }
        shell.tooltips = { destroy() { calls.push('tooltips'); throw new Error('tooltip cleanup'); } };
        shell.breakpoints = { stop() { calls.push('breakpoints'); } };
        let errors = 0;
        try { shell.destroy(); } catch (error) { errors = error.errors?.length ?? 1; }
        return { calls, errors, scopeDisposed: shell._scope.disposed, onDestroyCleared: shell.onDestroy === null };
    });
    expect(result.calls).toEqual(expect.arrayContaining([
        'app', 'mobilePanel', 'panelDock', 'keyboardHelp', 'tooltips',
        'viewportOverlays', 'workspaceTabs', 'panelDockView', 'viewportFrame', 'topbar',
        'loadingOverlay', 'breakpoints', 'scope',
    ]));
    expect(result).toMatchObject({ errors: 3, scopeDisposed: true, onDestroyCleared: true });
});
