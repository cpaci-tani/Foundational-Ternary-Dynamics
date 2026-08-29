// @ts-check
import { test, expect } from '@playwright/test';
import { attachConsoleWatcher, gotoAndReady, realErrors } from './_helpers.js';

test.describe('Scale 0 UI audit probe', () => {
    test('captures rAF, subscriber cost, DOM/canvas work, method calls, and action latency', async ({ page }) => {
        const consoleErrors = attachConsoleWatcher(page);
        await gotoAndReady(page);

        const report = await page.evaluate(async () => {
            const probe = await import('/tests/scale0-ui-audit-probe.js');
            const { rafCoordinator } = await import('/js/lib/raf-coordinator.js');

            const root = document.createElement('div');
            root.id = 'scale0-audit-probe-fixture';
            const value = document.createElement('span');
            const canvas = document.createElement('canvas');
            canvas.width = 16;
            canvas.height = 16;
            root.append(value, canvas);
            document.body.appendChild(root);

            let calls = 0;
            const requestTarget = { sample() { calls += 1; return calls; } };
            const sub = rafCoordinator.subscribe('scale0-audit-probe-self-test', {
                hz: 20,
                cb: () => {
                    value.textContent = String(requestTarget.sample());
                    canvas.getContext('2d').fillRect(0, 0, 2, 2);
                },
            });

            probe.startScale0UiAuditProbe({
                rootSelector: '#scale0-audit-probe-fixture',
                subscriberIds: ['scale0-audit-probe-self-test'],
            });
            probe.trackScale0UiMethods('fixture', requestTarget, ['sample']);
            await probe.measureScale0UiActionToPaint('fixture-action', () => {
                root.dataset.auditAction = 'complete';
            });
            await new Promise((resolve) => setTimeout(resolve, 1_200));
            const result = await probe.stopScale0UiAuditProbe();
            sub.unsubscribe();
            root.remove();
            return result;
        });

        // Sampling adequacy belongs here; the strict 60 FPS assertion belongs
        // in each warmed per-interface audit, not in this startup-time probe
        // self-test.
        expect(report.frames.count).toBeGreaterThan(30);
        expect(report.frames.effectiveFps).toBeGreaterThan(30);
        expect(report.callbacks['scale0-audit-probe-self-test'].count).toBeGreaterThan(10);
        expect(report.callbacks['scale0-audit-probe-self-test'].maxMs).toBeGreaterThanOrEqual(0);
        expect(report.methods['fixture.sample']).toBeGreaterThan(10);
        expect(report.actions.count).toBe(1);
        expect(report.actions.maxMs).toBeGreaterThanOrEqual(0);
        expect(report.actions.maxMs).toBeLessThan(250);
        expect(report.dom.mutationRecords).toBeGreaterThan(10);
        expect(report.dom.canvasDraws).toBeGreaterThan(10);
        expect(report.errors).toEqual([]);
        expect(realErrors(consoleErrors)).toEqual([]);
    });

    test('detects a synthetic Long Task', async ({ page }) => {
        await gotoAndReady(page);
        const report = await page.evaluate(async () => {
            const probe = await import('/tests/scale0-ui-audit-probe.js?long-task=1');
            probe.startScale0UiAuditProbe();
            await new Promise((resolve) => requestAnimationFrame(resolve));
            const end = performance.now() + 65;
            while (performance.now() < end) {
                // Intentional harness self-test: occupy the main thread >50 ms.
            }
            await new Promise((resolve) => setTimeout(resolve, 100));
            return probe.stopScale0UiAuditProbe();
        });

        expect(report.longTaskSupported).toBe(true);
        expect(report.longTasks.some((entry) => entry.duration >= 50)).toBe(true);
        expect(report.frames.maxMs).toBeGreaterThan(50);
    });
});
