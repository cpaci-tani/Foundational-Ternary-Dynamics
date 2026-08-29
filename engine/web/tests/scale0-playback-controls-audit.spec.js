// @ts-check
import { test, expect } from '@playwright/test';
import { attachConsoleWatcher, gotoAndReady, realErrors, switchMode } from './_helpers.js';

test.describe('Scale 0 playback controls audit gate', () => {
    test.beforeEach(async ({ page }, testInfo) => {
        testInfo.setTimeout(90_000);
        page.setDefaultTimeout(30_000);
        await gotoAndReady(page);
        await page.waitForFunction(() => document.getElementById('app')?.dataset.shellReady === 'true');
    });

    test('every speed preset round-trips through the shared logarithmic scale', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        const results = await page.evaluate(async () => {
            const { sliderValueToSpeed, speedToSliderValue } = await import(
                '/js/ui/components/play-bar/speed-scale.js'
            );
            document.querySelector('.play-bar-settings')?.click();
            const samples = [];
            for (const chip of document.querySelectorAll('[data-speed-preset]')) {
                const expected = Number.parseFloat(chip.dataset.speedPreset);
                chip.click();
                await new Promise((resolve) => requestAnimationFrame(() => requestAnimationFrame(resolve)));
                samples.push({
                    expected,
                    slider: Number.parseFloat(document.getElementById('ticks-per-frame')?.value || 'NaN'),
                    mapped: sliderValueToSpeed(document.getElementById('ticks-per-frame')?.value),
                    inverse: speedToSliderValue(expected),
                    appSpeed: window.__ftdCtx?.ticksPerFrame,
                    active: chip.classList.contains('is-active'),
                    checked: chip.getAttribute('aria-checked'),
                });
            }
            return samples;
        });

        for (const sample of results) {
            expect(sample.slider).toBeCloseTo(sample.inverse, 3);
            expect(sample.mapped).toBeCloseTo(sample.expected, 3);
            expect(sample.appSpeed).toBeCloseTo(sample.expected, 3);
            expect(sample.active).toBe(true);
            expect(sample.checked).toBe('true');
        }
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('hidden playback settings perform zero DOM work', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        const report = await page.evaluate(async () => {
            const probe = await import('/tests/scale0-ui-audit-probe.js');
            document.getElementById('play-bar-settings-popover')?.setAttribute('hidden', '');
            await new Promise((resolve) => setTimeout(resolve, 3000));
            probe.startScale0UiAuditProbe({
                label: 'gate2-hidden-playback-settings',
                rootSelector: '#play-bar',
            });
            await new Promise((resolve) => setTimeout(resolve, 2000));
            return probe.stopScale0UiAuditProbe();
        });

        expect(report.dom.mutationRecords).toBe(0);
        expect(report.dom.addedNodes).toBe(0);
        expect(report.dom.removedNodes).toBe(0);
        expect(report.errors).toEqual([]);
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('a slider burst sends one speed update to the active owner only', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        const result = await page.evaluate(async () => {
            const ctx = window.__ftdCtx;
            const scale0 = await import('/js/scales/scale0/controller.js');
            let active = scale0.getActivePhysicsOwner(ctx);
            for (let i = 0; i < 100 && typeof active?.setTicksPerFrame !== 'function'; i += 1) {
                await new Promise((resolve) => setTimeout(resolve, 50));
                active = scale0.getActivePhysicsOwner(ctx);
            }
            const main = ctx?.bridge;
            if (!active?.setTicksPerFrame) {
                throw new Error('Playback-speed owner unavailable');
            }
            const originalActive = active.setTicksPerFrame;
            const originalMain = main?.setTicksPerFrame;
            let activeCalls = 0;
            let mainCalls = 0;
            active.setTicksPerFrame = function (...args) {
                activeCalls += 1;
                return originalActive.apply(this, args);
            };
            if (main !== active && typeof originalMain === 'function') {
                main.setTicksPerFrame = function (...args) {
                    mainCalls += 1;
                    return originalMain.apply(this, args);
                };
            }
            try {
                const slider = document.getElementById('ticks-per-frame');
                for (let value = 51; value <= 75; value += 1) {
                    slider.value = String(value);
                    slider.dispatchEvent(new Event('input', { bubbles: true }));
                }
                await new Promise((resolve) => requestAnimationFrame(() => requestAnimationFrame(resolve)));
                return {
                    activeCalls,
                    mainCalls,
                    distinctOwners: main !== active,
                    appSpeed: ctx.ticksPerFrame,
                };
            } finally {
                active.setTicksPerFrame = originalActive;
                if (main !== active && typeof originalMain === 'function') {
                    main.setTicksPerFrame = originalMain;
                }
            }
        });

        expect(result.activeCalls).toBe(1);
        if (result.distinctOwners) expect(result.mainCalls).toBe(0);
        expect(result.appSpeed).toBeCloseTo(10, 8);
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('single-step pauses the worker before issuing exactly one tick', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        const result = await page.evaluate(async () => {
            const ctx = window.__ftdCtx;
            const scale0 = await import('/js/scales/scale0/controller.js');
            const active = scale0.getActivePhysicsOwner(ctx);
            if (!active?.setRunning || !active?.tickOnce) throw new Error('Worker playback owner unavailable');
            const originalRun = active.setRunning;
            const originalTick = active.tickOnce;
            const events = [];
            active.setRunning = function (value) {
                events.push(`run:${!!value}`);
                return originalRun.call(this, value);
            };
            active.tickOnce = function (...args) {
                events.push('tick');
                return originalTick.apply(this, args);
            };
            try {
                if (document.getElementById('btn-play')?.dataset.paused !== 'false') {
                    document.getElementById('btn-play')?.click();
                }
                await new Promise((resolve) => requestAnimationFrame(resolve));
                events.length = 0;
                document.getElementById('btn-step')?.click();
                await new Promise((resolve) => requestAnimationFrame(resolve));
                return {
                    events,
                    paused: document.getElementById('btn-play')?.dataset.paused,
                    running: ctx.running,
                };
            } finally {
                active.setRunning = originalRun;
                active.tickOnce = originalTick;
            }
        });

        expect(result.events).toEqual(['run:false', 'tick']);
        expect(result.paused).toBe('true');
        expect(result.running).toBe(false);
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('reset cancels an in-flight +100 chain after its first synchronous step', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        const result = await page.evaluate(async () => {
            let stepClicks = 0;
            const step = document.getElementById('btn-step');
            const count = () => { stepClicks += 1; };
            step.addEventListener('click', count, { capture: true });
            try {
                document.querySelector('.play-bar-settings')?.click();
                document.querySelector('[data-step-by="100"]')?.click();
                document.getElementById('btn-reset')?.click();
                await new Promise((resolve) => setTimeout(resolve, 100));
                return {
                    stepClicks,
                    paused: document.getElementById('btn-play')?.dataset.paused,
                };
            } finally {
                step.removeEventListener('click', count, { capture: true });
            }
        });

        expect(result.stepClicks).toBe(1);
        expect(result.paused).toBe('true');
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('ten remount and settings cycles retain one playback owner and stable resources', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        const result = await page.evaluate(async () => {
            const scale0 = await import('/js/scales/scale0/controller.js');
            const { rafCoordinator } = await import('/js/lib/raf-coordinator.js');
            const before = {
                bars: document.querySelectorAll('#play-bar').length,
                nodes: document.querySelectorAll('#play-bar *').length,
                subscribers: rafCoordinator.size(),
            };

            for (let i = 0; i < 10; i += 1) {
                scale0.mountScale0PlaybackUI();
                const settings = document.querySelector('.play-bar-settings');
                settings.click();
                settings.click();
            }

            let speedInputEvents = 0;
            const slider = document.getElementById('ticks-per-frame');
            const countInput = () => { speedInputEvents += 1; };
            slider.addEventListener('input', countInput);
            document.querySelector('[data-speed-nudge="5"]')?.click();
            await new Promise((resolve) => requestAnimationFrame(() => requestAnimationFrame(resolve)));
            slider.removeEventListener('input', countInput);

            return {
                before,
                after: {
                    bars: document.querySelectorAll('#play-bar').length,
                    nodes: document.querySelectorAll('#play-bar *').length,
                    subscribers: rafCoordinator.size(),
                },
                speedInputEvents,
                settingsExpanded: document.querySelector('.play-bar-settings')?.getAttribute('aria-expanded'),
            };
        });

        expect(result.after).toEqual(result.before);
        expect(result.speedInputEvents).toBe(1);
        expect(result.settingsExpanded).toBe('false');
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('prime-tick state is single-owned, persisted, and hidden outside Scale 0', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        const before = await page.evaluate(async () => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            const button = document.getElementById('btn-prime-tick');
            return {
                hidden: button.hidden,
                aria: button.getAttribute('aria-pressed'),
                state: getScale0State().primeTickOnLoad,
            };
        });
        expect(before.hidden).toBe(false);
        expect(before.aria).toBe(before.state ? 'true' : 'false');

        await page.locator('#btn-prime-tick').click();
        const toggled = await page.evaluate(async () => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            return {
                state: getScale0State().primeTickOnLoad,
                stored: localStorage.getItem('ftd.scale0.primeTickOnLoad'),
                aria: document.getElementById('btn-prime-tick')?.getAttribute('aria-pressed'),
            };
        });
        expect(toggled.state).toBe(!before.state);
        expect(toggled.stored).toBe(toggled.state ? '1' : '0');
        expect(toggled.aria).toBe(toggled.state ? 'true' : 'false');

        await switchMode(page, 'particles');
        await expect(page.locator('#btn-prime-tick')).toBeHidden();
        await switchMode(page, 'lattice');
        await expect(page.locator('#btn-prime-tick')).toBeVisible();
        await expect(page.locator('#btn-prime-tick')).toHaveAttribute(
            'aria-pressed',
            toggled.state ? 'true' : 'false',
        );
        expect(realErrors(consoleErrors)).toEqual([]);
    });
});
