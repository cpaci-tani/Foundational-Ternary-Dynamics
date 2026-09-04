/**
 * Scale-0 transaction-tracker telemetry channel registry.
 *
 * Declarative descriptors for the reconstructed-manifestation-transaction
 * telemetry produced by transaction-tracker.js (see
 * engine/web/js/scales/scale0/runtime/transaction-tracker.js). Named the same
 * way as SCALE0_GRID_CHANNELS (./scale0-grid-channels.js), but the `buffer`
 * path resolves against a TransactionTracker#snapshotTelemetry() result
 * (namespaced under `txn.*`) rather than the diagnostics/audit/lagrangian hub
 * streams — the tracker accumulates a stateful cross-tick reconstruction, not
 * a value re-queryable from a single bridge getter each tick, so it does not
 * fit telemetryHub's per-tick RingBuffer collector model. The transaction
 * panel (ui/overlays/transaction-panel.js) reads the tracker directly for its
 * own display; this registry exists so a future telemetry-grid wiring pass
 * has a single canonical, documented list of channel names/units to bind
 * against instead of re-deriving them from the panel's ad hoc DOM code.
 */

export const SCALE0_TRANSACTION_CHANNELS = [
    { key: 'txnGenesis',       title: 'Genesis Events',        buffer: 'txn.eventCounts.Genesis',            unit: 'ct' },
    { key: 'txnEvaporation',   title: 'Evaporation Events',    buffer: 'txn.eventCounts.Evaporation',        unit: 'ct' },
    { key: 'txnAnnihilation',  title: 'Annihilation Events',   buffer: 'txn.eventCounts.Annihilation',       unit: 'ct' },
    { key: 'txnPairProd',      title: 'Pair-Production Events', buffer: 'txn.eventCounts.PairProduction',    unit: 'ct' },
    { key: 'txnWeakFlip',      title: 'Weak-Transmutation Events', buffer: 'txn.eventCounts.WeakTransmutation', unit: 'ct' },
    { key: 'txnMovement',      title: 'Movement Events',       buffer: 'txn.eventCounts.Movement',           unit: 'ct' },
    { key: 'txnLiveRecords',   title: 'Live Records',          buffer: 'txn.liveRecordCount',                unit: 'ct' },
    { key: 'txnLifetimeMin',   title: 'Min Lifetime',          buffer: 'txn.lifetime.min',                   unit: 'ticks' },
    { key: 'txnLifetimeMedian', title: 'Median Lifetime',      buffer: 'txn.lifetime.median',                unit: 'ticks' },
    { key: 'txnLatencyMin',    title: 'Min Transaction Latency', buffer: 'txn.transactionLatency.min',       unit: 'ticks' },
    { key: 'txnLatencyMedian', title: 'Median Transaction Latency', buffer: 'txn.transactionLatency.median', unit: 'ticks' },
    { key: 'txnCycleCount',    title: 'Complete Cycles',       buffer: 'txn.cycle.count',                    unit: 'ct' },
    { key: 'txnCycleMin',      title: 'Min Cycle Length',      buffer: 'txn.cycle.min',                      unit: 'ticks' },
    { key: 'txnCycleMedian',   title: 'Median Cycle Length',   buffer: 'txn.cycle.median',                   unit: 'ticks' },
];
