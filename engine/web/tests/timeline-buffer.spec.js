// @ts-check
import { test, expect } from '@playwright/test';

test('TimelineBuffer eviction respects budget', async ({ page }) => {
    await page.goto('/');
    const r = await page.evaluate(async () => {
        const { TimelineBuffer } = await import('/js/scales/scale0/timeline/buffer.js');
        const buf = new TimelineBuffer({ budgetBytes: 50_000, latticeN: 8 });
        for (let t = 0; t < 50; t++) {
            buf.push({ tick: t, ts: Date.now(), lod: 0,
                lattice: new Int8Array(8 * 8 * 8),
                flux:    new Float32Array(3 * 8 * 8 * 8),
                particles: [], audit: {} });
        }
        return { size: buf.size, bytesUsed: buf.bytesUsed(), oldest: buf.oldestTick, latest: buf.latestTick };
    });
    expect(r.bytesUsed).toBeLessThanOrEqual(50_000);
    expect(r.size).toBeGreaterThan(0);
    expect(r.latest).toBe(49);
    // Eviction must advance the oldest tick past 0.
    expect(r.oldest).toBeGreaterThan(0);
});

test('TimelineBuffer nearestBefore finds the correct snapshot', async ({ page }) => {
    await page.goto('/');
    const r = await page.evaluate(async () => {
        const { TimelineBuffer } = await import('/js/scales/scale0/timeline/buffer.js');
        const buf = new TimelineBuffer({ budgetBytes: 10_000_000, latticeN: 4 });
        for (const t of [0, 10, 20, 30, 40]) {
            buf.push({ tick: t, ts: 0, lod: 0,
                lattice: new Int8Array(64), flux: new Float32Array(192),
                particles: [], audit: {} });
        }
        return [5, 10, 25, 40, 100].map(q => buf.nearestBefore(q)?.tick);
    });
    expect(r).toEqual([0, 10, 20, 40, 40]);
});

test('TimelineBuffer asZones groups contiguous LOD runs', async ({ page }) => {
    await page.goto('/');
    const r = await page.evaluate(async () => {
        const { TimelineBuffer } = await import('/js/scales/scale0/timeline/buffer.js');
        const buf = new TimelineBuffer({ budgetBytes: 10_000_000, latticeN: 4 });
        const lods = [2, 2, 1, 1, 1, 0, 0, 0, 0];
        for (let i = 0; i < lods.length; i++) {
            buf.push({ tick: i, ts: 0, lod: lods[i],
                lattice: new Int8Array(8), flux: new Float32Array(24),
                particles: [], audit: {} });
        }
        return buf.asZones();
    });
    expect(r).toEqual([
        { lod: 2, fromTick: 0, toTick: 1 },
        { lod: 1, fromTick: 2, toTick: 4 },
        { lod: 0, fromTick: 5, toTick: 8 },
    ]);
});

test('blockAverageScalar halves cube edge', async ({ page }) => {
    await page.goto('/');
    const r = await page.evaluate(async () => {
        const { blockAverageScalar } = await import('/js/scales/scale0/timeline/lod.js');
        // 4x4x4 of constant 8 — block-average by 2 should give 2x2x2 of 8.
        const src = new Int8Array(64).fill(8);
        const out = blockAverageScalar(src, 4, 1);
        return { len: out.length, allEight: [...out].every(v => v === 8) };
    });
    expect(r.len).toBe(8);
    expect(r.allEight).toBe(true);
});

test('degradeSnapshot drops lattice at LOD 3', async ({ page }) => {
    await page.goto('/');
    const r = await page.evaluate(async () => {
        const { degradeSnapshot } = await import('/js/scales/scale0/timeline/buffer.js');
        const snap = { tick: 1, ts: 0, lod: 0,
            lattice: new Int8Array(8), flux: new Float32Array(24),
            particles: [], audit: { energy: 1.2 } };
        const out = degradeSnapshot(snap, 3, 2);
        return { lod: out.lod, lattice: out.lattice, flux: out.flux, audit: out.audit };
    });
    expect(r.lod).toBe(3);
    expect(r.lattice).toBe(null);
    expect(r.flux).toBe(null);
    expect(r.audit.energy).toBe(1.2);
});

test('MemoryRecorder defers full snapshots until sample cadence', async ({ page }) => {
    await page.goto('/');
    const r = await page.evaluate(async () => {
        const { MemoryRecorder } = await import('/js/scales/scale0/timeline/memory-recorder.js');
        const rec = new MemoryRecorder({
            budgetBytes: 1_000_000,
            latticeN: 4,
            ticksPerSecond: 10,
            tiers: [
                { lod: 0, cadenceSeconds: 0.5, durationSeconds: 10 },
                { lod: 1, cadenceSeconds: 1.0, durationSeconds: 10 },
            ],
        });
        let tick = 0;
        let diagCalls = 0;
        let snapshotCalls = 0;
        const caps = {
            getScale0Diagnostics: () => {
                diagCalls++;
                return { tick };
            },
            getScale0Snapshot: () => {
                snapshotCalls++;
                return {
                    tick,
                    ts: 0,
                    lod: 0,
                    lattice: new Int8Array(4 * 4 * 4),
                    flux: new Float32Array(3 * 4 * 4 * 4),
                    particles: [],
                    audit: {},
                };
            },
        };

        for (tick = 0; tick < 4; tick++) rec.onTick(caps);
        const beforeCadence = snapshotCalls;
        tick = 4;
        rec.onTick(caps);

        return {
            beforeCadence,
            snapshotCalls,
            diagCalls,
            size: rec.buffer.size,
            latest: rec.buffer.latestTick,
        };
    });

    expect(r.beforeCadence).toBe(0);
    expect(r.snapshotCalls).toBe(1);
    expect(r.diagCalls).toBe(5);
    expect(r.size).toBe(1);
    expect(r.latest).toBe(4);
});
