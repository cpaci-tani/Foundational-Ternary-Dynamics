// @ts-check
/**
 * Scale-0 transaction tracker — history-journal drain + reconstruction.
 *
 * Two kinds of coverage here:
 *
 *   1. Pure-logic tests (no browser, no WASM): import transaction-tracker.js
 *      as a plain Node ESM module (same technique scenario-parity.spec.js
 *      uses for scenario-registry.js) and feed it synthetic drained-event
 *      batches shaped exactly like WasmBridge#drainHistoryEvents() returns.
 *      These exercise the actual reconstruction algorithm — null-orientation
 *      recovery, record continuity across Movement, cycle detection — and
 *      run for real regardless of which WASM binary is deployed, since the
 *      tracker has no WASM dependency itself.
 *
 *   2. Live-bridge tests (real Chromium + the deployed WASM binary): assert
 *      the panel/tracker no-op gracefully on a mock-owned scenario (this
 *      passes against ANY deployed WASM binary, old or new, since MockBridge
 *      never implements the three history-journal methods) and against
 *      whatever the CURRENT bridge capability actually is. The tests that
 *      require enableHistoryJournal/drainHistoryEvents to be LIVE on a
 *      WASM-owned scenario are marked test.skip with the reason: this
 *      worktree adds the C++/Embind bindings (engine/wasm/bindings_render_bridge.cpp)
 *      but does not rebuild the WASM binary — engine/web/wasm/*.wasm still
 *      predates this feature until the integrator runs build_wasm.bat.
 */
import { test, expect } from '@playwright/test';
import { dirname, resolve } from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';
import { gotoAndReady } from './_helpers.js';

const __dirname = dirname(fileURLToPath(import.meta.url));
const WEB_ROOT = resolve(__dirname, '..');
const TRACKER_MODULE = resolve(WEB_ROOT, 'js', 'scales', 'scale0', 'runtime', 'transaction-tracker.js');

async function loadTracker() {
    return import(pathToFileURL(TRACKER_MODULE).href);
}

/** Build one drained-event batch (the shape drainHistoryEvents() returns) from plain rows. */
function batch(rows) {
    return {
        rowCount: rows.length,
        kind: rows.map((r) => r.kind),
        tick: rows.map((r) => r.tick),
        eventId: rows.map((r) => r.eventId),
        site: rows.map((r) => r.site),
        stateBefore: rows.map((r) => r.stateBefore),
        stateAfter: rows.map((r) => r.stateAfter),
        particleIdBefore: rows.map((r) => r.particleIdBefore),
        particleIdAfter: rows.map((r) => r.particleIdAfter),
        chiralityBefore: rows.map(() => 0),
        chiralityAfter: rows.map(() => 0),
    };
}

test.describe('TransactionTracker — pure reconstruction logic (no browser/WASM needed)', () => {
    test('isTransactionTrackingSupported requires all three history-journal methods', async () => {
        const { isTransactionTrackingSupported } = await loadTracker();
        expect(isTransactionTrackingSupported(null)).toBe(false);
        expect(isTransactionTrackingSupported({})).toBe(false);
        expect(isTransactionTrackingSupported({ enableHistoryJournal: () => {} })).toBe(false);
        expect(isTransactionTrackingSupported({
            enableHistoryJournal: () => {}, historyJournalEnabled: () => {}, drainHistoryEvents: () => {},
        })).toBe(true);
    });

    test('empty/absent drains are harmless no-ops (the mock-bridge / worker-bridge fallback shape)', async () => {
        const { TransactionTracker } = await loadTracker();
        const tracker = new TransactionTracker();
        expect(() => tracker.ingestTick({ rowCount: 0 })).not.toThrow();
        expect(() => tracker.ingestTick(null)).not.toThrow();
        expect(() => tracker.ingestTick(undefined)).not.toThrow();
        expect(tracker.snapshotTelemetry().totalRecordCount).toBe(0);
    });

    test('genesis assigns a birth tick keyed by particle_id, not by site', async () => {
        const { TransactionTracker, HISTORY_EVENT_KIND } = await loadTracker();
        const tracker = new TransactionTracker();
        tracker.ingestTick(batch([
            { tick: 5, site: 42, kind: HISTORY_EVENT_KIND.GENESIS, stateBefore: 0, stateAfter: 1, particleIdBefore: -1, particleIdAfter: 100, eventId: 0 },
        ]));
        const rec = tracker.records.get(100);
        expect(rec.birthTick).toBe(5);
        expect(rec.birthSite).toBe(42);
        expect(tracker.liveRecordCount()).toBe(1);
    });

    test('movement relocates the SAME particle_id record instead of closing it', async () => {
        const { TransactionTracker, HISTORY_EVENT_KIND } = await loadTracker();
        const tracker = new TransactionTracker();
        tracker.ingestTick(batch([
            { tick: 5, site: 42, kind: HISTORY_EVENT_KIND.GENESIS, stateBefore: 0, stateAfter: 1, particleIdBefore: -1, particleIdAfter: 100, eventId: 0 },
        ]));
        tracker.ingestTick(batch([
            { tick: 6, site: 42, kind: HISTORY_EVENT_KIND.MOVEMENT, stateBefore: 1, stateAfter: 0, particleIdBefore: 100, particleIdAfter: -1, eventId: 1 },
            { tick: 6, site: 43, kind: HISTORY_EVENT_KIND.MOVEMENT, stateBefore: 0, stateAfter: 1, particleIdBefore: -1, particleIdAfter: 100, eventId: 1 },
        ]));
        expect(tracker.records.size).toBe(1);
        expect(tracker.records.get(100).site).toBe(43);
        expect(tracker.records.get(100).expiryTick).toBeNull();
    });

    test('a null reconstructed from a +1 death is oriented 0-down; from a -1 death, 0-up', async () => {
        const { TransactionTracker, HISTORY_EVENT_KIND } = await loadTracker();
        const tracker = new TransactionTracker();
        // Site 7 dies from +1 at tick 10, reborn as -1 at tick 14.
        tracker.ingestTick(batch([{ tick: 10, site: 7, kind: HISTORY_EVENT_KIND.EVAPORATION, stateBefore: 1, stateAfter: 0, particleIdBefore: 900, particleIdAfter: -1, eventId: 0 }]));
        tracker.ingestTick(batch([{ tick: 14, site: 7, kind: HISTORY_EVENT_KIND.GENESIS, stateBefore: 0, stateAfter: -1, particleIdBefore: -1, particleIdAfter: 901, eventId: 1 }]));
        const down = tracker.nullIntervals.find((iv) => iv.site === 7);
        expect(down).toBeTruthy();
        expect(down.orientation).toBe('0-down');
        expect(down.toTick - down.fromTick).toBe(4);
        expect(down.cycleConsistent).toBe(true);

        // Site 7 dies from -1 at tick 20, reborn as +1 at tick 25.
        tracker.ingestTick(batch([{ tick: 20, site: 7, kind: HISTORY_EVENT_KIND.ANNIHILATION, stateBefore: -1, stateAfter: 0, particleIdBefore: 901, particleIdAfter: -1, eventId: 2 }]));
        tracker.ingestTick(batch([{ tick: 25, site: 7, kind: HISTORY_EVENT_KIND.GENESIS, stateBefore: 0, stateAfter: 1, particleIdBefore: -1, particleIdAfter: 902, eventId: 3 }]));
        const up = tracker.nullIntervals.find((iv) => iv.site === 7 && iv.orientation === '0-up');
        expect(up).toBeTruthy();
        expect(up.toTick - up.fromTick).toBe(5);
        // Cycle detection needs THREE consecutive alternating-sign births at
        // the SAME site to close a cycle; site 7's chain so far only holds two
        // (birth@14=-1, birth@25=+1) since this tracker never observed an
        // earlier birth before the tick-10 death that started the sequence.
        expect(tracker.completedCycles.length).toBe(0);

        // Site 7 dies from +1 at tick 30, reborn as -1 at tick 35 -- this
        // THIRD birth (-1) closes the alternating triple (-1@14, +1@25, -1@35)
        // and completes one full +1<->0<->-1 cycle, measured birth-to-birth.
        tracker.ingestTick(batch([{ tick: 30, site: 7, kind: HISTORY_EVENT_KIND.EVAPORATION, stateBefore: 1, stateAfter: 0, particleIdBefore: 902, particleIdAfter: -1, eventId: 4 }]));
        tracker.ingestTick(batch([{ tick: 35, site: 7, kind: HISTORY_EVENT_KIND.GENESIS, stateBefore: 0, stateAfter: -1, particleIdBefore: -1, particleIdAfter: 903, eventId: 5 }]));
        expect(tracker.completedCycles.some((c) => c.site === 7 && c.fromTick === 14 && c.toTick === 35 && c.length === 21)).toBe(true);

        const snap = tracker.snapshotTelemetry();
        expect(snap.cycle.count).toBe(1);
        expect(snap.cycle.min).toBe(21);
        expect(snap.transactionLatency.min).toBe(4);
        // Dwell times observed: 4 (10->14), 5 (20->25), 5 (30->35).
        expect(snap.transactionLatency.max).toBe(5);
        expect(snap.transactionLatency.count).toBe(3);
    });

    test('weak transmutation flips a record in place without touching null bookkeeping', async () => {
        const { TransactionTracker, HISTORY_EVENT_KIND } = await loadTracker();
        const tracker = new TransactionTracker();
        tracker.ingestTick(batch([{ tick: 1, site: 3, kind: HISTORY_EVENT_KIND.GENESIS, stateBefore: 0, stateAfter: 1, particleIdBefore: -1, particleIdAfter: 5, eventId: 0 }]));
        tracker.ingestTick(batch([{ tick: 2, site: 3, kind: HISTORY_EVENT_KIND.WEAK_TRANSMUTATION, stateBefore: 1, stateAfter: -1, particleIdBefore: 5, particleIdAfter: 5, eventId: 1 }]));
        expect(tracker.records.size).toBe(1);
        expect(tracker.records.get(5).expiryTick).toBeNull();
        expect(tracker.nullIntervals.length).toBe(0);
        expect(tracker.snapshotTelemetry().eventCounts.WeakTransmutation).toBe(1);
    });

    test('event kind counts and reset() are accurate', async () => {
        const { TransactionTracker, HISTORY_EVENT_KIND } = await loadTracker();
        const tracker = new TransactionTracker();
        tracker.ingestTick(batch([
            { tick: 1, site: 1, kind: HISTORY_EVENT_KIND.GENESIS, stateBefore: 0, stateAfter: 1, particleIdBefore: -1, particleIdAfter: 1, eventId: 0 },
            { tick: 1, site: 2, kind: HISTORY_EVENT_KIND.GENESIS, stateBefore: 0, stateAfter: -1, particleIdBefore: -1, particleIdAfter: 2, eventId: 1 },
        ]));
        expect(tracker.snapshotTelemetry().eventCounts.Genesis).toBe(2);
        tracker.reset();
        const snap = tracker.snapshotTelemetry();
        expect(snap.eventCounts.Genesis).toBe(0);
        expect(snap.totalRecordCount).toBe(0);
    });
});

test.describe('Transaction panel — live bridge behavior', () => {
    test('the panel and tracker no-op gracefully on a mock-owned scenario', async ({ page }) => {
        await gotoAndReady(page);
        await expect.poll(
            () => page.evaluate(() => !!(window.__ftdCtx && window.__ftdCtx.bridge)),
            { timeout: 20_000, message: 'window.__ftdCtx.bridge never became available' },
        ).toBe(true);

        // flux-pulse runs on the JS MockBridge (per CLAUDE.md's dual-bridge note:
        // flux-*/s0-* scenarios do not all run on WASM). MockBridge never
        // implements enableHistoryJournal/historyJournalEnabled/drainHistoryEvents,
        // so this assertion is bridge-shape driven and holds regardless of which
        // WASM binary (old or history-journal-aware) is deployed.
        await page.evaluate(() => {
            const sel = document.getElementById('scenario-select');
            sel.value = 'flux-pulse';
            sel.dispatchEvent(new Event('change', { bubbles: true }));
        });
        await page.waitForTimeout(800);

        const state = await page.evaluate(() => {
            const bridge = window.__ftdCtx?.bridge;
            const supported = !!bridge
                && typeof bridge.enableHistoryJournal === 'function'
                && typeof bridge.historyJournalEnabled === 'function'
                && typeof bridge.drainHistoryEvents === 'function';
            const panel = document.getElementById('transaction-panel');
            return {
                hasPanel: !!panel,
                unavailableHidden: panel ? panel.querySelector('#tp-unavailable')?.hidden : null,
                supported,
            };
        });
        expect(state.hasPanel, 'transaction panel must be mounted into #panel-transactions').toBe(true);
        if (!state.supported) {
            expect(state.unavailableHidden, 'panel must show the not-available hint when the bridge lacks the journal').toBe(false);
        }
    });

    // Integration (2026-09-04): the two tests below were bodiless test.skip(true)
    // placeholders pending the WASM rebuild. The rebuild landed (bindings verified
    // present in ftd_core{,64,_mt}.wasm), so they now assert against the live
    // main-thread WasmBridge (ctx.bridge) — the same instance the direct
    // sampler specs use. They self-skip (with the probe result) only if the
    // deployed binary genuinely lacks the bindings.
    test('the deployed WASM binary exposes the history-journal bindings and the journal toggles cleanly', async ({ page }) => {
        test.setTimeout(60_000);
        await gotoAndReady(page);
        await expect.poll(() => page.evaluate(() => !!window.__ftdCtx?.bridge), { timeout: 15_000 }).toBe(true);
        const r = await page.evaluate(async () => {
            const b = window.__ftdCtx?.bridge;
            const { isTransactionTrackingSupported } = await import('/js/scales/scale0/runtime/transaction-tracker.js');
            const supported = isTransactionTrackingSupported(b);
            const m = b?._module;
            const probe = { supported, moduleReachable: !!m, drain: typeof m?.drainHistoryEvents === 'function' };
            if (!supported) return { probe };
            const before = b.historyJournalEnabled();
            const on = b.enableHistoryJournal(true);
            const during = b.historyJournalEnabled();
            b.enableHistoryJournal(false);
            const after = b.historyJournalEnabled();
            return { probe, before, on, during, after };
        });
        test.skip(!r.probe.supported, `deployed WASM lacks the journal bindings (probe: ${JSON.stringify(r.probe)})`);
        expect(r.before, 'journal must be OFF by default (opt-in, performance)').toBe(false);
        expect(r.during, 'enableHistoryJournal(true) must be reflected by historyJournalEnabled()').toBe(true);
        expect(r.after, 'enableHistoryJournal(false) must switch it back off').toBe(false);
    });

    test('a live tick loop drains real genesis rows and the tracker ingests them', async ({ page }) => {
        test.setTimeout(90_000);
        await gotoAndReady(page);
        await expect.poll(() => page.evaluate(() => !!window.__ftdCtx?.bridge), { timeout: 15_000 }).toBe(true);
        const r = await page.evaluate(async () => {
            const b = window.__ftdCtx?.bridge;
            const { isTransactionTrackingSupported, TransactionTracker, HISTORY_EVENT_KIND } =
                await import('/js/scales/scale0/runtime/transaction-tracker.js');
            if (!isTransactionTrackingSupported(b) || !b?.injectFlux || !b?.tick) {
                return { skip: 'bridge lacks journal bindings or direct inject/tick surface' };
            }
            // The default scenario's toggle defaults leave genesis/movement OFF on
            // ctx.bridge (probe-measured 2026-09-04); the journal only has
            // something to record once manifestation can actually happen.
            b.setToggle('genesis', true);
            b.setToggle('movement', true);
            b.enableHistoryJournal(true);
            const N = b.latticeSize, mid = Math.floor(N / 2);
            // Supercritical blob: |J| >> K_GENESIS so genesis fires within the loop.
            b.injectFlux(mid, mid, mid, 50.0, 0.0, 0.0);
            const tracker = new TransactionTracker();
            let rows = 0, genesisRows = 0, badKind = 0, badGenesis = 0;
            const kinds = new Set();
            for (let i = 0; i < 12; i++) {
                b.tick();
                // The native journal clears at the START of the next tick(),
                // so drain immediately after each tick.
                const d = b.drainHistoryEvents();
                rows += d.rowCount;
                for (let j = 0; j < d.rowCount; j++) {
                    const k = d.kind[j]; kinds.add(k);
                    if (k < 0 || k > 5) badKind++;
                    if (k === HISTORY_EVENT_KIND.GENESIS) {
                        genesisRows++;
                        if (d.stateBefore[j] !== 0 || d.stateAfter[j] === 0) badGenesis++;
                    }
                }
                tracker.ingestTick(d);
            }
            b.enableHistoryJournal(false);
            const snap = tracker.snapshotTelemetry();
            return { skip: null, rows, genesisRows, badKind, badGenesis, kinds: [...kinds].sort(),
                     snapIsObject: !!snap && typeof snap === 'object' };
        });
        test.skip(!!r.skip, r.skip || '');
        expect(r.rows, 'the drained journal must contain rows after ticking a supercritical blob').toBeGreaterThan(0);
        expect(r.badKind, 'every drained kind must be a valid HistoryEventKind (0..5)').toBe(0);
        expect(r.genesisRows, 'at least one Genesis row must fire from the injected blob').toBeGreaterThan(0);
        expect(r.badGenesis, 'every Genesis row must go stateBefore=0 -> stateAfter!=0').toBe(0);
        expect(r.snapIsObject, 'tracker.snapshotTelemetry() must return an object after ingesting real rows').toBe(true);
    });
});
