/**
 * Pure helpers for the GPU-server card's "stop servers when idle" settings.
 *
 * Same validation shape as the Python dev server's `IdlePolicy`
 * (`engine/web/server_controls.py`: minutes clamp to 1..1440) so the browser
 * and the dev server never disagree about what a valid setting looks like.
 *
 * `gpu-server-card.js` is deliberately a classic (non-module) script with no
 * imports (see its own header comment), so it keeps an inline copy of this
 * same small amount of logic rather than importing this file. This module
 * exists so that logic has a canonical, Node-importable, unit-tested
 * definition — keep the two in sync when either changes.
 */

export const IDLE_MINUTES_MIN = 1;
export const IDLE_MINUTES_MAX = 1440;
export const IDLE_MINUTES_DEFAULT = 30;
export const IDLE_ENABLED_DEFAULT = true;
export const IDLE_ENABLED_STORAGE_KEY = 'ftd-gpu-idle-enabled';
export const IDLE_MINUTES_STORAGE_KEY = 'ftd-gpu-idle-minutes';

/** Clamp any input to an integer in [IDLE_MINUTES_MIN, IDLE_MINUTES_MAX]. */
export function clampIdleMinutes(value, fallback = IDLE_MINUTES_DEFAULT) {
    const n = Math.trunc(Number(value));
    if (!Number.isFinite(n)) return fallback;
    return Math.min(IDLE_MINUTES_MAX, Math.max(IDLE_MINUTES_MIN, n));
}

/**
 * Read {enabled, minutes} back from a Storage-like object (localStorage or a
 * test double implementing getItem/setItem). Missing/invalid keys fall back
 * to defaults; a throwing storage (private-mode, disabled site data) is
 * treated the same as "nothing stored" rather than propagating.
 */
export function readStoredIdleSettings(storage) {
    let enabled = IDLE_ENABLED_DEFAULT;
    let minutes = IDLE_MINUTES_DEFAULT;
    try {
        const rawEnabled = storage.getItem(IDLE_ENABLED_STORAGE_KEY);
        if (rawEnabled !== null && rawEnabled !== undefined) enabled = rawEnabled === '1';
        const rawMinutes = storage.getItem(IDLE_MINUTES_STORAGE_KEY);
        if (rawMinutes !== null && rawMinutes !== undefined) minutes = clampIdleMinutes(rawMinutes);
    } catch (_e) {
        // Storage unavailable — defaults stand.
    }
    return { enabled, minutes };
}

/**
 * Write {enabled, minutes} to a Storage-like object, clamping minutes first
 * so a round-trip always yields a valid value even if the caller passed
 * something out of range. Returns the normalized values actually intended
 * (independent of whether the write itself succeeded).
 */
export function writeStoredIdleSettings(storage, { enabled, minutes }) {
    const normalized = { enabled: !!enabled, minutes: clampIdleMinutes(minutes) };
    try {
        storage.setItem(IDLE_ENABLED_STORAGE_KEY, normalized.enabled ? '1' : '0');
        storage.setItem(IDLE_MINUTES_STORAGE_KEY, String(normalized.minutes));
    } catch (_e) {
        // Best-effort only — the caller still gets the normalized values back.
    }
    return normalized;
}
