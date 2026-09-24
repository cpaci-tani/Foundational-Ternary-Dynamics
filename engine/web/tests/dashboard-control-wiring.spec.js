import { test, expect } from '@playwright/test';
import { bootDashboard, openDockPanel, switchMode } from './_helpers.js';

async function activeScale0Tick(page) {
    return page.evaluate(async () => {
        const { getActiveScale0Bridge, getActiveScale0Capability } = await import('/js/scales/scale0/state/store.js');
        const owner = getActiveScale0Bridge(window.__ftdCtx);
        return owner?.currentTick?.()
            ?? owner?.getDiagnostics?.()?.tick
            ?? getActiveScale0Capability(window.__ftdCtx)?.getScale0Diagnostics?.()?.tick
            ?? null;
    });
}

/** Read the runtime which the public mode controls currently address. */
async function activeModeRuntime(page, mode) {
    return page.evaluate(async (currentMode) => {
        const bridge = window._ftdBridge;
        if (currentMode === 'particles') {
            const { scale1State } = await import('/js/scales/scale1/state/store.js?v=7');
            return {
                scenario: scale1State.currentScenarioId,
                tick: bridge.peGetTick(),
                count: bridge.peGetParticleData().count,
            };
        }
        if (currentMode === 'atoms' || currentMode === 'molecules') {
            const diagnostics = bridge.aeGetDiagnostics();
            const { getAEExperimentState } = await import('/js/scales/scale2/experiment-runtime.js');
            return {
                scenario: currentMode === 'molecules'
                    ? bridge.aeGetMoleculeDiagnostics()?.referenceLabel
                    : getAEExperimentState()?.id ?? null,
                tick: diagnostics.tick,
                count: diagnostics.atomCount,
            };
        }
        const activeBridge = window.__ftdCtx?.inspector?.bridge;
        const diagnostics = activeBridge?.getDiagnostics?.() ?? null;
        return {
            scenario: diagnostics?.scenario ?? activeBridge?._scenarioName ?? null,
            tick: diagnostics?.tick ?? null,
            count: diagnostics?.bodyCount ?? 0,
        };
    }, mode);
}

test.describe('dashboard public control wiring', () => {
    test('transport, speed, reset, and public scenario selectors drive their active mode', async ({ page }) => {
        await bootDashboard(page, { engine: 'wasm' });
        await page.waitForFunction(() => document.getElementById('app')?.dataset.shellReady === 'true');
        await page.waitForFunction(async () => {
            const { getActiveScale0Bridge, getActiveScale0Capability } = await import('/js/scales/scale0/state/store.js');
            const owner = getActiveScale0Bridge(window.__ftdCtx);
            return Number.isFinite(owner?.currentTick?.()
                ?? owner?.getDiagnostics?.()?.tick
                ?? getActiveScale0Capability(window.__ftdCtx)?.getScale0Diagnostics?.()?.tick);
        });
        const lattice = { before: await activeScale0Tick(page), scenario: await page.locator('#scenario-select').inputValue() };
        await page.locator('#btn-step').click();
        await expect.poll(() => activeScale0Tick(page)).toBeGreaterThan(lattice.before);
        const speed = page.locator('#ticks-per-frame');
        const beforeSpeed = Number(await speed.inputValue());
        await page.locator('[data-speed-nudge="5"]').click();
        expect(Number(await speed.inputValue())).toBeGreaterThan(beforeSpeed);
        await page.locator('#btn-play').click();
        await expect(page.locator('#btn-play')).toHaveAttribute('data-paused', 'false');
        await page.locator('#btn-reset').click();
        await expect(page.locator('#scenario-select')).toHaveValue(lattice.scenario);

        const modes = [
            ['particles', 'pe-scenario-select'], ['atoms', 'ae-scenario-select'],
            ['molecules', 'mol-scenario-select'], ['planetary', 'planetary-scenario-select'],
            ['cosmic', 'cosmic-scenario-select'],
        ];
        for (const [mode, selector] of modes) {
            await switchMode(page, mode);
            await expect(page.locator(`#${selector}`)).toBeAttached();
            const selected = await page.locator(`#${selector}`).inputValue();
            // The atom runtime does not retain every static preset's id. Pin a
            // six-atom fixture so a disconnected selector cannot pass by reading
            // the previously mounted one-atom hydrogen reference.
            const next = mode === 'atoms' ? 'ae-he-cluster'
                : await page.locator(`#${selector} option`).evaluateAll((options, current) =>
                    options.map(option => option.value).find(value => value && value !== current), selected);
            const expectedRuntime = mode === 'atoms' ? { count: 6 } : { scenario: next };
            expect(next, `${mode} exposes a second public scenario`).toBeTruthy();
            await page.selectOption(`#${selector}`, next);
            await expect(page.locator(`#${selector}`)).toHaveValue(next);
            await expect.poll(() => activeModeRuntime(page, mode), {
                message: `${mode} scenario selection did not reach its active runtime`,
            }).toMatchObject(expectedRuntime);
            const loaded = await activeModeRuntime(page, mode);
            expect(loaded.count, `${mode} scenario creates live runtime data`).toBeGreaterThan(0);
            await page.locator('#btn-step').click();
            await expect.poll(() => activeModeRuntime(page, mode).then(runtime => runtime.tick), {
                message: `${mode} Step did not advance its active runtime`,
            }).toBeGreaterThan(loaded.tick);
            const stepped = await activeModeRuntime(page, mode);
            await page.locator('#btn-reset').click();
            await expect(page.locator(`#${selector}`)).toHaveValue(next);
            await expect.poll(() => activeModeRuntime(page, mode), {
                message: `${mode} Reset did not restore the selected runtime scenario`,
            }).toMatchObject(expectedRuntime);
            const reset = await activeModeRuntime(page, mode);
            expect(reset.count, `${mode} Reset recreates runtime data`).toBeGreaterThan(0);
            expect(reset.tick, `${mode} Reset reseeds rather than retaining the stepped tick`).toBeLessThan(stepped.tick);
        }
    });

    test('particle controls mutate native toggle and integration state through their public inputs', async ({ page }) => {
        await bootDashboard(page, { engine: 'wasm', mode: 'particles' });
        await page.selectOption('#pe-scenario-select', 's1-quantum-exchange-eligible');
        await openDockPanel(page, 'controls');
        const result = await page.evaluate(() => {
            const bridge = window._ftdBridge;
            const rows = [...document.querySelectorAll('[data-pe-toggle]')].map(input => {
                const key = input.dataset.peToggle;
                const before = !!bridge.peGetToggle(key);
                input.checked = !before;
                input.dispatchEvent(new Event('change', { bubbles: true }));
                const changed = bridge.peGetToggle(key) === !before;
                input.checked = before;
                input.dispatchEvent(new Event('change', { bubbles: true }));
                return { key, changed, restored: bridge.peGetToggle(key) === before };
            });
            const dt = document.getElementById('pe-dt-slider');
            dt.value = '1.5'; dt.dispatchEvent(new Event('input', { bubbles: true }));
            const soft = document.getElementById('pe-soft-slider');
            soft.value = '0.37'; soft.dispatchEvent(new Event('input', { bubbles: true }));
            return { rows, dt: bridge.peGetDt(), dtLabel: document.getElementById('pe-dt-value')?.textContent, softLabel: document.getElementById('pe-soft-value')?.textContent };
        });
        expect(result.rows.length).toBeGreaterThan(0);
        expect(result.rows.every(row => row.changed && row.restored), JSON.stringify(result.rows)).toBe(true);
        expect(result.dt).toBeCloseTo(1.5);
        expect(result.dtLabel).toBe('1.5');
        expect(result.softLabel).toBe('0.37');
    });

    test('atom controls mutate runtime toggles and numeric dynamics through their public inputs', async ({ page }) => {
        await bootDashboard(page, { engine: 'wasm', mode: 'atoms' });
        await openDockPanel(page, 'controls');
        const result = await page.evaluate(async () => {
            const { AE_PHYSICS_SPECS } = await import('/js/scales/scale2/scenario-registry.js');
            const bridge = window._ftdBridge;
            const rows = AE_PHYSICS_SPECS.map(spec => {
                const input = document.getElementById(spec.elementId);
                const before = !!bridge.aeGetRuntimeState().toggles[spec.key];
                input.checked = !before;
                input.dispatchEvent(new Event('change', { bubbles: true }));
                const changed = bridge.aeGetRuntimeState().toggles[spec.key] === !before;
                input.checked = before;
                input.dispatchEvent(new Event('change', { bubbles: true }));
                return { key: spec.key, changed, restored: bridge.aeGetRuntimeState().toggles[spec.key] === before };
            });
            const set = (id, value) => {
                const input = document.getElementById(id);
                input.value = value;
                input.dispatchEvent(new Event('input', { bubbles: true }));
            };
            set('ae-dt-slider', '0.25');
            set('ae-soft-slider', '0.8');
            set('ae-thermostat-slider', '1.75');
            const runtime = bridge.aeGetRuntimeState();
            return { rows, dt: runtime.dt, softening: runtime.softening, thermostatTemp: runtime.thermostatTemp };
        });
        expect(result.rows.every(row => row.changed && row.restored), JSON.stringify(result.rows)).toBe(true);
        expect(result.dt).toBeCloseTo(0.25);
        expect(result.softening).toBeCloseTo(0.8);
        expect(result.thermostatTemp).toBeCloseTo(1.75);
    });

    test('settings groups preserve independent selected values and modal returns keyboard focus', async ({ page }) => {
        await bootDashboard(page, { engine: 'wasm' });
        const opener = page.locator('#btn-settings');
        await opener.focus();
        await opener.click();
        await expect(page.locator('#settings-modal')).toHaveClass(/visible/);
        await expect(page.locator('#settings-close')).toBeFocused();
        await page.locator('[data-setting="density"][data-value="compact"]').click();
        await page.locator('[data-setting="panel-width"][data-value="wide"]').click();
        await page.locator('[data-setting="tooltips"][data-value="off"]').click();
        await page.locator('[data-setting="status-bar"][data-value="hidden"]').click();
        const state = await page.evaluate(() => ({
            density: document.documentElement.dataset.density,
            panelWidth: document.documentElement.dataset.panelWidth,
            tooltips: document.documentElement.dataset.tooltips,
            statusBar: document.documentElement.dataset.statusBar,
            active: [...document.querySelectorAll('[data-setting].active')].map(el => `${el.dataset.setting}:${el.dataset.value}`),
        }));
        expect(state).toMatchObject({ density: 'compact', panelWidth: 'wide', tooltips: 'off', statusBar: 'hidden' });
        expect(state.active).toEqual(expect.arrayContaining(['density:compact', 'panel-width:wide', 'tooltips:off', 'status-bar:hidden']));
        await page.keyboard.press('Escape');
        await expect(page.locator('#settings-modal')).not.toHaveClass(/visible/);
        await expect(opener).toBeFocused();
    });

    test('shared viewport toggles target the active renderer and disable unsupported capabilities', async ({ page }) => {
        await bootDashboard(page, { engine: 'wasm' });
        const selectMode = async (mode) => {
            await switchMode(page, mode);
        };
        const clickState = async (id) => page.evaluate((buttonId) => {
            const button = document.getElementById(buttonId);
            const before = button?.getAttribute('aria-pressed');
            button?.click();
            return { disabled: !!button?.disabled, before, after: button?.getAttribute('aria-pressed') };
        }, id);

        const lattice = await clickState('toggle-grid');
        expect(lattice.disabled).toBe(false);
        expect(lattice.after).not.toBe(lattice.before);
        const latticePrivate = {
            orientation: await clickState('toggle-boundary-orientation'),
            clock: await clickState('toggle-global-clock'),
        };

        await selectMode('particles');
        const particle = await clickState('toggle-axes');
        expect(particle.disabled).toBe(false);
        expect(particle.after).not.toBe(particle.before);

        await selectMode('atoms');
        const atom = await clickState('toggle-grid');
        expect(atom.disabled).toBe(false);
        expect(atom.after).not.toBe(atom.before);

        await selectMode('planetary');
        const scale4 = await page.evaluate(async () => {
            const api = await import('/js/scales/scale4/controller.js?v=12');
            const before = api.getViewControlState();
            document.getElementById('toggle-axes').click();
            document.getElementById('toggle-grid').click();
            return { before, after: api.getViewControlState(), caps: api.getViewControlCapabilities() };
        });
        expect(scale4.caps).toMatchObject({ axes: true, grid: true });
        expect(scale4.after.axes).not.toBe(scale4.before.axes);
        expect(scale4.after.grid).not.toBe(scale4.before.grid);

        await selectMode('cosmic');
        const scale5 = await page.evaluate(async () => {
            const api = await import('/js/scales/scale5/controller.js');
            const axes = document.getElementById('toggle-axes');
            const grid = document.getElementById('toggle-grid');
            const before = api.getViewControlState();
            axes.click(); grid.click();
            return { before, after: api.getViewControlState(), caps: api.getViewControlCapabilities(), axesDisabled: axes.disabled, gridDisabled: grid.disabled };
        });
        expect(scale5.caps).toMatchObject({ axes: false, grid: true });
        expect(scale5.axesDisabled).toBe(true);
        expect(scale5.gridDisabled).toBe(false);
        expect(scale5.after.grid).not.toBe(scale5.before.grid);

        await selectMode('meta');
        const meta = await page.evaluate(async () => {
            const api = await import('/js/scales/scale6/controller.js');
            return { caps: api.getViewControlCapabilities(), axesDisabled: document.getElementById('toggle-axes').disabled, gridDisabled: document.getElementById('toggle-grid').disabled };
        });
        expect(meta.caps).toMatchObject({ axes: false, grid: false });
        expect(meta.axesDisabled).toBe(true);
        expect(meta.gridDisabled).toBe(true);

        await selectMode('lattice');
        await expect(page.locator('#toggle-boundary-orientation')).toHaveAttribute('aria-pressed', latticePrivate.orientation.after);
        await expect(page.locator('#toggle-global-clock')).toHaveAttribute('aria-pressed', latticePrivate.clock.after);
    });

    test('GPU start uses the selected lattice value only in its routed request', async ({ page }) => {
        const requests = [];
        await page.route('**/api/gpu-server/status', route => route.fulfill({ json: { running: false, exeExists: true, idle: { enabled: true, minutes: 30 } } }));
        await page.route('**/api/gpu-server/start', async route => {
            requests.push(route.request().postDataJSON());
            await route.fulfill({ json: { error: 'fixture: start suppressed' } });
        });
        await bootDashboard(page, { engine: 'wasm' });
        await expect(page.locator('#gpu-lattice')).toBeVisible();
        await page.selectOption('#gpu-lattice', '97');
        await page.locator('#gpu-start').click();
        await expect.poll(() => requests.length).toBe(1);
        expect(requests[0]).toMatchObject({ lattice: 97, autoStop: true, idleMinutes: 30 });
        await expect(page.locator('#gpu-msg')).toContainText('fixture: start suppressed');
    });
});
