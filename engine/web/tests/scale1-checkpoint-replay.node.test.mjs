import test from 'node:test';
import assert from 'node:assert/strict';
import {
    captureScale1Checkpoint,
    checkpointDigest,
    importScale1Checkpoint,
    markScale1ReplayStart,
    restoreSavedScale1Checkpoint,
    serializeScale1Checkpoint,
    verifyScale1Replay,
    clearScale1CheckpointSession,
} from '../js/scales/scale1/checkpoint-replay.js';

function fakeBridge() {
    let state = { tick: 3, x: 7 };
    return {
        peExportCheckpoint() {
            return {
                schema: 'ftd.scale1.dashboard-checkpoint', schemaVersion: 1,
                capturedAt: 'ignored', catalogTypes: [[0, 'electron']],
                native: { tick: state.tick, x: state.x },
            };
        },
        peRestoreCheckpoint(checkpoint) {
            state = { tick: checkpoint.native.tick, x: checkpoint.native.x };
            return true;
        },
        peTick() { state.tick++; state.x = state.x * 3 + 1; },
        state: () => ({ ...state }),
    };
}

test('checkpoint capture, JSON import, and restore preserve native state', () => {
    clearScale1CheckpointSession();
    const bridge = fakeBridge();
    const captured = captureScale1Checkpoint(bridge);
    bridge.peTick();
    restoreSavedScale1Checkpoint(bridge);
    assert.deepEqual(bridge.state(), { tick: 3, x: 7 });
    const json = serializeScale1Checkpoint();
    bridge.peTick();
    const imported = importScale1Checkpoint(json, bridge);
    assert.equal(imported.digest, captured.digest);
    assert.deepEqual(bridge.state(), { tick: 3, x: 7 });
});

test('replay verifier reproduces the exact captured end record', async () => {
    clearScale1CheckpointSession();
    const bridge = fakeBridge();
    markScale1ReplayStart(bridge);
    bridge.peTick();
    bridge.peTick();
    const result = await verifyScale1Replay(bridge, { yieldEvery: 1 });
    assert.equal(result.match, true);
    assert.equal(result.ticks, 2);
    assert.deepEqual(bridge.state(), { tick: 5, x: 67 });
});

test('checkpoint digest ignores capture wall time but detects physics changes', () => {
    const bridge = fakeBridge();
    const first = bridge.peExportCheckpoint();
    const second = structuredClone(first);
    second.capturedAt = 'later';
    assert.equal(checkpointDigest(first), checkpointDigest(second));
    second.native.x++;
    assert.notEqual(checkpointDigest(first), checkpointDigest(second));
});
