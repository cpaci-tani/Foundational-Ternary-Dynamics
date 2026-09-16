/**
 * Pure, testable heartbeat-cadence decision shared by ws-bridge.js's native
 * `{cmd:'ping'}` pulse and its `/api/heartbeat` dev-server pulse (spec
 * §5, `docs/superpowers/specs/2026-09-16-idle-shutdown-and-kill-all.md`).
 *
 * Kept free of DOM/WebSocket/timer globals so Node's test runner can import
 * and exercise it directly (`engine/web/tests/gpu-idle-controls.node.test.mjs`).
 * The caller supplies `nowMs`/`visible` from whatever clock and
 * `document.visibilityState` it has on hand; this module makes no calls of
 * its own.
 */

/**
 * @param {number} nowMs - current time, any monotonic millisecond clock.
 * @param {number} lastSentMs - time of the last heartbeat actually sent, or
 *   a non-finite/negative sentinel (e.g. -Infinity) meaning "never sent".
 * @param {boolean} visible - true while the page counts as active (not a
 *   backgrounded/hidden tab).
 * @param {number} intervalMs - minimum spacing between heartbeats.
 * @returns {boolean} true exactly when a heartbeat should be sent now.
 */
export function shouldHeartbeat(nowMs, lastSentMs, visible, intervalMs) {
    if (!visible) return false;
    if (!Number.isFinite(lastSentMs) || lastSentMs < 0) return true;
    return (nowMs - lastSentMs) >= intervalMs;
}
