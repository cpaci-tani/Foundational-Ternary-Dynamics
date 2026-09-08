// Real native process + actual production WebSocketBridge. No mocked decoder,
// socket, command dispatcher or physics clock. This is a forced-CPU contract
// check; it makes no rendering/performance or physical recovery claim.
import assert from 'node:assert/strict';
import { spawn } from 'node:child_process';
import { createServer } from 'node:net';
import { once } from 'node:events';
import { readFile, writeFile } from 'node:fs/promises';
import { resolve } from 'node:path';
import { createHash } from 'node:crypto';
import { WebSocketBridge } from '../js/ws-bridge.js';

const [executableArg, outputArg] = process.argv.slice(2);
assert(executableArg && outputArg, 'Usage: node native-observation-v3-integration.mjs <ws_server> <output.json>');
const executable = resolve(executableArg), output = resolve(outputArg);
const reservation = createServer();
reservation.listen(0, '127.0.0.1');
await once(reservation, 'listening');
const port = reservation.address().port;
await new Promise((resolveClose, reject) => reservation.close(error => error ? reject(error) : resolveClose()));
const child = spawn(executable, ['7', String(port)], {
    env: { ...process.env, FTD_FORCE_CPU: '1' }, windowsHide: true,
    stdio: ['ignore', 'pipe', 'pipe'],
});
let log = '';
child.stdout.on('data', bytes => { log += bytes.toString(); });
child.stderr.on('data', bytes => { log += bytes.toString(); });
const sleep = ms => new Promise(done => setTimeout(done, ms));
const cases = [];
let bridge;
try {
    for (let attempt = 0; !log.includes('Listening on'); ++attempt) {
        assert(attempt < 300 && child.exitCode == null, `native startup failed: ${log}`);
        await sleep(20);
    }
    bridge = new WebSocketBridge(`ws://127.0.0.1:${port}`);
    await bridge.connect();
    assert.equal(bridge._nativeBinaryVersion, 3);
    assert.match(bridge._nativeInstanceId, /^[a-f0-9]{32}$/);
    const instance = bridge._nativeInstanceId;
    cases.push('actual_info_negotiates_v3');
    await bridge._sendJSON({ cmd: 'inject_particle', x: 3, y: 3, z: 3,
        state: 1, fx: .125, fy: -.0625, fz: .03125 });
    const before = await bridge._sendJSON({ cmd: 'get_dynamical_state_digest' });
    const commands = [
        { cmd: 'get_particles' },
        { cmd: 'get_flux_volume', axisSamples: 7 },
        { cmd: 'get_field_sample', kind: 'e', stride: 1, token: 101 },
        { cmd: 'get_field_sample', kind: 'divJ', stride: 1, token: 102 },
    ];
    const observed = await Promise.all(commands.map(command => bridge._sendJSON(command)));
    assert.deepEqual(observed.map(item => item.type), ['particles', 'volume', 'field', 'field']);
    for (const item of observed) {
        assert.equal(item.wireVersion, 3);
        assert.equal(item.provenance.sampleTick, 0);
        assert.equal(item.provenance.nativeInstanceId, instance);
        assert.equal(item.provenance.sourceEpoch, before.sourceEpoch);
        assert(Object.isFrozen(item.provenance));
    }
    cases.push('concurrent_actual_binary_requests_correlate_and_retain_provenance');
    const after = await bridge._sendJSON({ cmd: 'get_dynamical_state_digest' });
    assert.equal(after.hashLo, before.hashLo); assert.equal(after.hashHi, before.hashHi);
    assert.equal(after.tick, before.tick);
    cases.push('actual_binary_observation_preserves_scoped_state');

    const particles = await bridge.getParticleDataAsync();
    assert(particles.count > 0);
    assert.equal(particles.provenance.nativeInstanceId, instance);
    cases.push('production_particle_cache_path_uses_negotiated_protocol');
    const tick = await bridge._sendJSON({ cmd: 'tick' });
    assert.equal(tick.type, 'tick_complete'); assert.equal(tick.tick, 1);
    const run = await bridge._sendJSON({ cmd: 'run', n: 1 });
    assert.equal(run.type, 'run_complete'); assert.equal(run.tick, 2);
    cases.push('special_json_tick_run_completions_settle_actual_promises');
    const slice = await bridge._sendJSON({ cmd: 'get_flux_slice', axis: 0, index: 3 });
    assert.equal(slice.sampleTick, 2); assert.equal(slice.sourceEpoch, before.sourceEpoch);
    cases.push('special_json_slice_correlates_actual_sample');
    for (const cmd of ['inspect_voxel', 'get_force_at']) {
        const point = await bridge._sendJSON({ cmd, x: 3, y: 3, z: 3 });
        assert.deepEqual([point.x, point.y, point.z], [3, 3, 3]);
        assert.equal(point.sampleTick, 2);
        assert.equal(point.sourceEpoch, before.sourceEpoch);
        assert.equal(point.nativeInstanceId, instance);
        cases.push(`actual_point_observation_${cmd}_retains_provenance_and_coordinates`);
    }

    // Production queues the next tick before viewport reads. Its ACK arrives
    // before these samples; local render dirtiness must not retire a sample
    // whose native metadata describes the newly acknowledged state.
    for (let iteration = 0; iteration < 3; ++iteration) {
        const nextTick = bridge._sendJSON({ cmd: 'tick' });
        const reads = ['get_particles', 'get_flux_volume'];
        const requests = reads.map(cmd => bridge._sendJSON({ cmd }));
        const advanced = await nextTick;
        const frames = await Promise.all(requests);
        for (let i = 0; i < frames.length; ++i) {
            let frame = frames[i];
            for (let retry = 0; frame.type === 'visual_deferred' && retry < 100; ++retry) {
                await sleep(Math.max(16, frame.retryAfterMs || 16));
                frame = await bridge._sendJSON({ cmd: reads[i] });
            }
            assert(frame.provenance, JSON.stringify(frame));
            assert.equal(frame.provenance.sampleTick, advanced.tick);
            assert.equal(frame.provenance.sourceEpoch, before.sourceEpoch);
            assert.equal(frame.provenance.nativeInstanceId, instance);
        }
    }
    cases.push('queued_tick_ack_then_current_particle_and_volume_samples_remain_acceptable');

    // Public resize owns source adoption, cache retirement and local generation.
    // The private RPC primitive intentionally does not perform those effects.
    await bridge.resize(7);
    const info = await bridge._sendJSON({ cmd: 'info' });
    assert(info.sourceEpoch > before.sourceEpoch);
    assert.equal(info.nativeInstanceId, instance);
    let resetSample = await bridge._sendJSON({ cmd: 'get_particles' });
    for (let retry = 0; resetSample.type === 'visual_deferred' && retry < 100; ++retry) {
        await sleep(Math.max(16, resetSample.retryAfterMs || 16));
        resetSample = await bridge._sendJSON({ cmd: 'get_particles' });
    }
    assert(resetSample.provenance, JSON.stringify(resetSample));
    assert.equal(resetSample.provenance.sourceEpoch, info.sourceEpoch);
    assert.equal(resetSample.provenance.sampleTick, 0);
    cases.push('actual_progress_and_source_replacement_complete_without_stale_sample');
    assert.equal(bridge._pendingQueue.length, 0);
    cases.push('no_request_promise_remains_pending');
    const record = { status: 'passed', case_count: cases.length, cases,
        backend: 'forced_cpu', lattice_size: 7,
        executable_sha256: createHash('sha256').update(await readFile(executable)).digest('hex'),
        scope: 'actual production JavaScript bridge against owned native CPU server; no GPU, FPS or physics certification' };
    await writeFile(output, JSON.stringify(record, null, 2) + '\n');
    console.log(JSON.stringify(record));
} finally {
    bridge?.dispose();
    if (child.exitCode == null) {
        child.kill();
        let cleanupTimer;
        try {
            await Promise.race([once(child, 'exit'), new Promise(resolve => {
                cleanupTimer = setTimeout(resolve, 5000);
                cleanupTimer.unref();
            })]);
        } finally {
            clearTimeout(cleanupTimer);
        }
    }
    await writeFile(output.replace(/\.json$/, '.server.log'), log);
}
