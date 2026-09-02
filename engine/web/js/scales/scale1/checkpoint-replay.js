/**
 * Versioned Scale-1 checkpoint and deterministic replay coordinator.
 *
 * The native checkpoint owns physics state.  This module owns only browser
 * workflow state (saved checkpoint, replay start, JSON import/export) and an
 * exact structural digest used to verify that a replay reaches the same
 * effective record.  It does not infer physical equivalence.
 */

let savedCheckpoint = null;
let replayStart = null;

function cloneCheckpoint(checkpoint) {
    return checkpoint == null ? null : structuredClone(checkpoint);
}

function canonicalCheckpoint(checkpoint) {
    return JSON.stringify({
        schema: checkpoint?.schema,
        schemaVersion: checkpoint?.schemaVersion,
        native: checkpoint?.native,
        catalogTypes: checkpoint?.catalogTypes || [],
        finitePortBattery: checkpoint?.finitePortBattery || null,
    });
}

export function checkpointDigest(checkpoint) {
    const text = canonicalCheckpoint(checkpoint);
    let hash = 0x811c9dc5;
    for (let i = 0; i < text.length; i++) {
        hash ^= text.charCodeAt(i);
        hash = Math.imul(hash, 0x01000193) >>> 0;
    }
    return hash.toString(16).padStart(8, '0');
}

export function captureScale1Checkpoint(bridge) {
    const checkpoint = bridge?.peExportCheckpoint?.();
    if (!checkpoint) throw new Error('The active Scale 1 view cannot be checkpointed.');
    savedCheckpoint = cloneCheckpoint(checkpoint);
    return {
        checkpoint: cloneCheckpoint(savedCheckpoint),
        digest: checkpointDigest(savedCheckpoint),
        tick: Number(savedCheckpoint.native?.tick || 0),
    };
}

export function restoreSavedScale1Checkpoint(bridge) {
    if (!savedCheckpoint) throw new Error('No Scale 1 checkpoint has been captured.');
    if (!bridge?.peRestoreCheckpoint?.(cloneCheckpoint(savedCheckpoint))) {
        throw new Error('The native Scale 1 engine rejected the saved checkpoint.');
    }
    return {
        digest: checkpointDigest(savedCheckpoint),
        tick: Number(savedCheckpoint.native?.tick || 0),
    };
}

export function markScale1ReplayStart(bridge) {
    const result = captureScale1Checkpoint(bridge);
    replayStart = cloneCheckpoint(result.checkpoint);
    return result;
}

export async function verifyScale1Replay(bridge, { yieldEvery = 16 } = {}) {
    if (!replayStart) throw new Error('Mark a replay start before verifying a segment.');
    const expected = bridge?.peExportCheckpoint?.();
    if (!expected) throw new Error('The active Scale 1 view cannot be replayed.');
    const startTick = Number(replayStart.native?.tick || 0);
    const endTick = Number(expected.native?.tick || 0);
    const ticks = endTick - startTick;
    if (!Number.isSafeInteger(ticks) || ticks < 1) {
        throw new Error('Advance at least one particle-engine tick before replay verification.');
    }
    if (!bridge.peRestoreCheckpoint(cloneCheckpoint(replayStart))) {
        throw new Error('The native Scale 1 engine rejected the replay start.');
    }
    const stride = Math.max(1, Math.floor(yieldEvery));
    for (let tick = 0; tick < ticks; tick++) {
        bridge.peTick();
        if ((tick + 1) % stride === 0) {
            await new Promise(resolve => setTimeout(resolve, 0));
        }
    }
    const actual = bridge.peExportCheckpoint();
    const expectedDigest = checkpointDigest(expected);
    const actualDigest = checkpointDigest(actual);
    return {
        match: actualDigest === expectedDigest,
        ticks,
        startTick,
        endTick,
        expectedDigest,
        actualDigest,
    };
}

export function serializeScale1Checkpoint(checkpoint = savedCheckpoint) {
    if (!checkpoint) throw new Error('No Scale 1 checkpoint has been captured.');
    return `${JSON.stringify(checkpoint, null, 2)}\n`;
}

export function importScale1Checkpoint(text, bridge, { remember = true } = {}) {
    const checkpoint = JSON.parse(String(text));
    if (checkpoint?.schema !== 'ftd.scale1.dashboard-checkpoint'
        || checkpoint?.schemaVersion !== 1 || !checkpoint?.native) {
        throw new TypeError('Unsupported Scale 1 dashboard checkpoint.');
    }
    if (!bridge?.peRestoreCheckpoint?.(cloneCheckpoint(checkpoint))) {
        throw new Error('The native Scale 1 engine rejected the imported checkpoint.');
    }
    if (remember) savedCheckpoint = cloneCheckpoint(checkpoint);
    return {
        digest: checkpointDigest(checkpoint),
        tick: Number(checkpoint.native.tick || 0),
    };
}

export function clearScale1CheckpointSession() {
    savedCheckpoint = null;
    replayStart = null;
}

export function getScale1CheckpointSession() {
    return {
        hasSavedCheckpoint: !!savedCheckpoint,
        hasReplayStart: !!replayStart,
        savedTick: savedCheckpoint ? Number(savedCheckpoint.native?.tick || 0) : null,
        replayStartTick: replayStart ? Number(replayStart.native?.tick || 0) : null,
    };
}
