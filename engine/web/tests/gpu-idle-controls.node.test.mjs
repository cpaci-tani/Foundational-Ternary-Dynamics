import test from 'node:test';
import assert from 'node:assert/strict';

// Idle auto-stop + "stop all engine servers" browser-side pieces (spec
// docs/superpowers/specs/2026-09-16-idle-shutdown-and-kill-all.md, §4/§5/§6):
//   - shouldHeartbeat: the pure cadence decision ws-bridge.js uses for its
//     native `{cmd:'ping'}` pulse and its `/api/heartbeat` dev-server pulse.
//   - the GPU-server card's idle-policy settings: localStorage round-trip
//     and out-of-range-minutes clamping.
import { shouldHeartbeat } from '../js/lib/heartbeat.js';
import {
    IDLE_MINUTES_MIN,
    IDLE_MINUTES_MAX,
    IDLE_MINUTES_DEFAULT,
    IDLE_ENABLED_DEFAULT,
    clampIdleMinutes,
    readStoredIdleSettings,
    writeStoredIdleSettings,
} from '../js/lib/idle-policy-settings.js';

// A minimal Storage-like double — no DOM/browser needed to exercise the
// round-trip logic.
function fakeStorage(initial = {}) {
    const map = new Map(Object.entries(initial));
    return {
        getItem(key) { return map.has(key) ? map.get(key) : null; },
        setItem(key, value) { map.set(key, String(value)); },
        removeItem(key) { map.delete(key); },
        _map: map,
    };
}

function throwingStorage() {
    return {
        getItem() { throw new Error('storage disabled'); },
        setItem() { throw new Error('storage disabled'); },
    };
}

test('shouldHeartbeat: never fires while hidden, regardless of elapsed time', () => {
    assert.equal(shouldHeartbeat(1_000_000, 0, false, 60_000), false);
    assert.equal(shouldHeartbeat(1_000_000, -Infinity, false, 60_000), false);
});

test('shouldHeartbeat: fires immediately the first time (no prior send) while visible', () => {
    assert.equal(shouldHeartbeat(0, -Infinity, true, 60_000), true);
    assert.equal(shouldHeartbeat(0, NaN, true, 60_000), true);
});

test('shouldHeartbeat: honours the interval — not due before it elapses, due at and after it', () => {
    const last = 10_000, interval = 60_000;
    assert.equal(shouldHeartbeat(last + interval - 1, last, true, interval), false, 'one ms early: not due');
    assert.equal(shouldHeartbeat(last + interval, last, true, interval), true, 'exactly at interval: due');
    assert.equal(shouldHeartbeat(last + interval + 5_000, last, true, interval), true, 'well past interval: due');
});

test('shouldHeartbeat: a visibility flip from hidden back to visible resumes the same clock (no forced resend)', () => {
    const last = 10_000, interval = 60_000;
    // Went hidden shortly after sending — still not due.
    assert.equal(shouldHeartbeat(last + 1_000, last, false, interval), false);
    // Comes back visible before the interval elapses: still not due yet.
    assert.equal(shouldHeartbeat(last + 30_000, last, true, interval), false);
    // ...and due once the same interval from the last real send has elapsed.
    assert.equal(shouldHeartbeat(last + interval, last, true, interval), true);
});

test('idle-policy-settings: exposes the same bounds server_controls.py enforces', () => {
    assert.equal(IDLE_MINUTES_MIN, 1);
    assert.equal(IDLE_MINUTES_MAX, 1440);
    assert.equal(IDLE_MINUTES_DEFAULT, 30);
    assert.equal(IDLE_ENABLED_DEFAULT, true);
});

test('clampIdleMinutes: clamps out-of-range, non-numeric, and fractional input', () => {
    assert.equal(clampIdleMinutes(0), 1);
    assert.equal(clampIdleMinutes(-5), 1);
    assert.equal(clampIdleMinutes(1441), 1440);
    assert.equal(clampIdleMinutes(999999), 1440);
    assert.equal(clampIdleMinutes(45.9), 45);
    assert.equal(clampIdleMinutes('30'), 30);
    assert.equal(clampIdleMinutes('not-a-number'), IDLE_MINUTES_DEFAULT);
    assert.equal(clampIdleMinutes(NaN), IDLE_MINUTES_DEFAULT);
    assert.equal(clampIdleMinutes(undefined), IDLE_MINUTES_DEFAULT);
    assert.equal(clampIdleMinutes(1), 1);
    assert.equal(clampIdleMinutes(1440), 1440);
});

test('idle settings round-trip through storage unchanged when already valid', () => {
    const storage = fakeStorage();
    const written = writeStoredIdleSettings(storage, { enabled: false, minutes: 45 });
    assert.deepEqual(written, { enabled: false, minutes: 45 });
    const read = readStoredIdleSettings(storage);
    assert.deepEqual(read, { enabled: false, minutes: 45 });
});

test('idle settings round-trip clamps an out-of-range minutes value on write', () => {
    const storage = fakeStorage();
    const written = writeStoredIdleSettings(storage, { enabled: true, minutes: 5000 });
    assert.equal(written.minutes, 1440);
    const read = readStoredIdleSettings(storage);
    assert.equal(read.minutes, 1440);
    assert.equal(read.enabled, true);
});

test('idle settings round-trip clamps a corrupted stored value on read (defensive read path)', () => {
    const storage = fakeStorage({
        'ftd-gpu-idle-enabled': '1',
        'ftd-gpu-idle-minutes': '-17',
    });
    const read = readStoredIdleSettings(storage);
    assert.equal(read.enabled, true);
    assert.equal(read.minutes, 1); // clamped, not silently accepted
});

test('idle settings default to enabled=true, minutes=30 when nothing is stored yet', () => {
    const storage = fakeStorage();
    const read = readStoredIdleSettings(storage);
    assert.deepEqual(read, { enabled: true, minutes: 30 });
});

test('idle settings degrade to defaults, not a throw, when storage access itself throws', () => {
    const storage = throwingStorage();
    assert.doesNotThrow(() => readStoredIdleSettings(storage));
    const read = readStoredIdleSettings(storage);
    assert.deepEqual(read, { enabled: true, minutes: 30 });
    assert.doesNotThrow(() => writeStoredIdleSettings(storage, { enabled: false, minutes: 10 }));
});
