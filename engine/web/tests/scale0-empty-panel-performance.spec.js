// @ts-check
/**
 * Absolute Scale-0 Scenario 1 (`empty`) side-panel performance campaign.
 *
 * Scope: the supported browser WasmBridgeProxy path at L=97. Every Scale-0
 * panel that is visible in the canonical panel registry is warmed, measured
 * for at least 240 foreground rAF intervals, then measured again after the
 * shared dock is collapsed. Native GPU and WebSocket backends are not
 * connected by this harness and are not represented as parity evidence.
 */
import { test, expect } from '@playwright/test';
import {
    attachConsoleWatcher,
    gotoAndReady,
    realErrors,
    selectScale0Scenario,
} from './_helpers.js';

// The campaign measures compositor cadence. Playwright trace screencasts add
// their own WebGL readback and would make the result a measurement of tracing.
test.use({ trace: 'off' });

const EMPTY = 'empty';
const LATTICE_SIZE = 97;
const PANELS = Object.freeze([
    'controls',
    'diagnostics',
    'telemetry-grid',
    'charts',
    'lagrangian',
    'inspector',
    'scene',
    'flux-slice',
    'wave-lab',
    'p1-observables',
    'spectrum',
    'dispersion',
    'knots',
    'gravity',
    'time',
    'thermo',
    'scale-context',
]);
const REQUESTED_PANELS = Object.freeze(
    (process.env.FTD_PANEL_PERF_ONLY || '')
        .split(',')
        .map((panel) => panel.trim())
        .filter(Boolean),
);
const CAMPAIGN_PANELS = REQUESTED_PANELS.length ? REQUESTED_PANELS : PANELS;

// These gates are deliberately absolute, not relative to a baseline run.
// Do not loosen them in response to a failing panel; a failure is evidence.
const GATES = Object.freeze({
    visibleFrames: 240,
    collapsedFrames: 180,
    minFps: 59.5,
    p95Ms: 16.9,
    p99Ms: 20,
    maxRootDomNodes: 2_500,
    maxRootCanvases: 12,
    maxCoordinatorSubscribers: 32,
    maxVisibleMutations: 2_000,
    maxVisibleCanvasDraws: 5_000,
    maxCallbackP99Ms: 4,
    maxCollapsedCallbackMs: 1,
    maxHeapGrowthBytes: 64 * 1024 * 1024,
});

test.describe.configure({ mode: 'serial' });
test.beforeEach(async ({ page }) => {
    page.setDefaultTimeout(30_000);
    await page.addInitScript(() => { window.__ftdTelemetryOnDemand = true; });
});

/** @param {import('@playwright/test').Page} page */
async function waitForEmptyWorker(page) {
    await expect.poll(async () => page.evaluate(async ({ scenarioId, latticeSize }) => {
        const { getScale0State } = await import('/js/scales/scale0/state/store.js');
        const state = getScale0State();
        const owner = state.fluxMock;
        return state.currentScenarioId === scenarioId
            && state.useFluxMock === true
            && owner?.isWorker === true
            && owner.ready === true
            && owner._scenarioId === scenarioId
            && Number(owner.latticeSize) === latticeSize;
    }, { scenarioId: EMPTY, latticeSize: LATTICE_SIZE }), {
        timeout: 90_000,
        message: `Scale-0 ${EMPTY} WasmBridgeProxy did not become ready at L=${LATTICE_SIZE}`,
    }).toBe(true);
}

function callbackSampleCount(report) {
    return Object.values(report.callbacks || {})
        .reduce((sum, entry) => sum + Number(entry.count || 0), 0);
}

function callbackWorst(report, key) {
    return Object.values(report.callbacks || {})
        .reduce((worst, entry) => Math.max(worst, Number(entry[key] || 0)), 0);
}

test('P1 Observables suspends on Empty intent and restores only after a nonempty generation commits', async ({ page }) => {
    test.setTimeout(180_000);
    const consoleErrors = attachConsoleWatcher(page);
    await gotoAndReady(page, { path: '/?engine=wasm', timeout: 90_000 });
    await selectScale0Scenario(page, 'flux-pulse', { settleMs: 750 });
    await page.locator('#tab-bar .tab[data-panel="p1-observables"]').click();

    await expect.poll(() => page.evaluate(() => ({
        applicability: window.__ftdP1Panel?.applicability ?? null,
        coordinatorActive: window.__ftdP1Panel?.coordinatorActive ?? null,
        mountedComponentCount: window.__ftdP1Panel?.mountedComponentCount ?? null,
        updateCount: window.__ftdP1Panel?.updateCount ?? 0,
    })), { timeout: 15_000 }).toMatchObject({
        applicability: 'applicable',
        coordinatorActive: true,
        mountedComponentCount: 8,
    });

    const intentBoundary = await page.evaluate(() => {
        const api = window.__ftdP1Panel;
        const select = /** @type {HTMLSelectElement|null} */ (document.getElementById('scenario-select'));
        if (!api || !select) throw new Error('P1 panel or scenario selector unavailable');
        const before = api.updateCount;
        select.value = 'empty';
        select.dispatchEvent(new Event('change', { bubbles: true }));
        api.update();
        const root = document.getElementById('panel-p1-observables');
        const panel = root?.querySelector('#p1-observables-panel');
        const message = root?.querySelector('.p1-inapplicable');
        return {
            before,
            afterManualUpdate: api.updateCount,
            applicability: panel?.dataset.applicability ?? null,
            messageStatus: message?.dataset.applicability ?? null,
            message: message?.textContent?.replace(/\s+/g, ' ').trim() ?? null,
            messageVisible: !!message && !message.hidden,
            contentHidden: !!root?.querySelector('.p1-applicable-content')?.hidden,
            mountedComponentCount: api.mountedComponentCount,
            sectionCount: root?.querySelectorAll('.p1-applicable-content > section').length ?? null,
            scientificControlCount: root?.querySelectorAll('.p1-applicable-content button, .p1-applicable-content input').length ?? null,
            coordinatorActive: api.coordinatorActive,
            subscriberPresent: [...(window.__ftdRAF?._subs?.keys?.() || [])]
                .some((id) => id.startsWith('p1-observables-panel')),
        };
    });

    expect(intentBoundary).toMatchObject({
        applicability: 'inapplicable-empty',
        messageStatus: 'inapplicable',
        messageVisible: true,
        contentHidden: true,
        mountedComponentCount: 0,
        sectionCount: 0,
        scientificControlCount: 0,
        coordinatorActive: false,
        subscriberPresent: false,
    });
    expect(intentBoundary.afterManualUpdate, 'manual update remains inert after Empty intent')
        .toBe(intentBoundary.before);
    expect(intentBoundary.message).toContain('No particle-list or field-volume sampling is performed');
    expect(intentBoundary.message).toContain('not a measurement of physical vacuum');

    await expect.poll(() => page.evaluate(async () => {
        const { getScale0State } = await import('/js/scales/scale0/state/store.js');
        return getScale0State().currentScenarioId;
    }), { timeout: 90_000 }).toBe(EMPTY);
    const frozenCount = await page.evaluate(() => window.__ftdP1Panel?.updateCount ?? null);
    await page.waitForTimeout(1_000);
    expect(await page.evaluate(() => window.__ftdP1Panel?.updateCount ?? null),
        'Empty performs no periodic P1 scientific pass').toBe(frozenCount);

    await selectScale0Scenario(page, 'flux-pulse', { settleMs: 750 });
    await expect.poll(() => page.evaluate(() => ({
        applicability: window.__ftdP1Panel?.applicability ?? null,
        coordinatorActive: window.__ftdP1Panel?.coordinatorActive ?? null,
        mountedComponentCount: window.__ftdP1Panel?.mountedComponentCount ?? null,
    })), { timeout: 15_000 }).toEqual({
        applicability: 'applicable',
        coordinatorActive: true,
        mountedComponentCount: 8,
    });
    expect(realErrors(consoleErrors)).toEqual([]);
});

test('Knots preserves user preference but suppresses all tracking work on Empty', async ({ page }) => {
    test.setTimeout(180_000);
    const consoleErrors = attachConsoleWatcher(page);
    await gotoAndReady(page, { path: '/?engine=wasm', timeout: 90_000 });
    await selectScale0Scenario(page, 'flux-pulse', { settleMs: 750 });
    await page.locator('#tab-bar .tab[data-panel="knots"]').click();
    expect(await page.evaluate(async () => {
        const [{ getScale0State }, { getScale0TelemetryDemand }] = await Promise.all([
            import('/js/scales/scale0/state/store.js'),
            import('/js/telemetry/demand.js'),
        ]);
        const state = getScale0State();
        return {
            trackingPreference: state.knotTracking,
            wantAudit: getScale0TelemetryDemand(window.__ftdCtx, state).wantAudit,
        };
    }), 'visible Knots with tracking off does not request the audit stream').toEqual({
        trackingPreference: false,
        wantAudit: false,
    });
    await page.locator('#kp-toggle-overlay').check();

    await expect.poll(() => page.evaluate(async () => {
        const {
            getScale0State,
            isKnotTrackingActive,
            isKnotZonesActive,
        } = await import('/js/scales/scale0/state/store.js');
        const state = getScale0State();
        return {
            applicability: window.__ftdKnotsPanel?.applicability ?? null,
            preference: state.knotTracking,
            scenarioApplicable: state.knotTrackingApplicable,
            effective: isKnotTrackingActive(state),
            coordinatorActive: window.__ftdKnotsPanel?.coordinatorActive ?? null,
            measurementActive: window.__ftdKnotsPanel?.measurementActive ?? null,
            contributionEnabled: window.__ftdKnotsPanel?.contributionEnabled ?? null,
            knotZonesRequested: state.knotZonesRequested,
            knotZonesEffective: isKnotZonesActive(state),
            knotZonesRendered: state.fieldFlags.showKnotZones,
        };
    }), { timeout: 15_000 }).toEqual({
        applicability: 'applicable',
        preference: true,
        scenarioApplicable: true,
        effective: true,
        coordinatorActive: true,
        measurementActive: true,
        contributionEnabled: true,
        knotZonesRequested: true,
        knotZonesEffective: true,
        knotZonesRendered: true,
    });

    await page.locator('#kp-toggle-tracking').uncheck();
    await expect.poll(() => page.evaluate(async () => {
        const {
            getScale0State,
            isKnotTrackingActive,
            isKnotZonesActive,
        } = await import('/js/scales/scale0/state/store.js');
        const state = getScale0State();
        const knotMesh = window.__ftdCtx?.viewport?._fieldRenderer?._knotZones ?? null;
        return {
            trackingPreference: state.knotTracking,
            trackingEffective: isKnotTrackingActive(state),
            knotZonesRequested: state.knotZonesRequested,
            knotZonesEffective: isKnotZonesActive(state),
            knotZonesRendered: state.fieldFlags.showKnotZones,
            knotMeshVisible: knotMesh?.visible ?? false,
            knotMeshDrawCount: knotMesh?.geometry?.drawRange?.count ?? null,
            schedulerActive: !!state.overlaySched?.active,
        };
    }), { timeout: 15_000 }).toEqual({
        trackingPreference: false,
        trackingEffective: false,
        knotZonesRequested: true,
        knotZonesEffective: false,
        knotZonesRendered: false,
        knotMeshVisible: false,
        knotMeshDrawCount: 0,
        schedulerActive: false,
    });
    await page.locator('#kp-toggle-tracking').check();
    await expect.poll(() => page.evaluate(async () => {
        const { getScale0State, isKnotZonesActive } = await import('/js/scales/scale0/state/store.js');
        const state = getScale0State();
        return {
            trackingPreference: state.knotTracking,
            knotZonesRequested: state.knotZonesRequested,
            knotZonesEffective: isKnotZonesActive(state),
            knotZonesRendered: state.fieldFlags.showKnotZones,
            knotMeshVisible: window.__ftdCtx?.viewport?._fieldRenderer?._knotZones?.visible ?? false,
        };
    }), { timeout: 15_000 }).toEqual({
        trackingPreference: true,
        knotZonesRequested: true,
        knotZonesEffective: true,
        knotZonesRendered: true,
        knotMeshVisible: true,
    });

    const visibilityBoundary = await page.evaluate(() => {
        const dock = window.__ftdCtx?.appShell?.panelDock;
        const api = window.__ftdKnotsPanel;
        if (!dock || !api) throw new Error('Knots visibility boundary unavailable');
        const snapshot = () => ({
            measurementActive: api.measurementActive,
            contributionEnabled: api.contributionEnabled,
            liveSubscriberPresent: [...(window.__ftdRAF?._subs?.keys?.() || [])]
                .includes('knots-panel'),
        });

        dock.setCollapsed(true);
        const afterCollapse = snapshot();
        dock.setCollapsed(false);
        const afterExpand = snapshot();
        dock.activate('controls');
        const afterTabHide = snapshot();
        dock.activate('knots');
        const afterTabRestore = snapshot();
        const floating = dock.floatPanel('knots', 420, 120);
        if (!floating) throw new Error('Knots floating window unavailable');
        const afterFloat = snapshot();
        floating.toggleCollapse();
        const afterFloatingCollapse = snapshot();
        floating.toggleCollapse();
        const afterFloatingRestore = snapshot();
        floating.dock();
        const afterDockRestore = snapshot();
        return {
            afterCollapse,
            afterExpand,
            afterTabHide,
            afterTabRestore,
            afterFloat,
            afterFloatingCollapse,
            afterFloatingRestore,
            afterDockRestore,
        };
    });
    expect(visibilityBoundary).toEqual({
        afterCollapse: {
            measurementActive: false,
            contributionEnabled: false,
            liveSubscriberPresent: false,
        },
        afterExpand: {
            measurementActive: true,
            contributionEnabled: true,
            liveSubscriberPresent: true,
        },
        afterTabHide: {
            measurementActive: false,
            contributionEnabled: false,
            liveSubscriberPresent: false,
        },
        afterTabRestore: {
            measurementActive: true,
            contributionEnabled: true,
            liveSubscriberPresent: true,
        },
        afterFloat: {
            measurementActive: true,
            contributionEnabled: true,
            liveSubscriberPresent: true,
        },
        afterFloatingCollapse: {
            measurementActive: false,
            contributionEnabled: false,
            liveSubscriberPresent: false,
        },
        afterFloatingRestore: {
            measurementActive: true,
            contributionEnabled: true,
            liveSubscriberPresent: true,
        },
        afterDockRestore: {
            measurementActive: true,
            contributionEnabled: true,
            liveSubscriberPresent: true,
        },
    });

    const intentBoundary = await page.evaluate(async () => {
        const [
            { getScale0State, isKnotTrackingActive, isKnotZonesActive },
            { getFieldLineKnotTracker },
            { getScale0TelemetryDemand },
        ] = await Promise.all([
            import('/js/scales/scale0/state/store.js'),
            import('/js/scales/scale0/runtime/field-line-knots.js'),
            import('/js/telemetry/demand.js'),
        ]);
        const api = window.__ftdKnotsPanel;
        const select = /** @type {HTMLSelectElement|null} */ (document.getElementById('scenario-select'));
        if (!api || !select) throw new Error('Knots panel or scenario selector unavailable');
        const before = api.updateCount;
        select.value = 'empty';
        select.dispatchEvent(new Event('change', { bubbles: true }));
        api.update();
        const state = getScale0State();
        const root = document.getElementById('panel-knots');
        const panel = root?.querySelector('#knots-panel');
        const message = root?.querySelector('.knots-inapplicable');
        return {
            before,
            afterManualUpdate: api.updateCount,
            applicability: panel?.dataset.applicability ?? null,
            messageStatus: message?.dataset.applicability ?? null,
            message: message?.textContent?.replace(/\s+/g, ' ').trim() ?? null,
            messageVisible: !!message && !message.hidden,
            contentHidden: !!root?.querySelector('.knots-applicable-content')?.hidden,
            controlsDisabled: [...(root?.querySelectorAll('.knots-applicable-content input, .knots-applicable-content button') || [])]
                .every((control) => control.disabled),
            preference: state.knotTracking,
            scenarioApplicable: state.knotTrackingApplicable,
            effective: isKnotTrackingActive(state),
            knotZonesRequested: state.knotZonesRequested,
            knotZonesApplicable: state.knotZonesApplicable,
            knotZonesEffective: isKnotZonesActive(state),
            knotZones: state.fieldFlags.showKnotZones,
            coordinatorActive: api.coordinatorActive,
            measurementActive: api.measurementActive,
            contributionEnabled: api.contributionEnabled,
            wantAudit: getScale0TelemetryDemand(window.__ftdCtx, state).wantAudit,
            historyLength: api.historyLength,
            trackerCounts: ['e', 'b', 'flux'].map((field) => getFieldLineKnotTracker(field).getTelemetry().count),
            subscriberPresent: [...(window.__ftdRAF?._subs?.keys?.() || [])]
                .some((id) => id.startsWith('knots-panel')),
        };
    });

    expect(intentBoundary).toMatchObject({
        applicability: 'inapplicable-empty',
        messageStatus: 'inapplicable',
        messageVisible: true,
        contentHidden: true,
        controlsDisabled: true,
        preference: true,
        scenarioApplicable: false,
        effective: false,
        knotZonesRequested: true,
        knotZonesApplicable: false,
        knotZonesEffective: false,
        knotZones: false,
        coordinatorActive: false,
        measurementActive: false,
        contributionEnabled: false,
        wantAudit: false,
        historyLength: 0,
        trackerCounts: [0, 0, 0],
        subscriberPresent: false,
    });
    expect(intentBoundary.afterManualUpdate, 'manual update remains inert after Empty intent')
        .toBe(intentBoundary.before);
    expect(intentBoundary.message).toContain('No field extraction, RK4 line integration');
    expect(intentBoundary.message).toContain('not evidence for physical vacuum or topological triviality');

    await expect.poll(() => page.evaluate(async () => {
        const { getScale0State } = await import('/js/scales/scale0/state/store.js');
        return getScale0State().currentScenarioId;
    }), { timeout: 90_000 }).toBe(EMPTY);
    const frozenCount = await page.evaluate(() => window.__ftdKnotsPanel?.updateCount ?? null);
    await page.waitForTimeout(1_000);
    expect(await page.evaluate(() => window.__ftdKnotsPanel?.updateCount ?? null),
        'Empty performs no periodic knot-panel pass').toBe(frozenCount);

    await selectScale0Scenario(page, 'flux-pulse', { settleMs: 750 });
    await expect.poll(() => page.evaluate(async () => {
        const {
            getScale0State,
            isKnotTrackingActive,
            isKnotZonesActive,
        } = await import('/js/scales/scale0/state/store.js');
        const state = getScale0State();
        return {
            applicability: window.__ftdKnotsPanel?.applicability ?? null,
            preference: state.knotTracking,
            scenarioApplicable: state.knotTrackingApplicable,
            effective: isKnotTrackingActive(state),
            coordinatorActive: window.__ftdKnotsPanel?.coordinatorActive ?? null,
            measurementActive: window.__ftdKnotsPanel?.measurementActive ?? null,
            knotZonesRequested: state.knotZonesRequested,
            knotZonesApplicable: state.knotZonesApplicable,
            knotZonesEffective: isKnotZonesActive(state),
            knotZonesRendered: state.fieldFlags.showKnotZones,
        };
    }), { timeout: 15_000 }).toEqual({
        applicability: 'applicable',
        preference: true,
        scenarioApplicable: true,
        effective: true,
        coordinatorActive: true,
        measurementActive: true,
        knotZonesRequested: true,
        knotZonesApplicable: true,
        knotZonesEffective: true,
        knotZonesRendered: true,
    });

    // The runtime rule must remain fail-closed without a panel instance. A
    // headless or temporarily unmounted Empty load cannot depend on UI code to
    // suppress retained tracking preference.
    await page.evaluate(() => window.__ftdKnotsPanel?.dispose?.());
    await selectScale0Scenario(page, EMPTY, { settleMs: 0 });
    await expect.poll(() => page.evaluate(async () => {
        const [
            { getScale0State, isKnotTrackingActive, isKnotZonesActive },
            { getScale0TelemetryDemand },
        ] = await Promise.all([
            import('/js/scales/scale0/state/store.js'),
            import('/js/telemetry/demand.js'),
        ]);
        const state = getScale0State();
        const knotMesh = window.__ftdCtx?.viewport?._fieldRenderer?._knotZones ?? null;
        return {
            current: state.currentScenarioId,
            preference: state.knotTracking,
            effective: isKnotTrackingActive(state),
            knotZonesRequested: state.knotZonesRequested,
            knotZonesEffective: isKnotZonesActive(state),
            knotZonesRendered: state.fieldFlags.showKnotZones,
            knotMeshVisible: knotMesh?.visible ?? false,
            knotMeshDrawCount: knotMesh?.geometry?.drawRange?.count ?? null,
            panelPresent: !!document.getElementById('knots-panel'),
            schedulerActive: !!state.overlaySched?.active,
            wantAudit: getScale0TelemetryDemand(window.__ftdCtx, state).wantAudit,
        };
    }), { timeout: 90_000 }).toEqual({
        current: EMPTY,
        preference: true,
        effective: false,
        knotZonesRequested: true,
        knotZonesEffective: false,
        knotZonesRendered: false,
        knotMeshVisible: false,
        knotMeshDrawCount: 0,
        panelPresent: false,
        schedulerActive: false,
        wantAudit: false,
    });

    await selectScale0Scenario(page, 'flux-pulse', { settleMs: 0 });
    await expect.poll(() => page.evaluate(async () => {
        const {
            getScale0State,
            isKnotTrackingActive,
            isKnotZonesActive,
        } = await import('/js/scales/scale0/state/store.js');
        const state = getScale0State();
        const knotMesh = window.__ftdCtx?.viewport?._fieldRenderer?._knotZones ?? null;
        return {
            current: state.currentScenarioId,
            trackingPreference: state.knotTracking,
            trackingEffective: isKnotTrackingActive(state),
            knotZonesRequested: state.knotZonesRequested,
            knotZonesEffective: isKnotZonesActive(state),
            knotZonesRendered: state.fieldFlags.showKnotZones,
            knotMeshVisible: knotMesh?.visible ?? false,
            knotMeshDrawCount: knotMesh?.geometry?.drawRange?.count ?? null,
            panelPresent: !!document.getElementById('knots-panel'),
        };
    }), { timeout: 90_000 }).toEqual({
        current: 'flux-pulse',
        trackingPreference: true,
        trackingEffective: true,
        knotZonesRequested: true,
        knotZonesEffective: true,
        knotZonesRendered: true,
        knotMeshVisible: true,
        knotMeshDrawCount: expect.any(Number),
        panelPresent: false,
    });
    expect(realErrors(consoleErrors)).toEqual([]);
});

test('distinguishes an unresolved Diagnostics source from a measured exact zero', async ({ page }) => {
    test.setTimeout(120_000);
    await gotoAndReady(page, { path: '/?engine=wasm', timeout: 90_000 });
    const result = await page.evaluate(async () => {
        const [
            { DiagnosticsTable },
            { formatValue },
            { sections: scale0Sections },
            { resolveChartTelemetryGroups, getChartFreshnessPresentation },
        ] = await Promise.all([
            import('/js/ui/panels/diagnostics-panel/table.js'),
            import('/js/ui/panels/diagnostics-panel/formatters.js'),
            import('/js/ui/panels/diagnostics-panel/descriptors/scale0.js'),
            import('/js/ui/panels/charts-panel/chart-card.js'),
        ]);
        const hub = { measured: { exactZero: 0 }, unavailable: {} };
        const table = new DiagnosticsTable({
            id: 'availability-contract',
            title: 'Availability Contract',
            variant: 'static',
            rows: [
                { id: 'missing', label: 'Missing', source: 'unavailable.notPublished' },
                { id: 'zero', label: 'Zero', source: 'measured.exactZero' },
            ],
        }, hub);
        document.body.appendChild(table.el);
        table.update();
        const values = Object.fromEntries([...table.el.querySelectorAll('.diag-data-row')]
            .map((row) => [row.dataset.row, row.querySelector('.diag-value')?.textContent]));
        table.destroy();

        const groupMeta = {
            diagnostics: { source: 'test', sourceEpoch: 1, tick: null, stale: false },
            audit: { source: 'test', sourceEpoch: 1, tick: 7, stale: false },
        };
        const provenanceHub = {
            s0: {
                diag: { tick: 7, manifested: 0 },
                audit: { ELTotal: 0, ERTotal: 0 },
            },
            getResetVersion: () => 0,
            getScale0TelemetryMeta: (group) => groupMeta[group] ?? null,
        };
        const particle = scale0Sections.find((section) => section.id === 'particle-state');
        const particleTable = new DiagnosticsTable(particle, provenanceHub, { resetScope: 0 });
        document.body.appendChild(particleTable.el);
        particleTable.update();
        const unresolvedTick = particleTable.el
            .querySelector('[data-row="manifested"] .diag-value')?.textContent;
        groupMeta.diagnostics = { ...groupMeta.diagnostics, tick: 7 };
        particleTable.update();
        const measuredTickZero = particleTable.el
            .querySelector('[data-row="manifested"] .diag-value')?.textContent;
        particleTable.destroy();

        const dual = scale0Sections.find((section) => section.id === 'dual-substrate');
        const dualTable = new DiagnosticsTable(dual, provenanceHub, { resetScope: 0 });
        document.body.appendChild(dualTable.el);
        dualTable.update();
        const currentAuditZero = dualTable.el
            .querySelector('[data-row="e-left"] .diag-value')?.textContent;
        groupMeta.audit = { ...groupMeta.audit, tick: null };
        dualTable.update();
        const unresolvedAudit = dualTable.el
            .querySelector('[data-row="e-left"] .diag-value')?.textContent;
        dualTable.destroy();

        const singletonGroups = resolveChartTelemetryGroups({ telemetryGroup: 'audit' });
        const mixedGroups = resolveChartTelemetryGroups({
            telemetryGroups: ['diagnostics', 'audit', 'diagnostics'],
        });
        const chartWaiting = getChartFreshnessPresentation(provenanceHub, mixedGroups);
        groupMeta.audit = { ...groupMeta.audit, tick: 5 };
        const chartMixed = getChartFreshnessPresentation(provenanceHub, mixedGroups);
        groupMeta.audit = { ...groupMeta.audit, tick: 7 };
        const chartCurrent = getChartFreshnessPresentation(provenanceHub, mixedGroups);
        return {
            values,
            scalarMissing: formatValue(undefined),
            scalarNull: formatValue(null),
            scalarZero: formatValue(0),
            pair: formatValue([undefined, 0], { kind: 'pair' }),
            booleanMissing: formatValue(undefined, { kind: 'boolean' }),
            booleanFalse: formatValue(false, { kind: 'boolean' }),
            unresolvedTick,
            measuredTickZero,
            currentAuditZero,
            unresolvedAudit,
            singletonGroups,
            mixedGroups,
            chartWaiting,
            chartMixed,
            chartCurrent,
        };
    });

    expect(result).toEqual({
        values: { missing: '—', zero: '0' },
        scalarMissing: '—',
        scalarNull: '—',
        scalarZero: '0',
        pair: '— / 0',
        booleanMissing: '—',
        booleanFalse: 'off',
        unresolvedTick: '—',
        measuredTickZero: '0',
        currentAuditZero: '0',
        unresolvedAudit: '—',
        singletonGroups: ['audit'],
        mixedGroups: ['diagnostics', 'audit'],
        chartWaiting: {
            state: 'mixed-waiting',
            text: 'state t7 · audit waiting',
        },
        chartMixed: {
            state: 'mixed',
            text: 'state t7 · audit t5',
        },
        chartCurrent: {
            state: 'current',
            text: 'state t7 · audit t7',
        },
    });
});

test('Flux Slice restores its nonempty domain and rejects rapid stale-generation reactivation', async ({ page }) => {
    test.skip(!CAMPAIGN_PANELS.includes('flux-slice'), 'Flux Slice is outside this focused run');
    test.setTimeout(120_000);
    const consoleErrors = attachConsoleWatcher(page);
    await gotoAndReady(page, { path: '/?engine=wasm', timeout: 90_000 });

    const readPanel = () => page.evaluate(() => {
        const root = document.getElementById('panel-flux-slice');
        const panel = window.__ftdFluxSlicePanel;
        return {
            status: root?.querySelector('#flux-slice-panel')?.dataset.applicability ?? null,
            rows: root?.querySelectorAll('.flux-slice-row').length ?? 0,
            canvases: root?.querySelectorAll('canvas').length ?? 0,
            hasMessage: !!root?.querySelector('.flux-slice-inapplicable'),
            subscribed: !!panel?._sub,
            wanted: panel?._prevWantedKeys?.size ?? 0,
        };
    });

    const initial = await readPanel();
    expect(initial.rows).toBeGreaterThan(0);
    expect(initial.canvases).toBe(initial.rows * 3);

    await selectScale0Scenario(page, EMPTY, { settleMs: 0 });
    await expect.poll(readPanel).toMatchObject({
        status: 'inapplicable-empty', rows: 0, canvases: 0,
        hasMessage: true, subscribed: false, wanted: 0,
    });

    await selectScale0Scenario(page, 'flux-pulse', { settleMs: 0 });
    await expect.poll(readPanel).toMatchObject({
        status: 'applicable', rows: initial.rows, canvases: initial.canvases,
        hasMessage: false, subscribed: true,
    });

    await page.evaluate(() => {
        const select = document.getElementById('scenario-select');
        for (const scenarioId of ['empty', 'flux-pulse', 'empty']) {
            select.value = scenarioId;
            select.dispatchEvent(new Event('change', { bubbles: true }));
        }
    });
    await expect.poll(readPanel).toMatchObject({
        status: 'inapplicable-empty', rows: 0, canvases: 0,
        hasMessage: true, subscribed: false, wanted: 0,
    });
    expect(realErrors(consoleErrors)).toEqual([]);
});

test('Spectrum restores live nonempty analysis and rejects rapid stale-generation reactivation', async ({ page }) => {
    test.skip(!CAMPAIGN_PANELS.includes('spectrum'), 'Spectrum is outside this focused run');
    test.setTimeout(120_000);
    const consoleErrors = attachConsoleWatcher(page);
    await gotoAndReady(page, { path: '/?engine=wasm', timeout: 90_000 });

    const readPanel = () => page.evaluate(() => {
        const root = document.getElementById('spectrum-panel');
        const api = window.__ftdSpectrumPanel;
        return {
            status: root?.dataset.applicability ?? null,
            messageVisible: !!root?.querySelector('.spectrum-inapplicable:not([hidden])'),
            contentHidden: !!root?.querySelector('.spectrum-applicable-content')?.hidden,
            coordinatorActive: api?.coordinatorActive ?? null,
            samplerWantsActive: api?.samplerWantsActive ?? null,
            hasSpectrum: !!api?.lastSpec,
        };
    });

    expect(await readPanel()).toMatchObject({
        status: 'applicable', messageVisible: false, contentHidden: false,
        coordinatorActive: true,
    });

    await selectScale0Scenario(page, EMPTY, { settleMs: 0 });
    await page.evaluate(() => {
        // Public/manual entry points must remain inert as well as the removed
        // coordinator callback.
        window.__ftdSpectrumPanel?.update();
        window.__ftdSpectrumPanel?.deepMeasure();
    });
    await expect.poll(readPanel).toMatchObject({
        status: 'inapplicable-empty', messageVisible: true, contentHidden: true,
        coordinatorActive: false, samplerWantsActive: false, hasSpectrum: false,
    });

    await selectScale0Scenario(page, 'flux-pulse', { settleMs: 0 });
    await page.evaluate(() => {
        document.querySelector('#tab-bar .tab[data-panel="spectrum"]')?.click();
        const play = document.getElementById('btn-play');
        if (play?.getAttribute('data-paused') === 'true') play.click();
    });
    await expect.poll(readPanel).toMatchObject({
        status: 'applicable', messageVisible: false, contentHidden: false,
        coordinatorActive: true,
    });
    await expect.poll(async () => page.evaluate(() => {
        const spectrum = window.__ftdSpectrumPanel?.lastSpec;
        return spectrum?.M === 32 && spectrum.spec?.E?.some((value) => value > 0);
    }), { timeout: 15_000, message: 'restored live spectrum did not repopulate' }).toBe(true);

    await page.evaluate(() => {
        const select = document.getElementById('scenario-select');
        for (const scenarioId of ['empty', 'flux-pulse', 'empty']) {
            select.value = scenarioId;
            select.dispatchEvent(new Event('change', { bubbles: true }));
        }
    });
    await expect.poll(readPanel).toMatchObject({
        status: 'inapplicable-empty', messageVisible: true, contentHidden: true,
        coordinatorActive: false, samplerWantsActive: false, hasSpectrum: false,
    });
    expect(realErrors(consoleErrors)).toEqual([]);
});

test('Gravity restores nonempty analysis and rejects rapid stale-generation reactivation', async ({ page }) => {
    test.skip(!CAMPAIGN_PANELS.includes('gravity'), 'Gravity is outside this focused run');
    test.setTimeout(300_000);
    const consoleErrors = attachConsoleWatcher(page);
    await gotoAndReady(page, { path: '/?engine=wasm', timeout: 90_000 });

    const readPanel = () => page.evaluate(async () => {
        const [{ getScale0QualificationState, getScale0State }, { getScale0TelemetryDemand }] = await Promise.all([
            import('/js/scales/scale0/state/store.js'),
            import('/js/telemetry/demand.js'),
        ]);
        const root = document.getElementById('gravity-panel');
        const api = window.__ftdGravityPanel;
        const state = getScale0State();
        const qualification = getScale0QualificationState();
        const demand = getScale0TelemetryDemand(window.__ftdCtx, state);
        return {
            status: root?.dataset.applicability ?? null,
            messageVisible: !!root?.querySelector('.gravity-inapplicable:not([hidden])'),
            pendingVisible: !!root?.querySelector('.gravity-pending:not([hidden])'),
            contentHidden: !!root?.querySelector('.gravity-applicable-content')?.hidden,
            controlsDisabled: [...(root?.querySelectorAll('.grav-qbtn') || [])]
                .every((control) => control.disabled),
            coordinatorActive: api?.coordinatorActive ?? null,
            armCoordinatorActive: api?.armCoordinatorActive ?? null,
            liveCoordinatorActive: api?.liveCoordinatorActive ?? null,
            samplerWantsActive: api?.samplerWantsActive ?? null,
            wantGravity: demand.wantGravity,
            hasMetrics: !!api?.lastMetrics,
            historyLength: api?.historyLength ?? null,
            authoritativeGenerationReady: api?.authoritativeGenerationReady ?? null,
            qualifiedLoadGeneration: api?.qualifiedLoadGeneration ?? null,
            qualificationStatus: qualification.status,
            loadStatus: qualification.authoritativeLoad?.status ?? null,
            loadGeneration: qualification.authoritativeLoad?.loadGeneration ?? null,
            anchorGeneration: qualification.anchor?.loadGeneration ?? null,
            currentGeneration: Number(window.__ftdCtx?._loadGeneration ?? -1),
        };
    });

    await expect.poll(readPanel, { timeout: 90_000 }).toMatchObject({
        status: 'applicable', messageVisible: false, contentHidden: false,
        coordinatorActive: true, armCoordinatorActive: true,
        liveCoordinatorActive: false, samplerWantsActive: false,
        wantGravity: false, authoritativeGenerationReady: true,
    });

    await page.evaluate(() => window.__ftdCtx?.appShell?.panelDock?.activate('gravity'));
    await expect.poll(readPanel).toMatchObject({
        status: 'applicable', coordinatorActive: true,
        armCoordinatorActive: false, liveCoordinatorActive: true,
        samplerWantsActive: true, wantGravity: true,
    });

    const hideGravity = (action) => page.evaluate(async (requestedAction) => {
        const [{ getScale0State }, { getScale0TelemetryDemand }] = await Promise.all([
            import('/js/scales/scale0/state/store.js'),
            import('/js/telemetry/demand.js'),
        ]);
        const dock = window.__ftdCtx?.appShell?.panelDock;
        const api = window.__ftdGravityPanel;
        if (!dock || !api) throw new Error('Gravity visibility boundary unavailable');
        if (requestedAction === 'collapse') dock.setCollapsed(true);
        else if (requestedAction === 'tab') dock.activate('controls');
        else throw new Error(`Unknown Gravity hide action: ${requestedAction}`);
        return {
            armCoordinatorActive: api.armCoordinatorActive,
            liveCoordinatorActive: api.liveCoordinatorActive,
            samplerWantsActive: api.samplerWantsActive,
            wantGravity: getScale0TelemetryDemand(window.__ftdCtx, getScale0State()).wantGravity,
        };
    }, action);

    expect(await hideGravity('collapse')).toEqual({
        armCoordinatorActive: true,
        liveCoordinatorActive: false,
        samplerWantsActive: false,
        wantGravity: false,
    });
    await page.evaluate(() => window.__ftdCtx?.appShell?.panelDock?.setCollapsed(false));
    await expect.poll(readPanel).toMatchObject({
        armCoordinatorActive: false, liveCoordinatorActive: true,
        samplerWantsActive: true, wantGravity: true,
    });

    expect(await hideGravity('tab')).toEqual({
        armCoordinatorActive: true,
        liveCoordinatorActive: false,
        samplerWantsActive: false,
        wantGravity: false,
    });
    await page.evaluate(() => window.__ftdCtx?.appShell?.panelDock?.activate('gravity'));
    await expect.poll(readPanel).toMatchObject({
        armCoordinatorActive: false, liveCoordinatorActive: true,
        samplerWantsActive: true, wantGravity: true,
    });

    await page.evaluate(() => {
        const dock = window.__ftdCtx?.appShell?.panelDock;
        const floating = dock?.floatPanel('gravity', 420, 120);
        if (!floating) throw new Error('Gravity floating window unavailable');
    });
    await expect.poll(readPanel).toMatchObject({
        armCoordinatorActive: false, liveCoordinatorActive: true,
        samplerWantsActive: true, wantGravity: true,
    });
    expect(await page.evaluate(async () => {
        const [{ getScale0State }, { getScale0TelemetryDemand }] = await Promise.all([
            import('/js/scales/scale0/state/store.js'),
            import('/js/telemetry/demand.js'),
        ]);
        const floating = window.__ftdCtx?.appShell?.panelDock
            ? (await import('/js/ui/components/floating-window/component.js')).floatingWindowManager
                .getWindow('gravity')
            : null;
        floating?.toggleCollapse();
        const api = window.__ftdGravityPanel;
        return {
            armCoordinatorActive: api?.armCoordinatorActive ?? null,
            liveCoordinatorActive: api?.liveCoordinatorActive ?? null,
            samplerWantsActive: api?.samplerWantsActive ?? null,
            wantGravity: getScale0TelemetryDemand(window.__ftdCtx, getScale0State()).wantGravity,
        };
    })).toEqual({
        armCoordinatorActive: true,
        liveCoordinatorActive: false,
        samplerWantsActive: false,
        wantGravity: false,
    });
    await page.evaluate(async () => {
        const { floatingWindowManager } = await import('/js/ui/components/floating-window/component.js');
        floatingWindowManager.getWindow('gravity')?.toggleCollapse();
    });
    await expect.poll(readPanel).toMatchObject({
        armCoordinatorActive: false, liveCoordinatorActive: true,
        samplerWantsActive: true, wantGravity: true,
    });
    await page.evaluate(async () => {
        const { floatingWindowManager } = await import('/js/ui/components/floating-window/component.js');
        floatingWindowManager.getWindow('gravity')?.dock();
    });
    await expect.poll(readPanel).toMatchObject({
        armCoordinatorActive: false, liveCoordinatorActive: true,
        samplerWantsActive: true, wantGravity: true,
    });

    await selectScale0Scenario(page, EMPTY, { settleMs: 0 });
    await page.evaluate(() => {
        // Public/manual entry points must be as inert as the removed arm/live
        // coordinator paths; quantity changes must not trigger slice work.
        window.__ftdGravityPanel?.update();
        window.__ftdGravityPanel?.setKind('force');
    });
    await expect.poll(readPanel).toMatchObject({
        status: 'inapplicable-empty', messageVisible: true, contentHidden: true,
        controlsDisabled: true, coordinatorActive: false,
        armCoordinatorActive: false, liveCoordinatorActive: false,
        samplerWantsActive: false, wantGravity: false,
        hasMetrics: false, historyLength: 0,
    });

    await page.evaluate(() => window.__ftdCtx?.appShell?.panelDock?.activate('time'));
    expect((await readPanel()).wantGravity,
        'visible Time must not request an undefined gravity aggregate on Empty').toBe(false);

    await selectScale0Scenario(page, 's0-seed-gravitational-wave', { settleMs: 0 });
    await page.evaluate(() => {
        document.querySelector('#tab-bar .tab[data-panel="gravity"]')?.click();
        const play = document.getElementById('btn-play');
        if (play?.getAttribute('data-paused') === 'true') play.click();
    });
    await expect.poll(readPanel).toMatchObject({
        status: 'applicable', messageVisible: false, contentHidden: false,
        controlsDisabled: false, coordinatorActive: true,
    });
    await expect.poll(async () => page.evaluate(() => (
        (window.__ftdGravityPanel?.lastMetrics?.L?.max || 0) > 0
    )), { timeout: 15_000, message: 'restored gravity proxy did not repopulate' }).toBe(true);

    const readImmediateLoadBoundary = (action) => page.evaluate(async (requestedAction) => {
        const [{ getScale0QualificationState, getScale0State }, { getScale0TelemetryDemand }] = await Promise.all([
            import('/js/scales/scale0/state/store.js'),
            import('/js/telemetry/demand.js'),
        ]);
        if (requestedAction === 'reset') {
            document.getElementById('btn-reset')?.click();
        } else if (requestedAction === 'resize') {
            const select = document.getElementById('lattice-size');
            const target = [...select.options]
                .map((option) => ({ value: option.value, size: Number(option.value), disabled: option.disabled }))
                .find((option) => !option.disabled && option.size !== Number(select.value));
            if (!target) throw new Error('No alternate supported lattice size');
            select.value = target.value;
            select.dispatchEvent(new Event('change', { bubbles: true }));
        } else {
            throw new Error(`Unknown authoritative boundary action: ${requestedAction}`);
        }
        const state = getScale0State();
        const qualification = getScale0QualificationState();
        const api = window.__ftdGravityPanel;
        return {
            requestedAction,
            latticeSize: Number(document.getElementById('lattice-size')?.value),
            status: api?.applicability ?? null,
            pendingVisible: !!api?.element?.querySelector('.gravity-pending:not([hidden])'),
            coordinatorActive: api?.coordinatorActive ?? null,
            samplerWantsActive: api?.samplerWantsActive ?? null,
            wantGravity: getScale0TelemetryDemand(window.__ftdCtx, state).wantGravity,
            hasMetrics: !!api?.lastMetrics,
            historyLength: api?.historyLength ?? null,
            qualificationStatus: qualification.status,
            loadStatus: qualification.authoritativeLoad?.status ?? null,
            loadGeneration: qualification.authoritativeLoad?.loadGeneration ?? null,
        };
    }, action);

    const resetBoundary = await readImmediateLoadBoundary('reset');
    expect(resetBoundary).toMatchObject({
        status: 'pending-load', pendingVisible: true,
        coordinatorActive: false, samplerWantsActive: false, wantGravity: false,
        hasMetrics: false, historyLength: 0,
        qualificationStatus: 'pending', loadStatus: 'pending',
    });
    expect(resetBoundary.loadGeneration).toBeGreaterThan(0);
    await expect.poll(readPanel, { timeout: 90_000 }).toMatchObject({
        status: 'applicable', pendingVisible: false,
        liveCoordinatorActive: true, samplerWantsActive: true, wantGravity: true,
        authoritativeGenerationReady: true,
        qualificationStatus: 'within-contract', loadStatus: null,
    });
    expect((await readPanel()).qualifiedLoadGeneration).toBe((await readPanel()).anchorGeneration);

    const resizeBoundary = await readImmediateLoadBoundary('resize');
    expect(resizeBoundary).toMatchObject({
        status: 'pending-load', pendingVisible: true,
        coordinatorActive: false, samplerWantsActive: false, wantGravity: false,
        hasMetrics: false, historyLength: 0,
        qualificationStatus: 'pending', loadStatus: 'pending',
    });
    await expect.poll(readPanel, { timeout: 90_000 }).toMatchObject({
        status: 'applicable', pendingVisible: false,
        liveCoordinatorActive: true, samplerWantsActive: true, wantGravity: true,
        authoritativeGenerationReady: true,
        qualificationStatus: 'within-contract', loadStatus: null,
        qualifiedLoadGeneration: resizeBoundary.loadGeneration,
        anchorGeneration: resizeBoundary.loadGeneration,
    });

    for (let cycle = 0; cycle < 2; cycle += 1) {
        const before = await page.evaluate(() => {
            window.__gravityPanelBeforeScaleSwitch = window.__ftdGravityPanel;
            return {
                connected: !!window.__ftdGravityPanel?.element?.isConnected,
                registryCurrent: window.__FTD_DEV__?.panels?.gravity === window.__ftdGravityPanel,
            };
        });
        expect(before).toEqual({ connected: true, registryCurrent: true });

        await page.selectOption('#engine-mode', 'particles');
        await expect.poll(() => page.evaluate(() => document.getElementById('app')?.classList.contains('mode-particles')))
            .toBe(true);
        expect(await page.evaluate(() => ({
            oldConnected: !!window.__gravityPanelBeforeScaleSwitch?.element?.isConnected,
            oldCoordinatorActive: window.__gravityPanelBeforeScaleSwitch?.coordinatorActive ?? null,
            oldSamplerWantsActive: window.__gravityPanelBeforeScaleSwitch?.samplerWantsActive ?? null,
            globalCleared: window.__ftdGravityPanel == null,
            registryCleared: window.__FTD_DEV__?.panels?.gravity == null,
        }))).toEqual({
            oldConnected: false, oldCoordinatorActive: false,
            oldSamplerWantsActive: false, globalCleared: true, registryCleared: true,
        });

        await page.selectOption('#engine-mode', 'lattice');
        await expect.poll(readPanel, { timeout: 90_000 }).toMatchObject({
            status: 'applicable', authoritativeGenerationReady: true,
            qualificationStatus: 'within-contract', loadStatus: null,
        });
        expect(await page.evaluate(() => ({
            newInstance: window.__ftdGravityPanel !== window.__gravityPanelBeforeScaleSwitch,
            connected: !!window.__ftdGravityPanel?.element?.isConnected,
            registryCurrent: window.__FTD_DEV__?.panels?.gravity === window.__ftdGravityPanel,
        }))).toEqual({ newInstance: true, connected: true, registryCurrent: true });
    }
    await page.evaluate(() => { delete window.__gravityPanelBeforeScaleSwitch; });

    await page.evaluate(() => {
        const select = document.getElementById('scenario-select');
        for (const scenarioId of ['empty', 's0-seed-gravitational-wave', 'empty']) {
            select.value = scenarioId;
            select.dispatchEvent(new Event('change', { bubbles: true }));
        }
    });
    await expect.poll(readPanel).toMatchObject({
        status: 'inapplicable-empty', messageVisible: true, contentHidden: true,
        controlsDisabled: true, coordinatorActive: false,
        samplerWantsActive: false, hasMetrics: false, historyLength: 0,
    });
    expect(realErrors(consoleErrors)).toEqual([]);
});

test('Gravity waits for paused lazy samplers without fabricating zero telemetry', async ({ page }) => {
    test.skip(!CAMPAIGN_PANELS.includes('gravity'), 'Gravity is outside this focused run');
    test.setTimeout(180_000);
    const consoleErrors = attachConsoleWatcher(page);
    await gotoAndReady(page, { path: '/?engine=wasm', timeout: 90_000 });
    await page.evaluate(() => {
        const dock = window.__ftdCtx?.appShell?.panelDock;
        dock?.setCollapsed(false);
        dock?.activate('controls');
    });
    await selectScale0Scenario(page, 's0-seed-gravitational-wave', { settleMs: 0 });

    await expect.poll(async () => page.evaluate(async () => {
        const store = await import('/js/scales/scale0/state/store.js');
        const state = store.getScale0State();
        const owner = store.getActiveScale0Bridge(window.__ftdCtx, state);
        return state.useFluxMock === true
            && owner?.isWorker === true
            && owner.ready === true
            && store.isScale0AuthoritativeGenerationReady(state);
    }), { timeout: 90_000, message: 'qualified worker-backed source scenario did not load' }).toBe(true);
    await page.evaluate(() => window.__ftdCtx.appShell.panelDock.activate('gravity'));
    await expect.poll(() => page.evaluate(() => ({
        ready: window.__ftdGravityPanel?.telemetryState === 'ready',
        populated: (window.__ftdGravityPanel?.lastMetrics?.L?.max || 0) > 0,
    })), { timeout: 15_000, message: 'source scenario did not populate Gravity telemetry' })
        .toEqual({ ready: true, populated: true });
    const previousTelemetry = await page.evaluate(() => (
        document.querySelector('#gravity-panel-telemetry')?.textContent || ''
    ));
    expect(previousTelemetry).not.toContain('Waiting for current-generation gravity samplers');
    expect(previousTelemetry).toContain('sampled mean / max; h=2');
    expect(previousTelemetry).toContain('Engine force');
    expect(previousTelemetry).not.toContain('gravity PE');

    await page.evaluate(() => window.__ftdCtx.appShell.panelDock.activate('controls'));
    await selectScale0Scenario(page, 'flux-pulse', { settleMs: 0 });
    await expect.poll(async () => page.evaluate(async () => {
        const store = await import('/js/scales/scale0/state/store.js');
        const state = store.getScale0State();
        const owner = store.getActiveScale0Bridge(window.__ftdCtx, state);
        return state.useFluxMock === true
            && owner?.isWorker === true
            && owner.ready === true
            && state.currentScenarioId === 'flux-pulse'
            && store.isScale0AuthoritativeGenerationReady(state);
    }), { timeout: 90_000, message: 'qualified worker-backed target scenario did not load' }).toBe(true);

    await page.evaluate(async () => {
        const store = await import('/js/scales/scale0/state/store.js');
        const state = store.getScale0State();
        window.__ftdCtx.running = false;
        store.setScale0PlaybackRunning(window.__ftdCtx, false, state);
    });
    await expect.poll(async () => page.evaluate(async () => {
        const store = await import('/js/scales/scale0/state/store.js');
        const state = store.getScale0State();
        const owner = store.getActiveScale0Bridge(window.__ftdCtx, state);
        return window.__ftdCtx.running === false && owner?.runningStateSettled === true;
    }), { timeout: 15_000, message: 'worker did not settle into the paused state' }).toBe(true);

    const held = await page.evaluate(async () => {
        const store = await import('/js/scales/scale0/state/store.js');
        const state = store.getScale0State();
        const owner = store.getActiveScale0Bridge(window.__ftdCtx, state);
        const api = window.__ftdGravityPanel;
        const stride = api.telemetryStride;
        const targetKeys = [
            `latency@${stride}`,
            `kretschmann@${stride}`,
            `gravity@${stride}`,
            'gravityMetricAgg@0',
        ];
        for (const key of targetKeys) delete owner._samplerCache[key];

        const worker = owner._worker;
        const originalPostMessage = worker.postMessage;
        const blocked = [];
        worker.postMessage = function holdGravitySamplerWants(message, transfer) {
            const targetChanges = message?.type === 'replaceSamplerWants'
                ? (message.changes || []).filter((change) => (
                    change.op === 'want' && targetKeys.includes(`${change.kind}@${change.stride}`)
                ))
                : [];
            if (targetChanges.length) {
                blocked.push({
                    type: message.type,
                    changes: message.changes.map((change) => ({ ...change })),
                });
                return;
            }
            return transfer === undefined
                ? originalPostMessage.call(worker, message)
                : originalPostMessage.call(worker, message, transfer);
        };
        window.__gravitySamplerBarrier = {
            owner, worker, originalPostMessage, blocked, targetKeys,
            tick: owner.currentTick(),
            dataVersion: owner.dataVersion,
            frameCounter: owner.frameCounter,
            fieldDataVersion: state.fieldDataVersion,
        };

        window.__ftdCtx.appShell.panelDock.activate('gravity');
        api.update();
        return {
            blocked: blocked.flatMap((message) => message.changes)
                .filter((change) => change.op === 'want')
                .map((change) => `${change.kind}@${change.stride}`).sort(),
            blockedMessages: blocked.length,
            lastMetrics: api.lastMetrics,
            historyLength: api.historyLength,
            wants: api.samplerWantsActive,
            telemetryState: api.telemetryState,
            telemetryText: document.querySelector('#gravity-panel-telemetry')?.textContent || '',
            tileReadouts: [...document.querySelectorAll('.grav-tile-readout')]
                .map((node) => node.textContent),
            tick: owner.currentTick(),
            dataVersion: owner.dataVersion,
            frameCounter: owner.frameCounter,
            fieldDataVersion: state.fieldDataVersion,
        };
    });

    try {
        expect(held).toMatchObject({
            blocked: [
                'gravity@2',
                'gravityMetricAgg@0',
                'kretschmann@2',
                'latency@2',
            ],
            blockedMessages: 1,
            lastMetrics: null,
            historyLength: 0,
            wants: true,
            telemetryState: 'waiting-samplers',
            telemetryText: 'Waiting for current-generation gravity samplers…',
            tick: held.tick,
            dataVersion: held.dataVersion,
            frameCounter: held.frameCounter,
            fieldDataVersion: held.fieldDataVersion,
        });
        expect(held.tileReadouts.filter((value) => value !== '—').length,
            'only the current target volume may repaint one rotating plane while samplers are held')
            .toBeLessThanOrEqual(1);

        await page.evaluate(() => {
            const barrier = window.__gravitySamplerBarrier;
            barrier.worker.postMessage = barrier.originalPostMessage;
            for (const message of barrier.blocked) {
                barrier.originalPostMessage.call(barrier.worker, message);
            }
        });
        await expect.poll(() => page.evaluate(() => {
            const barrier = window.__gravitySamplerBarrier;
            return barrier.targetKeys.every((key) => {
                const at = key.lastIndexOf('@');
                return barrier.owner.hasSamplerSnapshot(
                    key.slice(0, at),
                    Number(key.slice(at + 1)),
                );
            });
        }), { timeout: 15_000, message: 'paused worker did not publish requested gravity samplers' }).toBe(true);

        const populated = await expect.poll(() => page.evaluate(async () => {
            const { getScale0State } = await import('/js/scales/scale0/state/store.js');
            const barrier = window.__gravitySamplerBarrier;
            const api = window.__ftdGravityPanel;
            return {
                hasMetrics: !!api.lastMetrics,
                telemetryState: api.telemetryState,
                historyLength: api.historyLength,
                latencyMax: api.lastMetrics?.L?.max || 0,
                kretschmannMax: api.lastMetrics?.K?.max || 0,
                tick: barrier.owner.currentTick(),
                dataVersion: barrier.owner.dataVersion,
                frameCounter: barrier.owner.frameCounter,
                fieldDataVersion: getScale0State().fieldDataVersion,
            };
        }), { timeout: 15_000, message: 'Gravity did not consume sampler-only paused readback' }).toMatchObject({
            hasMetrics: true,
            telemetryState: 'ready',
            historyLength: 1,
            tick: held.tick,
            dataVersion: held.dataVersion,
            frameCounter: held.frameCounter + 1,
            fieldDataVersion: held.fieldDataVersion,
        });
        void populated;
        const scientificValues = await page.evaluate(() => ({
            latencyMax: window.__ftdGravityPanel.lastMetrics.L.max,
            kretschmannMax: window.__ftdGravityPanel.lastMetrics.K.max,
        }));
        expect(scientificValues.latencyMax).toBeGreaterThan(0);
        expect(scientificValues.kretschmannMax).toBeGreaterThan(0);

        const collapsed = await page.evaluate(async () => {
            const [{ getScale0State }, { getScale0TelemetryDemand }] = await Promise.all([
                import('/js/scales/scale0/state/store.js'),
                import('/js/telemetry/demand.js'),
            ]);
            window.__ftdCtx.appShell.panelDock.setCollapsed(true);
            return {
                live: window.__ftdGravityPanel.liveCoordinatorActive,
                wants: window.__ftdGravityPanel.samplerWantsActive,
                wantGravity: getScale0TelemetryDemand(window.__ftdCtx, getScale0State()).wantGravity,
                historyLength: window.__ftdGravityPanel.historyLength,
            };
        });
        expect(collapsed).toEqual({ live: false, wants: false, wantGravity: false, historyLength: 1 });
    } finally {
        await page.evaluate(() => {
            const barrier = window.__gravitySamplerBarrier;
            if (barrier?.worker && barrier?.originalPostMessage) {
                barrier.worker.postMessage = barrier.originalPostMessage;
            }
            delete window.__gravitySamplerBarrier;
        });
    }
    expect(realErrors(consoleErrors)).toEqual([]);
});

test('Gravity live sidepanel sustains 60 Hz at the qualified backend limit and releases collapsed work', async ({ page }, testInfo) => {
    test.skip(!CAMPAIGN_PANELS.includes('gravity'), 'Gravity is outside this focused run');
    test.setTimeout(300_000);
    const consoleErrors = attachConsoleWatcher(page);
    const directWasm = process.env.FTD_GRAVITY_PANEL_BACKEND === 'direct-wasm';
    const gravityLatticeSize = Number(
        process.env.FTD_GRAVITY_PANEL_SIZE || (directWasm ? 33 : LATTICE_SIZE),
    );
    if (directWasm) {
        await page.addInitScript(() => { window.__ftdWasmWorker = false; });
    }
    await gotoAndReady(page, { path: '/?engine=wasm', timeout: 90_000 });

    if (directWasm) {
        const directLimits = await page.evaluate(() => Object.fromEntries(
            [33, 49, 65, 97].map((size) => {
                const option = [...document.querySelectorAll('#lattice-size option')]
                    .find((candidate) => Number(candidate.value) === size);
                return [size, { disabled: option?.disabled ?? null, label: option?.textContent ?? '' }];
            }),
        ));
        expect(directLimits[33].disabled).toBe(false);
        for (const size of [49, 65, 97]) {
            expect(directLimits[size].disabled).toBe(true);
            expect(directLimits[size].label).toContain('WASM worker / Native GPU');
        }
    }

    const workerSupported = await page.evaluate(() => globalThis.crossOriginIsolated === true
        && typeof SharedArrayBuffer !== 'undefined');
    if (!directWasm && !workerSupported) {
        testInfo.annotations.push({
            type: 'unsupported-path',
            description: 'Live L=97 Gravity performance requires the COOP/COEP WasmBridgeProxy path',
        });
        test.skip(true, 'WasmBridgeProxy is unavailable on this server/browser');
    }

    const sizeEnabled = await page.evaluate((size) => {
        const option = [...document.querySelectorAll('#lattice-size option')]
            .find((candidate) => Number(candidate.value) === size);
        return !!option && !option.disabled;
    }, gravityLatticeSize);
    if (!sizeEnabled) {
        testInfo.annotations.push({
            type: 'unsupported-path',
            description: `L=${gravityLatticeSize} is not enabled for the active browser backend`,
        });
        test.skip(true, `L=${gravityLatticeSize} is unavailable`);
    }

    await selectScale0Scenario(page, 's0-seed-gravitational-wave', { settleMs: 0 });
    await page.selectOption('#lattice-size', String(gravityLatticeSize));
    await expect.poll(() => page.evaluate(async ({ scenarioId, latticeSize, directWasm }) => {
        const { getScale0QualificationState, getScale0State,
            getActiveScale0Bridge, isScale0AuthoritativeGenerationReady } =
            await import('/js/scales/scale0/state/store.js');
        const state = getScale0State();
        const owner = getActiveScale0Bridge(window.__ftdCtx, state);
        const ownerReady = directWasm
            ? state.useFluxMock === false
                && owner?.isWasm === true
                && owner?.isWorker !== true
            : state.useFluxMock === true
                && owner?.isWorker === true
                && owner?.ready === true
                && owner?._scenarioId === scenarioId;
        return state.currentScenarioId === scenarioId
            && ownerReady
            && Number(owner.latticeSize) === latticeSize
            && isScale0AuthoritativeGenerationReady(getScale0QualificationState());
    }, { scenarioId: 's0-seed-gravitational-wave', latticeSize: gravityLatticeSize, directWasm }), {
        timeout: 90_000,
        message: `qualified gravitational-wave owner did not become ready at L=${gravityLatticeSize}`,
    }).toBe(true);

    await page.evaluate(() => {
        const dock = window.__ftdCtx?.appShell?.panelDock;
        if (!dock) throw new Error('Panel dock unavailable');
        dock.setCollapsed(false);
        dock.activate('gravity');
        const play = document.getElementById('btn-play');
        if (play?.getAttribute('data-paused') === 'true') play.click();
    });
    await expect.poll(() => page.evaluate(() => ({
        status: window.__ftdGravityPanel?.applicability ?? null,
        live: window.__ftdGravityPanel?.liveCoordinatorActive ?? null,
        wants: window.__ftdGravityPanel?.samplerWantsActive ?? null,
        hasMetrics: !!window.__ftdGravityPanel?.lastMetrics,
    })), { timeout: 30_000 }).toMatchObject({
        status: 'applicable', live: true, wants: true, hasMetrics: true,
    });
    await page.waitForTimeout(1_500);

    const live = await page.evaluate(async ({ visibleFrames }) => {
        const probe = await import('/tests/scale0-ui-audit-probe.js');
        const waitFrames = async (count) => {
            for (let i = 0; i < count; i += 1) {
                await new Promise((resolve) => requestAnimationFrame(resolve));
            }
        };
        probe.startScale0UiAuditProbe({
            rootSelector: '#panel-gravity',
            subscriberPrefixes: ['gravity-panel'],
        });
        await waitFrames(visibleFrames + 2);
        return probe.stopScale0UiAuditProbe();
    }, { visibleFrames: Math.max(GATES.visibleFrames, 720) });

    const collapsedBoundary = await page.evaluate(async () => {
        const [{ getScale0State }, { getScale0TelemetryDemand }] = await Promise.all([
            import('/js/scales/scale0/state/store.js'),
            import('/js/telemetry/demand.js'),
        ]);
        window.__ftdCtx?.appShell?.panelDock?.setCollapsed(true);
        const api = window.__ftdGravityPanel;
        return {
            arm: api?.armCoordinatorActive ?? null,
            live: api?.liveCoordinatorActive ?? null,
            wants: api?.samplerWantsActive ?? null,
            wantGravity: getScale0TelemetryDemand(window.__ftdCtx, getScale0State()).wantGravity,
        };
    });
    expect(collapsedBoundary).toEqual({ arm: true, live: false, wants: false, wantGravity: false });

    const collapsed = await page.evaluate(async ({ collapsedFrames }) => {
        const probe = await import('/tests/scale0-ui-audit-probe.js');
        const waitFrames = async (count) => {
            for (let i = 0; i < count; i += 1) {
                await new Promise((resolve) => requestAnimationFrame(resolve));
            }
        };
        probe.startScale0UiAuditProbe({
            rootSelector: '#panel-gravity',
            subscriberPrefixes: ['gravity-panel'],
        });
        await waitFrames(collapsedFrames + 2);
        return probe.stopScale0UiAuditProbe();
    }, { collapsedFrames: GATES.collapsedFrames });

    await testInfo.attach(`scale0-gravity-live-performance-L${gravityLatticeSize}.json`, {
        body: JSON.stringify({
            scenario: 's0-seed-gravitational-wave',
            backend: directWasm ? 'direct-wasm' : 'wasm-worker',
            latticeSize: gravityLatticeSize,
            viewportFluxVolume: 'default visible presentation',
            gates: GATES,
            collapsedBoundary,
            live,
            collapsed,
        }, null, 2),
        contentType: 'application/json',
    });

    expect(live.frames.count).toBeGreaterThanOrEqual(GATES.visibleFrames);
    expect(live.frames.effectiveFps, JSON.stringify({
        liveFrames: live.frames,
        liveCallbacks: live.callbacks,
        liveDom: live.dom,
        collapsedFrames: collapsed.frames,
        collapsedCallbacks: collapsed.callbacks,
    }, null, 2))
        .toBeGreaterThanOrEqual(GATES.minFps);
    expect(live.frames.p95Ms).toBeLessThanOrEqual(GATES.p95Ms);
    expect(live.frames.p99Ms).toBeLessThanOrEqual(GATES.p99Ms);
    expect(live.longTasks).toEqual([]);
    expect(callbackWorst(live, 'p99Ms')).toBeLessThanOrEqual(GATES.maxCallbackP99Ms);
    expect(live.dom.mutationRecords).toBeLessThanOrEqual(GATES.maxVisibleMutations);
    expect(live.dom.canvasDraws).toBeLessThanOrEqual(GATES.maxVisibleCanvasDraws);
    expect(live.errors).toEqual([]);
    if (live.resourcesStart.heapBytes && live.resourcesEnd.heapBytes) {
        expect(live.resourceDelta.heapBytes).toBeLessThanOrEqual(GATES.maxHeapGrowthBytes);
    }

    expect(collapsed.frames.count).toBeGreaterThanOrEqual(GATES.collapsedFrames);
    expect(collapsed.frames.effectiveFps).toBeGreaterThanOrEqual(GATES.minFps);
    expect(collapsed.frames.p95Ms).toBeLessThanOrEqual(GATES.p95Ms);
    expect(collapsed.frames.p99Ms).toBeLessThanOrEqual(GATES.p99Ms);
    expect(callbackWorst(collapsed, 'maxMs')).toBeLessThanOrEqual(GATES.maxCollapsedCallbackMs);
    expect(collapsed.dom.mutationRecords).toBe(0);
    expect(collapsed.dom.canvasDraws).toBe(0);
    expect(collapsed.longTasks).toEqual([]);
    expect(collapsed.errors).toEqual([]);
    expect(realErrors(consoleErrors)).toEqual([]);
});

test('all 17 visible panels sustain 60 Hz at empty L=97 and stop panel work when collapsed', async ({ page }, testInfo) => {
    test.setTimeout(600_000);
    const consoleErrors = attachConsoleWatcher(page);
    await gotoAndReady(page, { path: '/?engine=wasm', timeout: 90_000 });

    const nonemptyFluxSlice = CAMPAIGN_PANELS.includes('flux-slice')
        ? await page.evaluate(() => {
            const root = document.getElementById('panel-flux-slice');
            const panel = window.__ftdFluxSlicePanel;
            const rows = root?.querySelectorAll('.flux-slice-row').length ?? 0;
            const canvases = root?.querySelectorAll('.flux-slice-canvas').length ?? 0;
            return {
                scenarioId: document.getElementById('scenario-select')?.value ?? null,
                rows,
                canvases,
                applicable: panel?._emptyInapplicable === false,
            };
        })
        : null;

    const workerSupported = await page.evaluate(() => globalThis.crossOriginIsolated === true
        && typeof SharedArrayBuffer !== 'undefined');
    if (!workerSupported) {
        testInfo.annotations.push({
            type: 'unsupported-path',
            description: 'L=97 absolute panel campaign requires the COOP/COEP WasmBridgeProxy path',
        });
        test.skip(true, 'WasmBridgeProxy is unavailable on this server/browser');
    }

    await selectScale0Scenario(page, EMPTY, { settleMs: 0 });
    const sizeEnabled = await page.evaluate((size) => {
        const option = [...document.querySelectorAll('#lattice-size option')]
            .find((candidate) => Number(candidate.value) === size);
        return !!option && !option.disabled;
    }, LATTICE_SIZE);
    if (!sizeEnabled) {
        testInfo.annotations.push({
            type: 'unsupported-path',
            description: `L=${LATTICE_SIZE} is not enabled for the active browser backend`,
        });
        test.skip(true, `L=${LATTICE_SIZE} is unavailable`);
    }
    await page.selectOption('#lattice-size', String(LATTICE_SIZE));
    await waitForEmptyWorker(page);

    await page.evaluate(() => {
        const dock = window.__ftdCtx?.appShell?.panelDock;
        if (!dock) throw new Error('Panel dock unavailable');
        dock.setCollapsed(false);
        const play = document.getElementById('btn-play');
        if (play?.getAttribute('data-paused') === 'true') play.click();
    });

    const campaign = await page.evaluate(async ({ panelIds, gates }) => {
        const probe = await import('/tests/scale0-ui-audit-probe.js');
        const { getPanelsForScale } = await import('/js/ui/scale-registry/panel-registry.js');
        const dock = window.__ftdCtx?.appShell?.panelDock;
        if (!dock) throw new Error('Panel dock unavailable');

        const waitFrames = async (count) => {
            for (let i = 0; i < count; i += 1) {
                await new Promise((resolve) => requestAnimationFrame(resolve));
            }
        };
        const wait = (ms) => new Promise((resolve) => setTimeout(resolve, ms));
        const subscriberPrefix = (panelId) => `${panelId}-panel`;
        const rendererMemory = () => {
            const memory = window.__ftdCtx?.viewport?.renderer?.info?.memory;
            return memory && Number.isFinite(memory.geometries) && Number.isFinite(memory.textures)
                ? { geometries: memory.geometries, textures: memory.textures }
                : null;
        };
        const resourceSnapshot = (root) => ({
            rootDomNodes: root.querySelectorAll('*').length + 1,
            rootCanvases: root.querySelectorAll('canvas').length,
            coordinatorSubscribers: window.__ftdRAF?.size?.() ?? null,
            subscriberIds: [...(window.__ftdRAF?._subs?.keys?.() || [])],
            renderer: rendererMemory(),
            heapObserved: Number(performance.memory?.usedJSHeapSize) > 0,
        });

        const registryIds = getPanelsForScale(0).map((panel) => panel.id);
        const visibleTabIds = [...document.querySelectorAll('#tab-bar .tab[data-panel]')]
            .filter((tab) => getComputedStyle(tab).display !== 'none')
            .map((tab) => tab.dataset.panel);
        const results = [];

        for (const panelId of panelIds) {
            const result = { panelId, status: 'unsupported', reason: '', active: null, collapsed: null };
            let probeRunning = false;
            try {
                dock.setCollapsed(false);
                const tab = document.querySelector(`#tab-bar .tab[data-panel="${panelId}"]`);
                const root = document.getElementById(`panel-${panelId}`);
                if (!tab || !root) throw new Error('canonical tab or panel root is missing');
                tab.click();
                await waitFrames(4);
                // Low-rate panel coordinators arm at 1-2 Hz. Keep first-time
                // rendering and subscription setup outside the measurement.
                await wait(1_250);
                await waitFrames(4);

                const rect = root.getBoundingClientRect();
                const style = getComputedStyle(root);
                if (!root.classList.contains('active')
                    || style.display === 'none'
                    || style.visibility === 'hidden'
                    || rect.width <= 0
                    || rect.height <= 0) {
                    throw new Error('panel did not become visibly active');
                }

                if (panelId === 'diagnostics') {
                    const hosts = [...root.querySelectorAll('.diag-scale0-root .diag-spark-host')];
                    const first = hosts[0];
                    const last = hosts.at(-1);
                    last?.scrollIntoView({ block: 'center' });
                    await wait(100);
                    await waitFrames(3);
                    const bottomVisible = !!last?.querySelector('canvas');
                    const bottomCanvasCount = root.querySelectorAll('canvas').length;
                    first?.scrollIntoView({ block: 'center' });
                    await wait(100);
                    await waitFrames(3);
                    result.diagnosticsVirtualization = {
                        trendHosts: hosts.length,
                        bottomVisible,
                        bottomCanvasCount,
                        topVisible: !!first?.querySelector('canvas'),
                        topCanvasCount: root.querySelectorAll('canvas').length,
                    };
                }

                if (panelId === 'lagrangian') {
                    const [componentModule, hubModule, stateModule] = await Promise.all([
                        import('/js/ui/panels/lagrangian-panel/component.js'),
                        import('/js/telemetry-hub.js'),
                        import('/js/scales/scale0/state/store.js'),
                    ]);
                    const { interpretEmptyObserverBaseline } = componentModule;
                    const { telemetryHub } = hubModule;
                    const { getScale0State } = stateModule;
                    const missing = interpretEmptyObserverBaseline(null, {
                        scenarioId: 'empty', telemetryMeta: { stale: false, tick: 0 },
                    });
                    const exactZero = interpretEmptyObserverBaseline({
                        bornInfeld: 0, total: 0,
                    }, {
                        scenarioId: 'empty', telemetryMeta: { stale: false, tick: 0 },
                    });
                    const actual = interpretEmptyObserverBaseline(
                        telemetryHub.s0?.lagrangian,
                        {
                            scenarioId: getScale0State().currentScenarioId,
                            telemetryMeta: telemetryHub.getScale0TelemetryMeta('lagrangian'),
                        },
                    );
                    const observerCard = root.querySelector('.lag-observer-baseline');
                    const trendHosts = [...root.querySelectorAll('.lag-data-col .diag-spark-host')];
                    const firstTerm = root.querySelector('.lag-term-card');
                    const lastTrend = trendHosts.at(-1);
                    lastTrend?.scrollIntoView({ block: 'center' });
                    await wait(100);
                    await waitFrames(3);
                    const bottomTrendVisible = !!lastTrend?.querySelector('canvas');
                    const bottomCanvasCount = root.querySelectorAll('canvas').length;
                    firstTerm?.scrollIntoView({ block: 'center' });
                    await wait(100);
                    await waitFrames(3);
                    result.lagrangianAudit = {
                        checkedTerms: root.querySelectorAll('.lag-term-toggle input:checked').length,
                        termCharts: root.querySelectorAll('.lag-charts-grid .uplot').length,
                        trendHosts: trendHosts.length,
                        bottomTrendVisible,
                        bottomCanvasCount,
                        topCanvasCount: root.querySelectorAll('canvas').length,
                        status: observerCard?.dataset.lagObserverStatus ?? null,
                        baselineText: root.querySelector('[data-lag-observer-value]')?.textContent ?? null,
                        excitationText: root.querySelector('[data-lag-excitation-value]')?.textContent ?? null,
                        statusText: root.querySelector('[data-lag-observer-status-text]')?.textContent ?? null,
                        interpretationText: root.querySelector('.lag-observer-interpretation')?.textContent
                            ?.replace(/\s+/g, ' ').trim() ?? null,
                        bornInfeldLabel: root.querySelector('.lag-term-toggle[data-term="bornInfeld"] .lag-term-label')
                            ?.textContent ?? null,
                        baselineBearingActionLabels: [...root.querySelectorAll('.lag-data-col .diag-section[data-section="lag-action"] .diag-metric')]
                            .slice(0, 3).map((node) => node.textContent),
                        actual,
                        missing,
                        exactZero,
                    };
                }

                if (panelId === 'flux-slice') {
                    const panel = window.__ftdFluxSlicePanel;
                    const controls = [...root.querySelectorAll(
                        '.flux-slice-axis-btn, .flux-slice-expand, .flux-slice-reset-mirror',
                    )];
                    result.fluxSliceApplicability = {
                        panelStatus: root.querySelector('#flux-slice-panel')?.dataset.applicability ?? null,
                        messageStatus: root.querySelector('.flux-slice-inapplicable')
                            ?.dataset.applicability ?? null,
                        message: root.querySelector('.flux-slice-inapplicable')?.textContent
                            ?.replace(/\s+/g, ' ').trim() ?? null,
                        rowCount: root.querySelectorAll('.flux-slice-row').length,
                        canvasCount: root.querySelectorAll('canvas').length,
                        controlsDisabled: controls.length > 0
                            && controls.every((control) => control.disabled),
                        selfDriveStopped: !panel?._sub,
                        wantedSamplerCount: panel?._prevWantedKeys?.size ?? 0,
                        registryRowsRetained: Object.keys(panel?._fields || {}).length,
                        subscriberPresent: [...(window.__ftdRAF?._subs?.keys?.() || [])]
                            .some((id) => id.startsWith('flux-slice-panel')),
                    };
                }

                if (panelId === 'spectrum') {
                    const api = window.__ftdSpectrumPanel;
                    // Direct/manual entry points are part of the no-work
                    // contract, not only the removed coordinator subscription.
                    api?.update();
                    api?.deepMeasure();
                    const message = root.querySelector('.spectrum-inapplicable');
                    const controls = [...root.querySelectorAll('#spectrum-panel-deep, #spectrum-panel-live')];
                    result.spectrumApplicability = {
                        panelStatus: root.querySelector('#spectrum-panel')?.dataset.applicability ?? null,
                        messageStatus: message?.dataset.applicability ?? null,
                        message: message?.textContent?.replace(/\s+/g, ' ').trim() ?? null,
                        messageVisible: !!message && !message.hidden,
                        contentHidden: !!root.querySelector('.spectrum-applicable-content')?.hidden,
                        controlsDisabled: controls.length === 2
                            && controls.every((control) => control.disabled),
                        coordinatorActive: api?.coordinatorActive ?? null,
                        samplerWantsActive: api?.samplerWantsActive ?? null,
                        hasSpectrum: !!api?.lastSpec,
                        subscriberPresent: [...(window.__ftdRAF?._subs?.keys?.() || [])]
                            .some((id) => id.startsWith('spectrum-panel')),
                    };
                }

                if (panelId === 'p1-observables') {
                    const api = window.__ftdP1Panel;
                    const before = api?.updateCount ?? null;
                    // Public/manual entry points must be inert as well as the
                    // removed coordinator subscription.
                    api?.update();
                    const message = root.querySelector('.p1-inapplicable');
                    result.p1ObservablesApplicability = {
                        panelStatus: root.querySelector('#p1-observables-panel')?.dataset.applicability ?? null,
                        messageStatus: message?.dataset.applicability ?? null,
                        message: message?.textContent?.replace(/\s+/g, ' ').trim() ?? null,
                        messageVisible: !!message && !message.hidden,
                        contentHidden: !!root.querySelector('.p1-applicable-content')?.hidden,
                        mountedComponentCount: api?.mountedComponentCount ?? null,
                        sectionCount: root.querySelectorAll('.p1-applicable-content > section').length,
                        scientificControlCount: root.querySelectorAll(
                            '.p1-applicable-content button, .p1-applicable-content input',
                        ).length,
                        coordinatorActive: api?.coordinatorActive ?? null,
                        updateCountStable: before === (api?.updateCount ?? null),
                        subscriberPresent: [...(window.__ftdRAF?._subs?.keys?.() || [])]
                            .some((id) => id.startsWith('p1-observables-panel')),
                    };
                }

                if (panelId === 'knots') {
                    const [
                        { getScale0State, isKnotTrackingActive },
                        { getFieldLineKnotTracker },
                        { getScale0TelemetryDemand },
                    ] = await Promise.all([
                        import('/js/scales/scale0/state/store.js'),
                        import('/js/scales/scale0/runtime/field-line-knots.js'),
                        import('/js/telemetry/demand.js'),
                    ]);
                    const api = window.__ftdKnotsPanel;
                    const before = api?.updateCount ?? null;
                    api?.update();
                    const state = getScale0State();
                    const message = root.querySelector('.knots-inapplicable');
                    result.knotsApplicability = {
                        panelStatus: root.querySelector('#knots-panel')?.dataset.applicability ?? null,
                        messageStatus: message?.dataset.applicability ?? null,
                        message: message?.textContent?.replace(/\s+/g, ' ').trim() ?? null,
                        messageVisible: !!message && !message.hidden,
                        contentHidden: !!root.querySelector('.knots-applicable-content')?.hidden,
                        controlsDisabled: [...root.querySelectorAll(
                            '.knots-applicable-content input, .knots-applicable-content button',
                        )].every((control) => control.disabled),
                        preference: state.knotTracking,
                        scenarioApplicable: state.knotTrackingApplicable,
                        effective: isKnotTrackingActive(state),
                        knotZones: state.fieldFlags.showKnotZones,
                        coordinatorActive: api?.coordinatorActive ?? null,
                        measurementActive: api?.measurementActive ?? null,
                        contributionEnabled: api?.contributionEnabled ?? null,
                        wantAudit: getScale0TelemetryDemand(window.__ftdCtx, state).wantAudit,
                        historyLength: api?.historyLength ?? null,
                        trackerCounts: ['e', 'b', 'flux']
                            .map((field) => getFieldLineKnotTracker(field).getTelemetry().count),
                        updateCountStable: before === (api?.updateCount ?? null),
                        subscriberPresent: [...(window.__ftdRAF?._subs?.keys?.() || [])]
                            .some((id) => id.startsWith('knots-panel')),
                    };
                }

                if (panelId === 'gravity') {
                    const api = window.__ftdGravityPanel;
                    const [{ getScale0State }, { getScale0TelemetryDemand }] = await Promise.all([
                        import('/js/scales/scale0/state/store.js'),
                        import('/js/telemetry/demand.js'),
                    ]);
                    // Empty must remain inert even through public/manual panel
                    // entry points, not only after coordinator removal.
                    api?.update();
                    api?.setKind('force');
                    const message = root.querySelector('.gravity-inapplicable');
                    const controls = [...root.querySelectorAll('.grav-qbtn')];
                    result.gravityApplicability = {
                        panelStatus: root.querySelector('#gravity-panel')?.dataset.applicability ?? null,
                        messageStatus: message?.dataset.applicability ?? null,
                        message: message?.textContent?.replace(/\s+/g, ' ').trim() ?? null,
                        messageVisible: !!message && !message.hidden,
                        contentHidden: !!root.querySelector('.gravity-applicable-content')?.hidden,
                        controlsDisabled: controls.length > 0
                            && controls.every((control) => control.disabled),
                        coordinatorActive: api?.coordinatorActive ?? null,
                        armCoordinatorActive: api?.armCoordinatorActive ?? null,
                        liveCoordinatorActive: api?.liveCoordinatorActive ?? null,
                        samplerWantsActive: api?.samplerWantsActive ?? null,
                        wantGravity: getScale0TelemetryDemand(
                            window.__ftdCtx,
                            getScale0State(),
                        ).wantGravity,
                        hasMetrics: !!api?.lastMetrics,
                        hasCppAggregate: !!api?.lastAgg,
                        historyLength: api?.historyLength ?? null,
                        subscriberPresent: [...(window.__ftdRAF?._subs?.keys?.() || [])]
                            .some((id) => id.startsWith('gravity-panel')),
                    };
                }

                const prefix = subscriberPrefix(panelId);
                const activeResources = resourceSnapshot(root);
                probe.startScale0UiAuditProbe({
                    rootSelector: `#panel-${panelId}`,
                    subscriberPrefixes: [prefix],
                });
                probeRunning = true;
                if (panelId === 'flux-slice') {
                    probe.trackScale0UiMethods('fluxSlice', window.__ftdFluxSlicePanel, [
                        'update', '_buildFrameSampleCache', '_paintSlice',
                    ]);
                }
                // One extra rAF allows the probe loop to establish its first
                // timestamp while still leaving >=240 measured intervals.
                await waitFrames(gates.visibleFrames + 2);
                const active = await probe.stopScale0UiAuditProbe();
                probeRunning = false;
                result.active = { report: active, resources: activeResources };

                dock.setCollapsed(true);
                await wait(100);
                await waitFrames(3);
                const collapsedResources = resourceSnapshot(root);
                probe.startScale0UiAuditProbe({
                    rootSelector: `#panel-${panelId}`,
                    subscriberPrefixes: [prefix],
                });
                probeRunning = true;
                await waitFrames(gates.collapsedFrames + 2);
                const collapsed = await probe.stopScale0UiAuditProbe();
                probeRunning = false;
                result.collapsed = { report: collapsed, resources: collapsedResources };
                result.status = 'qualified';
            } catch (error) {
                result.reason = String(error?.stack || error);
                if (probeRunning) {
                    try { await probe.stopScale0UiAuditProbe(); } catch { /* preserve original failure */ }
                }
            } finally {
                dock.setCollapsed(false);
                await waitFrames(2);
            }
            results.push(result);
        }
        return { registryIds, visibleTabIds, results };
    }, { panelIds: CAMPAIGN_PANELS, gates: GATES });

    await testInfo.attach('scale0-empty-panel-performance-L97.json', {
        body: JSON.stringify({ scenario: EMPTY, latticeSize: LATTICE_SIZE, gates: GATES, ...campaign }, null, 2),
        contentType: 'application/json',
    });

    expect(campaign.registryIds, 'canonical Scale-0 panel registry').toEqual(PANELS);
    expect(campaign.visibleTabIds, 'all and only the 17 canonical Scale-0 tabs are visible')
        .toEqual(PANELS);
    expect(CAMPAIGN_PANELS.every((panel) => PANELS.includes(panel)),
        `unknown FTD_PANEL_PERF_ONLY panel: ${CAMPAIGN_PANELS.join(',')}`).toBe(true);
    expect(campaign.results).toHaveLength(CAMPAIGN_PANELS.length);

    if (nonemptyFluxSlice) {
        expect(nonemptyFluxSlice.scenarioId,
            'Flux Slice nonempty control begins on the normal field scenario').not.toBe(EMPTY);
        expect(nonemptyFluxSlice.applicable,
            'Flux Slice remains applicable for its normal nonempty scenario').toBe(true);
        expect(nonemptyFluxSlice.rows,
            'Flux Slice retains its nonempty field-driver registry').toBeGreaterThan(0);
        expect(nonemptyFluxSlice.canvases,
            'Flux Slice retains three nonempty plane canvases per field row')
            .toBe(nonemptyFluxSlice.rows * 3);
    }

    for (const panel of campaign.results) {
        const label = `${panel.panelId} ${panel.reason || ''}`;
        expect.soft(panel.status, `${label}: panel must be observable`).toBe('qualified');
        if (panel.status !== 'qualified') continue;
        const active = panel.active.report;
        const collapsed = panel.collapsed.report;
        const activeResources = panel.active.resources;
        const collapsedResources = panel.collapsed.resources;

        if (panel.panelId === 'diagnostics') {
            expect.soft(panel.diagnosticsVirtualization.trendHosts,
                `${label}: Diagnostics retains every trend host`).toBeGreaterThan(0);
            expect.soft(panel.diagnosticsVirtualization.bottomVisible,
                `${label}: scrolled bottom trend is rendered`).toBe(true);
            expect.soft(panel.diagnosticsVirtualization.topVisible,
                `${label}: restored top trend is rendered`).toBe(true);
            expect.soft(panel.diagnosticsVirtualization.bottomCanvasCount,
                `${label}: bottom viewport canvas bound`)
                .toBeLessThanOrEqual(GATES.maxRootCanvases);
            expect.soft(panel.diagnosticsVirtualization.topCanvasCount,
                `${label}: top viewport canvas bound`)
                .toBeLessThanOrEqual(GATES.maxRootCanvases);
        }

        if (panel.panelId === 'lagrangian') {
            const audit = panel.lagrangianAudit;
            expect.soft(audit.checkedTerms, `${label}: every enabled scientific term remains visible`)
                .toBeGreaterThan(0);
            expect.soft(audit.termCharts, `${label}: one visible trend per enabled term`)
                .toBe(audit.checkedTerms);
            expect.soft(audit.trendHosts, `${label}: action trends remain available`)
                .toBeGreaterThan(0);
            expect.soft(audit.bottomTrendVisible, `${label}: scrolled action trend renders on demand`)
                .toBe(true);
            expect.soft(audit.bottomCanvasCount, `${label}: scrolled canvas allocation bound`)
                .toBeLessThanOrEqual(GATES.maxRootCanvases);
            expect.soft(audit.topCanvasCount, `${label}: restored canvas allocation bound`)
                .toBeLessThanOrEqual(GATES.maxRootCanvases);
            expect.soft(audit.status, `${label}: null-control observer baseline status`)
                .toBe('supported-null-control');
            expect.soft(audit.actual.status, `${label}: current sample interpretation`)
                .toBe('supported-null-control');
            expect.soft(Number.isFinite(audit.actual.observerBaseline),
                `${label}: observer baseline is a finite published value`).toBe(true);
            expect.soft(audit.actual.excitation, `${label}: empty-control baseline-subtracted excitation`)
                .toBe(0);
            expect.soft(audit.baselineText, `${label}: published baseline is not unavailable`)
                .not.toBe('—');
            expect.soft(audit.excitationText, `${label}: exact zero is preserved in the UI`)
                .toBe('0');
            expect.soft(audit.missing, `${label}: missing telemetry is not synthesized as zero`)
                .toEqual({ status: 'unavailable', observerBaseline: null, excitation: null });
            expect.soft(audit.exactZero, `${label}: measured exact zero remains distinct`)
                .toEqual({ status: 'supported-null-control', observerBaseline: 0, excitation: 0 });
            expect.soft(audit.interpretationText, `${label}: scientific interpretation avoids vacuum claims`)
                .toContain('not physical vacuum energy or zero-point energy');
            expect.soft(audit.bornInfeldLabel, `${label}: Born-Infeld term carries its empty-control role`)
                .toContain('observer baseline in empty control');
            expect.soft(audit.baselineBearingActionLabels,
                `${label}: baseline-bearing aggregate channels are explicit`)
                .toEqual([
                    'Action S (observer baseline included)',
                    'ℒ total (observer baseline included)',
                    'Hamiltonian H (observer baseline included)',
                ]);
        }

        if (panel.panelId === 'flux-slice') {
            const audit = panel.fluxSliceApplicability;
            expect.soft(audit.panelStatus, `${label}: explicit panel applicability state`)
                .toBe('inapplicable-empty');
            expect.soft(audit.messageStatus, `${label}: explicit message applicability state`)
                .toBe('inapplicable');
            expect.soft(audit.message, `${label}: null-control field-domain boundary`)
                .toContain('does not define a field-slice domain');
            expect.soft(audit.message, `${label}: no fabricated vacuum interpretation`)
                .toContain('not a measurement of physical vacuum or zero-point fields');
            expect.soft(audit.rowCount, `${label}: inapplicable field rows are not mounted`).toBe(0);
            expect.soft(audit.canvasCount, `${label}: inapplicable canvases are not mounted`).toBe(0);
            expect.soft(audit.controlsDisabled, `${label}: field-domain controls are disabled`).toBe(true);
            expect.soft(audit.selfDriveStopped, `${label}: panel polling is stopped`).toBe(true);
            expect.soft(audit.subscriberPresent, `${label}: no panel rAF subscription remains`).toBe(false);
            expect.soft(audit.wantedSamplerCount, `${label}: panel sampler demand is released`).toBe(0);
            expect.soft(audit.registryRowsRetained,
                `${label}: nonempty driver registry is retained for later restoration`).toBeGreaterThan(0);
            expect.soft(active.methods, `${label}: no extraction or paint method was called`)
                .toEqual({});
            expect.soft(callbackSampleCount(active), `${label}: no periodic panel callback ran`).toBe(0);
            expect.soft(active.dom.mutationRecords, `${label}: static inapplicable state has no churn`)
                .toBe(0);
            expect.soft(active.dom.canvasDraws, `${label}: inapplicable state performs no draw`)
                .toBe(0);
        }

        if (panel.panelId === 'spectrum') {
            const audit = panel.spectrumApplicability;
            expect.soft(audit.panelStatus, `${label}: explicit panel applicability state`)
                .toBe('inapplicable-empty');
            expect.soft(audit.messageStatus, `${label}: explicit message applicability state`)
                .toBe('inapplicable');
            expect.soft(audit.messageVisible, `${label}: inapplicable explanation is visible`).toBe(true);
            expect.soft(audit.contentHidden, `${label}: live analysis surface is hidden`).toBe(true);
            expect.soft(audit.message, `${label}: null-control spectrum-domain boundary`)
                .toContain('does not define a field or spatial-spectrum domain');
            expect.soft(audit.message, `${label}: no fabricated vacuum interpretation`)
                .toContain('not a measurement of physical vacuum or zero-point fluctuations');
            expect.soft(audit.controlsDisabled, `${label}: live/deep controls are disabled`).toBe(true);
            expect.soft(audit.coordinatorActive, `${label}: spectrum coordinator is stopped`).toBe(false);
            expect.soft(audit.subscriberPresent, `${label}: no spectrum rAF subscription remains`).toBe(false);
            expect.soft(audit.samplerWantsActive, `${label}: spectrum sampler demand is released`).toBe(false);
            expect.soft(audit.hasSpectrum, `${label}: no stale spectrum is presented`).toBe(false);
            expect.soft(callbackSampleCount(active), `${label}: no periodic spectrum callback ran`).toBe(0);
            expect.soft(active.dom.mutationRecords, `${label}: static inapplicable state has no churn`)
                .toBe(0);
            expect.soft(active.dom.canvasDraws, `${label}: inapplicable state performs no draw`)
                .toBe(0);
        }

        if (panel.panelId === 'gravity') {
            const audit = panel.gravityApplicability;
            expect.soft(audit.panelStatus, `${label}: explicit panel applicability state`)
                .toBe('inapplicable-empty');
            expect.soft(audit.messageStatus, `${label}: explicit message applicability state`)
                .toBe('inapplicable');
            expect.soft(audit.messageVisible, `${label}: inapplicable explanation is visible`).toBe(true);
            expect.soft(audit.contentHidden, `${label}: gravity analysis surface is hidden`).toBe(true);
            expect.soft(audit.message, `${label}: null-control gravity-domain boundary`)
                .toContain('does not define a gravity-source, metric, or gravity-proxy domain');
            expect.soft(audit.message, `${label}: no fabricated vacuum interpretation`)
                .toContain('not a measurement of physical vacuum, inert vacuum, zero-point energy');
            expect.soft(audit.controlsDisabled, `${label}: gravity-domain controls are disabled`).toBe(true);
            expect.soft(audit.coordinatorActive, `${label}: gravity arm/live coordinators are stopped`)
                .toBe(false);
            expect.soft(audit.armCoordinatorActive, `${label}: gravity arm coordinator is stopped`)
                .toBe(false);
            expect.soft(audit.liveCoordinatorActive, `${label}: gravity live coordinator is stopped`)
                .toBe(false);
            expect.soft(audit.subscriberPresent, `${label}: no gravity rAF subscription remains`)
                .toBe(false);
            expect.soft(audit.samplerWantsActive, `${label}: gravity sampler demand is released`)
                .toBe(false);
            expect.soft(audit.wantGravity, `${label}: inapplicable Gravity requests no native reduction`)
                .toBe(false);
            expect.soft(audit.hasMetrics, `${label}: no stale proxy metrics are presented`).toBe(false);
            expect.soft(audit.hasCppAggregate, `${label}: no stale C++ aggregate is presented`).toBe(false);
            expect.soft(audit.historyLength, `${label}: no null-control history is retained`).toBe(0);
            expect.soft(callbackSampleCount(active), `${label}: no periodic gravity callback ran`).toBe(0);
            expect.soft(active.dom.mutationRecords, `${label}: static inapplicable state has no churn`)
                .toBe(0);
            expect.soft(active.dom.canvasDraws, `${label}: inapplicable state performs no draw`)
                .toBe(0);
        }

        if (panel.panelId === 'p1-observables') {
            const audit = panel.p1ObservablesApplicability;
            expect.soft(audit.panelStatus, `${label}: explicit panel applicability state`)
                .toBe('inapplicable-empty');
            expect.soft(audit.messageStatus, `${label}: explicit message applicability state`)
                .toBe('inapplicable');
            expect.soft(audit.messageVisible, `${label}: inapplicable explanation is visible`).toBe(true);
            expect.soft(audit.contentHidden, `${label}: experiment surface is hidden`).toBe(true);
            expect.soft(audit.message, `${label}: null-control experiment boundary`)
                .toContain('prepares no source, excitation, material clock');
            expect.soft(audit.message, `${label}: no fabricated vacuum interpretation`)
                .toContain('not a measurement of physical vacuum');
            expect.soft(audit.mountedComponentCount, `${label}: experiment components are unmounted`).toBe(0);
            expect.soft(audit.sectionCount, `${label}: stale experiment cards are removed`).toBe(0);
            expect.soft(audit.scientificControlCount, `${label}: experiment controls are removed`).toBe(0);
            expect.soft(audit.coordinatorActive, `${label}: P1 coordinator is stopped`).toBe(false);
            expect.soft(audit.subscriberPresent, `${label}: no P1 rAF subscription remains`).toBe(false);
            expect.soft(audit.updateCountStable, `${label}: manual update is inert`).toBe(true);
            expect.soft(callbackSampleCount(active), `${label}: no periodic P1 callback ran`).toBe(0);
            expect.soft(active.dom.mutationRecords, `${label}: static inapplicable state has no churn`)
                .toBe(0);
            expect.soft(active.dom.canvasDraws, `${label}: inapplicable state performs no draw`)
                .toBe(0);
        }

        if (panel.panelId === 'knots') {
            const audit = panel.knotsApplicability;
            expect.soft(audit.panelStatus, `${label}: explicit panel applicability state`)
                .toBe('inapplicable-empty');
            expect.soft(audit.messageStatus, `${label}: explicit message applicability state`)
                .toBe('inapplicable');
            expect.soft(audit.messageVisible, `${label}: inapplicable explanation is visible`).toBe(true);
            expect.soft(audit.contentHidden, `${label}: detector surface is hidden`).toBe(true);
            expect.soft(audit.controlsDisabled, `${label}: detector controls are disabled`).toBe(true);
            expect.soft(audit.message, `${label}: null-control detector boundary`)
                .toContain('defines no field-line or streamline-sweep domain');
            expect.soft(audit.message, `${label}: no fabricated topology interpretation`)
                .toContain('not evidence for physical vacuum or topological triviality');
            expect.soft(audit.scenarioApplicable, `${label}: scenario tracking gate is closed`).toBe(false);
            expect.soft(audit.effective, `${label}: retained preference cannot schedule work`).toBe(false);
            expect.soft(audit.knotZones, `${label}: stale knot-zone rendering is disabled`).toBe(false);
            expect.soft(audit.coordinatorActive, `${label}: Knots coordinators are stopped`).toBe(false);
            expect.soft(audit.measurementActive, `${label}: contribution measurement is stopped`).toBe(false);
            expect.soft(audit.contributionEnabled, `${label}: trackers cannot request contributions`).toBe(false);
            expect.soft(audit.wantAudit, `${label}: visible inapplicable panel requests no audit stream`).toBe(false);
            expect.soft(audit.historyLength, `${label}: EM history is cleared`).toBe(0);
            expect.soft(audit.trackerCounts, `${label}: detector histories are cleared`).toEqual([0, 0, 0]);
            expect.soft(audit.updateCountStable, `${label}: manual update is inert`).toBe(true);
            expect.soft(audit.subscriberPresent, `${label}: no Knots rAF subscription remains`).toBe(false);
            expect.soft(callbackSampleCount(active), `${label}: no periodic Knots callback ran`).toBe(0);
            expect.soft(active.dom.mutationRecords, `${label}: static inapplicable state has no churn`)
                .toBe(0);
            expect.soft(active.dom.canvasDraws, `${label}: inapplicable state performs no draw`)
                .toBe(0);
        }

        expect.soft(active.frames.count, `${label}: visible sample adequacy`)
            .toBeGreaterThanOrEqual(GATES.visibleFrames);
        expect.soft(active.frames.effectiveFps, `${label}: visible effective FPS`)
            .toBeGreaterThanOrEqual(GATES.minFps);
        expect.soft(active.frames.p95Ms, `${label}: visible p95 frame interval`)
            .toBeLessThanOrEqual(GATES.p95Ms);
        expect.soft(active.frames.p99Ms, `${label}: visible p99 frame interval`)
            .toBeLessThanOrEqual(GATES.p99Ms);
        expect.soft(active.longTaskSupported, `${label}: Long Tasks API must be observable`).toBe(true);
        expect.soft(active.longTasks, `${label}: visible long tasks`).toEqual([]);
        expect.soft(active.errors, `${label}: visible page/probe errors`).toEqual([]);

        expect.soft(activeResources.rootDomNodes, `${label}: absolute panel DOM bound`)
            .toBeLessThanOrEqual(GATES.maxRootDomNodes);
        expect.soft(activeResources.rootCanvases, `${label}: absolute panel canvas bound`)
            .toBeLessThanOrEqual(GATES.maxRootCanvases);
        expect.soft(activeResources.coordinatorSubscribers, `${label}: absolute coordinator bound`)
            .toBeLessThanOrEqual(GATES.maxCoordinatorSubscribers);
        expect.soft(active.dom.mutationRecords, `${label}: visible DOM work bound`)
            .toBeLessThanOrEqual(GATES.maxVisibleMutations);
        expect.soft(active.dom.canvasDraws, `${label}: visible canvas work bound`)
            .toBeLessThanOrEqual(GATES.maxVisibleCanvasDraws);
        expect.soft(active.resourceDelta.rafSubscribers, `${label}: visible coordinator stability`).toBe(0);
        expect.soft(active.resourceDelta.domNodes, `${label}: visible DOM allocation stability`).toBe(0);
        expect.soft(active.resourceDelta.canvases, `${label}: visible canvas allocation stability`).toBe(0);
        if (activeResources.heapObserved) {
            expect.soft(active.resourceDelta.heapBytes, `${label}: visible heap-growth proxy`)
                .toBeLessThanOrEqual(GATES.maxHeapGrowthBytes);
        }
        expect.soft(callbackWorst(active, 'p99Ms'), `${label}: panel coordinator callback p99`)
            .toBeLessThanOrEqual(GATES.maxCallbackP99Ms);

        expect.soft(collapsed.frames.count, `${label}: collapsed sample adequacy`)
            .toBeGreaterThanOrEqual(GATES.collapsedFrames);
        expect.soft(collapsed.frames.effectiveFps, `${label}: collapsed effective FPS`)
            .toBeGreaterThanOrEqual(GATES.minFps);
        expect.soft(collapsed.frames.p95Ms, `${label}: collapsed p95 frame interval`)
            .toBeLessThanOrEqual(GATES.p95Ms);
        expect.soft(collapsed.frames.p99Ms, `${label}: collapsed p99 frame interval`)
            .toBeLessThanOrEqual(GATES.p99Ms);
        expect.soft(collapsed.longTaskSupported, `${label}: collapsed Long Tasks API observable`).toBe(true);
        expect.soft(collapsed.longTasks, `${label}: collapsed long tasks`).toEqual([]);
        expect.soft(collapsed.errors, `${label}: collapsed page/probe errors`).toEqual([]);
        expect.soft(collapsed.dom.mutationRecords, `${label}: collapsed panel DOM work stops`).toBe(0);
        expect.soft(collapsed.dom.canvasDraws, `${label}: collapsed panel canvas work stops`).toBe(0);
        expect.soft(collapsed.resourceDelta.rafSubscribers, `${label}: collapsed coordinator stability`).toBe(0);
        expect.soft(collapsed.resourceDelta.domNodes, `${label}: collapsed DOM allocation stability`).toBe(0);
        expect.soft(collapsed.resourceDelta.canvases, `${label}: collapsed canvas allocation stability`).toBe(0);
        expect.soft(callbackWorst(collapsed, 'maxMs'), `${label}: collapsed callback is guard-only`)
            .toBeLessThanOrEqual(GATES.maxCollapsedCallbackMs);

        const activeWork = active.dom.mutationRecords
            + active.dom.canvasDraws
            + callbackSampleCount(active);
        const workClass = activeWork > 0 ? 'dynamic-work-observed' : 'static-zero-periodic-work';
        console.log(`[empty-panel-perf] ${panel.panelId} ${workClass} `
            + `fps=${active.frames.effectiveFps.toFixed(2)} p95=${active.frames.p95Ms.toFixed(2)}ms `
            + `p99=${active.frames.p99Ms.toFixed(2)}ms dom=${active.dom.mutationRecords} `
            + `canvas=${active.dom.canvasDraws} root-canvases=${activeResources.rootCanvases} `
            + `collapsed-dom=${collapsed.dom.mutationRecords} `
            + `collapsed-canvas=${collapsed.dom.canvasDraws}`);
        expect.soft(collapsedResources.rootDomNodes, `${label}: collapsed root remains allocation-bounded`)
            .toBeLessThanOrEqual(GATES.maxRootDomNodes);
    }

    testInfo.annotations.push({
        type: 'performance-scope',
        description: `absolute foreground Chromium panel campaign (${CAMPAIGN_PANELS.join(',')}), empty L=${LATTICE_SIZE}, WasmBridgeProxy only; native GPU/WebSocket parity not asserted`,
    });
    testInfo.annotations.push({
        type: 'collapse-semantics',
        description: 'collapsed work-stop is panel-root DOM/canvas work plus bounded guard-only coordinator callbacks; the viewport and worker remain live by design',
    });
    expect(realErrors(consoleErrors)).toEqual([]);
});
