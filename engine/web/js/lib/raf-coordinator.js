/**
 * Single rAF coordinator for all dashboard panels.
 *
 * Pre-existing panels each set up their own self-driving rAF loop
 * (flux-slice-panel.js, p1-observables-panel.js). With 5+ panels each
 * running rAF + per-frame `performance.now()` checks, we get:
 *   - N independent throttle clocks drifting from each other
 *   - N rAF callbacks queued every frame
 *   - no shared visibility / pause logic
 *
 * This module provides one coordinator with a subscribe API:
 *
 *   import { rafCoordinator } from '../../lib/raf-coordinator.js';
 *   const sub = rafCoordinator.subscribe('my-panel', { hz: 4, cb: () => panel.update() });
 *   // ... later
 *   sub.unsubscribe();
 *
 * Behavior:
 *   - Single requestAnimationFrame loop runs while any subscriber exists
 *   - Each subscriber declares its own Hz; coordinator schedules nextDueAt
 *   - On document.hidden, all 4-Hz subscribers pause; 60-Hz keep ticking
 *     (so 3D viewport doesn't freeze)
 *   - cb() exceptions are caught + console.warn'd; one bad subscriber
 *     doesn't kill the loop
 */

const VISIBILITY_PAUSE_THRESHOLD_HZ = 30;   // subscribers slower than this pause when hidden
const ERROR_BUDGET = 10;                    // consecutive throws before auto-unsubscribe

class RAFCoordinator {
    constructor() {
        this._subs = new Map();             // id → { hz, cb, lastFireAt, nextDueAt }
        this._rafId = null;
        this._lastTickMs = 0;
        this._hidden = (typeof document !== 'undefined') && document.hidden;
        if (typeof document !== 'undefined') {
            document.addEventListener('visibilitychange', () => {
                this._hidden = document.hidden;
                // Reset due-times so high-Hz subscribers don't burst on resume
                if (!this._hidden) {
                    const now = performance.now();
                    for (const sub of this._subs.values()) {
                        sub.nextDueAt = now + (1000 / sub.hz);
                    }
                }
            });
        }
    }

    /**
     * Register a subscriber. Returns an object with `.unsubscribe()`.
     *
     * @param {string} id   - unique identifier (warning logged on duplicate; replaces)
     * @param {object} opts
     * @param {number} opts.hz   - target frequency in Hz; clamped to (0, 60]
     * @param {Function} opts.cb - callback; called with (deltaMs) from last fire
     */
    subscribe(id, { hz = 4, cb }) {
        if (typeof cb !== 'function') {
            throw new Error('rafCoordinator.subscribe: cb must be a function');
        }
        const clampedHz = Math.max(0.1, Math.min(60, hz));
        const now = performance.now();
        if (this._subs.has(id)) {
            // eslint-disable-next-line no-console
            console.warn(`[rafCoordinator] re-subscribing ${id}; previous registration replaced`);
        }
        this._subs.set(id, {
            id,
            hz: clampedHz,
            cb,
            lastFireAt: now,
            nextDueAt: now + (1000 / clampedHz),
            errorStreak: 0,    // RAF-3 audit pass 2: auto-unsub after ERROR_BUDGET
        });
        this._ensureRunning();
        return {
            unsubscribe: () => {
                this._subs.delete(id);
                if (this._subs.size === 0) this._stop();
            },
        };
    }

    _ensureRunning() {
        if (this._rafId != null) return;
        this._lastTickMs = performance.now();
        const loop = () => {
            this._rafId = requestAnimationFrame(loop);
            this._tick();
        };
        this._rafId = requestAnimationFrame(loop);
    }

    _stop() {
        if (this._rafId != null) {
            cancelAnimationFrame(this._rafId);
            this._rafId = null;
        }
    }

    _tick() {
        const now = performance.now();
        this._lastTickMs = now;
        for (const sub of this._subs.values()) {
            // When tab hidden, suspend all subs slower than threshold.
            if (this._hidden && sub.hz < VISIBILITY_PAUSE_THRESHOLD_HZ) continue;
            if (now < sub.nextDueAt) continue;
            const deltaMs = now - sub.lastFireAt;
            sub.lastFireAt = now;
            // Schedule next fire — use additive scheduling clamped to 'now'
            // so a slow callback doesn't permanently lag the cadence.
            sub.nextDueAt = Math.max(now + 1, sub.nextDueAt + (1000 / sub.hz));
            try {
                sub.cb(deltaMs);
                sub.errorStreak = 0;
            } catch (err) {
                sub.errorStreak += 1;
                // eslint-disable-next-line no-console
                console.warn(`[rafCoordinator] ${sub.id} cb threw (${sub.errorStreak}/${ERROR_BUDGET}):`, err);
                // RAF-3 audit pass 2: a callback that throws every frame
                // logs every frame and pins the closure forever. After
                // ERROR_BUDGET consecutive throws, drop the subscriber so
                // the loop self-heals.
                if (sub.errorStreak >= ERROR_BUDGET) {
                    // eslint-disable-next-line no-console
                    console.error(`[rafCoordinator] auto-unsubscribing ${sub.id} after ${ERROR_BUDGET} consecutive throws`);
                    this._subs.delete(sub.id);
                }
            }
        }
        if (this._subs.size === 0) this._stop();
    }

    /** Diagnostic: returns subscriber count (for tests / debug). */
    size() {
        return this._subs.size;
    }

    /**
     * Drop every subscriber and stop the rAF loop. Useful for HMR /
     * test-environment teardown. Safe to call when no subs exist.
     * (RAF-4 audit pass 2.)
     */
    clear() {
        this._subs.clear();
        this._stop();
    }
}

export const rafCoordinator = new RAFCoordinator();

// Expose to window for console-debugging.
if (typeof window !== 'undefined') {
    window.__ftdRAF = rafCoordinator;
}
