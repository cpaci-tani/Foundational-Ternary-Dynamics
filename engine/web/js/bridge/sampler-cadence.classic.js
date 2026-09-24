// Shared classic-worker sampler scheduler. Kept free of DOM/WASM dependencies
// so its cadence and demand semantics can be exercised deterministically.
(function installSamplerCadence(root) {
  'use strict';

  const GRAVITY_SAMPLER_INTERVAL_MS = 250; // Legacy/Time cadence; live Gravity uses exact state identity.
  const GRAVITY_METRIC_AGG_KEY = 'gravityMetricAgg@0';
  const BOUNDED_INSTRUMENT_SAMPLER_KINDS = Object.freeze([
    'latency', 'kretschmann', 'gravity', 'tau', 'lapse', 'dbPhase',
  ]);
  const boundedInstrumentSamplerKindSet = new Set(BOUNDED_INSTRUMENT_SAMPLER_KINDS);
  const properTimeSamplerKindSet = new Set(['tau', 'lapse', 'dbPhase']);

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
   * Bound an expensive full-volume reduction by wall time and observed cost.
   * Frame-count cadence turns into sparse chart updates whenever a large
   * lattice naturally produces fewer frames. This scheduler targets a
   * best-effort observation interval and enforces a post-reduction cooldown
   * from the measured cost, reserving a fixed worker duty budget. It
   * never samples an unchanged simulation tick, except for an explicit
   * demand/source reset that has no prior observation.
   */
  function createBoundedReductionCadence({
    targetIntervalMs = 125,
    retryIntervalMs = targetIntervalMs,
    maxDutyCycle = 0.25,
  } = {}) {
    const target = Math.max(1, Number(targetIntervalMs) || 125);
    const retry = Math.max(1, Number(retryIntervalMs) || target);
    const duty = Math.min(1, Math.max(0.01, Number(maxDutyCycle) || 0.25));
    let nextDueAt = Number.NEGATIVE_INFINITY;
    let lastSampleTick = null;
    let hasCompletedAttempt = false;

    return {
      shouldRun(wanted, hasSample, nowMs, tick) {
        if (!wanted) {
          this.reset();
          return false;
        }
        const now = Number(nowMs);
        if (!Number.isFinite(now)) return false;
        const tickNumber = (tick === null || tick === undefined || tick === '')
          ? Number.NaN : Number(tick);
        const safeTick = Number.isSafeInteger(tickNumber) && tickNumber >= 0
          ? tickNumber : null;
        // A failed/unavailable attempt still records its tick in complete().
        // Do not turn that failure into an unbounded same-tick retry loop.
        if (safeTick !== null && safeTick === lastSampleTick) return false;
        // A successful unknown-clock result cannot be refreshed honestly: it
        // has no identity proving the state advanced. Retain it until a
        // source/demand reset supplies a new observation boundary.
        if (safeTick === null && hasSample && hasCompletedAttempt) return false;
        // An unavailable attempt may advance the simulation before its retry
        // deadline. It still waits for that deadline; only the first/reset
        // attempt has a -Infinity deadline and runs immediately.
        if (!hasSample || !hasCompletedAttempt) return now >= nextDueAt;
        if (safeTick === null) return false;
        return now >= nextDueAt;
      },
      complete(nowMs, tick, elapsedMs = 0, { retry: isRetry = false } = {}) {
        const now = Number(nowMs);
        const safeNow = Number.isFinite(now) ? now : 0;
        const tickNumber = (tick === null || tick === undefined || tick === '')
          ? Number.NaN : Number(tick);
        const safeTick = Number.isSafeInteger(tickNumber) && tickNumber >= 0
          ? tickNumber : null;
        const elapsed = Math.max(0, Number(elapsedMs) || 0);
        lastSampleTick = safeTick;
        hasCompletedAttempt = true;
        nextDueAt = safeNow + Math.max(isRetry ? retry : target, elapsed / duty);
      },
      reset() {
        nextDueAt = Number.NEGATIVE_INFINITY;
        lastSampleTick = null;
        hasCompletedAttempt = false;
      },
      get nextDueAt() { return nextDueAt; },
      get lastTick() { return lastSampleTick; },
      get targetIntervalMs() { return target; },
      get retryIntervalMs() { return retry; },
      get maxDutyCycle() { return duty; },
    };
  }

  /**
   * Visit the sampler wants that are due for this worker publication.
   *
   * Time and legacy instrument samplers use a bounded cadence; live Gravity
   * supplies an atomic per-state decision. The cadence class is resolved by the owner-set
   * union: any ordinary/direct/viewport co-owner upgrades that key to realtime.
   * gravityMetricAgg is special: telemetry demand is its scheduler owner. A
   * visible Time panel can therefore receive it without a second direct owner,
   * while hidden/Empty demand suppresses a briefly stale aggregate want.
   */
  function visitScheduledSamplers(wantedSamplers, {
    wantGravity = false,
    wantProperTime = false,
    cadence,
    nowMs,
    forceGravityBatch = false,
    // Gravity's atomic panel observation can be state-driven rather than
    // wall-clock-driven.  Its bounded sampler kinds then run only when the
    // caller has a new state to commit; realtime co-owners remain realtime.
    gravityPerState = false,
    forceProperTimeBatch = false,
    allowUndemandedBoundedInstrument = false,
  } = {}, visit = () => {}) {
    if (!wantedSamplers || typeof wantedSamplers.entries !== 'function') return false;
    if (!cadence || typeof cadence.shouldRun !== 'function') return false;

    let hasExplicitGravityMetricAgg = false;
    let hasBoundedSampler = false;
    let hasGravityBoundedSampler = false;
    let hasProperTimeBoundedSampler = false;
    for (const [, want] of wantedSamplers.entries()) {
      const kind = String(want?.kind || '');
      if (kind === 'gravityMetricAgg') {
        hasExplicitGravityMetricAgg = true;
        // A newly-added paused Gravity batch is allowed to populate atomically
        // before the controller's demand-mask message reaches this worker.
        if (wantGravity || allowUndemandedBoundedInstrument) {
          hasBoundedSampler = true;
          hasGravityBoundedSampler = true;
        }
      } else if (isBoundedInstrumentSamplerWant(want)
          && ((properTimeSamplerKindSet.has(kind) && (wantProperTime || allowUndemandedBoundedInstrument))
              || (!properTimeSamplerKindSet.has(kind) && (wantGravity || allowUndemandedBoundedInstrument)))) {
        hasBoundedSampler = true;
        if (properTimeSamplerKindSet.has(kind)) hasProperTimeBoundedSampler = true;
        if (!properTimeSamplerKindSet.has(kind)) hasGravityBoundedSampler = true;
      }
    }
    if (wantGravity && !hasExplicitGravityMetricAgg) {
      hasBoundedSampler = true;
      hasGravityBoundedSampler = true;
    }

    // A Gravity observation may be fresh for every completed worker state.
    // Its force must not drag tau/lapse/phase along at that rate: Time remains
    // an explicitly bounded instrument collector.  The ordinary cadence is
    // still evaluated so co-owned proper-time samplers retain their schedule.
    // A first Time-only demand remains immediately observable. Once Gravity
    // owns state-driven sampling, do not let its old 4 Hz timer run L/K/F on
    // unchanged paused state. Proper-time still has its own bounded schedule.
    const needsTimedBoundedBatch = hasBoundedSampler
      && (!gravityPerState || hasProperTimeBoundedSampler);
    const boundedBatchDue = needsTimedBoundedBatch
      ? cadence.shouldRun(nowMs, forceProperTimeBatch || (forceGravityBatch && !wantGravity))
      : false;
    const gravityBatchDue = gravityPerState
      ? (forceGravityBatch && wantGravity && hasGravityBoundedSampler)
      : (boundedBatchDue || (forceGravityBatch && wantGravity && hasGravityBoundedSampler));

    for (const [key, want] of wantedSamplers.entries()) {
      const kind = String(want?.kind || '');
      if (kind === 'gravityMetricAgg' && !wantGravity
          && !allowUndemandedBoundedInstrument) continue;
      if (isBoundedInstrumentSamplerWant(want)) {
        const wanted = properTimeSamplerKindSet.has(kind) ? wantProperTime : wantGravity;
        if (!wanted && !allowUndemandedBoundedInstrument) continue;
        const due = properTimeSamplerKindSet.has(kind) ? boundedBatchDue : gravityBatchDue;
        if (!due) continue;
      }
      if (kind === 'gravityMetricAgg' && !(wantGravity ? gravityBatchDue : boundedBatchDue)) continue;
      visit(key, want);
    }

    if (wantGravity && !hasExplicitGravityMetricAgg && gravityBatchDue) {
      visit(GRAVITY_METRIC_AGG_KEY, { kind: 'gravityMetricAgg', stride: 0 });
    }
    return gravityBatchDue || boundedBatchDue;
  }

  root.FTD_SAMPLER_CADENCE = Object.freeze({
    GRAVITY_SAMPLER_INTERVAL_MS,
    GRAVITY_METRIC_AGG_KEY,
    BOUNDED_INSTRUMENT_SAMPLER_KINDS,
    isBoundedInstrumentSamplerWant,
    createBoundedSamplerCadence,
    advanceDemandFrameCadence,
    createBoundedReductionCadence,
    visitScheduledSamplers,
  });
})(typeof self !== 'undefined' ? self : globalThis);
