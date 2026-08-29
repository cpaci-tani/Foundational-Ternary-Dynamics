// @ts-check
/**
 * Browser-side instrumentation for the gated Scale 0 UI audit.
 *
 * This module is served only from /tests and is never imported by production.
 * It measures the browser frame loop, Long Tasks, selected rafCoordinator
 * callbacks, DOM/canvas work inside one interface root, method request counts,
 * action-to-next-rAF latency, and coarse lifecycle/resource deltas.
 */

import { rafCoordinator } from '../js/lib/raf-coordinator.js';

const CANVAS_METHODS = [
    'clearRect', 'drawImage', 'fill', 'fillRect', 'putImageData', 'stroke', 'strokeRect',
];

function percentile(sorted, q) {
    if (!sorted.length) return 0;
    const index = Math.min(sorted.length - 1, Math.max(0, Math.ceil(sorted.length * q) - 1));
    return sorted[index];
}

function summarize(samples) {
    const sorted = samples.slice().sort((a, b) => a - b);
    const sum = sorted.reduce((acc, value) => acc + value, 0);
    return {
        count: sorted.length,
        meanMs: sorted.length ? sum / sorted.length : 0,
        medianMs: percentile(sorted, 0.5),
        p95Ms: percentile(sorted, 0.95),
        p99Ms: percentile(sorted, 0.99),
        maxMs: sorted.length ? sorted[sorted.length - 1] : 0,
    };
}

function snapshotResources(root) {
    return {
        rafSubscribers: rafCoordinator.size(),
        domNodes: root ? root.querySelectorAll('*').length + 1 : 0,
        canvases: root ? root.querySelectorAll('canvas').length : 0,
        heapBytes: Number(performance.memory?.usedJSHeapSize) || 0,
    };
}

function getProbe() {
    const probe = window.__ftdScale0UiAuditProbe;
    if (!probe?.running) throw new Error('Scale 0 UI audit probe is not running');
    return probe;
}

function subscriberMatches(probe, id) {
    return probe.subscriberIds.has(id)
        || probe.subscriberPrefixes.some((prefix) => id.startsWith(prefix));
}

function wrapMatchingSubscribers(probe) {
    for (const [id, sub] of rafCoordinator._subs.entries()) {
        if (!subscriberMatches(probe, id) || probe.wrappedSubscribers.has(sub)) continue;
        const original = sub.cb;
        const wrapped = (...args) => {
            const start = performance.now();
            try {
                return original(...args);
            } finally {
                const elapsed = performance.now() - start;
                let samples = probe.callbackSamples.get(id);
                if (!samples) {
                    samples = [];
                    probe.callbackSamples.set(id, samples);
                }
                samples.push(elapsed);
            }
        };
        probe.wrappedSubscribers.set(sub, { original, wrapped });
        sub.cb = wrapped;
    }
}

function installCanvasCounters(probe) {
    if (typeof CanvasRenderingContext2D === 'undefined') return;
    for (const method of CANVAS_METHODS) {
        const original = CanvasRenderingContext2D.prototype[method];
        if (typeof original !== 'function') continue;
        const wrapped = function (...args) {
            const canvas = this.canvas;
            if (!probe.root || (canvas instanceof Element && probe.root.contains(canvas))) {
                probe.canvasDraws += 1;
            }
            return original.apply(this, args);
        };
        probe.canvasOriginals.set(method, original);
        CanvasRenderingContext2D.prototype[method] = wrapped;
    }
}

function restoreProbe(probe) {
    cancelAnimationFrame(probe.rafId);
    probe.longTaskObserver?.disconnect();
    probe.mutationObserver?.disconnect();
    window.removeEventListener('error', probe.onError);
    window.removeEventListener('unhandledrejection', probe.onUnhandledRejection);

    for (const [sub, entry] of probe.wrappedSubscribers.entries()) {
        if (sub.cb === entry.wrapped) sub.cb = entry.original;
    }
    for (const restore of probe.methodRestores) restore();
    if (typeof CanvasRenderingContext2D !== 'undefined') {
        for (const [method, original] of probe.canvasOriginals.entries()) {
            CanvasRenderingContext2D.prototype[method] = original;
        }
    }
}

/**
 * @param {{
 *   rootSelector?: string,
 *   subscriberIds?: string[],
 *   subscriberPrefixes?: string[],
 * }} [options]
 */
export function startScale0UiAuditProbe(options = {}) {
    if (window.__ftdScale0UiAuditProbe?.running) {
        throw new Error('Scale 0 UI audit probe is already running');
    }

    const root = options.rootSelector ? document.querySelector(options.rootSelector) : null;
    if (options.rootSelector && !root) {
        throw new Error(`Scale 0 UI audit root not found: ${options.rootSelector}`);
    }

    const longTaskSupported = typeof PerformanceObserver !== 'undefined'
        && PerformanceObserver.supportedEntryTypes?.includes('longtask');
    const probe = {
        running: true,
        root,
        startedAt: performance.now(),
        stoppedAt: 0,
        lastFrameAt: 0,
        rafId: 0,
        frameDeltas: [],
        longTasks: [],
        longTaskSupported,
        callbackSamples: new Map(),
        subscriberIds: new Set(options.subscriberIds || []),
        subscriberPrefixes: options.subscriberPrefixes || [],
        wrappedSubscribers: new Map(),
        methodCounts: new Map(),
        methodRestores: [],
        actionLatencies: [],
        mutationRecords: 0,
        addedNodes: 0,
        removedNodes: 0,
        canvasDraws: 0,
        errors: [],
        canvasOriginals: new Map(),
        mutationObserver: null,
        longTaskObserver: null,
        resourcesStart: snapshotResources(root),
        onError: (event) => probe.errors.push(`error: ${event.message || 'unknown'}`),
        onUnhandledRejection: (event) => probe.errors.push(`unhandledrejection: ${String(event.reason)}`),
    };

    if (longTaskSupported) {
        probe.longTaskObserver = new PerformanceObserver((list) => {
            for (const entry of list.getEntries()) {
                probe.longTasks.push({ startTime: entry.startTime, duration: entry.duration });
            }
        });
        probe.longTaskObserver.observe({ entryTypes: ['longtask'] });
    }

    if (root) {
        probe.mutationObserver = new MutationObserver((records) => {
            probe.mutationRecords += records.length;
            for (const record of records) {
                probe.addedNodes += record.addedNodes?.length || 0;
                probe.removedNodes += record.removedNodes?.length || 0;
            }
        });
        probe.mutationObserver.observe(root, {
            attributes: true,
            characterData: true,
            childList: true,
            subtree: true,
        });
    }

    window.addEventListener('error', probe.onError);
    window.addEventListener('unhandledrejection', probe.onUnhandledRejection);
    installCanvasCounters(probe);

    const frame = (now) => {
        if (!probe.running) return;
        if (probe.lastFrameAt > 0) probe.frameDeltas.push(now - probe.lastFrameAt);
        probe.lastFrameAt = now;
        wrapMatchingSubscribers(probe);
        probe.rafId = requestAnimationFrame(frame);
    };
    probe.rafId = requestAnimationFrame(frame);
    window.__ftdScale0UiAuditProbe = probe;
    return snapshotResources(root);
}

/** Track calls to named methods on a live bridge/capability/panel object. */
export function trackScale0UiMethods(label, target, methodNames) {
    const probe = getProbe();
    if (!target) throw new Error(`Cannot track methods on missing target: ${label}`);

    for (const method of methodNames) {
        const original = target[method];
        if (typeof original !== 'function') continue;
        const key = `${label}.${method}`;
        target[method] = function (...args) {
            probe.methodCounts.set(key, (probe.methodCounts.get(key) || 0) + 1);
            return original.apply(this, args);
        };
        probe.methodRestores.push(() => {
            if (target[method] !== original) target[method] = original;
        });
    }
}

/** Measure synchronous UI action dispatch to the next foreground rAF. */
export async function measureScale0UiActionToPaint(label, action) {
    const probe = getProbe();
    const startedAt = performance.now();
    action();
    const paintedAt = await new Promise((resolve) => {
        requestAnimationFrame(() => resolve(performance.now()));
    });
    const duration = paintedAt - startedAt;
    probe.actionLatencies.push({ label, duration });
    return duration;
}

export async function stopScale0UiAuditProbe() {
    const probe = getProbe();
    probe.running = false;
    probe.stoppedAt = performance.now();
    await new Promise((resolve) => setTimeout(resolve, 0));

    const frameSummary = summarize(probe.frameDeltas);
    const callbackSummary = {};
    for (const [id, samples] of probe.callbackSamples.entries()) {
        callbackSummary[id] = summarize(samples);
    }
    const actionValues = probe.actionLatencies.map((entry) => entry.duration);
    const resourcesEnd = snapshotResources(probe.root);
    const report = {
        durationMs: probe.stoppedAt - probe.startedAt,
        frames: {
            ...frameSummary,
            effectiveFps: frameSummary.meanMs > 0 ? 1000 / frameSummary.meanMs : 0,
            intervalsOver20ms: probe.frameDeltas.filter((value) => value > 20).length,
            intervalsOver33_4ms: probe.frameDeltas.filter((value) => value > 33.4).length,
        },
        longTaskSupported: probe.longTaskSupported,
        longTasks: probe.longTasks.slice(),
        callbacks: callbackSummary,
        methods: Object.fromEntries(probe.methodCounts.entries()),
        actions: {
            ...summarize(actionValues),
            samples: probe.actionLatencies.slice(),
        },
        dom: {
            mutationRecords: probe.mutationRecords,
            addedNodes: probe.addedNodes,
            removedNodes: probe.removedNodes,
            canvasDraws: probe.canvasDraws,
        },
        resourcesStart: probe.resourcesStart,
        resourcesEnd,
        resourceDelta: {
            rafSubscribers: resourcesEnd.rafSubscribers - probe.resourcesStart.rafSubscribers,
            domNodes: resourcesEnd.domNodes - probe.resourcesStart.domNodes,
            canvases: resourcesEnd.canvases - probe.resourcesStart.canvases,
            heapBytes: resourcesEnd.heapBytes && probe.resourcesStart.heapBytes
                ? resourcesEnd.heapBytes - probe.resourcesStart.heapBytes
                : 0,
        },
        errors: probe.errors.slice(),
    };

    restoreProbe(probe);
    window.__ftdScale0UiAuditProbe = null;
    return report;
}
