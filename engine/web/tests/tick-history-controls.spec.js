import { test, expect } from '@playwright/test';

test('tick history buffers preserve every sample and window by real tick', async ({ page }) => {
    await page.goto('/js/telemetry-hub.js', { waitUntil: 'domcontentloaded' });
    const result = await page.evaluate(async () => {
        const [{ RingBuffer, MultiRingBuffer }, { TickHistoryControl }] = await Promise.all([
            import('/js/telemetry-hub.js?tick-history-buffer-test=1'),
            import('/js/ui/charts/history-window.js?tick-history-buffer-test=1'),
        ]);
        localStorage.removeItem('ftd.chart-history.test-buffer-window');
        const host = document.createElement('div');
        document.body.appendChild(host);
        const control = new TickHistoryControl(host, {
            id: 'test-buffer-window',
            defaultTicks: 15,
        });

        const ring = new RingBuffer(2);
        ring.push(1, 0);
        ring.push(2, 10);
        ring.push(3, 20);
        ring.push(4, 30);
        const windowCount = control.visibleCount(ring);
        control.setMode('all');
        const allCount = control.visibleCount(ring);

        const multi = new MultiRingBuffer(2, ['a', 'b']);
        multi.push({ a: 1, b: 10 }, 4);
        multi.push({ a: 2, b: 20 }, 8);
        multi.push({ a: 3, b: 30 }, 12);

        const value = {
            windowCount,
            allCount,
            ringCount: ring.count,
            ringCapacity: ring.size,
            ringValues: Array.from({ length: ring.count }, (_, i) => ring.get(i)),
            ringTicks: Array.from({ length: ring.count }, (_, i) => ring.getTick(i)),
            multiCount: multi.count,
            multiCapacity: multi.size,
            multiA: Array.from({ length: multi.count }, (_, i) => multi.views.a.get(i)),
            multiB: Array.from({ length: multi.count }, (_, i) => multi.views.b.get(i)),
            multiTicks: Array.from({ length: multi.count }, (_, i) => multi.views.a.getTick(i)),
            summary: control.el.querySelector('.tick-history-control__summary')?.textContent,
            inputDisabled: control.input.disabled,
        };
        control.destroy();
        localStorage.removeItem('ftd.chart-history.test-buffer-window');
        host.remove();
        return value;
    });

    expect(result).toEqual({
        windowCount: 2,
        allCount: 4,
        ringCount: 4,
        ringCapacity: 4,
        ringValues: [1, 2, 3, 4],
        ringTicks: [0, 10, 20, 30],
        multiCount: 3,
        multiCapacity: 4,
        multiA: [1, 2, 3],
        multiB: [10, 20, 30],
        multiTicks: [4, 8, 12],
        summary: 'all retained ticks',
        inputDisabled: true,
    });
});

test('temporal side panels expose consistent history controls', async ({ page }) => {
    await page.goto('/index.html', { waitUntil: 'domcontentloaded' });
    await page.waitForFunction(() => document.querySelector('#panel-charts [data-history-control="charts-panel"]'));

    const panelControls = await page.evaluate(() => {
        const expected = {
            charts: 'charts-panel',
            diagnostics: 'diagnostics-panel',
            'telemetry-grid': 'telemetry-grid-panel',
            lagrangian: 'lagrangian-panel',
            'wave-lab': 'wave-lab-panel',
            gravity: 'gravity-panel',
            time: 'time-panel',
            thermo: 'thermo-panel',
            knots: 'knots-panel',
            'p1-observables': 'p1-observables-panel',
            inspector: 'inspector-panel',
        };
        return Object.fromEntries(Object.entries(expected).map(([panelId, controlId]) => {
            const control = document.querySelector(`#panel-${panelId} [data-history-control="${controlId}"]`);
            return [panelId, control ? {
                last: control.querySelector('[data-history-mode="window"]')?.getAttribute('aria-pressed'),
                all: control.querySelector('[data-history-mode="all"]')?.getAttribute('aria-pressed'),
                ticks: control.querySelector('input')?.value,
            } : null];
        }));
    });

    for (const [panelId, control] of Object.entries(panelControls)) {
        expect(control, `${panelId} history control`).not.toBeNull();
        expect(control.last, `${panelId} defaults to a rolling window`).toBe('true');
        expect(control.all, `${panelId} all mode starts inactive`).toBe('false');
        expect(Number(control.ticks), `${panelId} has a positive tick span`).toBeGreaterThan(0);
    }
});
