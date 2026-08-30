#!/usr/bin/env node

import assert from 'node:assert/strict';

const url = process.argv[2] || 'ws://127.0.0.1:9191';
const socket = new WebSocket(url);
socket.binaryType = 'arraybuffer';

const inbox = [];
const waiters = [];
let nextRequestId = 1;

function classify(data) {
    if (typeof data === 'string') {
        try {
            return { type: 'json', value: JSON.parse(data) };
        } catch {
            return { type: 'text', value: data };
        }
    }
    if (data instanceof ArrayBuffer)
        return { type: 'binary', value: data };
    if (ArrayBuffer.isView(data))
        return { type: 'binary', value: data.buffer };
    return { type: 'other', value: data };
}

function dispatch(message) {
    for (let i = 0; i < waiters.length; i++) {
        if (waiters[i].predicate(message)) {
            const waiter = waiters.splice(i, 1)[0];
            clearTimeout(waiter.timer);
            waiter.resolve(message);
            return;
        }
    }
    inbox.push(message);
}

socket.addEventListener('message', (event) => {
    const message = classify(event.data);
    if (process.env.FTD_WS_SMOKE_DEBUG)
        console.error('WS <=', message.type, message.value);
    dispatch(message);
});

function waitFor(predicate, description, timeoutMs = 20000) {
    const queued = inbox.findIndex(predicate);
    if (queued >= 0)
        return Promise.resolve(inbox.splice(queued, 1)[0]);
    return new Promise((resolve, reject) => {
        const waiter = { predicate, resolve, timer: null };
        waiter.timer = setTimeout(() => {
            const index = waiters.indexOf(waiter);
            if (index >= 0) waiters.splice(index, 1);
            reject(new Error(`Timed out waiting for ${description}`));
        }, timeoutMs);
        waiters.push(waiter);
    });
}

async function request(payload, timeoutMs = 20000) {
    const requestId = nextRequestId++;
    socket.send(JSON.stringify({ ...payload, _requestId: requestId }));
    const message = await waitFor(
        (candidate) => candidate.type === 'json'
            && candidate.value._requestId === requestId,
        `${payload.cmd} response`, timeoutMs);
    return message.value;
}

async function requestBinary(payload, predicate, timeoutMs = 20000) {
    const pending = waitFor(
        (candidate) => candidate.type === 'binary' && predicate(candidate.value),
        `${payload.cmd} binary response`, timeoutMs);
    socket.send(JSON.stringify(payload));
    return (await pending).value;
}

function u32(view, byteOffset) {
    return view.getUint32(byteOffset, true);
}

function assertCanonicalDigestShape(digest, expected = {}) {
    assert.equal(digest.type, 'dynamical_state_digest');
    assert.equal(digest.schemaVersion, 1);
    assert.equal(digest.latticeSize, expected.latticeSize);
    assert.equal(digest.siteCount, expected.latticeSize ** 3);
    assert(Number.isSafeInteger(digest.tick));
    assert(Number.isSafeInteger(digest.stateVersion));
    assert(Number.isSafeInteger(digest.sourceEpoch));
    assert.equal(digest.telemetrySourceEpoch, digest.sourceEpoch);
    assert.match(digest.hashLo, /^[0-9a-f]{16}$/);
    assert.match(digest.hashHi, /^[0-9a-f]{16}$/);
    assert(Number.isSafeInteger(digest.nonfiniteValueCount));
    assert(Number.isSafeInteger(digest.nondefaultValueCount));
    assert.equal(digest.deviceToHostBytes, 32,
        'native CUDA digest must copy only the fixed accumulator');
    assert.equal(digest.fullMirrorCalls, 0,
        'canonical digest must not materialize the full CUDA voxel mirror');
    assert.equal(digest.compute, 'GPU');
    assert.equal(digest.runtime, 'native');
    assert.equal(digest.transport, 'websocket');
}

const FTV2 = Object.freeze({
    MAGIC: 0x32565446,
    HEADER_BYTES: 20,
    LATTICE_SIZE: 4,
    STRIDE: 8,
    ORIGIN: 12,
    AXIS_COUNT: 16,
});

function centeredGrid(latticeSize, stride) {
    const center = Math.floor((latticeSize - 1) / 2);
    const origin = center - Math.floor(center / stride) * stride;
    const axisCount = Math.floor((latticeSize - 1 - origin) / stride) + 1;
    return { origin, axisCount };
}

function ftv2Stats(buffer) {
    const view = new DataView(buffer);
    assert(buffer.byteLength >= FTV2.HEADER_BYTES, 'FTV2 header size');
    assert.equal(u32(view, 0), FTV2.MAGIC, 'FTV2 magic');
    const latticeSize = u32(view, FTV2.LATTICE_SIZE);
    const stride = u32(view, FTV2.STRIDE);
    const origin = u32(view, FTV2.ORIGIN);
    const axisCount = u32(view, FTV2.AXIS_COUNT);
    const expectedGrid = centeredGrid(latticeSize, stride);
    assert.equal(origin, expectedGrid.origin, 'FTV2 center-anchored origin');
    assert.equal(axisCount, expectedGrid.axisCount, 'FTV2 center-anchored axis count');
    const count = axisCount ** 3;
    assert.equal(buffer.byteLength, FTV2.HEADER_BYTES + count * 4, 'FTV2 payload size');
    let maxAbs = 0;
    for (let i = 0; i < count; i++)
        maxAbs = Math.max(maxAbs, Math.abs(view.getFloat32(FTV2.HEADER_BYTES + i * 4, true)));
    return { latticeSize, stride, origin, axisCount, count, maxAbs };
}

function fts2Stats(buffer, expected = {}) {
    const view = new DataView(buffer);
    assert.equal(u32(view, 0), 0x32535446, 'FTS2 magic');
    if (expected.token !== undefined) assert.equal(u32(view, 4), expected.token, 'FTS2 token');
    if (expected.kindCode !== undefined) assert.equal(u32(view, 8), expected.kindCode, 'FTS2 kind');
    const components = u32(view, 12);
    const count = u32(view, 16);
    const effectiveStride = u32(view, 20);
    const origin = u32(view, 24);
    const expectedBytes = 28 + count * (3 + components) * 4;
    assert.equal(buffer.byteLength, expectedBytes, 'FTS2 payload size');
    const payloadStart = 28 + count * 3 * 4;
    let maxAbs = 0;
    for (let i = 0; i < count * components; i++)
        maxAbs = Math.max(maxAbs, Math.abs(view.getFloat32(payloadStart + i * 4, true)));
    return { components, count, effectiveStride, origin, maxAbs };
}

await new Promise((resolve, reject) => {
    socket.addEventListener('open', resolve, { once: true });
    socket.addEventListener('error', () => reject(new Error(`Could not connect to ${url}`)), { once: true });
});

try {
    let largeResult = null;
    const info = await request({ cmd: 'info' });
    assert.equal(info.backend, 'cuda', 'native smoke must exercise the CUDA backend');
    assert.equal(info.latticeSize, 16);
    assert.equal(info.telemetryProtocolVersion, 2);
    assert.equal(info.telemetryPush, true);
    assert.equal(info.telemetryRecoveryRequired, false);
    assert.equal(info.restartRequired, false);
    assert(Number.isSafeInteger(info.telemetryEpoch));
    assert(Number.isSafeInteger(info.sourceEpoch));
    assert.equal(info.telemetrySourceEpoch, info.sourceEpoch);
    assert.equal(inbox.some((message) => message.type === 'json'
        && message.value.type === 'telemetry_snapshot'), false,
    'a reconnect must not receive stale telemetry before it establishes demand');

    const empty = await request({
        cmd: 'setup_scenario',
        name: 'empty',
    }, 30000);
    assert.equal(empty.ok, true);
    assert.equal(empty.scenario, 'empty');

    const emptyDigest0 = await request({ cmd: 'get_dynamical_state_digest' }, 30000);
    assertCanonicalDigestShape(emptyDigest0, { latticeSize: 16 });
    assert.equal(emptyDigest0.tick, 0);
    assert.equal(emptyDigest0.sourceEpoch, empty.sourceEpoch);
    assert.equal(emptyDigest0.nonfiniteValueCount, 0);
    assert.equal(emptyDigest0.nondefaultValueCount, 0);
    assert.equal(emptyDigest0.exactDefaultRecord, true);

    socket.send(JSON.stringify({ cmd: 'tick' }));
    const emptyTick = await waitFor(
        (message) => message.type === 'json'
            && message.value.type === 'tick_complete'
            && message.value.tick === 1,
        'empty tick completion', 30000);
    assert.equal(emptyTick.value.tick, 1);

    const emptyDigest1 = await request({ cmd: 'get_dynamical_state_digest' }, 30000);
    assertCanonicalDigestShape(emptyDigest1, { latticeSize: 16 });
    assert.equal(emptyDigest1.tick, 1);
    assert(emptyDigest1.stateVersion > emptyDigest0.stateVersion);
    assert.equal(emptyDigest1.hashLo, emptyDigest0.hashLo,
        'empty canonical hash low lane must ignore clocks');
    assert.equal(emptyDigest1.hashHi, emptyDigest0.hashHi,
        'empty canonical hash high lane must ignore clocks');
    assert.equal(emptyDigest1.nonfiniteValueCount, 0);
    assert.equal(emptyDigest1.nondefaultValueCount, 0);
    assert.equal(emptyDigest1.exactDefaultRecord, true);

    const oversizedRun = await request({ cmd: 'run', n: 9 });
    assert.equal(oversizedRun.operation, 'run');
    assert.match(oversizedRun.error, /interactive chunk limit 8/);

    const invalid = await request({
        cmd: 'setup_scenario',
        name: 's0-field-uniform-e',
        applyProfile: true,
        toggle_damping: false,
        toggle_selective_damping: true,
        fluxBoundaryMode: 2,
    }, 30000);
    assert.match(invalid.error, /selective_damping requires damping/);

    const unknown = await request({
        cmd: 'setup_scenario',
        name: 's0-field-does-not-exist',
    }, 30000);
    assert.match(unknown.error, /failed to dispatch scenario/);

    const profile = await request({
        cmd: 'setup_scenario',
        name: 's0-field-uniform-e',
        applyProfile: true,
        toggle_damping: false,
        toggle_selective_damping: false,
        fluxBoundaryMode: 2,
    }, 30000);
    assert.equal(profile.ok, true);
    assert.equal(profile.scenario, 's0-field-uniform-e');
    assert(Number.isSafeInteger(profile.telemetryEpoch));
    assert(profile.sourceEpoch > info.sourceEpoch,
        'scenario replacement must advance sourceEpoch');
    assert.equal(profile.telemetrySourceEpoch, profile.sourceEpoch);
    assert.equal(profile.fluxBoundaryMode, 2);
    assert.equal(profile.toggles.damping, false);
    assert.equal(profile.toggles.selective_damping, false);

    const volumeFrame = await requestBinary(
        { cmd: 'get_flux_volume', axisSamples: 5 },
        (buffer) => buffer.byteLength >= FTV2.HEADER_BYTES
            && u32(new DataView(buffer), 0) === FTV2.MAGIC);
    const volume = ftv2Stats(volumeFrame);
    assert.equal(volume.latticeSize, 16, 'FTV2 lattice size');
    assert(volume.stride >= 1);
    assert(volume.origin < volume.stride, 'FTV2 origin must be the first center-anchored sample');

    const fieldFrame = await requestBinary(
        { cmd: 'get_field_sample', kind: 'e', stride: 1, token: 42 },
        (buffer) => buffer.byteLength >= 28
            && u32(new DataView(buffer), 0) === 0x32535446
            && u32(new DataView(buffer), 4) === 42);
    const fieldView = new DataView(fieldFrame);
    assert.equal(u32(fieldView, 8), 0, 'E sample kind code');
    assert.equal(u32(fieldView, 12), 3, 'E sample component count');
    const fieldCount = u32(fieldView, 16);
    assert.equal(u32(fieldView, 20), 1, 'FTS2 effective stride');
    assert.equal(u32(fieldView, 24), 0, 'FTS2 full-grid origin');
    assert(fieldCount > 0, 'uniform-E scenario must produce visible field samples');
    const fieldDataStart = 28 + 3 * fieldCount * 4;
    let maxE = 0;
    for (let i = 0; i < 3 * fieldCount; i++)
        maxE = Math.max(maxE, Math.abs(fieldView.getFloat32(fieldDataStart + 4 * i, true)));
    assert(maxE > 0.05, `uniform-E sample unexpectedly blank (max=${maxE})`);

    const moving = await request({
        cmd: 'setup_scenario',
        name: 's0-seed-moving-source-reciprocity',
    }, 30000);
    assert.equal(moving.ok, true);
    assert(moving.sourceEpoch > profile.sourceEpoch,
        'each successful source replacement must advance sourceEpoch');

    // Telemetry is now a native publisher.  Subscribe once, receive the
    // baseline without asking a panel to calculate anything, then let the
    // native producer's largest L=16 QoS interval expire before the tick we
    // use for the unsolicited-push/cache-only regression below.
    const demand = await request({
        cmd: 'set_telemetry_demand',
        diagnostics: true,
        audit: true,
        gravity: true,
        lagrangian: true,
        everyTicks: {
            diagnostics: 1,
            audit: 1,
            gravity: 1,
            lagrangian: 1,
        },
    });
    assert.equal(demand.type, 'telemetry_demand');
    assert.equal(demand.enabledMask, 15);
    assert.equal(demand.minIntervalMs.lagrangian, 500);
    assert.equal(demand.sourceEpoch, moving.sourceEpoch);
    assert.equal(demand.telemetrySourceEpoch, demand.sourceEpoch);

    const baselineSnapshot = await waitFor(
        (message) => message.type === 'json'
            && message.value.type === 'telemetry_snapshot'
            && message.value.publishedMask === 15
            && message.value.groupMeta?.diagnostics?.tick === 0,
        'initial unsolicited telemetry snapshot', 30000);
    assert.equal(
        Object.prototype.hasOwnProperty.call(baselineSnapshot.value, '_requestId'),
        false,
        'publisher deltas must not impersonate request-correlated responses');
    await new Promise((resolve) => setTimeout(resolve, 550));

    socket.send(JSON.stringify({ cmd: 'tick' }));
    const tick = await waitFor(
        (message) => message.type === 'json' && message.value.type === 'tick_complete',
        'tick completion', 30000);
    assert.equal(tick.value.tick, 1);

    // Do not send another command before waiting: this must be delivered by
    // the native idle publisher after the due tick, not by a panel request.
    const pushedTelemetry = await waitFor(
        (message) => message.type === 'json'
            && message.value.type === 'telemetry_snapshot'
            && message.value.publishedMask === 15
            && message.value.groupMeta?.diagnostics?.tick === 1
            && message.value.groupMeta?.audit?.tick === 1
            && message.value.groupMeta?.gravity?.tick === 1
            && message.value.groupMeta?.lagrangian?.tick === 1,
        'post-tick unsolicited telemetry snapshot', 30000);
    const pushedVersion = pushedTelemetry.value.snapshotVersion;
    assert.equal(pushedTelemetry.value.sourceEpoch, moving.sourceEpoch);
    assert.equal(
        Object.prototype.hasOwnProperty.call(pushedTelemetry.value, '_requestId'),
        false,
        'unsolicited telemetry must remain independent of RPC request IDs');
    assert.equal(pushedTelemetry.value.groups.diagnostics.tick, 1);

    const diagnostics = await request({ cmd: 'get_diagnostics' });
    assert.equal(diagnostics.tick, 1);
    assert(Number.isFinite(diagnostics.totalEnergy));
    const audit = await request({ cmd: 'get_energy_audit' });
    assert(Number.isFinite(audit.totalEnergy));
    assert(Number.isFinite(audit.gaussViolation));
    const gravity = await request({ cmd: 'get_gravity_metric' });
    assert.equal(typeof gravity.active, 'boolean');
    assert(Number.isFinite(gravity.latencyMax));
    const lagrangian = await request({ cmd: 'get_lagrangian' });
    assert(Number.isFinite(lagrangian.total));
    assert(Number.isFinite(lagrangian.hamiltonian));
    const telemetry = await request({
        cmd: 'get_telemetry',
        diagnostics: true,
        audit: true,
        lagrangian: true,
        gravity: true,
    });
    assert.equal(telemetry.type, 'telemetry');
    assert.equal(telemetry.snapshotVersion, pushedVersion,
        'get_telemetry must read the published cache, not launch a new reduction');
    assert.equal(telemetry.sourceEpoch, moving.sourceEpoch);
    assert(Number.isFinite(telemetry.groups?.diagnostics?.totalEnergy));
    assert(Number.isFinite(telemetry.groups?.audit?.totalEnergy));
    assert(Number.isFinite(telemetry.groups?.lagrangian?.total));
    assert.equal(typeof telemetry.groups?.gravity?.active, 'boolean');
    assert.equal(telemetry.groupMeta?.diagnostics?.stale, false);

    // A second cache read after idle time must preserve the publication
    // version. If this regresses into a request-triggered reduction it will
    // advance the server cache/push a new version even though no tick occurred.
    await new Promise((resolve) => setTimeout(resolve, 100));
    const telemetryAgain = await request({
        cmd: 'get_telemetry',
        diagnostics: true,
        audit: true,
        lagrangian: true,
        gravity: true,
    });
    assert.equal(telemetryAgain.snapshotVersion, pushedVersion,
        'cache-only get_telemetry must not advance snapshotVersion');
    const voxel = await request({ cmd: 'inspect_voxel', x: 8, y: 8, z: 8 });
    assert.equal(voxel.x, 8);
    assert(Number.isFinite(voxel.divJ));
    const force = await request({ cmd: 'get_force_at', x: 8, y: 8, z: 8 });
    assert.equal(force.x, 8);
    assert(Number.isFinite(force.coulombMag));

    const particleFrame = await requestBinary(
        { cmd: 'get_particles' },
        (buffer) => buffer.byteLength >= 8
            && u32(new DataView(buffer), 0) === 0x32505446);
    const particleView = new DataView(particleFrame);
    const particleCount = u32(particleView, 4);
    assert(particleCount > 0, 'moving-source scenario must return particles');
    assert.equal(particleFrame.byteLength, 8 + particleCount * 9 * 4, 'FTP2 frame layout');

    const spinStart = 8 + particleCount * 7 * 4;
    const colorChargeStart = spinStart + particleCount * 4;
    for (let i = 0; i < particleCount; i++) {
        assert(Number.isFinite(particleView.getFloat32(spinStart + i * 4, true)));
        assert(Number.isFinite(particleView.getFloat32(colorChargeStart + i * 4, true)));
    }

    // Direct edits are sampled only after the native 16 ms quiet window. This
    // specifically guards against a stale in-flight snapshot retiring early
    // and immediately rearming an empty-cache reduction mid-edit.
    const mutationStartedAt = Date.now();
    socket.send(JSON.stringify({
        cmd: 'inject_flux_add', x: 8, y: 8, z: 8, fx: 0.001, fy: 0, fz: 0,
    }));
    const mutationInvalidation = await waitFor(
        (message) => message.type === 'json'
            && message.value.type === 'telemetry_invalidated'
            && message.value.epoch > pushedTelemetry.value.epoch,
        'direct-mutation telemetry invalidation', 30000);
    assert.equal(mutationInvalidation.value.tick, 1);
    assert.equal(mutationInvalidation.value.sourceEpoch, moving.sourceEpoch);
    assert.equal(mutationInvalidation.value.freshMask, 0);
    assert.equal(mutationInvalidation.value.reason, 'state_mutated');
    assert.equal(
        Object.prototype.hasOwnProperty.call(mutationInvalidation.value, '_requestId'),
        false,
        'telemetry invalidation must remain an unsolicited state-boundary push');
    const mutationSnapshot = await waitFor(
        (message) => message.type === 'json'
            && message.value.type === 'telemetry_snapshot'
            && (message.value.publishedMask & 1) !== 0
            && message.value.groupMeta?.diagnostics?.epoch
                > pushedTelemetry.value.groupMeta.diagnostics.epoch,
        'debounced direct-mutation telemetry snapshot', 30000);
    assert(Date.now() - mutationStartedAt >= 10,
        'direct-mutation snapshot bypassed the 16 ms quiet window');
    assert.equal(mutationSnapshot.value.groupMeta.diagnostics.tick, 1);

    // A tick can arrive before an edit's debounce window expires. Slow groups
    // must still be forced to a post-edit sample rather than waiting for
    // their nominal 8/12-tick cadence. Leave native QoS spacing to expire so
    // this exercise is about provenance, not a deliberate producer throttle.
    const sparseDemand = await request({
        cmd: 'set_telemetry_demand',
        diagnostics: true,
        audit: true,
        gravity: true,
        lagrangian: true,
        everyTicks: {
            diagnostics: 1,
            audit: 8,
            gravity: 4,
            lagrangian: 12,
        },
    });
    assert.equal(sparseDemand.type, 'telemetry_demand');
    await new Promise((resolve) => setTimeout(resolve, 550));
    socket.send(JSON.stringify({
        cmd: 'inject_flux_add', x: 8, y: 8, z: 8, fx: 0.001, fy: 0, fz: 0,
    }));
    socket.send(JSON.stringify({ cmd: 'tick' }));
    const immediateTickInvalidation = await waitFor(
        (message) => message.type === 'json'
            && message.value.type === 'telemetry_invalidated'
            && message.value.epoch > mutationSnapshot.value.epoch,
        'edit-before-tick telemetry invalidation', 30000);
    const immediateTick = await waitFor(
        (message) => message.type === 'json'
            && message.value.type === 'tick_complete'
            && message.value.tick === 2,
        'edit-before-tick completion', 30000);
    assert.equal(immediateTick.value.tick, 2);
    // GPU producer QoS may return the forced groups in more than one delta
    // (for example diagnostics/audit/gravity followed by lagrangian). The
    // contract is that every demanded group becomes fresh for this settled
    // tick, not that unrelated reductions share one transport frame.
    const forcedGroups = [
        [1, 'diagnostics'],
        [2, 'audit'],
        [4, 'gravity'],
        [8, 'lagrangian'],
    ];
    const forcedDeadline = Date.now() + 30000;
    let forcedPostTickMask = 0;
    while (forcedPostTickMask !== 15) {
        const remainingMs = forcedDeadline - Date.now();
        assert(remainingMs > 0, 'timed out waiting for forced post-edit telemetry groups');
        const publication = await waitFor(
            (message) => message.type === 'json'
                && message.value.type === 'telemetry_snapshot'
                && message.value.epoch > immediateTickInvalidation.value.epoch
                && (message.value.publishedMask & ~forcedPostTickMask) !== 0,
            'next forced post-edit telemetry group', remainingMs);
        assert.equal(publication.value.sourceEpoch, moving.sourceEpoch);
        for (const [bit, group] of forcedGroups) {
            if ((publication.value.publishedMask & bit) === 0) continue;
            assert(publication.value.groupMeta?.[group], `${group} metadata is required`);
            assert.equal(publication.value.groupMeta[group].tick, 2,
                `${group} must be sampled at the settled post-edit tick`);
            assert(publication.value.groupMeta[group].epoch
                > immediateTickInvalidation.value.epoch,
            `${group} must be newer than the edit invalidation`);
            assert.equal(publication.value.groupMeta[group].stale, false);
            forcedPostTickMask |= bit;
        }
    }
    assert.equal(forcedPostTickMask, 15);

    const largeSize = Math.trunc(Number(process.env.FTD_WS_LARGE || 0));
    if (largeSize > 0) {
        const preflight = await request({ cmd: 'preflight_resize', size: largeSize });
        assert.equal(preflight.accepted, true, `L=${largeSize} must pass memory preflight`);
        const startedAt = Date.now();
        const large = await request({
            cmd: 'resize_scenario',
            size: largeSize,
            name: 'quantum-well',
        }, 120000);
        const setupMs = Date.now() - startedAt;
        assert.equal(large.ok, true);
        assert.equal(large.latticeSize, largeSize);
        assert.equal(large.scenario, 'quantum-well');

        socket.send(JSON.stringify({ cmd: 'tick' }));
        const largeTick = await waitFor(
            (message) => message.type === 'json'
                && message.value.type === 'tick_complete'
                && message.value.tick === 1,
            'large-lattice tick completion', 120000);
        assert.equal(largeTick.value.tick, 1);

        // A host-mirror-only extension must be rejected transactionally before
        // it can mutate a full-GPU interactive profile. The live bridge must
        // remain usable without a compensating profile write.
        if (largeSize > 64) {
            const guardedProfile = await request({
                cmd: 'apply_profile',
                toggle_forces: true,
                toggle_cluster_inertia: true,
            }, 30000);
            assert.equal(guardedProfile.operation, 'apply_profile');
            assert.match(guardedProfile.error, /cluster_inertia|host|full-GPU/i);
            socket.send(JSON.stringify({ cmd: 'tick' }));
            const recoveredTick = await waitFor(
                (message) => message.type === 'json'
                    && message.value.type === 'tick_complete'
                    && message.value.tick === 2,
                'post-error recovered tick completion', 120000);
            assert.equal(recoveredTick.value.tick, 2);
        }

        const largeVolume = await requestBinary(
            { cmd: 'get_flux_volume', axisSamples: 53 },
            (buffer) => buffer.byteLength >= FTV2.HEADER_BYTES
                && u32(new DataView(buffer), 0) === FTV2.MAGIC,
            120000);
        const largeVolumeStats = ftv2Stats(largeVolume);
        assert(largeVolumeStats.axisCount <= 53,
            `FTV2 axis cap exceeded (${largeVolumeStats.axisCount})`);
        assert(largeVolume.byteLength < 1024 * 1024, 'large FTV2 frame must stay below 1 MiB');

        const setupLargeScenario = async (name) => {
            const response = await request({ cmd: 'setup_scenario', name }, 120000);
            assert.equal(response.ok, true, `${name} setup`);
            assert.equal(response.scenario, name, `${name} scenario echo`);
            assert.equal(response.latticeSize, largeSize, `${name} lattice size`);
        };
        const sampleLargeVolume = async () => {
            const frame = await requestBinary(
                { cmd: 'get_flux_volume', axisSamples: 53 },
                (buffer) => buffer.byteLength >= FTV2.HEADER_BYTES
                    && u32(new DataView(buffer), 0) === FTV2.MAGIC,
                120000);
            return ftv2Stats(frame);
        };
        const sampleLargeField = async (kind, kindCode, components, token) => {
            const frame = await requestBinary(
                { cmd: 'get_field_sample', kind, stride: 2, token },
                (buffer) => buffer.byteLength >= 28
                    && u32(new DataView(buffer), 0) === 0x32535446
                    && u32(new DataView(buffer), 4) === token,
                120000);
            const stats = fts2Stats(frame, { token, kindCode });
            assert.equal(stats.components, components, `${kind} component count`);
            return stats;
        };

        // Fixed-support and low-amplitude native seeds must survive compact
        // block sampling; these are the regressions that previously yielded a
        // visually empty large-L desktop scene.
        await setupLargeScenario('s0-seed-wilson-loop');
        const wilsonVolume = await sampleLargeVolume();
        assert(wilsonVolume.maxAbs > 0, `Wilson-loop J vanished from FTV2 at L=${largeSize}`);

        await setupLargeScenario('s0-seed-emergent-ic4-subthreshold');
        const ic4Volume = await sampleLargeVolume();
        assert(ic4Volume.maxAbs > 0, `IC4 subthreshold J vanished from FTV2 at L=${largeSize}`);
        const ic4Particles = await requestBinary(
            { cmd: 'get_particles' },
            (buffer) => buffer.byteLength >= 8
                && u32(new DataView(buffer), 0) === 0x32505446,
            120000);
        assert.equal(u32(new DataView(ic4Particles), 4), 0,
            'IC4 subthreshold control must remain unmanifested');

        await setupLargeScenario('flux-vortex');
        const vortexVolume = await sampleLargeVolume();
        assert(vortexVolume.maxAbs > 0, `vortex J vanished from FTV2 at L=${largeSize}`);
        const vortexB = await sampleLargeField('b', 1, 3, 201);
        const vortexCurl = await sampleLargeField('vorticity', 5, 1, 202);
        assert(vortexB.count > 0 && vortexB.maxAbs > 0,
            `vortex B sample blank at L=${largeSize}`);
        assert(vortexCurl.count > 0 && vortexCurl.maxAbs > 0,
            `vortex vorticity sample blank at L=${largeSize}`);

        await setupLargeScenario('s0-seed-massive-body');
        const massiveVolume = await sampleLargeVolume();
        assert.equal(massiveVolume.maxAbs, 0, 'massive-body must not fabricate J');
        // The mass sites are the tick-0 initial condition; the Poisson well is
        // produced by the latency phase. Mirror the dashboard's default
        // prime-tick-on-load path before requiring the derived field.
        socket.send(JSON.stringify({ cmd: 'tick' }));
        const massivePrime = await waitFor(
            (message) => message.type === 'json'
                && message.value.type === 'tick_complete'
                && message.value.tick === 1,
            'massive-body prime tick', 120000);
        assert.equal(massivePrime.value.tick, 1);
        const poissonLatency = await sampleLargeField('poissonLatency', 17, 1, 203);
        assert(poissonLatency.count > 0 && poissonLatency.maxAbs > 0,
            `massive-body real Poisson latency blank at L=${largeSize}`);

        largeResult = {
            size: largeSize,
            setupMs,
            volumeBytes: largeVolume.byteLength,
            wilsonMax: wilsonVolume.maxAbs,
            ic4Max: ic4Volume.maxAbs,
            vortexMax: vortexVolume.maxAbs,
            vortexBCount: vortexB.count,
            vortexCurlCount: vortexCurl.count,
            poissonLatencyMax: poissonLatency.maxAbs,
        };
    }

    console.log(JSON.stringify({
        ok: true,
        backend: info.backend,
        fieldSamples: fieldCount,
        uniformEMax: maxE,
        particles: particleCount,
        tick: tick.value.tick,
        large: largeResult,
    }));
} finally {
    socket.close();
}
