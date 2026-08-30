// @ts-check

import { test, expect } from '@playwright/test';

test('native connect waits for authoritative engine info before resize decisions', async ({ page }) => {
    await page.goto('/js/ws-bridge.js', { waitUntil: 'domcontentloaded' });

    const result = await page.evaluate(async () => {
        const { WebSocketBridge } = await import('/js/ws-bridge.js?connect-order-test=1');
        const OriginalWebSocket = window.WebSocket;

        class FakeWebSocket {
            static CONNECTING = 0;
            static OPEN = 1;
            static CLOSED = 3;

            constructor() {
                this.readyState = FakeWebSocket.CONNECTING;
                queueMicrotask(() => {
                    this.readyState = FakeWebSocket.OPEN;
                    this.onopen?.();
                });
            }

            send(payload) {
                const message = JSON.parse(payload);
                if (message.cmd !== 'info') return;
                setTimeout(() => {
                    this.onmessage?.({
                        data: JSON.stringify({
                            latticeSize: 64,
                            gpu: true,
                            backend: 'cuda',
                            version: '2.18.0',
                        }),
                    });
                }, 25);
            }

            close() {
                this.readyState = FakeWebSocket.CLOSED;
                this.onclose?.();
            }
        }

        window.WebSocket = FakeWebSocket;
        try {
            const bridge = new WebSocketBridge('ws://connect-order-test');
            let resolved = false;
            const connected = bridge.connect().then(() => { resolved = true; });
            await new Promise(resolve => setTimeout(resolve, 5));
            const resolvedBeforeInfo = resolved;
            const sizeBeforeInfo = bridge.latticeSize;
            await connected;
            return {
                resolvedBeforeInfo,
                sizeBeforeInfo,
                resolvedAfterInfo: resolved,
                sizeAfterInfo: bridge.latticeSize,
                gpuAfterInfo: bridge.isNativeGPU,
            };
        } finally {
            window.WebSocket = OriginalWebSocket;
        }
    });

    expect(result.resolvedBeforeInfo).toBe(false);
    expect(result.sizeBeforeInfo).toBe(32);
    expect(result.resolvedAfterInfo).toBe(true);
    expect(result.sizeAfterInfo).toBe(64);
    expect(result.gpuAfterInfo).toBe(true);
});

test('every native socket generation announces readiness for scenario replay', async ({ page }) => {
    await page.goto('/js/ws-bridge.js', { waitUntil: 'domcontentloaded' });

    const result = await page.evaluate(async () => {
        const { WebSocketBridge } = await import('/js/ws-bridge.js?connection-generation-test=1');
        const OriginalWebSocket = window.WebSocket;
        const originalCtx = window.__ftdCtx;

        class FakeWebSocket {
            static CONNECTING = 0;
            static OPEN = 1;
            static CLOSED = 3;

            constructor() {
                this.readyState = FakeWebSocket.CONNECTING;
                queueMicrotask(() => {
                    this.readyState = FakeWebSocket.OPEN;
                    this.onopen?.();
                });
            }

            send(payload) {
                const message = JSON.parse(payload);
                if (message.cmd !== 'info') return;
                queueMicrotask(() => this.onmessage?.({
                    data: JSON.stringify({
                        _requestId: message._requestId,
                        latticeSize: 181,
                        gpu: true,
                        backend: 'cuda',
                    }),
                }));
            }

            close() { this.readyState = FakeWebSocket.CLOSED; }
        }

        window.WebSocket = FakeWebSocket;
        try {
            const bridge = new WebSocketBridge('ws://connection-generation-test');
            const generations = [];
            window.__ftdCtx = {
                bridge,
                onBridgeConnectionReady({ generation }) { generations.push(generation); },
            };
            await bridge.connect();

            // A fresh socket may connect while the selected DOM value and the
            // client-side scenario id are unchanged. It must still announce a
            // new generation so the controller can atomically replay profile.
            bridge._connected = false;
            bridge.ready = false;
            bridge._ws.readyState = FakeWebSocket.CLOSED;
            await bridge.connect();

            return { generations, generation: bridge._connectionGeneration };
        } finally {
            window.WebSocket = OriginalWebSocket;
            window.__ftdCtx = originalCtx;
        }
    });

    expect(result.generations).toEqual([1, 2]);
    expect(result.generation).toBe(2);
});

test('retired socket callbacks cannot invalidate a replacement connection', async ({ page }) => {
    await page.goto('/js/ws-bridge.js', { waitUntil: 'domcontentloaded' });

    const result = await page.evaluate(async () => {
        const { WebSocketBridge } = await import('/js/ws-bridge.js?stale-socket-test=1');
        const OriginalWebSocket = window.WebSocket;
        const originalCtx = window.__ftdCtx;

        class ControlledWebSocket {
            static CONNECTING = 0;
            static OPEN = 1;
            static CLOSING = 2;
            static CLOSED = 3;
            static instances = [];

            constructor() {
                this.readyState = ControlledWebSocket.CONNECTING;
                this.sent = [];
                ControlledWebSocket.instances.push(this);
            }

            open() {
                this.readyState = ControlledWebSocket.OPEN;
                this.onopen?.();
            }

            send(payload) { this.sent.push(JSON.parse(payload)); }

            reply(data) { this.onmessage?.({ data: JSON.stringify(data) }); }

            close() { this.readyState = ControlledWebSocket.CLOSING; }
        }

        window.WebSocket = ControlledWebSocket;
        try {
            const bridge = new WebSocketBridge('ws://stale-socket-test');
            let reconnects = 0;
            bridge._scheduleReconnect = () => { reconnects++; };
            window.__ftdCtx = { bridge, running: true };

            const firstConnect = bridge.connect();
            const socketA = ControlledWebSocket.instances[0];
            socketA.open();
            const infoA = socketA.sent.find(message => message.cmd === 'info');
            socketA.reply({
                _requestId: infoA._requestId,
                latticeSize: 64,
                gpu: true,
                backend: 'cuda',
            });
            await firstConnect;
            const staleClose = socketA.onclose;

            // Retire A without delivering its close event yet, then complete a
            // replacement B connection. While B is open but still awaiting
            // info, the recovery barrier must keep rAF ticks off the socket.
            bridge._connected = false;
            bridge.ready = false;
            socketA.readyState = ControlledWebSocket.CLOSING;
            const secondConnect = bridge.connect();
            const socketB = ControlledWebSocket.instances[1];
            socketB.open();
            bridge.tick();
            const simulationBeforeInfo = socketB.sent.filter(
                message => message.cmd === 'tick' || message.cmd === 'run',
            );
            const queuedBehindInfo = bridge._queuedSimulationTicks;
            bridge.cancelQueuedTicks();
            const infoB = socketB.sent.find(message => message.cmd === 'info');
            socketB.reply({
                _requestId: infoB._requestId,
                latticeSize: 181,
                gpu: true,
                backend: 'cuda',
            });
            await secondConnect;

            bridge.run(1);
            const probePromise = bridge._sendJSON({ cmd: 'probe' });
            const probe = socketB.sent.find(message => message.cmd === 'probe');

            // The delayed A event used to clear B's ready flags, simulation
            // watchdog, visual requests and shared pending response queue.
            staleClose?.();
            const intactAfterStaleClose = {
                ownsB: bridge._ws === socketB,
                connected: bridge._connected,
                ready: bridge.ready,
                simulationInFlight: bridge._simulationInFlight,
                pending: bridge._pendingQueue.length,
                generation: bridge._connectionGeneration,
            };
            socketB.reply({ _requestId: probe._requestId, ok: true, source: 'B' });
            const probeResponse = await probePromise;
            socketB.reply({ type: 'run_complete', tick: 2 });

            return {
                simulationBeforeInfo,
                queuedBehindInfo,
                intactAfterStaleClose,
                probeResponse,
                reconnects,
                simulationAfterAck: bridge._simulationInFlight,
            };
        } finally {
            window.WebSocket = OriginalWebSocket;
            window.__ftdCtx = originalCtx;
        }
    });

    expect(result.simulationBeforeInfo).toEqual([]);
    expect(result.queuedBehindInfo).toBe(1);
    expect(result.intactAfterStaleClose).toEqual({
        ownsB: true,
        connected: true,
        ready: true,
        simulationInFlight: true,
        pending: 1,
        generation: 2,
    });
    expect(result.probeResponse).toMatchObject({ ok: true, source: 'B' });
    expect(result.reconnects).toBe(0);
    expect(result.simulationAfterAck).toBe(false);
});

test('established socket error schedules one reconnect across the following close', async ({ page }) => {
    await page.goto('/js/ws-bridge.js', { waitUntil: 'domcontentloaded' });

    const result = await page.evaluate(async () => {
        const { WebSocketBridge } = await import('/js/ws-bridge.js?error-close-reconnect-test=1');
        const OriginalWebSocket = window.WebSocket;

        class ControlledWebSocket {
            static CONNECTING = 0;
            static OPEN = 1;
            static CLOSED = 3;
            static instances = [];

            constructor() {
                this.readyState = ControlledWebSocket.CONNECTING;
                this.sent = [];
                ControlledWebSocket.instances.push(this);
            }

            open() {
                this.readyState = ControlledWebSocket.OPEN;
                this.onopen?.();
            }

            send(payload) { this.sent.push(JSON.parse(payload)); }

            reply(data) { this.onmessage?.({ data: JSON.stringify(data) }); }
        }

        window.WebSocket = ControlledWebSocket;
        try {
            const bridge = new WebSocketBridge('ws://error-close-reconnect-test');
            const connected = bridge.connect();
            const socket = ControlledWebSocket.instances[0];
            socket.open();
            const info = socket.sent.find(message => message.cmd === 'info');
            socket.reply({
                _requestId: info._requestId,
                latticeSize: 181,
                gpu: true,
                backend: 'cuda',
            });
            await connected;

            let reconnects = 0;
            bridge._scheduleReconnect = () => { reconnects++; };
            const pending = bridge._sendJSON({ cmd: 'probe' })
                .then(() => 'resolved', error => error?.message || String(error));
            socket.onerror?.(new Error('transport failed'));
            const afterError = {
                connected: bridge._connected,
                ready: bridge.ready,
                reconnects,
                pending: bridge._pendingQueue.length,
            };
            socket.readyState = ControlledWebSocket.CLOSED;
            socket.onclose?.();
            const afterClose = {
                connected: bridge._connected,
                ready: bridge.ready,
                reconnects,
                pending: bridge._pendingQueue.length,
            };

            // A pre-open failure is still a rejected connection attempt, not
            // an autonomous background reconnect loop.
            const attemptBridge = new WebSocketBridge('ws://failed-handshake-test');
            let attemptReconnects = 0;
            attemptBridge._scheduleReconnect = () => { attemptReconnects++; };
            const attempt = attemptBridge.connect()
                .then(() => 'resolved', error => error?.message || String(error));
            const attemptSocket = ControlledWebSocket.instances[1];
            attemptSocket.onerror?.(new Error('handshake failed'));

            return {
                afterError,
                afterClose,
                pendingResult: await pending,
                attemptResult: await attempt,
                attemptReconnects,
            };
        } finally {
            window.WebSocket = OriginalWebSocket;
        }
    });

    expect(result.afterError).toEqual({
        connected: false,
        ready: false,
        reconnects: 1,
        pending: 0,
    });
    expect(result.afterClose).toEqual(result.afterError);
    expect(result.pendingResult).toBe('transport failed');
    expect(result.attemptResult).toBe('handshake failed');
    expect(result.attemptReconnects).toBe(0);
});

test('native visual caches wake paused Scale 0 without polling the same epoch', async ({ page }) => {
    // Use a same-origin source page without booting the full dashboard.  The
    // bridge can then be exercised with a deterministic fake WebSocket.
    await page.goto('/js/ws-bridge.js', { waitUntil: 'domcontentloaded' });

    const result = await page.evaluate(async () => {
        const { WebSocketBridge } = await import('/js/ws-bridge.js?visual-cache-test=1');
        const bridge = new WebSocketBridge('ws://visual-cache-test');
        const sent = [];
        let refreshes = 0;

        bridge._connected = true;
        bridge.ready = true;
        bridge._ws = {
            send(payload) {
                sent.push(JSON.parse(payload));
            },
        };
        window.__ftdCtx = {
            bridge,
            running: false,
            onBridgePostFrame() { refreshes++; },
        };

        // Multiple consumers may ask for the same cache during one render
        // sweep. Only the first read should emit a request.
        bridge.getFluxVolume();
        bridge.getFluxVolume();
        const initialVolumeRequests = sent.filter(m => m.cmd === 'get_flux_volume').length;

        bridge._handleJSON(JSON.stringify({ type: 'flux_volume', data: [0, 0.5, 1] }));
        const cachedVolume = Array.from(bridge.getFluxVolume());
        const sameEpochVolumeRequests = sent.filter(m => m.cmd === 'get_flux_volume').length;

        // A physics mutation advances the epoch only after the native server
        // acknowledges completion, and permits one new request.
        bridge.tick();
        bridge._handleJSON(JSON.stringify({ type: 'tick_complete', tick: 1 }));
        bridge.getFluxVolume();
        bridge.getFluxVolume();
        const nextEpochVolumeRequests = sent.filter(m => m.cmd === 'get_flux_volume').length;

        // Binary particle delivery uses the same paused-refresh path.
        bridge.getParticleData();
        bridge.getParticleData();
        const particleRequests = sent.filter(m => m.cmd === 'get_particles').length;
        const emptyFtp2 = new ArrayBuffer(8);
        const ftp2 = new DataView(emptyFtp2);
        ftp2.setUint32(0, 0x32505446, true);
        ftp2.setUint32(4, 0, true);
        bridge._handleBinary(emptyFtp2);

        delete window.__ftdCtx;
        return {
            initialVolumeRequests,
            sameEpochVolumeRequests,
            nextEpochVolumeRequests,
            particleRequests,
            cachedVolume,
            refreshes,
        };
    });

    expect(result.initialVolumeRequests).toBe(1);
    expect(result.sameEpochVolumeRequests).toBe(1);
    expect(result.nextEpochVolumeRequests).toBe(2);
    expect(result.particleRequests).toBe(1);
    expect(result.cachedVolume).toEqual([0, 0.5, 1]);
    expect(result.refreshes).toBe(3);
});

test('native ticks are backpressured and Scale 0 never creates the WASM fallback', async ({ page }) => {
    await page.goto('/js/ws-bridge.js', { waitUntil: 'domcontentloaded' });

    const result = await page.evaluate(async () => {
        const { WebSocketBridge } = await import('/js/ws-bridge.js?backpressure-test=1');
        const bridge = new WebSocketBridge('ws://backpressure-test');
        const sent = [];
        let forcedUploads = 0;
        bridge._connected = true;
        bridge.ready = true;
        bridge._ws = { send(payload) { sent.push(JSON.parse(payload)); } };
        bridge._sendJSON = async () => ({ ok: true, tick: 0 });
        window.__ftdCtx = {
            bridge,
            running: true,
            onBridgePostFrame(_samplers, forceUpload) {
                if (forceUpload) forcedUploads++;
            },
        };

        bridge.setToggle('damping', true);
        bridge.setParam('dt', 0.5);
        bridge.injectFlux(1, 1, 1, 1, 0, 0);
        bridge.setupScenario('flux-pulse');
        await new Promise(resolve => setTimeout(resolve, 70));

        bridge.tick();
        bridge.tick();
        bridge.tick();
        const beforeAck = sent.filter(m => m.cmd === 'tick' || m.cmd === 'run');
        bridge._handleJSON(JSON.stringify({ type: 'tick_complete', tick: 1 }));
        const afterFirstAck = sent.filter(m => m.cmd === 'tick' || m.cmd === 'run');
        bridge._handleJSON(JSON.stringify({ type: 'tick_complete', tick: 2 }));

        // Paused Step-by-N calls must remain exact rather than coalescing like
        // real-time playback demand.
        window.__ftdCtx.running = false;
        bridge.tick();
        bridge.tick();
        bridge.tick();
        const pausedBeforeAck = sent.filter(m => m.cmd === 'tick' || m.cmd === 'run');
        bridge._handleJSON(JSON.stringify({ type: 'tick_complete', tick: 3 }));
        const pausedAfterAck = sent.filter(m => m.cmd === 'tick' || m.cmd === 'run');
        bridge._handleJSON(JSON.stringify({ type: 'run_complete', tick: 5 }));

        delete window.__ftdCtx;
        return {
            fallbackCreated: bridge._fallback !== null,
            beforeAck,
            afterFirstAck,
            pausedBeforeAck,
            pausedAfterAck,
            inFlightAfterDrain: bridge._simulationInFlight,
            forcedUploads,
        };
    });

    expect(result.fallbackCreated).toBe(false);
    expect(result.beforeAck).toEqual([{ cmd: 'tick' }]);
    expect(result.afterFirstAck).toEqual([{ cmd: 'tick' }, { cmd: 'tick' }]);
    expect(result.pausedBeforeAck).toEqual([{ cmd: 'tick' }, { cmd: 'tick' }, { cmd: 'tick' }]);
    expect(result.pausedAfterAck).toEqual([
        { cmd: 'tick' }, { cmd: 'tick' }, { cmd: 'tick' }, { cmd: 'run', n: 2 },
    ]);
    expect(result.inFlightAfterDrain).toBe(false);
    // One refresh for the completed scenario transaction plus four completed
    // simulation commands.
    expect(result.forcedUploads).toBe(5);
});

test('native run is chunked, cancellable, and typed CUDA errors retire playback transport', async ({ page }) => {
    await page.goto('/js/ws-bridge.js', { waitUntil: 'domcontentloaded' });

    const result = await page.evaluate(async () => {
        const { WebSocketBridge } = await import('/js/ws-bridge.js?run-chunk-test=1');
        const bridge = new WebSocketBridge('ws://run-chunk-test');
        const sent = [];
        const completions = [];
        let pauses = 0;
        let reconnects = 0;
        bridge._connected = true;
        bridge.ready = true;
        bridge.latticeSize = 65;
        bridge._ws = { send(payload) { sent.push(JSON.parse(payload)); } };
        bridge._scheduleReconnect = () => { reconnects++; };
        window.__ftdCtx = {
            bridge,
            running: false,
            pauseSimulation() {
                pauses++;
                this.running = false;
            },
            onBridgeSimulationError() { this.pauseSimulation(); },
            onBridgeSimulationComplete(info) { completions.push(info); },
            onBridgePostFrame() {},
        };

        bridge.run(7);
        const first = sent.slice();
        const queuedBeforeCancel = bridge._queuedSimulationTicks;
        bridge.cancelQueuedTicks();
        bridge._handleJSON(JSON.stringify({ type: 'run_complete', tick: 2 }));
        const afterCancelAck = sent.slice();

        bridge.run(5);
        window.__ftdCtx.running = true;
        const beforeError = sent.slice();
        bridge._handleJSON(JSON.stringify({
            error: 'CUDA launch failed',
            operation: 'run',
        }));
        const releasedAfterError = !bridge._simulationInFlight
            && bridge._simulationTicksInFlight === 0
            && bridge._queuedSimulationTicks === 0;
        delete window.__ftdCtx;

        return {
            first,
            queuedBeforeCancel,
            afterCancelAck,
            beforeError,
            releasedAfterError,
            disconnectedAfterError: !bridge._connected && !bridge.ready,
            pauses,
            reconnects,
            completionTicks: completions.map(item => item.ticks),
        };
    });

    expect(result.first).toEqual([{ cmd: 'run', n: 2 }]);
    expect(result.queuedBeforeCancel).toBe(5);
    expect(result.afterCancelAck).toEqual(result.first);
    expect(result.beforeError.at(-1)).toEqual({ cmd: 'run', n: 2 });
    expect(result.releasedAfterError).toBe(true);
    expect(result.disconnectedAfterError).toBe(true);
    expect(result.pauses).toBe(1);
    expect(result.reconnects).toBe(1);
    expect(result.completionTicks).toEqual([2]);
});

test('stuck native tick watchdog emits an engine error and reconnects cleanly', async ({ page }) => {
    await page.goto('/js/ws-bridge.js', { waitUntil: 'domcontentloaded' });

    const result = await page.evaluate(async () => {
        const { WebSocketBridge } = await import('/js/ws-bridge.js?watchdog-recovery-test=1');
        const bridge = new WebSocketBridge('ws://watchdog-recovery-test');
        const originalSetTimeout = window.setTimeout;
        const originalCtx = window.__ftdCtx;
        const sent = [];
        const engineErrors = [];
        let watchdog = null;
        let closeCount = 0;
        let reconnectCount = 0;

        const onEngineError = event => engineErrors.push(event.detail);
        window.addEventListener('ftd:engine-error', onEngineError);
        window.setTimeout = (callback, delay, ...args) => {
            if (delay >= 100_000) {
                watchdog = () => callback(...args);
                return 4242;
            }
            return originalSetTimeout(callback, delay, ...args);
        };

        try {
            bridge._connected = true;
            bridge.ready = true;
            bridge._diagnosticsInFlight = true;
            bridge._energyInFlight = true;
            bridge._lagrangianInFlight = true;
            bridge._gravityMetricInFlight = true;
            bridge._particleRequestInFlight = true;
            bridge._volumeRequestInFlight = true;
            bridge._ws = {
                send(payload) { sent.push(JSON.parse(payload)); },
                close() { closeCount++; },
            };
            bridge._scheduleReconnect = () => { reconnectCount++; };
            window.__ftdCtx = { bridge, running: true };

            bridge.tick();
            watchdog?.();

            return {
                sent,
                hadWatchdog: typeof watchdog === 'function',
                connected: bridge._connected,
                ready: bridge.ready,
                simulationInFlight: bridge._simulationInFlight,
                simulationTicksInFlight: bridge._simulationTicksInFlight,
                queuedSimulationTicks: bridge._queuedSimulationTicks,
                diagnosticsInFlight: bridge._diagnosticsInFlight,
                energyInFlight: bridge._energyInFlight,
                lagrangianInFlight: bridge._lagrangianInFlight,
                gravityMetricInFlight: bridge._gravityMetricInFlight,
                particleRequestInFlight: bridge._particleRequestInFlight,
                volumeRequestInFlight: bridge._volumeRequestInFlight,
                closeCount,
                reconnectCount,
                engineErrors,
            };
        } finally {
            window.setTimeout = originalSetTimeout;
            window.__ftdCtx = originalCtx;
            window.removeEventListener('ftd:engine-error', onEngineError);
        }
    });

    expect(result.sent).toEqual([{ cmd: 'tick' }]);
    expect(result.hadWatchdog).toBe(true);
    expect(result.connected).toBe(false);
    expect(result.ready).toBe(false);
    expect(result.simulationInFlight).toBe(false);
    expect(result.simulationTicksInFlight).toBe(0);
    expect(result.queuedSimulationTicks).toBe(0);
    expect(result.diagnosticsInFlight).toBe(false);
    expect(result.energyInFlight).toBe(false);
    expect(result.lagrangianInFlight).toBe(false);
    expect(result.gravityMetricInFlight).toBe(false);
    expect(result.particleRequestInFlight).toBe(false);
    expect(result.volumeRequestInFlight).toBe(false);
    expect(result.closeCount).toBe(1);
    expect(result.reconnectCount).toBe(1);
    expect(result.engineErrors).toEqual([{
        operation: 'tick',
        error: 'Native tick timed out',
    }]);
});

test('native tick attempts do not advance Scale 0 state before server completion', async ({ page }) => {
    await page.goto('/index_dag.html', { waitUntil: 'domcontentloaded' });
    const result = await page.evaluate(async () => {
        const { runScale0PhysicsTicks } = await import('/js/scales/scale0/runtime/tick.js?native-version-test=1');
        let attempts = 0;
        const state = {
            fluxMock: null,
            useFluxMock: false,
            latticeNeedsUpload: false,
            fieldDataVersion: 10,
        };
        const ctx = {
            bridge: {
                isNativeGPU: true,
                capabilities: { scale0: { tickScale0() { attempts++; } } },
            },
        };
        runScale0PhysicsTicks(ctx, state, 3);
        return { attempts, version: state.fieldDataVersion, upload: state.latticeNeedsUpload };
    });
    expect(result).toEqual({ attempts: 3, version: 10, upload: false });
});

test('typed native visual errors release retry flags instead of freezing readbacks', async ({ page }) => {
    await page.goto('/js/ws-bridge.js', { waitUntil: 'domcontentloaded' });
    const result = await page.evaluate(async () => {
        const { WebSocketBridge } = await import('/js/ws-bridge.js?visual-error-test=1');
        const bridge = new WebSocketBridge('ws://visual-error-test');
        bridge._particleRequestInFlight = true;
        bridge._particleRequestEpoch = 4;
        bridge._volumeRequestInFlight = true;
        bridge._volumeRequestEpoch = 4;
        bridge._sliceRequestsInFlight.add('0_10');
        bridge._sliceRequestEpoch.set('0_10', 4);
        bridge._fieldSampleRequestTokenByKey.set('e@2', 7);
        bridge._fieldSampleRequestsByToken.set(7, { key: 'e@2', kind: 'e', stride: 2 });
        bridge._fieldSampleRequestEpoch.set('e@2', 4);

        for (const operation of ['get_particles', 'get_flux_volume', 'get_flux_slice', 'get_field_sample']) {
            bridge._handleJSON(JSON.stringify({ error: `${operation} failed`, operation }));
        }
        return {
            particle: bridge._particleRequestInFlight,
            particleEpoch: bridge._particleRequestEpoch,
            volume: bridge._volumeRequestInFlight,
            volumeEpoch: bridge._volumeRequestEpoch,
            slices: bridge._sliceRequestsInFlight.size,
            sliceEpochs: bridge._sliceRequestEpoch.size,
            fieldTokens: bridge._fieldSampleRequestsByToken.size,
            fieldKeys: bridge._fieldSampleRequestTokenByKey.size,
            fieldEpochs: bridge._fieldSampleRequestEpoch.size,
        };
    });
    expect(result).toEqual({
        particle: false,
        particleEpoch: 0,
        volume: false,
        volumeEpoch: 0,
        slices: 0,
        sliceEpochs: 0,
        fieldTokens: 0,
        fieldKeys: 0,
        fieldEpochs: 0,
    });
});

test('native-only constants and intent-only confinement are not presented as writable physics', async ({ page }) => {
    await page.goto('/js/ws-bridge.js', { waitUntil: 'domcontentloaded' });
    const result = await page.evaluate(async () => {
        const { createSubstrateControlsCard } = await import(
            '/js/scales/scale0/ui/controls/substrate-controls.js?native-readonly-test=1'
        );
        const { createPhysicsTogglesCard } = await import(
            '/js/scales/scale0/ui/controls/physics-toggles.js?native-readonly-test=1'
        );
        const { wireScale0Controls } = await import(
            '/js/scales/scale0/ui/controls/wire.js?native-readonly-test=1'
        );
        const { syncComboSliders } = await import(
            '/js/scales/scale0/runtime/scenario-loader.js?native-readonly-test=1'
        );
        const host = document.createElement('div');
        host.id = 'panel-controls';
        host.append(createSubstrateControlsCard(), createPhysicsTogglesCard());
        document.body.appendChild(host);
        const values = { kb: 0.5123, gn: 0.0123, damping: 0.008765 };
        const bridge = {
            isNativeGPU: true,
            isWasm: false,
            latticeSize: 33,
            capabilities: { scale0: { setToggle() {} } },
            setToggle() {},
            getParam(name) { return values[name]; },
        };
        const ctx = { bridge, viewport: {}, clearCharts() {} };
        wireScale0Controls(ctx, { setLatticeNeedsUpload() {} });
        syncComboSliders(ctx, { useFluxMock: false, fluxMock: null });
        const ids = ['combo-kb', 'combo-gn', 'combo-damp'];
        const sliders = ids.map(id => document.getElementById(id));
        const confinement = document.getElementById('t-confinement');
        return {
            disabled: sliders.map(el => el.disabled),
            readOnly: sliders.map(el => el.getAttribute('aria-readonly')),
            values: sliders.map(el => el.value),
            displays: ['combo-kb-val', 'combo-gn-val', 'combo-damp-val']
                .map(id => document.getElementById(id).textContent),
            badges: host.querySelectorAll('.ctrl-native-fixed').length,
            confinementDisabled: confinement.disabled,
            confinementChecked: confinement.checked,
            confinementLabel: confinement.closest('.toggle-row').querySelector('label').textContent,
        };
    });
    expect(result.disabled).toEqual([true, true, true]);
    expect(result.readOnly).toEqual(['true', 'true', 'true']);
    expect(result.values.map(Number)).toEqual([0.5123, 0.0123, 0.008765]);
    expect(result.displays).toEqual(['0.512', '0.012', '0.0088']);
    expect(result.badges).toBe(3);
    expect(result.confinementDisabled).toBe(true);
    expect(result.confinementChecked).toBe(false);
    expect(result.confinementLabel).toContain('visual proxy only');
});

test('scenario-local visibility tuning restores user volume controls on the next load', async ({ page }) => {
    await page.goto('/js/ws-bridge.js', { waitUntil: 'domcontentloaded' });
    const result = await page.evaluate(async () => {
        const {
            applyScenarioVisualProfile,
            captureOverlayPreferences,
            restoreOverlayPreferences,
        } = await import('/js/scales/scale0/runtime/scenario-loader.js?visual-pref-restore-test=1');
        document.body.innerHTML = `
            <button id="toggle-flux-volume" class="active"></button>
            <button id="toggle-flux-slice"></button>
            <button id="toggle-flux-lines"></button>
            <button id="toggle-b-field"></button>
            <input id="flux-point-scale" type="range" min="0.1" max="3" step="0.05" value="2.2">
            <span id="flux-point-scale-val">2.2</span>
            <input id="flux-threshold" type="range" min="0" max="0.1" step="0.0001" value="0.007">
            <span id="flux-threshold-val">0.007</span>
            <input id="flux-opacity" type="range" min="0" max="1" step="0.01" value="0.42">
            <span id="flux-opacity-val">0.42</span>
        `;
        const calls = [];
        const viewport = {
            setFluxPointScale(v) { calls.push(['point', v]); },
            setFluxSlicePointScale(v) { calls.push(['slicePoint', v]); },
            setFluxThreshold(v) { calls.push(['threshold', v]); },
            setFluxSliceThreshold(v) { calls.push(['sliceThreshold', v]); },
            setFluxOpacity(v) { calls.push(['opacity', v]); },
            setFluxSliceOpacity(v) { calls.push(['sliceOpacity', v]); },
        };
        const adapter = {
            raw: viewport,
            setFluxVolumeVisible(v) { calls.push(['volume', v]); },
            setFluxSliceVisible(v) { calls.push(['slice', v]); },
            setOverlayVisible() {},
            syncForceStyle() {},
            syncScalarRenderMode() {},
        };
        const ctx = { viewport };
        const state = { forceStyle: 'arrows', latticeNeedsUpload: false };
        const initial = captureOverlayPreferences(state, ctx);

        applyScenarioVisualProfile(
            ctx, state, adapter, 's0-field-electric-dipole', initial,
        );
        const duringDipole = {
            point: document.getElementById('flux-point-scale').value,
            threshold: document.getElementById('flux-threshold').value,
            opacity: document.getElementById('flux-opacity').value,
        };

        const nextScenarioPrefs = captureOverlayPreferences(state, ctx);
        restoreOverlayPreferences(nextScenarioPrefs, state, adapter);
        const afterRestore = {
            point: document.getElementById('flux-point-scale').value,
            threshold: document.getElementById('flux-threshold').value,
            opacity: document.getElementById('flux-opacity').value,
        };

        applyScenarioVisualProfile(
            ctx, state, adapter, 's0-field-uniform-b', nextScenarioPrefs,
        );
        const uniformBActive = document.getElementById('toggle-flux-volume')
            .classList.contains('active');
        const afterUniformBPrefs = captureOverlayPreferences(state, ctx);

        return {
            initial: {
                point: initial.fluxPointScale,
                threshold: initial.fluxThreshold,
                opacity: initial.fluxOpacity,
                volume: initial.fluxVolume,
            },
            duringDipole,
            restoredPrefs: {
                point: nextScenarioPrefs.fluxPointScale,
                threshold: nextScenarioPrefs.fluxThreshold,
                opacity: nextScenarioPrefs.fluxOpacity,
            },
            afterRestore,
            uniformBActive,
            volumeAfterUniformB: afterUniformBPrefs.fluxVolume,
            calls,
        };
    });

    expect(result.initial).toEqual({ point: 2.2, threshold: 0.007, opacity: 0.42, volume: true });
    expect(result.duringDipole).toEqual({ point: '2.6', threshold: '0.0001', opacity: '0.85' });
    expect(result.restoredPrefs).toEqual({ point: 2.2, threshold: 0.007, opacity: 0.42 });
    expect(result.afterRestore).toEqual({ point: '2.2', threshold: '0.007', opacity: '0.42' });
    expect(result.uniformBActive).toBe(false);
    expect(result.volumeAfterUniformB).toBe(true);
    expect(result.calls).toEqual(expect.arrayContaining([
        ['threshold', 0.0001],
        ['threshold', 0.007],
        ['volume', false],
    ]));
});

test('compact center seeds receive one large-L camera focus without overriding later manual motion', async ({ page }) => {
    await page.goto('/js/ws-bridge.js', { waitUntil: 'domcontentloaded' });
    const result = await page.evaluate(async () => {
        const { applyScenarioCameraFocus, SCALE0_SCENARIO_VISUAL_PROFILES } = await import(
            '/js/scales/scale0/runtime/scenario-loader.js?seed-camera-focus-test=1'
        );
        const vector = (x, y, z) => ({
            x, y, z,
            set(nx, ny, nz) { this.x = nx; this.y = ny; this.z = nz; },
        });
        let updates = 0;
        const viewport = {
            camera: { position: vector(120, 110, 250), fov: 60 },
            controls: {
                target: vector(90.5, 90.5, 90.5),
                minDistance: 0.1,
                maxDistance: 1000,
                update() { updates++; },
            },
        };
        const ctx = { viewport };
        const applied = applyScenarioCameraFocus(ctx, 's0-seed-moore-cell', 181, true);
        const focused = {
            target: [viewport.controls.target.x, viewport.controls.target.y, viewport.controls.target.z],
            position: [viewport.camera.position.x, viewport.camera.position.y, viewport.camera.position.z],
            distance: Math.hypot(
                viewport.camera.position.x - viewport.controls.target.x,
                viewport.camera.position.y - viewport.controls.target.y,
                viewport.camera.position.z - viewport.controls.target.z,
            ),
        };
        viewport.camera.position.set(11, 12, 13);
        viewport.controls.target.set(4, 5, 6);
        const reapplied = applyScenarioCameraFocus(ctx, 's0-seed-moore-cell', 181, false);
        const manual = {
            target: [viewport.controls.target.x, viewport.controls.target.y, viewport.controls.target.z],
            position: [viewport.camera.position.x, viewport.camera.position.y, viewport.camera.position.z],
        };
        viewport.camera.position.set(120, 110, 250);
        viewport.controls.target.set(90.5, 90.5, 90.5);
        const wilsonApplied = applyScenarioCameraFocus(
            ctx, 's0-seed-wilson-loop', 181, true,
        );
        const wilsonDistance = Math.hypot(
            viewport.camera.position.x - viewport.controls.target.x,
            viewport.camera.position.y - viewport.controls.target.y,
            viewport.camera.position.z - viewport.controls.target.z,
        );
        const ids = [
            's0-seed-octahedron',
            's0-seed-cuboctahedron',
            's0-seed-stella-octangula',
            's0-seed-moore-cell',
            's0-seed-moore-decomposition',
            's0-seed-observer-cell',
            's0-seed-massive-body',
        ];
        return {
            applied,
            reapplied,
            wilsonApplied,
            wilsonDistance,
            focused,
            manual,
            updates,
            radii: ids.map(id => SCALE0_SCENARIO_VISUAL_PROFILES[id]?.focusRadius),
        };
    });

    expect(result.applied).toBe(true);
    expect(result.reapplied).toBe(false);
    expect(result.wilsonApplied).toBe(true);
    expect(result.wilsonDistance).toBeGreaterThan(60);
    expect(result.wilsonDistance).toBeLessThan(75);
    expect(result.focused.target).toEqual([90.5, 90.5, 90.5]);
    expect(result.focused.distance).toBeGreaterThan(5);
    expect(result.focused.distance).toBeLessThan(20);
    expect(result.manual).toEqual({ target: [4, 5, 6], position: [11, 12, 13] });
    expect(result.updates).toBe(2);
    expect(result.radii).toEqual(new Array(7).fill(5));
});

test('native binary flux-volume frame decodes without JSON allocation', async ({ page }) => {
    await page.goto('/js/ws-bridge.js', { waitUntil: 'domcontentloaded' });

    const result = await page.evaluate(async () => {
        const { WebSocketBridge } = await import('/js/ws-bridge.js?binary-volume-test=1');
        const bridge = new WebSocketBridge('ws://binary-volume-test');
        bridge._connected = true;
        bridge.ready = true;
        bridge._volumeRequestInFlight = true;
        const frame = new ArrayBuffer(8 + 3 * 4);
        const header = new DataView(frame);
        header.setUint32(0, 0x31565446, true);
        header.setUint32(4, 3, true);
        new Float32Array(frame, 8).set([0, 0.5, 1]);
        bridge._handleBinary(frame);
        return {
            values: Array.from(bridge._volumeCache),
            inFlight: bridge._volumeRequestInFlight,
            type: bridge._volumeCache.constructor.name,
        };
    });

    expect(result.values).toEqual([0, 0.5, 1]);
    expect(result.inFlight).toBe(false);
    expect(result.type).toBe('Float32Array');
});

test('native FTV2 flux volume requests and caches only the renderer-sized grid', async ({ page }) => {
    await page.goto('/js/ws-bridge.js', { waitUntil: 'domcontentloaded' });

    const result = await page.evaluate(async () => {
        const { WebSocketBridge } = await import('/js/ws-bridge.js?compact-volume-test=1');
        const bridge = new WebSocketBridge('ws://compact-volume-test');
        const sent = [];
        bridge._connected = true;
        bridge.ready = true;
        bridge.latticeSize = 181;
        bridge._ws = { send(payload) { sent.push(JSON.parse(payload)); } };

        bridge.getFluxVolume();
        const axisCount = 3;
        const count = axisCount ** 3;
        const frame = new ArrayBuffer(16 + count * 4);
        const header = new DataView(frame);
        header.setUint32(0, 0x32565446, true);
        header.setUint32(4, 181, true);
        header.setUint32(8, 4, true);
        header.setUint32(12, axisCount, true);
        const density = new Float32Array(frame, 16, count);
        density[0] = 0.25;
        density[count - 1] = 2.5;
        bridge._handleBinary(frame);

        return {
            request: sent[0],
            cache: {
                latticeSize: bridge._volumeCache.latticeSize,
                stride: bridge._volumeCache.stride,
                origin: bridge._volumeCache.origin,
                axisCount: bridge._volumeCache.axisCount,
                length: bridge._volumeCache.data.length,
                first: bridge._volumeCache.data[0],
                last: bridge._volumeCache.data[count - 1],
            },
        };
    });

    expect(result.request).toEqual({ cmd: 'get_flux_volume', axisSamples: 53 });
    expect(result.cache).toEqual({
        latticeSize: 181,
        stride: 4,
        origin: expect.any(Number),
        axisCount: 3,
        length: 27,
        first: 0.25,
        last: 2.5,
    });
});

test('compact FTV2 descriptor renders at native sampled lattice coordinates without expansion', async ({ page }) => {
    await page.goto('/index_dag.html', { waitUntil: 'domcontentloaded' });
    await page.setContent(`
        <script type="importmap">
        {"imports":{"three":"/js/vendor/three/build/three.module.js","three/addons/":"/js/vendor/three/examples/jsm/"}}
        </script>
    `);

    const result = await page.evaluate(async () => {
        const THREE = await import('three');
        const { ViewportFluxRenderer } = await import('/js/viewport/flux-renderer.js?compact-render-test=1');
        const scene = new THREE.Scene();
        const renderer = new ViewportFluxRenderer({
            scene,
            latticeSize: 9,
            halfN: 4.5,
            boundaryShape: 'cube',
            insideBoundary: () => true,
            applyScenarioScale: () => {},
            buildStreamlineMesh: () => null,
            writeStreamlinesIntoMesh: () => {},
        });
        renderer._fluxOrganic = false;
        const density = new Float32Array(27);
        density[0] = 1;
        density[26] = 2;
        renderer.updateFluxVolume({
            data: density,
            latticeSize: 9,
            stride: 4,
            axisCount: 3,
        }, 9);
        const geometry = renderer._fluxVolume.geometry;
        const positions = Array.from(geometry.getAttribute('position').array.slice(0, 6));
        return {
            drawCount: geometry.drawRange.count,
            positions,
            capacity: geometry.getAttribute('position').count,
        };
    });

    expect(result.drawCount).toBe(2);
    expect(result.positions).toEqual([0.5, 0.5, 0.5, 8.5, 8.5, 8.5]);
    expect(result.capacity).toBe(9 ** 3);
});

test('FTV2 remains live for derived topology and gravity consumers', async ({ page }) => {
    await page.goto('/index_dag.html', { waitUntil: 'domcontentloaded' });
    await page.setContent(`
        <script type="importmap">
        {"imports":{"three":"/js/vendor/three/build/three.module.js","three/addons/":"/js/vendor/three/examples/jsm/"}}
        </script>
    `);
    const result = await page.evaluate(async () => {
        const THREE = await import('three');
        const { K_GENESIS } = await import('/js/constants.js');
        const { buildDerivedSubstrateData } = await import(
            '/js/scales/scale0/runtime/field-overlays.js?compact-consumers-test=1'
        );
        const { fieldTopologyMethods } = await import(
            '/js/viewport/field-topology-renderer.js?compact-consumers-test=1'
        );
        const { gravitySlice, maxRhoOf } = await import(
            '/js/scales/scale0/analysis/gravity-analysis.js?compact-consumers-test=1'
        );
        const data = new Float32Array(27).fill(0.1);
        data[13] = K_GENESIS;
        const volume = { data, latticeSize: 9, stride: 4, axisCount: 3 };
        const frame = buildDerivedSubstrateData({
            fieldFlags: { showDarkMatterHalo: true, showGenesisIsosurface: true },
        }, {}, {
            getScale0FluxVolume: () => volume,
            getScale0ParticleFrame: () => ({ positions: new Float32Array(0), count: 0 }),
        }, 9);

        const topology = {
            _scene: new THREE.Scene(),
            _syncCenterAndRadius() {},
            _darkMatterHalo: null,
            _genesisIsosurface: null,
        };
        Object.assign(topology, fieldTopologyMethods);
        topology.updateDarkMatterHalo(null, frame.darkMatterHalo.magnitude, 9);
        topology.updateGenesisIsosurface(frame.genesisIsosurface.magnitude, 9, K_GENESIS);

        const rho = maxRhoOf(data, data.length);
        const gravity = gravitySlice(data, 3, 0, 1, 'latency', rho, 4);
        return {
            descriptorPreserved: frame.darkMatterHalo.magnitude === volume
                && frame.genesisIsosurface.magnitude === volume,
            darkCount: topology._darkMatterHalo.geometry.drawRange.count,
            genesisCount: topology._genesisIsosurface.geometry.drawRange.count,
            gravityMax: Math.max(...gravity),
            gravityLength: gravity.length,
        };
    });
    expect(result.descriptorPreserved).toBe(true);
    expect(result.darkCount).toBeGreaterThan(0);
    expect(result.genesisCount).toBeGreaterThan(0);
    expect(result.gravityMax).toBeGreaterThan(0);
    expect(result.gravityLength).toBe(9);
});

test('native flux-slice panel cadence scales down with lattice size', async ({ page }) => {
    await page.goto('/index_dag.html', { waitUntil: 'domcontentloaded' });
    const result = await page.evaluate(async () => {
        const { effectiveFluxSliceUpdateEvery } = await import(
            '/js/scales/scale0/ui/overlays/flux-slice-panel.js?native-cadence-test=1'
        );
        return {
            wasm: effectiveFluxSliceUpdateEvery({ isNativeGPU: false, latticeSize: 181 }, 2),
            native33: effectiveFluxSliceUpdateEvery({ isNativeGPU: true, latticeSize: 33 }, 2),
            native65: effectiveFluxSliceUpdateEvery({ isNativeGPU: true, latticeSize: 65 }, 2),
            native181: effectiveFluxSliceUpdateEvery({ isNativeGPU: true, latticeSize: 181 }, 2),
        };
    });
    expect(result).toEqual({ wasm: 2, native33: 4, native65: 6, native181: 8 });
});

test('slice rasterizers choose CUDA nearest sampled plane when the midpoint is absent', async ({ page }) => {
    await page.goto('/index_dag.html', { waitUntil: 'domcontentloaded' });
    const result = await page.evaluate(async () => {
        const {
            nearestSamplePlane,
            resolveSamplePlane,
            sliceVectorMag,
            sliceScalarSigned,
            sliceDerivedFrame,
        } = await import('/js/scales/scale0/ui/overlays/flux-slice-helpers.js?nearest-plane-test=1');
        // Interior CUDA sampling can return ...88,91,... for requested mid=90.
        const positions = new Float32Array([
            88.5, 10.5, 20.5,
            91.5, 10.5, 20.5,
        ]);
        const vector = {
            positions,
            vectors: new Float32Array([2, 0, 0, 7, 0, 0]),
            count: 2,
        };
        const scalar = {
            positions,
            values: new Float32Array([2, -7]),
            count: 2,
        };
        const distantOnly = {
            positions: new Float32Array([10.5, 10.5, 20.5]),
            values: new Float32Array([99]),
            count: 1,
            effectiveStride: 3,
            origin: 1,
        };
        const N = 181;
        const idx = (N - 1 - 20) * N + 10;
        return {
            plane: nearestSamplePlane(positions, 2, 0, 90),
            metadataPlane: resolveSamplePlane(distantOnly, 0, 90, N),
            zeroMidplane: sliceScalarSigned(distantOnly, 0, 90, N)[idx],
            vector: sliceVectorMag(vector, 0, 90, N)[idx],
            scalar: sliceScalarSigned(scalar, 0, 90, N)[idx],
            derived: sliceDerivedFrame(scalar, 0, 90, N)[idx],
        };
    });
    expect(result).toEqual({
        plane: 91,
        metadataPlane: 91,
        zeroMidplane: 0,
        vector: 7,
        scalar: -7,
        derived: -7,
    });
});

test('native FTP2 particles preserve mechanical positions, spin, and color charge', async ({ page }) => {
    await page.goto('/js/ws-bridge.js', { waitUntil: 'domcontentloaded' });

    const result = await page.evaluate(async () => {
        const { WebSocketBridge } = await import('/js/ws-bridge.js?ftp2-test=1');
        const bridge = new WebSocketBridge('ws://ftp2-test');
        const count = 2;
        const frame = new ArrayBuffer(8 + count * 9 * 4);
        const header = new DataView(frame);
        header.setUint32(0, 0x32505446, true);
        header.setUint32(4, count, true);
        let offset = 8;
        new Float32Array(frame, offset, count * 3).set([1.7, 2.5, 3.5, 8.2, 9.5, 10.5]); offset += count * 3 * 4;
        new Float32Array(frame, offset, count * 3).set([0.29, 0.87, 0.5, 0.97, 0.44, 0.44]); offset += count * 3 * 4;
        new Float32Array(frame, offset, count).set([6, 6]); offset += count * 4;
        new Float32Array(frame, offset, count).set([1, -1]); offset += count * 4;
        new Float32Array(frame, offset, count).set([2, 3]);
        bridge._handleBinary(frame);
        const list = bridge.getScale0ParticleList();
        return {
            positions: Array.from(bridge._particleData.positions),
            spin: Array.from(bridge._particleData.spin),
            colorCharge: Array.from(bridge._particleData.colorCharge),
            count: bridge._particleData.count,
            list,
        };
    });

    expect(result.count).toBe(2);
    expect(result.positions[0]).toBeCloseTo(1.7, 5);
    expect(result.positions[3]).toBeCloseTo(8.2, 5);
    expect(result.spin).toEqual([1, -1]);
    expect(result.colorCharge).toEqual([2, 3]);
    expect(result.list).toEqual([
        expect.objectContaining({ x: 1, y: 2, z: 3, state: 1, charge: 1, spin: 1, color: 2 }),
        expect.objectContaining({ x: 8, y: 9, z: 10, state: -1, charge: -1, spin: -1, color: 3 }),
    ]);
});

test('tracked native responses are correlated and fire-and-forget errors cannot steal them', async ({ page }) => {
    await page.goto('/js/ws-bridge.js', { waitUntil: 'domcontentloaded' });

    const result = await page.evaluate(async () => {
        const { WebSocketBridge } = await import('/js/ws-bridge.js?request-id-test=1');
        const bridge = new WebSocketBridge('ws://request-id-test');
        const sent = [];
        bridge._connected = true;
        bridge.ready = true;
        bridge._ws = { send(payload) { sent.push(JSON.parse(payload)); } };

        const firstPromise = bridge._sendJSON({ cmd: 'first' });
        const secondPromise = bridge._sendJSON({ cmd: 'second' });
        bridge._handleJSON(JSON.stringify({ error: 'untracked mutation failed' }));
        bridge._handleJSON(JSON.stringify({
            ok: true,
            value: 'second',
            _requestId: sent[1]._requestId,
        }));
        const second = await secondPromise;
        const pendingAfterSecond = bridge._pendingQueue.length;
        bridge._handleJSON(JSON.stringify({
            ok: true,
            value: 'first',
            _requestId: sent[0]._requestId,
        }));
        const first = await firstPromise;

        return {
            distinctIds: sent[0]._requestId !== sent[1]._requestId,
            first: first.value,
            second: second.value,
            pendingAfterSecond,
            pendingAtEnd: bridge._pendingQueue.length,
        };
    });

    expect(result.distinctIds).toBe(true);
    expect(result.first).toBe('first');
    expect(result.second).toBe('second');
    expect(result.pendingAfterSecond).toBe(1);
    expect(result.pendingAtEnd).toBe(0);
});

test('native scenario profile is serialized atomically after all loader mutations', async ({ page }) => {
    await page.goto('/js/ws-bridge.js', { waitUntil: 'domcontentloaded' });

    const result = await page.evaluate(async () => {
        const { WebSocketBridge } = await import('/js/ws-bridge.js?scenario-profile-test=1');
        const bridge = new WebSocketBridge('ws://scenario-profile-test');
        const tracked = [];
        const fireAndForget = [];
        bridge._connected = true;
        bridge.ready = true;
        bridge._ws = { send(payload) { fireAndForget.push(JSON.parse(payload)); } };
        const clusterPresentBeforeAck = Object.prototype.hasOwnProperty.call(
            bridge._toggles, 'cluster_inertia',
        );
        bridge._toggles.stale_client_only = true;
        bridge._sendJSON = async (message) => {
            tracked.push(message);
            return {
                ok: true,
                scenario: message.name,
                latticeSize: bridge.latticeSize,
                fluxBoundaryMode: message.fluxBoundaryMode,
                toggles: {
                    wave_propagation: message.toggle_wave_propagation,
                    damping: message.toggle_damping,
                    selective_damping: message.toggle_selective_damping,
                    latency_field: message.toggle_latency_field,
                    // This term intentionally was not present in the bridge's
                    // bootstrap cache. Engine truth must still surface it.
                    cluster_inertia: true,
                },
                params: {},
            };
        };

        bridge.beginScenarioConfiguration('s0-seed-massive-body');
        bridge.setToggle('wave_propagation', false);
        bridge.setToggle('damping', false);
        bridge.setToggle('selective_damping', false);
        bridge.setupScenario('s0-seed-massive-body');
        bridge.setFluxBoundaryMode(1);
        bridge.setToggle('latency_field', true);
        bridge.commitScenarioConfiguration('s0-seed-massive-body');
        await new Promise(resolve => setTimeout(resolve, 80));

        return {
            tracked,
            fireAndForget,
            activeScenario: bridge._activeScenario,
            boundary: bridge._fluxBoundaryMode,
            latency: bridge.getToggle('latency_field'),
            clusterPresentBeforeAck,
            clusterAfterAck: bridge.getToggle('cluster_inertia'),
            staleAfterAck: bridge.getToggle('stale_client_only'),
        };
    });

    expect(result.fireAndForget).toEqual([]);
    expect(result.tracked).toHaveLength(1);
    expect(result.tracked[0]).toMatchObject({
        cmd: 'setup_scenario',
        name: 's0-seed-massive-body',
        applyProfile: true,
        fluxBoundaryMode: 1,
        toggle_wave_propagation: false,
        toggle_damping: false,
        toggle_selective_damping: false,
        toggle_latency_field: true,
    });
    expect(result.activeScenario).toBe('s0-seed-massive-body');
    expect(result.boundary).toBe(1);
    expect(result.latency).toBe(true);
    expect(result.clusterPresentBeforeAck).toBe(false);
    expect(result.clusterAfterAck).toBe(true);
    expect(result.staleAfterAck).toBe(false);
});

test('rejected native scenario profile restores authoritative toggles, boundary, and params', async ({ page }) => {
    await page.goto('/js/ws-bridge.js', { waitUntil: 'domcontentloaded' });

    const result = await page.evaluate(async () => {
        const { WebSocketBridge } = await import('/js/ws-bridge.js?scenario-rejection-test=1');
        const bridge = new WebSocketBridge('ws://scenario-rejection-test');
        const callbacks = [];
        bridge._connected = true;
        bridge.ready = true;
        bridge._ws = { send() {} };
        bridge._toggles = { wave_propagation: true, damping: false };
        bridge._confirmedToggles = { ...bridge._toggles };
        bridge._fluxBoundaryMode = 0;
        bridge._confirmedFluxBoundaryMode = 0;
        bridge._params = { dt: 0.25, omega0: 0.1 };
        bridge._confirmedParams = { ...bridge._params };
        window.__ftdCtx = {
            bridge,
            onBridgeProfileUpdate(profile) { callbacks.push(profile); },
        };
        bridge._sendJSON = async () => ({
            ok: false,
            error: 'profile dependency rejected',
            scenario: 'empty',
            latticeSize: 65,
            fluxBoundaryMode: 0,
            toggles: { wave_propagation: true, damping: false },
            params: { dt: 0.25, omega0: 0.1 },
        });

        bridge.beginScenarioConfiguration('empty', 91);
        bridge.setToggle('wave_propagation', false);
        bridge.setToggle('damping', true);
        bridge.setFluxBoundaryMode(2);
        bridge._params = { dt: 0.9, omega0: 0.8 };
        bridge.setupScenario('empty');
        bridge.commitScenarioConfiguration('empty');
        await new Promise(resolve => setTimeout(resolve, 80));

        delete window.__ftdCtx;
        return {
            toggles: { ...bridge._toggles },
            confirmedToggles: { ...bridge._confirmedToggles },
            boundary: bridge._fluxBoundaryMode,
            confirmedBoundary: bridge._confirmedFluxBoundaryMode,
            params: { ...bridge._params },
            confirmedParams: { ...bridge._confirmedParams },
            latticeSize: bridge.latticeSize,
            callback: callbacks.at(-1),
        };
    });

    expect(result.toggles).toEqual({ wave_propagation: true, damping: false });
    expect(result.confirmedToggles).toEqual(result.toggles);
    expect(result.boundary).toBe(0);
    expect(result.confirmedBoundary).toBe(0);
    expect(result.params).toEqual({ dt: 0.25, omega0: 0.1 });
    expect(result.confirmedParams).toEqual(result.params);
    expect(result.latticeSize).toBe(65);
    expect(result.callback).toMatchObject({
        error: 'profile dependency rejected',
        authoritativeScenarioAck: true,
        scenarioId: 'empty',
        loadGeneration: 91,
        latticeSize: 65,
        toggles: { wave_propagation: true, damping: false },
        fluxBoundaryMode: 0,
    });
});

test('native scenario acknowledgement mismatch is rejected before qualification callback', async ({ page }) => {
    await page.goto('/js/ws-bridge.js', { waitUntil: 'domcontentloaded' });

    const result = await page.evaluate(async () => {
        const { WebSocketBridge } = await import('/js/ws-bridge.js?scenario-mismatch-test=1');
        const bridge = new WebSocketBridge('ws://scenario-mismatch-test');
        const callbacks = [];
        bridge._connected = true;
        bridge.ready = true;
        bridge._ws = { send() {} };
        window.__ftdCtx = {
            bridge,
            onBridgeProfileUpdate(profile) { callbacks.push(profile); },
        };
        bridge._sendJSON = async message => ({
            ok: true,
            scenario: 'flux-pulse',
            latticeSize: 33,
            fluxBoundaryMode: message.fluxBoundaryMode,
            toggles: { wave_propagation: message.toggle_wave_propagation },
            params: {},
        });

        bridge.beginScenarioConfiguration('empty', 92);
        bridge.setToggle('wave_propagation', false);
        bridge.setupScenario('empty');
        bridge.commitScenarioConfiguration('empty');
        await new Promise(resolve => setTimeout(resolve, 80));

        delete window.__ftdCtx;
        return {
            activeScenario: bridge._activeScenario,
            callback: callbacks.at(-1),
        };
    });

    expect(result.activeScenario).toBeNull();
    expect(result.callback).toMatchObject({
        authoritativeScenarioAck: true,
        scenarioId: 'empty',
        loadGeneration: 92,
    });
    expect(result.callback.error).toContain('Scenario acknowledgement mismatch');
    expect(result.callback.error).toContain('scenario echo flux-pulse != empty');
});

test('rapid native scenario changes coalesce to the latest pending allocation', async ({ page }) => {
    await page.goto('/js/ws-bridge.js', { waitUntil: 'domcontentloaded' });

    const result = await page.evaluate(async () => {
        const { WebSocketBridge } = await import('/js/ws-bridge.js?scenario-coalesce-test=1');
        const bridge = new WebSocketBridge('ws://scenario-coalesce-test');
        const sent = [];
        const resolvers = [];
        const callbacks = [];
        bridge._connected = true;
        bridge.ready = true;
        bridge._ws = { send() {} };
        window.__ftdCtx = {
            bridge,
            onBridgeProfileUpdate(profile) { callbacks.push(profile); },
        };
        bridge._sendJSON = (message) => {
            sent.push(message);
            return new Promise(resolve => resolvers.push(() => resolve({
                ok: true,
                scenario: message.name,
                latticeSize: bridge.latticeSize,
                fluxBoundaryMode: message.fluxBoundaryMode,
                toggles: { wave_propagation: message.toggle_wave_propagation },
                params: {},
            })));
        };

        const stage = (name, loadGeneration) => {
            bridge.beginScenarioConfiguration(name, loadGeneration);
            bridge.setToggle('wave_propagation', true);
            bridge.setupScenario(name);
            bridge.commitScenarioConfiguration(name);
        };

        stage('flux-pulse', 11);
        await new Promise(resolve => setTimeout(resolve, 70)); // first allocation is now in flight
        stage('flux-dipole', 12);
        stage('flux-vortex', 13);
        stage('flux-standing', 14);
        resolvers.shift()();
        await new Promise(resolve => setTimeout(resolve, 25));
        resolvers.shift()();
        await new Promise(resolve => setTimeout(resolve, 0));

        delete window.__ftdCtx;
        return {
            commands: sent.map(({ cmd, name }) => ({ cmd, name })),
            activeScenario: bridge._activeScenario,
            inFlight: bridge._scenarioRequestInFlight,
            acknowledgements: callbacks.map((profile) => ({
                authoritativeScenarioAck: profile.authoritativeScenarioAck,
                scenarioId: profile.scenarioId,
                loadGeneration: profile.loadGeneration,
            })),
        };
    });

    expect(result.commands).toEqual([
        { cmd: 'setup_scenario', name: 'flux-pulse' },
        { cmd: 'setup_scenario', name: 'flux-standing' },
    ]);
    expect(result.activeScenario).toBe('flux-standing');
    expect(result.inFlight).toBe(false);
    expect(result.acknowledgements).toEqual([
        { authoritativeScenarioAck: true, scenarioId: 'flux-pulse', loadGeneration: 11 },
        { authoritativeScenarioAck: true, scenarioId: 'flux-standing', loadGeneration: 14 },
    ]);
});

test('prepared native resize applies its profile without rebuilding the lattice again', async ({ page }) => {
    await page.goto('/js/ws-bridge.js', { waitUntil: 'domcontentloaded' });

    const result = await page.evaluate(async () => {
        const { WebSocketBridge } = await import('/js/ws-bridge.js?prepared-profile-test=1');
        const bridge = new WebSocketBridge('ws://prepared-profile-test');
        const sent = [];
        bridge._connected = true;
        bridge.ready = true;
        bridge._ws = { send() {} };
        bridge._preparedScenario = { name: 'flux-pulse', clientLoadGeneration: 71 };
        bridge._sendJSON = async message => {
            sent.push(message);
            return {
                ok: true,
                scenario: message.name,
                latticeSize: bridge.latticeSize,
                fluxBoundaryMode: message.fluxBoundaryMode,
                toggles: { wave_propagation: message.toggle_wave_propagation },
                params: {},
            };
        };

        bridge.beginScenarioConfiguration('flux-pulse', 71);
        bridge.setToggle('wave_propagation', true);
        bridge.setupScenario('flux-pulse');
        bridge.commitScenarioConfiguration('flux-pulse');
        await new Promise(resolve => setTimeout(resolve, 80));
        return sent;
    });

    expect(result).toHaveLength(1);
    expect(result[0]).toMatchObject({
        cmd: 'apply_profile',
        name: 'flux-pulse',
        toggle_wave_propagation: true,
    });
});

test('stale prepared native resize cannot survive scenario-generation turnover', async ({ page }) => {
    await page.goto('/js/ws-bridge.js', { waitUntil: 'domcontentloaded' });

    const result = await page.evaluate(async () => {
        const { WebSocketBridge } = await import('/js/ws-bridge.js?prepared-turnover-test=1');
        const bridge = new WebSocketBridge('ws://prepared-turnover-test');
        const sent = [];
        bridge._connected = true;
        bridge.ready = true;
        bridge._ws = { send() {} };
        bridge._sendJSON = async message => {
            sent.push(message);
            return {
                ok: true,
                scenario: message.name,
                latticeSize: bridge.latticeSize,
                fluxBoundaryMode: message.fluxBoundaryMode,
                toggles: { wave_propagation: message.toggle_wave_propagation },
                params: {},
            };
        };

        // Models a resize ACK arriving after generation 80 was superseded.
        bridge._preparedScenario = { name: 'flux-pulse', clientLoadGeneration: 80 };
        bridge.beginScenarioConfiguration('flux-pulse', 82);
        bridge.setToggle('wave_propagation', true);
        bridge.setupScenario('flux-pulse');
        bridge.commitScenarioConfiguration('flux-pulse');
        await new Promise(resolve => setTimeout(resolve, 80));
        return {
            commands: sent.map(({ cmd, name }) => ({ cmd, name })),
            preparedScenario: bridge._preparedScenario,
        };
    });

    expect(result.commands).toEqual([{ cmd: 'setup_scenario', name: 'flux-pulse' }]);
    expect(result.preparedScenario).toBeNull();
});

test('superseded native resize stops after preflight before dispatch', async ({ page }) => {
    await page.goto('/js/ws-bridge.js', { waitUntil: 'domcontentloaded' });

    const result = await page.evaluate(async () => {
        const { WebSocketBridge } = await import('/js/ws-bridge.js?resize-preflight-turnover-test=1');
        const bridge = new WebSocketBridge('ws://resize-preflight-turnover-test');
        const sent = [];
        let current = true;
        let releasePreflight;
        bridge._connected = true;
        bridge.ready = true;
        bridge._ws = { send() {} };
        bridge.preflightResize = () => new Promise(resolve => { releasePreflight = resolve; });
        bridge._sendOperationWithRetry = async command => {
            sent.push(command);
            return { ok: true, latticeSize: command.size };
        };
        const pending = bridge.resizeScenario(49, 'flux-pulse', 90, () => current)
            .then(() => ({ resolved: true }))
            .catch(error => ({
                resolved: false,
                superseded: !!error.resizeSuperseded,
                phase: error.resizeFailurePhase,
            }));
        current = false;
        releasePreflight({ ok: true, size: 49 });
        const outcome = await pending;
        return { outcome, sent, preparedScenario: bridge._preparedScenario };
    });

    expect(result.outcome).toEqual({
        resolved: false,
        superseded: true,
        phase: 'preflight',
    });
    expect(result.sent).toEqual([]);
    expect(result.preparedScenario).toBeNull();
});

test('native FTS1 field samples are deduplicated, decoded, and refreshed by epoch', async ({ page }) => {
    await page.goto('/js/ws-bridge.js', { waitUntil: 'domcontentloaded' });

    const result = await page.evaluate(async () => {
        const { WebSocketBridge } = await import('/js/ws-bridge.js?field-sample-test=1');
        const bridge = new WebSocketBridge('ws://field-sample-test');
        const sent = [];
        const callbacks = [];
        bridge._connected = true;
        bridge.ready = true;
        bridge._ws = { send(payload) { sent.push(JSON.parse(payload)); } };
        window.__ftdCtx = {
            bridge,
            running: true,
            onBridgePostFrame(hadSamplers, forceUpload) {
                callbacks.push({ hadSamplers, forceUpload });
            },
        };

        bridge.getEFieldSampled(3);
        bridge.getEFieldSampled(3);
        const firstRequest = sent.find(message => message.cmd === 'get_field_sample');

        const makeFrame = (token, values) => {
            const count = 1;
            const frame = new ArrayBuffer(20 + count * 6 * 4);
            const header = new DataView(frame);
            header.setUint32(0, 0x31535446, true);
            header.setUint32(4, token, true);
            header.setUint32(8, 0, true); // e
            header.setUint32(12, 3, true);
            header.setUint32(16, count, true);
            new Float32Array(frame, 20, 3).set([1.5, 2.5, 3.5]);
            new Float32Array(frame, 32, 3).set(values);
            return frame;
        };
        bridge._handleBinary(makeFrame(firstRequest.token, [4, 5, 6]));
        const firstSample = bridge.getEFieldSampled(3);
        const sameEpochRequests = sent.filter(message => message.cmd === 'get_field_sample').length;

        bridge._handleJSON(JSON.stringify({ type: 'tick_complete', tick: 1 }));
        bridge.getEFieldSampled(3);
        bridge.getEFieldSampled(3);
        const requestsAfterTick = sent.filter(message => message.cmd === 'get_field_sample');
        const secondRequest = requestsAfterTick[1];
        bridge._handleBinary(makeFrame(secondRequest.token, [7, 8, 9]));
        const secondSample = bridge.getEFieldSampled(3);
        delete window.__ftdCtx;

        return {
            firstCommand: firstRequest,
            firstPositions: Array.from(firstSample.positions),
            firstVectors: Array.from(firstSample.vectors),
            secondVectors: Array.from(secondSample.vectors),
            sameEpochRequests,
            requestsAfterTick: requestsAfterTick.length,
            callbacks,
        };
    });

    expect(result.firstCommand).toMatchObject({ cmd: 'get_field_sample', kind: 'e', stride: 3 });
    expect(result.firstPositions).toEqual([1.5, 2.5, 3.5]);
    expect(result.firstVectors).toEqual([4, 5, 6]);
    expect(result.secondVectors).toEqual([7, 8, 9]);
    expect(result.sameEpochRequests).toBe(1);
    expect(result.requestsAfterTick).toBe(2);
    expect(result.callbacks).toContainEqual({ hadSamplers: true, forceUpload: false });
});

test('native FTS2 preserves effective stride and origin independently of sparse payload', async ({ page }) => {
    await page.goto('/js/ws-bridge.js', { waitUntil: 'domcontentloaded' });
    const result = await page.evaluate(async () => {
        const { WebSocketBridge } = await import('/js/ws-bridge.js?fts2-test=1');
        const bridge = new WebSocketBridge('ws://fts2-test');
        const sent = [];
        bridge._connected = true;
        bridge.ready = true;
        bridge.latticeSize = 181;
        bridge._ws = { send(payload) { sent.push(JSON.parse(payload)); } };
        bridge.getVorticitySampled(2);
        const request = sent[0];
        const frame = new ArrayBuffer(28 + 4 * 4);
        const header = new DataView(frame);
        header.setUint32(0, 0x32535446, true);
        header.setUint32(4, request.token, true);
        header.setUint32(8, 5, true); // vorticity
        header.setUint32(12, 1, true);
        header.setUint32(16, 1, true);
        header.setUint32(20, 3, true);
        header.setUint32(24, 1, true);
        new Float32Array(frame, 28, 3).set([10.5, 10.5, 10.5]);
        new Float32Array(frame, 40, 1).set([4.25]);
        bridge._handleBinary(frame);
        const sample = bridge.getVorticitySampled(2);

        bridge.getPoissonLatencySampled(4);
        const poissonRequest = sent[1];
        const poissonFrame = new ArrayBuffer(28 + 4 * 4);
        const poissonHeader = new DataView(poissonFrame);
        poissonHeader.setUint32(0, 0x32535446, true);
        poissonHeader.setUint32(4, poissonRequest.token, true);
        poissonHeader.setUint32(8, 17, true); // real voxel.latency
        poissonHeader.setUint32(12, 1, true);
        poissonHeader.setUint32(16, 1, true);
        poissonHeader.setUint32(20, 4, true);
        poissonHeader.setUint32(24, 0, true);
        new Float32Array(poissonFrame, 28, 3).set([90.5, 90.5, 90.5]);
        new Float32Array(poissonFrame, 40, 1).set([0.875]);
        bridge._handleBinary(poissonFrame);
        const poisson = bridge.getPoissonLatencySampled(4);
        return {
            request,
            effectiveStride: sample.effectiveStride,
            origin: sample.origin,
            positions: Array.from(sample.positions),
            values: Array.from(sample.values),
            poissonRequest,
            poissonKind: poisson.kind,
            poissonValues: Array.from(poisson.values),
        };
    });
    expect(result.request).toMatchObject({ cmd: 'get_field_sample', kind: 'vorticity', stride: 2 });
    expect(result).toMatchObject({
        effectiveStride: 3,
        origin: 1,
        positions: [10.5, 10.5, 10.5],
        values: [4.25],
        poissonRequest: { cmd: 'get_field_sample', kind: 'poissonLatency', stride: 4 },
        poissonKind: 'poissonLatency',
        poissonValues: [0.875],
    });
});

test('native mass-gravity 3D latency selects real Poisson samples without replacing proxy latency', async ({ page }) => {
    await page.goto('/js/ws-bridge.js', { waitUntil: 'domcontentloaded' });
    const result = await page.evaluate(async () => {
        const { scale0FieldKindOverrides } = await import(
            '/js/scales/scale0/runtime/field-overlays.js?poisson-latency-route-test=1'
        );
        const { createFieldSampleCache } = await import(
            '/js/scales/scale0/runtime/field-sample-cache.js?poisson-latency-route-test=1'
        );
        const requested = [];
        const capability = {
            getScale0FieldSamples({ kind, stride }) {
                requested.push({ kind, stride });
                return { positions: new Float32Array(0), values: new Float32Array(0), count: 0 };
            },
        };
        const nativeCtx = { bridge: { isNativeGPU: true } };
        const massiveState = {
            currentScenarioId: 's0-seed-massive-body',
            useFluxMock: false,
            fluxMock: null,
        };
        const massiveOverride = scale0FieldKindOverrides(nativeCtx, massiveState);
        const massiveCache = createFieldSampleCache(capability, null, 3, massiveOverride);
        massiveCache.ensureSample('latency');

        const proxyOverride = scale0FieldKindOverrides(
            { bridge: { isNativeGPU: false } },
            massiveState,
        );
        const otherOverride = scale0FieldKindOverrides(nativeCtx, {
            ...massiveState,
            currentScenarioId: 's0-seed-schwarzschild',
        });
        const proxyCache = createFieldSampleCache(capability, null, 3, proxyOverride);
        proxyCache.ensureSample('latency');
        return {
            requested,
            massiveKind: massiveOverride?.latency,
            proxyOverride,
            otherOverride,
        };
    });

    expect(result.requested).toEqual([
        { kind: 'poissonLatency', stride: 3 },
        { kind: 'latency', stride: 3 },
    ]);
    expect(result.massiveKind).toBe('poissonLatency');
    expect(result.proxyOverride).toBeNull();
    expect(result.otherOverride).toBeNull();
});

test('native FTS1 demand is fair and bounded to two multi-megabyte frames', async ({ page }) => {
    await page.goto('/js/ws-bridge.js', { waitUntil: 'domcontentloaded' });

    const result = await page.evaluate(async () => {
        const { WebSocketBridge } = await import('/js/ws-bridge.js?field-backpressure-test=1');
        const bridge = new WebSocketBridge('ws://field-backpressure-test');
        const sent = [];
        bridge._connected = true;
        bridge.ready = true;
        bridge._ws = { send(payload) { sent.push(JSON.parse(payload)); } };

        bridge.getEFieldSampled(2);
        bridge.getBFieldSampled(2);
        bridge.getPoyntingSampled(2);
        bridge.getDivJSampled(2);
        const before = sent.filter(m => m.cmd === 'get_field_sample');

        const emptyFrame = (request) => {
            const kindCodes = { e: 0, b: 1, poynting: 2, divJ: 3 };
            const frame = new ArrayBuffer(20);
            const header = new DataView(frame);
            header.setUint32(0, 0x31535446, true);
            header.setUint32(4, request.token, true);
            header.setUint32(8, kindCodes[request.kind], true);
            header.setUint32(12, request.kind === 'divJ' ? 1 : 3, true);
            header.setUint32(16, 0, true);
            return frame;
        };

        bridge._handleBinary(emptyFrame(before[0]));
        const afterOne = sent.filter(m => m.cmd === 'get_field_sample');
        bridge._handleBinary(emptyFrame(before[1]));
        const afterTwo = sent.filter(m => m.cmd === 'get_field_sample');

        return {
            before: before.map(m => m.kind),
            afterOne: afterOne.map(m => m.kind),
            afterTwo: afterTwo.map(m => m.kind),
            inFlight: bridge._fieldSampleRequestsByToken.size,
            queued: bridge._fieldSampleDemandByKey.size,
        };
    });

    expect(result.before).toEqual(['e', 'b']);
    expect(result.afterOne).toEqual(['e', 'b', 'poynting']);
    expect(result.afterTwo).toEqual(['e', 'b', 'poynting', 'divJ']);
    expect(result.inFlight).toBe(2);
    expect(result.queued).toBe(0);
});

test('native telemetry getters are cache-only and merge unsolicited group deltas by provenance', async ({ page }) => {
    await page.goto('/js/ws-bridge.js', { waitUntil: 'domcontentloaded' });

    const result = await page.evaluate(async () => {
        const { WebSocketBridge } = await import('/js/ws-bridge.js?telemetry-push-store-test=1');
        const bridge = new WebSocketBridge('ws://telemetry-push-store-test');
        const commands = [];
        bridge._connected = true;
        bridge.ready = true;
        bridge._ws = { send() {} };
        bridge._sendJSON = async message => {
            commands.push(message.cmd);
            if (message.cmd === 'set_telemetry_demand') {
                return { type: 'telemetry_demand', epoch: 7, snapshotVersion: 1, tick: 12 };
            }
            if (message.cmd === 'get_telemetry') return {
                type: 'telemetry',
                epoch: 7,
                snapshotVersion: 1,
                tick: 12,
                groups: {},
            };
            return { ok: true };
        };

        const beforeDiagnostics = bridge.getDiagnostics();
        const beforeGravity = bridge.getGravityMetricAgg();
        const beforeLagrangian = bridge.getLagrangian();
        const commandsAfterGetters = commands.length;
        bridge.setTelemetryDemand({
            diagnostics: true,
            audit: true,
            gravity: false,
            lagrangian: false,
            everyTicks: { diagnostics: 1, audit: 8, gravity: 4, lagrangian: 12 },
        });
        await new Promise(resolve => setTimeout(resolve, 0));

        bridge._handleJSON(JSON.stringify({
            type: 'telemetry_snapshot', epoch: 7, snapshotVersion: 2, tick: 12,
            groups: { diagnostics: { tick: 12, totalEnergy: 42 } },
            groupMeta: {
                diagnostics: {
                    epoch: 7, stateVersion: 7, tick: 12, snapshotVersion: 2, stale: false,
                },
            },
        }));
        bridge._handleJSON(JSON.stringify({
            type: 'telemetry_snapshot', epoch: 8, snapshotVersion: 3, tick: 13,
            groups: { audit: { tick: 13, totalEnergy: 19, dynamicEnergy: 18 } },
            groupMeta: {
                audit: {
                    epoch: 8, stateVersion: 8, tick: 13, snapshotVersion: 3, stale: false,
                },
            },
        }));
        // A later aggregate publication containing an older diagnostics
        // source version cannot overwrite the current cached group.
        bridge._handleJSON(JSON.stringify({
            type: 'telemetry_snapshot', epoch: 7, snapshotVersion: 99, tick: 14,
            groups: { diagnostics: { tick: 14, totalEnergy: 1 } },
            groupMeta: {
                diagnostics: {
                    epoch: 7, stateVersion: 6, tick: 14, snapshotVersion: 99, stale: true,
                },
            },
        }));

        const beforeCachedReads = commands.length;
        const snapshot = bridge.getTelemetrySnapshot();
        const cachedDiagnostics = bridge.getDiagnostics();
        const cachedAudit = bridge.getEnergyAudit();
        const cachedGravity = bridge.getGravityMetricAgg();
        const cachedLagrangian = bridge.getLagrangian();
        const commandsAfterCachedReads = commands.length;
        clearTimeout(bridge._telemetryDemandExpiryTimer);

        return {
            commands,
            beforeDiagnostics,
            beforeGravity,
            beforeLagrangian,
            commandsAfterGetters,
            beforeCachedReads,
            commandsAfterCachedReads,
            snapshot,
            cachedDiagnostics,
            cachedAudit,
            cachedGravity,
            cachedLagrangian,
        };
    });

    expect(result.beforeDiagnostics).toBeNull();
    expect(result.beforeGravity).toBeNull();
    expect(result.beforeLagrangian).toBeNull();
    expect(result.commandsAfterGetters).toBe(0);
    expect(result.commands).toContain('set_telemetry_demand');
    expect(result.commands.filter(cmd => cmd === 'get_telemetry')).toHaveLength(1);
    expect(result.commandsAfterCachedReads).toBe(result.beforeCachedReads);
    expect(result.cachedDiagnostics).toMatchObject({ tick: 12, totalEnergy: 18 });
    expect(result.cachedAudit).toMatchObject({ tick: 13, totalEnergy: 19, dynamicEnergy: 18 });
    expect(result.cachedGravity).toBeNull();
    expect(result.cachedLagrangian).toBeNull();
    expect(result.snapshot.diagnostics).toMatchObject({ tick: 12, totalEnergy: 18 });
    expect(result.snapshot.audit).toMatchObject({ tick: 13, dynamicEnergy: 18 });
    expect(result.snapshot.groupMeta.diagnostics).toMatchObject({
        epoch: 7, stateVersion: 7, tick: 12, stale: false,
    });
    expect(result.snapshot.groupMeta.audit).toMatchObject({
        epoch: 8, stateVersion: 8, tick: 13, stale: false,
    });
});

test('native scalar physics controls reach set_param and read from server-backed cache', async ({ page }) => {
    await page.goto('/js/ws-bridge.js', { waitUntil: 'domcontentloaded' });

    const result = await page.evaluate(async () => {
        const { WebSocketBridge } = await import('/js/ws-bridge.js?scalar-control-test=1');
        const bridge = new WebSocketBridge('ws://scalar-control-test');
        const sent = [];
        bridge._connected = true;
        bridge.ready = true;
        bridge._ws = { send(payload) { sent.push(JSON.parse(payload)); } };

        bridge.setDt(0.25);
        bridge.setOmega0(0.3);
        bridge.setLangevinParams(0.05, 0.02);
        bridge._lastDiag = { tick: 7, dt: 0.25, physicalTime: 1.75 };

        return {
            sent,
            dt: bridge.getDt(),
            physicalTime: bridge.getPhysicalTime(),
            omega0: bridge.getOmega0(),
            temperature: bridge.getLangevinTemp(),
            gamma: bridge.getLangevinGamma(),
            unknownToggle: bridge.getToggle('not_a_real_term'),
        };
    });

    expect(result.sent.map(({ _requestId, ...message }) => message)).toEqual([
        { cmd: 'set_param', name: 'dt', value: 0.25 },
        { cmd: 'set_param', name: 'omega0', value: 0.3 },
        { cmd: 'set_param', name: 'langevin_T', value: 0.05 },
        { cmd: 'set_param', name: 'langevin_gamma', value: 0.02 },
    ]);
    const requestIds = result.sent.map(message => message._requestId);
    expect(requestIds.every(Number.isFinite)).toBe(true);
    expect(new Set(requestIds).size).toBe(requestIds.length);
    expect(result).toMatchObject({
        dt: 0.25,
        physicalTime: 1.75,
        omega0: 0.3,
        temperature: 0.05,
        gamma: 0.02,
        unknownToggle: false,
    });
});

test('live native profile edits are acknowledged, coalesced, and rejected edits roll back', async ({ page }) => {
    await page.goto('/js/ws-bridge.js', { waitUntil: 'domcontentloaded' });

    const result = await page.evaluate(async () => {
        const { WebSocketBridge } = await import('/js/ws-bridge.js?live-profile-test=1');
        const bridge = new WebSocketBridge('ws://live-profile-test');
        const commands = [];
        const callbacks = [];
        bridge._connected = true;
        bridge.ready = true;
        bridge._activeScenario = 'flux-pulse';
        bridge._toggles = { damping: true, selective_damping: true, wave_propagation: true };
        bridge._confirmedToggles = { ...bridge._toggles };
        bridge._fluxBoundaryMode = 2;
        bridge._confirmedFluxBoundaryMode = 2;
        window.__ftdCtx = {
            bridge,
            onBridgeProfileUpdate(profile) { callbacks.push(profile); },
        };
        bridge._sendJSON = async command => {
            commands.push(command);
            if (commands.length === 1) {
                return { error: 'selective_damping requires damping' };
            }
            return {
                ok: true,
                scenario: 'flux-pulse',
                fluxBoundaryMode: 0,
                toggles: {
                    damping: false,
                    selective_damping: false,
                    wave_propagation: true,
                },
            };
        };

        bridge.setToggle('damping', false);
        const optimistic = bridge.getToggle('damping');
        await new Promise(resolve => setTimeout(resolve, 10));
        const rolledBack = bridge.getToggle('damping');

        // Same-turn edits are one candidate, so prerequisites/dependents and
        // boundary policy are validated together without transient warnings.
        bridge.setToggle('selective_damping', false);
        bridge.setToggle('damping', false);
        bridge.setFluxBoundaryMode(0);
        await new Promise(resolve => setTimeout(resolve, 10));

        delete window.__ftdCtx;
        return {
            commands,
            callbacks,
            optimistic,
            rolledBack,
            damping: bridge.getToggle('damping'),
            selective: bridge.getToggle('selective_damping'),
            boundary: bridge._fluxBoundaryMode,
        };
    });

    expect(result.optimistic).toBe(false);
    expect(result.rolledBack).toBe(true);
    expect(result.commands).toHaveLength(2);
    expect(result.commands[0]).toMatchObject({
        cmd: 'apply_profile',
        toggle_damping: false,
    });
    expect(result.commands[0]).not.toHaveProperty('toggle_selective_damping');
    expect(result.commands[1]).toMatchObject({
        cmd: 'apply_profile',
        toggle_damping: false,
        toggle_selective_damping: false,
        fluxBoundaryMode: 0,
    });
    expect(result).toMatchObject({ damping: false, selective: false, boundary: 0 });
    expect(result.callbacks.at(-1)).toMatchObject({
        fluxBoundaryMode: 0,
        toggles: { damping: false, selective_damping: false },
    });
});

test('telemetry demand falls back to the v1 batched cache without reviving scalar panel RPCs', async ({ page }) => {
    await page.goto('/js/ws-bridge.js', { waitUntil: 'domcontentloaded' });

    const result = await page.evaluate(async () => {
        const { WebSocketBridge } = await import('/js/ws-bridge.js?telemetry-v1-fallback-test=1');
        const bridge = new WebSocketBridge('ws://telemetry-v1-fallback-test');
        const commands = [];
        bridge._connected = true;
        bridge.ready = true;
        bridge._ws = { send() {} };
        bridge._sendJSON = async message => {
            commands.push(message.cmd);
            if (message.cmd === 'set_telemetry_demand') {
                return { error: 'unknown command: set_telemetry_demand' };
            }
            if (message.cmd === 'get_telemetry') {
                return {
                type: 'telemetry',
                    diagnostics: { tick: 4, totalEnergy: 12 },
                    audit: { tick: 4, totalEnergy: 13, dynamicEnergy: 11 },
                };
            }
            return { error: `unexpected ${message.cmd}` };
        };
        bridge.setTelemetryDemand({
            diagnostics: true,
            audit: true,
            gravity: false,
            lagrangian: false,
            everyTicks: { diagnostics: 1, audit: 8, gravity: 4, lagrangian: 12 },
        });
        await new Promise(resolve => setTimeout(resolve, 0));
        const beforeGetters = commands.length;
        const diagnostics = bridge.getDiagnostics();
        const audit = bridge.getEnergyAudit();
        bridge.getDiagnostics();
        bridge.getEnergyAudit();
        const afterGetters = commands.length;
        clearTimeout(bridge._telemetryDemandExpiryTimer);

        return {
            commands,
            mode: bridge._telemetryMode,
            beforeGetters,
            afterGetters,
            diagnostics,
            audit,
        };
    });

    expect(result.commands).toEqual(['set_telemetry_demand', 'get_telemetry']);
    expect(result.mode).toBe('legacy-batch');
    expect(result.afterGetters).toBe(result.beforeGetters);
    expect(result.diagnostics).toMatchObject({ tick: 4, totalEnergy: 11 });
    expect(result.audit).toMatchObject({ tick: 4, totalEnergy: 13, dynamicEnergy: 11 });
});

test('latest telemetry demand is re-dispatched when a panel changes visibility during an ACK', async ({ page }) => {
    await page.goto('/js/ws-bridge.js', { waitUntil: 'domcontentloaded' });

    const result = await page.evaluate(async () => {
        const { WebSocketBridge } = await import('/js/ws-bridge.js?telemetry-latest-demand-test=1');
        const bridge = new WebSocketBridge('ws://telemetry-latest-demand-test');
        const requests = [];
        const demandResolvers = [];
        bridge._connected = true;
        bridge.ready = true;
        bridge._ws = { send() {} };
        bridge._sendJSON = command => {
            requests.push(command);
            if (command.cmd === 'set_telemetry_demand') {
                return new Promise(resolve => demandResolvers.push(resolve));
            }
            return Promise.resolve({ type: 'telemetry', groups: {} });
        };
        bridge.setTelemetryDemand({
            diagnostics: true,
            audit: false,
            gravity: false,
            lagrangian: false,
            everyTicks: { diagnostics: 1, audit: 8, gravity: 4, lagrangian: 12 },
        });
        await Promise.resolve();
        bridge.setTelemetryDemand({ audit: true });
        demandResolvers.shift()?.({ type: 'telemetry_demand', epoch: 1, snapshotVersion: 1, tick: 1 });
        await new Promise(resolve => setTimeout(resolve, 0));
        // The hydration cache read follows the first ACK, then the newer
        // complete demand is dispatched after the first in-flight guard drops.
        demandResolvers.shift()?.({ type: 'telemetry_demand', epoch: 1, snapshotVersion: 2, tick: 1 });
        await new Promise(resolve => setTimeout(resolve, 0));
        clearTimeout(bridge._telemetryDemandExpiryTimer);
        return {
            requests: requests.map(request => ({
                cmd: request.cmd,
                diagnostics: request.diagnostics,
                audit: request.audit,
            })),
            applied: bridge._telemetryAppliedDemand,
        };
    });

    const demandRequests = result.requests.filter(request => request.cmd === 'set_telemetry_demand');
    expect(demandRequests).toEqual([
        { cmd: 'set_telemetry_demand', diagnostics: true, audit: false },
        { cmd: 'set_telemetry_demand', diagnostics: true, audit: true },
    ]);
    expect(result.applied).toMatchObject({ diagnostics: true, audit: true });
});

test('telemetry control and push decoders reject stale socket/source generations', async ({ page }) => {
    await page.goto('/js/ws-bridge.js', { waitUntil: 'domcontentloaded' });

    const result = await page.evaluate(async () => {
        const { WebSocketBridge } = await import('/js/ws-bridge.js?telemetry-stale-socket-test=1');
        const bridge = new WebSocketBridge('ws://telemetry-stale-socket-test');
        const oldSocket = { send() {} };
        const currentSocket = { send() {} };
        let resolveDemand;
        bridge._connected = true;
        bridge.ready = true;
        bridge._ws = oldSocket;
        bridge._sendJSON = () => new Promise(resolve => { resolveDemand = resolve; });
        bridge.setTelemetryDemand({ diagnostics: true, everyTicks: { diagnostics: 1 } });
        await Promise.resolve();
        // Simulate a source replacement while the old subscription ACK is
        // still pending. Its completion must not schedule a stale cache pull.
        bridge._resetTelemetryRequests();
        bridge._ws = currentSocket;
        resolveDemand?.({ type: 'telemetry_demand', epoch: 1, snapshotVersion: 1, tick: 1 });
        await new Promise(resolve => setTimeout(resolve, 0));
        const modeAfterOldAck = bridge._telemetryMode;

        bridge._handleJSON(JSON.stringify({
            type: 'telemetry_snapshot', epoch: 9, snapshotVersion: 9, tick: 9,
            groups: { diagnostics: { tick: 9, totalEnergy: 99 } },
            groupMeta: {
                diagnostics: {
                    epoch: 9, stateVersion: 9, tick: 9, snapshotVersion: 9, stale: false,
                },
            },
        }), oldSocket);
        const afterOldPush = bridge.getTelemetrySnapshot();

        bridge._handleJSON(JSON.stringify({
            type: 'telemetry_snapshot', epoch: 10, snapshotVersion: 10, tick: 10,
            groups: { diagnostics: { tick: 10, totalEnergy: 10 } },
            groupMeta: {
                diagnostics: {
                    epoch: 10, stateVersion: 10, tick: 10, snapshotVersion: 10, stale: false,
                },
            },
        }), currentSocket);
        const afterCurrentPush = bridge.getTelemetrySnapshot();
        clearTimeout(bridge._telemetryDemandExpiryTimer);

        return {
            modeAfterOldAck,
            pending: bridge._pendingQueue.length,
            afterOldPush,
            afterCurrentPush,
        };
    });

    expect(result.modeAfterOldAck).toBe('unknown');
    expect(result.pending).toBe(0);
    expect(result.afterOldPush.diagnostics).toBeUndefined();
    expect(result.afterCurrentPush.diagnostics).toMatchObject({ tick: 10, totalEnergy: 10 });
    expect(result.afterCurrentPush.groupMeta.diagnostics).toMatchObject({
        epoch: 10, stateVersion: 10, tick: 10, stale: false,
    });
});

test('poisoned native telemetry source requires a desktop restart instead of reconnecting', async ({ page }) => {
    await page.goto('/js/ws-bridge.js', { waitUntil: 'domcontentloaded' });

    const result = await page.evaluate(async () => {
        const { WebSocketBridge } = await import('/js/ws-bridge.js?telemetry-restart-required-test=1');
        const bridge = new WebSocketBridge('ws://telemetry-restart-required-test');
        let closed = false;
        let reconnects = 0;
        let hostEvent = null;
        bridge._connected = true;
        bridge.ready = true;
        bridge._ws = { send() {}, close() { closed = true; } };
        bridge._scheduleReconnect = () => { reconnects++; };
        const listener = event => { hostEvent = event.detail; };
        window.addEventListener('ftd:engine-error', listener);
        try {
            bridge._handleJSON(JSON.stringify({
                type: 'native_recovery_required', operation: 'telemetry',
                error: 'native telemetry snapshot timed out', restartRequired: true,
            }));
        } finally {
            window.removeEventListener('ftd:engine-error', listener);
            clearTimeout(bridge._telemetryDemandExpiryTimer);
        }
        return {
            restartRequired: bridge._restartRequired,
            connected: bridge._connected,
            ready: bridge.ready,
            closed,
            reconnects,
            hostEvent,
        };
    });

    expect(result).toMatchObject({
        restartRequired: true,
        connected: false,
        ready: false,
        closed: true,
        reconnects: 0,
        hostEvent: { restartRequired: true, operation: 'telemetry' },
    });
});

test('native destructive operations retry only after a telemetry-settling deferral', async ({ page }) => {
    await page.goto('/js/ws-bridge.js', { waitUntil: 'domcontentloaded' });

    const result = await page.evaluate(async () => {
        const { WebSocketBridge } = await import('/js/ws-bridge.js?telemetry-settling-retry-test=1');
        const bridge = new WebSocketBridge('ws://telemetry-settling-retry-test');
        const commands = [];
        let resizeCalls = 0;
        bridge._connected = true;
        bridge.ready = true;
        bridge._ws = { send() {} };
        bridge._sendJSON = async command => {
            commands.push(command.cmd);
            if (command.cmd === 'preflight_resize') {
                return { accepted: true, size: command.size };
            }
            if (command.cmd === 'resize' && resizeCalls++ === 0) {
                return {
                    type: 'operation_deferred', operation: 'resize',
                    reason: 'telemetry_settling', retryAfterMs: 1,
                };
            }
            if (command.cmd === 'resize') {
                return {
                    ok: true, latticeSize: command.size,
                    telemetrySourceEpoch: 2,
                };
            }
            return { error: `unexpected ${command.cmd}` };
        };
        const response = await bridge.resize(64);
        clearTimeout(bridge._telemetryDemandExpiryTimer);
        return {
            commands,
            latticeSize: bridge.latticeSize,
            sourceEpoch: bridge._expectedTelemetrySourceEpoch,
            response,
        };
    });

    expect(result.commands).toEqual(['preflight_resize', 'resize', 'resize']);
    expect(result).toMatchObject({ latticeSize: 64, sourceEpoch: 2, response: { latticeSize: 64 } });
});

test('unsolicited telemetry is gated until reconnect info and scenario replay are authoritative', async ({ page }) => {
    await page.goto('/js/ws-bridge.js', { waitUntil: 'domcontentloaded' });

    const result = await page.evaluate(async () => {
        const { WebSocketBridge } = await import('/js/ws-bridge.js?telemetry-reconnect-gate-test=1');
        const bridge = new WebSocketBridge('ws://telemetry-reconnect-gate-test');
        bridge._connected = true;
        bridge.ready = false;
        bridge._connectionRecoveryPending = true;
        bridge._ws = { send() {} };
        const push = (sourceEpoch, epoch, energy) => bridge._handleJSON(JSON.stringify({
            type: 'telemetry_snapshot', sourceEpoch, epoch, snapshotVersion: epoch, tick: epoch,
            groups: { diagnostics: { tick: epoch, totalEnergy: energy } },
            groupMeta: {
                diagnostics: {
                    epoch, stateVersion: epoch, tick: epoch, snapshotVersion: epoch, stale: false,
                },
            },
        }));

        push(1, 1, 101); // old server source, before info
        const beforeInfo = bridge.getTelemetrySnapshot();
        bridge.ready = true;
        bridge._connectionRecoveryPending = false;
        // This is the source token that a real `info` acknowledgement carries.
        // A later frame from the former source must not become valid merely
        // because the profile transaction has finished.
        bridge._observeTelemetrySourceEpoch(3);
        bridge._scenarioDraft = { name: 's0-seed-wilson-loop' };
        push(2, 2, 202); // old source, before scenario ACK/replay
        const duringSetup = bridge.getTelemetrySnapshot();
        bridge._scenarioDraft = null;
        push(1, 2, 202); // former source, after setup but below authoritative source epoch
        const afterOldEpoch = bridge.getTelemetrySnapshot();
        push(3, 3, 303); // authoritative source
        const afterReplay = bridge.getTelemetrySnapshot();
        bridge._handleJSON(JSON.stringify({
            type: 'telemetry_invalidated', sourceEpoch: 3, epoch: 4, snapshotVersion: 4, tick: 4,
            reason: 'state_mutated', groups: {},
        }));
        const afterMutation = bridge.getTelemetrySnapshot();
        const directAfterMutation = bridge.getDiagnostics();
        clearTimeout(bridge._telemetryDemandExpiryTimer);
        return {
            beforeInfo, duringSetup, afterOldEpoch, afterReplay,
            afterMutation, directAfterMutation,
        };
    });

    expect(result.beforeInfo.diagnostics).toBeUndefined();
    expect(result.duringSetup.diagnostics).toBeUndefined();
    expect(result.afterOldEpoch.diagnostics).toBeUndefined();
    expect(result.afterReplay.diagnostics).toMatchObject({ tick: 3, totalEnergy: 303 });
    expect(result.afterMutation).toMatchObject({ epoch: 4, stale: true });
    expect(result.directAfterMutation).toBeNull();
});

test('TelemetryHub keeps staggered Scale-0 group provenance and ignores stale aggregate values', async ({ page }) => {
    await page.goto('/js/telemetry-hub.js', { waitUntil: 'domcontentloaded' });

    const result = await page.evaluate(async () => {
        const { TelemetryHub } = await import('/js/telemetry-hub.js?telemetry-hub-provenance-test=1');
        const hub = new TelemetryHub();
        hub.ingestScale0Snapshot({
            type: 'telemetry_snapshot', sourceEpoch: 1, epoch: 20, snapshotVersion: 10, tick: 100,
            groups: { diagnostics: { tick: 100, totalEnergy: 40 } },
            groupMeta: {
                diagnostics: {
                    epoch: 20, stateVersion: 20, tick: 100, snapshotVersion: 10, stale: false,
                    receivedAt: 1000,
                },
            },
        });
        hub.ingestScale0Snapshot({
            type: 'telemetry_snapshot', sourceEpoch: 1, epoch: 24, snapshotVersion: 11, tick: 104,
            groups: { audit: { tick: 104, totalEnergy: 44, dynamicEnergy: 43 } },
            groupMeta: {
                audit: {
                    epoch: 24, stateVersion: 24, tick: 104, snapshotVersion: 11, stale: false,
                    receivedAt: 1100,
                },
            },
        });
        hub.ingestScale0Snapshot({
            type: 'telemetry', sourceEpoch: 1, epoch: 20, snapshotVersion: 99, tick: 101,
            groups: { diagnostics: { tick: 101, totalEnergy: 1 } },
            groupMeta: {
                diagnostics: {
                    epoch: 20, stateVersion: 19, tick: 101, snapshotVersion: 99, stale: true,
                    receivedAt: 1200,
                },
            },
        });

        const beforeInvalidation = {
            diagnostics: hub.getScale0TelemetryMeta('diagnostics'),
            audit: hub.getScale0TelemetryMeta('audit'),
        };
        // A mutation/source boundary can arrive before its first replacement
        // reduction. Retained group values must become visibly stale rather
        // than being relabelled with the new source epoch.
        hub.ingestScale0Snapshot({
            type: 'telemetry_invalidated', sourceEpoch: 1, epoch: 25, snapshotVersion: 12, tick: 105,
            groups: {},
        });

        return {
            diagnostics: hub.s0.diag,
            audit: hub.s0.audit,
            beforeInvalidation,
            diagMeta: hub.getScale0TelemetryMeta('diagnostics'),
            auditMeta: hub.getScale0TelemetryMeta('audit'),
        };
    });

    expect(result.diagnostics).toMatchObject({ tick: 100, totalEnergy: 40 });
    expect(result.audit).toMatchObject({ tick: 104, totalEnergy: 44, dynamicEnergy: 43 });
    expect(result.beforeInvalidation.diagnostics).toMatchObject({
        epoch: 20, stateVersion: 20, tick: 100, stale: false,
    });
    expect(result.beforeInvalidation.audit).toMatchObject({
        epoch: 24, stateVersion: 24, tick: 104, stale: false,
    });
    expect(result.diagMeta).toMatchObject({ epoch: 20, stateVersion: 20, tick: 100, stale: true });
    expect(result.auditMeta).toMatchObject({ epoch: 24, stateVersion: 24, tick: 104, stale: true });
});

test('native playback prioritizes its next tick and recovers typed visual deferrals at telemetry publish', async ({ page }) => {
    await page.goto('/js/ws-bridge.js', { waitUntil: 'domcontentloaded' });

    const result = await page.evaluate(async () => {
        const { WebSocketBridge } = await import('/js/ws-bridge.js?visual-deferred-priority-test=1');
        const bridge = new WebSocketBridge('ws://visual-deferred-priority-test');
        const events = [];
        const postFrames = [];
        const originalCtx = window.__ftdCtx;
        bridge._connected = true;
        bridge.ready = true;
        bridge._ws = { send(payload) { events.push(`send:${JSON.parse(payload).cmd}`); } };
        window.__ftdCtx = {
            bridge,
            onBridgeSimulationComplete() { events.push('complete'); },
            onBridgePostFrame(...args) { postFrames.push(args); events.push('post'); },
        };

        bridge._simulationInFlight = true;
        bridge._simulationTicksInFlight = 1;
        bridge._queuedSimulationTicks = 1;
        bridge._handleSimulationComplete({ type: 'tick_complete', tick: 1 });
        if (bridge._simulationWatchdog) clearTimeout(bridge._simulationWatchdog);

        bridge._visualEpoch = 4;
        bridge._particleRequestInFlight = true;
        bridge._particleRequestEpoch = 4;
        bridge._volumeRequestInFlight = true;
        bridge._volumeRequestEpoch = 4;
        bridge._sliceRequestsInFlight.add('0_2');
        bridge._sliceRequestEpoch.set('0_2', 4);
        bridge._fieldSampleRequestsByToken.set(77, {
            key: 'e@2', kind: 'e', stride: 2, epoch: 4, token: 77,
        });
        bridge._fieldSampleRequestTokenByKey.set('e@2', 77);
        bridge._fieldSampleRequestEpoch.set('e@2', 4);
        bridge._handleJSON(JSON.stringify({
            type: 'visual_deferred', operation: 'get_particles', reason: 'telemetry_priority', retryAfterMs:500,
        }));
        bridge._handleJSON(JSON.stringify({
            type: 'visual_deferred', operation: 'get_flux_volume', reason: 'telemetry_priority', retryAfterMs:500,
        }));
        bridge._handleJSON(JSON.stringify({
            type: 'visual_deferred', operation: 'get_flux_slice', reason: 'telemetry_priority', retryAfterMs:500,
        }));
        bridge._handleJSON(JSON.stringify({
            type: 'visual_deferred', operation: 'get_field_sample', reason: 'telemetry_priority', retryAfterMs:500,
        }));
        const afterDeferred = {
            particleInFlight: bridge._particleRequestInFlight,
            volumeInFlight: bridge._volumeRequestInFlight,
            slices: bridge._sliceRequestsInFlight.size,
            fieldInFlight: bridge._fieldSampleRequestsByToken.size,
            fieldQueued: bridge._fieldSampleDemandByKey.size,
            retryArmed: !!bridge._visualDeferredRetryTimer,
        };
        bridge._handleJSON(JSON.stringify({
            type: 'telemetry_snapshot', epoch: 3, snapshotVersion: 3, tick: 3,
            groups: { diagnostics: { tick: 3, totalEnergy: 3 } },
            groupMeta: {
                diagnostics: {
                    epoch: 3, stateVersion: 3, tick: 3, snapshotVersion: 3, stale: false,
                },
            },
        }));
        // A subsequent push without deferred visual work is a cached-only
        // presentation wake-up, not an engine error or fresh visual request.
        bridge._handleJSON(JSON.stringify({
            type: 'telemetry_snapshot', epoch: 4, snapshotVersion: 4, tick: 4,
            groups: { audit: { tick: 4, totalEnergy: 4, dynamicEnergy: 4 } },
            groupMeta: {
                audit: {
                    epoch: 4, stateVersion: 4, tick: 4, snapshotVersion: 4, stale: false,
                },
            },
        }));
        clearTimeout(bridge._telemetryDemandExpiryTimer);
        clearTimeout(bridge._visualDeferredRetryTimer);
        window.__ftdCtx = originalCtx;
        return { events, postFrames, afterDeferred };
    });

    expect(result.events.indexOf('send:tick')).toBeLessThan(result.events.indexOf('post'));
    expect(result.afterDeferred).toEqual({
        particleInFlight: false,
        volumeInFlight: false,
        slices: 0,
        fieldInFlight: 0,
        fieldQueued: 1,
        retryArmed: true,
    });
    expect(result.postFrames).toContainEqual([false, true]);
    expect(result.postFrames).toContainEqual([false, false]);
});
