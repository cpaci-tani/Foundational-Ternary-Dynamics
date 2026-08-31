// @ts-check
/**
 * Scale-0 scientific telemetry availability contract.
 *
 * Exact numeric zero is a measurement. Missing, null, non-finite, stale, and
 * ABI-absent channels are unavailable; they must never become a calm zero.
 */
import { test, expect } from '@playwright/test';
import { gotoAndReady, selectScale0Scenario } from './_helpers.js';

test.beforeEach(async ({ page }) => {
    test.setTimeout(120_000);
    await gotoAndReady(page, { path: '/?engine=wasm', timeout: 90_000 });
});

test('preserves exact zero, gaps missing/non-finite channels, and retains signed action terms', async ({ page }) => {
    const result = await page.evaluate(async () => {
        const { TelemetryHub } = await import('/js/telemetry-hub.js');
        const hub = new TelemetryHub();
        const meta = (stateVersion, tick) => ({
            epoch: 1, sourceEpoch: 1, stateVersion, snapshotVersion: stateVersion,
            tick, stale: false,
        });

        hub.ingestScale0Snapshot({
            sourceEpoch: 1,
            snapshotVersion: 1,
            groups: {
                diagnostics: {
                    tick: 10,
                    totalFlux: 0,
                    totalEnergy: null,
                    manifested: 0,
                    positive: 0,
                    // negative deliberately absent
                    entropy: Number.POSITIVE_INFINITY,
                    fieldSpinX: 0,
                    fieldSpinY: 0,
                    fieldSpinZ: 0,
                    fieldHelicity: Number.NEGATIVE_INFINITY,
                },
            },
            groupMeta: { diagnostics: meta(1, 10) },
        }, 'native');

        hub.ingestScale0Snapshot({
            sourceEpoch: 1,
            snapshotVersion: 2,
            groups: {
                audit: {
                    dynamicEnergy: 0,
                    fieldEnergy: 0,
                    waveEnergy: 0,
                    particleKE: 0,
                    EFieldEnergy: 0,
                    // BFieldEnergy and two Poynting lanes deliberately absent
                    totalPoynting: { x: 0 },
                    gaussViolation: 0,
                    maxGaussError: null,
                    waveLTotal: 3,
                    waveRTotal: 4,
                },
            },
            groupMeta: { audit: meta(1, 10) },
        }, 'native');

        hub.ingestScale0Snapshot({
            sourceEpoch: 1,
            snapshotVersion: 3,
            groups: {
                lagrangian: {
                    fieldKinetic: -1,
                    fieldGradient: -2,
                    bornInfeld: -3,
                    coupling: -4,
                    velocity: -5,
                    gauss: -6,
                    dissipation: -7,
                    total: -28,
                    hamiltonian: 28,
                    // totalAction deliberately absent
                },
            },
            groupMeta: { lagrangian: meta(1, 10) },
        }, 'native');

        return {
            diagnostics: {
                flux: hub.flux.last(),
                energy: hub.energy.last(),
                manifested: hub.manifested.last(),
                positive: hub.positive.last(),
                negativeNaN: Number.isNaN(hub.negative.last()),
                chargeNaN: Number.isNaN(hub.charges.last()),
                entropyNaN: Number.isNaN(hub.entropy.last()),
                fieldSpin: hub.fieldSpin.last(),
                fieldHelicityNaN: Number.isNaN(hub.fieldHelicity.last()),
            },
            audit: {
                driftNaN: Number.isNaN(hub.s0.audit.energyDrift),
                gauss: hub.gauss.last(),
                ebDiffNaN: Number.isNaN(hub.ebDiff.last()),
                poyntingNaN: Number.isNaN(hub.aud.poyntingMag.last()),
                maxGaussNaN: Number.isNaN(hub.aud.maxGaussError.last()),
                waveLeft: hub.aud.waveLeft.last(),
                waveRight: hub.aud.waveRight.last(),
            },
            lagrangian: {
                values: [
                    hub.lag.fieldKinetic.last(), hub.lag.fieldGradient.last(),
                    hub.lag.bornInfeld.last(), hub.lag.coupling.last(),
                    hub.lag.velocity.last(), hub.lag.gauss.last(),
                    hub.lag.dissipation.last(),
                ],
                actionNaN: Number.isNaN(hub.lag.action.last()),
            },
        };
    });

    expect(result.diagnostics).toEqual({
        flux: 0,
        energy: 0,
        manifested: 0,
        positive: 0,
        negativeNaN: true,
        chargeNaN: true,
        entropyNaN: true,
        fieldSpin: 0,
        fieldHelicityNaN: true,
    });
    expect(result.audit).toEqual({
        driftNaN: true,
        gauss: 0,
        ebDiffNaN: true,
        poyntingNaN: true,
        maxGaussNaN: true,
        waveLeft: 3,
        waveRight: 4,
    });
    expect(result.lagrangian).toEqual({
        values: [-1, -2, -3, -4, -5, -6, -7],
        actionNaN: true,
    });
});

test('suppresses stale values and stats, then displays a fresh measured zero', async ({ page }) => {
    const result = await page.evaluate(async () => {
        const [{ TelemetryHub }, { DiagnosticsTable }] = await Promise.all([
            import('/js/telemetry-hub.js'),
            import('/js/ui/panels/diagnostics-panel/table.js'),
        ]);
        const hub = new TelemetryHub();
        const publish = (sourceEpoch, stateVersion, tick, totalFlux) => {
            hub.ingestScale0Snapshot({
                sourceEpoch,
                snapshotVersion: stateVersion,
                groups: { diagnostics: { tick, totalFlux } },
                groupMeta: {
                    diagnostics: {
                        sourceEpoch, stateVersion, snapshotVersion: stateVersion,
                        tick, stale: false,
                    },
                },
            }, 'native');
        };
        publish(1, 1, 10, 5);

        const table = new DiagnosticsTable({
            id: 'stale-contract',
            title: 'Stale Contract',
            telemetryGroups: ['diagnostics'],
            rows: [{
                id: 'flux', label: 'Flux', source: 's0.diag.totalFlux', trend: 'flux',
            }],
        }, hub, { resetScope: 0 });
        document.body.appendChild(table.el);
        table.update();
        const liveText = table.cells.get('flux').textContent;
        const liveStats = table.stats.get('flux').count;
        const historyBefore = hub.flux.total;

        hub.ingestScale0Snapshot({
            type: 'telemetry_invalidated', sourceEpoch: 2, snapshotVersion: 2,
        }, 'native');
        table.update();
        const staleText = table.cells.get('flux').textContent;
        const staleStats = table.stats.get('flux').count;
        const staleClass = table.cells.get('flux').closest('tr')
            .classList.contains('diag-row-telemetry-stale');

        publish(2, 2, 11, 0);
        table.update();
        const zeroText = table.cells.get('flux').textContent;
        const zeroStats = table.stats.get('flux').count;
        const historyAfter = hub.flux.total;
        table.destroy();
        return {
            liveText, liveStats, historyBefore,
            staleText, staleStats, staleClass,
            zeroText, zeroStats, historyAfter,
        };
    });

    expect(result).toEqual({
        liveText: '5', liveStats: 1, historyBefore: 1,
        staleText: '—', staleStats: 1, staleClass: true,
        zeroText: '0', zeroStats: 2, historyAfter: 2,
    });
});

test('Diagnostics sparklines keep redrawing after a shared ring reaches capacity', async ({ page }) => {
    const result = await page.evaluate(async () => {
        const [{ MultiRingBuffer }, { DiagnosticsTable }] = await Promise.all([
            import('/js/telemetry-hub.js'),
            import('/js/ui/panels/diagnostics-panel/table.js'),
        ]);
        const ring = new MultiRingBuffer(500, ['value']);
        const hub = {
            sample: { value: 0 },
            trend: ring.views.value,
            getResetVersion: () => 0,
        };
        const table = new DiagnosticsTable({
            id: 'ring-rollover',
            title: 'Ring Rollover',
            rows: [{
                id: 'value', label: 'Value', source: 'sample.value', trend: 'trend',
            }],
        }, hub);
        document.body.appendChild(table.el);

        // Make this a deterministic lifecycle test; viewport intersection is
        // covered separately by the panel virtualization campaign.
        table.sparkObserver?.disconnect();
        const entry = table.sparkEntries[0];
        table.mountSpark(entry);
        let redraws = 0;
        const originalUpdate = entry.spark.update.bind(entry.spark);
        entry.spark.update = () => {
            redraws++;
            originalUpdate();
        };

        for (let value = 1; value <= 525; value++) {
            ring.push({ value });
            hub.sample.value = value;
            table.update();
        }
        const redrawsAfterRollover = redraws;
        const stampAfterRollover = entry.stamp;

        // Audit collection can refine the newest shared row without pushing a
        // second sample. It must repaint but must not count as another sample.
        ring.views.value.setLast(526);
        hub.sample.value = 526;
        table.update();
        const lastY = entry.spark.ys[Math.min(ring.count, entry.spark.visibleSamples) - 1];
        const statsCount = table.stats.get('value').count;
        const stampAfterPatch = entry.stamp;
        table.destroy();
        return {
            redrawsAfterRollover,
            redrawsAfterPatch: redraws,
            ringCount: ring.count,
            ringTotal: ring.total,
            statsCount,
            lastY,
            stampChangedOnPatch: stampAfterPatch !== stampAfterRollover,
        };
    });

    expect(result).toEqual({
        redrawsAfterRollover: 525,
        redrawsAfterPatch: 526,
        ringCount: 500,
        ringTotal: 525,
        statsCount: 525,
        lastY: 526,
        stampChangedOnPatch: true,
    });
});

test('deduplicates reused worker-group provenance and clears a reset Telemetry Grid trace', async ({ page }) => {
    const result = await page.evaluate(async () => {
        const [{ TelemetryHub, telemetryHub }, { TelemetryGridPanelComponent }] = await Promise.all([
            import('/js/telemetry-hub.js'),
            import('/js/ui/panels/telemetry-grid/component.js'),
        ]);

        const local = new TelemetryHub();
        let diagTick = 40;
        const audit = {
            dynamicEnergy: 2,
            fieldEnergy: 1,
            waveEnergy: 1,
            particleKE: 0,
            EFieldEnergy: 1,
            BFieldEnergy: 1,
            totalPoynting: { x: 0, y: 0, z: 0 },
            gaussViolation: 0,
        };
        const owner = {
            capabilities: {
                scale0: {
                    getScale0Diagnostics: () => ({ tick: diagTick, totalFlux: 1 }),
                    getScale0EnergyAudit: () => audit,
                },
            },
            getScale0TelemetryGroupMeta: (group) => group === 'audit' ? ({
                backend: 'wasm-worker', sourceEpoch: 7, stateVersion: 3,
                sampleTick: 40, status: 'available', stale: false,
            }) : ({
                backend: 'wasm-worker', sourceEpoch: 7, stateVersion: diagTick,
                sampleTick: diagTick, status: 'available', stale: false,
            }),
        };
        local.collectScale0(owner, null, false);
        local.collectScale0Audit(owner, null, false);
        diagTick = 41;
        local.collectScale0(owner, null, false);
        local.collectScale0Audit(owner, null, false);

        telemetryHub.resetScale(0);
        telemetryHub.ingestScale0Snapshot({
            sourceEpoch: 1,
            groups: { diagnostics: { tick: 1, totalFlux: 5 } },
            groupMeta: { diagnostics: { sourceEpoch: 1, stateVersion: 1, tick: 1 } },
        }, 'native');
        const host = document.createElement('section');
        host.id = 'telemetry-grid-contract-host';
        host.className = 'panel active';
        document.body.appendChild(host);
        const component = new TelemetryGridPanelComponent(host).init();
        const entry = component.charts.get('flux');
        if (!entry.built) component._buildChart(entry);
        entry.onScreen = true;
        component.update();
        const before = {
            value: entry.valueEl.textContent,
            samples: entry.lastN,
        };
        telemetryHub.resetScale(0);
        component.update();
        const after = {
            value: entry.valueEl.textContent,
            samples: entry.lastN,
            state: entry.card.dataset.telemetryState,
        };
        component.cleanup();
        host.remove();
        return {
            auditHistoryRows: local.aud.dynamicEnergy.total,
            auditMeta: local.getScale0TelemetryMeta('audit'),
            before,
            after,
        };
    });

    expect(result.auditHistoryRows).toBe(1);
    expect(result.auditMeta.tick).toBe(40);
    expect(result.before).toEqual({ value: '5.0000 J', samples: 1 });
    expect(result.after).toEqual({ value: '—', samples: 0, state: 'waiting' });
});

test('direct/native readiness, metadata gaps, ABI gaps, and formatters never fabricate zero', async ({ page }) => {
    const result = await page.evaluate(async () => {
        const [{ WasmBridge }, { WebSocketBridge }, { formatValue }] = await Promise.all([
            import('/js/bridge/wasm-bridge.js'),
            import('/js/ws-bridge.js'),
            import('/js/ui/panels/diagnostics-panel/formatters.js'),
        ]);

        const unready = Object.create(WasmBridge.prototype);
        unready._module = null;
        unready._bridge = null;

        const short = Object.create(WasmBridge.prototype);
        short._bridge = {};
        short._lastScale0Audit = null;
        short._lastScale0AuditTick = -1;
        short.currentTick = () => 0;
        short._module = {
            getDiagnosticsView: () => new Float64Array(23),
            getEnergyAuditView: () => new Float64Array(19),
        };
        const diag = short.getDiagnostics();
        const audit = short.getEnergyAudit();

        const native = Object.create(WebSocketBridge.prototype);
        native._telemetrySyntheticVersion = 100;
        const nativeMeta = native._normalizeTelemetryGroupMeta({
            epoch: null,
            groupMeta: {
                diagnostics: {
                    sourceEpoch: undefined,
                    stateVersion: '',
                    tick: Number.POSITIVE_INFINITY,
                    snapshotVersion: '',
                },
            },
        }, 'diagnostics', { tick: Number.NaN }, null);

        return {
            unready: {
                diagnostics: unready.getDiagnostics(),
                audit: unready.getEnergyAudit(),
                lagrangian: unready.getLagrangian(),
            },
            compact: {
                hasFieldSpin: Object.hasOwn(diag, 'fieldSpinX'),
                dynamicEnergyUndefined: audit.dynamicEnergy === undefined,
                restEnergyUndefined: audit.particleRestEnergy === undefined,
                momentumUndefined: audit.particleMomentum === undefined,
            },
            nativeMeta,
            formatted: {
                zero: formatValue(0),
                missing: formatValue(undefined),
                positiveInfinity: formatValue(Number.POSITIVE_INFINITY),
                negativeInfinity: formatValue(Number.NEGATIVE_INFINITY),
                nan: formatValue(Number.NaN),
            },
        };
    });

    expect(result.unready).toEqual({ diagnostics: null, audit: null, lagrangian: null });
    expect(result.compact).toEqual({
        hasFieldSpin: false,
        dynamicEnergyUndefined: true,
        restEnergyUndefined: true,
        momentumUndefined: true,
    });
    expect(result.nativeMeta).toMatchObject({
        epoch: null,
        sourceEpoch: null,
        stateVersion: null,
        tick: null,
        snapshotVersion: 101,
    });
    expect(result.formatted).toEqual({
        zero: '0', missing: '—', positiveInfinity: '—', negativeInfinity: '—', nan: '—',
    });
});

test('worker proxy anchors repeated audit receipt age and fails closed on an unavailable sample', async ({ page }) => {
    const result = await page.evaluate(async () => {
        const { WasmBridgeProxy } = await import('/js/bridge/wasm-bridge-proxy.js');
        const proxy = Object.create(WasmBridgeProxy.prototype);
        proxy._lastAudit = null;
        proxy._lastAuditMeta = null;

        const firstAudit = { dynamicEnergy: 12.5, totalEnergy: 13.0 };
        const firstMeta = {
            backend: 'wasm-worker',
            sourceEpoch: 7,
            stateVersion: 11,
            sampleTick: 40,
            tick: 40,
            sampledAt: 25,
            stale: false,
            status: 'available',
        };
        proxy._acceptTelemetryGroupFrame('audit', firstAudit, firstMeta, 100);
        const first = proxy.getScale0TelemetryGroupMeta('audit');

        // A later transport frame republishes the same observation. Deliberately
        // vary sampledAt to prove the proxy anchors both clocks to the first
        // receipt rather than allowing transport cadence to make old data young.
        proxy._acceptTelemetryGroupFrame(
            'audit',
            { ...firstAudit },
            { ...firstMeta, sampleTick: 999, tick: 999, sampledAt: 999 },
            460,
        );
        const repeated = proxy.getScale0TelemetryGroupMeta('audit');
        const receiptAgeAtSecondFrame = 460 - repeated.receivedAt;

        // Even if a faulty/older worker accompanies failure metadata with the
        // retained payload, the proxy must not expose that payload as current.
        proxy._acceptTelemetryGroupFrame('audit', firstAudit, {
            ...firstMeta,
            stateVersion: 12,
            sampleTick: 48,
            tick: 48,
            sampledAt: 500,
            stale: true,
            status: 'nonfinite',
        }, 700);
        const failed = proxy.getScale0TelemetryGroupMeta('audit');

        return {
            first,
            repeated,
            receiptAgeAtSecondFrame,
            failed,
            failedValue: proxy.getEnergyAudit(),
        };
    });

    expect(result.first).toMatchObject({
        stateVersion: 11, sampleTick: 40, sampledAt: 25, receivedAt: 100,
    });
    expect(result.repeated).toMatchObject({
        stateVersion: 11, sampleTick: 40, sampledAt: 25, receivedAt: 100,
    });
    expect(result.receiptAgeAtSecondFrame).toBe(360);
    expect(result.failed).toMatchObject({
        stateVersion: 12, sampleTick: 48, sampledAt: 500,
        receivedAt: 700, stale: true, status: 'nonfinite',
    });
    expect(result.failedValue).toBeNull();
});

test('real L=97 worker cadence keeps the audit sample tick/version stable while diagnostics advance', async ({ page }) => {
    test.setTimeout(180_000);
    await selectScale0Scenario(page, 'empty', { settleMs: 0 });
    const supported = await page.evaluate(() => globalThis.crossOriginIsolated === true
        && typeof SharedArrayBuffer !== 'undefined'
        && !![...document.querySelectorAll('#lattice-size option')]
            .find(option => option.value === '97' && !option.disabled));
    test.skip(!supported, 'L=97 WasmBridgeProxy path unavailable');
    await page.selectOption('#lattice-size', '97');

    await expect.poll(async () => page.evaluate(async () => {
        const { getScale0State } = await import('/js/scales/scale0/state/store.js');
        const state = getScale0State();
        return state.currentScenarioId === 'empty'
            && state.fluxMock?.isWorker === true
            && state.fluxMock?.ready === true
            && state.fluxMock?.latticeSize === 97;
    }), { timeout: 90_000 }).toBe(true);

    // Opening Diagnostics changes worker telemetry demand asynchronously. The
    // worker deliberately publishes one fail-closed `inactive` observation
    // until that demand reaches it, so establish the first available audit as
    // the cadence-test barrier instead of racing that transition.
    await page.evaluate(() => {
        document.querySelector('#tab-bar .tab[data-panel="diagnostics"]')?.click();
        const play = document.getElementById('btn-play');
        if (play?.getAttribute('data-paused') === 'true') play.click();
    });
    await expect.poll(async () => page.evaluate(async () => {
        const { getScale0State } = await import('/js/scales/scale0/state/store.js');
        const meta = getScale0State().fluxMock?.getScale0TelemetryGroupMeta?.('audit');
        return meta?.status === 'available'
            && meta.stale === false
            && Number.isFinite(meta.receivedAt);
    }), { timeout: 30_000 }).toBe(true);

    const result = await page.evaluate(async () => {
        const { getScale0State } = await import('/js/scales/scale0/state/store.js');
        const owner = getScale0State().fluxMock;

        const rows = [];
        let lastFrame = -1;
        const deadline = performance.now() + 20_000;
        while (performance.now() < deadline) {
            await new Promise(resolve => requestAnimationFrame(resolve));
            const frame = owner.frameCounter;
            if (frame === lastFrame) continue;
            lastFrame = frame;
            const diag = owner.getDiagnostics();
            const diagnosticsMeta = owner.getScale0TelemetryGroupMeta('diagnostics');
            const auditMeta = owner.getScale0TelemetryGroupMeta('audit');
            if (diag && diagnosticsMeta && auditMeta) {
                rows.push({
                    frame,
                    observedAt: performance.now(),
                    diagTick: diag.tick,
                    diagHasDynamicEnergy: Object.hasOwn(diag, 'dynamicEnergy'),
                    energySampleSource: diag.energySampleSource,
                    diagnosticsVersion: diagnosticsMeta.stateVersion,
                    diagnosticsTick: diagnosticsMeta.tick,
                    auditVersion: auditMeta.stateVersion,
                    auditTick: auditMeta.tick,
                    auditSampleTick: auditMeta.sampleTick,
                    auditSampledAt: auditMeta.sampledAt,
                    auditReceivedAt: auditMeta.receivedAt,
                    auditStale: auditMeta.stale,
                });
            }
            const byAudit = new Map();
            for (const row of rows) {
                const group = byAudit.get(row.auditVersion) || [];
                group.push(row);
                byAudit.set(row.auditVersion, group);
            }
            if ([...byAudit.values()].some(group => new Set(group.map(row => row.diagTick)).size >= 2)) break;
        }
        return rows;
    });

    expect(result.length).toBeGreaterThan(1);
    expect(result.every(row => row.diagnosticsTick === row.diagTick)).toBe(true);
    expect(result.every(row => row.auditTick === row.auditSampleTick)).toBe(true);
    expect(result.every(row => row.auditStale === false)).toBe(true);
    expect(result.every(row => Number.isFinite(row.auditReceivedAt))).toBe(true);
    expect(result.every(row => row.diagHasDynamicEnergy)).toBe(true);
    expect(result.every(row => row.energySampleSource === 'per-tick-ledger')).toBe(true);
    const groups = new Map();
    for (const row of result) {
        const group = groups.get(row.auditVersion) || [];
        group.push(row);
        groups.set(row.auditVersion, group);
    }
    const reused = [...groups.values()].find(
        group => new Set(group.map(row => row.diagTick)).size >= 2,
    );
    expect(reused, 'L=97 should reuse at least one audit over multiple diagnostic frames').toBeTruthy();
    expect(new Set(reused.map(row => row.auditTick)).size).toBe(1);
    expect(new Set(reused.map(row => row.auditSampledAt)).size).toBe(1);
    expect(new Set(reused.map(row => row.auditReceivedAt)).size).toBe(1);
    expect(new Set(reused.map(row => row.diagnosticsVersion)).size).toBeGreaterThan(1);
    expect(reused.filter(row => row.diagnosticsTick !== row.auditSampleTick)
        .every(row => row.diagHasDynamicEnergy === true)).toBe(true);
    const ordered = [...reused].sort((a, b) => a.observedAt - b.observedAt);
    expect(ordered.at(-1).observedAt - ordered.at(-1).auditReceivedAt)
        .toBeGreaterThanOrEqual(ordered[0].observedAt - ordered[0].auditReceivedAt);
});
