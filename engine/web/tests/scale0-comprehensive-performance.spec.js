import { test, expect } from '@playwright/test';
import { writeFileSync, mkdirSync, existsSync } from 'node:fs';
import { resolve } from 'node:path';
import { gotoAndReady, selectScale0Scenario, attachConsoleWatcher, realErrors } from './_helpers.js';
import { prepareDampingPerformanceProfile } from './scale0-damping-performance-preparation.js';

// Fixed acceptance thresholds, registered before execution. A failed row is
// retained and reported; the campaign continues so failures cannot hide scope.
test.use({ trace: 'off' });
const sizes = (process.env.FTD_AUDIT_SIZES || '33,97').split(',').map(Number);
const kind = process.env.FTD_AUDIT_KIND || 'overlays';
const profile = process.env.FTD_AUDIT_PROFILE || 'baseline';
if (!['overlays', 'panels'].includes(kind) || sizes.some(n => ![33, 97].includes(n))) throw new Error('Unknown registered matrix kind/size');
if (!['baseline', 'repair-v1', 'repair-v2', 'wave-v3', 'time-gravity-v4'].includes(profile)
    || (['repair-v1', 'wave-v3', 'time-gravity-v4'].includes(profile) && kind !== 'panels')) throw new Error('Unknown registered profile');
const candidateIdentity = process.env.FTD_AUDIT_CANDIDATE_ID || null;
if (profile === 'wave-v3' && !/^[a-f0-9]{64}$/.test(candidateIdentity || '')) throw new Error('Wave v3 requires its frozen candidate identity');
if (profile === 'wave-v3' && sizes.join(',') !== '33,97') throw new Error('Wave v3 requires the complete ordered L33/L97 matrix');
if (profile === 'time-gravity-v4' && !/^[a-f0-9]{64}$/.test(candidateIdentity || '')) throw new Error('Time/Gravity v4 requires its frozen candidate identity');
if (profile === 'time-gravity-v4' && sizes.join(',') !== '33,97') throw new Error('Time/Gravity v4 requires the complete ordered L33/L97 matrix');
const reportDate = ['wave-v3', 'time-gravity-v4'].includes(profile) ? '2026-09-08' : '2026-09-07';
// Preregistered union of panels with baseline callback failures, including
// unchanged comparison workloads. Thresholds, preparations and cadence match
// the baseline. Separate files retain every failed baseline measurement.
const repairPanels = ['flux-slice', 'wave-lab', 'p1-observables', 'spectrum', 'gravity', 'time', 'thermo', 'scale-context'];
const gates = { minFrames: 600, minDurationMs: 12000, minFps: 59.5, p95Ms: 17, p99Ms: 20, maxLongFrames: 0, actionSamples: 10, actionP95Ms: 50 };

for (const size of sizes) {
    test(`complete ${kind} hardware matrix L=${size}`, async ({ page }, info) => {
        info.setTimeout(1800000);
        expect(process.env.FTD_HARDWARE_WEBGL, 'hardware campaign requires explicit hardware WebGL').toBe('1');
        if (size === 33) await page.addInitScript(() => { window.__ftdWasmWorker = false; });
        const errors = attachConsoleWatcher(page);
        await gotoAndReady(page, { path: '/?engine=wasm', timeout: 90000 });
        await page.selectOption('#lattice-size', String(size));
        const plan = await page.evaluate(async ({ kind, profile, repairPanels }) => {
            const { FIELD_TOGGLE_BINDINGS } = await import('/js/scales/scale0/ui/dom.js');
            const { getScale0OverlayApplicability } = await import('/js/scales/scale0/ui/overlays/applicability.js');
            const { SCALE0_SCENARIOS } = await import('/js/scales/scale0/scenario-registry.js');
            const preferred = ['flux-pulse', 'flux-dual-substrate', 'flux-baryon', 'flux-random-genesis',
                's0-seed-time-twin-clocks', 's0-seed-de-broglie-clock', 's0-seed-massive-body'];
            const ids = [...new Set([...preferred, ...SCALE0_SCENARIOS.map(s => s.id)])];
            if (kind === 'panels') {
                const { getPanelsForScale } = await import('/js/ui/scale-registry/panel-registry.js');
                return getPanelsForScale('0').filter(p => profile === 'time-gravity-v4' ? ['time', 'gravity'].includes(p.id)
                    : profile === 'wave-v3' ? p.id === 'wave-lab'
                    : profile !== 'repair-v1' || repairPanels.includes(p.id)).map(p => ({ id: p.id, scenario: {
                    time: 's0-seed-time-twin-clocks', thermo: 'flux-thermalization',
                    'wave-lab': 's0-field-rf-lattice-wave', gravity: 's0-seed-massive-body',
                }[p.id] || 'flux-pulse' }));
            }
            return [...FIELD_TOGGLE_BINDINGS.map(([id]) => id), 'toggle-flux-volume', 'toggle-flux-slice', 'toggle-sm-reference']
                .map(id => profile === 'repair-v2' && id === 'toggle-damping-zones'
                    ? { id, scenario: 's0-seed-sloop', preparationProfile: 'damping-zones-modified-v1' }
                    : { id, scenario: ids.find(s => getScale0OverlayApplicability(s).applicable.has(id)) || null });
        }, { kind, profile, repairPanels });
        expect(plan).toHaveLength(profile === 'time-gravity-v4' ? 2 : profile === 'wave-v3' ? 1 : profile === 'repair-v1' ? 8 : kind === 'panels' ? 18 : 36);
        if (profile === 'wave-v3') expect(plan).toEqual([{ id: 'wave-lab', scenario: 's0-field-rf-lattice-wave' }]);
        if (profile === 'time-gravity-v4') expect([...plan].sort((a, b) => a.id.localeCompare(b.id))).toEqual([
            { id: 'gravity', scenario: 's0-seed-massive-body' },
            { id: 'time', scenario: 's0-seed-time-twin-clocks' },
        ]);
        if (profile === 'repair-v1') expect(plan.map(row => row.id).sort()).toEqual([...repairPanels].sort());
        expect(new Set(plan.map(row => row.id)).size).toBe(plan.length);
        const report = { schema: 'scale0-hardware-audit-v1', kind, profile, candidateIdentity, size, gates, plan, scope: profile === 'time-gravity-v4'
            ? 'Time retained presentation and Gravity same-publication slab observations: four visibility states on direct L33 and worker L97; original preparation, numerical support, cadence and inclusive callback/frame gates. Raw timing vectors retained after measurement. No other panel or physical recovery approval.'
            : profile === 'wave-v3'
            ? 'Wave Lab correctness and arithmetic successor: four visibility states on direct L33 and worker L97; original preparation, sampling and inclusive callback/frame gates. Raw timing vectors retained after measurement. No other panel or physical recovery approval.'
            : profile === 'repair-v1'
            ? 'Eight preregistered panels with baseline callback failures; four visibility states, same preparations and gates; distinct repair candidate. This followup does not replace baseline failures or establish universal performance.'
            : '36 registered toggles or 18 registered panels; selected applicable preparation per interface, two backend sizes. Interface callback timings include nested owner reads; standalone ownerRead timing rows are diagnostic and excluded from the callback gate. Full-frame gate includes all foreground work. repair-v2 covers the full matrix because shared Conservation and worker reads changed.', rows: [], errors: [] };
        const output = resolve('../../docs/evidence', `scale0-hardware-${profile === 'baseline' ? '' : `${profile}-`}${kind}-L${size}-${reportDate}.json`);
        mkdirSync(resolve('../../docs/evidence'), { recursive: true });
        if (existsSync(output)) throw new Error(`Refusing to overwrite previous campaign evidence: ${output}`);
        const save = () => writeFileSync(output, JSON.stringify(report, null, 2) + '\n');
        save();
        for (const row of plan) {
            if (!row.scenario) { report.rows.push({ ...row, failure: 'no registered applicable preparation' }); save(); continue; }
            try {
                await selectScale0Scenario(page, row.scenario, { settleMs: 0 });
                await expect.poll(() => page.evaluate(async ({ size, scenario }) => {
                    const { getScale0State, isScale0AuthoritativeGenerationReady } = await import('/js/scales/scale0/state/store.js');
                    const s = getScale0State();
                    const owner = s.useFluxMock ? s.fluxMock : window.__ftdCtx.bridge;
                    const lifecycle = owner?.lifecycleDebug;
                    return !!owner && owner.isWasm === true && Number(owner.latticeSize) === size && owner.ready !== false
                        && isScale0AuthoritativeGenerationReady(s)
                        && s.qualificationAnchor.loadGeneration === window.__ftdCtx._loadGeneration
                        && s.currentScenarioId === scenario
                        && (!lifecycle || lifecycle.configurationToken === lifecycle.appliedConfigurationToken)
                        && (size === 97 ? owner.isWorker === true : owner.isWorker !== true);
                }, { size, scenario: row.scenario }), { timeout: 90000 }).toBe(true);
                const preparation = row.preparationProfile
                    ? await prepareDampingPerformanceProfile(page) : null;
                const availability = await page.evaluate(async ({ row, kind }) => {
                    const ctx = window.__ftdCtx;
                    ctx.pauseSimulation();
                    const { COL_TO_TOGGLES } = await import('/js/scales/scale0/ui/overlays/presets.js');
                    for (const id of Object.values(COL_TO_TOGGLES).flat()) {
                        const button = document.getElementById(id);
                        if (button?.classList.contains('active') && !button.classList.contains('is-inapplicable')) button.click();
                    }
                    const dock = ctx.appShell.panelDock;
                    dock.setCollapsed(false);
                    dock.activate(kind === 'panels' ? row.id : 'controls');
                    if (kind === 'panels') {
                        if (row.id === 'inspector') {
                            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
                            const s = getScale0State();
                            const owner = s.useFluxMock ? s.fluxMock : ctx.bridge;
                            if (ctx.inspector?.bridge !== owner) throw new Error('Inspector has the wrong physics owner');
                            const center = Math.floor(owner.latticeSize / 2);
                            if (!ctx.inspector?.selectLatticePosition({ x: center, y: center, z: center })) {
                                throw new Error('Inspector selected-voxel workload unavailable');
                            }
                        }
                        const { isPanelLive } = await import('/js/ui/panels/panel-visibility.js?v=2');
                        return isPanelLive(document.getElementById(`panel-${row.id}`));
                    }
                    const button = document.getElementById(row.id);
                    if (!button || button.classList.contains('is-inapplicable')) return false;
                    if (!button.classList.contains('active')) button.click();
                    return button.classList.contains('active');
                }, { row, kind });
                if (!availability) throw new Error('live engine marks preparation unavailable');
                // Warm initialization, shaders and any outstanding pause tick.
                await page.waitForTimeout(2000);
                const states = kind === 'panels' ? ['docked', 'floated', 'collapsed', 'hidden'] : ['paused', 'playing'];
                for (const state of states) {
                    await page.evaluate(async ({ state, row, kind }) => {
                        const ctx = window.__ftdCtx;
                        const { floatingWindowManager } = await import('/js/ui/components/floating-window/component.js');
                        const win = floatingWindowManager.getWindow(row.id);
                        document.documentElement.classList.remove('ui-hidden');
                        if (state === 'floated' && !ctx.appShell.panelDock.floatPanel(row.id, 90, 90)) throw new Error('Panel failed to float');
                        if (state === 'collapsed' && win && !win.isCollapsed) win.toggleCollapse();
                        if (state === 'hidden') {
                            if (win?.isCollapsed) win.toggleCollapse();
                            document.documentElement.classList.add('ui-hidden');
                        }
                        if (state === 'paused') ctx.pauseSimulation();
                        if (state === 'playing' || kind === 'panels') {
                            const play = document.getElementById('btn-play');
                            if (play?.getAttribute('data-paused') === 'true') play.click();
                        }
                    }, { state, row, kind });
                    await expect.poll(() => page.evaluate(async () => {
                        const { getScale0State } = await import('/js/scales/scale0/state/store.js');
                        const s = getScale0State();
                        const owner = s.useFluxMock ? s.fluxMock : window.__ftdCtx.bridge;
                        return owner?.runningStateSettled !== false;
                    })).toBe(true);
                    await page.waitForTimeout(1000);
                    const measurement = await page.evaluate(async ({ gates, row, kind, state, preparation, profile }) => {
                        const probe = await import('/tests/scale0-ui-audit-probe.js');
                        const { getScale0State, getScale0QualificationState, isScale0AuthoritativeGenerationReady } = await import('/js/scales/scale0/state/store.js');
                        const { isPanelLive } = await import('/js/ui/panels/panel-visibility.js?v=2');
                        const ctx = window.__ftdCtx;
                        const s = getScale0State();
                        const owner = s.useFluxMock ? s.fluxMock : ctx.bridge;
                        const gl = ctx.viewport.renderer.getContext();
                        const ext = gl.getExtension('WEBGL_debug_renderer_info');
                        const renderer = ext ? String(gl.getParameter(ext.UNMASKED_RENDERER_WEBGL)) : '';
                        const readTick = () => owner.currentTick?.() ?? owner.getDiagnostics?.()?.tick ?? owner.tickCount ?? null;
                        const tickBefore = readTick();
                        const qualificationBefore = getScale0QualificationState();
                        const readProfile = () => preparation ? Object.fromEntries(
                            Object.keys(preparation.modified.terms).map(key => [key, owner.isWorker
                                ? owner.getEngineTruthToggle(key) : owner.getToggle(key)])) : null;
                        const termsBefore = readProfile();
                        const dampingGeometryBefore = preparation ? ctx.viewport._dampingZones?.geometry.drawRange.count : null;
                        const lifecycleBefore = owner.lifecycleDebug ?? null;
                        const pausedBefore = document.getElementById('btn-play')?.dataset.paused;
                        const panel = document.getElementById(`panel-${row.id}`);
                        const readMount = () => {
                            const currentHost = panel?.closest('.floating-window');
                            return kind === 'panels' ? { connected: panel?.isConnected === true, live: isPanelLive(panel), floated: !!currentHost, collapsed: currentHost?.classList.contains('is-collapsed') === true } : null;
                        };
                        const mount = readMount();
                        const generation = ctx._loadGeneration;
                        probe.startScale0UiAuditProbe({
                            rootSelector: kind === 'panels' ? `#panel-${row.id}` : '#viewport-overlay',
                            subscriberPrefixes: ['flux-slice-panel', 'conservation-micropanel', 'dispersion-panel',
                                'knots-panel', 'gravity-panel', 'p1-observables-panel', 'scale-context-panel',
                                'spectrum-panel', 'thermo-panel', 'wave-lab-panel', 'time-panel', 'transaction-panel'],
                        });
                        probe.trackScale0UiMethods('ownerRead', owner, ['getScale0FieldSamples', 'getFluxVolume', 'getFluxSlice', 'sampleFluxAtCells', 'getDiagnostics', 'getEnergyAudit', 'getParticleData']);
                        if (kind === 'panels') {
                            for (const key of ['diagnosticsPanel', 'chartsPanel', 'telemetryGridPanel', 'lagrangianPanel', 'inspector']) {
                                probe.trackScale0UiMethods(`panelUpdate.${key}`, ctx[key], ['update']);
                            }
                        }
                        let result;
                        try { await new Promise((resolveFrame, rejectFrame) => {
                            let active = true;
                            let rafId = 0;
                            const timeout = setTimeout(() => { active = false; cancelAnimationFrame(rafId); rejectFrame(new Error('Measurement frame timeout')); }, 60000);
                            let frames = 0;
                            const start = performance.now();
                            function frame(now) {
                                if (!active) return;
                                if (++frames > gates.minFrames && now - start >= gates.minDurationMs) { active = false; clearTimeout(timeout); resolveFrame(); }
                                else rafId = requestAnimationFrame(frame);
                            }
                            rafId = requestAnimationFrame(frame);
                        });
                        // Registered interaction sample: synchronous dispatch to
                        // next foreground rAF, not an actual display-present timestamp.
                        // Paired actions restore the same visibility and physics owner.
                        if (kind === 'overlays' || ['docked', 'floated'].includes(state)) {
                            const { floatingWindowManager } = await import('/js/ui/components/floating-window/component.js');
                            for (let i = 0; i < gates.actionSamples; i++) {
                                await probe.measureScale0UiActionToPaint(`${row.id}:toggle:${i}`, () => {
                                    if (kind === 'overlays') document.getElementById(row.id).click();
                                    else if (state === 'floated') floatingWindowManager.getWindow(row.id).toggleCollapse();
                                    else ctx.appShell.panelDock.setCollapsed(i % 2 === 0);
                                });
                            }
                        }
                        } finally { result = await probe.stopScale0UiAuditProbe({ retainTimingSamples: ['wave-v3', 'time-gravity-v4'].includes(profile) }); }
                        const current = getScale0State();
                        return { ...result, renderer, backend: owner.isWorker ? 'wasm-worker' : 'wasm-direct',
                            tickBefore, tickAfter: readTick(), qualificationBefore, qualificationAfter: getScale0QualificationState(), lifecycleBefore, lifecycleAfter: owner.lifecycleDebug ?? null, mount, mountAfter: readMount(), pausedBefore, pausedAfter: document.getElementById('btn-play')?.dataset.paused,
                            termsBefore, termsAfter: readProfile(), dampingGeometryBefore,
                            dampingGeometryAfter: preparation ? ctx.viewport._dampingZones?.geometry.drawRange.count : null,
                            authoritativeReady: isScale0AuthoritativeGenerationReady(current) && current.qualificationAnchor.loadGeneration === ctx._loadGeneration,
                            isWasm: owner.isWasm === true,
                            sameOwner: owner === (current.useFluxMock ? current.fluxMock : ctx.bridge),
                            sameGeneration: generation === ctx._loadGeneration, visibility: document.visibilityState };
                    }, { gates, row, kind, state, preparation, profile });
                    const failures = [];
                    const f = measurement.frames;
                    if (![f.count, f.effectiveFps, f.p95Ms, f.p99Ms, measurement.durationMs].every(Number.isFinite)) failures.push('frame instrumentation');
                    if (!measurement.renderer || /swiftshader|software/i.test(measurement.renderer)) failures.push('hardware provenance');
                    if (measurement.visibility !== 'visible') failures.push('background page');
                    if (measurement.durationMs < gates.minDurationMs || f.count < gates.minFrames || f.effectiveFps < gates.minFps || f.p95Ms > gates.p95Ms || f.p99Ms > gates.p99Ms
                        || f.intervalsOver33_4ms > gates.maxLongFrames || measurement.longTasks.length) failures.push('foreground frame gate');
                    if (!measurement.longTaskSupported) failures.push('long-task instrumentation unavailable');
                    if ((kind === 'overlays' || ['docked', 'floated'].includes(state))
                        && (measurement.actions.count !== gates.actionSamples || !Number.isFinite(measurement.actions.p95Ms) || measurement.actions.p95Ms > gates.actionP95Ms)) failures.push('action to next-frame budget');
                    if (Object.entries(measurement.callbacks).filter(([id]) => !id.startsWith('method:ownerRead.')).some(([,c]) => c.p95Ms > 2 || c.maxMs > 8)) failures.push('interface callback budget');
                    const updateOwner = { diagnostics: 'diagnosticsPanel', charts: 'chartsPanel', 'telemetry-grid': 'telemetryGridPanel', lagrangian: 'lagrangianPanel', inspector: 'inspector' }[row.id];
                    if (kind === 'panels' && updateOwner && ['docked', 'floated'].includes(state)
                        && !(measurement.callbacks[`method:panelUpdate.${updateOwner}.update`]?.count > 0)) failures.push('panel update instrumentation missing');
                    if (kind === 'panels' && ['collapsed', 'hidden'].includes(state) && measurement.dom.canvasDraws) failures.push('hidden canvas work');
                    const expectedQualification = preparation ? 'suspended' : 'within-contract';
                    if (!measurement.sameOwner || !measurement.sameGeneration || !measurement.authoritativeReady || !measurement.isWasm
                        || measurement.qualificationBefore.status !== expectedQualification || measurement.qualificationAfter.status !== expectedQualification
                        || JSON.stringify(measurement.lifecycleBefore) !== JSON.stringify(measurement.lifecycleAfter)) failures.push('state ownership/qualification');
                    if (preparation && (JSON.stringify(measurement.termsBefore) !== JSON.stringify(preparation.modified.terms)
                        || JSON.stringify(measurement.termsAfter) !== JSON.stringify(preparation.modified.terms)
                        || JSON.stringify(measurement.qualificationBefore) !== JSON.stringify(preparation.modified.qualification)
                        || JSON.stringify(measurement.qualificationBefore) !== JSON.stringify(measurement.qualificationAfter)
                        || JSON.stringify(measurement.lifecycleBefore) !== JSON.stringify(preparation.modified.lifecycle)
                        || JSON.stringify(measurement.qualificationBefore.anchor) !== JSON.stringify(preparation.baseline.qualification.anchor)
                        || measurement.dampingGeometryBefore !== 288 || measurement.dampingGeometryAfter !== 288)) {
                        failures.push('registered modified damping profile/glyph workload');
                    }
                    if (measurement.errors.length) failures.push('instrumented runtime errors');
                    if (![measurement.tickBefore, measurement.tickAfter].every(t => Number.isSafeInteger(t) && t >= 0)) failures.push('clock provenance');
                    const playing = state === 'playing' || kind === 'panels';
                    if (measurement.pausedBefore !== (playing ? 'false' : 'true') || measurement.pausedAfter !== (playing ? 'false' : 'true')) failures.push('play state');
                    if (playing && !(measurement.tickAfter > measurement.tickBefore)) failures.push('playing clock did not advance');
                    if (kind === 'panels') {
                        const m = measurement.mount;
                        if (JSON.stringify(m) !== JSON.stringify(measurement.mountAfter) || !m.connected || m.live !== !['collapsed', 'hidden'].includes(state)
                            || m.floated !== (state !== 'docked') || m.collapsed !== (state === 'collapsed')) failures.push('panel mount state');
                    }
                    if (state === 'paused' && measurement.tickBefore !== measurement.tickAfter) failures.push('paused clock changed');
                    report.rows.push({ ...row, preparation, state, ...measurement, failures });
                    save();
                    console.log(JSON.stringify({ id: row.id, size, state, fps: f.effectiveFps, p99: f.p99Ms, failures }));
                }
            } catch (error) { report.rows.push({ ...row, failure: String(error) }); save(); }
            finally {
                await page.evaluate(async id => {
                    document.documentElement.classList.remove('ui-hidden');
                    const { floatingWindowManager } = await import('/js/ui/components/floating-window/component.js');
                    floatingWindowManager.getWindow(id)?.dock();
                    window.__ftdCtx.pauseSimulation();
                }, row.id);
            }
        }
        report.errors = realErrors(errors);
        save();
        await info.attach(`hardware-${kind}-L${size}`, { path: output, contentType: 'application/json' });
        expect(report.rows).toHaveLength(plan.length * (kind === 'panels' ? 4 : 2));
        const failedRows = report.rows.filter(r => r.failure || r.failures?.length);
        // Keep raw vectors in the report attachment; avoid dumping them into assertion output.
        expect(profile === 'time-gravity-v4' ? failedRows.map(({ id, state, failure, failures }) => ({ id, state, failure, failures }))
            : failedRows, 'all registered rows meet gates').toEqual([]);
        expect(report.errors).toEqual([]);
    });
}
