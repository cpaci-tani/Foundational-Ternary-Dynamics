// Shared classic-worker sampler scheduler. Kept free of DOM/WASM dependencies
// so its cadence and demand semantics can be exercised deterministically.
(function installSamplerCadence(root) {
  'use strict';

  const GRAVITY_SAMPLER_INTERVAL_MS = 250; // 4 Hz maximum expensive readback rate
  const GRAVITY_METRIC_AGG_KEY = 'gravityMetricAgg@0';
  const BOUNDED_INSTRUMENT_SAMPLER_KINDS = Object.freeze([
    'latency', 'kretschmann', 'gravity',
  ]);
  const boundedInstrumentSamplerKindSet = new Set(BOUNDED_INSTRUMENT_SAMPLER_KINDS);

  function isBoundedInstrumentSamplerWant(want) {
    return want?.cadenceClass === 'bounded-instrument'
      && boundedInstrumentSamplerKindSet.has(String(want?.kind || ''));
  }

  function createBoundedSamplerCadence(intervalMs = GRAVITY_SAMPLER_INTERVAL_MS) {
    const parsed = Number(intervalMs);
    const interval = Number.isFinite(parsed) && parsed > 0
      ? parsed : GRAVITY_SAMPLER_INTERVAL_MS;
    let nextDueAt = Number.NEGATIVE_INFINITY;

    return {
      shouldRun(nowMs, force = false) {
        const now = Number(nowMs);
        if (!Number.isFinite(now)) return false;
        if (!force && now < nextDueAt) return false;
        nextDueAt = now + interval;
        return true;
      },
      reset() {
        nextDueAt = Number.NEGATIVE_INFINITY;
      },
      get nextDueAt() {
        return nextDueAt;
      },
    };
  }

  // Pure frame-count gate used by the worker's energy-audit collector. Keeping
  // the transition explicit makes hidden-demand zero-work behavior testable
  // without loading WASM.
  function advanceDemandFrameCadence(wanted, counter, hasSample, everyFrames = 1) {
    if (!wanted) return { sample: false, nextCounter: 0 };
    const every = Math.max(1, Math.trunc(Number(everyFrames) || 1));
    const current = Math.trunc(Number(counter) || 0);
    const sample = current <= 0 || !hasSample;
    return {
      sample,
      nextCounter: (sample ? every : current) - 1,
    };
  }

  /**
   * Visit the sampler wants that are due for this worker publication.
   *
   * Gravity/Time instrument samplers share one cadence decision, keeping the
   * Gravity observation atomic. The cadence class is resolved by the owner-set
   * union: any ordinary/direct/viewport co-owner upgrades that key to realtime.
   * gravityMetricAgg is special: telemetry demand is its scheduler owner. A
   * visible Time panel can therefore receive it without a second direct owner,
   * while hidden/Empty demand suppresses a briefly stale aggregate want.
   */
  function visitScheduledSamplers(wantedSamplers, {
    wantGravity = false,
    cadence,
    nowMs,
    forceGravityBatch = false,
    allowUndemandedBoundedInstrument = false,
  } = {}, visit = () => {}) {
    if (!wantedSamplers || typeof wantedSamplers.entries !== 'function') return false;
    if (!cadence || typeof cadence.shouldRun !== 'function') return false;

    let hasExplicitGravityMetricAgg = false;
    let hasBoundedSampler = false;
    for (const [, want] of wantedSamplers.entries()) {
      const kind = String(want?.kind || '');
      if (kind === 'gravityMetricAgg') {
        hasExplicitGravityMetricAgg = true;
        // A newly-added paused Gravity batch is allowed to populate atomically
        // before the controller's demand-mask message reaches this worker.
        if (wantGravity || allowUndemandedBoundedInstrument) hasBoundedSampler = true;
      } else if (isBoundedInstrumentSamplerWant(want)
          && (wantGravity || allowUndemandedBoundedInstrument)) {
        hasBoundedSampler = true;
      }
    }
    if (wantGravity && !hasExplicitGravityMetricAgg) hasBoundedSampler = true;

    const gravityBatchDue = hasBoundedSampler
      ? cadence.shouldRun(nowMs, forceGravityBatch)
      : false;

    for (const [key, want] of wantedSamplers.entries()) {
      const kind = String(want?.kind || '');
      if (kind === 'gravityMetricAgg' && !wantGravity
          && !allowUndemandedBoundedInstrument) continue;
      if (isBoundedInstrumentSamplerWant(want)) {
        if (!wantGravity && !allowUndemandedBoundedInstrument) continue;
        if (!gravityBatchDue) continue;
      }
      if (kind === 'gravityMetricAgg' && !gravityBatchDue) continue;
      visit(key, want);
    }

    if (wantGravity && !hasExplicitGravityMetricAgg && gravityBatchDue) {
      visit(GRAVITY_METRIC_AGG_KEY, { kind: 'gravityMetricAgg', stride: 0 });
    }
    return gravityBatchDue;
  }

  root.FTD_SAMPLER_CADENCE = Object.freeze({
    GRAVITY_SAMPLER_INTERVAL_MS,
    GRAVITY_METRIC_AGG_KEY,
    BOUNDED_INSTRUMENT_SAMPLER_KINDS,
    isBoundedInstrumentSamplerWant,
    createBoundedSamplerCadence,
    advanceDemandFrameCadence,
    visitScheduledSamplers,
  });
})(typeof self !== 'undefined' ? self : globalThis);
